# Design Context Pack

> 本文件用于交给 GPT 做设计裁决。Codex 生成 context pack 时只整理仓库上下文，不做设计判断。

## 1. 当前仓库状态

| 字段 | 内容 |
|---|---|
| Current git commit hash | `c9e5a6383613c032a891b905c8dc15fdb8c0bf2c` |
| Working tree status | 存在未提交的流程/skill 相关变更与新增目录；本次仅新增 `.context/design_context_pack.md`，不修改安全方案正文。 |
| Context pack generated at | 2026-04-27 Asia/Shanghai |
| Requested topic | 针对 `security_workflow/03_detailed_design/03_detailed_design_master.md`：SEC1 也需要加密；补充内容直接进入 md 并生成可下载文件；章节顺序调整为 Key/Cert 放到安全启动、认证、debug 后；统一大/小标题编号；增加板级安全设计内容。 |

Working tree status 摘要：

```text
 M codex/skills/ngu800-security/SKILL.md
 M codex/skills/ngu800-security/prompts/03B_逐章生成详设.md
 M codex/skills/ngu800-security/prompts/09_最终检查与增量更新.md
 M codex/skills/ngu800-security/rules/workflow_rules.md
?? .context/
?? 00_project/
?? 05_traceability/
?? change_requests/
?? prompts/
?? tools/check_cr_sync.py
```

## 2. inputs_manifest 摘要

| Source ID | 文件 / 来源 | 类型 | 状态 | 与本次主题关系 |
|---|---|---|---|---|
| `SRC-001` | `security_inputs/current_plan/安全方案.pdf` | 当前安全方案基线 | `draft` / `partial` | 可作为总方案和功能点基线；若与 eHSM、启动方案或后续冻结口径冲突，细节可调整。 |
| `SRC-002` | `security_inputs/ip_manuals/ehsm/` | eHSM 资料目录级策略 | `confirmed` / `preferred` | eHSM 是 Root of Trust 与密码服务核心输入；SEC1 加密若升级为强制要求，需要检查 eHSM 固件字段、key slot、KDF branch、OTP/eFuse 排布。 |
| `SRC-003` | `security_inputs/soc_arch/启动方案.pdf` | 启动方案 | `draft` / `preferred` | 启动流程主参考；当前已确认 `sec1` 从 NOR Flash 加载，直接影响 SEC1 加密、验证、装载顺序。 |
| `SRC-004` | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | 安全子系统硬件方案 | `draft` / `partial` | 用于安全子系统、管理子系统、集成边界判断；与板级章节和接口边界相关。 |
| `SRC-005` | `security_inputs/board/管理子系统.pdf` | 管理子系统方案 | `draft` / `preferred` | 管理子系统总体架构和流程原则上遵循；涉及安全的内容需二次裁决，不直接继承弱化安全假设。 |

## 3. constraints 摘要

