# Chip Security Development Workflow V1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

## Execution Rules

- Update checkboxes immediately after verification.
- Do not mark a task complete when its verification command has not run.
- Do not create Git commits unless the project owner explicitly requests them.

**Goal:** Build a usable V1 workflow foundation that enforces chip-level security artifacts, OpenSpec change evidence, project Gates, onboarding, and knowledge-promotion review, then migrate the NGU800 MCTP change as the first pilot.

**Architecture:** Use Markdown as the human-reviewable source and small JSON manifests for machine state. Keep OpenSpec's standard `spec-driven` artifacts unchanged, add security-specific evidence and approval files beside them, and implement deterministic validators with Python 3.8 standard library only. This plan stops at L0-L3 candidate evaluation; cross-chip L4 validation and the final L5 Skill require a later plan after a second chip provides evidence.

**Tech Stack:** OpenSpec 1.2.0, Python 3.8 standard library, `unittest`, Markdown, JSON, Git.

**Repository constraint:** Do not create Git commits unless the user explicitly requests them. Each task ends with a status/diff checkpoint instead of a commit.

---

## Scope

This plan implements:

- The common workflow repository structure.
- Normative workflow documents.
- Chip-project, OpenSpec evidence, problem, retrospective, and onboarding templates.
- Project initialization.
- G0-G6 structural Gate evaluation.
- OpenSpec evidence validation.
- Onboarding-pack generation.
- L0-L3 knowledge candidate collection and evaluation.
- The NGU800 MCTP OpenSpec migration pilot.

This plan does not implement:

- MCTP/libmctp/SPDM production code.
- Cross-chip L4 validation.
- The final `chip-security-development` L5 Skill.
- Automatic approval of security decisions or promotion.

## File Map

### Common workflow repository

```text
security_-scheme/chip-security-development-workflow/
├── openspec/
│   ├── config.yaml
│   └── changes/add-chip-security-workflow-v1/
│       ├── .openspec.yaml
│       ├── proposal.md
│       ├── specs/chip-security-development-workflow/spec.md
│       ├── design.md
│       └── tasks.md
├── workflow/
│   ├── lifecycle.md
│   ├── gates.md
│   ├── artifact-contracts.md
│   ├── roles-and-approvals.md
│   ├── openspec-integration.md
│   └── knowledge-promotion.md
├── templates/
│   ├── chip-project/
│   ├── openspec-change/
│   ├── evidence/
│   ├── problem-record/
│   ├── retrospective/
│   └── onboarding/
├── scripts/
│   ├── workflow_lib.py
│   ├── init_security_project.py
│   ├── check_project_gates.py
│   ├── check_openspec_evidence.py
│   ├── build_onboarding_pack.py
│   ├── collect_knowledge_candidates.py
│   └── evaluate_promotion.py
├── tests/
│   ├── __init__.py
│   ├── test_contract_files.py
│   ├── test_workflow_lib.py
│   ├── test_init_security_project.py
│   ├── test_project_gates.py
│   ├── test_openspec_evidence.py
│   ├── test_onboarding_pack.py
│   ├── test_knowledge_promotion.py
│   └── test_end_to_end.py
├── knowledge/
│   ├── candidates/.gitkeep
│   ├── validated-patterns/.gitkeep
│   ├── anti-patterns/.gitkeep
│   └── promotion-log.md
├── onboarding/role-checklists/
└── examples/ngu800/project-map.md
```

### NGU800 MCTP pilot

```text
gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/
└── changes/add-mctp-mailbox-spdm-endpoint/
    ├── .openspec.yaml
    ├── proposal.md
    ├── specs/mctp-mailbox-spdm-endpoint/spec.md
    ├── design.md
    ├── tasks.md
    ├── security-workflow.json
    ├── approvals.json
    ├── evidence/
    │   ├── verification.md
    │   ├── code-map.md
    │   ├── known-issues.md
    │   ├── knowledge-candidates.json
    │   └── problem-records/.gitkeep
    └── retrospective.md
```

## Task 1: Dogfood OpenSpec for the workflow itself

**Files:**
- Create: `chip-security-development-workflow/openspec/config.yaml`
- Create: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/.openspec.yaml`
- Create: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/proposal.md`
- Create: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/specs/chip-security-development-workflow/spec.md`
- Create: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/design.md`
- Create: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/tasks.md`
- Source: `chip-security-development-workflow/docs/superpowers/specs/2026-06-09-chip-security-development-workflow-design.md`
- Source: `chip-security-development-workflow/docs/superpowers/plans/2026-06-09-chip-security-development-workflow-v1.md`

- [x] **Step 1: Initialize OpenSpec without installing editor integrations**

Run:

```bash
cd /home/may-pc/share/code/ngu800/secure/security_-scheme/chip-security-development-workflow
openspec init . --tools none
openspec new change add-chip-security-workflow-v1 --schema spec-driven
```

Expected:

```text
openspec/config.yaml exists
openspec/changes/add-chip-security-workflow-v1/.openspec.yaml exists
```

- [x] **Step 2: Configure the workflow OpenSpec context**

Set `openspec/config.yaml` to:

```yaml
schema: spec-driven

context: |
  This project defines a human-and-AI chip security development workflow.
  Security architecture and knowledge promotion require explicit human approval.
  Markdown is human-reviewable truth; JSON carries machine state.
  Python tools must use the Python 3.8 standard library only.

rules:
  proposal:
    - State security impact, scope, and approval authority.
  specs:
    - Use stable requirement IDs and verifiable scenarios.
  design:
    - Define fact ownership, Gate behavior, and automation limits.
  tasks:
    - Include tests, evidence, retrospective, and no-commit checkpoints.
```

- [x] **Step 3: Write the workflow proposal**

Write `proposal.md` with these exact sections:

```markdown
## Why

Chip security projects currently keep architecture baselines, code-change designs,
implementation plans, debugging history, and reusable knowledge in separate systems
without a single lifecycle or promotion rule. The workflow must connect those assets
without making any one tool the owner of every fact.

## What Changes

- Add a chip-level P0-P6 security development lifecycle.
- Add a feature-level OpenSpec change loop with evidence and retrospective artifacts.
- Add G0-G6 structural Gates with explicit human approval roles.
- Add deterministic project, evidence, onboarding, and promotion tools.
- Pilot the workflow with the NGU800 MCTP mailbox/SPDM endpoint change.

## Capabilities

### New Capabilities
- `chip-security-development-workflow`: Cross-chip security project lifecycle,
  artifact contracts, Gates, onboarding, and evidence-based knowledge promotion.

### Modified Capabilities

## Impact

