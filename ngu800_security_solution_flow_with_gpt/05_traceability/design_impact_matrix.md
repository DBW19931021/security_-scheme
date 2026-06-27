# 设计影响矩阵

## 目的

本文件用于记录每个 CR 对约束、baseline、详细设计、实现设计、测试的影响。每个正式 CR 都应引用或复制本矩阵。

## CR 影响矩阵模板

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes / No / TBD` |  |  |  |
| 约束 | `security_workflow/01_constraints.md` | `Yes / No / TBD` |  |  |  |
| Baseline | `security_workflow/02_baseline.md` | `Yes / No / TBD` |  |  |  |
| 详细设计 | boot | `Yes / No / TBD` |  | `03_detailed_design/01_boot.md` |  |
| 详细设计 | key/cert | `Yes / No / TBD` |  | `03_detailed_design/02_key_cert.md` |  |
| 详细设计 | attestation | `Yes / No / TBD` |  | `03_detailed_design/03_attestation.md` |  |
| 详细设计 | debug/lifecycle | `Yes / No / TBD` |  | `03_detailed_design/04_lifecycle_debug.md` |  |
| 详细设计 | board/OOB | `Yes / No / TBD` |  | `03_detailed_design/05_board_security.md` |  |
| 详细设计 | interface/mailbox | `Yes / No / TBD` |  | `03_detailed_design/06_interface.md` |  |
| 详细设计 | manufacturing/RMA | `Yes / No / TBD` |  | `03_detailed_design/07_manufacturing_rma.md` |  |
| 实现设计 | FW header / eFuse | `Yes / No / TBD` |  | `04_impl_design/efuse_key_fw_header_design.md` |  |
| 实现设计 | mailbox | `Yes / No / TBD` |  | `04_impl_design/mailbox_if.md` |  |
| 实现设计 | SPDM report | `Yes / No / TBD` |  | `04_impl_design/spdm_report.md` |  |
| 实现设计 | manufacturing provisioning | `Yes / No / TBD` |  | `04_impl_design/manufacturing_provisioning.md` |  |
| Code rules | 开发约束 | `Yes / No / TBD` |  | `security_workflow/05_code_rules.md` |  |
| Traceability | 追踪链路 | `Yes / No / TBD` |  | `security_workflow/06_traceability.md` |  |
| Test | QEMU / mock / review checklist | `Yes / No / TBD` |  | TBD |  |
| Master / Export | 总详设 / 导出版 | `Yes / No / TBD` |  | `03_detailed_design_master.md`; export file |  |
| Project records | 决策/变更/问题 | `Yes / No / TBD` |  | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md` |  |

## 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes / No` |
| 是否影响 baseline | `Yes / No / TBD` |
| 是否影响两个以上文件 | `Yes / No / TBD` |
| 是否影响安全主路径 | `Yes / No / TBD` |
| 是否需要 GPT 设计裁决 | `Yes / No / TBD` |
| 是否允许 Codex 直接修改正文 | `No` |

---

