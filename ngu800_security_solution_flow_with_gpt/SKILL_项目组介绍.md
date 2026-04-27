# NGU800 Security Skill 项目组介绍

## 背景与原始需求

这个 Skill 的出发点，来自最开始的真实需求：

> “我现在需要做一个ngu800的安全方案，详细的设计方案，完全可以照着直接落地的。我手头的资料有很多，包括要做的功能点，soc的架构，安全ip手册，项目组已经确定下来的总体方案或者安全可能要依赖的系统方案等等，这些都是安全方案详设需要考虑的。 当然我可以按照这些资料来梳理，但是有些系统方案项目上可能还会变， 安全子系统的ip的资料可能随时会补充，所以我希望在完成详设的同时，可以把这个过程提炼成一个skill，下次有新的资料补充或者变更，我可以很快的修改方案。”

围绕这个原始需求，我开发这个 Skill 的最初目的主要有三点：

- 输入会持续变化，比如 SoC 架构调整、安全 IP 手册补充、项目组总体方案变化、接口定义更新
- 输出不能只是泛泛而谈，而是要形成可直接落地的详细设计方案，包括模块职责、启动流程、密钥体系、证书链、测量与证明、升级与回滚、调试管控、生命周期、异常处理、制造 / 产测流程、Host 交互等
- 以后要能快速增量修改，所以不能每次都从头重新问大模型，而要把“怎么收资料、怎么分析、怎么写、怎么识别变更影响”固化成 Skill

从这个角度看，这个 Skill 不是单纯为了“提高写文档效率”，而是为了把安全方案设计过程沉淀成一套能持续复用、能跟着输入变化一起演进的方法和流水线。

## 1. 这是什么

`ngu800-security-design` 是我为 NGU800 / NGU800P 安全方案设计整理的一套流程型 Skill。

它不是一个“帮忙写文档”的普通提示词，而是一套面向工程落地的安全设计流水线，目标是把分散输入材料逐步收敛为：

- 可评审的安全方案
- 可落地的实现级设计
- 可继续约束代码开发的规则与追踪关系

## 2. 这个 Skill 主要解决什么问题

项目里做安全方案时，常见痛点不是“不会写文档”，而是下面这些问题：

- 输入资料很多，来源不一致，容易互相冲突
- 方案结论容易直接从原始材料跳出来，中间缺少约束和基线沉淀
- 章节级详设能过评审，但不够支撑 RTL / FW / Driver 实现
- 新资料加入后，已有设计很难做增量更新和影响分析
- 后续代码开发时，缺少从“输入来源”到“设计结论”再到“实现与测试”的追踪链路

这个 Skill 就是为了解决这些问题而设计的。

## 3. 它的核心思路

核心原则只有一句话：

**不能从原始输入直接生成完整安全方案，必须先过约束和基线。**

也就是说，Skill 不鼓励“看几份 PDF 后直接写一版大文档”，而是要求按固定流水线逐步收敛：

```text
inputs_manifest
→ constraints
→ baseline
→ chapter-level detailed design
→ implementation-level design
→ code rules
→ traceability
→ change impact
```

这样做的价值是：

- 每个结论都有来源
- 每个章节都有前置约束和 baseline 依据
- 实现层不会和方案层脱节
- 新输入进来后可以做增量更新，而不是整包推倒重写

## 4. 这个 Skill 的定位

它更像一个“安全方案工程化方法包”，而不是单次生成器。

它做的事情：

- 统一输入读取顺序和优先级
- 统一方案输出目录结构
- 统一每个阶段该产出什么
- 统一冲突处理、假设标注、开放问题和冻结项表达方式
- 统一后续实现约束和 traceability 关系

它不直接替代的事情：

- 不替代项目组评审
- 不替代架构冻结决策
- 不保证输入本身正确
- 不应该绕过人工判断，直接把所有内容标成 `[CONFIRMED]`

## 5. 流水线怎么走

### Step 0：登记输入

先把输入资料放到：

```text
security_inputs/
```

再维护：

```text
security_inputs/inputs_manifest.md
```

这个文件是整个流程的入口，用来记录：

- 输入来源
- 适用范围
- 优先级
- 是否必须遵循
- 冲突关系
- 变更登记

### Step 1：提取约束

输出文件：

```text
security_workflow/01_constraints.md
```

这一层回答的问题是：

- 现在到底有哪些硬约束、软约束、参考约束
- 这些约束来自哪里
- 当前是 `[CONFIRMED]`、`[ASSUMED]` 还是 `[TBD]`
- 它们会影响哪些章节、哪些实现文件

