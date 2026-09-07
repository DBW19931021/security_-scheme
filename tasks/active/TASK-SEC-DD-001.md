# TASK-SEC-DD-001：NGU800P 安全软件正式详设

## TASK BRIEF

- Task ID：TASK-SEC-DD-001
- 角色：`TASK-SEC-SOC-FW-001` 的详设工作包和历史追溯记录，不是第三个顶层任务。
- 状态：active
- Owner：GSP
- 背景：EMU 预计约4周后可用，需要在此前把 SRC-0016/SRC-0017 转换为能直接指导开发和测试的正式详设。
- 目标：完成 `SEC-FEAT-001`～`SEC-FEAT-020` 的架构、接口、软件模块、错误/恢复和 code/test 追溯，使满足准入条件的Feature可以逐项进入编码。
- 非目标：当前详设启动阶段不修改 `gsp-pmp-rmp-omp` 或 `baremetal`，不运行安全测试，不证明硬件已通过，不执行任何 Git 操作。
- 主入口：`docs/05-software-design/NGU800P安全软件详细设计.md`。
- 代码调查流程：`docs/09-plans/CODE-INVESTIGATION-WORKFLOW.md`。
- 输入：SRC-0016、SRC-0017、Vendor SRC-0001～0015、Vendor代码SRC-0018、历史Review SRC-0019、测试SRC-0020、ADR-0001～0019、CE-SEC-001～011、已关闭OPEN-CONFLICT-001～005/007～009、开放OPEN-CONFLICT-006、公司代码只读盘点。
- 输出仓库：全部详设、任务和追溯只写入 `security_-scheme`；后续批准的生产实现写入 `gsp-pmp-rmp-omp`，可执行测试写入 `baremetal`。
- 开发排程：`docs/09-plans/NGU800P安全软件开发计划.md`；候选任务由`TASK-SEC-DEV-PLAN-001`跟踪，不代表编码授权。

## 协作方式

- Codex负责：读取资料和代码、事实分级、设计草案、接口/状态机/错误表、目标代码映射、测试追溯、差距和冲突报告。
- 负责人负责：批准产品行为，裁决规范冲突，确认算法/OTP/LCS/Debug/Key/Recovery等安全策略，确认Owner和跨团队接口，批准进入编码。
- 默认执行：Codex按依赖关系继续推进，不为普通文档组织或可从资料得出的事实反复请求选择。
- 请求裁决：只有候选方案会改变批准方案、存在证据冲突、边界无法唯一确定或涉及不可逆/发布策略时，才向负责人提交2～3个选项、影响和推荐项。

## TASK PLAN

