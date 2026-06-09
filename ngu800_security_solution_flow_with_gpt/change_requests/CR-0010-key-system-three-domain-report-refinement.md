# CR-0010 密钥体系三域模型与项目组汇报版细化

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0010` |
| Title | 密钥体系三域模型与项目组汇报版细化 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-05-18 |
| Source / Context Pack | `docs/NGU800_安全方案详细设计_项目组汇报版_第八章密钥体系修正版 (1).md` + 用户要求“参照这个文档，再修改修改” |
| Related Decision ID | CR-0009 DRK 与设备证明派生边界修正 |

## 2. 背景

CR-0009 已修正“DRK 派生所有 key branch”的错误口径。用户提供的第八章修正版进一步把密钥体系拆成三套职责清晰的体系：

1. 外部签名 / 授权体系；
2. 设备内部 eHSM key 体系；
3. 设备身份与证书体系。

该拆分有助于项目组理解固件签名私钥、Debug/RMA 授权私钥、设备证明私钥、固件解密 key、证书 CA 私钥之间的边界，避免把所有密钥都画成一棵设备内派生树。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 相关详设章节 | `10_full_design.md` 第 8 章 |
| 相关汇报文档 | `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` 第 8 章 |
| 已接受 CR | CR-0009 已确认 DRK 不作为 FW verify / FW encrypt / debug auth 的共同上游 |
| 需保持 TBD | exact eHSM key ID / key level / key purpose 仍由 eHSM owner 冻结 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 三域模型 | 第 8 章按外部签名/授权、设备内部 eHSM key、设备身份与证书三套体系组织说明 | 项目组更容易理解私钥位置和设备侧保存对象 | `[CONFIRMED]` |
| 外部签名/授权 | 固件签名、升级授权、Debug CA/Tool、RMA 授权、设备证书签发 CA 私钥均在离线 HSM/CA/KMS/受保护工具侧 | 设备只保存 anchor/hash/policy 并验签，不生成这些外部私钥 | `[CONFIRMED]` |
| 设备内部 eHSM key | SOC Encrypt/Upgrade Encrypt、Device Attestation Private Key、counter/lifecycle/control field 等由 eHSM/OTP/KMU 管理，不导出 | 对齐 eHSM Root of Trust 与不可导出边界 | `[CONFIRMED]` |
| 设备身份与证书 | Device Attestation Private Key 在 eHSM 内部生成或派生，公钥/CSR 由离线 CA/HSM 签发证书 | 区分设备证明私钥和 CA 私钥 | `[CONFIRMED]` |
| DRK 语义 | DRK 可作为 eHSM 内部 device-local protection/wrapping context 或设备证明派生域语义，但不得表达为外部签名私钥来源或 FW verify/encrypt/debug 的软件可见 KDF 上游 | 兼容 eHSM 内部实现可能性，同时避免误导工程实现 | `[CONFIRMED]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 强化第 8 章三域模型、总图、分类表、制造/硬件需求 |
| `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` | Yes | 同步项目组汇报版第 8 章 |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `10_full_design.md` | 吸收三域模型、外部私钥位置、设备侧 anchor、Debug Auth 方向、Device Attestation Key/CSR/证书关系 | 不把 FW verify/encrypt/debug 改回 DRK 派生 |
| 汇报版文档 | 使用同样口径，保留 SoC/硬件需求 | 不引入历史/旧口径叙述作为最终方案正文 |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| 单一 key tree 表达 | 第 8 章 | 三域模型 | 防止外部签名私钥、设备内部 key、CA key 混淆 |
| Debug Auth 被理解成设备生成私钥给外部 | 第 8 章 | 外部授权方签名，设备侧验签并开受限 scope | 与 Debug/RMA 安全边界一致 |

## 8. 不允许 Codex 自行改变的内容

- 不得自行冻结 exact eHSM key ID / key level / key purpose。
- 不得改变 SEC1/SEC2 签名 + 加密强制策略。
- 不得把设备证明成功等同于 debug 授权成功。
- 不得把离线 CA/HSM 私钥放入设备侧。

## 9. 验收标准

- [x] 第 8 章明确三域模型。
- [x] 第 8 章明确外部 signing/auth private key 不在设备侧。
- [x] 第 8 章明确 Device Attestation Private Key 不出 eHSM，CSR/public key 可导出给 CA 签发证书。
- [x] 第 8 章明确 Debug Auth 方向为外部授权方签名、设备侧验签。
- [x] 第 8 章明确 SoC/硬件需求。
- [x] Mermaid 图可渲染。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-18 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`；`docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` |
| 未修改但检查过的文件 | `docs/NGU800_安全方案详细设计_项目组汇报版_第八章密钥体系修正版 (1).md` |
| 未完成项 | exact eHSM key ID / key level / key purpose 仍待 eHSM owner 冻结 |
| 执行说明 | 吸收参考稿的三域模型、外部授权私钥位置、Device Attestation CSR/证书签发、Debug/RMA 授权方向、Key Epoch 与 FMC A/B 关系，并保持 CR-0009 的 DRK 边界 |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PENDING` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `No` |