这一步的意义是把“输入材料”变成“结构化约束”。

### Step 2：形成基线

输出文件：

```text
security_workflow/02_baseline.md
```

这一层回答的问题是：

- Root of Trust 怎么定义
- First Mutable Stage 是谁
- First Cryptographic Verifier 是谁
- BootROM / SEC1 / SEC2 / eHSM / Host / Board 边界怎么划
- 当前采用哪些方案，拒绝哪些方案
- 哪些是冻结敏感项

这一步的意义是把“很多可能的设计方向”收敛成“当前项目采用的统一口径”。

### Step 3：生成章节级详设

输出目录：

```text
security_workflow/03_detailed_design/
```

典型章节包括：

- 启动链路
- 密钥与证书体系
- 远程度量与 attestation
- 生命周期与安全调试
- 板级安全
- 接口设计
- 量产 / RMA
- 故障恢复
- 风险与开放问题

每章都要求包含：

- 本章目标
- 生效约束 ID
- 生效 baseline 决策
- Mermaid 架构图
- Mermaid 时序图
- 对实现层的影响
- 冻结项
- 开放问题

这一步主要服务评审和方案对齐。

### Step 4：生成实现级详设

输出目录：

```text
security_workflow/04_impl_design/
```

这一层是 V2 流程里最关键的增强点。

它重点覆盖：

- eFuse / OTP 规划
- key hierarchy
- firmware header / image format
- mailbox interface
- SPDM report
- lifecycle control
- manufacturing / provisioning

为什么这一层重要：

- 章节级详设适合讲清方案
- 实现级详设才足够支撑 RTL / FW / Driver 落地

如果跳过这一步，文档容易停留在“可读但不可实现”的状态。

### Step 5：生成代码规则

输出文件：

```text
security_workflow/05_code_rules.md
```

这一步把方案结论转成开发约束，例如：

- 哪些模块必须做校验
- 哪些模块不能信任 Host 输入
- 哪些计数器、状态位、生命周期控制必须在安全域完成

这样后续 Codex 或工程人员在写代码时，就不是“重新理解方案”，而是直接按规则执行。

### Step 6：生成追踪矩阵

输出文件：

```text
security_workflow/06_traceability.md
```

这一步要求建立下面这条链路：

```text
Source → Constraint → Baseline → Detailed Design → Impl Design → Code Module → Test
```

它解决的是“结论从哪里来，最终落到了哪里去”的问题。

### Step 7：做最终检查和增量更新

输出文件：

```text
security_workflow/04_change_impact.md
```

这一层负责检查：

- 约束是否前后一致
- baseline 是否前后一致
- 各章节之间是否打架
- impl 设计是否和章节级详设脱节
- 是否存在没有依据的 `[CONFIRMED]`
- Mermaid 是否能渲染
- 文档是否适合导出 PDF
- code rules / traceability 是否需要刷新

如果后续新资料进来，也是在这一层做增量分析，而不是全量重做。

## 6. 如何使用

从项目组协作视角，可以按下面的方式理解：

1. 项目组持续补充输入资料到 `security_inputs/`
2. 先更新 `inputs_manifest.md`
3. 用 Skill 按顺序推进各阶段产出
4. 每个阶段先评审再进入下一阶段
5. 新输入加入后，先做 change impact，再决定更新哪些章节和实现设计

如果一句话概括使用方式，就是：

**先登记输入，再抽约束，再收敛 baseline，再做详设，再下沉到实现，再形成规则和追踪，最后支持增量更新。**

## 7. 为什么这个 Skill 比“直接让 AI 写方案”更合适

直接让 AI 从材料生成一版完整方案，短期看很快，但通常有几个问题：

- 结论来源不清楚
- 冲突材料容易被静默合并
- 假设会被写成既定事实
- 章节之间容易不一致
- 方案和实现之间缺一层桥接
- 后续一旦输入变化，很难局部更新

这个 Skill 的优势恰好在于把这些风险显式化。

## 8. 后期增量迭代示例：新增管理子系统文档

前面的流程更多是“从 0 到 1 建立安全方案”。但项目真正进入中后期后，更常见的工作不是重写整套方案，而是：

- 新增一份系统方案
- 某个接口定义变化
- 某个 IP 手册补充字段
- 项目组冻结了一个新的架构口径
- 发现某个现有流程存在安全缺口

这时 Skill 的价值不是“重新生成所有文档”，而是帮助我们判断影响范围，并做增量更新。

