---
title: "GSP 软件设计"
status: draft
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0032
  - SRC-0033
owners:
  - GSP
last_reviewed: 2026-08-21
supersedes: []
superseded_by: []
---

# Purpose

定义 GSP 接收 FMC 信任、建立安全运行态、验证/释放 Runtime 固件并提供 measurement/attestation 服务的职责和入口门禁。

> 主详设反向链接：[《NGU800P安全软件详细设计》第8章“GSP详细设计”](NGU800P安全软件详细设计.md#第8章-gsp详细设计与runtime加载隔离)

# Scope

覆盖 GSP Reset/entry、FMC handoff、FreeRTOS 启动前后安全初始化、Runtime image 验证、measurement 接管、SPDM/MCTP production service 准入和失败处理。

# Confirmed facts

1. GSP当前链接地址是`0x1010_0808_0000/0x40000`，packager/tests使用`0x1000_0808_0000`；地址域原则已按ADR-0004裁决，但两个旧绝对地址与ADR-0006新2 MiB目标冲突，不是新D0批准地址。
2. GSP `pre_main()` 创建 FreeRTOS task，默认 `main()` 只循环打印 hello-world。
3. 只有显式 QEMU test 宏会执行 SPDM/MCTP/PLDM 测试；默认 main 不验证 Runtime image，也不启动 production responder service。
4. GSP 构建包含 `ehsm_stub`、attestation sign/cert stub 和 SPDM crypto/cert stub。
5. 当前 measurement store 是本镜像 BSS 全局数组，没有从 BootROM/FMC 的可信 handoff 接管。

完整证据见 [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。

# Documented facts

- SRC-0016/SRC-0017 规定 GSP 软件安全能力、Runtime 验证、measurement/attestation 和系统隔离目标。
- 具体 Runtime image 列表、顺序、地址、权限和失败策略必须在有效软件方案或受控 amendment 中体现。

# Vendor implementation observations

Vendor eHSM/Core可提供verify/decrypt、key/counter/signing等命令输入；Vendor公共Host代码和direct Mailbox布局保持不变，NGU800P通过外部port/custom配置集成。GSP的scheduler、service ownership、Runtime release和SoC firewall不属于Vendor范围。

# Assumptions

ADR-0028已固定GSP在前1 MiB：静态加载和启动footprint位于前880 KiB，FMC退出后再回收128 KiB为动态heap/buffer，最后16 KiB Measurement永久排除。SPDM/MCTP、证书处理、task stack和heap均计入上述GSP容量，不另设协议物理区。本轮仍不假定Runtime启动顺序、具体RTOS priority、SPDM wire、provider key/cert或MMP DDR release接口。GSP唯一eHSM Owner和替代旧OMP/Q&P身份均已冻结。

# Proposed design

## 启动阶段

| Design ID | GSP 动作 | 门禁 |
|---|---|---|
| GSP-DES-001 | 在scheduler/普通service前验证FMC输入、entry System Address、内部stage/status和Measurement Table；确认entry位于固定`SEC_RAM_GSP_STATIC` | entry必须位于`0x1010_0500_0000～0x1010_050D_BFFF`；当前不采用Handoff ABI |
| GSP-DES-002 | 建立只读/受控的BootROM/FMC Measurement view，校验table version、实际count/length、边界、CRC/commit和双Header稳定性；接受当前启动Table中唯一、FMC生产、verify PASS、release authorized且已commit的GSP条目，并要求其16字节rollback counter与FMC条目及`stored_counter_after`快照一致 | 不允许用空BSS store伪装上游Measurement；缺失、重复、Header前后不一致或不稳定快照一律在普通service前fail-close |
| GSP-DES-003 | 第一个、最高优先级的`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner，使用一个64字节对齐、256字节静态context slot，以`EHSM_DRV_MODE_WAIT_AND_POLL`初始化production adapter、active cache/timeout scope、error/audit、counter和key/cert provider；完成启动门禁后同一task转入长期service loop | stub/provider不可达；其他task只走typed queue，任何task不得绕过service直接调用Vendor或Mailbox；quarantine后不移交Owner |
| GSP-DES-003A | eHSM此前始终只运行BL；GSP建立受控ingress，等待Host下发并seal完整Vendor type 0 eHSM FW package，再以`image_out=NULL/boot=true`请求BL验证/启动；该包不解释NGU Header Overlay。只有`firmware_done == 1 && firmware_err == 0`才发布服务 | 保存raw FW status/version及Vendor type 0 counter；首版FW ready后仍继续poll | FMC不得预先加载；不得假设FW预置；eHSM FW使用独立counter域；FW错误/timeout阻断全部依赖服务 |
| GSP-DES-004 | PMP/RMP使用固定256 KiB目标区原地解密、源摘要、`memmove`、目标回读摘要后Measurement/release；MMP保留独立门禁但目标为获批DDR Profile | 每个image独立gate；PMP/RMP package/runtime分别不超过256 KiB；MMP DDR未绑定时blocked | type 2/3、Vendor PASS直通release、普通Host DDR和重叠`memcpy`均拒绝；单Runtime确定失败只隔离该对象 |
| GSP-DES-005 | 配置GSP static、FMC回收、Measurement、PMP、RMP和Host ingress固定Region；GSP entry后先清零回读FMC 128 KiB再加入动态池；eHSM master对整个2 MiB放行 | 配置readback和lock成功；Host不可访问明文目标/执行/Measurement；Measurement不进入allocator | 未闭环FMC/eHSM事务时禁止回收；PMA/Firewall不足则停止实现 |
| GSP-DES-006 | 仅在真实Measurement、静态Issuer前缀、DICE Alias派生、动态Leaf、crypto/sign和物理transport就绪后启动SPDM/MCTP | 禁止QEMU synthetic data、静态Leaf或软件Key进入production |
| GSP-DES-007 | 通过统一eHSM service向受信任内部SoC模块提供签名、随机数、Hash及安全软件方案Feature需要的其他能力；ADR-0017三套Profile所需primitive为产品必需能力 | 首版先满足启动/Measurement/SPDM必需路径；业务不得直调Vendor | ADR-0019确认不向Host开放GSP服务；禁止Host passthrough；三套Profile外算法只作baremetal能力/回归 |
| GSP-DES-008 | GSP发起Key/Certificate/Rotation及方案要求的受控操作，eHSM根据实际LCS作最终允许/拒绝 | GSP先完成caller、operation、参数、地址、usage和审计检查 | 不得覆盖eHSM拒绝，也不得把任意Host输入直接透传 |
| GSP-DES-009 | 验证Entry0=BootROM生产FMC、Entry1=FMC生产GSP的最终readback digest；构造96B DICE TCB Context，经唯一eHSM service用slot13 UDS派生opaque CDI/Alias Key；固定Profile writer组装动态Leaf并由slot14签发，Alias Key签Report/SPDM transcript | static prefix Header v2/metadata/slot14公钥绑定有效；动态Leaf`<=4096 B`；新增工作峰值`<=12 KiB` | 不导出UDS/CDI/private key；不生成BootROM/FMC多层证书；不使用Measurement Region；任一失败保持Attestation NOT_READY |

## Runtime image 通用验证合同

每个image必须有唯一typed-stage image/instance/die、source、最大`Code_Size`、固定Vendor type 1、唯一System Address `load_addr`、`entry_addr=load_addr`、global rollback-counter规则、measurement实例、依赖、release owner和失败传播规则。通用验证函数必须返回completion、raw eHSM code、已认证Header Overlay、loader源/目标摘要、counter/measurement结果和可审计stage error，不能只返回单一`security_status_t`。eHSM Vendor FW是Vendor type 0专用流程，不解释Runtime Header Overlay。

首版GSP下游镜像集合固定为PMP、RMP、MMP和eHSM Vendor FW。OMP不是第五个下游镜像，也没有独立package、Measurement或release状态。四个批准对象分别维护`EMPTY/STAGED/VERIFYING/VERIFIED/LOADED/RELEASED/ISOLATED`状态及独立错误/Measurement记录。

FMC已在初始化时通过BL staged-candidate接口提交SoC global counter。eHSM FW type 0启动使用独立counter域，不消费或证明SoC global counter；GSP不得依赖FW启动副作用。

- PMP/RMP/MMP得到确定完成的验证失败时，只隔离对应Runtime。其他独立验证通过的Runtime可继续；显式依赖失败目标的consumer保持`NOT_RELEASED`，但不伪造为自身验证失败。
- eHSM Vendor FW失败时隔离FW并禁用依赖FW的服务，GSP最小错误上报/管理控制面按批准策略保留。
- eHSM timeout/BUSY/unknown completion属于共享service故障，仍按ADR-0011全局quarantine，不能按单Runtime失败继续发命令。

## FreeRTOS/并发

- GSP入口先创建静态queue/context和第一个、最高优先级的`security_service_task`；该task独占完成安全启动gate，非必要task/service在gate通过前不得运行或不得取得业务输入。
- bootstrap和runtime使用同一typed dispatch；queue在bootstrap门禁通过前不对普通runtime caller开放，门禁通过后同一task进入长期service loop。
- 所有其他task通过同一typed queue串行请求；首版最多一条在途事务，不按16个硬件channel并发，也不把互斥锁视为直接调用Vendor的授权。
- `security_service_task`等待queue时必须阻塞，每个poll operation必须有批准deadline；其优先级不得屏蔽timer/RAS等平台关键中断。
- timeout或BUSY异常后service全局进入`QUARANTINED`，后续task在访问Mailbox前失败；不能只让原caller失败后继续发命令。
- measurement/counter使用各自锁；eHSM context只有service owner可访问，首版没有ISR owner。
- production service 不得修改已提交的BootROM FMC条目或FMC GSP条目；唯一SoC State提交后Table在本boot不可变。已release Runtime/Die1 reset或隔离时SPDM进入`NOT_READY/CONTENT_CHANGED`，同boot不得重新release，恢复要求完整SoC reboot。

## GSP Mailbox模式

- 首版从GSP entry开始，并在Vendor FW ready后继续使用`EHSM_DRV_MODE_WAIT_AND_POLL`；当前详设和发布不依赖interrupt。
- Vendor FW ready后的interrupt仅作为后续可选优化，必须通过独立设计变更重新启用，不得由业务层自行切换。
- 未来若启用，切换前必须完成中断控制器、16路IRQ/channel、ISR/callback、锁和task唤醒初始化，并确认没有未闭环poll transaction、late response或将被复用的context。
- 未来若启用，按Vendor初始化/清除顺序处理pending note和interrupt后再发布interrupt mode；任何业务调用只能看到一个已提交mode。初始化失败时，只有在channel/context/pending状态仍一致时才允许保持poll；无法证明时fail-close，不能静默混用两种模式。

# Open questions

- ADR-0028已固定GSP entry和880+128 KiB容量；OPEN-CONFLICT-006只继续管理PMA/Firewall及最终map/watermark Evidence。
- 镜像集合已冻结为PMP/RMP/MMP/eHSM FW；仍需各自package source、最大尺寸、rollback counter、load/entry、显式依赖、Owner和release接口。
- `security_service_task`的最终RTOS priority数值、stack峰值、queue深度，以及哪些早期硬件API必须在scheduler启动前完成。
- eHSM Vendor FW package来源、最大尺寸、签名/版本Owner、加载地址/API和失败恢复；加载Owner已冻结为GSP。
- Counter路径已冻结：FMC初始化调用eHSM BL新增staged-candidate exact-match API；GSP启动type 0 eHSM FW使用独立counter域。
- Vendor direct aperture/status/error在SRC-0022中的准确宏及GSP可访问视图。
- GSP各operation最终`timeout_us`、service channel ID、I/O descriptor最大数和arena总大小。
- Measurement producer/consumer、ABI、commit、16 KiB物理Region和State后不可变行为已冻结；产品实际最大实例数及物理PMA/Firewall仍需输入。
- SPDM/MCTP transport、完整虚拟链上限、OID/Profile和启动时点；DICE单层Stage/Key/Cert方向已冻结，eHSM UDS KDF/KeyGen准确command与wire发布仍为`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
- 面向其他内部模块的通用算法service caller ACL、quota和API；Host路由禁止。
- Runtime验证失败时GSP只阻断release并上报哪些RAS事件；局部隔离、reset、recovery或受限服务由RAS/产品策略决定。

# Implementation impact

- 当前 `solutions/gsp/app/src/main.c` 需要 production init orchestration；QEMU test 宏和 hello loop 保持 test/demo 边界。
- `components/security/sub.mk` 需要 production/test source split，防止 eHSM/cert/crypto/sign stub 进入发布 target。
- measurement store 需要版本化共享 ABI 或受控复制，而不是每个固件独立清零的 BSS 数组。

# Verification impact

- Scheduler 前门禁、handoff 篡改、Runtime 依赖顺序和下游不可达。
- production link map 无 stub；QEMU test data 无法进入 production path。
- Measurement并发/重启/slot overwrite，SPDM输出必须对应真实启动digest；覆盖FMC/GSP hash变化、相同TCB稳定Alias、动态Leaf扩展、CDI导出拒绝、12 KiB峰值和reset重建。
- eHSM FW运行期更新只写inactive slot；不得复用BL type 0命令，reset后由BL最终验证/启动。
- EMU 覆盖真实 eHSM、Runtime load/release、firewall lock 和 error propagation。

# References

- SRC-0016/SRC-0017 GSP、运行态验证和认证相关章节。
- [安全启动](../03-architecture/secure-boot.md)。
- [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。
- [GSP 地址域冲突](../../sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md)。
- [安全RAM布局](../03-architecture/security-ram-layout.md)。
- [Measurement Table接口](../04-interfaces/measurement-table.md)。
- [OPEN-CONFLICT-006](../../sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md)。
- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)。
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)。
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)。
- [ADR-0013](../../decisions/ADR-0013-gsp-operation-image-service-and-isolation-profile.md)。
- [eHSM产品Operation Profile](../04-interfaces/ehsm-product-operation-profile.md)。
- [eHSM Host Adapter详细合同](../04-interfaces/ehsm-host-adapter.md)。
- [安全固件包与Native Header Overlay](../04-interfaces/secure-firmware-package.md)。
- [镜像验签解密、Loader与Release接口](../04-interfaces/image-verify-loader.md)。
- [安全公共ABI与Registry](../04-interfaces/security-common-abi.md)。
- [BootROM/FMC/GSP函数级状态机](boot-stage-state-machines.md)。
- [ADR-0014](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)。
- [ADR-0015](../../decisions/ADR-0015-gsp-lifetime-ehsm-service-owner.md)。
- [ADR-0016](../../decisions/ADR-0016-gsp-replaces-omp-product-image.md)。
- [CE-SEC-009](../../evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md)。
- [OPEN-CONFLICT-008](../../sources/conflict-reports/CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS.md)。
- [CE-SEC-011](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)。
- [OPEN-CONFLICT-009](../../sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md)。

