#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_symm_cipher.h"

#define DEMO_SYMM_CIPHER_KEY_HANDLE (0x100000U)

typedef struct {
    ehsm_key_type_e key_type;
    ehsm_symm_algo_e algo;
    ehsm_cipher_mode_e cipher_mode;
    ehsm_padding_mode_e padding;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    const uint8_t *std_iv;
    uint32_t std_iv_size;
    const uint8_t *std_plaintext;
    uint32_t std_plaintext_size;
    const uint8_t *std_ciphertext;
    uint32_t std_ciphertext_size;
} demo_symm_cipher_std_st;

// clang-format off
static const uint8_t s_symm_cipher_std_key[64] = {
    0x89,0xB7,0xA3,0x35,0x92,0xE1,0x0D,0xB7,0xA0,0xB0,0xE2,0x7E,0x16,0xB0,0xE2,0xF1,
    0xE3,0xCA,0x70,0xDE,0xFB,0xA1,0x6F,0x3F,0x1E,0xF8,0x71,0xB0,0x29,0x04,0xBB,0x1D,
    0x53,0x16,0x33,0x6C,0xAA,0x0E,0xD0,0xD1,0x61,0x03,0x27,0x99,0xF3,0x73,0xE4,0x98,
    0x05,0x84,0xB5,0x79,0x68,0xA7,0x67,0xB4,0xF6,0x4D,0x85,0x8B,0xDB,0xB6,0xE7,0xC8
};

static const uint8_t s_symm_cipher_std_iv[16] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};

static const uint8_t s_symm_cipher_std_plaintext[64] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f
};

static const uint8_t s_aes128_ecb_std_ciphertext[64] = {
    0x9E,0x0C,0xD6,0xEF,0xC2,0x3A,0x96,0x3E,0x12,0x07,0xF0,0x41,0x5D,0xF5,0xB4,0x3C,
    0x93,0xA8,0x57,0x12,0x83,0x94,0xF8,0xC7,0x88,0xF0,0x57,0xCF,0x1E,0xE0,0xC7,0x91,
    0x75,0x36,0xF4,0xC8,0xCA,0xF9,0xB9,0xAE,0x72,0x58,0x88,0xCD,0x00,0x18,0xA4,0x5D,
    0x10,0x49,0x4B,0xFF,0x74,0x88,0x38,0x31,0xED,0x57,0x90,0xDF,0x10,0x81,0xCC,0xDD
};

static const uint8_t s_aes256_cbc_std_ciphertext_pkcs7[80] = {
    0x7D,0xF5,0xF0,0xA4,0xF6,0x41,0xB7,0xCE,0x11,0x0C,0xAC,0xA0,0xDC,0x4E,0x93,0x68,
    0xAE,0xD9,0x7E,0x01,0x91,0x76,0xC3,0x4E,0x46,0x39,0xFD,0x55,0xE6,0xA7,0x48,0xC5,
    0xBC,0xF6,0x2E,0xA8,0xB6,0x33,0xAF,0x5A,0x1E,0x70,0x1D,0x64,0x88,0xDF,0x07,0xE0,
    0xB7,0xB2,0xA9,0xA6,0x8E,0x23,0xB6,0x8B,0x10,0xC6,0x83,0xDA,0x5E,0x0E,0x9E,0x2C,
    0xA0,0x78,0xC3,0xDC,0xD4,0x37,0xBC,0x5B,0x40,0x95,0x8A,0x34,0xD8,0x03,0x46,0xC9
};

static const uint8_t s_sm4_xts_std_ciphertext[64] = {
    0xFB,0x33,0xE5,0x7D,0xE7,0x93,0x12,0x4F,0x71,0xCE,0xD5,0xC9,0x9E,0xCA,0xBD,0xC0,
    0x05,0xC5,0x1F,0x9A,0xFE,0xCE,0x84,0x26,0xE0,0x27,0xB7,0x72,0xE5,0x7A,0x37,0x5C,
    0x1D,0x17,0xB0,0xAA,0x87,0xF3,0x13,0xED,0xA3,0x5F,0xB4,0xF7,0x85,0x39,0xB0,0xB1,
    0xFA,0x51,0xC7,0xCA,0xB5,0x1E,0xA3,0x62,0x9D,0x2F,0xE5,0x59,0x11,0xFD,0xD0,0xC1
};

