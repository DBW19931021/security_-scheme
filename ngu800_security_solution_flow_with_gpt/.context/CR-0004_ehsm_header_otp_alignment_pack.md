# CR-0004 Context Pack: eHSM Header / OTP Alignment

> 用途：交给 GPT / security owner 生成正式 CR-0004。
> Codex 在本文件中只整理上下文、证据、冲突、影响面和待裁决问题，不冻结安全方案，不修改正文。

## 1. 当前仓库状态

| 字段 | 内容 |
|---|---|
| 当前工作目录 | `/home/may-pc/share/code/ngu800/secure/security_-scheme/ngu800_security_solution_flow_with_gpt` |
| Current git commit | `df815e2` |
| Context pack date | 2026-05-07 Asia/Shanghai |
| Work mode | `context-pack` |
| Requested topic | 针对 `efuse_key_fw_header_design.md` 中 FW Header 与 OTP/eFuse 排布同 eHSM TRM 冲突的问题，生成给 GPT 的 CR-0004 输入包。 |
| 本次 Codex 动作边界 | 仅新增本 context pack；不应用正文修改；不将任何设计选项升级为 `[CONFIRMED]`。 |

当前工作树已有多处未提交修改和未跟踪文件。特别说明：

- `change_requests/CR-0004-ehsm-native-header-otp-layout-alignment.md` 是 Codex 先前生成的 untracked draft，应视为可参考草案，不是 GPT / owner 正式 CR。
- 本 pack 是本轮正确流程的输入材料：先给 GPT / owner，再由 GPT / owner 生成或确认 CR。
- 后续只有在正式 CR accepted 后，Codex 才应进入 accepted-apply 修改正文。

## 2. 任务背景

用户指出两个同类问题：

1. `OSR_eHSM_Firmware_TRM_1.0` 中已经规定 eHSM 推荐/原生的固件镜像头格式，而 `security_workflow/04_impl_design/efuse_key_fw_header_design.md` 又定义了一套 `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t`。
2. eHSM TRM 已经定义 OTP 控制字段、版本计数器、key slot 排布，而 `efuse_key_fw_header_design.md` 又定义了一套 `OTP-0..OTP-7`、`LIFECYCLE_STATE`、`SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER`。

用户希望遵循既有协作分工：

- GPT / security owner：做方案设计和 CR 裁决。
- Codex：整理上下文 pack，并在 CR accepted 后落实到仓库。

## 3. Source Priority 摘要

| Source ID | 文件 / 来源 | 当前状态 | 与本主题关系 |
|---|---|---|---|
| `SRC-002` | `security_inputs/ip_manuals/ehsm/` | `confirmed` / `preferred` | eHSM 已定义的固件字段、OTP/eFuse 排布、key slot 语义、计数器、生产阶段操作优先沿用。 |
| `SRC-003` | `security_inputs/soc_arch/启动方案.pdf` | `draft` / `preferred` | 启动流程、SEC1/SEC2 职责、SEC1 来源仍需保持。 |
| `CR-0001` | SEC1 encryption / FW protection | `applied` | SEC1 sign+encrypt 已收敛，不应因 header/OTP 对齐而降低。 |
| `CR-0003` | SEC2/runtime policy / board/debug/DMA/OOB | `applied` | SEC2 sign+encrypt、runtime 默认策略等不应被本主题回退。 |
| Codex draft CR-0004 | `change_requests/CR-0004-ehsm-native-header-otp-layout-alignment.md` | untracked draft | 仅可作为材料参考，不是 owner 结论。 |

Manifest 中的关键证据：

| 文件 | 行号 | 摘要 |
|---|---:|---|
| `security_inputs/inputs_manifest.md` | 26-27 | `SRC-001` 的固件头格式可参考但需尽量对齐 eHSM；`SRC-002` 为 preferred，eHSM 已定义的固件字段、OTP/eFuse、key slot、计数器和生产阶段操作优先按 eHSM 定义设计。 |
| `security_inputs/inputs_manifest.md` | 44-47 | CHG-002 / CHG-004 已明确 eFuse 字段、生产阶段操作和 eHSM 已定义技术细节应优先沿用。 |

## 4. eHSM TRM 证据摘要

### 4.1 eHSM FW Control / SOC Control

