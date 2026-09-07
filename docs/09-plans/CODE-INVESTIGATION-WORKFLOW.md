---
title: "正式详设代码调查与证据回填工作流"
status: active
evidence_state: DOCUMENTED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0021
owners:
  - GSP
last_reviewed: 2026-08-26
supersedes: []
superseded_by: []
---

# 目的

本流程把用户提供的 Work × Codex 轻量协作方法适配到现有 `security_-scheme` 布局，用于正式详设阶段的小粒度代码调查、证据回填和人工裁决。它不创建第二套设计、追溯或变更目录。

# 与现有工程的目录映射

| 工件 | 项目位置 | 说明 |
|---|---|---|
| 正式设计 | `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` | 主入口为 `docs/05-software-design/NGU800P安全软件详细设计.md` |
| 调查任务 | `tasks/active/INV-SEC-xxx-<topic>.md` | 一个任务只确认一个核心代码事实或一条紧密调用链 |
| 已完成调查任务 | `tasks/completed/` | 证据完成评审后移动；不要求Git提交 |
| 代码证据 | `evidence/code-investigations/CE-SEC-xxx-<topic>.md` | 记录代码现状、路径、符号、条件编译、差异和限制 |
| eHSM RTL证据规则 | `docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md` | eHSM内部硬件问题必须增加树哈希、实例链、elaboration和逻辑锥证据 |
| 需求/代码追溯 | `requirements/requirement-traceability.yaml` | Requirement到Design/Code/Case/Evidence主追溯 |
| Feature追踪 | `docs/09-plans/SECURITY-FEATURE-REALIZATION-MATRIX.md` | Feature成熟度和工程出口 |
| 测试追踪 | `tests/matrices/security-test-matrix.yaml` | 工作簿、执行入口、波次和Evidence |
| 决策 | `decisions/ADR-*.md` | 重大架构/安全裁决 |
| 冲突 | `sources/conflict-reports/`、`requirements/open-questions.yaml` | 证据冲突和待负责人问题 |
| 变更历史 | `CHANGELOG.md`及各文档Change history | 不另建重复change-log |
| 模板 | `tasks/templates/`、`evidence/templates/` | 调查任务和代码证据模板 |

# 角色和边界

- Codex负责真实代码搜索、关键函数体阅读、跨文件调用链、宏/Kconfig/Makefile/链接脚本、接口/结构体/错误码/状态机和平台差异调查，并输出代码证据。
- 正式详设负责人负责把代码事实与SRC-0017/SRC-0016、目标方案和安全原则对照，更新正式设计和追溯。
- 用户/负责人裁决架构、安全、产品范围、代码保留/替换、冲突采用方、风险接受和编码准入。
- 代码调查阶段默认只读：不得修改业务代码、构建文件或正式设计结论，不执行任何Git命令。调查报告写入 `security_-scheme`。
- eHSM内部硬件问题以SRC-0035为第一查询源；`source-vault/vendor_rtl`只读。RTL调查不得修改/格式化快照，且不能把RTL现状自动写成SoC产品目标。

# 信息分类

调查和设计必须明确区分：

- `CODE_FACT`：已由确切文件、符号和调用链确认的代码现状。
- `INFERENCE`：有代码依据但仍缺构建、运行、RTL或版本证据的推断。
- `TARGET_DESIGN`：批准或提议的目标设计，不代表已经实现。
- `GAP`：代码现状与目标设计不一致或尚未实现。
- `UNKNOWN`：当前资料、代码或环境不足以确认。
- `CONFLICTING`：两份有效证据明显冲突，等待负责人裁决。

以上分类用于代码调查；正式硬件/安全文档仍使用项目规定的事实状态枚举。

# 标准闭环

```text
正式详设发现代码依赖问题
  → 创建小粒度INV-SEC调查任务
  → Codex只读调查真实代码
  → 输出CE-SEC代码证据
  → 复核关键文件/符号和证据充分性
  → 区分CODE_FACT / TARGET_DESIGN / GAP / UNKNOWN / CONFLICTING
  → 更新详设、Requirement、Feature和测试追溯
  → 需要时创建ADR/冲突报告或下一调查任务
  → Feature满足门禁后，由负责人批准独立编码任务
```

若核心问题是eHSM内部硬件，则在“只读调查真实代码”步骤内同时执行[《eHSM Vendor RTL调查与证据规则》](RTL-INVESTIGATION-WORKFLOW.md)：先锁定SRC-0035树哈希和实际elaboration，再沿实例/逻辑锥形成`VENDOR_IMPLEMENTATION`证据；涉及SoC连接或产品策略时切回相应Source，不能从eHSM RTL外推。

