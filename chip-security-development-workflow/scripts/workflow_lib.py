#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable, List


VALID_STATUSES = {"PASS", "FAIL", "WARN", "REVIEW"}


@dataclass(frozen=True)
class CheckResult:
    code: str
    status: str
    message: str
    path: str = ""

    def __post_init__(self) -> None:
        if self.status not in VALID_STATUSES:
            raise ValueError("invalid check status: %s" % self.status)


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def require_file(path: Path, code: str) -> CheckResult:
    if path.is_file() and path.stat().st_size > 0:
        return CheckResult(code, "PASS", "required file exists", str(path))
    return CheckResult(
        code, "FAIL", "required file is missing or empty", str(path)
    )


def require_headings(
    path: Path, headings: Iterable[str], code: str
) -> List[CheckResult]:
    if not path.is_file():
        return [CheckResult(code, "FAIL", "file is missing", str(path))]
    text = path.read_text(encoding="utf-8")
    results = []
    for heading in headings:
        status = "PASS" if heading in text else "FAIL"
        results.append(
            CheckResult(code, status, "heading %r" % heading, str(path))
        )
    return results


def require_json_fields(
    path: Path, fields: Iterable[str], code: str
) -> List[CheckResult]:
    if not path.is_file():
        return [CheckResult(code, "FAIL", "JSON file is missing", str(path))]
    try:
        data = load_json(path)
    except (OSError, ValueError) as error:
        return [
            CheckResult(
                code, "FAIL", "invalid JSON: %s" % error, str(path)
            )
        ]
    results = []
    for field in fields:
        value = data
        for part in field.split("."):
            if not isinstance(value, dict) or part not in value:
                value = None
                break
            value = value[part]
        status = "PASS" if value not in (None, "", [], {}) else "FAIL"
        results.append(
            CheckResult(code, status, "field %s is populated" % field, str(path))
        )
    return results


def approval_result(approvals_path: Path, gate: str) -> CheckResult:
    if not approvals_path.is_file():
        return CheckResult(
            "APPROVAL",
            "FAIL",
            "approval file is missing",
            str(approvals_path),
        )
    try:
        data = load_json(approvals_path)
    except (OSError, ValueError) as error:
        return CheckResult(
            "APPROVAL",
            "FAIL",
            "invalid approval JSON: %s" % error,
            str(approvals_path),
        )
    record = data.get("gates", {}).get(gate, {})
    if (
        record.get("status") == "approved"
        and record.get("approvers")
        and record.get("date")
        and record.get("evidence")
    ):
        return CheckResult(
            "APPROVAL",
            "PASS",
            "%s approval is recorded" % gate,
            str(approvals_path),
        )
    return CheckResult(
        "APPROVAL",
        "REVIEW",
        "%s requires human approval" % gate,
        str(approvals_path),
    )


def all_tasks_complete(text: str) -> bool:
    boxes = re.findall(r"^- \[([ xX])\]", text, flags=re.MULTILINE)
    return bool(boxes) and all(value.lower() == "x" for value in boxes)


def result_exit_code(results: Iterable[CheckResult]) -> int:
    return 1 if any(item.status == "FAIL" for item in results) else 0


def render_results(
    results: Iterable[CheckResult], as_json: bool = False
) -> str:
    items = list(results)
    if as_json:
        return json.dumps(
            [asdict(item) for item in items], ensure_ascii=False, indent=2
        )
    lines = []
    for item in items:
        suffix = " (%s)" % item.path if item.path else ""
        lines.append(
            "[%s] %s: %s%s"
            % (item.status, item.code, item.message, suffix)
        )
    return "\n".join(lines)