本次新增 `security_inputs/board/管理子系统.pdf` 就是一个典型例子。

### 8.1 用户是怎么提出变更的

这次输入变化不是简单地说“把新 PDF 加进去”，而是带着明确使用口径：

> “我在 board 目录下增加了一个管理子系统文档，给这个输入文档在 manifest 中增加备注，这个文档中的总体架构和流程需要遵循，但是涉及到安全的地方，特别是考虑不够或者明显存在安全漏洞的地方，可以不遵循并且指出。”

这个需求里其实包含三层信息：

- 新增了一个输入源：`security_inputs/board/管理子系统.pdf`
- 该文档的总体架构和流程需要作为系统级输入
- 但安全相关内容不能盲从，尤其是有安全缺口的部分要指出并给出安全侧裁决

这正是 Skill 适合处理的场景：输入资料有价值，但不能直接把资料里的所有内容都当成安全结论。

### 8.2 第一步：先登记输入，而不是直接改详设

按照 Skill 的流程，第一步不是直接打开 `05_board_security.md` 开写，而是先更新：

```text
security_inputs/inputs_manifest.md
```

本次新增：

```text
SRC-005 = security_inputs/board/管理子系统.pdf
CHG-005 = 新增管理子系统文档
```

并在 manifest 中明确使用策略：

- 管理子系统文档里的总体架构、模块职责、系统流程、集成位置和流程编排原则上遵循
- 涉及安全且考虑不足、未覆盖关键攻击面、与既定安全基线冲突、或者明显存在安全漏洞的地方，不直接继承
- 这些问题后续要在约束、详设或风险章节里显式指出差异、风险和替代设计

这一步的价值是：新资料从一开始就被放进了“可追踪、可裁决”的输入体系里，而不是散落在某次对话或某个章节的临时描述中。

### 8.3 第二步：判断是否需要更新约束

登记输入后，需要判断新文档是否改变安全约束。

管理子系统文档中包含了这些会影响安全边界的内容：

- BMC / OAM / 模组 MCU / GPU / 板级 MCU 之间的带外管理通道
- SMBus/I2C、I3C、SPI、PCIe、UART、JTAG 等链路
- JTAG 可接入 GPU JTAGBUS、寄存器空间、DRAM、Flash、安全子系统、CPU 调试单元、板级 MCU
- 管理子系统 DMA、mailbox、中断、互斥访问机制
- 电源、上下电、复位、PowerBrake 等板级控制信号
- 单 Die / 双 Die 场景下的 OOB 入口和地址映射关系

这些不是普通背景材料，而是直接影响安全边界、调试暴露面、DMA 隔离、生命周期和证明状态。

所以本次不是只改某一章详设，而是先在 `01_constraints.md` 增加了新的约束：

- `C-BOARD-01`：管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决
- `C-BOARD-02`：带外管理通道不得成为安全策略绕过路径
- `C-BOARD-03`：JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制
- `C-BOARD-04`：管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须被隔离和审计

这一步体现了 Skill 的核心思想：新资料先变成约束，再进入设计。

### 8.4 第三步：判断是否需要更新 baseline

新增管理子系统文档不仅影响某个接口，还影响系统边界，所以需要检查 `02_baseline.md`。

本次 baseline 增加了 Board / OOB / 管理子系统边界：

- BMC / OOB / 板级 MCU / 管理子系统不进入 Root of Trust
- SMBus/I2C、I3C、PCIe VDM、SPI、UART、JTAG 只能作为受控管理或转发通道
- JTAG、DMA、Flash 更新、电源复位、PowerBrake 等高权限能力必须经 lifecycle、debug auth、firewall、scope 和审计约束
- 管理子系统文档中如果存在未鉴权调试、直接访问安全子系统、直接访问 DRAM/Flash/寄存器空间或绕过 SEC/eHSM 的流程，不作为安全 baseline 采用

这一步的价值是把“这份文档总体要遵循，但安全细节不能盲从”变成了项目级统一口径。

### 8.5 第四步：只更新受影响的详设章节

这次没有全量重跑所有章节，而是只更新真正受影响的章节：

- `03_detailed_design/05_board_security.md`
- `03_detailed_design/04_lifecycle_debug.md`
- `03_detailed_design/06_interface.md`
- `03_detailed_design/10_full_design.md`

其中 `05_board_security.md` 原来只是占位章节，这次被展开成正式章节，内容包括：