| Constraint ID | Category | Statement 摘要 | Status | 与本次主题关系 |
|---|---|---|---|---|
| `C-ROOT-01` | Root of Trust | Root of Trust 必须在 eHSM；BootROM 不持有 Root Private Key。 | `[CONFIRMED]` | SEC1 解密/验签所需密钥和私钥边界必须落在 eHSM，而不是 BootROM 或管理核。 |
| `C-BOOT-01` | Boot | 所有镜像必须经过安全子系统验签。 | `[CONFIRMED]` | 当前约束明确“验签”，未明确“SEC1 必须加密”；本次变更可能需要 GPT 裁决是否升级约束。 |
| `C-BOOT-02` | Boot | Boot 顺序必须由安全核控制；Host 不允许直接拉起 MCU。 | `[CONFIRMED]` | SEC1 从 NOR 加载并由 BootROM/eHSM 路径验证，后续由 SEC1/SEC2 收口。 |
| `C-BOOT-03` | BootROM | BootROM 不实现复杂加解密，只做最小加载、eHSM mailbox 调用、跳转。 | `[CONFIRMED]` | 若 SEC1 强制加密，解密动作不能由 BootROM 软件实现复杂 crypto，应由 eHSM/安全子系统承担。 |
| `C-IF-01` | Interface | 所有密码操作必须走 eHSM。 | `[CONFIRMED]` | SEC1 解密、镜像验签、key unwrap/KDF 均需通过 eHSM 能力表达。 |
| `C-KEY-01` | Key | 私钥不可导出，不能被 Host/BootROM/管理核读取。 | `[CONFIRMED]` | SEC1 加密涉及 FW Encrypt Key/KEK/CEK 时必须保持 non-exportable 边界。 |
| `C-KEY-02` | Key | Key 必须绑定生命周期。 | `[CONFIRMED]` | SEC1 加密密钥启用、调试态/RMA 态行为需与 lifecycle 绑定。 |
| `C-DEBUG-01` | Debug | USER 态关闭调试能力，禁止 JTAG。 | `[CONFIRMED]` | 板级安全章节纳入 master 时需保持 USER/PROD JTAG 默认关闭。 |
| `C-DEBUG-02` | Debug/RMA | DEBUG / RMA 必须认证。 | `[CONFIRMED]` | 板级 JTAG、OOB debug、RMA 入口必须经认证和 scope 控制。 |
| `C-HOST-01` | Host | Host 不可信，只能投递固件/触发流程/读取状态。 | `[CONFIRMED]` | 管理子系统/OOB 不应因为物理独立而高于 Host 信任级别。 |
| `C-BOARD-01` | Board/OOB | 管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决。 | `[CONFIRMED]` | 支撑把 `05_board_security.md` 内容加入 master，同时保留“系统流程可遵循、安全细节需裁决”的口径。 |
| `C-BOARD-02` | Board/OOB | 带外管理通道不得成为安全策略绕过路径。 | `[CONFIRMED]` | BMC/OOB/SMBus/I3C/JTAG/PCIe VDM 等只能作为受控链路。 |
| `C-BOARD-03` | Board/OOB Debug | JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制。 | `[CONFIRMED]` | 板级章节并入 master 时必须同步 JTAG 高风险入口策略。 |
| `C-BOARD-04` | Board/OOB DMA | 管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须隔离和审计。 | `[CONFIRMED]` | 影响 master 中 board/OOB、interface/mailbox、安全状态机内容。 |
| `C-ATT-01` | Attestation | 必须支持设备认证。 | `[CONFIRMED]` | 板级 binding、debug state、SEC1/SEC2 measurement 可能影响证明报告。 |
| `C-MFG-01` | Manufacturing | 必须定义 Root Key 灌装与锁定流程。 | `[CONFIRMED]` | SEC1 加密、board binding、JTAG 测试路径锁定可能影响制造流程。 |

## 4. baseline 摘要

| Baseline Topic | 当前裁决 | Status | 与本次主题关系 |
|---|---|---|---|
| Root of Trust | eHSM 是唯一 Root of Trust；Root Key 在 eFuse/OTP 安全区，BootROM 不持有私钥。 | `CONFIRMED` | SEC1 加密和密钥体系调整不能改变 eHSM RoT 归属。 |
| Boot / First Verifier | `BootROM -> SEC1(NOR/本地) -> SEC2(Host 下发) -> 子系统 FW`；First Mutable Stage = SEC1；First verifier = eHSM；BootROM 只做最小加载与编排。 | `CONFIRMED` | 本次 SEC1 也需要加密会影响 SEC1 镜像处理规则、FW header、verify/decrypt 顺序。 |
| Key / Cert | 私钥不出 eHSM；eHSM 负责 verifier、KDF、counter、lifecycle 绑定；首版证书/anchor 模型部分已收敛。 | `CONFIRMED / 部分冻结` | Key/Cert 章节拟移动到 boot/attestation/debug 后统一介绍；SEC1 加密可能要求 FW Encrypt branch 从“预留/可选”变为“强制”。 |
| Attestation | 需要 measurement table、设备认证、report、nonce、签名与证书/anchor 对接。 | `CONFIRMED / 部分冻结` | 板级 binding、SEC1/SEC2 measurement、加密策略变化可能影响 report 内容。 |
| Debug / Lifecycle | USER 态关闭 debug；DEBUG/RMA 必须认证；scope、expire、audit、lifecycle gating 需要收口。 | `CONFIRMED / 部分冻结` | 板级 JTAG 和管理子系统 debug 路径必须与该口径一致。 |
| Interface / Mailbox | FW Header 和 mailbox command model 是冻结敏感项，影响 BootROM/SEC/Host/eHSM 对接。 | `部分冻结` | SEC1 加密可能影响 `VERIFY_SEC1`、`VERIFY_IMAGE`、header 的 enc 字段和错误码。 |
| Manufacturing / RMA | Root Key 灌装、锁定、USER freeze、测试 trust 清理、RMA auth/audit 需要定义。 | `CONFIRMED / 部分冻结` | SEC1 加密密钥灌装、板级 JTAG 测试路径锁定、board binding 可能影响制造流程。 |
| Board / OOB | BMC/OOB/管理子系统可承载管理流程，但不进入 Root of Trust；OOB 信任级别不高于 Host。 | `CONFIRMED` | 本次明确要求把板级安全设计加入 master，应采用 `05_board_security.md` 的正式章节内容。 |

