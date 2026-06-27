#ifndef EHSM_KEY_ASSIST_H
#define EHSM_KEY_ASSIST_H

#include "types.h"

#pragma pack(1)

/** @addtogroup key-format-struct
 * @{
 */

/**
 * @brief 用于辅助64位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为8 */
    uint8_t key_value[8];   /**< 明文密钥值 */
} ehsm_key_format_symm_plain_64_st;

/**
 * @brief 用于辅助64位对称密钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为8 */
    uint8_t key_value[16];  /**< 密文密钥值，这里进行了PKCS7填充，因此为16字节 */
} ehsm_key_format_symm_cipher_64_st;
/**
 * @brief 用于辅助128位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为16 */
    uint8_t key_value[16];  /**< 明文密钥值 */
} ehsm_key_format_symm_plain_128_st;

/**
 * @brief 用于辅助128位对称密钥密文导入/导出的结构体。
 * @note 注意，此结构体仅针对传输密钥的块大小是16字节的情况，对于传输密钥为DES/TDES时，密钥密文长度为24字节
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为16 */
    uint8_t key_value[32];  /**< 密文密钥值，这里进行了PKCS7填充，因此为32字节 */
} ehsm_key_format_symm_cipher_128_st;

/**
 * @brief 用于辅助192位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为24 */
    uint8_t key_value[24];  /**< 明文密钥值 */
} ehsm_key_format_symm_plain_192_st;

/**
 * @brief 用于辅助192位对称密钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为24 */
    uint8_t key_value[32];  /**< 密文密钥值，这里进行了PKCS7填充，因此为32字节 */
} ehsm_key_format_symm_cipher_192_st;

/**
 * @brief 用于辅助256位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为32 */
    uint8_t key_value[32];  /**< 明文密钥值 */
} ehsm_key_format_symm_plain_256_st;

/**
 * @brief 用于辅助256位对称密钥密文导入/导出的结构体。
 * @note 注意，此结构体仅针对传输密钥的块大小是16字节的情况，对于传输密钥为DES/TDES时，密钥密文长度为40字节
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为32 */
    uint8_t key_value[48];  /**< 密文密钥值，这里进行了PKCS7填充，因此为48字节 */
} ehsm_key_format_symm_cipher_256_st;

/**
 * @brief 用于辅助384位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为48 */
    uint8_t key_value[48];  /**< 明文密钥值 */
} ehsm_key_format_symm_plain_384_st;

/**
 * @brief 用于辅助384位对称密钥密文导入/导出的结构体。
 * @note 注意，此结构体仅针对传输密钥的块大小是16字节的情况，对于传输密钥为DES/TDES时，密钥密文长度为56字节
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为48 */
    uint8_t key_value[64];  /**< 密文密钥值，这里进行了PKCS7填充，因此为64字节 */
} ehsm_key_format_symm_cipher_384_st;

/**
 * @brief 用于辅助512位对称密钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为64 */
    uint8_t key_value[64];  /**< 明文密钥值 */
} ehsm_key_format_symm_plain_512_st;

/**
 * @brief 用于辅助512位对称密钥密文导入/导出的结构体。
 * @note 注意，此结构体仅针对传输密钥的块大小是16字节的情况，对于传输密钥为DES/TDES时，密钥密文长度为72字节
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_SYMM_KEY ，见 @ref ehsm_key_part_e */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为0 */
    uint16_t priv_key_size; /**< 对称密钥明文数据长度，这里应当设置为64 */
    uint8_t key_value[80];  /**< 密文密钥值，这里进行了PKCS7填充，因此为80字节 */
} ehsm_key_format_symm_cipher_512_st;

/** @brief 用于辅助DES密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_64_st ehsm_key_format_des_plain_st;
/** @brief 用于辅助DES密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_64_st ehsm_key_format_des_cipher_st;
/** @brief 用于辅助AES128密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_128_st ehsm_key_format_aes128_plain_st;
/** @brief 用于辅助AES128密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_128_st ehsm_key_format_aes128_cipher_st;
/** @brief 用于辅助TDES128密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_128_st ehsm_key_format_tdes128_plain_st;
/** @brief 用于辅助TDES128密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_128_st ehsm_key_format_tdes128_cipher_st;
/** @brief 用于辅助SM4密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_128_st ehsm_key_format_sm4_plain_st;
/** @brief 用于辅助SM4密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_128_st ehsm_key_format_sm4_cipher_st;
/** @brief 用于辅助TDES192密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_192_st ehsm_key_format_tdes192_plain_st;
/** @brief 用于辅助TDES192密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_192_st ehsm_key_format_tdes192_cipher_st;
/** @brief 用于辅助AES192密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_192_st ehsm_key_format_aes192_plain_st;
/** @brief 用于辅助AES192密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_192_st ehsm_key_format_aes192_cipher_st;
/** @brief 用于辅助AES256密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_256_st ehsm_key_format_aes256_plain_st;
/** @brief 用于辅助AES256密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_256_st ehsm_key_format_aes256_cipher_st;
/** @brief 用于辅助AES128_XTS密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_256_st ehsm_key_format_aes128_xts_plain_st;
/** @brief 用于辅助AES128_XTS密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_256_st ehsm_key_format_aes128_xts_cipher_st;
/** @brief 用于辅助SM4_XTS密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_256_st ehsm_key_format_sm4_xts_plain_st;
/** @brief 用于辅助SM4_XTS密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_256_st ehsm_key_format_sm4_xts_cipher_st;
/** @brief 用于辅助AES192_XTS密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_384_st ehsm_key_format_aes192_xts_plain_st;
/** @brief 用于辅助AES192_XTS密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_384_st ehsm_key_format_aes192_xts_cipher_st;
/** @brief 用于辅助AES256_XTS密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_512_st ehsm_key_format_aes256_xts_plain_st;
/** @brief 用于辅助AES256_XTS密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_512_st ehsm_key_format_aes256_xts_cipher_st;
/** @brief 用于辅助HMAC64密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_64_st ehsm_key_format_hmac64_plain_st;
/** @brief 用于辅助HMAC64密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_64_st ehsm_key_format_hmac64_cipher_st;
/** @brief 用于辅助HMAC128密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_128_st ehsm_key_format_hmac128_plain_st;
/** @brief 用于辅助HMAC128密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_128_st ehsm_key_format_hmac128_cipher_st;
/** @brief 用于辅助HMAC192密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_192_st ehsm_key_format_hmac192_plain_st;
/** @brief 用于辅助HMAC192密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_192_st ehsm_key_format_hmac192_cipher_st;
/** @brief 用于辅助HMAC256密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_256_st ehsm_key_format_hmac256_plain_st;
/** @brief 用于辅助HMAC256密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_256_st ehsm_key_format_hmac256_cipher_st;
/** @brief 用于辅助HMAC384密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_384_st ehsm_key_format_hmac384_plain_st;
/** @brief 用于辅助HMAC384密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_384_st ehsm_key_format_hmac384_cipher_st;
/** @brief 用于辅助HMAC512密钥明文导入/导出的结构体 */
typedef ehsm_key_format_symm_plain_512_st ehsm_key_format_hmac512_plain_st;
/** @brief 用于辅助HMAC512密钥密文导入/导出的结构体 */
typedef ehsm_key_format_symm_cipher_512_st ehsm_key_format_hmac512_cipher_st;

