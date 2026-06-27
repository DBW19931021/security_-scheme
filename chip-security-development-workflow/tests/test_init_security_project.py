import tempfile
import unittest
from pathlib import Path

from scripts.init_security_project import initialize_project
from scripts.workflow_lib import load_json


ROOT = Path(__file__).resolve().parents[1]


class InitSecurityProjectTest(unittest.TestCase):
    def test_initializer_creates_unapproved_project(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "demo-security"
            initialize_project(
                templates_root=ROOT / "templates/chip-project",
                target=target,
                chip="demo-chip",
                code_repository_root="/work/demo-code",
                openspec_root="/work/demo-code/security/docs/openspec",
                security_owner="Alice",
                architecture_owner="Bob",
                verification_owner="Carol",
                skill_maintainer="Dana",
            )
            config = load_json(target / "workflow-project.json")
            approvals = load_json(target / "approvals.json")
            self.assertEqual(config["chip"], "demo-chip")
            self.assertEqual(config["owners"]["security_owner"], "Alice")
            self.assertTrue(
                (target / "00_project/project_profile.md").is_file()
            )
            self.assertTrue(
                (target / "security_workflow/02_baseline.md").is_file()
            )
            self.assertTrue(
                (target / "security_workflow/06_traceability.md").is_file()
            )
            self.assertTrue(
                (target / "system_verification/security_test_report.md").is_file()
            )
            self.assertTrue(
                all(
                    gate["status"] == "pending"
                    for gate in approvals["gates"].values()
                )
            )

    def test_initializer_refuses_nonempty_target(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "demo-security"
            target.mkdir()
            (target / "existing.txt").write_text("keep", encoding="utf-8")
            with self.assertRaises(ValueError):
                initialize_project(
                    ROOT / "templates/chip-project",
                    target,
                    "demo",
                    "/code",
                    "/code/openspec",
                    "A",
                    "B",
                    "C",
                    "D",
                )


if __name__ == "__main__":
    unittest.main()
