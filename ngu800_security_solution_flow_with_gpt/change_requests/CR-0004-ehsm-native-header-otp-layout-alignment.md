# CR-0004: eHSM 原生镜像头、OTP/eFuse 与 Key Slot 对齐

Status: accepted-for-application / applied-by-codex / owner-review-pending
Base Commit: `df815e2`
Owner: Security Owner / Architecture Review
Codex Role: accepted-apply；落实已授权方向，未冻结项保持 `[TBD]`
Created: 2026-05-07
Reviewed: 2026-05-07
Accepted: 2026-05-08
Applied: 2026-05-08
Source Context Pack: `.context/CR-0004_ehsm_header_otp_alignment_pack.md`
Primary Topic: eHSM native secure boot image header / OTP layout / version counter / key slot alignment
Change Type: source-conformance correction + implementation design refactor
Risk Level: high

---

## 0. CR 自审结论与应用边界

本文件已由用户在 2026-05-08 授权进入 accepted-apply。应用边界如下：

- P-0001 到 P-0008 作为本次可落实的方案方向。
- 本 CR 明确保留的 `[TBD]` 项不得在正文中升级为 `[CONFIRMED]`。
- exact key ID、exact OTP/control bit、manifest ABI、per-image rollback、per-image CEK / wrapped CEK、eHSM manifest parser 仍保持未冻结。

原 CR 草案中存在一个关键问题：把 eHSM TRM 已确认的事实、Codex 推荐的适配方案、以及仍需 GPT / owner 裁决的架构选择混在一起，并把多项方案判断写成 `[CONFIRMED]`。这不符合当前协作边界：

- GPT / security owner：负责方案设计和 CR 裁决。
- Codex：负责 context pack、来源核对、差异定位、accepted CR 之后的仓库落实。

因此，本修订版只保留官方资料可直接支持的 `[CONFIRMED]` 事实；NGU 适配选择由本次用户授权后进入正文，但在正文中必须显式区分 `eHSM-native`、`NGU-logical-alias`、`manifest-extension`、`eHSM-customization-TBD` 和 `NGU-SoC-integration-TBD`。

## 1. 背景

`security_workflow/04_impl_design/efuse_key_fw_header_design.md` 当前定义了：

- `ngu_fw_min_hdr_t` / `ngu_fw_signed_hdr_t`
- `OTP-0..OTP-7`
- `LIFECYCLE_STATE`
- `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER`
- NGU 自定义 key hierarchy / key slot

这些内容在实现级详设中容易被理解为 NGU 的 physical FW header、physical OTP/eFuse 排布和 physical key slot 模型。

但 `SRC-002 eHSM资料目录级策略` 已要求：eHSM 已明确定义的固件字段、OTP/eFuse 排布、key slot 语义、计数器和生产阶段操作，应优先按 eHSM 定义设计。当前问题不是“是否要 follow eHSM”，而是需要把 eHSM-native 事实、NGU logical alias、manifest extension、eHSM customization TBD 分开。

## 2. 官方来源可确认的事实

### F-0001: eHSM 技术细节优先沿用

```text
[CONFIRMED] inputs_manifest.md 中 `SRC-002` 为 confirmed / preferred；eHSM 已明确定义的固件字段、OTP/eFuse 排布、key slot 语义、计数器和生产阶段操作优先按 eHSM 定义设计。
```

来源：

- `security_inputs/inputs_manifest.md`
- `.context/CR-0004_ehsm_header_otp_alignment_pack.md`

### F-0002: eHSM 已定义 native secure boot image header

```text
[CONFIRMED] eHSM Firmware TRM / Bootloader TRM 已定义 1KB plaintext image head，字段包括 Signature、Public_Key、Encrypt_IV、Valid_Flag、Image_Type、Plain_Flag、Naked_Flag、Reserved、Code_Size、Version_Counter、Public_Key_Ext；Code 从 offset 1024 开始，可明文或密文。
```