## CR-0001 影响矩阵：SEC1 加密、固件保护链、Master 同步

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `No` | 本 CR 基于既有 context pack 与已登记输入执行，不新增输入源 | `security_inputs/inputs_manifest.md` | `no-change` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 新增 `C-BOOT-04`，确认 SEC1 必须签名 + 加密、解密走 eHSM、Host 不下发 SEC1 | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 SEC1 protection policy、Host 不下发 SEC1；FW Encrypt Branch 后续已由 CR-0003 扩展为 SEC1 + SEC2 强制启用 | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | SEC1 校验规则改为强制解密，关闭“关键镜像是否加密”笼统开放项 | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | key/cert | `Yes` | FW Encrypt Branch 对 SEC1 强制，增加 FW_KEK / CEK / wrapped CEK / key slot lifecycle 口径；CR-0003 已扩展为 SEC1 + SEC2 强制 | `03_detailed_design/02_key_cert.md` | `applied` |
| 详细设计 | attestation | `Yes` | SEC1 measurement 反映 verify + decrypt 成功后的受控状态，并预留 image protection policy 表达 | `03_detailed_design/03_attestation.md` | `applied` |
| 详细设计 | debug/lifecycle | `Yes` | USER/PROD 下 SEC1/SEC2 解密 key / FW_KEK 受 lifecycle gating，RMA 不得绕过 SEC1/SEC2 加密策略 | `03_detailed_design/04_lifecycle_debug.md` | `applied` |
| 详细设计 | board/OOB | `Yes` | 源章节作为 master 第 7 章并入；源文件检查后保持原口径 | `03_detailed_design/05_board_security.md` | `inspected / no-change` |
| 详细设计 | interface/mailbox | `Yes` | `VERIFY_SEC1` 强制 verify + decrypt，`VERIFY_IMAGE` 按 image_type/policy 解密，补充字段和错误模型 | `03_detailed_design/06_interface.md` | `applied` |
| 详细设计 | manufacturing/RMA | `Yes` | 加入 FW_KEK / image protect key 灌装、USER 前锁定和 RMA 不绕过 SEC1 加密策略 | `03_detailed_design/07_manufacturing_rma.md` | `applied` |
| 实现设计 | FW header / eFuse | `Yes` | Header 增加 enc_mode / enc_flags / wrapped CEK / IV / AAD / ciphertext 字段，新增 image type policy | `04_impl_design/efuse_key_fw_header_design.md` | `applied` |
| 实现设计 | mailbox | `Yes` | `VERIFY_SEC1` profile、decrypt_required、key_slot、wrapped_cek_present、policy_mismatch 等实现级字段和错误码 | `04_impl_design/mailbox_if.md` | `applied` |
| 实现设计 | SPDM report | `Yes` | 增加 image_confidentiality_policy、decrypt_applied / rollback_checked 等表达方向 | `04_impl_design/spdm_report.md` | `applied` |
| 实现设计 | manufacturing provisioning | `Yes` | 加入 FW_KEK / image protect key provisioning 和 USER freeze 锁定要求 | `04_impl_design/manufacturing_provisioning.md` | `applied` |
| Code rules | 开发约束 | `No` | 本 CR 未要求修改 code rules | `security_workflow/05_code_rules.md` | `no-change` |
| Traceability | 追踪链路 | `Yes` | 新增 `T-BOOT-004 / SEC1_ENCRYPT_REQUIRED` 需求追踪和测试建议 | `security_workflow/06_traceability.md` | `applied` |
| Test | QEMU / mock / review checklist | `TBD` | 新增测试建议但未创建测试文件 | TBD | `open` |
| Master / Export | 总详设 / 导出版 | `Yes` | master 重排为 11 章，Key/Cert 后置，板级章节并入；后续汇总版统一生成 `10_full_design.md` | `03_detailed_design_master.md`; `10_full_design.md` | `applied` |
| Project records | 决策/变更/问题 | `Yes` | 更新 decision_log、changelog、open_questions，并将 CR 状态推进到 applied | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md` | `applied` |

## CR-0001 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Yes` |
| 是否需要 GPT 设计裁决 | `Yes`，已由 CR-0001 给出裁决 |
| 是否允许 Codex 直接修改正文 | `Yes, after CR accepted/execution authorized; Codex only applied CR decisions` |

---

## CR-0002 影响矩阵：输入源引用显示名可读性增强

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes` | 增加引用显示名约定，不新增输入源 | `security_inputs/inputs_manifest.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 将裸 `SRC-005` 显示增强为 `SRC-005 管理子系统方案` | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 将裸 `SRC-005` 显示增强为 `SRC-005 管理子系统方案` | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | debug/lifecycle | `Yes` | 将裸 `SRC-005` 显示增强为 `SRC-005 管理子系统方案` | `03_detailed_design/04_lifecycle_debug.md` | `applied` |
| 详细设计 | board/OOB | `Yes` | 将裸 `SRC-005` 显示增强为 `SRC-005 管理子系统方案` | `03_detailed_design/05_board_security.md` | `applied` |
| 详细设计 | interface/mailbox | `Yes` | 将裸 `SRC-005` 显示增强为 `SRC-005 管理子系统方案` | `03_detailed_design/06_interface.md` | `applied` |
| 详细设计 | master / full design | `Yes` | 将总详设中的裸 `SRC-005` 与残留裸 `SRC-003` 改为可读显示名 | `03_detailed_design/03_detailed_design_master.md`; `03_detailed_design/10_full_design.md` | `applied` |
| 实现设计 | FW header / eFuse | `No` | 不涉及实现字段或接口结构 | `04_impl_design/efuse_key_fw_header_design.md` | `no-change` |
| 实现设计 | mailbox | `No` | 不涉及 mailbox 字段或行为 | `04_impl_design/mailbox_if.md` | `no-change` |
| Code rules | 开发约束 | `Yes` | 文件状态行中的输入源显示名增强 | `security_workflow/05_code_rules.md` | `applied` |
| Traceability | 追踪链路 | `Yes` | 追踪表中的 source 显示名增强 | `security_workflow/06_traceability.md` | `applied` |
| Change impact | 变更影响记录 | `Yes` | 变更影响记录中的 source 显示名增强 | `security_workflow/04_change_impact.md` | `applied` |
| Test | QEMU / mock / review checklist | `No` | 仅文档引用显示方式变化，不需要新增测试 | TBD | `no-change` |
| Project records | 决策/变更/问题 | `Yes` | 记录引用显示名裁决和本次仓库级变更 | `00_project/decision_log.md`; `00_project/changelog.md`; `change_requests/CR-0002-readable-source-references.md` | `applied` |

