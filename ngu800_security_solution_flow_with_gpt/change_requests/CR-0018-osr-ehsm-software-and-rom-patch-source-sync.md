# CR-0018 OSR eHSM Software And ROM Patch Source Sync

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0018` |
| Title | OSR eHSM 软件代码与 eHSM4.0 ROM Patch 机制输入源同步 |
| Status | `applied` |
| Mode | `accepted-for-application` |
| Owner | user / security owner |
| Reviewer | owner-review-pending |
| Created Date | 2026-06-27 |
| Source / Context Pack | `security_inputs/sw/`; `security_inputs/ip_manuals/ehsm/eHSM4.0 Patch方案.pdf`; 用户 2026-06-27 明确说明 |
| Related Decision ID | `DEC-0023` |

## 2. 背景

用户在 `security_inputs/` 中新增 OSR eHSM 软件相关代码、文档和工具，并说明后续 HSM 代码已经提供的安全服务原则上以该代码为准，安全方案需要适配该代码。用户同时在 `security_inputs/ip_manuals/ehsm/` 中新增 `eHSM4.0 Patch方案.pdf`，作为 OSR ROM 代码 patch 机制的输入资料。

本 CR 只同步输入源、约束和影响分析，不直接把 OSR 代码中的所有字段、命令、key ID、OTP offset、错误码和工具 CLI 一次性升级为完整方案冻结项。字段级适配必须在后续 CR 中逐项对照代码、TRM、Host API 和现有 `10_full_design.md` 后执行。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| inputs_manifest 摘要 | 已有 `SRC-006/SRC-007` TRM 与 `SRC-008` 当前方案 2.0；本 CR 新增 `SRC-009 OSR eHSM 软件代码包` 与 `SRC-010 eHSM4.0 ROM Patch 方案` |
| constraints 摘要 | 已有 `C-SRC-01`、`C-BOOT-06/07/08`、`C-EHSM-01`；本 CR 新增 OSR 软件事实源和 patch 机制约束 |
| baseline 摘要 | eHSM TRM 是 physical header / OTP / key / counter 事实源；本 CR 要求后续把 OSR 软件代码纳入 source-conformance baseline |
| 相关详设章节 | boot、key/cert、interface、manufacturing/RMA、full design |
| 相关实现级文档 | `efuse_key_fw_header_design.md`; `mailbox_if.md`; `ehsm_source_conformance_matrix.md`; `manufacturing_provisioning.md` |
| 已知冲突 | 暂未发现必须立即推翻现有 Root of Trust / Host boundary / SEC1+SEC2 sign+encrypt 口径的冲突 |
| 待关闭 TBD | OSR 代码与 `10_full_design.md` 的命令字段、OTP/key/counter、manifest ABI、image tool CLI、patch OTP layout 逐项差异仍需后续 CR 关闭 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| OSR eHSM 软件事实源 | `security_inputs/sw/` 中 BL/FW/Host/API/tool 代码和随包文档作为 eHSM 已提供安全服务、mailbox 命令、Host API、secure boot verify/upgrade、OTP/key/counter、debug/lifecycle 等实现事实源 | 代码是后续可调用服务和字段结构的实际边界；方案不能继续只按旧 TRM 抽象口径落地 | `[CONFIRMED]` |
| 适配原则 | 方案和实现应优先适配 OSR eHSM 已提供服务；若现有方案需要 eHSM 不支持的能力，应先登记差异并开 customization / wrapper / policy CR | 防止设计出不存在的 eHSM 服务或并列 ABI | `[CONFIRMED]` |
| Patch 机制 | eHSM4.0 ROM patch 机制是硬件 BOOT 读取 OTP patch 表并在 CPU 取 IROM 指令时无感替换的机制，不是 Host/SEC 运行期软件热补丁入口 | Patch 作用在 ROM 指令路径，若误当运行期可写机制会形成安全旁路 | `[CONFIRMED]` |
| Patch 字段冻结 | Patch_en、Patch_addr、Patch_data 的最终 OTP offset、行数、enable 编码和量产配置流程必须与 RTL/eHSM owner 对齐后再进入实现级字段冻结 | PDF 给出方案方向和示例布局，但 NGU 集成还需确认 OTP 空间、烧录权限和验收流程 | `[TBD]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_inputs/inputs_manifest.md` | Yes | 登记 `SRC-009/SRC-010`、冲突/变更入口 |
| `security_workflow/01_constraints.md` | Yes | 新增 OSR 软件事实源约束与 ROM patch 约束 |
| `security_workflow/02_baseline.md` | Yes | 增加 source precedence 和 eHSM source-conformance baseline |
| `security_workflow/05_code_rules.md` | Yes | 增加代码落地时必须按 OSR 软件/API 对齐的规则 |
| `security_workflow/06_traceability.md` | Yes | 建立 source -> constraint -> design/update trace |
| `security_workflow/04_change_impact.md` | Yes | 记录后续需要更新的方案章节和实现分片 |
| `00_project/decision_log.md` | Yes | 登记用户冻结的 source-of-truth 裁决 |
| `00_project/changelog.md` | Yes | 登记本次仓库级变更 |
| `00_project/open_questions.md` | Yes | 登记 OSR 代码对齐差异和 patch 字段冻结问题 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `inputs_manifest.md` | 新增输入源和 source precedence 说明 | 不删除 `SRC-008` 当前方案源 |
| `01_constraints.md` | 新增 `C-SRC-02`、`C-EHSM-02`、`C-EHSM-03` | 不把未逐项核对的 command ID / OTP offset 全部写成 `[CONFIRMED]` |
| `02_baseline.md` | 只补 source precedence 和 baseline 方向 | 不重写 Root of Trust / Host boundary |
| `05_code_rules.md` | 增加开发规则，要求代码实现按 OSR API/source-conformance | 不替换现有安全模块命名和组件边界 |
| `06_traceability.md` | 增加追踪行和 BLOCKED 候选 | 不关闭既有 open questions |
| `04_change_impact.md` | 明确后续需要更新方案的范围 | 不声称 full design 已完成适配 |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| 只按旧 TRM 抽象描述 eHSM 服务能力 | constraints / baseline / code rules | TRM + OSR 软件代码/API 双重 source-conformance，服务/命令/字段以 OSR 代码为实现事实源 | 新增代码包已经成为后续可落地服务边界 |
| ROM patch 未登记为安全输入 | inputs / constraints | 登记 eHSM4.0 patch 机制及其 OTP/BOOT/CPU-IROM 边界 | Patch 会影响 ROM 实际执行指令，需要纳入安全方案约束 |

## 8. 不允许 Codex 自行改变的内容

- 不改变 Root of Trust = eHSM。
- 不改变 Host 不可信、不得直接访问 eHSM / OTP / secure resource 的边界。
- 不改变 SEC1 / SEC2 正式路径 sign + encrypt 和 eHSM verify+decrypt output path 方向。
- 不关闭 manifest ABI、exact key ID、exact OTP/control bit、SEC1 exact command path、tool golden vector 等 TBD。
- 不把 OSR 代码里观察到的每个字段、offset、key ID 自动升格为 NGU 系统级冻结项。

## 9. 验收标准

- [x] 新输入源已登记到 `inputs_manifest.md`。
- [x] 新 source-of-truth 和 patch 机制已进入 `01_constraints.md`。
- [x] 已记录是否需要更新方案的影响分析。
- [x] 未新增无依据 `[CONFIRMED]` 字段级 ABI。
- [x] 未关闭问题已登记到 `00_project/open_questions.md`。
- [x] 新设计裁决已登记到 `00_project/decision_log.md`。
- [x] 仓库变更已登记到 `00_project/changelog.md`。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-06-27 |
| 修改文件 | `security_inputs/inputs_manifest.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `security_workflow/04_change_impact.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md` |
| 未修改但检查过的文件 | `05_traceability/file_sync_checklist.md`; `05_traceability/design_impact_matrix.md`; `security_inputs/sw/SW_changelist.md`; OSR BL/FW/Host API 关键源码；`eHSM4.0 Patch方案.pdf` |
| 未完成项 | `10_full_design.md`、`04_impl_design/*.md` 与 OSR 代码逐项适配尚未执行，需要后续 CR |
| 执行说明 | 本 CR 先建立输入事实源和约束，避免方案继续脱离 OSR 代码；字段级落地保留为后续专项 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `owner-review-pending` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `No` |
