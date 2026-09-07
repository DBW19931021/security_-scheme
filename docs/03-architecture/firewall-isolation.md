---
title: "防火墙与隔离"
status: review_ready_with_open_inputs
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0022
  - SRC-0025
owners:
  - GSP
last_reviewed: 2026-08-19
supersedes: []
superseded_by: []
---

# Purpose

定义NGU800P安全软件使用SoC Firewall隔离执行区、Host下发、目标区解密明文、eHSM共享区、Measurement及跨Die/外设访问的总体原则。2 MiB安全RAM的精确Region、权限和生命周期由[安全RAM布局](security-ram-layout.md)作为唯一详细章节维护，本文不复制其分区表。

2 MiB安全RAM相关的规范性Firewall、PMP/MMU/PMA分工和Owner转换合同已合入[NGU800P安全软件详细设计第5章](../05-software-design/NGU800P安全软件详细设计.md)；本专题继续承接后续Multi-Die/UCIe/外设Firewall资料。

# Scope

本轮覆盖2 MiB安全RAM的Firewall接口边界以及后续Multi-Die/UCIe/Host DMA隔离必须遵守的共同规则；具体寄存器、master ID、窗口数量和硬件lock语义等待RTL/接口资料。

# Confirmed facts

- SRC-0031/ADR-0028已固定2 MiB为GSP static 880 KiB、FMC复用128 KiB、Measurement 16 KiB、PMP/RMP各256 KiB和Host ingress 512 KiB；MMP主要使用受保护DDR。C908及全部软件合同统一使用baremetal System Address `0x1010_0500_0000`；eHSM作为受信任master拥有整个2 MiB aperture。
- Host只能直接写专用ingress Region；不得直接访问目标区明文、eHSM packet、Measurement或BootROM/FMC/GSP/PMP/RMP执行区。
- security只报告Firewall/权限错误并阻断release；reset/watchdog/隔离升级动作由RAS策略决定。

# Documented facts

- SRC-0017提供SoC隔离和安全边界上位原则；SRC-0016规定安全启动、明文保护、Measurement和release的软件流程。
- 任何Firewall实际窗口、权限编码、默认值和lock行为都必须由NGU800P RTL/接口Evidence确认，不能从Vendor eHSM代码推断。

# SRC-0025详细输入：UserId、authority与寄存器候选

SRC-0025补充了当前SRC-0022尚未包含的SRAM、`sec_cfg`和`spifc` Firewall截图输入。其逻辑、UserId和位图按`DOCUMENTED`管理；实例base、复位值和冲突字段按`CONFLICTING`管理，详见[来源转录](../../source-vault/internal-specs/firewall/SRC-0025/ngu800p_firewall_logic_register_transcription.md)和[冲突报告](../../sources/conflict-reports/CONFLICT-SRC-0025-FIREWALL-DEFAULTS-AND-BINDING.md)。

## 判定链

```text
check_en=0 -> 透明旁路到Slave
check_en=1 -> 地址恰好命中一个enabled Region
           -> 对唯一Region检查UserId对应的读/写authority位
           -> 两项均通过才到Slave
           -> 任一失败均阻断Slave；hide_en仅选择TNUI返回Error或OK
```

Hide模式不改变安全裁决。测试必须同时观察TNUI响应与Slave select/副作用；不能把`hide_en=1`时的Response OK误判成真实访问成功。

## 16个UserId和28个authority位

| UserId | Master | 写位 | 读位 | 单Master读写掩码 |
|---:|---|---:|---:|---:|
| `0x0` | die0 reserved | - | - | `0x00000000` |
| `0x1` | die0 RAS Core | 0 | 1 | `0x00000003` |
| `0x2` | die0 管理MCU Core | 4 | 5 | `0x00000030` |
| `0x3` | die0 Codec MCU Core | 8 | 9 | `0x00000300` |
| `0x4` | die0 功耗管理MCU Core | 12 | 13 | `0x00003000` |
| `0x5` | die0 Host/PCIe RAM | 16 | 17 | `0x00030000` |
| `0x6` | die0 管理/TOP NOC DMA | 20 | 21 | `0x00300000` |
| `0x7` | die0 eHSM | 24 | 25 | `0x03000000` |
| `0x8` | die1 reserved | - | - | `0x00000000` |
| `0x9` | die1 RAS Core | 2 | 3 | `0x0000000C` |
| `0xA` | die1 管理MCU Core | 6 | 7 | `0x000000C0` |
| `0xB` | die1 Codec MCU Core | 10 | 11 | `0x00000C00` |
| `0xC` | die1 功耗管理MCU Core | 14 | 15 | `0x0000C000` |
| `0xD` | die1 Host/PCIe RAM | 18 | 19 | `0x000C0000` |
| `0xE` | die1 管理/TOP NOC DMA | 22 | 23 | `0x00C00000` |
| `0xF` | die1 eHSM | 26 | 27 | `0x0C000000` |

