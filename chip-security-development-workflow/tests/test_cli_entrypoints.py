import subprocess
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class CliEntrypointTest(unittest.TestCase):
    def test_scripts_can_run_directly(self):
        for relative in [
            "scripts/check_project_gates.py",
            "scripts/check_openspec_evidence.py",
            "scripts/build_onboarding_pack.py",
            "scripts/collect_knowledge_candidates.py",
            "scripts/evaluate_promotion.py",
        ]:
            result = subprocess.run(
                [sys.executable, str(ROOT / relative), "--help"],
                cwd=str(ROOT),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertEqual(
                result.returncode,
                0,
                msg="%s failed: %s" % (relative, result.stderr),
            )
