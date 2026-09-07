# 项目目的

本仓库是 SoC 安全架构、需求、决策、实现关联、验证、Evidence 和长期问题知识的唯一事实源。仓库内新增或修改的 Markdown 默认使用中文。

# 顶层任务模型

本项目只保留两个顶层交付任务：

1. `TASK-SEC-BAREMETAL-EHSM-001`：在 `baremetal` 落地 eHSM 全接口/全能力可执行测试，覆盖当前 `../fsp/test_demo` 中的 Vendor `ehsm_demo_test()`，顶层执行遵守 baremetal 规则。
2. `TASK-SEC-SOC-FW-001`：完成 SoC 安全固件详设与实现；Wing-M130/eHSM BL 以 Vendor 基线和集成为主，C908 BootROM/FMC/GSP 以批准的软件详设为准实现完整产品链。

`TASK-SEC-DD-001`、`TASK-SEC-DEV-PLAN-001`、`TASK-SEC-EMU-PREP-001`、`INV-SEC-*`、`DEV-SEC-*` 和 `TST-SEC-*` 均是上述两个顶层任务的工作包、调查或追溯记录，不得作为额外顶层任务并行管理。项目总入口为 `docs/09-plans/NGU800P两项任务总计划.md`。

# 开始工作前

按顺序阅读：

1. `PROJECT_STATUS.md`
2. `manifests/repositories.yaml`
3. `sources/source-index.yaml`
4. 相关 `docs/`；涉及eHSM内部硬件时必须读取`SRC-0035` Source Card和`docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md`
5. 相关 `decisions/`
6. 相关 `openspec/changes/`
7. 相关 `tasks/active/`

# 事实状态

不得把假设描述为已确认硬件行为。只使用：`CONFIRMED`、`DOCUMENTED`、`VENDOR_IMPLEMENTATION`、`ASSUMPTION`、`PROPOSED`、`DEPRECATED`、`OBSOLETE`、`CONFLICTING`。

每项硬件相关结论必须包含 Source ID、适用芯片/版本、事实状态和待确认问题。聊天历史不是正式事实源。

# 方案与 Vendor 权威边界

