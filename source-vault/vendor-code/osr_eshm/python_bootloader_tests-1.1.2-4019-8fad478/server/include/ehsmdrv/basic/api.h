#ifndef EHSM_BASE_API_H
#define EHSM_BASE_API_H

#include "types.h"
#include "key_assist.h"
#include <stdbool.h>

/**
 * @defgroup fw_api FW (Firmware) 相关的API接口
 * @{
 */

/**
 * @brief 为eHSM的功能调用提供上下文环境。
 */
typedef struct {
    uint32_t _data[56]; /**< 内部维护数据 */
} ehsm_ctx_st;

/**
 * @brief 为eHSM的三段式(init/update/finish)算法计算提供存储空间，此内存空间应当在
 * init/update/finish 调用期间保持有效状态。
 */
typedef struct {
    uint32_t _data[512 / 4]; /**< 内部维护数据 */
} ehsm_session_st;

#pragma pack(1)

/** @defgroup key-format-struct 密钥导入/导入数据格式结构
 * @{
 */

/**
 * @brief 导入/导出密钥数据结构体的头部。
 * @note
 * 结构体中size字段无法表示实际密钥长度，`key_value`长度为0表示后续的密钥值根据算法不同，其具体格式和长度是可变的，实际
 * 长度由 `pub_key_size` 和 `priv_key_size` 之和来表示。如果是对称密钥，则 `pub_key_size` 为 0，密钥长度由
 * `priv_key_size` 指示。关于密钥值的具体格式，见 @ref key-format 。实现使用时，可使用其它辅助结构体，见 @ref
 * key-format-struct 。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 密钥包含的数据信息，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥数据长度，如果是RSA，这里是E的长度（必须4字节对齐），N 的长度由密钥类型推导 */
    uint16_t priv_key_size; /**< 私钥数据或对称密钥数据长度，如果是RSA，这里是D的长度，N
                               的长度由密钥类型推导；如果是RSA-CRT，这里是私钥5分量的总长度 */
    uint8_t key_value[0];   /**< 密钥值，对称密钥和非对称密钥的结构见描述 @ref key-format */
} ehsm_key_format_st;

/** @} */

/**
 * @brief SM2密钥交换算法相关的参数。
 */
typedef struct {
    uint8_t sm2_role;                       /**< SM2密钥交换角色，见 @ref ehsm_sm2_role_e */
    uint8_t reserved0[3];                   /**< 保留字段 */
    uint32_t local_tmp_key_handle;          /**< SM2密钥交换本地临时密钥句柄 */
    EHSM_SHM raddr_t s1_s2_value_addr;      /**< SM2密钥交换 s1_s2 值的主机地址 */
    EHSM_SHM raddr_t sa_sb_value_addr;      /**< SM2密钥交换 sa_sb 值的主机地址 */
    EHSM_SHM raddr_t peer_temp_pubkey_addr; /**< SM2密钥交换对方临时公钥的主机地址 */
    uint32_t peer_temp_pubkey_size;         /**< 对方临时公钥的长度 */
} ehsm_sm2_params_st;

/**
 * @brief Bootloader/Firmware的版本号构体。
 */
typedef struct {
    uint8_t type;               /**< 固件类型，0：Bootloader，1：固件 */
    uint8_t ver_major;          /**< 主版本号 */
    uint8_t ver_minor;          /**< 次版本号 */
    uint8_t ver_patch;          /**< 修订版本号 */
    uint8_t ver_pre_release[8]; /**< 预发布版本字符串，如 `alpha.1`、`rc3`；正式版本则为空字符串 */
    uint8_t reserved0[8];       /**< 保留 */
    uint32_t pke_engine_ver;    /**< PKE 引擎版本号 */
    uint32_t pke_lib_ver;       /**< PKE 库版本号。格式：0x23080301 表示 2023-08-03 版本 1 */
    uint32_t ske_engine_ver;    /**< SKE 引擎版本号 */
    uint32_t ske_lib_ver;       /**< SKE 库版本号。格式同上 */
    uint32_t hash_engine_ver;   /**< HASH 引擎版本号 */
    uint32_t hash_lib_ver;      /**< HASH 库版本号。格式同上 */
    uint32_t trng_engine_ver;   /**< TRNG 引擎版本号 */
    uint32_t trng_lib_ver;      /**< TRNG 库版本号。格式同上 */
    uint32_t hw_ver;            /**< 硬件版本号，详见 OSR eHSM TRM 手册中的 SYS_VER1 */
    uint8_t uid[16];            /**< 从 OTP 读取的 16 字节 UID */
    uint8_t reserved1[56];      /**< 保留 */
} ehsm_version_st;

// #% #if CONFIG_BL_INJECT_ERR_ENABLE
/**
 * @brief 注错接口的错误信号数据。
 */
typedef struct {
    uint32_t fw_trig[2];    /**< 这两个word值会设置到 EMU ERR_FW_0 和 ERR_FW_1 */
    uint32_t hw_trig[2];    /**< 这两个word值会设置到 EMU ERR_HW_TRIG_0 和 ERR_HW_TRIG_1 */
    uint32_t alarm_trig[2]; /**< 这两个word值会设置到 EMU FUSA_ALARM_TRIG_0 和 FUSA_ALARM_TRIG_1. */
} ehsm_inject_error_st;
// #% #endif // CONFIG_BL_INJECT_ERR_ENABLE

/**
 * @brief SOC调试开启/关闭对应的使能位图。
 */
typedef struct {
    uint32_t
        bitmaps[5]; /**< 前4个word，每个bit对应128个SOC调试端口中的一个；最后一个word的最低位对应第129个SOC调试端口 */
} ehsm_soc_dbg_bitmap_st;

/**
 * @brief 算法自检结果。
 */
typedef struct {
    uint32_t result; /**< 指示哪些算法测试通过，bit为1的对应算法测试通过（mask中未测试的算法没有结果）*/
    uint32_t mask; /**< 指示哪些算法被测试，bit为1的对应算法被测试，见 @ref self-test-algo */
} ehsm_self_test_result_st;

/**
 * @brief 1024位DH参数，参数值以大端形式存储在value数组中
 */
typedef struct {
    uint32_t p_len;       /**< 参数P的长度，应当固定为128 */
    uint8_t p_value[128]; /**< 参数P的值 */
    uint32_t q_len;       /**< 参数Q的长度，应当固定为128 */
    uint8_t q_value[128]; /**< 参数Q的值 */
    uint32_t g_len;       /**< 参数G的长度，应当固定为128 */
    uint8_t g_value[128]; /**< 参数G的值 */
    uint32_t h_len; /**< 参数H的长度，此参数用于加速计算，可以设为0，不提供此参数，或提供时应当固定为128 */
    uint8_t h_value[128]; /**< 参数H的值 */
} ehsm_dh_params_1024_st;

/**
 * @brief 2048位DH参数，参数值以大端形式存储在value数组中
 */
typedef struct {
    uint32_t p_len;       /**< 参数P的长度，应当固定为256 */
    uint8_t p_value[256]; /**< 参数P的值 */
    uint32_t q_len;       /**< 参数Q的长度，应当固定为256 */
    uint8_t q_value[256]; /**< 参数Q的值 */
    uint32_t g_len;       /**< 参数G的长度，应当固定为256 */
    uint8_t g_value[256]; /**< 参数G的值 */
    uint32_t h_len; /**< 参数H的长度，此参数用于加速计算，可以设为0，不提供此参数，或提供时应当固定为256 */
    uint8_t h_value[256]; /**< 参数H的值 */
} ehsm_dh_params_2048_st;

/**
 * @brief 3072位DH参数，参数值以大端形式存储在value数组中
 */
typedef struct {
    uint32_t p_len;       /**< 参数P的长度，应当固定为384 */
    uint8_t p_value[384]; /**< 参数P的值 */
    uint32_t q_len;       /**< 参数Q的长度，应当固定为384 */
    uint8_t q_value[384]; /**< 参数Q的值 */
    uint32_t g_len;       /**< 参数G的长度，应当固定为384 */
    uint8_t g_value[384]; /**< 参数G的值 */
    uint32_t h_len; /**< 参数H的长度，此参数用于加速计算，可以设为0，不提供此参数，或提供时应当固定为384 */
    uint8_t h_value[384]; /**< 参数H的值 */
} ehsm_dh_params_3072_st;

/**
 * @brief 4096位DH参数，参数值以大端形式存储在value数组中
 */
typedef struct {
    uint32_t p_len;       /**< 参数P的长度，应当固定为512 */
    uint8_t p_value[512]; /**< 参数P的值 */
    uint32_t q_len;       /**< 参数Q的长度，应当固定为512 */
    uint8_t q_value[512]; /**< 参数Q的值 */
    uint32_t g_len;       /**< 参数G的长度，应当固定为512 */
    uint8_t g_value[512]; /**< 参数G的值 */
    uint32_t h_len; /**< 参数H的长度，此参数用于加速计算，可以设为0，不提供此参数，或提供时应当固定为512 */
    uint8_t h_value[512]; /**< 参数H的值 */
} ehsm_dh_params_4096_st;

typedef enum {
    EHSM_HASH_ALGO_SM3 = 0,        ///< SM3 哈希算法
    EHSM_HASH_ALGO_MD5 = 1,        ///< MD5 哈希算法
    EHSM_HASH_ALGO_SHA256 = 2,     ///< SHA-256 哈希算法
    EHSM_HASH_ALGO_SHA384 = 3,     ///< SHA-384 哈希算法
    EHSM_HASH_ALGO_SHA512 = 4,     ///< SHA-512 哈希算法
    EHSM_HASH_ALGO_SHA1 = 5,       ///< SHA-1 哈希算法
    EHSM_HASH_ALGO_SHA224 = 6,     ///< SHA-224 哈希算法
    EHSM_HASH_ALGO_SHA512_224 = 7, ///< SHA-512/224 哈希算法
    EHSM_HASH_ALGO_SHA512_256 = 8, ///< SHA-512/256 哈希算法
    EHSM_HASH_ALGO_SHA3_224 = 9,   ///< SHA3-224 哈希算法
    EHSM_HASH_ALGO_SHA3_256 = 10,  ///< SHA3-256 哈希算法
    EHSM_HASH_ALGO_SHA3_384 = 11,  ///< SHA3-384 哈希算法
    EHSM_HASH_ALGO_SHA3_512 = 12,  ///< SHA3-512 哈希算法
} ehsm_hash_algo_e;

typedef enum {
    EHSM_RNG_ALGO_SM4_CTR_DRBG = 0, ///< 基于 SM4-CTR 的确定性随机位生成器
    EHSM_RNG_ALGO_AES_CTR_DRBG = 1, ///< 基于 AES-CTR 的确定性随机位生成器
} ehsm_rng_algo_e;

typedef enum {
    EHSM_SYMM_ALGO_DES = 0,      ///< DES 对称加密算法
    EHSM_SYMM_ALGO_TDES_128 = 1, ///< 3DES 128位对称加密算法
    EHSM_SYMM_ALGO_TDES_192 = 2, ///< 3DES 192位对称加密算法
    EHSM_SYMM_ALGO_AES_128 = 5,  ///< AES-128 对称加密算法
    EHSM_SYMM_ALGO_AES_192 = 6,  ///< AES-192 对称加密算法
    EHSM_SYMM_ALGO_AES_256 = 7,  ///< AES-256 对称加密算法
    EHSM_SYMM_ALGO_SM4 = 8,      ///< SM4 对称加密算法
} ehsm_symm_algo_e;

typedef enum {
    EHSM_MAC_MODE_CMAC = 7,    ///< CMAC 消息认证码模式
    EHSM_MAC_MODE_CBC_MAC = 8, ///< CBC-MAC 消息认证码模式
    EHSM_MAC_MODE_GMAC = 9,    ///< GMAC 消息认证码模式
} ehsm_mac_mode_e;

typedef enum {
    EHSM_PADDING_NONE = 0,
    EHSM_PADDING_PKCS7 = 2,          /**< PKCS7，兼容PKCS5 */
    EHSM_PADDING_ONE_WITH_ZEROS = 3, /**< ISO7816-4*/
} ehsm_padding_mode_e;

typedef enum {
    EHSM_RSA_PADDING_NONE = 0,
    EHSM_RSA_PADDING_PSS = 1,
} ehsm_rsa_padding_mode_e;

typedef enum {
    EHSM_CIPHER_MODE_ECB = 1, ///< ECB（电子密码本）模式
    EHSM_CIPHER_MODE_XTS = 2, ///< XTS（可调整密文块链接）模式
    EHSM_CIPHER_MODE_CBC = 3, ///< CBC（密码块链接）模式
    EHSM_CIPHER_MODE_CFB = 4, ///< CFB（密码反馈）模式
    EHSM_CIPHER_MODE_OFB = 5, ///< OFB（输出反馈）模式
    EHSM_CIPHER_MODE_CTR = 6, ///< CTR（计数器）模式
} ehsm_cipher_mode_e;

typedef enum {
    EHSM_AEAD_MODE_GCM = 0, ///< GCM（伽罗瓦计数器）认证加密模式
    EHSM_AEAD_MODE_CCM = 1, ///< CCM（计数器与CBC-MAC）认证加密模式
} ehsm_aead_mode_e;

typedef enum {
    EHSM_KEY_TYPE_DES = 0x01,                  ///< DES 密钥
    EHSM_KEY_TYPE_TDES_128 = 0x02,             ///< 3DES 128位密钥
    EHSM_KEY_TYPE_TDES_192 = 0x03,             ///< 3DES 192位密钥
    EHSM_KEY_TYPE_AES_128 = 0x04,              ///< AES-128 密钥
    EHSM_KEY_TYPE_AES_192 = 0x05,              ///< AES-192 密钥
    EHSM_KEY_TYPE_AES_256 = 0x06,              ///< AES-256 密钥
    EHSM_KEY_TYPE_SM4 = 0x07,                  ///< SM4 密钥
    EHSM_KEY_TYPE_SM2 = 0x08,                  ///< SM2 密钥
    EHSM_KEY_TYPE_RSA_1024 = 0x09,             ///< RSA-1024 密钥
    EHSM_KEY_TYPE_RSA_2048 = 0x0a,             ///< RSA-2048 密钥
    EHSM_KEY_TYPE_RSA_3072 = 0x0b,             ///< RSA-3072 密钥
    EHSM_KEY_TYPE_RSA_4096 = 0x0c,             ///< RSA-4096 密钥
    EHSM_KEY_TYPE_RSA_1024_CRT = 0x0d,         ///< RSA-1024 CRT 格式密钥
    EHSM_KEY_TYPE_RSA_2048_CRT = 0x0e,         ///< RSA-2048 CRT 格式密钥
    EHSM_KEY_TYPE_RSA_3072_CRT = 0x0f,         ///< RSA-3072 CRT 格式密钥
    EHSM_KEY_TYPE_RSA_4096_CRT = 0x10,         ///< RSA-4096 CRT 格式密钥
    EHSM_KEY_TYPE_DH = 0x11,                   ///< DH（Diffie-Hellman）密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1 = 0x12, ///< ECC BrainpoolP160r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1 = 0x13, ///< ECC BrainpoolP192r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1 = 0x14, ///< ECC BrainpoolP224r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1 = 0x15, ///< ECC BrainpoolP256r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1 = 0x16, ///< ECC BrainpoolP320r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1 = 0x17, ///< ECC BrainpoolP384r1 曲线密钥
    EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1 = 0x18, ///< ECC BrainpoolP512r1 曲线密钥
    EHSM_KEY_TYPE_ECC_SECP_192R1 = 0x19,       ///< ECC secp192r1 曲线密钥（NIST P-192）
    EHSM_KEY_TYPE_ECC_SECP_224R1 = 0x1a,       ///< ECC secp224r1 曲线密钥（NIST P-224）
    EHSM_KEY_TYPE_ECC_SECP_256R1 = 0x1b,       ///< ECC secp256r1 曲线密钥（NIST P-256）
    EHSM_KEY_TYPE_ECC_SECP_384R1 = 0x1c,       ///< ECC secp384r1 曲线密钥（NIST P-384）
    EHSM_KEY_TYPE_ECC_SECP_521R1 = 0x1d,       ///< ECC secp521r1 曲线密钥（NIST P-521）
    EHSM_KEY_TYPE_ED25519 = 0x1e,              ///< Ed25519 签名密钥
    EHSM_KEY_TYPE_X25519 = 0x1f,               ///< X25519 密钥交换密钥
    EHSM_KEY_TYPE_SM4_XTS = 0x20,              ///< SM4 XTS 模式密钥
    EHSM_KEY_TYPE_AES_128_XTS = 0x21,          ///< AES-128 XTS 模式密钥
    EHSM_KEY_TYPE_AES_192_XTS = 0x22,          ///< AES-192 XTS 模式密钥
    EHSM_KEY_TYPE_AES_256_XTS = 0x23,          ///< AES-256 XTS 模式密钥
    EHSM_KEY_TYPE_CHACHA = 0x24,               ///< ChaCha20 流密码密钥
    EHSM_KEY_TYPE_HMAC = 0x25,                 ///< HMAC 消息认证码密钥
    EHSM_KEY_TYPE_ECC_SECP_160K1 = 0x26,       ///< ECC secp160k1 曲线密钥
    EHSM_KEY_TYPE_ECC_SECP_192K1 = 0x27,       ///< ECC secp192k1 曲线密钥
    EHSM_KEY_TYPE_ECC_SECP_224K1 = 0x28,       ///< ECC secp224k1 曲线密钥
    EHSM_KEY_TYPE_ECC_SECP_256K1 = 0x29,       ///< ECC secp256k1 曲线密钥
    EHSM_KEY_TYPE_SM9_ENC_USERPRIV = 0x2a,     ///< SM9 加密用户私钥
    EHSM_KEY_TYPE_SM9_SIGN_USERPRIV = 0x2b,    ///< SM9 签名用户私钥
    EHSM_KEY_TYPE_SM9_EXCHG_USERPRIV = 0x2c,   ///< SM9 密钥交换用户私钥
    EHSM_KEY_TYPE_SM9_EXCHG_TEMP = 0x2d,       ///< SM9 密钥交换临时密钥
                                               // #% #if CONFIG_HOST_PQC_EN
    EHSM_KEY_TYPE_ML_KEM_512 = 0x30,           ///< PQC ML-KEM-512 密钥（NIST安全级别1，等效AES-128）
    EHSM_KEY_TYPE_ML_KEM_768 = 0x31,           ///< PQC ML-KEM-768 密钥（NIST安全级别3，等效AES-192）
    EHSM_KEY_TYPE_ML_KEM_1024 = 0x32,          ///< PQC ML-KEM-1024 密钥（NIST安全级别5，等效AES-256）
    EHSM_KEY_TYPE_ML_DSA_44 = 0x33,            ///< PQC ML_DSA_44 密钥
    EHSM_KEY_TYPE_ML_DSA_65 = 0x34,            ///< PQC ML_DSA_65 密钥
    EHSM_KEY_TYPE_ML_DSA_87 = 0x35,            ///< PQC ML_DSA_87 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_128S = 0x36,    ///< PQC SLH_DSA_SHA2_128S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_128S = 0x37,   ///< PQC SLH_DSA_SHAKE_128S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_128F = 0x38,    ///< PQC SLH_DSA_SHA2_128F 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_128F = 0x39,   ///< PQC SLH_DSA_SHAKE_128F 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_192S = 0x3A,    ///< PQC SLH_DSA_SHA2_192S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_192S = 0x3B,   ///< PQC SLH_DSA_SHAKE_192S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_192F = 0x3C,    ///< PQC SLH_DSA_SHA2_192F 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_192F = 0x3D,   ///< PQC SLH_DSA_SHAKE_192F 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_256S = 0x3E,    ///< PQC SLH_DSA_SHA2_256S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_256S = 0x3F,   ///< PQC SLH_DSA_SHAKE_256S 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHA2_256F = 0x40,    ///< PQC SLH_DSA_SHA2_256F 密钥
    EHSM_KEY_TYPE_SLH_DSA_SHAKE_256F = 0x41,   ///< PQC SLH_DSA_SHAKE_256F 密钥
    // #% #endif /* CONFIG_HOST_PQC_EN */
} ehsm_key_type_e;