机读唯一来源为[firewall-userid-authority-map.yaml](../../requirements/firewall-userid-authority-map.yaml)。`local_id=userid&7`为0时必须先拒绝；其余UserId使用`write_bit=4*(local_id-1)+2*die`、`read_bit=write_bit+1`。authority的bits[31:28]必须为0。

双die读写掩码分别为：RAS `0x0000000F`、管理MCU `0x000000F0`、Codec `0x00000F00`、功耗MCU `0x0000F000`、Host `0x000F0000`、DMA `0x00F00000`、eHSM `0x0F000000`；全部14个有效Master为`0x0FFFFFFF`。

## 截图寄存器候选和默认值

截图给出的offset为`F_SRAM_ENABLE=0x000`、`F_SECCFG_ENABLE=0x004`、`F_SPIFC_ENABLE=0x008`，Region1..5配置位于`0x00C..0x060`，错误寄存器位于`0x064..0x09C`；Set/Clear offset为`0x400/0x800`。但base仅为`0xxx_0000`占位，当前SRC-0022生成头未找到该CSR组，因此offset也只能与未来匹配的base/版本整体使用，不能孤立编码。

| 项目 | SRC-0025截图值 | 当前状态 |
|---|---:|---|
| 三个实例`check_en/hide_en` reset | 均为1 | `CONFLICTING`：逻辑需求称总开关默认关闭 |
| Region1..5 enable reset | 均为1 | `CONFLICTING`：与R1覆盖R2-R5共同导致多命中 |
| R1 authority | `0x000000F0` | `DOCUMENTED`位图；地址/default adoption被阻断 |
| R2 authority | `0x0000000F` | `DOCUMENTED`位图；地址/default adoption被阻断 |
| R3 authority | `0x0000F000` | `DOCUMENTED`位图；地址/default adoption被阻断 |
| R4 authority | `0x00000F00` | `CONFLICTING`：位图为Codec，文字称MM Core |
| R5 authority | `0x0FFFFFFF` | `DOCUMENTED`位图；产品全放行必须另行批准 |

量产软件不得依赖上述reset默认态。初始化必须在请求源隔离时完成policy校验、地址/属性/authority写入、逐项readback、错误清理、Region enable，最后才打开`check_en`；任一步失败都保持隔离并fail-close。

# Vendor implementation observations

Vendor只定义eHSM/Core及Host Mailbox能力，不定义NGU800P SoC Firewall、Host aperture、C908 master ID或RAS行为。

# Assumptions

不假定64 KiB是硬件最小粒度，不假定窗口数量足以一一映射ADR-0028的六个固定Region，也不假定warm reset后的Firewall默认值。物理offset/size已经冻结；硬件无法表达16 KiB Measurement或其他边界时必须停止实现并重新评审，不能静默向外扩权。

# Proposed design

1. Power-on默认deny，只按当前stage开放最小Region；每次owner切换固定遵循撤销旧权限、full-system barrier、配置新权限、回读、发布owner的顺序，两个批准PMA候选均不执行data clean/invalidate。
2. FMC/GSP/PMP/RMP及MMP DDR CODE必须从loader阶段的RW/NX切换为RX；DATA/stack/heap始终RW/NX，禁止RWX窗口。若顶层Firewall只能覆盖较粗的连续范围，CODE/DATA的W^X由linker配合C908 PMP/MMU或更细粒度Firewall完成。
3. Host ingress在submit后硬件撤销Host写权限；本版没有独立plaintext Region，Host aperture与所有原地解密目标Region永久隔离。
4. Measurement位于前1 MiB末尾16 KiB，但不是GSP普通可分配内存。Measurement的软件owner/commit保留，对Host只提供协议转换后的脱敏数据；未使用尾部也不得用于证书或scratch。独立Mailbox Region已取消并并入各stage context arena。
5. Firewall配置/回读/lock失败立即阻断目标entry/release，撤销已开放权限并上报RAS；security不得直接reset。
6. 若硬件窗口不足，优先保持Host、所有解密/execute目标和Measurement边界；不得扩大Host窗口、把Measurement并入GSP heap或合并CODE/DATA，应创建设计冲突。

# 流片前测试边界澄清

