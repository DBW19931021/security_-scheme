import json
import tempfile
import unittest
from pathlib import Path

from scripts.build_onboarding_pack import build_pack
from scripts.check_project_gates import evaluate_project_gate
from scripts.init_security_project import initialize_project


ROOT = Path(__file__).resolve().parents[1]


class EndToEndWorkflowTest(unittest.TestCase):
    def test_initialize_check_and_build_onboarding(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "demo"
            openspec = target / "openspec"
            initialize_project(
                ROOT / "templates/chip-project",
                target,
                "demo-chip",
                "/code/demo",
                str(openspec),
                "Alice",
                "Bob",
                "Carol",
                "Dana",
            )
            approvals_path = target / "approvals.json"
            approvals = json.loads(
                approvals_path.read_text(encoding="utf-8")
            )
            approvals["gates"]["G0"] = {
                "status": "approved",
                "approvers": ["Alice"],
                "date": "2026-06-09",
                "evidence": "project review",
            }
            approvals_path.write_text(
                json.dumps(approvals), encoding="utf-8"
            )
            results = evaluate_project_gate(target, "G0")
            self.assertFalse(any(item.status == "FAIL" for item in results))
            (openspec / "changes").mkdir(parents=True)
            output = target / "onboarding-pack.md"
            build_pack(target, output)
            self.assertTrue(output.is_file())
            self.assertIn(
                "demo-chip", output.read_text(encoding="utf-8")
            )
