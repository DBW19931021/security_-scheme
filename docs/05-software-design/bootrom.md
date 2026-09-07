---
title: "BootROM 软件设计"
status: review
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0023
  - SRC-0032
  - SRC-0033
  - SRC-0034
  - SRC-0036
owners:
  - GSP
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

定义 NGU800P BootROM 在 eHSM/FMC 安全启动中的职责、状态机、失败边界和目标代码落点。本文区分调查时的 `CODE_FACT` 与待评审的 `PROPOSED` 设计，不把 demo/stub 当成产品行为。

# Scope

覆盖 Reset 到 FMC release：BootROM初始化、模式判定、eHSM ready/error、FMC真实验签/解密、Native Header Overlay、counter candidate、load/entry、Measurement、失败终态和一次性跳转。按ADR-0029，BootROM对DICE只贡献FMC最终readback digest，不读取UDS、不生成CDI/Key/CSR/X.509；后续动态证书全部由GSP负责。

> 主详设反向链接：[《NGU800P安全软件详细设计》第6章“BootROM安全启动详细设计”](NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)

# Confirmed facts

以下为清单参考点 `master@08b29c7...` 对应调查时工作区的代码事实；工作区已有修改，且本次未用 Git 区分差异：

1. BootROM 默认入口是 `Reset_Handler -> pre_main -> main -> __exit`；`main()` 当前是 hello-world，没有 eHSM 或 FMC handoff。
2. `security` 组件和 `bootrom_secure_demo.c` 会参与构建，但 `SECURE_DEMO` 默认未定义。
3. 手工打开 demo 后只会构造静态测试包并调用 `ehsm_verify_decrypt_stub()`；成功结果通过 `memcpy()` 和模拟 measurement 位产生。
4. demo 的 FMC load/entry 常量没有用于实际加载或跳转。
5. 公司 BootROM 路径没有生产 ready/self-test/LCS/Mailbox/timeout/reset adapter，也没有 Vendor Host API 调用。

完整证据见 [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)。

# Documented facts

- SRC-0017 是系统/架构上位约束，SRC-0016 是软件落地基线。
- SRC-0016 §2.1～§2.3 描述 eHSM 启动和 BootROM/FMC/GSP 安全启动链；具体实现合同必须由本详设展开，并最终回到有效软件方案或受控 amendment。
- Vendor 只覆盖 eHSM/Core，不定义 SoC BootROM、Flash、reset domain、FMC 内存或 handoff 行为。
- SRC-0023给出负责人确定的`secureboot.001～007`高优先级Checklist；其中原Protected Manifest实现载体已由ADR-0030替换为Header Overlay与typed-stage policy，其余模式判定、eHSM Ready、真实验签/强制解密、回滚、Measurement、load/release和失败闭锁要求继续逐项覆盖。
- SRC-0023/ADR-0018已冻结值0时的顶层矩阵：`boot_pin.secure_boot[3]`、default 0、0非安全/1安全；USER忽略Strap并强制安全启动；已识别非USER状态中Strap=0进入非安全顶层、Strap=1进入安全启动。ADR-0024规定值0时LCS异常进入`RESTRICTED_NONSECURE`；ADR-0034进一步定义非安全子Profile。SoC LCS不依赖eHSM Autoload；当前Strap真实寄存器绑定等待RTL同步。
- SRC-0034/ADR-0031在该矩阵前增加最高优先级`non_sec_boot` eFuse覆盖：可靠值1时包括USER在内均强制进入`RESTRICTED_NONSECURE`，值0保持原矩阵；读取/ECC/镜像/来源异常阻断安全和非安全FMC release。BootROM只消费复位锁存的只读逻辑快照，不获得raw eFuse读写能力。
- ADR-0034固定非安全路径的两个子Profile：值0、DEV/MANU、Strap=0且制造Profile有效时选择`MANUFACTURING_PROVISIONING`并加载独立C908 Provisioning FW；值1、LCS异常和其他非制造组合选择`RESTRICTED_NONSECURE`。BootROM仍只有安全/非安全两个顶层分支，且自身不安装Key或写OTP。

