# CR-0012 启动/升级/Debug 三套扁平密钥模型与 eFuse 统一表述

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0012` |
| Title | 启动/升级/Debug 三套扁平密钥模型与 eFuse 统一表述 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-05-19 |
| Source / Context Pack | 用户 review：Debug、Update 等授权域不要设计多层 key；总体按“启动一套、升级一套、Debug 一套”收敛；SoC 非易失密钥存储统一为 eFuse |
| Related Decision ID | CR-0010 / CR-0011 |

## 2. 背景

当前第 8 章在外部签名 / 授权密钥、证书与锚点对象中仍存在多层授权表达，例如多级 Debug 授权、RMA 单独授权、设备证明多级签发、单独 key rotation signing key 等。该表达对方案评审和硬件需求沟通不够友好，也超过了当前项目希望的首版复杂度。

项目组当前希望首版按照最小可落地原则收敛：

- 启动授权一套签名 key / anchor。
- 升级授权一套签名 key / anchor，升级包、key rotation capsule、FMC A/B slot 切换和旧 key 吊销均归入升级授权域。
- Debug 授权一套签名 key / anchor，Debug 与 RMA 通过 token scope 区分，不再拆多级 Debug 授权证书。
- 设备证明保留设备内部证明私钥和一套设备证明证书签发方，不在 SoC 方案中拆多级签发对象。
- 上述“三套”只约束外部签名 / 授权私钥；固件制作侧仍需要由离线 HSM / KMS 管理固件加密 / 包裹材料，设备侧仍必须保留 `Soc Encrypt Key`、`Soc Upgrade Encrypt Key`、`FW_KEK / image protect key` 或 eHSM owner 确认的等价 key policy。
- SoC 上用于持久保存 key、anchor、counter、control、lifecycle 的非易失器件统一表述为 eFuse。

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 启动 key | 使用 `secure_boot_signing_key` / `secure_boot_signer_anchor` 表达启动域唯一固件签名授权 | 负责 FMC/GSP/runtime 启动镜像真实性 | `[CONFIRMED]` |
| 升级 key | 使用 `update_authorization_signing_key` / `update_authorization_anchor` 表达升级域唯一授权 | 同时覆盖升级包、key rotation capsule、key epoch、FMC A/B slot 切换和旧 key 吊销 | `[CONFIRMED]` |
| Debug key | 使用 `debug_authorization_signing_key` / `debug_authorization_anchor` 表达 Debug/RMA 授权 | Debug/RMA 通过 token scope、challenge、UID、expiry 区分，不拆多层证书链 | `[CONFIRMED]` |
| 固件制作加密 key | 在外部 key 表中保留 `firmware_packaging_encrypt_key`，由离线 HSM / KMS 用于固件制作侧加密或包裹 CEK | KMS/HSM 需要管理固件机密性保护材料；它不是签名授权 key | `[CONFIRMED]` |
| 设备内部对称解密 key | 保留 `soc_encrypt_key`、`soc_upgrade_encrypt_key`、`FW_KEK / image protect key` 等设备内部 eHSM 对称解密 / unwrap policy | 三套简化规则针对外部签名授权，不删除固件机密性保护所需的内部对称 key | `[CONFIRMED]` |
| 设备证明签发 | 使用 `device_attestation_ca_signing_key` 表达一套设备证明证书签发方 | 设备证明仍需证书签发，但 SoC 方案只表达单一签发方 | `[CONFIRMED]` |
| eFuse 表述 | 正文中用于 SoC 非易失 key/anchor/control/counter/lifecycle 存储的表述统一为 eFuse | SoC 已确定非易失密钥存储器件为 eFuse，避免多种名称混用 | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 更新第 8 章密钥体系、架构图、时序图、key mapping 与 eFuse 表述 |
| `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` | Yes | 同步项目组汇报版 |

## 5. 验收标准

- [x] 第 8 章外部授权 key 只保留启动、升级、Debug、设备证明签发四类；其中启动/升级/Debug 为首版三套核心授权域。
- [x] 明确“三套核心授权域”仅指外部签名 / 授权私钥；外部固件制作加密 key 与内部对称解密 / unwrap key 不被删除。
- [x] 升级域不再单独列独立 key rotation 签名私钥，key rotation capsule 归入 `update_authorization_signing_key`。
- [x] Debug/RMA 不再拆多级 Debug 授权或 RMA 单独 key，统一由 Debug 授权 key 通过 scope 区分。
- [x] SoC 非易失存储统一表述为 eFuse，目标文档不再出现旧的混合表述。
- [x] Mermaid 图可渲染。
- [x] `git diff --check` 通过。

## 6. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-19 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`；`docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` |
| 未完成项 | exact eHSM key ID / key level / key purpose 仍待 eHSM owner 冻结；设备证明证书块是否默认随 report 携带仍待产品接入策略确认 |
| 执行说明 | 将启动、升级、Debug 三套外部签名 / 授权域扁平化；key rotation 归入升级授权；Debug/RMA 归入 Debug scoped token；设备证明只表达单一签发方；固件制作侧加密 key 保留在离线 HSM / KMS，设备侧对称解密 / unwrap key 保留在 eHSM key 体系；两份目标文档中的 SoC 非易失存储表述统一为 eFuse。 |