## CR-0002 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes`，仅显示引用名，不改变 baseline 语义 |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `No`，不改变安全设计语义 |
| 是否需要 GPT 设计裁决 | `No`，仅为引用可读性增强 |
| 是否允许 Codex 直接修改正文 | `Yes, after CR accepted; Codex only changed source reference display names` |

---

## CR-0003 影响矩阵：Runtime Image 策略、Board Binding、调试/DMA/OOB 边界与 V2.4 冻结准备

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `No` | 基于既有 manifest、context pack 和 CR-0003 裁决执行，不新增输入源 | `security_inputs/inputs_manifest.md` | `no-change` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 增加 SEC2 强制 sign+encrypt、runtime 默认策略、attestation report 状态字段、DMA 默认拒绝、USER freeze/RMA 边界 | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 SEC2 protection policy、runtime image policy、board binding 阶段策略、OOB proxy 和 DMA/JTAG 基线 | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | SEC2 改为 mandatory sign+encrypt；PM/RAS/Codec 默认 sign+encrypt；signature-only 白名单和 recovery policy 保留 TBD | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | key/cert | `Yes` | FW Encrypt Branch 至少覆盖 SEC1 + SEC2；runtime signature-only 白名单保留产品策略 TBD | `03_detailed_design/02_key_cert.md` | `applied` |
| 详细设计 | attestation | `Yes` | report 必须覆盖 lifecycle/debug/secure_boot/rollback；image protection/decrypt/board binding 字段按 ASSUMED/TBD 标注 | `03_detailed_design/03_attestation.md` | `applied` |
| 详细设计 | debug/lifecycle | `Yes` | USER/PROD JTAG 默认关闭；JTAG open 必须 lifecycle + auth + scope + timeout + audit；SEC2 decrypt 不可被 debug 关闭 | `03_detailed_design/04_lifecycle_debug.md` | `applied` |
| 详细设计 | board/OOB | `Yes` | board binding 默认进入 attestation、不阻断 SEC1；DMA 默认拒绝；OOB/BMC proxy 只作为 transport proxy | `03_detailed_design/05_board_security.md` | `applied` |
| 详细设计 | interface/mailbox | `Yes` | OOB/BMC 信任级别、JTAG/DMA、provisioning proxy、冻结敏感项更新 | `03_detailed_design/06_interface.md` | `applied` |
| 详细设计 | manufacturing/RMA | `Yes` | USER freeze 覆盖 SEC1/SEC2 decrypt key/FW_KEK；RMA 不得长期保留 SEC1/SEC2 decrypt bypass | `03_detailed_design/07_manufacturing_rma.md` | `applied` |
| 实现设计 | FW header / eFuse | `Yes` | `IMAGE_TYPE_SEC2` 改为 `ENCRYPT_REQUIRED`；runtime policy 和 USER freeze key lock 更新 | `04_impl_design/efuse_key_fw_header_design.md` | `applied` |
| 实现设计 | mailbox | `Yes` | `VERIFY_IMAGE(SEC2)` mandatory decrypt profile；DMA 默认拒绝；signature-only policy_state 表达 | `04_impl_design/mailbox_if.md` | `applied` |
| 实现设计 | SPDM report | `Yes` | report header/measurement/lifecycle block 增加 rollback、board_bind_result、image_policy_state、event_log_policy 等方向 | `04_impl_design/spdm_report.md` | `applied` |
| 实现设计 | manufacturing provisioning | `Yes` | SEC2 解密/unwrap 验证、FW_ENCRYPT_EN 覆盖 SEC1+SEC2、RMA 约束同步 | `04_impl_design/manufacturing_provisioning.md` | `applied` |
| Code rules | 开发约束 | `Yes` | 新增/强化 SEC2 加密、runtime 白名单、JTAG timeout/audit、DMA 默认拒绝、OOB proxy、attestation、manufacturing 规则 | `security_workflow/05_code_rules.md` | `applied` |
| Traceability | 追踪链路 | `Yes` | 新增 T-BOOT-005、T-BOARD-005、T-BOARD-006，更新 T-BOARD-003/004/T-ATT-001 和测试建议 | `security_workflow/06_traceability.md` | `applied` |
| Test | QEMU / mock / review checklist | `TBD` | 新增测试建议但未创建测试文件 | TBD | `open` |
| Master / Export | 总详设 / 导出版 | `Yes` | `10_full_design.md` 同步 CR-0003 状态和章节口径；V2.4 保持 pending review | `03_detailed_design/10_full_design.md` | `applied` |
| Project records | 决策/变更/问题 | `Yes` | 更新 decision_log、changelog、open_questions，并将 CR-0003 状态推进到 applied | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `change_requests/CR-0003-runtime-image-policy-board-binding-attestation-mfg-freeze.md` | `applied` |

