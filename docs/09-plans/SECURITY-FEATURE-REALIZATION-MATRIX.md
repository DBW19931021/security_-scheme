---
title: "NGU800P 安全 Feature 落实矩阵"
status: active
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0020
  - SRC-0034
owners: []
last_reviewed: 2026-08-25
supersedes: []
superseded_by: []
---

# 目的

把 SRC-0017 的系统安全能力和 SRC-0016 的软件落地方案拆成可设计、可实现、可验证、可签核的 Feature。本文是工作跟踪矩阵，不替代两份 PDF，也不把代码存在等同于功能完成。

本矩阵不是最终详细设计。最终目标是在本仓库`docs/03-architecture/`、`docs/04-interfaces/`和`docs/05-software-design/`形成可完全指导软件开发的正式详设，再在`../gsp-pmp-rmp-omp`完成产品实现；测试计划/工作簿保存在本仓库，eHSM全能力可执行测试和EMU自动化由运行在安全核上的独立baremetal软件栈在`../baremetal/components/ngu_security/`落实，不由GSP固件承载。

# 盘点口径

- 方案权威链：`SRC-0017 -> SRC-0016 -> accepted ADR/amendment/requirements -> implementation/tests/evidence`。
- 公司代码盘点范围：`../gsp-pmp-rmp-omp/components/security`、`solutions/bootrom`、`solutions/fmc`、`solutions/gsp`；盘点日期为 2026-07-21。
- 公司代码仓库存在既有未提交修改。本次只读盘点、未运行构建或测试，因此下表只表示“观察到的资产”，不表示当前工作区已验证。
- `SRC-0018` 只证明 OSR eHSM/Core 交付快照的实现，不证明 NGU800P SoC 行为。
- 现有代码仓中的 test/stub/demo 和历史 Expected 只作为代码事实与差距，不作为 Feature 目标、最终 oracle、验收或发布依据；EMU/产品不允许通过打桩或模拟成功提升成熟度。
- 代码成熟度是本矩阵的工作标记，不属于硬件事实状态：
  - `C0`：在上述安全范围未定位到对应生产实现。
  - `C1`：存在数据结构、demo、stub 或局部骨架。
  - `C2`：存在 Host/QEMU 测试或标准协议框架，但生产 provider/硬件通路未接入。
  - `C3`：目标固件与真实硬件通路已接入，尚缺 EMU Evidence。
  - `C4`：EMU 验收及发布 Evidence 完成。

# 正式详细设计交付结构

建议采用“一份主详设 + 可独立评审的专题详设”，避免单文档失控，同时保证开发只从主入口即可导航全部内容：

```text
security_-scheme/
  docs/03-architecture/          每个架构主题一份文档，如secure-boot.md、lifecycle.md
  docs/04-interfaces/            每个接口主题一份文档，如ehsm-mailbox.md、spdm.md
  docs/05-software-design/
    NGU800P安全软件详细设计.md    主入口、Feature 索引和全局约束
    bootrom.md / fmc.md / gsp.md 模块详设
    error-handling.md            跨模块错误契约
    logging-audit.md             日志、审计和清零
  tasks/active/                  总任务和小粒度INV-SEC调查任务
  evidence/code-investigations/ CE-SEC只读代码证据
  requirements/                 Requirement及主追溯
  tests/cases/                   版本化测试工作簿
  tests/matrices/                Feature/Requirement/code/case/Evidence 追踪
```

主详设必须固定产品范围、总体架构、Feature 索引、跨模块时序和全局约束；专题详设必须给出模块/文件职责、公共和私有 API 原型、结构体/字段/字节序、状态机、线程/中断/并发、内存/地址/缓存、错误与恢复、reset/掉电、安全分析、日志/清零、配置、测试钩子和目标代码位置。只写流程性文字而缺少这些开发契约，不能认为详设完成。

# Feature 落实矩阵