- Adds files only under `security_-scheme/chip-security-development-workflow`.
- The pilot later modifies only `gsp-pmp-rmp-omp/components/ngu_security/docs`.
- Uses OpenSpec 1.2.0 and Python 3.8 standard library.
- Does not approve or alter NGU800 security architecture decisions.
```

- [x] **Step 4: Write the workflow requirements**

Create `specs/chip-security-development-workflow/spec.md` with requirements named:

```text
WF-REQ-001 Chip-level P0-P6 lifecycle
WF-REQ-002 Feature-level OpenSpec loop
WF-REQ-003 Single-source ownership
WF-REQ-004 G0-G6 Gate evaluation
WF-REQ-005 Human approval authority
WF-REQ-006 Verification evidence
WF-REQ-007 Problem and retrospective records
WF-REQ-008 Knowledge promotion levels
WF-REQ-009 Onboarding paths
WF-REQ-010 NGU800 pilot isolation
```

Each requirement must have at least one `WHEN`/`THEN` scenario. Use the following pattern:

```markdown
### Requirement: WF-REQ-005 Human approval authority
Automation MUST NOT approve security architecture decisions or knowledge promotion.

#### Scenario: Promotion conditions are mechanically satisfied
- **WHEN** a knowledge candidate satisfies all structural conditions for its target level
- **THEN** the evaluator MUST return `REVIEW`
- **AND** it MUST leave the candidate approval state unchanged
```

- [x] **Step 5: Move the confirmed design into OpenSpec**

Copy the complete content of:

```text
docs/superpowers/specs/2026-06-09-chip-security-development-workflow-design.md
```

to:

```text
openspec/changes/add-chip-security-workflow-v1/design.md
```

Do not summarize or omit sections. Change the document status line to:

```text
状态：OpenSpec 设计事实源，已批准实施
```

- [x] **Step 6: Seed OpenSpec tasks from this implementation plan**

Copy the task titles and checkbox steps from this plan into the change `tasks.md`.
Keep the OpenSpec file as the progress source during implementation. Add this heading:

```markdown
## Execution Rules