/**
 * @brief 用于辅助RSA1024公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为8 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t e_value[8];     /**< E值 */
    uint8_t n_value[128];   /**< N值 */
} ehsm_key_format_rsa1024_plain_pubkey_st;

/**
 * @brief 用于辅助RSA1024私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为128 */
    uint8_t n_value[128];   /**< N值明文 */
    uint8_t d_value[128];   /**< D值明文 */
} ehsm_key_format_rsa1024_plain_privkey_st;

/**
 * @brief 用于辅助RSA1024私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为128 */
    uint8_t n_value[128];   /**< N值明文，128字节 */
    uint8_t d_value[144];   /**< D值密文，进行了PKCS7填充，因此是144字节 */
} ehsm_key_format_rsa1024_cipher_privkey_st;

/**
 * @brief 用于辅助RSA1024密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为128 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[128];   /**< N值明文，128字节 */
    uint8_t d_value[128];   /**< D值明文，128字节 */
} ehsm_key_format_rsa1024_plain_keypair_st;

/**
 * @brief 用于辅助RSA1024密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为128 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[128];   /**< N值明文，128字节 */
    uint8_t d_value[144];   /**< D值密文，144字节 */
} ehsm_key_format_rsa1024_cipher_keypair_st;

/**
 * @brief 用于辅助RSA2048公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为8 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t e_value[8];     /**< E值 */
    uint8_t n_value[256];   /**< N值 */
} ehsm_key_format_rsa2048_plain_pubkey_st;

/**
 * @brief 用于辅助RSA2048私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为256 */
    uint8_t n_value[256];   /**< N值明文 */
    uint8_t d_value[256];   /**< D值明文 */
} ehsm_key_format_rsa2048_plain_privkey_st;

/**
 * @brief 用于辅助RSA2048私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为256 */
    uint8_t n_value[256];   /**< N值明文，256字节 */
    uint8_t d_value[272];   /**< D值密文，进行了PKCS7填充，因此是272字节 */
} ehsm_key_format_rsa2048_cipher_privkey_st;

/**
 * @brief 用于辅助RSA2048密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为256 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[256];   /**< N值明文，256字节 */
    uint8_t d_value[256];   /**< D值明文，256字节 */
} ehsm_key_format_rsa2048_plain_keypair_st;

/**
 * @brief 用于辅助RSA2048密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为256 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[256];   /**< N值明文，256字节 */
    uint8_t d_value[272];   /**< D值密文，272字节 */
} ehsm_key_format_rsa2048_cipher_keypair_st;

/**
 * @brief 用于辅助RSA3072公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为8 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t e_value[8];     /**< E值 */
    uint8_t n_value[384];   /**< N值 */
} ehsm_key_format_rsa3072_plain_pubkey_st;

/**
 * @brief 用于辅助RSA3072私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为384 */
    uint8_t n_value[384];   /**< N值明文 */
    uint8_t d_value[384];   /**< D值明文 */
} ehsm_key_format_rsa3072_plain_privkey_st;

/**
 * @brief 用于辅助RSA3072私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为384 */
    uint8_t n_value[384];   /**< N值明文，384字节 */
    uint8_t d_value[400];   /**< D值密文，进行了PKCS7填充，因此是400字节 */
} ehsm_key_format_rsa3072_cipher_privkey_st;

/**
 * @brief 用于辅助RSA3072密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为384 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[384];   /**< N值明文，384字节 */
    uint8_t d_value[384];   /**< D值明文，384字节 */
} ehsm_key_format_rsa3072_plain_keypair_st;

/**
 * @brief 用于辅助RSA3072密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为384 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[384];   /**< N值明文，384字节 */
    uint8_t d_value[400];   /**< D值密文，400字节 */
} ehsm_key_format_rsa3072_cipher_keypair_st;

/**
 * @brief 用于辅助RSA4096公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 应当设置为8 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t e_value[8];     /**< E值 */
    uint8_t n_value[512];   /**< N值 */
} ehsm_key_format_rsa4096_plain_pubkey_st;

/**
 * @brief 用于辅助RSA4096私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为512 */
    uint8_t n_value[512];   /**< N值明文 */
    uint8_t d_value[512];   /**< D值明文 */
} ehsm_key_format_rsa4096_plain_privkey_st;

/**
 * @brief 用于辅助RSA4096私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为512 */
    uint8_t n_value[512];   /**< N值明文，512字节 */
    uint8_t d_value[528];   /**< D值密文，进行了PKCS7填充，因此是528字节 */
} ehsm_key_format_rsa4096_cipher_privkey_st;

/**
 * @brief 用于辅助RSA4096密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为512 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[512];   /**< N值明文，512字节 */
    uint8_t d_value[512];   /**< D值明文，512字节 */
} ehsm_key_format_rsa4096_plain_keypair_st;

/**
 * @brief 用于辅助RSA4096密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< D值明文长度，这里应当设置为512 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[512];   /**< N值明文，512字节 */
    uint8_t d_value[528];   /**< D值密文，528字节 */
} ehsm_key_format_rsa4096_cipher_keypair_st;

/**
 * @brief 用于辅助RSA1024-CRT私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为320 */
    uint8_t p_value[64];    /**< P值明文，64字节 */
    uint8_t q_value[64];    /**< Q值明文，64字节 */
    uint8_t dp_value[64];   /**< DP值明文，64字节 */
    uint8_t dq_value[64];   /**< DQ值明文，64字节 */
    uint8_t u_value[64];    /**< U值明文，64字节 */
} ehsm_key_format_rsa1024_crt_plain_privkey_st;

