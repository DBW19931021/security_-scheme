# NGU800 Security Design Context Pack

> 用途：交给 ChatGPT / 项目组 / security owner 进行详细方案复核与裁决。
> Codex 在本文件中只整理仓库上下文、证据、冲突、影响面与待裁决问题，不新增 `[CONFIRMED]` 安全结论，不修改安全方案正文。

## 1. 当前仓库状态

| 字段 | 内容 |
|---|---|
| 当前工作目录 | `/home/may-pc/share/code/ngu800/secure/security_-scheme/ngu800_security_solution_flow_with_gpt` |
| Current git commit | `df815e2` |
| Context pack date | 2026-05-07 Asia/Shanghai |
| Work mode | `context-pack` |
| Requested topic | 基于 `security_workflow/03_detailed_design/10_full_design.md`，使用 superpowers / NGU800 security skill 为 ChatGPT 做详细方案设计复核上下文包。 |
| 本次 Codex 动作边界 | 仅更新 `.context/design_context_pack.md`；不修改 `10_full_design.md`、章节正文、实现设计、code rules、traceability 或 CR。 |

Working tree 当前已有多处未提交变更和删除项。本 context pack 只把这些状态作为风险输入，不判断其来源，也不回退任何文件。

关键状态：

- `security_workflow/03_detailed_design/10_full_design.md` 当前为已修改状态。
- `security_workflow/03_detailed_design/03_detailed_design_master.md` 当前在工作树中显示为删除。
- `security_workflow/03_detailed_design/03_detailed_design_master_v2.4.md` 当前在工作树中显示为删除。
- `codex/skills/ngu800-security/` 与 prompts/templates/rules 有多处未提交变更。
- `change_requests/CR-0002-readable-source-references.md` 为未跟踪文件。

## 2. 本次上下文包目标

用户希望形成的协作方式是：

- ChatGPT / 项目组 / security owner：负责安全架构、方案设计、取舍和冻结裁决。
- Codex：负责把已裁决内容落到仓库文件、实现设计、规则、traceability、测试和代码。

因此，本轮建议采用 A 路线：

| 路线 | 产物 | 作用 |
|---|---|---|
| A / context pack | `.context/design_context_pack.md` | 给 ChatGPT / owner 提供复核材料和待裁决问题。 |
| 后续 B / proposal CR | `change_requests/CR-xxxx-*.md` | ChatGPT / owner 裁决后，由 Codex 起草或应用 CR。 |
| 后续 C / accepted apply | workflow / impl / rules / traceability | 仅在 CR 或 owner 明确批准后同步正文和实现文件。 |

## 3. 输入资料摘要

| Source ID | 来源 | 当前状态 | 对本次复核的意义 |
|---|---|---|---|
| `SRC-001` | `security_inputs/current_plan/安全方案.pdf` | `draft` / `partial` | 当前方案基线，可作为总体功能输入；细节若与优先级更高来源冲突，需要二次裁决。 |
| `SRC-002` | `security_inputs/ip_manuals/ehsm/` | `confirmed` / `preferred` | eHSM 资料是 Root of Trust、key slot、OTP/eFuse、counter、生产阶段操作等安全细节的优先来源。 |
| `SRC-003` | `security_inputs/soc_arch/启动方案.pdf` | `draft` / `preferred` | 启动方案确认 `sec1` 从 NOR Flash 加载，直接影响 SEC1 验证、解密、放行顺序。 |
| `SRC-004` | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | `draft` / `partial` | 用于安全子系统、管理子系统、集成边界判断。 |
| `SRC-005` | `security_inputs/board/管理子系统.pdf` | `draft` / `preferred` | 管理子系统总体架构和流程可参考；涉及安全边界时不能直接继承弱安全假设。 |

当前 manifest 中已记录的输入/冲突处理结论：

- `CF-001` 至 `CF-003` 已按现有 baseline 处理。
- `CHG-001` 至 `CHG-005` 已纳入输入管理。
- `SRC-005` 对板级安全、接口边界、Host/管理子系统交互、启动/装载流程、风险章节有影响。

## 4. 已批准或已记录结论摘要

以下为仓库中已有来源表达的结论，本 context pack 不新增确认级裁决。

