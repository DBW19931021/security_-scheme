# Project Status

2026-09-02接受ADR-0034并更新制造灌装最终合同：BootROM仍只有安全/非安全两个顶层分支；`non_sec_boot=0`、DEV/MANU、Strap=0且Profile有效时，非安全分支选择`MANUFACTURING_PROVISIONING`并加载独立C908 Provisioning FW；`non_sec_boot=1`、LCS异常和其他非制造组合选择`RESTRICTED_NONSECURE`，不得访问制造接口。Vendor目标eHSM BL必须把C908视为不可信caller，提供typed query/install/generate/proof/finalize/LCS并验证硬件制造条件、LCS、signed ticket、设备和固定对象顺序；对象状态增加`PROGRAMMING_PARTIAL`和严格同材料恢复。设备在DEV完成最终配置安全启动预演，MANU只收口，`MANU→USER`最后提交。USER后不允许pin/`non_sec_boot=1`重开制造；若声明安全链失效后仍能烧写逃生位，须绑定独立强授权维修硬件路径。主详设、专题、OpenSpec和约束已同步；Vendor/RTL/产品代码和测试未修改，实施仍未授权。

2026-09-01登记SRC-0036并接受ADR-0033：NGU800P type 1 Native Header尾部Overlay固定为offset1008～1015 LE64 `load_addr`、offset1016～1019 LE32 `ngu_header_crc32`、offset1020～1023四字节零reserved。CRC采用CRC-32/ISO-HDLC并覆盖Header offset256～1015共760字节，发布工具先写CRC再执行Vendor签名；BootROM/FMC/GSP在preflight和Vendor PASS后的稳定Header上双检。CRC只做格式损坏筛查，不替代Vendor签名/CMAC、typed-stage policy或Code完整性验证。旧8字节零reserved包不兼容；主详设、接口、OpenSpec、测试约束与BootRom镜像头说明已同步，产品代码和制包器实现仍需单独授权。

2026-08-26登记SRC-0035并接受ADR-0032：`source-vault/vendor_rtl`成为eHSM内部硬件实现细节、当前RTL实现基线和问题查询第一入口。当前快照共121个RTL文件、5733091 bytes，树哈希`ab0b2030a50e4ea0cf832dcb01c9eaa00df56816c8bc081b90549cca5027e95b`，顶层候选`osr_ehsm_top`，OSR头版本`v1.1.0-a / 4019 4_1_0_dev_Intellifusion`。直接结论保持`VENDOR_IMPLEMENTATION`；后续调查必须锁定hash并证明top/filelist/define/parameter/generate/wrapper/elaboration，不能外推NGU800P SoC连接、产品策略或硅片行为。目录只读且含Key/KEK字面量，禁止复制、外发或用作量产Key。当前缺正式delivery note、filelist、外部define和构建/仿真环境；本次未修改RTL、软件或测试代码。

2026-08-25登记SRC-0034并接受ADR-0031：eFuse新增1 bit `non_sec_boot`，默认逻辑0不覆盖既有LCS×`secure_boot`矩阵，可靠烧写/读取为1后由BootROM最高优先级强制进入现有受限非安全启动，包括USER。BootROM只消费复位锁存、只读、带valid/ECC/镜像状态的逻辑快照，不获得raw eFuse访问；输入异常阻断安全和非安全FMC release。准确word/bit/编码、只读视图、复位时序和烧写/锁定Owner由OPEN-DESIGN-024管理；未绑定时产品port为`BLOCKED_BY_NON_SEC_BOOT_BINDING`。主详设保持最终方案表达，未修改产品代码、RTL、烧写工具或测试工作簿。

2026-08-21登记SRC-0033/CE-SEC-016并接受ADR-0030：NGU800P type 1 SoC固件删除128B NGU Manifest，包固定为`Native Header[1024] + Code[Code_Size]`；复用Header `Public_Key_Ext`末16B，其中offset1008～1015为LE64 `load_addr`、offset1016～1023保留且必须为0。`Code_Size`成为签名/加密/搬移/Measurement唯一长度，`entry_addr=load_addr`；image/instance/die/Profile/key/LCS/policy由受信typed-stage registry和provisioning matrix提供。旧Manifest包、fallback和dual parser均无兼容义务。主详设第3/5～9/12～16章、Package/Loader/ABI/RAM及BootROM/FMC/GSP专题已同步；产品代码、制包器和E2E实现仍未授权。

2026-08-20已在登录状态下逐行核对飞书`Security`最终表，并将安全测试开发基线从本地v5的76条切换为在线63条：47条Mailbox BASIC、7条eHSM负向、7条SoC软件/协同、2条EDA/硬件协同；软件/BSP主导编号为`001～061`，`062～063`由RTL-DV/EDA构造真实故障。工程内v5继续作为历史快照。开发指导新增两项显式门禁：Debug鉴权必须先区分`SOC_DEBUG`与`EHSM_DEBUG`；Case 060只证明启动核成功重配SRAM五Region，不能据此宣称非启动核/eHSM配置写拒绝已覆盖。全部Case仍为`NOT_EXECUTED`，未修改飞书、baremetal或eHSM固件。

2026-08-19登记SRC-0032并接受ADR-0029，将DICE风格一级动态证明合入主详设：BootROM只提交FMC最终readback Hash，FMC只提交GSP Hash，二者不增加UDS/CDI/KDF/X.509栈；GSP验证两个Measurement Entry后构造96字节TCB Context，经eHSM以slot13 UDS不导出派生CDI/Alias Key，组装动态Firmware Alias Leaf并由slot14 Device Attestation Issuer签发，Alias Key签Report/SPDM transcript。Cert0/Cert1改存Root→Intermediate→Device Issuer三张静态前缀，SPDM Slot0运行期拼接动态Leaf。Measurement 16 KiB不复用，GSP新增动态证明峰值门禁12 KiB。准确eHSM command/Key Attribute、企业OID/DN/有效期和SPDM wire转OPEN-DESIGN-023/015管理；未修改产品代码、linker或Vendor固件。

2026-08-19登记SRC-0031并接受ADR-0028，冻结2 MiB安全SRAM精确布局：前1 MiB依次为GSP静态区880 KiB、FMC启动复用区128 KiB和Measurement固定区16 KiB；中间512 KiB分别归功耗核PMP和RAS核RMP各256 KiB；末尾512 KiB为Host/Device完整密文包共享窗口；MMP主要驻留DDR，不占本2 MiB常驻Region。布局取消独立plaintext Region，FMC/GSP/PMP/RMP改为Host ingress与尚未运行目标Region分离的受控原地加载。FMC退出后其128 KiB经撤权、清零和回读后才可加入GSP动态池，Measurement 16 KiB当前只保存Measurement，未使用部分不得被GSP复用。精确PMA/Firewall绑定、MMP DDR carveout/release以及release link map峰值验证仍开放；未修改产品代码、linker或baremetal。

2026-08-14形成项目组简版《NGU800P eHSM、Firewall与SEC_CFG总体介绍》：基于本地最终表v5核对76条Case及52/14/7/3分类，汇总三个模块在SoC中的位置、默认安全状态、寄存器分组、集成关系和测试范围；明确Firewall数据访问权限与配置Owner分离，并保留`OPEN-CONFLICT-014/015`数值绑定边界。全部Case仍为`NOT_EXECUTED`，未修改baremetal、eHSM固件或产品代码。

