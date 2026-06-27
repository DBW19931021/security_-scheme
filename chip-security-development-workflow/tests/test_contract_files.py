import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ContractFilesTest(unittest.TestCase):
    EXPECTED = {
        "workflow/lifecycle.md": ["# ", "P0", "P6", "功能级 OpenSpec"],
        "workflow/gates.md": ["# ", "G0", "G6", "PASS", "REVIEW"],
        "workflow/artifact-contracts.md": [
            "# ",
            "proposal.md",
            "verification.md",
            "retrospective.md",
        ],
        "workflow/roles-and-approvals.md": [
            "# ",
            "Security Owner",
            "Verification Owner",
            "Skill Maintainer",
        ],
        "workflow/openspec-integration.md": [
            "# ",
            "proposal",
            "spec",
            "design",
            "tasks",
            "evidence",
        ],
        "workflow/knowledge-promotion.md": ["# ", "L0", "L5", "人工批准"],
    }

    def test_normative_files_exist_and_have_required_terms(self):
        for relative, terms in self.EXPECTED.items():
            path = ROOT / relative
            self.assertTrue(path.is_file(), relative)
            text = path.read_text(encoding="utf-8")
            for term in terms:
                self.assertIn(term, text, f"{term!r} missing from {relative}")

    def test_normative_files_do_not_claim_automatic_security_approval(self):
        for relative in self.EXPECTED:
            path = ROOT / relative
            if not path.is_file():
                continue
            text = path.read_text(encoding="utf-8")
            self.assertNotIn("自动化可以批准安全架构", text)
            self.assertNotIn("满足条件后自动晋升为 L5", text)

    def test_required_templates_exist(self):
        required = [
            "templates/chip-project/workflow-project.json.tmpl",
            "templates/chip-project/approvals.json",
            "templates/chip-project/00_project/project_profile.md.tmpl",
            "templates/chip-project/00_project/architecture_overview.md",
            "templates/chip-project/security_inputs/inputs_manifest.md",
            "templates/chip-project/security_workflow/00_threat_model.md",
            "templates/chip-project/security_workflow/01_constraints.md",
            "templates/chip-project/security_workflow/02_baseline.md",
            "templates/chip-project/security_workflow/03_detailed_design/00_chapter_plan.md",
            "templates/chip-project/security_workflow/04_impl_design/README.md",
            "templates/chip-project/security_workflow/05_code_rules.md",
            "templates/chip-project/security_workflow/06_traceability.md",
            "templates/chip-project/security_workflow/09_open_risks.md",
            "templates/chip-project/system_verification/integration_test_plan.md",
            "templates/chip-project/system_verification/security_test_report.md",
            "templates/chip-project/system_verification/threat_coverage.md",
            "templates/chip-project/system_verification/release_risk_register.md",
            "templates/chip-project/system_verification/release_checklist.md",
            "templates/openspec-change/security-workflow.json.tmpl",
            "templates/openspec-change/approvals.json",
            "templates/evidence/verification.md",
            "templates/evidence/code-map.md",
            "templates/evidence/known-issues.md",
            "templates/evidence/knowledge-candidates.json",
            "templates/problem-record/problem-record.md",
            "templates/retrospective/retrospective.md",
            "templates/onboarding/00_quick_start.md",
            "templates/onboarding/01_architecture_reading_path.md",
            "templates/onboarding/02_first_change_walkthrough.md",
        ]
        for relative in required:
            self.assertTrue((ROOT / relative).is_file(), relative)


if __name__ == "__main__":
    unittest.main()
