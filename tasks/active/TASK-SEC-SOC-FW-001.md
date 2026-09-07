# TASK-SEC-SOC-FW-001：SoC安全固件详细设计与开发

## TASK BRIEF

- 类型：顶层任务二。
- 状态：design_freeze_active。
- Owner：GSP。
- 目标：完成Wing-M130/eHSM BL集成以及C908 BootROM/FMC/GSP产品安全固件的详细设计、实现、联调和流片前验证。
- 设计/任务/追溯仓库：`security_-scheme`。
- 产品代码仓库：`../gsp-pmp-rmp-omp`；Wing-M130/eHSM BL按Vendor交付和批准集成方式管理。
- 当前授权：只执行B0详细设计；不修改产品代码，不执行Git命令。
- 已批准B0-R1方向并由ADR-0030/0033更新：删除Manifest；Native Header offset1008存LE64 `load_addr`、offset1016存覆盖Header 256～1015的LE32 CRC-32/ISO-HDLC、offset1020保留4B为0，entry=load，`Code_Size`唯一长度。CRC在preflight和Vendor PASS后双检但不替代密码认证。ADR-0022最终Measurement地址为64位`SOC_PA`且不带domain。ADR-0019确认Vendor Counter始终为16字节并统一命名`rollback_counter`；版本仅存外部release metadata。FMC固定type 1；BootROM以`check_version=0`验证并由BL暂存认证candidate，不读/比/写stored counter；FMC初始化回传Measurement expected candidate，BL exact-match后commit/readback；proof后接收同值GSP，其他SoC镜像匹配已提交值。
- 已批准B0-R2方向：产品唯一使用`0x1010_0500_0000`起始2 MiB System Address并替代旧080x；SRC-0031/ADR-0028进一步冻结GSP静态880 KiB、FMC复用128 KiB、Measurement 16 KiB、PMP/RMP各256 KiB和Host ingress 512 KiB，MMP主要驻留DDR。独立plaintext Region取消，FMC/GSP/PMP/RMP采用受控原地加载；FMC退出后其128 KiB经撤权、清零和回读才可加入GSP动态池。eHSM可访问整个2 MiB；Mailbox follow Vendor；security只上报错误，reset由RAS策略决定。剩余PMA/Firewall、MMP DDR、reset/release PC视图和release link map峰值Evidence受OPEN-CONFLICT-006/OPEN-DESIGN-010约束。

## 当前B0详细设计计划

| 轮次 | 主题 | 必须冻结的内容 | 状态 |
|---|---|---|---|
| B0-R1 | 启动链与核心合同 | 角色、Wing-M130/eHSM BL边界、BootROM/FMC/GSP时序、Package/Header Overlay/typed-stage policy、verify/loader/Measurement/error/release合同 | completed_with_follow_up |
| B0-R2 | eHSM和平台接口 | Host adapter、Vendor Mailbox、2 MiB安全RAM/Firewall、常驻与启动复用、buffer/cache、ready/self-test、timeout、RAS错误上报、NGU800P port参数 | design_integrated / implementation_inputs_pending；SRC-0031/ADR-0028已冻结2 MiB精确offset/size和受控原地加载，direct/status、reset/release PC视图、PMA/Firewall、MMP DDR和release link map峰值作为实现DoR继续开放 |
| B0-R3 | 各stage软件状态机 | BootROM、FMC、GSP的函数/API、数据结构、正常/失败/恢复状态机、代码落点 | design_integrated / implementation_inputs_pending；ADR-0019/0022/0030已冻结Header Overlay、Measurement和Counter方向，Runtime资料待补 |
| B0-R4 | SPDM与安全资产 | Measurement/SPDM、静态Issuer前缀、DICE一级动态Leaf/sign provider、LCS/Debug、Key/OTP/Cert/Rotation | design_accepted_with_bindings；ADR-0029关闭OPEN-DESIGN-022拓扑：BootROM/FMC只贡献FMC/GSP Hash，GSP经slot13 UDS派生Alias并组装动态Leaf，slot14签Leaf、Alias签Report；Cert0/1保存三张静态Issuer前缀。OPEN-DESIGN-023/015继续阻断eHSM command、OID/Profile和SPDM wire |
| B0-R5 | 更新和系统集成 | Package工具、正常更新/OOB、Multi-Die/UCIe/Firewall、构建和EMU hooks | design_integrated / platform_inputs_pending；流程和失败原则已入正文，实际分区、寄存器、UCIe/Firewall和EMU命令待平台输入 |
| B0-R6 | 全量冻结评审 | 20个Feature的Requirement/Code/Case/Evidence和所有冲突关闭 | design_body_complete / readiness_review_complete / open_inputs_tracked；CE-SEC-014已完成编码前落实性审查，后续进入测试规划v0.3和分批实现DoR |

## 编码门禁

对应Feature只有同时满足以下条件才可进入B1/B2/B3：

