# eHSM Source-Conformance Matrix（CR-0004）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级 source gate
适用范围：FW header、OTP/eFuse、control field、version counter、key slot、secure boot command、manufacturing command
主要来源：`CR-0004`、`SRC-006 eHSM Firmware TRM`、`SRC-007 eHSM Bootloader TRM`

---

# 1. 使用规则

任何实现级 physical field 必须满足以下条件之一：

1. 映射到 eHSM TRM / Host API / Bootloader TRM 已定义字段或命令。
2. 映射到 accepted CR / decision log。
3. 明确标记为 `manifest-extension`、`NGU-logical-alias`、`eHSM-customization-TBD` 或 `NGU-SoC-integration-TBD`。

不得把 Codex 草案、旧 NGU 建议字段或未确认推断写成 `[CONFIRMED] physical field`。

---

# 2. Header / Image Format

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| Physical secure boot image header | `ngu_fw_min_hdr_t`, `ngu_fw_signed_hdr_t` | eHSM 1KB Image Head | `eHSM-native` | `SRC-006`, `SRC-007`, `CR-0004` | `[CONFIRMED]` | 旧 NGU struct 不再作为 physical wire/storage format |
| Signature | `sig_off / sig_len` | `Signature` offset 0 size 256 | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 长度由 eHSM algorithm control field 决定 |
| Public key | `cert_off / cert_len`, signer fields | `Public_Key`, `Public_Key_Ext` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | cert chain 是否进入 manifest/report 另行冻结 |
| Encryption IV | `nonce_iv_*` | `Encrypt_IV` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 语义按 eHSM TRM |
| eHSM image type | old `image_type` mixed use | `Image_Type` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 不承载 NGU SEC1/SEC2/runtime 编码 |
| NGU project image type | `IMAGE_TYPE_SEC1/SEC2/...` | NGU manifest `ngu_image_type` | `manifest-extension` | `CR-0004` | `[CONFIRMED]` | release/measurement/attestation 使用 |
| Algorithm fields | `algo_family/hash_algo/sig_algo/enc_algo` | `SocBootAlg/SocUpgradeAlg` or equivalent control field | `eHSM-native` + `manifest-extension` | `SRC-006`, `CR-0004` | `[CONFIRMED]` | eHSM control field 是 authority；manifest 仅 expected profile |
| Code size | `payload_len/ciphertext_len` | `Code_Size` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NGU manifest 可再描述 payload |
| Protected NGU metadata | old signed header fields | Code region manifest | `manifest-extension` | `CR-0004` | `[CONFIRMED] / [TBD ABI]` | manifest ABI 未冻结 |
| per-image wrapped CEK | `wrapped_cek_*` | no confirmed native field | `eHSM-customization-TBD` | `CR-0004` | `[TBD]` | 需 eHSM owner 确认 |

---

# 3. OTP / Control / Counter

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| OTP logical partition | `OTP-0..OTP-7` | eHSM OTP/control/key/counter layout | `NGU-logical-alias` | `CR-0004` | `[CONFIRMED]` | 不表达 physical offset |
| Lifecycle | `LIFECYCLE_STATE` | eHSM lifecycle / hardware lifecycle field | `eHSM-native` / `NGU-SoC-integration-TBD` | `SRC-006`, `SRC-007` | `[CONFIRMED] / [TBD bit]` | exact encoding 需 owner/RTL 确认 |
| Secure boot enable | `SECURE_BOOT_EN` | eHSM / hardware control field | `eHSM-native` / `NGU-SoC-integration-TBD` | `SRC-006`, `CR-0004` | `[CONFIRMED direction]` | exact bit 未冻结 |
| Algorithm select | `algo_family` | `SocBootAlg`, `SocUpgradeAlg`, `EhsmCodeVerifyAlg`, `EhsmCodeUpgradeAlg` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NGU 不覆盖 |
| Anti-rollback counter | `SEC1_MIN_VER`, `SEC2_MIN_VER`, `*_MIN_VER` | eHSM SOC FW / eHSM FW Version Counter | `eHSM-native` + `NGU-logical-alias` | `SRC-006`, `CR-0004` | `[CONFIRMED] / [TBD per-image]` | old fields are logical rollback domains |
| JTAG force disable | `JTAG_FORCE_DISABLE` | SoC/board JTAG MUX + lifecycle/debug auth | `NGU-SoC-integration-TBD` | `CR-0003`, `CR-0004` | `[TBD bit]` | 不写成 eHSM physical OTP bit |
| Attestation enable | `ATTEST_EN` | eHSM / SEC policy | `NGU-logical-alias` / `NGU-SoC-integration-TBD` | `CR-0004` | `[TBD bit]` | report/service policy |