- 管理子系统输入摘要
- BMC / OOB / 板级 MCU / 管理子系统的信任边界
- SMBus/I2C、I3C、SPI、PCIe VDM、UART、JTAG 的安全策略
- JTAG 高风险能力的安全裁决
- JTAG scope 建议
- 管理子系统 DMA / mailbox / 中断 / 互斥访问策略
- 电源、上下电和复位安全策略
- 单 Die / 双 Die 与 board/die binding
- 冻结敏感项和开放问题

`04_lifecycle_debug.md` 补充的是 JTAG / CPLD / MUX 与生命周期、debug auth、scope bitmap 的关系。

`06_interface.md` 补充的是 OOB、JTAG、DMA、复位等接口边界。

`10_full_design.md` 则同步总详设，保证总文档不会落后于章节文档。

### 8.6 第五步：判断实现级文档是否需要马上改

这一步很关键。

虽然新文档影响实现，但管理子系统 PDF 还没有给出足够字段级信息，比如：

- JTAG scope bitmap 的最终位定义
- CPLD / MUX 控制寄存器归属
- 管理子系统 DMA 的 UserID 和 firewall region
- OOB proxy 是否承担 provisioning
- PowerBrake、PG/FAULT、reset 是否进入 attestation report

所以本轮没有强行改 `04_impl_design/*.md`，而是在 `04_change_impact.md` 中标记为：

```text
check needed / pending field freeze
```

受影响但暂不重生成的实现级文件包括：

- `04_impl_design/mailbox_if.md`
- `04_impl_design/spdm_report.md`
- `04_impl_design/manufacturing_provisioning.md`
- `04_impl_design/efuse_key_fw_header_design.md`
- `[TBD] firewall_access_rules`

这一步体现了 Skill 的另一个原则：没有冻结的信息不能伪装成已冻结设计。

### 8.7 第六步：同步 code rules 和 traceability

新增约束不应该只停留在方案文档里，还要继续传递到工程开发阶段。

因此本次同步更新了：

```text
security_workflow/05_code_rules.md
security_workflow/06_traceability.md
```

在 code rules 中新增了 `R-BOARD-*`，例如：

- BMC、OOB、板级 MCU、管理子系统只能作为受控链路或代理，不得进入 Root of Trust
- OOB 链路不得直接修改 lifecycle、secure boot、debug enable、rollback counter、Root/anchor
- JTAG 打开必须经过 lifecycle 检查、debug auth、scope bitmap 和失效策略
- USER/PROD 生命周期不得存在板级 JTAG 直通或常开路径
- 管理子系统 DMA 只能访问 firewall 白名单 buffer
- 影响安全启动、恢复、debug 或证明状态的电源/复位/PowerBrake 事件必须进入安全状态机或审计

在 traceability 中新增了 `T-BOARD-*`，把链路连起来：

```text
SRC-005
→ C-BOARD-*
→ Baseline Board / OOB Trust Model
→ 05_board_security.md / 06_interface.md / 10_full_design.md
→ mailbox_if / spdm_report / manufacturing_provisioning
→ sec/oob_req_mgr.* / sec/jtag_scope_ctrl.* / rtl/jtag_mux_ctrl
→ test_oob_cannot_bypass_sec.c / test_user_jtag_denied.c / test_mgmt_dma_firewall.c
```

这说明新增输入最后不是只影响“文档文字”，而是一路传递到未来代码模块和测试用例。

### 8.8 第七步：生成 change impact，留下增量更新记录

最后更新：

```text
security_workflow/04_change_impact.md
```

这个文件记录了：

- 本次新增了什么输入
- 从输入里提取了哪些安全相关要点
- 新增或更新了哪些约束
- 哪些 baseline 发生影响
- 哪些章节已经更新
- 哪些实现级文件需要后续检查
- 哪些冻结风险仍然存在

对项目组来说，这个文件非常重要，因为它回答了一个经常被问到的问题：

> “这次新资料进来，到底影响了哪些地方？”

有了 change impact，就不需要靠人脑记忆“当时为什么改了这几章”。

### 8.9 最终结果是什么

本次增量更新最终形成了一个完整闭环：

| 层级 | 本次结果 |
|---|---|
| 输入登记 | 新增 `SRC-005` 和 `CHG-005` |
| 约束 | 新增 `C-BOARD-01` ~ `C-BOARD-04` |
| Baseline | 增加 Board / OOB / 管理子系统信任边界 |
| 章节级详设 | 展开 `05_board_security.md`，同步 `04_lifecycle_debug.md`、`06_interface.md`、`10_full_design.md` |
| 实现级详设 | 暂不强行更新，标记字段级信息待冻结 |
| Code Rules | 新增 `R-BOARD-*` |
| Traceability | 新增 `T-BOARD-*` |
| Change Impact | 记录本轮影响分析、冻结风险和后续动作 |