## 5. 与本次主题相关的原文摘录

| 文件 | 行号 | 原文摘录 | 为什么相关 |
|---|---|---|---|
| `security_inputs/inputs_manifest.md` | 12-18 | `SRC-002` eHSM 已定义固件字段、OTP/eFuse 排布、key slot 语义、计数器和生产阶段操作优先按 eHSM 定义设计；`SRC-003` 确认 `sec1` 从 NOR Flash 加载；`SRC-005` 管理子系统总体架构和流程原则上遵循，安全细节需二次裁决。 | 三个输入源分别约束 SEC1 来源、eHSM 加密/密钥能力、板级管理系统采用策略。 |
| `security_inputs/inputs_manifest.md` | 44-46 | `CHG-005` 新增管理子系统文档，影响总体架构、板级安全、接口边界、Host/管理子系统交互、启动/装载流程及风险章节。 | 说明板级安全不是孤立章节，可能影响 master 和接口/启动边界。 |
| `security_workflow/01_constraints.md` | 46-74 | `C-BOOT-01` 要求所有镜像经过安全子系统验签；`C-BOOT-03` 要求 BootROM 不实现复杂加解密。 | 当前约束强调验签，SEC1 强制加密尚需裁决；若加密，解密路径不能落到 BootROM 复杂软件实现。 |
| `security_workflow/01_constraints.md` | 177-185 | 管理子系统总体架构和流程可遵循；涉及 Root、debug、JTAG、secure boot、lifecycle、provisioning、firmware update、secure memory、OTP/eFuse、安全子系统访问时必须以安全基线为准。 | 支撑板级章节进入 master，并限定哪些内容不能盲从管理子系统文档。 |
| `security_workflow/01_constraints.md` | 227-258 | USER/PROD 默认关闭 JTAG；JTAG 必须经 debug auth、scope bitmap、板级 MUX；管理子系统 DMA 不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区等。 | 板级安全并入 master 时必须保留的安全红线。 |
| `security_workflow/02_baseline.md` | 25-31 | Root of Trust = eHSM；First Mutable Stage = SEC1；BootROM Role = 最小加载与编排；Host 不可信；Board/OOB 可承载管理流程但不进入 RoT。 | 本次 master 调整的全局基线。 |
| `security_workflow/02_baseline.md` | 77-85 | Secure Boot chain 为 `BootROM -> SEC1（NOR / 本地）-> SEC2（Host 下发）-> 子系统 FW`，所有 firmware 必须经过安全子系统校验。 | SEC1 加密要求必须落在这条启动链里。 |
| `security_workflow/02_baseline.md` | 108-126 | `SRC-005` 总体架构、模块职责、带外链路、电源/复位流程、单/双 Die 约束作为系统输入；BMC/OOB/Sideband 不高于 Host。 | 板级安全章节进入 master 的 baseline 依据。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 20-26 | 当前集成来源包括 baseline、boot、key/cert、attestation、lifecycle/debug、interface、manufacturing/RMA，未列入 `05_board_security.md`。 | master 尚未正式集成板级安全章节。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 34-40 | 当前已集成章节顺序为 baseline、Root/Key/Cert、安全启动、attestation、lifecycle/debug、interface、manufacturing/RMA。 | 与用户要求“Key/Cert 放到安全启动、认证、debug 后再统一介绍”冲突。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 230-232 | 紧邻出现 `# 二、Root of Trust、密钥体系与证书体系` 与 `# 5. Root of Trust、密钥体系与证书体系`。 | 典型大标题/源章节标题编号混排问题。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 779-783 | 当前 boot 章节写明“量产关键镜像建议支持可选加密，但首版最小必需是完整性和执行放行控制”。 | 与用户“SEC1 也需要加密”存在口径差异。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 943-953 | Verify path 包含 header 解析、key_id/signer hash、revoke、rollback、hash/signature 校验与“可选解密”；并允许 SEC2/运行期固件按策略选择签名 only 或签名+加密。 | 需要 GPT 裁决是否把 SEC1 从可选解密改为强制加密/解密，以及是否影响 SEC2/runtime。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 959-982 | SEC1 校验规则第 8 项为“可选解密”；SEC1/SEC2 发起 `VERIFY_IMAGE` 时第 7 项为“可选解密”。 | SEC1 强制加密的直接修改点候选。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 1098-1116 | 开放问题仍包含“首版是否默认启用关键镜像加密”；本章结论仍为 anti-rollback、吊销、可选解密。 | 若 SEC1 加密被裁决为确定需求，需要关闭/改写此开放问题及结论。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 2411-2415 | 接口命令表中 `VERIFY_IMAGE` 描述为“固件验签 / 可选解密”。 | SEC1/镜像加密若变为强制，需要同步接口描述或标注 SEC1 与其他镜像差异。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | 3148-3163 | `# 八、待补章节清单` 下仍列出 `## 8.2 板级安全设计`，内容是建议补充 BMC/OOB/Sideband trust、板级调试/更新/绑定等。 | master 仍把板级安全作为待补，而仓库已有正式 `05_board_security.md`。 |
| `security_workflow/03_detailed_design/01_boot.md` | 90-100 | SEC1 执行前必须经 eHSM 验证；后续固件必须经 SEC1/SEC2 调用 eHSM 验证；量产关键镜像建议支持可选加密。 | 源章节本身也含“可选加密”，若只改 master 会产生源/总文档不一致。 |
| `security_workflow/03_detailed_design/01_boot.md` | 269-299 | Verify path、SEC1 校验规则、后续镜像规则均包含“可选解密”；SEC2/运行期固件可选择签名 only 或签名+加密。 | SEC1 加密影响源章节、master、接口、header 字段的同步范围。 |
| `security_workflow/03_detailed_design/01_boot.md` | 413-417 | 开放问题包含“首版是否默认启用关键镜像加密”。 | SEC1 加密变更可能关闭或重写该开放问题。 |
| `security_workflow/03_detailed_design/02_key_cert.md` | 197-237 | FW Encrypt Key / KEK 用于固件机密性保护；FW Encrypt Branch 负责派生/解包 CEK；当前写法为“若首版不启用加密，可逻辑保留实现占位”。 | SEC1 强制加密可能要求 FW Encrypt branch 由可选占位变为至少对 SEC1 必选。 |
| `security_workflow/03_detailed_design/02_key_cert.md` | 303 | KDF label `NGU800:FW:ENC` 表示固件加密/解密 branch。 | SEC1 加密可复用或扩展该 key branch。 |
| `security_workflow/03_detailed_design/05_board_security.md` | 45-61 | `SRC-005` 总体架构、模块职责、带外管理链路、电源/复位流程、单/双 Die 约束作为系统流程输入；BMC/OOB/Sideband 不高于 Host。 | 可作为 master 新增板级安全章节的直接来源。 |
| `security_workflow/03_detailed_design/05_board_security.md` | 80-85 | 不得违反的边界包括 BMC/OOB 不成为 RoT 扩展、管理子系统不得直接修改 lifecycle/secure boot/debug/rollback/root/anchor、JTAG USER/PROD 不常开、DMA 不访问安全区域。 | 板级安全进入 master 时必须保留的安全底线。 |
| `security_workflow/03_detailed_design/05_board_security.md` | 219-242 | DMA 只能访问普通 staging/data buffer 和 firewall 显式允许区域；不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、recovery、证书/策略、安全共享缓冲区。 | 板级管理子系统和 SEC1/SEC2 安全边界相关。 |
| `security_workflow/03_detailed_design/05_board_security.md` | 284-310 | 双 Die 证明、board binding 是否参与 firmware verify decision、DMA/firewall/UserID、JTAG scope、MUX 控制权等仍未冻结。 | 板级章节并入 master 时仍需保留开放项，不应被误写成已冻结。 |
| `security_workflow/03_detailed_design/10_full_design.md` | 513-617 | 完整设计中仍保留“可选解密”“签名 only 或签名+加密”“即使首版不强制镜像加密也预留 enc_algo”。 | 可作为对照：已有更完整整合稿，但加密口径仍与用户本次新要求不同。 |
| `05_traceability/file_sync_checklist.md` | 9-18 | Boot flow、FW 加密、Board/OOB、Interface/Mailbox 等变更均映射到 constraints、baseline、详细设计、实现设计、traceability、decision/changelog 等同步文件。 | 本次需求影响安全主路径且超过两个文件，按流程应先形成 CR。 |

