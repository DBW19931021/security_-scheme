# 代码调查证据

本目录保存正式详设阶段的只读代码证据报告。报告使用 `CE-SEC-xxx-<topic>.md` 命名，并与 `tasks/active/INV-SEC-xxx-<topic>.md` 共享编号。

代码证据只说明特定只读基线下的代码现状，不自动成为目标设计、硬件事实或测试通过结论。报告必须记录路径、符号、调用链、构建条件、限制和与目标设计的差异。

调查完成后，由正式详设负责人抽查关键源码并回填设计/追溯；原始构建、测试或硬件Evidence仍按批次保存在相应 `evidence/` 子目录。

## 当前报告

| Evidence | 对应任务 | 状态 | 主要结论 |
|---|---|---|---|
| [CE-SEC-001](CE-SEC-001-bootrom-ehsm-startup.md) | INV-SEC-001 | evidence_ready / awaiting_design_review | 默认BootROM无生产eHSM/FMC链；可选demo走stub；Vendor OSR port不可直接复用 |
| [CE-SEC-002](CE-SEC-002-fmc-gsp-verification-chain.md) | INV-SEC-002 | evidence_ready / awaiting_design_review | FMC/GSP为hello-world；验证链止于stub；无loader/jump；GSP地址域冲突 |
| [CE-SEC-003](CE-SEC-003-ehsm-demo-entry-inventory.md) | INV-SEC-003 | evidence_ready_level_1 / level_2_pending | `ehsm_demo_test()`的Root/BL/FW一级入口已盘点；二级case和`bl_demo`复用映射待继续 |
| [CE-SEC-004](CE-SEC-004-security-ram-map-and-mailbox.md) | INV-SEC-004 | evidence_ready | 现有080x linker与目标2 MiB冲突；形成安全RAM/复用/FW边界输入 |
| [CE-SEC-005](CE-SEC-005-ehsm-context-memory-lifetime.md) | INV-SEC-005 | evidence_ready | context/session生命周期不要求独立Mailbox Region，但需要stage-local静态arena |
| [CE-SEC-006](CE-SEC-006-ehsm-mailbox-vendor-baseline.md) | INV-SEC-006 | evidence_ready | Vendor 16 channel、地址转换、poll及ready/error门禁收敛 |
| [CE-SEC-007](CE-SEC-007-ngu800p-ehsm-port-binding.md) | INV-SEC-007 | evidence_ready | 4 KiB通用Mailbox不兼容Vendor direct布局；C908 cache/timer/IRQ原语可用 |
| [CE-SEC-008](CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md) | INV-SEC-008 | evidence_ready | Vendor poll只有send前cache钩子且timeout无法判定提交状态；首版需单在途、active scope和quarantine |
| [CE-SEC-009](CE-SEC-009-secure-package-verify-loader-contract.md) | INV-SEC-009 | evidence_ready / conflict_open | Vendor verify只返回raw status且按命令长度处理Code；公司两仓为不兼容stub骨架；形成package/Manifest/verify/loader候选合同和OPEN-CONFLICT-008 |
| [CE-SEC-010](CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md) | INV-SEC-010 | evidence_ready / platform_input_required | 确认2 MiB双地址、16路IRQ精确编码和当前linker视图；direct/status、PMA及Firewall绑定仍缺平台证据 |
| [CE-SEC-011](CE-SEC-011-vendor-version-counter-contract.md) | INV-SEC-011 | evidence_ready / conflict_open | 冻结16字节单向编码、BL暂存和FW启动提交行为；Host 64位counter与FW不配套；建立OPEN-CONFLICT-009 |
| [CE-SEC-015](CE-SEC-015-vendor-bl-error-injection-capability.md) | INV-SEC-015 | evidence_ready / awaiting_design_review | Host 定义 `0xFF13` 注错格式，但当前 BL 2.3.5 source/config/ELF 无 handler；当前软件 case 必须 INCONCLUSIVE |
