#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_mac.h"

#define DEMO_MAC_KEY_HANDLE (0x100000U)

typedef struct {
    ehsm_symm_algo_e algo;
    ehsm_mac_mode_e mode;
    ehsm_key_type_e key_type;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    const uint8_t *std_iv;
    uint32_t std_iv_size;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_mac;
    uint32_t std_mac_size;
} demo_mac_std_st;

// clang-format off
static const uint8_t s_mac_std_key[32] = {
    0x89,0xB7,0xA3,0x35,0x92,0xE1,0x0D,0xB7,0xA0,0xB0,0xE2,0x7E,0x16,0xB0,0xE2,0xF1,
    0xE3,0xCA,0x70,0xDE,0xFB,0xA1,0x6F,0x3F,0x1E,0xF8,0x71,0xB0,0x29,0x04,0xBB,0x1D,
};

static const uint8_t s_mac_std_iv[12] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b
};

static const uint8_t s_mac_std_msg[64] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f
};

static const uint8_t s_aes128_cmac_std_mac[16] = {
    0x5C,0xF5,0xA9,0xA1,0x32,0x6A,0xD5,0xDC,0x5C,0x79,0x1C,0xF3,0xEE,0xFB,0x40,0xA8
};

static const uint8_t s_aes256_cbcmac_std_mac[16] = {
    0xA0,0x48,0xE5,0x66,0x26,0x13,0x19,0x9A,0x2E,0xE3,0xA5,0x62,0xC8,0x1C,0x16,0x60
};

static const uint8_t s_sm4_gmac_std_mac[16] = {
    0x3E,0xF1,0x39,0x80,0x34,0xC7,0xEC,0x2E,0x0B,0x22,0x1D,0x8E,0xC8,0xAE,0x20,0xCF
};


static const demo_mac_std_st s_mac_std_data_arr[3] = {
    {
        .algo = EHSM_SYMM_ALGO_AES_128,
        .mode = EHSM_MAC_MODE_CMAC,
        .alg_str = "AES_128_CMAC",
        .key_type = EHSM_KEY_TYPE_AES_128,
        .std_key = s_mac_std_key,
        .std_key_size = 16U,
        .std_iv = NULL,
        .std_iv_size = 0U,
        .std_msg = s_mac_std_msg,
        .std_msg_size = sizeof(s_mac_std_msg),
        .std_mac = s_aes128_cmac_std_mac,
        .std_mac_size = sizeof(s_aes128_cmac_std_mac),
    }, {
        .algo = EHSM_SYMM_ALGO_AES_256,
        .mode = EHSM_MAC_MODE_CBC_MAC,
        .alg_str = "AES_256_CBCMAC",
        .key_type = EHSM_KEY_TYPE_AES_256,
        .std_key = s_mac_std_key,
        .std_key_size = 32U,
        .std_iv = NULL,
        .std_iv_size = 0U,
        .std_msg = s_mac_std_msg,
        .std_msg_size = sizeof(s_mac_std_msg),
        .std_mac = s_aes256_cbcmac_std_mac,
        .std_mac_size = sizeof(s_aes256_cbcmac_std_mac),
    }, {
        .algo = EHSM_SYMM_ALGO_SM4,
        .mode = EHSM_MAC_MODE_GMAC,
        .alg_str = "SM4_GMAC",
        .key_type = EHSM_KEY_TYPE_SM4,
        .std_key = s_mac_std_key,
        .std_key_size = 16U,
        .std_iv = s_mac_std_iv,
        .std_iv_size = sizeof(s_mac_std_iv),
        .std_msg = s_mac_std_msg,
        .std_msg_size = sizeof(s_mac_std_msg),
        .std_mac = s_sm4_gmac_std_mac,
        .std_mac_size = sizeof(s_sm4_gmac_std_mac),
    }
};

// clang-format on

