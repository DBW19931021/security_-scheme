#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, Optional, Sequence


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TEMPLATES_ROOT = ROOT / "templates/chip-project"


def render_text(text: str, replacements: Dict[str, str]) -> str:
    for key, value in replacements.items():
        text = text.replace("{{%s}}" % key, value)
    return text


def initialize_project(
    templates_root: Path,
    target: Path,
    chip: str,
    code_repository_root: str,
    openspec_root: str,
    security_owner: str,
    architecture_owner: str,
    verification_owner: str,
    skill_maintainer: str,
) -> None:
    if target.exists() and any(target.iterdir()):
        raise ValueError("target directory is not empty: %s" % target)
    target.mkdir(parents=True, exist_ok=True)
    replacements = {
        "CHIP_NAME": chip,
        "SECURITY_PROJECT_ROOT": str(target.resolve()),
        "CODE_REPOSITORY_ROOT": code_repository_root,
        "OPENSPEC_ROOT": openspec_root,
        "SECURITY_OWNER": security_owner,
        "ARCHITECTURE_OWNER": architecture_owner,
        "VERIFICATION_OWNER": verification_owner,
        "SKILL_MAINTAINER": skill_maintainer,
    }
    for source in templates_root.rglob("*"):
        if not source.is_file():
            continue
        relative = source.relative_to(templates_root)
        if relative.suffix == ".tmpl":
            relative = relative.with_suffix("")
        destination = target / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        text = source.read_text(encoding="utf-8")
        destination.write_text(
            render_text(text, replacements), encoding="utf-8"
        )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Initialize an unapproved chip security workflow project."
    )
    parser.add_argument("--target", required=True, type=Path)
    parser.add_argument("--chip", required=True)
    parser.add_argument("--code-repository-root", required=True)
    parser.add_argument("--openspec-root", required=True)
    parser.add_argument("--security-owner", required=True)
    parser.add_argument("--architecture-owner", required=True)
    parser.add_argument("--verification-owner", required=True)
    parser.add_argument("--skill-maintainer", required=True)
    parser.add_argument(
        "--templates-root", type=Path, default=DEFAULT_TEMPLATES_ROOT
    )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    initialize_project(
        templates_root=args.templates_root,
        target=args.target,
        chip=args.chip,
        code_repository_root=args.code_repository_root,
        openspec_root=args.openspec_root,
        security_owner=args.security_owner,
        architecture_owner=args.architecture_owner,
        verification_owner=args.verification_owner,
        skill_maintainer=args.skill_maintainer,
    )
    print("Created unapproved security project: %s" % args.target.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