| 来源 | 结论摘要 | 当前含义 |
|---|---|---|
| `CR-0001-sec1-encryption-fw-protection-master-sync.md` | SEC1 需要签名加密；BootROM 不实现复杂解密；SEC1 解密/unwrap 由 eHSM/安全服务承担；SEC2/运行期镜像加密策略保留产品策略空间。 | SEC1 加密已作为 CR-0001 的核心收敛点，但 `10_full_design.md` 仍标为待 GPT / 人工复核。 |
| `CR-0002-readable-source-references.md` | 改善来源引用可读性。 | 不改变安全架构本身。 |
| `00_project/decision_log.md` / `DEC-0001` | SEC1 sign+encrypt 已记录为设计决策。 | 可作为后续同步正文/实现设计时的 approved source。 |
| `00_project/decision_log.md` / `DEC-0002` | Root/Key/Cert 详细说明放在 boot、attestation、debug 之后。 | 影响全书章节顺序和导出版结构。 |
| `00_project/decision_log.md` / `DEC-0003` | 板级安全设计纳入 master/full design。 | `10_full_design.md` 应持续包含 board/OOB 章节，不应再把它视作待补。 |
| `00_project/decision_log.md` / `DEC-0004` | master/full design 统一编号。 | 需要防止源章节标题与导出版标题重复叠加。 |
| `00_project/decision_log.md` / `DEC-0005` | 源文件引用改为更可读的文件名/章节。 | 支撑 ChatGPT/owner 审查时快速定位证据。 |

`10_full_design.md` 当前整合版关键基线表达：

- 版本：V2.4。
- 状态：`CR-0001 applied，待 GPT / 人工复核`。
- 定位：整合版，不替代源章节；源章节仍是维护来源。
- Root of Trust：eHSM。
- First Mutable Stage：SEC1。
- First Cryptographic Verifier：eHSM。
- BootROM：最小加载与编排，不持有 Root Private Key，不实现复杂 crypto。
- Host：不可信。
- Board/OOB：不进入 RoT，不高于 Host。
- Manufacturing：USER freeze、key lock、debug disable、anti-rollback、SEC1 decrypt key / FW_KEK 锁定等为冻结敏感动作。

## 5. `10_full_design.md` 重点证据摘录

以下行号基于当前工作树文件，用于 ChatGPT/owner 复核时快速定位。