# 调查任务粒度

一个任务应满足：

- 只回答一个核心事实或一条紧密相关调用链。
- 可以在一次只读调查中给出路径、符号、构建条件和结论。
- 明确不调查什么，避免扩展为“分析整个安全方案”。
- 输出唯一的 `CE-SEC` 证据报告。

编号规则：调查任务和证据共享编号，例如：

```text
tasks/active/INV-SEC-001-bootrom-ehsm-startup.md
evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md
```

# 代码证据最低要求

每个重要结论必须尽量包含：

1. 仓库、清单中的只读branch/commit、调查日期和对应任务。
   - eHSM RTL调查改为记录SRC-0035、树哈希、顶层/filelist/defines/parameters/elaboration和调查日期；没有独立Git时不得虚构commit。
2. 文件路径、函数/结构体/宏/寄存器/符号及紧凑行范围。
3. 完整入口、下游调用、返回值、错误路径和数据所有权。
4. Kconfig、Makefile、编译宏、链接脚本和平台/版本差异。
5. 哪部分是已确认事实，哪部分是推断、缺口或无法确认。
6. 与目标设计的差异、影响范围和建议的下一调查。

不允许仅凭文件名、函数名、注释或历史Review确认实现；必须阅读关键函数体。历史Review只作为调查线索。

# 设计回填和评审

证据报告形成后：

1. 抽查关键源文件和调用链。
2. 对照SRC-0017、SRC-0016、Vendor文档和正式详设。
3. 将代码现状写入 `Vendor implementation observations` 或明确的代码现状小节，不写成目标设计。
4. 将目标行为写入 `Proposed design` 或批准后的正式契约，并标明依据。
5. GAP进入Feature/Requirement/任务追踪；冲突进入conflict report并提交负责人裁决。
6. 更新目标代码文件/符号、测试工作簿case和`baremetal`可执行入口。
7. 只有通过正式详设中的按Feature编码门禁，才能进入实现。

# 当前首批调查

1. `INV-SEC-001`：BootROM与eHSM启动交互、ready/self-test/timeout和FMC release；`CE-SEC-001`已形成并回填设计，状态为`evidence_ready / awaiting_design_review`。
2. `INV-SEC-002`：FMC/GSP镜像验签、解密、加载、measurement和跳转调用链；`CE-SEC-002`已形成并回填设计，调查时新增OPEN-CONFLICT-003，后于2026-07-22按ADR-0004关闭，状态为`evidence_ready / awaiting_design_review`。
3. `INV-SEC-003`：Vendor `ehsm_demo_test()`完整case与`bl_demo`产品复用映射；`CE-SEC-003`已完成Root/BL/FW一级入口证据，二级command/case和A/B/C/D复用表继续调查。
4. `INV-SEC-004～007`：安全RAM、context生命周期、Vendor Mailbox事实和NGU800P port绑定均已形成CE-SEC-004～007；相关布局、Vendor direct和阶段模式已进入ADR-0006～0010。
5. `INV-SEC-008`：Vendor poll的cache/timeout/迟到响应约束；CE-SEC-008已形成并回填ADR-0011和eHSM Host adapter详细合同，状态为`evidence_ready / design_updated`。

后续OTP/LCS、Key Slot/Rotation、Debug gating、SPDM measurement、Multi-Die/Firewall和Certificate Slot等任务，在相应详设章节开始时按同一模板创建，不提前批量生成空任务。

# Change history

- 2026-08-26：接入SRC-0035 RTL第一查询源；eHSM内部硬件调查增加树哈希、elaboration、实例/逻辑锥和敏感Key边界，详细步骤转交RTL调查规则。
- 2026-07-23：完成INV/CE-SEC-008；冻结首版单在途、active cache/timeout scope、零自动retry和timeout quarantine，建立eHSM Host adapter详细合同。
- 2026-07-22：启动INV-SEC-003并形成一级入口证据；支撑DD-02 baremetal全功能case与gsp产品复用边界，未修改代码仓。
- 2026-07-22：补记INV-SEC-002后续裁决；GSP采用NoC/system canonical地址，当前不采用Handoff。
- 2026-07-21：依据SRC-0021适配现有工程目录，建立INV-SEC/CE-SEC编号、只读调查、代码证据和详设回填闭环。
- 2026-07-21：完成首个闭环INV-SEC-001/CE-SEC-001，回填安全启动、BootROM和eHSM Mailbox首轮设计；等待设计评审后归档任务。
- 2026-07-21：完成INV-SEC-002/CE-SEC-002，回填FMC、GSP、防回滚和完整启动链；发现并登记GSP地址域OPEN-CONFLICT-003。