## CR-0003 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Yes` |
| 是否需要 GPT / owner 设计裁决 | `Yes`，已由 CR-0003 给出阶段性裁决；剩余项保留 `[ASSUMED]` / `[TBD]` |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied CR-0003 decisions` |

---

## CR-0004 影响矩阵：eHSM Native Header、OTP/Key/Counter Source-Conformance

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes` | 为 eHSM Firmware TRM / Bootloader TRM 增加单文件 source entry，登记 CF-004~CF-007 | `security_inputs/inputs_manifest.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 新增 eHSM native header、verify+decrypt deploy path、OTP/key/counter source-conformance 约束 | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 eHSM image container、OTP/key/counter mapping、algorithm authority baseline | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | 将镜像格式改为 eHSM native header + NGU manifest；SEC1/SEC2 使用 verify+decrypt output path | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | key/cert | `Yes` | 将 key hierarchy 改为 logical alias + eHSM key ID mapping；wrapped CEK 保持 TBD | `03_detailed_design/02_key_cert.md` | `applied` |
| 详细设计 | attestation | `No` | 本轮不改变 report 字段 ABI，仅后续可能引用 manifest policy state | `03_detailed_design/03_attestation.md` | `inspect-later` |
| 详细设计 | debug/lifecycle | `No` | 不改变 CR-0003 调试/lifecycle 策略；exact control bit 仍 TBD | `03_detailed_design/04_lifecycle_debug.md` | `no-change` |
| 详细设计 | board/OOB | `No` | 不改变 board/OOB 信任边界 | `03_detailed_design/05_board_security.md` | `no-change` |
| 详细设计 | interface/mailbox | `Yes` | 将 `VERIFY_SEC1 / VERIFY_IMAGE` 表述为 NGU wrapper/profile 到 eHSM command 的映射 | `03_detailed_design/06_interface.md` | `applied` |
| 详细设计 | manufacturing/RMA | `No` | 章节源文件本轮未直接修改；实现级 manufacturing 已对齐 eHSM command mapping | `03_detailed_design/07_manufacturing_rma.md` | `deferred-sync` |
| 实现设计 | FW header / eFuse | `Yes` | 重构为 eHSM native header、NGU protected manifest、OTP/key/counter logical alias mapping | `04_impl_design/efuse_key_fw_header_design.md` | `applied` |
| 实现设计 | source matrix | `Yes` | 新增字段级 source-conformance gate | `04_impl_design/ehsm_source_conformance_matrix.md` | `applied` |
| 实现设计 | mailbox | `Yes` | 对齐 `bl_verify_image / soc_verify / fw_upgrade`，移除 wrapper 中发明 physical key slot/wrapped CEK 的倾向 | `04_impl_design/mailbox_if.md` | `applied` |
| 实现设计 | SPDM report | `No` | 本轮未修改；若后续 report 引用 manifest ABI，再开后续 CR | `04_impl_design/spdm_report.md` | `no-change` |
| 实现设计 | manufacturing provisioning | `Yes` | provisioning target 改为 eHSM key ID / control field / logical alias reference，新增 eHSM command mapping | `04_impl_design/manufacturing_provisioning.md` | `applied` |
| Code rules | 开发约束 | `Yes` | 新增 eHSM native header、source-conformance、NVM only verify 禁止、wrapped CEK TBD 规则 | `security_workflow/05_code_rules.md` | `applied` |
| Traceability | 追踪链路 | `Yes` | 新增 T-EHSM-001~003 和 BLOCKED 候选项 | `security_workflow/06_traceability.md` | `applied` |
| Test | QEMU / mock / review checklist | `TBD` | 新增测试建议但未创建测试文件 | TBD | `open` |
| Master / Export | 总详设 / 导出版 | `Yes` | `10_full_design.md` 需要同步 eHSM native header / manifest / source-conformance 口径 | `03_detailed_design/10_full_design.md` | `applied` |
| Project records | 决策/变更/问题 | `Yes` | 更新 CR、decision log、changelog、open questions | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `change_requests/CR-0004-*.md` | `applied` |

## CR-0004 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Yes` |
| 是否需要 GPT / owner 设计裁决 | `Yes`，用户已于 2026-05-08 授权按修订后 CR 执行；CR 保留项仍为 `[TBD]` |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied CR-0004 accepted scope and preserved TBD items` |

---

## CR-0005 影响矩阵：10_full_design 作为唯一代码落地详设入口

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes` | 登记用户关于 `10_full_design.md` 作为完整详设和代码落地主入口的裁决 | `security_inputs/inputs_manifest.md` | `applied` |
| CR | Change Request | `Yes` | 新增 CR-0005 记录文档事实源和流程裁决 | `change_requests/CR-0005-single-full-design-code-landing-spec.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `No` | 本 CR 不改变安全架构约束 | `security_workflow/01_constraints.md` | `no-change` |
| Baseline | `security_workflow/02_baseline.md` | `No` | 本 CR 不改变安全 baseline | `security_workflow/02_baseline.md` | `no-change` |
| 详细设计 | full design | `Yes` | 将 `10_full_design.md` 升级为完整详设和代码落地主入口，新增第 10 章全量嵌入实现级分片正文 | `03_detailed_design/10_full_design.md` | `applied` |
| 实现设计 | 04_impl_design 分片 | `Yes` | 所有分片增加 CR-0005 source-of-truth notice，标记为 editing shard / extracted implementation shard | `04_impl_design/*.md` | `applied` |
| Workflow docs | README / Skill | `Yes` | 更新推荐工作流、mandatory pipeline、D2 implementation design 和 final check 规则 | `README_使用说明.md`; `codex/skills/ngu800-security/SKILL.md` | `applied` |
| Prompts | Codex/GPT prompts | `Yes` | 更新实现级生成、CR apply、review、traceability、final check prompt，要求同步 `10_full_design.md` | `codex/skills/ngu800-security/prompts/*`; `prompts/*` | `applied` |
| Templates | Design templates | `Yes` | 更新 full/impl/template source-of-truth 规则，避免后续生成只停留在分片 | `codex/skills/ngu800-security/templates/*`; `templates/*` | `applied` |
| Code rules | 开发约束 | `Yes` | 新增 `R-DOC-*` 文档事实源 / 代码落地规则 | `security_workflow/05_code_rules.md` | `applied` |
| Traceability | 追踪链路 | `Yes` | 新增 `T-DOC-001`，将 `10_full_design.md` 与 `04_impl_design` 同步关系纳入追踪 | `security_workflow/06_traceability.md` | `applied` |
| Change impact | 增量影响记录 | `Yes` | 新增 CR-0005 影响记录和一致性检查 | `security_workflow/04_change_impact.md` | `applied` |
| Project records | 决策/变更 | `Yes` | 记录 DEC-0015 和 CHG-0006 | `00_project/decision_log.md`; `00_project/changelog.md` | `applied` |
| Open questions | 开放问题 | `No` | 本 CR 不新增安全字段或架构 TBD | `00_project/open_questions.md` | `no-change` |

## CR-0005 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `No`，不改变安全 baseline |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Indirect`，不改变安全策略，但影响代码落地事实源 |
| 是否需要 GPT / owner 设计裁决 | `Yes`，用户已于 2026-05-08 明确裁决并授权执行 |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied CR-0005 documentation source-of-truth scope` |

---

## CR-0006 影响矩阵：固件包格式、制作流程与设备侧 verify/decrypt 流程

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes` | 登记用户关于固件格式、制作流程、设备侧验签解密流程的补充要求，并引用 `SRC-001` 第 6/7 章作为流程参考 | `security_inputs/inputs_manifest.md` | `applied` |
| CR | Change Request | `Yes` | 新增 CR-0006 记录本轮设计边界和禁止恢复旧 physical header 的要求 | `change_requests/CR-0006-firmware-package-build-verify-flow.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 新增 `C-BOOT-08`，要求平台侧制作工具和设备侧 verify/decrypt 路径共享 eHSM-native + protected manifest 契约 | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 Firmware Package Build/Verify Contract baseline | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | 补充固件包布局图、平台侧制作流程图、设备侧 verify/decrypt 时序图和流程说明 | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | full design | `Yes` | 同步第 3 章和第 10 章，使 `10_full_design.md` 仍是完整代码落地主入口；2026-05-09 精化 SEC1 最终制品组成、Code region 签名/认证覆盖和制作/设备侧对应关系 | `03_detailed_design/10_full_design.md` | `applied / refined-2026-05-09` |
| 实现设计 | FW header / eFuse | `Yes` | 补充实现级包格式、工具链制作、设备侧 verify/decrypt、工具交付物和 open issue | `04_impl_design/efuse_key_fw_header_design.md` | `applied` |
| 实现设计 | mailbox | `No` | 本轮不改变 mailbox command ID / req/rsp ABI，仅复用既有 `VERIFY_SEC1 / VERIFY_IMAGE` profile | `04_impl_design/mailbox_if.md` | `no-change` |
| 实现设计 | source matrix | `No` | 本轮不新增 eHSM physical field；仍沿用 CR-0004 source-conformance matrix | `04_impl_design/ehsm_source_conformance_matrix.md` | `no-change` |
| Code rules | 开发约束 | `Yes` | 新增 image packager / manifest trust / old custom header 禁止规则；2026-05-09 补充 SEC1 Code region 认证覆盖、未认证 header 字段使用限制和 tamper vector 规则 | `security_workflow/05_code_rules.md` | `applied / refined-2026-05-09` |
| Traceability | 追踪链路 | `Yes` | 新增 `T-FW-PKG-001` 和相关测试建议；2026-05-09 补充 SEC1 auth coverage 和 tamper vector 测试追踪 | `security_workflow/06_traceability.md` | `applied / refined-2026-05-09` |
| Change impact | 增量影响记录 | `Yes` | 新增 CR-0006 影响记录和一致性检查 | `security_workflow/04_change_impact.md` | `applied` |
| Project records | 决策/变更/问题 | `Yes` | 记录 DEC-0016、CHG-0007、OQ-0016；2026-05-09 追加 DEC-0017、CHG-0008 | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md` | `applied / refined-2026-05-09` |
| Templates | 生成模板 | `Yes` | 更新 boot image format 模板，后续生成必须包含 build/verify flow 且不得回退旧 header | `templates/boot_image_format_template.md`; `codex/skills/ngu800-security/templates/boot_image_format_template.md` | `applied` |

## CR-0006 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Yes`，影响 firmware package build 和 secure boot verify/decrypt path |
| 是否需要 GPT / owner 设计裁决 | `Yes`，用户已于 2026-05-08 明确裁决并授权开始 |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied CR-0006 flow/detail scope and preserved eHSM TBD items` |

---

## CR-0007 影响矩阵：生效约束 ID 可读性与跳转链接增强

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `No` | 不新增输入资料；基于用户可读性反馈执行 | `security_inputs/inputs_manifest.md` | `no-change` |
| CR | Change Request | `Yes` | 新增 CR-0007，记录约束链接化和摘要增强范围 | `change_requests/CR-0007-readable-constraint-links.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 为每个 `C-xxx` 约束标题增加稳定 HTML anchor，不改约束正文 | `security_workflow/01_constraints.md` | `applied` |
| 详细设计 | full design | `Yes` | 将各章“生效约束 ID”改为可点击链接 + 一句话摘要，并增加约束链接说明 | `security_workflow/03_detailed_design/10_full_design.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `No` | 不改变安全 baseline | `security_workflow/02_baseline.md` | `no-change` |
| Code rules | 开发约束 | `No` | 不改变工程开发约束 | `security_workflow/05_code_rules.md` | `no-change` |
| Traceability | 追踪链路 | `No` | 不改变需求到代码/测试追踪关系 | `security_workflow/06_traceability.md` | `no-change` |
| Project records | 变更记录 | `Yes` | 记录 CHG-0009；不新增安全裁决，因此不更新 decision log | `00_project/changelog.md` | `applied` |

## CR-0007 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `No` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `No`，仅增强文档可读性和跳转 |
| 是否需要 GPT / owner 设计裁决 | `No`，用户已明确授权可读性改造，不改变安全结论 |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied readability/linking scope` |

---

## CR-0014 影响矩阵：current_plan 2.0 当前方案源同步

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `Yes` | 新增 `SRC-008 当前收敛安全软件方案 2.0`，将 `SRC-001` 降级为历史流程参考 | `security_inputs/inputs_manifest.md` | `applied` |
| CR | Change Request | `Yes` | 新增 CR-0014，记录 source precedence 与旧口径替换范围 | `change_requests/CR-0014-current-plan-v2-source-of-truth-sync.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `Yes` | 新增 `C-SRC-01`，更新 `C-BOOT-08` / `C-UPDATE-02` 来源 | `security_workflow/01_constraints.md` | `applied` |
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 current plan source baseline，不改变 eHSM RoT 等安全主裁决 | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | 固件包流程来源改为 `SRC-008`，删除 SEC2/runtime A/B recovery 建议残留 | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | full design | `Yes` | 版本升到 V2.6，登记 `SRC-008` 为当前方案源，并同步固件包实现级来源 | `03_detailed_design/10_full_design.md` | `applied` |
| 实现设计 | FW header / eFuse | `Yes` | 固件包和设备侧 verify/decrypt 流程来源改为 `SRC-008` | `04_impl_design/efuse_key_fw_header_design.md` | `applied` |
| Code rules | 开发约束 | `Yes` | 替换旧 CR-0008 FMC A/B / key rotation 绑定规则为单 FMC + OOB 重刷与 OOB 不放行规则 | `security_workflow/05_code_rules.md` | `applied` |
| Traceability | 追踪链路 | `Yes` | 新增 `T-SRC-001`，更新 `T-FW-PKG-001` 与 `T-UPD-002` 来源 | `security_workflow/06_traceability.md` | `applied` |
| Change impact | 增量影响记录 | `Yes` | 新增 CR-0014 影响记录和一致性检查 | `security_workflow/04_change_impact.md` | `applied` |
| Workflow docs | README | `Yes` | 工作流说明增加当前 source precedence | `README_使用说明.md` | `applied` |
| Project docs | 导出版方案 | `Yes` | 页首/修订说明同步 2.0 PDF 当前为准 | selected `docs/*.md` | `applied` |
| Project records | 决策/变更 | `Yes` | 记录 DEC-0019、CHG-0011 | `00_project/decision_log.md`; `00_project/changelog.md` | `applied` |
| Open questions | 未冻结项 | `No` | 仅确认 source precedence，不关闭 manifest ABI、exact key ID、OOB/QSPI ABI 等开放问题 | `00_project/open_questions.md` | `no-change` |

## CR-0014 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `Yes`，仅增加 source precedence baseline |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Yes`，涉及 boot / update / OOB recovery / FW package source |
| 是否需要 GPT / owner 设计裁决 | `Yes`，用户已于 2026-06-03 明确新版 PDF 为当前收敛版 |
| 是否允许 Codex 直接修改正文 | `Yes, after user execution authorization; Codex only applied source-of-truth and stale-wording sync` |

---

## CR-0015 影响矩阵：安全组件原子 Rename

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `No` | 不新增输入资料 | `security_inputs/inputs_manifest.md` | `no-change` |
| CR | Change Request | `Yes` | 记录用户确认的原子 rename 和禁止兼容层范围 | `change_requests/CR-0015-security-module-atomic-rename.md` | `applied` |
| 约束 | `security_workflow/01_constraints.md` | `No` | 不改变安全约束 | `security_workflow/01_constraints.md` | `no-change` |
| Baseline | `security_workflow/02_baseline.md` | `No` | 不改变 Root of Trust 或安全 baseline | `security_workflow/02_baseline.md` | `no-change` |
| 详细设计 | code/path references | `Yes` | 只同步旧组件路径和代码符号引用 | relevant detailed design files | `applied` |
| 实现设计 | component/API references | `Yes` | 同步文件、头文件、函数、类型和宏名称 | relevant implementation design files | `applied` |
| Code rules | naming rules | `Yes` | supersede 旧 `ngu_*` ABI 保留规则 | `security_workflow/05_code_rules.md`; component development principles | `applied` |
| Traceability | code/test mapping | `Yes` | 同步新路径和测试入口 | `security_workflow/06_traceability.md` | `applied` |
| Test | host/packager/target build | `Yes` | 增加负向命名扫描并运行完整回归 | component tests and build targets | `applied` |
| Project records | decision/changelog | `Yes` | 记录 DEC-0020 和 CHG-0012 | `00_project/decision_log.md`; `00_project/changelog.md` | `applied` |
| Open questions | security design TBD | `No` | 不新增或关闭安全开放问题 | `00_project/open_questions.md` | `no-change` |

## CR-0015 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `No` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `No semantic change`，仅破坏性源码/API 命名迁移 |
| 是否需要 owner 设计裁决 | `Yes`，用户已于 2026-06-10 选择方案 A |
| 是否允许 Codex 直接修改正文 | `Yes, after written spec review; only code/path references may change` |

---

## CR-0017 影响矩阵：SPDM Responder Production Service

| 层级 | 对象 | 是否影响 | 影响说明 | 必须同步文件 | 状态 |
|---|---|---|---|---|---|
| 输入 | inputs manifest / 新资料 | `No` | 不新增安全输入资料 | `security_inputs/inputs_manifest.md` | `no-change` |
| CR | Change Request | `Yes` | 记录 responder runtime 从 QEMU test 抽取为 production service | `change_requests/CR-0017-spdm-responder-production-service.md` | `applied` |
| 约束 / Baseline | RoT、安全语义 | `No` | 不改变 RoT、Host boundary、measurement、certificate 或 signature 语义 | `01_constraints.md`; `02_baseline.md` | `no-change` |
| 实现设计 | MCTP/SPDM runtime ownership | `Yes` | responder endpoint、adapter、static task 归 production service；requester/test fixtures 留在 QEMU tests | component OpenSpec, Superpowers design/plan and code guide | `applied` |
| 代码 | production responder service | `Yes` | 新增 service API/实现并进入 GSP component build | `components/security/include/security/spdm`; `components/security/src/spdm`; `components/security/sub.mk` | `applied` |
| 测试 | host/QEMU contracts | `Yes` | 两轮 TDD，验证 service lifecycle、GET_VERSION、双 task 和 requester self-delete | component responder/runtime tests | `applied` |
| Traceability | evidence / code map | `Yes` | 新增 OpenSpec requirement、tasks、verification evidence，并更新旧 QEMU code map | component OpenSpec evidence | `applied` |
| Project records | decision/changelog | `Yes` | 记录 DEC-0022、CHG-0014，并标注 CR-0016 部分被替代 | `00_project/decision_log.md`; `00_project/changelog.md`; `CR-0016` | `applied` |
| Open questions | QEMU platform blocker | `No` | 不新增问题；真实 QEMU 串口 PASS 继续受既有 address-map blocker 约束 | component known issues | `no-change` |

## CR-0017 影响结论

| 项目 | 结论 |
|---|---|
| 是否必须建立 CR | `Yes` |
| 是否影响 baseline | `No` |
| 是否影响两个以上文件 | `Yes` |
| 是否影响安全主路径 | `Implementation ownership only`，不改变 attestation 安全语义 |
| 是否需要 owner 设计裁决 | `Yes`，用户已确认 production responder service 划分并授权开始调整 |
| 是否允许 Codex 直接修改正文 | `Yes, implementation-only; only runtime ownership, code, tests and traceability may change` |