- `SRC-0017 芯片系统安全方案`是上位系统/架构方案，定义硬件基础、系统架构和安全原则。
- `SRC-0016 芯片安全软件方案`是依赖 SRC-0017 的下位工程落地方案；软件侧最终采用的方案必须体现在其最新有效版本中。
- 本项目中的 Vendor 仅指 eHSM 及 eHSM 内部 Core，不包含 NGU800P SoC。Vendor 文档、代码、测试和建议不得直接定义 SoC 行为。
- `TASK-SEC-BAREMETAL-EHSM-001` 及其用例梳理/开发子任务的当前 Vendor 代码工作参考固定为 `../fsp`（当前绝对路径为 `/home/may-pc/share/code/ngu800/secure/fsp`）；Demo 覆盖根从 `../fsp/test_demo/demo/test.c` 中的 `ehsm_demo_test()` 开始，并沿当前 BL/FW entry 和 Host command 调用链展开。
- `../fsp` 是会实时更新的独立活参考仓库，不是固定快照。每次开始用例盘点、设计、移植或回归前，必须记录当时的 remote、branch、commit 和工作区状态；代码结论必须引用该 commit 下的路径、符号和行范围，发现 FSP 更新后需重新核对受影响的 case 和 Evidence。
- `source-vault/vendor-code/osr_eshm`（SRC-0018）继续作为 2026-06-26 交付的不可变历史快照和旧 Evidence 复核入口，不再作为新用例梳理或开发的默认当前参考。既有任务、设计和 Evidence 中对 SRC-0018/`osr_eshm` 的引用仅表示其形成时的基线；新工作必须优先核对 `../fsp`。
- `source-vault/vendor_rtl`（SRC-0035）是当前eHSM内部硬件实现细节、RTL实现基线和问题查询的第一入口。所有结论必须锁定Source ID/树哈希、相对路径、module/instance/symbol、紧凑行范围以及top/filelist/define/parameter/generate/wrapper条件；未证明实际elaboration时只能写为候选配置并标为`ASSUMPTION`，同时登记缺失输入。
- SRC-0035直接证明的结论保持`VENDOR_IMPLEMENTATION`，不因源码存在自动提升为`CONFIRMED`；Vendor手册继续表达正式接口意图。RTL与手册/FSP/测试/Evidence明显冲突时必须标为`CONFLICTING`并升级，不能以“RTL优先”静默覆盖。
- 本项目对 `../fsp` 默认只读；用例开发落地在 `../baremetal`。除非用户另行明确授权 FSP 变更，不得修改 `fsp` 中的 `ehsm_bootrom`/`ehsm_firmware` Vendor 代码、`test_demo` 或工具链/构建环境。
- NGU800P SoC地址空间、寄存器base/offset/bitfield、IRQ编号/名称及同类RTL同步硬件常量，以`../baremetal/components/chip_riscv_c908_common/include/ngu800p`当前有效生成头（SRC-0022）为第一权威源；`gsp-pmp-rmp-omp`同名头仅为实现镜像，Vendor示例地址不得覆盖它。该权威范围不包含baremetal的test/stub/demo/Expected，也不自动决定寄存器块Owner、wrapper语义、cache、RAS、reset或产品流程。
- eHSM/Core 的内禀硬件实现特性、寄存器实现、状态机、reset/default表达式和内部连接先查SRC-0035；正式接口/协议说明对照当前有效Vendor手册，软件使用行为对照任务锁定版本的FSP。除非RTL缺失或无法elaborate、来源冲突、依赖NGU800P SoC集成参数/产品策略、需要动态验证，或新Vendor交付明确替代旧版本，不得把现有资料已经回答的内容重复列为open question或要求负责人再次提供。
- 上述默认基线不扩大 Vendor 权威范围：Vendor 代码仍是 `VENDOR_IMPLEMENTATION` 证据，不直接定义 NGU800P SoC 地址、cache、一致性、RAS、reset 或产品软件策略；发现资料内部冲突或与软件方案明显冲突时仍按冲突升级流程处理。
- 采纳 Vendor 建议前保持 `PROPOSED`；只有写入并通过“芯片安全软件方案”管理后，才能作为项目软件方案和实现依据。
- 密钥轮换策略已经批准；ADR-0021已把SRC-0015的三类SoC Key、每类一次、HSM 1字节Bitmap、48字节双层封装、USER鉴权、写Key→Bitmap→destroy→reset机制纳入软件方案增量。ADR-0026已关闭OPEN-DESIGN-014的软件设计裁决；Vendor定制ABI/版本、Bitmap掉电、KMS接口和可执行recipe继续作为实施绑定。
- ADR-0025已批准量产RTL Root/Install KEK按die唯一、软件不可读，并冻结16个OTP Key对象。Vendor RTL宏只能作为集成接口，不能把同一明文常量固化进所有量产die。KMS按设备托管RTL/Root Key句柄并生成设备专属封装；任何Key明文、可复用blob或Device Private Key不得进入仓库、日志或MES。
- 对本工程而言 Vendor 文档分类为 `NOT_CONFIDENTIAL_FOR_THIS_PROJECT`；仍需遵守版权、授权和公司对外分发规定。
- Vendor 正式交付代码按快照整体登记，不逐文件建立 Source Card。新的正式 delivery/release 必须附 note、分配新 Source ID，并通过 `Supersedes` 关联旧快照；`../fsp` 的持续更新先作为按 commit 锁定的工作参考，需进入受控基线或正式 Evidence 时再按本规则登记。
- 本节决策依据见 `decisions/ADR-0001-solution-authority-and-vendor-boundary.md`。

# 仓库边界