- Update checkboxes immediately after verification.
- Do not mark a task complete when its verification command has not run.
- Do not create Git commits unless the project owner explicitly requests them.
```

- [x] **Step 7: Validate the workflow change**

Run:

```bash
openspec validate add-chip-security-workflow-v1 --strict --no-interactive
openspec status --change add-chip-security-workflow-v1
```

Expected:

```text
Validation succeeds.
proposal, specs, design, and tasks are reported complete.
```

- [x] **Step 8: Record a no-commit checkpoint**

Run:

```bash
git status --short -- chip-security-development-workflow
git diff --check -- chip-security-development-workflow
```

Expected: only intended workflow files are listed and `git diff --check` is silent.

## Task 2: Create normative workflow documents

**Files:**
- Create: `chip-security-development-workflow/workflow/lifecycle.md`
- Create: `chip-security-development-workflow/workflow/gates.md`
- Create: `chip-security-development-workflow/workflow/artifact-contracts.md`
- Create: `chip-security-development-workflow/workflow/roles-and-approvals.md`
- Create: `chip-security-development-workflow/workflow/openspec-integration.md`
- Create: `chip-security-development-workflow/workflow/knowledge-promotion.md`
- Test: `chip-security-development-workflow/tests/test_contract_files.py`

- [x] **Step 1: Write the failing contract-file test**

Create `tests/__init__.py` as an empty file and create:

```python
# tests/test_contract_files.py
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ContractFilesTest(unittest.TestCase):
    EXPECTED = {
        "workflow/lifecycle.md": ["# ", "P0", "P6", "功能级 OpenSpec"],
        "workflow/gates.md": ["# ", "G0", "G6", "PASS", "REVIEW"],
        "workflow/artifact-contracts.md": ["# ", "proposal.md", "verification.md", "retrospective.md"],
        "workflow/roles-and-approvals.md": ["# ", "Security Owner", "Verification Owner", "Skill Maintainer"],
        "workflow/openspec-integration.md": ["# ", "proposal", "spec", "design", "tasks", "evidence"],
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
            text = (ROOT / relative).read_text(encoding="utf-8")
            self.assertNotIn("自动化可以批准安全架构", text)
            self.assertNotIn("满足条件后自动晋升为 L5", text)


if __name__ == "__main__":
    unittest.main()
```

- [x] **Step 2: Run the test and verify it fails**

Run:

```bash
python3 -m unittest tests.test_contract_files -v
```

Expected: FAIL because the six `workflow/*.md` files do not exist.

- [x] **Step 3: Write the six normative files**

Split the approved design without duplicating ownership:

```text
lifecycle.md
  P0-P6, feature loop, entry/exit rules, simplified-change rule

gates.md
  G0-G6, required evidence, approval role, PASS/FAIL/WARN/REVIEW semantics

artifact-contracts.md
  Required files, headings, stable IDs, JSON manifests, link rules

roles-and-approvals.md
  Role definitions, separation of duties, approval recording

openspec-integration.md
  Standard OpenSpec artifacts plus evidence/retrospective extension

knowledge-promotion.md
  L0-L5, automatic evaluation, manual approval, non-generalizable facts
```

Each document starts with:

```markdown
# <Document Title>

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.
```

Use `MUST`, `MUST NOT`, `SHOULD`, and `MAY` consistently.

- [x] **Step 4: Run the contract-file test**

Run:

```bash
python3 -m unittest tests.test_contract_files -v
```

Expected: 2 tests PASS.

- [x] **Step 5: Update OpenSpec progress and checkpoint**

Mark Task 2 complete in the workflow change `tasks.md`, then run:

```bash
git diff --check -- chip-security-development-workflow
```

Expected: silent.

## Task 3: Create artifact templates

**Files:**
- Create: `chip-security-development-workflow/templates/chip-project/workflow-project.json.tmpl`
- Create: `chip-security-development-workflow/templates/chip-project/approvals.json`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/project_profile.md.tmpl`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/roles_and_approvals.md.tmpl`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/architecture_overview.md`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/decision_log.md`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/open_questions.md`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/status_dashboard.md.tmpl`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/capability_map.md`
- Create: `chip-security-development-workflow/templates/chip-project/00_project/implementation_roadmap.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_inputs/inputs_manifest.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/00_threat_model.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/01_constraints.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/02_baseline.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/03_detailed_design/00_chapter_plan.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/04_impl_design/README.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/05_code_rules.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/06_traceability.md`
- Create: `chip-security-development-workflow/templates/chip-project/security_workflow/09_open_risks.md`
- Create: `chip-security-development-workflow/templates/chip-project/system_verification/integration_test_plan.md`
- Create: `chip-security-development-workflow/templates/chip-project/system_verification/security_test_report.md`
- Create: `chip-security-development-workflow/templates/chip-project/system_verification/threat_coverage.md`
- Create: `chip-security-development-workflow/templates/chip-project/system_verification/release_risk_register.md`
- Create: `chip-security-development-workflow/templates/chip-project/system_verification/release_checklist.md`
- Create: `chip-security-development-workflow/templates/openspec-change/security-workflow.json.tmpl`
- Create: `chip-security-development-workflow/templates/openspec-change/approvals.json`
- Create: `chip-security-development-workflow/templates/evidence/verification.md`
- Create: `chip-security-development-workflow/templates/evidence/code-map.md`
- Create: `chip-security-development-workflow/templates/evidence/known-issues.md`
- Create: `chip-security-development-workflow/templates/evidence/knowledge-candidates.json`
- Create: `chip-security-development-workflow/templates/problem-record/problem-record.md`
- Create: `chip-security-development-workflow/templates/retrospective/retrospective.md`
- Create: `chip-security-development-workflow/templates/onboarding/00_quick_start.md`
- Create: `chip-security-development-workflow/templates/onboarding/01_architecture_reading_path.md`
- Create: `chip-security-development-workflow/templates/onboarding/02_first_change_walkthrough.md`
- Test: `chip-security-development-workflow/tests/test_contract_files.py`

- [x] **Step 1: Extend the failing contract test for templates**

Add:

```python
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
        ]
        for relative in required:
            self.assertTrue((ROOT / relative).is_file(), relative)
```

- [x] **Step 2: Run the template test and verify it fails**

Run:

```bash
python3 -m unittest tests.test_contract_files.ContractFilesTest.test_required_templates_exist -v
```

Expected: FAIL on the first missing template.

- [x] **Step 3: Create the machine-readable project templates**

Use this exact `workflow-project.json.tmpl` structure:

```json
{
  "schema_version": 1,
  "chip": "{{CHIP_NAME}}",
  "security_project_root": "{{SECURITY_PROJECT_ROOT}}",
  "code_repository_root": "{{CODE_REPOSITORY_ROOT}}",
  "openspec_root": "{{OPENSPEC_ROOT}}",
  "owners": {
    "security_owner": "{{SECURITY_OWNER}}",
    "architecture_owner": "{{ARCHITECTURE_OWNER}}",
    "verification_owner": "{{VERIFICATION_OWNER}}",
    "skill_maintainer": "{{SKILL_MAINTAINER}}"
  }
}
```

Use this exact approval model for both project and change templates:

```json
{
  "schema_version": 1,
  "gates": {
    "G0": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G1": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G2": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G3": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G4": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G5": {"status": "pending", "approvers": [], "date": "", "evidence": ""},
    "G6": {"status": "pending", "approvers": [], "date": "", "evidence": ""}
  }
}
```

The initializer must never replace `pending` with `approved`.

- [x] **Step 4: Create the OpenSpec security metadata template**

Use:

```json
{
  "schema_version": 1,
  "change_id": "{{CHANGE_ID}}",
  "chip": "{{CHIP_NAME}}",
  "security_project_root": "{{SECURITY_PROJECT_ROOT}}",
  "baseline_refs": {
    "sources": [],
    "threats": [],
    "constraints": [],
    "decisions": [],
    "change_requests": [],
    "open_questions": []
  },
  "owners": {
    "feature_owner": "",
    "verification_owner": "",
    "reviewer": ""
  }
}
```

- [x] **Step 5: Create Markdown templates with required headings**

Use these heading contracts:

```text
verification.md:
  # Verification Evidence
  ## Environment
  ## Commands and Results
  ## Requirement Coverage
  ## Unverified Items
  ## Residual Risks

code-map.md:
  # Code and Test Map
  table columns:
  Requirement | Design | Task | Code | Test | Evidence

known-issues.md:
  # Known Issues
  ## Open Issues
  ## Accepted Risks
  ## Closed Issues

problem-record.md:
  YAML frontmatter with id/status/category/chip/change/environment/
  security_impact/knowledge_level/promotion_candidate/owner/reviewer
  and sections for symptom, reproduction, impact, investigation,
  rejected approaches, root cause, solution, evidence, applicability,
  counterexample, prevention

retrospective.md:
  # Retrospective
  ## Delivered Scope
  ## Design Deviations
  ## Problems and Root Causes
  ## Rejected Approaches
  ## Effective Practices
  ## Ineffective Practices
  ## Known Issues and Debt
  ## Reusable Knowledge Candidates
  ## Baseline and Documentation Follow-up
```

Use these chip-level contracts:

```text
architecture_overview.md:
  actors, trust boundaries, boot stages, external interfaces, diagram links

03_detailed_design/00_chapter_plan.md:
  chapter, owner, bound constraints, status, implementation impact

04_impl_design/README.md:
  implementation shard rules and binding to the full design

05_code_rules.md:
  MUST/MUST NOT/SHOULD rules with constraint and design references

06_traceability.md:
  Source | Threat | Constraint | Decision/CR | Requirement |
  Design | Code | Test | Evidence | Status

09_open_risks.md:
  risk ID, impact, likelihood, owner, mitigation, status

system_verification/*:
  integration scope, threat coverage, test evidence, residual risk,
  release blockers, and approval sections
```

- [x] **Step 6: Run all contract tests**

Run:

```bash
python3 -m unittest tests.test_contract_files -v
```

Expected: all contract and template tests PASS.

- [x] **Step 7: Update OpenSpec progress and checkpoint**

Run:

```bash
git diff --check -- chip-security-development-workflow
```

Expected: silent.

## Task 4: Build the common Python validation library

**Files:**
- Create: `chip-security-development-workflow/scripts/workflow_lib.py`
- Create: `chip-security-development-workflow/tests/test_workflow_lib.py`

- [x] **Step 1: Write failing library tests**

Create:

```python
# tests/test_workflow_lib.py
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
        self.assertEqual(result_exit_code([CheckResult("A", "REVIEW", "review")]), 0)
        self.assertEqual(result_exit_code([CheckResult("A", "FAIL", "bad")]), 1)


if __name__ == "__main__":
    unittest.main()
```

- [x] **Step 2: Run tests and verify import failure**

Run:

```bash
python3 -m unittest tests.test_workflow_lib -v
```

Expected: ERROR because `scripts.workflow_lib` does not exist.

- [x] **Step 3: Implement `workflow_lib.py`**

Implement:

```python
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
    return CheckResult(code, "FAIL", "required file is missing or empty", str(path))


def require_headings(path: Path, headings: Iterable[str], code: str) -> List[CheckResult]:
    if not path.is_file():
        return [CheckResult(code, "FAIL", "file is missing", str(path))]
    text = path.read_text(encoding="utf-8")
    results = []
    for heading in headings:
        status = "PASS" if heading in text else "FAIL"
        results.append(CheckResult(code, status, "heading %r" % heading, str(path)))
    return results


def all_tasks_complete(text: str) -> bool:
    boxes = re.findall(r"^- \[([ xX])\]", text, flags=re.MULTILINE)
    return bool(boxes) and all(value.lower() == "x" for value in boxes)


def result_exit_code(results: Iterable[CheckResult]) -> int:
    return 1 if any(item.status == "FAIL" for item in results) else 0


def render_results(results: Iterable[CheckResult], as_json: bool = False) -> str:
    items = list(results)
    if as_json:
        return json.dumps([asdict(item) for item in items], ensure_ascii=False, indent=2)
    lines = []
    for item in items:
        suffix = " (%s)" % item.path if item.path else ""
        lines.append("[%s] %s: %s%s" % (item.status, item.code, item.message, suffix))
    return "\n".join(lines)
```

- [x] **Step 4: Run library tests**

Run:

```bash
python3 -m unittest tests.test_workflow_lib -v
```

Expected: 5 tests PASS.

- [x] **Step 5: Run all tests and checkpoint**

Run:

```bash
python3 -m unittest discover -s tests -v
git diff --check -- chip-security-development-workflow
```

Expected: all current tests PASS; diff check is silent.

## Task 5: Implement chip security project initialization

**Files:**
- Create: `chip-security-development-workflow/scripts/init_security_project.py`
- Create: `chip-security-development-workflow/tests/test_init_security_project.py`

- [x] **Step 1: Write the failing initializer test**

Create:

```python
# tests/test_init_security_project.py
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
            self.assertTrue((target / "00_project/project_profile.md").is_file())
            self.assertTrue((target / "security_workflow/02_baseline.md").is_file())
            self.assertTrue((target / "security_workflow/06_traceability.md").is_file())
            self.assertTrue((target / "system_verification/security_test_report.md").is_file())
            self.assertTrue(all(
                gate["status"] == "pending"
                for gate in approvals["gates"].values()
            ))

    def test_initializer_refuses_nonempty_target(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "demo-security"
            target.mkdir()
            (target / "existing.txt").write_text("keep", encoding="utf-8")
            with self.assertRaises(ValueError):
                initialize_project(
                    ROOT / "templates/chip-project", target, "demo",
                    "/code", "/code/openspec", "A", "B", "C", "D"
                )


if __name__ == "__main__":
    unittest.main()
```

- [x] **Step 2: Run tests and verify import failure**

Run:

```bash
python3 -m unittest tests.test_init_security_project -v
```

Expected: ERROR because `scripts.init_security_project` does not exist.

- [x] **Step 3: Implement template rendering and initialization**

Implement these public functions:

```python
def render_text(text: str, replacements: dict) -> str:
    for key, value in replacements.items():
        text = text.replace("{{%s}}" % key, value)
    return text


def initialize_project(
    templates_root: Path,
    target: Path,
    chip: str,
    code_repository_root: str,
    openspec_root: str,
    security_owner: str,
    architecture_owner: str,
    verification_owner: str,
    skill_maintainer: str,
) -> None:
    if target.exists() and any(target.iterdir()):
        raise ValueError("target directory is not empty: %s" % target)
    target.mkdir(parents=True, exist_ok=True)
    replacements = {
        "CHIP_NAME": chip,
        "SECURITY_PROJECT_ROOT": str(target.resolve()),
        "CODE_REPOSITORY_ROOT": code_repository_root,
        "OPENSPEC_ROOT": openspec_root,
        "SECURITY_OWNER": security_owner,
        "ARCHITECTURE_OWNER": architecture_owner,
        "VERIFICATION_OWNER": verification_owner,
        "SKILL_MAINTAINER": skill_maintainer,
    }
    for source in templates_root.rglob("*"):
        if not source.is_file():
            continue
        relative = source.relative_to(templates_root)
        if relative.suffix == ".tmpl":
            relative = relative.with_suffix("")
        destination = target / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        text = source.read_text(encoding="utf-8")
        destination.write_text(render_text(text, replacements), encoding="utf-8")
```

Add an `argparse` CLI requiring all owner arguments. The CLI must print the created project root and must not approve any Gate.

- [x] **Step 4: Run initializer tests**

Run:

```bash
python3 -m unittest tests.test_init_security_project -v
```

Expected: 2 tests PASS.

- [x] **Step 5: Exercise the CLI in a temporary directory**

Run:

```bash
tmp_dir="$(mktemp -d)"
python3 scripts/init_security_project.py \
  --target "${tmp_dir}/demo-security" \
  --chip demo-chip \
  --code-repository-root /work/demo-code \
  --openspec-root /work/demo-code/security/docs/openspec \
  --security-owner Alice \
  --architecture-owner Bob \
  --verification-owner Carol \
  --skill-maintainer Dana
python3 -m json.tool "${tmp_dir}/demo-security/workflow-project.json" >/dev/null
python3 -m json.tool "${tmp_dir}/demo-security/approvals.json" >/dev/null
rm -rf "${tmp_dir}"
```

Expected: both JSON files validate and no Gate has status `approved`.

- [x] **Step 6: Update progress and checkpoint**

Run all tests and `git diff --check`.

## Task 6: Implement G0-G6 structural Gate evaluation

**Files:**
- Create: `chip-security-development-workflow/scripts/check_project_gates.py`
- Create: `chip-security-development-workflow/tests/test_project_gates.py`
- Modify: `chip-security-development-workflow/scripts/workflow_lib.py`

- [x] **Step 1: Write failing G0/G1 tests**

Create:

```python
# tests/test_project_gates.py
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
            json.dumps({
                "schema_version": 1,
                "chip": "demo",
                "owners": {
                    "security_owner": "Alice",
                    "architecture_owner": "Bob",
                    "verification_owner": "Carol",
                    "skill_maintainer": "Dana"
                }
            }),
            encoding="utf-8",
        )
        (root / "approvals.json").write_text(
            json.dumps({
                "schema_version": 1,
                "gates": {
                    "G0": {"status": "approved", "approvers": ["Alice"], "date": "2026-06-09", "evidence": "review"},
                    "G1": {"status": "pending", "approvers": [], "date": "", "evidence": ""}
                }
            }),
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
            self.assertFalse(any(item.message == "approved automatically" for item in results))
```

- [x] **Step 2: Run tests and verify import failure**

Run:

```bash
python3 -m unittest tests.test_project_gates -v
```

Expected: ERROR because the Gate checker does not exist.

- [x] **Step 3: Add approval checking to `workflow_lib.py`**

Implement:

```python
def approval_result(approvals_path: Path, gate: str) -> CheckResult:
    if not approvals_path.is_file():
        return CheckResult("APPROVAL", "FAIL", "approval file is missing", str(approvals_path))
    data = load_json(approvals_path)
    record = data.get("gates", {}).get(gate, {})
    if record.get("status") == "approved" and record.get("approvers") and record.get("date") and record.get("evidence"):
        return CheckResult("APPROVAL", "PASS", "%s approval is recorded" % gate, str(approvals_path))
    return CheckResult("APPROVAL", "REVIEW", "%s requires human approval" % gate, str(approvals_path))
```

- [x] **Step 4: Implement project Gate evaluation**

`evaluate_project_gate(project_root, gate)` must:

```text
G0:
  workflow-project.json
  approvals.json
  project_profile.md
  roles_and_approvals.md
  inputs_manifest.md
  all four owner values non-empty
  G0 approval

G1:
  all G0 structural checks
  threat model
  constraints
  baseline
  decision log
  open questions
  G1 approval
```

For `G2` through `G5`, dispatch to:

```python
from scripts.check_openspec_evidence import evaluate_change_gate
```

and require `--change-root`.

For `G6`, dispatch to:

```python
from scripts.evaluate_promotion import evaluate_candidate
```

and require `--candidate`.

The CLI must support:

```text
--project-root
--gate G0..G6
--change-root
--candidate
--json
```

- [x] **Step 5: Run Gate tests**

Run:

```bash
python3 -m unittest tests.test_project_gates -v
```

Expected: 2 tests PASS.

- [x] **Step 6: Verify CLI output**

Create a temporary initialized project, then run:

```bash
python3 scripts/check_project_gates.py --project-root "${project}" --gate G0
```

Expected before approval:

```text
[REVIEW] APPROVAL: G0 requires human approval
```

Exit status must be `0`; only `FAIL` blocks mechanically.

- [x] **Step 7: Update progress and checkpoint**

Run all tests and `git diff --check`.

## Task 7: Implement OpenSpec evidence and archive-readiness checks

**Files:**
- Create: `chip-security-development-workflow/scripts/check_openspec_evidence.py`
- Create: `chip-security-development-workflow/tests/test_openspec_evidence.py`

- [x] **Step 1: Write failing change-Gate tests**

Create:

```python
# tests/test_openspec_evidence.py
import json
import tempfile
import unittest
from pathlib import Path

from scripts.check_openspec_evidence import evaluate_change_gate


class OpenSpecEvidenceTest(unittest.TestCase):
    def make_change(self, root: Path, complete_tasks: bool = False) -> None:
        files = {
            "proposal.md": "## Why\n\n## What Changes\n\n## Capabilities\n\n## Impact\n",
            "design.md": "## Context\n\n## Goals / Non-Goals\n\n## Decisions\n\n## Risks / Trade-offs\n",
            "tasks.md": "- [x] done\n" if complete_tasks else "- [ ] open\n",
            "specs/demo/spec.md": "## ADDED Requirements\n\n### Requirement: DEMO-REQ-001 Demo\n\n#### Scenario: Demo\n- **WHEN** input\n- **THEN** output\n",
            "evidence/verification.md": "# Verification Evidence\n## Environment\n## Commands and Results\n## Requirement Coverage\n## Unverified Items\n## Residual Risks\n",
            "evidence/code-map.md": "# Code and Test Map\n\n| Requirement | Design | Task | Code | Test | Evidence |\n|---|---|---|---|---|---|\n| DEMO-REQ-001 | design | 1.1 | a.c | test_a | verification |\n",
            "evidence/known-issues.md": "# Known Issues\n## Open Issues\n## Accepted Risks\n## Closed Issues\n",
            "retrospective.md": "# Retrospective\n## Delivered Scope\n## Design Deviations\n## Problems and Root Causes\n## Rejected Approaches\n## Effective Practices\n## Ineffective Practices\n## Known Issues and Debt\n## Reusable Knowledge Candidates\n## Baseline and Documentation Follow-up\n",
        }
        for relative, text in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
        (root / "security-workflow.json").write_text(
            json.dumps({
                "schema_version": 1,
                "change_id": "demo",
                "chip": "demo",
                "baseline_refs": {
                    "sources": ["SRC-001"],
                    "threats": ["THR-001"],
                    "constraints": ["C-001"],
                    "decisions": ["DEC-001"],
                    "change_requests": [],
                    "open_questions": []
                },
                "owners": {
                    "feature_owner": "Alice",
                    "verification_owner": "Bob",
                    "reviewer": "Carol"
                }
            }),
            encoding="utf-8",
        )
        gates = {}
        for gate in ["G2", "G3", "G4", "G5"]:
            gates[gate] = {
                "status": "approved",
                "approvers": ["Owner"],
                "date": "2026-06-09",
                "evidence": "review"
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
            self.assertTrue(any(item.code == "TASKS_COMPLETE" and item.status == "FAIL" for item in results))

    def test_g5_accepts_complete_evidence_and_retrospective(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.make_change(root, complete_tasks=True)
            results = evaluate_change_gate(root, "G5")
            self.assertFalse(any(item.status == "FAIL" for item in results))
```

- [x] **Step 2: Run tests and verify import failure**

Run:

```bash
python3 -m unittest tests.test_openspec_evidence -v
```

Expected: ERROR because the checker does not exist.

- [x] **Step 3: Implement `evaluate_change_gate`**

Required checks:

```text
G2:
  proposal.md, design.md, at least one specs/*/spec.md
  security-workflow.json
  non-empty chip, change_id, all owner fields
  at least one source/constraint/decision/CR/open-question reference
  G2 approval

G3:
  all G2 checks
  tasks.md contains at least one checkbox
  design contains testing and risk sections
  G3 approval

G4:
  all tasks checked
  verification headings complete
  code-map headings/table complete
  G4 approval

G5:
  all G4 checks
  known-issues headings complete
  retrospective headings complete
  G5 approval
```

Use `FAIL` for missing structure and `REVIEW` for missing human approval.

- [x] **Step 4: Add the CLI**

Support:

```bash
python3 scripts/check_openspec_evidence.py \
  --change-root /path/to/change \
  --gate G2 \
  --json
```

Print using `render_results()` and exit using `result_exit_code()`.

- [x] **Step 5: Run evidence tests**

Run:

```bash
python3 -m unittest tests.test_openspec_evidence -v
```

Expected: 3 tests PASS.

- [x] **Step 6: Update progress and checkpoint**

Run all tests and `git diff --check`.

## Task 8: Build onboarding-pack generation

**Files:**
- Create: `chip-security-development-workflow/scripts/build_onboarding_pack.py`
- Create: `chip-security-development-workflow/tests/test_onboarding_pack.py`
- Create: `chip-security-development-workflow/onboarding/role-checklists/security-owner.md`
- Create: `chip-security-development-workflow/onboarding/role-checklists/feature-developer.md`
- Create: `chip-security-development-workflow/onboarding/role-checklists/verification-owner.md`

- [x] **Step 1: Write the failing onboarding test**

Create:

```python
# tests/test_onboarding_pack.py
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
                json.dumps({
                    "chip": "demo",
                    "openspec_root": str(root / "openspec")
                }),
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
            (change / "proposal.md").write_text("## Why\n", encoding="utf-8")
            output = root / "onboarding-pack.md"
            build_pack(root, output)
            text = output.read_text(encoding="utf-8")
            self.assertIn("30 分钟", text)
            self.assertIn("半天", text)
            self.assertIn("一到两天", text)
            self.assertIn("demo-change", text)
            self.assertIn("project_profile.md", text)
```

- [x] **Step 2: Run and verify import failure**

Run:

```bash
python3 -m unittest tests.test_onboarding_pack -v
```

Expected: ERROR because the builder does not exist.

- [x] **Step 3: Implement onboarding generation**

Implement:

```python
def markdown_link(label: str, path: Path, output: Path) -> str:
    return "[%s](%s)" % (label, path.resolve().relative_to(output.parent.resolve()))


def build_pack(project_root: Path, output: Path) -> None:
    config = load_json(project_root / "workflow-project.json")
    openspec_root = Path(config["openspec_root"])
    active_changes = sorted(
        path.name for path in (openspec_root / "changes").glob("*")
        if path.is_dir() and path.name != "archive"
    )
    sections = [
        ("30 分钟：理解项目", [
            "00_project/project_profile.md",
            "00_project/status_dashboard.md",
            "00_project/architecture_overview.md",
            "00_project/capability_map.md",
        ]),
        ("半天：理解设计依据", [
            "security_inputs/inputs_manifest.md",
            "security_workflow/00_threat_model.md",
            "security_workflow/01_constraints.md",
            "security_workflow/02_baseline.md",
            "00_project/decision_log.md",
            "security_workflow/06_traceability.md",
        ]),
    ]
```

Finish the function by writing only links and concise generated summaries. Do not copy security design body text. Add a final `一到两天：接手首个 Change` section listing active changes and the reading order:

```text
proposal → spec → design → tasks → evidence → retrospective → code/tests
```

The generated document title must be:

```text
# <chip> Security Project Onboarding
```

- [x] **Step 4: Write role checklists**

Each checklist must state:

- What the role approves.
- What the role must not delegate to automation.
- Which Gate the role owns.
- The minimum evidence to review.

- [x] **Step 5: Run onboarding tests**

Run:

```bash
python3 -m unittest tests.test_onboarding_pack -v
```

Expected: 1 test PASS.

- [x] **Step 6: Update progress and checkpoint**

Run all tests and `git diff --check`.

## Task 9: Implement knowledge candidate collection and L0-L3 evaluation

**Files:**
- Create: `chip-security-development-workflow/scripts/collect_knowledge_candidates.py`
- Create: `chip-security-development-workflow/scripts/evaluate_promotion.py`
- Create: `chip-security-development-workflow/tests/test_knowledge_promotion.py`
- Create: `chip-security-development-workflow/knowledge/promotion-log.md`
- Create: `chip-security-development-workflow/knowledge/candidates/.gitkeep`
- Create: `chip-security-development-workflow/knowledge/validated-patterns/.gitkeep`
- Create: `chip-security-development-workflow/knowledge/anti-patterns/.gitkeep`

- [x] **Step 1: Write failing knowledge tests**

Create:

```python
# tests/test_knowledge_promotion.py
import json
import tempfile
import unittest
from pathlib import Path

from scripts.collect_knowledge_candidates import collect_candidates
from scripts.evaluate_promotion import evaluate_candidate


class KnowledgePromotionTest(unittest.TestCase):
    def test_collector_preserves_source_and_does_not_approve(self):
        with tempfile.TemporaryDirectory() as tmp:
            change = Path(tmp) / "change"
            output = Path(tmp) / "knowledge/candidates"
            source = change / "evidence/knowledge-candidates.json"
            source.parent.mkdir(parents=True)
            source.write_text(json.dumps({
                "schema_version": 1,
                "candidates": [{
                    "id": "KCP-DEMO-001",
                    "title": "Bounded shared-memory ring",
                    "current_level": "L1",
                    "target_level": "L2",
                    "category": "transport",
                    "validated_chips": ["demo"],
                    "occurrences": 1,
                    "evidence": ["verification.md"],
                    "applicability": "Single-producer/single-consumer channels",
                    "non_applicability": "Multi-writer channels",
                    "counterexamples": ["Unbounded queue"],
                    "chip_specific_tokens": [],
                    "approval": {"status": "pending", "approved_by": ""}
                }]
            }), encoding="utf-8")
            paths = collect_candidates(change, output)
            data = json.loads(paths[0].read_text(encoding="utf-8"))
            self.assertEqual(data["source_change"], str(change.resolve()))
            self.assertEqual(data["approval"]["status"], "pending")

    def test_l3_ready_candidate_returns_review_not_pass(self):
        candidate = {
            "id": "KCP-DEMO-002",
            "current_level": "L2",
            "target_level": "L3",
            "validated_chips": ["demo"],
            "occurrences": 2,
            "evidence": ["a", "b"],
            "applicability": "Bounded endpoint transport",
            "non_applicability": "Shared multi-writer bus",
            "counterexamples": ["Direct callback transport"],
            "chip_specific_tokens": [],
            "approval": {"status": "pending", "approved_by": ""}
        }
        results = evaluate_candidate(candidate)
        self.assertTrue(any(item.status == "REVIEW" for item in results))
        self.assertFalse(any(item.message == "promotion approved" for item in results))

    def test_l3_rejects_chip_specific_tokens(self):
        candidate = {
            "id": "KCP-DEMO-003",
            "current_level": "L2",
            "target_level": "L3",
            "validated_chips": ["demo"],
            "occurrences": 2,
            "evidence": ["a", "b"],
            "applicability": "Transport",
            "non_applicability": "None",
            "counterexamples": ["Direct callback"],
            "chip_specific_tokens": ["NGU800", "0x12340000"],
            "approval": {"status": "pending", "approved_by": ""}
        }
        results = evaluate_candidate(candidate)
        self.assertTrue(any(item.status == "FAIL" for item in results))
```

- [x] **Step 2: Run tests and verify import failure**

Run:

```bash
python3 -m unittest tests.test_knowledge_promotion -v
```

Expected: ERROR because the scripts do not exist.

- [x] **Step 3: Implement candidate collection**

`collect_candidates(change_root, output_root)` must:

- Read `evidence/knowledge-candidates.json`.
- Require unique candidate IDs.
- Add `source_change` as an absolute path.
- Preserve `approval.status`.
- Refuse to overwrite a different existing candidate with the same ID.
- Write one JSON file per candidate using `<id>.json`.
- Never change `approval.status` to `approved`.

- [x] **Step 4: Implement L0-L3 evaluation**

Use these rules:

```text
L1:
  source change and at least one evidence link

L2:
  L1 conditions
  occurrences >= 2
  at least one validated chip

L3:
  L2 conditions
  applicability non-empty
  non_applicability non-empty
  at least one counterexample
  chip_specific_tokens empty
```

When all mechanical conditions pass:

- Return `PASS` for each condition.
- Return a final `REVIEW` check if approval is pending.
- Return final `PASS` only when an existing approval record names an approver.
- Never modify the candidate file.

For target levels `L4` and `L5`, return:

```text
REVIEW: target level requires the later cross-chip/Skill validation plan
```

Do not claim those levels are implemented in V1.

- [x] **Step 5: Run knowledge tests**

Run:

```bash
python3 -m unittest tests.test_knowledge_promotion -v
```

Expected: 3 tests PASS.

- [x] **Step 6: Update progress and checkpoint**

Run all tests and `git diff --check`.

## Task 10: Migrate the NGU800 MCTP design into a complete OpenSpec pilot

**Files:**
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/.openspec.yaml`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/proposal.md`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/specs/mctp-mailbox-spdm-endpoint/spec.md`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/design.md`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/tasks.md`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/security-workflow.json`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/approvals.json`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/evidence/*`
- Create: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/retrospective.md`
- Modify: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/config.yaml`
- Modify: `gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/development_principles.md`
- Delete after migration: `gsp-pmp-rmp-omp/components/ngu_security/docs/superpowers/specs/2026-06-09-mctp-mailbox-spdm-endpoint-design.md`

- [x] **Step 1: Create the standard OpenSpec change**

Run from `gsp-pmp-rmp-omp/components/ngu_security/docs`:

```bash
openspec new change add-mctp-mailbox-spdm-endpoint --schema spec-driven
```

Expected: the change directory and `.openspec.yaml` are created.

- [x] **Step 2: Write the MCTP proposal**

The proposal must state:

```text
Why:
  Current host tests directly dispatch requester bytes into libspdm responder.
  This bypasses MCTP packetization, mailbox scheduling, backpressure, and
  shared-memory validation needed by the final dual-core design.

What Changes:
  vendor pinned libmctp
  fixed EIDs 0x01/0x02
  simulated mailbox/shared SRAM SPSC rings
  two logical FreeRTOS tasks
  SPDM type 0x05 dispatch
  Control query support with Set Endpoint ID disabled
  bounded static memory and host tests

Impact:
  all code and docs remain under components/ngu_security
  existing ngu_* names remain unchanged
  new files/functions omit ngu_ prefix
  no CodeGraph during this task
```

- [x] **Step 3: Write the MCTP requirements**

Create requirements:

```text
MCTP-REQ-001 Fixed endpoint addressing
MCTP-REQ-002 Packet-only shared channel
MCTP-REQ-003 MCTP fragmentation and reassembly
MCTP-REQ-004 Message Tag request/response matching
MCTP-REQ-005 Static MCTP Control behavior
MCTP-REQ-006 SPDM message type dispatch
MCTP-REQ-007 Independent requester/responder scheduling
MCTP-REQ-008 Bounded static memory
MCTP-REQ-009 Input validation and reset
MCTP-REQ-010 Host end-to-end attestation verification
MCTP-REQ-011 Component-only change scope
MCTP-REQ-012 Naming and development-tool constraints
```

Each requirement must contain concrete scenarios, including:

- requester local EID `0x01`, peer `0x02`
- responder local EID `0x02`, peer `0x01`
- `Set Endpoint ID` returns unsupported and leaves EID unchanged
- 2049-byte MCTP message fragments and reassembles
- requester callback never calls responder dispatch directly
- existing direct-wire test remains unchanged

- [x] **Step 4: Move the full approved MCTP design**

Move the full content of:

```text
components/ngu_security/docs/superpowers/specs/2026-06-09-mctp-mailbox-spdm-endpoint-design.md
```

to the change `design.md`. Preserve all architecture, data flow, error handling,
security boundary, test strategy, and completion criteria sections.

- [x] **Step 5: Write detailed MCTP implementation tasks**

Use these groups:

```text
1. Vendor and pin libmctp
2. Build-time configuration and fixed-EID upstream patch
3. Shared-channel tests and implementation
4. Mailbox binding tests and implementation
5. MCTP endpoint tests and implementation
6. SPDM responder adapter tests and implementation
7. MCTP Control tests
8. SPDM end-to-end MCTP attestation test
9. FreeRTOS static-task wrapper and GSP build verification
10. Evidence, code map, retrospective, and archive checks
```

Each implementation group must include:

```text
write failing test
run and confirm failure
implement minimum behavior
run and confirm pass
update evidence/code-map
```

- [x] **Step 6: Add security workflow metadata**

Use:

```json
{
  "schema_version": 1,
  "change_id": "add-mctp-mailbox-spdm-endpoint",
  "chip": "ngu800",
  "security_project_root": "/home/may-pc/share/code/ngu800/secure/security_-scheme/ngu800_security_solution_flow_with_gpt",
  "baseline_refs": {
    "sources": ["SRC-005", "SRC-008"],
    "threats": [],
    "constraints": ["C-HOST-01", "C-ACCESS-01", "C-BOARD-04"],
    "decisions": ["DEC-0019"],
    "change_requests": [],
    "open_questions": ["OQ-0006"]
  },
  "owners": {
    "feature_owner": "NGU800 security development owner",
    "verification_owner": "NGU800 security verification owner",
    "reviewer": "NGU800 security reviewer"
  }
}
```

Record G2 as approved with:

```json
{
  "status": "approved",
  "approvers": ["project owner"],
  "date": "2026-06-09",
  "evidence": "Design approved in the Codex collaboration session"
}
```

Leave G3-G6 pending.

- [x] **Step 7: Add evidence and retrospective records**

Copy the common templates and preserve the implementation evidence available at
migration time. The original plan assumed implementation had not started, but
the pilot completed before workflow V1 closed, so the record MUST reflect the
actual verified state and MUST NOT erase existing results.

In `known-issues.md`, record:

- real dual-core mailbox registers and SRAM addresses are not available
- cache maintenance and hardware interrupt details remain target-integration work
- dynamic Bus Owner EID assignment is outside V1

Do not mark these as implementation failures; they are approved non-goals or future integration items.

- [x] **Step 8: Update component OpenSpec rules**

Add to `config.yaml`:

```yaml
  proposal:
    - 安全功能 change 必须说明关联的芯片级约束、决策或开放问题。
  design:
    - 安全功能 design 必须包含安全边界、错误恢复、测试策略和事实源绑定。
  tasks:
    - tasks 必须包含 evidence、code-map、retrospective 和 Gate 检查。
```

Update `development_principles.md` so the MCTP special rules link to:

```text
openspec/changes/add-mctp-mailbox-spdm-endpoint/
```

instead of the old Superpowers design filename.

- [x] **Step 9: Validate G2 and OpenSpec**

Run:

```bash
cd /home/may-pc/share/code/ngu800/secure/gsp-pmp-rmp-omp/components/ngu_security/docs
openspec validate add-mctp-mailbox-spdm-endpoint --strict --no-interactive

cd /home/may-pc/share/code/ngu800/secure/security_-scheme/chip-security-development-workflow
python3 scripts/check_openspec_evidence.py \
  --change-root /home/may-pc/share/code/ngu800/secure/gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint \
  --gate G2
```

Expected: OpenSpec validation and all G2 structural checks succeed. If completed
implementation evidence is present, G5 mechanical checks may pass, but G3-G5
remain `REVIEW` until their human approvals are recorded.

- [x] **Step 10: Delete the duplicate MCTP Superpowers design**

Delete:

```text
components/ngu_security/docs/superpowers/specs/2026-06-09-mctp-mailbox-spdm-endpoint-design.md
```

Only after `design.md` has been compared against it and no section is missing.

- [x] **Step 11: Record a no-commit checkpoint**

Run:

```bash
git status --short -- components/ngu_security/docs
git diff --check -- components/ngu_security/docs
```

Expected: only intended OpenSpec migration files and rule updates are listed.

## Task 11: Add end-to-end workflow verification and NGU800 example mapping

**Files:**
- Create: `chip-security-development-workflow/tests/test_end_to_end.py`
- Create: `chip-security-development-workflow/examples/ngu800/project-map.md`
- Modify: `chip-security-development-workflow/openspec/changes/add-chip-security-workflow-v1/tasks.md`
- Delete after OpenSpec migration and final verification:
  - `chip-security-development-workflow/docs/superpowers/specs/2026-06-09-chip-security-development-workflow-design.md`
  - `chip-security-development-workflow/docs/superpowers/plans/2026-06-09-chip-security-development-workflow-v1.md`

- [x] **Step 1: Write the failing end-to-end test**

Create:

```python
# tests/test_end_to_end.py
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
            approvals = json.loads(approvals_path.read_text(encoding="utf-8"))
            approvals["gates"]["G0"] = {
                "status": "approved",
                "approvers": ["Alice"],
                "date": "2026-06-09",
                "evidence": "project review"
            }
            approvals_path.write_text(json.dumps(approvals), encoding="utf-8")
            results = evaluate_project_gate(target, "G0")
            self.assertFalse(any(item.status == "FAIL" for item in results))
            (openspec / "changes").mkdir(parents=True)
            output = target / "onboarding-pack.md"
            build_pack(target, output)
            self.assertTrue(output.is_file())
            self.assertIn("demo-chip", output.read_text(encoding="utf-8"))
```

- [x] **Step 2: Run the end-to-end test**

Run:

```bash
python3 -m unittest tests.test_end_to_end -v
```

Expected: PASS after prior tasks are complete.

- [x] **Step 3: Create the NGU800 project mapping**

Write `examples/ngu800/project-map.md` with links to:

```text
security project:
  security_-scheme/ngu800_security_solution_flow_with_gpt

code repository:
  gsp-pmp-rmp-omp

security component:
  components/ngu_security

component OpenSpec:
  components/ngu_security/docs/openspec

pilot change:
  add-mctp-mailbox-spdm-endpoint
```

Clearly label NGU800 content as an example, not a universal security rule.

- [x] **Step 4: Run the complete V1 test suite**

Run:

```bash
cd /home/may-pc/share/code/ngu800/secure/security_-scheme/chip-security-development-workflow
python3 -m unittest discover -s tests -v
```

Expected: all tests PASS with no skipped tests.

- [x] **Step 5: Run all structural validators**

Run:

```bash
openspec validate add-chip-security-workflow-v1 --strict --no-interactive

python3 scripts/check_openspec_evidence.py \
  --change-root /home/may-pc/share/code/ngu800/secure/gsp-pmp-rmp-omp/components/ngu_security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint \
  --gate G2
```

Expected: both commands succeed.

- [x] **Step 6: Verify no premature Skill or L4/L5 claim exists**

Run:

```bash
find . -path '*/skills/*' -o -name 'validate_skill.py'
rg -n 'current_level.*L[45]|target_level.*L[45].*approved|自动批准' \
  workflow templates scripts knowledge examples openspec
```

Expected:

- No generated Skill directory.
- No `validate_skill.py`.
- No approved L4/L5 candidate.
- Any `自动批准` occurrence is in a prohibition statement.

- [x] **Step 7: Complete the OpenSpec V1 tasks and verification record**

In the common workflow change:

- Mark every actually verified task complete.
- Add `evidence/verification.md`, `evidence/code-map.md`, and `retrospective.md` using the same templates implemented by this plan.
- Run the common evidence checker at G5.

Expected: G5 is `REVIEW` until Feature Owner and Reviewer approvals are recorded.

- [x] **Step 8: Remove temporary duplicate Superpowers documents**

After the common OpenSpec `design.md` and `tasks.md` have been compared with the temporary documents, delete:

```text
docs/superpowers/specs/2026-06-09-chip-security-development-workflow-design.md
docs/superpowers/plans/2026-06-09-chip-security-development-workflow-v1.md
```

The OpenSpec change then becomes the only implementation history for V1.

- [x] **Step 9: Final no-commit checkpoint**

Run:

```bash
git -C /home/may-pc/share/code/ngu800/secure/security_-scheme status --short
git -C /home/may-pc/share/code/ngu800/secure/security_-scheme diff --check

git -C /home/may-pc/share/code/ngu800/secure/gsp-pmp-rmp-omp status --short -- components/ngu_security/docs
git -C /home/may-pc/share/code/ngu800/secure/gsp-pmp-rmp-omp diff --check -- components/ngu_security/docs
```

Expected:

- Only workflow V1 and component OpenSpec pilot files are modified.
- No implementation files outside `components/ngu_security/docs` are changed.
- Both diff checks are silent.

## Follow-up Plans

After this V1 plan is implemented and reviewed:

1. Record formal G3-G5 approvals for workflow V1 and the NGU800 pilot.
2. Apply the initializer, Gates, evidence checks and onboarding flow to a second
   chip.
3. Use cross-chip evidence to evaluate L4 candidates.
4. Create the L5 `chip-security-development` Skill only after that validation.
