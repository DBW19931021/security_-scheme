---
title: "方案基线控制"
status: active
evidence_state: DOCUMENTED
applicability:
  - NGU800P
source_ids:
  - SRC-0015
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0023
  - SRC-0024
  - SRC-0034
  - SRC-0035
  - SRC-0036
owners: []
last_reviewed: 2026-09-01
supersedes: []
superseded_by: []
---

# 目的

本文件维护 NGU800P 安全方案的有效基线组成、上下位关系、已接受裁决、临时修订和未决冲突。它不复制 SRC-0016/SRC-0017 PDF 全文，也不替代正式 PDF。

# 当前有效基线

| 层级 | 当前文件 | Source ID | 角色 | 当前状态 |
|---|---|---|---|---|
| 系统/架构上位方案 | `芯片系统安全方案.pdf` | SRC-0017 | 硬件基础、系统架构、安全边界和原则 | 当前原始基线；正式版本号/Owner/批准记录待补齐 |
| 软件工程落地方案 | `芯片安全软件方案v1.2 (1).pdf` | SRC-0016 | 软件流程、接口、参数、实现约束和采纳后的最终软件方案 | v1.2；完整审批记录待补，密钥轮换机制由ADR-0021作为受控增量补齐 |
| eHSM内部RTL实现基线 | `source-vault/vendor_rtl` | SRC-0035 | eHSM内部当前RTL实现细节和问题查询第一入口 | `VENDOR_IMPLEMENTATION`；缺正式filelist/release note/elaboration和动态验证，不能等同量产或硅片确认 |

# 权威关系

1. SRC-0016 必须依赖并符合 SRC-0017 的硬件前提、架构边界和安全原则。
2. Vendor 仅覆盖 eHSM 及其内部 Core，不定义 SoC。
3. Vendor文档、软件代码和RTL是接口意图及实现证据；被采纳的建议必须进入SRC-0016或其后续有效版本。eHSM内部实现查询优先级由第8项和ADR-0032细化。
4. 密钥轮换策略已经批准；ADR-0021已把SRC-0015三类SoC Key、每类一次、HSM Bitmap、48字节封装、USER鉴权和单向切换机制纳入软件方案增量。ADR-0026关闭软件设计裁决；Vendor定制ABI、Bitmap/掉电和外部KMS绑定继续作为实施输入。
5. SRC-0022是当前RTL同步的NGU800P SoC地址、寄存器和IRQ数值权威源；唯一窄范围例外是负责人2026-07-29指定的SRC-0023 `boot_pin.secure_boot[3]`字段/极性，当前生成头必须向该产品合同同步。GSP镜像和Vendor示例地址不得覆盖。SRC-0022不定义baremetal测试流程，也不替代系统/软件方案对Owner、时序和安全策略的规定。
6. SRC-0034/ADR-0031在BootROM模式矩阵前增加`non_sec_boot`最高优先级覆盖：可靠值1强制现有受限非安全，值0才继续SRC-0023 Checklist/Strap矩阵，输入异常阻断两条release。SRC-0023仍是Strap产品权威：`secureboot.001～007`已进入主详设第6章；`boot_pin.secure_boot[3]`固定default 0、0非安全/1安全，值0时USER强制安全。当前SRC-0022生成头的bit0 `SEC_BOOT`/bit3 `DIE_ID`与之冲突，真实绑定等待RTL同步。ADR-0024进一步冻结值0时LCS异常进入受限非安全；SoC LCS不依赖eHSM Autoload。
7. SRC-0024是负责人提供的OTP Table 34和16-slot Key表产品权威，目标仓是`security_-scheme`；baremetal中的manifest、fixture和case binding只派生同一表，不形成第二份方案基线。
8. SRC-0035是eHSM内部硬件实现细节、当前RTL实现基线和问题查询第一入口。使用时必须锁定树哈希并证明top/filelist/define/parameter/generate/wrapper条件；结论为`VENDOR_IMPLEMENTATION`。它不替代Vendor手册接口意图、SRC-0022 SoC数值源、SRC-0017/SRC-0016产品方案或匹配环境Evidence。
9. SRC-0036/ADR-0033冻结Native Header CRC字段和双阶段校验：offset1016为LE32 CRC-32/ISO-HDLC，覆盖offset256～1015；offset1020为4B零reserved。CRC只做格式完整性筛查，不替代Vendor密码认证。