2026-08-14登记SRC-0030并完成Firewall配置Owner修正：SEC_CFG、SPIFC和SRAM三个Firewall均只有启动核可配置，eHSM和其他核配置写均应被阻断。配置Owner判定并入软件Case 072，真实eHSM不可达部分由条件性硬件Case 076补齐；数据访问矩阵保持SRC-0029结论。最终表更新为v5，全部仍为`NOT_EXECUTED`，未修改baremetal或eHSM固件。

2026-08-14登记SRC-0029并完成SEC_CFG/SPIFC Firewall默认权限修正：两个目标默认均仅允许启动核，eHSM和其他核拒绝；SRAM仍允许启动核和eHSM并覆盖五个Region。软件Case 069/070及条件性硬件Case 076、相关需求/说明/矩阵已同步，最终表更新为v4。全部仍为`NOT_EXECUTED`，未修改baremetal或eHSM固件。

## Current phase

工作空间基础设施、方案/Vendor/RTL/测试资料登记和安全测试v0.2审视已完成。SRC-0035已经成为eHSM内部硬件实现第一查询源，RTL证据规则已建立；当前仍缺正式elaboration输入和动态验证。任务二单一主详设第1～16章及附录已形成。复杂PKI继续放在Host/KMS/离线CA，Device不构造PKCS#10、不解析通用X.509。ADR-0029进一步把证书拓扑冻结为静态Root→Intermediate→Device Issuer前缀加GSP一级动态Firmware Alias Leaf；OPEN-DESIGN-022设计裁决关闭，准确eHSM/Profile绑定转OPEN-DESIGN-023。未修改两个代码仓或Vendor RTL快照，未执行Git或安全测试。

2026-08-12登记Firewall截图输入SRC-0025，补齐SRAM、sec_cfg、spifc的架构、16个UserId/28个authority位、默认值、候选CSR和错误诊断。UserId/authority已形成机读实现输入；CSR基址、复位、Region默认窗口等与SRC-0022未完成绑定，统一由OPEN-CONFLICT-014阻断。49条细化测试和评审工作簿已形成，均为`NOT_EXECUTED`；未修改产品代码或baremetal。

2026-08-12继续登记SEC_CFG内网整理稿SRC-0026，12张截图及逐项转录已归档；17个32-bit寄存器、status/hardware-error位、UID/Debug观察模型、软件接口、主详设第5.9.2节和12条测试已形成。截图地址`0x1010_07B0_0000/4 KiB`与SRC-0022生成头`0x1010_0820_0000/16 KiB`冲突，且当前baremetal HSM_STATUS0映射仍为无效/地址0；FW error、LCS编码、Debug合成、UID-valid和64-bit一致性共同由OPEN-CONFLICT-015阻断。所有SEC_CFG测试为`NOT_EXECUTED`，未修改产品代码或baremetal。

2026-08-13登记SRC-0027并完成SoC安全方案测试用例收敛。v0.3.8保留52条Mailbox BASIC和14条eHSM负向，SoC收敛为7条软件/协同Case：SEC_CFG全部17寄存器、`dbg_en_cfg`从Die Debug、SEC_CFG/SPIFC默认权限、SRAM默认权限、SRAM重新配置和Mailbox IRQ；Codex移植指导合计73条。`soc_dbg_en_out`已加入Mailbox Debug Auth成功/失败和`CLOSE_DEBUG`判据。另列2项EDA必测错误中断和1项条件性eHSM Master权限补充，不进入软件移植计数。全部为`NOT_EXECUTED`，未修改baremetal或eHSM固件。

2026-08-13登记SRC-0028并参考mailbox/spinlock测试交付方式形成安全测试最终格式版。单一`Security`工作表固定13列，连续编号`NGU800P-D0-SECURITY-001～076`，包括52条Mailbox BASIC、14条eHSM负向、7条SoC软件/协同和3条EDA/硬件需求；后续Codex只移植`001～073`。同步形成SoC总体架构、eHSM/SEC_CFG/Debug/Firewall/IRQ/错误上报功能及Case说明。工作簿分段目视检查、重新导入范围核对和公式错误扫描已完成；全部仍为`NOT_EXECUTED`，未修改baremetal、eHSM固件或产品代码。

2026-08-13复审最终表Mailbox“输入”列，确认旧版对52条BASIC和14条eHSM负向使用了过度通用模板。现已逐命令改写实际API、关键参数、调用顺序、算法矩阵及命令特有的读回/恢复/删除动作；`get_version`和`get_challenge`等无副作用命令不再包含无关算法循环、资源清理或通用健康查询。自动审计确认`001～066`连续、66条输入互不重复、旧模板关键句命中0项；全部仍为`NOT_EXECUTED`。

2026-08-13继续复审最终表“通过准则”列，已将52条Mailbox BASIC、14条eHSM负向、7条SoC和3条EDA共76条准则逐条收敛为命令/功能直接判定条件；公共PASS/FAIL/INCONCLUSIVE及Host本地拒绝边界统一移入说明文档。自动审计确认76条均非空、旧通用模板命中0项、最长84个字符；修订表为`NGU800P_Security_Case_Table_Final_v2.xlsx`，前一版保留为历史，全部仍为`NOT_EXECUTED`。

2026-08-14继续复审最终表“前置条件、输出、测试目的”。66条Mailbox Case均明确eHSM正常启动及BL/FW阶段，并将“service channel 1/poll/100 ms合同有效”改写为服务通道1、轮询等待和单次命令100 ms超时配置；76条输出和目的逐Case重写，删除家族输出模板和“允许副作用”笼统说法。自动审计确认三类字段均非空、Mailbox阶段全部匹配、旧模板及“合同”命中0项；该轮表为`NGU800P_Security_Case_Table_Final_v3.xlsx`，现由v4替代，全部仍为`NOT_EXECUTED`。

## Active security topics

已完成Vendor文档、方案基线、代码快照和历史Review登记。证书方向现由ADR-0029冻结为DICE风格单层动态证明；待绑定主题包括slot13 UDS KDF/确定性Alias KeyGen command与Key Attribute、P-256/SM2动态证书OID/DN/有效期/serial/SM2 ID、Flash base/erase粒度、KMS/CA/MES接口，以及既有eHSM MMIO/PMA、Counter、Runtime和SPDM wire Profile。

eHSM内部硬件查询现统一先使用SRC-0035，并按RTL调查规则记录树哈希、实例链、配置和逻辑锥。可由当前实际elaboration直接回答的问题不再重复打开；缺filelist/define/wrapper、涉及SoC产品连接、来源冲突或需要仿真/硅片证明的事项继续显式开放。

Firewall专题已纳入主详设第5章和架构目录。软件可直接使用`requirements/firewall-userid-authority-map.yaml`实现UserId校验和authority位图；不得从截图占位base、矛盾reset或重叠Region默认值生成量产MMIO/policy。

SEC_CFG专题已纳入架构、接口、主详设和验证目录。软件可以使用`requirements/sec-cfg-register-map.yaml`和`requirements/sec-cfg-requirements.yaml`构建参数化模型、raw snapshot和测试桩；不得使用冲突base、地址0占位、未定义FW error/LCS/Debug语义生成产品接口。