/**
 * @brief 用于辅助RSA1024-CRT私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为320 */
    uint8_t p_value[64];    /**< P值密文，64字节 */
    uint8_t q_value[64];    /**< Q值密文，64字节 */
    uint8_t dp_value[64];   /**< DP值密文，64字节 */
    uint8_t dq_value[64];   /**< DQ值密文，64字节 */
    uint8_t u_value[64];    /**< U值密文，64字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa1024_crt_cipher_privkey_st;

/**
 * @brief 用于辅助RSA1024-CRT密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为320 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[128];   /**< N值明文，128字节 */
    uint8_t p_value[64];    /**< P值明文，64字节 */
    uint8_t q_value[64];    /**< Q值明文，64字节 */
    uint8_t dp_value[64];   /**< DP值明文，64字节 */
    uint8_t dq_value[64];   /**< DQ值明文，64字节 */
    uint8_t u_value[64];    /**< U值明文，64字节 */
} ehsm_key_format_rsa1024_crt_plain_keypair_st;

/**
 * @brief 用于辅助RSA1024-CRT密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_1024 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为320 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[128];   /**< N值明文，128字节 */
    uint8_t p_value[64];    /**< P值密文，64字节 */
    uint8_t q_value[64];    /**< Q值密文，64字节 */
    uint8_t dp_value[64];   /**< DP值密文，64字节 */
    uint8_t dq_value[64];   /**< DQ值密文，64字节 */
    uint8_t u_value[64];    /**< U值密文，64字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa1024_crt_cipher_keypair_st;

/**
 * @brief 用于辅助RSA2048-CRT私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为640 */
    uint8_t p_value[128];   /**< P值明文，128字节 */
    uint8_t q_value[128];   /**< Q值明文，128字节 */
    uint8_t dp_value[128];  /**< DP值明文，128字节 */
    uint8_t dq_value[128];  /**< DQ值明文，128字节 */
    uint8_t u_value[128];   /**< U值明文，128字节 */
} ehsm_key_format_rsa2048_crt_plain_privkey_st;

/**
 * @brief 用于辅助RSA2048-CRT私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为640 */
    uint8_t p_value[128];   /**< P值密文，128字节 */
    uint8_t q_value[128];   /**< Q值密文，128字节 */
    uint8_t dp_value[128];  /**< DP值密文，128字节 */
    uint8_t dq_value[128];  /**< DQ值密文，128字节 */
    uint8_t u_value[128];   /**< U值密文，128字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa2048_crt_cipher_privkey_st;

/**
 * @brief 用于辅助RSA2048-CRT密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为640 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[256];   /**< N值明文，256字节 */
    uint8_t p_value[128];   /**< P值明文，128字节 */
    uint8_t q_value[128];   /**< Q值明文，128字节 */
    uint8_t dp_value[128];  /**< DP值明文，128字节 */
    uint8_t dq_value[128];  /**< DQ值明文，128字节 */
    uint8_t u_value[128];   /**< U值明文，128字节 */
} ehsm_key_format_rsa2048_crt_plain_keypair_st;

/**
 * @brief 用于辅助RSA2048-CRT密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_2048 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为640 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[256];   /**< N值明文，256字节 */
    uint8_t p_value[128];   /**< P值密文，128字节 */
    uint8_t q_value[128];   /**< Q值密文，128字节 */
    uint8_t dp_value[128];  /**< DP值密文，128字节 */
    uint8_t dq_value[128];  /**< DQ值密文，128字节 */
    uint8_t u_value[128];   /**< U值密文，128字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa2048_crt_cipher_keypair_st;

/**
 * @brief 用于辅助RSA3072-CRT私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为960 */
    uint8_t p_value[192];   /**< P值明文，192字节 */
    uint8_t q_value[192];   /**< Q值明文，192字节 */
    uint8_t dp_value[192];  /**< DP值明文，192字节 */
    uint8_t dq_value[192];  /**< DQ值明文，192字节 */
    uint8_t u_value[192];   /**< U值明文，192字节 */
} ehsm_key_format_rsa3072_crt_plain_privkey_st;

/**
 * @brief 用于辅助RSA3072-CRT私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为960 */
    uint8_t p_value[192];   /**< P值密文，192字节 */
    uint8_t q_value[192];   /**< Q值密文，192字节 */
    uint8_t dp_value[192];  /**< DP值密文，192字节 */
    uint8_t dq_value[192];  /**< DQ值密文，192字节 */
    uint8_t u_value[192];   /**< U值密文，192字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa3072_crt_cipher_privkey_st;

/**
 * @brief 用于辅助RSA3072-CRT密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为960 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[384];   /**< N值明文，384字节 */
    uint8_t p_value[192];   /**< P值明文，192字节 */
    uint8_t q_value[192];   /**< Q值明文，192字节 */
    uint8_t dp_value[192];  /**< DP值明文，192字节 */
    uint8_t dq_value[192];  /**< DQ值明文，192字节 */
    uint8_t u_value[192];   /**< U值明文，192字节 */
} ehsm_key_format_rsa3072_crt_plain_keypair_st;

/**
 * @brief 用于辅助RSA3072-CRT密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_3072 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为960 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[384];   /**< N值明文，384字节 */
    uint8_t p_value[192];   /**< P值密文，192字节 */
    uint8_t q_value[192];   /**< Q值密文，192字节 */
    uint8_t dp_value[192];  /**< DP值密文，192字节 */
    uint8_t dq_value[192];  /**< DQ值密文，192字节 */
    uint8_t u_value[192];   /**< U值密文，192字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa3072_crt_cipher_keypair_st;

/**
 * @brief 用于辅助RSA4096-CRT私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为1280 */
    uint8_t p_value[256];   /**< P值明文，256字节 */
    uint8_t q_value[256];   /**< Q值明文，256字节 */
    uint8_t dp_value[256];  /**< DP值明文，256字节 */
    uint8_t dq_value[256];  /**< DQ值明文，256字节 */
    uint8_t u_value[256];   /**< U值明文，256字节 */
} ehsm_key_format_rsa4096_crt_plain_privkey_st;