# 已接受裁决

| ADR | 内容 | 是否已合入 PDF |
|---|---|---|
| ADR-0001 | 方案权威层级、Vendor 边界、密钥轮换批准状态、Vendor 文档分类和代码快照管理 | 部分为治理规则；密钥轮换机制后由ADR-0021补齐 |
| ADR-0002 | PDF/派生结论文档分工以及冲突升级机制 | 治理规则，不要求复制进 PDF 正文 |
| ADR-0003 | 批准W0-R1-01～04/06～11；自检位图以Bootloader为准，Host定义错误 | 自检协议和W0原则应进入SRC-0016后续有效版本或受控amendment；R1-05后续状态由ADR-0004更新 |
| ADR-0004 | Die1独立Measurement实例；当前不采用版本化Handoff。旧address-domain方案已被2026-07-28“全链路System Address、无Local/System映射”裁决替代 | 应进入SRC-0016后续有效版本或受控amendment；当前详设和测试按最终地址裁决推进 |
| ADR-0005 | 原裁决保留Measurement 64位load/entry loader结果快照；ADR-0022最终把两字段统一为`SOC_PA`且移除Measurement domain；counter采用Vendor 16字节；Wing-M130/eHSM BL保持Vendor业务基线 | 应进入SRC-0016后续有效版本或受控amendment；counter Owner/顺序已由ADR-0019补齐，具体Vendor API绑定仍是实现DoR |
| ADR-0006 | 产品唯一使用`0x1010_0500_0000`起始2 MiB System Address范围且不存在Local/System映射；Mailbox follow Vendor；security只上报错误，reset由RAS决定 | 总体边界继续有效；精确Region由ADR-0028冻结，量产linker仍等待PMA/Firewall、MMP DDR和release footprint Evidence |
| ADR-0007 | 新2 MiB替代旧080x；生命周期隔离、W^X、Owner转换和清零 | 旧P1容量拓扑、PMP/RMP/MMP全部SRAM常驻、独立plaintext及尾部复用方式已由ADR-0028替代 |
| ADR-0008 | eHSM作为受信任master可访问整个2 MiB；取消独立Mailbox Region并采用stage-local固定arena；首版只使用one-shot typed API，不启用异步/流式API、不分配session | 应进入SRC-0016后续有效版本或受控amendment；首版slot/并发/retry/回收已由ADR-0011冻结，最终PMA和arena总量待集成；IRQ/channel映射只属于未来interrupt change |
| ADR-0009 | NGU800P SoC地址/寄存器/IRQ以SRC-0022 baremetal RTL同步生成头为第一权威源；地址数值与接口语义分开管理 | 治理和硬件绑定规则；不要求复制生成头到PDF，软件方案应引用受控宏/版本 |
| ADR-0010 | eHSM采用Vendor direct 16×4 KiB Mailbox且Vendor公共源码不修改；BootROM/FMC/GSP首版全程poll，interrupt仅为Vendor FW ready后的后续独立优化；RAS未ready采用静态错误记录和fail-stop | 应进入SRC-0016后续有效版本或受控amendment；direct aperture/status宏和RAS数值参数仍待集成 |
| ADR-0011 | 每stage一个64字节对齐/256字节context slot和单在途；GSP唯一service串行化；首版one-shot only且不分配session；active cache/timeout scope；零自动retry；timeout acceptance unknown并quarantine | 应进入SRC-0016后续有效版本或受控amendment；最终PMA、operation timeout和arena总大小仍待集成；CE-SEC-010已确认IRQ数值，逐channel映射不阻断首版poll |
| ADR-0012 | BootROM直接读SoC LCS；FMC固定type 1并以`check_version=0`验证，BL暂存认证candidate；BootROM提交FMC Measurement；FMC初始化exact-match后提交/readback；proof后才接收同值GSP；Vendor FW由GSP加载 | 应进入SRC-0016后续有效版本或受控amendment；Vendor staged-candidate commit接口仍待交付，Measurement逻辑ABI已由ADR-0022批准、物理16 KiB由ADR-0028冻结，实际实例数/PMA仍待输入 |
| ADR-0013 | GSP镜像为eHSM FW/PMP/RMP/MMP；签名/RNG/Hash及方案能力必须实现；内部通用算法低优先级开放；eHSM作LCS最终授权；单Runtime失败局部隔离 | 应进入SRC-0016后续有效版本或受控amendment；算法集合已由ADR-0017冻结，具体provisioning、各镜像参数和Host API继续开放 |
| ADR-0014 | Vendor原生Image Type保持0/1/2/3；提交前和Vendor PASS后双阶段精确校验Code Size | Vendor层和长度门禁继续有效；NGU Manifest部分已由ADR-0030替代 |
| ADR-0015 | GSP从bootstrap到runtime由同一最高优先级`security_service_task`持续作为唯一eHSM Owner | 已进入主详设第8章；其他任务只可通过typed queue，不向Host开放GSP服务 |
| ADR-0016 | GSP替代旧OMP/Q&P产品固件；OMP不再作为独立产品镜像，不分配独立RAM/package/Measurement/counter/release身份 | 应进入SRC-0016后续有效版本或受控amendment；OMP历史代码仅作GSP迁移输入，未来恢复双镜像必须重新设计 |
| ADR-0017 | 产品Secure Package只允许三套完整算法Profile：SHA-256+RSA-2048-PSS+AES-128-CBC、SHA-256+ECDSA-P256+AES-128-CBC、SM3+SM2+SM4-CBC；其他Vendor算法只作baremetal能力/回归输入，产品release拒绝 | 应进入SRC-0016后续有效版本或受控amendment；ADR-0020已批准单一provisioning/release matrix，实际设备/SKU行待Owner补齐 |
| ADR-0018 | 值0时Strap产品字段以SRC-0023为准：`boot_pin.secure_boot[3]`、default 0、0非安全/1安全；USER强制安全；LCS独立于eHSM Autoload | `non_sec_boot=1`覆盖由ADR-0031定义；LCS异常原fail-close部分由ADR-0024替代；当前生成头冲突由OPEN-CONFLICT-010管理 |
| ADR-0019 | Counter统一为`rollback_counter[16]`；FMC初始化调用eHSM BL staged-candidate exact-match commit API；不向Host开放GSP服务 | Counter与Host边界继续有效；Manifest `version`部分由ADR-0030删除，版本只存外部release metadata |
| ADR-0020 | 冻结失败终态、Provisioning Matrix、Lifecycle/全局Debug、更新/OOB、Multi-Die、RAS/审计/清零和流片前发布原则；SPDM完整Profile前阻断实现；批准no-stub/test隔离 | 应进入SRC-0016下一受控版本/amendment；OPEN-DESIGN-014后由ADR-0026关闭，015/019继续管理剩余Profile/发布输入 |
| ADR-0021 | SoC Key轮换采用SRC-0015：Verify/Encrypt/Debug每类一次，HSM管理1字节Bitmap，48字节双层封装，USER鉴权，写Key→Bitmap→destroy→reset | 作为SRC-0016受控增量进入主详设第10章；真实slot/bit、Vendor定制ABI/版本、掉电/KMS/recipe继续开放 |
| ADR-0022 | Measurement Table采用128B Header、可变数量128B Firmware Entry和唯一128B SoC State；CRC-32C、32位commit和A/B Snapshot | 已进入主详设第9章；包内不携带Measurement identity；ADR-0028进一步固定16 KiB物理Region和126项硬上限 |
| ADR-0023 | 复杂流程优先图示；仓库内引用使用可辨识超链接；主详设与专题/ADR/Source/Evidence建立双向链接 | 作为全项目文档约束执行，并进入OpenSpec |
| ADR-0024 | 精简Manifest；LCS异常进入受限非安全且不写Measurement/审计；eHSM按eFuse自主执行自检，BootROM不发起 | Manifest部分已由ADR-0030替代；LCS异常和自检Owner裁决继续有效 |
| ADR-0030 | 删除NGU Manifest；Header offset1008为LE64 `load_addr`；`Code_Size`唯一长度、entry=load、typed-stage policy | 当前Package/loader/registry基础合同；其中offset1016后8B定义已由ADR-0033更新；旧Manifest包无兼容路径；SRC-0016下一受控版本/amendment需同步 |
| ADR-0033 | Header offset1016为覆盖offset256～1015的LE32 CRC-32/ISO-HDLC，offset1020为4B零reserved；preflight/PASS后双检且CRC不替代密码认证 | 当前Header Overlay格式校验合同；旧8B零reserved包无兼容路径 |
| ADR-0031 | `non_sec_boot`为默认0的1 bit eFuse最高优先级覆盖；可靠值1强制现有受限非安全启动，输入异常阻断两条release；BootROM只消费只读快照 | 应进入SRC-0016下一受控版本/amendment；准确物理位、ECC/valid/镜像、只读视图和烧写/锁定Owner由OPEN-DESIGN-024绑定 |
| ADR-0025 | 量产RTL Root/Install KEK按die唯一；OTP-KMU采用16槽对象基线；KMS/KEK轮换与一机一密一致 | 应进入SRC-0016下一受控版本/amendment；实施仍受OPEN-CONFLICT-012约束 |
| ADR-0026 | DEV阶段Chip Root→Level1→Level2→证书→MANU/USER；每设备单Attestation Profile；slot13 UDS七项权限；物理Key ID 0～15；Cert0/1与typed制造接口 | 应进入SRC-0016下一受控版本/amendment；OPEN-CONFLICT-013及OPEN-DESIGN-014软件设计关闭，Key Attribute/backend等外部输入作为实施绑定 |
| ADR-0028 | 2 MiB固定为GSP静态880 KiB、FMC复用128 KiB、Measurement 16 KiB、PMP/RMP各256 KiB、Host ingress 512 KiB；MMP驻留DDR；取消独立plaintext并采用受控原地加载 | 已进入主详设第5章和布局专题；PMA/Firewall、MMP DDR Profile及release link map峰值Evidence仍是实现DoR |
| ADR-0029 | DICE风格一级动态证明：ROM只记录FMC Hash、FMC只记录GSP Hash；Cert0/1保存三张静态Issuer前缀；GSP组装动态Firmware Alias Leaf；eHSM以UDS派生不可导出Alias Key并签名 | 已进入主详设第10/11章及证书/SPDM/Measurement专题；关闭OPEN-DESIGN-022，准确eHSM派生命令、Key Attribute、OID/DN/有效期/SM2编码由OPEN-DESIGN-023管理 |
| ADR-0032 | SRC-0035作为eHSM内部RTL实现细节、实现基线和问题查询第一入口；强制elaboration证据、只读快照、敏感Key和SoC/方案边界 | 工程治理规则，不改变产品功能；后续RTL调查和Expected必须引用具体Source/hash/configuration |

