#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_hmac.h"

#define DEMO_HMAC_KEY_HANDLE (0x100000U)

typedef struct {
    ehsm_hash_algo_e algo;
    const char *alg_str;
    const uint8_t *std_key;
    uint16_t std_key_size;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_mac;
    uint32_t std_mac_size;
} demo_hmac_std_st;

// clang-format off
static const uint8_t s_hmac_std_key[64] = {
    0x89,0xB7,0xA3,0x35,0x92,0xE1,0x0D,0xB7,0xA0,0xB0,0xE2,0x7E,0x16,0xB0,0xE2,0xF1,
    0xE3,0xCA,0x70,0xDE,0xFB,0xA1,0x6F,0x3F,0x1E,0xF8,0x71,0xB0,0x29,0x04,0xBB,0x1D,
    0x53,0x16,0x33,0x6C,0xAA,0x0E,0xD0,0xD1,0x61,0x03,0x27,0x99,0xF3,0x73,0xE4,0x98,
    0x05,0x84,0xB5,0x79,0x68,0xA7,0x67,0xB4,0xF6,0x4D,0x85,0x8B,0xDB,0xB6,0xE7,0xC8
};

static const uint8_t s_hmac_std_msg[96] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f
};

static const uint8_t s_sha256_std_mac[32] = {
    0x9D,0x4B,0x64,0x3E,0x97,0x58,0x0E,0xB3,0x3E,0x5A,0xE3,0xBA,0x90,0x88,0xD6,0x31,
    0x8D,0x89,0x75,0xF4,0xBA,0x0D,0x46,0x94,0x2E,0x27,0xB1,0xC2,0xF0,0x5C,0xA9,0x3B
};

static const demo_hmac_std_st s_hmac_std_data_arr[1] = {
    {
        .algo = EHSM_HASH_ALGO_SHA256,
        .alg_str = "SHA256",
        .std_key = s_hmac_std_key,
        .std_key_size = 32U,
        .std_msg = s_hmac_std_msg,
        .std_msg_size = sizeof(s_hmac_std_msg),
        .std_mac = s_sha256_std_mac,
        .std_mac_size = 32U,
    }
};
// clang-format on

/**
 * @brief Hmac 一次性计算生成或校验 mac 值的示例，使用 onepass API。
 * @param[in] gen_hmac 是否是 hmac 生成。
 *          - true 指 hmac 生成；
 *          - false 指 hmac 校验；
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hmac_std_st。
 *
 * @note 输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 */
static void hmac_cal_onepass(bool_t gen_hmac, const demo_hmac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1); /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 消息的总长度 */
    uint8_t *mac = ehsm_demo_get_buffer(2); /* 存储输出 mac 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size;                      /* 指定存储 mac 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    const uint8_t *std_mac = NULL;          /* 标准 hmac 的指针 */
    uint32_t key_handle = DEMO_HMAC_KEY_HANDLE; /* 用于指定导入密钥的 key handle */
    bool_t verify_result = false; /* 接收 eHSM 返回的 mac 校验结果，仅用于 hmac 校验过程 */
    bool_t need_remove = false;   /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 打印标准密钥、标准 message、标准 hmac 数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std hmac for valid message is: \r\n    ", std_data->std_mac, std_data->std_mac_size);

    /* 将消息拷贝至 SoC 与 eHSM 的共享内存上，消息为{0，1，2，...，95}，消息总长 96 字节。 */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size);
    std_mac = std_data->std_mac;
    if (!gen_hmac) {
        /* 校验需要拷贝 mac 值 */
        memcpy(mac, std_data->std_mac, std_data->std_mac_size);
    }
    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入 hmac 密钥并获取 key handle，若 eHSM RAM 已经存在 hmac 密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示 hmac 计算，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), EHSM_KEY_TYPE_HMAC, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 2. 调用 hash onepass API 计算并获取 mac，eHSM 会计算 mac 并回写到 SoC 提供的 mac buffer，首先由 SoC 输入
         * mac buffer 的长度，需要保证大于等于所选 hash 算法的标准 mac 长度，当 API 执行完毕，会返回实际的 mac size。
         */

        if (gen_hmac) {
            /** 3. 调用 onepass API 计算并读取 mac 值，eHSM 会计算 mac 并回写 mac_size 长度的 mac 值到 SoC 提供的
             * mac buffer。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_onepass_gen(ctx, std_data->algo, key_handle, msg, msg_size, mac, mac_size);
            ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* eHSM FW 返回成功，需要校验生成的 mac 和返回的 mac size 是否正确 */
                ret = demo_check_data(NULL, std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 3. 调用 onepass API 校验 mac，eHSM 会计算 hmac 并与输入的 hmac 值进行比较，比较结果会回写到
             * verify_result。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_onepass_verify(
                ctx, std_data->algo, key_handle, msg, msg_size, mac, mac_size, &verify_result);
            ret = demo_check_val("The execution of onepass API:", EHSM_OK, ret);
            ret = demo_check_val("Verify mac:", true, verify_result);
        }
    }

    if (need_remove) {
        demo_remove_key(ctx, key_handle);
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Demo of HMAC-%s-%s stepwise ends with %s !!!] \r\n\r\n", std_data->alg_str,
        (gen_hmac ? "generation" : "verification"), ret_str);
}