| Feature ID | Feature / 方案依据 | 当前软件资产观察 | 当前成熟度 | EMU 前必须冻结的细节 | 实现/测试出口 |
|---|---|---|---|---|---|
| SEC-FEAT-001 | 系统角色、信任边界和职责；SRC-0016 §1、SRC-0017 §4/§7 | CE-SEC-014复核确认BootROM/FMC/GSP当前入口均未形成production安全启动链，并已区分纯逻辑、平台和Vendor实施边界 | C1 | SoC、BootROM、C908、eHSM、FMC、GSP、Runtime、Host/BMC/OOB 的 RACI；可信输入、可写资源、调用方向和失败责任 | 一份批准的边界图与接口清单；每个后续 Feature 有唯一 Owner/consumer |
| SEC-FEAT-002 | eHSM 上电、ready、自检、`non_sec_boot`/LCS/Strap读取；SRC-0016 §2.1～§2.3、SRC-0034 | CE-SEC-001确认公司BootROM无生产adapter；Vendor OSR port的timeout为空实现；自检位图已裁决为Bootloader定义、Host错误 | C2 / design_approved_platform_pending | ADR-0031冻结`non_sec_boot`默认0、可靠值1最高优先级强制现有受限非安全、输入异常终止和BootROM只读快照；OPEN-DESIGN-024补物理绑定。ADR-0024冻结eHSM自主自检和值0时LCS异常路径；继续补ready deadline/MMIO数值 | 增加值0兼容、值1覆盖USER/Strap、自读/ECC/镜像异常、0→1烧写/复位重锁存；用例038/093/094覆盖无START_SELF_TEST、PASS/NOT_REQUIRED/FAIL和LCS异常；bit18=TRNG、bit19=unknown |
| SEC-FEAT-003 | eHSM native package、Header Overlay、typed-stage registry、制包；SRC-0016 §3.1～§3.5 | CE-SEC-009确认Vendor固定1024字节Header；SRC-0033/0036、CE-SEC-016、ADR-0030/0033确认末16B可复用且被认证，删除Manifest并冻结offset1008/1016/1020、Header CRC、`Code_Size`唯一长度和entry=load；CE-SEC-014确认当前旧Manifest/packager仍只属骨架 | C2 | production删除Manifest parser/dual format；实现CRC-32/ISO-HDLC双检；继续补OPEN-CONFLICT-006的Firewall/PMA、实际release matrix、MMP DDR和OPEN-DESIGN-019生成源 | [《安全固件包与Native Header Overlay合同》](../04-interfaces/secure-firmware-package.md)+三Profile golden/negative corpus；真实sign/encrypt pipeline；拒绝旧Manifest、旧8B reserved、错CRC/Overlay、旧080x、零签名和两仓私有ABI |
| SEC-FEAT-004 | eHSM 验签、解密、rollback 和结果返回；SRC-0016 §2/§3.4 | CE-SEC-009确认Vendor API只返回raw status，SoC输出包含Header+明文Code；CE-SEC-014复核当前公司flow仍调用stub并在loader/release前直接记录旧BSS Measurement | C1 | typed request/result、completion unknown、稳定Header post-check、Header CRC/Overlay/typed-stage policy、`target+1024`/`Code_Size`双摘要、buffer ownership/清零和raw error已冻结；继续补真实Vendor绑定和deadline | `image-verify-loader.md`；真实production adapter API；EMU无stub/simulated success；用例覆盖offset1008/1016/1020、CRC与Header稳定性和旧格式拒绝 |
| SEC-FEAT-005 | BootROM->FMC->GSP->Runtime安全release与fail-close；SRC-0016 §2.1～§2.3 | CE-SEC-014确认三阶段入口、loader、Firewall、Measurement commit、release和RAS闭环均未落地；目标链和函数级状态机已形成，ADR-0015冻结GSP全生命周期唯一eHSM Owner | C1 | eHSM FW走Vendor type0专用boot且不解释Overlay；PMP/RMP/MMP走type1 package+typed-stage fixed target；同一security service task从bootstrap转入runtime loop；继续冻结PMA/Firewall/MMP DDR及OPEN-DESIGN-010依赖/release输入 | `image-verify-loader.md`+每stage独立/链式/Owner并发测试；Vendor PASS不得直通release；单Runtime确定失败局部隔离，共享service unknown全局quarantine |
| SEC-FEAT-006 | Anti-rollback；SRC-0016 §3/§4/§6/§7，SRC-0017 §8.1 | Vendor手册/代码均为16字节；CE-SEC-011确认BL候选暂存和FW副作用 | C2 | ADR-0019统一`rollback_counter[16]`并要求FMC主动调用eHSM BL新增专用API；补齐command/packing/LCS/交付版本、寿命和掉电恢复 | 低/同/高、timeout unknown、readback、掉电/Host重发；区分candidate staged与commit proven |
| SEC-FEAT-007 | 正常运行态固件更新；SRC-0016 §2.5、§6.6 | 未观察到安全组件内完整 staging/verify/write/activate 实现 | C0 | 包格式、授权、staging 区、eHSM 校验、C908 写 Flash、active 切换、掉电恢复、版本/类型检查、审计 | 更新状态机 + Flash mock + power-cut point 表；用例 062/063/099 |
| SEC-FEAT-008 | OOB 恢复与板级安全边界；SRC-0016 §2.4/§8 | 未观察到安全组件内 OOB 生产协议；OOB MCU 本身不在当前公司代码范围 | C0 | Host/BMC/OOB/SoC 职责、outer wrapper、授权和 anti-replay、固定 FMC 分区、写保护、刷后 SoC 重验、审计 | 跨团队接口合同和联调 runbook；用例 065/066/083/116 |
| SEC-FEAT-009 | Measurement table与SPDM证明输入；SRC-0016 §4.3、SRC-0017 §9.1 | CE-SEC-014复核当前仍为私有BSS 16-slot旧结构，无共享Region、CRC/commit/finalize/snapshot，verify stub成功后直接record | C2 / design_approved_platform_pending | ADR-0022批准128B Header、实际count/length的紧凑128B Firmware列表、唯一128B SoC State、CRC-32C/32位commit及reset全清零；Entry表示verify/load/release-authorized事实，State提交后本boot不可修改；OPEN-DESIGN-021待最大实例容量 | `measurement-table.md` + 可变count/length、ABI round-trip/CRC/torn-write/reset全清零/重复实例/snapshot/eHSM-FW counter域；v0.2不改，下一版高亮 |
| SEC-FEAT-010 | SPDM/MCTP 设备认证与远程证明；SRC-0016 §4、SRC-0017 §9.2、SRC-0032 | 已有 libspdm/libmctp、Host/QEMU 框架和 responder service；crypto/cert/eHSM sign/物理 transport 仍为 stub 或模拟 | C2 / dynamic_topology_approved_profile_pending | ADR-0029已冻结SlotID 0运行链为RootHash+静态Issuer前缀+动态Firmware Alias Leaf，Alias签Report/transcript；仍需冻结算法/OID、完整链和消息上限、measurement index、nonce/replay、session/timeout、MCTP EID/MTU/共享内存/mailbox及真实provider ABI | 替换provider的ABI + Host requester验证静态链、动态Leaf/TCB绑定和Alias签名；用例071～075/077/115；EMU真实mailbox E2E |
| SEC-FEAT-011 | 生命周期模型与权限矩阵；SRC-0016 §5.1、SRC-0017 §8.3 | measurement/report 有 lifecycle 字段；未观察到生产状态读取与转换实现 | C1 | 状态枚举、允许转换、授权主体、不可逆点、复位后行为、各状态下 boot/key/debug/OTP/接口权限矩阵 | 逐状态 decision table + negative tests；用例 044/045/069/082 |
| SEC-FEAT-012 | Debug/RMA challenge-response；SRC-0016 §5.2/§9.3、SRC-0017 §8.3.1 | 未观察到 NGU800P 生产 debug auth；Vendor 快照有 eHSM/Core 参考实现 | C0 | token/证书格式、签名输入、UID/chip binding、TTL/nonce/replay、唯一SoC全局Debug enable、reset/timeout/revoke、RMA与审计；token含scope字段拒绝 | Host token generator/verifier + 状态机mock；用例078～082/106 |
| SEC-FEAT-013 | Key hierarchy、OTP Key 安装、KMU/用途隔离；SRC-0016 §6.1～§6.2、SRC-0017 §8.4 | 未观察到 `install_random_key`/`install_encrypt_key` 的 SoC adapter | C0 | key type/slot/size/usage、包装格式、CRC、调用权限、LCS、写入确认、KMU 加载、导出禁止、错误和清零 | API contract + slot/permission matrix + negative corpus；用例 043～048/110 |
| SEC-FEAT-014 | Device Attestation Key、证书链和证书 Slot；SRC-0016 §6.3～§6.5、SRC-0032 | 报告、cert provider 和签名链路存在，但证书/密码 provider 是 test stub | C2 / dice_single_layer_approved | Root→Intermediate→Device Issuer静态前缀、Cert0/1 A/B、ROM/FMC Hash输入、96B TCB Context、eHSM UDS/CDI/Alias派生、GSP固定Profile动态Leaf、运行链和reset重建已冻结；准确eHSM命令、Key Attribute、OID/DN/有效期/SM2编码仍阻断 | 静态前缀生成/验证工具+动态Leaf golden DER/负向corpus；用例067～075；EMU接入eHSM派生/Issuer签Leaf/Alias签Report和真实证书槽 |
| SEC-FEAT-015 | SoC Key Rotation；SRC-0015/0024、SRC-0016 §7、ADR-0021/0025/0026 | CE-SEC-013确认当前交付无专用轮换命令；通用安装接口拒绝USER/DEBUG且不可替代 | C0 | 已冻结一机一密、Table 34、16槽、Level顺序、slot14单Profile、slot13 UDS权限、Cert0/1、制造接口、Verify/Encrypt/Debug每类一次、HSM Bitmap、48字节封装和写Key→Bitmap→destroy→reset | OpenSpec `soc-key-rotation-v1`和`device-personalization-and-provisioning-v1`；Key Attribute/backend及Vendor/RTL/Flash/KMS/CA/MES绑定到齐后进入实现 |
| SEC-FEAT-016 | 密码服务、TRNG、健康检测和自检；SRC-0017 §8.5/§8.8 | 公司代码主要使用 provider stub；Vendor 提供算法/TRNG tests 和实现输入；自检位图已裁决 | C2 | 三套产品Profile所需能力进入内部typed service；ADR-0019确认不向Host开放GSP服务 | eHSM产品operation profile + 三Profile标准向量 + Vendor回归映射 + Host路由不可达测试 |
| SEC-FEAT-017 | Multi-Die启动、UCIe、SRAM Firewall、Die1 Debug；SRC-0016 §9、SRC-0017 §6/§10 | 未观察到对应生产安全代码；Die1 Measurement实例已裁决；CE-SEC-004确认2 MiB地址头和现有linker冲突 | C0 | Die0/Die1 owner、镜像实例、P1常驻/启动复用视图、Host/plaintext/Mailbox/Measurement/GSP/微核隔离、Firewall粒度/锁定、错误上报和RAS reset边界 | 跨Die时序+Firewall policy table；用例104～108；安全RAM见ADR-0006/0007，最终linker等待OPEN-CONFLICT-006剩余项 |
| SEC-FEAT-018 | PCIe/UART/JTAG/DFT/Host DMA 访问控制；SRC-0016 §1.3/§8、SRC-0017 §8.3.1 | 在本次安全组件盘点范围未建立方案到实现映射 | C0 | 每个接口的 LCS/Debug/Owner/白名单/地址域/锁定位/复位默认值；不可绕过路径 | 寄存器与软件 owner 映射；用例 083/091/101/108；RTL/EMU 联合验收 |
| SEC-FEAT-019 | 状态、错误、中断、日志、审计和敏感材料清零；SRC-0016 §2.2/§6.6、SRC-0017 §8.6～§8.7 | CE-SEC-001/002确认错误枚举过窄、demo返回值忽略、失败不记录measurement、slot错误时返回值与result状态不一致；ADR-0006已冻结RAS职责 | C1 | 错误码namespace、source/stage/severity、retry/Halt、RAS report/action request、中断清除、日志脱敏、审计、清零；security不得直接reset | 统一错误矩阵+fault injection hook；用例041/091/102/109/110/116；验证reset只由RAS执行 |
| SEC-FEAT-020 | 制造灌装与安全初始化；SRC-0016 §6.5～§6.6、SRC-0017 §7/§8.4 | 未观察到端到端量产流程实现；Vendor otptool 仅是 eHSM 输入 | C0 | 制造站/KMS/HSM/设备职责、输入包、身份认证、顺序/幂等、LCS 转换、回读确认、失败报废/返修、审计和敏感数据处理 | 可复跑 dry-run + 每步 Evidence 模板；用例 043～048/067～069/116 |