# Vendor implementation observations

- SRC-0018 Host 提供 driver/port/Mailbox/context/self-test/status/LCS/reset 构件；BootROM/FMC/GSP 以移植/复用其中通用业务代码为主，改动收敛到NGU800P port、构建、项目ABI、错误和安全门禁。
- Vendor Host Demo和通用API已经提供`EHSM_DRV_MODE_WAIT_AND_POLL`；负责人确认BootROM使用poll，不引入BL/BootROM路径中不存在的interrupt方案。
- OSR m130 示例使用平台固定地址/magic；其 timeout hook 恒不超时，ready demo 也没有有界等待，而且表达式在任一done位出现时就退出，不能直接作为 NGU800P 生产实现。
- Vendor状态定义和BL TRM表明，BootROM放行条件为`hw_boot_done=1 && bootloader_done=1`且`hw_boot_err=0 && bootloader_err=0`；错误位优先于done位处理，所有raw status必须保存。
- BL/Host 自检 bit18/bit19 差异已裁决：采用Bootloader定义，Host错误；bit18/`0x40000`=`TRNG`，bit19/`0x80000`保持unknown/reserved，并始终保留raw bitmap。

# Assumptions

当前不把以下值作为设计常量：`non_sec_boot`物理word/bit/编码、ECC/valid/镜像和只读视图寄存器/API，Vendor direct eHSM aperture/status寄存器的NGU800P准确MMIO绑定、各operation timeout数值、Firewall参数、FMC Flash地址、FMC最终Region offset和RAS action映射。`non_sec_boot`逻辑语义和Strap模式矩阵已冻结，但代码仍只使用RTL生成命名接口。ADR-0010已冻结Vendor公共代码不修改、16个direct channel、BootROM poll、ready/error组合及RAS未ready终态；ADR-0011已冻结BootROM一个256字节静态context slot、单在途、零自动retry和timeout quarantine。C908、Header Overlay、loader和linker统一使用baremetal System Address；OPEN-CONFLICT-006只继续约束Region容量、PMA和Firewall。

# Proposed design

下表是第6章的专题摘要。受开放项影响的物理绑定仍为`PROPOSED/CONFLICTING`，已批准的安全链和失败门禁按主详设执行；本表不能单独触发编码：