/**
 * @brief 用于辅助RSA4096-CRT私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为1280 */
    uint8_t p_value[256];   /**< P值密文，256字节 */
    uint8_t q_value[256];   /**< Q值密文，256字节 */
    uint8_t dp_value[256];  /**< DP值密文，256字节 */
    uint8_t dq_value[256];  /**< DQ值密文，256字节 */
    uint8_t u_value[256];   /**< U值密文，256字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa4096_crt_cipher_privkey_st;

/**
 * @brief 用于辅助RSA4096-CRT密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为1280 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[512];   /**< N值明文，512字节 */
    uint8_t p_value[256];   /**< P值明文，256字节 */
    uint8_t q_value[256];   /**< Q值明文，256字节 */
    uint8_t dp_value[256];  /**< DP值明文，256字节 */
    uint8_t dq_value[256];  /**< DQ值明文，256字节 */
    uint8_t u_value[256];   /**< U值明文，256字节 */
} ehsm_key_format_rsa4096_crt_plain_keypair_st;

/**
 * @brief 用于辅助RSA4096-CRT密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_RSA_4096 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< E值长度，应当设置为8 */
    uint16_t priv_key_size; /**< 5分量明文长度，这里应当设置为1280 */
    uint8_t e_value[8];     /**< E值明文，8字节 */
    uint8_t n_value[512];   /**< N值明文，512字节 */
    uint8_t p_value[256];   /**< P值密文，256字节 */
    uint8_t q_value[256];   /**< Q值密文，256字节 */
    uint8_t dp_value[256];  /**< DP值密文，256字节 */
    uint8_t dq_value[256];  /**< DQ值密文，256字节 */
    uint8_t u_value[256];   /**< U值密文，256字节 */
    uint8_t padding[16];    /**< PKCS7填充后的密文值 */
} ehsm_key_format_rsa4096_crt_cipher_keypair_st;

/**
 * @brief 用于辅助SM2公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_SM2 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为65 */
    uint16_t priv_key_size; /**< 为0 */

    uint8_t prefix;      /**< 前导字节，必须为0x04 */
    uint8_t x_value[32]; /**< 公钥X值明文，32字节 */
    uint8_t y_value[32]; /**< 公钥Y值明文，32字节 */
} ehsm_key_format_sm2_plain_pubkey_st;

/**
 * @brief 用于辅助SM2私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_SM2 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */
} ehsm_key_format_sm2_plain_privkey_st;

/**
 * @brief 用于辅助SM2私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_SM2 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */
} ehsm_key_format_sm2_cipher_privkey_st;

/**
 * @brief 用于辅助SM2密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_SM2 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为65 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t prefix;         /**< 前导字节，必须为0x04 */
    uint8_t x_value[32];    /**< 公钥X值明文，32字节 */
    uint8_t y_value[32];    /**< 公钥Y值明文，32字节 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */

} ehsm_key_format_sm2_plain_keypair_st;

/**
 * @brief 用于辅助SM2密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_SM2 */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为65 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t prefix;         /**< 前导字节，必须为0x04 */
    uint8_t x_value[32];    /**< 公钥X值明文，32字节 */
    uint8_t y_value[32];    /**< 公钥Y值明文，32字节 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */

} ehsm_key_format_sm2_cipher_keypair_st;

/**
 * @brief 用于辅助ECC160公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 160 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为40 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[20];    /**< 公钥X值明文，20字节 */
    uint8_t y_value[20];    /**< 公钥Y值明文，20字节 */
} ehsm_key_format_ecc_160_plain_pubkey_st;

/**
 * @brief 用于辅助ECC160私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 160 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为20 */
    uint8_t priv_value[20]; /**< 私钥明文值，20字节 */
} ehsm_key_format_ecc_160_plain_privkey_st;

/**
 * @brief 用于辅助ECC160私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 160 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为20 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */
} ehsm_key_format_ecc_160_cipher_privkey_st;

/**
 * @brief 用于辅助ECC160密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 160 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为40 */
    uint16_t priv_key_size; /**< 私钥长度，应为20 */
    uint8_t x_value[20];    /**< 公钥X值明文，20字节 */
    uint8_t y_value[20];    /**< 公钥Y值明文，20字节 */
    uint8_t priv_value[20]; /**< 私钥明文值，20字节 */

} ehsm_key_format_ecc_160_plain_keypair_st;

/**
 * @brief 用于辅助ECC160密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 160 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为40 */
    uint16_t priv_key_size; /**< 私钥长度，应为20 */
    uint8_t x_value[20];    /**< 公钥X值明文，20字节 */
    uint8_t y_value[20];    /**< 公钥Y值明文，20字节 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */

} ehsm_key_format_ecc_160_cipher_keypair_st;

/**
 * @brief 用于辅助ECC192公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 192 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为48 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[24];    /**< 公钥X值明文，24字节 */
    uint8_t y_value[24];    /**< 公钥Y值明文，24字节 */
} ehsm_key_format_ecc_192_plain_pubkey_st;

/**
 * @brief 用于辅助ECC192私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 192 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为24 */
    uint8_t priv_value[24]; /**< 私钥明文值，24字节 */
} ehsm_key_format_ecc_192_plain_privkey_st;

/**
 * @brief 用于辅助ECC192私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 192 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为24 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */
} ehsm_key_format_ecc_192_cipher_privkey_st;

/**
 * @brief 用于辅助ECC192密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 192 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为48 */
    uint16_t priv_key_size; /**< 私钥长度，应为24 */
    uint8_t x_value[24];    /**< 公钥X值明文，24字节 */
    uint8_t y_value[24];    /**< 公钥Y值明文，24字节 */
    uint8_t priv_value[24]; /**< 私钥明文值，24字节 */

} ehsm_key_format_ecc_192_plain_keypair_st;

/**
 * @brief 用于辅助ECC192密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 192 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为48 */
    uint16_t priv_key_size; /**< 私钥长度，应为24 */
    uint8_t x_value[24];    /**< 公钥X值明文，24字节 */
    uint8_t y_value[24];    /**< 公钥Y值明文，24字节 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */

} ehsm_key_format_ecc_192_cipher_keypair_st;

/**
 * @brief 用于辅助ECC224公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 224 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为56 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[28];    /**< 公钥X值明文，28字节 */
    uint8_t y_value[28];    /**< 公钥Y值明文，28字节 */
} ehsm_key_format_ecc_224_plain_pubkey_st;

/**
 * @brief 用于辅助ECC224私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 224 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为28 */
    uint8_t priv_value[28]; /**< 私钥明文值，28字节 */
} ehsm_key_format_ecc_224_plain_privkey_st;

/**
 * @brief 用于辅助ECC224私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 224 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为28 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */
} ehsm_key_format_ecc_224_cipher_privkey_st;