设计含义：

- NGU 不应再把 `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t` 写成与 eHSM native header 并列的 physical verification container。
- 但“eHSM native header 是否是唯一 physical image container”仍需 owner 表述得更精确，因为项目可能存在外层传输包、升级包或工具侧中间格式。

### F-0003: eHSM `Image_Type` 已有官方含义

```text
[CONFIRMED] eHSM native header 的 `Image_Type` 已由 TRM 定义，用于区分 eHSM / SOC 镜像及使用 SOC key 还是 eHSM key 的镜像类型；它不是 NGU 的 `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 项目级 image_type。
```

注意：

- Firmware TRM 与 Bootloader TRM 对可见取值范围略有差异，应在 implementation matrix 中按具体命令和阶段分别映射。
- 不能直接把 NGU image type 写入 eHSM `Image_Type`，除非 eHSM owner 明确授权或提供 customization 资料。

### F-0004: 算法 authority 来自 eHSM OTP/control field

```text
[CONFIRMED] eHSM TRM 定义 `EhsmCodeVerifyAlg / EhsmCodeUpgradeAlg / SocBootAlg / SocUpgradeAlg` 等 control field；secure boot / upgrade 验签算法由这些 OTP/control field 决定。
```

设计含义：

- NGU manifest 可以记录 expected profile 用于一致性检查和审计。
- NGU header / manifest 不应覆盖 eHSM control field 成为算法权威。

### F-0005: eHSM 版本计数器不是多个 32-bit counter

```text
[CONFIRMED] eHSM TRM 定义 eHSM FW 和 SOC FW 的 Version Counter；Version Counter 为 16 字节单向计数，支持 128 个版本。
```

设计含义：

- 当前 `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 不能写成 eHSM physical OTP 32-bit counter。
- 是否需要 per-image rollback counter 是 `[TBD]`。

### F-0006: eHSM OTP key layout 已有 key ID / level / purpose

```text
[CONFIRMED] eHSM TRM 定义 OTP key layout，包含 Chip Root Key、Device Root Key、Soc Debug Verify Key、Soc FW Verify Key、Soc Encrypt Key、Soc Upgrade Encrypt Key、Soc Upgrade Verify Key、Soc Private Key、Secret Key / User Auth Key 等 key ID / level / purpose。
```

设计含义：

- NGU 的 `FW Verify Key / FW_KEK / Debug Auth Seed / Attestation Seed / Upgrade Key` 等名称应先映射为 logical alias。
- exact key ID mapping 不能由 Codex 发明，必须由 owner / eHSM 资料确认。

### F-0007: encrypted boot image 不能走 NVM only verify

```text
[CONFIRMED] eHSM TRM 描述 RAM deploy 可执行 verify + decrypt；NVM deploy 是 only verify，并要求镜像不能加密。
```

设计含义：

- CR-0001 / CR-0003 已确认 SEC1 / SEC2 sign + encrypt 时，NVM only verify 不适用。
- SEC1 / SEC2 的具体 early-boot command path 仍需区分 Bootloader `bl_verify_image` 与 Firmware `soc_verify`。

## 3. 拟议设计方向

以下内容不是 accepted decision。owner 接受前，Codex 不得据此修改方案正文。

### P-0001: eHSM native header 作为密码学验证/解密容器

```text
[PROPOSED] 对 SEC1 / SEC2 等安全启动镜像，采用 eHSM native secure boot image header 作为密码学 verify/decrypt container；NGU 不再定义与其并列的 physical verification header。
```

建议表述从“唯一 physical image container”改为“密码学 verify/decrypt container”。原因：

- eHSM TRM 确认 native secure boot image format。
- 但项目仍可能存在外层升级包、传输包、打包工具中间格式。
- 外层格式若存在，不应作为 trust anchor，也不得替代 eHSM header 的安全语义。

### P-0002: NGU metadata 进入 protected manifest