- 本仓库负责规格、决策、OpenSpec、全部项目级任务规划、正式安全软件详细设计、测试策略/计划/工作簿、Feature/Requirement/code/case/Evidence 追踪和发布 Evidence 索引，是上述工程资料的唯一事实源。
- 正式详设由 GSP 负责产出和评审，但文件存放在本仓库：系统/架构结论进入 `docs/03-architecture/`，接口契约进入 `docs/04-interfaces/`，软件详细设计和主入口进入 `docs/05-software-design/`。
- `../gsp-pmp-rmp-omp` 是独立多人软件实现仓库；GSP 在其中完成正式安全软件代码和必要的实现侧单元/契约测试。该仓库不新增本项目的任务计划或正式详设，只允许保留构建、API 使用、代码注释等实现所必需的就地说明，并继续遵守其 `AGENTS.md`。
- `../baremetal`（用户口头称 `baremental`）是独立多人测试/裸机实现仓库；GSP 在其中维护可执行测试代码、EMU runner、测试工具和必要测试数据，默认收敛在 `components/ngu_security/tests/`、`components/ngu_security/tools/`。测试策略、测试计划、版本化工作簿、case 规划和发布追踪不放在该仓库，并遵守其 `AGENTS.md`。
- `../fsp` 是独立、持续更新的 eHSM Vendor/FSP 参考仓库；本仓库对它只建立引用和按任务锁定的 commit 关系，不将其纳入 `security_-scheme` Git 历史，也不在未授权的用例任务中修改它。
- `source-vault/vendor_rtl`是本仓库内不可变、只读的受控硬件输入，不是RTL开发工作树。不得修改、格式化、自动修复或写入生成物；树哈希变化时停止沿用SRC-0035并为新投递建立新Source ID和`Supersedes`。
- `../baremetal`同时承载SRC-0022所登记的RTL同步SoC map/寄存器生成头；这一窄范围是硬件数值权威源，不改变该仓库其余代码和文档的测试实现角色，也不授权在调查任务中修改它。
- 不在两个代码仓库新增项目级计划的第二份副本；本次不迁移或删除其中可能已有的历史文档。`security_-scheme` 不复制维护第二套可执行软件或测试代码。
- 不删除或重建任何 `.git`，不把独立软件仓库作为普通目录纳入本仓库。
- 开始代码/测试任务前确认目标是 `gsp-pmp-rmp-omp` 还是 `baremetal`、当前分支、只读基线 commit、修改范围和已有未提交修改的归属。
- 不得覆盖、清理或改写与当前任务无关的工作区修改；无法安全区分时停止修改并报告，不得执行 `reset` 或 `clean`。

## Git 操作硬约束

- **允许直接在多人仓库默认分支的工作区修改文件，不要求创建 feature/bugfix 分支。**
- **所有任务都不需要 Git 提交；Codex 不得执行 `git add`、`git commit`、`git push`、merge、rebase 或 tag。**
- 分支名和基线 commit 仅用于记录修改所基于的代码状态，不作为交付物，也不要求产生新的 commit、MR/PR 或合并记录。
- 交付以未提交工作区 diff 为准；RESULT 必须记录实际修改文件、diff 摘要、执行命令、测试结果、Evidence 和剩余风险。
- 只有用户以后明确改变本约束时，才可执行任何写入 Git 索引、历史或远端的操作。

# 变更规则

实现行为变化时：创建或更新 OpenSpec、更新设计、更新需求追踪、补充测试、记录 Evidence、更新项目状态；重大安全决策另建 ADR。

## 文档可视化与链接

- 对三个及以上步骤、分支、参与者、状态或内存对象的关系，优先使用Mermaid流程图、时序图、状态图、数据流图或内存示意图；图示不替代字段offset、条件、错误语义和正文约束。
- 仓库内文档引用必须写成“可辨识名称 + 文档类型/用途 + 可点击相对链接”，禁止只写裸文件名、裸Source ID或裸ADR编号。
- 主详设引用的关键专题、ADR索引、Source Card和Evidence必须提供返回主详设对应章节的反向链接；修改一端时同步检查另一端。
- 原始PDF/Vendor资料通过Source Card作为受控导航入口，引用时写明资料名称、类型、Source ID和状态。
- 规则依据见`decisions/ADR-0023-document-visualization-and-bidirectional-links.md`，并已写入`openspec/config.yaml`。

# 代码调查与正式详设回填

