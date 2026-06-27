# OpenSpec Integration

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Standard Artifacts

The workflow preserves OpenSpec's standard sequence:

```text
proposal → specs → design → tasks
```

- `proposal` states why the change exists and what it affects.
- `spec` uses stable requirement IDs and `WHEN`/`THEN` scenarios.
- `design` defines architecture, security boundaries, alternatives, errors,
  recovery, resources, and test strategy.
- `tasks` tracks implementation and verification progress.

## Security Extensions

Each material security change MUST add:

```text
security-workflow.json
approvals.json
evidence/
retrospective.md
```

`security-workflow.json` binds the change to Source, Threat, Constraint,
Decision/CR, and Open Question IDs. `evidence` contains verification,
code-to-test mapping, known issues, problem records, and promotion candidates.

## Superpowers Handoff

- Confirmed brainstorming output MUST move into `design.md`.
- A writing-plans result MUST move into `tasks.md`.
- A non-trivial debugging result MUST move into a problem record.
- Verification-before-completion output MUST move into
  `evidence/verification.md`.
- Temporary Superpowers files SHOULD be removed after complete migration.

## Archive Readiness

OpenSpec validation is necessary but not sufficient. G5 also requires completed
tasks, evidence, code map, known issues, retrospective, and human approval.
