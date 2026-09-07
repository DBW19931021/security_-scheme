---
title: "安全测试策略与用例版本流程"
status: draft
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0020
owners: []
last_reviewed: 2026-07-22
supersedes: []
superseded_by: []
---

# Purpose

定义流片前安全测试用例如何从方案、Vendor 实现和风险派生，如何版本化、标注冲突、执行并进入发布 Evidence。

# Scope

- eHSM 硬件/BL/FW/Host、SoC BootROM、FMC、GSP、Runtime 软件栈。
- OTP/eFuse、密钥、生命周期、Debug、Secure Boot、升级/OOB、远程证明、SPDM、多 Die/Firewall、错误与故障注入。
- 本文件管理策略，不把尚未执行的测试项描述为已验证事实。

# Confirmed facts

- 安全测试的方案追踪、原始输入、当前及后续测试工作簿、测试策略/计划、case规划和Evidence索引由`security_-scheme`管理；eHSM全能力可执行case、EMU runner和工具由独立baremetal软件栈在`../baremetal/components/ngu_security/`实现并运行于安全核，不属于GSP固件。Codex不执行任何Git提交。
- SRC-0017 是系统/架构上位方案，SRC-0016 是软件工程落地方案；测试预期必须遵守该权威链。
- Vendor 仅指 eHSM/Core；Vendor 测试不能单独证明 NGU800P SoC 通过。

# Documented facts

- SRC-0020 是 2026-07-21 接收的统一模板初版，原始 76 行、75 个编号和 1 个 ROM Patch 占位。
- v0.2 保留模板列，形成 100 个编号用例，并在工作簿内增加版本说明和覆盖审视。
- OPEN-CONFLICT-001/002/003均已关闭。v0.3按ADR-0003/0004高亮修改用例093、107和GSP地址域相关Expected，v0.2保持不变。

# Vendor implementation observations

- SRC-0018 提供 BL/FW Python tests、crypto、OTP、lifecycle、debug、upgrade、selftest 和 ROM Patch 回归输入。
- Vendor 回归结果必须绑定 SRC-0018 的确切版本、配置和原始日志；不自动提升为 SoC `CONFIRMED`。

# Assumptions

- v0.2 中新增的执行载体、Owner 和自动化状态仍需各团队确认。
- TRNG 阈值、故障注入点、清零范围、SPDM profile 等参数在未形成批准要求前保持待确认。

# Proposed design

## 用例资料分层

1. 原始输入：`source-vault/internal-specs/test-inputs/SRC-xxxx/`，不可覆盖。
2. 版本化工作簿和case规划：`security_-scheme/tests/cases/`保存v0.2及后续工作版本，由本工程测试治理流程维护；每次更新生成新文件且不覆盖旧版本。
3. 可执行测试：`../baremetal/components/ngu_security/tests/`，由独立baremetal软件栈承载测试代码、runner和必要测试数据并运行于安全核；不经过GSP固件，也不在该仓库维护项目测试规划或工作簿。
4. 索引：`security_-scheme/tests/matrices/security-test-matrix.yaml` 记录工作簿当前版本、可执行入口和跨仓库追溯。
5. 实际结果：`security_-scheme/evidence/`，按执行批次/芯片/版本保存；若原始大日志存于外部系统，则登记不可变引用和哈希。

## 版本和高亮

- 黄色：修改已有内容。
- 绿色：新增测试行。
- 橙色：待确认或因冲突阻塞。
- 新版本必须带“版本说明”，记录 Source ID、原版本、修改数量、冲突和执行状态。

## 预期派生顺序

`SRC-0017 → 最新有效SRC-0016 → accepted ADR/amendment/requirements/OpenSpec → 最新有效版本化工作簿 → 可执行测试预期`。Vendor文档/软件代码/RTL/tests用于验证能力和发现差距；eHSM内部硬件Expected必须绑定SRC-0035树哈希及实际elaboration。若改变项目方案，先更新软件方案或受控amendment。

现有`gsp-pmp-rmp-omp`/`baremetal`中的test、stub、demo、synthetic flow及历史Expected只表示当前代码事实和差距，不得反向进入上述派生链。现有runner/build机制可以复用，但可执行case及Expected必须来自最新有效工作簿；EMU/产品PASS不得依赖stub、simulated success、test key/cert/provider、未批准hardcode或silent fallback。

## Vendor eHSM Demo覆盖规则

- 经负责人确认，SRC-0018 `ehsm_demo_test()`及其BL/FW Demo是eHSM全部能力验证的强制coverage input；一级入口清单见[`EHSM-DEMO-CASE-CATALOG.md`](../../tests/cases/EHSM-DEMO-CASE-CATALOG.md)。
- 每个Demo entry内部的command、算法、模式、key type、输入长度、同步/异步、正负向和边界组合必须继续展开成独立case；被注释、编译宏关闭、重复或具有破坏性的入口也必须登记，不能静默遗漏。
- Vendor command拼装无特殊差异时可复用；baremetal负责顶层case注册、选择、runner、timeout、结果和Evidence。Vendor顶层顺序和`ends with success`打印不构成PASS oracle。
- BL/FW状态和OTP/LCS/Debug/Key/升级前置条件分别建profile；不可逆case不进入默认回归，需独立审批和可恢复/专用实例。
- 该完整覆盖用于证明eHSM功能/接口能力；SoC安全启动、SPDM和发布结论仍必须由软件方案派生的产品链case补充，不能由Vendor Demo回归替代。

## 冲突处理

