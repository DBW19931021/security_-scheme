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
| Baseline | `security_workflow/02_baseline.md` | `Yes` | 增加 SEC1 protection policy、Host 不下发 SEC1、FW Encrypt Branch 至少对 SEC1 强制启用 | `security_workflow/02_baseline.md` | `applied` |
| 详细设计 | boot | `Yes` | SEC1 校验规则改为强制解密，关闭“关键镜像是否加密”笼统开放项 | `03_detailed_design/01_boot.md` | `applied` |
| 详细设计 | key/cert | `Yes` | FW Encrypt Branch 对 SEC1 强制，增加 FW_KEK / CEK / wrapped CEK / key slot lifecycle 口径 | `03_detailed_design/02_key_cert.md` | `applied` |
| 详细设计 | attestation | `Yes` | SEC1 measurement 反映 verify + decrypt 成功后的受控状态，并预留 image protection policy 表达 | `03_detailed_design/03_attestation.md` | `applied` |
| 详细设计 | debug/lifecycle | `Yes` | USER/PROD 下 SEC1 解密 key / FW_KEK 受 lifecycle gating，RMA 不得绕过 SEC1 加密策略 | `03_detailed_design/04_lifecycle_debug.md` | `applied` |
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
| Master / Export | 总详设 / 导出版 | `Yes` | master 重排为 11 章，Key/Cert 后置，板级章节并入，生成 `03_detailed_design_master_v2.4.md` | `03_detailed_design_master.md`; `03_detailed_design_master_v2.4.md` | `applied` |
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
