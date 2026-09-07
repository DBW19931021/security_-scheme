# ADR-0032：eHSM Vendor RTL硬件实现基线与查询规则

- 状态：accepted
- 日期：2026-08-26
- 决策人：项目负责人
- 来源：[SRC-0035《OSR eHSM 4019 Vendor RTL硬件实现快照》Source Card](../sources/source-cards/SRC-0035.md)
- 关联：ADR-0001、ADR-0002、ADR-0009、ADR-0025
- 替代关系：更新此前“eHSM/Core内禀硬件细节默认由Vendor手册和匹配代码共同回答”的查询顺序；不替代上述资料或任何SoC/方案权威源

## Context

项目负责人已把`source-vault/vendor_rtl`加入工程输入，并指定后续eHSM内部硬件细节、基线和问题查询均以该RTL为来源。现有规则只登记了Vendor软件快照、活FSP代码和Vendor手册，没有冻结RTL快照身份、实现权威范围、elaboration条件、敏感内容和与NGU800P SoC资料的边界。

当前目录包含121个Verilog/SystemVerilog文件，顶层候选为`osr_ehsm_top`，树哈希和版本头已登记为SRC-0035；但未发现正式filelist、release note、构建/仿真环境、约束或独立Git元数据，且目录包含Key/KEK字面量宏。因此必须把“第一查询源”与“无条件产品真值”严格区分。

## Decision

### 1. 当前实现基线

- SRC-0035成为当前eHSM内部RTL实现细节、实现基线和问题查询的第一入口。
- 所有结论必须锁定`SRC-0035 + tree hash + 相对路径 + module/symbol/line + 配置条件`，不得只引用裸目录或“当前RTL”。
- 直接由RTL证明的事实状态为`VENDOR_IMPLEMENTATION`；只有匹配版本、配置和目标环境的仿真/EMU/FPGA/硅片Evidence才能进一步提升，不得把RTL文本直接标为`CONFIRMED`。

### 2. 查询顺序

eHSM内部硬件查询固定按以下顺序执行：

1. 读取SRC-0035 Source Card并校验树哈希和有效状态；
2. 确认实际顶层、filelist、外部define、parameter/generate、wrapper和库绑定；
3. 沿完整实例链和逻辑锥读取源、状态、reset/error/clear/lock及消费者；
4. 对照Vendor手册确认接口意图，对照任务锁定commit的FSP确认软件使用假设；
5. 涉及SoC时切换到SRC-0022、目标SoC RTL/CSR/集成资料和方案Source；
6. 按`VENDOR_IMPLEMENTATION / ASSUMPTION / CONFLICTING`形成结论和验证建议；资料不足项另列待确认问题。

具体证据格式和停止条件由[《eHSM Vendor RTL调查与证据规则》工程约束](../docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md)执行。

### 3. 权威边界

SRC-0035可作为以下当前实现事实的第一来源：eHSM内部模块层级、顶层端口、内部总线/译码、寄存器实现、信号宽度、状态机、reset/default表达式、内部CPU、KMU/OTP连接、密码模块、Mailbox、DMA和内部外设。

SRC-0035不单独定义：

- NGU800P SoC顶层wrapper、System Address/IRQ、PMA/Firewall、clock/reset/power/RAS产品连接；
- eFuse产品位、Lifecycle、Key Slot、Secure Boot、Recovery、软件ABI和其他产品策略；
- 实际综合filelist和define、工艺宏、PPA/时序/版图、模拟TRNG质量、DFT物理可达性或硅后行为；
- 量产逐die Key、ATE、不可读/lock和operation proof。

SRC-0022继续是NGU800P SoC地址/寄存器/IRQ数值第一权威源；SRC-0017/SRC-0016和accepted ADR/amendment继续定义系统及软件目标。Vendor手册继续定义正式接口说明。不同来源明显不一致时按ADR-0002建立冲突报告，不得用“RTL优先”静默覆盖。

### 4. 快照和敏感内容控制

- `source-vault/vendor_rtl`按不可变只读输入管理；不得直接修复、格式化或写入生成物。
- 当前树哈希为`ab0b2030a50e4ea0cf832dcb01c9eaa00df56816c8bc081b90549cca5027e95b`。内容变化时立即停止沿用SRC-0035；新投递建立新Source ID和`Supersedes`。
- 新交付应同时取得delivery/release note、正式filelist、外部define/parameter、wrapper/库清单和验证版本关系；缺失项必须写入结论限制。
- RTL含Key/KEK字面量。不得在设计、报告、日志、测试Expected或对外材料中复制这些数值，也不得把它们作为量产Key或一机一密完成证据；OPEN-CONFLICT-012继续有效。
- 未确认授权与保密等级前，RTL按`RESTRICTED_PENDING_CONFIRMATION`处理，禁止外发或上传第三方服务。

## Consequences

- eHSM内部问题有了单一、可复现的第一查询入口，已能由当前RTL回答的问题不再重复向负责人索取。
- 所有结论必须先证明实际elaboration条件，避免把未实例化模块、FPGA分支或孤立宏误写成产品硬件。
- Vendor手册/FSP、SoC资料和产品方案仍各自保持权威边界；发现真实差距时会形成可见冲突，而不是被查询顺序掩盖。
- 工程获得可审计的版本更新路径，但本ADR不授权修改RTL、产品软件或测试代码。

## Rejected alternatives

- 只登记路径、不分配Source ID和树哈希：无法复现结论或识别静默替换，拒绝。
- 把RTL直接定义为`CONFIRMED`硬件行为：缺少elaboration、验证和硅片Evidence，拒绝。
- 让eHSM RTL覆盖SoC地址/寄存器/策略：超出Vendor边界并与SRC-0022/SRC-0016/SRC-0017分工冲突，拒绝。
- 把所有存在文件都当作量产编译输入：当前没有filelist和外部define证明，拒绝。
- 在文档中记录Key宏值以便查询：扩大敏感材料暴露且可能被误用于量产，拒绝。
