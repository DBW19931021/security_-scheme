# CR-0014 current_plan 2.0 当前方案源同步

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0014` |
| Title | `芯片安全软件方案_2.0.pdf` 作为当前收敛方案源同步 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-06-03 |
| Source / Context Pack | 用户指示：`security_inputs/current_plan/芯片安全软件方案_2.0.pdf` 是最新收敛版，无特殊说明时以该版为准 |
| Related Decision ID | `DEC-0019` |

## 2. 背景

`security_inputs/current_plan/` 新增 `芯片安全软件方案_2.0.pdf`。该 PDF 已将当前安全软件方案重新收敛到以下主线：

- `FMC(SEC1)` / `GSP(SEC2)` 命名和启动链。
- 单 FMC 固定分区，不再采用 SoC Flash 内部 FMC A/B 备份和 BootROM fallback slot。
- OOB MCU / 板级安全 MCU 纳入板级安全边界，通过 QSPI 受控重刷 NOR Flash 固定 FMC 分区。
- FMC、GSP、关键 runtime 固件采用 eHSM native package，NGU manifest 位于被 eHSM 认证/解密保护的 Code region。
- measurement table、attestation report、静态设备证明证书链、制造灌装、USER freeze、密钥轮换、RMA 和审计流程按 2.0 版解释。

本 CR 的目标不是发明新安全架构，而是把用户已明确的当前方案源优先级同步到仓库，并修复仍残留的旧 A/B / 旧 current_plan 引用。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `a293736` |
| inputs_manifest 摘要 | `SRC-001` 仍登记为当前安全方案基线；新版 PDF 尚未登记 |
| constraints 摘要 | 未显式写入 `SRC-008` source precedence；`C-BOOT-08` 仍以 `SRC-001` 第 6/7 章作为流程来源 |
| baseline 摘要 | 已有单 FMC + OOB 重刷口径，但未声明 2.0 PDF 为当前方案源 |
| 相关详设章节 | `01_boot.md` 仍残留 SEC2/runtime A/B 槽位建议；`10_full_design.md` 已大体吸收 FMC/GSP、OOB 和 key rotation 口径 |
| 相关实现级文档 | `efuse_key_fw_header_design.md` 仍引用 `SRC-001` 第 6/7 章作为固件包流程来源 |
| 已知冲突 | 旧 `SRC-001` 和 CR-0008 的 FMC A/B/fallback 口径已被 CR-0013 与 `SRC-008` supersede |
| 待冻结开放项 | manifest ABI、exact key ID、exact OTP/control bit、OOB/QSPI ABI、工具 CLI/golden vector 仍保持 open |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 当前方案源 | `SRC-008 当前收敛安全软件方案 2.0` 作为当前安全软件方案基线；旧 `SRC-001` 降级为历史流程参考 | 用户明确新版 PDF 为最新收敛版；需要避免旧 current_plan 与工作流文档形成并列事实源 | `[CONFIRMED]` |
| 源优先级边界 | `SRC-008` 不覆盖 accepted CR、decision_log、官方 eHSM/TRM、后续用户特殊说明或源内明确例外 | 保持 eHSM physical facts 和已接受裁决的优先级 | `[CONFIRMED]` |
| A/B 残留口径 | 删除或替换仍把 FMC/GSP/runtime 恢复绑定到 A/B slot 的 code rules / boot recovery 文字 | 2.0 版和 CR-0013 已收敛为单 FMC + OOB 重刷 | `[CONFIRMED]` |
| 未冻结实现项 | `SRC-008` 未给出 bit-level ABI / exact mapping 的内容继续保持开放项或待 owner 冻结状态 | 防止把方案级 PDF 误用为字段级 RTL/API 冻结材料 | `[CONFIRMED]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_inputs/inputs_manifest.md` | Yes | 新增 `SRC-008`，降级 `SRC-001`，登记 conflict/change intake |
| `security_workflow/01_constraints.md` | Yes | 新增 `C-SRC-01`，更新 `C-BOOT-08` / `C-UPDATE-02` 来源 |
| `security_workflow/02_baseline.md` | Yes | 增加 current plan source baseline |
| `security_workflow/03_detailed_design/01_boot.md` | Yes | 更新固件包来源，删除 SEC2/runtime A/B 建议残留 |
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 主详设登记 `SRC-008` 当前方案源，更新相关来源引用 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | Yes | 固件包实现级来源改为 `SRC-008` |
| `security_workflow/05_code_rules.md` | Yes | 替换旧 CR-0008 FMC A/B / key rotation 绑定规则 |
| `security_workflow/06_traceability.md` | Yes | 增加 source precedence 追踪链，更新 FW package / OOB recovery 来源 |
| `security_workflow/04_change_impact.md` | Yes | 登记 CR-0014 影响和一致性检查 |
| `05_traceability/design_impact_matrix.md` | Yes | 追加 CR-0014 影响矩阵 |
| `00_project/decision_log.md` | Yes | 记录 DEC-0019 |
| `00_project/changelog.md` | Yes | 记录 CHG-0011 |
| `README_使用说明.md` | Yes | 工作流说明增加 current source precedence |
| `docs/*.md` 当前方案导出版 | Yes | 页首/修订说明同步当前方案源 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| manifest / constraints / baseline | 明确 `SRC-008` 当前为准，旧 `SRC-001` 历史参考 | 不覆盖官方 eHSM/TRM physical facts |
| boot / code rules | 替换 A/B 或 known-good slot 残留口径为单 FMC + OOB 重刷、GSP/runtime Host 重发 | 不降低 FMC/GSP 强制签名 + 加密 |
| full design / impl design | 更新固件包流程来源为 `SRC-008`，保留 eHSM-native 契约 | 不恢复旧 NGU physical header |
| traceability / change impact / docs | 让 source precedence 可追踪、可评审 | 不关闭仍未冻结的 open questions |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| `SRC-001` 作为当前安全方案基线 | `inputs_manifest.md`、约束、实现级来源 | `SRC-008 当前收敛安全软件方案 2.0` | 用户明确 2.0 是当前收敛版 |
| SEC2/runtime 建议 A/B 槽位 | `01_boot.md` | Host 重新下发并重新 eHSM verify/release | 2.0 版区分正常更新与刷机恢复 |
| FMC_A/FMC_B / `fmc_slot_metadata` code rule | `05_code_rules.md` | 单 FMC 固定分区 + OOB MCU 受控重刷 | CR-0013 与 2.0 版已取消 FMC A/B |
| key rotation 绑定 FMC A/B | `05_code_rules.md` | key epoch / pending / deprecated / OOB 可恢复路径 | 单分区模式下不再存在 inactive slot |

## 8. 不允许 Codex 自行改变的内容

- 不改变 eHSM 是唯一 Root of Trust。
- 不改变 BootROM -> eHSM verify/decrypt -> FMC 的启动可信裁决链。
- 不把 OOB MCU 提升为 SoC Root of Trust 或第一密码学验证者。
- 不新增 eHSM physical header、OTP/control/key/counter 字段。
- 不关闭 manifest ABI、exact key ID、OOB/QSPI ABI、工具 CLI/golden vector 等 open questions。

## 9. 验收标准

- [x] `SRC-008` 已登记为当前收敛方案源。
- [x] `C-SRC-01` 已写入约束。
- [x] baseline、full design、impl design、code rules、traceability 均体现当前 source precedence。
- [x] `05_code_rules.md` 不再要求 FMC_A/FMC_B 或 key rotation 绑定 inactive slot。
- [x] `01_boot.md` 不再建议 SEC2/runtime 采用 A/B recovery 槽位。
- [x] 未新增无依据 `[CONFIRMED]` physical ABI。
- [x] `git diff --check` 通过。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-06-03 |
| 修改文件 | `security_inputs/inputs_manifest.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `security_workflow/04_change_impact.md`; `05_traceability/design_impact_matrix.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `README_使用说明.md`; selected `docs/*.md` |
| 未修改但检查过的文件 | `05_traceability/file_sync_checklist.md`; `05_traceability/design_impact_matrix.md`; `change_requests/CR_template.md`; `CR-0012`; `CR-0013` |
| 未完成项 | manifest ABI、exact eHSM key ID、OOB/QSPI ABI、工具 CLI/golden vector 等仍按 open questions 追踪 |
| 执行说明 | 本 CR 是 source-of-truth 同步和旧口径补洞，不新增字段级硬件/接口冻结。 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PENDING` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `No, pending review` |
