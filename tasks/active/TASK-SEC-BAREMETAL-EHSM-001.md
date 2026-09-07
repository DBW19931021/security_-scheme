# TASK-SEC-BAREMETAL-EHSM-001：baremetal eHSM测试Case开发

## TASK BRIEF

- 类型：顶层任务一。
- 状态：planning_active。
- Owner：GSP。
- 目标：把SRC-0018 OSR Host Demo完整移植为baremetal可执行case，验证eHSM全部功能和接口。
- 计划/清单仓库：`security_-scheme`。
- 代码仓库：`../baremetal`。
- 非目标：不定义或实现BootROM/FMC/GSP产品安全链；不以Vendor Demo总体success作为发布结论。
- 当前授权：只允许更新`security_-scheme`计划、case list和调查Evidence；尚未授权修改`baremetal`。

## 执行计划

| 阶段 | 工作 | 输出 | 状态 |
|---|---|---|---|
| A0 Case清单 | 展开`ehsm_demo_test()`全部Root/BL/FW/可选入口及内部command组合 | 最终case list、fixture/破坏性/宏/版本矩阵 | active；流片前硬件验证聚焦版Mailbox/SoC设计包完成；Vendor Demo内部组合仍待展开 |
| A1 移植设计 | 明确baremetal case注册、port、runner、timeout、日志、结果和目录 | 实施任务/OpenSpec、目标文件清单 | pending |
| A2 代码移植 | 复用Host command组包/解析，完成baremetal适配 | 可构建case和runner | pending；需批准 |
| A3 前置验证 | 编译、静态检查、可前移dry-run | 构建/日志Evidence | pending |
| A4 EMU执行 | BL/FW/破坏性profile分批执行真实eHSM case | 逐case PASS/FAIL/INCONCLUSIVE Evidence | pending；等待EMU |

## 复用规则

- 无特殊差异时复用Vendor Host command构造、response解析和测试向量。
- baremetal负责顶层调用、case选择、timeout、结果输出和资源恢复。
- OTP/LCS/Debug/Key/升级等case必须标记破坏性并使用专用fixture/实例。
- 发现Vendor Demo、文档、eHSM实际返回或NGU800P环境冲突时，停止受影响case并登记冲突。

## 当前下一步

完成INV-SEC-003二级源码盘点，然后形成一个单独的baremetal实施任务。该实施任务获批准前不修改代码仓。

## 2026-08-03流片前硬件验证聚焦版设计交付

- [v0.3.5安全测试用例工作簿](../../tests/cases/NGU800P-security-test-cases-v0.3.5.xlsx)：52条Mailbox BASIC + 14条真实到达eHSM的负向用例 + 8条SoC软件用例，共74条Codex可移植软件用例；8项硬件条目只保留与软件case关联的内部补充验证；说明性术语已中文化并新增术语说明；`NOT_EXECUTED`。
- [NGU800P安全测试用例设计与Codex移植指导](../../docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md)：供后续Codex检查`../baremetal`后规划移植，不预设代码结构。
- [软件功能case与硬件补充验证项](../../docs/06-verification/NGU800P软件不便覆盖的硬件安全验证项.md)：逐项映射软件构造、软件判据与RTL/DV剩余内部验证。
- [测试用例设计工作流记录](../../docs/06-verification/NGU800P-security-test-case-workflow-record.md)：记录来源、边界、形成过程、待补输入和完成判定。
- v0.3/v0.3.1/v0.3.2/v0.3.3/v0.3.4作为历史版本保留；v0.3.5保持v0.3.4的用例范围，并将说明性英文术语统一改为中文解释。映射未冻结时保留case并输出INCONCLUSIVE。
- 所有用例均禁止修改或定制eHSM BL/FW，禁止新增测试命令、测试钩子或特殊返回；Firewall配置权限测试不依赖鉴权/LCS。
- 边界：上述设计包不等同于Vendor Demo二级组合`L2_complete`；v0.2逐项历史对账和INV-SEC-003仍需完成，且不授权修改`baremetal`。

## 验收

- [ ] Vendor交付的全部功能/接口均有可追溯case。
- [ ] 每个case有输入、Expected、raw result、前置/恢复、可执行入口和Evidence。
- [ ] baremetal构建和runner通过。
- [ ] EMU结果绑定eHSM/RTL/软件版本。
- [ ] 未执行任何Git提交。