- 当前确认的硬件权限方向为：只有承担启动核角色的C908具备Firewall配置权限；eHSM和其他核均不具备配置权限。
- Firewall配置寄存器的硬件访问控制不要求鉴权，也不依赖LCS；GSP后续增加的鉴权、命令授权或配置窗口属于产品软件限制，不能作为低层Firewall配置权限测试的前置条件。
- 软件用例只保留两类：在既有SRAM重新配置Case内验证启动核与非启动核的配置权限差异；验证启动核配置SRAM Region后授权/未授权访问隔离生效。eHSM没有公开软件配置路径时由条件性EDA补齐。权限编码、Region和Master组合放入vector，不按每个寄存器或Region机械拆case。
- 软件回读只能间接证明结果；未授权事务是否在互连处被阻断、是否未产生slave select/write-enable，以及边界/burst逐拍权限，必须由RTL/DV硬件验证Evidence证明。
- 以上权限方向记录为`PROPOSED_OWNER_DIRECTION`；精确master ID、配置寄存器、默认值、Region粒度、权限编码和非法访问response仍须由SRC-0022/RTL冻结，禁止猜测。

# Open questions

- Firewall窗口数量、最小粒度、地址对齐、读/写/执行权限编码、master/USER ID和lock机制。
- power-on、warm reset、cold reset、watchdog及局部reset后的默认权限和配置保留范围。
- Host DMA、eHSM DMA、C908、RAS及Die1对统一System Address aperture的可达范围和权限。
- 配置写入/回读/barrier顺序、错误中断、RAS report接口和EMU可观测点。
- OPEN-CONFLICT-014：截图base/reset/Region/R4/bit27/高地址/安全属性/lock/burst与目标D0 RTL/CSR绑定。

# Implementation impact

- Firewall驱动和System Address范围检查收敛到NGU800P platform/port层；业务代码只使用Region ID、owner转换API和受控buffer handle。
- ADR-0016已确认GSP替代OMP/Q&P，OMP不再拥有独立Firewall profile。ADR-0028已固定SRAM容量和offset；OPEN-CONFLICT-006剩余PMA、Firewall及footprint Evidence关闭前不生成生产窗口表或linker，也不修改代码仓。

# Verification impact

- 静态验证Region总和、无重叠、对齐、W^X、窗口可实现性和配置表唯一来源。
- EMU负向验证Host越界/seal后写、访问Measurement/原地解密目标/execute、错误master、跨边界及Firewall回读/lock失败。
- 每项失败断言下游不可达、raw错误保留、RAS收到事件，且security没有直接reset。

# References

- [安全RAM布局](security-ram-layout.md)
- [ADR-0006](../../decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md)
- [ADR-0007](../../decisions/ADR-0007-security-ram-lifecycle-reuse-and-resident-scope.md)
- [ADR-0008](../../decisions/ADR-0008-ehsm-security-ram-access-boundary.md)
- [ADR-0028](../../decisions/ADR-0028-security-sram-fixed-layout-and-in-place-loader.md)
- [CE-SEC-004](../../evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md)
- [OPEN-CONFLICT-006](../../sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md)
- [SRC-0025 Source Card](../../sources/source-cards/SRC-0025.md)
- [Firewall需求](../../requirements/firewall-requirements.yaml)
- [Firewall测试设计](../06-verification/firewall-test-design.md)
- [OPEN-CONFLICT-014](../../sources/conflict-reports/CONFLICT-SRC-0025-FIREWALL-DEFAULTS-AND-BINDING.md)

# Change history

- 2026-08-19：按SRC-0031/ADR-0028同步固定六段SRAM布局、原地解密目标隔离、Measurement 16 KiB和MMP DDR边界；删除独立plaintext/旧P1容量表述。
- 2026-08-14：按SRC-0030将Firewall配置Owner收敛为仅启动核；eHSM和其他核均无配置权限。配置Owner判定并入既有SRAM重新配置Case，eHSM不可达部分由条件性EDA补齐。
- 2026-08-03：澄清Firewall配置权限测试不依赖鉴权/LCS；确认C908可配置、非安全管理核不可配置，并将流片前软件case收敛为配置权限和SRAM隔离两类，内部阻断由RTL/DV验证。
- 2026-07-24：2 MiB RAM的权限矩阵、Owner转换、W^X、Firewall/PMP/PMA分工、readback/lock停止条件和负向测试已合入主详设第5章；专题转为`review_ready_with_open_inputs`，继续等待硬件窗口、master、reset和Multi-Die资料。
- 2026-07-23：负责人批准取消独立Mailbox Region并采用stage-local固定arena；eHSM继续作为整个2 MiB的受信任full-aperture master。
- 2026-07-22：按ADR-0007改为生命周期复用布局；GSP/Measurement/Mailbox连续，BootROM/FMC尾部回收，PMP/RMP/MMP常驻，并明确粗粒度SoC Firewall与固件内部W^X分工。
- 2026-07-22：依据ADR-0006补充2 MiB安全RAM Firewall总体边界；精确分区由`security-ram-layout.md`维护，硬件参数和最终地址仍开放。
- 2026-08-12：登记SRC-0025，补齐16个UserId、28个authority位、默认掩码、候选CSR offset和错误诊断；base/reset/重叠Region/R4/bit27/高地址/安全属性/lock/burst进入OPEN-CONFLICT-014，禁止直接生成产品常量。
