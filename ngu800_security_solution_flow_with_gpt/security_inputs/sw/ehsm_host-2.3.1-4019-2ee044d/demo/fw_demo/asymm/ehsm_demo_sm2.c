#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_sm2.h"

#define DEMO_SM2_KEY_HANDLE (0x100000U)

typedef struct {
    ehsm_key_type_e key_type;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    uint16_t std_pub_key_size;
    uint16_t std_priv_key_size;
    const uint8_t *std_plaintext;
    uint32_t std_plaintext_size;
    const uint8_t *std_ciphertext;
    uint32_t std_ciphertext_size;
} demo_sm2_cipher_std_st;

typedef struct {
    ehsm_key_type_e key_type;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    uint16_t std_pub_key_size;
    uint16_t std_priv_key_size;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_signature;
    uint32_t std_signature_size;
} demo_sm2_sign_std_st;

// clang-format off
static const uint8_t s_sm2_std_key[65 + 32] = {
    /* 公钥 */
    0x04, /* 未压缩前缀 */
    0xFE,0xA4,0x7E,0x11,0x85,0x5F,0x09,0x99,0xC2,0x34,0x4C,0x9A,0x65,0x6B,0xA5,0xE8,
    0xE8,0x3C,0xF7,0x11,0x4B,0x74,0x04,0xFA,0x3F,0x6C,0x28,0xF2,0x6D,0x4A,0xFF,0x45,
    0xE2,0x99,0x9D,0x83,0x32,0x10,0x67,0x6F,0x9B,0x4A,0xF1,0x1F,0x78,0xB1,0xD5,0xDE,
    0xA6,0x54,0x48,0xC0,0xA6,0x00,0xF1,0x88,0x94,0x93,0xA6,0x32,0x63,0x05,0x9C,0x5F,
    /* 私钥 */
    0x3A,0x2C,0xCE,0x8B,0x4B,0x07,0x7C,0x79,0xBA,0x8F,0x2B,0x5B,0x94,0x3B,0xAC,0x27,
    0x31,0x25,0x79,0xEE,0xCB,0xBE,0xFB,0x79,0x61,0x9F,0x76,0x51,0xFC,0xF3,0x1D,0x42
};

static const uint8_t s_sm2_std_plaintext[80] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f
};

/**
 * SM2 加密使用随机数，用于外部工具验证。
 * static const uint8_t s_sm2_std_rand_k[32] = {
 *    0xF7,0xBD,0x4F,0xBA,0xBA,0xD5,0x04,0x3F,0xBE,0x20,0xD8,0x72,0x53,0xF7,0x62,0xB9,
 *    0xBE,0xF8,0x7E,0x73,0xD4,0x0A,0x9E,0xA7,0x71,0x5B,0xA6,0x10,0xD8,0x29,0x1E,0x3B
 * };
 */
static const uint8_t s_sm2_std_ciphertext[65 + 32 + 80] = {
    /* C1: k[G] */
    0x04,
    0xDA,0x12,0xDB,0xBE,0x28,0xDB,0x87,0xF9,0x8B,0x1C,0x97,0x1F,0x03,0x38,0xA1,0x26,
    0x07,0x86,0x46,0xDE,0xC4,0x02,0xFA,0xEF,0x9A,0x3F,0x6E,0xD4,0xCE,0xA1,0x4C,0xFE,
    0xF6,0x1A,0xB5,0x91,0x0A,0x11,0xEE,0x8D,0x9C,0xF4,0xF8,0xC5,0x80,0xC9,0xF4,0xC5,
    0xE9,0x80,0x0C,0x30,0xE6,0x12,0x0C,0x40,0x96,0xF6,0x84,0xD3,0x84,0xAF,0xCE,0x97,
    /* C3: SM3 hash digest */
    0x54,0xBB,0x0A,0xB9,0x8C,0xCA,0x79,0x4E,0xA4,0x75,0x7E,0x41,0xE6,0x8B,0x62,0x3F,
    0xD7,0x11,0x8F,0x41,0x18,0x64,0x18,0xFE,0x65,0x25,0xED,0xAD,0x7F,0x8B,0xD9,0xE2,
    /* C2: Ciphertext */
    0xE2,0xD6,0x8F,0xA5,0x38,0xD7,0x8C,0x9C,0x32,0x2D,0x4D,0xC9,0x5B,0x42,0x7C,0xEE,
    0xA6,0x7E,0x1B,0x96,0x25,0x18,0x94,0x6D,0x6B,0xB2,0x02,0x89,0x71,0x1A,0x92,0x88,
    0xA4,0x4B,0xEB,0x99,0x37,0x84,0x93,0xE7,0x70,0x0F,0xE7,0xE1,0x05,0x1C,0xC5,0x27,
    0x2B,0x1C,0xD8,0x06,0xFE,0x6E,0xD6,0x8A,0x20,0x80,0xA9,0xBB,0xE0,0x5A,0x4A,0x7D,
    0xC0,0xE1,0xC9,0xE5,0x40,0x60,0x98,0xFA,0xF7,0x53,0xE3,0x66,0xCA,0x64,0x03,0x23
};

