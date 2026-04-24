# NGU800 SPDM / Attestation Report 实现级设计（Starter v0.1）

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
    uint32_t image_type;
    uint32_t image_version;
    uint8_t  measurement_hash[48];
    uint32_t flags;
} ngu_att_measurement_block_t;
```

建议 measurement 至少覆盖：
- SEC1
- SEC2
- 管理子系统关键固件
- lifecycle/debug 状态

---

# 5. Lifecycle / Debug Status Block

```c
typedef struct {
    uint32_t lifecycle_state;
    uint32_t debug_enable_state;
    uint32_t anti_rollback_state;
    uint32_t secure_boot_state;
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
- Nonce / Session Binding 信息

私钥不得离开 eHSM。

---

# 9. GM / 国际算法映射

| algo_family | hash_algo | sig_algo |
|---|---|---|
| GM | SM3 | SM2 |
| INTL | SHA-256 / SHA-384 | ECDSA / RSA |

---

# 10. 当前阶段结论

本文件已经给出 report 字段级方向。  
后续应结合真实 verifier 需求进一步细化 slot、证书模型和 session 绑定语义。