## 6. 已知冲突

| 冲突 ID | 冲突描述 | 涉及文件 | 当前状态 | 需要 GPT 裁决的问题 |
|---|---|---|---|---|
| `KX-001` | 用户要求“SEC1 也需要加密”，但当前 master、boot、key/cert、full design 中多处仍为“可选加密/可选解密/签名 only 或签名+加密”。 | `03_detailed_design_master.md`; `01_boot.md`; `02_key_cert.md`; `10_full_design.md`; `04_impl_design/efuse_key_fw_header_design.md`; `04_impl_design/mailbox_if.md` | `[OPEN]` | 是否将 SEC1 encryption/decryption 从可选能力升级为 confirmed hard requirement？该要求是否仅适用于 SEC1，还是适用于 SEC2/关键运行期镜像？ |
| `KX-002` | 当前 constraints/baseline 明确“所有镜像必须验签”，但没有明确“SEC1 必须加密”。 | `01_constraints.md`; `02_baseline.md`; `01_boot.md` | `[OPEN]` | 是否需要新增/修改约束和 baseline，以支撑 SEC1 强制加密，而不是只改 master 文案？ |
| `KX-003` | 用户要求 Key/Cert 放在安全启动、认证、debug 后统一介绍；当前 master 将 Root/Key/Cert 放在第二章。 | `03_detailed_design_master.md`; `10_full_design.md` | `[OPEN]` | 新的 master 章节顺序应如何定义？Root of Trust 是否与 Key/Cert 一起后移，还是 RoT 总体口径保留在架构/baseline 中、Key/Cert 细节后置？ |
| `KX-004` | master 中存在大标题中文序号和原章节数字编号叠加，例如 `# 二、...` 后紧接 `# 5. ...`。 | `03_detailed_design_master.md` | `[OPEN]` | 应采用哪一种统一编号规范：全书中文序号、阿拉伯数字多级标题，还是导出版重新编号并去除源章节标题？ |
| `KX-005` | master 当前集成来源未包含 `05_board_security.md`，并在待补章节中仍列出板级安全设计；但仓库已有正式板级安全章节。 | `03_detailed_design_master.md`; `05_board_security.md`; `10_full_design.md` | `[OPEN]` | 是否直接把 `05_board_security.md` 作为正式章节并入 master？是否同时删除待补列表中的板级安全待补项？ |
| `KX-006` | 用户要求“直接补充进 md 文件并生成一个可以直接下载的文件”，但当前流程要求影响主路径/多文件的变更必须先生成 CR，不得直接改正文。 | `file_sync_checklist.md`; `CR_template.md`; `prompts/01_generate_context_pack.md`; `03_detailed_design_master.md` | `[OPEN]` | 本次是否按新流程先由 GPT 生成 CR，再由 Codex 应用；导出版文件名和版本号如何确定？ |

