#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_aead.h"

#define DEMO_AEAD_KEY_HANDLE (0x100000U)

typedef struct {
    ehsm_symm_algo_e algo;
    ehsm_aead_mode_e mode;
    ehsm_key_type_e key_type;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    const uint8_t *std_aad;
    uint16_t std_aad_size;
    const uint8_t *std_tag;
    uint16_t std_tag_size;
    const uint8_t *std_nonce; /* nonce 或 nonce */
    uint32_t std_nonce_size;
    const uint8_t *std_plaintext;
    uint32_t std_plaintext_size;
    const uint8_t *std_ciphertext;
    uint32_t std_ciphertext_size;
} demo_aead_std_st;

// clang-format off
static const uint8_t s_aead_std_key[32] = {
    0x89,0xB7,0xA3,0x35,0x92,0xE1,0x0D,0xB7,0xA0,0xB0,0xE2,0x7E,0x16,0xB0,0xE2,0xF1,
    0xE3,0xCA,0x70,0xDE,0xFB,0xA1,0x6F,0x3F,0x1E,0xF8,0x71,0xB0,0x29,0x04,0xBB,0x1D,
};

static const uint8_t s_aead_std_aad[20] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13
};

static const uint8_t s_aead_std_nonce[12] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b
};

static const uint8_t s_aead_std_plaintext[64] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f
};

static const uint8_t s_aes128_gcm_std_tag[16] = {
    0x96,0x1A,0xB1,0xEE,0x96,0x90,0x02,0xBB,0xA3,0x2F,0x1F,0xEC,0x36,0xDD,0x83,0xD9
};

static const uint8_t s_aes128_gcm_std_ciphertext[64] = {
    0xB5,0x7C,0xEE,0x72,0x24,0x33,0xB7,0xED,0x22,0x45,0x2E,0xAD,0xB4,0x85,0x6C,0x2E,
    0x81,0x3C,0xDB,0xB6,0x1F,0x8B,0xB3,0x3E,0x79,0x47,0x1E,0x41,0x1A,0x84,0x14,0xFB,
    0x6A,0x38,0x2B,0xF3,0xF2,0x73,0x65,0xE3,0xE8,0x18,0x7D,0x35,0x0D,0xCB,0x76,0x74,
    0xDF,0xEB,0x71,0xC5,0x8D,0xE3,0x91,0x31,0xA9,0x74,0x93,0xBD,0x9A,0x7E,0x53,0xFA
};

static const uint8_t s_sm4_ccm_std_tag[16] = {
    0x10,0xB4,0x34,0xCB,0x95,0x13,0xF2,0x6E,0x2F,0x73,0x33,0x4C,0x6C,0xE3,0xA9,0xC1
};

static const uint8_t s_sm4_ccm_std_ciphertext[64] = {
    0x5F,0x70,0x68,0xB2,0xDB,0x3B,0x32,0xA8,0xF2,0x55,0x36,0x9D,0x4A,0x9B,0xA9,0xFF,
    0x07,0x23,0x2D,0xB5,0x03,0x55,0x35,0x2A,0x2F,0x0E,0x3F,0x44,0x39,0x43,0x85,0x27,
    0xD0,0x2F,0x03,0x8A,0x22,0x83,0x4D,0x47,0xB7,0xF8,0x68,0x06,0x71,0x31,0x7B,0xBB,
    0x83,0xE4,0x7E,0x7C,0xB9,0xFA,0x3B,0x82,0x1B,0x27,0x71,0xA7,0x15,0xF1,0x56,0x38
};


static const demo_aead_std_st s_aead_std_data_arr[2] = {
    {
        .algo = EHSM_SYMM_ALGO_AES_128,
        .mode = EHSM_AEAD_MODE_GCM,
        .alg_str = "AES_128_GCM",
        .key_type = EHSM_KEY_TYPE_AES_128,
        .std_key = s_aead_std_key,
        .std_key_size = 16U,
        .std_aad = s_aead_std_aad,
        .std_aad_size = sizeof(s_aead_std_aad),
        .std_tag = s_aes128_gcm_std_tag,
        .std_tag_size = sizeof(s_aes128_gcm_std_tag),
        .std_nonce = s_aead_std_nonce,
        .std_nonce_size = sizeof(s_aead_std_nonce),
        .std_plaintext = s_aead_std_plaintext,
        .std_plaintext_size = sizeof(s_aead_std_plaintext),
        .std_ciphertext = s_aes128_gcm_std_ciphertext,
        .std_ciphertext_size = sizeof(s_aes128_gcm_std_ciphertext),
    }, {
        .algo = EHSM_SYMM_ALGO_SM4,
        .mode = EHSM_AEAD_MODE_CCM,
        .alg_str = "AES_SM4_CCM",
        .key_type = EHSM_KEY_TYPE_SM4,
        .std_key = s_aead_std_key,
        .std_key_size = 16U,
        .std_aad = s_aead_std_aad,
        .std_aad_size = sizeof(s_aead_std_aad),
        .std_tag = s_sm4_ccm_std_tag,
        .std_tag_size = sizeof(s_sm4_ccm_std_tag),
        .std_nonce = s_aead_std_nonce,
        .std_nonce_size = sizeof(s_aead_std_nonce),
        .std_plaintext = s_aead_std_plaintext,
        .std_plaintext_size = sizeof(s_aead_std_plaintext),
        .std_ciphertext = s_sm4_ccm_std_ciphertext,
        .std_ciphertext_size = sizeof(s_sm4_ccm_std_ciphertext),
    }
};

