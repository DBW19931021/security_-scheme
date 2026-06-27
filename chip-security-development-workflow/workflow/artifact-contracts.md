# Artifact Contracts

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Chip Project Artifacts

A project MUST include:

```text
workflow-project.json
approvals.json
00_project/
security_inputs/inputs_manifest.md
security_workflow/
system_verification/
```

The project manifest MUST record chip name, security-project root, code
repository root, OpenSpec root, and required owners.

## OpenSpec Change Artifacts

A material security change MUST include:

```text
proposal.md
specs/<capability>/spec.md
design.md
tasks.md
security-workflow.json
approvals.json
evidence/verification.md
evidence/code-map.md
evidence/known-issues.md
evidence/knowledge-candidates.json
retrospective.md
```

`proposal.md` owns motivation and impact. `spec.md` owns verifiable behavior.
`design.md` owns the feature architecture. `tasks.md` owns implementation
progress. `verification.md` owns executed evidence. `retrospective.md` owns
the completed-change learning record.

## Stable IDs

The following objects MUST use stable IDs:

```text
SRC-*   source
THR-*   threat
C-*     constraint
DEC-*   decision
CR-*    change request
OQ-*    open question
*-REQ-* OpenSpec requirement
PRB-*   problem record
KCP-*   knowledge candidate
```

## Link Contract

Traceability SHOULD use relative paths and exact IDs. A link checker MAY verify
existence and status consistency, but MUST NOT decide whether a security
conclusion is technically correct.

## Machine State

JSON files carry machine-readable state. Markdown carries human-reviewable
reasoning. Initializers MUST create Gate approvals as `pending`; they MUST NOT
invent confirmed decisions, owners, evidence, or approvals.
