import json
import tempfile
import unittest
from pathlib import Path

from scripts.check_project_gates import evaluate_project_gate


class ProjectGateTest(unittest.TestCase):
    def make_project(self, root: Path) -> None:
        required = [
            "00_project/project_profile.md",
            "00_project/roles_and_approvals.md",
            "00_project/decision_log.md",
            "00_project/open_questions.md",
            "security_inputs/inputs_manifest.md",
            "security_workflow/00_threat_model.md",
            "security_workflow/01_constraints.md",
            "security_workflow/02_baseline.md",
        ]
        for relative in required:
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("# Content\n", encoding="utf-8")
        (root / "workflow-project.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "chip": "demo",
                    "owners": {
                        "security_owner": "Alice",
                        "architecture_owner": "Bob",
                        "verification_owner": "Carol",
                        "skill_maintainer": "Dana",
                    },
                }
            ),
            encoding="utf-8",
        )
        (root / "approvals.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "gates": {
                        "G0": {
                            "status": "approved",
                            "approvers": ["Alice"],
                            "date": "2026-06-09",
                            "evidence": "review",
                        },
                        "G1": {
                            "status": "pending",
                            "approvers": [],
                            "date": "",
                            "evidence": "",
                        },
                    },
                }
            ),
            encoding="utf-8",
        )

    def test_g0_passes_with_files_and_approval(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_project(root)
            results = evaluate_project_gate(root, "G0")
            self.assertFalse(any(item.status == "FAIL" for item in results))

    def test_g1_requires_human_approval(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_project(root)
            results = evaluate_project_gate(root, "G1")
            self.assertTrue(any(item.status == "REVIEW" for item in results))
            self.assertFalse(
                any(item.message == "approved automatically" for item in results)
            )