static const demo_symm_cipher_std_st s_symm_cipher_std_data_arr[3] = {
    {
        .algo = EHSM_SYMM_ALGO_AES_128,
        .cipher_mode = EHSM_CIPHER_MODE_ECB,
        .padding = EHSM_PADDING_NONE,
        .alg_str = "AES_128_ECB_PADDING_NONE",
        .key_type = EHSM_KEY_TYPE_AES_128,
        .std_key = s_symm_cipher_std_key,
        .std_key_size = 16U,
        .std_iv = NULL,   /* ECB 无需 iv */
        .std_iv_size = 0U,/* ECB 无需 iv */
        .std_plaintext = s_symm_cipher_std_plaintext,
        .std_plaintext_size = sizeof(s_symm_cipher_std_plaintext),
        .std_ciphertext = s_aes128_ecb_std_ciphertext,
        .std_ciphertext_size = sizeof(s_aes128_ecb_std_ciphertext),
    },{
        .algo = EHSM_SYMM_ALGO_AES_256,
        .cipher_mode = EHSM_CIPHER_MODE_CBC,
        .padding = EHSM_PADDING_PKCS7,
        .alg_str = "AES_256_CBC_PADDING_PKCS7",
        .key_type = EHSM_KEY_TYPE_AES_256,
        .std_key = s_symm_cipher_std_key,
        .std_key_size = 32U,
        .std_iv = s_symm_cipher_std_iv,
        .std_iv_size = sizeof(s_symm_cipher_std_iv),
        .std_plaintext = s_symm_cipher_std_plaintext,
        .std_plaintext_size = sizeof(s_symm_cipher_std_plaintext),
        .std_ciphertext = s_aes256_cbc_std_ciphertext_pkcs7,
        .std_ciphertext_size = sizeof(s_aes256_cbc_std_ciphertext_pkcs7),
    },{
        .algo = EHSM_SYMM_ALGO_SM4,
        .cipher_mode = EHSM_CIPHER_MODE_XTS,
        .padding = EHSM_PADDING_NONE,
        .alg_str = "SM4_XTS",
        .key_type = EHSM_KEY_TYPE_SM4_XTS,
        .std_key = s_symm_cipher_std_key,
        .std_key_size = 32U,
        .std_iv = s_symm_cipher_std_iv,
        .std_iv_size = sizeof(s_symm_cipher_std_iv),
        .std_plaintext = s_symm_cipher_std_plaintext,
        .std_plaintext_size = sizeof(s_symm_cipher_std_plaintext),
        .std_ciphertext = s_sm4_xts_std_ciphertext,
        .std_ciphertext_size = sizeof(s_sm4_xts_std_ciphertext),
    }
};
// clang-format on

/**
 * @brief 一次性计算对称算法加解密示例，使用 onepass APIs。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_symm_cipher_std_st。
 *
 */