---

# 4. Key Slot / Key Purpose

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| Chip Root | `UDS / Root Secret` | Chip Root Key / secure storage | `eHSM-native` | `SRC-006` | `[CONFIRMED role] / [TBD provisioning flow]` | provisioning flow 待 eHSM/ATE 联调冻结 |
| Device Root | `DRK` | Device Root Key | `eHSM-native` / `NGU-logical-alias` | `SRC-006` | `[CONFIRMED]` | DRK may be semantic only |
| FW verify | `FW Verify Key` | Soc FW Verify Key / Soc Upgrade Verify Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | owner must freeze mapping |
| FW decrypt | `FW Encrypt Key / FW_KEK` | Soc Encrypt Key / Soc Upgrade Encrypt Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | SEC1/SEC2 mandatory use |
| Debug auth | `Debug Auth Seed / Key` | Soc Debug Verify Key / User Auth Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | debug branch independent from attestation |
| Attestation | `Attestation Seed / Device Identity Key` | Soc Private Key / Secret Key / owner mapping | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | attestation model TBD |

---

# 5. Secure Boot / Upgrade Commands

| NGU Profile | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|
| `VERIFY_SEC1` | Bootloader `bl_verify_image` or equivalent ROM path | `eHSM-native` wrapper | `SRC-007`, `CR-0004` | `[ASSUMED]` | exact BootROM callable path TBD |
| `VERIFY_IMAGE(SEC2)` | Firmware `soc_verify` | `eHSM-native` wrapper | `SRC-006`, `CR-0004` | `[CONFIRMED direction]` | NGU wrapper adds manifest policy |
| encrypted SEC1/SEC2 deploy | RAM deploy / output buffer | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NVM only verify not allowed for encrypted images |
| NVM only verify | NVM deploy | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED not for SEC1/SEC2 encrypted]` | image cannot be encrypted |
| FW upgrade | `fw_upgrade` / `bl_fw_upgrade` | `eHSM-native` wrapper | `SRC-006`, `SRC-007` | `[CONFIRMED direction]` | output boot image |

---

# 6. Manufacturing Commands

| NGU Concept | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|
| Key install | `install_random_key`, `install_encrypt_key` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | exact slot mapping TBD |
| Lifecycle change | `change_lifecycle` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | SEC is control plane |
| Control field change | `change_control_field` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | exact bit mapping TBD |
| OTP readback / validation | eHSM status / readback / attested validation | `eHSM-native` / `eHSM-customization-TBD` | `CR-0004` | `[TBD]` | sensitive fields may be non-readable |

---

# 7. Open Items

| Item | Status | Blocking Area |
|---|---|---|
| manifest ABI bit-level layout | `[TBD]` | SEC FW / tools |
| eHSM manifest parser | `[TBD]` | eHSM firmware |
| exact OTP/control bit mapping | `[TBD]` | RTL / eHSM |
| exact key ID mapping | `[TBD]` | eHSM / security owner |
| per-image rollback counter | `[TBD]` | product update policy |
| per-image CEK / wrapped CEK | `[TBD]` | eHSM customization |
| recovery image policy | `[TBD]` | recovery / RMA |