```text
[PROPOSED] NGU 项目级 metadata 不放入自定义 physical header，而放入 eHSM Code region 起始处的 `ngu_image_manifest_t` 或等价 manifest。
```

需要 owner 裁决：

1. manifest 是否必须位于 Code region 起始位置。
2. eHSM 签名/验签工具对 Code region 的覆盖范围和实现细节是否已确认。
3. manifest 由 BootROM / SEC 解析，还是要求 eHSM firmware / bootloader 解析。
4. manifest ABI 是否首版冻结，还是先保持 `[TBD]`。

### P-0003: eHSM `Image_Type` 与 NGU `ngu_image_type` 分层

```text
[PROPOSED] eHSM `Image_Type` 保持 eHSM TRM 定义；NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 等项目级类型放入 manifest 或 policy table。
```

保持 `[TBD]`：

- eHSM firmware 是否需要理解 NGU image type。
- Recovery image 的 signer、anchor、rollback、decrypt policy。
- runtime image 是否全部沿用 eHSM secure boot image format。

### P-0004: SEC1 / SEC2 sign+encrypt 使用 verify+decrypt output path

```text
[PROPOSED] SEC1 / SEC2 sign+encrypt 路径采用 eHSM verify+decrypt 到受控 RAM / staging / output buffer 的模式；NVM only verify 不用于 encrypted SEC1 / SEC2。
```

需要进一步分层：

- SEC1 early boot：优先核对 BootROM 可调用的 eHSM Bootloader `bl_verify_image` 或等价 ROM path。
- SEC2 runtime/load：优先核对 eHSM Firmware `soc_verify` 或 SEC wrapper。
- output buffer 必须受 firewall / address whitelist / DMA policy 保护。

### P-0005: OTP-0..OTP-7 降级为 NGU logical view

```text
[PROPOSED] `OTP-0..OTP-7` 不再表达 physical OTP/eFuse 排布，只作为 NGU logical view / documentation alias。
```

要求：

- eHSM 已有字段标为 `eHSM-native`。
- NGU 项目策略字段标为 `NGU-logical-alias` 或 `manifest-extension`。
- eHSM 无承载依据的字段标为 `eHSM-customization-TBD` 或 `NGU SoC integration field`。

### P-0006: rollback 先对齐 eHSM counter，per-image counter 保持 TBD

```text
[PROPOSED] 当前多个 `*_MIN_VER` 降级为 NGU logical rollback domain；物理 anti-rollback 先映射到 eHSM SOC FW Version Counter 或 owner 确认的等价机制。
```

不能直接确认：

- “V1 物理 anti-rollback 一定只使用 eHSM SOC FW counter”。
- “SEC1 / SEC2 / PM / RAS / Codec 独立 counter 已存在”。

### P-0007: key hierarchy 改为 logical alias + eHSM key ID mapping

```text
[PROPOSED] NGU key hierarchy 中的 key name 是逻辑别名；具体物理 key slot 必须映射到 eHSM key ID / level / purpose 或 owner 确认的 customization。
```

保持 `[TBD]`：

- Attestation key / seed 与 eHSM `Soc Private Key / Secret Key / User Auth Key` 的关系。
- Debug Auth Seed exact key ID。
- per-image CEK / wrapped CEK 是否为首版硬需求。
- secure boot image 是否支持 per-image wrapped CEK 扩展。

### P-0008: 建立 eHSM source-conformance matrix

```text
[PROPOSED] 新增 `security_workflow/04_impl_design/ehsm_source_conformance_matrix.md`，作为 header / OTP / key / counter / command 的 source gate。
```

matrix 至少包含：

