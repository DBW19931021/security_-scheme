---
title: "安全启动"
status: draft
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0023
  - SRC-0033
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

定义 NGU800P 从 SoC reset 到 Runtime 的安全启动边界、信任传递、release 条件和失败责任。CE-SEC-001/002 已完成 BootROM/eHSM/FMC/GSP 首轮代码现状调查。

# Scope

参与方包括 SoC reset/ROM、BootROM、C908/eHSM、FMC、GSP 和 Runtime。Host/BMC/OOB 不是正常启动链中的隐式信任源；Recovery 另见 `recovery.md`。

# Confirmed facts

- 当前 BootROM 默认链没有 eHSM 启动、FMC verify/load/release 或 jump，返回后进入启动文件死循环。
- 静态RAM包和`ehsm_stub`只允许存在于独立demo/test构建；production构建固定排除该入口，不提供运行期开关，也不得把demo结果作为安全启动成功证据。
- FMC 默认是裸机 hello-world，GSP 默认是 FreeRTOS hello-world；二者都没有调用 `verify_image()`、loader、release 或 jump。
- 当前唯一 verify path 使用 `ehsm_stub`，Host/QEMU 测试也使用 synthetic package/measurement。
- GSP package与linker历史上使用过不同address view；最终裁决为C908、Header Overlay、loader、linker和eHSM共享descriptor统一使用baremetal System Address，不建立Local/System映射。旧`0x1010_0808_0000`及local/remap输入均不得作为新D0地址。
- CE-SEC-004确认当前地址头存在local `0x1000_0500_0000`起始2 MiB和system/NoC `0x1010_0500_0000`视图，但当前BootROM/FMC/GSP及PMP/RMP/OMP linker与新目标冲突。
- 当前公司路径不能证明 BootROM→eHSM→FMC→GSP→Runtime 的生产信任链已经实现。
- 详细代码证据见 [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md) 和 [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。

# Documented facts

- SRC-0017 提供系统级硬件、架构和原则；SRC-0016 将其落到软件安全启动流程。
- 任何 Vendor 建议只有进入有效软件方案或受控 amendment 后，才能成为 SoC 启动行为。
- 方案、Vendor、公司代码或测试明显冲突时必须保持 `CONFLICTING` 并提交裁决。
- SRC-0023的`secureboot.001～007`是BootROM高优先级设计Checklist，已在主详设第6章逐项展开。负责人指定截图为Strap产品权威：`boot_pin.secure_boot[3]`、default 0、0非安全/1安全；在`non_sec_boot=0`时USER强制安全。当前SRC-0022生成头冲突，真实Strap寄存器绑定等待RTL同步。ADR-0024进一步规定值0时LCS读取失败、非法、`UNDEFINED`、来源无效或组合未批准进入受限非安全路径，且不写Measurement Entry/SoC State或启动审计。SoC LCS读取不依赖eHSM Autoload。
- SRC-0034/ADR-0031在该矩阵前增加1 bit `non_sec_boot` eFuse最高优先级覆盖：可靠值1时包括USER在内均进入`RESTRICTED_NONSECURE`，值0继续原矩阵；读取/ECC/镜像/来源异常进入启动策略输入错误终态。BootROM只消费复位锁存的只读逻辑快照，不获得raw eFuse读写能力。
- ADR-0034规定BootROM仍只有安全/非安全两个顶层分支；值0、DEV/MANU、Strap=0且Profile有效时选择`MANUFACTURING_PROVISIONING`并加载独立Provisioning FW，其他非安全原因选择`RESTRICTED_NONSECURE`。BootROM不安装Key；制造FW只能调用eHSM BL typed接口。

> 主详设反向链接：[《NGU800P安全软件详细设计》第2章“端到端安全启动链”](../05-software-design/NGU800P安全软件详细设计.md#第2章-系统角色信任边界与端到端安全启动链)

# Vendor implementation observations

- Vendor Host/BL 可提供 eHSM command、Mailbox、自检和状态能力参考；BootROM/FMC/GSP 以移植/复用 OSR Host 通用业务代码为主。NGU800P Mailbox机制follow Vendor，但Vendor reset实现不进入production产品路径。
- Vendor OSR platform demo 不是 NGU800P SoC 启动实现；其中无 timeout ready 等待、固定地址/timer/reset 等平台内容必须由 NGU800P port 替代。自检位图已裁决采用 Bootloader 定义，Host 定义错误。

# Assumptions

当前不假定各Region最终offset/容量、operation deadline、`max_fw_entries`、Firewall窗口或RAS event/action数值。已经冻结的是：2 MiB范围及baremetal System Address基址`0x1010_0500_0000`、不建立Local/System映射、三套必实现算法Profile、首版poll/单在途/零retry、Measurement逻辑ABI、Mailbox follow Vendor以及reset动作由RAS决定。下述sequence按主详设执行；缺失的平台数值只阻断真实绑定，不重开已冻结方向。

# Proposed design

## 信任传递

| 阶段 | 可信输入 | 必须完成的验证/状态 | 允许的输出 | release 条件 |
|---|---|---|---|---|
| BootROM | ROM/SoC reset state、只读`non_sec_boot`快照、批准的LCS/Strap/eHSM接口、FMC package | 先按`non_sec_boot`/LCS/Strap输出`mode/subprofile/reason`；安全路径判断eHSM ready/error、自检raw结果、验证固定type 1 FMC、Header policy/load/Measurement | 对应非安全子Profile镜像，或安全路径中的FMC plaintext、candidate和已commit Measurement | 非安全分支只按锁定Profile release且禁止fallback；安全分支所有门禁成功后release |
| FMC | BootROM committed FMC Measurement条目、BL RAM candidate、批准的GSP package/source | 初始化回传expected candidate；BL exact-match后compare/update/readback；随后以`check_version=0`验证/解密GSP并要求等于已提交值；commit GSP Measurement；建立保护 | counter proof、GSP Measurement记录和既有release接口 | rollback counter为`PROVEN_EQUAL/PROVEN_UPDATED`且GSP Measurement可证明；地址按ADR-0004执行 |
| GSP | Measurement Table、静态Issuer前缀、eHSM FW/PMP/RMP/MMP packages | 接管可信measurement；以FMC/GSP Hash和UDS派生一级DICE Alias Key并生成动态Leaf；加载eHSM FW并分别验证/release Runtime；设置证明和安全服务 | 动态Leaf/Report、每镜像独立Measurement/状态和release接口 | DICE/证书门禁及当前目标镜像依赖、保护配置成功 |
| Runtime | 当前软件方案规定的输入 | 只消费批准字段并遵守上游release结果 | 正常服务 | 不得绕过上游失败 |

## 正常序列骨架

```text
SoC reset
  -> prove FMC reset/clock/NX and Measurement write owner
  -> invalidate Measurement Header, then clear/readback the whole Region
  -> BootROM minimum init
  -> read reset-latched read-only non_sec_boot policy fuse snapshot
  -> if asserted: restricted non-secure, no eHSM wait/Measurement Entry/SoC State/audit
  -> if deasserted: read SoC LCS and, when required, latched secure_boot Strap
  -> select boot mode (LCS anomaly -> restricted non-secure)
  -> policy fuse invalid/ECC/mirror error: terminal, release neither path
  -> eHSM ready + raw status; eHSM self-test is eFuse-driven and autonomous
  -> eHSM self-test + raw bitmap
  -> locate/seal/preflight FMC package
  -> real eHSM verify/decrypt (FMC type 1, check_version=0)
  -> authenticated Header Overlay + typed-stage policy
  -> confirm BL staged the authenticated 16-byte FMC candidate
  -> loader source digest + copy + target readback digest + W^X/cache/Firewall
  -> commit real FMC Measurement
  -> one-time release FMC
  -> FMC init returns expected candidate to BL
  -> BL exact-match + monotonic compare/update/readback
  -> FMC verifies GSP with check_version=0, requires committed counter equality, then releases GSP
  -> GSP loads eHSM Vendor FW
  -> GSP independently verifies/releases PMP, RMP and MMP
```

## 统一安全不变量

1. 上游 stage 未成功时，下游 entry 不可达。
2. EMU/产品 target 不得存在 demo、stub、simulated success、test key/cert/provider、未批准hardcode或silent fallback；能力未完成时不可达、禁用或fail-close。
3. 每一级只消费当前软件方案批准的Measurement Table、Header Overlay/typed-stage loader和错误/release信息；外部可写内存中的裸指针/长度必须重新验证。当前不新增版本化Handoff。
4. image type、source、load、entry、长度、对齐、重叠和地址溢出均需 allowlist 检查。
5. timeout/busy后不得默认command未执行；不可逆操作需要查询协议并上报RAS。安全stage不得直接reset，实际reset/watchdog/隔离由RAS策略决定。
6. 原始 eHSM/SoC 状态和错误必须保留，映射后仍可追溯。
7. 敏感明文、key material、challenge 和临时 command buffer 的 owner/清零时点必须唯一。
8. Vendor原生密码验证、NGU Header Overlay和typed-stage policy是三层合同：Vendor PASS后才把Overlay视为已认证，Overlay/受信策略通过后才进入loader；FMC candidate暂存、loader和Measurement成功后BootROM才允许release FMC，FMC初始化counter proof及GSP后续门禁成功后才允许release GSP。
9. eHSM Vendor FW使用独立counter域的Vendor type 0专用包并由GSP以Vendor boot流程加载，且不解释NGU Overlay；FMC/GSP/PMP/RMP/MMP及Die1 SoC stage包全部固定Vendor type 1并使用`check_version=0`，产品入口拒绝type 2/3；项目类型只存在于受信typed-stage registry，不覆盖Vendor wire枚举。
10. BootROM先读取一次只读`non_sec_boot`快照：可靠值1最高优先级进入`RESTRICTED_NONSECURE`，值0再按Lifecycle分类，输入异常阻断两条release。值0时USER强制安全，DEV/MANU+Strap0选择`MANUFACTURING_PROVISIONING`，其他非安全原因选择`RESTRICTED_NONSECURE`；两个子Profile不创建安全Measurement/SoC State/启动审计且禁止fallback。实现只使用受控生成接口。

这些不变量仍需与 SRC-0016/SRC-0017 逐条复核并经负责人评审，之后才能提升状态。

# Open questions

- BootROM、FMC、GSP和Runtime的正式Owner/评审人，以及Measurement Table/loader/release接口Owner。
- PMP/RMP/MMP/eHSM FW各自package source、最大尺寸、依赖和release接口；镜像集合和单Runtime局部隔离原则已冻结。
- 各stage package source和保护切换时点；FMC/GSP/PMP/RMP的2 MiB System Address与容量已由ADR-0028冻结，MMP DDR地址和release Profile仍等待OPEN-DESIGN-010。
- OPEN-DESIGN-012产品矩阵已由ADR-0018/0024/0031/0034关闭；OPEN-DESIGN-024等待`non_sec_boot`准确word/bit/编码、ECC/valid/镜像、只读视图、复位锁存、DEV/MANU烧写Owner和USER独立维修入口，关闭前保持`BLOCKED_BY_NON_SEC_BOOT_BINDING`；OPEN-CONFLICT-010保持`BLOCKED_BY_RTL_SYNC`。
- eHSM ready/error和自检消费规则已经冻结：eHSM按eFuse自主执行，BootROM不发起；剩余为真实MMIO、operation deadline及RAS上报原语。
- Measurement逻辑ABI和顺序已由ADR-0022冻结为真实load后构造、最后commit、commit成功后release；每次启动先失效Header并清零整个16 KiB固定Region，不使用generation。16 KiB物理上最多容纳126个Firmware Entry，OPEN-DESIGN-021仍需冻结产品实际实例上限和SPDM最大block数。
- C908 PC/linker、Header Overlay、loader和eHSM共享descriptor已统一使用baremetal System Address，不建立Local/System映射；ADR-0028已冻结2 MiB精确Region，PMP/RMP各256 KiB，MMP改驻DDR，FMC退出后128 KiB可受控回收给GSP。ADR-0016已确认GSP替代OMP/Q&P。PMA、Firewall、MMP DDR和各镜像link map峰值仍需平台Evidence。

# Implementation impact

- BootROM 当前 hello-world/demo/stub 不能作为 production 路径；后续实现需增加policy-fuse只读平台port、纯模式策略分支和`BOOT_POLICY_INPUT_ERROR`终态，但不得增加raw eFuse驱动。
- FMC/GSP 当前 hello-world 和 QEMU test 入口也不能作为 production 启动链。
- 已批准建立跨BootROM/FMC/GSP的stage/error namespace、Measurement记录和可检查release gate；当前不创建Handoff ABI。
- EMU/产品 source/build 必须消除对 eHSM/crypto/cert/sign stub 和 simulated success 的依赖，不得把现有 test/stub flow 当作目标实现。

# Verification impact

- 每个 stage 需要独立成功/失败 oracle 和“下游不可达”观测点。
- 需要 chain test 验证错误传播、handoff 篡改、地址边界、重放/rollback、timeout/reset 和 measurement 顺序。
- 当前关联用例 038、041、049、051、053、055、056、063、070、093、094、095；v0.3 将用例093改为bit18=`TRNG`、bit19=unknown/reserved并高亮变更，同时补齐executable mapping。
- SRC-0023 `secureboot.001～007`必须在下一版测试工作簿分配正式Case ID并逐项映射；位号和模式矩阵已关闭，最终地址/MMIO等平台输入未关闭前相应用例保持`blocked/exploratory`。

# References

- SRC-0016 §2.1～§2.3。
- SRC-0017 系统边界、安全启动与隔离相关章节。
- SRC-0023 BootROM Secure Boot重点Checklist与Strap截图。
- SRC-0034 `non_sec_boot` eFuse强制非安全启动输入。
- [BootROM 软件设计](../05-software-design/bootrom.md)。
- [eHSM Mailbox 接口](../04-interfaces/ehsm-mailbox.md)。
- [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)。
- [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。
- [CE-SEC-004](../../evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md)。
- [安全RAM布局](security-ram-layout.md)。
- [Measurement Table接口](../04-interfaces/measurement-table.md)。
- [安全固件包与Native Header Overlay](../04-interfaces/secure-firmware-package.md)。
- [镜像验签解密、Loader与Release接口](../04-interfaces/image-verify-loader.md)。
- [CE-SEC-009](../../evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md)。
- [OPEN-CONFLICT-008](../../sources/conflict-reports/CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS.md)。
- [GSP 地址域冲突](../../sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md)。
- [OPEN-CONFLICT-006](../../sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md)。
- [OPEN-CONFLICT-010](../../sources/conflict-reports/CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY.md)。
- [《NGU800P安全软件详细设计》第6章](../05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)。
- [ADR-0024《Manifest精简、LCS异常启动策略与eHSM自检Owner》](../../decisions/ADR-0024-manifest-simplification-boot-mode-and-ehsm-self-test.md)。
- [ADR-0031《non_sec_boot eFuse强制受限非安全启动》](../../decisions/ADR-0031-non-sec-boot-efuse-override.md)。
- [ADR-0034《非安全制造灌装、eHSM BL密钥安装与USER最终提交》](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)。

# Change history

- 2026-09-02：按ADR-0034将非安全路径区分为DEV/MANU制造Provisioning和受限子Profile，保留两个顶层启动分支并禁止Profile fallback。
- 2026-08-25：接受SRC-0034/ADR-0031；增加`non_sec_boot`最高优先级覆盖、BootROM只读策略快照、输入异常终态和RTL/eFuse绑定门禁。
- 2026-07-29：负责人指定SRC-0023截图为`secure_boot`产品权威；更新字段/极性并将当前生成头差异标记为RTL同步阻断。
- 2026-07-24：接入SRC-0023，新增Lifecycle×Strap模式门禁和位号冲突；安全链重排为epoch compare先于loader、真实load后commit Measurement、最后一次性release。
- 2026-07-28：接受ADR-0024；LCS异常进入受限非安全且无Measurement/启动审计；eHSM自检由eFuse驱动自主执行，BootROM仅消费结果。
- 2026-07-23：当时接入Vendor原生包→authenticated Header→NGU Manifest/policy→loader→Measurement/counter→release分层；2026-07-28细化loader双摘要；2026-08-21 Manifest环节由Header Overlay/typed-stage policy取代。
- 2026-07-23：冻结BootROM committed FMC Measurement条目→FMC epoch→GSP epoch比较→counter update→GSP Measurement commit→release的启动链。
- 2026-07-23：接受ADR-0013；GSP镜像集合固定为eHSM FW/PMP/RMP/MMP，分别验证、记录和release；确定完成的单Runtime失败局部隔离，共享eHSM service故障仍全局quarantine。
- 2026-07-22：接受ADR-0006；安全启动接入2 MiB安全RAM目标、Mailbox follow Vendor和RAS reset边界；旧GSP绝对地址转入OPEN-CONFLICT-006。
- 2026-07-22：接受ADR-0004；Die1使用独立Measurement实例；2026-07-28地址合同更新为统一baremetal System Address且不建立Local/System映射；当前不采用Handoff。
- 2026-07-22：登记W0批准原则、自检位图裁决和OSR Host通用业务代码复用边界；现有test/stub不作为目标依据，版本化Handoff保持R1-05待评审。
- 2026-07-21：依据 CE-SEC-001 固化 BootROM 当前链、建立启动信任传递骨架和 fail-close 不变量；FMC/GSP 细节等待 INV-SEC-002。
- 2026-07-21：依据 CE-SEC-002 补齐 FMC/GSP hello-world/stub 现状、release 缺口和 OPEN-CONFLICT-003 地址域门禁。
- 2026-07-28：启动链改为BootROM `check_version=0`暂存FMC candidate、FMC初始化exact-match提交，再验证GSP；Measurement Entry使用`RELEASE_AUTHORIZED`语义。
