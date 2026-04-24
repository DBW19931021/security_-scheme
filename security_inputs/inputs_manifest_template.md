# Inputs Manifest Template

Use this file as the working source inventory for NGU800 security design inputs.

When you add a new reference, record not only what the file is, but also how it should be used.
This lets the skill distinguish between:
- details that must be followed
- areas that are only informative
- sections that should be ignored
- project-specific remarks that override the default reading of the document

## Source Inventory

| Source ID | Path | Title / Topic | Owner | Version / Date | Confidence | Use Policy | Applies To | Must Follow | Optional Reference | Ignore / Out of Scope | Supersedes | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SRC-001 | `soc_arch/` |  |  |  | `confirmed` / `draft` / `assumed` | `authoritative` / `preferred` / `reference-only` / `partial` |  |  |  |  |  |  |
| SRC-002 | `boot_flow/` |  |  |  | `confirmed` / `draft` / `assumed` | `authoritative` / `preferred` / `reference-only` / `partial` |  |  |  |  |  |  |

## Field Guidance

| Field | How to use it |
|---|---|
| `Confidence` | Records whether the source is frozen, draft, or assumption-level |
| `Use Policy` | Tells the skill how strongly to rely on this source overall |
| `Applies To` | List the design chapters or domains this source is relevant to |
| `Must Follow` | State the parts that must be obeyed, even if other sections are ignored |
| `Optional Reference` | State the parts that are helpful background but not binding |
| `Ignore / Out of Scope` | State the parts that should not be used in the design |
| `Notes` | Free-form project remarks, caveats, or interpretation rules |

## Recommended Confidence Meanings

| Value | Meaning |
|---|---|
| `confirmed` | Review-approved or authoritative project input |
| `draft` | Useful but not yet frozen or not clearly approved |
| `assumed` | Working assumption used to keep the design moving |

## Recommended Use Policy Meanings

| Value | Meaning |
|---|---|
| `authoritative` | Default source of truth for the covered topic unless a newer approved source replaces it |
| `preferred` | Strong reference source; use it unless a higher-priority source conflicts |
| `reference-only` | Informative background; do not treat as binding by default |
| `partial` | Only some parts are valid; rely on `Must Follow`, `Optional Reference`, and `Ignore / Out of Scope` |

## Registration Rule For New Inputs

When a new file is added:
1. assign the next `SRC-xxx`
2. fill in the file path and title
3. record the user-provided remarks in `Must Follow`, `Optional Reference`, `Ignore / Out of Scope`, and `Notes`
4. if it updates an older file, fill in `Supersedes`
5. add an entry in `Change Intake`

## Conflict Log

| Conflict ID | Topic | Preferred Source ID | Other Source ID | Reason | Impacted Chapters | Status |
|---|---|---|---|---|---|---|
| CF-001 |  |  |  |  |  | `open` / `resolved` |

## Change Intake

| Change ID | Date | New / Updated Sources | Summary of Change | Expected Impact | Freeze Risk | Registration Notes |
|---|---|---|---|---|---|---|
| CHG-001 |  |  |  |  | `yes` / `no` / `unknown` |  |

## Open Questions

| Q ID | Topic | Blocking Area | Needed From | Status |
|---|---|---|---|---|
| Q-001 |  |  |  | `open` / `closed` |