| 工作包 | Codex执行内容 | 主要输出 | 需要负责人决定的事项 | 状态 |
|---|---|---|---|---|
| DD-00 详设启动 | 建立主入口、章节模板、Feature分组、完成定义和编码门禁 | 本任务、详设主入口、滚动计划 | 是否调整工作包优先级 | completed |
| DD-01 系统边界与启动链 | 执行INV-SEC-001/002，对照SRC-0017/SRC-0016和公司代码，梳理SoC/BootROM/eHSM/FMC/GSP/Runtime/Host职责、输入输出、release条件和失败终态 | CE-SEC-001/002、`secure-boot.md`、`bootrom.md`、`fmc.md`、`gsp.md`初稿和启动时序 | 跨团队Owner、各master aperture Evidence及方案边界 | in_progress；GSP canonical地址已裁决，待设计评审 |
| DD-02 eHSM启动与Adapter | 拆分DD-02A/02B：建立`ehsm_demo_test()`完整case list和baremetal移植规则；建立`bl_demo`能力到gsp产品安全链的底层函数/格式复用边界；继续冻结ready/self-test、Vendor Mailbox、2 MiB安全RAM/buffer/cache、timeout、RAS错误上报 | `ehsm-osr-host-porting.md`、`EHSM-DEMO-CASE-CATALOG.md`、`ehsm-mailbox.md`、`ehsm-host-adapter.md`、主详设第4章、`security-ram-layout.md`、`error-handling.md`及目标文件/符号映射 | 剩余二级case、eHSM direct/status MMIO、2 MiB最终PMA/Firewall、operation timeout/RAS数值和OPEN-CONFLICT-006；首版模式/并发/retry/late-response已关闭 | integrated_with_platform_inputs；第4章主体合同已合入，平台数值和baremetal二级case继续 |
| DD-03 镜像/Header Overlay/验证/回滚 | 固化Header offset1008/1016/1020、Header CRC、typed-stage policy、验签解密顺序、counter、measurement和fail-close行为 | `secure-boot.md`、`anti-rollback.md`、接口数据表和corpus计划 | ADR-0030/0033删除Manifest并冻结Header CRC、`Code_Size`唯一长度/entry=load；三套算法Profile和FMC主动更新API方向已冻结，准确command/packing/LCS/status/readback/交付版本及耗尽/掉电参数属于实现DoR | active / logical_design_continues |
| DD-04 LCS/Key/Cert/Debug/Rotation | 定义生命周期权限、OTP/Key Slot、证书、Debug/RMA、轮换和制造灌装状态机 | 相关03/04设计、slot/权限/失败矩阵 | Key/OTP/LCS/Debug/Rotation具体产品行为和不可逆点 | pending |
| DD-05 Measurement/Attestation/SPDM | 定义Measurement Table、静态Issuer前缀+GSP一级动态Firmware Alias Leaf、sign provider、SPDM Profile和MCTP transport | `measurement-table.md`、`device-attestation.md`、`certificate-management.md`、`spdm.md`和provider ABI | ADR-0022已批准Measurement；ADR-0029批准ROM/FMC只交Hash、GSP固定Profile组证、eHSM不可导出派生/签名；OPEN-DESIGN-021最大实例继续开放，eHSM/OID/SPDM wire由OPEN-DESIGN-023/015阻断 | design_integrated / profile_blocked |
| DD-06 Update/OOB/Recovery | 定义staging、验证、写入、激活、掉电恢复、OOB授权和再次安全启动 | `firmware-update.md`、`recovery.md`、`oob-interface.md` | Flash/OOB Owner、分区/授权/掉电策略 | pending |
| DD-07 Multi-Die/Firewall/接口 | 定义Die0/Die1顺序、UCIe、2 MiB安全RAM Firewall、PCIe/UART/JTAG/DFT/DMA访问矩阵 | `security-ram-layout.md`、`firewall-isolation.md`、主详设第5章、`host-device-interface.md` | Firewall窗口/粒度/master/lock/reset默认值和Die1边界 | active / security_ram_contract_integrated；P1权限/复用/清零已合入，容量和硬件绑定待冻结 |
| DD-08 跨Feature统一规则 | 统一错误、中断、日志、审计、敏感材料清零、并发、内存和reset规则 | `error-handling.md`、`logging-audit.md`和全局规则 | 发布日志/审计要求、清零范围和故障策略 | pending |
| DD-09 追溯与评审 | 建立Source→Requirement→Design→Code→Case→Evidence链，逐Feature检查编码准入 | traceability更新、评审记录、编码候选任务清单 | 逐Feature批准或退回、风险接受 | pending |

## 当前立即执行步骤

