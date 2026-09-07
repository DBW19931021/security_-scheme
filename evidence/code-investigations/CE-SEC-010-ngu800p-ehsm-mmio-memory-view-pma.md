# CE-SEC-010：NGU800P eHSM MMIO、安全RAM地址视图与PMA绑定调查

## Evidence metadata

- Evidence ID：CE-SEC-010
- 日期：2026-07-24
- 父任务：INV-SEC-010
- 资料基线：SRC-0022；`baremetal`与`gsp-pmp-rmp-omp`当前只读工作区
- 适用范围：NGU800P D0 C908侧eHSM direct Mailbox/status/IRQ绑定、2 MiB安全RAM地址视图、cache/PMA与Firewall集成
- 操作：只读检索RTL同步生成头、C908平台初始化、IRQ路由、linker和现有map；未修改两个代码仓，未构建/测试，未执行Git命令

## 权威文件快照

以下文件位于SRC-0022登记范围或其同一`baremetal`平台公共目录。生成头均标记生成日期`2026-07-15`、寄存器版本`0.85r_0708`：

| 文件 | SHA-256 |
|---|---|
| `include/ngu800p/config_bus_address_mapping.h` | `8E61CBDD293BE02FE42ADAD358E4031500C4BADED486AC33E3255714058E040F` |
| `include/ngu800p/subsys_address_mapping.h` | `CDF2ECE69E29C8AA7868E10182AA650DF48B14F0B21C1956F7C8F193DBBF3686` |
| `include/ngu800p/ngu800p_ints.h` | `D54A787D7376AA924A7A108FE20407222A57013EFEAD26DEE4B0728B882772AA` |
| `include/ngu800p_interrupt_topology.h` | `CF041E02879D4415CE9B370E186CEF89C3860AC357CA387AEBB526773BAD3D81` |

## FACT-01：2 MiB RAM的两套地址已由SRC-0022明确

- `DOCUMENTED`：`config_bus_address_mapping.h:101-110`定义`MANAGEMENT_SUBSYS_SRAM_BASE=0x100005000000ULL`、`SIZE=0x200000`，范围`0x1000_0500_0000～0x1000_051F_FFFF`，并注明remap到`0x1010_0500_0000～0x1010_051F_FFFF`。
- `DOCUMENTED`：`subsys_address_mapping.h:170-179`定义`MANAGEMENT_NOC_S9_SRAM_BASE=0x101005000000ULL`、`SIZE=0x200000`，并注明Die0 remap为`0x1000_0500_0000`。

因此可冻结同一物理RAM的数值关系：

| address domain | base | size |
|---|---:|---:|
| `LOCAL_REMAP` | `0x1000_0500_0000` | `0x20_0000` |
| `SYSTEM_NOC` | `0x1010_0500_0000` | `0x20_0000` |

该事实不自动回答C908各stage的PC/linker选择，也不自动给出PMA或Firewall属性。

## FACT-02：当前SRC-0022仍没有eHSM direct aperture和Host status/error绑定

在`baremetal/components/chip_riscv_c908_common/include/ngu800p`及两个代码仓的相关平台/solution范围检索`EHSM_PORT_MAILBOX_REG_BASE_ADDR`、`mailbox_reg_base`、`REG_HSM_STATUS_0`、`HSM_STATUS_0`、`BOOTLOADER_DONE/ERR`、`FIRMWARE_DONE/ERR`等符号：

- 找到eHSM时钟、复位、UART pinmux和16路Mailbox IRQ；
- 找到4 KiB/84-message的通用`SECURITY_SUBSYS_MAILBOX`；
- 没有找到Vendor direct 16×4 KiB aperture的base/size/register block；
- 没有找到C908 Host可见HSM status/error寄存器的base、offset、bitfield、读取/清除语义或reset value。

`GAP`：这是“当前生成头/代码检索范围未包含绑定”，不能表述为RTL硬件不存在。真实MMIO实现必须等待RTL/map生成补齐；不得使用4 KiB通用Mailbox、Vendor OSR样例地址或猜测常量代替。

## FACT-03：16路eHSM IRQ编码已确认，channel对应关系尚未确认

`ngu800p_ints.h:1443-1563`连续定义`NGU800P_EHSM_O_MAILBOX_IRQ1～IRQ16`。`ngu800p_interrupt_topology.h:89-101`定义编码格式`[31:16]=APLIC domain ID`、`[15:0]=source ID`。

