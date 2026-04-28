#!/usr/bin/env python3
"""Lightweight Change Request synchronization checker.

Usage:
    python3 tools/check_cr_sync.py change_requests/CR-YYYYMMDD-topic.md
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CHANGELOG = ROOT / "00_project" / "changelog.md"
DECISION_LOG = ROOT / "00_project" / "decision_log.md"


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        return ""


def extract_cr_id(cr_path: Path, cr_text: str) -> str:
    match = re.search(r"\bCR-\d{8}-[A-Za-z0-9_-]+\b", cr_text)
    if match:
        return match.group(0)
    match = re.search(r"\bCR-\d{8}-[A-Za-z0-9_-]+\b", cr_path.name)
    if match:
        return match.group(0)
    return cr_path.stem


def has_affected_files(cr_text: str) -> bool:
    patterns = [
        r"##\s*5\.\s*受影响文件",
        r"受影响文件",
        r"Affected Files",
        r"Files Changed",
    ]
    if not any(re.search(p, cr_text, re.IGNORECASE) for p in patterns):
        return False

    # Basic check: at least one Markdown-style table row or path-looking token
    # appears after the affected-files section.
    affected_section = re.split(r"受影响文件|Affected Files|Files Changed", cr_text, maxsplit=1, flags=re.IGNORECASE)
    tail = affected_section[1] if len(affected_section) > 1 else cr_text
    return bool(re.search(r"`[^`]+`|security_workflow/|03_detailed_design|04_impl_design|\.md\b", tail))


def find_unclosed_keywords(text: str) -> list[str]:
    keywords = []
    for pattern in [r"\[TBD\]", r"\bTBD\b", r"待补", r"未关闭", r"\[OPEN\]"]:
        if re.search(pattern, text):
            keywords.append(pattern)
    return keywords


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("Usage: python3 tools/check_cr_sync.py <CR file path>")
        return 2

    cr_path = Path(argv[1])
    if not cr_path.is_absolute():
        cr_path = ROOT / cr_path

    cr_text = read_text(cr_path)
    if not cr_text:
        print(f"[FAIL] CR file not found or empty: {cr_path}")
        return 1

    cr_id = extract_cr_id(cr_path, cr_text)
    changelog_text = read_text(CHANGELOG)
    decision_log_text = read_text(DECISION_LOG)

    checks = []
    checks.append(("CR contains affected files", has_affected_files(cr_text)))
    checks.append(("changelog.md contains CR ID", cr_id in changelog_text))
    checks.append(("decision_log.md contains CR ID or related decision", cr_id in decision_log_text or "Related Decision ID" in cr_text))

    unclosed = find_unclosed_keywords(cr_text)

    print(f"CR sync check: {cr_id}")
    print(f"CR path: {cr_path}")
    print()

    failed = False
    for name, ok in checks:
        status = "PASS" if ok else "WARN"
        print(f"[{status}] {name}")
        if not ok:
            failed = True

    if unclosed:
        print(f"[WARN] CR contains unclosed keywords: {', '.join(unclosed)}")
        failed = True
    else:
        print("[PASS] No unclosed TBD/open keywords found in CR text")

    print()
    if failed:
        print("Result: WARN - please review the warnings above.")
        return 1

    print("Result: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

