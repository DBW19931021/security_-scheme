# 未关闭问题 Open Questions

## 目的

本文件用于集中记录尚未冻结、仍需项目组或安全负责人裁决的问题。任何阻塞 baseline、主路径设计、实现接口冻结或验证闭环的问题，都应登记在这里。

## 记录字段

| Question ID | Topic | Blocking Area | Owner | Status | Needed Decision |
|---|---|---|---|---|---|
| OQ-0001 | 设计变更管理流程试运行 | CR / GPT / Codex 协作流程 | TBD | `[OPEN]` | 后续第一个真实 CR 完成后，确认是否需要调整模板字段和状态流 |
| OQ-0002 | SEC1 是否强制加密 | Boot / FW Protection | NGU800 Security Design | `[CLOSED]` | 已由 `CR-0001` / `DEC-0001` 裁决：SEC1 正式安全启动路径必须签名 + 加密 |
| OQ-0003 | runtime image signature-only 白名单 | Boot / Key / Product Policy | Product Security Owner | `[OPEN]` | `CR-0003` 已裁决 SEC2 强制 sign+encrypt，并将 PM/RAS/Codec USER/PROD 默认定为 sign+encrypt；仍需冻结哪些非敏感 runtime image 可进入 signature-only 白名单及准入条件 |
| OQ-0004 | board binding 是否参与 firmware release decision | Board / Boot / Manufacturing | Security Owner / Board Owner | `[OPEN]` | `CR-0003` 已裁决 V2.4 阶段 board binding 默认进入 attestation、不阻断 SEC1；仍需冻结是否参与 SEC2/runtime verify/decrypt/release decision |
| OQ-0005 | JTAG scope bitmap 与 MUX 控制权 | Debug / Board / RTL | RTL / Board Owner | `[OPEN]` | `CR-0003` 已裁决 USER/PROD 默认关闭和 lifecycle+auth+scope+timeout+audit 策略；仍需冻结 bit-level mapping、eHSM debug bitmap、CPLD/MUX 控制寄存器归属 |
| OQ-0006 | 管理子系统 DMA / firewall / UserID | Board / Interface / RTL | RTL / SoC Integration | `[OPEN]` | `CR-0003` 已裁决安全资源默认拒绝、只允许白名单 staging/data buffer；仍需冻结 UserID、firewall region、地址范围、错误隐藏和审计字段 |
| OQ-0007 | Attestation report image protection / board / event 字段 | Attestation / SPDM | Security Owner / Verifier Owner | `[OPEN]` | `CR-0003` 已裁决 lifecycle/debug/secure_boot/rollback 为 report 必须项，并将 image protection policy/decrypt_applied/board_bind_result 作为 `[ASSUMED]`；仍需冻结字段位置和 PowerBrake/PG/FAULT/reset 是否进入主 report |
| OQ-0008 | OOB/BMC provisioning proxy 字段级设计 | Manufacturing / Board | Manufacturing / Board Owner | `[OPEN]` | `CR-0003` 已裁决 OOB/BMC 不得成为 trust anchor、只可作为 transport proxy；仍需冻结命令格式、认证方式、审计记录、失败回滚、rate limit / lockout |
| OQ-0009 | OOB MCU FMC 重刷恢复 ABI | Boot / Recovery / RMA / Board | Security Owner / Board Owner | `[OPEN]` | 首版不采用长期静态 recovery image；需冻结 OOB secure boot、QSPI ownership/arbiter、NOR FMC 主区域写保护、恢复授权 capsule、状态/审计记录和掉电保护 |
| OQ-0010 | Manufacturing / RMA 验收闭环 | Manufacturing / RMA / Attestation | Manufacturing / Security Owner | `[OPEN]` | 冻结 Root injection mode、OTP/eFuse 不可读字段验收方式、RMA re-acceptance 和返修后 attestation/status 归档策略 |
| OQ-0011 | NGU protected manifest ABI | FW Header / Boot / Tooling | Security Owner / SEC FW Owner | `[OPEN]` | `CR-0004` 已裁决 NGU metadata 放入 eHSM Code region protected manifest；仍需冻结 manifest 是否必须位于 Code region 起始位置、bit-level ABI、extension/TLV 格式和工具生成规则 |
| OQ-0012 | eHSM 是否解析 NGU manifest | eHSM / Boot / Interface | eHSM Owner / Security Owner | `[OPEN]` | `CR-0004` 当前采用 BootROM / SEC 在 eHSM verify/decrypt 成功后解析 manifest；是否要求 eHSM firmware / bootloader 直接解析 NGU manifest 仍需裁决 |
| OQ-0013 | exact eHSM key ID 与 OTP/control bit mapping | Key / OTP / Manufacturing | eHSM Owner / RTL Owner | `[OPEN]` | `CR-0004` 已裁决 NGU key names 和 OTP-0..OTP-7 仅为 logical alias；仍需冻结 FW verify/decrypt、debug auth、attestation、control field 的 exact eHSM key ID / bit mapping |
| OQ-0014 | per-image rollback counter 与 wrapped CEK customization | Update / FW Protection / eHSM | eHSM Owner / Product Security Owner | `[OPEN]` | `CR-0004` 已将 `*_MIN_VER` 降为 logical rollback domain，并将 per-image CEK / wrapped CEK 标为 TBD；若产品要求 per-image counter 或 wrapped CEK，需要 eHSM customization CR |
| OQ-0015 | SEC1 early boot exact eHSM command path | BootROM / eHSM Bootloader | BootROM Owner / eHSM Owner | `[OPEN]` | `CR-0004` 已要求 SEC1 sign+encrypt 不走 NVM only verify；仍需冻结 BootROM 调用 eHSM Bootloader `bl_verify_image` 或等价 ROM path 的 exact ABI、output buffer 和错误模型 |
| OQ-0016 | Image packager CLI、manifest ABI 与 golden vector | Tooling / BootROM / SEC FW / eHSM Adapter | Tooling Owner / SEC FW Owner / eHSM Owner | `[OPEN]` | `CR-0006` 已冻结固件制作与设备侧 verify/decrypt 的流程方向；仍需冻结 `ngu_image_manifest_t` bit-level ABI、packager CLI 参数、source-conformance report 格式和 BootROM/SEC/eHSM adapter 联调用 golden vector |

## 状态定义

| Status | 含义 |
|---|---|
| `[OPEN]` | 尚未关闭 |
| `[IN_REVIEW]` | 正在评审 |
| `[BLOCKED]` | 阻塞设计或实现冻结 |
| `[CLOSED]` | 已关闭 |
| `[SUPERSEDED]` | 已被其他问题或决策替代 |