| Design ID | BootROM 必须执行的动作 | 成功出口 | 失败出口/限制 |
|---|---|---|---|
| BR-DES-001 | 建立唯一production入口和静态context；production target不得启用demo或链接可达stub | 进入最小平台初始化 | 构建门禁失败 |
| BR-DES-002 | 从不可变reset vector进入后首先回读证明FMC reset/clock/NX及Measurement写Owner；清除Header magic/valid/commit并readback invalid，再清零、执行write/release fence并回读整个固定Measurement Region；两个批准PMA候选均不做data clean/invalidate；随后初始化最小SoC/timer基础 | 读取启动策略输入 | 任一reset/clock/Firewall/Measurement readback无法证明时FMC保持不可执行并进入统一终态 |
| BR-DES-003 | 先读取一次复位锁存、带valid/ECC/镜像状态的`non_sec_boot`只读快照：可靠值1输出`NON_SECURE_BOOT/RESTRICTED_NONSECURE`；可靠值0再读取SoC LCS和按需读取`boot_pin.secure_boot[3]`，USER强制secure，DEV/MANU+Strap0输出`MANUFACTURING_PROVISIONING`，其他非安全原因输出`RESTRICTED_NONSECURE` | `SECURE_BOOT`进入eHSM链；非安全路径按锁定子Profile加载独立镜像且不创建安全Measurement Entry/SoC State或启动审计 | policy-fuse输入异常阻断两条release；两个子Profile不得fallback；值1/LCS异常不可达制造接口；BootROM禁止raw eFuse和Key安装 |
| BR-DES-004 | 使用唯一service owner和一个64字节对齐、256字节静态context slot，以`EHSM_DRV_MODE_WAIT_AND_POLL`读取eHSM原始状态并有界等待；错误优先，只有`hw_boot_done && bootloader_done && !hw_boot_err && !bootloader_err`才放行 | 保存原始status/time，service保持READY | 任一error或timeout不得继续；timeout acceptance unknown且service quarantine，不自动retry |
| BR-DES-005 | eHSM按自身eFuse自主判断并执行或跳过自检；BootROM不发起，只读取Vendor发布的raw状态/bitmap；按Bootloader定义解释bit18=`TRNG`，bit19保持unknown/reserved | PASS或eFuse明确NOT_REQUIRED后定位FMC | 禁止BootROM发送START_SELF_TEST；raw bitmap、映射版本和未知位必须保留，不得按Host错误宏解释 |
| BR-DES-006 | 定位并seal FMC Vendor原生package，固定Vendor type 1，执行未认证preflight、`Code_Size == package_size-1024`及output容量覆盖完整package，再以`boot=0/check_version=0`调用真实eHSM BL verify/decrypt | Vendor PASS且BL已暂存candidate后进入认证Header | 不允许type 2 FMC、synthetic/零签名/明文伪密文或stub；timeout进入quarantine |
| BR-DES-007 | Preflight及PASS后稳定认证Header均对offset256～1015计算CRC-32/ISO-HDLC并与offset1016 LE32比较；解析offset1008 LE64 `load_addr`和offset1020零reserved，要求精确等于typed FMC目标；Profile/key/LCS来自受信matrix，`Code_Size`为唯一长度，`entry_addr=load_addr` | 获得可信load request和FMC rollback counter | CRC只作格式筛查，不替代Vendor验签；任一失败不得load/release；PASS前字段只能用于拒绝；旧格式、尾随对象和Host自由地址拒绝 |
| BR-DES-008 | 确认BL RAM candidate属于本次FMC事务，并与认证Header逐字节一致；BootROM不读/比/写stored counter | 通过后调用统一loader | candidate缺失/失效/不一致不得加载；BootROM stored read/compare/update必须不可达 |
| BR-DES-009 | loader按Profile对`target+1024`的`Code_Size`字节完成源/目标readback双摘要、range/W^X/Firewall与`fence.i`；以最终FMC digest创建并commit唯一`RELEASE_AUTHORIZED` Measurement条目，该digest是BootROM唯一DICE贡献 | commit/readback成功进入最终release gate | 不得构造CDI/Key/X.509；源/目标摘要不等、从包内元数据照抄digest或Entry声称FMC已运行均拒绝 |
| BR-DES-010 | 复验所有门禁和一次性标志，以批准的privilege/interrupt/cache状态跳转实际FMC entry | 控制权转交FMC | jump返回或重复release视为FATAL |

候选状态机：

```text
RESET
  -> ESTABLISH_RESET_BASELINE
  -> INVALIDATE_AND_CLEAR_MEASUREMENT
  -> MIN_INIT
  -> READ_BOOT_POLICY_INPUTS
  -> SELECT_BOOT_MODE
       non_sec_boot=1 -> NONSECURE_LOCATE(RESTRICTED) -> NONSECURE_LOAD -> NONSECURE_RELEASE
       non_sec_boot=0 + DEV/MANU + Strap=0 -> NONSECURE_LOCATE(MANUFACTURING) -> NONSECURE_LOAD -> NONSECURE_RELEASE
       non_sec_boot=0 + secure -> EHSM_WAIT_READY
       policy-fuse invalid -> BOOT_POLICY_INPUT_ERROR -> APPROVED_TERMINAL_STATE
  -> READ_EHSM_BOOT_RESULT
  -> LOCATE_FMC
  -> PREFLIGHT_FMC
  -> VERIFY_AND_DECRYPT
  -> AUTH_HEADER_OVERLAY_AND_TYPED_POLICY
  -> CONFIRM_STAGED_FMC_CANDIDATE
  -> LOAD_FMC
  -> COMMIT_FMC_MEASUREMENT
  -> FINAL_RELEASE_CHECK
  -> RELEASE_FMC

任一阶段失败 -> CAPTURE_RAW_ERROR -> APPROVED_TERMINAL_STATE
```