## 7. 待关闭 TBD

| TBD ID | 原文位置 | TBD 内容 | Blocking Area | 需要的决策 |
|---|---|---|---|---|
| `TBD-SEC1-ENC-001` | `security_workflow/03_detailed_design/01_boot.md:415` | 首版是否默认启用关键镜像加密，还是先只冻结完整性与放行控制。 | Boot / FW Encryption | 裁决 SEC1 是否强制加密；如强制，是否关闭该开放问题或改为 SEC2/runtime 策略问题。 |
| `TBD-SEC1-ENC-002` | `security_workflow/03_detailed_design/03_detailed_design_master.md:1098` | 首版是否默认启用关键镜像加密。 | Master / Boot / FW Encryption | 与源 boot 章节保持一致；决定 master 中“可选解密”的替换范围。 |
| `TBD-CERT-001` | `security_workflow/03_detailed_design/02_key_cert.md:273` | 是否首版全面切到 X.509 取决于项目证书基础设施成熟度。 | Key / Cert | Key/Cert 后置章节中需保留或关闭该 TBD。 |
| `TBD-CERT-002` | `security_workflow/03_detailed_design/02_key_cert.md:280` | 是否要求 report 默认内嵌完整 cert chain。 | Key / Cert / Attestation | 影响 attestation 和 Key/Cert 章节的最终口径。 |
| `TBD-BOARD-001` | `security_workflow/03_detailed_design/05_board_security.md:284` | 双 Die 场景是否需要主/从 Die 分别出具证明，或由主 Die 汇总证明。 | Board / Attestation | 板级章节并入 master 时需保留为开放问题或由 GPT 裁决。 |
| `TBD-BOARD-002` | `security_workflow/03_detailed_design/05_board_security.md:285` | board binding 是否首版默认参与 firmware verify decision。 | Board / Boot / Manufacturing | 若 board binding 影响 SEC1/SEC2 验证，需要同步 boot、attestation、manufacturing。 |
| `TBD-BOARD-003` | `security_workflow/03_detailed_design/05_board_security.md:297` | DMA / firewall / UserID / 地址白名单仍为 `[TBD] firewall_access_rules`。 | Board / Interface / Firewall | 板级章节进入 master 时不能误标为已冻结，需要登记开放项。 |
| `TBD-BOARD-004` | `security_workflow/03_detailed_design/05_board_security.md:314-321` | JTAG scope bitmap、CPLD/JTAG MUX 控制、OOB provisioning proxy、DMA UserID/firewall region、电源异常入 report、双 Die report、测试路径锁定仍开放。 | Board / Debug / Attestation / Manufacturing | 是否由本次 CR 只集成为开放项，还是同步做部分裁决。 |
| `TBD-PROCESS-001` | `00_project/open_questions.md` | 变更流程试运行中需确认流程粒度、CR 命名、导出版命名。 | Change Management | 本次是流程试运行，可在 CR 中补充导出版命名和状态流转。 |

