# ADR-0007：安全RAM生命周期复用与常驻固件范围

- 状态：accepted
- 日期：2026-07-22
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017
- 相关证据：CE-SEC-004
- 相关冲突：OPEN-CONFLICT-006
- 替代关系：补充ADR-0006；否决`security-ram-layout.md`最初P0静态永久切片，改用P1生命周期布局

> 2026-07-23后续裁决：本ADR中“eHSM只访问Mailbox窄窗口”的限制已由[ADR-0008](ADR-0008-ehsm-security-ram-access-boundary.md)修正。eHSM可以访问整个2 MiB安全RAM；独立Mailbox物理Region已取消，BootROM/FMC/GSP分别使用stage-local固定`EHSM_CONTEXT_ARENA`。

> 2026-08-19后续裁决：本ADR中“PMP/RMP/MMP全部驻留、独立Host ingress/plaintext、BootROM/FMC复用最高Runtime池”的容量拓扑已由[ADR-0028《2 MiB安全SRAM精确划分与受控原地加载》](ADR-0028-security-sram-fixed-layout-and-in-place-loader.md)替代。继续有效的仅是System Address、Host隔离、生命周期Owner转换、W^X、清零和以最终link map验证容量的原则。

## 背景

ADR-0006确认2 MiB安全RAM总体边界后，首版P0把BootROM、FMC、GSP、协议工作区和所有buffer都按永久同时驻留处理。负责人指出该思路没有利用启动阶段生命周期：BootROM和FMC退出后其RAM可以回收给后续微核；Measurement和Mailbox应与GSP允许地址连续以简化Firewall；SPDM等协议栈属于GSP，不应单独划物理Region。

当前代码README/Linker把PMP/RMP/OMP/MMP各按512 KiB窗口分配，正好占满2 MiB；这些是窗口上限而不是release镜像实际footprint，不能直接沿用为新目标。

## 决策

1. 新2 MiB安全RAM目标正式替代BootROM/FMC/GSP当前080x RAM/linker地址；旧地址只保留为现状和迁移证据。
2. PMP、RMP、MMP最终运行在该2 MiB RAM中，必须进入同一分区表、Manifest/loader和Firewall设计。
3. BootROM固化代码不计入RAM；其data/BSS/stack与FMC image/data/stack放在2 MiB尾部启动复用区。BootROM/FMC退出并由GSP接管后，尾部必须清零、撤销旧权限，并回收给最后加载的微核，当前推荐复用为MMP Region。
4. GSP与Measurement Table在物理地址上连续排列，使GSP可以使用连续allowed range。Mailbox context的物理放置由后续评估决定；ADR-0008已取消eHSM的Region级访问限制。
5. SPDM、MCTP、证书处理及其他GSP协议栈统一计入GSP image/data/stack/heap预算，不设置`SEC_RAM_SPDM_WORK`等独立物理Region；GSP stack/heap必须根据任务和协议峰值给足容量。
6. Host ingress和plaintext仍是独立物理区：Host只访问ingress，eHSM/loader按owner访问plaintext；不能因为简化Firewall而合并两者权限。
7. 顶层SoC Firewall优先按连续GSP complex、共享加载工作区和各微核Region配置；固件内部CODE/DATA的W^X由linker配合CPU PMP/MMU或更细Firewall实现，禁止RWX。
8. 精确offset/size仍为`PROPOSED`，必须以BootROM/FMC/GSP/PMP/RMP/MMP release link map、stack/heap峰值、最大package/plaintext和Firewall粒度证明总和不超过2 MiB。

## 地址视图的最终裁决

C908取指/PC、Manifest、loader、linker和eHSM共享descriptor全部直接使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`定义的System Address `0x1010_0500_0000～0x1010_051F_FFFF`。产品软件不建立Local/System映射；`0x1000_0500_0000`只作为RTL历史事实保留，不能进入Manifest、linker、公共ABI、loader或测试Expected。最终linker只继续等待精确Region容量、PMA和Firewall输入。

## OMP边界

负责人本次明确PMP/RMP/MMP在2 MiB内，但未要求为OMP单独分区。代码注释显示Q&P CPU运行“OMP/GSP”，因此P1当时暂按GSP取代OMP独立镜像处理。后续ADR-0016已经正式确认该方向：GSP替代旧OMP/Q&P产品固件，OMP不再作为独立常驻镜像。

## 影响

- `security-ram-layout.md`的P0表标记为`SUPERSEDED`，P1使用“最终常驻视图+启动复用视图”。
- BootROM/FMC不再形成永久不可回收Region；复用切换固定包含owner撤销、清零、write/release与full-system barrier、Firewall回读和新owner发布，两个批准PMA候选均不执行data clean/invalidate。
- PMP/RMP/MMP进入FW-C-012、linker、Manifest、Measurement和测试范围。
- 当前不修改`gsp-pmp-rmp-omp`；OMP角色后续已由ADR-0016关闭，待容量、PMA、地址视图和OpenSpec批准后创建实施任务。

## 参考

- `docs/03-architecture/security-ram-layout.md`
- `decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md`
- `evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md`
- `sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md`