## Active OpenSpec changes

- `bootrom-non-sec-boot-efuse-override-v1`：SRC-0034/ADR-0031已冻结默认0和值1强制`RESTRICTED_NONSECURE`；ADR-0034区分DEV/MANU制造与受限非安全子Profile。准确RTL/eFuse/Provisioning/USER维修绑定等待OPEN-DESIGN-024，implementation未授权。
- `dice-single-dynamic-attestation-v1`：SRC-0032/ADR-0029已冻结BootROM/FMC最小测量、96B TCB Context、UDS/CDI不导出、GSP一级动态Leaf、Alias Report签名和12 KiB峰值；实现等待OPEN-DESIGN-023/015。
- `security-ram-layout-v2`：SRC-0031/ADR-0028已冻结2 MiB精确offset/size、FMC回收、Measurement 16 KiB、PMP/RMP各256 KiB、Host ingress 512 KiB、受控原地加载和MMP DDR方向；PMA/Firewall、MMP DDR Profile与release link map峰值Evidence仍阻断编码。
- `native-header-load-address-v1`：SRC-0033/0036、ADR-0030/0033已批准无Manifest包格式、offset1008 load、offset1016 Header CRC、offset1020零reserved、`Code_Size`唯一长度、`entry_addr=load_addr`和typed-stage policy；implementation未授权。
- `secure-package-manifest-v1`：`SUPERSEDED`，仅保留历史追溯；不得承接新实现或测试任务。
- `soc-key-rotation-v1`：设计已批准，implementation未授权；等待Vendor定制Host/BL/FW、物理slot/bit、掉电/KMS/recipe后才能进入编码。
- `measurement-table-abi-v1`：逻辑storage ABI已由ADR-0022批准；ADR-0028固定物理Region为16 KiB、理论最多126个Firmware Entry。产品实际`max_fw_entries`、PMA/Firewall和SPDM wire仍待输入，implementation未授权。
- `bootrom-mode-and-ehsm-self-test-v2`：ADR-0018/0024/0034已批准；Strap合同为`boot_pin.secure_boot[3]`、0非安全/1安全，DEV/MANU的0选择制造子Profile，其他非安全原因选择受限子Profile；eHSM按eFuse自主自检。真实Strap绑定仍`BLOCKED_BY_RTL_SYNC`，implementation未授权。
- `device-personalization-and-provisioning-v1`：ADR-0026/0029基线继续有效；ADR-0034增加非安全制造子Profile、BL typed制造接口、部分写恢复和USER最终提交，实施等待OPEN-DESIGN-025及Vendor/RTL/Flash/KMS/CA/MES/wire绑定。

## Active implementation tasks

顶层任务仅有`TASK-SEC-BAREMETAL-EHSM-001`和`TASK-SEC-SOC-FW-001`。`TASK-SEC-DD-001`、`TASK-SEC-DEV-PLAN-001`和`TASK-SEC-EMU-PREP-001`保留为从属工作包/阶段追溯；代码证据已形成`CE-SEC-001～014`。任务二主体详设和编码前落实性分级已经完成，公共纯逻辑、平台绑定、Vendor绑定和延期功能已分开；产品编码尚未授权。任务一当前在线最终表共63条：47条Mailbox BASIC、7条真实到达eHSM的负向用例、7条SoC软件/协同用例和2条EDA/硬件需求；Codex开发范围固定为前61条。Host本地负向为0；eHSM固件和硬件均为DUT。历史工作簿全部保留。所有用例禁止eHSM固件定制；Vendor Demo二级组合、v0.2逐项对账和评审仍待完成，尚未授权本轮修改`baremetal`。

## Blocked items