## 8. 推荐影响文件

| 文件 | 推荐动作 | 原因 |
|---|---|---|
| `change_requests/CR_template.md` | `inspect` | 本次影响两个以上文件、影响 boot/key/cert/attestation/debug/interface/manufacturing/board 主路径，应先生成 CR。 |
| `change_requests/CR-*.md` | `modify` | 建议由 GPT 根据本 context pack 生成正式 Change Request，Codex 再按 CR 修改仓库。 |
| `00_project/decision_log.md` | `modify` | SEC1 强制加密、章节顺序、板级章节并入 master 都属于设计/文档结构裁决，需要记录。 |
| `00_project/changelog.md` | `modify` | 后续应用 CR 时需要记录仓库级变更。 |
| `00_project/open_questions.md` | `modify` | 若仍有 SEC2/runtime 加密策略、board binding、JTAG scope 等未关闭问题，需要登记。 |
| `security_workflow/01_constraints.md` | `inspect / maybe modify` | 如果 GPT 裁决 SEC1 加密为强制要求，应新增或调整 boot/FW encryption 约束；当前 Codex 不做裁决。 |
| `security_workflow/02_baseline.md` | `inspect / maybe modify` | 若 SEC1 加密从建议变为 baseline，需同步 Boot/FW protection baseline。 |
| `security_workflow/03_detailed_design/03_detailed_design_master.md` | `modify` | 用户目标文件：章节顺序、编号、SEC1 加密口径、板级章节并入、待补清单清理。 |
| `security_workflow/03_detailed_design/01_boot.md` | `inspect / maybe modify` | 源 boot 章节含“可选加密/可选解密”，如果只改 master 会造成源/总文档不一致。 |
| `security_workflow/03_detailed_design/02_key_cert.md` | `inspect / maybe modify` | FW Encrypt Branch 当前是首版可选/占位，SEC1 强制加密可能要求 key branch、CEK/KEK、KDF 口径升级。 |
| `security_workflow/03_detailed_design/03_attestation.md` | `inspect` | SEC1 加密和 board binding 可能影响 measurement/report，但是否修改需 GPT 裁决。 |
| `security_workflow/03_detailed_design/04_lifecycle_debug.md` | `inspect` | 板级 JTAG/OOB debug 需要与 lifecycle/debug auth 章节一致。 |
| `security_workflow/03_detailed_design/05_board_security.md` | `inspect / no-change likely` | 作为 master 板级章节来源；除非 GPT 发现需补充，否则可不改源章节。 |
| `security_workflow/03_detailed_design/06_interface.md` | `inspect / maybe modify` | `VERIFY_IMAGE`、`VERIFY_SEC1`、共享内存、OOB 请求可能受 SEC1 加密和板级管理通道影响。 |
| `security_workflow/03_detailed_design/07_manufacturing_rma.md` | `inspect` | SEC1 加密密钥灌装、USER 前 JTAG 清理、board binding 可能影响制造/RMA。 |
| `security_workflow/03_detailed_design/10_full_design.md` | `inspect / maybe modify` | 该整合稿包含板级章节和“可选加密”旧口径；若作为导出版来源需同步。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | `inspect / maybe modify` | SEC1 加密会影响 FW header enc 字段、CEK/KEK、OTP/eFuse key/anchor/counter 表达。 |
| `security_workflow/04_impl_design/mailbox_if.md` | `inspect / maybe modify` | SEC1 解密/验证接口、错误码、policy flags、OOB 管理请求收口可能影响 mailbox。 |
| `security_workflow/04_impl_design/spdm_report.md` | `inspect` | board binding、debug state、电源/复位事件是否入 report 待裁决。 |
| `security_workflow/04_impl_design/manufacturing_provisioning.md` | `inspect` | 加密 key provisioning、JTAG 测试路径锁定、OOB provisioning proxy 可能影响制造流程。 |
| `security_workflow/06_traceability.md` | `modify after CR` | 若正文文件变更，需要同步追踪关系。 |
| `05_traceability/file_sync_checklist.md` | `inspect / no-change likely` | 已有 Boot/FW encryption/Board/OOB 映射，可用于执行 CR 的同步检查。 |
| 导出版文件 | `modify / create` | 用户要求生成可直接下载文件；建议在 CR 中明确文件名、版本号、来源 master。 |

