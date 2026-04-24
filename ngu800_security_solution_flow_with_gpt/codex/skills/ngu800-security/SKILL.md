---
name: ngu800-security-design
description: Generate NGU800 security design through constraints, baseline, chapter-level detailed design, implementation design, code rules, traceability, and incremental update. Use for secure boot, attestation, key hierarchy, lifecycle/debug, board security, mailbox, efuse, manufacturing, and update design.
---

# NGU800 Security Design Skill V2

## 1. Purpose

This skill is dedicated to NGU800 / NGU800P security方案设计与工程落地。

It is not a generic document-writing skill.  
It must produce a constraint-driven, reviewable, implementation-oriented security design package.

This skill supports:

- 输入资料持续补充后的增量更新
- 先抽约束，再定基线
- 再做章节级详设
- 再做实现级详设
- 再形成 code rules / traceability
- 再指导 Codex 后续代码开发

## 2. Core Rule

The agent MUST NOT directly generate a full security design document from raw inputs.

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
security_workflow/05_code_rules.md
    ↓
security_workflow/06_traceability.md
    ↓
security_workflow/04_change_impact.md
```

If this order is skipped, the output is incomplete.

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

## Stage D1 - Chapter-Level Detailed Design
Generate chapter files under:

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

## Stage D2 - Implementation-Level Design
Generate implementation files under:

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

## Stage E - Code Rules
Generate:

```text
security_workflow/05_code_rules.md
```

Convert architecture/design conclusions into MUST / MUST NOT / SHOULD development rules.

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
- no unsupported `[CONFIRMED]`
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

## 7. Output Style

- default language: Simplified Chinese
- tone: formal engineering design
- style: implementation-oriented, review-friendly
- tables preferred for mappings
- Mermaid diagrams must be renderable
- mark assumptions as `[ASSUMED]`
- mark unresolved items as `[TBD]`
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
