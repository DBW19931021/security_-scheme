# CR-0002：输入源引用显示名可读性增强

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0002-readable-source-references` |
| Title | 输入源引用显示名可读性增强 |
| Status | `applied` |
| Owner | Codex |
| Reviewer | 项目组 / GPT |
| Created Date | 2026-04-30 |
| Source / Context Pack | N/A，本 CR 基于当前仓库引用扫描生成 |
| Related Decision ID | `DEC-0005` |

## 2. 背景

当前各设计文档中大量使用 `SRC-005` 这类编号引用输入资料。编号有利于追溯，但阅读正文时无法直接看出对应的是哪份文档，需要反查 `security_inputs/inputs_manifest.md`，影响项目组评审效率。

本 CR 只增强引用显示方式，不改变任何安全设计结论、约束强度、baseline、接口字段或实现要求。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `df815e2` |
| inputs_manifest 摘要 | 已登记 `SRC-001` 至 `SRC-005`，其中 `SRC-005` 对应 `security_inputs/board/管理子系统.pdf` |
| constraints 摘要 | 当前约束中板级安全约束引用 `SRC-005` |
| baseline 摘要 | 当前 baseline 中管理子系统总体架构引用 `SRC-005` |
| 相关详设章节 | `04_lifecycle_debug.md`、`05_board_security.md`、`06_interface.md`、`10_full_design.md` 等 |
| 相关实现级文档 | 不涉及实现级接口字段修改 |
| 已知冲突 | 无 |
| 待关闭问题 | 无新增待决项 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 输入源引用显示方式 | 正文保留 `SRC-xxx` 编号，同时追加文档名称，例如 `SRC-005 管理子系统方案` | 保留机器/人工追溯能力，同时提升正文可读性 | `[CONFIRMED]` |
| 文件路径呈现方式 | 在 `inputs_manifest.md` 中集中维护编号、显示名与路径映射；正文优先使用短显示名 | 避免在正文中反复出现长路径，降低阅读噪声 | `[CONFIRMED]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_inputs/inputs_manifest.md` | Yes | 增加引用显示名约定 |
| `security_workflow/01_constraints.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/02_baseline.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | Yes | 将裸 `SRC-005` / `SRC-003` 改为可读显示名 |
| `security_workflow/03_detailed_design/04_lifecycle_debug.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/03_detailed_design/05_board_security.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/03_detailed_design/06_interface.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 将裸 `SRC-005` / `SRC-003` 改为可读显示名 |
| `security_workflow/04_change_impact.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/05_code_rules.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `security_workflow/06_traceability.md` | Yes | 将裸 `SRC-005` 改为可读显示名 |
| `00_project/decision_log.md` | Yes | 登记引用显示名裁决，并同步历史表述中的裸 `SRC-005` |
| `00_project/changelog.md` | Yes | 登记本 CR 仓库级变更 |
| `05_traceability/design_impact_matrix.md` | Yes | 登记本 CR 影响矩阵 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| 全部受影响设计文档 | 仅将裸编号显示增强为 `SRC-xxx 文档名称` | 不改变设计结论、风险等级、约束状态、接口字段、流程、标题结构 |
| `security_inputs/inputs_manifest.md` | 新增显示名约定表 | 不改变 Source Inventory 已有输入源策略 |
| 项目记录文件 | 追加本 CR 记录 | 不改写 CR-0001 设计裁决语义 |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| `SRC-005` | 设计正文和追踪文件 | `SRC-005 管理子系统方案` | 让读者直接识别引用文档 |
| `SRC-003` | 总详设中的 SEC1 来源引用 | `SRC-003 启动方案` | 让读者直接识别引用文档 |

## 8. 不允许 Codex 自行改变的内容

Codex 执行本 CR 时不得自行改变以下内容：

- 不修改 Root of Trust、First Verifier、Host trust boundary 等安全架构口径。
- 不修改 SEC1 加密、签名、解密路径等 CR-0001 已确认设计结论。
- 不新增或关闭任何安全待决项。
- 不调整章节内容、章节顺序、接口字段、表格语义或实现设计。

## 9. 验收标准

- [x] `inputs_manifest.md` 提供 `SRC-001` 至 `SRC-005` 的推荐显示名与路径映射。
- [x] 设计正文中的 `SRC-005` 已改为可读显示名。
- [x] 总详设中残留的 `SRC-003` 已改为可读显示名。
- [x] 未改变任何安全方案设计语义。
- [x] 本 CR 已登记到 `decision_log.md`、`changelog.md` 和 `design_impact_matrix.md`。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-04-30 |
| 修改文件 | `security_inputs/inputs_manifest.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/03_detailed_design_master.md`; `security_workflow/03_detailed_design/04_lifecycle_debug.md`; `security_workflow/03_detailed_design/05_board_security.md`; `security_workflow/03_detailed_design/06_interface.md`; `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/04_change_impact.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `05_traceability/design_impact_matrix.md`; `change_requests/CR-0002-readable-source-references.md` |
| 未修改但检查过的文件 | `security_inputs/inputs_manifest.template.md`; `change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md` |
| 未完成项 | 无 |
| 执行说明 | 仅增强输入源引用显示名，保留 `SRC-xxx` 追踪编号；未改变安全方案正文语义、接口字段或设计裁决 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PENDING` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `No` |