# Change history

- 2026-08-21：按ADR-0030删除Runtime Manifest parser；PMP/RMP/MMP/Die1改由typed-stage registry固定身份/Profile/目标，包仅提供已认证Header `load_addr`和`Code_Size`。
- 2026-08-19：按SRC-0032/ADR-0029增加GSP-DES-009；BootROM/FMC只贡献FMC/GSP Hash，GSP完成slot13 UDS不导出派生、一级动态Firmware Alias证书、Alias Report签名和12 KiB峰值门禁。
- 2026-07-27：接受ADR-0019；GSP消费FMC已提交rollback counter，不依赖FW启动副作用；关闭Host服务开放项。
- 2026-07-24：接入CE-SEC-011；确认GSP启动eHSM FW会触发BL暂存SOC epoch的OTP提交，与FMC release前提交门禁冲突，GSP-DES-003A转由OPEN-CONFLICT-009阻断编码。
- 2026-07-24：负责人决定Counter细节延期；GSP加载Owner不重开，详设默认FMC已有16字节Counter接口，具体Vendor绑定移至实现/EMU前。
- 2026-07-24：接受ADR-0016；GSP成为旧OMP/Q&P的唯一产品替代，OMP不再作为独立package、RAM、Measurement、counter或release对象，历史OMP代码仅作为GSP迁移输入。
- 2026-07-24：接受ADR-0015并关闭OPEN-DESIGN-009；冻结第一个最高优先级`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner，其他task只走typed queue，quarantine不通过Owner移交恢复。OPEN-DESIGN-010等待Runtime资料。
- 2026-07-23：接受ADR-0014并接入公共ABI/函数级状态机；提出从bootstrap到runtime持续使用一个`security_service_task`作为唯一eHSM owner，并登记OPEN-DESIGN-009/010管理Owner确认及PMP/RMP/MMP依赖/release输入。
- 2026-07-23：接入CE-SEC-009和包/verify/loader候选合同；eHSM FW固定走Vendor type 0、`boot=true/image_out=NULL`专用流程，PMP/RMP/MMP走SoC package+NGU Manifest和独立loader/release门禁。
- 2026-07-23：冻结GSP接管Measurement门禁；当时草案使用current generation/epoch术语。ADR-0019/0022最终改为当前启动已reset且完整性有效的稳定Table、`rollback_counter[16]`和唯一committed GSP条目，不再保留generation字段。
- 2026-07-23：接受ADR-0013；冻结PMP/RMP/MMP/eHSM FW镜像集合、签名/RNG/Hash等方案必需能力、通用算法低优先级内部开放、GSP发起/eHSM LCS最终裁决、Host API待定和单Runtime局部隔离。
- 2026-07-23：接受ADR-0012；eHSM Vendor FW由GSP阶段负责定位、验证、加载和ready门禁，FMC不加载；依赖FW的Runtime/SPDM/运行期服务必须等待FW成功。
- 2026-07-23：接受ADR-0011；GSP首版冻结唯一service owner、一个256字节静态context slot和单在途队列；零自动retry，timeout/BUSY异常后service全局quarantine。
- 2026-07-23：接受ADR-0010；GSP首版全程poll，Vendor FW ready后的interrupt仅为后续独立优化；Vendor公共代码和direct布局保持不变。
- 2026-07-23：按ADR-0008取消eHSM窄Mailbox窗口；建议将GSP eHSM packet/cmd/rsp/context放入固定data/BSS pool，不使用普通任务栈承载异步/流式context。
- 2026-07-22：按ADR-0007将SPDM/MCTP和相关stack/heap并入`SEC_RAM_GSP_RUNTIME`，Measurement/Mailbox与其连续；不再保留独立协议物理Region。
- 2026-07-22：接入ADR-0006；GSP执行和安全buffer改用2 MiB Region合同，旧080x绝对地址受OPEN-CONFLICT-006阻断；reset动作归RAS。
- 2026-07-21：依据 CE-SEC-002 填入 GSP FreeRTOS/hello-world/QEMU/stub 现状和首轮 production init/Runtime release 设计骨架。
- 2026-07-28：冻结eHSM FW type 0独立counter域与SoC stage type 1全局counter域（type 2/3拒绝）、Runtime `check_version=0`、Measurement不可变和运行期eHSM FW只存inactive/reset后BL验证。
