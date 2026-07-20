#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_ecdsa.h"

#define DEMO_ECDSA_KEY_HANDLE (0x100000U)

typedef struct {
    const char *alg_str;
    ehsm_hash_algo_e algo;
    ehsm_key_type_e key_type;
    const uint8_t *std_key;
    uint16_t std_key_size;
    uint16_t std_pub_key_size;
    uint16_t std_priv_key_size;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_signature;
    uint32_t std_signature_size;
} demo_ecdsa_sign_std_st;

// clang-format off
static const uint8_t s_ecdsa_std_key[64 + 32] = {
    /* 公钥 */
    0x06,0x1E,0x02,0x79,0xE8,0x9F,0x56,0xBF,0x16,0x31,0x43,0x15,0xF1,0x13,0xEA,0xAC,
    0xEA,0x56,0x92,0x15,0x30,0x16,0xA9,0xC6,0xD7,0xC0,0x27,0xB9,0x16,0xBB,0x50,0xCC,
    0x89,0x40,0xFA,0x1B,0x2A,0xA6,0xBB,0x9D,0x5F,0xA4,0xDF,0x53,0x6E,0x9D,0xB8,0x6D,
    0x8F,0x2D,0x12,0x5A,0xAA,0xCF,0x0A,0x14,0x42,0xB5,0x18,0x38,0x6B,0xA5,0xC6,0x5A,
    /* 私钥 */
    0x30,0x47,0xC1,0xD1,0xDD,0x76,0x68,0xA8,0xC6,0x26,0xD8,0x28,0x9D,0xE7,0x31,0x5F,
    0xD1,0x0D,0x57,0xB8,0x37,0x0C,0x3B,0x4A,0xB6,0xA4,0x78,0x90,0xAC,0xF7,0xCF,0x8A
};

static const uint8_t s_ecdsa_std_msg[80] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f
};

/**
 * ECDSA256R1-SHA256 加密使用的 digest，用于外部工具验证。
 * static const uint8_t s_ecdsa256r1_sha256_std_digest[32] = {
 *      0xC5,0x67,0x05,0xFE,0xA5,0xB1,0x10,0xB8,0xDC,0x63,0x68,0x85,0x33,0xCE,0xD2,0x11,
 *      0x67,0xE6,0x28,0x01,0x73,0x87,0xC8,0x85,0x42,0x3B,0x83,0x5A,0x55,0xED,0xD5,0xEF
 * };
 *
 * ECDSA256R1-SHA256 加密的使用随机数，用于外部工具验证。
 * static const uint8_t s_ecdsa256r1_sha256_std_rand_k[32] = {
 *     0x6F,0x9B,0xC7,0x51,0x95,0x0A,0xF1,0x47,0x29,0x86,0xF5,0xD0,0x70,0x36,0x2F,0xDF,
 *     0x52,0xA6,0x5B,0xB0,0xC8,0xD7,0x35,0xBC,0x06,0xD6,0x28,0x47,0xD4,0x03,0x6A,0xBE
 * };
 */
static const uint8_t s_ecdsa256r1_sha256_std_signature[64] = {
    0xF8,0x66,0xEE,0x34,0xF4,0x5B,0xC5,0x4D,0x14,0xF0,0xE1,0xC3,0x20,0xCD,0x11,0x47,
    0x07,0x1A,0x77,0x8A,0x13,0x63,0xA2,0xF3,0x89,0x1D,0xD5,0x8A,0x6F,0xCA,0xDA,0x28,
    0xD6,0x0B,0xFB,0xAC,0xF3,0xF5,0x64,0xA6,0xD0,0xA4,0xF0,0x6E,0xCB,0xAB,0xA8,0x47,
    0x63,0xD6,0xC9,0x03,0x97,0xD1,0xC2,0xD3,0xDC,0x68,0xD5,0xEB,0x1B,0x1B,0xD8,0x64
};