static void symm_cipher_onepass(bool_t enc, const demo_symm_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_SYMM_CIPHER_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    uint8_t *iv = ehsm_demo_get_buffer(1);     /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size;                          /* 输入 iv 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(2);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *output = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size; /* 指定输出的 buffer 长度，eSHM 会回写实际返回的 output size */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    bool_t need_remove = false;             /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(iv, 0x0U, 16U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    iv_size = std_data->std_iv_size;
    memcpy(iv, std_data->std_iv, iv_size);    /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    if (EHSM_CIPHER_MODE_ECB != std_data->cipher_mode) {
        print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    }
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT), std_data->key_type, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 输出 buffer 长度，
         * - 非 XTS 模式：
         *      - 若没有 padidng，要求大于等于 input size；
         *      - 若有 padding，要求大于等于 input_size +block size;
         *  - XTS 模式，要求大于等于 input size；
         */
        output_size = std_input_size + 32U;

        /**
         * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、填充模式、密钥 key handle、是否加密和 eHSM
         * 初始化分段计算的 session。
         * output_size 首先作为输入，表示 output buffer 的长度，必须足够大，来容纳输出数据；其次当加解密完成后，API
         * 会回写输出数据的实际长度给 output_size。
         */
        ret = ehsm_symm_cipher_onepass(ctx, std_data->algo, std_data->cipher_mode, std_data->padding, key_handle, enc,
            iv, iv_size, input, std_input_size, output, &output_size);
        ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，需要校验输出数据和长度 */
            ret = demo_check_data(NULL, std_output, std_output_size, output, output_size);
        }
    }

    if (need_remove) {
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算对称算法加解密示例，使用 onepass APIs, 密钥使用明文密钥。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_symm_cipher_std_st。
 *
 */
static void symm_cipher_onepass_with_plain_key(bool_t enc, const demo_symm_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);    /* 存储密钥值的buffer */
    uint8_t *iv = ehsm_demo_get_buffer(1);     /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size;                          /* 输入 iv 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(2);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *output = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size; /* 指定输出的 buffer 长度，eSHM 会回写实际返回的 output size */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */

    /* 初始化数据 buffer */
    memset(iv, 0x0U, 16U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    iv_size = std_data->std_iv_size;
    memcpy(iv, std_data->std_iv, iv_size);    /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 将密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    if (EHSM_CIPHER_MODE_ECB != std_data->cipher_mode) {
        print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    }
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 输出 buffer 长度，
     * - 非 XTS 模式：
     *      - 若没有 padidng，要求大于等于 input size；
     *      - 若有 padding，要求大于等于 input_size +block size;
     *  - XTS 模式，要求大于等于 input size；
     */
    output_size = std_input_size + 32U;

    /**
     * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、填充模式、密钥 key handle、是否加密和 eHSM
     * 初始化分段计算的 session。
     * output_size 首先作为输入，表示 output buffer 的长度，必须足够大，来容纳输出数据；其次当加解密完成后，API
     * 会回写输出数据的实际长度给 output_size。
     */
    ret = ehsm_symm_cipher_onepass_with_plain_key(ctx, std_data->algo, std_data->cipher_mode, std_data->padding, key,
        std_data->std_key_size, enc, iv, iv_size, input, std_input_size, output, &output_size);
    ret = demo_check_val("The execution of cipher API:", EHSM_OK, ret);
    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，需要校验输出数据和长度 */
        ret = demo_check_data(NULL, std_output, std_output_size, output, output_size);

        ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
        ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
    }
}

/**
 * @brief 分段计算对称算法加解密示例，使用 init、update 和 finish APIs。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_symm_cipher_std_st。
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 对于 init API，若明文不是 block size 的整数倍，则需要配置有效填充模式，且非 EHSM_PADDING_NONE，
 *      @ref ehsm_padding_mode_e。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加解密计算，并忽略任何中间数据，如 ctx，session
 * 等。
 */