typedef enum {
    EHSM_KEY_PART_PUBLIC_KEY = 1,  /**< 密钥数据结构体中仅包含公钥 */
    EHSM_KEY_PART_PRIVATE_KEY = 2, /**< 密钥数据结构体中仅包含私钥 */
    EHSM_KEY_PART_SYMM_KEY = 2,    /**< 密钥数据结构体包含的是对称密钥 */
    EHSM_KEY_PART_KEY_PAIR = 3,    /**< 密钥数据结构体包含的是密钥对 */
} ehsm_key_part_e;

typedef enum {
    EHSM_DERIVE_ALGO_KDFX963 = 1,
    EHSM_DERIVE_ALGO_PBKDF2 = 2,
} ehsm_derive_algo_e;

typedef enum {
    EHSM_DERIVE_TYPE_PASSWORD = 1,        /**< 使用password派生 */
    EHSM_DERIVE_TYPE_FROM_PARENT_KEY = 2, /**< 从父密钥派生 */
} ehsm_derive_type_e;

typedef enum {
    EHSM_SM2_ROLE_SPONSOR = 0,
    EHSM_SM2_ROLE_RESPONSOR = 1,
} ehsm_sm2_role_e;

typedef enum {
    EHSM_SM9_ENC_TYPE_STREAM = 0,
    EHSM_SM9_ENC_TYPE_BLOCK = 1,
} ehsm_sm9_enc_type_e;

typedef enum {
    EHSM_SM9_PADDING_NONE = 0,
    EHSM_SM9_PADDING_PKCS7 = 1,
} ehsm_sm9_padding_mode_e;

typedef enum {
    EHSM_SM9_ROLE_SPONSOR = 0,
    EHSM_SM9_ROLE_RESPONSOR = 1,
} ehsm_sm9_role_e;

typedef enum {
    EHSM_PROC_INIT = 1,    ///< 三段式处理的开始阶段
    EHSM_PROC_UPDATE = 2,  ///< 三段式处理的更新数据阶段
    EHSM_PROC_FINISH = 4,  ///< 三段式处理的结束阶段
    EHSM_PROC_ONEPASS = 7, /// 一次性处理
} ehsm_proc_mode_e;

/**
 * @brief SM9密钥交换算法相关的参数。
 */
typedef struct {
    EHSM_SHM raddr_t peer_tmp_pub_addr; /**< 对方临时公钥的主机地址 */
    EHSM_SHM raddr_t kgc_pub_key_addr;  /**< KGC系统加密主公钥的主机地址，公钥大小为64字节 */
    EHSM_SHM raddr_t fp12g_addr;        /**< e(P1, pub_key)值的主机地址，如果设置为NULL，将在eHSM内部计算 */
    EHSM_SHM raddr_t self_id_addr;      /**< 本方用户ID的主机地址 */
    EHSM_SHM raddr_t peer_id_addr;      /**< 对方用户ID的主机地址 */
    uint32_t peer_id_size;              /**< 对方用户ID的字节数，最大可为1024 */
    uint32_t self_id_size;              /**< 本方用户ID的字节数，最大可为1024 */
    EHSM_SHM raddr_t s1_s2_addr;        /**< S1（或S2）验证值的主机地址，大小为32字节 */
    EHSM_SHM raddr_t sa_sb_addr;        /**< SA（或SB）验证值的主机地址，大小为32字节 */
} ehsm_sm9_params_st;

/**
 * @brief ECC明文密钥数据结构
 */
typedef struct {
    uint32_t curve_id;        /**< ECC曲线对应的密钥编号，见 @ref ehsm_key_type_e */
    EHSM_SHM raddr_t privkey; /**< ECC私钥的存储地址，长度由curve_id对应的曲线类型决定 */
    EHSM_SHM raddr_t pubkey;  /**< ECC公钥的存储地址，长度由curve_id对应的曲线类型决定，不包含前缀`04` */
} ehsm_ecc_key_st;

/**
 * @brief RSA明文密钥数据结构
 */
typedef struct {
    uint8_t crt_mode;     /**< CRT模式，1代表私钥使用CRT模式 */
    uint8_t reserved0[3]; /**< 保留字段 */
    uint32_t n_byte_sz;   /**< n值的字节数 */
    uint32_t e_byte_sz;   /**< e值的字节数 */
    EHSM_SHM raddr_t n;   /**< n值的存储地址 */
    EHSM_SHM raddr_t e;   /**< e值的存储地址 */
    EHSM_SHM raddr_t d;   /**< d值的存储地址 */
    EHSM_SHM raddr_t p;   /**< p值的存储地址 */
    EHSM_SHM raddr_t q;   /**< q值的存储地址 */
    EHSM_SHM raddr_t dp;  /**< dp值的存储地址 */
    EHSM_SHM raddr_t dq;  /**< dq值的存储地址 */
    EHSM_SHM raddr_t u;   /**< u值的存储地址 */
} ehsm_rsa_key_st;

/**
 * @brief SM2明文密钥数据结构
 */
typedef struct {
    uint8_t privkey[32]; /**< SM2私钥 */
    uint8_t pubkey[65];  /**< SM2公钥, `04` + X + Y */
} ehsm_sm2_key_st;

// #% #if CONFIG_HOST_PQC_EN
/**
 * @brief PQC 签名模式枚举
 */
typedef enum {
    EHSM_PQC_PURE_DSA_WITH_MSG = 0U,        /**< 由 Host 输入消息，HSM 使用 Pure DSA 接口签名验签 */
    EHSM_PQC_PRE_HASH_DSA_WITH_DIGEST = 1U, /**< 由 Host 输入消息摘要，HSM 使用 Pre-Hash DSA 接口签名验签 */
} ehsm_pqc_sign_mode_e;

/**
 * @brief PQC 数字签名算法类型
 */
typedef enum {
    EHSM_PQC_DSA_ALGO_ML_DSA = 0U,  /**< ML-DSA - 基于格的模块化DSA算法(FIPS 204) */
    EHSM_PQC_DSA_ALGO_SLH_DSA = 1U, /**< SLH-DSA - 基于哈希的无状态层次化DSA算法(FIPS 205) */
} ehsm_pqc_sign_algo_e;

typedef enum {
    EHSM_PQC_HASH_ALGO_SHA256 = 2,     /**< SHA-256 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA384 = 3,     /**< SHA-384 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA512 = 4,     /**< SHA-512 哈希算法 */
    EHSM_PQC_HASH_ALGO_RSV0 = 5,       /**< 预留：SHA-1哈希算法（不建议使用，存在安全风险）*/
    EHSM_PQC_HASH_ALGO_SHA224 = 6,     /**< SHA-224 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA512_224 = 7, /**< SHA-512/224 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA512_256 = 8, /**< SHA-512/256 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA3_224 = 9,   /**< SHA3-224 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA3_256 = 10,  /**< SHA3-256 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA3_384 = 11,  /**< SHA3-384 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHA3_512 = 12,  /**< SHA3-512 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHAKE128 = 13,  /**< SHAKE128 哈希算法 */
    EHSM_PQC_HASH_ALGO_SHAKE256 = 14,  /**< SHAKE256 哈希算法 */
    EHSM_PQC_HASH_ALGO_END = 15,
} ehsm_pqc_hash_algo_e;

/**
 * @brief PQC 明文密钥数据结构
 *
 * @note
 * - **PQC SLH-DSA 的签名生成需要密钥对（公私钥对）**
 *
 * @warning 安全提醒：
 * - 此结构包含明文密钥材料，必须在共享内存中分配
 * - 使用完毕后应安全清除内存内容
 * - 私钥材料不应在非安全环境中长时间存储
 */
typedef struct {
    uint32_t algo_id;         /**< 密钥算法 ID，取值参考 @ref ehsm_key_type_e 中的 PQC DSA 算法类型 */
    EHSM_SHM raddr_t privkey; /**< PQC 私钥（DSA签名）或解封密钥（ML-KEM解封）的存储地址 */
    uint32_t privkey_size;    /**< 私钥/解封密钥的字节长度，由 algo_id 算法类型决定 */
    EHSM_SHM raddr_t pubkey;  /**< PQC 公钥（DSA验签）或封装密钥（ML-KEM封装）的存储地址 */
    uint32_t pubkey_size;     /**< 公钥/封装密钥的字节长度，由 algo_id 算法类型决定 */
} ehsm_pqc_key_st;

/**
 * @brief PQC 共享密钥输出类型
 */
typedef enum {
    EHSM_PQC_OUT_KEY_HANDLE = 0U, /**< 输出为密钥句柄，密钥保存在eHSM内部 */
    EHSM_PQC_OUT_PLAIN_KEY = 1U,  /**< 输出为明文密钥，直接返回密钥内容 */
} ehsm_pqc_out_type_e;
// #% #endif /* CONFIG_HOST_PQC_EN */
/**
 * @defgroup key-priv 密钥权限
 * @{
 */
#define EHSM_KEY_PRIV_SIGN                   (0x0001U)  /**< 密钥可用于签名或生成MAC/HMAC */
#define EHSM_KEY_PRIV_VERIFY                 (0x0002U)  /**< 密钥可用于验签或验证MAC/HMAC */
#define EHSM_KEY_PRIV_ENCRYPT                (0x0004U)  /**< 密钥可用于加密 */
#define EHSM_KEY_PRIV_DECRYPT                (0x0008U)  /**< 密钥可用于解密 */
#define EHSM_KEY_PRIV_TIMESTAMP              (0x0010U)  /**< 保留 */
#define EHSM_KEY_PRIV_SECURE_BOOT            (0x0020U)  /**< 保留 */
#define EHSM_KEY_PRIV_SECURE_STORAGE         (0x0040U)  /**< 保留 */
#define EHSM_KEY_PRIV_KEY_CREATION           (0x0080U)  /**< 密钥可用于派生或协商新密钥 */
#define EHSM_KEY_PRIV_TRANSPORT_KEY_CREATION (0x0100U)  /**< 密钥可用于派生传输密钥 */
#define EHSM_KEY_PRIV_UTCSYNC                (0x0200U)  /**< 保留 */
#define EHSM_KEY_PRIV_TRANSPORT              (0x0400U)  /**< 密钥可作为传输密钥使用 */
#define EHSM_KEY_PRIV_REMOVE                 (0x0800U)  /**< 密钥可以被移除 */
#define EHSM_KEY_PRIV_IMPORT_PLAIN           (0x1000U)  /**< 密钥可以被明文导入 */
#define EHSM_KEY_PRIV_IMPORT_CIPHER          (0x2000U)  /**< 密钥可以被密文导入 */
#define EHSM_KEY_PRIV_EXPORT_PLAIN           (0x4000U)  /**< 密钥可以被明文导出 */
#define EHSM_KEY_PRIV_EXPORT_CIPHER          (0x8000U)  /**< 密钥可以被密文导出 */
#define EHSM_KEY_PRIV_BOOT_FAIL_USAGE        (0x10000U) /**< 保留 */
#define EHSM_KEY_PRIV_DEBUG_USAGE            (0x20000U) /**< 保留 */
#define EHSM_KEY_PRIV_WILDCARD_PROTECT       (0x40000U) /**< 保留 */
#define EHSM_KEY_PRIV_WRITE_PROTECT          (0x80000U) /**< 密钥写保护（不能覆盖）*/
/** @} */

typedef enum {
    /** @brief 中断模式。
     *
     * 会初始化Mailbox寄存器，底层Mailbox使用中断收取数据，根据ctx中的async配置API决定会立即返回 `EHSM_ERR_NEED_POLL`
     * 还是等待操作完成。
     */
    EHSM_DRV_MODE_INTERRUPT = 0,

    /** @brief 同步等待模式。
     *
     * 会初始化Mailbox寄存器，底层Mailbox使用轮询收取数据，API总是等待响应并返回结果，忽略ctx中的async配置。
     */
    EHSM_DRV_MODE_WAIT_AND_POLL = 1,

    /** @brief 发送并由用户POLL。
     *
     * 不清Mailbox寄存器，不配置中断，也不轮询接收，API总是立刻返回，需要反复调用`ehsm_ctx_poll`判断是否完成，
     * 忽略ctx中的async配置。
     */
    EHSM_DRV_MODE_SEND_AND_PEEK = 2,
} ehsm_drv_mode_e;

typedef enum {
    EHSM_KEY_LEVEL_1 = 1, /**< 此OTP密钥由CHIP ROOT KEY加密，仅支持 `TEST_MODE` 和 `DEVVELOP_MODE` */
    EHSM_KEY_LEVEL_2
    = 2, /**< 此OTP密钥由DEVICE ROOT KEY加密，仅支持 `TEST_MODE` 、`DEVELOP_MODE` 和 `MANUFACTURE_MODE` */
} ehsm_key_level_e;

typedef enum {
    EHSM_INSTALL_KEY_TYPE_SYMM = 1,
    EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY = 2,
    EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH = 4,
} ehsm_install_key_type_e;

/**
 * @brief 命令处理完成的回调函数原型。
 *
 */
typedef void (*ehsm_rsp_cb_func_t)(ehsm_ctx_st *ctx);

/**
 * @brief 初始化驱动库。
 *
 * @param[in] drv_mode 底层使用的驱动模式，想要支持异步必须使用中断模式，见 @ref ehsm_drv_mode_e
 *
 * @retval EHSM_OK 初始化驱动库成功。
 * @retval 其它值 初始化驱动库失败。
 *
 * @note 此函数不涉及向 eHSM 发送命令。
 */
uint32_t ehsm_driver_init_library(ehsm_drv_mode_e drv_mode);

/**
 * @brief 获取驱动库的版本号。
 *
 * 版本号的格式为 major.minor.patch，使用32位整数的低24位，每字节表示一个字段。例如：0x00010203 表示 v1.2.3 。
 *
 * @return 驱动库的版本号
 *
 * @note 此函数不涉及向 eHSM 发送命令。
 */
uint32_t ehsm_driver_get_version(void);

/**
 * @brief 初始化 eHSM API 调用的 context 。
 *
 * 说明：
 *
 * - 当配置了 `async=true` 且为 @ref EHSM_DRV_MODE_INTERRUPT 模式时，API函数不会等待 eHSM 处理完才返回，而是先返回
 * @ref EHSM_ERR_NEED_POLL
 * - 异步时，如果 `rsp_callback` 不为 NULL，则任务完成时 `rsp_callback` 会被调用；否则，需要主动调用
 * @ref ehsm_ctx_poll 获取任务是否完成
 *
 * @param[out] ctx  新分配的 ehsm context 地址
 * @param[in] mb_ch 使用的 mailbox channel
 * @param[in] async 是否异步模式，仅在 @ref EHSM_DRV_MODE_INTERRUPT 模式时有效
 * @param[in] rsp_callback 异步模式下，任务完成时的回调函数，可以为NULL。同步模式下此参数忽略。
 *
 * @note 此函数仅初始化 `ctx` 内存，不涉及向 eHSM 发送命令。
 */
void ehsm_ctx_init(EHSM_SHM ehsm_ctx_st *ctx, uint8_t mb_ch, bool_t async, ehsm_rsp_cb_func_t rsp_callback);

/**
 * @brief 异步模式下，查询之前调用的任务是否已完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 此函数仅查询 `ctx` 中保存的状态，不涉及向 eHSM 发送命令。
 */
uint32_t ehsm_ctx_poll(EHSM_SHM ehsm_ctx_st *ctx);