1. 已建立 DD-01 的 Source/章节/代码资产对照表。
2. 已执行`INV-SEC-001`，形成`CE-SEC-001`并回填BootROM/eHSM启动事实；当前等待设计评审。
3. 已执行`INV-SEC-002`，形成`CE-SEC-002`并回填FMC/GSP验签解密加载链；OPEN-CONFLICT-003已按ADR-0004关闭。
4. 已形成`docs/09-plans/W0首轮设计评审包.md`，整理BootROM→eHSM→FMC→GSP→Runtime的可批准原则、输入输出、release/measurement不变量和待裁决项。
5. `docs/04-interfaces/boot-handoff.md`已按ADR-0004标记deferred；当前不采用Handoff，不创建相应ABI/SRAM/编码任务。统一error和Measurement Table继续收敛。
6. 负责人已批准W0-R1-01～04、06～11；R1-05当前不采用。D1～D3均已关闭。
7. 现有test/stub/demo/历史Expected只作为代码事实和差距，不作为详设或最终oracle；EMU/产品禁止stub、模拟成功和有最终落地风险的实现。
8. 继续根据已批准结论更新Requirement、OpenSpec、测试oracle和目标代码映射；未明确字段保持待裁决。
9. DD-02已按负责人确认拆分；baremetal Vendor Demo二级case属于`TASK-SEC-BAREMETAL-EHSM-001`，产品`bl_demo`复用映射和产品接口冻结属于`TASK-SEC-SOC-FW-001`。
10. 当前执行`SOC安全固件首轮详设评审包-启动链与核心合同.md`；OPEN-CONFLICT-004/005/009已关闭。地址字段和`rollback_counter[16]`可进入layout设计；eHSM BL专用API的准确command/packing/LCS/status/readback/交付版本仍阻断真实更新代码。
11. B0-R2已建立2 MiB安全RAM独立章节并冻结Mailbox/RAS职责及P1生命周期原则；ADR-0016已关闭OMP角色，OPEN-CONFLICT-006剩余PC/linker视图、容量、PMA和Firewall问题阻断最终linker。
12. 已完成INV/CE-SEC-007并接受ADR-0010：OPEN-CONFLICT-007采用Vendor direct合同关闭，Vendor公共源码不修改，4 KiB通用Mailbox不用于eHSM；direct aperture/status准确宏作为集成同步门禁。BootROM/FMC/GSP首版全程poll，Vendor FW ready后的interrupt仅为后续可选优化；RAS未ready早期终态已冻结。真实MMIO常量暂停，其他port合同继续设计。
13. 已完成INV/CE-SEC-008并接受ADR-0011：冻结每stage一个256字节静态context slot和单在途、GSP唯一service、active cache/timeout scope、首版零自动retry及timeout quarantine；新增`ehsm-host-adapter.md`结构/API/状态机/测试合同。最终PMA和数值参数继续集成。
14. 已接受ADR-0012/0019并完成counter产品语义批准：冻结BootROM直接读LCS/counter compare-only、GSP加载Vendor FW和Host重新下发职责；GSP package的`rollback_counter[16]`作为全SoC锚点，FMC唯一主动更新，其他镜像匹配已提交值并局部隔离。OPEN-CONFLICT-005/009已关闭。
15. 已接受ADR-0013/0019并建立`ehsm-product-operation-profile.md`：冻结eHSM FW/PMP/RMP/MMP镜像集合、签名/RNG/Hash和方案Feature能力、内部通用算法低优先级开放、GSP发起/eHSM LCS最终授权、不向Host开放GSP安全服务及单Runtime局部隔离。
16. 负责人已批准ADR-0022/OpenSpec `measurement-table-abi-v1`：128B Header、实际数量128B紧凑Firmware列表、唯一128B SoC State、CRC-32C/32位commit和reset全清零；不含BootROM Entry、state count、generation、eHSM状态、domain、Key ID和时间戳。新增OPEN-DESIGN-021等待最大实例容量。
17. 已将CE-SEC-011的16字节thermometer/unary编码、BL候选暂存、FW启动提交、写后回读和Host通用Counter不等价事实合入主详设第9章；ADR-0019进一步要求FMC主动调用eHSM BL新增专用API，Vendor FW启动副作用不得作为产品提交路径。
18. 已按ADR-0030/0033将Vendor固定Header、offset1008/1016/1020 Overlay、Header CRC、typed-stage Registry、各stage准入、制包发布和negative corpus完整合入主详设第3章；旧Manifest和旧8B零reserved方案仅保留历史追溯。
19. 已接受ADR-0017/0020并关闭OPEN-DESIGN-004/011：三套产品算法Profile和公共ID已冻结，设备/镜像/key/board/LCS采用单一provisioning/release matrix；实际配置行由Owner在制包/OTP/EMU前补齐，缺失默认拒绝。
20. 已将eHSM BL/Host双路径移植、Vendor direct 16通道Mailbox、ready/self-test、首版全poll、typed Adapter、单context/单在途、cache/deadline、timeout quarantine、GSP终身Owner、RAS终态和测试要求完整合入主详设第4章；真实MMIO、PMA和数值参数保持开放。
21. 已将2 MiB双地址、P1常驻/启动复用、统一layout源、权限/Owner转换、Host ingress/plaintext/执行区生命周期、BootROM/FMC尾部回收、Firewall/PMP/PMA分工、cache和清零合同完整合入主详设第5章；下一章收敛BootROM详细设计。
22. 已接受并更新ADR-0020：关闭OPEN-DESIGN-003/011/013/016/017/018/020；SPDM secure-session目标保留，完整Profile前阻断wire实现，产品/EMU no-stub和test隔离已批准。
23. 2026-07-29接受ADR-0025/0026并关闭OPEN-CONFLICT-013及OPEN-DESIGN-014的软件设计裁决；2026-08-04按SRC-0024纠正Table 34、16-slot、Level、slot14单Profile和slot13 UDS权限。OPEN-CONFLICT-011/012、Key Attribute/backend和外部平台输入继续作为实施绑定。
24. 2026-09-02接受ADR-0034：非安全顶层分支区分DEV/MANU `MANUFACTURING_PROVISIONING`与`RESTRICTED_NONSECURE`；Vendor eHSM BL typed制造API、部分写恢复、USER前产品安全启动预演和`MANU→USER`最后提交已合入第2/6/10/16章。实施等待Vendor/RTL/制造绑定。