static void symm_cipher_stepwise(bool_t enc, const demo_symm_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_SYMM_CIPHER_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    uint8_t *iv = ehsm_demo_get_buffer(1);     /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size;                          /* 输入 iv 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(2);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经加密/解密的数据总长度 */

    const uint8_t *std_input = NULL;  /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;          /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL; /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;         /* 用于指定标准输出的总长度 */

    /**
     * Finish API 使用：
     * - 首先作为输入，用于指定 output buffer 的长度
     *      - 最后一段输入数据无需 padding 时，无论加解密都必须大于等于最后一段输入长度；
     *      - 最后一段输入数据需要 padding 时，
     *          - 若为加密，则必须大于等于最后一段加上 padding 后的长度；
     *          - 若为解密，则必须大于等于最后一段去掉 padding 后的长度；
     * - 当 finish API 成功执行后，用于接收最后一段输出数据的长度。
     */
    uint32_t last_output_size;

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key_data, 0x0U, 512U);
    memset(iv, 0x0U, 16U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    iv_size = std_data->std_iv_size;
    memcpy(iv, std_data->std_iv, iv_size);    /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    if (EHSM_CIPHER_MODE_ECB != std_data->cipher_mode) {
        print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    }
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT), std_data->key_type, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、填充模式、密钥 key handle、是否加密和 eHSM
         * 初始化分段计算的 session。
         * - Init AP 需要指定本次 aead 计算的输入数据的总长度。
         */
        ret = ehsm_symm_cipher_init(
            ctx, std_data->algo, std_data->cipher_mode, std_data->padding, key_handle, enc, iv, iv_size, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、输入数据、输入数据的字节长度、输出 buffer，eHSM
         * 计算完成后回写输出数据到输出 buffer。update API 仅支持 block size 整数倍的输入长度。假设本次计算 16
         * 个字节，累计共 16 字节。第一次调用 update API 之前，必须先调用一次 init API。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        input_size = 16U;
        ret = ehsm_symm_cipher_update(ctx, input, input_size, output); /* 调用 update API 更新 */
        ret = demo_check_val("The 1st Update API ", EHSM_OK, ret);
        already_inputed_size = input_size;
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM
         * 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16
         * + 16 = 32 字节。
         */
        input_size = 16U;
        ret = ehsm_symm_cipher_update(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size]); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd Update API ", EHSM_OK, ret);
        already_inputed_size += input_size;
    }

    if (EHSM_OK == ret) {
        /**
         * 4. 调用 finish API。
         * - 对于加密
         *     - 若有剩余的明文，明文长度可以是非 block size 整数倍，即包含未输入的所有明文；对于 XTS 模式，
         *         最后一段输入数据长度必须大于等于 block size + 1；
         *     - 若没有剩余的明文，设置 input 未 NULL，input size 为 0 即可；
         * - 对于解密
         *     - 若有剩余的明文，明文长度可以是非 block size 整数倍，即包含未输入的所有明文；对于 XTS 模式，
         *         最后一段输入数据长度必至少大于等于 2*block size，即必须是 2，3，... 个 block。
         *     - 若没有剩余的明文，设置 input 未 NULL，input size 为 0 即可；
         * - API 调用时需要指定 output buffer 的长度
         *      - 对于加密，若无 padding 必须大于等于输入长度，若有 padding 必须大于输入 padding 后的长度；
         *      - 对于解密，必须大于等于输入的长度；
         * - API 完成计算后会将最后一次输出数据及输出长度返回给 SoC，若解密且包含 padding，则输出的明文包含
         * padding。
         * - 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        input_size = std_input_size - already_inputed_size;

        /**
         * 输出 buffer 长度，
         * - 非 XTS 模式：
         *      - 若没有 padidng，要求大于等于 input size；
         *      - 若有 padding，要求大于等于 input_size +block size;
         *  - XTS 模式，要求大于等于 input size；
         */
        last_output_size = input_size + 32U;
        ret = ehsm_symm_cipher_finish(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], &last_output_size);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，需要校验输出数据和长度，输出数据的总长度等于 already_inputed_size +
             * last_output_size */
            ret = demo_check_data(NULL, std_output, std_output_size, output, already_inputed_size + last_output_size);
        }
    }

    /* 移除密钥 */
    demo_remove_key(ctx, key_handle);

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算对称算法加解密示例，使用 init、update 和 finish APIs, 使用明文密钥。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_symm_cipher_std_st。
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 对于 init API，若明文不是 block size 的整数倍，则需要配置有效填充模式，且非 EHSM_PADDING_NONE，
 *      @ref ehsm_padding_mode_e。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加解密计算，并忽略任何中间数据，如 ctx，session
 * 等。
 */