/**
 * @brief 一次性计算对称算法 mac 生成和校验示例，使用 onepass API。
 * @param[in] gen_mac 是否 mac 生成
 *          - true 指 mac 生成；
 *          - false 指 mac 校验；
 * @param[in] std_data 标准数据，详见 @ref demo_mac_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void mac_onepass(bool_t gen_mac, const demo_mac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_MAC_KEY_HANDLE;           /* 用于指定导入密钥的 key handle */
    uint8_t *iv = ehsm_demo_get_buffer(1);      /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size = 0U;                      /* 输入 iv 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(2);     /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size = std_data->std_msg_size; /* 输入数据的长度 */
    uint8_t *mac = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size = std_data->std_mac_size; /* mac 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 mac 校验过程 */
    bool_t need_remove = false;             /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key_data, 0x0U, 512U);
    memset(iv, 0x0U, 16U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, msg_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_mac) {
        /* 校验时 mac 作为输入 */
        memcpy(mac, std_data->std_mac, mac_size); /* 将 mac 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }
    if (EHSM_MAC_MODE_GMAC == std_data->mode) {
        /* GMAC 需要固定 12 字节的 iv，其他模式不需要 */
        iv_size = std_data->std_iv_size;
        memcpy(iv, std_data->std_iv, iv_size); /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        if (gen_mac) {
            /**
             * 4. 调用 onepass API。需要输入所有消息，API 完成计算后会将生成的 mac 返回给 SoC。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            ret = ehsm_mac_onepass_gen(
                ctx, std_data->algo, std_data->mode, key_handle, iv, iv_size, msg, msg_size, mac, mac_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验 mac */
                ret = demo_check_data("mac comparison:", std_data->std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 4. 调用 onepass API。需要输入所有消息和 mac 值，API 完成计算后会比对传入的 mac 值和 eHSM 生成的 mac
             * 值是否一致，并将校验结果返回给 SoC。保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            ret = ehsm_mac_onepass_verify(ctx, std_data->algo, std_data->mode, key_handle, iv, iv_size, msg, msg_size,
                mac, mac_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify mac:", true, verify_result);
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
 * @brief 一次性计算对称算法 mac 生成和校验示例，使用 onepass API，使用明文密钥。
 * @param[in] gen_mac 是否 mac 生成
 *          - true 指 mac 生成；
 *          - false 指 mac 校验；
 * @param[in] std_data 标准数据，详见 @ref demo_mac_std_st
 *
 * @note
 * - 任意一次调用 API 返回错误后，应终止本次计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void mac_onepass_with_plain_key(bool_t gen_mac, const demo_mac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *iv = ehsm_demo_get_buffer(1);      /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size = 0U;                      /* 输入 iv 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(2);     /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size = std_data->std_msg_size; /* 输入数据的长度 */
    uint8_t *mac = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size = std_data->std_mac_size; /* mac 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 mac 校验过程 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(iv, 0x0U, 16U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, msg_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_mac) {
        /* 校验时 mac 作为输入 */
        memcpy(mac, std_data->std_mac, mac_size); /* 将 mac 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }
    if (EHSM_MAC_MODE_GMAC == std_data->mode) {
        /* GMAC 需要固定 12 字节的 iv，其他模式不需要 */
        iv_size = std_data->std_iv_size;
        memcpy(iv, std_data->std_iv, iv_size); /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    if (gen_mac) {
        /**
         * 4. 调用 onepass API。需要输入所有消息，API 完成计算后会将生成的 mac 返回给 SoC。
         * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        ret = ehsm_mac_onepass_gen_with_plain_key(ctx, std_data->algo, std_data->mode, key, std_data->std_key_size, iv,
            iv_size, msg, msg_size, mac, mac_size);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，需要校验 mac */
            ret = demo_check_data("mac comparison:", std_data->std_mac, mac_size, mac, mac_size);
        }
    } else { /* 校验 */
        /**
         * 4. 调用 onepass API。需要输入所有消息和 mac 值，API 完成计算后会比对传入的 mac 值和 eHSM 生成的 mac
         * 值是否一致，并将校验结果返回给 SoC。保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        ret = ehsm_mac_onepass_verify_with_plain_key(ctx, std_data->algo, std_data->mode, key, std_data->std_key_size,
            iv, iv_size, msg, msg_size, mac, mac_size, &verify_result);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* 检查校验结果 */
            ret = demo_check_val("Verify mac:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 分段计算对称算法 mac 生成和校验的示例，使用 init、update 和 finish APIs。
 * @param[in] gen_mac 是否 mac 生成
 *          - true 指 mac 生成；
 *          - false 指 mac 校验；
 * @param[in] std_data 标准数据，详见 @ref demo_mac_std_st
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void mac_stepwise(bool_t gen_mac, const demo_mac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_MAC_KEY_HANDLE;           /* 用于指定导入密钥的 key handle */
    uint8_t *iv = ehsm_demo_get_buffer(1);  /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size = 0U;                  /* 输入 iv 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(2); /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 输入数据的长度 */
    uint8_t *mac = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size = std_data->std_mac_size; /* mac 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 mac 校验过程 */
    bool_t need_remove = false;             /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key_data, 0x0U, 512U);
    memset(iv, 0x0U, 16U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_mac) {
        /* 校验时 mac 作为输入 */
        memcpy(mac, std_data->std_mac, mac_size); /* 将 mac 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    if (EHSM_MAC_MODE_GMAC == std_data->mode) {
        /* GMAC 需要固定 12 字节的 iv，其他模式不需要 */
        iv_size = std_data->std_iv_size;
        memcpy(iv, std_data->std_iv, iv_size); /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), std_data->key_type, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、密钥 key handle、是否 mac 生成、iv(iv)
         * 及长度、输入数据的总长度、mac 的长度和 eHSM 初始化分段计算的 session。
         *
         */
        ret = ehsm_mac_init(ctx, std_data->algo, std_data->mode, key_handle, gen_mac, iv, iv_size, mac_size, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、输入数据、输入数据的字节长度、输出 buffer，eHSM
         * 计算完成后回写输出数据到输出 buffer。update API 仅支持 block size 整数倍的输入长度。假设本次计算 16
         * 个字节，累计共 16 字节。第一次调用 update API 之前，必须先调用一次 init API。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        msg_size = 16U;
        ret = ehsm_mac_update(ctx, msg, msg_size); /* 调用 update API 更新 */
        already_inputed_size = msg_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM 支持多次更新，每次更新
         * eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16 + 16 = 32 字节。
         */
        msg_size = 16U;
        ret = ehsm_mac_update(ctx, &msg[already_inputed_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_mac) {
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据、生成的 mac 返回给 SoC。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            msg_size = std_data->std_msg_size - already_inputed_size;
            ret = ehsm_mac_finish_gen(ctx, &msg[already_inputed_size], msg_size, mac);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验 mac */
                ret = demo_check_data("mac comparison:", std_data->std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据返回给 SoC，并校验输入的 mac。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            msg_size = std_data->std_msg_size - already_inputed_size;
            ret = ehsm_mac_finish_verify(ctx, &msg[already_inputed_size], msg_size, mac, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify mac:", true, verify_result);
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
 * @brief 分段计算对称算法 mac 生成和校验的示例，使用 init、update 和 finish APIs，使用明文密钥。
 * @param[in] gen_mac 是否 mac 生成
 *          - true 指 mac 生成；
 *          - false 指 mac 校验；
 * @param[in] std_data 标准数据，详见 @ref demo_mac_std_st
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入数据进行更新。
 * - 该函数调用 1 次 init、2 次 update 和 1 次 finish。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次加计算，并忽略任何中间数据，如 ctx，session 等。
 */
static void mac_stepwise_with_plain_key(bool_t gen_mac, const demo_mac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0);             /* 存储密钥值的buffer */
    uint8_t *iv = ehsm_demo_get_buffer(1);  /* 输入 iv 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size = 0U;                  /* 输入 iv 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(2); /* 输入数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 输入数据的长度 */
    uint8_t *mac = ehsm_demo_get_buffer(3); /* 存储输出数据的 buff，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size = std_data->std_mac_size; /* mac 的长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置信息的 buffer */
    uint32_t already_inputed_size;          /* 多次更新时，记录已经输入的数据总长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的验证结果，仅用于 mac 校验过程 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(iv, 0x0U, 16U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size); /* 将输入数据拷贝至 SoC 与 eHSM 的共享内存上。 */
    if (!gen_mac) {
        /* 校验时 mac 作为输入 */
        memcpy(mac, std_data->std_mac, mac_size); /* 将 mac 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }

    if (EHSM_MAC_MODE_GMAC == std_data->mode) {
        /* GMAC 需要固定 12 字节的 iv，其他模式不需要 */
        iv_size = std_data->std_iv_size;
        memcpy(iv, std_data->std_iv, iv_size); /* 将 iv 拷贝至 SoC 与 eHSM 的共享内存上。 */
    }
    memcpy(key, std_data->std_key, std_data->std_key_size); /* 密钥值拷贝至 SoC 与 eHSM 的共享内存上。 */

    /* 打印密钥、iv、输入数据、输出数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std iv is: \r\n    ", std_data->std_iv, std_data->std_iv_size);
    print_hex("The std msg is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 init API 进行初始化，输入 ctx、算法、密码模式、明文密钥、密钥长度、是否 mac 生成、iv(iv)
     * 及长度、输入数据的总长度、mac 的长度和 eHSM 初始化分段计算的 session。
     *
     */
    ret = ehsm_mac_init_with_plain_key(
        ctx, std_data->algo, std_data->mode, key, std_data->std_key_size, gen_mac, iv, iv_size, mac_size, session);
    ret = demo_check_val("The execution of init API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、输入数据、输入数据的字节长度、输出 buffer，eHSM
         * 计算完成后回写输出数据到输出 buffer。update API 仅支持 block size 整数倍的输入长度。假设本次计算 16
         * 个字节，累计共 16 字节。第一次调用 update API 之前，必须先调用一次 init API。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        msg_size = 16U;
        ret = ehsm_mac_update(ctx, msg, msg_size); /* 调用 update API 更新 */
        already_inputed_size = msg_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，update API 仅支持 block size 整数倍的输入长度。eHSM 支持多次更新，每次更新
         * eHSM 都会基于上一次更新返回的 session 接着计算，假设本次计算 16 字节，则累计算 16 + 16 = 32 字节。
         */
        msg_size = 16U;
        ret = ehsm_mac_update(ctx, &msg[already_inputed_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_inputed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        if (gen_mac) {
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据、生成的 mac 返回给 SoC。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            msg_size = std_data->std_msg_size - already_inputed_size;
            ret = ehsm_mac_finish_gen(ctx, &msg[already_inputed_size], msg_size, mac);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /** eHSM FW 返回成功，需要校验 mac */
                ret = demo_check_data("mac comparison:", std_data->std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 4. 调用 finish API。API 完成计算后会将最后一次输出数据返回给 SoC，并校验输入的 mac。
             * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
             */
            msg_size = std_data->std_msg_size - already_inputed_size;
            ret = ehsm_mac_finish_verify(ctx, &msg[already_inputed_size], msg_size, mac, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify mac:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief 对称算法 mac 生成和校验的演示入口函数。包含不同对称算法、密码模式的示例。
 */
void ehsm_demo_mac_entry(void)
{
    uint32_t i;
    const demo_mac_std_st *std_data_arr = s_mac_std_data_arr;
    uint32_t cnt = sizeof(s_mac_std_data_arr) / sizeof(demo_mac_std_st);

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for symmetric mac starts. ==================== "
                     "\r\n\r\n");

    /* TODO: 加入其他 mac 算法 */
    for (i = 0U; i < cnt; i++) {
        /* 一次性生成 */
        ehsm_port_printf("[%s generation for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        mac_onepass(true, &std_data_arr[i]);
        mac_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 一次性校验 */
        ehsm_port_printf("[%s verification for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        mac_onepass(false, &std_data_arr[i]);
        mac_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 分段生成 */
        ehsm_port_printf("[%s generation for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        mac_stepwise(true, &std_data_arr[i]);
        mac_stepwise_with_plain_key(true, &std_data_arr[i]);

        /* 分段校验 */
        ehsm_port_printf("[%s verification for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        mac_stepwise(false, &std_data_arr[i]);
        mac_stepwise_with_plain_key(false, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for symmetric mac ends. ==================== \r\n\r\n");
}