/**
 * @brief 用于辅助ECC224密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 224 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为56 */
    uint16_t priv_key_size; /**< 私钥长度，应为28 */
    uint8_t x_value[28];    /**< 公钥X值明文，28字节 */
    uint8_t y_value[28];    /**< 公钥Y值明文，28字节 */
    uint8_t priv_value[28]; /**< 私钥明文值，28字节 */

} ehsm_key_format_ecc_224_plain_keypair_st;

/**
 * @brief 用于辅助ECC224密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 224 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为56 */
    uint16_t priv_key_size; /**< 私钥长度，应为28 */
    uint8_t x_value[28];    /**< 公钥X值明文，28字节 */
    uint8_t y_value[28];    /**< 公钥Y值明文，28字节 */
    uint8_t priv_value[32]; /**< 私钥密文值，有PKCS7填充，32字节 */

} ehsm_key_format_ecc_224_cipher_keypair_st;

/**
 * @brief 用于辅助ECC256公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 256 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为64 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[32];    /**< 公钥X值明文，32字节 */
    uint8_t y_value[32];    /**< 公钥Y值明文，32字节 */
} ehsm_key_format_ecc_256_plain_pubkey_st;

/**
 * @brief 用于辅助ECC256私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 256 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */
} ehsm_key_format_ecc_256_plain_privkey_st;

/**
 * @brief 用于辅助ECC256私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 256 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */
} ehsm_key_format_ecc_256_cipher_privkey_st;

/**
 * @brief 用于辅助ECC256密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 256 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为64 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t x_value[32];    /**< 公钥X值明文，32字节 */
    uint8_t y_value[32];    /**< 公钥Y值明文，32字节 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */

} ehsm_key_format_ecc_256_plain_keypair_st;

/**
 * @brief 用于辅助ECC256密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 256 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为64 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t x_value[32];    /**< 公钥X值明文，32字节 */
    uint8_t y_value[32];    /**< 公钥Y值明文，32字节 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */

} ehsm_key_format_ecc_256_cipher_keypair_st;

/**
 * @brief 用于辅助ECC320公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 320 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为80 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[40];    /**< 公钥X值明文，40字节 */
    uint8_t y_value[40];    /**< 公钥Y值明文，40字节 */
} ehsm_key_format_ecc_320_plain_pubkey_st;

/**
 * @brief 用于辅助ECC320私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 320 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为40 */
    uint8_t priv_value[40]; /**< 私钥明文值，40字节 */
} ehsm_key_format_ecc_320_plain_privkey_st;

/**
 * @brief 用于辅助ECC320私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 320 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为40 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */
} ehsm_key_format_ecc_320_cipher_privkey_st;

/**
 * @brief 用于辅助ECC320密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 320 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为80 */
    uint16_t priv_key_size; /**< 私钥长度，应为40 */
    uint8_t x_value[40];    /**< 公钥X值明文，40字节 */
    uint8_t y_value[40];    /**< 公钥Y值明文，40字节 */
    uint8_t priv_value[40]; /**< 私钥明文值，40字节 */

} ehsm_key_format_ecc_320_plain_keypair_st;

/**
 * @brief 用于辅助ECC320密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 320 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为80 */
    uint16_t priv_key_size; /**< 私钥长度，应为40 */
    uint8_t x_value[40];    /**< 公钥X值明文，40字节 */
    uint8_t y_value[40];    /**< 公钥Y值明文，40字节 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */

} ehsm_key_format_ecc_320_cipher_keypair_st;

/**
 * @brief 用于辅助ECC384公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 384 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为96 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[48];    /**< 公钥X值明文，48字节 */
    uint8_t y_value[48];    /**< 公钥Y值明文，48字节 */
} ehsm_key_format_ecc_384_plain_pubkey_st;

/**
 * @brief 用于辅助ECC384私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 384 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为48 */
    uint8_t priv_value[48]; /**< 私钥明文值，48字节 */
} ehsm_key_format_ecc_384_plain_privkey_st;

/**
 * @brief 用于辅助ECC384私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 384 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为48 */
    uint8_t priv_value[64]; /**< 私钥密文值，有PKCS7填充，64字节 */
} ehsm_key_format_ecc_384_cipher_privkey_st;

/**
 * @brief 用于辅助ECC384密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 384 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为96 */
    uint16_t priv_key_size; /**< 私钥长度，应为48 */
    uint8_t x_value[48];    /**< 公钥X值明文，48字节 */
    uint8_t y_value[48];    /**< 公钥Y值明文，48字节 */
    uint8_t priv_value[48]; /**< 私钥明文值，48字节 */

} ehsm_key_format_ecc_384_plain_keypair_st;

/**
 * @brief 用于辅助ECC384密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 384 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为96 */
    uint16_t priv_key_size; /**< 私钥长度，应为48 */
    uint8_t x_value[48];    /**< 公钥X值明文，48字节 */
    uint8_t y_value[48];    /**< 公钥Y值明文，48字节 */
    uint8_t priv_value[64]; /**< 私钥密文值，有PKCS7填充，64字节 */

} ehsm_key_format_ecc_384_cipher_keypair_st;

/**
 * @brief 用于辅助ECC512公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 512 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为128 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[64];    /**< 公钥X值明文，64字节 */
    uint8_t y_value[64];    /**< 公钥Y值明文，64字节 */
} ehsm_key_format_ecc_512_plain_pubkey_st;

/**
 * @brief 用于辅助ECC512私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 512 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为64 */
    uint8_t priv_value[64]; /**< 私钥明文值，64字节 */
} ehsm_key_format_ecc_512_plain_privkey_st;

/**
 * @brief 用于辅助ECC512私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 512 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为64 */
    uint8_t priv_value[80]; /**< 私钥密文值，有PKCS7填充，80字节 */
} ehsm_key_format_ecc_512_cipher_privkey_st;

/**
 * @brief 用于辅助ECC512密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 512 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为128 */
    uint16_t priv_key_size; /**< 私钥长度，应为64 */
    uint8_t x_value[64];    /**< 公钥X值明文，64字节 */
    uint8_t y_value[64];    /**< 公钥Y值明文，64字节 */
    uint8_t priv_value[64]; /**< 私钥明文值，64字节 */

} ehsm_key_format_ecc_512_plain_keypair_st;