/**
 * @brief 生成随机数。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 后处理算法类型，见 @ref ehsm_rng_algo_e
 * @param[out] rand_buf 输出 buffer
 * @param[in] rand_size 生成的随机数长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_gen_random(
    EHSM_SHM ehsm_ctx_st *ctx, ehsm_rng_algo_e algo, EHSM_SHM uint8_t *rand_buf, uint32_t rand_size);

/**
 * @brief HASH计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在
 * @ref ehsm_hash_update 和 @ref ehsm_hash_finish 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hash_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM ehsm_session_st *session);

/**
 * @brief HASH计算，更新消息数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_hash_init 或 @ref ehsm_hash_update
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hash_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief HASH计算，生成HASH结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_hash_init 或 @ref ehsm_hash_update
 * @param[out] digest 存放HASH值的buffer。
 * @param[in,out] digest_size 输入时存储的是 digest buffer 长度，输出时是实际生成的HASH值长度。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hash_finish(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *digest, uint32_t *digest_size);

/**
 * @brief HASH计算，一次完成计算并输出结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] msg 输入的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[out] digest 存放HASH值的buffer。
 * @param[in,out] digest_size 输入时存储的是 digest buffer 长度，输出时是实际生成的HASH值长度。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hash_onepass(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *msg,
    uint32_t msg_size, EHSM_SHM uint8_t *digest, uint32_t *digest_size);

/**
 * @brief HMAC计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key_handle 用于HMAC计算的密钥句柄
 * @param[in] gen_hmac 操作方式，必须跟最后调用的 @ref ehsm_hmac_finish_gen 或 @ref ehsm_hmac_finish_verify 相匹配
 *              - `true` 生成HMAC值
 *              - `false` 校验HAMC值
 * @param[in,out] session 用于ehsm内部缓存HMAC计算中间值的 buffer，此参数指向的内存在 @ref ehsm_hmac_update ，
 * @ref ehsm_hmac_finish_gen 和 @ref ehsm_hmac_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_hmac,
    EHSM_SHM ehsm_session_st *session);

/**
 * @brief HMAC计算，更新消息数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_hmac_init 或 @ref ehsm_hmac_update
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief HMAC计算，生成HMAC结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_hmac_init 或 @ref ehsm_hmac_update
 * @param[out] hmac 存放HMAC值的buffer
 * @param[in] hmac_size 期望生成的HMAC长度，不能小于8，也不能大于实际算法对应的HMAC长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_finish_gen(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *hmac, uint32_t hmac_size);

/**
 * @brief HMAC计算，验证HMAC值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_hmac_init 或 @ref ehsm_hmac_update
 * @param[in] hmac 待验证的HMAC值存放的位置
 * @param[in] hmac_size HMAC值大小，不能小于8
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_finish_verify(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *hmac, uint32_t hmac_size, bool_t *verify_result);

/**
 * @brief HMAC计算，一次完成计算并生成HMAC结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key_handle 用于HMAC计算的密钥句柄
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[out] hmac 存放HMAC值的buffer
 * @param[in] hmac_size 期望生成的HMAC长度，不能小于8，也不能大于实际算法对应的HMAC长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *hmac, uint32_t hmac_size);

/**
 * @brief HMAC计算，一次完成计算并验证HMAC值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key_handle 用于HMAC计算的密钥句柄
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[in] hmac 待验证的HMAC值存放的位置
 * @param[in] hmac_size HMAC值大小，不能小于8
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *hmac, uint32_t hmac_size,
    bool_t *verify_result);

/**
 * @brief HMAC计算，初始化，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key 用于HMAC计算的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] gen_hmac 操作方式，必须跟最后调用的 @ref ehsm_hmac_finish_gen 或 @ref ehsm_hmac_finish_verify 相匹配
 *              - `true` 生成HMAC值
 *              - `false` 校验HAMC值
 * @param[in,out] session 用于ehsm内部缓存HMAC计算中间值的 buffer，此参数指向的内存在 @ref ehsm_hmac_update ，
 * @ref ehsm_hmac_finish_gen 和 @ref ehsm_hmac_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    uint32_t key_size, bool_t gen_hmac, EHSM_SHM ehsm_session_st *session);

/**
 * @brief HMAC计算，一次完成计算并生成HMAC结果，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key 用于HMAC计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[out] hmac 存放HMAC值的buffer
 * @param[in] hmac_size 期望生成的HMAC长度，不能小于8，也不能大于实际算法对应的HMAC长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM uint8_t *hmac, uint32_t hmac_size);

/**
 * @brief HMAC计算，一次完成计算并验证HMAC值，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法类型，见 @ref ehsm_hash_algo_e
 * @param[in] key 用于HMAC计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[in] hmac 待验证的HMAC值存放的位置
 * @param[in] hmac_size HMAC值大小，不能小于8
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_hmac_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *hmac, uint32_t hmac_size, bool_t *verify_result);

/**
 * @brief MAC计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key_handle 用于计算MAC的对称密钥的句柄
 * @param[in] gen_mac 操作方式，必须跟最后调用的 @ref ehsm_mac_finish_gen 或 @ref ehsm_mac_finish_verify 相匹配
 *              - `true` 生成MAC值
 *              - `false` 校验MAC值
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] mac_size 期望生成或需要校验的MAC长度，不能小于8，也不能大于实际算法对应的MAC长度
 * @param[in,out] session 用于ehsm内部缓存MAC计算中间值的 buffer，此参数指向的内存在
 *  @ref ehsm_mac_update , @ref ehsm_mac_finish_gen 和 @ref ehsm_mac_finish_verify 中仍会使用，
 * 因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode, uint32_t key_handle,
    bool_t gen_mac, EHSM_SHM const uint8_t *iv, uint32_t iv_size, uint32_t mac_size, EHSM_SHM ehsm_session_st *session);

/**
 * @brief MAC计算，更新消息数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_mac_init 或 @ref ehsm_mac_update
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度，
 *              - 对于CBC-MAC，要求输入的总长度不能为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief MAC计算，生成MAC结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_mac_init 或 @ref ehsm_mac_update
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度，
 *              - 对于CBC-MAC，要求输入的总长度不能为0
 *              - 对于CMAC、CBC-MAC、GMAC，要求当前接口 `msg_size` 不能为0
 * @param[out] mac 存放MAC值的buffer，其长度需大于等于 @ref ehsm_mac_init 传入的参数 `mac_size`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_finish_gen(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *mac);

/**
 * @brief MAC计算，验证MAC值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_mac_init 或 @ref ehsm_mac_update
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度，
 *              - 对于CBC-MAC，要求输入的总长度不能为0
 *              - 对于CMAC、CBC-MAC、GMAC，要求当前接口 `msg_size` 不能为0
 * @param[in] mac 待验证的MAC值存放的位置，其长度需等于 @ref ehsm_mac_init 传入的参数 `mac_size`
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_finish_verify(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *mac, bool_t *verify_result);

/**
 * @brief MAC计算，一次完成计算并生成MAC结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key_handle 用于计算MAC的对称密钥的句柄
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[out] mac 存放MAC值的buffer
 * @param[in] mac_size 期望生成的MAC长度，不能小于8，也不能大于实际算法对应的MAC长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    uint32_t key_handle, EHSM_SHM const uint8_t *iv, uint32_t iv_size, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM uint8_t *mac, uint32_t mac_size);

/**
 * @brief MAC计算，一次完成计算并验证MAC值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key_handle 用于计算MAC的对称密钥的句柄
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[in] mac 待验证的MAC值存放的位置
 * @param[in] mac_size MAC值大小，不能小于8
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    uint32_t key_handle, EHSM_SHM const uint8_t *iv, uint32_t iv_size, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *mac, uint32_t mac_size, bool_t *verify_result);

/**
 * @brief MAC计算，初始化，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key 用于MAC计算的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] gen_mac 操作方式，必须跟最后调用的 @ref ehsm_mac_finish_gen 或 @ref ehsm_mac_finish_verify 相匹配
 *              - `true` 生成MAC值
 *              - `false` 校验MAC值
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] mac_size 期望生成或需要校验的MAC长度，不能小于8，也不能大于实际算法对应的MAC长度
 * @param[in,out] session 用于ehsm内部缓存MAC计算中间值的 buffer，此参数指向的内存在
 *  @ref ehsm_mac_update , @ref ehsm_mac_finish_gen 和 @ref ehsm_mac_finish_verify 中仍会使用，
 * 因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t gen_mac, EHSM_SHM const uint8_t *iv, uint32_t iv_size,
    uint32_t mac_size, EHSM_SHM ehsm_session_st *session);

/**
 * @brief MAC计算，一次完成计算并生成MAC结果，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key 用于MAC计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[out] mac 存放MAC值的buffer
 * @param[in] mac_size 期望生成的MAC长度，不能小于8，也不能大于实际算法对应的MAC长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *iv, uint32_t iv_size,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *mac, uint32_t mac_size);

/**
 * @brief MAC计算，一次完成计算并验证MAC值，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称算法的ID，见 @ref ehsm_symm_algo_e
 * @param[in] mode MAC算法模式，见 @ref ehsm_mac_mode_e
 * @param[in] key 用于MAC计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] iv GMAC算法使用的IV值，其它模式请传NULL。
 * @param[in] iv_size IV值的字节长度，GMAC必须是12字节，其它模式请填0。
 * @param[in] msg 输入更新的消息数据。
 * @param[in] msg_size 消息数据长度。
 * @param[in] mac 待验证的MAC值存放的位置
 * @param[in] mac_size MAC值大小，不能小于8
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_mac_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *iv, uint32_t iv_size,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *mac, uint32_t mac_size,
    bool_t *verify_result);

/**
 * @brief 对称加密/解密计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称加密算法，见 @ref ehsm_symm_algo_e
 * @param[in] mode 对称加密模式，见 @ref ehsm_cipher_mode_e
 * @param[in] padding 对称加密填充算法，见 @ref ehsm_padding_mode_e ，对于XTS模式，此参数被忽略
 * @param[in] key_handle 对称密钥句柄
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] iv IV/Nonce值
 *  - ECB模式下忽略此值，应传入 `NULL`
 *  - CTR模式下为 Initial Counter
 *  - XTS模式下为 initial i value
 * @param[in] iv_size `iv`数据的长度，应为算法的 block size
 * @param[in,out] session 用于ehsm内部缓存 cipher 计算中间值的 buffer，此参数指向的内存在 @ref ehsm_symm_cipher_update
 * 和 @ref ehsm_symm_cipher_finish 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *iv, uint32_t iv_size,
    EHSM_SHM ehsm_session_st *session);

/**
 * @brief 对称加密/解密计算，更新数据。
 *
 * 此函数处理连续的块数据，输入数据长度必须block size对齐，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * 此函数不会进行填充或去填充：
 * - 在解密模式下，必须保留至少一个block密文数据给 @ref ehsm_symm_cipher_finish 调用，以便去掉填充
 * - 在加密模式下，保证输入数据 block size 对齐即可
 *
 * 在XTS模式下，无论是加密还是解密，必须保留至少 2 个 block 的数据给 @ref ehsm_symm_cipher_finish 处理
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_symm_cipher_init 初始化
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，必须 block size 对齐
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_update(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output);

/**
 * @brief 对称加密/解密计算，结束计算。
 *
 * 此函数处理最后一部分数据：
 * - 在加密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在加密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 可以为任意长度
 * - 在解密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在解密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 必须是 block size 对齐且至少有 1 个 block。
 *
 * 在XTS模式下，无论是加密还是解密，必须保留至少 2 个 block 的数据给此函数处理
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_symm_cipher_init 或 @ref ehsm_symm_cipher_update
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，见说明
 * @param[out] output 输出buffer
 * @param[in,out] output_size 传入时为输出buffer长度，加密且有填充时，其长度应比`input_size`大一个block size；
 * 完成时为实际的输出数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_finish(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief 对称加密/解密计算，一次完成计算。
 *
 * 此函数一次处理所有数据：
 * - 在加密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在加密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 可以为任意长度
 * - 在解密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在解密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 必须是 block size 对齐且至少有 1 个 block。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称加密算法，见 @ref ehsm_symm_algo_e
 * @param[in] mode 对称加密模式，见 @ref ehsm_cipher_mode_e
 * @param[in] padding 对称加密填充算法，见 @ref ehsm_padding_mode_e ，对于XTS模式，此参数被忽略

 * @param[in] key_handle 对称密钥句柄
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] iv IV/Nonce值
 *  - ECB模式下忽略此值，应传入 `NULL`
 *  - CTR模式下为 Initial Counter
 *  - XTS模式下为 initial i value
 * @param[in] iv_size `iv`数据的长度，应为算法的 block size
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，见说明
 * @param[out] output 输出buffer
 * @param[in,out] output_size 传入时为输出buffer长度，加密且有填充时，其长度应比`input_size`大一个block size；
 * 完成时为实际的输出数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_onepass(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *iv, uint32_t iv_size,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief 对称加密/解密计算，初始化，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称加密算法，见 @ref ehsm_symm_algo_e
 * @param[in] mode 对称加密模式，见 @ref ehsm_cipher_mode_e
 * @param[in] padding 对称加密填充算法，见 @ref ehsm_padding_mode_e ，对于XTS模式，此参数被忽略
 * @param[in] key 用于加密/解密的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] iv IV/Nonce值
 *  - ECB模式下忽略此值，应传入 `NULL`
 *  - CTR模式下为 Initial Counter
 *  - XTS模式下为 initial i value
 * @param[in] iv_size `iv`数据的长度，应为算法的 block size
 * @param[in,out] session 用于ehsm内部缓存 cipher 计算中间值的 buffer，此参数指向的内存在 @ref ehsm_symm_cipher_update
 * 和 @ref ehsm_symm_cipher_finish 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc, EHSM_SHM const uint8_t *iv,
    uint32_t iv_size, EHSM_SHM ehsm_session_st *session);

/**
 * @brief 对称加密/解密计算，一次完成计算，使用明文密钥计算。
 *
 * 此函数一次处理所有数据：
 * - 在加密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在加密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 可以为任意长度
 * - 在解密模式下，如果填充是 @ref EHSM_PADDING_NONE ，则 `input_size` 应当为 0 或 block size 对齐
 * - 在解密模式下，如果填充不是 @ref EHSM_PADDING_NONE ，则 `input_size` 必须是 block size 对齐且至少有 1 个 block。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 对称加密算法，见 @ref ehsm_symm_algo_e
 * @param[in] mode 对称加密模式，见 @ref ehsm_cipher_mode_e
 * @param[in] padding 对称加密填充算法，见 @ref ehsm_padding_mode_e ，对于XTS模式，此参数被忽略
 * @param[in] key 用于加密/解密的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] iv IV/Nonce值
 *  - ECB模式下忽略此值，应传入 `NULL`
 *  - CTR模式下为 Initial Counter
 *  - XTS模式下为 initial i value
 * @param[in] iv_size `iv`数据的长度，应为算法的 block size
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，见说明
 * @param[out] output 输出buffer
 * @param[in,out] output_size 传入时为输出buffer长度，加密且有填充时，其长度应比`input_size`大一个block size；
 * 完成时为实际的输出数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_symm_cipher_onepass_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo,
    ehsm_cipher_mode_e mode, ehsm_padding_mode_e padding, EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc,
    EHSM_SHM const uint8_t *iv, uint32_t iv_size, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief SM2加密/解密计算，一次完成计算。
 *
 * @note SM2加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] enc 是否加密
 *          - `true` 加密，密钥句柄对应的密钥中必须包含公钥
 *          - `false` 解密，密钥句柄对应的密钥中必须包含私钥
 * @param[in] input 输入的明文或密文，密文的格式为C1C3C2，其中
 *          - C1为曲线上的点，65字节，以`0x04`开头，以及完整 x 和 y 坐标
 *          - C2为实际的密文数据，与对应的明文长度相同
 *          - C3为SM3的结果，32字节，用于保证完整性
 * @param[in] input_size 输入数据的长度，最大不能超过1024字节
 * @param[out] output    保存输出数据的buffer，如果是加密，则密文格式 C1C3C2，见`input`的描述
 * @param[in,out] output_size 传入时为输出buffer的长度，如果是加密，应大于等于`input_size + 97`；如果是解密，应大于等于
 * `input_size - 97`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_cipher(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief SM2加密/解密计算，一次完成计算，使用明文密钥计算。
 *
 * @note SM2加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于加密/解密的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_sm2_key_st
 * @param[in] enc 是否加密
 *          - `true` 加密，密钥数据必须包含公钥
 *          - `false` 解密，密钥数据必须包含私钥
 * @param[in] input 输入的明文或密文，密文的格式为C1C3C2，其中
 *          - C1为曲线上的点，65字节，以`0x04`开头，以及完整 x 和 y 坐标
 *          - C2为实际的密文数据，与对应的明文长度相同
 *          - C3为SM3的结果，32字节，用于保证完整性
 * @param[in] input_size 输入数据的长度，最大不能超过1024字节
 * @param[out] output    保存输出数据的buffer，如果是加密，则密文格式 C1C3C2，见`input`的描述
 * @param[in,out] output_size 传入时为输出buffer的长度，如果是加密，应大于等于`input_size + 97`；如果是解密，应大于等于
 * `input_size - 97`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_cipher_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief SM2签名/验签计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_sm2_sign_finish_gen 或 @ref
 * ehsm_sm2_sign_finish_verify 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_sm2_sign_update 、
 * @ref ehsm_sm2_sign_finish_gen 和 @ref ehsm_sm2_sign_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_init(
    EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t gen_sig, EHSM_SHM ehsm_session_st *session);

/**
 * @brief SM2签名/验签计算，初始化，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于签名/验签的密钥值，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format 和 @ref
 * ehsm_sm2_key_st
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_sm2_sign_finish_gen 或 @ref ehsm_sm2_sign_finish_verify
 * 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_sm2_sign_update 、 @ref
 * ehsm_sm2_sign_finish_gen 和 @ref ehsm_sm2_sign_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_init_with_plain_key(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t gen_sig, EHSM_SHM ehsm_session_st *session);

/**
 * @brief SM2签名/验签计算，更新数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_sm2_sign_init 或 @ref ehsm_sm2_sign_init_with_plain_key
 * 或 @ref ehsm_sm2_sign_update
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief SM2签名/验签计算，生成签名值。
 *
 * 生成的SM2签名值的长度固定为64字节。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_sm2_sign_init 或 @ref ehsm_sm2_sign_update
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于64
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_finish_gen(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *sig, uint32_t sig_size);

/**
 * @brief SM2签名/验签计算，验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_sm2_sign_init 或 @ref ehsm_sm2_sign_update
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受64字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_finish_verify(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *sig, uint32_t sig_size, bool_t *verify_result);

/**
 * @brief SM2签名生成/验证扩展接口，支持明文密钥、密钥句柄、摘要输入等多种模式的一次性签名操作。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] use_plain_key 是否使用明文密钥
 *              - `true` 使用明文密钥，key参数有效，key_handle参数被忽略
 *              - `false` 使用密钥句柄，key_handle参数有效，key参数被忽略
 * @param[in] key_handle SM2密钥句柄，当use_plain_key为false时使用
 * @param[in] key 用于签名/验证的明文密钥，当use_plain_key为true时使用，格式参考 @ref plain-key-format 和 @ref
 * ehsm_sm2_key_st
 * @param[in] gen_sig 操作方式
 *              - `true` 生成签名值
 *              - `false` 验证签名值
 * @param[in] is_digest 输入数据类型
 *              - `true` input为已计算好的消息摘要(E值)，长度必须为32字节
 *              - `false` input为原始消息数据，由eHSM内部计算摘要
 * @param[in] input 输入数据，根据is_digest参数决定是消息原文还是消息摘要
 * @param[in] input_size 输入数据长度，当is_digest为true时必须为32字节
 * @param[in,out] sig_addr 签名值的地址
 *              - 生成签名时：作为输出buffer的地址，存放生成的签名值
 *              - 验证签名时：作为输入buffer的地址，存放待验证的签名值
 * @param[in] sig_size 签名buffer长度，SM2签名固定为64字节
 * @param[out] verify_result 验证结果存放地址，仅在gen_sig为false时有效
 *              - `true` 验证通过
 *              - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm2_sign_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, bool_t use_plain_key, uint32_t key_handle,
    EHSM_SHM const uint8_t *key, bool_t gen_sig, bool_t is_digest, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uintptr_t sig_addr, uint32_t sig_size, bool_t *verify_result);

/**
 * @brief SM2签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于64
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/true,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief SM2验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受64字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *sig, uint32_t sig_size,
    bool_t *verify_result)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/false,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, verify_result);
}

/**
 * @brief SM2签名值生成，一次计算完成, 外部传入计算好的消息摘要(E值)方式。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] digest 输入的消息的摘要(E值)数据
 * @param[in] digest_size 输入的消息的摘要数据长度，仅支持32字节长度。
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于64
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_gen_with_digest(ehsm_ctx_st *ctx, uint32_t key_handle,
    const uint8_t *digest, uint32_t digest_size, EHSM_SHM uint8_t *sig, uint32_t sig_size)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/true,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief SM2验证签名值，一次计算完成, 外部传入计算好的消息摘要(E值)方式。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM2密钥句柄
 * @param[in] digest 输入消息的摘要数据
 * @param[in] digest_size 输入摘要数据长度，仅支持32字节长度。
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受64字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_verify_with_digest(ehsm_ctx_st *ctx, uint32_t key_handle,
    const uint8_t *digest, uint32_t digest_size, EHSM_SHM const uint8_t *sig, uint32_t sig_size, bool_t *verify_result)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/false,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, verify_result);
}

/**
 * @brief SM2签名值生成，一次计算完成，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于签名/验签的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_sm2_key_st
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于64
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/true, /*key_handle=*/0, key, /*gen_sig=*/true,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief SM2验证签名值，一次计算完成，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于签名/验签的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_sm2_key_st
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受64字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx,
    EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *sig,
    uint32_t sig_size, bool_t *verify_result)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/true, /*key_handle=*/0, key, /*gen_sig=*/false,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, verify_result);
}