// clang-format on

/**
 * @brief 一次性计算 aead 算法加解密示例，使用 onepass APIs。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_aead_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void aead_onepass(bool_t enc, const demo_aead_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_AEAD_KEY_HANDLE;          /* 用于指定导入密钥的 key handle */
    uint8_t *aad = ehsm_demo_get_buffer(1);    /* 输入 aad 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t aad_size;                         /* 输入 add 的长度 */
    uint8_t *tag = ehsm_demo_get_buffer(2);    /* tag 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t tag_size;                         /* tag 的长度 */
    uint8_t *nonce = ehsm_demo_get_buffer(3);  /* 输入 nonce 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t nonce_size;                       /* 输入 nonce 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(4);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(5); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 aead 解密过程 */
    bool_t need_remove = false;             /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(aad, 0x0U, 512U);
    memset(tag, 0x0U, 16U);
    memset(nonce, 0x0U, 16U);
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
    aad_size = std_data->std_aad_size;
    tag_size = std_data->std_tag_size;
    nonce_size = std_data->std_nonce_size;
    memcpy(aad, std_data->std_aad, aad_size);       /* 将 aad 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(nonce, std_data->std_nonce, nonce_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size);       /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!enc) {
        /* 解密时 tag 作为输入 */
        memcpy(tag, std_data->std_tag, tag_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、nonce、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std aad is: \r\n    ", std_data->std_aad, std_data->std_aad_size);
    print_hex("The std tag is: \r\n    ", std_data->std_tag, std_data->std_tag_size);
    print_hex("The std nonce is: \r\n    ", std_data->std_nonce, std_data->std_nonce_size);
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

        if (enc) {
            /*  4. 调用 onepass API 加密。API 完成计算后会将最后输出数据、生成的 tag 返回给 SoC。*/
            input_size = std_input_size;
            ret = ehsm_aead_onepass_enc(ctx, std_data->algo, std_data->mode, key_handle, nonce, nonce_size, aad,
                aad_size, input, input_size, output, tag, tag_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验输出数据和 tag */
                ret = demo_check_data("Output comparison:", std_output, std_input_size, output, std_output_size);
                ret = demo_check_data("Tag comparison:", std_data->std_tag, std_data->std_tag_size, tag, tag_size);
            }
        } else {
            /* 4. 解密调用 onepass API 解密。API 完成计算后会将最后输出数据返回给 SoC，并校验输入的 tag。*/
            input_size = std_input_size;
            ret = ehsm_aead_onepass_dec(ctx, std_data->algo, std_data->mode, key_handle, nonce, nonce_size, aad,
                aad_size, input, input_size, output, tag, tag_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify tag:", true, verify_result);
            }
        }
    }

    if (need_remove) {
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 aead 算法加解密示例，使用 onepass APIs, 使用明文密钥。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_aead_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void aead_onepass_with_plain_key(bool_t enc, const demo_aead_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);    /* 存储密钥值的buffer */
    uint8_t *aad = ehsm_demo_get_buffer(1);    /* 输入 aad 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t aad_size;                         /* 输入 add 的长度 */
    uint8_t *tag = ehsm_demo_get_buffer(2);    /* tag 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t tag_size;                         /* tag 的长度 */
    uint8_t *nonce = ehsm_demo_get_buffer(3);  /* 输入 nonce 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t nonce_size;                       /* 输入 nonce 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(4);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(5); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 aead 解密过程 */

    /* 初始化数据 buffer */
    memset(aad, 0x0U, 512U);
    memset(tag, 0x0U, 16U);
    memset(nonce, 0x0U, 16U);
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
    aad_size = std_data->std_aad_size;
    tag_size = std_data->std_tag_size;
    nonce_size = std_data->std_nonce_size;
    memcpy(aad, std_data->std_aad, aad_size);       /* 将 aad 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(nonce, std_data->std_nonce, nonce_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size);       /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!enc) {
        /* 解密时 tag 作为输入 */
        memcpy(tag, std_data->std_tag, tag_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、nonce、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std aad is: \r\n    ", std_data->std_aad, std_data->std_aad_size);
    print_hex("The std tag is: \r\n    ", std_data->std_tag, std_data->std_tag_size);
    print_hex("The std nonce is: \r\n    ", std_data->std_nonce, std_data->std_nonce_size);
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    if (enc) {
        /*  4. 调用 onepass API 加密。API 完成计算后会将最后输出数据、生成的 tag 返回给 SoC。*/
        input_size = std_input_size;
        ret = ehsm_aead_onepass_enc_with_plain_key(ctx, std_data->algo, std_data->mode, key, std_data->std_key_size,
            nonce, nonce_size, aad, aad_size, input, input_size, output, tag, tag_size);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，需要校验输出数据和 tag */
            ret = demo_check_data("Output comparison:", std_output, std_input_size, output, std_output_size);
            ret = demo_check_data("Tag comparison:", std_data->std_tag, std_data->std_tag_size, tag, tag_size);
        }
    } else {
        /* 4. 解密调用 onepass API 解密。API 完成计算后会将最后输出数据返回给 SoC，并校验输入的 tag。*/
        input_size = std_input_size;
        ret = ehsm_aead_onepass_dec_with_plain_key(ctx, std_data->algo, std_data->mode, key, std_data->std_key_size,
            nonce, nonce_size, aad, aad_size, input, input_size, output, tag, tag_size, &verify_result);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* 检查校验结果 */
            ret = demo_check_val("Verify tag:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 aead 算法加解密示例，使用 init、update 和 finish APIs。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_aead_std_st
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void aead_stepwise(bool_t enc, const demo_aead_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_AEAD_KEY_HANDLE;          /* 用于指定导入密钥的 key handle */
    uint8_t *aad = ehsm_demo_get_buffer(1);    /* 输入 aad 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t aad_size;                         /* 输入 add 的长度 */
    uint8_t *tag = ehsm_demo_get_buffer(2);    /* tag 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t tag_size;                         /* tag 的长度 */
    uint8_t *nonce = ehsm_demo_get_buffer(3);  /* 输入 nonce 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t nonce_size;                       /* 输入 nonce 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(4);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(5); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经加密/解密的数据总长度 */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 aead 解密过程 */
    bool_t need_remove = false;             /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key_data, 0x0U, 512U);
    memset(aad, 0x0U, 512U);
    memset(tag, 0x0U, 16U);
    memset(nonce, 0x0U, 16U);
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
    aad_size = std_data->std_aad_size;
    tag_size = std_data->std_tag_size;
    nonce_size = std_data->std_nonce_size;
    memcpy(aad, std_data->std_aad, aad_size);       /* 将 aad 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(nonce, std_data->std_nonce, nonce_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size);       /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!enc) {
        /* 解密时 tag 作为输入 */
        memcpy(tag, std_data->std_tag, tag_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、nonce、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std aad is: \r\n    ", std_data->std_aad, std_data->std_aad_size);
    print_hex("The std tag is: \r\n    ", std_data->std_tag, std_data->std_tag_size);
    print_hex("The std nonce is: \r\n    ", std_data->std_nonce, std_data->std_nonce_size);
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
         * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、密钥 key handle、是否加密、nonce(iv) 及长度、aad
         * 及长度，输入数据的总长度、标签 tag 的长度和 eHSM 初始化分段计算的 session。
         *
         */
        ret = ehsm_aead_init(ctx, std_data->algo, std_data->mode, key_handle, enc, nonce, nonce_size, aad, aad_size,
            std_input_size, tag_size, session);
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
        ret = ehsm_aead_update(ctx, input, input_size, output); /* 调用 update API 更新 */
        already_inputed_size = input_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM 支持多次更新，每次更新
         * eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16 + 16 = 32 字节。
         */
        input_size = 16U;
        ret = ehsm_aead_update(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size]); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_inputed_size += input_size;
    }

    if (EHSM_OK == ret) {
        if (enc) { /* 加密 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据、生成的 tag 返回给 SoC。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            input_size = std_input_size - already_inputed_size;
            ret = ehsm_aead_finish_enc(
                ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], tag);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验输出数据和 tag */
                ret = demo_check_data("Output comparison:", std_output, std_input_size, output, std_output_size);
                ret = demo_check_data("Tag comparison:", std_data->std_tag, std_data->std_tag_size, tag, tag_size);
            }
        } else { /* 解密 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据返回给 SoC，并校验输入的 tag。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            input_size = std_input_size - already_inputed_size;
            ret = ehsm_aead_finish_dec(
                ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], tag, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify tag:", true, verify_result);
            }
        }
    }

    if (need_remove) {
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 aead 算法加解密示例，使用 init、update 和 finish APIs，使用明文密钥。
 * @param[in] enc 是否加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_aead_std_st
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void aead_stepwise_with_plain_key(bool_t enc, const demo_aead_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *aad = ehsm_demo_get_buffer(1);    /* 输入 aad 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t aad_size;                         /* 输入 add 的长度 */
    uint8_t *tag = ehsm_demo_get_buffer(2);    /* tag 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t tag_size;                         /* tag 的长度 */
    uint8_t *nonce = ehsm_demo_get_buffer(3);  /* 输入 nonce 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t nonce_size;                       /* 输入 nonce 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(4);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的总长度 */
    uint8_t *output = ehsm_demo_get_buffer(5); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经加密/解密的数据总长度 */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 aead 解密过程 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(aad, 0x0U, 512U);
    memset(tag, 0x0U, 16U);
    memset(nonce, 0x0U, 16U);
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
    aad_size = std_data->std_aad_size;
    tag_size = std_data->std_tag_size;
    nonce_size = std_data->std_nonce_size;
    memcpy(aad, std_data->std_aad, aad_size);       /* 将 aad 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(nonce, std_data->std_nonce, nonce_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(input, std_input, std_input_size);       /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!enc) {
        /* 解密时 tag 作为输入 */
        memcpy(tag, std_data->std_tag, tag_size); /* 将 nonce 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、nonce、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std aad is: \r\n    ", std_data->std_aad, std_data->std_aad_size);
    print_hex("The std tag is: \r\n    ", std_data->std_tag, std_data->std_tag_size);
    print_hex("The std nonce is: \r\n    ", std_data->std_nonce, std_data->std_nonce_size);
    print_hex("The std input data is: \r\n    ", std_input, std_input_size);
    print_hex("The std output data is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、明文密钥值、明文密钥长度、是否加密、nonce(iv) 及长度、aad
     * 及长度，输入数据的总长度、标签 tag 的长度和 eHSM 初始化分段计算的 session。
     *
     */
    ret = ehsm_aead_init_with_plain_key(ctx, std_data->algo, std_data->mode, key, std_data->std_key_size, enc, nonce,
        nonce_size, aad, aad_size, std_input_size, tag_size, session);
    ret = demo_check_val("The execution of init API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、输入数据、输入数据的字节长度、输出 buffer，eHSM
         * 计算完成后回写输出数据到输出 buffer。update API 仅支持 block size 整数倍的输入长度。假设本次计算 16
         * 个字节，累计共 16 字节。第一次调用 update API 之前，必须先调用一次 init API。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        input_size = 16U;
        ret = ehsm_aead_update(ctx, input, input_size, output); /* 调用 update API 更新 */
        already_inputed_size = input_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM 支持多次更新，每次更新
         * eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16 + 16 = 32 字节。
         */
        input_size = 16U;
        ret = ehsm_aead_update(
            ctx, &input[already_inputed_size], input_size, &output[already_inputed_size]); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_inputed_size += input_size;
    }

    if (EHSM_OK == ret) {
        if (enc) { /* 加密 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据、生成的 tag 返回给 SoC。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            input_size = std_input_size - already_inputed_size;
            ret = ehsm_aead_finish_enc(
                ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], tag);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验输出数据和 tag */
                ret = demo_check_data("Output comparison:", std_output, std_input_size, output, std_output_size);
                ret = demo_check_data("Tag comparison:", std_data->std_tag, std_data->std_tag_size, tag, tag_size);
            }
        } else { /* 解密 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据返回给 SoC，并校验输入的 tag。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            input_size = std_input_size - already_inputed_size;
            ret = ehsm_aead_finish_dec(
                ctx, &input[already_inputed_size], input_size, &output[already_inputed_size], tag, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify tag:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 对称算法加解密的演示入口函数。包含不同对称算法、密码模式、填充模式的加解密示例。
 */
void ehsm_demo_aead_entry(void)
{
    uint32_t i;
    const demo_aead_std_st *std_data_arr = s_aead_std_data_arr;
    uint32_t cnt = sizeof(s_aead_std_data_arr) / sizeof(demo_aead_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for aead starts. ==================== "
                     "\r\n\r\n");

    /* TODO: 加入其他 aead 算法 */
    for (i = 0U; i < cnt; i++) {
        /* 一次性加密 */
        ehsm_port_printf("[%s encryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        aead_onepass(true, &std_data_arr[i]);
        aead_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 一次性解密 */
        ehsm_port_printf("[%s decryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        aead_onepass(false, &std_data_arr[i]);
        aead_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 分段加密 */
        ehsm_port_printf("[%s encryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        aead_stepwise(true, &std_data_arr[i]);
        aead_stepwise_with_plain_key(true, &std_data_arr[i]);

        /* 分段解密 */
        ehsm_port_printf("[%s decryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        aead_stepwise(false, &std_data_arr[i]);
        aead_stepwise_with_plain_key(false, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for aead ends. ==================== \r\n\r\n");
}