- 正式详设依赖当前代码事实时，按 `docs/09-plans/CODE-INVESTIGATION-WORKFLOW.md` 创建小粒度 `tasks/active/INV-SEC-xxx-<topic>.md`，不得用“分析整个安全方案”代替可验收问题。
- 调查阶段只读，不修改业务代码、构建文件或正式设计结论，不执行任何 Git 写操作；为锁定 `../fsp` 活参考基线，允许只读获取 remote/branch/commit/工作区状态。报告写入 `evidence/code-investigations/CE-SEC-xxx-<topic>.md`。
- 重要代码结论必须记录仓库清单中的只读基线、文件、符号、紧凑行范围、调用链、构建/宏/平台条件、错误路径和限制；必须阅读关键函数体，不能只依据名称、注释或历史Review。
- 调查报告区分 `CODE_FACT`、`INFERENCE`、`TARGET_DESIGN`、`GAP`、`UNKNOWN` 和 `CONFLICTING`。代码现状不自动成为目标设计，目标设计也不描述为已实现。
- eHSM内部硬件调查必须同时执行`docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md`：先证明顶层/filelist/define/parameter/generate和实例链，再沿完整逻辑锥核对reset/error/clear/lock/CDC及消费者；模块存在、宏名、注释或单个assign均不足以确认实际硬件。
- 证据经抽查后回填正式详设、Requirement、Feature、测试和Evidence追溯；架构、安全、冲突、风险接受和编码准入由用户/负责人裁决。

# 基线沉淀与冲突升级