`APPROVED_TERMINAL_STATE`在BootROM侧保存raw错误、撤销权限、尽力清零、上报RAS并保持fail-close；任何失败路径都不可到达`HANDOFF_FMC`。RAS尚未ready时按ADR-0010写入静态`EARLY_SECURITY_ERROR_RECORD`，在有限deadline内只重试上报；仍不可用则记录`RAS_REPORT_UNAVAILABLE`，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出。BootROM不自行执行watchdog/eHSM/system reset。

# Open questions

1. OPEN-DESIGN-024保持开放：`non_sec_boot`准确word/bit/编码、ECC/valid/镜像、复位锁存、只读视图、烧写/锁定Owner和Lifecycle权限是什么？关闭前产品port为`BLOCKED_BY_NON_SEC_BOOT_BINDING`。
2. OPEN-DESIGN-012产品矩阵已关闭；OPEN-CONFLICT-010保持`BLOCKED_BY_RTL_SYNC`，等待与SRC-0023一致的RTL生成命名宏或批准映射后才能做真实寄存器绑定和EMU Expected。
3. Vendor direct eHSM aperture和status/error寄存器在SRC-0022中的准确宏/地址是什么？transport方向已关闭，4 KiB通用Mailbox不得使用。
4. ready/self-test/verify等operation的最终`timeout_us`、RAS report/ready通道和early report deadline是什么？失败终态已固定为关闭普通中断后的无限WFI循环，首版自动retry为0，timeout后不复用slot。
5. FMC包位置、最大长度、load/entry allowlist、目标privilege及既有启动接口参数是什么？
6. `SEC_RAM_BOOTROM`实际data/BSS/stack峰值是否满足启动复用预算，warm reset时由谁清零和重新配置Firewall？

# Implementation impact

- 当前 `solutions/bootrom/app/src/main.c` 只能作为入口骨架，不能在 production 保持 hello-world/可选 demo 结构。
- `bootrom_secure_demo.c` 与 `ehsm_stub.c` 仅作为历史 `CODE_FACT`；EMU/产品实现不得调用或包含它们，也不得以现有test/demo流程和Expected作为目标。构建需对stub/simulated success/test provider做负向检查。
- 预计新增/重构BootROM boot-flow、Boot Policy Fuse只读port、输出`mode/subprofile/reason`的纯策略、两个非安全stage profile、eHSM port、FMC source/loader、Measurement记录和error terminal模块；明确不链接raw eFuse写驱动、Key安装、X.509 writer、DICE KDF、Alias KeyGen或证书签名模块，不新增Handoff。
- `security_status_t` 需要统一映射 transport/eHSM/boot-stage/raw-code，设计在 `error-handling.md` 完成。
- 当前Makefile主构建通过`chip_riscv_bootrom`纳入C908 common的cache/timer/IRQ实现；CDK工程仍引用不存在的`chip_riscv_dummy`，编码前必须重新生成/修正并执行source-list一致性检查。

# Verification impact

- Host/unit：状态机阶段、每个错误注入点、load/entry range、overflow/alignment；其Expected从批准方案和最新工作簿派生，不从历史stub流程派生。
- Build：qemu/emu/fpga/evb/prod 的 source/macro/link map 断言，确认EMU/产品无 `ehsm_verify_decrypt_stub`、simulated success或test provider可达符号。
- EMU：逐项覆盖SRC-0023 `secureboot.001～007`以及SRC-0034/ADR-0034，包括`non_sec_boot`值0兼容、值1覆盖USER/全部Strap、读取/ECC/镜像异常、复位重锁存、DEV/MANU+Strap0制造Profile、其他非安全原因受限Profile、Profile错配/缺失/越权/无fallback、值1不能制造、LCS异常无安全审计、ready timeout、真实verify/decrypt、rollback、Header Overlay/typed-stage policy、Measurement和受控release。
- 当前关联工作簿用例 038、051、093、094、095；v0.2 本轮不修改。

