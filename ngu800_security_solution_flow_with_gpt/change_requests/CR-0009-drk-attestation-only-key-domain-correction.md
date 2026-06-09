# CR-0009 DRK 与设备证明派生边界修正

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0009` |
| Title | DRK 与设备证明派生边界修正 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-05-18 |
| Source / Context Pack | 用户 review：第 8 章 8.5 / 8.6 中不应把 FW verify / FW encrypt / debug 都画成由 DRK 派生 |
| Related Decision ID | Key hierarchy / attestation / eHSM native key slot alignment |

## 2. 背景

当前 `10_full_design.md` 及项目组汇报版文档第 8 章将 `DRK` 画成 `FW Verify Branch`、`FW Encrypt Branch`、`Attestation Branch`、`Debug Auth Branch` 的共同上游，并在时序图中写成“派生 FW Verify / Encrypt / Attestation / Debug branches”。

该表述会误导为：固件验签 key、固件解密 key、debug auth key 都由设备根派生密钥 `DRK` 派生。此口径与 eHSM native key slot / signer anchor / FW_KEK policy 的落地方向不一致，也不符合“设备认证密钥对才属于设备唯一派生域”的设计边界。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| inputs_manifest 摘要 | `SRC-006/SRC-007` 明确 eHSM native SOC FW verify/encrypt key、upgrade verify/encrypt key、counter/control field 是实现级事实源 |
| constraints 摘要 | 私钥不得导出，key 使用绑定 lifecycle，所有正式密码操作走 eHSM |
| baseline 摘要 | eHSM 是 Root of Trust；BootROM 不持有 root/private key；Host 不可信 |
| 相关详设章节 | `10_full_design.md` 第 8 章 |
| 相关实现级文档 | `efuse_key_fw_header_design.md` / source conformance matrix |
| 已知冲突 | DRK 被描述为所有 key branch 的共同上游 |
| 待关闭 TBD | exact eHSM key ID / level / purpose 仍需 eHSM owner 冻结 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| DRK 作用域 | DRK 仅作为设备身份 / 设备证明派生域的逻辑对象，不作为固件验签、固件解密、debug auth 的共同上游 | 避免把设备唯一身份根扩展成通用业务密钥根 | `[CONFIRMED]` |
| 固件验签 | 固件验签使用 eHSM native `Soc FW Verify Key` / `Soc Upgrade Verify Key` / signer anchor / cert anchor / owner mapping | 对齐 eHSM TRM 和 CR-0004 source-conformance | `[CONFIRMED]` |
| 固件解密 | 固件解密使用 eHSM native `Soc Encrypt Key` / `Soc Upgrade Encrypt Key` / FW_KEK policy / owner mapping | 对齐 SEC1/SEC2 强制签名+加密和 eHSM key slot 模型 | `[CONFIRMED]` |
| Debug Auth | Debug Auth 使用独立 debug auth anchor / user auth key / cert policy，不复用普通 attestation key | 防止证明成功被误当成 debug 授权 | `[CONFIRMED]` |
| 设备证明 | Device Attestation KeyPair / Alias Key 可由 UDS / Root Secret / DRK / Attestation Seed 在 eHSM 内部派生或生成，私钥不可导出 | 设备唯一身份需要设备绑定能力 | `[CONFIRMED]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 修正第 8 章架构图、时序图、密钥层级、label 和结论 |
| `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` | Yes | 修正项目组汇报版同源章节 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `10_full_design.md` | 将 DRK 从共同上游改成设备证明派生域；固件验签/解密/debug 改成独立 eHSM key domain / key slot / anchor / policy | 不改 RoT、SEC1/SEC2 签名+加密、FMC A/B、Host trust boundary |
| 汇报版文档 | 与 `10_full_design.md` 保持一致，避免领导/项目组误解密钥派生关系 | 不减少硬件需求和工程落地细节 |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| DRK 派生 FW Verify / FW Encrypt / Attestation / Debug branches | 第 8 章图和时序 | DRK 仅服务设备证明；FW verify/encrypt/debug 是独立 eHSM key domain | 避免设备身份根被误作通用业务密钥根 |
| 所有 key branch 都从 Root / UDS 语义派生 | 第 8 章图下说明 | eHSM/OTP 是共同安全域；只有设备证明密钥对属于设备唯一派生域 | 对齐 eHSM native key slot |
| `NGU800:FW:VERIFY` / `NGU800:FW:ENC` 作为普通 KDF label | 第 8.11 节 | 改为 policy/domain tag，不表达从 DRK 派生 | 防止实现误按 label 做 DRK-KDF |

## 8. 不允许 Codex 自行改变的内容

- 不得改变 eHSM 为 Root of Trust 的口径。
- 不得改变 SEC1/SEC2 正式路径签名 + 加密要求。
- 不得自行冻结 exact eHSM key ID / key level / key purpose。
- 不得把 Debug Auth 与 Attestation 合并。

## 9. 验收标准

- [x] 第 8.5 架构图不再显示 DRK 指向 FW Verify / FW Encrypt / Debug。
- [x] 第 8.6 时序图不再描述从 DRK 派生 FW Verify / FW Encrypt / Debug。
- [x] 第 8.9 密钥层级明确区分设备证明派生域、固件验签域、固件加密域、debug auth 域。
- [x] 第 8.11 label 说明不再把 FW verify/encrypt 作为 DRK KDF label。
- [x] Mermaid 图可渲染。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-18 |
| 修改文件 | `10_full_design.md`、项目组汇报版文档 |
| 未修改但检查过的文件 | `inputs_manifest.md`、eHSM source-conformance 相关章节 |
| 未完成项 | exact eHSM key ID / key level / key purpose 仍待 eHSM owner 冻结 |
| 执行说明 | 根据用户 review 修正 DRK 语义边界，并保持 eHSM native key slot 口径 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PENDING` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `No` |