/**
 * @brief SM2签名值生成，一次计算完成，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于签名/验签的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_sm2_key_st
 * @param[in] digest 输入的消息的摘要(E值)数据
 * @param[in] digest_size 输入的消息的摘要数据长度，仅支持32字节长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于64
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_gen_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM uint8_t *sig,
    uint32_t sig_size)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/true, /*key_handle=*/0, key, /*gen_sig=*/true,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief SM2验证签名值，一次计算完成，使用明文密钥计算。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于签名/验签的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_sm2_key_st
 * @param[in] digest 输入消息的摘要数据
 * @param[in] digest_size 输入摘要数据长度，仅支持32字节长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受64字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_sm2_sign_onepass_verify_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM const uint8_t *sig,
    uint32_t sig_size, bool_t *verify_result)
{
    return ehsm_sm2_sign_onepass_ex(ctx, /*use_plain_key=*/true, /*key_handle=*/0, key, /*gen_sig=*/false,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, verify_result);
}

/**
 * @brief RSA加密/解密计算，一次完成计算。
 *
 * @note RSA加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle RSA密钥句柄
 * @param[in] enc 是否加密
 *          - `true` 加密，密钥句柄对应的密钥中必须包含公钥
 *          - `false` 解密，密钥句柄对应的密钥中必须包含私钥
 * @param[in] input 输入的明文或密文，输入后会作为一个大数参与运算，其值应小于N，长度可以小于对应密钥长度，字节序为大端
 * @param[in] input_size 输入数据的长度
 * @param[out] output    保存输出数据的buffer
 * @param[in,out] output_size 传入时为输出buffer的长度，完成时为实际的输出数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_cipher(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief RSA加密/解密计算，一次完成计算，使用明文密钥计算。
 *
 * @note RSA加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于加密/解密的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_rsa_key_st
 * @param[in] enc 是否加密
 *          - `true` 加密，密钥句柄对应的密钥中必须包含公钥
 *          - `false` 解密，密钥句柄对应的密钥中必须包含私钥
 * @param[in] input 输入的明文或密文，输入后会作为一个大数参与运算，其值应小于N，长度可以小于对应密钥长度，字节序为大端
 * @param[in] input_size 输入数据的长度
 * @param[out] output    保存输出数据的buffer
 * @param[in,out] output_size 传入时为输出buffer的长度，完成时为实际的输出数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_cipher_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size);

/**
 * @brief RSA签名/验签计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
 * @param[in] key_handle RSA密钥句柄
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_rsa_sign_finish_gen 或 @ref ehsm_rsa_sign_finish_verify
 * 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_rsa_sign_update 、 @ref
 * ehsm_rsa_sign_finish_gen 和 @ref ehsm_rsa_sign_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_sig,
    ehsm_rsa_padding_mode_e padding, EHSM_SHM ehsm_session_st *session);

/**
 * @brief RSA签名/验签计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
 * @param[in] key 用于RSA签名/验签计算的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format 和 @ref
 * ehsm_rsa_key_st
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_rsa_sign_finish_gen 或 @ref ehsm_rsa_sign_finish_verify
 * 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_rsa_sign_update 、 @ref
 * ehsm_rsa_sign_finish_gen 和 @ref ehsm_rsa_sign_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, bool_t gen_sig, ehsm_rsa_padding_mode_e padding, EHSM_SHM ehsm_session_st *session);

/**
 * @brief RSA签名/验签计算，更新数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_rsa_sign_init 或 @ref ehsm_rsa_sign_update
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief RSA签名/验签计算，生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_rsa_sign_init 或 @ref ehsm_rsa_sign_update
 * @param[out] sig 保存签名值的buffer
 * @param[in,out] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，生成的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_finish_gen(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *sig, uint32_t *sig_size, uint32_t salt_size);

/**
 * @brief RSA签名/验签计算，验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_rsa_sign_init 或 @ref ehsm_rsa_sign_update
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，待验签数据的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_finish_verify(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *sig, uint32_t sig_size,
    uint32_t salt_size, bool_t *verify_result);

/**
 * @brief RSA签名生成/验证扩展接口，支持明文密钥、密钥句柄、摘要输入等多种模式的一次性签名操作。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法，见 @ref ehsm_hash_algo_e
 * @param[in] use_plain_key 是否使用明文密钥
 *              - `true` 使用明文密钥，key参数有效，key_handle参数被忽略
 *              - `false` 使用密钥句柄，key_handle参数有效，key参数被忽略
 * @param[in] key_handle RSA密钥句柄，当use_plain_key为false时使用
 * @param[in] key 用于签名/验证的明文密钥，当use_plain_key为true时使用，格式见 @ref ehsm_rsa_key_st
 * @param[in] padding RSA填充模式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] gen_sig 操作方式
 *              - `true` 生成签名值
 *              - `false` 验证签名值
 * @param[in] is_digest 输入数据类型
 *              - `true` input为已计算好的消息摘要，长度由algo参数决定
 *              - `false` input为原始消息数据，由eHSM内部计算摘要
 * @param[in] input 输入数据，根据is_digest参数决定是消息原文还是消息摘要
 * @param[in] input_size 输入数据长度
 * @param[in,out] sig_addr 签名值的地址
 *              - 生成签名时：作为输出buffer的地址，存放生成的签名值
 *              - 验证签名时：作为输入buffer的地址，存放待验证的签名值
 * @param[in,out] sig_size 签名buffer长度
 *              - 生成签名时：输入时为buffer长度，输出时为实际签名数据长度
 *              - 验证签名时：输入时为待验证的签名数据长度
 * @param[in] salt_size PSS填充模式使用的salt长度，其他填充模式应设置为0
 * @param[out] verify_result 验证结果存放地址，仅在gen_sig为false时有效
 *              - `true` 验证通过
 *              - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_rsa_sign_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, bool_t use_plain_key,
    uint32_t key_handle, EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, bool_t gen_sig, bool_t is_digest,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uintptr_t sig_addr, uint32_t *sig_size,
    uint32_t salt_size, bool_t *verify_result);

/**
 * @brief RSA签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
 * @param[in] key_handle RSA密钥句柄
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，生成的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle,
    ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig,
    uint32_t *sig_size, uint32_t salt_size)
{
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, padding,
        /*gen_sig=*/true, /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, salt_size,
        /*verify_result=*/NULL);
}

/**
 * @brief RSA验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，SM3和SHA3系列不支持
 * @param[in] key_handle RSA密钥句柄
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，待验签数据的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    uint32_t key_handle, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *sig, uint32_t sig_size, uint32_t salt_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, padding,
        /*gen_sig=*/false, /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, &sig_size_tmp, salt_size, verify_result);
}

/**
 * @brief RSA签名值生成，一次计算完成, 外部传入计算好的消息摘要方式。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
                          该参数仅PSS填充模式需要输入，需要输入的hash算法需要和计算消息的摘要算法一致。
 * @param[in] key_handle RSA密钥句柄
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要数据长度，最大支持64字节长度。
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，生成的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_gen_with_digest(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    uint32_t key_handle, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM uint8_t *sig, uint32_t *sig_size, uint32_t salt_size)
{
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, padding,
        /*gen_sig=*/true, /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, salt_size,
        /*verify_result=*/NULL);
}

/**
 * @brief RSA验证签名值，一次计算完成, 外部传入计算好的消息摘要方式。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，SM3和SHA3系列不支持，
                          该参数仅PSS填充模式需要输入，需要输入的hash算法需要和计算消息的摘要算法一致。
 * @param[in] key_handle RSA密钥句柄
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要数据长度，最大支持64字节长度。
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，待验签数据的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_verify_with_digest(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    uint32_t key_handle, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM const uint8_t *sig, uint32_t sig_size, uint32_t salt_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, padding,
        /*gen_sig=*/false, /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, &sig_size_tmp, salt_size,
        verify_result);
}

/**
 * @brief RSA签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
 * @param[in] key 用于RSA签名值生成的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_rsa_key_st
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，生成的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM uint8_t *sig, uint32_t *sig_size, uint32_t salt_size)
{
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, key, padding,
        /*gen_sig=*/true, /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, salt_size,
        /*verify_result=*/NULL);
}

/**
 * @brief RSA验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，SM3和SHA3系列不支持
 * @param[in] key 用于RSA验证签名值的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_rsa_key_st
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，待验签数据的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *sig, uint32_t sig_size, uint32_t salt_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, key, padding,
        /*gen_sig=*/false, /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, &sig_size_tmp, salt_size, verify_result);
}

/**
 * @brief RSA签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，MD5、SM3和SHA3系列不支持
 * @param[in] key 用于RSA签名值生成的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_rsa_key_st
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度
 * @param[out] sig 保存签名值的buffer
 * @param[in,out] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，生成的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_gen_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *digest,
    uint32_t digest_size, EHSM_SHM uint8_t *sig, uint32_t *sig_size, uint32_t salt_size)
{
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, key, padding,
        /*gen_sig=*/true, /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, salt_size,
        /*verify_result=*/NULL);
}

/**
 * @brief RSA验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，仅支持 SHA1 和 SHA2 系列算法，SM3和SHA3系列不支持
 * @param[in] key 用于RSA验证签名值的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_rsa_key_st
 * @param[in] padding 填充方式，见 @ref ehsm_rsa_padding_mode_e
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[in] salt_size 当填充方式为 `EHSM_RSA_PADDING_PSS` 时，待验签数据的盐值的长度；当填充方式为
 * `EHSM_RSA_PADDING_NONE`时， 'salt_size' 应为0
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_rsa_sign_onepass_verify_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, EHSM_SHM const uint8_t *digest,
    uint32_t digest_size, EHSM_SHM const uint8_t *sig, uint32_t sig_size, uint32_t salt_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_rsa_sign_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, key, padding,
        /*gen_sig=*/false, /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, &sig_size_tmp, salt_size,
        verify_result);
}

/**
 * @brief ECDSA签名/验签计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key_handle ECDSA密钥句柄
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_ecdsa_finish_gen 或 @ref
 * ehsm_ecdsa_finish_verify 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_ecdsa_update 、 @ref
 * ehsm_ecdsa_finish_gen 和 @ref ehsm_ecdsa_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_sig,
    EHSM_SHM ehsm_session_st *session);

/**
 * @brief ECDSA签名/验签计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key 用于ECDSA签名/验签计算的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format 和
 * @ref ehsm_ecc_key_st
 * @param[in] gen_sig 是否生成签名，必须跟最后调用的 @ref ehsm_ecdsa_finish_gen 或 @ref
 * ehsm_ecdsa_finish_verify 相匹配
 *          - `true` 生成签名值
 *          - `false` 验证签名值
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_ecdsa_update 、 @ref
 * ehsm_ecdsa_finish_gen 和 @ref ehsm_ecdsa_finish_verify 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    bool_t gen_sig, EHSM_SHM ehsm_session_st *session);

/**
 * @brief ECDSA签名/验签计算，更新数据。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ecdsa_init 或 @ref ehsm_ecdsa_update
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_update(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size);

/**
 * @brief ECDSA签名/验签计算，生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ecdsa_init 或 @ref ehsm_ecdsa_update
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_finish_gen(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *sig, uint32_t *sig_size);

/**
 * @brief ECDSA签名/验签计算，验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ecdsa_init 或 @ref ehsm_ecdsa_update
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_finish_verify(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *sig, uint32_t sig_size, bool_t *verify_result);

/**
 * @brief ECDSA签名值生成或验证，一次计算完成。
 *
 * @note 此函数提供了最大的灵活性，但参数较多，建议使用对应的封装版函数。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] use_plain_key 是否使用外部输入的明文密钥值进行签名生成或验证
 * @param[in] key_handle ECDSA密钥句柄，当 `use_plain_key` 为 `false` 时有效，否则忽略
 * @param[in] key ECDSA公钥或私钥值，当 `use_plain_key` 为 `true` 时有效，否则忽略
 * @param[in] gen_sig 是进行签名的生成，还是验证
 * @param[in] is_digest 输入的数据是已计算好的消息摘要值，还是消息明文
 * @param[in] input 输入数据地址
 * @param[in] input_size 输入数据长度
 * @param[in] sig_addr
 * 在生成签名时，此地址表示保存输出的签名值的buffer地址；在验证签名时，此地址表示用于验证的签名值数据地址
 * @param[in,out] sig_size
 * 在生成签名时，此地址保存了输出buffer的长度，并在计算完成后更新为签名值的长度；在验证签名时，此地址应当保存签名值的长度
 * @param[out] verify_result 在验证签名时，存放验证结果的内存位置（生成签名时忽略此参数）
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_ecdsa_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, bool_t use_plain_key,
    uint32_t key_handle, EHSM_SHM const uint8_t *key, bool_t gen_sig, bool_t is_digest, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uintptr_t sig_addr, uint32_t *sig_size, bool_t *verify_result);
/**
 * @brief ECDSA签名值生成，一次计算完成。
 *
 * 此函数使用 key handle 和原始消息数据生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key_handle ECDSA密钥句柄
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t *sig_size)
{
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/true,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief ECDSA验证签名值，一次计算完成。
 *
 * 此函数使用 key handle 和原始消息数据验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key_handle ECDSA密钥句柄
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *sig, uint32_t sig_size,
    bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/false,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, &sig_size_tmp, verify_result);
}

/**
 * @brief ECDSA签名值生成，一次计算完成。
 *
 * 此函数使用 key handle 和消息数据摘要值生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法，见 @ref ehsm_hash_algo_e
 * @param[in] key_handle ECDSA密钥句柄
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度,最大长度是64字节
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_gen_with_digest(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    uint32_t key_handle, EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM uint8_t *sig,
    uint32_t *sig_size)
{
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/true,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief ECDSA验证签名值，一次计算完成。
 *
 * 此函数使用 key handle 和消息数据摘要值验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo HASH算法，见 @ref ehsm_hash_algo_e
 * @param[in] key_handle ECDSA密钥句柄
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度,最大长度是64字节
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_verify_with_digest(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    uint32_t key_handle, EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM const uint8_t *sig,
    uint32_t sig_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/false, key_handle, /*key=*/NULL, /*gen_sig=*/false,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, &sig_size_tmp, verify_result);
}

/**
 * @brief ECDSA签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key 用于ECDSA签名值生成的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_ecc_key_st
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig,
    uint32_t *sig_size)
{
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, /*key=*/key, /*gen_sig=*/true,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief ECDSA验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key 用于ECDSA验证签名值的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_ecc_key_st
 * @param[in] msg 输入更新的消息数据
 * @param[in] msg_size 输入的消息数据长度
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo,
    EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM const uint8_t *sig,
    uint32_t sig_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, /*key=*/key, /*gen_sig=*/false,
        /*is_digest=*/false, msg, msg_size, (uintptr_t)sig, &sig_size_tmp, verify_result);
}

/**
 * @brief ECDSA签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key 用于ECDSA签名值生成的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_ecc_key_st
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度,最大长度是64字节
 * @param[out] sig 保存签名值的buffer
 * @param[in,out] sig_size 输入时为buffer长度，完成时为写入buffer中的签名数据长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_gen_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM uint8_t *sig, uint32_t *sig_size)
{
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, /*key=*/key, /*gen_sig=*/true,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, sig_size, /*verify_result=*/NULL);
}

/**
 * @brief ECDSA验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo 签名对应的HASH算法，见 @ref ehsm_hash_algo_e ，支持所有HASH算法
 * @param[in] key 用于ECDSA验证签名值的明文密钥，参考 @ref plain-key-format 和 @ref ehsm_ecc_key_st
 * @param[in] digest 输入的消息摘要数据
 * @param[in] digest_size 输入的消息摘要长度,最大长度是64字节
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_ecdsa_onepass_verify_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM uint8_t *sig, uint32_t sig_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sig_size;
    return ehsm_ecdsa_onepass_ex(ctx, algo, /*use_plain_key=*/true, /*key_handle=*/0, /*key=*/key, /*gen_sig=*/false,
        /*is_digest=*/true, digest, digest_size, (uintptr_t)sig, &sig_size_tmp, verify_result);
}

