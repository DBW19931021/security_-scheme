# 仓库级变更日志 Changelog

## 目的

本文件用于记录 NGU800 安全方案仓库的流程、模板、追踪文件和设计内容变更历史。每次按 CR 修改仓库后，都应追加一条记录。

## 记录字段

| Change ID | Date | CR ID | Summary | Files Changed | Status |
|---|---|---|---|---|---|
| CHG-0001 | 2026-04-27 | N/A | 初始化设计变更管理流程骨架 | `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `change_requests/CR_template.md`; `change_requests/README.md`; `.context/design_context_pack_template.md`; `05_traceability/file_sync_checklist.md`; `05_traceability/design_impact_matrix.md`; `prompts/01_generate_context_pack.md`; `prompts/02_generate_change_request_with_gpt.md`; `prompts/03_apply_change_request_with_codex.md`; `prompts/04_review_codex_diff_with_gpt.md`; `tools/check_cr_sync.py` | `[applied]` |
| CHG-0002 | 2026-04-27 | `CR-0001-sec1-encryption-fw-protection-master-sync` | Applied commit: working tree pending commit. 按 CR-0001 收敛 SEC1 强制签名 + 加密、FW protection、源章节/实现级同步、master 重排、导出版生成和板级安全并入 | `change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `05_traceability/design_impact_matrix.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/03_detailed_design/02_key_cert.md`; `security_workflow/03_detailed_design/03_attestation.md`; `security_workflow/03_detailed_design/04_lifecycle_debug.md`; `security_workflow/03_detailed_design/06_interface.md`; `security_workflow/03_detailed_design/07_manufacturing_rma.md`; `security_workflow/03_detailed_design/03_detailed_design_master.md`; `security_workflow/03_detailed_design/03_detailed_design_master_v2.4.md`; `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/04_impl_design/mailbox_if.md`; `security_workflow/04_impl_design/manufacturing_provisioning.md`; `security_workflow/04_impl_design/spdm_report.md`; `security_workflow/06_traceability.md` | `[applied]` |

## 状态定义

| Status | 含义 |
|---|---|
| `[draft]` | 变更草案已提出 |
| `[accepted]` | CR 已接受，允许 Codex 执行 |
| `[applied]` | Codex 已完成仓库修改 |
| `[reviewed]` | GPT/人工已完成 diff 复核 |
| `[closed]` | 变更闭环完成 |
| `[superseded]` | 被后续 CR 替代 |
