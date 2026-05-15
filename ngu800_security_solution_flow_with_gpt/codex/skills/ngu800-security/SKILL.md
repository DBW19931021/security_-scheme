---
name: ngu800-security-design
description: Use when applying, synchronizing, or validating approved NGU800/NGU800P security design changes involving secure boot, attestation, keys, lifecycle/debug, mailbox, eFuse, manufacturing, implementation design, code rules, traceability, or incremental updates.
---

# NGU800 Security Design Skill V2.1

## 1. Purpose

This skill is dedicated to NGU800 / NGU800P security方案裁决落实与工程落地。

It is not a generic document-writing skill, and it is not the owner of architecture decisions.
It must apply approved security decisions into a constraint-driven, reviewable, implementation-oriented design package.

This skill supports:

- 输入资料持续补充后的增量更新
- 先抽约束，再定基线
- 再做章节级详设
- 再做实现级详设
- 再把实现级详设全量合入 `10_full_design.md`
- 再形成 code rules / traceability
- 再指导 Codex 后续代码开发

## 2. Core Rule

The agent MUST NOT directly generate a full security design document from raw inputs.

### 2.1 Decision Authority Gate

ChatGPT / user / security owner owns architecture decisions.
Codex owns repository application, consistency sync, implementation design expansion, validation, traceability, and code implementation.

Codex MUST NOT create or upgrade `[CONFIRMED]` security decisions unless the source is one of:

1. user-frozen conclusion;
2. accepted Change Request;
3. signed-off project baseline;
4. official eHSM manual / TRM / Host API / RTL interface document;
5. explicit `00_project/decision_log.md` entry.

If none of the above exists, Codex may only:

- draft `[PROPOSED]` wording for ChatGPT / owner review;
- mark working assumptions as `[ASSUMED]`;
- mark unresolved dependencies as `[TBD]`;
- create or update CR impact analysis without treating the design as frozen.

Codex MUST NOT treat its own generated prose as a source of truth.

### 2.2 Change Request Gate

Change Request gate:

凡是满足以下任一条件的变更，必须先生成 Change Request，不得直接修改安全方案正文：

1. 影响两个以上文件；
2. 影响 `security_workflow/02_baseline.md`；
3. 影响 boot / key / cert / attestation / debug / interface / manufacturing 任一安全主路径；
4. 影响对应的 implementation design、code rules、traceability 或 master/full design 输出。

此类变更必须先使用 `change_requests/CR_template.md` 建立 CR，并结合 `05_traceability/file_sync_checklist.md` 与 `05_traceability/design_impact_matrix.md` 完成影响分析，再执行正文修改。

The CR must state whether it is:

- `proposal-only`: only preparing material for ChatGPT / owner review;
- `accepted-for-application`: applying already approved decisions into workflow files;
- `implementation-only`: applying design conclusions into code rules, tests, or code.

### 2.3 Mandatory Pipeline

The mandatory pipeline is:

```text
security_inputs/inputs_manifest.md
    ↓
security_workflow/01_constraints.md
    ↓
security_workflow/02_baseline.md
    ↓
security_workflow/03_detailed_design/
    ↓
security_workflow/04_impl_design/
    ↓
security_workflow/03_detailed_design/10_full_design.md
    ↓
security_workflow/05_code_rules.md
    ↓
security_workflow/06_traceability.md
    ↓
security_workflow/04_change_impact.md
```

If this order is skipped, the output is incomplete.

### 2.4 Work Modes

Use the narrowest mode that fits the request:

| Mode | When to use | Codex may do | Codex must not do |
|---|---|---|---|
| `context-pack` | ChatGPT / project owner needs material for scheme design | summarize inputs, conflicts, open questions, impact matrix | freeze baseline or write `[CONFIRMED]` decisions |
| `proposal-only` | a new design direction is being explored | create CR draft, `[PROPOSED]` constraints, review questions | modify master/full design as final truth |
| `accepted-apply` | approved CR or decision exists | update constraints, baseline, chapters, impl design, rules, traceability | introduce new decisions outside the approved source |
| `implementation-only` | design is already approved and code work starts | update code rules, implementation plan, code, tests | reopen architecture decisions silently |