| 文件位置 | 摘要 | 设计含义 |
|---|---|---|
| `10_full_design.md:3-7` | V2.4，状态为 CR-0001 applied，待 GPT / 人工复核；章节源文件仍是事实来源。 | 不能把整合版直接当成最终冻结版；需要 owner 复核状态。 |
| `10_full_design.md:50-58` | eHSM RoT、SEC1 first mutable stage、eHSM first verifier、SEC1 sign+encrypt、BootROM minimal、Host untrusted、Board/OOB not RoT。 | 可作为本轮复核的当前 baseline 摘要。 |
| `10_full_design.md:521-534` | 镜像分类中 SEC1 来自 NOR，由 eHSM 验证，由 BootROM release；SEC1 must sign+encrypt；SEC2/later 通过 SEC1/SEC2 调 eHSM 验证。 | SEC1 加密口径已进入整合版；SEC2/later 仍需要策略裁决。 |
| `10_full_design.md:572-581` | Verify path 按 image_type/policy 处理 decrypt；SEC1 decrypt mandatory；SEC2/runtime sign+encrypt recommended/assumed，signature-only 可由显式产品策略允许。 | 最需要 ChatGPT/owner 细化的是 SEC2/PM/RAS/Codec 等后续镜像的默认策略和例外条件。 |
| `10_full_design.md:589-601` | `VERIFY_SEC1` 必须解析 header、检查 revoke/version/signature/hash，并执行 mandatory decrypt/unwrap；decrypt 失败阻断 boot；BootROM 不 fallback。 | 接口、错误码、BootROM 边界、eHSM key slot 都要同步。 |
| `10_full_design.md:605-614` | `VERIFY_IMAGE` 对后续镜像按策略处理 lifecycle、board_bind_flags、trust anchor、rollback、signature、decrypt。 | 后续镜像策略字段、policy 表和 board binding 需要定稿。 |
| `10_full_design.md:717-723` | Boot 冻结敏感项包括 VERIFY_SEC1 参数模型、release state、image_type 到 counter_id、USER non-secure boot、recovery trust。 | 这些项目影响接口冻结和实现设计。 |
| `10_full_design.md:729-733` | Boot 开放问题包括哪些非敏感 runtime image 可 signature-only、recovery image_type/signer、SEC1 role、non-secure maintenance、dual-die/board binding。 | 这些应进入 ChatGPT/owner 裁决清单。 |
| `10_full_design.md:1267-1274` | Attestation 冻结敏感项包括 Device Identity vs Alias、cert chain、measurement set、image protection policy、session binding、board/die binding。 | attestation 不是单独问题，会被 boot image policy 和 board binding 牵动。 |
| `10_full_design.md:1416-1419` | Lifecycle flow 中 TEST/DEV/MANUFACTURE/PROD/DEST 已表达；RMA 的 DEBUG/RMA 映射和独立编码仍有 TBD。 | RMA/debug 编码需要 eHSM/OTP 或安全 owner 裁决。 |
| `10_full_design.md:1516-1519` | eHSM big bitmap confirmed；NGU800 subsystem bit domain assumed；最终 bit-level mapping 和 SRC-005 JTAG targets 到 SoC/board scope 是 TBD。 | JTAG scope/MUX 不应被写成已冻结。 |
| `10_full_design.md:2051-2053` | CH0 mandatory；CH1/CH2 optional；不要伪造多通道支持。 | mailbox 实现和文档要避免过度承诺。 |
| `10_full_design.md:2096-2099` | mailbox header/token/length/caller_id SEC/C908 confirmed；lifecycle_state 只作为 quick reject assumed，最终以 eHSM/OTP state 为准。 | 接口层可以快速拒绝，但安全权威仍在 eHSM/OTP。 |
| `10_full_design.md:3042-3055` | 双算法结构和默认产品算法策略仍 TBD。 | 影响证书、签名、加密、boot policy 与产品 SKU。 |
| `10_full_design.md:3061-3064` | FW encryption branch 至少 SEC1 mandatory；later images policy-based；rollback floor 绑定 OTP counter。 | SEC1 已收敛；runtime encryption policy 仍未冻结。 |
| `10_full_design.md:3081-3107` | manufacturing key objects/user actions；Seed/UDS injection assumed；full cert chain provisioning TBD。 | 制造方案需要 owner 决定 root 注入模式、证书链写入和验收方式。 |
| `10_full_design.md:3499-3511` | USER freeze actions 包括 SECURE_BOOT_EN、DEBUG_AUTH_EN、JTAG_FORCE_DISABLE、ANTI_ROLLBACK_EN、FW_ENCRYPT_EN 至少 SEC1、key slot lock、test trust cleanup、lifecycle USER、audit。 | USER freeze checklist 可作为实现/产测闭环依据，但仍需和 eHSM field-level TRM 对齐。 |
| `10_full_design.md:3596-3600` | RMA 不允许 long-open debug、不允许 bypass challenge/auth、不允许 test trust/debug 残留、不允许长期开启 SEC1 decrypt bypass；RMA report/status assumed。 | RMA 策略已很明确，但 report/status 和 re-acceptance 仍需裁决。 |
| `10_full_design.md:3651-3656` | Manufacturing 冻结敏感项包括 Root injection mode、SEC2/later image encryption、OTP readback、provisioning chain、dual-die transaction、RMA re-acceptance。 | 制造冻结前必须关闭这些设计输入。 |
| `10_full_design.md:3686-3696` | CR-0001 settled：SEC1 encryption、source、eHSM decrypt、BootROM boundary、board security、chapter order。 | 说明 CR-0001 已落入整合版，但还要通过 GPT/人工复核关闭版本状态。 |
| `10_full_design.md:3697-3710` | Open items：runtime image signature-only policy、SEC2/later encryption、X.509 full chain、image protection policy、board binding、JTAG scope/MUX、DMA/firewall/UserID、OOB proxy、PowerBrake/report、RMA re-attestation/status。 | 这是最直接的下一轮裁决清单。 |
| `10_full_design.md:3714-3719` | 依赖 eHSM field-level TRM/key slots、管理子系统 field interface、产品安全策略、制造 workstation/HSM/KMS。 | 缺失资料会阻塞最终冻结。 |

## 6. 当前开放问题摘要