| 来源 | 证据 | 设计含义 |
|---|---|---|
| `OSR_eHSM_Firmware_TRM_1.0` text lines 1236-1300 | `FW Control-eHSM` 定义 `EhsmCodeUpgradeAlg`、`EhsmCodeVerifyAlg`，均为 64-bit control field 中的算法选择位。 | 算法选择是 eHSM OTP/control field 的物理语义，不应由 NGU 镜像头自由声明为权威。 |
| `OSR_eHSM_Firmware_TRM_1.0` text lines 1315-1377 | `FW Control-SOC` 定义 `SocUpgradeAlg`、`SocBootAlg`。 | SOC 启动/升级验签算法应映射到 eHSM `SocBootAlg / SocUpgradeAlg`。 |

### 4.2 eHSM Version Counter

| 来源 | 证据 | 设计含义 |
|---|---|---|
| `OSR_eHSM_Firmware_TRM_1.0` text lines 1380-1405 | OTP 包含 eHSM FW 和 SOC FW 的 `Version Counter`；它是 16 字节单向数据；升级时新 counter 必须大于等于 OTP 中记录值；总计支持 128 个版本。 | 当前多个 32-bit `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 不能直接写成 eHSM 物理 OTP 字段。需要 GPT 裁决：单 SOC package counter，还是 eHSM 定制 per-image counter。 |

### 4.3 eHSM OTP Key Layout

| 来源 | 证据 | 设计含义 |
|---|---|---|
| `OSR_eHSM_Firmware_TRM_1.0` text lines 1432-1509 | OTP key 从 offset `0x70` 开始，表中定义 key ID / Level / 用途，例如 `Soc Debug Verify Key`、`Soc FW Verify Key`、`Soc Encrypt Key`、`Soc Upgrade Encrypt Key`、`Soc Upgrade Verify Key`。 | NGU 的 `FW Verify Key / FW_KEK / Image CEK / Attestation Seed / Debug Auth Seed` 需要映射到 eHSM key ID / level / purpose，不能直接作为新增物理 key slot。 |

### 4.4 eHSM Secure Boot Deployment

| 来源 | 证据 | 设计含义 |
|---|---|---|
| `OSR_eHSM_Firmware_TRM_1.0` text lines 2700-2745 | SOC FW 支持 RAM deploy：eHSM 读取镜像，解密到 SoC RAM，并校验签名。 | SEC1/SEC2 mandatory encrypt 更适合走 staging RAM / output buffer 模型。 |
| `OSR_eHSM_Firmware_TRM_1.0` text lines 2770-2789 | NVM deploy 需要 eHSM 能读 NVM 地址，且镜像只包含签名值，不能加密。 | SEC1/SEC2 sign+encrypt 不能走 NVM only verify 模式。 |

### 4.5 eHSM Native Secure Boot Image Header

| 来源 | 证据 | 设计含义 |
|---|---|---|
| `OSR_eHSM_Firmware_TRM_1.0` text lines 2811-2877 | 安全启动镜像格式图：1KB plaintext image head，字段包括 `Signature`、`Public_Key`、`Encrypt_IV`、`Valid_Flag`、`Image_Type`、`Plain_Flag`、`Naked_Flag`、`Reserved`、`Code_Size`、`Version_Counter`、`Public_Key_Ext`；`Code` 位于 header 之后，可加密。 | eHSM 已有物理 image container。NGU 不宜再定义并列 physical header。 |
| `OSR_eHSM_Firmware_TRM_1.0` text lines 2889-2982 | 表 16 给出字段 offset/size：`Signature` offset 0 size 256；`Public_Key` offset 256 size 320；`Encrypt_IV` offset 576 size 16；`Image_Type` offset 596 size 1；`Code` offset 1024。 | 当前 `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t` 与 eHSM 原生 layout 冲突。 |
| `OSR_eHSM_Firmware_TRM_1.0` text lines 2996-3008 | 验证算法参考 `SocBootAlg`；加密算法 AES128-CBC 或 SM4-CBC；算法类型在 OTP 中配置。 | NGU header 中 `algo_family / hash_algo / sig_algo / enc_algo / enc_mode` 不能作为算法 authority。 |

## 5. 当前 NGU 实现级设计证据

| 文件 | 行号 | 当前内容 | 冲突点 |
|---|---:|---|---|
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 20-27 | 写明“优先复用 eHSM 已定义能力与字段语义”，包括 OTP、生命周期、控制字段、版本计数器、Bootloader/Firmware 镜像验证能力。 | 原则正确，但后续章节没有建立 eHSM field-level mapping。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 45-56 | 定义 `OTP-0..OTP-7` 逻辑分区。 | 容易被误读为物理 eFuse/OTP 排布，和 eHSM OTP layout 并列。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 62-79 | 定义 `LIFECYCLE_STATE` 8 bit、`SECURE_BOOT_EN`、`DEBUG_AUTH_EN`、`FW_ENCRYPT_EN`、`ANTI_ROLLBACK_EN` 等 1-bit 字段。 | eHSM lifecycle/control field 已有物理编码；当前写法像新增物理字段。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 81-90 | 定义多个 32-bit `SEC1_MIN_VER / SEC2_MIN_VER / PMP_MIN_VER / ...`。 | eHSM 原生版本计数器是 16 字节 SOC/eHSM FW counter，不是多个 32-bit floor。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 94-115 | 定义 NGU key hierarchy：UDS/DRK/FW Verify/FW Encrypt/Image CEK/Attestation/Debug。 | 需要映射到 eHSM key ID / level / purpose，否则是并列 key model。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 119-171 | 定义 `ngu_fw_min_hdr_t` 和 `ngu_fw_signed_hdr_t`。 | 直接与 eHSM native 1KB header 冲突。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 176-184 | 把 `algo_family`、rollback、wrapped CEK、nonce/IV、AAD、ciphertext 等写入 signed region，并定义 SEC1/SEC2 policy。 | 策略需求有效，但承载位置需要改为 eHSM native header + protected manifest 或 eHSM customization。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 187-194 | `IMAGE_TYPE_SEC1/SEC2/PMP/RMP/OMP/MMP/RECOVERY` 与 rollback counter 映射。 | NGU image type 与 eHSM `Image_Type` 同名但语义不同；counter mapping 未对齐 eHSM 128-bit counter。 |

## 6. 已知冲突

| 冲突 ID | 冲突描述 | 涉及文件 | 当前状态 | 需要 GPT 裁决的问题 |
|---|---|---|---|---|
| CF-004 | eHSM 已定义 native secure boot image header；NGU 实现级设计又定义并列 physical header。 | `OSR_eHSM_Firmware_TRM_1.0`; `efuse_key_fw_header_design.md`; `mailbox_if.md`; `01_boot.md` | `[OPEN]` | 是否采用 eHSM native header 作为唯一 physical image container？NGU 自定义字段应放在 manifest 还是 eHSM reserved/custom extension？ |
| CF-005 | eHSM 已定义 OTP control / version counter / key slot layout；NGU 实现级设计又定义逻辑 OTP 分区和多个 32-bit counter。 | `OSR_eHSM_Firmware_TRM_1.0`; `efuse_key_fw_header_design.md`; `manufacturing_provisioning.md`; `02_key_cert.md` | `[OPEN]` | NGU 逻辑字段如何映射到 eHSM physical field / key ID / counter？是否需要 eHSM 定制扩展？ |
| CF-006 | SEC1/SEC2 mandatory encrypt 与 eHSM NVM only verify 条件冲突。 | `OSR_eHSM_Firmware_TRM_1.0`; `01_boot.md`; `mailbox_if.md` | `[OPEN]` | SEC1/SEC2 是否统一采用 RAM deploy / staging buffer / output buffer verify+decrypt path？ |
| CF-007 | NGU `Image Type` 语义与 eHSM `Image_Type` 语义不同。 | `efuse_key_fw_header_design.md`; `OSR_eHSM_Firmware_TRM_1.0` | `[OPEN]` | eHSM `Image_Type` 是否保持 key-domain semantics，NGU image type 另放 manifest？ |

## 7. 可供 GPT 选择的设计方向

### Option A: eHSM native header + NGU protected manifest

把 eHSM 1KB native header 作为唯一 physical image container。

NGU 项目级 metadata 不再定义为 physical header，而是放在 eHSM `Code` 区开头的 `ngu_image_manifest_t`。该 manifest 随 Code 一起被 eHSM 原生签名/加密保护。

```text
[eHSM Native Image Header, 1KB, plaintext]
  Signature
  Public_Key
  Encrypt_IV
  Valid_Flag
  Image_Type       // eHSM key-domain semantics
  Plain_Flag
  Naked_Flag
  Code_Size
  Version_Counter
  Public_Key_Ext