- 当前 Codex Windows PATH 中未发现 OpenSpec CLI，无法读取版本或执行本地原生校验。
- Vendor 文档适用的 eHSM/Core 修订、版本对应关系，以及两份内部方案的正式 Owner/批准版本仍待确认。Vendor 已确认不包含 SoC 范围。
- SRC-0018 缺少正式交付单、原始压缩包哈希和明确责任人，且 Bootloader 目录后缀与 `SW_changelist.md` 不一致。
- SRC-0035已完成静态登记，但缺正式delivery/release note、filelist、外部define/parameter、wrapper/库、约束及匹配构建/仿真Evidence；这些输入到齐前只能按明确源码候选配置形成`VENDOR_IMPLEMENTATION`结论。RTL授权/保密等级仍待确认，当前禁止外发。
- SRC-0019 中 1 条候选发现已复核并完成裁决，剩余 16 条仍待重新验证；未复核项不能作为正式设计结论。
- OPEN-CONFLICT-014未关闭：Firewall CSR基址、reset、Region默认窗口、R4名称、bit27标签、48-bit高位/安全属性/lock/burst仍需目标D0 RTL/CSR和DV Evidence。
- OPEN-CONFLICT-015未关闭：SEC_CFG目标D0 base/size、FW error位、LCS编码、Debug scope/lock/认证合成、UID-valid和跨32-bit一致性仍需RTL/CSR、eHSM BL/FW、Security/eFuse及DV Evidence。
- eHSM BL/Host自检位图差异已按Vendor回复和负责人裁决关闭：采用Bootloader定义，bit18/`0x40000`=`TRNG`；Host定义错误，bit19/`0x80000`保持unknown/reserved。原始Vendor回复材料仍待按Source流程补录，但不再阻断设计/用例093下一版本更新。
- SRC-0020/v0.2 用例尚缺逐项 Owner、RTL/软件版本、执行载体、自动化状态和实际 Evidence，不能作为流片前已通过结论。
- EMU 首个可用日期、RTL/eHSM/BootROM baseline、加载/日志/复位/故障注入能力和不可逆资源策略待环境团队确认；未确认前只能完成 runner 骨架，不能冻结实际执行命令。
- `baremetal` 的当前分支、只读 baseline commit、remote 和既有未提交修改尚未登记；本次遵守“无 Git 操作”约束，只记录为 UNKNOWN。
- OPEN-CONFLICT-004已关闭并由ADR-0022/0030更新：Measurement `load_addr/entry_addr`均为`uint64_t SOC_PA`且不带domain，只作loader成功执行后的审计快照；Header Overlay只存LE64 `load_addr`，typed-stage loader保持执行地址Owner，`entry_addr=load_addr`。
- OPEN-CONFLICT-005/009已由ADR-0019关闭：旧差异是32位而非32字节；产品唯一使用`rollback_counter[16]`。BootROM以`check_version=0`验证type 1 FMC并由BL暂存认证candidate；FMC初始化从有效FMC Measurement回传expected candidate，BL exact-match后compare/commit/readback；proof成立后才接收同值GSP。通用64位Counter、raw OTP、GSP值和FW启动副作用均不得替代。
- OPEN-DESIGN-004/011已关闭：ADR-0017冻结三套Secure Package算法Profile及ID 1/2/3，所有产品构建保留三条路径且三套全部通过完整启动链、更新链和负向测试；ADR-0020批准单一provisioning/release matrix。实际设备/SKU配置行尚需Product/Provisioning/KMS/Release Owner补齐，缺少精确匹配时默认拒绝。
- ADR-0020已关闭OPEN-DESIGN-003/013/016/017/018/020；失败终态、Lifecycle/Debug、更新/OOB/Recovery、Multi-Die、统一RAS/审计/清零和流片前发布原则均已冻结，精确硬件数值/Owner名单作为实现或验证输入。
- OPEN-DESIGN-015保持开放：动态Alias证书`KEY_EXCHANGE -> FINISH`、拒绝PSK、Measurement summary和每设备单Profile已冻结；transport、完整虚拟链/消息上限、block和session wire冻结前状态为`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
- OPEN-DESIGN-019进一步裁决：产品/EMU no-stub、test资产禁止和host-unit mock隔离已批准；registry/profile固定采用四个分类权威YAML源和统一生成器，输出C header、linker include、制包/测试schema及`security_abi_registry.json`，不使用巨型配置。正式toolchain/flags、KMS、SBOM、兼容矩阵和发布Owner仍待输入。
- OPEN-DESIGN-007已关闭：ADR-0022批准128字节Header、实际数量128字节紧凑Firmware Entry、唯一128字节SoC State，`total_len=256+fw_entry_count×128`；BootROM不建普通Entry；删除`state_entry_count/generation/eHSM状态/domain/key_id/signer_id/时间戳`；每结构只留一个reserved并保留CRC-32C/32位commit。ADR-0028固定物理16 KiB和126项硬上限；OPEN-DESIGN-021继续收集最大独立实例数以冻结产品实际`max_fw_entries`。
- OPEN-DESIGN-014的软件设计已由ADR-0026/0034关闭：三类SoC Key轮换、一机一密、16槽、Root顺序、单Profile、UDS、Cert0/1、DEV/MANU非安全制造、partial恢复和USER最后提交已冻结。Vendor BL制造接口由OPEN-DESIGN-025跟踪，其他外部事项继续作为实施绑定。
- OPEN-DESIGN-022已由ADR-0029关闭：采用Root→Intermediate→Device Issuer三张静态前缀加GSP动态Firmware Alias Leaf，保留固定PoP、signed install ticket、Cert0/1 A/B和SlotID 0；Device继续不解析通用X.509。
- OPEN-DESIGN-023已建立：等待slot13 UDS域分离KDF、确定性P-256/SM2 Alias KeyGen、opaque handle/sign准确command和Key Attribute，以及动态Leaf企业OID/DN/有效期/AlgorithmIdentifier/SM2 ID/TCB扩展编码。
- OPEN-DESIGN-024已建立：`non_sec_boot`逻辑策略已定；准确eFuse word/bit/编码、ECC/valid/镜像、复位锁存、BootROM只读视图、DEV/MANU烧写/readback/lock Owner和USER独立维修入口仍待目标D0资料，关闭前产品port保持`BLOCKED_BY_NON_SEC_BOOT_BINDING`且不得声明USER事后逃生。
- OPEN-DESIGN-025已建立：等待Vendor eHSM BL typed capability/identity/query/install/generate/public-key/fixed-PoP/proof/finalize/LCS接口，以及硬件制造授权、接受点、`PROGRAMMING_PARTIAL`、readback和掉电恢复合同；到齐前量产Provisioning保持`BLOCKED_BY_VENDOR_DELIVERY`。
- OPEN-CONFLICT-011已建立：当前SRC-0018只有通用安装接口且拒绝USER/DEBUG，不能满足云天定制专用轮换机制；等待匹配Vendor定制Host/BL/FW、专用command和release note，产品轮换实现保持`BLOCKED_BY_VENDOR_DELIVERY`。
- OPEN-CONFLICT-012目标已批准但实现待绑定：量产RTL Key必须按die唯一，Vendor共享宏不能作为量产值；隐藏个性化载体、Secure ATE、CPU/Debug/Scan不可读、lock和operation proof到齐前保持`BLOCKED_BY_RTL_BINDING`。
- OPEN-CONFLICT-013已关闭并由SRC-0024/ADR-0034纠正：设备保持DEV完成Chip Root→slot1～5 Level1→目标Profile Level2（slot6/7/12轮换位置保持blank）→证书→最终配置产品安全启动预演；MANU只收口，MANU→USER最后提交。slot14单Profile，UDS固定slot13/Level2/asymm/七项权限。
- OPEN-CONFLICT-006已按SRC-0031/ADR-0028进一步部分关闭：C908、Header Overlay、loader、linker和eHSM共享descriptor只使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`的System Address；`0x1000_0500_0000`只保留为RTL历史事实。2 MiB精确Region offset/size已经冻结，MMP改驻DDR；剩余PMA属性、Firewall CSR/粒度/窗口绑定、MMP DDR Profile及各镜像release link map/栈堆峰值Evidence仍阻断最终linker和精确Expected。
- OPEN-CONFLICT-007已关闭：ADR-0010采用Vendor direct 16×4 KiB布局且Vendor公共源码不修改；SRC-0022的4 KiB/84-message通用Mailbox不用于eHSM。CE-SEC-010确认direct aperture/status准确宏仍未在SRC-0022定位，作为RTL/地址头同步门禁阻断真实MMIO；16路IRQ已确认是APLIC domain 9、source 78～93，首版poll不需要猜测IRQ与Vendor channel映射。
- OPEN-CONFLICT-008/OPEN-DESIGN-008原按ADR-0014关闭；其中Manifest方案已由ADR-0030/0033替代。当前type 1包无Manifest，Header offset1008为LE64 load、offset1016为LE32 Header CRC32、offset1020为4B零reserved，Code从1024开始且长度唯一为`Code_Size`；旧Manifest和旧8B零reserved包必须拒绝并重新生成。
- OPEN-DESIGN-012产品策略已由ADR-0018/0024/0031/0034关闭：值1强制`RESTRICTED_NONSECURE`；值0时USER强制安全，DEV/MANU+Strap0选择`MANUFACTURING_PROVISIONING`，其他非安全原因选择`RESTRICTED_NONSECURE`，Strap1安全；两个子Profile无fallback且无安全Measurement/审计。OPEN-CONFLICT-010仍为`BLOCKED_BY_RTL_SYNC`。
- C908主Makefile构建链会纳入common cache/timer/IRQ原语；三个CDK工程仍指向不存在的`chip_riscv_dummy`，必须在编码前修复/重新生成并验证source list一致性。

## Open hardware questions

见 `requirements/open-questions.yaml`。当前Vendor资料已明确的eHSM/Core内禀特性不重复列为开放问题；只保留NGU800P SoC绑定、项目策略、资料冲突或资料缺失项。

## Recent confirmed decisions