| 字段 | 说明 |
|---|---|
| NGU Concept | NGU 逻辑概念 |
| Current Document Field | 当前文档字段名 |
| eHSM Native Field / Command | eHSM 对应字段或命令 |
| Mapping Type | `eHSM-native` / `NGU-logical-alias` / `manifest-extension` / `eHSM-customization-TBD` / `NGU-SoC-integration-TBD` |
| Source | TRM / manifest / decision log / accepted CR |
| Status | `[CONFIRMED]` / `[PROPOSED]` / `[ASSUMED]` / `[TBD]` |
| Notes | 差异说明、开放问题 |

## 4. 原草案中需要纠正的问题

| 问题 | 原草案位置 | 修正意见 |
|---|---|---|
| Context pack 路径错误 | 元数据中的 `Source Context Pack` 指向不存在的临时路径 | 改为 `.context/CR-0004_ehsm_header_otp_alignment_pack.md` |
| draft CR 却大量使用 `[CONFIRMED]` | D-0016 到 D-0025 | 除官方来源事实外，设计选择降级为 `[PROPOSED]` / `[TBD]` |
| “唯一 physical image container”表述过强 | D-0016 | 改为 eHSM native header 是密码学 verify/decrypt container；外层传输/升级包另行界定 |
| manifest 位置和存在被写成 confirmed | D-0017 | 改为 `[PROPOSED]`，需确认签名覆盖、解析主体、ABI |
| eHSM `Image_Type` 被抽象成 key-domain 但未列明具体取值差异 | D-0018 | 按 Firmware / Bootloader / command stage 分开映射 |
| SEC1/SEC2 RAM deploy 被直接 confirmed | D-0020 | NVM only verify 不适用于加密是 confirmed；具体 SEC1/SEC2 command path 是 proposed/TBD |
| V1 rollback 使用 SOC FW counter 被直接 confirmed | D-0023 | 改为 proposed；per-image counter 保持 TBD |
| key hierarchy exact mapping 不足 | D-0024 | 只能确认 eHSM key layout 存在；exact NGU key alias mapping 仍 TBD |
| 把执行 prompt 写入 CR 正文 | Section 10 / 11 | 删除；CR 只保留设计裁决和验收边界 |

## 5. 不得回退的既有裁决

本 CR 不得降低 CR-0001 / CR-0003 已有结论：

1. `[CONFIRMED]` SEC1 必须 sign + encrypt。
2. `[CONFIRMED]` SEC2 必须 sign + encrypt。
3. `[CONFIRMED]` BootROM 不直接实现复杂 crypto。
4. `[CONFIRMED]` eHSM 是 Root of Trust / First Cryptographic Verifier。
5. `[CONFIRMED]` Host 不下发 SEC1，不进入信任链。
6. `[CONFIRMED]` Board / OOB / BMC 不进入 Root of Trust。
7. `[CONFIRMED]` USER / PROD JTAG 默认关闭。
8. `[CONFIRMED]` DMA 对安全资源默认拒绝。
9. `[CONFIRMED]` OOB / BMC 不得接触 root secret、device private key、FW_KEK 明文。

## 6. 保持 TBD 的问题

以下问题不在本 CR 草案中冻结：

1. `ngu_image_manifest_t` bit-level ABI。
2. eHSM firmware / bootloader 是否解析 NGU manifest。
3. eHSM reserved/custom field 是否可用。
4. per-image rollback counter。
5. per-image CEK / wrapped CEK。
6. recovery image policy。
7. board binding 是否参与 release decision。
8. dual-die / board-die report 汇总方式。
9. JTAG scope bit-level mapping。
10. DMA / firewall / UserID 具体地址表。
11. exact OTP/control bit mapping。
12. exact NGU key alias to eHSM key ID mapping。
13. RMA re-acceptance 细节。
14. full X.509 cert chain provisioning。
15. 国密 / 国际算法默认 SKU 选择。

## 7. 建议影响文件

owner 接受本 CR 后，Codex 可进入 accepted-apply，并至少检查 / 修改：

