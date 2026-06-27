#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.workflow_lib import (
    CheckResult,
    load_json,
    render_results,
    result_exit_code,
)


def _condition(
    code: str, passed: bool, message: str
) -> CheckResult:
    return CheckResult(code, "PASS" if passed else "FAIL", message)


def evaluate_candidate(candidate: Dict[str, Any]) -> List[CheckResult]:
    target = candidate.get("target_level", "")
    if target in ("L4", "L5"):
        return [
            CheckResult(
                "PROMOTION_SCOPE",
                "REVIEW",
                "target level requires the later cross-chip/Skill "
                "validation plan",
            )
        ]
    if target not in ("L1", "L2", "L3"):
        return [
            CheckResult(
                "TARGET_LEVEL",
                "FAIL",
                "V1 evaluates target levels L1-L3 only",
            )
        ]

    results = [
        _condition(
            "SOURCE_CHANGE",
            bool(candidate.get("source_change")),
            "source change is recorded",
        ),
        _condition(
            "EVIDENCE",
            bool(candidate.get("evidence")),
            "at least one evidence link is recorded",
        )
    ]
    if target in ("L2", "L3"):
        results.extend(
            [
                _condition(
                    "OCCURRENCES",
                    int(candidate.get("occurrences", 0)) >= 2,
                    "candidate has at least two occurrences",
                ),
                _condition(
                    "VALIDATED_CHIPS",
                    bool(candidate.get("validated_chips")),
                    "at least one validated chip is recorded",
                ),
            ]
        )
    if target == "L3":
        results.extend(
            [
                _condition(
                    "APPLICABILITY",
                    bool(candidate.get("applicability")),
                    "applicability is defined",
                ),
                _condition(
                    "NON_APPLICABILITY",
                    bool(candidate.get("non_applicability")),
                    "non-applicability is defined",
                ),
                _condition(
                    "COUNTEREXAMPLE",
                    bool(candidate.get("counterexamples")),
                    "at least one counterexample is recorded",
                ),
                _condition(
                    "CHIP_NEUTRAL",
                    not candidate.get("chip_specific_tokens"),
                    "chip-specific names, addresses, and paths are removed",
                ),
            ]
        )

    if any(item.status == "FAIL" for item in results):
        return results
    approval = candidate.get("approval", {})
    approved = (
        approval.get("status") == "approved"
        and bool(approval.get("approved_by"))
    )
    results.append(
        CheckResult(
            "HUMAN_APPROVAL",
            "PASS" if approved else "REVIEW",
            (
                "promotion approval is recorded"
                if approved
                else "promotion requires human approval"
            ),
        )
    )
    return results


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Evaluate a knowledge candidate for L1-L3 readiness."
    )
    parser.add_argument("--candidate", required=True, type=Path)
    parser.add_argument("--json", action="store_true")
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    results = evaluate_candidate(load_json(args.candidate))
    print(render_results(results, as_json=args.json))
    return result_exit_code(results)


if __name__ == "__main__":
    raise SystemExit(main())