static const demo_ecdsa_sign_std_st s_ecdsa_sign_std_data_arr[1] = {
    {
        .alg_str = "ECDSA256R1 SHA256 signature",
        .algo = EHSM_HASH_ALGO_SHA256,
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .std_key = s_ecdsa_std_key,
        .std_key_size = sizeof(s_ecdsa_std_key),
        .std_pub_key_size = 64U,
        .std_priv_key_size = 32U,
        .std_msg = s_ecdsa_std_msg,
        .std_msg_size = sizeof(s_ecdsa_std_msg),
        .std_signature = s_ecdsa256r1_sha256_std_signature,
        .std_signature_size = sizeof(s_ecdsa256r1_sha256_std_signature),
    }
};

// clang-format on

/**
 * @brief 一次性计算 ECDSA 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_ecdsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 */
static void ecdsa_onepass(bool_t gen_sig, const demo_ecdsa_sign_std_st *std_data)
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
    uint32_t key_handle = DEMO_ECDSA_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t need_remove = false;                  /* 用于标记密钥是否需要移除 */

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
        need_remove = true; /* 标记导入的密钥需要移除 */
        if (gen_sig) {
            /**
             * 2. 调用 ECDSA 签名生成 API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             * - signature_size：
             *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于指定曲线的模长度；
             *      - 再作为输出，eHSM 会回写实际产生的签名长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_onepass_gen(ctx, std_data->algo, key_handle, msg, msg_size, signature, &signature_size);
            ret = demo_check_val("The execution of sign onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_ecdsa_onepass_verify(
                    ctx, std_data->algo, key_handle, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else {
            /* 2. 调用 ECDSA 签名验证 API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_onepass_verify(
                ctx, std_data->algo, key_handle, msg, msg_size, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of verify onepass API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
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
 * @brief 一次性计算 ECDSA 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_ecdsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 */
static void ecdsa_onepass_with_plain_key(bool_t gen_sig, const demo_ecdsa_sign_std_st *std_data)
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
    ehsm_ecc_key_st *ecc_key = (ehsm_ecc_key_st *)ehsm_demo_get_buffer(3); /* 存储密钥值的buffer */

    /* 初始化数据 buffer */
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

    /* 2. 准备密钥数据，存储再ecc_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);
    ecc_key->curve_id = EHSM_KEY_TYPE_ECC_SECP_256R1;
    ecc_key->pubkey = ehsm_port_addr_to_raddr(key);
    ecc_key->privkey = ehsm_port_addr_to_raddr(&key[64]);

    if (gen_sig) {
        /**
         * 3. 调用 ECDSA 签名生成 API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
         * - signature_size：
         *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于指定曲线的模长度；
         *      - 再作为输出，eHSM 会回写实际产生的签名长度；
         */
        signature_size = std_data->std_signature_size;
        ret = ehsm_ecdsa_onepass_gen_with_plain_key(
            ctx, std_data->algo, (uint8_t *)ecc_key, msg, msg_size, signature, &signature_size);
        ret = demo_check_val("The execution of sign onepass API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
            print_hex("The generated signature is:\r\n    ", signature, signature_size);

            /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
            ret = ehsm_ecdsa_onepass_verify_with_plain_key(
                ctx, std_data->algo, (uint8_t *)ecc_key, msg, msg_size, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
        }
    } else {
        /* 3. 调用 ECDSA 签名验证 API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
        signature_size = std_data->std_signature_size;
        ret = ehsm_ecdsa_onepass_verify_with_plain_key(
            ctx, std_data->algo, (uint8_t *)ecc_key, msg, msg_size, signature, signature_size, &verify_result);
        ret = demo_check_val("The execution of verify onepass API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，检查签名验证结果 */
            ret = demo_check_val("Signature verification:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算 ECDSA 签名生成和验证的示例。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_ecdsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 SM3 的 hash 算法，使用 ECDSA 公钥和 ID 计算 SM3 digest 的第一个 block。
 *      - ECDSA 签名使用固定的 ID，是签名者的可辨别标识，长度一般是 16 字节，以 ASCII 编码表示为“1234567812345678”，
 *          即 U8 字节串 {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
 * - 对于 update API，主要是对输入的消息计算 SM3 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 SM3 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 */
static void ecdsa_stepwise(bool_t gen_sig, const demo_ecdsa_sign_std_st *std_data)
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
    uint32_t key_handle = DEMO_ECDSA_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t need_remove = false;                  /* 用于标记密钥是否需要移除 */

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
        need_remove = true; /* 标记导入的密钥需要移除 */

        /* 2. 调用 init API。需要输入 hash 算法，密钥 handle、生成/验证方向、分段计算的会话 buffer。*/
        ret = ehsm_ecdsa_init(ctx, std_data->algo, key_handle, gen_sig, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* 3.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_ecdsa_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 3.1 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_ecdsa_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 4. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于指定曲线的模长度；
             *      - 再作为输出，eHSM 会回写实际产生的签名长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_finish_gen(ctx, signature, &signature_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_ecdsa_onepass_verify(
                    ctx, std_data->algo, key_handle, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else { /* 签名验证 */
            /* 4. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_finish_verify(ctx, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
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
 * @brief 分段计算 ECDSA 签名生成和验证的示例，使用明文密钥。
 * @param[in] gen_sig 是否为签名生成
 *          - true 指签名生成；
 *          - false 指签名验证；
 * @param[in] std_data 标准数据，详见 @ref demo_ecdsa_sign_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 * - 对于签名生成，由于签名过程中引入随机数，eHSM 产生的签名和标准签名不一致，无法直接通过比对数据校验，
 * 因此采用先调用签名生成 API 对明文消息签名，再调用签名验证 API 对产生的签名进行验证。
 * - 对于签名验证，直接用标准数据进行 验证即可。
 * - 为确保 eHSM 的 API 都是正常的，应当先调用签名验证接口，对标准数据进行验证，证明 eHSM 的 API
 * 和功能是符合标准的，然后再采用先签名，后验签的方式验证签名 API 的功能。
 * - 对于 init API，主要是初始化 SM3 的 hash 算法，使用 ECDSA 公钥和 ID 计算 SM3 digest 的第一个 block。
 *      - ECDSA 签名使用固定的 ID，是签名者的可辨别标识，长度一般是 16 字节，以 ASCII 编码表示为“1234567812345678”，
 *          即 U8 字节串 {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
 * - 对于 update API，主要是对输入的消息计算 SM3 hash digest，所有的消息必须在 update 阶段输入。
 * - 对于 finish API，主要是对 update 阶段迭代计算的 SM3 hash 值进行签名，请确保已经通过 update API 将所有的消息输入。
 */
static void ecdsa_stepwise_with_plain_key(bool_t gen_sig, const demo_ecdsa_sign_std_st *std_data)
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
    bool_t verify_result = false;           /* 用于接收签名验证的结果 */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    ehsm_ecc_key_st *ecc_key = (ehsm_ecc_key_st *)ehsm_demo_get_buffer(3); /* 存储密钥值的buffer */

    /* 初始化数据 buffer */
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

    /* 2. 准备密钥数据，存储再ecc_key中*/
    memcpy(key, std_data->std_key, std_data->std_key_size);
    ecc_key->curve_id = EHSM_KEY_TYPE_ECC_SECP_256R1;
    ecc_key->pubkey = ehsm_port_addr_to_raddr(key);
    ecc_key->privkey = ehsm_port_addr_to_raddr(&key[64]);

    /* 3. 调用 init API。需要输入 hash 算法，密钥 handle、生成/验证方向、分段计算的会话 buffer。*/
    ret = ehsm_ecdsa_init_with_plain_key(ctx, std_data->algo, (uint8_t *)ecc_key, gen_sig, session);
    ret = demo_check_val("The execution of init API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 4.1 第一次调用 update API 更新，假设本次更新 16 字节的消息。*/
        msg_size = 16U;
        ret = ehsm_ecdsa_update(ctx, msg, msg_size);
        ret = demo_check_val("The 1st execution of update API:", EHSM_OK, ret);
        already_inputed_size = msg_size;

        /* 4.1 第二次调用 update API 更新，假设本次更新剩余所有消息。*/
        msg_size = std_data->std_msg_size - already_inputed_size;
        ret = ehsm_ecdsa_update(ctx, &msg[already_inputed_size], msg_size);
        ret = demo_check_val("The 2nd execution of update API:", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_sig) { /* 签名生成 */
            /**
             * 5. 调用 finish API。需要输入所有消息，API 完成计算后会将签名返回给 SoC。
             *      - 首先作为输入，指定 signature buffer 的长度，必须大于等于指定曲线的模长度；
             *      - 再作为输出，eHSM 会回写实际产生的签名长度；
             */
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_finish_gen(ctx, signature, &signature_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);

            if (EHSM_OK == ret) {
                /*由于签名生成含有随机数，因此无法直接校输出的签名，采用先签名，后验签的方式校验 */
                print_hex("The generated signature is:\r\n    ", signature, signature_size);

                /* 这里仅仅为了验证 eHSM 生成的签名，因此采用 onepass API。 */
                ret = ehsm_ecdsa_onepass_verify_with_plain_key(
                    ctx, std_data->algo, (uint8_t *)ecc_key, msg, msg_size, signature, signature_size, &verify_result);
                ret = demo_check_val(
                    "The execution of verify onepass API for verifing generated signature:", EHSM_OK, ret);
            }
        } else { /* 签名验证 */
            /* 5. 调用 finish API。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
            signature_size = std_data->std_signature_size;
            ret = ehsm_ecdsa_finish_verify(ctx, signature, signature_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，检查签名验证结果 */
                ret = demo_check_val("Signature verification:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief ECDSA 生成和校验的演示入口函数。
 */
void ehsm_demo_ecdsa_entry(void)
{
    uint32_t i;
    const demo_ecdsa_sign_std_st *std_data_arr = s_ecdsa_sign_std_data_arr;
    uint32_t cnt = sizeof(s_ecdsa_sign_std_data_arr) / sizeof(demo_ecdsa_sign_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for ECDSA signature starts. ==================== "
                     "\r\n\r\n");

    /* TODO: 增加 曲线和 hash 算法 */
    for (i = 0U; i < cnt; i++) {
        /* 一次性签名验证，先验证 ECDSA 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_onepass(false, &std_data_arr[i]);

        /* 一次性签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s generation for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_onepass(true, &std_data_arr[i]);

        /* 分段签名验证，先验证 ECDSA 签名 API 的功能是否正常 */
        ehsm_port_printf("[%s verification for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_stepwise(false, &std_data_arr[i]);

        /* 分段签名生成，采用先生成签名，后验证签名的方式 */
        ehsm_port_printf("[%s decryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_stepwise(true, &std_data_arr[i]);

        /* 一次性签名验证，先验证 ECDSA 签名 API 的功能是否正常, 使用明文密钥*/
        ehsm_port_printf("[%s verification for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 一次性签名生成，采用先生成签名，后验证签名的方式, 使用明文密钥*/
        ehsm_port_printf("[%s generation for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 分段签名验证，先验证 ECDSA 签名 API 的功能是否正常, 使用明文密钥*/
        ehsm_port_printf("[%s verification for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_stepwise_with_plain_key(false, &std_data_arr[i]);

        /* 分段签名生成，采用先生成签名，后验证签名的方式, 使用明文密钥*/
        ehsm_port_printf("[%s decryption for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        ecdsa_stepwise_with_plain_key(true, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for ECDSA signature ends. ==================== \r\n\r\n");
}

