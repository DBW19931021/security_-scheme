import json
import tempfile
import unittest
from pathlib import Path

from scripts.build_onboarding_pack import build_pack


class OnboardingPackTest(unittest.TestCase):
    def test_pack_contains_three_reading_levels_and_links(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / "project"
            root.mkdir()
            (root / "workflow-project.json").write_text(
                json.dumps(
                    {
                        "chip": "demo",
                        "openspec_root": str(root / "openspec"),
                    }
                ),
                encoding="utf-8",
            )
            for relative in [
                "00_project/project_profile.md",
                "00_project/status_dashboard.md",
                "00_project/architecture_overview.md",
                "00_project/capability_map.md",
                "00_project/decision_log.md",
                "security_inputs/inputs_manifest.md",
                "security_workflow/00_threat_model.md",
                "security_workflow/01_constraints.md",
                "security_workflow/02_baseline.md",
                "security_workflow/06_traceability.md",
            ]:
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("# Doc\n", encoding="utf-8")
            change = root / "openspec/changes/demo-change"
            change.mkdir(parents=True)
            (change / "proposal.md").write_text(
                "## Why\n", encoding="utf-8"
            )
            output = root / "onboarding-pack.md"
            build_pack(root, output)
            text = output.read_text(encoding="utf-8")
            self.assertIn("30 分钟", text)
            self.assertIn("半天", text)
            self.assertIn("一到两天", text)
            self.assertIn("demo-change", text)
            self.assertIn("project_profile.md", text)
