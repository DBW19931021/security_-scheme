# Chip Security Development Lifecycle

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Chip-Level Lifecycle

Every new chip security project MUST use these phases:

| Phase | Purpose | Required Exit |
|---|---|---|
| P0 | Project initialization | Project profile, roles, paths, and input inventory exist |
| P1 | Inputs and threat analysis | Sources, threats, constraints, risks, and open questions are recorded |
| P2 | Architecture and baseline | Security baseline, decisions, detailed design, code rules, and traceability are reviewable |
| P3 | Capability planning | Capability map, dependencies, owners, risks, and roadmap are defined |
| P4 | Feature delivery | Material capabilities use the feature-level OpenSpec loop |
| P5 | System security verification | Integration, threat coverage, residual risks, and release blockers are recorded |
| P6 | Release and knowledge extraction | Retrospectives and promotion candidates are reviewed |

The project MUST NOT treat a later phase as complete while an earlier blocking
Gate has status `FAIL`.

## 功能级 OpenSpec 循环

Every material feature MUST follow:

```text
Intake
→ Baseline Gate
→ Proposal
→ Spec and Design
→ Tasks
→ Development
→ Verification
→ Retrospective
→ Archive
→ Knowledge Promotion
```

Security behavior, trust boundaries, protocols, keys, boot, lifecycle,
manufacturing, attestation, or public ABI changes MUST use the full loop.

Spelling, formatting, and behavior-neutral documentation changes MAY use a
simplified path. The change record MUST state why the full path is unnecessary.

## Change Routing

- A local implementation issue MUST stay in the OpenSpec change.
- A confirmed feature-design change MUST update the OpenSpec spec or design.
- A chip-level architecture change MUST stop implementation and enter the
  Security Workflow Change Request process.
- A reusable lesson MUST enter knowledge promotion only after retrospective
  evidence exists.

## Completion

P6 is complete only when system verification, release risks, known issues,
retrospectives, and promotion candidates have explicit owners and statuses.