| ID | 来源 | 状态 | 需要裁决的核心问题 |
|---|---|---|---|
| `OQ-0001` | `00_project/open_questions.md` | Open | CR/GPT/Codex 流程试运行粒度、命名、状态流转是否固定。 |
| `OQ-0003` | `00_project/open_questions.md` | Open | SEC2/PM/RAS/Codec 等后续镜像是否默认加密，哪些可 signature-only。 |
| `OQ-0004` | `00_project/open_questions.md` | Open | board binding 是否参与 firmware verify decision。 |
| `OQ-0005` | `00_project/open_questions.md` | Open | JTAG scope bitmap 和板级 MUX 控制权、字段来源、bit-level mapping。 |
| `OQ-0006` | `00_project/open_questions.md` | Open | 管理子系统 DMA / firewall / UserID / 地址白名单如何冻结。 |
| `OQ-0007` | `00_project/open_questions.md` | Open | attestation report 中 image protection policy 放在哪里、如何编码。 |
| `OQ-0008` | `00_project/open_questions.md` | Open | OOB/BMC provisioning proxy 是否允许，允许时边界、认证和审计如何定义。 |

`security_workflow/06_traceability.md` 中当前 blocked / pending 方向：

- debug port 129-bit bitmap。
- JTAG scope / MUX。
- DMA region / UserID。
- PowerBrake / PG / FAULT / reset 入 report。
- image_type 到 counter_id mapping。
- SEC2/later image encryption policy。
- board binding 默认策略。
- shared memory final location。

## 7. 已知复核风险

| 风险 ID | 风险描述 | 影响 |
|---|---|---|
| `RISK-STATUS-001` | `10_full_design.md` 标为 `CR-0001 applied，待 GPT / 人工复核`，但正文中已有大量 `[CONFIRMED]` 表达。 | 需要 owner 明确 V2.4 是否进入 reviewed/frozen baseline，或继续保持 applied/pending 状态。 |
| `RISK-SOURCE-001` | `10_full_design.md` 说明源章节仍是事实来源，但当前工作树中 master 文件显示删除。 | 后续同步机制需要明确：`10_full_design.md` 是导出版、主维护源，还是临时整合输出。 |
| `RISK-SYNC-001` | SEC1 encryption 已进入 CR-0001 和 full design，但后续 SEC2/runtime 加密策略仍 open。 | 如果直接冻结全书，可能误把 assumed/recommended 策略当作 mandatory requirement。 |
| `RISK-BOARD-001` | Board/OOB 已纳入 full design，但 JTAG scope、MUX、DMA/UserID/firewall、OOB provisioning proxy 仍未冻结。 | 板级章节可以进入方案，但实现和 SoC/板级接口不能提前闭合。 |
| `RISK-ATT-001` | SPDM/report 实现设计仍是 starter 级别，full design 中 report 项目更多。 | 证明报告结构、image protection policy、board/die binding 需要进一步收敛后才能指导代码。 |
| `RISK-MFG-001` | Manufacturing 依赖 Root injection mode、OTP readback、provisioning chain、dual-die transaction、RMA re-acceptance。 | 产测流程和 eFuse/OTP 字段验收需要项目组/制造系统输入。 |
| `RISK-TEMPLATE-001` | skill/templates 已更新，要求更强的 Source Status / Rule Status 约束，但现有 `05_code_rules.md`、`06_traceability.md` 仍可能是旧格式。 | 后续 accepted apply 时可能需要做一次规则/追踪模板升级 CR，避免规则与 traceability 语义漂移。 |

## 8. 推荐影响文件

本轮不直接修改以下文件，只列出后续若进入 CR / accepted-apply 时的影响面。

