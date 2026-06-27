import json
import tempfile
import unittest
from pathlib import Path

from scripts.check_openspec_evidence import evaluate_change_gate


class OpenSpecEvidenceTest(unittest.TestCase):
    def make_change(self, root: Path, complete_tasks: bool = False) -> None:
        files = {
            "proposal.md": (
                "## Why\n\n## What Changes\n\n## Capabilities\n\n## Impact\n"
            ),
            "design.md": (
                "## Context\n\n## Goals / Non-Goals\n\n## Decisions\n\n"
                "## Testing Strategy\n\n## Risks / Trade-offs\n"
            ),
            "tasks.md": "- [x] done\n" if complete_tasks else "- [ ] open\n",
            "specs/demo/spec.md": (
                "## ADDED Requirements\n\n"
                "### Requirement: DEMO-REQ-001 Demo\n\n"
                "#### Scenario: Demo\n"
                "- **WHEN** input\n- **THEN** output\n"
            ),
            "evidence/verification.md": (
                "# Verification Evidence\n## Environment\n"
                "## Commands and Results\n## Requirement Coverage\n"
                "## Unverified Items\n## Residual Risks\n"
            ),
            "evidence/code-map.md": (
                "# Code and Test Map\n\n"
                "| Requirement | Design | Task | Code | Test | Evidence |\n"
                "|---|---|---|---|---|---|\n"
                "| DEMO-REQ-001 | design | 1.1 | a.c | test_a | verification |\n"
            ),
            "evidence/known-issues.md": (
                "# Known Issues\n## Open Issues\n## Accepted Risks\n"
                "## Closed Issues\n"
            ),
            "retrospective.md": (
                "# Retrospective\n## Delivered Scope\n## Design Deviations\n"
                "## Problems and Root Causes\n## Rejected Approaches\n"
                "## Effective Practices\n## Ineffective Practices\n"
                "## Known Issues and Debt\n## Reusable Knowledge Candidates\n"
                "## Baseline and Documentation Follow-up\n"
            ),
        }
        for relative, text in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
        (root / "security-workflow.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "change_id": "demo",
                    "chip": "demo",
                    "baseline_refs": {
                        "sources": ["SRC-001"],
                        "threats": ["THR-001"],
                        "constraints": ["C-001"],
                        "decisions": ["DEC-001"],
                        "change_requests": [],
                        "open_questions": [],
                    },
                    "owners": {
                        "feature_owner": "Alice",
                        "verification_owner": "Bob",
                        "reviewer": "Carol",
                    },
                }
            ),
            encoding="utf-8",
        )
        gates = {}
        for gate in ["G2", "G3", "G4", "G5"]:
            gates[gate] = {
                "status": "approved",
                "approvers": ["Owner"],
                "date": "2026-06-09",
                "evidence": "review",
            }
        (root / "approvals.json").write_text(
            json.dumps({"schema_version": 1, "gates": gates}),
            encoding="utf-8",
        )

    def test_g2_accepts_complete_design_artifacts(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_change(root)
            results = evaluate_change_gate(root, "G2")
            self.assertFalse(any(item.status == "FAIL" for item in results))

    def test_g4_rejects_unfinished_tasks(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_change(root, complete_tasks=False)
            results = evaluate_change_gate(root, "G4")
            self.assertTrue(
                any(
                    item.code == "TASKS_COMPLETE" and item.status == "FAIL"
                    for item in results
                )
            )

    def test_g5_accepts_complete_evidence_and_retrospective(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_change(root, complete_tasks=True)
            results = evaluate_change_gate(root, "G5")
            self.assertFalse(any(item.status == "FAIL" for item in results))