- `security_-scheme` 本身作为安全工程和工作流的唯一 Git 仓库，不创建嵌套 Git 仓库。
- `gsp-pmp-rmp-omp` 保持独立 Git 历史，只通过清单和任务关联。
- 安全软件正式详设及评审结论存放在`security_-scheme/docs/03-architecture/`、`docs/04-interfaces/`和`docs/05-software-design/`，必须达到可直接指导开发的粒度；BootROM/FMC/GSP产品代码实现进入`gsp-pmp-rmp-omp`。
- eHSM全接口/全算法能力验证由`baremetal`独立软件栈承载并运行在安全核上，不属于GSP固件task/service。测试策略、计划、版本化工作簿和case规划存放在`security_-scheme`；可执行能力case、EMU runner/tools和必要测试数据进入`baremetal`。GSP产品固件只实现安全软件方案明确需要的eHSM能力。
- 所有项目级任务规划和开发追溯只在 `security_-scheme` 管理，不在两个代码仓新增第二份主计划；已有历史文档不在本次迁移或删除。
- SRC-0017 是系统/架构上位方案；SRC-0016 是依赖它的软件工程落地下位方案。
- Vendor 仅指 eHSM 及其内部 Core，不包含 SoC；采纳的 Vendor 建议必须进入芯片安全软件方案。
- 密钥轮换策略已批准；ADR-0021已把SRC-0015机制作为受控软件方案增量，ADR-0026关闭软件设计裁决；剩余Vendor/RTL/Flash/KMS/CA/MES事项按实施绑定管理。
- 本工程 Vendor 文档不保密；Vendor 代码按交付快照管理，不逐文件登记。
- SRC-0035是eHSM内部RTL实现细节、当前实现基线和问题查询第一入口；结论保持`VENDOR_IMPLEMENTATION`并要求实际elaboration证据。它不覆盖SRC-0022 SoC数值、系统/软件方案或匹配环境Evidence；目录只读且Key字面量禁止复制/外发/量产使用。规则见ADR-0032。
- 上述规则见 `decisions/ADR-0001-solution-authority-and-vendor-boundary.md`。
- SRC-0016/SRC-0017 PDF 保持受控原始基线；派生工程结论按需写入目标 docs，不复制 PDF 全文。
- 方案与 Vendor 文档/代码、测试或实测明显冲突时，必须在当前任务内提出并等待负责人裁决；规则见 ADR-0002。
- W0-R1-01～04、06～10已批准；BootROM/FMC/GSP以移植/复用OSR Host通用业务代码为主，但NGU800P port、构建、ABI、错误和安全门禁独立落实。
- DD-02移植边界已由负责人细化为双路径：`baremetal`完整覆盖Vendor `ehsm_demo_test()`的全部BL/FW/可选功能并形成case list，command拼装无特殊差异时可复用且顶层follow baremetal；`gsp-pmp-rmp-omp`第一阶段主要使用`bl_demo`能力，只复用底层函数/command格式，产品payload、安全启动和SPDM编排以软件方案为准。
- W0-R1-11已批准：现有代码仓test/stub/demo/synthetic flow及历史Expected只作为`CODE_FACT`和差距，不是目标设计、最终oracle、验收或发布依据；EMU/产品禁止stub、simulated success、test key/cert/provider、未批准hardcode和silent fallback。完整记录见ADR-0003。
- ADR-0004已接受并由ADR-0030更新：Die1在内部Measurement Table独立记录`NGU_FW_TYPE_DIE1_FW`/`die_id=1`；Header Overlay/loader直接使用baremetal System Address，不进行Local/System转换；R1-05当前不采用，旧GSP绝对地址禁用。
- 工程顶层任务收敛为两项：`baremetal` eHSM测试Case开发，以及SoC安全固件详设/开发；历史DD/DEV/EMU/INV/TST记录均作为从属工作包管理。
- ADR-0005已接受并由ADR-0022更新最终ABI：Measurement `load_addr/entry_addr`保留为64位loader结果快照，但最终只存`SOC_PA`、不带domain；counter宽度以Vendor为准统一为16字节；Wing-M130/eHSM BL保持Vendor业务基线，只处理确认的集成缺口。
- ADR-0006已接受并由2026-07-28裁决更新：安全RAM产品System Address基址`0x1010_0500_0000`、大小2 MiB并由软件分区/Firewall隔离；NGU800P Mailbox机制follow Vendor；security只上报错误并阻断release，reset由RAS策略决定和执行。
- ADR-0007/0008的隔离、W^X、eHSM aperture和stage-local `.ehsm_context_arena`原则继续有效；旧P1容量拓扑已由SRC-0031/ADR-0028替代。现行布局固定GSP静态880 KiB、FMC复用128 KiB、Measurement 16 KiB、PMP/RMP各256 KiB和Host ingress 512 KiB；MMP主要驻留DDR，独立plaintext Region取消。
- ADR-0008/0011已接受并补充批准：eHSM可以访问整个2 MiB；独立Mailbox Region已取消，BootROM/FMC/GSP分别使用stage-local固定arena；首版只使用one-shot typed API，不启用异步/流式API、不分配`ehsm_session_st`，timeout未闭环context不得放普通函数栈。
- 当前NGU800P/4019 Mailbox基线为16个channel；Host port hook只校验active descriptor并保持baremetal 64位System Address数值不变，不建立Local/System映射，eHSM BL再使用Vendor内部remap window访问SoC。BootROM、FMC和GSP首版eHSM路径全程poll，ready采用错误优先并要求`hw_boot_done && bootloader_done`且相关错误位为0。
- 负责人确认eHSM访问SoC RAM时不携带/不强制可供软件依赖的non-cacheable属性；Mailbox MMIO固定non-cacheable/strongly ordered，共享RAM最终只允许`NON_CACHEABLE`或`HARDWARE_COHERENT`且两个候选均不执行data clean/invalidate。现有Vendor资料已经回答的eHSM/Core事实不得重复向负责人询问，除非资料更新、缺失或发生冲突。
- 当前C908软件基线的cache line为64字节；共享对象仍要求独占完整line，但两个批准PMA候选下不调用range data clean/invalidate，只保留barrier。timeout时间源采用64位单调微秒接口。`SECURITY_SUBSYS_MAILBOX`不得作为eHSM port base，OSR `0x80000000`也不得作为NGU800P fallback。
- ADR-0009已接受：涉及NGU800P SoC地址、寄存器和IRQ硬件常量时以SRC-0022 `baremetal` RTL同步生成头为第一权威源；GSP镜像和Vendor示例地址不得覆盖。该规则不扩展到baremetal test/stub/demo/Expected。
- ADR-0010已接受：Vendor公共源码不修改；BootROM/FMC/GSP首版全程poll，interrupt仅为Vendor FW ready后的后续独立优化；RAS未ready时保存静态错误记录、有限重试上报，超时关闭普通中断并进入无限WFI fail-stop循环。
- ADR-0011已接受：每stage一个64字节对齐/256字节静态context slot和最多一条在途事务；GSP唯一service串行化；Vendor无参数cache/timeout钩子由active scope驱动；首版零自动retry，timeout acceptance unknown并quarantine整个service/slot。
- ADR-0012已接受并由2026-07-28裁决完成Counter时序更新：BootROM直接读取SoC LCS，以`check_version=0`验证type 1 FMC并由BL暂存candidate，不读/比/写stored counter；FMC初始化exact-match commit/readback，proof后接收同值GSP。Vendor FW仍由GSP按type 0独立counter域加载，completion unknown仍quarantine。
- ADR-0013/0019：GSP镜像固定为eHSM FW/PMP/RMP/MMP；通用算法低优先级只向受信任内部SoC模块开放；不向Host开放GSP通用算法、Key/Certificate/Rotation、raw eHSM或其他安全服务。
- ADR-0014/0019/0024/0030/0033：Vendor原生`Image_Type`编码保持0/1/2/3，但产品SoC stage包只接受type 1并拒绝type 2/3；NGU Manifest已删除，Header offset1008存64位System Address `load_addr`、offset1016存LE32 Header CRC32、offset1020保留4B为0，rollback counter只来自Header 16B字段，entry等于load；版本与其他策略移到外部release metadata和受信registry/matrix。
- ADR-0017已接受：Profile 1=SHA-256+RSA-2048-PSS+AES-128-CBC，Profile 2=SHA-256+ECDSA-P256+AES-128-CBC，Profile 3=SM3+SM2+SM4-CBC；CBC按Vendor 16字节IV/no-padding，其他Vendor算法只作baremetal能力/回归输入，产品release拒绝。
- ADR-0020已接受并更新：关闭失败终态、Provisioning Matrix、精确相邻Lifecycle转换、无scope的唯一SoC全局Debug开关、A/B更新metadata、OOB、Multi-Die、RAS/审计/清零和流片前发布原则；SPDM固定证书握手并拒绝PSK、Attestation同时支持P-256与SM2，剩余wire参数在完整Profile前阻断；no-stub/test隔离已批准。OPEN-DESIGN-014后来由ADR-0021收敛为设计完成、Vendor交付阻断。
- ADR-0021已接受：SoC Verify/Encrypt/Debug Key每类只轮换一次，eHSM内部管理1字节Bitmap，使用48字节双层密文和USER鉴权，按写新Key→Bitmap→destroy旧Key→reset单向生效；通用安装接口不得替代，当前实现等待Vendor定制交付。
- ADR-0025/0026已接受：量产RTL Root/Install KEK按die唯一，KMS按设备托管句柄；OTP采用物理Key ID 0～15。单Device Private槽按每设备Profile二选一，不支持同机双长期Attestation身份。
- ADR-0015已接受：GSP第一个、最高优先级的`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner；其他task只能通过typed queue请求，bootstrap完成后不移交Owner，quarantine后只允许RAS批准的新boot instance重新初始化。
- ADR-0016已接受：GSP替代旧OMP/Q&P产品固件，OMP不再作为独立产品镜像、RAM Region、package、Measurement、counter或release对象；历史OMP代码只作为GSP功能迁移输入。
- 正式详设最终交付为一份可独立阅读的`NGU800P安全软件详细设计.md`；专题文件允许逐章生成，但批准的规范性内容必须合入主文档。详设保持SRC-0016原则，改变原则的候选内容必须显式提出裁决。
- Measurement→FMC rollback-counter合同已批准：BootROM在跳转前commit唯一FMC Entry及认证candidate；FMC初始化把它作为expected candidate传给BL，exact-match后compare/commit/readback；proof成立后才接收`check_version=0`且counter等于已提交值的GSP，再执行GSP Measurement commit→release。GSP不是第二个更新Owner。
- ADR-0022已批准逻辑ABI、ADR-0028已固定物理16 KiB，但不等于编码授权：真实实现仍需产品实际`max_fw_entries`、最终`NON_CACHEABLE/HARDWARE_COHERENT`属性、Firewall和32位commit跨master可见性Evidence；data clean/invalidate调用固定为0。
- ADR-0023/0024已接受：复杂设计优先补充图示，文档引用使用可辨识超链接并建立双向返回；BootROM不发起eHSM自检，eHSM按eFuse自主执行或跳过，BootROM只消费ready/error/raw结果。
- CE-SEC-014/016完成BootROM/FMC/GSP编码前及Header尾部审查：三个产品入口均未实现目标安全链；旧Manifest/Measurement/verify骨架、stub/null source graph和旧080x linker均不能直接作为产品实现。公共纯逻辑可在单独授权后先做；真实BootROM/FMC/GSP分别受平台、Vendor counter API和Runtime profile阻断。

## Recent source updates

- 2026-08-26：登记SRC-0035 Vendor RTL快照并接受ADR-0032；记录121文件/5733091 bytes/树哈希/顶层和版本头，新增RTL调查工作流、权威边界、只读快照和敏感Key约束。未修改RTL、产品软件或测试代码。
- 2026-08-12：登记SRC-0025 Firewall逻辑/寄存器截图与转录；新增OPEN-CONFLICT-014、机读UserId/authority映射、主详设/架构合入、49条细化测试和评审工作簿。状态为`DOCUMENTED / CONFLICTING / NOT_EXECUTED`，未修改产品代码或baremetal。
- 2026-07-29：接受ADR-0026；关闭OPEN-CONFLICT-013和OPEN-DESIGN-014的软件方案裁决，同步主详设、OpenSpec、专题、基线、任务和项目状态；implementation仍未授权。
- 2026-07-29：接受ADR-0025并扩展主详设第10章；新增OpenSpec `device-personalization-and-provisioning-v1`和OPEN-CONFLICT-012/013，同步Key/Certificate/OTP/Provisioning/Manufacturing专题。未修改代码仓、Vendor快照或测试工作簿。
- 2026-07-28：接受ADR-0023/0024并同步OpenSpec、主详设和专题：新增图示/可辨识超链接/双向链接约束；精简Manifest；更新LCS异常、非安全审计和eHSM自检Owner。
- 2026-07-21：登记 15 份 Vendor PDF（SRC-0001～SRC-0015）。
- 2026-07-21：登记两份 NGU800P 初步方案基线（SRC-0016、SRC-0017）。
- 详细结果见 `sources/intake-reports/INTAKE-2026-07-21-vendor-docs-and-baselines.md`。
- 2026-07-21：登记 OSR eHSM 代码混合快照（SRC-0018）和历史 Review 文档集（SRC-0019）。
- 代码接收和候选发现流程见 `sources/intake-reports/INTAKE-2026-07-21-vendor-code-and-historical-review.md` 与 `sources/historical-review-index.yaml`。
- 2026-07-21：接受 ADR-0001，关闭方案层级、Vendor 范围、Vendor 文档保密分类和密钥轮换策略批准状态相关问题。
- 2026-07-27：完成SRC-0015全文和逐页复核；接受ADR-0021、建立OpenSpec `soc-key-rotation-v1`和CE-SEC-013，关闭OPEN-BASELINE-004并部分收敛OPEN-DESIGN-014。
- 2026-07-21：接受 ADR-0002，并创建 `docs/09-plans/BASELINE-CONTROL.md` 管理 PDF、增量裁决和冲突升级。
- 2026-07-21：独立复核 FINDING-TRNG-05，建立首个开放冲突报告和 OPEN-CONFLICT-001。
- 2026-07-21：登记安全测试初版 SRC-0020；生成 `tests/cases/NGU800P-security-test-cases-v0.2.xlsx`，共 100 条用例并高亮变更。
- 2026-07-21：建立安全测试版本/Evidence流程；发现 Die1 measurement 表述冲突并建立 OPEN-CONFLICT-002。

## Recent planning updates

- 2026-07-21：只读盘点 `../gsp-pmp-rmp-omp/components/security` 和 BootROM/FMC/GSP 接入资产。已观察到 image/header/manifest、policy、measurement、BootROM demo 和 MCTP/SPDM Host/QEMU 框架，但关键生产路径仍存在 eHSM/crypto/cert/transport stub；本次未修改公司代码、未运行测试。
- 2026-07-21：创建 `docs/09-plans/SECURITY-FEATURE-REALIZATION-MATRIX.md`，把方案拆为 20 个 Feature，并标出 C0～C2 当前成熟度、EMU 前细节出口和测试映射。
- 2026-07-21：创建 `docs/09-plans/EMU前安全方案落实工作计划.md`、`docs/06-verification/EMU-TEST-READINESS-PLAN.md` 和 `tasks/active/TASK-SEC-EMU-PREP-001.md`，建立 T-4～T0 的方案/实现/测试双轨计划。
- 2026-07-21：根据负责人进一步确认修正仓库分工：正式详设、全部任务规划、测试规划/工作簿和开发追溯统一进入`security_-scheme`；产品软件代码进入`gsp-pmp-rmp-omp`，eHSM全能力可执行测试/EMU自动化进入实际目录名`baremetal`。2026-07-28进一步明确后者由运行在安全核上的独立baremetal软件栈承载，不属于GSP固件。本次未修改两个代码仓。
- 2026-07-21：创建 `docs/05-software-design/NGU800P安全软件详细设计.md` 和 `TASK-SEC-DD-001`，将20个Feature拆为DD-00～DD-09工作包；详设按Feature滚动闭环，DD-01启动，尚未进入编码。
- 2026-07-21：登记SRC-0021 `Work × Codex轻量协作工作流`；复用现有设计/追溯目录，新增项目适配流程、INV-SEC/CE-SEC模板、代码调查Evidence目录和首批INV-SEC-001/002。未修改两个代码仓。
- 2026-07-21：完成INV-SEC-001只读调查并形成CE-SEC-001。确认默认BootROM没有生产eHSM/FMC链，可选demo使用synthetic package和`ehsm_stub`；Vendor OSR port存在空timeout hook且平台参数不能直接复用。已回填`secure-boot.md`、`ehsm-mailbox.md`和`bootrom.md`，未执行Git、构建或测试。
- 2026-07-21：完成INV-SEC-002并形成CE-SEC-002。确认FMC/GSP默认仍为hello-world，唯一verify flow使用stub，制包工具不签名/加密，anti-rollback和measurement仍为骨架；新增OPEN-CONFLICT-003并回填`fmc.md`、`gsp.md`、`anti-rollback.md`和`secure-boot.md`。未执行Git、构建或测试。
- 2026-07-22：创建`docs/09-plans/NGU800P安全软件开发计划.md`和`TASK-SEC-DEV-PLAN-001`，将20个Feature转换为W0～W4、DEV-SEC-001～013和TST-SEC-001～005候选任务；新增实施任务模板和DoR/DoD。当前仅启动W0，不修改两个代码仓。
- 2026-07-22：创建`W0首轮设计评审包.md`和`boot-handoff.md`，补齐`error-handling.md`；整理10项可批准安全原则、三项冲突处理建议和首批编码准入差距。未修改两个代码仓。
- 2026-07-22：登记负责人W0评审结果：R1-01～04、06～10批准，R1-05待解释后评审，新增并批准R1-11；Bootloader自检位图为准、Host错误，关闭OPEN-CONFLICT-001。同步OpenSpec规则、详设/计划/测试追溯，未修改两个代码仓。
- 2026-07-22：接受ADR-0004；D2/D3采用推荐方案并关闭OPEN-CONFLICT-002/003，R1-05不采用；当时详设按Measurement Table、Manifest/loader、错误和既有release接口继续，其中Manifest载体已于2026-08-21被ADR-0030替代。
- 2026-07-22：DD-02拆为baremetal完整eHSM能力验证和gsp产品安全链移植；新增`ehsm-osr-host-porting.md`、Vendor Demo一级case catalog及INV/CE-SEC-003。当前仅更新`security_-scheme`，未修改两个代码仓。
- 2026-07-22：建立`NGU800P两项任务总计划`和两个顶层任务记录；任务二进入B0-R1，形成`SOC安全固件首轮详设评审包-启动链与核心合同.md`。
- 2026-07-22：对SRC-0016第1～15、33页和SRC-0017第5～6、13～16页进行文本与页面复核；登记OPEN-CONFLICT-004 Measurement地址宽度/职责和OPEN-CONFLICT-005 counter宽度/比较/更新问题。
- 2026-07-22：负责人部分裁决OPEN-CONFLICT-004，Measurement `load_addr`采用64位；`entry_addr`和字段职责等待后续输入。
- 2026-07-22：接受ADR-0005；OPEN-CONFLICT-005宽度按Vendor 16字节统一，Wing-M130/eHSM BL边界冻结；FW-C-001～011改为逐项详设检查表，不要求一次批准全部ABI。
- 2026-07-22：负责人确认Measurement `entry_addr`为64位；OPEN-CONFLICT-004完整关闭，Measurement地址字段不再阻断B0后续layout设计。
- 2026-07-22：完成INV/CE-SEC-004并进入B0-R2；建立`security-ram-layout.md`、ADR-0006和OPEN-CONFLICT-006，同步Mailbox/RAS职责。未修改两个代码仓。
- 2026-07-22：接受ADR-0007并重构安全RAM详设；OPEN-CONFLICT-006部分裁决，P1采用常驻+启动复用视图。未修改两个代码仓。
- 2026-07-23：负责人批准取消独立Mailbox Region并采用stage-local固定arena，关闭OPEN-DESIGN-005的物理放置选择；同日后续ADR-0011冻结首版slot/并发/retry/late-response规则，最终PMA和arena总量继续集成。未修改两个代码仓。
- 2026-07-23：完成INV/CE-SEC-006，关闭16 channel、地址转换机制、BootROM poll和ready/error位开放项，并把Vendor既有事实默认基线规则写入AGENTS/OpenSpec；第7项RAS通道和pre-ready持久化继续待项目组裁决。未修改两个代码仓。
- 2026-07-23：完成INV/CE-SEC-007；发现通用4 KiB Mailbox与Vendor 64 KiB eHSM布局冲突，建立OPEN-CONFLICT-007并禁止误绑定；收敛C908 64字节cache line、64位微秒timer、IRQ/主Makefile原语，登记CDK dummy路径缺口。未修改两个代码仓。
- 2026-07-23：登记SRC-0022并接受ADR-0009；SoC地址/寄存器/IRQ以RTL同步baremetal生成头为第一权威源。2 MiB RAM和4 KiB Mailbox数值已冻结，OPEN-CONFLICT-006/007改为使用/接口语义问题；未修改两个代码仓。
- 2026-07-23：接受ADR-0010；OPEN-CONFLICT-007采用Vendor direct Option A并关闭，Vendor公共源码只读；冻结BootROM/FMC/GSP首版全程poll、Vendor FW ready后interrupt仅为后续可选优化，以及RAS未ready早期终态。未修改两个代码仓。
- 2026-07-23：完成INV/CE-SEC-008并接受ADR-0011；形成`ehsm-host-adapter.md`，冻结首版单在途、256字节slot、GSP串行service、Vendor兼容cache/timeout scope、零自动retry和timeout quarantine。未修改两个代码仓。
- 2026-07-23：接受ADR-0012的历史Counter顺序；该顺序已由2026-07-28 staged-candidate exact-match裁决取代。LCS直读、Vendor FW由GSP加载及completion unknown quarantine原则保留。
- 2026-07-23：接受ADR-0013并建立eHSM产品operation profile；冻结GSP镜像集合、必需能力、内部通用服务优先级、LCS授权和Runtime局部隔离边界。未修改两个代码仓。
- 2026-07-23：批准FMC从BootROM committed Measurement条目取得自身epoch；新增`measurement-table.md`并回填BootROM/FMC/GSP/anti-rollback/secure-boot/RAM布局/计划/OpenSpec，FW-C-007从冲突阻断改为部分冻结。未修改两个代码仓。
- 2026-07-23：完成INV/CE-SEC-009并启动B0-R3 package/verify/loader切片；新增安全固件包/Manifest和typed verify/loader/release候选合同，登记OPEN-CONFLICT-008/OPEN-DESIGN-008。未修改Vendor及两个代码仓，未执行Git、构建或测试。
- 2026-07-23：接受ADR-0014并关闭OPEN-CONFLICT-008/OPEN-DESIGN-008；新增公共ABI registry、OpenSpec `secure-package-manifest-v1` change及BootROM/FMC/GSP函数级状态机，测试矩阵记录v0.3新增方向。未修改Vendor及两个代码仓，未执行Git或安全测试。
- 2026-07-24：接受ADR-0015并关闭OPEN-DESIGN-009；同步GSP、Host Adapter、函数级状态机、OpenSpec和任务追溯。OPEN-DESIGN-010按负责人要求等待后续资料。未修改两个代码仓，未执行Git或安全测试。
- 2026-07-24：完成INV/CE-SEC-010；确认2 MiB local/system地址、eHSM IRQ1～16精确编码、当前BootROM/FMC与GSP/Runtime的混合linker视图，以及当前M-mode cache开启但PMA/Firewall未绑定的事实。回填安全RAM、Mailbox、Host Adapter、OpenSpec和开放项；未修改两个代码仓，未执行Git、构建或安全测试。
- 2026-07-24：接受ADR-0016并关闭OPEN-CONFLICT-006的GSP/OMP角色子项；同时按负责人目标将主详设定义为最终单一完整交付，建立16章及附录的合入状态表。未修改两个代码仓，未执行Git、构建或安全测试。
- 2026-07-24：完成INV/CE-SEC-011；确认Vendor 16字节thermometer/unary编码、BL candidate暂存、Vendor FW启动OTP提交/readback，以及Host Counter接口与匹配FW/16字节资源的映射未证明。登记OPEN-CONFLICT-009并合入主详设第9章；未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-24：负责人决定Counter细节先不管；统一按16字节值并默认存在对应接口继续详设，OPEN-CONFLICT-009改为延期绑定，不重开FW加载Owner。
- 2026-07-24：把已批准的固定Vendor Header、128字节NGU Manifest v1、公共ABI Registry、stage准入、制包发布和negative corpus完整合入主详设第3章；下一步按整本顺序合入第4章。未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-24：接受ADR-0017并关闭OPEN-DESIGN-004；冻结三套产品Secure Package Profile和公共ID，依据SRC-0018映射RSA-PSS、ECDSA-P256、SM2/SM3及AES/SM4-CBC/no-padding Vendor原生细节；登记OPEN-DESIGN-011。未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-24：将eHSM BL/Host双路径移植、Vendor direct 16通道Mailbox、ready/self-test、三stage首版poll、typed Adapter、单context/单在途、cache/deadline、timeout quarantine、GSP终身Owner、RAS终态和测试合同完整合入主详设第4章；下一步转入第5章。未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-24：将2 MiB双地址域、P1常驻/启动复用、统一layout源、权限/Owner转换、Host ingress/plaintext/执行区状态机、BootROM/FMC尾部回收、Firewall/PMP/PMA分工、cache和清零合同完整合入主详设第5章；下一步转入第6章BootROM。未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-29：负责人指定以SRC-0023截图为准；更新`secure_boot`为`boot_pin.secure_boot[3]`、default 0、0非安全/1安全并同步矩阵。当前SRC-0022生成头冲突，OPEN-CONFLICT-010改为`BLOCKED_BY_RTL_SYNC`。
- 2026-07-24：登记SRC-0023和CE-SEC-012，建立OPEN-CONFLICT-010/OPEN-DESIGN-012；将`secureboot.001～007`完整合入主详设第6章，冻结BootROM逻辑API、静态context、模式/ready/verify/Manifest/epoch/load/Measurement/release状态机、失败闭锁和测试入口。下一步转入第7章FMC；未修改两个代码仓或Vendor快照，未执行Git、构建或安全测试。
- 2026-07-27：建立ADR-0022/OpenSpec `measurement-table-abi-v1`候选，重写Measurement接口并合入主详设第9章，更新Feature/开发/测试追溯；v0.2工作簿保持不变。等待负责人批准六组决策，未修改代码仓、Vendor快照或执行Git。
- 2026-07-27：按负责人反馈把ADR-0022修订为SRC-0016字段优先的1280B精简布局，恢复两个Entry count，删除Measurement `table_flags/algorithm_profile`及其余冗余字段，补充总体占位图和BootROM slot；旧4224B候选被替代且从未授权编码。v0.2工作簿保持不变，未修改代码仓、Vendor快照或执行Git。
- 2026-07-27：负责人逐项批准ADR-0022最终逻辑ABI：改为实际count/length的紧凑Firmware列表和唯一SoC State；BootROM不建普通Entry；删除`state_entry_count/generation/eHSM状态/domain/key_id/signer_id/时间戳`，保留CRC/commit和单一reserved。关闭OPEN-DESIGN-007，新增OPEN-DESIGN-021管理最大实例容量。v0.2工作簿保持不变，未修改代码仓、Vendor快照或执行Git。

## Verification status

基础检查入口：使用项目Python运行`tools/scripts/project_check.py`。2026-08-13 v0.3.8工作簿已重新导入，12个工作表均完成视觉检查，关键范围内容正确且公式错误扫描为0；SoC用例为7条、EDA/硬件需求为3项。项目检查结果以本次工作记录为准；所有新增用例和EDA条目均为`NOT_EXECUTED`，尚未执行安全测试。

## Next actions

1. 由Vendor/eHSM/RTL Owner补齐SRC-0035正式delivery/release note、filelist、外部define/parameter、SoC wrapper/库清单和匹配lint/elaboration/仿真版本；新投递不得覆盖当前快照。
2. ADR-0029/OPEN-DESIGN-022拓扑已关闭；由Vendor/eHSM和PKI Owner补齐OPEN-DESIGN-023的KDF-KeyGen command、Key Attribute和动态证书Profile。
3. RTL/DFT/制造Owner补OPEN-CONFLICT-012的一机一密载体、ATE、lock/proof；Vendor后续交付关闭生命周期和轮换差距。
4. Flash/KMS/CA/MES/制造Owner补精确base/erase、企业OID/DN/有效期/serial、endpoint/schema和transport framing，并据ADR-0026/0029生成Provisioning与DICE Certificate Profile/registry。
5. 继续通过OPEN-DESIGN-021补最大Firmware实例，并通过OPEN-CONFLICT-006补RAM/PMA/Firewall，不与本轮Key设计混合。
6. 输入到齐后再生成下一版高亮测试工作簿；代码仓仍需单独实施授权。
7. RTL/寄存器Owner关闭OPEN-CONFLICT-014后，再冻结Firewall MMIO常量、默认Region policy及9条BLOCKED细化测试的确定性Expected。