| 文件 | 后续动作建议 | 触发条件 |
|---|---|---|
| `change_requests/CR-xxxx-*.md` | 新建或更新 proposal CR。 | ChatGPT/owner 对 V2.4 复核问题给出裁决后。 |
| `00_project/decision_log.md` | 记录新增或关闭的 owner 决策。 | V2.4 状态、SEC2/later encryption、board binding、JTAG/DMA/OOB 等任何项被裁决。 |
| `00_project/open_questions.md` | 关闭已裁决项，新增未解决依赖。 | ChatGPT/owner 输出明确结论后。 |
| `security_inputs/inputs_manifest.md` | 更新 source 状态或新增 eHSM/TRM/board field interface 输入。 | 项目组补充正式资料后。 |
| `security_workflow/01_constraints.md` | 同步新增/修正约束。 | 后续镜像加密、board binding、debug/DMA/OOB 等从 assumed/TBD 升级为 confirmed/proposed 时。 |
| `security_workflow/02_baseline.md` | 同步 baseline。 | 安全主路径裁决变化时。 |
| `security_workflow/03_detailed_design/*.md` | 同步章节正文。 | CR 被接受后。 |
| `security_workflow/03_detailed_design/10_full_design.md` | 重新导出或更新整合版状态。 | 源章节完成同步后。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 更新 eFuse/key slot/header/policy/counter mapping。 | eHSM field-level TRM、SEC2/later encryption、board binding 策略明确后。 |
| `security_workflow/04_impl_design/mailbox_if.md` | 更新 VERIFY_SEC1 / VERIFY_IMAGE / caller / channel / error code / shared memory。 | boot verify/decrypt policy、channel、shared buffer 明确后。 |
| `security_workflow/04_impl_design/spdm_report.md` | 更新 report schema、measurement、policy、board/die binding、RMA/debug 状态。 | attestation 冻结项裁决后。 |
| `security_workflow/04_impl_design/manufacturing_provisioning.md` | 更新 provisioning、USER freeze、RMA re-acceptance、dual-die transaction。 | 制造和 RMA 输入明确后。 |
| `security_workflow/05_code_rules.md` | 升级或同步代码规则。 | accepted apply 阶段，且实现设计足够稳定后。 |
| `security_workflow/06_traceability.md` | 同步需求到实现/测试追踪。 | 正文或实现设计更新后。 |

## 9. 建议交给 ChatGPT / owner 的裁决问题

1. `10_full_design.md` V2.4 当前应保持 `CR-0001 applied，待 GPT / 人工复核`，还是可以升级为 reviewed baseline？如果升级，版本号和状态如何写？
2. `10_full_design.md` 与源章节的关系如何定义：它是导出版、主维护文档，还是临时整合稿？当前 master 文件删除状态是否符合预期？
3. SEC2、PM、RAS、Codec、non-sensitive runtime image 的默认固件保护策略是什么：默认 sign+encrypt，还是默认 sign-only 并列白名单例外？
4. signature-only runtime image 的准入条件是什么：image_type、lifecycle、debug state、product SKU、board binding、release policy 是否都必须参与？
5. Recovery image 的 image_type、signer、trust anchor、rollback counter、decrypt policy 如何定义？是否允许 recovery 走独立 trust anchor？
6. board binding 是否参与 firmware verify decision？如果参与，是仅用于 attestation，还是参与 verify/decrypt/release 决策？
7. dual-die 场景中 board/die binding 和 attestation 是分别出 report，还是主 Die 汇总 report？
8. JTAG scope bitmap、eHSM 129-bit debug bitmap、板级 MUX 控制、SRC-005 JTAG target 到 SoC/board scope 的映射由哪个正式输入冻结？
9. 管理子系统 DMA / firewall / UserID / interrupt / reset 权限的字段来源、地址范围和默认拒绝策略如何定义？
10. OOB/BMC 是否允许作为 provisioning proxy？如果允许，认证、命令转发、数据可见性、审计和失败回滚边界是什么？
11. attestation report 中 image protection policy、decrypt_applied、board_bind_result、debug/RMA state、PowerBrake/PG/FAULT/reset event 应如何编码？
12. Manufacturing 的 Root injection mode 是 Seed/UDS 注入、直接 root key 注入，还是两者都作为产品选项？对应验收方式是什么？
13. OTP/eFuse 不可读字段如何做产测验收：只读 digest/status/counter，还是需要 eHSM 提供 attested provisioning report？
14. RMA 后 re-attestation/status、RMA re-acceptance 和返厂后 USER/PROD 恢复策略如何定义？
15. 是否需要单独建立下一轮 CR，将 `05_code_rules.md` 与 `06_traceability.md` 升级到新版 Source Status / Rule Status 规则？

## 10. 推荐下一步

建议把本 context pack 交给 ChatGPT / owner，让其先输出：

1. V2.4 的状态裁决。
2. 必须在详细方案冻结前关闭的问题清单。
3. 可以保留到实现设计或项目策略阶段的 `[ASSUMED]` / `[TBD]` 清单。
4. 下一条 Change Request 的标题、范围和状态。

Codex 后续只根据 owner 明确结论或 accepted CR 执行仓库同步。