/**
 * @brief 用于辅助ECC512密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 512 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为128 */
    uint16_t priv_key_size; /**< 私钥长度，应为64 */
    uint8_t x_value[64];    /**< 公钥X值明文，64字节 */
    uint8_t y_value[64];    /**< 公钥Y值明文，64字节 */
    uint8_t priv_value[80]; /**< 私钥密文值，有PKCS7填充，80字节 */

} ehsm_key_format_ecc_512_cipher_keypair_st;

/**
 * @brief 用于辅助ECC521公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 521 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为132 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t x_value[66];    /**< 公钥X值明文，66字节 */
    uint8_t y_value[66];    /**< 公钥Y值明文，66字节 */
} ehsm_key_format_ecc_521_plain_pubkey_st;

/**
 * @brief 用于辅助ECC521私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 521 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为66 */
    uint8_t priv_value[66]; /**< 私钥明文值，66字节 */
} ehsm_key_format_ecc_521_plain_privkey_st;

/**
 * @brief 用于辅助ECC521私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 521 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为66 */
    uint8_t priv_value[80]; /**< 私钥密文值，有PKCS7填充，80字节 */
} ehsm_key_format_ecc_521_cipher_privkey_st;

/**
 * @brief 用于辅助ECC521密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 521 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为132 */
    uint16_t priv_key_size; /**< 私钥长度，应为66 */
    uint8_t x_value[66];    /**< 公钥X值明文，66字节 */
    uint8_t y_value[66];    /**< 公钥Y值明文，66字节 */
    uint8_t priv_value[66]; /**< 私钥明文值，66字节 */

} ehsm_key_format_ecc_521_plain_keypair_st;

/**
 * @brief 用于辅助ECC521密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 521 位的ECC曲线参数，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为132 */
    uint16_t priv_key_size; /**< 私钥长度，应为66 */
    uint8_t x_value[66];    /**< 公钥X值明文，66字节 */
    uint8_t y_value[66];    /**< 公钥Y值明文，66字节 */
    uint8_t priv_value[80]; /**< 私钥密文值，有PKCS7填充，80字节 */

} ehsm_key_format_ecc_521_cipher_keypair_st;

typedef ehsm_key_format_ecc_160_plain_pubkey_st ehsm_key_format_ecc_bp_p160r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_160_plain_privkey_st ehsm_key_format_ecc_bp_p160r1_plain_privkey_st;
typedef ehsm_key_format_ecc_160_cipher_privkey_st ehsm_key_format_ecc_bp_p160r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_160_plain_keypair_st ehsm_key_format_ecc_bp_p160r1_plain_keypair_st;
typedef ehsm_key_format_ecc_160_cipher_keypair_st ehsm_key_format_ecc_bp_p160r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_192_plain_pubkey_st ehsm_key_format_ecc_bp_p192r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_192_plain_privkey_st ehsm_key_format_ecc_bp_p192r1_plain_privkey_st;
typedef ehsm_key_format_ecc_192_cipher_privkey_st ehsm_key_format_ecc_bp_p192r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_192_plain_keypair_st ehsm_key_format_ecc_bp_p192r1_plain_keypair_st;
typedef ehsm_key_format_ecc_192_cipher_keypair_st ehsm_key_format_ecc_bp_p192r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_224_plain_pubkey_st ehsm_key_format_ecc_bp_p224r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_224_plain_privkey_st ehsm_key_format_ecc_bp_p224r1_plain_privkey_st;
typedef ehsm_key_format_ecc_224_cipher_privkey_st ehsm_key_format_ecc_bp_p224r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_224_plain_keypair_st ehsm_key_format_ecc_bp_p224r1_plain_keypair_st;
typedef ehsm_key_format_ecc_224_cipher_keypair_st ehsm_key_format_ecc_bp_p224r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_256_plain_pubkey_st ehsm_key_format_ecc_bp_p256r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_256_plain_privkey_st ehsm_key_format_ecc_bp_p256r1_plain_privkey_st;
typedef ehsm_key_format_ecc_256_cipher_privkey_st ehsm_key_format_ecc_bp_p256r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_256_plain_keypair_st ehsm_key_format_ecc_bp_p256r1_plain_keypair_st;
typedef ehsm_key_format_ecc_256_cipher_keypair_st ehsm_key_format_ecc_bp_p256r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_320_plain_pubkey_st ehsm_key_format_ecc_bp_p320r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_320_plain_privkey_st ehsm_key_format_ecc_bp_p320r1_plain_privkey_st;
typedef ehsm_key_format_ecc_320_cipher_privkey_st ehsm_key_format_ecc_bp_p320r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_320_plain_keypair_st ehsm_key_format_ecc_bp_p320r1_plain_keypair_st;
typedef ehsm_key_format_ecc_320_cipher_keypair_st ehsm_key_format_ecc_bp_p320r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_384_plain_pubkey_st ehsm_key_format_ecc_bp_p384r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_384_plain_privkey_st ehsm_key_format_ecc_bp_p384r1_plain_privkey_st;
typedef ehsm_key_format_ecc_384_cipher_privkey_st ehsm_key_format_ecc_bp_p384r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_384_plain_keypair_st ehsm_key_format_ecc_bp_p384r1_plain_keypair_st;
typedef ehsm_key_format_ecc_384_cipher_keypair_st ehsm_key_format_ecc_bp_p384r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_512_plain_pubkey_st ehsm_key_format_ecc_bp_p512r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_512_plain_privkey_st ehsm_key_format_ecc_bp_p512r1_plain_privkey_st;
typedef ehsm_key_format_ecc_512_cipher_privkey_st ehsm_key_format_ecc_bp_p512r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_512_plain_keypair_st ehsm_key_format_ecc_bp_p512r1_plain_keypair_st;
typedef ehsm_key_format_ecc_512_cipher_keypair_st ehsm_key_format_ecc_bp_p512r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_192_plain_pubkey_st ehsm_key_format_ecc_sec_p192r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_192_plain_privkey_st ehsm_key_format_ecc_sec_p192r1_plain_privkey_st;
typedef ehsm_key_format_ecc_192_cipher_privkey_st ehsm_key_format_ecc_sec_p192r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_192_plain_keypair_st ehsm_key_format_ecc_sec_p192r1_plain_keypair_st;
typedef ehsm_key_format_ecc_192_cipher_keypair_st ehsm_key_format_ecc_sec_p192r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_224_plain_pubkey_st ehsm_key_format_ecc_sec_p224r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_224_plain_privkey_st ehsm_key_format_ecc_sec_p224r1_plain_privkey_st;
typedef ehsm_key_format_ecc_224_cipher_privkey_st ehsm_key_format_ecc_sec_p224r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_224_plain_keypair_st ehsm_key_format_ecc_sec_p224r1_plain_keypair_st;
typedef ehsm_key_format_ecc_224_cipher_keypair_st ehsm_key_format_ecc_sec_p224r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_256_plain_pubkey_st ehsm_key_format_ecc_sec_p256r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_256_plain_privkey_st ehsm_key_format_ecc_sec_p256r1_plain_privkey_st;
typedef ehsm_key_format_ecc_256_cipher_privkey_st ehsm_key_format_ecc_sec_p256r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_256_plain_keypair_st ehsm_key_format_ecc_sec_p256r1_plain_keypair_st;
typedef ehsm_key_format_ecc_256_cipher_keypair_st ehsm_key_format_ecc_sec_p256r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_384_plain_pubkey_st ehsm_key_format_ecc_sec_p384r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_384_plain_privkey_st ehsm_key_format_ecc_sec_p384r1_plain_privkey_st;
typedef ehsm_key_format_ecc_384_cipher_privkey_st ehsm_key_format_ecc_sec_p384r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_384_plain_keypair_st ehsm_key_format_ecc_sec_p384r1_plain_keypair_st;
typedef ehsm_key_format_ecc_384_cipher_keypair_st ehsm_key_format_ecc_sec_p384r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_521_plain_pubkey_st ehsm_key_format_ecc_sec_p521r1_plain_pubkey_st;
typedef ehsm_key_format_ecc_521_plain_privkey_st ehsm_key_format_ecc_sec_p521r1_plain_privkey_st;
typedef ehsm_key_format_ecc_521_cipher_privkey_st ehsm_key_format_ecc_sec_p521r1_cipher_privkey_st;
typedef ehsm_key_format_ecc_521_plain_keypair_st ehsm_key_format_ecc_sec_p521r1_plain_keypair_st;
typedef ehsm_key_format_ecc_521_cipher_keypair_st ehsm_key_format_ecc_sec_p521r1_cipher_keypair_st;

