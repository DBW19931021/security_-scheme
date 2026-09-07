# SRC-0035 eHSM Vendor RTL入库报告

## 入库结论

`source-vault/vendor_rtl`登记为SRC-0035，自2026-08-26起作为eHSM内部硬件实现细节、当前RTL实现基线和问题查询的第一入口。资料状态为`VENDOR_IMPLEMENTATION`，不自动等于目标NGU800P SoC集成、量产配置或硅片`CONFIRMED`行为。

## 快照摘要

| 项目 | 值 |
|---|---|
| 路径 | `source-vault/vendor_rtl` |
| 文件/目录 | 121个文件、32个目录 |
| 类型 | 116个Verilog、5个SystemVerilog |
| 总大小 | 5733091 bytes |
| 树哈希 | `ab0b2030a50e4ea0cf832dcb01c9eaa00df56816c8bc081b90549cca5027e95b` |
| 顶层候选 | `rtl/osr_ehsm_top.v: osr_ehsm_top` |
| OSR头版本 | `v1.1.0-a / 4019 4_1_0_dev_Intellifusion` |
| Wing-M130组件 | `V1.0 / 40bef22aa166df6a / 20250225` |
| 独立Git | 未发现 |
| 正式filelist/release note/testbench | 未发现 |

树哈希算法为：对全部文件按正斜杠相对路径排序，将每项`relative_path<TAB>size<TAB>file_sha256`用LF连接，对UTF-8字节计算SHA-256。该哈希只固定当前目录快照，不等于Vendor原始交付包哈希或签名。

## 目录能力范围

当前快照包含eHSM顶层候选、内部AHB/AXI和DMA、Wing-M130 CPU、Hash/PKE/SKE/TRNG、CRC/ECC/Memory、KMU/Mailbox/Boot/CGU/RGU、UART/Timer/WDT和若干wrapper/cell模型。它可以支持对eHSM内部模块、端口、译码、状态机、reset/default、连接和参数表达式的静态调查。

以下内容不由该快照单独回答：

- NGU800P SoC顶层wrapper、NoC地址、IRQ路由、PMA/Firewall、时钟/复位/电源/RAS连接；
- SoC产品Lifecycle、启动、Key Slot、eFuse位分配、软件API和安全策略；
- 实际综合filelist、命令行define、约束、宏库、PVT、时序/PPA、版图和硅后行为；
- 模拟/黑盒/工艺单元、TRNG模拟特性和DFT物理可达性；
- 量产逐die Key载体及不可读、lock、ATE和operation proof。

## 入库时发现的门禁

1. `osr_ehsm_top.v`存在`OSR_FPGA`和`OSR_FPGA_ROM`条件分支，但投递不含正式filelist或外部define清单。所有“实际生效”结论必须先补全或显式限定elaboration条件。
2. 目录未出现源码级`` `include``，三个`osr_define_*.v`文件是否参与产品编译及顺序未知，不能因为文件存在就认定宏生效。
3. RTL包含Key/KEK字面量宏。入库文档不抄录数值；这些值不得进入日志、设计、测试Expected或量产配置，也不能关闭OPEN-CONFLICT-012。
4. 83个文件带一致OSR版本头，但并非全部文件都有相同来源标识；整个目录的release一致性仍需delivery note/filelist证明。
5. 本轮未做lint、elaboration、仿真或综合，不能把静态可读性等同于可构建或功能通过。

## 后续使用规则

- 查询eHSM内部硬件问题时，先锁定SRC-0035及树哈希，再按[《eHSM Vendor RTL调查与证据规则》工程约束](../../docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md)沿顶层、配置和逻辑锥定位。
- 结论至少引用`SRC-0035 + 相对路径 + module/symbol + 紧凑行范围 + 配置条件 + 事实状态`。
- Vendor手册、FSP代码、测试或Evidence与RTL不一致时建立冲突报告，不允许静默覆盖。
- 新投递不得覆盖本目录；建立新Source ID、重新计算树哈希并记录`Supersedes`。

## 本次未做事项

- 未修改RTL源码、软件仓或测试用例。
- 未验证RTL能否独立编译、综合或运行。
- 未对任何Key值、产品地址、eFuse位或SoC连接作新的设计裁决。
