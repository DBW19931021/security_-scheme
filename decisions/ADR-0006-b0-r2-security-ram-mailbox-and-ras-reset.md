# ADR-0006：B0-R2安全RAM、Mailbox与RAS Reset边界

- 状态：accepted
- 日期：2026-07-22
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关冲突：OPEN-CONFLICT-006
- 相关合同：FW-C-001、FW-C-002、FW-C-006、FW-C-007、FW-C-008、FW-C-009
- 替代关系：补充ADR-0003～0005；RAM目标与ADR-0004既有GSP地址的关系由OPEN-CONFLICT-006裁决

> 2026-07-22后续裁决：本ADR确认的2 MiB总体边界、Mailbox和RAS职责继续有效；其“各阶段永久独立切片”的早期解释已由[ADR-0007](ADR-0007-security-ram-lifecycle-reuse-and-resident-scope.md)修正。BootROM/FMC使用尾部启动复用区，GSP与Measurement连续，SPDM等计入GSP，PMP/RMP/MMP进入同一总体布局。后续ADR-0008又取消独立Mailbox物理Region并改为stage-local `.ehsm_context_arena` section；P1精确数值仍未冻结。

> 2026-08-19后续裁决：精确布局现由[ADR-0028《2 MiB安全SRAM精确划分与受控原地加载》](ADR-0028-security-sram-fixed-layout-and-in-place-loader.md)冻结；PMP/RMP各256 KiB驻留、MMP主要使用DDR、Measurement固定16 KiB、Host ingress固定512 KiB，独立plaintext Region取消。PMA/Firewall和最终footprint验证仍开放。

> 2026-07-28后续裁决：ADR-0010确认Mailbox follow Vendor包含保持Vendor direct 16×4 KiB布局和公共代码不修改；4 KiB通用SoC Mailbox不用于eHSM wrapper。BootROM/FMC/GSP首版均全程poll；Vendor FW ready后的interrupt仅为后续独立优化。RAS未ready时使用静态错误记录、有限上报等待和关闭普通中断后的无限WFI fail-stop循环。

## 背景

B0-R2需要冻结C908侧安全固件的RAM、eHSM Mailbox和错误处置边界。负责人确认NGU800P提供一块由软件分配并可用Firewall隔离的2 MiB RAM，安全启动和运行需要在其中隔离固件执行区、Host下发区、解密明文区、eHSM packet区和Measurement Table等资源。

## 决策

### 1. 安全RAM总体边界

1. 产品软件唯一使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE=0x1010_0500_0000`、`MANAGEMENT_NOC_S9_SRAM_SIZE=0x0020_0000`（2 MiB），末地址为`0x1010_051F_FFFF`。`0x1000_0500_0000`只保留为RTL历史事实，产品软件不定义映射且输入时直接拒绝。
2. 除BootROM固化代码本体外，BootROM RAM数据/栈、FMC、GSP及安全处理缓冲区在该2 MiB范围内由软件统一规划。
3. Region必须按权限和生命周期划分，而不是要求所有stage永久同时占用：最终常驻时至少包含GSP runtime（含stage-local eHSM context arena）、Measurement、Host固件下发、解密明文及PMP/RMP/MMP；BootROM RAM与FMC执行区允许在尾部启动复用区重叠使用并在退出后回收。错误记录计入当前stage/GSP数据预算，guard/reserved是否需要由容量与硬件粒度证明。
4. 软件分配不是无边界通用heap：采用版本化区域表、固定Region ID、范围/重叠检查、生命周期状态机和Firewall profile；动态分配只能发生在指定工作区内部。
5. Host只允许访问专用下发区；解密明文、eHSM packet、Measurement、执行区和安全工作区不得向Host开放写权限。
6. 各区域在owner切换时执行write/release与full-system barrier、Firewall权限转换和清零；`NON_CACHEABLE`与`HARDWARE_COHERENT`两个批准候选均不执行data clean/invalidate，任一步失败时禁止release下一级。

后续ADR-0007已裁决：该2 MiB正式替代BootROM/FMC/GSP旧080x地址，PMP/RMP/MMP仍驻留其中；ADR-0016又确认GSP替代OMP/Q&P、OMP不作为独立产品镜像。C908地址视图已冻结为System Address；未决范围只剩PMA、精确Region容量和Firewall参数。在这些输入关闭前不冻结最终linker Region offset或代码修改任务。

### 2. eHSM Mailbox

1. NGU800P Mailbox command/response、packet和note/interrupt语义follow SRC-0018 Vendor eHSM Host机制，不另造第二套协议。
2. NGU800P只实现port适配：MMIO基址/channel、System Address active-descriptor同值校验、共享内存PMA/barrier、timer、critical section和错误上报；首版不启用中断，两个批准PMA候选均不执行data clean/invalidate。
3. Vendor OSR固定地址、timer/reset实现不能直接复制；command/response wire格式和已交付Host行为保持Vendor兼容。
4. Vendor公共源码不修改；NGU800P通过项目侧custom header、port函数、外部build和adapter集成。Vendor direct aperture准确base未进入SRC-0022前，禁止用4 KiB通用Mailbox或OSR地址代替。

### 3. Reset与RAS边界

1. BootROM/FMC/GSP安全软件和production eHSM adapter只检测、记录并上报错误，不直接决定或执行系统/eHSM reset。
2. reset、watchdog、局部复位、整机复位或不复位均由RAS策略决定并执行。
3. production接口不得把Vendor `ehsm_port_reset_ehsm()`作为普通timeout/retry路径；需要reset生效的LCS/Debug/OTP等操作必须向RAS提交结构化请求/事件。
4. RAS尚未可用的早期启动阶段，按ADR-0010保存静态`EARLY_SECURITY_ERROR_RECORD`、阻断下一级、撤销权限并尽力清零；在有限deadline内只重试RAS上报，超时后记录`RAS_REPORT_UNAVAILABLE`，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出，不得自行reset。
5. baremetal能力测试若需要reset，必须通过批准的测试fixture/RAS控制路径，不反向定义产品reset策略。

## 影响

- 新增独立安全RAM布局详设，并为每个Region定义地址、大小、owner、Host/eHSM/C908权限、生命周期、清零和Firewall策略。
- eHSM Mailbox公共层可以按Vendor代码继续冻结；NGU800P端口参数单独管理。
- 统一错误结构的处置字段从“当前stage执行reset”改为“RAS action request / report status”；security stage只阻断release并报告。
- OPEN-CONFLICT-006剩余问题关闭前，不修改现有BootROM/FMC/GSP/PMP/RMP/OMP/MMP linker或Manifest地址。

## 参考

- `docs/03-architecture/security-ram-layout.md`
- `docs/04-interfaces/ehsm-mailbox.md`
- `decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md`
- `docs/05-software-design/error-handling.md`
- `sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md`
- `evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md`