/**
 * @brief Hmac 分段计算生成或校验 mac 值的示例函数，使用 init、update 和 finish APIs。
 * @param[in] gen_hmac 是否是 hmac 生成。
 *          - true 指 hmac 生成；
 *          - false 指 hmac 校验；
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hmac_std_st。
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入消息更新 mac。该函数输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 * 该函数调用 1 次 init、3 次 update 和 1 次 finish，3 次 update 过程的输入消息长度分别为 10，70，16 字节。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次 hmac 计算，并忽略任何中间数据，如 ctx，session
 * 等。
 */
static void hmac_cal_stepwise(bool_t gen_hmac, const demo_hmac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 密钥数据 buffer 的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1); /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 消息的总长度 */
    uint8_t *mac = ehsm_demo_get_buffer(2); /* 存储输出 mac 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size;                      /* 指定存储 mac 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    const uint8_t *std_mac = NULL;          /* 标准 mac 的指针 */
    uint32_t already_size;                  /* 记录已经输入的消息长度 */
    uint32_t key_handle = DEMO_HMAC_KEY_HANDLE; /* 指定导入密钥的 key handle */
    bool_t verify_result = false; /* 接收 eHSM 返回的 mac 校验结果，仅用于 mac 校验过程 */
    bool_t need_remove = false;   /* 用于标记密钥是否需要移除 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key_data, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 打印标准密钥、标准 message、标准 mac 数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, std_data->std_mac_size);

    /* 将消息拷贝至 SoC 与 eHSM 的共享内存上，消息为{0，1，2，...，95}，消息总长 96 字节。 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size);
    std_mac = std_data->std_mac;
    if (!gen_hmac) {
        /* 校验需要拷贝 mac 值 */
        memcpy(mac, std_data->std_mac, std_data->std_mac_size);
    }

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 导入 hmac 密钥并获取 key handle，若 eHSM RAM 已经存在 hmac 密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示 hmac 计算，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->std_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY), EHSM_KEY_TYPE_HMAC, EHSM_KEY_PART_SYMM_KEY,
        std_data->std_key, std_data->std_key_size, 0U, std_data->std_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /**
         * 2. 调用 init API 进行 hmac 计算的初始化，输入 ctx、hash 算法、密钥的 key handle、操作方式（是否是 hmac
         * 生成）和 eHSM 初始化分段计算的 session。
         */
        ret = ehsm_hmac_init(ctx, std_data->algo, key_handle, gen_hmac, session);
        ret = demo_check_val("The execution of init API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、明文消息、消息的字节长度，eHSM 读取明文消息并计算 mac。
         * 假设本次计算 10 个字节，累计共 10 字节。第一次调用 update API 之前，必须先调用一次 init API。
         * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        msg_size = 10U;
        ret = ehsm_hmac_update(ctx, msg, msg_size); /* 调用 update API 更新 */
        already_size = msg_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算 70 个字节，累计共 70 + 10 = 80 字节。
         */
        msg_size = 70U;
        ret = ehsm_hmac_update(ctx, &msg[already_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_size += msg_size;
    }

    if (EHSM_OK == ret) {
        /** 3.3 第三次调用 update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算剩余的 16 个字节，累计共 70 + 10 + 16 = 96 字节。
         */
        msg_size = std_data->std_msg_size - already_size;
        ret = ehsm_hmac_update(ctx, &msg[already_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 3nd execution of finish API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        if (gen_hmac) {
            /** 4. 调用 finish API 计算并读取 mac 值，eHSM 会计算 mac 并回写 mac_size 长度的 mac 值到 SoC 提供的 mac
             * buffer。调用 finish API 前，需要确保所有明文消息已经 update，或者无明文消息。保存中间数据的 session
             * 指针已经在调用 init API 时保存在 ctx 中。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_finish_gen(ctx, mac, mac_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* eHSM FW 返回成功，需要校验生成的 mac 和返回的 mac size 是否正确 */
                ret = demo_check_data("Hmac value comparison", std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 4. 调用 finish API 校验 hmac，eHSM 会计算 hmac 并与输入的 hmac 值进行比较，比较结果会回写到
             * verify_result。调用 finish API 前，需要确保所有明文消息已经 update，或者无明文消息。保存中间数据的
             * session 指针已经在调用 init API 时保存在 ctx 中。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_finish_verify(ctx, mac, mac_size, &verify_result);
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
 * @brief Hmac 一次性计算生成或校验 mac 值的示例，使用明文密钥。
 * @param[in] gen_hmac 是否是 hmac 生成。
 *          - true 指 hmac 生成；
 *          - false 指 hmac 校验；
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hmac_std_st。
 *
 * @note 输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 */
static void hmac_onepass_with_plain_key(bool_t gen_hmac, const demo_hmac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    uint8_t *key = ehsm_demo_get_buffer(0); /* 明文密钥的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_size;                      /* 密钥的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1); /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 消息的总长度 */
    uint8_t *mac = ehsm_demo_get_buffer(2); /* 存储输出 mac 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size;                      /* 指定存储 mac 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    const uint8_t *std_mac = NULL;          /* 标准 hmac 的指针 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的 mac 校验结果，仅用于 hmac 校验过程 */

    /* 初始化数据 buffer */
    memset(key, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    key_size = std_data->std_key_size;
    memcpy(key, std_data->std_key, key_size);
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size);
    if (!gen_hmac) {
        /* 校验需要拷贝 mac 值 */
        memcpy(mac, std_data->std_mac, std_data->std_mac_size);
    }

    /* 打印标准密钥、标准 message、标准 hmac 数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std hmac for valid message is: \r\n    ", std_data->std_mac, std_data->std_mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    std_mac = std_data->std_mac;

    if (gen_hmac) {
        /**
         * 2. 调用 HMAC 生成 API，使用明文密钥。需要输入所有消息，API 完成计算后会将 mac 返回给 SoC。
         */
        mac_size = std_data->std_mac_size;
        ret = ehsm_hmac_onepass_gen_with_plain_key(ctx, std_data->algo, key, key_size, msg, msg_size, mac, mac_size);
        ret = demo_check_val("The execution of hmac onepass gen with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，需要校验生成的 mac 和返回的 mac size 是否正确 */
            print_hex("The generated hmac is:\r\n    ", mac, mac_size);
            ret = demo_check_data(NULL, std_mac, mac_size, mac, mac_size);
        }
    } else {
        /* 2. 调用 HMAC 验证 API，使用明文密钥。需要输入所有消息，API 完成计算后会将验证结果返回给 SoC。*/
        mac_size = std_data->std_mac_size;
        ret = ehsm_hmac_onepass_verify_with_plain_key(
            ctx, std_data->algo, key, key_size, msg, msg_size, mac, mac_size, &verify_result);
        ret = demo_check_val("The execution of hmac onepass verify with plain key API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /** eHSM FW 返回成功，检查 hmac 验证结果 */
            ret = demo_check_val("HMAC verification:", true, verify_result);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Demo of HMAC-%s-%s with plain key ends with %s !!!] \r\n\r\n", std_data->alg_str,
        (gen_hmac ? "generation" : "verification"), ret_str);
}

/**
 * @brief Hmac 分段计算生成或校验 mac 值的示例函数，使用明文密钥。
 * @param[in] gen_hmac 是否是 hmac 生成。
 *          - true 指 hmac 生成；
 *          - false 指 hmac 校验；
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hmac_std_st。
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入消息更新 mac。该函数输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 * 该函数调用 1 次 init、3 次 update 和 1 次 finish，3 次 update 过程的输入消息长度分别为 10，70，16 字节。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次 hmac 计算，并忽略任何中间数据，如 ctx，session
 * 等。
 */
static void hmac_stepwise_with_plain_key(bool_t gen_hmac, const demo_hmac_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session(); /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                           FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *key = ehsm_demo_get_buffer(0); /* 明文密钥的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_size;                      /* 密钥的长度 */
    uint8_t *msg = ehsm_demo_get_buffer(1); /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                      /* 消息的总长度 */
    uint8_t *mac = ehsm_demo_get_buffer(2); /* 存储输出 mac 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t mac_size;                      /* 指定存储 mac 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    const uint8_t *std_mac = NULL;          /* 标准 mac 的指针 */
    uint32_t already_size;                  /* 记录已经输入的消息长度 */
    bool_t verify_result = false;           /* 接收 eHSM 返回的 mac 校验结果，仅用于 mac 校验过程 */

    /* 初始化数据 buffer */
    memset(session, 0x0U, sizeof(ehsm_session_st));
    memset(key, 0x0U, 512U);
    memset(msg, 0x0U, 512U);
    memset(mac, 0x0U, 512U);

    /* 拷贝数据 */
    key_size = std_data->std_key_size;
    memcpy(key, std_data->std_key, key_size);
    memcpy(msg, std_data->std_msg, std_data->std_msg_size);
    std_mac = std_data->std_mac;
    if (!gen_hmac) {
        /* 校验需要拷贝 mac 值 */
        memcpy(mac, std_data->std_mac, std_data->std_mac_size);
    }

    /* 打印标准密钥、标准 message、标准 mac 数据。 */
    print_hex("The std key is: \r\n    ", std_data->std_key, std_data->std_key_size);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std mac is: \r\n    ", std_data->std_mac, std_data->std_mac_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步，自动忽略回调函数。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用 init API 进行 hmac 计算的初始化，使用明文密钥。
     */
    ret = ehsm_hmac_init_with_plain_key(ctx, std_data->algo, key, key_size, gen_hmac, session);
    ret = demo_check_val("The execution of hmac init with plain key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 update API 进行更新，输入 ctx、明文消息、消息的字节长度，eHSM 读取明文消息并计算 mac。
         * 假设本次计算 10 个字节，累计共 10 字节。第一次调用 update API 之前，必须先调用一次 init API。
         * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        msg_size = 10U;
        ret = ehsm_hmac_update(ctx, msg, msg_size); /* 调用 update API 更新 */
        already_size = msg_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 3.2 第二次调用 update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算 70 个字节，累计共 70 + 10 = 80 字节。
         */
        msg_size = 70U;
        ret = ehsm_hmac_update(ctx, &msg[already_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        already_size += msg_size;
    }

    if (EHSM_OK == ret) {
        /** 3.3 第三次调用 update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算剩余的 16 个字节，累计共 70 + 10 + 16 = 96 字节。
         */
        msg_size = std_data->std_msg_size - already_size;
        ret = ehsm_hmac_update(ctx, &msg[already_size], msg_size); /* 调用 update API 更新 */
        ret = demo_check_val("The 3nd execution of finish API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        if (gen_hmac) {
            /** 4. 调用 finish API 计算并读取 mac 值，eHSM 会计算 mac 并回写 mac_size 长度的 mac 值到 SoC 提供的 mac
             * buffer。调用 finish API 前，需要确保所有明文消息已经 update，或者无明文消息。保存中间数据的 session
             * 指针已经在调用 init API 时保存在 ctx 中。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_finish_gen(ctx, mac, mac_size);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* eHSM FW 返回成功，需要校验生成的 mac 和返回的 mac size 是否正确 */
                ret = demo_check_data("Hmac value comparison", std_mac, mac_size, mac, mac_size);
            }
        } else { /* 校验 */
            /**
             * 4. 调用 finish API 校验 hmac，eHSM 会计算 hmac 并与输入的 hmac 值进行比较，比较结果会回写到
             * verify_result。调用 finish API 前，需要确保所有明文消息已经 update，或者无明文消息。保存中间数据的
             * session 指针已经在调用 init API 时保存在 ctx 中。
             */
            mac_size = std_data->std_mac_size; /* 指定生成的 mac buffer 的长度，具体为 [8, std_data->std_mac_size] */
            ret = ehsm_hmac_finish_verify(ctx, mac, mac_size, &verify_result);
            ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);
            if (EHSM_OK == ret) {
                /* 检查校验结果 */
                ret = demo_check_val("Verify mac:", true, verify_result);
            }
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Demo of HMAC-%s-%s stepwise with plain key ends with %s !!!] \r\n\r\n", std_data->alg_str,
        (gen_hmac ? "generation" : "verification"), ret_str);
}

/**
 * @brief hmac 生成和校验 mac 值的演示入口函数。包含不同 hash 算法生成和校验 mac 值的示例。
 */
void ehsm_demo_hmac_entry(void)
{
    const demo_hmac_std_st *std_data_arr = s_hmac_std_data_arr;
    uint32_t cnt = sizeof(s_hmac_std_data_arr) / sizeof(demo_hmac_std_st);
    uint32_t i;

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for hmac starts. ==================== "
                     "\r\n\r\n");

    /* TODO: 加入其他 hash 算法 */
    for (i = 0U; i < cnt; i++) {
        /* 对指定的 hash 算法，进行 Hmac 生成的一次性计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_cal_onepass(true, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 校验的一次性计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_cal_onepass(false, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 生成的分段计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_cal_stepwise(true, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 校验的分段计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_cal_stepwise(false, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 生成的一次性计算，使用明文密钥，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for onepass with plain key with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_onepass_with_plain_key(true, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 校验的一次性计算，使用明文密钥，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for onepass with plain key with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_onepass_with_plain_key(false, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 生成的分段计算，使用明文密钥，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for stepwise with plain key with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_stepwise_with_plain_key(true, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行 Hmac 校验的分段计算，使用明文密钥，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for stepwise with plain key with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hmac_stepwise_with_plain_key(false, &std_data_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for hamc ends. ==================== \r\n\r\n");
}

