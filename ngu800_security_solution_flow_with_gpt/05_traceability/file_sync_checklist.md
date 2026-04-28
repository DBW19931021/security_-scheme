# 文件同步检查表

## 目的

本文件定义“变更类型 -> 必须同步文件”的映射。Codex 执行 CR 前必须读取本文件，并据此检查是否遗漏必改文件。

## 变更类型到必须同步文件映射

| 变更类型 | 必须同步文件 | 条件同步文件 | 检查重点 |
|---|---|---|---|
| Boot flow 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/04_impl_design/mailbox_if.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `03_detailed_design_master.md`; 导出版; `05_code_rules.md`; `04_change_impact.md` | BootROM/SEC1/SEC2/eHSM/Host 职责、verify/decrypt/measure/release 顺序一致 |
| FW 加密变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/03_detailed_design/02_key_cert.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/04_impl_design/mailbox_if.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `03_detailed_design_master.md`; 导出版; `05_code_rules.md`; `00_project/open_questions.md` | SEC1/SEC2 加密、CEK/KEK、signed region、encrypted payload、rollback、measurement 口径一致 |
| Key/Cert 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/02_key_cert.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/04_impl_design/manufacturing_provisioning.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `03_attestation.md`; `04_lifecycle_debug.md`; master/export; `05_code_rules.md` | Root/UDS/DRK、signer anchor、attestation cert、debug anchor、证书链策略 |
| Attestation 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/03_attestation.md`; `security_workflow/04_impl_design/spdm_report.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `01_boot.md`; `04_lifecycle_debug.md`; `05_board_security.md`; master/export; `05_code_rules.md` | measurement、report header、cert block、nonce、签名覆盖范围 |
| Debug/Lifecycle 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/04_lifecycle_debug.md`; `security_workflow/04_impl_design/mailbox_if.md`; `security_workflow/04_impl_design/manufacturing_provisioning.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `05_board_security.md`; `03_attestation.md`; master/export; `05_code_rules.md` | USER 默认关闭未授权 debug、scope、expire、audit、RMA 恢复 |
| Board/OOB 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/05_board_security.md`; `security_workflow/03_detailed_design/06_interface.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `04_lifecycle_debug.md`; `03_attestation.md`; master/export; `05_code_rules.md` | BMC/OOB 不进 RoT、JTAG MUX、DMA、I2C/I3C、power/reset 安全边界 |
| Interface/Mailbox 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/06_interface.md`; `security_workflow/04_impl_design/mailbox_if.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `01_boot.md`; `03_attestation.md`; `04_lifecycle_debug.md`; master/export; `05_code_rules.md` | caller、command ID、req/rsp、shared memory、error code、busy/timeout、地址白名单 |
| Manufacturing/RMA 变化 | `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/07_manufacturing_rma.md`; `security_workflow/04_impl_design/manufacturing_provisioning.md`; `security_workflow/06_traceability.md`; `00_project/decision_log.md`; `00_project/changelog.md` | `02_key_cert.md`; `04_lifecycle_debug.md`; `05_board_security.md`; master/export; `05_code_rules.md` | provisioning、USER freeze、测试 trust 清理、审计、RMA close |

## 通用同步要求

- 若变更影响两个以上文件、影响 baseline、或影响任一安全主路径，必须先建立 CR。
- 若 CR 产生新架构裁决，必须更新 `00_project/decision_log.md`。
- 每次仓库级修改必须更新 `00_project/changelog.md`。
- 若存在未关闭 `[TBD]` 或待补项，必须登记到 `00_project/open_questions.md`。
- Codex 不得自行改变安全架构口径，只能按 CR 落地。