/**
 * @brief SM9加密/解密计算，一次完成计算。
 *
 * @note SM9加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM9密钥句柄, 解密有效，加密时传入0
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] enc_type 加密类型 @ref ehsm_sm9_enc_type_e
 * @param[in] padding padding类型 @ref ehsm_sm9_padding_mode_e ，在enc_type为EHSM_SM9_ENC_TYPE_BLOCK的情况有效
 * @param[in] key2_size SM9内部计算MAC的密钥长度
 * @param[in] hid SM9加密密钥生成标识，默认取值为0x3
 * @param[in] kgc_pub_key KGC公钥数据，长度为64字节
 * @param[in] input 输入的明文或密文，密文的格式为C1C3C2，其中
 *          - C1为曲线上的点，65字节，以`0x04`开头，以及完整 x 和 y 坐标
 *          - C2为实际的密文数据，与对应的明文长度相同
 *          - C3为SM3的结果，32字节，用于保证完整性
 * @param[in] input_size 输入数据长度, 不能大于1024
 * @param[in] output 输出数据的buffer
 * @param[in,out] output_size 传入时为输出buffer的长度，如果是加密，应大于等于`input_size + 97`；如果是解密，应大于等于
 * `input_size - 97`, 完成时为实际的输出数据长度
 * @param[in] id 用户的ID
 * @param[in] id_size 用户ID的长度，长度必须小于1024字节
 * @param[in] fp12g SM9算法内部中间值，长度为384字节,可以传空值，如果是空值，HSM内部进行计算
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm9_cipher(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, ehsm_sm9_enc_type_e enc_type,
    ehsm_sm9_padding_mode_e padding, uint8_t key2_size, uint8_t hid, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size,
    EHSM_SHM const uint8_t *id, uint32_t id_size, EHSM_SHM const uint8_t *fp12g);

/**
 * @brief SM9加密/解密计算，一次完成计算，解密时私钥使用明文密钥。
 *
 * @note SM9加解密不支持三段式调用。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key SM9用户私钥, 解密有效，加密时传入NULL，参考 @ref plain-key-format
 * @param[in] enc 是否加密
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] enc_type 加密类型 @ref ehsm_sm9_enc_type_e
 * @param[in] padding padding类型 @ref ehsm_sm9_padding_mode_e ，在enc_type为EHSM_SM9_ENC_TYPE_BLOCK的情况有效
 * @param[in] key2_size SM9内部计算MAC的密钥长度
 * @param[in] hid SM9加密密钥生成标识，默认取值为0x3
 * @param[in] kgc_pub_key KGC公钥数据，长度为64字节
 * @param[in] input 输入的明文或密文，密文的格式为C1C3C2，其中
 *          - C1为曲线上的点，65字节，以`0x04`开头，以及完整 x 和 y 坐标
 *          - C2为实际的密文数据，与对应的明文长度相同
 *          - C3为SM3的结果，32字节，用于保证完整性
 * @param[in] input_size 输入数据长度, 不能大于1024
 * @param[in] output 输出数据的buffer
 * @param[in,out] output_size 传入时为输出buffer的长度，如果是加密，应大于等于`input_size + 97`；如果是解密，应大于等于
 * `input_size - 97`, 完成时为实际的输出数据长度
 * @param[in] id 用户的ID
 * @param[in] id_size 用户ID的长度，长度必须小于1024字节
 * @param[in] fp12g SM9算法内部中间值，长度为384字节,可以传空值，如果是空值，HSM内部进行计算
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm9_cipher_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc,
    ehsm_sm9_enc_type_e enc_type, ehsm_sm9_padding_mode_e padding, uint8_t key2_size, uint8_t hid,
    EHSM_SHM const uint8_t *kgc_pub_key, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    uint32_t *output_size, EHSM_SHM const uint8_t *id, uint32_t id_size, EHSM_SHM const uint8_t *fp12g);

/**
 * @brief SM9签名值生成，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle SM9密钥句柄
 * @param[in] msg 输入的消息数据
 * @param[in] msg_size 输入的消息数据长度，不能大于1024
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于97
 * @param[in] kgc_pub_key KGC公钥数据，长度为64字节
 * @param[in] fp12g SM9算法内部中间值，可以传空值，如果是空值，HSM内部进行计算
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm9_sign_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *msg,
    uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *fp12g);

/**
 * @brief SM9验证签名值，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] msg 输入的消息数据
 * @param[in] msg_size 输入的消息数据长度，不能大于1024
 * @param[in] id 用户的ID
 * @param[in] id_size 用户ID的长度，不能大于1024字节
 * @param[in] hid SM9签名密钥生成标识，默认取值为0x1
 * @param[in] kgc_pub_key KGC公钥数据，长度为64字节
 * @param[in] fp12g SM9算法内部中间值，可以传空值，如果是空值，HSM内部进行计算
 * @param[in] sig 待验证的签名值
 * @param[in] sig_size 签名值长度，仅接受97字节签名值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm9_sign_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *id, uint32_t id_size, uint8_t hid, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *fp12g, EHSM_SHM const uint8_t *sig, uint32_t sig_size, bool_t *verify_result);

/**
 * @brief SM9签名值生成，一次计算完成，使用明文密钥。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key SM9用户私钥，参考 @ref plain-key-format
 * @param[in] msg 输入的消息数据
 * @param[in] msg_size 输入的消息数据长度，不能大于1024
 * @param[out] sig 保存签名值的buffer
 * @param[in] sig_size 签名buffer的长度，不能小于97
 * @param[in] kgc_pub_key KGC公钥数据，长度为64字节
 * @param[in] fp12g SM9算法内部中间值，可以传空值，如果是空值，HSM内部进行计算
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_sm9_sign_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size,
    EHSM_SHM const uint8_t *kgc_pub_key, EHSM_SHM const uint8_t *fp12g);

/**
 * @brief CHACHA加密/解密计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle 对称密钥句柄
 * @param[in] enc 是否加密，此参数必须与最后调用的 @ref ehsm_chacha_finish_enc 或 @ref ehsm_chacha_finish_dec 相匹配
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] nonce nonce值
 * @param[in] nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in,out] session 用于ehsm内部缓存CHACHA计算中间值的 buffer，此参数指向的内存在 @ref ehsm_chacha_update 、 @ref
 * ehsm_chacha_finish_enc 和 @ref ehsm_chacha_finish_dec 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_init(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM ehsm_session_st *session);

/**
 * @brief CHACHA加密/解密计算，更新数据。
 *
 * 此函数处理连续的块数据，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_chacha_init 初始化
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，严格要求 block size 对齐
            - 若为加密，则必须保留至少一个字节的输入数据给 @ref ehsm_chacha_finish_enc 使用；
            - 若为解密，则必须保留至少一个字节的输入数据给 @ref ehsm_chacha_finish_dec 使用；
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_update(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output);

/**
 * @brief CHACHA加密/解密计算，计算结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_chacha_init 或 @ref ehsm_chacha_update
 * @param[in] input 输入数据，不能为空
 * @param[in] input_size 输入数据长度，不能为0，不要求对齐block size
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer，其空间大小不能小于tag_size
 * @param[in] tag_size 指定生成的TAG值长度，必须为16字节
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_finish_enc(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag, uint32_t tag_size);

/**
 * @brief CHACHA加密/解密计算，验证结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_chacha_init 或 @ref ehsm_chacha_update
 * @param[in] input 输入数据，不能为空
 * @param[in] input_size 输入数据长度，不能为0，不要求对齐block size
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[in] tag_size TAG值长度，必须为16字节
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_finish_dec(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result);

/**
 * @brief CHACHA加密计算，一次计算完成。
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle 对称密钥句柄
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer
 * @param[in] tag_size 指定生成的TAG值长度，必须为16字节
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_onepass_enc(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag,
    uint32_t tag_size);

/**
 * @brief CHACHA解密计算，一次计算完成。
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle 对称密钥句柄
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[in] tag_size 指定生成的TAG值长度，必须为16字节
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_onepass_dec(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag,
    uint32_t tag_size, bool_t *verify_result);

/**
 * @brief CHACHA加密/解密计算，初始化，使用明文密钥
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于CHACHA加密/解密计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] enc 是否加密，此参数必须与最后调用的 @ref ehsm_chacha_finish_enc 或 @ref ehsm_chacha_finish_dec 相匹配
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] nonce nonce值
 * @param[in] nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in,out] session 用于ehsm内部缓存CHACHA计算中间值的 buffer，此参数指向的内存在 @ref ehsm_chacha_update 、 @ref
 * ehsm_chacha_finish_enc 和 @ref ehsm_chacha_finish_dec 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, uint32_t key_size,
    bool_t enc, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size,
    uint32_t constant, EHSM_SHM ehsm_session_st *session);

/**
 * @brief CHACHA加密计算，一次计算完成，使用明文密钥
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于CHACHA加密计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer
 * @param[in] tag_size 指定生成的TAG值长度，必须为16字节
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_onepass_enc_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, uint32_t constant, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM uint8_t *tag, uint32_t tag_size);

/**
 * @brief CHACHA解密计算，一次计算完成，使用明文密钥
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key 用于CHACHA解密计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce_size 必须为8字节
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度
 * @param[in] constant 计数器初始值
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[in] tag_size 指定生成的TAG值长度，必须为16字节
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_chacha_onepass_dec_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, uint32_t constant, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result);

/**
 * @brief AEAD加密/解密计算，初始化。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key_handle 对称密钥句柄
 * @param[in] enc 是否加密，此参数必须与最后调用的 @ref ehsm_aead_finish_enc 或 @ref ehsm_aead_finish_dec 相匹配
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] data_size 后续调用 @ref ehsm_aead_update 更新的所有数据的长度之和，必须与后续实际更新的数据长度相等
 * @param[in] tag_size 指定生成或校验的TAG值长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_aead_update 、 @ref
 * ehsm_aead_finish_enc 和 @ref ehsm_aead_finish_dec 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_init(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode, uint32_t key_handle,
    bool_t enc, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size,
    uint32_t data_size, uint32_t tag_size, EHSM_SHM ehsm_session_st *session);

/**
 * @brief AEAD加密/解密计算，初始化，使用明文密钥
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key 用于AEAD加密/解密计算的明文密钥，对应的内存在finish接口之前不能释放，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] enc 是否加密，此参数必须与最后调用的 @ref ehsm_aead_finish_enc 或 @ref ehsm_aead_finish_dec 相匹配
 *          - `true` 加密
 *          - `false` 解密
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] data_size 后续调用 @ref ehsm_aead_update 更新的所有数据的长度之和，必须与后续实际更新的数据长度相等
 * @param[in] tag_size 指定生成或校验的TAG值长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 * @param[in,out] session 用于ehsm内部缓存HASH计算中间值的 buffer，此参数指向的内存在 @ref ehsm_aead_update 、 @ref
 * ehsm_aead_finish_enc 和 @ref ehsm_aead_finish_dec 中仍会使用，因此需保证其有效性，不能提前释放。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size,
    EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t data_size, uint32_t tag_size,
    EHSM_SHM ehsm_session_st *session);

/**
 * @brief AEAD加密/解密计算，更新数据。
 *
 * 此函数处理连续的块数据，输入数据长度必须block size对齐，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_aead_init 初始化
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，必须 block size 对齐
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_update(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output);

/**
 * @brief AEAD加密/解密计算，计算结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_aead_init 或 @ref ehsm_aead_update
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，不要求对齐block size
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer，其空间大小不能小于 @ref ehsm_aead_init 所传入的参数 `tag_size`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_finish_enc(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag);

/**
 * @brief AEAD加密/解密计算，验证结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_aead_init 或 @ref ehsm_aead_update
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，不要求对齐block size
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_finish_dec(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag, bool_t *verify_result);

/**
 * @brief AEAD加密计算，一次计算完成。
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key_handle 对称密钥句柄
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer
 * @param[in] tag_size 指定生成的TAG值长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_onepass_enc(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    uint32_t key_handle, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM uint8_t *tag, uint32_t tag_size);

/**
 * @brief AEAD解密计算，一次计算完成。
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key_handle 对称密钥句柄
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[in] tag_size 输入的TAG长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_onepass_dec(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    uint32_t key_handle, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result);

/**
 * @brief AEAD加密计算，一次计算完成，使用明文密钥
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key 用于AEAD加密计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[out] tag 存入生成的TAG值的buffer
 * @param[in] tag_size 指定生成的TAG值长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_onepass_enc_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size,
    EHSM_SHM const uint8_t *aad, uint32_t aad_size, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag, uint32_t tag_size);

/**
 * @brief AEAD解密计算，一次计算完成，使用明文密钥
 *
 * 此函数处理连续的块数据，输入数据长度为任意长度，实际写入 `output` 中的数据与输入数据长度相同。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] algo AEAD使用的对称算法类型，见 @ref ehsm_symm_algo_e ，不支持DES和TDES
 * @param[in] mode AEAD模式，见 @ref ehsm_aead_mode_e
 * @param[in] key 用于AEAD解密计算的明文密钥，参考 @ref plain-key-format
 * @param[in] key_size 明文密钥字节数
 * @param[in] nonce nonce值
 * @param[in] nonce_size nonce值的长度，对于GCM，必须为12字节，对于CCM，取值范围是 7~13
 * @param[in] aad 附加数据，可以为NULL
 * @param[in] aad_size 附加数据长度，不能大于128
 * @param[in] input 输入数据
 * @param[in] input_size 输入数据长度，可以是任意长度，包括0
 * @param[out] output 输出buffer，其空间大小不能小于 `input_size`
 * @param[in] tag 输入的TAG值
 * @param[in] tag_size 输入的TAG长度，对于GCM，取值范围是 1~16，对于CCM，取值范围是{4,6,8,10,12,14,16}
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_aead_onepass_dec_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size,
    EHSM_SHM const uint8_t *aad, uint32_t aad_size, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result);

// #% #if CONFIG_HOST_PQC_EN
/**
 * @brief PQC 签名生成或验证，一次计算完成。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] is_det 是否使用确定性签名
 * - `false`: 非确定性签名，由 HSM 生成签名需要的随机数（推荐，生产环境使用）
 * - `true`: 确定性签名，对于 ML-DSA 使用 32 字节全 0 代替随机数，对于 SLH-DSA 使用 PK.seed 代替随机数
 * @param[in] use_plain_key 是否使用外部输入的明文密钥值进行签名生成或验证
 * @param[in] key_handle PQC 密钥句柄，当 `use_plain_key` 为 `false` 时有效，否则忽略
 * - 签名生成时
 *      - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，则需要有私钥
 *      - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，则需要有密钥对
 * - 签名验证时需要包含密钥算法 ID 和公钥
 * @param[in] key PQC 明文密钥数据，见 @ref ehsm_pqc_key_st。当 `use_plain_key` 为 `true` 时有效，否则忽略
 * - 签名生成时
 *      - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，则需要包含密钥类型和私钥
 *      - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，则需要包含密钥类型、私钥和公钥
 * - 签名验证时需要包含密钥算法 ID 和公钥
 * @param[in] gen_sig 是进行签名的生成，还是验证
 * @param[in] input 输入数据的 buffer，当 `input_size` 为 0 时可以为 `NULL`（空消息签名）
 * @param[in] input_size buffer 中输入数据的字节长度，可以为 0（空消息签名，符合 FIPS 204/205 标准）
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息/消息摘要一起编码后，进行签名生成或验证，可以为
 * `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[in] sig_addr
 * 在生成签名时，此地址表示保存输出的签名值的 buffer 地址；在验证签名时，此地址表示用于验证的签名值数据地址
 * @param[in,out] sign_size
 * 生成签名时，此地址保存了输出 buffer 的字节长度，计算完成后更新为签名值的长度；验证签名时，此地址应当为签名值的长度
 * @param[out] verify_result 在验证签名时，存放验证结果的内存位置（生成签名时忽略此参数）
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 长度信息：
 * | 算法                     |  公钥长度  | 私钥长度  | 签名值长度 |
 * |--------------------------|-----------|-----------|-----------|
 * | ML-DSA-44                | 1312字节  | 2560字节  | 2420字节  |
 * | ML-DSA-65                | 1952字节  | 4032字节  | 3309字节  |
 * | ML-DSA-87                | 2592字节  | 4896字节  | 4627字节  |
 * | SLH-DSA-SHA2/SHAKE-128s  | 32字节    | 32字节    | 7856字节  |
 * | SLH-DSA-SHA2/SHAKE-128f  | 32字节    | 32字节    | 17088字节 |
 * | SLH-DSA-SHA2/SHAKE-192s  | 48字节    | 48字节    | 16224字节 |
 * | SLH-DSA-SHA2/SHAKE-192f  | 48字节    | 48字节    | 35664字节 |
 * | SLH-DSA-SHA2/SHAKE-256s  | 64字节    | 64字节    | 29792字节 |
 * | SLH-DSA-SHA2/SHAKE-256f  | 64字节    | 64字节    | 49856字节 |
 * - **Context字符串长度限制**: 0-255字节 (符合FIPS 204/205标准)
 * - **空消息签名**: 支持对空消息（`input=NULL, input_size=0`）进行签名和验证，符合 FIPS 204/205 标准
 */
uint32_t ehsm_pqc_dsa_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_pqc_sign_algo_e sign_algo,
    ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, bool_t is_det, bool_t use_plain_key,
    uint32_t key_handle, EHSM_SHM const ehsm_pqc_key_st *key, bool_t gen_sig, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size, EHSM_SHM uintptr_t sig_addr,
    uint32_t *sign_size, bool_t *verify_result);

/**
 * @brief PQC 签名值生成，一次计算完成，使用 key_handle 指定密钥，对原始消息生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] is_det 是否使用确定性签名
 * - `false`: 非确定性签名，由 HSM 生成签名需要的随机数（推荐，生产环境使用）
 * - `true`: 确定性签名，对于 ML-DSA 使用 32 字节全 0 代替随机数，对于 SLH-DSA 使用 PK.seed 代替随机数
 * @param[in] key_handle 用于 PQC 签名值生成的密钥句柄
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，对应的密钥中需包含私钥
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，对应的密钥中需包含密钥对
 * @param[in] msg 输入消息的 buffer，当 `msg_size` 为 0 时可以为 `NULL`（空消息签名）
 * @param[in] msg_size buffer 中消息的字节长度，可以为 0（空消息签名）
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息编码后一起生成签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[out] sign 保存签名值的 buffer
 * @param[in,out] sign_size 输入时为 buffer 的字节长度，完成时为写入 buffer 中的签名值的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 */
static inline uint32_t ehsm_pqc_dsa_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, ehsm_pqc_sign_algo_e sign_algo,
    ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, bool_t is_det, uint32_t key_handle,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size,
    EHSM_SHM uint8_t *sign, uint32_t *sign_size)
{
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, is_det,
        /*use_plain_key=*/false, key_handle,
        /*key=*/NULL,
        /*gen_sig=*/true, msg, msg_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, sign_size,
        /*verify_result=*/NULL);
}

/**
 * @brief PQC 签名值验证，一次计算完成，使用 key_handle 指定密钥，对原始消息验证签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] key_handle 用于 PQC 签名验证的密钥句柄，对应的密钥中必须包含公钥
 * @param[in] msg 输入消息的 buffer，当 `msg_size` 为 0 时可以为 `NULL`（空消息签名）
 * @param[in] msg_size buffer 中消息的字节长度，可以为 0（空消息签名）
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息编码后一起验证签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[in] sign 输入签名值的 buffer
 * @param[in] sign_size buffer 中的签名值的字节长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 * @note 对于签名验证操作（direction=false），is_det 参数会被忽略
 */