- SRC-0017/SRC-0016/ADR/OpenSpec关系明确，无开放的受影响冲突。
- wire ABI、结构体字段、API、状态机、owner、地址域、错误/RAS动作边界和清零已冻结。
- 目标文件/符号、测试case、Expected和Evidence路径已登记。
- 负责人批准对应OpenSpec和实施任务。
- EMU/产品路径不存在stub、simulated success、test key/provider、未批准hardcode或silent fallback。

## 当前输出

- 总计划：`docs/09-plans/NGU800P两项任务总计划.md`。
- 正式详设最终单一完整交付：`docs/05-software-design/NGU800P安全软件详细设计.md`。专题文件是逐章工作区，获批内容必须合入该文档，不能用链接目录替代整本详设。
- 主详设第3章已按ADR-0030/0033更新：固定1024字节Vendor Header、offset1008/1016/1020 Overlay、Header CRC、无Manifest、`Code_Size`唯一长度、entry=load、公共ABI Registry、stage准入、真实制包发布和golden/negative corpus均已进入正文；ADR-0017冻结三套算法Profile，ADR-0020批准单一provisioning/release matrix。ADR-0022已批准Measurement逻辑ABI；OPEN-DESIGN-021最大实例、实际SKU行、OPEN-CONFLICT-006和OPEN-DESIGN-010继续按各自范围阻断具体绑定或实现。
- 主详设第4章已合入：eHSM BL/Host双路径移植、Vendor direct 16通道Mailbox、ready/self-test、全阶段poll、typed Adapter、单context/单在途、cache/deadline、timeout quarantine、GSP终身Owner、RAS终态、代码落点和测试要求已进入正文；真实MMIO、PMA、deadline/RAS数值保持显式开放。
- 主详设第5章已按SRC-0031/ADR-0028更新：2 MiB唯一System Address、精确Region、权限矩阵、Host ingress与目标Region受控原地加载、FMC 128 KiB回收、Measurement 16 KiB隔离、PMP/RMP各256 KiB和MMP DDR边界已进入正文；最终PMA/Firewall、MMP DDR、release PC和link map峰值保持显式开放。
- 当前评审包：`docs/09-plans/SOC安全固件首轮详设评审包-启动链与核心合同.md`。
- B0-R2安全RAM：`docs/03-architecture/security-ram-layout.md`；总体边界见ADR-0006，精确布局与受控原地加载见ADR-0028，剩余PMA/Firewall、MMP DDR和footprint输入见OPEN-CONFLICT-006/OPEN-DESIGN-010。
- B0-R2 Firewall专题：`outputs/firewall/NGU800P_Firewall_专题报告.docx`、`docs/03-architecture/firewall-isolation.md`与主详设第5.9节；SRC-0025已补齐16个UserId/authority编码，OPEN-CONFLICT-014继续阻断CSR基址、reset和默认Region量产绑定。测试入口为`tests/cases/NGU800P-security-test-cases-v0.3.6-firewall-review.xlsx`，49条均为`NOT_EXECUTED`。
- B0-R2 SEC_CFG专题：`outputs/sec-cfg/NGU800P_SEC_CFG_寄存器与软件使用专题报告.docx`、`docs/03-architecture/sec-cfg-status-observation.md`、`docs/04-interfaces/sec-cfg-register-interface.md`与主详设第5.9.2节；SRC-0026已补齐17个寄存器、status/hardware-error位、UID和Debug观察模型。截图`0x1010_07B0_0000/4 KiB`与SRC-0022 `0x1010_0820_0000/16 KiB`冲突，且FW error、LCS、Debug、UID-valid和64-bit一致性由OPEN-CONFLICT-015阻断。12条细化测试均为`NOT_EXECUTED`。
- B0-R2 Mailbox Vendor基线：`evidence/code-investigations/CE-SEC-006-ehsm-mailbox-vendor-baseline.md`；16 channel、地址转换机制、BootROM/FMC/GSP首版poll、ready/error门禁及RAS未ready终态已收敛，剩余为NGU800P真实MMIO、共享RAM cache/PMA、deadline、RAS通道和arena参数。
- B0-R2 NGU800P port绑定：`evidence/code-investigations/CE-SEC-007-ngu800p-ehsm-port-binding.md`；确认通用4 KiB Mailbox不能替代Vendor 64 KiB eHSM孔径，ADR-0010已按Vendor direct合同关闭OPEN-CONFLICT-007；64字节cache line、64位微秒timer和Makefile原语可用，direct aperture/status准确宏、2 MiB cache/PMA属性及真实MMIO仍待输入。
- B0-R2 Host adapter：`docs/04-interfaces/ehsm-host-adapter.md`与`evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md`；首版单在途、256字节slot、GSP串行service、active cache/timeout scope、零自动retry和timeout quarantine已冻结。
- B0-R2 平台事实核查：`evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md`；确认eHSM IRQ1～16为APLIC domain 9/source 78～93，当前GSP/Runtime与BootROM/FMC存在system/local混合linker视图；direct/status、2 MiB PMA/coherence、Firewall实例和新D0 reset/release PC视图仍待平台输入。
- B0-R2 Stage operation边界：BootROM LCS直读/counter compare-only；FMC主动调用eHSM BL专用API更新`rollback_counter`；GSP加载Vendor FW，其他镜像匹配已提交值并可局部隔离。
- B0-R2/B0-R4产品operation：内部通用算法低优先级开放、LCS最终授权和Runtime局部隔离；ADR-0019确认不向Host开放GSP服务。
- B0-R3 Measurement/counter：冻结BootROM committed FMC rollback counter、FMC/GSP一致性和eHSM BL update→GSP Measurement commit→release。ADR-0022已关闭Measurement逻辑ABI；ADR-0028已冻结物理16 KiB且理论最多126个Firmware Entry。command/packing/LCS/交付版本、OPEN-DESIGN-021产品实例上限和PMA/Firewall在实现前冻结。
- B0-R3 Package/verify/loader：ADR-0030批准`secure-firmware-package.md`、`security-common-abi.md`和`image-verify-loader.md`的Vendor原生包、Header Overlay、typed-stage registry、raw/typed verify、`target+1024`统一loader及stage release门禁；旧Manifest OpenSpec已superseded。`boot-stage-state-machines.md`已展开BootROM/FMC/GSP函数级顺序；PMA/Firewall/MMP DDR、counter实现、Measurement可见性和Runtime profile仍受对应开放项阻断。
- B0-R3 GSP Owner：ADR-0015冻结第一个最高优先级`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner，其他task只经typed queue请求，quarantine后不移交Owner；OPEN-DESIGN-010的PMP/RMP/MMP依赖/release输入按负责人要求等待后续资料。
- B0-R3 GSP产品身份：ADR-0016冻结GSP替代旧OMP/Q&P产品固件；OMP历史代码只作GSP迁移输入，不建立独立产品identity、RAM、Measurement或release链。
- B0-R3算法Profile：ADR-0017冻结Profile 1=SHA-256+RSA-2048-PSS+AES-128-CBC、Profile 2=SHA-256+ECDSA-P256+AES-128-CBC、Profile 3=SM3+SM2+SM4-CBC；其他Vendor算法只作baremetal能力/回归输入。ADR-0020批准设备/镜像/key域/board/LCS单一matrix，实际配置行在制包/OTP/EMU前补齐，缺失默认拒绝。
- B0-R4～B0-R6：ADR-0029保留复杂PKI离线化、固定PoP/ticket和Cert A/B原则，并把证明链更新为三张静态Issuer前缀+GSP一级动态Firmware Alias Leaf；Measurement 16 KiB不复用，GSP新增峰值不超过12 KiB。eHSM KDF-KeyGen/Profile由OPEN-DESIGN-023、SPDM wire由OPEN-DESIGN-015继续阻断。
- 编码前落实性审查：`docs/09-plans/SOC安全固件编码前落实性审查-v1.md`；确认三个产品入口均未实现目标链，公共ABI/构建/linker需重构，并把可先做纯逻辑、平台阻断、Vendor阻断和延期功能分开管理。当前仍无代码修改授权。

## 当前下一步

1. ADR-0029/OPEN-DESIGN-022设计已关闭；由Vendor/eHSM Owner补齐slot13 UDS KDF、确定性Alias KeyGen和opaque handle/sign command。
2. 根据ADR-0026/0029，由RTL/Vendor/Flash/KMS/CA/MES Owner补齐OID/DN/有效期/SM2 ID/Flash/wire绑定并生成Provisioning与DICE Certificate Profile/registry。
3. 对仍未到齐的RTL/Vendor/Flash/KMS/CA/MES输入保持实施阻断，不使用stub、raw接口或猜测常量代替。
4. 代码实现仍需单独授权；2 MiB SRAM精确布局已冻结，PMA/Firewall、MMP DDR、Counter、Runtime release与SPDM输入继续按既有开放项管理。

## 验收

- [x] 安全软件方案全部Feature已形成可直接指导开发的详细设计；外部平台/Vendor绑定作为显式DoR，不以猜测值填充。
- [x] 所有批准专题已合入一份可独立阅读的`NGU800P安全软件详细设计.md`，正文完整覆盖第1～16章和附录，不以外部链接代替规范内容。
- [x] Header Overlay/typed-stage policy、Measurement、公共接口职责、状态机、Owner、错误和不兼容规则已冻结；尚缺的PMA/Firewall/MMP DDR/Vendor command均已原位登记并明确阻断范围。
- [ ] Wing-M130/eHSM BL版本、配置和集成差异明确。
- [ ] BootROM/FMC/GSP实现、构建和EMU验证完成。
- [ ] 流片前P0用例和Evidence满足发布门禁。
- [x] 未执行任何Git命令或提交。