- SRC-0016/SRC-0017 PDF 是当前受控原始基线，不要求完整重写为 Markdown；有效基线组成维护在 `docs/09-plans/BASELINE-CONTROL.md`。
- `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 等目录只沉淀经过分析或裁决的可执行结论，不复制 PDF 全文。
- “软件方案为准”表示项目规范目标和采纳状态，不表示可以忽略 Vendor 文档、代码、测试或实测中的明显矛盾。
- 发现系统方案—软件方案、方案—Vendor文档/软件代码/RTL、Vendor文档—代码—RTL、方案—测试/实测冲突，或SoC/eHSM/Core/Host边界不清时，立即把受影响结论标为`CONFLICTING`，停止在受影响范围继续外推、设计或实施。
- 冲突必须在当前任务内向用户提出，并写入 `sources/conflict-reports/`；报告包含双方 Source/版本/章节或代码位置、冲突类型、影响、可继续/停止范围、2～3 个选项、推荐项和需要用户裁决的问题。
- 用户裁决后更新 ADR、BASELINE-CONTROL、软件方案或受控 amendment、相关需求/OpenSpec、Source Card、open question 和项目状态。
- 完整规则见 `decisions/ADR-0002-baseline-derivation-and-conflict-escalation.md`。

## 当前详设冲突门禁

- `OPEN-CONFLICT-004`已关闭并由ADR-0022/0030及负责人最终地址裁决更新语义：Measurement `load_addr`和`entry_addr`均保留为`uint64_t` System Address，不携带domain，只作loader成功执行后的只读审计快照；Native Header offset1008只存LE64 `load_addr`，entry固定等于load。C908产品软件不存在Local/System映射，local/remap值只作历史RTL事实并直接拒绝输入。
- `OPEN-CONFLICT-005`已由ADR-0019关闭：Vendor手册、Header和代码中的SoC `Version_Counter`始终为16字节；旧冲突来自SRC-0016的`uint32_t`示例，即32位/4字节而不是32字节。项目统一使用opaque `rollback_counter[16]`，Header、Measurement、工具和软件接口不得截断为32/64位，也不得建立第二套项目Counter；Host通用64位Counter不是该资源。
- `OPEN-CONFLICT-009`已由ADR-0019关闭并由ADR-0030更新后置门禁：FMC必须在GSP完整验证、Header Overlay/typed-stage policy/digest/rollback一致性和loader全部成功后，主动调用eHSM BL新增的FMC专用16字节rollback-counter update/readback API；提交被证明后才能提交GSP Measurement并release GSP。不得依赖Vendor FW启动副作用、raw OTP或通用64位Counter替代该API。command/packing、产品LCS权限、状态码/readback、交付版本、寿命和掉电语义属于eHSM BL/FMC实现DoR，必须在编码和EMU验证前补齐。
- `OPEN-CONFLICT-006`已部分裁决并由ADR-0028更新：新2 MiB目标替代旧080x，PMP/RMP各256 KiB，MMP主要驻留DDR，FMC 128 KiB可回收，GSP静态880 KiB、Measurement 16 KiB、Host ingress 512 KiB，eHSM可访问整个2 MiB；独立Mailbox/plaintext Region已取消。ADR-0016确认GSP替代旧OMP/Q&P。C908产品软件、Header Overlay、loader、linker和eHSM共享descriptor全部使用System Address；现有旧local linker只是待替换遗留实现。剩余PMA、Firewall、MMP DDR和release footprint关闭前，不得发布生产linker或精确Expected。
- `OPEN-CONFLICT-006`的地址数值来源已由ADR-0009收敛：产品唯一使用SRC-0022定义的`MANAGEMENT_NOC_S9_SRAM_BASE=0x1010_0500_0000`和2 MiB容量。local/remap `0x1000_0500_0000`只保留为历史RTL事实，产品输入直接拒绝且不定义转换关系；旧linker、baremetal测试布局和现有map均不得作为新P1最终常量。
- `OPEN-CONFLICT-007`已由ADR-0010关闭：采用Vendor direct 16×4 KiB Mailbox合同，Vendor公共源码保持不变；SRC-0022中的4 KiB/84-message通用Mailbox不用于eHSM wrapper。CE-SEC-010确认direct aperture/status准确宏仍未同步到SRC-0022，该缺口只阻断真实MMIO实现，不重开transport方向，也不得使用4 KiB块或OSR样例地址代替。
- ADR-0030/0033已替代ADR-0024的Manifest部分：NGU800P type 1包删除Manifest，固定为Header[1024]+Code[Code_Size]；Header offset1008为LE64 `load_addr`、offset1016为覆盖Header 256～1015的LE32 CRC-32/ISO-HDLC、offset1020为4B零reserved，entry=load，`Code_Size`是唯一签名/加密/搬移/Measurement长度。CRC在preflight和Vendor PASS后双检但不替代密码认证。身份/Profile/policy来自受信typed-stage registry和provisioning matrix。Loader分别计算`target+1024`源Code和目标读回摘要并常量时间比较；旧Manifest、旧8B零reserved、fallback和dual parser禁止。ADR-0024的LCS异常及eHSM自检Owner裁决继续有效。
- BootROM模式新增SRC-0034/ADR-0031最高优先级覆盖：`non_sec_boot`是默认0的1 bit eFuse字段；可靠值1时包括USER在内均强制进入现有受限非安全路径，值0时才执行SRC-0023矩阵：`boot_pin.secure_boot[3]`、default 0、0受限非安全/1安全、USER强制安全。policy-fuse读取/ECC/镜像/来源异常阻断两条FMC release；BootROM只消费专用只读快照，不得获得raw eFuse。准确物理位/编码/只读视图/烧写锁定由OPEN-DESIGN-024管理。当前Strap的SRC-0022生成头冲突仍等待RTL同步。ADR-0024规定值0时LCS异常进入独立受限非安全路径。受限路径不创建Measurement Entry/SoC State或启动审计，也不开放安全资源。
- OPEN-CONFLICT-012不重开一机一密目标，只阻断量产RTL绑定：逐die隐藏载体、Secure ATE、不可读/lock/operation proof到齐前，不得把Vendor共享宏或软件配置宣称为一机一密实现。
- OPEN-CONFLICT-013已由ADR-0026关闭并由SRC-0024纠正早期转录：设备保持DEV完成Chip Root→slot1～5 Level1→slot6～15 Level2→证书后再相邻切换MANU/USER；软件保留P-256/SM2能力，但每设备Profile只在slot14保存和使用一把长期私钥；UDS固定为slot13/Level2/asymm/七项权限；Table 34和物理Key ID 0～15固定。baremetal只派生同一方案基线。
- 未受上述冲突影响的启动阶段职责、eHSM command/transport 分层、Header Overlay/typed-stage loader职责、RAM安全不变量和安全路径fail-close设计可以继续推进。

## Wing-M130/eHSM BL边界

- Wing-M130/eHSM BL默认保持Vendor业务基线；eHSM依据自身eFuse自主判断并执行自检，BootROM不得发起，只消费ready/error和raw自检结果。项目工作是冻结配套版本、配置、ready/error、自检结果消费、Mailbox和升级边界，并完成NGU800P集成。
- 只有经Evidence确认的NGU800P配置/接口缺口、版本不配套、安全缺陷或Vendor正式变更才允许提出BL变更任务，不得因C908开发而复制或重写Vendor BL业务流程。

# 安全规则

- 不推断未记录的 OTP、生命周期、复位、密钥或调试行为。
- ADR-0026规定制造接口必须使用独立Provisioning FW、canonical CBOR/COSE signed recipe和批准的typed object/operation集合；禁止向Controller/Host暴露raw OTP offset、raw eHSM command、caller自选slot/usage/last_key或任意内存访问。进入USER后制造Provisioning命令必须由LCS和构建双重不可达。
- Vendor 代码是实现证据，不是正式规范。
- Vendor RTL是eHSM内部当前实现的第一查询证据，但仍是`VENDOR_IMPLEMENTATION`而非产品规范或硅片确认；SRC-0022继续定义SoC地址/寄存器/IRQ数值，SRC-0017/SRC-0016及accepted裁决继续定义产品目标。
- SRC-0035含Key/KEK字面量。不得把数值复制到文档、日志、测试Expected、软件、KMS/MES或对外材料，不得用它们宣称一机一密；未确认保密授权前不得外发或上传第三方服务。
- 未经批准不变更密码算法、Key Slot、Anti-rollback 或 Recovery 策略。
- 不绕过失败验证；资料、代码、RTL 结论和实测冲突时停止扩散修改并报告。
- ADR-0006规定：NGU800P Mailbox的command/response、packet及note/interrupt机制follow Vendor eHSM；NGU800P只实现MMIO/channel、地址转换、cache/barrier、timer、critical section和错误上报等port适配，不另造第二套协议。
- ADR-0009规定：涉及SoC地址、寄存器和IRQ硬件常量时先查SRC-0022并记录宏名、domain和生成版本；地址值存在不代表wrapper/接口语义已确认，语义冲突必须升级。`baremetal`测试流程和stub仍不属于这一权威范围。
- CE-SEC-010确认当前eHSM IRQ1～16为APLIC domain 9、source 78～93（编码`0x0009_004E～0x0009_005D`）。首版poll不要求猜测IRQ与Vendor channel映射；未来启用interrupt前必须取得逐channel权威映射并完成切换验证。
- ADR-0010规定：Vendor交付目录保持只读，NGU800P只通过项目侧custom header、port函数、外部build和adapter集成，不修改/fork Vendor公共代码。BootROM/FMC/GSP首版全程固定poll；Vendor FW ready后的interrupt仅为后续独立优化，必须重新完成设计变更和切换门禁，不阻断当前详设或发布。
- ADR-0010规定：RAS未ready时，安全stage提交静态最小错误记录、阻断release、撤销权限并尽力清零，在有限deadline内只重试RAS上报；超时后记录`RAS_REPORT_UNAVAILABLE`、关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出，不自行reset或继续启动。
- ADR-0011规定：首版BootROM/FMC/GSP各使用一个64字节对齐、256字节静态Vendor context slot且每stage最多一条在途事务；GSP所有task经唯一eHSM service串行化。首版只使用one-shot typed API，不启用流式API、不分配`ehsm_session_st`。Vendor公共poll只有send前cache钩子，adapter须安装active cache/timeout scope并保证在途no-touch；首版所有eHSM operation自动retry为0，timeout视为acceptance unknown并quarantine整个service/slot。Context arena最终只允许`NON_CACHEABLE`或`HARDWARE_COHERENT`，待SoC稳定后裁决；所选属性不能满足Vendor内部rsp可见性时升级兼容性问题，不得修改Vendor公共代码。
- CE-SEC-010确认当前C908无条件启用I/D cache，但未发现当前M-mode BootROM/FMC/GSP对2 MiB RAM配置PBMT/PMA，也未发现本项目Firewall/IOPMP实例绑定。不得把`CONFIG_XUANTIE_SVPBMT`、PMP或通用IOPMP驱动的存在解释为non-cacheable、coherent或Firewall已配置；平台证据未到齐前不得实现真实shared context或最终Firewall。
- ADR-0015规定：GSP第一个、最高优先级的`security_service_task`从bootstrap到runtime持续作为唯一eHSM service/context Owner，完成启动门禁后同一task进入长期service loop，不发生Owner移交。其他task只能通过typed queue请求，不得直接调用Vendor Host/Mailbox或取得raw handle；quarantine后不得通过切换Owner或同boot热重建恢复，只允许RAS批准的新boot instance重新初始化。OPEN-DESIGN-010资料未到齐前不得猜测PMP/RMP/MMP加载顺序、release接口或Expected。
- ADR-0016规定：旧OMP源码、linker和构建痕迹只作为GSP迁移/复用调查输入，不能生成独立OMP产品产物、registry identity、RAM Region、Measurement、counter、release或测试Expected；需要复用的功能必须逐项归入GSP模块和Feature追溯。
- ADR-0020规定：BootROM失败不release且RAS不可用时关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出；FMC只对确定可清理且无不可逆副作用的接收失败重新arm；GSP单Runtime失败只隔离对应Runtime及依赖者；timeout/状态未知quarantine且零自动retry。security只请求RAS动作，不直接reset。
- ADR-0020规定：设备/SKU/镜像的算法Profile、Vendor Image Type、key域、boot/upgrade key、board/LCS绑定必须来自单一provisioning/release matrix，实际行缺失时默认拒绝。Lifecycle/Debug采用typed challenge/token且只控制一个无scope的SoC全局Debug开关，并强制按时限/reset/LCS变化/安全错误关闭；更新只写inactive并由下次安全启动最终判定；Die0是Multi-Die安全根。
- ADR-0020/0026规定：本版本使用静态X.509证书链，软件保留P-256/SM2两套能力但每设备只选择一个长期Attestation Profile，Cert0/Cert1为该单一Profile的A/B链；具体SPDM wire由OPEN-DESIGN-015继续管理，完整Profile前wire实现为`DESIGN_BLOCKED_BY_PROFILE_INPUT`。产品/EMU禁止stub和全部test资产，mock只允许在构建隔离的host-unit target并证明未进入产品。云天定制Key Rotation设计已冻结但实现为`BLOCKED_BY_VENDOR_DELIVERY`；OPEN-DESIGN-019继续等待registry/profile生成方式确认。
- ADR-0026规定：Cert0/Cert1各64 KiB并独立擦除块对齐，采用256B Header、0x1000 chain起始、commit-last、扫描最大有效sequence且禁止active pointer；精确Flash base/erase粒度、制造command数值/transport、KMS/CA/MES schema属于实施绑定，不得用临时常量或raw接口替代。
- ADR-0020规定：Feature 001～020必须形成Source→Design→Code→Case→Evidence闭环；高严重度缺陷关闭，环境缺能力标BLOCKED，waiver显式批准，流片前方案和release artifact必须签署。
- ADR-0021规定：SoC Key轮换只覆盖Verify/Encrypt/Debug三类且每类一次；SoC侧不得指定物理slot/raw Bitmap。GSP只提交内部typed `key_type`、固定48字节密文和授权引用；eHSM完成写新Key、证明、Bitmap、destroy旧Key后才返回成功，再由GSP请求平台reset。OTP/Bitmap状态未知不得清理、自动retry或回旧Key。当前SRC-0018通用安装接口拒绝USER/DEBUG且不得替代专用轮换命令；匹配Vendor定制Host/BL/FW到齐前不授权编码。
- 正式详细设计最终必须汇总到`docs/05-software-design/NGU800P安全软件详细设计.md`这一份可独立阅读的完整文档。专题文件可逐章生成，但批准内容必须合入主文档；不得用链接目录替代最终设计。详设应保持SRC-0016原则，任何可能改变原方案的内容必须标为`PROPOSED`或`CONFLICTING`并向负责人提出；开放项应原位和汇总双重记录。
- ADR-0006规定：BootROM/FMC/GSP及production eHSM adapter只检测、保存raw证据、阻断release并上报错误；reset/watchdog/局部复位/整机复位或不复位由RAS策略决定并执行。Vendor reset函数不得进入production普通timeout/retry路径。
- ADR-0006～0008及负责人最终地址裁决规定：安全RAM产品System Address基址为`0x1010_0500_0000`、总容量2 MiB，由版本化Region表统一分区并通过Firewall隔离Host及其他非信任master；local/remap `0x1000_0500_0000`不进入产品软件且不存在地址映射。PMP/RMP/MMP常驻；BootROM data/stack和FMC位于尾部启动复用区，退出后清零回收；SPDM/MCTP等协议栈计入GSP。eHSM作为受信任master可以访问整个2 MiB，不设置Region级Firewall限制，但adapter仍必须检查System Address、长度、active descriptor、生命周期、PMA和barrier；两个批准PMA候选均不做data clean/invalidate。Host不得直接访问明文、Measurement或执行区。独立Mailbox物理Region已取消；BootROM/FMC/GSP分别使用stage-local固定`EHSM_CONTEXT_ARENA`。首版禁止异步和流式API，timeout未闭环context不得放在普通函数栈上。

# 最终实现与测试依据

- `gsp-pmp-rmp-omp` 和 `baremetal` 当前已有的 test、stub、demo、synthetic package/measurement 及其 Expected 只能作为 `CODE_FACT` 和差距证据，不得反向定义目标设计、最终 test oracle、验收结果或发布条件。
- 最终实现与测试判定以有效的 `SRC-0017`、最新有效 `SRC-0016`、accepted ADR/amendment、批准的 Requirement/OpenSpec、最新版本化测试工作簿和目标环境 Evidence 为共同依据；出现明显冲突或边界不清时仍按冲突升级流程由负责人裁决。
- 涉及eHSM内部硬件Expected时必须绑定SRC-0035的具体RTL版本和实际elaboration配置；未完成匹配编译/仿真/EMU/FPGA/硅片验证的RTL观察不得标为测试通过或`CONFIRMED`。
- 原有 stub 仅用于早期软件栈流程测试。EMU 和产品 target 不得包含或调用 stub、simulated success、test key/cert/provider、未批准 hardcode、silent fallback 或其他可能掩盖最终落地风险的实现。能力未完成时必须不可达、禁用或 fail-close，不得伪造完成状态。
- 现有 runner/build 基础设施可以复用，但其 stub 行为和历史 Expected 不得直接继承；新的可执行测试必须由最新有效测试工作簿和批准方案派生。
- BootROM/FMC/GSP 可优先移植或复用 OSR Host 的通用业务代码，但必须经过 NGU800P port、构建隔离、项目 ABI、错误处理和安全门禁适配；OSR 固定地址、timer、reset magic 等平台实现不得直接视为可复用事实，实际改动量在实施调查中确认。
- DD-02必须区分两条路径：`baremetal`以case list完整覆盖SRC-0018 `ehsm_demo_test()`全部BL/FW/可选功能，command拼装无特殊差异时可复用，顶层执行follow baremetal规则；`gsp-pmp-rmp-omp`第一阶段主要以`bl_demo`能力为输入，只复用合适的底层函数和command格式，产品payload、任务串联、安全启动和SPDM调用以芯片安全软件方案及批准详设为准。两条路径分别立项，不得把Vendor Demo顶层流程移植成产品流程。
- 本节依据ADR-0003/0004/0024/0030；W0-R1-05当前不采用，不得创建Handoff专用ABI、SRAM或编码任务。跨阶段度量信息使用Measurement Table；Header Overlay load固定为baremetal权威头中的64位System Address且entry=load，C908/Header Overlay/loader/linker/eHSM共享descriptor不建立Local/System映射；错误使用统一错误/日志。只有出现具体必要性并重新批准后才可提出阶段交接机制。

# 完成规则

任务只有在批准范围内形成可审查的未提交工作区 diff、要求的构建和测试通过、RESULT 完整、追踪和 Evidence 已更新、剩余风险明确后才可标记完成。完成不要求也不允许由 Codex 产生 Git commit、push、MR/PR 或合并。
