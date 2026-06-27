import json
import tempfile
import unittest
from pathlib import Path

from scripts.workflow_lib import (
    CheckResult,
    all_tasks_complete,
    load_json,
    require_file,
    require_headings,
    result_exit_code,
)


class WorkflowLibTest(unittest.TestCase):
    def test_require_file_and_headings(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "sample.md"
            path.write_text("# Title\n\n## Required\n", encoding="utf-8")
            self.assertEqual(require_file(path, "FILE").status, "PASS")
            checks = require_headings(path, ["# Title", "## Required"], "HEAD")
            self.assertTrue(all(item.status == "PASS" for item in checks))

    def test_missing_heading_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "sample.md"
            path.write_text("# Title\n", encoding="utf-8")
            checks = require_headings(path, ["## Missing"], "HEAD")
            self.assertEqual(checks[0].status, "FAIL")

    def test_load_json(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "data.json"
            path.write_text(json.dumps({"schema_version": 1}), encoding="utf-8")
            self.assertEqual(load_json(path)["schema_version"], 1)

    def test_all_tasks_complete(self):
        self.assertTrue(all_tasks_complete("- [x] done\n- [X] done\n"))
        self.assertFalse(all_tasks_complete("- [x] done\n- [ ] open\n"))

    def test_exit_code_only_fails_on_fail(self):
        self.assertEqual(result_exit_code([CheckResult("A", "PASS", "ok")]), 0)
        self.assertEqual(
            result_exit_code([CheckResult("A", "REVIEW", "review")]), 0
        )
        self.assertEqual(result_exit_code([CheckResult("A", "FAIL", "bad")]), 1)


if __name__ == "__main__":
    unittest.main()