# 有效基线组成规则

当前有效方案由以下内容共同解释，但优先级和用途不同：

1. 当前 SRC-0017/SRC-0016 PDF。
2. 尚未合入下一版 PDF 的 accepted ADR/amendment。
3. 已批准 requirements、OpenSpec 和目标目录中的派生工程结论。
4. Vendor文档/软件代码/RTL、review和Evidence用于说明接口意图、当前实现、验证、差距和候选变更，不自行扩展有效产品方案。SRC-0035虽是eHSM内部实现第一查询源，仍只具有`VENDOR_IMPLEMENTATION`层级。

# 派生文档原则

- `docs/03-architecture/`：沉淀系统架构裁决后的详细结论。
- `docs/04-interfaces/`：沉淀接口、状态、权限和边界结论。
- `docs/05-software-design/`：沉淀软件流程、错误处理和工程实现结论。
- `requirements/`：沉淀可验证的批准要求。
- `decisions/`：记录重要裁决及其理由。
- `openspec/`：管理需要改变行为或设计的提案与实施。

这些文档只记录新增、澄清或裁决后的结论，不复制 PDF 全文。

# 冲突状态

OPEN-CONFLICT-001～005、007～010、013均已关闭；OPEN-CONFLICT-006继续管理安全RAM/Linker平台绑定，OPEN-CONFLICT-011/012是已批准目标的Vendor/RTL交付绑定：