# References

- SRC-0016 §2.1～§2.3。
- SRC-0017 系统边界和安全启动相关章节。
- SRC-0018 Vendor Host/BL 快照。
- SRC-0023 BootROM Secure Boot重点Checklist与Strap截图。
- SRC-0034 `non_sec_boot` eFuse强制非安全启动输入。
- [eHSM Mailbox 接口](../04-interfaces/ehsm-mailbox.md)。
- [安全启动架构](../03-architecture/secure-boot.md)。
- [安全RAM布局](../03-architecture/security-ram-layout.md)。
- [ADR-0006](../../decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md)。
- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)。
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)。
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)。
- [Measurement Table接口](../04-interfaces/measurement-table.md)。
- [安全固件包与Native Header Overlay](../04-interfaces/secure-firmware-package.md)。
- [镜像验签解密、Loader与Release接口](../04-interfaces/image-verify-loader.md)。
- [安全公共ABI与Registry](../04-interfaces/security-common-abi.md)。
- [BootROM/FMC/GSP函数级状态机](boot-stage-state-machines.md)。
- [ADR-0014](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)。
- [ADR-0024《Manifest精简、LCS异常启动策略与eHSM自检Owner》](../../decisions/ADR-0024-manifest-simplification-boot-mode-and-ehsm-self-test.md)。
- [ADR-0031《non_sec_boot eFuse强制受限非安全启动》](../../decisions/ADR-0031-non-sec-boot-efuse-override.md)。
- [ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)。
- [ADR-0034《非安全制造灌装、eHSM BL密钥安装与USER最终提交》](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)。
- [eHSM Host Adapter详细合同](../04-interfaces/ehsm-host-adapter.md)。
- [CE-SEC-009](../../evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md)。
- [OPEN-CONFLICT-008](../../sources/conflict-reports/CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS.md)。
- [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)。
- [CE-SEC-007](../../evidence/code-investigations/CE-SEC-007-ngu800p-ehsm-port-binding.md)。
- [CE-SEC-012](../../evidence/code-investigations/CE-SEC-012-bootrom-secure-mode-strap-checklist.md)。
- [OPEN-CONFLICT-010](../../sources/conflict-reports/CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY.md)。
- [通用Mailbox与eHSM绑定冲突](../../sources/conflict-reports/CONFLICT-NGU800P-GENERIC-VS-EHSM-MAILBOX-MMIO.md)。
- `sources/conflict-reports/CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP.md`。

# Change history