[Code region, covered by eHSM verification/encryption model]
  ngu_image_manifest_t
  actual firmware payload
```

待 GPT 裁决点：

- manifest 是否必须位于 Code 区起始位置？
- manifest 是否需要独立 magic/version/hash？
- release decision 是由 eHSM verify result + SEC/BootROM manifest policy 共同决定，还是 eHSM 需要解析 manifest？

### Option B: eHSM native header with eHSM customization

保留 eHSM header 外形，但由 eHSM owner 明确授权 reserved/custom fields 承载 NGU image type、policy、rollback、board binding 等。

待 GPT 裁决点：

- 是否允许修改 eHSM firmware / bootloader 解析器？
- 是否会破坏 OSR eHSM 工具链或升级镜像生成流程？
- reserved 字段可用性谁确认？

### Option C: 保留 NGU header，做 adapter

继续保留 `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t`，由 adapter 转成 eHSM command。

待 GPT 裁决点：

- 该方案是否违反 `SRC-002` preferred？
- 如果保留，如何避免 eHSM TRM 与 NGU design 两套事实源？

Codex 观察：Option A 最符合 `SRC-002`，但最终选择应由 GPT / security owner 裁决。

## 8. OTP / eFuse 对齐需要 GPT 裁决的问题

1. `OTP-0..OTP-7` 是否只保留为 NGU logical view，不再表达 physical offset？
2. eHSM lifecycle 32-bit encoding 是否作为唯一 physical lifecycle encoding？
3. `SECURE_BOOT_EN / DEBUG_AUTH_EN / JTAG_FORCE_DISABLE / FW_ENCRYPT_EN / ATTEST_EN / ANTI_ROLLBACK_EN` 如何映射到 eHSM `FW Control-eHSM`、`FW Control-SOC`、硬件 control field 或 NGU manifest / policy table？
4. `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 是否改为 logical rollback domain？首版是否只使用 eHSM SOC 128-bit package counter？
5. 若需要 per-image rollback，是否要求 eHSM OTP / counter service customization？
6. NGU key hierarchy 中的 `FW Verify Key / FW_KEK / Debug Auth Seed / Attestation Seed` 如何映射到 eHSM key ID 9-16 或其它官方 key slots？
7. `Image CEK / wrapped CEK` 是否是项目硬需求？如果是，属于 eHSM customization、manifest extension，还是不在首版支持？
8. Manufacturing provisioning 是否必须改写为 eHSM `install_random_key / install_encrypt_key / change_lifecycle / change_control_field` 命令映射？