| 生成宏 | APLIC | source ID | encoded IRQ |
|---|---:|---:|---:|
| `IRQ1` | 9 | 78 | `0x0009_004E` |
| `IRQ2` | 9 | 79 | `0x0009_004F` |
| `IRQ3` | 9 | 80 | `0x0009_0050` |
| `IRQ4` | 9 | 81 | `0x0009_0051` |
| `IRQ5` | 9 | 82 | `0x0009_0052` |
| `IRQ6` | 9 | 83 | `0x0009_0053` |
| `IRQ7` | 9 | 84 | `0x0009_0054` |
| `IRQ8` | 9 | 85 | `0x0009_0055` |
| `IRQ9` | 9 | 86 | `0x0009_0056` |
| `IRQ10` | 9 | 87 | `0x0009_0057` |
| `IRQ11` | 9 | 88 | `0x0009_0058` |
| `IRQ12` | 9 | 89 | `0x0009_0059` |
| `IRQ13` | 9 | 90 | `0x0009_005A` |
| `IRQ14` | 9 | 91 | `0x0009_005B` |
| `IRQ15` | 9 | 92 | `0x0009_005C` |
| `IRQ16` | 9 | 93 | `0x0009_005D` |

`CODE_FACT`：`src/sys/irq_msi.c`通过`SOC_IRQ_APLIC_ID()`和`SOC_IRQ_SOURCE_ID()`拆分该编码并配置对应APLIC source。

`GAP`：两个代码仓没有发现这些宏的实际consumer，也没有权威表明确`IRQ1～16`与Vendor `channel 0～15`是一一顺序映射还是存在偏移/置换。首版BootROM/FMC/GSP均为poll，因此该映射不阻断首版；任何后续interrupt change都必须先补齐逐channel表并在EMU验证。

## FACT-04：当前linker证明存在两种执行视图，但不足以冻结新目标

`gsp-pmp-rmp-omp`当前linker及现有map显示：

| 镜像 | 当前链接地址 | 视图/性质 |
|---|---:|---|
| BootROM text | `0x1000_0800_0000` | 旧Security IROM，local |
| BootROM data/stack | `0x1000_0808_0000` | 旧Security SRAM，local |
| FMC | `0x1000_080C_0000` | 旧Security SRAM，local |
| GSP | `0x1010_0808_0000` | 旧Security SRAM，system/NoC |
| OMP | `0x1010_0500_0000` | 当前2 MiB RAM，system/NoC，512 KiB窗口 |
| RMP | `0x1010_0508_0000` | 当前2 MiB RAM，system/NoC，512 KiB窗口 |
| PMP | `0x1010_0510_0000` | 当前2 MiB RAM，system/NoC，512 KiB窗口 |

`baremetal`当前测试linker则把GSP/RMP/PMP/MMP分别放在`0x1010_0500_0000/0x1010_0508_0000/0x1010_0510_0000/0x1010_0518_0000`，均使用system/NoC视图。

`CODE_FACT`：现有BootROM/FMC/GSP map中的实际section地址分别匹配上述linker；当前BootROM demo仍把FMC load/entry写为旧local地址。没有找到已经完成的D0产品loader/release路径证明新2 MiB目标下每个stage的PC视图和转换点。

结论：

1. 当前运行期GSP/微核实现支持“system/NoC可作为C908 linker视图”的方向性证据。
2. 当前早期BootROM/FMC又使用local视图，说明不能仅凭现有linker推断所有stage必须统一使用同一视图。
3. 旧080x和baremetal测试布局均不能覆盖已批准P1生命周期布局。
4. ADR-0004的Manifest `SYSTEM_NOC` canonical原则继续有效；最终C908 PC/linker和loader转换点仍需启动/RTL Owner提供reset/release aperture证据，OPEN-CONFLICT-006不能据此关闭。

## FACT-05：当前固件启用cache，但没有2 MiB物理PMA配置证据

- `CODE_FACT`：C908 `system.c:338-343`的`cache_init()`无条件启用D-cache和I-cache，`SystemInit():421-423`调用cache、section和PMP初始化。
- `CODE_FACT`：`_mmu_init()`及其页表/PBMT逻辑位于`#if CONFIG_RISCV_SMODE`内。BootROM/FMC Makefile中的`CONFIG_RISCV_SMODE`仍被注释；当前GSP工程定义也未观察到S-mode启用。
- `CODE_FACT`：chip `sub.mk`定义`CONFIG_XUANTIE_SVPBMT=1`，但在当前M-mode构建中不会单独执行`_mmu_init()`或为2 MiB RAM建立PBMT页映射。
- `CODE_FACT`：现有PMP初始化只设置一个通用PMP项，没有针对`0x1000/0x1010_0500_0000`的cacheability配置；PMP本身也不定义内存cache属性。
- `GAP`：未找到硬件PMA表、启动前CSR配置或专门覆盖这2 MiB RAM的PMA/PBMT条目。

