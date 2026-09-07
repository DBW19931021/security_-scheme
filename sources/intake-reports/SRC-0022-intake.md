# SRC-0022 入库报告：NGU800P RTL同步SoC地址与寄存器生成头

## 输入与范围

- 外部工作区：`../baremetal/components/chip_riscv_c908_common/include/ngu800p`
- 适用：NGU800P D0 SoC map、寄存器定义和IRQ硬件常量。
- 项目负责人输入：这些地址已经在`baremetal`中准确维护并同步到RTL；后续地址问题以该代码为准。
- 操作：只读检索和哈希；未修改`baremetal`，未执行Git命令。

## 版本与完整性

- 自动生成日期：2026-07-15。
- 寄存器版本：`0.85r_0708`。
- 文件数：51。
- 总大小：2162612 bytes。
- 目录manifest SHA-256：`2969fdcf644b381f53c6279f8886f9f541e6c70bafb98997b35e3f582423daa6`。

manifest哈希计算方法见SRC-0022 Source Card。该哈希固定本次调查看到的目录状态；SRC-0022本身是living reference，后续变更必须重新核对生成日期、RTL适用性和受影响设计。

## 本次抽查

| 项目 | 权威定义 | 结果 |
|---|---|---|
| 2 MiB RAM local/remap视图 | `config_bus_address_mapping.h`中的`MANAGEMENT_SUBSYS_SRAM_*` | `0x1000_0500_0000` / `0x200000` |
| 2 MiB RAM NoC/system视图 | `subsys_address_mapping.h`中的`MANAGEMENT_NOC_S9_SRAM_*` | `0x1010_0500_0000` / `0x200000` |
| Security Subsystem Mailbox local/remap视图 | `SECURITY_SUBSYS_MAILBOX_*` | `0x1000_0841_0000` / `0x1000` |
| Security Subsystem Mailbox NoC/system视图 | `SECURITY_NOC_S8_MAILBOX_*` | `0x1010_0841_0000` / `0x1000` |
| Mailbox寄存器布局 | `regs/mailbox_security.h` | 84-message通用布局 |
| eHSM IRQ | `ngu800p_ints.h` | 独立枚举IRQ1～16 |

## 对B0-R2的影响

1. OPEN-CONFLICT-006不再等待另一份地址表来确认2 MiB RAM的两组基址和容量；本次接收时剩余问题包括C908取指/linker视图、GSP/OMP关系、容量和Firewall参数。后续ADR-0016已经关闭GSP/OMP产品关系，当前只保留地址视图、PMA、容量和Firewall参数。
2. ADR-0010批准Vendor direct Mailbox合同，4 KiB/84-message `SECURITY_SUBSYS_MAILBOX`不作为eHSM wrapper。当前SRC-0022尚未找到direct aperture/status宏，须在真实MMIO实现前同步补齐。
3. `gsp-pmp-rmp-omp`中同名头的相关值虽然本次比较一致，但生成日期较早，不再作为后续地址裁决源。

## 待确认语义

- Vendor direct eHSM aperture和Host status/error准确宏尚未在当前SRC-0022中定位；这是集成同步输入，不再是wrapper/direct方案选择。
- 负责人此前已批准该2 MiB承载全部安全子系统软件；硬件宏名`MANAGEMENT_SUBSYS_SRAM`不改变这一项目用途，不再作为开放问题。
