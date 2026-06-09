# CR-0008: FMC A/B 防变砖与密钥 slot 轮换策略

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0008` |
| Title | FMC A/B 防变砖与密钥 slot 轮换策略 |
| Status | `accepted-for-application` |
| Owner | 项目安全方案 owner |
| Reviewer | 项目组 / eHSM owner / BootROM owner |
| Created Date | 2026-05-18 |
| Source / Context Pack | 用户于 2026-05-18 明确裁决 |
| Related Decision ID | `D-CR0008-FMC-AB-KEY-SLOT-ROTATION` |

## 2. 背景

用户明确修正 recovery 口径：

- NGU800 各级固件中，只有 FMC 存放在 Flash 中。
- GSP 及后续固件由 Host 下发，失败后可重新下发，不设计片上 recovery 分区。
- `slot` 在本议题中指密钥多 slot / key epoch / revoke 语义，不是 GSP/runtime 固件 A/B 分区。
- FMC 防变砖机制从“长期静态独立 recovery FMC”收敛为“Flash 中 FMC_A / FMC_B 双分区 + 受保护 fmc_slot_metadata”。
- 密钥轮换目标是支持服务器厂商 / 用户自主把控、轮换和撤销签名 / 加密密钥，同时不导致 FMC 不可恢复。

现有 `10_full_design.md` 中仍保留了旧口径：

- Recovery image 使用独立 image_type / signer / rollback / decrypt policy 的开放问题；
- SEC2 与主要运行期固件 A/B 槽位建议；
- recovery trust model 作为独立恢复镜像策略。

这些内容需要按本 CR 修正。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| inputs_manifest 摘要 | `SRC-002/SRC-006/SRC-007` 要求 eHSM 已定义的 header、OTP、key slot、Version Counter 优先按 eHSM 定义设计 |
| baseline 摘要 | eHSM 为 Root of Trust；FMC(SEC1) 来自本地 Flash；GSP(SEC2) 由 Host 投递 |
| 相关详设章节 | `10_full_design.md` 第 3 章、第 10.3 节、第 11 章 |
| 相关实现级文档 | `efuse_key_fw_header_design.md`、`ehsm_source_conformance_matrix.md`、`manufacturing_provisioning.md` |
| 已知冲突 | 旧 recovery 独立 image 口径与“仅 FMC 位于 Flash，GSP/runtime 不做片上 recovery 分区”冲突 |
| 待关闭 TBD | `fmc_slot_metadata` bit-level ABI、BootROM slot 选择规则、key alias 到 eHSM exact key ID 映射、key revoke 表达方式 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| FMC 防变砖机制 | Flash 中预留 `FMC_A` / `FMC_B` 两个等价启动分区和受保护 `fmc_slot_metadata` | FMC 是 Flash 中唯一需要片上备份保护的一级可变固件 | `[CONFIRMED]` |
| GSP/runtime recovery | GSP 及后续固件由 Host 下发，失败后重新下发，不设计片上 recovery 分区 | 避免把 Host-delivered 固件误设计成 Flash A/B | `[CONFIRMED]` |
| recovery FMC | 首版不引入长期静态独立 recovery FMC | FMC_A/B 已提供片上恢复能力，避免额外 recovery authority 和后门质疑 | `[CONFIRMED]` |
| recovery_boot_auth blob | 首版不作为必选机制 | 只有后续产品策略要求静态 rescue/recovery 固件且受客户密钥轮换控制时才引入 | `[CONFIRMED]` |
| key slot 语义 | `slot` 在本 CR 中指 eHSM key ID / key purpose / owner-confirmed logical key slot / key epoch，不指固件 A/B | 对齐用户澄清和 eHSM source-conformance 规则 | `[CONFIRMED]` |
| key slot 轮换 | 先用旧 active key 授权安装新 key，再用新 key 验证 inactive FMC，启动确认后切 active，形成新 key 体系下备份后才 revoke old key | 支持客户密钥自主把控与撤销，同时避免变砖 | `[CONFIRMED]` |
| eHSM key mapping | 结合 eHSM TRM 中 SOC FW Verify Key、SOC Encrypt Key、SOC Upgrade Verify Key、SOC Upgrade Encrypt Key、SOC Version Counter 等能力设计；exact key ID / key slot 映射仍由 eHSM owner 冻结 | 避免 Codex 自行发明 physical key slot | `[CONFIRMED / TBD exact ID]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 更新完整详设中的 recovery、FMC A/B、key slot rotation、开放问题 |
| `docs/ngu800_security_solution_feature_framework_hw_requirements.md` | Yes | 同步给领导/硬件同事看的功能框架和硬件需求口径 |
| `security_workflow/05_code_rules.md` | Should | 增加实现规则，防止后续代码按旧 recovery 口径开发 |
| `security_workflow/06_traceability.md` | Should | 更新 recovery traceability 的设计口径 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `10_full_design.md` | 将 Recovery 独立镜像策略替换为 FMC A/B 防变砖与 key slot 轮换策略；删除 SEC2/runtime A/B 建议；加入 eHSM key slot 轮换设计 | 不改变 eHSM RoT、FMC/GSP sign+encrypt、Host 不可信、eHSM native header 口径 |
| `feature_framework_hw_requirements.md` | 将 recovery 说明更新为 FMC A/B，说明 GSP/runtime 不做片上 recovery 分区 | 不把 FMC A/B 扩展为 GSP/runtime A/B |
| `05_code_rules.md` | 增加 FMC A/B、key rotation、old key delayed revoke、no static recovery FMC 规则 | 不新增代码接口字段 |
| `06_traceability.md` | 更新 T-UPD-002 描述 | 不关闭仍需 owner 冻结的 exact key ID / metadata ABI |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| Recovery image 独立 image_type / signer / counter / decrypt policy 是首版必需方向 | `10_full_design.md` / docs | 首版采用 FMC_A/FMC_B；静态 recovery FMC 和 recovery boot auth blob 非必选 | 用户明确收敛 |
| SEC2 与主要运行期固件建议 A/B 槽位 | `10_full_design.md` | GSP/runtime 由 Host 下发，失败后重新下发 | 只有 FMC 在 Flash 中 |
| slot 同时暗含 firmware slot 和 key slot | 多处 | 固件 slot 仅 FMC_A/FMC_B；密钥 slot 为 eHSM key ID / logical alias / key epoch | 避免概念混淆 |