## 首轮负责人输入

以下信息有助于提高详设速度，但当前不阻塞Codex开始DD-01：

- GSP详设总负责人及BootROM、FMC、GSP、测试章节评审人。
- SRC-0016/SRC-0017的正式Owner、批准版本和适用D0修订。
- EMU首个可用日期及已知加载、日志、reset、Flash、mailbox、OTP/eFuse和故障注入能力。
- 算法发布profile、SPDM呈现profile、各master aperture/Measurement物理合同Owner；OPEN-CONFLICT-001原始Vendor回复材料补录Owner。

## 编码启动条件

当前不创建生产编码任务。开发主计划已登记DEV-SEC候选目录；某个Feature满足主详设中的按Feature编码门禁并经负责人批准后，才用`IMPLEMENTATION-TASK-template.md`创建单独的 `gsp-pmp-rmp-omp` 实现任务；对应测试规划仍在 `security_-scheme`，可执行测试任务进入 `baremetal`。

## 最终文档合入规则

- 专题设计可以按章独立生成和评审，但最终正式交付必须是一份可独立阅读的`docs/05-software-design/NGU800P安全软件详细设计.md`。
- 专题获批后必须把规范性角色、流程、ABI、结构体、状态机、错误、内存、测试和开放项合入主文档；专题文件保留证据和更细表格。
- 主文档必须尽量保持SRC-0016原则。可能改变原方案的内容不得静默合入，必须以冲突或候选方案提交负责人裁决。
- Open Question不妨碍整本结构完成，但必须在受影响章节原位标注并在附录统一汇总。

## TASK ACCEPTANCE

- [ ] 20个Feature均有受控详设章节、事实状态、Owner和Source依据。
- [ ] 所有章节已合入单一完整主详设，主文档不依赖读者打开专题文件才能理解规范行为。
- [ ] 所有实现行为均有ABI/状态机/权限/错误/reset/掉电/清零定义，或明确标记为受阻。
- [ ] 每个Feature均映射到 `gsp-pmp-rmp-omp` 目标文件/符号和 `baremetal` 可执行测试入口。
- [ ] 每个Feature均关联Requirement、工作簿case和Evidence计划。
- [ ] 所有冲突已裁决、隔离或形成负责人批准的风险处置。
- [ ] 满足准入的Feature形成独立编码候选任务；未满足的Feature不提前编码。
- [ ] 未修改两个代码仓，除非后续获得单独明确批准；未执行任何Git操作。

## 当前阶段记录

