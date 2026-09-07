# INV-SEC-010：NGU800P eHSM MMIO、安全RAM地址视图与PMA绑定

## 调查背景

- 对应Feature：SEC-FEAT-001、005、017、019。
- 对应正式详设：`docs/04-interfaces/ehsm-mailbox.md`、`ehsm-host-adapter.md`、`docs/03-architecture/security-ram-layout.md`。
- 触发问题：B0-R2仍缺Vendor direct Mailbox aperture、Host status/error、16路IRQ准确绑定，以及C908对2 MiB安全RAM的PC/linker和PMA/cache属性证据。
- 目标资料/代码：SRC-0022 `../baremetal/components/chip_riscv_c908_common/include/ngu800p`及`../baremetal`当前平台、linker和solution代码；必要时只读对照`../gsp-pmp-rmp-omp`。
- 只读基线：引用`manifests/repositories.yaml`和SRC-0022登记；按项目硬约束不执行任何Git命令。

## 调查目标

从RTL同步生成头和实际构建/linker/启动代码中确认可以冻结的平台常量与地址域语义；对资料中确实不存在的内容形成准确的RTL/平台集成gap，而不是使用Vendor样例地址或通用Mailbox替代。

## 需要回答的问题

1. SRC-0022是否已经包含Vendor direct 16×4 KiB Mailbox aperture的local/remap或NoC/system base、大小和寄存器block定义？
2. C908可见的eHSM Host status、bootloader/firmware done/error及错误输出是否已有准确base/offset/bitfield宏？
3. `NGU800P_EHSM_O_MAILBOX_IRQ1～16`的准确IRQ号、顺序和与Vendor channel的映射能否由生成头确认？
4. C908 BootROM/FMC/GSP当前linker、启动入口、地址转换和load路径使用local/remap还是NoC/system view；哪些属于旧080x现状而非新D0目标？
5. 2 MiB `MANAGEMENT_SUBSYS_SRAM`是否有PMA/PBMT/MMU/cacheability、Firewall窗口或master属性配置证据？
6. 调查结果能关闭OPEN-CONFLICT-006或OPEN-DESIGN-001的哪些子项；哪些仍必须由RTL/平台Owner补充？

## 建议检查范围

- `../baremetal/components/chip_riscv_c908_common/include/ngu800p/`全部RTL同步map/寄存器/IRQ头。
- `../baremetal/components/chip_riscv_c908_common/src/`中的system/cache/PMA/MMU/IRQ初始化。
- `../baremetal/solutions/bootrom`、`fmc`、`gsp`及相应chip组件的linker、启动和加载路径。
- `../gsp-pmp-rmp-omp`仅用于对照现有linker/入口镜像，不作为硬件常量权威源。
- 必须记录文件、符号、紧凑行范围、构建/宏条件和缺失检索范围，不能只根据名称推断。

## 证据要求

- 区分`CODE_FACT`、`INFERENCE`、`TARGET_DESIGN`、`GAP`、`UNKNOWN`和`CONFLICTING`。
- 硬件常量只从SRC-0022冻结；GSP镜像、Vendor样例和test Expected只作差距证据。
- 对“未找到”记录检索目录、模式和相邻可见事实，不能将未找到描述为硬件不存在。
- 地址必须记录64位值和`SYSTEM_NOC/LOCAL_REMAP/EHSM_REMOTE`domain。

## 输出

- 代码证据：`evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md`。
- 回填Mailbox/Host Adapter、安全RAM、OPEN-CONFLICT-006/OPEN-DESIGN-001、任务和项目状态。
- 如果发现有效资料之间的新冲突，建立独立conflict report并在对话中提交负责人裁决。

## 限制和停止条件

- 只修改`security_-scheme`调查和设计资料。
- 不修改Vendor快照、`gsp-pmp-rmp-omp`或`baremetal`，不执行Git、构建或测试。
- 不使用`SECURITY_SUBSYS_MAILBOX`或Vendor OSR样例base替代eHSM direct aperture。
- 不从旧080x linker或测试Expected冻结新2 MiB目标地址。
- 无权威PMA/Firewall资料时登记gap，不凭通用cache API推断物理属性。

## 验收

- [x] eHSM direct aperture/status/error和16路IRQ逐项有“已确认宏”或“明确缺失gap”。
- [x] C908地址视图和当前linker/load路径具有可复核调用链。
- [x] PMA/cache/Firewall证据和未知边界明确。
- [x] 受影响开放项、详设、计划和项目状态同步。
- [x] 未修改两个代码仓，未执行Git命令。

## 完成记录

- 2026-07-24：完成只读调查并形成[CE-SEC-010](../../evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md)。
- 已确认2 MiB local/system地址对和16路eHSM IRQ encoded值；确认当前SRC-0022仍缺direct aperture/status/error绑定。
- 当前linker显示early stage local、GSP/Runtime system的混合现状，尚无新D0 PC/release证据；cache已启用但未找到2 MiB物理PMA或项目Firewall实例。
- 未修改`baremetal`、`gsp-pmp-rmp-omp`或Vendor资料；未构建、未测试、未执行Git命令。