这个例子能直观说明：后期使用这个 Skill 时，不是每次都全量重写安全方案，而是先登记输入，再判断影响范围，再只更新受影响的层级，最后把变更记录和追踪链路补齐。

### 8.10 这个例子体现了什么价值

这个管理子系统文档案例体现了 Skill 后期持续迭代的几个价值：

- 新资料进来后，不会直接冲散已有方案
- 有价值的系统流程可以吸收进来
- 安全上不合理的内容不会被盲目继承
- 变更影响范围可以被清楚识别
- 哪些文件已更新、哪些文件待冻结，都有记录
- 方案结论可以继续传递到 code rules、traceability、代码模块和测试计划

换句话说，Skill 让安全方案从“每次靠人工重新梳理”变成“按固定方法增量演进”。

## 9. 这个 Skill 的几个关键规则

- 必须先读 `security_inputs/inputs_manifest.md`
- 不能跳过 `01_constraints.md`
- 不能跳过 `02_baseline.md`
- 不能跳过 `04_impl_design/`
- 冲突不能静默合并
- 假设必须标成 `[ASSUMED]`
- 未闭合问题必须标成 `[TBD]`
- 目录或流程变化时，要同步更新 `SKILL.md`、`prompts/` 和 `README_使用说明.md`

这些规则的目的，是保证流程始终可复用、可审计、可维护。

## 10. 对项目组的价值

对项目组来说，这个 Skill 的价值主要有五点：

- 把安全方案从“文档编写行为”变成“可复用流程”
- 让输入、结论、实现、测试之间建立清晰追踪关系
- 降低跨文档不一致和口径漂移
- 支撑新资料加入后的增量更新
- 为后续 Codex 辅助代码开发提供规则基础

## 11. 适合在哪些场景使用

适合：

- 安全方案仍在持续吸收输入资料
- 方案需要多轮评审和冻结
- 既要做架构说明，也要逐步走向实现
- 后续希望让 AI / Codex 继续参与实现设计和代码生成

不太适合：

- 只想快速生成一页概述，不关心追踪与实现
- 输入非常少，且没有增量维护需求
- 项目组不准备维护 `inputs_manifest` 和中间产物

## 12. 可以怎么向项目组做口头介绍

如果做一个简短介绍，可以直接这样讲：

“这个 Skill 不是为了帮我们一次性写完安全方案，而是为了把安全方案设计过程流程化。它先把输入资料登记清楚，再提取约束，再形成统一 baseline，然后生成章节级详设和实现级详设，最后沉淀成代码规则和追踪矩阵。这样一来，我们的方案不仅更容易评审，也更容易落地，而且后面资料变化时可以做增量更新，不需要每次都从头来过。” 

如果结合管理子系统这个后期案例，也可以这样讲：

“后期有新文档进来时，这个 Skill 不会让我们从头重写方案，而是先把输入登记到 manifest，再判断它影响哪些约束和 baseline，然后只更新受影响章节。比如这次管理子系统文档，我们遵循它的总体架构和流程，但对 JTAG、OOB、DMA、复位这些安全敏感点重新做安全裁决，最后同步到板级安全、接口、总详设、code rules、traceability 和 change impact。这样项目后期新增资料时，方案能持续演进，而且每次改动都有来源、原因和影响范围。”

## 13. 当前工程里的对应落点

在这个工程里，Skill 和目录已经是一一对应的：

- `codex/skills/ngu800-security/`
  - Skill 本体、规则、prompts、templates
- `security_inputs/`
  - 输入材料和 `inputs_manifest.md`
- `security_workflow/01_constraints.md`
  - 约束提取结果
- `security_workflow/02_baseline.md`
  - 基线收敛结果
- `security_workflow/03_detailed_design/`
  - 章节级详设
- `security_workflow/04_impl_design/`
  - 实现级详设
- `security_workflow/05_code_rules.md`
  - 代码规则
- `security_workflow/06_traceability.md`
  - 追踪矩阵
- `security_workflow/04_change_impact.md`
  - 变更影响分析

这说明这个 Skill 不是停留在概念上，而是已经和当前工程工作流绑定起来了。

## 14. 一句话总结

这个 Skill 的本质，是把 NGU800 安全方案设计从“写文档”升级成“按阶段收敛、可评审、可实现、可追踪、可增量更新”的工程化流程。
