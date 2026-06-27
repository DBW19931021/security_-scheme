#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import Optional, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.workflow_lib import load_json


def markdown_link(label: str, path: Path, output: Path) -> str:
    relative = os.path.relpath(str(path.resolve()), str(output.parent.resolve()))
    return "[%s](%s)" % (label, Path(relative).as_posix())


def build_pack(project_root: Path, output: Path) -> None:
    project_root = Path(project_root)
    output = Path(output)
    config = load_json(project_root / "workflow-project.json")
    chip = config["chip"]
    openspec_root = Path(config["openspec_root"])
    changes_root = openspec_root / "changes"
    active_changes = sorted(
        path
        for path in changes_root.glob("*")
        if path.is_dir() and path.name != "archive"
    )
    sections = [
        (
            "30 分钟：理解项目",
            [
                "00_project/project_profile.md",
                "00_project/status_dashboard.md",
                "00_project/architecture_overview.md",
                "00_project/capability_map.md",
            ],
        ),
        (
            "半天：理解设计依据",
            [
                "security_inputs/inputs_manifest.md",
                "security_workflow/00_threat_model.md",
                "security_workflow/01_constraints.md",
                "security_workflow/02_baseline.md",
                "00_project/decision_log.md",
                "security_workflow/06_traceability.md",
            ],
        ),
    ]

    lines = [
        "# %s Security Project Onboarding" % chip,
        "",
        "本阅读包只提供路径和接手顺序，不复制安全设计正文。",
        "",
    ]
    for heading, relatives in sections:
        lines.extend(["## %s" % heading, ""])
        for relative in relatives:
            path = project_root / relative
            if path.is_file():
                lines.append(
                    "- %s" % markdown_link(Path(relative).name, path, output)
                )
        lines.append("")

    lines.extend(
        [
            "## 一到两天：接手首个 Change",
            "",
            "阅读顺序：proposal → spec → design → tasks → evidence → "
            "retrospective → code/tests。",
            "",
        ]
    )
    if active_changes:
        for change in active_changes:
            proposal = change / "proposal.md"
            link_path = proposal if proposal.is_file() else change
            lines.append(
                "- %s: %s"
                % (
                    change.name,
                    markdown_link("proposal", link_path, output),
                )
            )
    else:
        lines.append("- 当前没有活动中的 OpenSpec change。")
    lines.append("")

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate a progressive chip-security onboarding pack."
    )
    parser.add_argument("--project-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    build_pack(args.project_root, args.output)
    print("Created onboarding pack: %s" % args.output.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
