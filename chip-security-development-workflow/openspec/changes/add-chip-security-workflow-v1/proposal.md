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

None.

## Impact

- Adds files only under `security_-scheme/chip-security-development-workflow`.
- The pilot later modifies only `gsp-pmp-rmp-omp/components/ngu_security/docs`.
- Uses OpenSpec 1.2.0 and Python 3.8 standard library.
- Does not approve or alter NGU800 security architecture decisions.
