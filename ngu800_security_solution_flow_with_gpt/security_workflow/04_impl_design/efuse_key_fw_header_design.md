# NGU800 eFuse / Key / FW Header 详细设计（Draft v1.0）

状态：详细设计草案（用于安全方案收敛、RTL/Boot/SEC/eHSM接口对齐）  
适用范围：NGU800 / NGU800P 安全启动、密钥体系、反回滚、制造灌装  

---

# 1. 设计目标

本文档收敛以下三类必须冻结的设计对象：

1. eFuse / OTP 字段规划
2. 密钥层级（Key Hierarchy / Key Ladder）
3. 固件镜像头（FW Header）与签名/加密封装格式

---

# 2. 设计原则

## 2.1 复用优先原则

优先复用 eHSM 已定义能力与字段语义，特别是：

- OTP / 生命周期 / 控制字段 / 版本计数器
- Bootloader / Firmware 的镜像验证能力
- Mailbox 命令模型
- 调试鉴权与 challenge-response 机制

## 2.2 单控制器原则

按当前项目基线，安全控制中心只有一个：

- 控制面：SEC 核 / C908
- 安全面：eHSM

## 2.3 双算法栈原则

方案必须同时支持：

- 国密栈：SM2 / SM3 / SM4
- 国际栈：ECDSA/RSA / SHA-2/3 / AES

---

# 3. eFuse / OTP 分区建议

| 区域编号 | 区域名称 | 访问主体 | 用途 | 安全等级 |
|---|---|---|---|---|
| OTP-0 | 生命周期区 | eHSM only | lifecycle 编码 / 锁定位 | 高 |
| OTP-1 | 控制字段区 | eHSM only | secure boot / debug / algo 配置 | 高 |
| OTP-2 | Root Key 材料区 | eHSM only | Root Secret / Root Key / UDS | 最高 |
| OTP-3 | 公钥摘要区 | eHSM only | FW signer hash / cert anchor hash | 高 |
| OTP-4 | 版本计数区 | eHSM only | anti-rollback monotonic counter | 高 |
| OTP-5 | 设备身份区 | eHSM only | UID / Device Identity Seed | 高 |
| OTP-6 | 板级 / 芯片参数区 | SEC/eHSM | 主从Die / board binding | 中 |
| OTP-7 | 非安全只读配置区 | 管理子系统可读 | tempsensor / board config | 低 |

---

# 4. eFuse 字段建议

## 4.1 生命周期区

| 字段名 | 位宽建议 | 描述 |
|---|---:|---|
| LIFECYCLE_STATE | 8 | TEST / DEVE / MANU / USER / DEBUG / DEST |
| LIFECYCLE_LOCK | 1 | 生命周期写锁 |
| DESTROY_DONE | 1 | 安全擦除完成标记 |

## 4.2 控制字段区

| 字段名 | 位宽建议 | 描述 |
|---|---:|---|
| SECURE_BOOT_EN | 1 | 启用安全启动 |
| DEBUG_AUTH_EN | 1 | 调试鉴权使能 |
| JTAG_FORCE_DISABLE | 1 | 强制禁用 JTAG |
| FW_ENCRYPT_EN | 1 | 启用镜像加密 |
| ATTEST_EN | 1 | 启用设备证明 |
| ANTI_ROLLBACK_EN | 1 | 启用反回滚 |

## 4.3 版本计数区

| 字段名 | 位宽建议 | 描述 |
|---|---:|---|
| SEC1_MIN_VER | 32 | SEC1 最低允许版本 |
| SEC2_MIN_VER | 32 | SEC2 最低允许版本 |
| PMP_MIN_VER | 32 | PMP 最低允许版本 |
| RMP_MIN_VER | 32 | RMP 最低允许版本 |
| OMP_MIN_VER | 32 | OMP 最低允许版本 |
| MMP_MIN_VER | 32 | MMP 最低允许版本 |

---

# 5. 密钥层级建议

```text
UDS / Root Secret (OTP/eFuse)
    ↓ KDF
Device Root Key (DRK)
    ↓───────────────┬───────────────────┬───────────────────┐
    ↓               ↓                   ↓                   ↓
FW Verify Key    FW Encrypt Key      Attestation Seed     Debug Auth Seed
```

## 5.1 职责说明

| 密钥层 | 用途 | 是否可导出 | 存储位置 |
|---|---|---|---|
| UDS / Root Secret | 根种子 | 否 | OTP/eFuse |
| DRK | 根派生密钥 | 否 | eHSM 内部 |
| FW Verify Key | 固件验签根 | 否（私钥） | eHSM / cert chain |
| FW Encrypt Key | 固件解密包裹密钥 | 否 | eHSM |
| Attestation Seed | 设备身份派生根 | 否 | eHSM |
| Debug Auth Seed | 调试鉴权 | 否 | eHSM |

---

# 6. FW Header 设计

## 6.1 Minimal Header（非签名区）

```c
typedef struct {
    uint32_t magic;
    uint16_t header_version;
    uint16_t total_header_len;
    uint32_t image_total_len;
    uint32_t signed_region_off;
    uint32_t signed_region_len;
    uint32_t payload_off;
    uint32_t sig_block_off;
} ngu_fw_min_hdr_t;
```

## 6.2 Signed Header（签名区）

```c
typedef struct {
    uint32_t image_type;
    uint32_t image_version;
    uint32_t min_rollback_ver;
    uint32_t load_addr;
    uint32_t entry_point;
    uint32_t payload_len;
    uint32_t payload_flags;
    uint16_t algo_family;
    uint16_t hash_algo;
    uint16_t sig_algo;
    uint16_t enc_algo;
    uint16_t key_slot;
    uint16_t cert_format;
    uint32_t cert_off;
    uint32_t cert_len;
    uint32_t sig_off;
    uint32_t sig_len;
    uint32_t lifecycle_mask;
    uint32_t board_bind_flags;
    uint8_t  signer_key_hash[32];
    uint8_t  reserved[64];
} ngu_fw_signed_hdr_t;
```

## 6.3 关键规则

- `load_addr / entry_point / image_version / algo_family` 必须进入 signed region
- rollback 字段必须进入 signed region
- 私钥不得参与 Host 可见路径

---

# 7. 制造 / 灌装要求

- Root Key / UDS 材料必须通过制造安全通道写入
- 写入后必须锁定
- MANU → USER 前必须清理测试 key / 测试 trust
- 进入 USER 前必须开启 secure boot / anti-rollback / debug lock

---

# 8. 结论

当前实现级设计已经给出：
- eFuse / OTP 分区与字段方向
- key hierarchy / key ladder 方向
- FW header 结构方向

后续可再拆分为：
- efuse_design.md
- key_hierarchy.md
- fw_header.md