- 2026-09-02：按ADR-0034在既有非安全顶层分支内区分`MANUFACTURING_PROVISIONING`与`RESTRICTED_NONSECURE`；只有值0、DEV/MANU、Strap=0允许加载制造FW，BootROM仍不写OTP或安装Key。
- 2026-09-01：按SRC-0036/ADR-0033把offset1016定义为LE32 Header CRC32、offset1020定义为4B零reserved，并加入提交eHSM前和Vendor PASS后的双阶段CRC校验；CRC不替代验签。
- 2026-08-25：按SRC-0034/ADR-0031增加`non_sec_boot`最高优先级eFuse覆盖、BootROM只读快照、输入异常终态和`BLOCKED_BY_NON_SEC_BOOT_BINDING`门禁。
- 2026-08-21：按ADR-0030删除Manifest流程；BootROM改为offset1008/1016 Header Overlay、typed FMC固定目标、`target+1024`/`Code_Size`加载和`entry_addr=load_addr`合同。
- 2026-08-19：按SRC-0032/ADR-0029明确BootROM只提交FMC最终readback Hash作为DICE贡献，不读取UDS、不生成CDI/Key/X.509，ROM不增加证书软件栈。
- 2026-07-29：负责人指定以SRC-0023截图为准；更新为`boot_pin.secure_boot[3]`、default 0、0非安全/1安全。当前SRC-0022生成头冲突，真实绑定标记`BLOCKED_BY_RTL_SYNC`。
- 2026-07-28：最终Counter时序改为FMC固定type 1、BootROM `check_version=0`并由BL暂存认证candidate；BootROM不读取/比较/写stored counter，candidate经完整后置校验后进入FMC Measurement。
- 2026-07-28：接受ADR-0024；Manifest改为固定128B无TLV格式；LCS异常进入受限非安全且不写Measurement/启动审计；eHSM按eFuse自主自检，BootROM不再发起自检。
- 2026-07-24：当时接受ADR-0018并采用RTL生成头`SEC_BOOT` bit0、非USER 0安全/1非安全；该位号/极性和OPEN-CONFLICT-010关闭状态已被2026-07-29裁决替代。LCS独立读取继续有效；USER强制安全自2026-08-25起仅在`non_sec_boot=0`时适用。
- 2026-07-24：接入SRC-0023 `secureboot.001～007`并同步主详设第6章；新增Lifecycle×Strap模式判定、OPEN-CONFLICT-010位号冲突、OPEN-DESIGN-012非USER策略，重排为epoch compare先于loader、实际load后提交Measurement、最后一次性release。
- 2026-07-23：接受ADR-0014并接入公共ABI/函数级状态机；冻结双阶段精确长度和Manifest v1；当时的首轮顺序已由2026-07-24第6章细化为模式判定在前、epoch compare先于loader。
- 2026-07-23：接入CE-SEC-009和包/verify/loader候选合同；其Manifest expected digest步骤已由2026-07-28无expected digest/loader双摘要裁决替代。
- 2026-07-23：批准Measurement→FMC epoch路径；BootROM必须在跳转前提交唯一FMC条目及16字节epoch，commit失败不得release。
- 2026-07-23：接受ADR-0012的原compare-only时序；该Counter时序已由2026-07-28裁决取代。BootROM直接读取SoC LCS以及ready成功为done=1且error=0的规则保留。
- 2026-07-23：接受ADR-0011；BootROM冻结一个256字节静态context slot、单在途、零自动retry，timeout后service/slot quarantine并执行早期fail-stop。
- 2026-07-23：接受ADR-0010；Vendor direct Mailbox且Vendor公共代码不修改，BootROM固定poll；RAS未ready终态已进一步固定为静态错误记录、有限上报等待和无限WFI fail-stop循环。
- 2026-07-23：确认BootROM可复用C908 64位微秒时钟和cache/barrier原语；登记OPEN-CONFLICT-007，禁止把4 KiB通用Mailbox误作Vendor eHSM Mailbox；记录CDK dummy路径构建元数据缺口。
- 2026-07-23：依据Vendor手册/代码与负责人输入冻结BootROM使用poll、16个Mailbox channel及ready/error门禁；移除相应开放问题，保留NGU MMIO绑定、deadline和RAS项目策略。
- 2026-07-22：接入ADR-0006安全RAM、Mailbox和RAS边界；BootROM失败只上报/阻断，不直接reset；Region大小和linker等待OPEN-CONFLICT-006。
- 2026-07-22：接受ADR-0004并决定不采用Handoff；当时的Manifest/loader载体已于2026-08-21替换为Header Overlay/typed-stage loader，BootROM跨阶段输出仍收敛到Measurement Table和既有FMC启动接口。
- 2026-07-22：登记自检位图采用Bootloader定义、OSR Host通用业务代码优先复用、现有test/stub非目标依据；R1-05未批准前将Handoff保持为候选。
- 2026-07-21：依据 CE-SEC-001 填入默认启动链、demo/stub 边界、生产缺口和首轮 BootROM 候选状态机；未修改或验证公司代码。
- 2026-07-28：FMC固定Vendor type 1、`check_version=0`，BootROM只确认BL candidate暂存并删除stored counter读/比较职责。