# 当前关键结论

1. 可复用基础主要集中在镜像/header/manifest、policy、measurement、MCTP/SPDM 标准框架，以及 OSR Host 通用业务代码；测试 runner/build 机制可复用，但其 stub flow 和历史 Expected 不能作为目标。OSR 平台地址/timer/reset 仍必须隔离到 NGU800P port 后核实。
2. 当前最重要交付不是继续扩展 Feature 清单，而是把生产接口和行为写进正式详设：eHSM adapter、OTP/eFuse/LCS、key/cert、真实 crypto provider、Flash 更新、OOB、多 Die/Firewall 和统一错误模型。
3. CE-SEC-014确认当前`verify_image()`仍由eHSM stub驱动，BootROM路径仍是demo，FMC/GSP入口不是产品链，产品source graph还包含stub/null provider且linker仍是旧080x；因此这些资产只能标为C1/C2，不能作为EMU功能已完成。
4. 当前 100 条测试全部标为 P0，不利于一个月内排程和放行判断；必须在不覆盖 v0.2 的前提下，由负责人批准后分为发布阻断、Feature 必测和兼容/扩展三层。
5. OPEN-CONFLICT-001～005/007～010/013及OPEN-DESIGN-003/004/006/007/008/011/013/014/016/017/018/020均已关闭。ADR-0025/0026及SRC-0024已冻结一机一密、Table 34、16槽、Level顺序、slot14单Profile、slot13 UDS权限、证书和制造接口；OPEN-CONFLICT-012管理RTL实现绑定。OPEN-CONFLICT-006继续阻断最终地址/linker，OPEN-CONFLICT-011阻断Key Rotation编码，OPEN-DESIGN-015/019/021分别保留SPDM wire Profile、registry/release流程和Measurement最大实例容量。

