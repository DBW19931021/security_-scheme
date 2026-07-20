# CR-0019 Vendor 代码与工具链变更控制

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0019-vendor-code-toolchain-change-control` |
| Title | Vendor eHSM BootROM/Firmware 与 Wing 工具链变更控制强约束 |
| Status | `accepted-for-application / applied-by-codex / owner-review-pending` |
| Owner | 用户 / 项目组 |
| Reviewer | 待项目组复核 |
| Created Date | 2026-07-02 |
| Source / Context Pack | 用户 2026-07-02 明确指示 |
| Related Decision ID | `DEC-0024` |

## 2. 背景

用户明确指出：eHSM BootROM、eHSM Firmware 和编译工具链均由 vendor 提供。后续对 `ehsm_bootrom`、`ehsm_firmware` 代码，以及 Wing 工具链安装、权限、软链接、运行时库和环境变量的任何改动，都必须谨慎处理。

本 CR 将该要求上升为后续开发强约束，并同步到 FSP OpenSpec、security constraints、code rules 和 traceability。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | 未在本 CR 中冻结 |
| inputs_manifest 摘要 | `SRC-009 OSR eHSM 软件代码包 4019` 已作为 eHSM 已提供安全服务的实现事实源 |
| constraints 摘要 | 已存在 `C-SRC-02` 约束 OSR 软件代码包事实源 |
| baseline 摘要 | 本 CR 不改变 Root of Trust、BootROM 边界、SEC1/SEC2 sign+encrypt 或 Host trust boundary |
| 相关详设章节 | eHSM source-conformance、BootROM、FW package、mailbox、manufacturing |
| 相关实现级文档 | `ehsm_source_conformance_matrix.md`、`mailbox_if.md`、工具链/构建脚本说明 |
| 已知冲突 | 无；本 CR 是开发流程和变更控制约束，不引入新 ABI |
| 待关闭 TBD | 工具链动态库兼容策略若要产品化，应由 vendor/项目组确认 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| Vendor 代码谨慎变更 | 后续修改 `ehsm_bootrom/**`、`ehsm_firmware/**` 前，必须先提醒用户该操作触及 vendor 代码，并说明原因、改动点、影响、回退和验证 | Vendor 代码是实现事实源，静默修改会破坏后续对齐和问题归因 | `[CONFIRMED]` |
| 工具链谨慎变更 | 后续修改 `/opt/wing_tool/**`、工具链软链接、动态库、权限、PATH/LD_LIBRARY_PATH 或工具链相关脚本前，必须同样执行提醒、说明和记录 | 工具链属于 vendor 构建环境，运行时库 workaround 可能影响可复现构建 | `[CONFIRMED]` |
| 记录要求 | 每次受控变更必须记录背景原因、改动点、影响分析、验证结果和遗留风险 | 让后续评审能区分 vendor 原始行为、项目 wrapper 和本地环境 workaround | `[CONFIRMED]` |
| 开发约束集中目录 | FSP 后续 OpenSpec、Codex/Superpowers、仓库专用 skill/workflow 和其他开发规则统一放入 `development_constraints/` | 避免仓库根目录规则入口分散，便于后续评审和维护 | `[CONFIRMED]` |
| 默认策略 | 优先通过外层脚本、环境变量、wrapper 或文档解决；必须改 vendor 内容时采用最小改动 | 降低 vendor 代码分叉和工具链漂移风险 | `[CONFIRMED]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `fsp/development_constraints/openspec/config.yaml` | Yes | 写入 FSP OpenSpec 默认上下文和 artifact 规则 |
| `fsp/development_constraints/openspec/development_principles.md` | Yes | 新增 FSP 开发强约束说明 |
| `fsp/development_constraints/README.md` | Yes | 说明 FSP 开发约束目录结构和放置规则 |
| `fsp/development_constraints/.codex/skills/*` | Yes | 将仓库相关 Codex/OpenSpec skill 归入开发约束目录 |
| `security_inputs/inputs_manifest.md` | Yes | 登记本次用户输入变更 |
| `security_workflow/01_constraints.md` | Yes | 新增 vendor 代码与工具链变更控制约束 |
| `security_workflow/05_code_rules.md` | Yes | 新增开发阶段 MUST 规则 |
| `security_workflow/06_traceability.md` | Yes | 新增追踪矩阵行 |
| `security_workflow/04_change_impact.md` | Yes | 记录本 CR 影响和一致性检查 |
| `00_project/decision_log.md` | Yes | 登记 `DEC-0024` |
| `00_project/changelog.md` | Yes | 登记本次变更 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `fsp/development_constraints/openspec/*` | 明确 vendor 代码和工具链变更前提醒、原因、影响、回退、验证和记录要求 | 不改变已有构建命令语义 |
| `fsp/development_constraints/README.md` | 明确后续开发约束、OpenSpec、Codex/Superpowers 等规则均归口在 `development_constraints/` | 不引入新的构建动作 |
| `fsp/development_constraints/.codex/skills/*` | 移入开发约束目录，保持原 skill 内容语义不变 | 不修改 skill 行为 |
| `security_workflow/01_constraints.md` | 新增 `[CONFIRMED]` 强约束，不引入安全架构新决策 | 不改变 RoT、Host 边界、BootROM crypto 边界 |
| `security_workflow/05_code_rules.md` | 新增 `R-DOC-008` 或等价规则，要求后续开发遵守 | 不改写既有 `R-FW-*` 规则语义 |
| `security_workflow/06_traceability.md` | 新增 trace 行，连接用户输入、CR、约束、OpenSpec 和 code rules | 不关闭既有 TBD |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| 无明确 vendor 变更控制规则 | FSP OpenSpec / code rules | Vendor 代码和工具链变更必须先提醒、说明、记录、验证 | 防止静默修改 vendor 交付物 |

## 8. 不允许 Codex 自行改变的内容

- 不得借本 CR 修改 `ehsm_bootrom/**` 或 `ehsm_firmware/**` 源码。
- 不得借本 CR 继续修改 `/opt/wing_tool/**`、`~/.bashrc` 或工具链动态库。
- 不得把当前本地工具链 workaround 描述为 vendor 官方方案。
- 不得新增未经来源支撑的 eHSM ABI、mailbox command、OTP layout、key slot 或工具 CLI `[CONFIRMED]`。

## 9. 验收标准

- [x] FSP OpenSpec 已记录强约束。
- [x] 安全方案 constraints 已新增 vendor 变更控制约束。
- [x] code rules 已新增开发阶段 MUST 规则。
- [x] traceability 已新增对应 trace 行。
- [x] 本 CR 不修改 vendor 代码或工具链状态。
- [x] FSP 开发约束目录已集中到 `development_constraints/`。
- [x] 新规则不关闭任何字段级 TBD。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-07-02 |
| 修改文件 | `fsp/development_constraints/README.md`; `fsp/development_constraints/openspec/config.yaml`; `fsp/development_constraints/openspec/development_principles.md`; `fsp/development_constraints/.codex/skills/*`; `security_inputs/inputs_manifest.md`; `security_workflow/01_constraints.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `security_workflow/04_change_impact.md`; `00_project/decision_log.md`; `00_project/changelog.md` |
| 未修改但检查过的文件 | `change_requests/CR_template.md`; existing OpenSpec examples |
| 未完成项 | 后续若实际修改 vendor 代码或工具链，需按本 CR 规则另行记录具体变更 |
| 执行说明 | 本 CR 只增加流程强约束，不改变安全架构或 vendor 源码 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 | 待复核 |
| Review 结论 | `owner-review-pending` |
| 阻塞问题 | 无 |
| 非阻塞建议 | 可在后续 CI/review checklist 中自动检查 vendor 路径变更是否附带说明 |
| 是否允许关闭 CR | `No` |
