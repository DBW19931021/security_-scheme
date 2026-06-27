#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import List, Optional, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.workflow_lib import (
    CheckResult,
    approval_result,
    load_json,
    render_results,
    require_file,
    require_json_fields,
    result_exit_code,
)


PROJECT_GATES = tuple("G%d" % value for value in range(7))


def _g0_checks(project_root: Path) -> List[CheckResult]:
    config = project_root / "workflow-project.json"
    results = [
        require_file(config, "PROJECT_CONFIG"),
        require_file(project_root / "approvals.json", "APPROVALS"),
        require_file(
            project_root / "00_project/project_profile.md", "PROJECT_PROFILE"
        ),
        require_file(
            project_root / "00_project/roles_and_approvals.md", "ROLES"
        ),
        require_file(
            project_root / "security_inputs/inputs_manifest.md",
            "INPUTS_MANIFEST",
        ),
    ]
    results.extend(
        require_json_fields(
            config,
            (
                "chip",
                "owners.security_owner",
                "owners.architecture_owner",
                "owners.verification_owner",
                "owners.skill_maintainer",
            ),
            "PROJECT_CONFIG",
        )
    )
    return results


def evaluate_project_gate(
    project_root: Path,
    gate: str,
    change_root: Optional[Path] = None,
    candidate: Optional[Path] = None,
) -> List[CheckResult]:
    project_root = Path(project_root)
    if gate not in PROJECT_GATES:
        return [CheckResult("GATE", "FAIL", "unsupported Gate: %s" % gate)]

    if gate in ("G2", "G3", "G4", "G5"):
        if change_root is None:
            return [
                CheckResult(
                    "CHANGE_ROOT",
                    "FAIL",
                    "%s requires --change-root" % gate,
                )
            ]
        from scripts.check_openspec_evidence import evaluate_change_gate

        return evaluate_change_gate(Path(change_root), gate)

    if gate == "G6":
        if candidate is None:
            return [
                CheckResult(
                    "CANDIDATE", "FAIL", "G6 requires --candidate"
                )
            ]
        from scripts.evaluate_promotion import evaluate_candidate

        try:
            data = load_json(Path(candidate))
        except (OSError, ValueError) as error:
            return [
                CheckResult(
                    "CANDIDATE",
                    "FAIL",
                    "cannot load candidate: %s" % error,
                    str(candidate),
                )
            ]
        return evaluate_candidate(data)

    results = _g0_checks(project_root)
    if gate == "G1":
        for relative, code in (
            ("security_workflow/00_threat_model.md", "THREAT_MODEL"),
            ("security_workflow/01_constraints.md", "CONSTRAINTS"),
            ("security_workflow/02_baseline.md", "BASELINE"),
            ("00_project/decision_log.md", "DECISION_LOG"),
            ("00_project/open_questions.md", "OPEN_QUESTIONS"),
        ):
            results.append(require_file(project_root / relative, code))
    results.append(approval_result(project_root / "approvals.json", gate))
    return results


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Evaluate chip security workflow Gates G0-G6."
    )
    parser.add_argument("--project-root", required=True, type=Path)
    parser.add_argument("--gate", required=True, choices=PROJECT_GATES)
    parser.add_argument("--change-root", type=Path)
    parser.add_argument("--candidate", type=Path)
    parser.add_argument("--json", action="store_true")
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    results = evaluate_project_gate(
        args.project_root, args.gate, args.change_root, args.candidate
    )
    print(render_results(results, as_json=args.json))
    return result_exit_code(results)


if __name__ == "__main__":
    raise SystemExit(main())