# Feature 完成定义

一个 Feature 只有同时满足以下条件，才能从 C2/C3 进入 C4：

- 方案细节已进入 SRC-0016 后续有效版本或 accepted amendment，并符合 SRC-0017。
- 对应章节已进入 `security_-scheme/docs/03-architecture/`、`docs/04-interfaces/` 或 `docs/05-software-design/` 的正式详设，开发无需依赖聊天记录或未受控笔记即可编码。
- Requirement/Scenario、接口 ABI、状态机、错误和安全边界均已冻结。
- EMU/产品实现不存在 demo/stub/simulated success、test key/cert/provider、未批准hardcode或silent fallback；未完成能力不可达、禁用或fail-close，且批准的Host/unit/QEMU回归通过。
- 至少覆盖正向、异常、边界、权限/LCS、timeout/reset、掉电或故障注入、敏感材料清理（适用时）。
- EMU 执行绑定 C908/eHSM/RTL/BootROM/FMC/GSP/Host 的确切版本和配置。
- 原始日志、命令、配置、镜像/向量哈希和结论保存在 `evidence/`，并完成 requirement->case->evidence 追踪。

# 待负责人裁决/确认

- SPDM对Die1 Measurement逐条/聚合呈现的具体profile，但不得丢失Die1实例/release语义。
- C908/eHSM对baremetal System Address范围的实际访问、最终`NON_CACHEABLE`或`HARDWARE_COHERENT`属性、barrier可见性、data clean/invalidate零调用和Firewall Evidence；产品不得建立Local/System转换。
- SRC-0016/SRC-0017 的正式 Owner、批准版本、适用芯片修订。
- MD5、SHA-512/256、AES-192-XTS、DES/TDES 的发布定位和门禁级别。
- EMU 的首个可用日期、RTL/eHSM/BootROM 基线和可用故障注入/观测能力。