## 3. Scope

This skill covers:

- Root of Trust
- BootROM / SEC1 / SEC2 / eHSM trust chain
- secure / non-secure boot
- firmware integrity / confidentiality / anti-rollback / recovery
- key hierarchy and certificate hierarchy
- device identity and remote attestation
- lifecycle and secure debug
- mailbox and security service interface
- host interaction boundary
- board-level security (BMC / OOB-MCU / SMBus / sideband)
- manufacturing / provisioning / RMA
- implementation design for eFuse / key / FW header / mailbox / SPDM

## 4. Input Reading Rules

### 4.1 inputs_manifest first
Always read:

```text
security_inputs/inputs_manifest.md
```

before reading raw design sources.

### 4.2 Source precedence
Unless manifest overrides it, use this precedence:

1. user-frozen conclusion
2. signed-off project baseline
3. official eHSM manuals / TRMs / Host API
4. boot / architecture / subsystem docs
5. draft notes
6. assumptions

### 4.3 Conflict handling
When two sources conflict:

- do not silently merge
- identify both sources
- choose current baseline according to precedence
- mark unresolved issue as `[TBD]`
- list impacted chapters and implementation files

### 4.4 Missing input handling
If some inputs are missing:

- continue only with explicit `[ASSUMED]` items
- do not present assumptions as confirmed fact
- mark freeze impact if missing input affects hardware or interface closure

## 5. Mandatory Workflow

## Stage A / B - Constraint Extraction
Generate:

```text
security_workflow/01_constraints.md
```

Each constraint must include:

- Constraint ID
- Category
- Statement
- Source
- Strength
- Impact
- Status
- Evidence
- Decision Rationale
- Chapter Binding
- Impl Binding

Category set:
- boot_chain
- trust_boundary
- key_cert_attestation
- lifecycle_debug
- host_boundary
- board_security
- update_rollback
- manufacturing_provisioning
- mailbox_interface
- efuse_otp
- impl_design

Strength set:
- HARD
- SOFT
- REFERENCE

Status set:
- [PROPOSED]
- [CONFIRMED]
- [ASSUMED]
- [TBD]

## Stage C - Architecture Baseline
Generate:

```text
security_workflow/02_baseline.md
```

It must define:

- Root of Trust
- First Mutable Stage
- First Cryptographic Verifier
- BootROM / SEC1 / SEC2 / eHSM / Host / Board boundaries
- manufacturing / provisioning baseline
- dual algorithm strategy
- freeze-sensitive items
- adopted vs rejected decisions
- whether it is ready to enter detailed design

Baseline updates that introduce new `[CONFIRMED]` decisions require an accepted decision source from the Decision Authority Gate.
Without that source, baseline content must stay `[PROPOSED]`, `[ASSUMED]`, or `[TBD]`.

## Stage D1 - Chapter-Level Detailed Design
Create or update chapter files under:

```text
security_workflow/03_detailed_design/
```

Preferred structure:
- 00_chapter_plan.md
- 00_architecture.md
- 01_boot.md
- 02_key_cert.md
- 03_attestation.md
- 04_lifecycle_debug.md
- 05_board_security.md
- 06_interface.md
- 07_manufacturing_rma.md
- 08_failure_recovery.md
- 09_risks_open_issues.md

Each chapter must include:

- 本章目标
- 生效约束 ID
- 生效 baseline 决策
- Mermaid graph TD
- Mermaid sequenceDiagram
- 设计正文
- 表格 / 结构 / 位图（如有依据）
- 对实现层的影响
- 冻结项
- 开放问题

Chapter prose must bind back to constraints and baseline decisions.
If a chapter needs a new architecture decision, stop and create or update a CR instead of inventing the decision in the chapter.