测试预期与方案、Vendor文档/软件代码/RTL或实测明显不一致时，受影响单元格和用例标橙色/`CONFLICTING`，建立`sources/conflict-reports/`和open question；裁决前只允许探索性验证，不允许形成确定性PASS/FAIL门禁。

## 流片前门禁

- P0 用例必须具备 Owner、RTL/软件版本、载体、输入、输出、通过准则、日志和 Evidence 路径。
- 必须覆盖正向、异常、边界、生命周期、权限、掉电/复位、回滚、故障注入和敏感材料清理。
- 安全启动链分别验证 eHSM、BootROM→FMC、FMC→GSP、GSP→Runtime，以及 OOB 恢复后的再次裁决。
- 所有橙色阻塞项必须完成裁决或形成负责人批准的风险接受记录。

## EMU 前测试就绪流程

当前执行计划见`docs/06-verification/EMU-TEST-READINESS-PLAN.md`，并由`tasks/active/TASK-SEC-EMU-PREP-001.md`跟踪。测试oracle来自`security_-scheme`的批准方案/正式详细设计/Requirement/ADR/OpenSpec并收敛到最新有效工作簿，再由独立baremetal软件栈在`baremetal`落实为运行于安全核的可执行测试。测试准备按以下层级前移：

1. L0 规格/静态：ABI、layout、权限、Source/Requirement 追踪。
2. L1 Host 单元/契约：纯软件解析、显式测试向量、状态机和错误映射；不模拟真实eHSM成功，也不替代L3验收。
3. L2 QEMU/Vendor sim：目标固件编排、标准协议和对应 Vendor 快照回归。
4. L3 EMU：C908/eHSM/RTL 真实接口、时序、OTP/eFuse、Flash、Firewall 和启动链。
5. L4 故障/发布：掉电、reset race、fault injection、长稳、统计和发布门禁。

当前 v0.2 的 100 条用例全部为 P0，这只是初版标签，不作为最终发布排序。后续经负责人批准的新版本应区分发布阻断、Feature 必测、兼容/扩展和冲突探索项；v0.2 保持不变。

EMU 前至少生成两个新工作版本：v0.3 补齐 Feature/Requirement、Owner、层级、波次、载体、自动化和阻塞状态；v0.4 绑定首轮 RTL/eHSM/软件 baseline、脚本和 Evidence 路径。每次均生成新文件并继续使用黄色/绿色/橙色高亮。

# Open questions

- SRC-0020 的 Owner/批准状态、版本命名和保密分类。
- MD5、SHA-512/256、AES-192-XTS、DES/TDES 的产品发布定位。
- SPDM对Die1 Measurement逐条或聚合呈现的profile细节；OPEN-CONFLICT-001原始Vendor回复材料待补录。
- 各用例具体环境、版本、自动化和 Evidence 保存路径。

# Implementation impact

- 本策略不直接修改 `gsp-pmp-rmp-omp`、`baremetal` 或 Vendor 源码。
- 后续需要实现软件测试钩子或故障注入点时，在 `gsp-pmp-rmp-omp` 通过 `security_-scheme` 中批准的任务/OpenSpec 实施；可执行 case/runner/tools 在 `baremetal` 实施，测试计划和工作簿仍在 `security_-scheme`，并分别保护两个多人仓库的既有修改。

# Verification impact

- 当前 v0.2 只完成用例设计审视，未执行测试。
- 未来每次执行应生成不可变结果、环境清单、日志哈希和发布矩阵；工作簿中的状态栏不能替代 Evidence。

# References

- SRC-0016：芯片安全软件方案 v1.2。
- SRC-0017：芯片系统安全方案。
- SRC-0018：OSR eHSM 4019 软件交付快照。
- SRC-0020：NGU800P 安全测试用例初版。
- `tests/cases/README.md`。
- `docs/09-plans/SECURITY-FEATURE-REALIZATION-MATRIX.md`。
- `docs/09-plans/EMU前安全方案落实工作计划.md`。
- `docs/06-verification/EMU-TEST-READINESS-PLAN.md`。
- `tasks/active/TASK-SEC-EMU-PREP-001.md`。
- `sources/intake-reports/INTAKE-2026-07-21-security-test-cases.md`。

# Change history

- 2026-07-22：登记Vendor `ehsm_demo_test()`全部功能/case强制覆盖规则；组包优先复用，baremetal负责顶层执行和稳定结果，Vendor Demo打印不作为SoC发布oracle。
- 2026-07-22：关闭OPEN-CONFLICT-002/003；Die1独立实例和GSP NoC/system canonical地址进入后续工作簿派生规则，当前不为Handoff新增测试范围。
- 2026-07-22：关闭自检位图冲突并规划用例093的v0.3高亮更新；明确最新工作簿派生链、现有test/stub非最终oracle和EMU/产品不得打桩形成PASS。
- 2026-07-21：接收 SRC-0020，建立用例版本/高亮/冲突/发布 Evidence 流程并生成 v0.2。
- 2026-07-21：增加 T-4～T0 EMU 测试就绪、L0～L4 分层、Wave 0～4 执行波次和 v0.3/v0.4 版本计划；本次未修改 v0.2、未执行测试。
- 2026-07-21：按负责人补充信息，将后续测试case、EMU runner和工具的主维护位置调整到`../baremetal/components/ngu_security/`；2026-07-28进一步明确其运行Owner是独立baremetal软件栈而非GSP固件，`security_-scheme`保留治理和Evidence索引。
- 2026-07-21：按负责人进一步确认，将测试策略、计划、版本化工作簿和 case 规划统一放在 `security_-scheme`；`baremetal` 仅保存可执行测试、EMU runner 和工具。
