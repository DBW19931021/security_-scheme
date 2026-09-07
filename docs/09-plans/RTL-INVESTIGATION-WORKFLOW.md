---
title: "eHSM Vendor RTL调查与证据规则"
status: active
evidence_state: DOCUMENTED
applicability:
  - OSR eHSM 4019
  - NGU800P D0 eHSM集成调查
source_ids:
  - SRC-0035
owners:
  - GSP
  - eHSM/RTL Owner（待绑定）
last_reviewed: 2026-08-26
supersedes: []
superseded_by: []
---

# 目的

本规则定义如何使用SRC-0035 `source-vault/vendor_rtl`回答eHSM内部硬件细节、确定当前RTL实现基线并形成可复核问题证据。它补充[《正式详设代码调查与证据回填工作流》](CODE-INVESTIGATION-WORKFLOW.md)，不把RTL现状自动提升为SoC产品方案或硅片确认事实。

# 权威范围

| 问题类型 | 第一查询源 | 必须补充的来源/边界 |
|---|---|---|
| eHSM内部模块、端口、信号、寄存器译码、状态机、reset/default、内部总线、KMU/OTP/密码模块连接 | SRC-0035 | 结论为`VENDOR_IMPLEMENTATION`；需要匹配elaboration条件 |
| eHSM接口意图、正式命令/寄存器说明 | 当前有效Vendor手册 | 用SRC-0035核对实现；冲突时升级 |
| eHSM软件命令、Host/BL/FW调用和Demo | 每任务锁定commit的`../fsp` | SRC-0018只作历史对照 |
| NGU800P SoC地址、寄存器、IRQ数值 | SRC-0022 | eHSM RTL不能覆盖SoC生成头 |
| SoC wrapper、PMA/Firewall、clock/reset/power/RAS、eFuse产品位 | 目标SoC RTL/CSR/集成资料及Evidence | SRC-0035只提供eHSM边界端口，不能外推连接 |
| 产品Lifecycle、Key、Boot、软件ABI和安全策略 | SRC-0017、SRC-0016、accepted ADR/OpenSpec/Requirement | RTL只用于实现差距和可行性证据 |
| 仿真、EMU、FPGA、硅片实际行为 | 匹配版本的执行Evidence | 源码存在不等于`CONFIRMED` |

# 强制查询顺序

1. 从[《SRC-0035 Source Card》](../../sources/source-cards/SRC-0035.md)确认状态、版本、树哈希、限制和是否已被替代。
2. 在任务或报告中锁定`Source ID + tree hash + 调查日期`；哈希不一致时停止，不得沿用旧结论。
3. 先找目标顶层和实例路径。默认顶层候选为`rtl/osr_ehsm_top.v:osr_ehsm_top`，但必须寻找正式filelist、外部wrapper、命令行define、参数、generate和库绑定；缺失时把结论限定为“当前源码候选配置”。
4. 从消费者/可观察端点沿组合与时序逻辑锥回溯到源，覆盖正常路径、reset、错误、清除、锁定、CDC/RDC和条件分支；不得只读宏名、注释、端口名或单个assign。
5. 记录所有影响结论的`` `ifdef``、localparam/parameter、generate、实例参数、宏、宽度和reset条件。未生效分支、未实例化模块和孤立文件不能当作产品硬件。
6. 对照当前有效Vendor手册和匹配FSP调用，判断是接口意图、RTL实现、软件假设还是差距；涉及SoC连接时再查SRC-0022和目标集成资料。
7. 输出事实状态和适用边界。直接RTL事实使用`VENDOR_IMPLEMENTATION`；缺elaboration或逻辑锥时使用`ASSUMPTION`并登记缺失输入；来源明显冲突时使用`CONFLICTING`并建立报告。

# RTL证据最低要求

每项重要结论必须包含：

1. SRC-0035、树哈希、OSR/Wing组件版本和适用芯片/配置。
2. 正斜杠相对路径、module/interface/package、instance path、parameter/macro/signal及紧凑行范围。
3. 顶层到目标逻辑的实例链，以及输入源、寄存器状态、输出消费者和可观察端点。
4. clock/reset domain、同步方式、复位值、更新条件、锁定/清除条件和异常路径。
5. filelist/define/generate/参数/宏库是否已证明；未证明项逐项列为限制。
6. 与Vendor手册、FSP、SRC-0022、方案目标和已有Evidence的对照结果。
7. 结论分类、可继续范围、停止范围、待确认问题和建议验证方法。

推荐引用格式：

```text
SRC-0035 @ tree ab0b...e95b
path: rtl/sys/<file>.v
module/instance: <module>/<instance>
lines: <start>-<end>
condition: <define/parameter/reset/clock>
state: VENDOR_IMPLEMENTATION
conclusion: <仅描述当前快照直接证明的行为>
limitations: <elaboration/SoC binding/simulation等>
```

# 不得采用的推断

- 不得因模块或宏“存在”就认定它进入当前量产配置。
- 不得从eHSM顶层端口名推断NGU800P SoC wrapper、地址、IRQ、Firewall、cache或reset策略。
- 不得从寄存器reset表达式推断eFuse烧写值、生命周期策略或上电后最终可见值，除非完整来源和覆盖链已证明。
- 不得用token/混淆RTL、黑盒或工艺cell名称推断不可观察的算法内部、模拟TRNG质量、PPA或安全强度。
- 不得把lint/编译成功等同于功能正确，把仿真通过等同于硅片确认。
- 不得把RTL中的Key字面量抄入报告、日志、测试Expected、软件、KMS/MES或量产配置；不得据此宣称一机一密已实现。

# 快照与变更控制

- `source-vault/vendor_rtl`是只读输入，禁止直接修改、格式化、自动修复或把生成物写回其中。
- 新Vendor RTL投递必须附delivery/release note和filelist，分配新Source ID，计算新树哈希并用`Supersedes`关联；禁止覆盖SRC-0035。
- 每个设计、缺陷、测试或实施任务都引用具体Source ID/树哈希，不能只写“当前RTL”或裸路径。
- 若调查期间发现树哈希变化、混入派生文件、版本头不一致或缺少必需elaboration输入，停止确定性结论并更新Source Card/入库报告。
- RTL原文及Key相关内容按`RESTRICTED_PENDING_CONFIRMATION`处理；未经授权不得对外分发或上传第三方服务。

# 问题与冲突闭环

可由SRC-0035直接回答且elaboration条件充分的问题，不得重复列为普通open question。只保留以下问题：

- 源码缺失、黑盒/宏不可见、filelist/define/parameter或顶层绑定未提供；
- SRC-0035内部自相矛盾，或与Vendor手册/FSP/测试/Evidence明显不一致；
- 结论依赖NGU800P SoC顶层集成或项目产品策略；
- 需要仿真、形式、CDC/RDC、FPGA、EMU或硅片Evidence才能确认。

冲突报告必须列出双方Source/版本/文件/行、实际差异、影响、可继续和停止范围、验证建议及负责人需要裁决的问题；受影响结论保持`CONFLICTING`，不得用“RTL优先”静默抹掉接口或方案冲突。

# Change history

- 2026-08-26：依据SRC-0035和ADR-0032建立eHSM内部RTL第一查询源、elaboration门禁、证据最低要求、只读快照和敏感Key处理规则。