## Stage D2 - Implementation-Level Design
Create or update implementation files under:

```text
security_workflow/04_impl_design/
```

Mandatory themes:
- eFuse / OTP
- key hierarchy
- firmware header / image format
- mailbox interface
- SPDM report
- lifecycle control
- manufacturing / provisioning

This stage is mandatory before code development.
Implementation design translates approved architecture into fields, interfaces, state machines, and testable rules. It must not become a hidden source for new architecture policy.

CR-0005 source-of-truth rule:

- `security_workflow/04_impl_design/*.md` are editing shards / extracted implementation shards.
- Implementation details from these shards MUST be fully synchronized into `security_workflow/03_detailed_design/10_full_design.md`.
- `10_full_design.md` is the complete detailed design and code landing specification.
- If an implementation shard conflicts with `10_full_design.md`, accepted CR / decision_log / official TRM / `10_full_design.md` win; the shard must be corrected.
- Do not use summaries in `10_full_design.md` as a substitute for implementation fields, structures, state machines, command tables, error codes, manufacturing rules, or source-conformance matrices.

## Stage E - Code Rules
Generate:

```text
security_workflow/05_code_rules.md
```

Convert architecture/design conclusions into MUST / MUST NOT / SHOULD development rules.

Code rules must point developers to `10_full_design.md` as the primary code landing specification, while allowing `04_impl_design` only as synchronized editing shards.

## Stage F - Traceability
Generate:

```text
security_workflow/06_traceability.md
```

Traceability must connect:

```text
Source → Constraint → Baseline → Detailed Design → Impl Design → Code Module → Test
```

## Stage G - Final Check / Incremental Update
Generate or update:

```text
security_workflow/04_change_impact.md
```

Must verify:
- constraint consistency
- baseline consistency
- chapter consistency
- impl consistency
- `04_impl_design` to `10_full_design.md` full-content synchronization
- no unsupported `[CONFIRMED]`
- no Codex-invented architecture decisions
- Mermaid renderability
- PDF readiness
- code rules freshness
- traceability freshness

## 6. NGU800 Default Hard Rules

Unless later explicitly changed:

1. eHSM is the first cryptographic verifier.
2. BootROM is the earliest SoC code but does not own private keys.
3. BootROM must not implement complex signature verification or key management logic.
4. SEC / C908 is the boot control plane.
5. eHSM provides crypto / key / OTP / lifecycle / debug auth / counter / verify services.
6. Host is untrusted for security decisions.
7. Host may deliver firmware but must not release execution.
8. All secure services must go through mailbox or defined secure interface.
9. USER lifecycle must disable unauthorized debug.
10. Manufacturing / provisioning must define key injection, lock, audit, and lifecycle transition.
11. The scheme must consider both GM and international algorithm stacks.

These rules are default guardrails for review and consistency. They do not allow Codex to mark a new project-specific field, bit layout, command ID, lifecycle encoding, key slot, certificate format, or RTL permission model as `[CONFIRMED]` without an approved source.

## 7. Output Style

- default language: Simplified Chinese
- tone: formal engineering design
- style: implementation-oriented, review-friendly
- tables preferred for mappings
- Mermaid diagrams must be renderable
- mark review drafts as `[PROPOSED]`
- mark assumptions as `[ASSUMED]`
- mark unresolved items as `[TBD]`
- never use `[CONFIRMED]` for Codex-only inference
- do not keep empty headings
- do not leave template bullets unresolved

## 8. Workflow Change Rule

If workflow or directory layout changes, first inspect:

```text
tree -a -L 4
codex/skills/ngu800-security/SKILL.md
codex/skills/ngu800-security/prompts/
codex/skills/ngu800-security/templates/
README_使用说明.md
security_workflow/01_constraints.md
security_workflow/02_baseline.md
```

Then output:
1. current workflow version
2. affected files
3. synchronized change list
4. updated package

Never silently change the workflow.