## 9. 需要 GPT 输出的 CR-0004 内容

建议 GPT / owner 生成正式 CR-0004 时包含：

1. CR status：`draft` 或 `proposal-only`，直到 owner acceptance 前不得写 `accepted/applied`。
2. Source authority：明确 `SRC-002 / OSR_eHSM_Firmware_TRM_1.0 / OSR_eHSM_Bootloader_TRM_1.0` 优先级。
3. Header 裁决：eHSM native header、NGU manifest、eHSM customization 三者取舍。
4. OTP 裁决：physical layout、logical alias、counter、key slot mapping。
5. 不得降低 CR-0001 / CR-0003：SEC1/SEC2 sign+encrypt 仍保持。
6. 受影响文件清单：至少包括 `inputs_manifest.md`、`01_constraints.md`、`02_baseline.md`、`01_boot.md`、`02_key_cert.md`、`06_interface.md`、`07_manufacturing_rma.md`、`10_full_design.md`、`efuse_key_fw_header_design.md`、`mailbox_if.md`、`manufacturing_provisioning.md`、`spdm_report.md`、`05_code_rules.md`、`06_traceability.md`、`design_impact_matrix.md`、`open_questions.md`。
7. 明确哪些项保持 `[TBD]`：per-image rollback、wrapped CEK、eHSM customization、Recovery policy、manifest ABI bit-level layout。

## 10. 为什么已有约束没挡住这个问题

本问题的根因不是没有写“优先 follow eHSM”，而是缺少 source-conformance gate。

### 10.1 Manifest 约束没有转成实现级检查

`inputs_manifest.md` 已经写了 `SRC-002 preferred`，但 `05_code_rules.md` / `06_traceability.md` 没有形成硬规则：

- 禁止创建与 eHSM native header 并列的 physical header。
- 禁止创建与 eHSM OTP layout 并列的 physical OTP 分区。
- 所有 NGU 逻辑字段必须映射到 eHSM field / key ID / counter / command / customization TBD。

### 10.2 “建议”文档被后续同步当成事实源

`efuse_key_fw_header_design.md` 中写的是“分区建议 / 字段建议”，但文件定位是实现级详设。后续 CR-0001 / CR-0003 在同步 SEC1/SEC2 加密策略时，把安全策略补到了这个自定义模型上，没有先问“这个模型是否应该存在”。

### 10.3 eHSM TRM 是 PDF，缺少 repo 内字段级矩阵

eHSM header、control field、counter、key slot 信息在 PDF 表格中。仓库里没有一个可 diff 的 Markdown `eHSM source-conformance matrix`，导致 Codex 和 reviewer 更容易检查仓库内部一致性，而不是逐项回到 TRM 字段。

### 10.4 建议补救 gate

正式 CR-0004 可要求新增：

```text
eHSM source-conformance matrix
```

每个字段分四类：

