# Knowledge Promotion

## Purpose

This document is normative for `chip-security-development-workflow` V1.
The approved OpenSpec design is the source for rationale; this file defines
the executable contract.

## Levels

```text
L0 Raw observation
L1 Current-change lesson
L2 Current-chip project pattern
L3 Cross-chip candidate pattern
L4 Validated general pattern
L5 Skill rule, template, or script
```

## Promotion Process

```text
automatic discovery
→ mechanical condition checks
→ review packet
→ 人工批准
```

Automation MAY collect, normalize, compare, and evaluate candidates. It MUST
leave approval state unchanged.

## Conditions

- L1 requires a source change and evidence.
- L2 requires repeated occurrence in the current chip project.
- L3 requires applicability, non-applicability, evidence, a counterexample,
  and removal of chip-specific paths, names, and addresses.
- L4 requires a second-chip validation or authoritative independent evidence.
- L5 requires stable inputs and outputs, forward validation, failure handling,
  and a Skill Maintainer approval.

V1 implements mechanical evaluation only through L3. Requests for L4 or L5
MUST return `REVIEW` and point to the later cross-chip validation plan.

## Prohibited Generalization

Register addresses, core names, eFuse layouts, product-only policy, and
unreviewed security decisions MUST remain project facts or examples. They MUST
NOT become universal Skill rules.