- 2026-09-02：接受ADR-0034并同步主详设、BootROM/OTP/Provisioning/Manufacturing专题和OpenSpec；未修改产品、Vendor、RTL或测试代码，未授权实施。
- 2026-07-29：接受ADR-0026并同步主详设/OpenSpec/专题/基线/任务；设计已批准但实施未授权。
- 2026-07-29：针对第10章单薄问题完成详细设计补充并建立OpenSpec `device-personalization-and-provisioning-v1`；未修改两个代码仓、Vendor快照或测试工作簿，未执行Git、构建或安全测试。
- 2026-07-27：负责人批准ADR-0022最终Measurement逻辑ABI，OPEN-DESIGN-007关闭；固定8/16 slot与generation候选被替代，当前只等待OPEN-DESIGN-021最大实例容量、OPEN-CONFLICT-006物理参数及OPEN-DESIGN-015 SPDM wire。未修改两个代码仓、Vendor快照或测试工作簿，未执行Git、构建或安全测试。
- 2026-07-27：形成ADR-0022/OpenSpec `measurement-table-abi-v1`候选并合入主详设第9章；OPEN-DESIGN-007进入`proposal_ready`，等待负责人一次性批准六组决策。未修改两个代码仓、Vendor快照或测试工作簿，未执行Git、构建或安全测试。
- 2026-07-21：创建正式详设主入口和任务分解；DD-00完成，DD-01启动。当前未修改公司代码、未执行构建或测试。
- 2026-07-21：登记SRC-0021并建立INV-SEC/CE-SEC流程；创建INV-SEC-001/002，等待按顺序执行只读调查。
- 2026-07-21：完成INV-SEC-001只读调查并形成CE-SEC-001；确认默认BootROM无eHSM/FMC生产链、可选demo使用stub，已回填BootROM/eHSM/安全启动首轮设计。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-21：完成INV-SEC-002并形成CE-SEC-002；确认FMC/GSP默认仍为hello-world、验证链止于stub/内存measurement且无loader/jump；新增OPEN-CONFLICT-003 GSP地址域冲突。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-22：接入安全软件开发主计划和实施任务模板；DD-02/DD-03及公共ABI成为W0近期闭环项，尚未创建活动编码任务。
- 2026-07-22：形成W0首轮设计评审包，当时提出跨stage Handoff候选ABI并补齐统一错误设计；Handoff后由ADR-0004裁决为当前不采用。
- 2026-07-22：登记W0-R1批准状态和R1-11；自检位图采用Bootloader定义、Host错误；OSR Host通用业务代码优先复用，R1-05 Handoff仍待评审。未修改两个代码仓。
- 2026-07-22：接受ADR-0004；关闭OPEN-CONFLICT-002/003，采用Die1独立Measurement实例和GSP NoC/system canonical地址；R1-05当前不采用。未修改两个代码仓。
- 2026-07-22：负责人明确DD-02双路径；建立baremetal Vendor Demo完整能力case catalog和gsp产品`bl_demo`移植边界。产品payload/编排以安全软件方案为准，尚未授权编码。
- 2026-07-22：接受ADR-0007；2 MiB安全RAM改为P1生命周期复用布局，BootROM/FMC尾部回收，PMP/RMP/MMP常驻，GSP/Measurement/Mailbox连续且SPDM并入GSP；未修改代码仓。
- 2026-07-22：接受ADR-0006并进入B0-R2；建立2 MiB安全RAM总体边界、Mailbox follow Vendor和RAS reset边界，登记OPEN-CONFLICT-006。未修改代码仓。
- 2026-07-23：完成INV/CE-SEC-007；建立OPEN-CONFLICT-007并冻结通用Mailbox禁用规则、64字节cache line和64位微秒timer合同。随后接受ADR-0010，采用Vendor direct合同关闭该冲突，并冻结三阶段首版poll及RAS未ready终态。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-23：完成INV/CE-SEC-008并接受ADR-0011；形成eHSM Host adapter详细合同，冻结首版单在途/cache/timeout/retry/late-response规则。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-23：接受ADR-0012；回填BootROM/FMC/GSP/Host adapter与anti-rollback，冻结LCS直读、BootROM compare-only、GSP加载Vendor FW和重新下发边界。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-23：接受ADR-0013并形成eHSM产品operation profile；回填GSP/Host adapter/secure boot及开放问题。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-23：批准Measurement→FMC epoch路径并新增Measurement Table接口详设；未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-24：完成INV/CE-SEC-010；只读确认2 MiB双地址、16路eHSM IRQ编码、当前linker混合视图及PMA/Firewall缺口，并回填B0-R2平台输入门禁。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-24：接受ADR-0016并关闭GSP/OMP产品角色子项；明确主详设是最终单一完整交付，建立专题合入规则和整本目录状态。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-24：完成INV/CE-SEC-011；确认Vendor 16字节Version Counter的编码、BL候选暂存和FW启动OTP提交/readback路径，发现其与“GSP加载Vendor FW且FMC在release前提交counter”的批准链冲突，登记OPEN-CONFLICT-009并合入主详设第9章。未修改两个代码仓或Vendor快照，未执行Git、构建或测试。
- 2026-07-24：负责人决定Counter细节延期；DD-03按16字节值和默认存在的FMC接口继续，Vendor绑定移至实现/EMU前门禁。
- 2026-07-24：把已批准的Package/Manifest/公共ABI/制包与测试合同完整合入主详设第3章；算法profile、最终地址、Counter绑定、Measurement ABI和Runtime依赖继续作为显式开放项。未修改两个代码仓或Vendor快照，未执行Git、构建或测试。
- 2026-07-24：接受ADR-0017；冻结SHA-256+RSA-2048-PSS+AES-128-CBC、SHA-256+ECDSA-P256+AES-128-CBC、SM3+SM2+SM4-CBC三套产品Profile，关闭OPEN-DESIGN-004并登记OPEN-DESIGN-011。未修改两个代码仓或Vendor快照，未执行Git、构建或测试。
- 2026-07-24：主详设第4章完成规范性合入；DD-02主体设计转为`integrated_with_platform_inputs`，真实MMIO、PMA、deadline/RAS数值及baremetal二级case继续跟踪。未修改两个代码仓、未执行Git/构建/测试。
- 2026-07-24：主详设第5章完成规范性合入；2 MiB生命周期/权限/清零主体转为`integrated_with_open_bindings`，最终PC/linker、容量、PMA、Firewall和Runtime顺序继续跟踪。未修改两个代码仓、未执行Git/构建/测试。
