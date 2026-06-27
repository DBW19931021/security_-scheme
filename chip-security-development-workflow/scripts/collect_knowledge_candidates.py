#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import List, Optional, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.workflow_lib import load_json


def collect_candidates(
    change_root: Path, output_root: Path
) -> List[Path]:
    change_root = Path(change_root)
    output_root = Path(output_root)
    source = change_root / "evidence/knowledge-candidates.json"
    data = load_json(source)
    candidates = data.get("candidates", [])
    identifiers = [item.get("id") for item in candidates]
    if any(not identifier for identifier in identifiers):
        raise ValueError("every knowledge candidate requires an id")
    if len(identifiers) != len(set(identifiers)):
        raise ValueError("knowledge candidate ids must be unique")

    output_root.mkdir(parents=True, exist_ok=True)
    written = []
    for source_candidate in candidates:
        candidate = dict(source_candidate)
        candidate["source_change"] = str(change_root.resolve())
        approval = dict(candidate.get("approval", {}))
        if approval.get("status") == "approved" and not approval.get(
            "approved_by"
        ):
            raise ValueError("approved candidate requires approved_by")
        candidate["approval"] = approval
        destination = output_root / ("%s.json" % candidate["id"])
        if destination.exists():
            existing = load_json(destination)
            if existing != candidate:
                raise ValueError(
                    "candidate id already exists with different content: %s"
                    % candidate["id"]
                )
        destination.write_text(
            json.dumps(candidate, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        written.append(destination)
    return written


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Collect knowledge candidates from an OpenSpec change."
    )
    parser.add_argument("--change-root", required=True, type=Path)
    parser.add_argument("--output-root", required=True, type=Path)
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    paths = collect_candidates(args.change_root, args.output_root)
    for path in paths:
        print(path.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