| Conflict | 状态 | 阻断范围 | 当前处理 |
|---|---|---|---|
| `CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP` | RESOLVED | 不再阻断；当前4019采用Bootloader位图 | bit18=`0x40000`=TRNG；Host定义错误；bit19保留raw，见ADR-0003 |
| `CONFLICT-SRC-0016-DIE1-MEASUREMENT` | RESOLVED | 不再阻断；Die1内部Measurement实例规则已冻结 | 独立记录`NGU_FW_TYPE_DIE1_FW`/`die_id=1`；SPDM可聚合但不得丢失实例/release语义，见ADR-0004 |
| `CONFLICT-CODE-GSP-ADDRESS-DOMAIN` | RESOLVED / SUPERSEDED | 地址视图不再阻断；旧080x绝对地址拒绝 | C908、Header Overlay、loader、linker和eHSM共享descriptor统一使用baremetal System Address，不建立Local/System转换 |
| `CONFLICT-MEASUREMENT-ADDRESS-WIDTH-OWNERSHIP` | RESOLVED | 不再阻断地址字段语义 | `load_addr/entry_addr`均为`uint64_t SOC_PA`且不带domain，只作loader结果快照；原ADR-0005的domain部分由ADR-0022更新 |
| `CONFLICT-COUNTER-WIDTH-COMPARE-UPDATE` | RESOLVED | eHSM BL/FMC实现参数 | ADR-0019：不存在32字节歧义；唯一使用`rollback_counter[16]`，32/64位方案拒绝 |
| `CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS` | OPEN / PARTIALLY DECIDED | 最终PMA/arena总量、精确容量、Firewall参数及最终linker/Expected | System Address视图、生命周期布局、eHSM full aperture、stage-local arena及首版one-shot/slot/并发/retry/回收已批准；ADR-0016已关闭GSP/OMP角色；P1数值不是冻结Region offset |
| `CONFLICT-NGU800P-GENERIC-VS-EHSM-MAILBOX-MMIO` | RESOLVED / INTEGRATION BINDING PENDING | 方向不再阻断；direct aperture/status准确宏缺失仍阻断真实MMIO | ADR-0010采用Vendor direct布局且Vendor源码不修改；4 KiB通用Mailbox不用于eHSM，准确base须同步进入SRC-0022 |
| `CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS` | RESOLVED / UPDATED_BY_ADR_0030 | SRC-0016下一受控版本/amendment仍需修正文档表述 | 保持Vendor 0/1/2/3 wire和公共代码不变；项目镜像由typed-stage registry区分；type 1使用Header Overlay；release工具/adapter在提交前和PASS后强制`Code_Size == package_size - 1024` |
| `CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE` | RESOLVED / IMPLEMENTATION_PENDING | eHSM BL专用API实现参数和EMU证明 | ADR-0019要求BootROM暂存FMC candidate、FMC初始化调用BL staged-candidate exact-match commit/readback API；实现/EMU前补齐command/packing/LCS/status/readback/交付版本，不依赖FW启动副作用 |
| `CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY` | DESIGN_RESOLVED / BLOCKED_BY_RTL_SYNC | 与SRC-0023一致的RTL生成命名宏或批准映射、MGMT/SEC镜像Owner和EMU读回 | 在`non_sec_boot=0`时产品字段为`boot_pin.secure_boot[3]`、default 0、0非安全/1安全且USER强制安全；当前bit0/bit3均不得直接绑定 |
| `CONFLICT-SRC-0015-CURRENT-VENDOR-KEY-ROTATION` | TARGET ACCEPTED / BLOCKED_BY_VENDOR_DELIVERY | USER态SoC Key Rotation产品实现和真实OTP/EMU | 轮换方案本身无概念冲突；等待匹配Vendor定制Host/BL/FW、专用command和release note |
| `CONFLICT-PER-DIE-RTL-KEY-VS-VENDOR-SHARED-MACRO` | TARGET ACCEPTED / BLOCKED_BY_RTL_BINDING | 量产RTL Key/netlist/ATE个性化 | 一机一密已批准；需隐藏逐die载体、不可读/lock/operation proof，Vendor宏不得作为共享量产值 |
| `CONFLICT-16-SLOT-OTP-PROVISIONING-ORDER-AND-ATTESTATION` | RESOLVED | 不再需要方案裁决；真实recipe仍等待实施绑定 | SRC-0024纠正旧转录；ADR-0026批准DEV Chip Root→Level1→Level2、slot14单Profile、slot13 UDS权限和物理Key ID 0～15 |