## 9. 给 GPT 的问题

1. SEC1 加密是否应裁决为 `[CONFIRMED]` 强制要求？如果是，是否只限定 SEC1，还是 SEC2/关键运行期 FW 也必须加密？
2. 如果 SEC1 必须加密，现有“可选解密/可选加密/签名 only”口径应如何替换？是否需要同步 `01_constraints.md`、`02_baseline.md`、`01_boot.md`、`02_key_cert.md`、`06_interface.md` 和实现设计文件？
3. 新版 master 的章节顺序应如何统一？建议 GPT 裁决是否采用“总览/baseline -> 安全启动 -> 证明/认证 -> 生命周期/debug -> 接口 -> 板级安全 -> Key/Cert -> 制造/RMA -> 风险/开放项/附录”的结构，或给出更合适的顺序。
4. Root of Trust 概念是否应保留在前置架构/baseline 中，而把密钥体系和证书体系细节后置？这样可以满足用户“Key/Cert 后面统一介绍”同时避免读者找不到 RoT 基线。
5. `05_board_security.md` 是否直接并入 `03_detailed_design_master.md` 作为正式章节？并入后是否删除待补清单中的板级安全待补项？
6. 大标题和小标题编号应采用哪套规范？是否在整合 master 时删除源章节原始一级标题，统一重编号为单一体系？
7. 导出版文件名和版本号如何定义？例如是否生成 `NGU800_安全方案详细设计说明书_整合版_V2.4.md`，并由 master 同步导出。
8. 本次 CR 是否只处理 master/导出版，还是同时修正源章节，避免后续重新整合时旧口径覆盖新版 master？