static void symm_cipher_stepwise_with_plain_key(bool_t enc, const demo_symm_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存*/
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *iv = ehsm_demo_get_buffer(1);     /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size;                          /* 输入 iv 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(2);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经加密/解密的数据总长度 */

    const uint8_t *std_input = NULL;  /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;          /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL; /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;         /* 用于指定标准输出的总长度 */

    /**
     * Finish API 使用：
     * - 首先作为输入，用于指定 output buffer 的长度
     *      - 最后一段输入数据无需 padding 时，无论加解密都必须大于等于最后一段输入长度；
     *      - 最后一段输入数据需要 padding 时，
     *          - 若为加密，则必须大于等于最后一段加上 padding 后的长度；
     *          - 若为解密，则必须大于等于最后一段去掉 padding 后的长度；
     * - 当 finish API 成功执行后，用于接收最后一段输出数据的长度。
     */
    uint32_t last_output_size;

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(iv, 0x0U, 16U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    iv_size = std_data->std_iv_size;
    memcpy(iv, std_data->std_iv, iv_size);    /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    if (EHSM_CIPHER_MODE_ECB != std_data->cipher_mode) {
        print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    }
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、填充模式、明文密钥值，明文密钥长度、是否加密和 eHSM
     * 初始化分段计算的 session。
     * - Init AP 需要指定本次 aead 计算的输入数据的总长度。
     */
    ret = ehsm_symm_cipher_init_with_plain_key(ctx, std_data->algo, std_data->cipher_mode, std_data->padding, key,
        std_data->std_key_size, enc, iv, iv_size, session);
    ret = demo_check_val("The execution of init API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、输入数据、输入数据的字节长度、输出 buffer，eHSM
         * 计算完成后回写输出数据到输出 buffer。update API 仅支持 block size 整数倍的输入长度。假设本次计算 16
         * 个字节，累计共 16 字节。第一次调用 update API 之前，必须先调用一次 init API。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        input_size = 16U;
        ret = ehsm_symm_cipher_update(ctx, input, input_size, output); /* 调用 update API 更新 */
        ret = demo_check_val("The 1st Update API ", EHSM_OK, ret);
        already_inputed_size = input_size;
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM
         * 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16
         * + 16 = 32 字节。
         */
        input_size = 16U;
        ret = ehsm_symm_cipher_update(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size]); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd Update API ", EHSM_OK, ret);
        already_inputed_size += input_size;
    }

    if (EHSM_OK == ret) {
        /**
         * 4. 调用 finish API。
         * - 对于加密
         *     - 若有剩余的明文，明文长度可以是非 block size 整数倍，即包含未输入的所有明文；对于 XTS 模式，
         *         最后一段输入数据长度必须大于等于 block size + 1；
         *     - 若没有剩余的明文，设置 input 未 NULL，input size 为 0 即可；
         * - 对于解密
         *     - 若有剩余的明文，明文长度可以是非 block size 整数倍，即包含未输入的所有明文；对于 XTS 模式，
         *         最后一段输入数据长度必至少大于等于 2*block size，即必须是 2，3，... 个 block。
         *     - 若没有剩余的明文，设置 input 未 NULL，input size 为 0 即可；
         * - API 调用时需要指定 output buffer 的长度
         *      - 对于加密，若无 padding 必须大于等于输入长度，若有 padding 必须大于输入 padding 后的长度；
         *      - 对于解密，必须大于等于输入的长度；
         * - API 完成计算后会将最后一次输出数据及输出长度返回给 SoC，若解密且包含 padding，则输出的明文包含
         * padding。
         * - 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        input_size = std_input_size - already_inputed_size;

        /**
         * 输出 buffer 长度，
         * - 非 XTS 模式：
         *      - 若没有 padidng，要求大于等于 input size；
         *      - 若有 padding，要求大于等于 input_size +block size;
         *  - XTS 模式，要求大于等于 input size；
         */
        last_output_size = input_size + 32U;
        ret = ehsm_symm_cipher_finish(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], &last_output_size);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，需要校验输出数据和长度，输出数据的总长度等于 already_inputed_size +
             * last_output_size */
            ret = demo_check_data(NULL, std_output, std_output_size, output, already_inputed_size + last_output_size);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 对称算法加解密的演示入口函数。包含不同对称算法、密码模式、填充模式的加解密示例。
 */
void ehsm_demo_symm_cipher_entry(void)
{
    uint32_t i;
    const demo_symm_cipher_std_st *std_data_arr = s_symm_cipher_std_data_arr;
    uint32_t cnt = sizeof(s_symm_cipher_std_data_arr) / sizeof(demo_symm_cipher_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for symmetric cipher starts. ==================== "
                     "\r\n\r\n");

    /* TODO: 加入其他对称算法、密码模式、填充模式 */
    for (i = 0U; i < cnt; i++) {
        /* 一次性加密 */
        ehsm_port_printf("[%s encryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        symm_cipher_onepass(true, &std_data_arr[i]);
        symm_cipher_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 一次性解密 */
        ehsm_port_printf("[%s decryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        symm_cipher_onepass(false, &std_data_arr[i]);
        symm_cipher_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 分段加密 */
        ehsm_port_printf("[%s encryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        symm_cipher_stepwise(true, &std_data_arr[i]);
        symm_cipher_stepwise_with_plain_key(true, &std_data_arr[i]);

        /* 分段解密 */
        ehsm_port_printf("[%s decryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        symm_cipher_stepwise(false, &std_data_arr[i]);
        symm_cipher_stepwise_with_plain_key(false, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for symmetric cipher ends. ==================== \r\n\r\n");
}

