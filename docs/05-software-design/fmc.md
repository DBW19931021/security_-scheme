---
title: "FMC 软件设计"
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
  - SRC-0036
owners:
  - GSP
last_reviewed: 2026-09-01
supersedes: []
superseded_by: []
---

# Purpose

定义FMC按当前软件方案接收BootROM启动结果、验证并释放GSP的职责、状态机、地址/缓冲区边界和失败出口。本文为首轮`PROPOSED`设计，当前代码现状来自CE-SEC-002。

> 主详设反向链接：[《NGU800P安全软件详细设计》第7章“FMC详细设计”](NGU800P安全软件详细设计.md#第7章-fmc详细设计)

# Scope

覆盖FMC entry到GSP release：消费BootROM Measurement、验证/加载GSP、counter、Native Header Overlay、保护切换、Measurement和jump。按ADR-0029，FMC对DICE只贡献GSP最终readback digest，不读取UDS、不生成CDI/Key/CSR/X.509。

# Confirmed facts

1. FMC目标当前由linker固定为`0x1000_080C_0000/0x40000`；这是`CODE_FACT`，与ADR-0006新2 MiB目标冲突，不是新D0批准地址。
2. FMC solution 只编译 hello-world `main.c`，没有 verify、loader、measurement、GSP release 或 jump。
3. `security` 组件当前 verify path 使用 `ehsm_stub`；没有 FMC production caller。
4. GSP packager/tests和GSP linker历史上使用过两种address view；最终裁决为C908、Header Overlay、loader和linker统一使用baremetal System Address，不进行local/remap转换。旧`0x1010_0808_0000`不是新D0地址。

完整证据见 [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。

# Documented facts

- SRC-0016 §2.1～§2.3 将 FMC 定位在 BootROM 与 GSP 之间的软件启动阶段。
- SRC-0017 的系统边界/隔离原则是 FMC 资源权限和 release 的上位依据。
- Vendor 只提供 eHSM/Core 能力，不定义 FMC 的 Flash、SRAM、address view 或 GSP jump。

# Vendor implementation observations

FMC复用DD-02定义的production eHSM command adapter和未修改的Vendor公共Host代码；不得直接移植OSR platform地址、timer或reset逻辑。ADR-0010规定FMC固定使用`EHSM_DRV_MODE_WAIT_AND_POLL`，不启用Mailbox中断。ADR-0011规定FMC建立自己的一个256字节静态context slot，不继承BootROM context，每次只允许一条在途事务。

FMC不负责加载或启动eHSM Vendor FW，该职责位于GSP初始化阶段。FMC是唯一SoC counter提交发起者：初始化时把BootROM FMC Entry中的expected candidate回传BL；BL exact-match后compare/update/readback。证明成立后才接收GSP，且GSP必须等于已提交值。

CE-SEC-011证明Vendor BL verify成功后把SoC候选值暂存到eHSM DRAM。eHSM BL必须新增FMC可调用的16字节staged-candidate commit API，先比较回传值与RAM candidate，再执行update/readback；FMC不得使用通用64位Counter、raw OTP或FW启动副作用。

# Assumptions

GSP package source、PMA/Firewall、privilege和部分平台失败绑定仍待冻结。16字节counter、Measurement逻辑ABI及16 KiB物理Region已经冻结；C908统一System Address和GSP Region offset已经冻结；GSP完整package上限512 KiB、FMC release前静态footprint上限880 KiB。现有`0x1000_0808_0000/0x1010_0808_0000`均不是新D0批准Expected。

# Proposed design

| Design ID | FMC 动作 | 成功条件 | 失败行为 |
|---|---|---|---|
| FMC-DES-001 | 检查Measurement Table并取得唯一BootROM生产、verify PASS、`RELEASE_AUTHORIZED`、已commit FMC条目的16字节expected candidate | 得到稳定candidate后进入counter提交 | 条目缺失/重复/未commit、counter截断或完整性失败均进入统一终态 |
| FMC-DES-002 | 使用自己的一个64字节对齐、256字节静态context slot，以`EHSM_DRV_MODE_WAIT_AND_POLL`初始化最小eHSM adapter、active cache/timeout scope、error/audit和共享measurement访问；Vendor公共代码保持不变 | 与BootROM eHSM状态一致，service进入READY | 禁止继承/复用BootROM context、静默降级或启用Mailbox中断 |
| FMC-DES-002A | 初始化时调用`ehsm_bl_commit_staged_soc_rollback_counter(expected_candidate)`；BL先exact-match RAM candidate，再执行低值拒绝/相等不写/更高单向写/readback | `PROVEN_EQUAL/PROVEN_UPDATED`且after等于candidate | mismatch/缺失/低值在接收GSP前terminal；unknown不重试 |
| FMC-DES-003 | counter proof后从批准介质定位GSP Vendor原生type 1 package，执行source/System Address range、`package_size>1024`、stage/`UINT32_MAX`上限、防溢出和完整package output容量门禁 | package位于allowlist并被seal | 拒绝type 2/3、越界、重叠、尾随/截短和零长度 |
| FMC-DES-004 | 通过唯一service以`boot=0/check_version=0`调用真实Vendor BL verify/decrypt，保存completion、raw ret和完整output span；PASS后复验已认证Header | Vendor/Header层成功且16字节rollback counter完整 | timeout视为acceptance unknown；零自动retry并quarantine |
| FMC-DES-005 | Preflight及PASS后稳定Header均对offset256～1015计算CRC-32/ISO-HDLC并与offset1016 LE32比较；解析offset1008 LE64 `load_addr`和offset1020零reserved，要求精确等于typed GSP目标；GSP身份、LCS/board/Profile来自受信registry/matrix，Header counter等于已提交SoC值 | 全部字段和受信策略符合软件方案 | CRC只作格式筛查，不替代Vendor验签；旧格式、Host自选目标/Profile、CRC/reserved错误、Header不稳定、counter截断或Vendor PASS直通均fail-close |
| FMC-DES-006 | 在固定`SEC_RAM_GSP_STATIC`起始地址原地接收完整解密包；复验后对`target+1024`的`Code_Size`字节计算源摘要，以审计过的`memmove`移到`target`，清旧Header尾/BSS并从最终地址回读重算摘要；`entry_addr=load_addr`，再检查W^X/Firewall与`fence.i` | package不超过512 KiB、GSP静态/启动footprint不超过880 KiB、源/目标摘要相同 | 禁止input/output重叠、`memcpy`重叠搬移、设备端去CBC补零、触及活跃FMC 128 KiB或Measurement；失败不得jump |
| FMC-DES-007 | 要求GSP rollback counter等于FMC初始化已提交值 | 等值 | 不等拒绝；不得以GSP值推进OTP |
| FMC-DES-008 | 创建仅含ABI字段的`RELEASE_AUTHORIZED` GSP Measurement条目；counter result/raw状态进入审计而非Measurement | GSP Measurement commit、clear/protection完成 | commit失败不得release |
| FMC-DES-009 | 按批准privilege/interrupt/Firewall状态和既有release接口跳转GSP固定entry | GSP可消费批准的Measurement/typed-stage结果 | entry返回或异常形成错误并上报RAS；FMC不直接reset |
| FMC-DES-010 | 把FMC-DES-006得到的GSP最终readback digest作为FMC唯一DICE贡献，随GSP Measurement提交 | GSP后续验证Entry0/1并生成一级动态证书 | FMC镜像不得链接X.509 writer、UDS KDF、Alias KeyGen或证书签名入口 |
| FMC-DES-010 | 对已确定完成的格式/认证/counter失败返回结构化结果，清零敏感临时数据并重新arm Host ingress；是否再次发送由上位机决定 | 新package作为新事务重新执行完整检查 | eHSM timeout/BUSY/unknown completion、counter写状态未知时service quarantine，不得重新arm验证路径或把重新发送当作自动retry |

候选状态机：

```text
FMC_ENTRY
  -> VALIDATE_BOOTROM_OUTPUT
  -> INIT_SECURITY_CONTEXT
  -> COMMIT_STAGED_FMC_COUNTER
  -> LOCATE_GSP_PACKAGE
  -> EHSM_VERIFY_DECRYPT
  -> VALIDATE_HEADER_OVERLAY_AND_ADDRESS
  -> CHECK_GSP_COUNTER_EQUAL
  -> RECORD_MEASUREMENT
  -> PROTECT_CLEAR_SYNC
  -> RELEASE_GSP

任一失败 -> CAPTURE_STAGE_AND_RAW_ERROR -> APPROVED_TERMINAL_STATE
```

FMC阶段RAS尚未ready时，`APPROVED_TERMINAL_STATE`按ADR-0010保存静态`EARLY_SECURITY_ERROR_RECORD`、阻断GSP release、撤销临时权限并尽力清零；有限deadline上报失败后关闭普通中断并进入无限WFI fail-stop循环，不自行reset。

# Open questions

- GSP package 的 storage/source/size Owner。
- eHSM DMA、FMC写入统一System Address aperture的硬件行为、PMA/barrier/指令侧同步和Firewall Evidence；data clean/invalidate调用固定为0。
- Vendor direct aperture/status/error在SRC-0022中的准确宏及FMC可访问视图。
- FMC各operation最终`timeout_us`、service channel ID、I/O descriptor最大数和arena总大小。
- eHSM BL新增专用16字节staged-candidate commit API；具体command/packing/LCS/交付版本在实现前补齐。
- Vendor完整编码/寿命/耗尽、DRAM candidate reset语义、write error到Host状态映射，以及更新后掉电重启等待Host重发GSP的状态机。
- Measurement Table的逻辑C layout、CRC-32C、32位commit、reset全清零和FMC rollback-counter producer/consumer语义已冻结；ADR-0028固定物理offset/16 KiB容量，仍需OPEN-DESIGN-021实际最大实例、PMA/Firewall及跨master可见性Evidence。
- GSP release/reset/entry privilege 与失败终态。
- FMC固定128 KiB、GSP静态880 KiB、Measurement 16 KiB和Host ingress 512 KiB已由ADR-0028冻结；仍需FMC/GSP release map证明容量，以及`NON_CACHEABLE/HARDWARE_COHERENT`唯一属性和Firewall窗口参数。

# Implementation impact

- `solutions/fmc/app/src/main.c` 需要由可测试的 boot-flow entry 替代，hello-world 仅保留到 demo target。
- 目标实现至少需要BootROM输出/Measurement consumer、package source、eHSM adapter consumer、address/range validator、counter client、measurement writer和GSP release模块；不新增Handoff模块。
- production build 必须拒绝 `ehsm_stub` 可达路径和无签名/加密 packager 产物。

# Verification impact

- L1：Measurement/Header Overlay/parser/address/counter状态机的显式测试向量；不模拟真实eHSM成功。
- L2：真实格式 corpus、stub 禁入和 link-map/package consistency 静态检查。
- L3 EMU：真实 eHSM verify/decrypt、GSP load/jump、错误时 GSP entry 不可达。
- L4：timeout/reset、counter power-cut、partial write、cache/地址alias和Measurement/Header Overlay篡改。

# References

- SRC-0016 §2.1～§2.3。
- [安全启动](../03-architecture/secure-boot.md)。
- [防回滚](../03-architecture/anti-rollback.md)。
- [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。
- [GSP 地址域冲突](../../sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md)。
- [安全RAM布局](../03-architecture/security-ram-layout.md)。
- [OPEN-CONFLICT-006](../../sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md)。
- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)。
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)。
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)。
- [Measurement Table接口](../04-interfaces/measurement-table.md)。
- [安全固件包与Native Header Overlay](../04-interfaces/secure-firmware-package.md)。
- [镜像验签解密、Loader与Release接口](../04-interfaces/image-verify-loader.md)。
- [安全公共ABI与Registry](../04-interfaces/security-common-abi.md)。
- [BootROM/FMC/GSP函数级状态机](boot-stage-state-machines.md)。
- [ADR-0014](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)。
- [ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)。
- [eHSM Host Adapter详细合同](../04-interfaces/ehsm-host-adapter.md)。
- [CE-SEC-009](../../evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md)。
- [OPEN-CONFLICT-008](../../sources/conflict-reports/CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS.md)。
- [CE-SEC-011](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)。
- [OPEN-CONFLICT-009](../../sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md)。