- `eHSM-native`
- `NGU-logical-alias`
- `manifest-extension`
- `eHSM-customization-TBD`

验收规则：

- 任何 implementation physical field 必须映射到 eHSM TRM 或 owner decision。
- 任何新增字段必须声明是 logical alias、manifest extension，还是 eHSM customization TBD。
- 不得把 Codex 整理出的 proposal 写成 `[CONFIRMED]`。

## 11. 推荐影响文件

| 文件 | 推荐动作 | 原因 |
|---|---|---|
| `change_requests/CR-0004-*.md` | `generate-by-GPT` | 正式 CR 应由 GPT / owner 根据本 pack 生成或确认。 |
| `security_inputs/inputs_manifest.md` | `modify-after-CR` | 增加 header/OTP conflict log；必要时为 eHSM Firmware TRM 单文件建立更细 source 条目。 |
| `security_workflow/01_constraints.md` | `modify-after-CR` | 增加 eHSM native header / OTP physical layout 优先约束。 |
| `security_workflow/02_baseline.md` | `modify-after-CR` | 把 eFuse/key/header baseline 改成 physical + logical mapping。 |
| `security_workflow/03_detailed_design/01_boot.md` | `modify-after-CR` | 区分 RAM deploy / NVM verify；明确 SEC1/SEC2 encrypted path。 |
| `security_workflow/03_detailed_design/02_key_cert.md` | `modify-after-CR` | 增加 NGU key alias 到 eHSM key slot 的映射。 |
| `security_workflow/03_detailed_design/06_interface.md` | `modify-after-CR` | 把 `VERIFY_SEC1 / VERIFY_IMAGE` 表达为 NGU wrapper/profile 与 eHSM command 的关系。 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | `modify-after-CR` | 核心重构文件。 |
| `security_workflow/04_impl_design/mailbox_if.md` | `modify-after-CR` | 对齐 `soc_verify / bl_verify_image / fw_upgrade`。 |
| `security_workflow/04_impl_design/manufacturing_provisioning.md` | `modify-after-CR` | 映射 eHSM install/change commands。 |
| `security_workflow/04_impl_design/spdm_report.md` | `inspect-after-CR` | 若 report 引用 image policy/header 字段，需要改为 manifest policy state。 |
| `security_workflow/05_code_rules.md` | `modify-after-CR` | 增加 source-conformance gate。 |
| `security_workflow/06_traceability.md` | `modify-after-CR` | 增加 header/OTP/key/counter conformance trace。 |
| `05_traceability/design_impact_matrix.md` | `modify-after-CR` | 记录 CR-0004 impact matrix。 |
| `00_project/open_questions.md` | `modify-after-CR` | 登记 per-image rollback、wrapped CEK、eHSM customization、manifest ABI 等开放项。 |

## 12. 给 GPT / owner 的问题

1. 是否确认 eHSM native secure boot image header 是唯一 physical image container？
2. NGU 的 `SEC1 / SEC2 / PM / RAS / Codec / Recovery` metadata 应放入 Code region manifest，还是要求 eHSM header customization？
3. `ngu_image_manifest_t` 是否由 SEC/BootROM 解析，还是需要 eHSM firmware 一并解析？
4. SEC1/SEC2 mandatory encrypt 是否统一要求 RAM deploy / staging buffer / output buffer verify+decrypt path？
5. eHSM `Image_Type` 是否只保留 eHSM key-domain 语义？
6. `SocBootAlg / SocUpgradeAlg` 是否作为算法 authority？manifest 中是否只能记录 expected profile？
7. 当前多个 `*_MIN_VER` 是否全部降级为 logical rollback domain？首版是否只使用 eHSM 128-bit SOC counter？
8. 是否需要 eHSM per-image rollback customization？若需要，谁提供 TRM/RTL/firmware 支撑？
9. NGU key hierarchy 如何映射到 eHSM key ID / level / purpose？
10. per-image CEK / wrapped CEK 是否为首版硬需求？如果是，是否属于 eHSM customization？
11. 是否要求 Codex 在 accepted-apply 前先建立 `eHSM source-conformance matrix`？
12. 对已生成的 Codex draft CR-0004，应丢弃、保留为草案，还是由 GPT 版本覆盖？

## 13. 推荐下一步

建议将本 pack 交给 GPT / owner，让其输出正式 CR-0004：

1. 明确 CR 状态和 owner。
2. 选择 header / manifest / customization 方案。
3. 冻结或保留 OTP/key/counter 映射项。
4. 给出 accepted-apply 的文件范围和禁止修改项。

Codex 后续只根据 GPT / owner 生成或确认的 CR 执行正文同步。