## 8. 不允许 Codex 自行改变的内容

- 不得自行发明 eHSM exact key ID、OTP bit offset、key slot 编号或 counter ID。
- 不得把 `FMC_A/B` 推广到 GSP/runtime 片上 A/B 分区。
- 不得引入 SoC 厂商单方可启动 recovery 的后门模型。
- 不得把 `recovery_boot_auth blob` 写成首版必选。

## 9. 验收标准

- [x] `10_full_design.md` 已明确 FMC A/B、fmc_slot_metadata、fallback、boot confirm、key slot rotation。
- [x] 旧 recovery 独立 image 口径已替换为非首版可选策略。
- [x] eHSM TRM 能力引用保持为 source-conformance，不自行冻结 exact key ID。
- [x] GSP/runtime 不再被描述为片上 A/B 固件分区。
- [x] 文档明确密钥轮换必须与 FMC A/B 状态机绑定，且 old key 延迟 revoke。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-18 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`；`docs/ngu800_security_solution_feature_framework_hw_requirements.md`；`security_workflow/05_code_rules.md`；`security_workflow/06_traceability.md`；本 CR 文件 |
| 未修改但检查过的文件 | `security_workflow/04_impl_design/efuse_key_fw_header_design.md`、`security_workflow/04_impl_design/ehsm_source_conformance_matrix.md` 中的 eHSM source-conformance 口径保持不变 |
| 未完成项 | exact key ID、fmc_slot_metadata bit-level ABI、BootROM failure counter、key revoke 表达仍需 owner 冻结 |
| 执行说明 | 按用户明确裁决进入 accepted-apply；首版采用 FMC_A/FMC_B 防变砖，GSP/runtime 不做片上 recovery 分区；密钥 slot 轮换与 FMC A/B 状态机绑定 |