因此不能把2 MiB RAM默认为non-cacheable，也不能仅凭`CONFIG_XUANTIE_SVPBMT`声称其已按NC映射。首版若运行在当前M-mode/cache开启模型，shared context的可见性仍必须由实际PMA/coherence资料证明；否则需要平台支持的独立non-cacheable映射/窗口，不能修改Vendor公共poll代码来掩盖。

## FACT-06：存在通用IOPMP驱动，但没有本项目Firewall绑定

- `CODE_FACT`：两个代码仓均包含CSI `iopmp.h`和`src/drivers/iopmp.c`，支持source/domain、TOR/NA4/NAPOT entry、R/W/X和lock/error配置。
- `CODE_FACT`：在排除driver自身声明/定义后，没有找到`csi_iopmp_init()`调用或`DEV_XT_IOPMP_TAG`设备实例。
- `GAP`：SRC-0022当前生成头中没有可识别为本2 MiB RAM Firewall的寄存器block；也没有窗口数、粒度、master ID、权限编码、默认状态、lock或reset行为。

通用驱动只能证明软件库存在相似能力，不能证明NGU800P已把它实例化为本方案所需Firewall，也不能用其默认结构冻结P1窗口。

## 对开放项的影响

### OPEN-DESIGN-001

已收敛：

- 16路IRQ的APLIC/domain/source/encoded数值；
- 首版poll不依赖IRQ/channel映射；
- 当前direct aperture/status/error确实未在SRC-0022出现，而不是被4 KiB通用Mailbox替代。

仍开放并阻断真实port：

- direct aperture base/size和C908访问domain；
- Host status/error base/offset/bitfield、读取/清除/reset语义；
- 2 MiB物理PMA/coherence以及context arena是否必须单独NC；
- 各stage service channel、operation deadline、arena I/O/session总量和RAS实际通知通道。

### OPEN-CONFLICT-006

已增加证据但未关闭：

- system/NoC视图已被当前GSP和四个baremetal Runtime linker实际使用；
- early BootROM/FMC现状仍使用local视图，且没有新D0 release/PC证据；
- 当前production仓存在OMP而没有MMP实现，baremetal则存在MMP测试linker，不能据此自行裁决GSP/OMP/MMP产品角色；
- 当前map只代表旧骨架，不是release容量证据；
- Firewall参数仍完全缺失。

## 平台输入清单和停止条件

在修改真实MMIO或最终linker前，平台/RTL/启动Owner必须提供：

1. eHSM direct aperture的local/system base、完整size、channel stride和C908允许访问的domain宏。
2. Host可见status/error寄存器的base、offset、bitfield、reset value、RO/W1C/clear和采样时序。
3. BootROM、FMC、GSP以及PMP/RMP/MMP的reset/release PC aperture；若不同stage使用不同视图，明确唯一转换Owner和转换公式。
4. 2 MiB RAM的硬件PMA/cache/coherence属性，以及能否对context arena提供独立NC/coherent属性。
5. 负责隔离Host/GSP/各Runtime的Firewall/IOPMP实例、窗口数、最小粒度、master ID、R/W/X、默认deny、lock/reset和readback机制；eHSM master保持全2 MiB放行。
6. 若未来启用中断，提供Vendor channel 0～15到生成宏`IRQ1～16`的逐项映射。

本Evidence形成时仍需项目负责人裁决GSP/OMP产品关系；2026-07-24后续ADR-0016已确认GSP替代原OMP/Q&P固件，OMP不再作为独立产品镜像。release map、Runtime依赖和MMP实现资料仍按OPEN-DESIGN-010后续补充；未补充前不冻结P1绝对offset或加载顺序。

## 本轮结论

- 没有发现新的有效资料冲突，不新建conflict report。
- 真实eHSM MMIO、最终PC/linker、PMA和Firewall仍未达到编码DoR。
- 不受阻的Manifest canonical domain、typed adapter接口、poll状态机、64位range检查和fake-MMIO设计可以继续；不得生成产品base常量、最终linker或精确地址Expected。
