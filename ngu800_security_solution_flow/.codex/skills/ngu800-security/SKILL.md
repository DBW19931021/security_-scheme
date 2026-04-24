---
name: ngu800-security-design
description: Generate NGU800 security design by first extracting constraints, then building a baseline, then producing full detailed design and PDF-ready markdown. Use for secure boot, attestation, key hierarchy, lifecycle/debug, board security, mailbox, efuse, and update design.
---

# NGU800 Security Design Skill (Constraint-First Edition)

## 1. Purpose

This skill is for NGU800 security detailed design only.
Its primary objective is not generic writing. It must produce a constraint-driven, implementation-oriented Chinese design document that is close to project review quality.

This skill must support:
- initial full security design generation
- incremental update after new inputs are added
- single-domain deep dive

The skill must prioritize:
1. extracting hard constraints from project inputs
2. resolving or isolating conflicts
3. building a stable design baseline
4. generating the detailed design document
5. producing PDF-ready markdown

## 2. Hard Rule: No Direct Full Design from Raw Inputs

The agent MUST NOT directly generate a full security design document from raw inputs.

The mandatory order is:

1. Input inventory / source reading
2. Constraint extraction
3. Constraint conflict handling
4. Design baseline generation
5. Detailed design generation
6. PDF-ready markdown rendering

If this order is skipped, the result must be considered incomplete.

## 3. Scope

This skill covers the following NGU800 security domains:
- Root of Trust
- BootROM / SEC1 / SEC2 / eHSM trust chain
- secure boot and non-secure boot
- firmware image format
- firmware integrity / confidentiality / anti-rollback / recovery
- key hierarchy and certificate hierarchy
- device identity and remote attestation
- lifecycle and secure debug
- mailbox and security service interface
- host interaction boundary
- board-level security (BMC / OOB-MCU / SMBus / sideband)
- manufacturing / provisioning / RMA
- risks / freeze items / open issues

## 4. Input Reading Rules

### 4.1 Use inputs_manifest first
If `security_inputs/inputs_manifest.md` exists, read it first and treat it as the active source inventory.

### 4.2 Source precedence
Use the following precedence unless explicitly overridden:
1. signed-off project baseline / frozen review conclusion
2. IP manuals with clear ownership/version
3. architecture or interface documents
4. draft notes / informal notes
5. assumptions

### 4.3 Conflict handling
When two sources conflict:
- do not silently merge
- identify both sources
- choose current baseline according to precedence
- mark the issue as `[TBD]` if unresolved
- list impacted chapters

### 4.4 Missing input handling
If some inputs are missing:
- continue only with explicit `[ASSUMED]` items
- do not present assumption as confirmed fact
- mark freeze impact if the missing input affects hardware or interface closure

## 5. Mandatory Workflow

## Stage A - Source Inventory
Output:
- input status table
- source inventory summary
- missing inputs list

## Stage B - Constraint Extraction
Before any design writing, generate a constraint table.
Mandatory categories:
- boot chain
- trust boundary
- key / cert / attestation
- lifecycle / debug
- host boundary
- board security
- update / anti-rollback
- manufacturing / provisioning
- interface / mailbox / efuse

Each constraint must include:
- ID
- category
- statement
- source
- strength
- impact
- status

Strength must be one of:
- HARD
- SOFT
- REFERENCE

Status must be one of:
- [CONFIRMED]
- [ASSUMED]
- [TBD]

## Stage C - Design Baseline
Generate a design baseline document before the main design.
It must include:
- Root of Trust
- first mutable stage
- first cryptographic verifier
- BootROM responsibilities
- SEC1 responsibilities
- SEC2 responsibilities
- eHSM responsibilities
- Host trust model
- board trust model
- dual-algorithm strategy
- freeze-sensitive items

## Stage D - Detailed Design
Only after Stage B and C are complete, generate the full detailed design.

## Stage E - Rendering
Final output must be:
- structured Markdown
- Chinese formal report style
- Mermaid diagrams included
- suitable for Pandoc export to PDF

## 6. Output Contract

### 6.1 Language and tone
- default language: Simplified Chinese
- tone: formal design specification
- style: implementation-oriented, review-friendly

### 6.2 Must include when input allows
- module responsibility split
- trust boundaries
- control flow
- data flow
- failure handling
- structures and fields
- risk / dependency / freeze items
- concise source traceability

### 6.3 Must output concrete structures when possible
If enough evidence exists, directly output:
- C-like structures
- report fields
- mailbox request/response structures
- efuse plan table
- image header format
- lifecycle table
- rollback counter table

### 6.4 Per-chapter diagram rule
For each major chapter in full-design mode, include:
- one Mermaid `graph TD` architecture diagram
- one Mermaid `sequenceDiagram` interaction/timing diagram

Diagrams must reflect real design conclusions, not generic placeholders.

### 6.5 PDF readiness
The final markdown must be compatible with Pandoc export.
Avoid:
- HTML-only layout tricks
- screenshots as core diagrams
- renderer-specific Mermaid hacks

## 7. Chapter Order for Full Design

1. 文档概述
2. 输入材料与设计基线
3. 安全目标、保护资产与威胁边界
4. NGU800 安全总体架构
5. Root of Trust、密钥体系与证书体系
6. 安全启动详细设计
7. 固件完整性、保密性、防回滚与恢复
8. 设备身份与远程度量证明设计
9. 安全调试与生命周期控制
10. 板级安全设计
11. 内外部接口设计
12. 制造、灌装、部署与 RMA
13. 失败处理与异常场景
14. 风险、依赖、冻结项与开放问题
15. 附录

## 8. Required Intermediate Files

The preferred workflow should produce or update these files:
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design.md`

If the task is incremental update, also produce:
- `security_workflow/04_change_impact.md`

Do not skip directly to `03_detailed_design.md` unless `01_constraints.md` and `02_baseline.md` already exist and are still valid.

## 9. Domain-Specific Minimum Rules

### 9.1 Secure boot
Must clearly answer:
- who verifies SEC1
- who verifies later firmware
- who decides execution release
- who checks rollback floor
- whether confidentiality is mandatory

### 9.2 Attestation
Must clearly answer:
- what identity root is used
- what cert model is used
- what report fields are signed
- how nonce and session bind the report

### 9.3 Lifecycle / debug
Must clearly answer:
- what each lifecycle allows
- how debug is opened
- what is impossible in PROD

### 9.4 Board security
Must clearly answer:
- BMC trust level
- OOB-MCU trust level
- sideband restrictions
- whether board binding is required

### 9.5 Mailbox / interface
Must clearly answer:
- caller/callee
- lifecycle restrictions
- error model
- ownership boundary

## 10. Incremental Update Mode

When new inputs are added:
1. update source inventory
2. update constraint table
3. identify changed constraints
4. produce change impact summary
5. regenerate only impacted sections
6. refresh risk / freeze / open issue tables

## 11. Templates

Use templates under `templates/`:
- constraint_table_template.md
- baseline_template.md
- full_design_template.md
- change_impact_template.md

These templates are mandatory structural guides.

## 12. Quality Bar

A good output from this skill must satisfy all of the following:
- the design is clearly derived from explicit constraints
- assumptions are visible and isolated
- freeze-sensitive items are obvious
- document sections are consistent with one another
- engineering teams can directly use the result as implementation input
