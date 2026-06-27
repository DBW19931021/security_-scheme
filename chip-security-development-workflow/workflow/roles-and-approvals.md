# Roles and Approvals

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Roles

### Security Owner

Owns the project security objective, P0/P1 readiness, security baseline
approval, release risk acceptance, and high-impact knowledge promotion.

### Architecture Owner

Owns trust boundaries, architecture decisions, baseline consistency, and the
decision whether an implementation question requires a Change Request.

### Feature Owner

Owns one OpenSpec change, its scope, task truthfulness, implementation
deviations, known issues, and retrospective.

### Verification Owner

Owns the test strategy, execution environment, requirement coverage,
unverified-item disclosure, and residual-risk evidence.

### Reviewer

Independently reviews design, code map, evidence, and archive readiness.

### Knowledge/Skill Maintainer

Owns knowledge candidate structure, promotion records, reusable templates,
forward validation, and the final Skill package after L4 evidence exists.

## Approval Rules

- Approvals MUST name a person or accountable role.
- Approvals MUST include a date and evidence reference.
- AI MAY prepare an approval packet but MUST NOT approve on behalf of a role.
- The author SHOULD NOT be the sole Reviewer for G4 or G5.
- Security architecture, key, lifecycle, debug, manufacturing, and promotion
  decisions require explicit human approval.
- A missing approval is `REVIEW`, not `PASS`.

## Separation of Duties

The Feature Owner and Verification Owner MAY be the same person in a small
project, but G4 and G5 MUST still identify an independent Reviewer.