static const demo_sm2_cipher_std_st s_sm2_cipher_std_data_arr[1] = {
    {
        .alg_str = "SM2 cipher",
        .key_type = EHSM_KEY_TYPE_SM2,
        .std_key = s_sm2_std_key,
        .std_key_size = sizeof(s_sm2_std_key),
        .std_pub_key_size = 65U,
        .std_priv_key_size = 32U,
        .std_plaintext = s_sm2_std_plaintext,
        .std_plaintext_size = sizeof(s_sm2_std_plaintext),
        .std_ciphertext = s_sm2_std_ciphertext,
        .std_ciphertext_size = sizeof(s_sm2_std_ciphertext),
    }
};

/**
 * SM2 签名使用固定的 ID，是签名者的可辨别标识，长度一般是 16 字节，以 ASCII 编码表示为“1234567812345678”，
 * 即 U8 字节串 {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38}。
 * static const uint8_t s_sm2_std_id[16] = {
 *    0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38
 * };
 */
static const uint8_t s_sm2_std_signature[64] = {
    0x0A,0x11,0x79,0x19,0xB7,0x18,0x95,0x6E,0xF3,0x9B,0x17,0x6C,0xA3,0xBC,0xB4,0x41,
    0x23,0xD0,0xFC,0x16,0x70,0x34,0xAA,0xED,0xD7,0x91,0x58,0x71,0x99,0x32,0x44,0xA9,
    0x8A,0x74,0xA1,0x20,0xA9,0x9E,0x12,0x98,0xE9,0x76,0xE0,0xA5,0xF5,0xCC,0x55,0xA4,
    0x4D,0x2E,0x67,0xDC,0x29,0x5F,0x9C,0x8D,0x8D,0xC4,0xA9,0x89,0xBD,0xDE,0xCB,0xE0
};

static const demo_sm2_sign_std_st s_sm2_sign_std_data_arr[1] = {
    {
        .alg_str = "SM2 signature",
        .key_type = EHSM_KEY_TYPE_SM2,
        .std_key = s_sm2_std_key,
        .std_key_size = sizeof(s_sm2_std_key),
        .std_pub_key_size = 65U,
        .std_priv_key_size = 32U,
        .std_msg = s_sm2_std_plaintext,
        .std_msg_size = sizeof(s_sm2_std_plaintext),
        .std_signature = s_sm2_std_signature,
        .std_signature_size = sizeof(s_sm2_std_signature),
    }
};

// clang-format on

/**
 * @brief 一次性计算 SM2 加解密的示例。
 * @param[in] enc 是否为加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_cipher_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于加密，由于加密过程中引入随机数，eHSM 产生的密文和标准密文不一致，无法直接通过比对数据校验，因此先调用
 * 加密 API 对明文消息加密，再调用解密 API 对产生的密文进行解密，最后比对 eHSM 产生的明文和标准数据。
 * - 对于解密，直接用标准数据进行验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用解密接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先加密，后解密的方式验证加密 API 的功能。
 */