static inline uint32_t ehsm_pqc_dsa_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, ehsm_pqc_sign_algo_e sign_algo,
    ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, uint32_t key_handle, EHSM_SHM const uint8_t *msg,
    uint32_t msg_size, EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size, EHSM_SHM uint8_t *sign,
    uint32_t sign_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sign_size;
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, /*is_det=*/false,
        /*use_plain_key=*/false, key_handle,
        /*key=*/NULL,
        /*gen_sig=*/false, msg, msg_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, &sig_size_tmp, verify_result);
}

/**
 * @brief PQC 签名值生成，一次计算完成，使用 key_handle 指定密钥，对消息摘要生成签名值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] is_det 是否使用确定性签名
 * - `false`: 非确定性签名，由 HSM 生成签名需要的随机数（推荐，生产环境使用）
 * - `true`: 确定性签名，对于 ML-DSA 使用 32 字节全 0 代替随机数，对于 SLH-DSA 使用 PK.seed 代替随机数
 * @param[in] key_handle 用于 PQC 签名值生成的密钥句柄
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，对应的密钥中需包含私钥
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，对应的密钥中需包含密钥对
 * @param[in] digest 输入消息摘要的 buffer
 * @param[in] digest_size buffer 中消息摘要的字节长度
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息摘要编码后一起生成签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[out] sign 保存签名值的 buffer
 * @param[in,out] sign_size 输入时为 buffer 的字节长度，完成时为写入 buffer 中的签名值的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 */
static inline uint32_t ehsm_pqc_dsa_onepass_gen_with_digest(EHSM_SHM ehsm_ctx_st *ctx, ehsm_pqc_sign_algo_e sign_algo,
    ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, bool_t is_det, uint32_t key_handle,
    EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size,
    EHSM_SHM uint8_t *sign, uint32_t *sign_size)
{
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, is_det,
        /*use_plain_key=*/false, key_handle,
        /*key=*/NULL,
        /*gen_sig=*/true, digest, digest_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, sign_size,
        /*verify_result=*/NULL);
}

/**
 * @brief PQC 签名值验证，一次计算完成，使用 key_handle 指定密钥，对消息摘要验证签名。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] key_handle PQC 密钥句柄，对应的密钥中必须包含公钥
 * @param[in] digest 输入消息摘要的 buffer
 * @param[in] digest_size buffer 中输入的消息摘要的字节长度
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息摘要编码后一起验证签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[in] sign 输入签名值的 buffer
 * @param[in] sign_size buffer 中的签名值的字节长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 * @note 对于签名验证操作（direction=false），is_det 参数会被忽略
 */
static inline uint32_t ehsm_pqc_dsa_onepass_verify_with_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_pqc_sign_algo_e sign_algo, ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, uint32_t key_handle,
    EHSM_SHM const uint8_t *digest, uint32_t digest_size, EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size,
    EHSM_SHM uint8_t *sign, uint32_t sign_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sign_size;
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, /*is_det=*/false,
        /*use_plain_key=*/false, key_handle,
        /*key=*/NULL,
        /*gen_sig=*/false, digest, digest_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, &sig_size_tmp,
        verify_result);
}

/**
 * @brief PQC 签名值生成，一次计算完成，由 Host 直接输入明文密钥，对原始消息生成签名。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] is_det 是否使用确定性签名
 * - `false`: 非确定性签名，由 HSM 生成签名需要的随机数（推荐，生产环境使用）
 * - `true`: 确定性签名，对于 ML-DSA 使用 32 字节全 0 代替随机数，对于 SLH-DSA 使用 PK.seed 代替随机数
 * @param[in] key PQC 明文密钥，见 @ref ehsm_pqc_key_st。
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，对应的密钥中需包含密钥算法 ID 和私钥
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，对应的密钥中需包含密钥算法 ID 和密钥对
 * @param[in] msg 输入消息的 buffer，当 `msg_size` 为 0 时可以为 `NULL`（空消息签名）
 * @param[in] msg_size buffer 中输入的消息的字节长度，可以为 0（空消息签名）
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息编码后一起生成签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[out] sign 保存签名值的 buffer
 * @param[in,out] sign_size 输入时为 buffer 的字节长度，完成时为写入 buffer 中的签名值的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 */
static inline uint32_t ehsm_pqc_dsa_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_pqc_sign_algo_e sign_algo, ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, bool_t is_det,
    EHSM_SHM const ehsm_pqc_key_st *key, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *pqc_ctx_str,
    uint32_t pqc_ctx_str_size, EHSM_SHM uint8_t *sign, uint32_t *sign_size)
{
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, is_det,
        /*use_plain_key=*/true, /*key_handle=*/0U, key,
        /*gen_sig=*/true, msg, msg_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, sign_size,
        /*verify_result=*/NULL);
}

/**
 * @brief PQC 签名值验证，一次计算完成，由 Host 直接输入明文密钥，对原始消息验证签名。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] key PQC 明文密钥，见 @ref ehsm_pqc_key_st。必须包含密钥算法 ID 和公钥
 * @param[in] msg 输入消息的 buffer，当 `msg_size` 为 0 时可以为 `NULL`（空消息签名）
 * @param[in] msg_size buffer 中输入的消息的字节长度，可以为 0（空消息签名）
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息编码后一起验证签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[in] sign 输入签名值的 buffer
 * @param[in] sign_size buffer 中的签名值的字节长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 * @note 对于签名验证操作（direction=false），is_det 参数会被忽略
 */
static inline uint32_t ehsm_pqc_dsa_onepass_verify_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_pqc_sign_algo_e sign_algo, ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo,
    EHSM_SHM const ehsm_pqc_key_st *key, EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *pqc_ctx_str,
    uint32_t pqc_ctx_str_size, EHSM_SHM uint8_t *sign, uint32_t sign_size, bool_t *verify_result)
{
    uint32_t sig_size_tmp = sign_size;
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, /*is_det=*/false,
        /*use_plain_key=*/true, /*key_handle=*/0U, key,
        /*gen_sig=*/false, msg, msg_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, &sig_size_tmp, verify_result);
}

/**
 * @brief PQC 签名值生成，一次计算完成，由 Host 直接输入明文密钥，对消息摘要生成签名。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] is_det 是否使用确定性签名
 * - `false`: 非确定性签名，由 HSM 生成签名需要的随机数（推荐，生产环境使用）
 * - `true`: 确定性签名，对于 ML-DSA 使用 32 字节全 0 代替随机数，对于 SLH-DSA 使用 PK.seed 代替随机数
 * @param[in] key PQC 明文密钥，见 @ref ehsm_pqc_key_st。
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_ML_DSA`，对应的密钥中需包含密钥算法 ID 和私钥
 * - 若 `sign_algo` 为 `EHSM_PQC_DSA_ALGO_SLH_DSA`，对应的密钥中需包含密钥算法 ID 和密钥对
 * @param[in] digest 输入消息摘要的 buffer
 * @param[in] digest_size buffer 中输入的消息摘要的字节长度
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息摘要编码后一起生成签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[out] sign 保存签名值的 buffer
 * @param[in,out] sign_size 输入时为 buffer 的字节长度，完成时为写入 buffer 中的签名值的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 */
static inline uint32_t ehsm_pqc_dsa_onepass_gen_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_pqc_sign_algo_e sign_algo, ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo, bool_t is_det,
    EHSM_SHM const ehsm_pqc_key_st *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size, EHSM_SHM uint8_t *sign, uint32_t *sign_size)
{
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, is_det,
        /*use_plain_key=*/true, /*key_handle=*/0U, key,
        /*gen_sig=*/true, digest, digest_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, sign_size,
        /*verify_result=*/NULL);
}

/**
 * @brief PQC 签名值验证，一次计算完成，由 Host 直接输入明文密钥，对消息摘要验证签名。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] sign_algo pqc 签名算法，见 @ref ehsm_pqc_sign_algo_e
 * @param[in] sign_mode pqc 签名模式，见 @ref ehsm_pqc_sign_mode_e
 * @param[in] hash_algo pqc Pre-Hash DSA 使用的 Hash 算法 OID 对应的算法，见 @ref ehsm_pqc_hash_algo_e，仅当 sign_mode
 * 不等于 `EHSM_PQC_PURE_DSA_WITH_MSG` 时有效
 * @param[in] key PQC 明文密钥，见 @ref ehsm_pqc_key_st，必须包含密钥算法 ID 和公钥
 * @param[in] digest 输入消息摘要的 buffer
 * @param[in] digest_size buffer 中输入的消息摘要的字节长度
 * @param[in] pqc_ctx_str 输入 context 字符串数据的 buffer，用于和消息摘要编码后一起验证签名，可以为 `NULL`
 * @param[in] pqc_ctx_str_size buffer 中 context 字符串数据的字节长度
 * - 取值范围：0 到 255 字节
 * - 符合 FIPS 204/205 标准中对 context 字符串长度的限制
 * - 设置为 0 表示不使用 context 字符串
 * @param[in] sign 保存签名值的 buffer
 * @param[in] sign_size buffer 中的签名值的字节长度
 * @param[out] verify_result 存放验证结果的内存位置
 * - `true` 验证通过
 * - `false` 验证不通过
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 密钥、签名值长度信息见 @ref ehsm_pqc_dsa_onepass_ex
 * @note 对于签名验证操作（direction=false），is_det 参数会被忽略
 */
static inline uint32_t ehsm_pqc_dsa_onepass_verify_with_plain_key_and_digest(EHSM_SHM ehsm_ctx_st *ctx,
    ehsm_pqc_sign_algo_e sign_algo, ehsm_pqc_sign_mode_e sign_mode, ehsm_pqc_hash_algo_e hash_algo,
    EHSM_SHM const ehsm_pqc_key_st *key, EHSM_SHM const uint8_t *digest, uint32_t digest_size,
    EHSM_SHM uint8_t *pqc_ctx_str, uint32_t pqc_ctx_str_size, EHSM_SHM uint8_t *sign, uint32_t sign_size,
    bool_t *verify_result)
{
    uint32_t sig_size_tmp = sign_size;
    return ehsm_pqc_dsa_onepass_ex(ctx, sign_algo, sign_mode, hash_algo, /*is_det=*/false,
        /*use_plain_key=*/true, /*key_handle=*/0U, key,
        /*gen_sig=*/false, digest, digest_size, pqc_ctx_str, pqc_ctx_str_size, (uintptr_t)sign, &sig_size_tmp,
        verify_result);
}

/**
 * @brief PQC 密钥封装/解封，生成的共享密钥会保存在 eHSM 内部。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] is_encaps 是否封装密钥
 * - `true` 封装
 * - `false` 解封
 * @param[in] use_plain_parent_key 是使用密钥句柄进行密钥封装，还是由 Host 输入明文密钥进行密钥封装
 * @param[in] parent_key_handle 父密钥句柄，封装时必须包含封装密钥，解封时必须包含解封密钥，仅
 * `use_plain_parent_key` 为 `false` 时有效
 * @param[in] parent_key 明文父密钥数据的 host 地址，见 @ref ehsm_pqc_key_st, 仅 `use_plain_parent_key` 为 `true` 时有效
 * @param[out] cipher_key_data
 * - 密钥封装时，为保存经过封装后的密文数据的 buffer。
 * - 密钥解封时，为输入待解封密文数据的 buffer。
 * @param[in,out] cipher_key_data_size
 * - 密钥封装时，输入时为 buffer 的字节长度，计算完成后为写入 buffer 的数据的字节长度
 * - 密钥解封时，为输入 buffer 中数据的字节长度
 * @param[in] ss_out_type 输出的共享密钥输出类型，见 @ref ehsm_pqc_out_type_e
 * @param[in] ss_key_type 指定生成的共享密钥的密钥类型，见 @ref ehsm_key_type_e，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in] ss_privilege 指定生成的共享密钥的密钥权限，由权限位组合而成，见 @ref key-priv，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key_handle 输入时指定共享密钥的密钥句柄，设置为 0xFFFFFFFF
 * 表示由密钥管理服务分配句柄。输出时作为实际返回的共享密钥句柄。仅当 ss_out_type 为 `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key 保存 32 字节明文共享密钥的 buffer，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @param[in] ss_key_size buffer 的字节长度，仅当 ss_out_type 为 `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval EHSM_ERR_PQC_KEM_FAKE_KEY 解封操作失败，输出的共享密钥为伪密钥（Fake Key），详见下方 @warning
 * @retval 其它值 任务发生错误
 *
 * @warning **FIPS-203 伪密钥（Fake Key）机制**：
 * 根据 FIPS-203 标准要求，当解封（Decapsulation）操作因密文无效而失败时，为防止侧信道攻击，
 * HSM 会输出一个 32 字节的伪密钥（Fake Key）而非返回空数据：
 * - 当 `ss_out_type` 为 `EHSM_PQC_OUT_PLAIN_KEY` 时，伪密钥会写入 `ss_key` 缓冲区，
 *   函数返回 `EHSM_ERR_PQC_KEM_FAKE_KEY`
 * - 当 `ss_out_type` 为 `EHSM_PQC_OUT_KEY_HANDLE` 时，伪密钥**不会**存储到 HSM 内部，
 *   `ss_key_handle` 不会被更新，函数返回其它错误码
 * - **重要**：调用方收到 `EHSM_ERR_PQC_KEM_FAKE_KEY` 返回值时，**必须丢弃** `ss_key` 中的数据，
 *   该数据不是有效的共享密钥
 *
 * @note 长度信息：
 * | 算法        | 公钥长度   | 私钥长度  | 密文长度   | 共享密钥长度 |
 * |-------------|-----------|-----------|-----------|-------------|
 * | ML-KEM-512  | 800字节   | 1632字节  | 768字节   | 32字节      |
 * | ML-KEM-768  | 1184字节  | 2400字节  | 1088字节  | 32字节      |
 * | ML-KEM-1024 | 1568字节  | 3168字节  | 1568字节  | 32字节      |
 * - **封装/解封要求**: 封装需要公钥，解封需要私钥
 */
uint32_t ehsm_pqc_ml_kem_ex(EHSM_SHM ehsm_ctx_st *ctx, bool_t is_encaps, bool_t use_plain_parent_key,
    uint32_t parent_key_handle, EHSM_SHM const ehsm_pqc_key_st *parent_key, EHSM_SHM uint8_t *cipher_key_data,
    uint32_t *cipher_key_data_size, ehsm_pqc_out_type_e ss_out_type, ehsm_key_type_e ss_key_type, uint32_t ss_privilege,
    uint32_t *ss_key_handle, EHSM_SHM uint8_t *ss_key, uint32_t ss_key_size);

/**
 * @brief PQC 密钥封装，由 Host 输入封装密钥句柄，来进行封装。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] parent_key_handle 父密钥句柄，必须包含封装密钥
 * @param[out] cipher_key_data 保存经过封装后的密文数据的 buffer。
 * @param[in,out] cipher_key_data_size 输入时为 buffer 的字节长度，计算完成后为写入 buffer 的数据的字节长度
 * @param[in] ss_out_type 指定生成的共享密钥的输出类型，见 @ref ehsm_pqc_out_type_e
 * @param[in] ss_key_type 指定生成的共享密钥的密钥类型，见 @ref ehsm_key_type_e，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in] ss_privilege 指定生成的共享密钥的密钥权限，由权限位组合而成，见 @ref key-priv，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key_handle 输入时指定共享密钥的密钥句柄，设置为 0xFFFFFFFF
 * 表示由密钥管理服务分配句柄。输出时作为实际返回的共享密钥句柄。仅当 ss_out_type 为 `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key 保存 32 字节明文共享密钥的 buffer，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @param[in] ss_key_size buffer 的字节长度，仅当 ss_out_type 为 `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 封装/解封密钥、共享密钥、密文长度信息见 @ref ehsm_pqc_ml_kem_ex
 */
static inline uint32_t ehsm_pqc_ml_kem_encaps(EHSM_SHM ehsm_ctx_st *ctx, uint32_t parent_key_handle,
    EHSM_SHM uint8_t *cipher_key_data, uint32_t *cipher_key_data_size, ehsm_pqc_out_type_e ss_out_type,
    ehsm_key_type_e ss_key_type, uint32_t ss_privilege, uint32_t *ss_key_handle, EHSM_SHM uint8_t *ss_key,
    uint32_t ss_key_size)
{
    return ehsm_pqc_ml_kem_ex(ctx, /*is_encaps=*/true, /*use_plain_parent_key=*/false, parent_key_handle,
        /*parent_key=*/NULL, cipher_key_data, cipher_key_data_size, ss_out_type, ss_key_type, ss_privilege,
        ss_key_handle, ss_key, ss_key_size);
}

/**
 * @brief PQC 密钥封装，由 Host 输入明文封装密钥，来进行封装。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] parent_key 明文父密钥数据，必须包含封装密钥，见 @ref ehsm_pqc_key_st
 * @param[out] cipher_key_data 保存经过封装后的密文数据的 buffer。
 * @param[in,out] cipher_key_data_size 输入时为 buffer 的字节长度，计算完成后为写入 buffer 的数据的字节长度
 * @param[in] ss_out_type 指定生成的共享密钥的输出类型，见 @ref ehsm_pqc_out_type_e
 * @param[in] ss_key_type 指定生成的共享密钥的密钥类型，见 @ref ehsm_key_type_e，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in] ss_privilege 指定生成的共享密钥的密钥权限，由权限位组合而成，见 @ref key-priv，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key_handle 输入时指定共享密钥的密钥句柄，设置为 0xFFFFFFFF
 * 表示由密钥管理服务分配句柄。输出时作为实际返回的共享密钥句柄。仅当 ss_out_type 为 `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key 保存 32 字节明文共享密钥的 buffer，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @param[in] ss_key_size buffer 的字节长度，仅当 ss_out_type 为 `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 *
 * @note 封装/解封密钥、共享密钥、密文长度信息见 @ref ehsm_pqc_ml_kem_ex
 */
