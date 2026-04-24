#!/usr/bin/env python3
"""
Render Mermaid blocks in a Markdown specification to PNG and export the result
to PDF via pandoc.

This keeps the source document in pure Markdown while ensuring Mermaid diagrams
show up as rendered figures in the final PDF instead of fenced code blocks.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path


MERMAID_BLOCK_RE = re.compile(r"```mermaid\s*\n(.*?)\n```", re.DOTALL)


def run(cmd: list[str], **kwargs) -> None:
    subprocess.run(cmd, check=True, **kwargs)


def require_tool(name: str, explicit_path: str | None = None) -> str:
    if explicit_path:
        return explicit_path
    found = shutil.which(name)
    if not found:
        raise SystemExit(
            f"missing required tool: {name}\n"
            f"hint: install it first, or pass --{name.replace('-', '_')} with an explicit path"
        )
    return found


def build_css(css_path: Path) -> None:
    css_path.write_text(
        textwrap.dedent(
            """
            @page {
              size: A4;
              margin: 18mm 16mm 18mm 16mm;
            }

            html {
              font-size: 10.5pt;
            }

            body {
              font-family: "Noto Sans CJK SC", "Source Han Sans SC", "PingFang SC", "Microsoft YaHei", sans-serif;
              color: #1f2328;
              line-height: 1.55;
            }

            h1, h2, h3, h4 {
              color: #111827;
              page-break-after: avoid;
            }

            h1 {
              font-size: 20pt;
              border-bottom: 1px solid #d0d7de;
              padding-bottom: 0.2em;
            }

            h2 {
              font-size: 15pt;
              margin-top: 1.2em;
              border-left: 4px solid #6b7280;
              padding-left: 0.5em;
            }

            h3 {
              font-size: 12pt;
            }

            table {
              width: 100%;
              border-collapse: collapse;
              font-size: 9pt;
              margin: 0.6em 0 1em 0;
            }

            th, td {
              border: 1px solid #d0d7de;
              padding: 6px 8px;
              vertical-align: top;
            }

            th {
              background: #f3f4f6;
            }

            img {
              display: block;
              margin: 0.7em auto 1em auto;
              max-width: 100%;
              height: auto;
            }

            pre, code {
              font-family: "DejaVu Sans Mono", "Noto Sans Mono CJK SC", monospace;
            }
            """
        ).strip()
        + "\n",
        encoding="utf-8",
    )


def render_mermaid_images(
    text: str,
    workdir: Path,
    mmdc: str,
    chrome_config: Path | None,
    image_ext: str,
) -> str:
    mermaid_dir = workdir / "mermaid"
    mermaid_dir.mkdir(parents=True, exist_ok=True)

    counter = 0

    def replace(match: re.Match[str]) -> str:
        nonlocal counter
        counter += 1
        stem = f"diagram_{counter:03d}"
        mmd_path = mermaid_dir / f"{stem}.mmd"
        img_path = mermaid_dir / f"{stem}.{image_ext}"
        mmd_path.write_text(match.group(1).strip() + "\n", encoding="utf-8")

        cmd = [mmdc, "-i", str(mmd_path), "-o", str(img_path), "-b", "transparent"]
        if chrome_config:
            cmd.extend(["-p", str(chrome_config)])
        run(cmd)
        return f"![图 {counter}]({mermaid_dir.name}/{img_path.name})"

    replaced = MERMAID_BLOCK_RE.sub(replace, text)
    if counter == 0:
        raise SystemExit("no mermaid blocks found in input markdown")
    return replaced


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_markdown", help="source Markdown file")
    parser.add_argument("output_pdf", help="target PDF file")
    parser.add_argument(
        "--workdir",
        help="temporary working directory for rendered images and exported markdown",
        default="/tmp/ngu800_security_export_runtime",
    )
    parser.add_argument("--title", help="override PDF title metadata")
    parser.add_argument("--pandoc", help="pandoc executable path")
    parser.add_argument("--mmdc", help="mermaid-cli executable path")
    parser.add_argument(
        "--chrome-config",
        help="puppeteer config JSON path passed to mermaid-cli",
        default="tools/puppeteer_mermaid_config.json",
    )
    parser.add_argument(
        "--image-ext",
        choices=["png", "svg"],
        default="png",
        help="diagram image format used before pandoc export; png is safer for PDF text preservation",
    )
    args = parser.parse_args()

    input_path = Path(args.input_markdown).resolve()
    output_pdf = Path(args.output_pdf).resolve()
    workdir = Path(args.workdir).resolve()
    workdir.mkdir(parents=True, exist_ok=True)
    output_pdf.parent.mkdir(parents=True, exist_ok=True)

    pandoc = require_tool("pandoc", args.pandoc)
    mmdc = require_tool("mmdc", args.mmdc)

    chrome_config = None
    if args.chrome_config:
        config_path = Path(args.chrome_config)
        if not config_path.is_absolute():
            config_path = Path.cwd() / config_path
        if config_path.exists():
            chrome_config = config_path.resolve()

    source_text = input_path.read_text(encoding="utf-8")
    export_text = render_mermaid_images(
        text=source_text,
        workdir=workdir,
        mmdc=mmdc,
        chrome_config=chrome_config,
        image_ext=args.image_ext,
    )

    export_md = workdir / f"{input_path.stem}.export.md"
    export_md.write_text(export_text, encoding="utf-8")

    css_path = workdir / "spec.css"
    build_css(css_path)

    title = args.title if args.title else input_path.stem
    env = os.environ.copy()
    env.setdefault("TMPDIR", str(workdir))

    cmd = [
        pandoc,
        export_md.name,
        "--from",
        "gfm",
        "--resource-path",
        ".",
        "--metadata",
        f"pagetitle={title}",
        "--toc",
        "--css",
        css_path.name,
        "--pdf-engine=weasyprint",
        "-o",
        str(output_pdf),
    ]
    run(cmd, env=env, cwd=workdir)
    print(f"exported_pdf={output_pdf}")
    print(f"export_markdown={export_md}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