历史 Review 中其余候选发现仍为 `pending_revalidation`，尚未自动认定为与软件方案冲突。

发现方案、Vendor 文档、Vendor 代码、测试或实测明显不一致时，必须创建 `sources/conflict-reports/` 报告并提交项目负责人裁决；受影响结论在裁决前标为 `CONFLICTING`。

# 待补充控制信息

- SRC-0017 的正式版本号、Owner、批准日期和适用芯片修订。
- SRC-0016 v1.2 的 Owner、完整批准记录和适用芯片修订。
- SRC-0015轮换机制已由ADR-0021和主详设第10章作为受控增量吸收；后续PDF版本需明确是否正式并入正文。
- 后续 PDF 新版本的 `Supersedes` 关系和 accepted amendment 吸收情况。

# Review history

- 2026-09-01：登记SRC-0036并接受ADR-0033；冻结Header CRC字段、算法/覆盖范围、制包顺序、preflight/PASS后双检和旧格式拒绝。
- 2026-08-26：登记SRC-0035并接受ADR-0032；把`source-vault/vendor_rtl`冻结为eHSM内部硬件实现第一查询源，补齐elaboration、只读快照、敏感Key、SoC边界和动态验证门禁。
- 2026-08-25：登记SRC-0034并接受ADR-0031；冻结`non_sec_boot`默认0/烧写1覆盖语义、BootROM只读快照、异常终态和实施绑定门禁。
- 2026-08-21：登记SRC-0033/CE-SEC-016并接受ADR-0030；删除Manifest方案，冻结Header offset1008/1016、`Code_Size`唯一长度、entry=load和typed-stage policy，旧OpenSpec改为superseded。