static inline uint32_t ehsm_pqc_ml_kem_encaps_with_plain_parent_key(EHSM_SHM ehsm_ctx_st *ctx,
    EHSM_SHM const ehsm_pqc_key_st *parent_key, EHSM_SHM uint8_t *cipher_key_data, uint32_t *cipher_key_data_size,
    ehsm_pqc_out_type_e ss_out_type, ehsm_key_type_e ss_key_type, uint32_t ss_privilege, uint32_t *ss_key_handle,
    EHSM_SHM uint8_t *ss_key, uint32_t ss_key_size)
{
    return ehsm_pqc_ml_kem_ex(ctx, /*is_encaps=*/true, /*use_plain_parent_key=*/true, /*key_handle=*/0U, parent_key,
        cipher_key_data, cipher_key_data_size, ss_out_type, ss_key_type, ss_privilege, ss_key_handle, ss_key,
        ss_key_size);
}

/**
 * @brief PQC 密钥解封，由 Host 输入解封密钥句柄，来进行解封。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] parent_key_handle 父密钥句柄，必须包含解封密钥
 * @param[in] cipher_key_data 输入待解封密文数据的 buffer。
 * @param[in] cipher_key_data_size 输入密文数据的字节长度
 * @param[in] ss_out_type 指定共享密钥的输出类型，见 @ref ehsm_pqc_out_type_e
 * @param[in] ss_key_type 指定共享密钥的密钥类型，见 @ref ehsm_key_type_e，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in] ss_privilege 指定共享密钥的密钥权限，由权限位组合而成，见 @ref key-priv，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key_handle 输入时指定共享密钥的密钥句柄，设置为 0xFFFFFFFF
 * 表示由密钥管理服务分配句柄。输出时作为实际返回的共享密钥句柄。仅当 ss_out_type 为 `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key 保存 32 字节明文共享密钥的 buffer，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @param[in] ss_key_size buffer 的字节长度，仅当 ss_out_type 为 `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval EHSM_ERR_PQC_KEM_FAKE_KEY 解封操作失败，输出的共享密钥为伪密钥（Fake Key），详见 @ref ehsm_pqc_ml_kem_ex
 * @retval 其它值 任务发生错误
 *
 * @note 封装/解封密钥、共享密钥、密文长度信息及 **伪密钥（Fake Key）机制** 见 @ref ehsm_pqc_ml_kem_ex
 *
 */
static inline uint32_t ehsm_pqc_ml_kem_decaps(EHSM_SHM ehsm_ctx_st *ctx, uint32_t parent_key_handle,
    EHSM_SHM uint8_t *cipher_key_data, uint32_t cipher_key_data_size, ehsm_pqc_out_type_e ss_out_type,
    ehsm_key_type_e ss_key_type, uint32_t ss_privilege, uint32_t *ss_key_handle, EHSM_SHM uint8_t *ss_key,
    uint32_t ss_key_size)
{
    uint32_t tmp_ciphertext_size = cipher_key_data_size;
    return ehsm_pqc_ml_kem_ex(ctx, /*is_encaps=*/false, /*use_plain_parent_key=*/false, parent_key_handle,
        /*parent_key=*/NULL, cipher_key_data, &tmp_ciphertext_size, ss_out_type, ss_key_type, ss_privilege,
        ss_key_handle, ss_key, ss_key_size);
}

/**
 * @brief PQC 密钥解封，由 Host 输入明文解封密钥，来进行解封。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] parent_key 明文父密钥数据，必须包含解封密钥，见 @ref ehsm_pqc_key_st
 * @param[in] cipher_key_data 输入待解封密文数据的 buffer。
 * @param[in] cipher_key_data_size 输入待解封密文数据的字节长度
 * @param[in] ss_out_type 指定共享密钥的输出类型，见 @ref ehsm_pqc_out_type_e
 * @param[in] ss_key_type 指定共享密钥的密钥类型，见 @ref ehsm_key_type_e，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in] ss_privilege 指定共享密钥的密钥权限，由权限位组合而成，见 @ref key-priv，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key_handle 输入时指定共享密钥的密钥句柄，设置为 0xFFFFFFFF
 * 表示由密钥管理服务分配句柄。输出时作为实际返回的共享密钥句柄。仅当 ss_out_type 为 `EHSM_PQC_OUT_KEY_HANDLE` 时有效
 * @param[in,out] ss_key 保存 32 字节明文共享密钥的 buffer，仅当 ss_out_type 为
 * `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @param[in] ss_key_size buffer 的字节长度，仅当 ss_out_type 为 `EHSM_PQC_OUT_PLAIN_KEY` 时有效
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval EHSM_ERR_PQC_KEM_FAKE_KEY 解封操作失败，输出的共享密钥为伪密钥（Fake Key），详见 @ref ehsm_pqc_ml_kem_ex
 * @retval 其它值 任务发生错误
 *
 * @note 封装/解封密钥、共享密钥、密文长度信息及 **伪密钥（Fake Key）机制** 见 @ref ehsm_pqc_ml_kem_ex
 */
static inline uint32_t ehsm_pqc_ml_kem_decaps_with_plain_parent_key(EHSM_SHM ehsm_ctx_st *ctx,
    EHSM_SHM const ehsm_pqc_key_st *parent_key, EHSM_SHM uint8_t *cipher_key_data, uint32_t cipher_key_data_size,
    ehsm_pqc_out_type_e ss_out_type, ehsm_key_type_e ss_key_type, uint32_t ss_privilege, uint32_t *ss_key_handle,
    EHSM_SHM uint8_t *ss_key, uint32_t ss_key_size)
{
    uint32_t tmp_ciphertext_size = cipher_key_data_size;
    return ehsm_pqc_ml_kem_ex(ctx, /*is_encaps=*/false, /*use_plain_parent_key=*/true, /*parent_key_handle=*/0U,
        parent_key, cipher_key_data, &tmp_ciphertext_size, ss_out_type, ss_key_type, ss_privilege, ss_key_handle,
        ss_key, ss_key_size);
}
// #% #endif /* CONFIG_HOST_PQC_EN */

/** @brief 无效的密钥句柄，如果希望由eHSM自动分配生成/导入/派生/协商的密钥句柄时，可使用此值 */
#define EHSM_KEY_HANDLE_INVALID (0xFFFFFFFFU)

/**
 * @brief 让eHSM内部生成随机密钥（对）。
 *
 * @note 请参考用户手册以了解完整的密钥管理方案。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_type 密钥类型，见 @ref ehsm_key_type_e （此函数不支持 DH 密钥对生成）
 * @param[in] privilege 密钥权限，由权限位组合而成，见 @ref key-priv
 * @param[in] rsa_e_bit_size RSA的公钥E值的位长度，例如17代表E值为0x10001。生成非RSA密钥时应设置为0。
 * @param[in] hmac_key_size HMAC的密钥长度，生成非HMAC密钥时应设置为0。
 * @param[in] dh_params 指向DH密钥的参数，生成非DH密钥对时应设置为 `NULL`。指向的参数只能是以下结构体的指针：
 *              - @ref ehsm_dh_params_1024_st
 *              - @ref ehsm_dh_params_2048_st
 *              - @ref ehsm_dh_params_3072_st
 *              - @ref ehsm_dh_params_4096_st
 * @param[in] dh_params_size DH密钥参数结构体的字节长度，生成非DH密钥对时应设置为 0
 * @param[in,out] key_handle 存放密钥句柄的指针
 *              - 如果提供的是 @ref EHSM_KEY_HANDLE_INVALID ，会由eHSM自动分配一个密钥句柄，完成时写入此位置；
 *              - 否则使用指定的密钥句柄，密钥句柄不合法时会失败；
 *              - 如果eHSM已有此句柄的密钥，视其权限可能会返回失败或覆盖成功
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_gen_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_type_e key_type, uint32_t privilege,
    uint32_t rsa_e_bit_size, uint32_t hmac_key_size, EHSM_SHM const void *dh_params, uint32_t dh_params_size,
    uint32_t *key_handle);

/**
 * @brief 导入密钥。
 *
 * @note 请参考 @ref key-mgr 和用户手册以了解完整的密钥管理方案。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] transport_key_handle
 *      - 密文导入时，此值是传输密钥句柄
 *      - 明文导入时此值应为 @ref EHSM_KEY_HANDLE_INVALID 。
 * @param[in] auth_key_handle 用于明文/密文导出时指定验证的密钥句柄。
 *      - 如果是密文导入，此值必须是合法密钥句柄
 *      - 如果是明文导入，此值可以是 @ref EHSM_KEY_HANDLE_INVALID ，也可以是一个合法密钥句柄
 *      - 可以和 `transport_key_handle` 相同，以使用同一个密钥
 * @param[in] key_data 导入的密钥数据，见 @ref ehsm_key_format_st
 * @param[in] key_data_size 导入密钥数据 buffer 的大小，见 @ref key-format
 * @param[in] mac 指向密钥数据结构的MAC值，如果 `auth_key_handle` 为 @ref EHSM_KEY_HANDLE_INVALID ，这里应传入`NULL`
 * @param[in] mac_size MAC值的长度，如果 `mac` 为 `NULL`，此值应为0。
 * @param[in,out] key_handle 存放密钥句柄的指针
 *              - 如果提供的是 @ref EHSM_KEY_HANDLE_INVALID ，会由eHSM自动分配一个密钥句柄，完成时写入此位置；
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_import_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t transport_key_handle, uint32_t auth_key_handle,
    EHSM_SHM const ehsm_key_format_st *key_data, uint32_t key_data_size, EHSM_SHM const uint8_t *mac, uint32_t mac_size,
    uint32_t *key_handle);

/**
 * @brief 导出密钥。
 *
 * @note 请参考 @ref key-mgr 和用户手册以了解完整的密钥管理方案。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] target_key_handle 要导出的目标密钥句柄
 * @param[in] transport_key_handle
 *      - 密文导出时，此值是传输密钥句柄
 *      - 明文导出时此值应为 @ref EHSM_KEY_HANDLE_INVALID 。
 * @param[in] auth_key_handle 用于明文/密文导出时指定验证的密钥句柄。
 *      - 如果是密文导出，此值必须是合法密钥句柄
 *      - 如果是明文导出，此值可以是 @ref EHSM_KEY_HANDLE_INVALID ，也可以是一个合法密钥句柄
 *      - 可以和 `transport_key_handle` 相同，以使用同一个密钥
 * @param[in] key_part 导出密钥的哪部分，见 @ref ehsm_key_part_e
 * @param[out] key_data 存放导出的密钥数据 buffer， 见 @ref ehsm_key_format_st
 * @param[in,out] key_data_size 传入时存放导出密钥数据 buffer 的大小，不能小于对应密钥的导出数据格式所需空间，见 @ref
 * key-format ；完成时存放实际输出到 `key_data` 中的数据长度
 * @param[out] mac 存放输出的密钥数据MAC值的buffer，如果 `auth_key_handle` 为 @ref EHSM_KEY_HANDLE_INVALID ，
 * 则此参数应当传入NULL
 * @param[in,out] mac_size 传入时保存的是`mac`指向buffer的大小，成功完成时保存的是输出的密钥MAC值长度。如果 `mac` 为
 * `NULL`，此值应为 `NULL`。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_export_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t target_key_handle, uint32_t transport_key_handle,
    uint32_t auth_key_handle, ehsm_key_part_e key_part, EHSM_SHM ehsm_key_format_st *key_data, uint32_t *key_data_size,
    EHSM_SHM uint8_t *mac, uint32_t *mac_size);

/**
 * @brief 密钥派生。
 *
 * @note 请参考 @ref key-mgr 和用户手册以了解完整的密钥管理方案。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] hash_algo 使用的HASH算法，不支持摘要长度小于32字节的算法，见 @ref ehsm_hash_algo_e
 * @param[in] derive_algo 派生算法类型，见 @ref ehsm_derive_algo_e
 * @param[in] derive_type 派生方式，见 @ref ehsm_derive_type_e
 * @param[in] privilege 生成的密钥权限，由权限位组合而成，见 @ref key-priv
 * @param[in] key_type 生成的密钥类型，仅支持对称密钥类型，见 @ref ehsm_key_type_e
 * @param[in] key_size 生成的密钥长度，对于生成HMAC密钥，最大不超过512字节；其它密钥类型此长度必须与对应类型相匹配。
 * @param[in] parent_key_handle 派生使用的父密钥句柄，仅 `derive_type` 为 @ref EHSM_DERIVE_TYPE_FROM_PARENT_KEY 时有效
 * @param[in] salt  盐值，可以为 `NULL`
 * @param[in] salt_size 盐值长度，不能大于128
 * @param[in] password 密码值，仅 `derive_type` 为 @ref EHSM_DERIVE_TYPE_PASSWORD 时有效，否则应传入 `NULL`
 * @param[in] password_size 密码值长度，不能大于512，仅 `derive_type` 为 @ref EHSM_DERIVE_TYPE_PASSWORD
 * 时有效，否则应传入 0
 * @param[in] iter_times 派生算法的迭代次数
 * @param[in,out] key_handle 存放密钥句柄的指针
 *              - 如果提供的是 @ref EHSM_KEY_HANDLE_INVALID ，会由eHSM自动分配一个密钥句柄，完成时写入此位置；
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_derive_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e hash_algo, ehsm_derive_algo_e derive_algo,
    ehsm_derive_type_e derive_type, uint32_t privilege, ehsm_key_type_e key_type, uint16_t key_size,
    uint32_t parent_key_handle, EHSM_SHM const uint8_t *salt, uint32_t salt_size, EHSM_SHM const uint8_t *password,
    uint32_t password_size, uint32_t iter_times, uint32_t *key_handle);

/**
 * @brief 派生密钥并将密钥值直接输出到SOC的物理安全端口。
 *
 * @note 此函数不会将密钥存储在 eHSM 内部，不会有对应的 key_handle。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] hash_algo 使用的HASH算法，见 @ref ehsm_hash_algo_e
 * @param[in] derive_algo 派生算法类型，见 @ref ehsm_derive_algo_e
 * @param[in] derive_type 派生方式，见 @ref ehsm_derive_type_e
 * @param[in] parent_key_handle 派生使用的父密钥句柄，仅 `derive_type` 为 @ref EHSM_DERIVE_TYPE_FROM_PARENT_KEY 时有效
 * @param[in] salt  盐值，可以为 `NULL`
 * @param[in] salt_size 盐值长度，不能大于128
 * @param[in] iter_times 派生算法的迭代次数
 * @param[in] soc_channel_id 密钥值输出到SOC端的物理安全通道
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_derive_key_to_soc(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e hash_algo,
    ehsm_derive_algo_e derive_algo, ehsm_derive_type_e derive_type, uint32_t parent_key_handle,
    EHSM_SHM const uint8_t *salt, uint32_t salt_size, uint32_t iter_times, uint8_t soc_channel_id);

/**
 * @brief 密钥交换。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] rmt_pub_key 对端公钥值，仅公钥值内容，格式见 @ref key-format
 * @param[in] rmt_pub_key_size 对端公钥值长度
 * @param[in] privilege 生成的目标密钥的权限，见 @ref key-priv
 * @param[in] key_type 生成的目标密钥类型，见 @ref ehsm_key_type_e ，仅能生成其中的对称密钥类型
 * @param[in] hmac_key_size 生成的目标密钥长度，仅生成HMAC密钥时有效，不能超过512字节。其它密钥类型传入0。
 * @param[in] local_key_handle 本地私钥的密钥句柄
 * @param[in] dh_params 如果是DH算法，传入相应的交换参数，否则应传入 `NULL`。指向的参数只能是以下结构体的指针：
 *              - @ref ehsm_dh_params_1024_st
 *              - @ref ehsm_dh_params_2048_st
 *              - @ref ehsm_dh_params_3072_st
 *              - @ref ehsm_dh_params_4096_st
 * @param[in] dh_params_size DH密钥参数结构体的字节长度，生成非DH密钥对时应设置为 0
 * @param[in] sm2_params 如果是SM2算法，传入相应的交换参数，否则应传入 `NULL`
 * @param[in,out] key_handle 存放密钥句柄的指针
 *              - 如果提供的是 @ref EHSM_KEY_HANDLE_INVALID ，会由eHSM自动分配一个密钥句柄，完成时写入此位置；
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_exchange_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *rmt_pub_key, uint32_t rmt_pub_key_size,
    uint32_t privilege, ehsm_key_type_e key_type, uint32_t hmac_key_size, uint32_t local_key_handle,
    EHSM_SHM const void *dh_params, uint32_t dh_params_size, EHSM_SHM ehsm_sm2_params_st *sm2_params,
    uint32_t *key_handle);

/**
 * @brief SM9密钥协商。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] privilege 生成的目标密钥的权限，见 @ref key-priv
 * @param[in] key_type 生成的目标密钥类型，见 @ref ehsm_key_type_e ，仅能生成其中的对称密钥类型
 * @param[in] role 密钥协商的角色，见 @ref ehsm_sm9_role_e
 * @param[in] user_priv_key_handle 本地的密钥协商用户私钥句柄
 * @param[in] user_tmp_key_handle 本地的密钥协商临时密钥句柄
 * @param[in] hmac_key_size 生成的目标密钥长度，仅生成HMAC密钥时有效，不能超过512字节。其它密钥类型传入0。
 * @param[in] extra_params SM9密钥协商的其他参数
 * @param[out] key_handle 存放密钥句柄的指针, 由eHSM自动分配一个密钥句柄，完成时写入此位置；
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_sm9_exchange_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t privilege, ehsm_key_type_e key_type, uint8_t role,
    uint32_t user_priv_key_handle, uint32_t user_tmp_key_handle, uint32_t hmac_key_size,
    ehsm_sm9_params_st *extra_params, uint32_t *key_handle);

/**
 * @brief 从包含私钥的密钥句柄计算公钥值。
 *
 * @note RSA算法不支持此功能。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle 密钥句柄
 * @param[in] dh_params 指向DH密钥的参数，生成非DH密钥对时应设置为 `NULL`。指向的参数只能是以下结构体的指针：
 *              - @ref ehsm_dh_params_1024_st
 *              - @ref ehsm_dh_params_2048_st
 *              - @ref ehsm_dh_params_3072_st
 *              - @ref ehsm_dh_params_4096_st
 * @param[in] dh_params_size DH密钥参数结构体的字节长度，生成非DH密钥对时应设置为 0
 * @param[out] pub_key 存放输出公钥值的buffer，仅包含公钥数据，公钥格式见 @ref key-format
 * @param[in,out] pub_key_size 输入时为 `pub_key` buffer的长度，完成时为写入 `pub_key` 的数据长度
 * @param[out] key_type 输出密钥类型
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_get_pub_from_priv(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const void *dh_params,
    uint32_t dh_params_size, EHSM_SHM uint8_t *pub_key, uint32_t *pub_key_size, ehsm_key_type_e *key_type);
/**
 * @brief 删除指定密钥。
 *
 * 如果密钥没有可删除权限，则删除会失败。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_handle 期望删除的密钥句柄
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_km_remove_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle);

/**
 * @brief 获取BootLoader/Firmware的版本号。
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] version 保存版本数据的buffer，见 @ref ehsm_version_st
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_get_version(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_version_st *version);

/**
 * @brief 校验安全启动镜像。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] image 镜像的存储位置
 * @param[in] image_size 镜像的大小
 * @param[in] check_version 是否检查 version counter
 * @param[in] boot 校验成功后是否跳转到FW中执行，仅对eHSM镜像有效
 * @param[out] image_out 镜像解密输出的地址，包含镜像头，仅对SOC镜像有效，如果是eHSM镜像此参数应为 `NULL`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_verify_image(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *image, uint32_t image_size,
    bool_t check_version, bool_t boot, EHSM_SHM uint8_t *image_out);

// #% #if CONFIG_BL_INJECT_ERR_ENABLE
/**
 * @brief 向 eHSM 注入错误并可在 SOC 端观察到对应的错误信号。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] values 注入的错误数据，见 @ref ehsm_inject_error_st ，此数据无需放在共享内存中
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_inject_error(EHSM_SHM ehsm_ctx_st *ctx, const ehsm_inject_error_st *values);
// #% #endif // CONFIG_BL_INJECT_ERR_ENABLE

/**
 * @brief 升级固件镜像的扩展函数，支持不同的处理阶段。
 *
 * @note 这是底层接口，建议根据需求使用封装接口，参考
 *  - @ref ehsm_upgrade_fw_image
 *  - @ref ehsm_upgrade_fw_image_init
 *  - @ref ehsm_upgrade_fw_image_update
 *  - @ref ehsm_upgrade_fw_image_finish
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] mode 处理模式，见 @ref ehsm_proc_mode_e
 * @param[in] input 输入数据地址
 * @param[in] input_size 输入数据长度
 * @param[out] output 输出数据地址
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_upgrade_fw_image_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_proc_mode_e mode, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uint8_t *output);

/**
 * @brief 升级固件镜像，一次性完成。
 *
 * 此操作输入一个安全升级镜像，得到一个安全启动镜像，根据安全启动镜像的 `Plain_Flag`
 * 决定是否对安全启动镜像的CODE区域加密。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] image 完整安全升级镜像数据地址
 * @param[in] image_size 完整安全升级镜像字节大小
 * @param[out] image_out 输出安全启动镜像的地址，可以等于 `image` + 1KB
 * 的位置（即在原地解密），否则不要与输入的区域有重叠，输出的数据长度为输入镜像长度 - 1KB，应提供足够的空间
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_upgrade_fw_image(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *image, uint32_t image_size, EHSM_SHM uint8_t *image_out)
{
    return ehsm_upgrade_fw_image_ex(ctx, EHSM_PROC_ONEPASS, image, image_size, image_out);
}

/**
 * @brief 升级固件镜像初始化。
 *
 * @note 必须将完整的镜像头输入，且此函数只处理镜像头，不会处理数据部分。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] image_header 安全升级镜像的镜像头地址
 * @param[in] header_size 镜像头长度
 * @param[out] image_out 用于存放输出的启动镜像的起始地址，实际init时不会向此地址输出数据，但不能传入NULL
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_upgrade_fw_image_init(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *image_header, uint32_t header_size, EHSM_SHM uint8_t *image_out)
{
    return ehsm_upgrade_fw_image_ex(ctx, EHSM_PROC_INIT, image_header, header_size, image_out);
}

/**
 * @brief 升级固件镜像更新。
 *
 * 此函数用于分块提交升级镜像的数据。
 *
 * @note 第一次调用此函数（即更新第一块升级镜像中的数据块）时，传入的数据长度不能小于镜像头的大小。因为第一块数据中需要
 * 包含安全启动镜像的镜像头(注意不是升级镜像的镜像头)，以便eHSM一次性读取并处理。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] body_block 输入的安全升级镜像数据块地址（镜像头必须在 @ref ehsm_upgrade_fw_image_init 中输入）
 * @param[in] block_size 输入数据长度，必须是16的整数倍
 * @param[out] image_out 用于存放输出的启动镜像对应数据的地址，输出数据的长度与输入数据相同
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_upgrade_fw_image_update(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *body_block, uint32_t block_size, EHSM_SHM uint8_t *image_out)
{
    return ehsm_upgrade_fw_image_ex(ctx, EHSM_PROC_UPDATE, body_block, block_size, image_out);
}

/**
 * @brief 升级固件镜像完成。
 *
 * 此函数用于分块提交升级镜像的最后一块数据并完成校验，此函数应当只对最后一个数据块调用一次。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] body_block 输入的安全升级镜像数据最后一块地址（镜像头必须在 @ref ehsm_upgrade_fw_image_init 中输入）
 * @param[in] block_size 输入数据长度，必须是16的整数倍
 * @param[out] image_out 用于存放输出的启动镜像对应数据的地址，输出数据的长度与输入数据相同
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
static inline uint32_t ehsm_upgrade_fw_image_finish(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *body_block, uint32_t block_size, EHSM_SHM uint8_t *image_out)
{
    return ehsm_upgrade_fw_image_ex(ctx, EHSM_PROC_FINISH, body_block, block_size, image_out);
}

/**
 * @brief 设置EHSM UTC-Timer时间。
 *
 * 此接口可以配置UTC时间到EHSM的UTC-Timer模块，以此实现UTC时间配置或同步的功能。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] utc_time 需要配置的UTC时间
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_set_utc_time(EHSM_SHM ehsm_ctx_st *ctx, uint32_t utc_time);

/**
 * @brief 获取EHSM UTC-Timer时间。
 *
 * 此接口可以获取EHSM当前的UTC-Timer的值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] utc_time 当前的EHSM的UTC时间
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_get_utc_time(EHSM_SHM ehsm_ctx_st *ctx, uint32_t *utc_time);

/**
 * @brief 创建（使能）一个计数器
 *
 * 此接口可以创建或使能一个EHSM的计数器。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] counter_id 被创建或使能的计数器的ID
 * @param[out] counter_value 被创建或使能的计数器的数值
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_create_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t *counter_id, uint64_t *counter_value);

/**
 * @brief 读取某个计数器的值
 *
 * 此接口可以读取某个计数器的值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] counter_id 需要被读取的计数器的ID
 * @param[out] counter_value 返回的计数器的值
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_read_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id, uint64_t *counter_value);

/**
 * @brief 为某个计数器增加计数值
 *
 * 此接口可以为某个计数器增加计数值
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] counter_id 需要增加计数值的计数器ID
 * @param[in] increase_value 需要增加的计数值
 * @param[out] current_value 增加数值后的计数值
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_increase_counter(
    EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id, uint64_t increase_value, uint64_t *current_value);

/**
 * @brief 删除（使失能）某个计数器
 *
 * 此接口可以删除（使失能）某个计数值
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] counter_id 需要删除的计数器ID
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_delete_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id);

/**
 * @brief eHSM 进入 WFI 状态。
 *
 * eHSM在收到命令后会，当所有Mailbox channel上都没有命令时，会进入WFI状态，直到下一次被Mailbox或其它中断唤醒。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 * @note 发送此命令后，在cpu_wfi信号置位之前，不应在任何Mailbox channel上发送命令。
 */