static void sm2_cipher(bool_t enc, const demo_sm2_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *input = ehsm_demo_get_buffer(1);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的长度 */
    uint8_t *output = ehsm_demo_get_buffer(2); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size;                      /* 输出数据的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    uint32_t key_handle = DEMO_SM2_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t need_remove = false;                /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 指定输入输出数据 */
    if (enc) {
        /* SM2 签名生成 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {

        /* SM2 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    input_size = std_input_size;
    memcpy(input, std_input, input_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、标准输入输出数据。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std input is: \r\n    ", std_input, std_input_size);
    print_hex("The std output is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 2. 调用 SM2 cipher API。需要输入所有消息，API 完成计算后会将输出数据返回给 SoC。
         * - output_size:
         *      - 首先作为输入，是输出 buffer 的大小：
         *          - 加密时，必须大于等于 input_size + 65 + 32；
         *          - 解密时，必须大于等于 output_size - 65 - 32；
         *      - API 执行完成后作为输出，是输出数据的实际长度：
         *          - 加密时，等于 input_size + 65 + 32；
         *          - 解密时，等于 output_size - 65 - 32；
         *      - 其中，65 是 C1 即未压缩的 SM2 椭圆曲线点长度，32 是 SM3 hash digest 长度。
         */
        output_size = std_output_size;
        ret = ehsm_sm2_cipher(ctx, key_handle, enc, input, input_size, output, &output_size);
        ret = demo_check_val("The execution of SM2 cipher API:", EHSM_OK, ret);

        /* 校验数据 */
        if (EHSM_OK == ret) {
            if (enc) {
                /* 由于加密含有随机数，因此无法直接校验输出数据，采用先加密，后解密生成明文，再比较明文和标准数据。 */
                print_hex("The generated ciphertext is:\r\n    ", output, output_size);

                /* 将 eHSM 加密后的密文作为输入，调用 SM2 解密，成功执行后的 input 是 eHSM 输出的明文 */
                input_size = std_input_size;
                ret = ehsm_sm2_cipher(ctx, key_handle, (!enc), output, output_size, input, &input_size);
                ret = demo_check_val("The execution of SM2 cipher API for decrypting to get plaintext:", EHSM_OK, ret);

                if (EHSM_OK == ret) {
                    /** eHSM FW 返回成功，此时用标准明文数据和 eHSM 先加密，后解密出的明文数据进行对比 */
                    ret = demo_check_data("Output comparison:", std_input, std_input_size, input, input_size);
                }
            } else {
                /* 解密，eHSM FW 返回成功，解密可以直接用标准数据校验 */
                ret = demo_check_data("Output comparison:", std_output, std_output_size, output, output_size);
            }
        }
    }

    if (need_remove) {
        /* 移除密钥 */
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 SM2 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 */
static void sm2_sign_ver_onepass(bool_t gen_sig, const demo_sm2_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    uint32_t key_handle = DEMO_SM2_KEY_HANDLE; /* 用于指定导入密钥的 key handle */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        if (gen_sig) {
            /**
             * 2. 调用 SM2 签名生成 API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             * - signature_size：指定 signature buffer 的长度，必须大于等于 64 字节，SM2 签名的长度固定为 64 字节。
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_onepass_gen(ctx, key_handle, msg, msg_size, signature, signature_size);
            ret = demo_check_val("The execution of sign onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_sm2_sign_onepass_verify(
                    ctx, key_handle, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else {
            /* 2. 调用 SM2 签名验证 API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_onepass_verify(
                ctx, key_handle, msg, msg_size, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
            }
        }
    }

    /* 移除密钥 */
    demo_remove_key(ctx, key_handle);

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 SM2 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 SM3 的 hash 算法，使用 SM2 公钥和 ID 计算 SM3 digest 的第一个 block。
 *      - SM2 签名使用固定的 ID，是签名者的可辨别标识，长度一般是 16 字节，以 ASCII 编码表示为“1234567812345678”，
 *          即 U8 字节串 {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
 * - 对于 update API，主要是对输入的消息计算 SM3 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 SM3 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 */
static void sm2_sign_ver_stepwise(bool_t gen_sig, const demo_sm2_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    uint32_t key_handle = DEMO_SM2_KEY_HANDLE; /* 用于指定导入密钥的 key handle */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_KEY_PAIR,
        std_data->std_key, std_data->std_key_size, std_data->std_pub_key_size, std_data->std_priv_key_size, key_data,
        &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 2. 调用 init API。需要输入密钥 handle、生成/验证方向、分段计算的会话 buffer。*/
        ret = ehsm_sm2_sign_init(ctx, key_handle, gen_sig, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* 3.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_sm2_sign_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 3.1 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_sm2_sign_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             * - signature_size：指定 signature buffer 的长度，必须大于等于 64 字节，SM2 签名的长度固定为 64 字节。
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_finish_gen(ctx, signature, signature_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_sm2_sign_onepass_verify(
                    ctx, key_handle, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else { /* 签名验证 */
            /* 4. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_finish_verify(ctx, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
            }
        }
    }

    /* 移除密钥 */
    demo_remove_key(ctx, key_handle);

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 SM2 加解密的示例，使用明文密钥。
 * @param[in] enc 是否为加密
 *          - true 指加密；
 *          - false 指解密；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_cipher_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于加密，由于加密过程中引入随机数，eHSM 产生的密文和标准密文不一致，无法直接通过比对数据校验，因此先调用
 * 加密 API 对明文消息加密，再调用解密 API 对产生的密文进行解密，最后比对 eHSM 产生的明文和标准数据。
 * - 对于解密，直接用标准数据进行验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用解密接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先加密，后解密的方式验证加密 API 的功能。
 */
static void sm2_cipher_with_plain_key(bool_t enc, const demo_sm2_cipher_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);    /* 明文密钥的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *input = ehsm_demo_get_buffer(1);  /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t input_size;                       /* 输入数据的长度 */
    uint8_t *output = ehsm_demo_get_buffer(2); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t output_size;                      /* 输出数据的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    const uint8_t *std_input = NULL;        /* 用于指定标准输入，加密时是明文，解密时是密文 */
    uint32_t std_input_size;                /* 用于指定标准输入的总长度 */
    const uint8_t *std_output = NULL;       /* 用于指定标准输出，加密时是密文，解密时是明文 */
    uint32_t std_output_size;               /* 用于指定标准输出的总长度 */
    ehsm_sm2_key_st *sm2_key = (ehsm_sm2_key_st *)ehsm_demo_get_buffer(3); /* 存储SM2密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(key, 0x0U, 512U);
    memset(input, 0x0U, 512U);
    memset(output, 0x0U, 512U);

    /* 准备密钥数据，存储在sm2_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置SM2密钥结构 */
    memcpy(sm2_key->pubkey, key, 65);       /* 公钥65字节（含0x04前缀） */
    memcpy(sm2_key->privkey, &key[65], 32); /* 私钥32字节 */

    /* 指定输入输出数据 */
    if (enc) {
        /* SM2 加密 */
        std_input = std_data->std_plaintext;
        std_input_size = std_data->std_plaintext_size;
        std_output = std_data->std_ciphertext;
        std_output_size = std_data->std_ciphertext_size;
    } else {
        /* SM2 解密 */
        std_input = std_data->std_ciphertext;
        std_input_size = std_data->std_ciphertext_size;
        std_output = std_data->std_plaintext;
        std_output_size = std_data->std_plaintext_size;
    }

    /* 拷贝数据 */
    input_size = std_input_size;
    memcpy(input, std_input, input_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、标准输入输出数据。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std input is: \r\n    ", std_input, std_input_size);
    print_hex("The std output is: \r\n    ", std_output, std_output_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 SM2 cipher API，使用明文密钥。需要输入所有消息，API 完成计算后会将输出数据返回给 SoC。
     * - output_size:
     *      - 首先作为输入，是输出 buffer 的大小：
     *          - 加密时，必须大于等于 input_size + 65 + 32；
     *          - 解密时，必须大于等于 output_size - 65 - 32；
     *      - API 执行完成后作为输出，是输出数据的实际长度：
     *          - 加密时，等于 input_size + 65 + 32；
     *          - 解密时，等于 output_size - 65 - 32；
     *      - 其中，65 是 C1 即未压缩的 SM2 椭圆曲线点长度，32 是 SM3 hash digest 长度。
     */
    output_size = std_output_size;
    ret = ehsm_sm2_cipher_with_plain_key(ctx, (uint8_t *)sm2_key, enc, input, input_size, output, &output_size);
    ret = demo_check_val("The execution of SM2 cipher with plain key API:", EHSM_OK, ret);

    /* 校验数据 */
    if (EHSM_OK == ret) {
        if (enc) {
            /* 由于加密含有随机数，因此无法直接校验输出数据，采用先加密，后解密生成明文，再比较明文和标准数据。 */
            print_hex("The generated ciphertext is:\r\n    ", output, output_size);

            /* 将 eHSM 加密后的密文作为输入，调用 SM2 解密，成功执行后的 input 是 eHSM 输出的明文 */
            input_size = std_input_size;
            ret = ehsm_sm2_cipher_with_plain_key(
                ctx, (uint8_t *)sm2_key, (!enc), output, output_size, input, &input_size);
            ret = demo_check_val(
                "The execution of SM2 cipher with plain key API for decrypting to get plaintext:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，此时用标准明文数据和 eHSM 先加密，后解密出的明文数据进行对比 */
                ret = demo_check_data("Output comparison:", std_input, std_input_size, input, input_size);
            }
        } else {
            /* 解密，eHSM FW 返回成功，解密可以直接用标准数据校验 */
            ret = demo_check_data("Output comparison:", std_output, std_output_size, output, output_size);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[SM2 cipher with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 一次性计算 SM2 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 */
static void sm2_sign_ver_onepass_with_plain_key(bool_t gen_sig, const demo_sm2_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0);       /* 存储密钥值的buffer */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    ehsm_sm2_key_st *sm2_key = (ehsm_sm2_key_st *)ehsm_demo_get_buffer(3); /* 存储SM2密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 准备密钥数据，存储在sm2_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置SM2密钥结构 */
    memcpy(sm2_key->pubkey, key, 65);       /* 公钥65字节（含0x04前缀） */
    memcpy(sm2_key->privkey, &key[65], 32); /* 私钥32字节 */

    /* 拷贝数据 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    if (gen_sig) {
        /**
         * 2. 调用 SM2 签名生成 API，使用明文密钥。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
         * - signature_size：指定 signature buffer 的长度，必须大于等于 64 字节，SM2 签名的长度固定为 64 字节。
         */
        signature_size = std_data->std_signature_size;
        ret = ehsm_sm2_sign_onepass_gen_with_plain_key(
            ctx, (uint8_t *)sm2_key, msg, msg_size, signature, signature_size);
        ret = demo_check_val("The execution of SM2 sign onepass with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
            print_hex("The generated signature is:\r\n    ", signature, signature_size);

            /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
            ret = ehsm_sm2_sign_onepass_verify_with_plain_key(
                ctx, (uint8_t *)sm2_key, msg, msg_size, signature, signature_size, &verify_result);
            ret = demo_check_val(
                "The execution of SM2 verify onepass with plain key API for verifing generated signature:", EHSM_OK,
                ret);
        }
    } else {
        /* 2. 调用 SM2 签名验证 API，使用明文密钥。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
        signature_size = std_data->std_signature_size;
        ret = ehsm_sm2_sign_onepass_verify_with_plain_key(
            ctx, (uint8_t *)sm2_key, msg, msg_size, signature, signature_size, &verify_result);
        ret = demo_check_val("The execution of SM2 verify onepass with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，检查签名验证结果 */
            ret = demo_check_val("SM2 signature verification:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[SM2 sign with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 SM2 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_sm2_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 SM3 的 hash 算法，使用 SM2 公钥和 ID 计算 SM3 digest 的第一个 block。
 *      - SM2 签名使用固定的 ID，是签名者的可辨别标识，长度一般是 16 字节，以 ASCII 编码表示为"1234567812345678"，
 *          即 U8 字节串 {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
 * - 对于 update API，主要是对输入的消息计算 SM3 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 SM3 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 */
static void sm2_sign_ver_stepwise_with_plain_key(bool_t gen_sig, const demo_sm2_sign_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *msg = ehsm_demo_get_buffer(1);       /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                            /* 输入消息的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 存储签名的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t signature_size;                      /* 签名的 fuffer 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    ehsm_sm2_key_st *sm2_key = (ehsm_sm2_key_st *)ehsm_demo_get_buffer(3); /* 存储SM2密钥结构的buffer */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(msg, 0x0U, 512U);
    memset(signature, 0x0U, 512U);

    /* 准备密钥数据，存储在sm2_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);

    /* 设置SM2密钥结构 */
    memcpy(sm2_key->pubkey, key, 65);       /* 公钥65字节（含0x04前缀） */
    memcpy(sm2_key->privkey, &key[65], 32); /* 私钥32字节 */

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size); /* 将标准输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_sig) {
        /* 验签需要输入签名，将标准签名拷贝至 SoC 与 eHSM 的共享内存上。 */
        memcpy(signature, std_data->std_signature, std_data->std_signature_size);
    }

    /* 打印密钥、标准消息、签名。 */
    print_hex("The std public key is: \r\n    ", std_data->std_key, std_data->std_pub_key_size);
    print_hex("The std private key is: \r\n    ", &(std_data->std_key[std_data->std_pub_key_size]),
        std_data->std_priv_key_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std signature is: \r\n    ", std_data->std_signature, std_data->std_signature_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 init API，使用明文密钥。需要输入 hash 算法，密钥 handle、生成/验证方向、分段计算的会话 buffer。*/
    ret = ehsm_sm2_sign_init_with_plain_key(ctx, (uint8_t *)sm2_key, gen_sig, session);
    ret = demo_check_val("The execution of SM2 sign init with plain key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 3.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_sm2_sign_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 3.2 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_sm2_sign_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于 64 字节；
             *      - 再作为输出，eHSM 会回写实际产生的签名长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_finish_gen(ctx, signature, signature_size);
            ret = demo_check_val("The execution of SM2 sign finish gen API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_sm2_sign_onepass_verify_with_plain_key(
                    ctx, (uint8_t *)sm2_key, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of SM2 verify onepass with plain key API for verifing generated signature:", EHSM_OK,
                    ret);
            }
        } else { /* 签名验证 */
            /* 4. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_sm2_sign_finish_verify(ctx, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of SM2 sign finish verify API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("SM2 signature verification:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[SM2 sign stepwise with plain key ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief SM2 加解密的演示入口函数。
 */
static void sm2_cipher_entry(void)
{
    uint32_t i;
    const demo_sm2_cipher_std_st *std_data_arr = s_sm2_cipher_std_data_arr;
    uint32_t cnt = sizeof(s_sm2_cipher_std_data_arr) / sizeof(demo_sm2_cipher_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for SM2 cipher starts. ==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        /* 解密，先验证 SM2 的 API 功能是否正常 */
        ehsm_port_printf("[%s decryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_cipher(false, &std_data_arr[i]);

        /* 加密，采用先加密，后解密，最后比对明文的方式 */
        ehsm_port_printf("[%s decryption for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_cipher(true, &std_data_arr[i]);

        /* 解密，使用明文密钥，先验证 SM2 的 API 功能是否正常 */
        ehsm_port_printf(
            "[%s decryption with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_cipher_with_plain_key(false, &std_data_arr[i]);

        /* 加密，使用明文密钥，采用先加密，后解密，最后比对明文的方式 */
        ehsm_port_printf(
            "[%s encryption with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_cipher_with_plain_key(true, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for SM2 cipher ends. ==================== \r\n\r\n");
}

/**
 * @brief SM2 签名生成和校验的演示入口函数。
 */
static void sm2_sign_ver_entry(void)
{
    uint32_t i;
    const demo_sm2_sign_std_st *std_data_arr = s_sm2_sign_std_data_arr;
    uint32_t cnt = sizeof(s_sm2_sign_std_data_arr) / sizeof(demo_sm2_sign_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for SM2 signature starts. ==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        /* 一次性签名验证，先验证 SM2 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_onepass(false, &std_data_arr[i]);

        /* 一次性签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s generation for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_onepass(true, &std_data_arr[i]);

        /* 分段签名验证，先验证 SM2 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_stepwise(false, &std_data_arr[i]);

        /* 分段签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s decryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_stepwise(true, &std_data_arr[i]);

        /* 一次性签名验证，使用明文密钥，先验证 SM2 签名 API 的功能是否正常 */
        ehsm_port_printf(
            "[%s verification with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 一次性签名生成，使用明文密钥，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf(
            "[%s generation with plain key for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 分段签名验证，使用明文密钥，先验证 SM2 签名 API 的功能是否正常 */
        ehsm_port_printf(
            "[%s verification with plain key for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_stepwise_with_plain_key(false, &std_data_arr[i]);

        /* 分段签名生成，使用明文密钥，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf(
            "[%s generation with plain key for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        sm2_sign_ver_stepwise_with_plain_key(true, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for SM2 signature ends. ==================== \r\n\r\n");
}

/**
 * @brief SM2 算法演示入口函数。
 */
void ehsm_demo_sm2_entry(void)
{
    /* SM2 加解密示例入口函数。 */
    sm2_cipher_entry();

    /* SM2 签名生成和校验示例入口函数。 */
    sm2_sign_ver_entry();
}

