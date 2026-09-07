# 2 MiB安全RAM目标与当前固件Linker冲突

## Identity

- Conflict ID: CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS
- Open Question: OPEN-CONFLICT-006
- Status: open / partially_decided
- Evidence state: CONFLICTING
- Owner: 项目负责人；SoC memory map/BootROM/FMC/GSP/PMP/RMP/MMP平台Owner待确认
- Decision required by: 冻结P1精确容量、Manifest地址和修改任何linker之前

## Conflict classification

- Type: target_design_vs_current_implementation / address_ownership / baseline_supersession
- Affected scope: BootROM/FMC/GSP/PMP/RMP/MMP最终linker、Manifest load/entry、Measurement地址快照和Firewall区域。OMP产品角色已由ADR-0016关闭。
- Safe-to-continue scope: 已批准的P1生命周期、GSP/Measurement连续、PMP/RMP/MMP纳入、eHSM full aperture和RAS只上报原则；Mailbox arena方案可继续评审。
- Must-stop scope: 最终Region offset/容量、linker修改、Firewall绑定和精确测试Expected；System Address总范围已裁决，不再开放地址视图选择。

## Source A：负责人确认的新目标和后续裁决

- Decision: ADR-0006、ADR-0007、ADR-0008。
- Target: baremetal `MANAGEMENT_NOC_S9_SRAM_BASE=0x1010_0500_0000`起始2 MiB System Address范围由安全软件统一分配，BootROM RAM/FMC/GSP和安全buffer均位于其中，并使用Firewall隔离；local/remap值不进入产品软件。
- Partial resolution: 新2 MiB正式替代旧080x；PMP/RMP/MMP驻留；BootROM/FMC尾部复用并回收；GSP/Measurement连续；SPDM等并入GSP；eHSM可访问整个2 MiB。独立Mailbox Region已取消，BootROM/FMC/GSP分别采用stage-local固定arena。ADR-0016进一步确认GSP替代旧OMP/Q&P固件，OMP不再作为独立产品镜像。

## Source B：当前地址头

- CE-SEC-004 FACT-01。
- 当前地址头确认该2 MiB local范围，并提供system/NoC视图`0x1010_0500_0000～0x1010_051F_FFFF`。

## Source C：当前linker和README

- CE-SEC-004 FACT-02/03。
- BootROM RAM当前在`0x1000_0808_0000`，FMC在`0x1000_080C_0000`，GSP在`0x1010_0808_0000`。
- 2 MiB当前被OMP/RMP/PMP各512 KiB linker占用，README还预留MMP第四个512 KiB区域。

## Source D：CE-SEC-010平台绑定复核

- SRC-0022再次确认local/remap `0x1000_0500_0000`、system/NoC `0x1010_0500_0000`和2 MiB容量，文件生成日期为2026-07-15、版本`0.85r_0708`。
- 当前GSP linker以及baremetal GSP/PMP/RMP/MMP linker都使用system/NoC视图；当前BootROM/FMC旧080x linker使用local视图。
- 没有找到新D0产品reset/release/PC路径证明所有stage统一使用任一视图；当前混合现状不能成为最终规则。
- 当前C908初始化启用I/D cache，但没有找到本2 MiB物理PMA、独立NC映射或项目Firewall实例/参数。
- `gsp-pmp-rmp-omp`当前有OMP而未见MMP产品实现；`baremetal`有MMP测试linker。该差异只增强产品角色未决事实，不授权从测试仓反推产品布局。

## 已解决的冲突部分

1. BootROM/FMC/GSP不再继续使用080x地址；新2 MiB是D0目标。
2. PMP/RMP/MMP不迁出，进入同一2 MiB分区表；当前512 KiB只视为旧窗口上限。
3. BootROM/FMC不是最终常驻Region，放在尾部启动复用区并在GSP接管后清零回收。
4. 旧GSP `0x1010_0808_0000`不再是新D0最终绝对地址。
5. P0静态永久切片被否决，改用P1常驻+启动复用视图。
6. eHSM作为受信任master可访问整个2 MiB，不设置Region级Firewall限制。
7. GSP是旧OMP/Q&P固件的产品替代；OMP不再分配独立RAM、package、Measurement、counter、loader或release身份。

## 仍未解决的精确冲突

