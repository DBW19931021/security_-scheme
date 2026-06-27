#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import List, Optional, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.workflow_lib import (
    CheckResult,
    all_tasks_complete,
    approval_result,
    load_json,
    render_results,
    require_file,
    require_headings,
    require_json_fields,
    result_exit_code,
)


CHANGE_GATES = ("G2", "G3", "G4", "G5")


def _has_text(path: Path, terms: Sequence[str], code: str) -> CheckResult:
    if not path.is_file():
        return CheckResult(code, "FAIL", "file is missing", str(path))
    text = path.read_text(encoding="utf-8").lower()
    if any(term.lower() in text for term in terms):
        return CheckResult(code, "PASS", "required section is present", str(path))
    return CheckResult(code, "FAIL", "required section is missing", str(path))


def _baseline_reference_result(path: Path) -> CheckResult:
    if not path.is_file():
        return CheckResult(
            "BASELINE_REFS", "FAIL", "security metadata is missing", str(path)
        )
    try:
        data = load_json(path)
    except (OSError, ValueError) as error:
        return CheckResult(
            "BASELINE_REFS", "FAIL", "invalid JSON: %s" % error, str(path)
        )
    references = data.get("baseline_refs", {})
    populated = any(
        references.get(key)
        for key in (
            "sources",
            "threats",
            "constraints",
            "decisions",
            "change_requests",
            "open_questions",
        )
    )
    return CheckResult(
        "BASELINE_REFS",
        "PASS" if populated else "FAIL",
        "at least one chip-level baseline reference is recorded",
        str(path),
    )


def _g2_checks(change_root: Path) -> List[CheckResult]:
    metadata = change_root / "security-workflow.json"
    specs = sorted((change_root / "specs").glob("*/spec.md"))
    results = [
        require_file(change_root / "proposal.md", "PROPOSAL"),
        require_file(change_root / "design.md", "DESIGN"),
        CheckResult(
            "SPEC",
            "PASS" if specs else "FAIL",
            "at least one capability delta spec exists",
            str(change_root / "specs"),
        ),
        require_file(metadata, "SECURITY_METADATA"),
    ]
    results.extend(
        require_json_fields(
            metadata,
            (
                "change_id",
                "chip",
                "owners.feature_owner",
                "owners.verification_owner",
                "owners.reviewer",
            ),
            "SECURITY_METADATA",
        )
    )
    results.append(_baseline_reference_result(metadata))
    return results


def evaluate_change_gate(
    change_root: Path, gate: str
) -> List[CheckResult]:
    change_root = Path(change_root)
    if gate not in CHANGE_GATES:
        return [
            CheckResult(
                "GATE", "FAIL", "unsupported change Gate: %s" % gate
            )
        ]

    results = _g2_checks(change_root)
    if gate in ("G3", "G4", "G5"):
        tasks_path = change_root / "tasks.md"
        results.extend(
            [
                require_file(tasks_path, "TASKS"),
                _has_text(
                    change_root / "design.md",
                    ("testing", "test strategy", "测试"),
                    "DESIGN_TESTING",
                ),
                _has_text(
                    change_root / "design.md",
                    ("risk", "trade-off", "风险"),
                    "DESIGN_RISK",
                ),
            ]
        )
        if tasks_path.is_file():
            task_text = tasks_path.read_text(encoding="utf-8")
            results.append(
                CheckResult(
                    "TASK_CHECKBOX",
                    "PASS"
                    if "- [ ]" in task_text or "- [x]" in task_text
                    else "FAIL",
                    "tasks contain at least one checkbox",
                    str(tasks_path),
                )
            )

    if gate in ("G4", "G5"):
        tasks_path = change_root / "tasks.md"
        complete = (
            tasks_path.is_file()
            and all_tasks_complete(tasks_path.read_text(encoding="utf-8"))
        )
        results.append(
            CheckResult(
                "TASKS_COMPLETE",
                "PASS" if complete else "FAIL",
                "all implementation tasks are complete",
                str(tasks_path),
            )
        )
        results.extend(
            require_headings(
                change_root / "evidence/verification.md",
                (
                    "## Environment",
                    "## Commands and Results",
                    "## Requirement Coverage",
                    "## Unverified Items",
                    "## Residual Risks",
                ),
                "VERIFICATION",
            )
        )
        results.extend(
            require_headings(
                change_root / "evidence/code-map.md",
                ("# Code and Test Map", "| Requirement |"),
                "CODE_MAP",
            )
        )

    if gate == "G5":
        results.extend(
            require_headings(
                change_root / "evidence/known-issues.md",
                ("## Open Issues", "## Accepted Risks", "## Closed Issues"),
                "KNOWN_ISSUES",
            )
        )
        results.extend(
            require_headings(
                change_root / "retrospective.md",
                (
                    "## Delivered Scope",
                    "## Design Deviations",
                    "## Problems and Root Causes",
                    "## Rejected Approaches",
                    "## Effective Practices",
                    "## Ineffective Practices",
                    "## Known Issues and Debt",
                    "## Reusable Knowledge Candidates",
                    "## Baseline and Documentation Follow-up",
                ),
                "RETROSPECTIVE",
            )
        )

    results.append(approval_result(change_root / "approvals.json", gate))
    return results


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Evaluate OpenSpec security evidence Gates G2-G5."
    )
    parser.add_argument("--change-root", required=True, type=Path)
    parser.add_argument("--gate", required=True, choices=CHANGE_GATES)
    parser.add_argument("--json", action="store_true")
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    results = evaluate_change_gate(args.change_root, args.gate)
    print(render_results(results, as_json=args.json))
    return result_exit_code(results)


if __name__ == "__main__":
    raise SystemExit(main())
