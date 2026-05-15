# NGU800 SPDM / Attestation Report 实现级设计（Starter v0.1）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级设计起始稿
目标：定义设备证明报告的字段级结构和签名覆盖范围

---

# 1. 设计目标

本文件用于收敛：

- 设备身份字段
- report header
- measurement block
- lifecycle/debug block
- cert chain block
- signature block
- nonce/session 绑定关系

---

# 2. Report Header 建议

`algo_family / hash_algo / sig_algo` 在 report 中表示证明报告自身的签名 / hash profile，不是 secure boot / upgrade 的算法 authority。secure boot / upgrade 算法选择仍以 eHSM `SocBootAlg / SocUpgradeAlg` 或等价 control field 为准。

```c
typedef struct {
    uint16_t report_version;
    uint16_t algo_family;
    uint16_t hash_algo;
    uint16_t sig_algo;
    uint32_t report_len;
    uint32_t nonce_len;
    uint32_t session_binding_flags;
    uint32_t measurement_count;
    uint32_t lifecycle_state;
    uint32_t debug_state;
    uint32_t secure_boot_state;
    uint32_t image_confidentiality_policy;
    uint32_t rollback_state;
    uint32_t board_bind_result;
    uint32_t event_log_policy;
} ngu_att_report_hdr_t;
```

---

# 3. Identity Block

```c
typedef struct {
    uint8_t  device_id[32];
    uint8_t  die_id[32];
    uint8_t  board_id_hash[32];
    uint8_t  signer_id[32];
} ngu_att_identity_block_t;
```

---

# 4. Measurement Block

```c
typedef struct {
    uint32_t slot_id;
    uint32_t ehsm_image_type;
    uint32_t ngu_image_type;
    uint32_t image_version;
    uint32_t ehsm_version_counter_checked;
    uint32_t ngu_rollback_domain;
    uint32_t rollback_checked;
    uint32_t decrypt_applied;
    uint32_t image_policy_state;
    uint8_t  measurement_hash[48];
    uint32_t flags;
} ngu_att_measurement_block_t;
```

建议 measurement 至少覆盖：
- SEC1
- SEC2
- PM / RAS / Codec 等关键 runtime image
- lifecycle/debug 状态

SEC1 / SEC2 对应 measurement 必须反映 verify + decrypt 成功后的受控镜像状态，不能只记录未解密包体存在性。`flags` 至少需要能表达 `SIGN_VERIFIED`、`DECRYPT_APPLIED`、`ROLLBACK_CHECKED`、`POLICY_MATCHED` 和 `SIGNATURE_ONLY_EXCEPTION` 等语义。

字段语义：
- `ehsm_image_type` 来自 eHSM native header，保持 eHSM TRM 定义。
- `ngu_image_type` 来自 NGU protected manifest / policy table，用于 SEC1 / SEC2 / PM / RAS / Codec / Recovery 等项目级证明语义。
- `ehsm_version_counter_checked` 与 `ngu_rollback_domain` 分别表达物理计数器检查结果和 NGU 逻辑 rollback domain，不得把 `*_MIN_VER` 当成 physical OTP counter。

---

# 5. Lifecycle / Debug Status Block

```c
typedef struct {
    uint32_t lifecycle_state;
    uint32_t debug_enable_state;
    uint32_t anti_rollback_state;
    uint32_t secure_boot_state;
    uint32_t rollback_state;
    uint32_t image_confidentiality_policy; /* 至少表达 SEC1/SEC2 强制签名 + 加密策略 */
    uint32_t board_bind_result;            /* [ASSUMED] board binding 默认进入证明 */
} ngu_att_lifecycle_block_t;
```

---

# 6. Certificate Chain Block

```c
typedef struct {
    uint32_t cert_format;
    uint32_t cert_chain_len;
    uint32_t cert_chain_off;
} ngu_att_cert_chain_block_t;
```

---

# 7. Signature Block

```c
typedef struct {
    uint32_t sig_format;
    uint32_t sig_len;
    uint32_t sig_off;
} ngu_att_sig_block_t;
```

---

# 8. 签名覆盖范围

必须覆盖：
- Report Header
- Identity Block
- Measurement Blocks
- Lifecycle/Debug Block
- Secure boot / image protection policy fields
- Rollback state
- Board binding result if present
- Nonce / Session Binding 信息

私钥不得离开 eHSM。

字段状态：
- `[CONFIRMED]` measurement、lifecycle、debug_state、secure_boot_state、rollback_state 必须被签名覆盖。
- `[ASSUMED]` image protection policy、decrypt_applied、`ngu_image_type` policy 和 board_bind_result 进入 report 或 measurement flags。
- `[TBD]` PowerBrake / PG / FAULT / reset event 进入主 report 还是扩展 event log。

---

# 9. GM / 国际算法映射

| algo_family | hash_algo | sig_algo |
|---|---|---|
| GM | SM3 | SM2 |
| INTL | SHA-256 / SHA-384 | ECDSA / RSA |

---

# 10. 当前阶段结论

本文件已经给出 report 字段级方向。
后续应结合真实 verifier 需求进一步细化 slot、证书模型、image protection policy / decrypt_applied / `ehsm_image_type` / `ngu_image_type` / board_bind_result 编码、event log 策略和 session 绑定语义。