1. 当前没有release link map/stack watermark证明GSP complex、共享工作区、PMP/RMP/MMP可在2 MiB内同时满足；产品仓也没有观察到MMP完整实现。现有map属于旧骨架，不能作为release容量证据。
2. 当前README与OMP/RMP/PMP实际linker顺序不完全一致，不能沿用旧顺序作为P1依据；OMP旧linker只作迁移证据。
3. 独立Mailbox Region及arena物理放置选择已由负责人关闭；ADR-0011已冻结首版每stage一个256字节/64字节对齐context slot、单在途、one-shot only、零自动retry和timeout quarantine。CE-SEC-010确认当前cache开启但没有2 MiB物理PMA证据；最终属性待SoC稳定后在`NON_CACHEABLE`与`HARDWARE_COHERENT`中唯一裁决。service channel、实际descriptor/buffer和arena总容量仍未冻结，首版不分配session。

## Impact

- Software: 需要重做多个linker、启动加载顺序和地址生成源。
- Security: 未裁决即混用可能覆盖运行固件、Host buffer或Measurement，导致任意代码执行/明文泄露。
- Compatibility: 地址变化会影响Manifest、Measurement、Firewall、EMU脚本和测试Expected。
- Capacity: 旧四个512 KiB窗口已经耗尽2 MiB；必须用release实际footprint重新压缩，不能把窗口上限当镜像需求。

## 当前推荐

- 采用`security-ram-layout.md`的修订P1顺序：低地址GSP complex（含协议栈和推荐的eHSM context arena）→Measurement→Host ingress→plaintext→PMP→RMP→MMP。
- BootROM/FMC在MMP尾部Region上建立启动复用视图，MMP最后加载。
- P1示例容量`448/64/256/256/256/256/512 KiB`只用于验证思路；448 KiB内含arena样例预算，不表示arena需要64 KiB。
- C908、Manifest、loader、linker和eHSM共享descriptor全部固定使用baremetal System Address，不建立Local/System转换；旧local linker只能作为迁移差距输入。

## Required owner decision

1. 各stage使用哪个service channel？SoC稳定后从`NON_CACHEABLE`与`HARDWARE_COHERENT`中选择哪个唯一PMA属性？实际one-shot descriptor/buffer和arena总容量是多少？
2. 各release固件和stack/heap上限、最大Host package/plaintext是多少，P1容量如何据此冻结？
3. Firewall最小粒度和窗口数如何实现Host/Measurement/各微核隔离，以及eHSM full-aperture master配置？

## Review history

- 2026-07-22：B0-R2核对负责人RAM输入和当前地址头/linker时发现；建立OPEN-CONFLICT-006，阻断最终绝对地址和linker实施。
- 2026-07-22：负责人确认新2 MiB替代旧080x，PMP/RMP/MMP纳入；否决P0静态切片，采用BootROM/FMC尾部复用、GSP/Measurement/Mailbox连续、SPDM并入GSP的P1方向。冲突收敛为地址视图、容量、OMP角色和Firewall参数。
- 2026-07-23：负责人确认eHSM可访问整个2 MiB；接受ADR-0008。随后批准取消独立Mailbox Region并改用stage-local固定arena；通用异步/流式及timeout未闭环context不得放普通函数栈，arena参数继续在B0-R2冻结。
- 2026-07-23：ADR-0011进一步冻结首版单context/单在途、256字节slot、GSP唯一service、零自动retry和timeout quarantine；OPEN-CONFLICT-006只保留最终PMA、service channel、arena总容量及原有PC/linker、GSP/OMP、容量和Firewall问题。
- 2026-07-24：完成CE-SEC-010；当前system/runtime与local/early混合linker事实不足以关闭新D0 PC视图，且PMA/Firewall绑定未找到。未新增冲突，继续等待启动/RTL/平台证据和GSP/OMP负责人裁决。
- 2026-07-24：负责人批准ADR-0016；GSP替代旧OMP/Q&P产品固件，OMP不再作为独立产品镜像。关闭产品角色子项，冲突继续等待PC视图、PMA、容量和Firewall证据。
- 2026-07-28：负责人裁决C908全链路只使用baremetal System Address，不存在Local/System映射，地址视图子项关闭；首版固定one-shot only且不分配session。冲突仅保留精确Region容量、PMA唯一选择、Firewall和arena集成参数。