| 文件 | 动作 |
|---|---|
| `security_inputs/inputs_manifest.md` | 增加 header / OTP conflict log；必要时增加 eHSM TRM 单文件 source entry |
| `security_workflow/01_constraints.md` | 增加 eHSM native header / OTP / key / counter source-conformance 约束 |
| `security_workflow/02_baseline.md` | 区分 eHSM physical field、NGU logical alias、manifest extension、customization TBD |
| `security_workflow/03_detailed_design/01_boot.md` | 区分 SEC1 early boot 和 SEC2 runtime 的 verify/decrypt path |
| `security_workflow/03_detailed_design/02_key_cert.md` | 建立 NGU key alias 到 eHSM key ID / purpose 的映射框架 |
| `security_workflow/03_detailed_design/06_interface.md` | 将 SEC wrapper 命令映射到 eHSM command / profile |
| `security_workflow/03_detailed_design/07_manufacturing_rma.md` | 改为 eHSM install / lifecycle / control field 命令映射 |
| `security_workflow/03_detailed_design/10_full_design.md` | 同步 accepted 裁决，不升级 reviewed baseline |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | 核心重构 |
| `security_workflow/04_impl_design/ehsm_source_conformance_matrix.md` | 新增 source-conformance matrix |
| `security_workflow/04_impl_design/mailbox_if.md` | 对齐 `bl_verify_image` / `soc_verify` / `fw_upgrade` 或项目 wrapper |
| `security_workflow/04_impl_design/manufacturing_provisioning.md` | 对齐 eHSM provisioning commands |
| `security_workflow/04_impl_design/spdm_report.md` | 若引用 image/header policy，改为 manifest / measurement state |
| `security_workflow/05_code_rules.md` | 增加 source-conformance gate |
| `security_workflow/06_traceability.md` | 增加 header / OTP / key / counter trace |
| `05_traceability/design_impact_matrix.md` | 增加 CR-0004 影响矩阵 |
| `00_project/open_questions.md` | 登记本 CR 保留 TBD |
| `00_project/decision_log.md` / `00_project/changelog.md` | owner 接受后记录 |

## 8. Owner 需要裁决的问题

1. 是否接受 P-0001：eHSM native header 作为 SEC1 / SEC2 密码学 verify/decrypt container。
2. 是否接受 P-0002：NGU metadata 放入 Code region protected manifest。
3. manifest 是否必须位于 Code region 起始位置。
4. manifest 由 BootROM / SEC 解析，还是 eHSM firmware / bootloader 也需要解析。
5. SEC1 early boot 使用 Bootloader `bl_verify_image`、Firmware `soc_verify`，还是项目自定义 wrapper。
6. SEC2 runtime/load 使用 Firmware `soc_verify` 还是项目自定义 wrapper。
7. `OTP-0..OTP-7` 是否只保留为 NGU logical view。
8. 当前多个 `*_MIN_VER` 是否全部降级为 logical rollback domain。
9. V1 是否接受单 SOC FW Version Counter，还是要求 per-image rollback customization。
10. NGU `FW Verify Key / FW_KEK / Debug Auth Seed / Attestation Seed` 如何映射到 eHSM key ID。
11. per-image CEK / wrapped CEK 是否为首版硬需求。
12. 是否接受新增 `ehsm_source_conformance_matrix.md` 作为 accepted-apply 前置 gate。

## 9. Accepted-Apply Gate

Codex 只有在以下条件满足后才能修改方案正文：

1. Owner 明确接受本 CR，或给出等价的 GPT / owner 正式 CR。
2. Owner 明确哪些 P-000x 进入 `[CONFIRMED]`，哪些保持 `[TBD]`。
3. SEC1 / SEC2 command path 至少有可追溯的阶段级裁决。
4. exact key ID / OTP bit mapping 若未冻结，必须保持 TBD，不得在正文中伪装为 physical fact。

在 accepted-apply 前，Codex 只能继续做：

- source-conformance matrix 草案；
- impact analysis；
- open question 整理；
- proposed wording；
- TRM 证据抽取。
