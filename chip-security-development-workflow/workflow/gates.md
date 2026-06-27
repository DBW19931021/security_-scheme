# Security Workflow Gates

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Result Semantics

```text
PASS    Mechanically checkable conditions and recorded approval are satisfied.
FAIL    A required artifact, field, link, or evidence item is missing or invalid.
WARN    Work may continue, but the risk or limitation MUST be recorded.
REVIEW  Mechanical conditions pass, but required human approval is pending.
```

Automation MUST NOT convert `REVIEW` to `PASS` by creating an approval.

## Gate Contract

| Gate | Name | Mechanical Conditions | Approval |
|---|---|---|---|
| G0 | Project Ready | Profile, roles, project paths, input manifest | Security Owner |
| G1 | Baseline Ready | Threats, constraints, risks, baseline, decisions, questions | Security Owner and Architecture Owner |
| G2 | Change Ready | proposal, spec, design, baseline links, owners | Feature Owner and Architecture Reviewer |
| G3 | Implement Ready | tasks, test strategy, code scope, risk assessment | Feature Owner and Verification Owner |
| G4 | Verify Ready | completed tasks, verification evidence, code map | Verification Owner and Reviewer |
| G5 | Archive Ready | retrospective, known issues, residual risks, evidence | Feature Owner and Reviewer |
| G6 | Promote Ready | candidate metadata and target-level evidence | Promotion-level owner |

## Evaluation Rules

- Missing mandatory structure MUST return `FAIL`.
- Missing human approval MUST return `REVIEW`.
- A recorded approval MUST name approvers, date, and evidence.
- A Gate result MUST include the checked path and a stable check code.
- `WARN` and `REVIEW` MUST remain visible in generated reports.
- G6 MUST NOT approve security architecture, key, lifecycle, debug,
  manufacturing, or knowledge-promotion decisions automatically.

## Gate Dependencies

G1 depends on G0. G3 depends on G2. G4 depends on G3. G5 depends on G4.
G6 depends on an archived or otherwise explicitly reviewed source change.