# Change history

- 2026-07-27：接入CE-SEC-014编码前落实性审查；更新SEC-FEAT-001/003/004/005/009的当前代码事实和实现阻断，不提升任何Feature成熟度。
- 2026-07-24：依据CE-SEC-011更新SEC-FEAT-006；区分BL candidate staged与FW OTP committed/readback proven，登记OPEN-CONFLICT-009并阻断counter提交/FMC release实现。
- 2026-07-24：负责人决定Counter细节延期；SEC-FEAT-006/009按16字节抽象接口继续，准确Vendor绑定移至实现/EMU前门禁。
- 2026-07-24：接受ADR-0015并关闭OPEN-DESIGN-009；SEC-FEAT-005加入GSP全生命周期唯一eHSM Owner及typed queue门禁，OPEN-DESIGN-010等待Runtime资料。
- 2026-07-23：依据CE-SEC-009更新SEC-FEAT-003～005；登记Vendor原生包冲突、Manifest v1和typed verify/loader/release候选合同，未提升代码成熟度。
- 2026-07-22：按ADR-0006更新SEC-FEAT-005/017/019；纳入2 MiB安全RAM、Mailbox follow Vendor和RAS reset职责，登记OPEN-CONFLICT-006阻断范围。
- 2026-07-22：按ADR-0007将SEC-FEAT-005/017改为P1生命周期布局；BootROM/FMC尾部回收、PMP/RMP/MMP常驻、GSP/Measurement/Mailbox连续且SPDM并入GSP。
- 2026-07-22：接受ADR-0004；SEC-FEAT-003/005/009/017解除OPEN-CONFLICT-002/003标记，写入Die1独立Measurement实例和GSP NoC/system canonical地址；当前不采用Handoff。
- 2026-07-22：自检位图按Bootloader定义关闭冲突，SEC-FEAT-002/016解除该冲突标记；登记现有test/stub非目标依据、EMU/产品禁止打桩以及OSR Host通用业务代码复用边界。
- 2026-07-21：基于两份方案、Vendor 快照、v0.2 测试用例和公司软件栈只读盘点，建立首版 Feature 落实矩阵。
- 2026-07-21：按负责人确认，将正式详设、任务规划、测试工作簿和开发追溯的主维护位置统一到 `security_-scheme`；软件实现和可执行测试分别保留在两个代码仓。
- 2026-07-21：以CE-SEC-001替换SEC-FEAT-001/002/005/019的泛化代码观察，并将示例目录树修正为当前实际专题文件布局。
- 2026-07-21：以CE-SEC-002更新SEC-FEAT-001/003～006/009/019，登记OPEN-CONFLICT-003；未提升任何Feature成熟度。