typedef ehsm_key_format_ecc_160_plain_pubkey_st ehsm_key_format_ecc_sec_p160k1_plain_pubkey_st;
typedef ehsm_key_format_ecc_160_plain_privkey_st ehsm_key_format_ecc_sec_p160k1_plain_privkey_st;
typedef ehsm_key_format_ecc_160_cipher_privkey_st ehsm_key_format_ecc_sec_p160k1_cipher_privkey_st;
typedef ehsm_key_format_ecc_160_plain_keypair_st ehsm_key_format_ecc_sec_p160k1_plain_keypair_st;
typedef ehsm_key_format_ecc_160_cipher_keypair_st ehsm_key_format_ecc_sec_p160k1_cipher_keypair_st;

typedef ehsm_key_format_ecc_192_plain_pubkey_st ehsm_key_format_ecc_sec_p192k1_plain_pubkey_st;
typedef ehsm_key_format_ecc_192_plain_privkey_st ehsm_key_format_ecc_sec_p192k1_plain_privkey_st;
typedef ehsm_key_format_ecc_192_cipher_privkey_st ehsm_key_format_ecc_sec_p192k1_cipher_privkey_st;
typedef ehsm_key_format_ecc_192_plain_keypair_st ehsm_key_format_ecc_sec_p192k1_plain_keypair_st;
typedef ehsm_key_format_ecc_192_cipher_keypair_st ehsm_key_format_ecc_sec_p192k1_cipher_keypair_st;

typedef ehsm_key_format_ecc_224_plain_pubkey_st ehsm_key_format_ecc_sec_p224k1_plain_pubkey_st;
typedef ehsm_key_format_ecc_224_plain_privkey_st ehsm_key_format_ecc_sec_p224k1_plain_privkey_st;
typedef ehsm_key_format_ecc_224_cipher_privkey_st ehsm_key_format_ecc_sec_p224k1_cipher_privkey_st;
typedef ehsm_key_format_ecc_224_plain_keypair_st ehsm_key_format_ecc_sec_p224k1_plain_keypair_st;
typedef ehsm_key_format_ecc_224_cipher_keypair_st ehsm_key_format_ecc_sec_p224k1_cipher_keypair_st;

typedef ehsm_key_format_ecc_256_plain_pubkey_st ehsm_key_format_ecc_sec_p256k1_plain_pubkey_st;
typedef ehsm_key_format_ecc_256_plain_privkey_st ehsm_key_format_ecc_sec_p256k1_plain_privkey_st;
typedef ehsm_key_format_ecc_256_cipher_privkey_st ehsm_key_format_ecc_sec_p256k1_cipher_privkey_st;
typedef ehsm_key_format_ecc_256_plain_keypair_st ehsm_key_format_ecc_sec_p256k1_plain_keypair_st;
typedef ehsm_key_format_ecc_256_cipher_keypair_st ehsm_key_format_ecc_sec_p256k1_cipher_keypair_st;

/**
 * @brief 用于辅助ED25519/X25519公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 ED25519/X25519类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应当设置为32 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t pub_value[32];  /**< 公钥值明文，32字节 */
} ehsm_key_format_ecc_25519_plain_pubkey_st;

/**
 * @brief 用于辅助ED25519/X25519私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 ED25519/X25519类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */
} ehsm_key_format_ecc_25519_plain_privkey_st;

/**
 * @brief 用于辅助ED25519/X25519私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 ED25519/X25519类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 为0 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */
} ehsm_key_format_ecc_25519_cipher_privkey_st;

/**
 * @brief 用于辅助ED25519/X25519密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 ED25519/X25519类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为32 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t pub_value[32];  /**< 公钥值明文，32字节 */
    uint8_t priv_value[32]; /**< 私钥明文值，32字节 */
} ehsm_key_format_ecc_25519_plain_keypair_st;

/**
 * @brief 用于辅助ED25519/X25519密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 ED25519/X25519类型，见 @ref ehsm_key_type_e */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为32 */
    uint16_t priv_key_size; /**< 私钥长度，应为32 */
    uint8_t pub_value[32];  /**< 公钥值明文，32字节 */
    uint8_t priv_value[48]; /**< 私钥密文值，有PKCS7填充，48字节 */
} ehsm_key_format_ecc_25519_cipher_keypair_st;

/**
 * @brief 用于辅助DH 1024位公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为128 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t pub_value[128]; /**< 公钥值明文，128字节 */
} ehsm_key_format_dh_1024_plain_pubkey_st;