- 2026-08-04：登记SRC-0024；将Table 34和16-slot Key表确立为security_-scheme唯一基线，纠正ADR早期转录并明确baremetal只派生实现/验证记录。

- 2026-07-29：接受ADR-0026；关闭OPEN-CONFLICT-013和OPEN-DESIGN-014的软件设计裁决，冻结灌装、Attestation、UDS、物理Key ID、Cert0/1和制造接口。
- 2026-07-29：接受ADR-0025；冻结量产一机一密和16槽OTP对象，登记OPEN-CONFLICT-012/013；第10章补齐KMS/RTL/制造/Device Identity/Cert0/1候选和实施门禁。
- 2026-07-28：接受ADR-0023/0024；新增“复杂关系优先图示、引用可辨识超链接和双向链接”约束；Manifest删除ABI major/minor、地址domain、board/Measurement/component绑定和TLV；LCS异常进入受限非安全且不写Measurement/审计；eHSM自检改为eFuse驱动的Vendor自主流程，BootROM不发起。
- 2026-07-27：接受ADR-0019并关闭OPEN-CONFLICT-005/009、OPEN-DESIGN-006；Manifest分离`version`与`rollback_counter`，FMC主动调用eHSM BL新增API，GSP不向Host开放服务。
- 2026-07-29：负责人指定以SRC-0023截图为准；产品字段更新为`boot_pin.secure_boot[3]`、default 0、0非安全/1安全。OPEN-CONFLICT-010改为`BLOCKED_BY_RTL_SYNC`。
- 2026-07-24：接受ADR-0018并当时关闭OPEN-CONFLICT-010/OPEN-DESIGN-012；当时采用的RTL bit0和相反极性已被2026-07-29裁决替代，LCS读取时序继续有效。
- 2026-07-21：创建基线控制入口；确认 PDF 保持原始基线、派生文档只记录裁决结论，并建立冲突升级规则。
- 2026-07-21：测试覆盖审视发现 SRC-0016 Die1 measurement 表述不一致，登记为局部开放冲突。
- 2026-07-22：接受ADR-0003；关闭自检位图冲突，批准W0-R1-01～04/06～11；R1-05当时待评审，后由ADR-0004更新为当前不采用。
- 2026-07-22：接受ADR-0004；关闭Die1 Measurement和GSP地址域冲突，R1-05改为当前不采用，Measurement Table承载跨阶段度量记录。
- 2026-07-22：任务二首轮详设复核发现Measurement 32位地址与64位canonical值、native 128位counter与32位global counter/严格`+1`规则冲突；登记OPEN-CONFLICT-004/005。
- 2026-07-22：OPEN-CONFLICT-004部分裁决，Measurement `load_addr`保留并改为64位；`entry_addr`和字段职责未决，冲突暂不关闭。
- 2026-07-22：接受ADR-0005；OPEN-CONFLICT-005宽度按Vendor统一为16字节，Wing-M130/eHSM BL保持Vendor业务基线；counter剩余语义和Measurement `entry_addr`继续开放。
- 2026-07-22：负责人确认Measurement `entry_addr`同样保留为64位；结合ADR-0004冻结地址为loader结果快照，关闭OPEN-CONFLICT-004。
- 2026-07-22：接受ADR-0006；冻结2 MiB安全RAM总体目标、Mailbox follow Vendor和RAS reset职责边界；登记OPEN-CONFLICT-006，停止最终linker/绝对地址冻结。
- 2026-07-22：接受ADR-0007；OPEN-CONFLICT-006部分裁决并改用P1生命周期布局；旧080x被替代，PMP/RMP/MMP常驻，BootROM/FMC尾部回收，GSP/Measurement/Mailbox连续且SPDM并入GSP。
- 2026-07-23：负责人批准取消独立Mailbox Region并采用各stage固定context arena；通用异步、流式及timeout未闭环context不放普通函数栈。
- 2026-07-23：接受ADR-0009并登记SRC-0022；SoC地址/寄存器/IRQ以baremetal RTL同步生成头为第一权威源，OPEN-CONFLICT-006/007收敛为使用/接口语义问题。
- 2026-07-23：接受ADR-0010并关闭OPEN-CONFLICT-007方向选择；冻结Vendor direct Mailbox/源码只读边界、BootROM/FMC/GSP首版全程poll及RAS未ready早期终态，interrupt仅为Vendor FW ready后的后续独立优化。
- 2026-07-23：接受ADR-0011；冻结首版单context/单在途、GSP串行service、Vendor兼容cache/timeout scope、零自动retry及timeout quarantine；最终PMA和数值参数继续集成。
- 2026-07-23：INV/CE-SEC-009发现SRC-0016原生`Image_Type`简化说明与Vendor 0/1/2/3定义不一致，且Vendor direct verify未检查Header `Code_Size`与命令长度相等；建立OPEN-CONFLICT-008，未修改Vendor和两个公司代码仓。
- 2026-07-23：项目负责人批准ADR-0014并关闭OPEN-CONFLICT-008/OPEN-DESIGN-008；Vendor wire/公共代码保持不变，采用双阶段精确长度校验、128字节Manifest v1和中心公共registry。SRC-0016原PDF不改，修正进入下一受控版本/amendment。
- 2026-07-24：项目负责人批准ADR-0015并关闭OPEN-DESIGN-009；GSP首个最高优先级`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner，其他task只经typed queue请求。OPEN-DESIGN-010所需PMP/RMP/MMP依赖/release资料后续补充，当前禁止猜测。
- 2026-07-24：项目负责人批准ADR-0016；GSP替代旧OMP/Q&P产品固件，OMP不再作为独立产品镜像或分配独立RAM/package/Measurement/counter/release身份。
- 2026-07-24：完成INV/CE-SEC-011并登记OPEN-CONFLICT-009；确认Vendor 16字节counter编码、BL candidate暂存和FW启动OTP提交/readback事实，counter提交与FMC→GSP release实现等待负责人/Vendor输入。
- 2026-07-24：负责人决定Counter细节延期；按16字节值和默认接口继续详设，Vendor绑定移至实现/EMU前。
- 2026-07-24：接受ADR-0017并关闭OPEN-DESIGN-004；冻结三套产品Secure Package算法Profile及公共ID 1/2/3，其他Vendor算法只作baremetal能力/回归覆盖；具体provisioning/release绑定转入OPEN-DESIGN-011。
- 2026-07-24：登记SRC-0023并建立OPEN-CONFLICT-010/OPEN-DESIGN-012；`secureboot.001～007`已合入主详设第6章，位号和非USER策略未关闭前禁止真实模式编码和确定性Expected。