# Change history

- 2026-09-01：按SRC-0036/ADR-0033把offset1016定义为LE32 Header CRC32、offset1020定义为4B零reserved，并在preflight和Vendor PASS后双检；CRC不替代密码认证。
- 2026-08-21：按ADR-0030删除Manifest；GSP固定目标改由Header offset1008与typed-stage registry精确匹配，原地搬移从`target+1024`开始并以`Code_Size`为唯一长度。
- 2026-08-19：按SRC-0032/ADR-0029明确FMC只提交GSP最终readback Hash作为DICE贡献，不读取UDS、不生成CDI/Key/X.509，动态证书全部在GSP生成。
- 2026-07-28：最终Counter时序改为FMC初始化先回传FMC Measurement expected candidate，BL与RAM candidate exact-match后compare/commit/readback；proof成立后才接收以`check_version=0`验证且counter等于已提交值的GSP。
- 2026-07-27：接受ADR-0019；统一`rollback_counter[16]`，Manifest新增`version`，FMC主动调用eHSM BL新增专用API并关闭OPEN-CONFLICT-009。
- 2026-07-24：接入CE-SEC-011并登记OPEN-CONFLICT-009；明确BL verify只暂存候选、FW启动才提交，因此现有“GSP加载FW”无法实现FMC release前counter提交，FMC-DES-007保持目标合同但禁止编码。
- 2026-07-24：负责人决定Counter细节延期；FMC-DES-007按默认存在的16字节抽象接口继续详设，具体Vendor绑定移至实现/EMU前门禁。
- 2026-07-23：接受ADR-0014并接入公共ABI/函数级状态机；当时的GSP验证后update顺序已由2026-07-28裁决取代，确定失败只等待新包、不重试旧命令的原则保留。
- 2026-07-23：接入CE-SEC-009和当时的包/verify/loader候选合同；当时采用Vendor PASS后Header复验、Manifest/policy门禁、统一loader源摘要/copy/目标回读摘要比较、counter/Measurement commit再release；Manifest部分已于2026-08-21由ADR-0030替代。
- 2026-07-23：批准Measurement→FMC epoch合同；FMC从唯一BootROM committed FMC条目取得自身epoch，要求与GSP epoch一致，并按counter update→GSP Measurement commit→release顺序执行。
- 2026-07-23：负责人最终批准GSP为全局security epoch锚点、FMC唯一更新counter；FMC/GSP epoch必须一致，更新成功后才能release。其他镜像失败隔离等待Host重发。首版不实现跨镜像整包Manifest或原子激活；未来若需要，必须通过独立ADR/OpenSpec重新设计。
- 2026-07-23：接受ADR-0011；FMC不继承BootROM context，使用一个256字节静态slot和单在途service；首版零自动retry，timeout/busy异常后quarantine。
- 2026-07-23：接受ADR-0010；FMC固定使用Vendor wait-and-poll，Vendor公共代码不修改；RAS未ready终态已进一步固定为静态错误记录、有限上报等待和无限WFI fail-stop循环。
- 2026-07-22：接入ADR-0006安全RAM/Mailbox/RAS合同；旧080x地址降为冲突现状，GSP目标改用Region ID表达，等待OPEN-CONFLICT-006。
- 2026-07-22：接受ADR-0004；2026-07-28最终更新为GSP、FMC loader和linker统一使用baremetal System Address且不建立Local/System映射；不采用Handoff的结论保留，当时的Manifest接口已于2026-08-21替换为Header Overlay/typed-stage loader。
- 2026-07-21：依据 CE-SEC-002 填入 FMC hello-world 现状、GSP release 缺口和首轮状态机；GSP 地址域保持 CONFLICTING。
- 2026-07-28：counter提交前移到FMC初始化，BL exact-match FMC candidate后compare/update/readback；GSP固定`check_version=0`且只做等值检查。