/**
 * @brief 用于辅助DH 1024位私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为128 */
    uint8_t priv_value[128]; /**< 私钥值明文，128字节 */
} ehsm_key_format_dh_1024_plain_privkey_st;

/**
 * @brief 用于辅助DH 1024位私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为128 */
    uint8_t priv_value[144]; /**< 私钥值密文，有PKCS7填充，144字节 */
} ehsm_key_format_dh_1024_cipher_privkey_st;

/**
 * @brief 用于辅助DH 1024位密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为128 */
    uint16_t priv_key_size;  /**< 私钥长度，应为128 */
    uint8_t pub_value[128];  /**< 公钥值明文，128字节 */
    uint8_t priv_value[128]; /**< 私钥值明文，128字节 */
} ehsm_key_format_dh_1024_plain_keypair_st;

/**
 * @brief 用于辅助DH 1024位密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为128 */
    uint16_t priv_key_size;  /**< 私钥长度，应为128 */
    uint8_t pub_value[128];  /**< 公钥值明文，128字节 */
    uint8_t priv_value[144]; /**< 私钥值密文，有PKCS7填充，144字节 */
} ehsm_key_format_dh_1024_cipher_keypair_st;

/**
 * @brief 用于辅助DH 2048位公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为256 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t pub_value[256]; /**< 公钥值明文，256字节 */
} ehsm_key_format_dh_2048_plain_pubkey_st;

/**
 * @brief 用于辅助DH 2048位私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为256 */
    uint8_t priv_value[256]; /**< 私钥值明文，256字节 */
} ehsm_key_format_dh_2048_plain_privkey_st;

/**
 * @brief 用于辅助DH 2048位私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为256 */
    uint8_t priv_value[272]; /**< 私钥值密文，有PKCS7填充，272字节 */
} ehsm_key_format_dh_2048_cipher_privkey_st;

/**
 * @brief 用于辅助DH 2048位密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为256 */
    uint16_t priv_key_size;  /**< 私钥长度，应为256 */
    uint8_t pub_value[256];  /**< 公钥值明文，256字节 */
    uint8_t priv_value[256]; /**< 私钥值明文，256字节 */
} ehsm_key_format_dh_2048_plain_keypair_st;

/**
 * @brief 用于辅助DH 2048位密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为256 */
    uint16_t priv_key_size;  /**< 私钥长度，应为256 */
    uint8_t pub_value[256];  /**< 公钥值明文，256字节 */
    uint8_t priv_value[272]; /**< 私钥值密文，有PKCS7填充，272字节 */
} ehsm_key_format_dh_2048_cipher_keypair_st;

/**
 * @brief 用于辅助DH 3072位公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为384 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t pub_value[384]; /**< 公钥值明文，384字节 */
} ehsm_key_format_dh_3072_plain_pubkey_st;

/**
 * @brief 用于辅助DH 3072位私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为384 */
    uint8_t priv_value[384]; /**< 私钥值明文，384字节 */
} ehsm_key_format_dh_3072_plain_privkey_st;

/**
 * @brief 用于辅助DH 3072位私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为384 */
    uint8_t priv_value[400]; /**< 私钥值密文，有PKCS7填充，400字节 */
} ehsm_key_format_dh_3072_cipher_privkey_st;

/**
 * @brief 用于辅助DH 3072位密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为384 */
    uint16_t priv_key_size;  /**< 私钥长度，应为384 */
    uint8_t pub_value[384];  /**< 公钥值明文，384字节 */
    uint8_t priv_value[384]; /**< 私钥值明文，384字节 */
} ehsm_key_format_dh_3072_plain_keypair_st;

/**
 * @brief 用于辅助DH 3072位密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为384 */
    uint16_t priv_key_size;  /**< 私钥长度，应为384 */
    uint8_t pub_value[384];  /**< 公钥值明文，384字节 */
    uint8_t priv_value[400]; /**< 私钥值密文，有PKCS7填充，400字节 */
} ehsm_key_format_dh_3072_cipher_keypair_st;

/**
 * @brief 用于辅助DH 4096位公钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;     /**< 权限位，见 @ref key-priv */
    uint8_t key_type;       /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;      /**< 应当设置为 @ref EHSM_KEY_PART_PUBLIC_KEY */
    uint8_t reserved[2];    /**< 保留 */
    uint16_t pub_key_size;  /**< 公钥长度，应为512 */
    uint16_t priv_key_size; /**< 为0 */
    uint8_t pub_value[512]; /**< 公钥值明文，512字节 */
} ehsm_key_format_dh_4096_plain_pubkey_st;

/**
 * @brief 用于辅助DH 4096位私钥明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为512 */
    uint8_t priv_value[512]; /**< 私钥值明文，512字节 */
} ehsm_key_format_dh_4096_plain_privkey_st;

/**
 * @brief 用于辅助DH 4096位私钥密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_PRIVATE_KEY */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 为0 */
    uint16_t priv_key_size;  /**< 私钥长度，应为512 */
    uint8_t priv_value[528]; /**< 私钥值密文，有PKCS7填充，528字节 */
} ehsm_key_format_dh_4096_cipher_privkey_st;

/**
 * @brief 用于辅助DH 4096位密钥对明文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为512 */
    uint16_t priv_key_size;  /**< 私钥长度，应为512 */
    uint8_t pub_value[512];  /**< 公钥值明文，512字节 */
    uint8_t priv_value[512]; /**< 私钥值明文，512字节 */
} ehsm_key_format_dh_4096_plain_keypair_st;

/**
 * @brief 用于辅助DH 4096位密钥对密文导入/导出的结构体。
 */
typedef struct {
    uint32_t privilege;      /**< 权限位，见 @ref key-priv */
    uint8_t key_type;        /**< 密钥类型，应当设置为 @ref EHSM_KEY_TYPE_DH */
    uint8_t part_info;       /**< 应当设置为 @ref EHSM_KEY_PART_KEY_PAIR */
    uint8_t reserved[2];     /**< 保留 */
    uint16_t pub_key_size;   /**< 公钥长度，应为512 */
    uint16_t priv_key_size;  /**< 私钥长度，应为512 */
    uint8_t pub_value[512];  /**< 公钥值明文，512字节 */
    uint8_t priv_value[528]; /**< 私钥值密文，有PKCS7填充，528字节 */
} ehsm_key_format_dh_4096_cipher_keypair_st;

/** @} */

#pragma pack()

#endif // EHSM_KEY_ASSIST_H