uint32_t ehsm_enter_wfi(EHSM_SHM ehsm_ctx_st *ctx);

// #% #if CONFIG_EHSM_ENABLE_TBBR
/**
 * @brief 校验TBBR镜像。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] img_size 镜像字节长度
 * @param[in] img_data 镜像数据指针
 * @param[in] trusted_fw_nv_ctr_in_otp OTP中的trusted固件version counter
 * @param[in] non_trusted_fw_nv_ctr_in_otp OTP中的non-trusted固件version counter
 * @param[out] trusted_fw_nv_ctr_in_cert eHSM返回的证书链镜像中的trusted固件version counter
 * @param[out] non_trusted_fw_nv_ctr_in_otp_cert eHSM返回的证书链镜像中的non-trusted固件version counter
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_verify_tbbr_img(EHSM_SHM ehsm_ctx_st *ctx, uint32_t img_size, EHSM_SHM uint8_t *img_data,
    uint32_t trusted_fw_nv_ctr_in_otp, uint32_t non_trusted_fw_nv_ctr_in_otp, uint32_t *trusted_fw_nv_ctr_in_cert,
    uint32_t *non_trusted_fw_nv_ctr_in_otp_cert);
// #% #endif // CONFIG_EHSM_ENABLE_TBBR

/**
 * @brief 设置串口波特率
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] baud_div UART 的频率分频值，指 CPU 频率值除以 UART 波特率值的结果
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_set_uart_baudrate(EHSM_SHM ehsm_ctx_st *ctx, uint32_t baud_div);

/**
 * @brief 生成一个随机密钥并且写入指定的OTP密钥槽中
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_level 密钥级别，见 @ref ehsm_key_level_e
 * @param[in] key_type 生成的密钥类型 见 @ref ehsm_install_key_type_e
 * @param[in] key_slot_id 安装的OTP密钥槽ID编号
 * @param[in] last_key 是否是最后一个OTP密钥，true：是最后一个，false：不是最后一个
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_install_random_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level,
    ehsm_install_key_type_e key_type, uint16_t key_slot_id, bool_t last_key);

/**
 * @brief 将外部加密的密钥值写入到指定的OTP密钥槽中
 *
 * 此命令内部使用 `CHIP RTL KEK EHSM` 或 `CHIP RTL KEK SOC` 密钥对输入值进行 CBC (IV=全0）解密，然后使用对应的 `CHIP
 * ROOT KEY` 或 `DEVICE ROOT KEY` 对密钥明文进行加密，并计算其CRC32值，最后将加密后的密钥值和CRC32值写入指定的OTP密钥槽
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_level 密钥级别，见 @ref ehsm_key_level_e
 * @param[in] key_type 生成的密钥类型 见 @ref ehsm_install_key_type_e
 * @param[in] key_slot_id 安装的OTP密钥槽ID编号
 * @param[in] last_key 是否是最后一个OTP密钥，true：是最后一个，false：不是最后一个
 * @param[in] input_data 密钥值或公钥HASH值，根据 `key_level` 的取值，被 `CHIP RTL KEK EHSM` 或 `CHIP RTL KEK SOC`
 * 加密过（CBC模式，IV=全0，算法由OTP HW-CTRL配置中的KeyAlgSel决定是AES128还是SM4）
 * @param[in] size `input_data` 的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_install_encrypted_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level,
    ehsm_install_key_type_e key_type, uint16_t key_slot_id, bool_t last_key, EHSM_SHM const uint8_t *input_data,
    uint32_t size);

/**
 * @brief 读取OTP数据。
 *
 * 此功能仅支持 `TEST_MODE` 和 `DEVELOPE_MODE`。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] buf 存储读取的OTP数据
 * @param[in] ehsm_src_addr 从 ehsm 内部的OTP地址读取（即0x33000000开始的OTP数据地址）
 * @param[in] size 要读取的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_read_otp(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size);

/**
 * @brief 写入OTP数据。
 *
 * 此功能仅支持 `TEST_MODE` 和 `DEVELOPE_MODE`。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] src_data 写入的源数据
 * @param[in] ehsm_dest_addr 写入的 ehsm 内部OTP地址（即0x33000000开始的OTP数据地址）
 * @param[in] size 写入数据的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_write_otp(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size);

/**
 * @brief 读取CFG地址范围的寄存器值。
 *
 * 此功能仅支持 `TEST_MODE` 和 `DEVELOPE_MODE`。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] buf 存储读取的CFG寄存器数据
 * @param[in] ehsm_src_addr 从 ehsm 内部的CFG地址读取（即0x33400000~0x334FFFFF地址区间）
 * @param[in] size 要读取的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_read_reg(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size);

/**
 * @brief 写入CFG地址范围的寄存器值。
 *
 * 此功能仅支持 `TEST_MODE` 和 `DEVELOPE_MODE`。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] src_data 写入的源数据
 * @param[in] ehsm_dest_addr 写入的 ehsm 内部CFG地址（即0x33400000~0x334FFFFF地址区间）
 * @param[in] size 写入数据的字节长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_write_reg(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size);

/**
 * @brief 挑战鉴权类型。
 */
typedef enum {
    EHSM_CHALLENGE_TYPE_EHSM_DEBUG = 1, /**< EHSM调试鉴权挑战，挑战值的长度为 32+16 字节 */
    EHSM_CHALLENGE_TYPE_SHE_DEBUG = 2,  /**< SHE管理调试鉴权挑战，BootLoader中不支持 */
    EHSM_CHALLENGE_TYPE_SOC_DEBUG = 3,  /**< SOC调试鉴权挑战，挑战值的长度为 32+16 字节 */
    EHSM_CHALLENGE_TYPE_USER_AUTH = 4,  /**< FW用户行为鉴权，BootLoader中不支持，挑战值长度为 32+16 字节*/
    EHSM_CHALLENGE_TYPE_FW_AUTH = 5,    /**< 国密固件认证鉴权挑战，仅BootLoader中支持，挑战值长度为 16 字节 */
} ehsm_challenge_type_e;

/**
 * @brief 权限算法类型。
 */
typedef enum {
    EHSM_AUTH_ALGO_SM3_SM2 = 1,
    EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1 = 2,
    EHSM_AUTH_ALGO_SM4_CMAC = 3,
    EHSM_AUTH_ALGO_AES128_CMAC = 4,
    EHSM_AUTH_ALGO_SHA256_RSA = 5,
} ehsm_auth_algo_e;

/**
 * @brief 获取鉴权挑战值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] challenge_type 挑战鉴权类型，见 @ref ehsm_challenge_type_e
 * @param[out] output 存放挑战值的地址，挑战值的长度见 @ref ehsm_challenge_type_e , `output`的可用长度不能小于挑战值长度
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_get_challenge(EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e challenge_type, EHSM_SHM uint8_t *output);

/**
 * @brief 完成鉴权并进行相应的调试配置操作。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] challenge_type 挑战鉴权类型，应当与调用 @ref ehsm_get_challenge 时相同，见 @ref ehsm_challenge_type_e
 * @param[in] algo 用于鉴权的算法类型，见 @ref ehsm_auth_algo_e
 * @param[in] sig 签名/CMAC值
 * @param[in] sig_size 签名/CMAC值的字节长度
 *          - 对于SM2和ECDSA_P256R1，为64字节
 *          - 对于AES和SM4的CMAC，为16字节
 *          - 对于RSA2048和RSA4096，分别为256字节和512字节
 * @param[in] pub_key 公钥值，仅挑战类型为 @ref EHSM_CHALLENGE_TYPE_SOC_DEBUG 或 @ref EHSM_CHALLENGE_TYPE_EHSM_DEBUG
 * 时且使用非对称算法鉴权时有效，其它时候传入 `NULL`
 * @param[in] pub_key_size 公钥值的长度
 *          - 对于SM2算法，长度为64，公钥值不带前面的 `0x04` 标记
 *          - 对于ECDSA_P256R1算法，长度为64
 *          - 对于RSA2048，长度为320（64字节E + 256字节N）
 *          - 对于RSA4096，长度为576（64字节E + 512字节N）
 * @param[in] soc_dbg_bitmap 对挑战类型为 @ref EHSM_CHALLENGE_TYPE_SOC_DEBUG 时有效，其它类型传入
 * `NULL`，对于bitmap中的值，为1的bit时对应的调试端口被打开，为0的bit对应的调试端口保持原状态。见 @ref
 * ehsm_soc_dbg_bitmap_st
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_debug_auth(EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e challenge_type, ehsm_auth_algo_e algo,
    EHSM_SHM const uint8_t *sig, uint32_t sig_size, EHSM_SHM const uint8_t *pub_key, uint32_t pub_key_size,
    EHSM_SHM const ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap);

/**
 * @brief 关闭调试。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] type 仅支持 @ref EHSM_CHALLENGE_TYPE_SOC_DEBUG 和 @ref EHSM_CHALLENGE_TYPE_EHSM_DEBUG
 * @param[in] soc_dbg_bitmap 对挑战类型为 @ref EHSM_CHALLENGE_TYPE_SOC_DEBUG 时有效，其它类型传入
 * `NULL`，对于bitmap中的值，为1的bit时对应的调试端口被关闭，为0的bit对应的调试端口保持原状态。见 @ref
 * ehsm_soc_dbg_bitmap_st
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_close_debug(
    EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e type, EHSM_SHM const ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap);

/**
 * @brief eHSM EMU状态信息结构体。
 */
typedef struct {
    uint32_t o_hsm_status[2];  /**< eHSM 状态 */
    uint32_t o_hsm_err_sensor; /**< 传感器检测到的错误 */
    uint32_t o_hsm_err_hw[2];  /**< 硬件检测到的错误 */
    uint32_t o_hsm_err_fw[2];  /**< 固件检测到的错误 */
} ehsm_emu_status_st;

/**
 * @brief 获取eHSM EMU状态。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] status_buf 存储EMU状态信息的共享内存地址，见 @ref ehsm_emu_status_st
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_get_emu_status(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_emu_status_st *status_buf);

/**
 * @brief eHSM 生命周期枚举值。
 */
typedef enum {
    EHSM_LC_TEST = 1,
    EHSM_LC_DEVELOP = 2,
    EHSM_LC_MANUFACTURE = 3,
    EHSM_LC_USER = 4,
    EHSM_LC_DEBUG = 5,
    EHSM_LC_DESTORY = 6,
} ehsm_lifecycle_e;

/**
 * @brief 更改 eHSM 生命周期。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] lifecycle 目标生命周期，不能比当前 eHSM 的生命周期小
 *
 * @note 如果当前 eHSM 生命周期为 EHSM_LC_USER ，想要修改生命周期必须先鉴权，见 @ref EHSM_CHALLENGE_TYPE_USER_AUTH
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_change_lifecycle(EHSM_SHM ehsm_ctx_st *ctx, ehsm_lifecycle_e lifecycle);

/**
 * @brief eHSM OTP中控制字段类型。
 */
typedef enum {
    EHSM_CTRL_FIELD_HW = 0,
    EHSM_CTRL_FIELD_EHSM = 1,
    EHSM_CTRL_FIELD_SOC = 2,
} ehsm_ctrl_field_e;

/**
 * @brief 修改OTP控制字段的值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] field 控制字段类型，见 @ref ehsm_ctrl_field_e
 * @param[in] value 存放64位值的共享内存地址。
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_change_control_field(EHSM_SHM ehsm_ctx_st *ctx, ehsm_ctrl_field_e field, EHSM_SHM const uint64_t *value);

#pragma pack()

/** @} */

#endif // EHSM_BASE_API_H
