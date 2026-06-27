#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_hash.h"
typedef struct {
    ehsm_hash_algo_e algo;
    const char *alg_str;
    const uint8_t *std_msg;
    uint32_t std_msg_size;
    const uint8_t *std_digest;
    uint32_t std_digest_size;
} demo_hash_std_st;

// clang-format off
static const uint8_t s_hash_std_msg[96] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f
};

static const uint8_t s_sha256_std_digest[32] = {
    0x08,0x35,0x9B,0x10,0x8F,0xA5,0x67,0xF5,0xDC,0xF3,0x19,0xFA,0x34,0x34,0xDA,0x6A,
    0xBB,0xC1,0xD5,0x95,0xF4,0x26,0x37,0x26,0x66,0x44,0x7F,0x09,0xCC,0x5A,0x87,0xDC
};

static const demo_hash_std_st s_hash_std_data_arr[1] = {
    {
        .algo = EHSM_HASH_ALGO_SHA256,
        .alg_str = "SHA256",
        .std_msg = s_hash_std_msg,
        .std_msg_size = sizeof(s_hash_std_msg),
        .std_digest = s_sha256_std_digest,
        .std_digest_size = 32U,
    }
};
// clang-format on

static volatile bool_t s_work_done = false;

/**
 * @brief 异步调用的 callback 函数。调用该函数后 s_work_done 会被置为 true。
 * @param[in] ctx eHSM context 地址，需事先调用过 @ref ehsm_ctx_init 初始化。
 */
static void ehsm_demo_callback(ehsm_ctx_st *ctx)
{
    (void)ctx;
    s_work_done = true;
}

/**
 * @brief 异步调用的 callback 函数。若调用了该函数，s_work_done 会被置为 true。
 * @param[in] ctx eHSM context 地址，需事先调用过 @ref ehsm_ctx_init 初始化。
 * @param[in] async 是否是异步。
 *          - true 指异步调用；
 *          - false 指同步调用；
 * @param[in] cb 回调函数，可以为 NULL。
 * @param[in] ret eHSM 返回的错误码。
 *
 * @note
 * - 驱动模式为 EHSM_DRV_MODE_INTERRUPT 且配置异步，则应该首先反复 POLL 并等待 eHSM 处理完成，即在 SoC
 * 的 Mailbox 中断处理函数中获取 eHSM 返回的错误码。若有回调函数，可以检查回调函数是否调用。
 * - 驱动模式为 EHSM_DRV_MODE_INTERRUPT 且配置同步，或者驱动模式为 EHSM_DRV_MODE_WAIT_AND_POLL 或
 * EHSM_DRV_MODE_SEND_AND_PEEK，则调用的 Host API 会完成相应的处理，无需反复 POLL，直接返回错误码即可。
 */
static uint32_t wait_work_done(ehsm_ctx_st *ctx, bool_t async, ehsm_rsp_cb_func_t cb, uint32_t ret)
{
    if (async) {
        /* 驱动模式为 EHSM_DRV_MODE_INTERRUPT 且配置异步，首先反复 POLL 并查询状态。 */
        while (ret == EHSM_ERR_NEED_POLL) {
            ret = ehsm_ctx_poll(ctx);
        }

        /* 若有回调函数，可以检查 Mailbox 中断是否调用回调函数，即 s_work_done 是否为 true。 */
        if (cb) {
            while (!s_work_done) { }
            s_work_done = false; /* 重置 s_work_done 的值为 false。 */
        }

        return ret;
    } else {
        /* 调用的 Host API 已经完成处理，这里直接返回错误码即可。*/
        return ret;
    }
}

/**
 * @brief 一次性计算 hash digest 的示例，使用 onepass API。
 * @param[in] async 是否是异步调用。
 *          - true 指异步调用；
 *          - false 指同步调用；
 * @param[in] cb 回调函数，仅当 async = true 时有效。
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hash_std_st。
 *
 * @note 输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 */
static void hash_gen_onepass(bool_t async, ehsm_rsp_cb_func_t cb, const demo_hash_std_st *std_data)
{
    uint32_t ret;
    uint8_t *msg = ehsm_demo_get_buffer(0);    /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                         /* 消息的总长度 */
    uint8_t *digest = ehsm_demo_get_buffer(1); /* 存储输出 digest 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t digest_size;                      /* 指定存储 digest 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();    /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint32_t std_digest_size = std_data->std_digest_size;
    const uint8_t *std_digest = NULL;

    /* 初始化数据 buffer */
    memset(msg, 0x0U, 512U);
    memset(digest, 0x0U, 512U);

    /* 打印 hash 算法、message、digest 数据。 */
    ehsm_port_printf("[Demo of %s onepass starts.] \r\n", std_data->alg_str);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std digest is: \r\n    ", std_data->std_digest, std_data->std_digest_size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, async, cb);

    /**
     * 2. 调用 hash onepass API 计算并获取 digest，eHSM 会计算 digest 并回写到 SoC 提供的 digest buffer，首先由 SoC
     * 输入 digest buffer 的长度，需要保证大于等于所选 hash 算法的标准 digest 长度，当 API 执行完毕，会返回实际的
     * digest size。
     */
    msg_size = std_data->std_msg_size;
    memcpy(msg, std_data->std_msg, msg_size);
    std_digest = std_data->std_digest;
    digest_size = 100U; /* digest buffer 的长度，必须大于等于选定 hash 算法的 digest 长度 */
    ret = ehsm_hash_onepass(ctx, std_data->algo, msg, msg_size, digest, &digest_size);
    ret = wait_work_done(ctx, async, cb, ret);
    ret = demo_check_val("The execution of onepass API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，需要校验生成的 digest 和返回的 digest size 是否正确 */
        ret = demo_check_data("Digest comparison", std_digest, std_digest_size, digest, digest_size);
    }

    if (ret) {
        ehsm_port_printf("[Demo of %s onepass ends with failure !!!] \r\n\r\n", std_data->alg_str);
    } else {
        ehsm_port_printf("[Demo of %s onepass ends with success !!!] \r\n\r\n", std_data->alg_str);
    }
}

/**
 * @brief 分段计算 hash digest 的示例函数，使用 init、update 和 finish APIs。
 * @param[in] async 是否是异步调用。
 *          - true 指异步调用；
 *          - false 指同步调用；
 * @param[in] cb 回调函数，仅当 async = true 时有效。
 * @param[in] std_data 使用的标准数据，详见 @ref demo_hash_std_st。
 *
 * @note
 * - 分段计算过程中，eHSM 支持多次输入消息更新 digest。该函数输入的消息为{0，1，2，...，95}，消息总长度为 96 字节。
 * 该函数调用 1 次 init、3 次 update 和 1 次 finish，3 次 update 过程的输入消息长度分别为 10，70，16 字节。
 * - 任意一次调用分段计算 API，如 init API，返回错误后，应终止本次 hash digest 计算，并忽略任何中间数据，如 ctx，session
 * 等。
 */
static void hash_gen_stepwise(bool_t async, ehsm_rsp_cb_func_t cb, const demo_hash_std_st *std_data)
{
    uint32_t ret;
    const char *ret_str = NULL;
    ehsm_session_st *session = ehsm_demo_get_session();   /* eHSM 分段计算的会话 buffer，用于存储 eHSM
                                                             FW 计算的中间数据，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t *msg = ehsm_demo_get_buffer(0);               /* 输入消息的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t msg_size;                                    /* 消息的总长度 */
    uint8_t *digest = ehsm_demo_get_buffer(1);            /* 存储输出 digest 的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t digest_size;                                 /* 指定存储 digest 的 buffer 长度 */
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();               /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint32_t std_digest_size = std_data->std_digest_size; /* 根据 hash 算法指定标准 digest 长度 */
    const uint8_t *std_digest = NULL;                     /* 标准 digetst 指针 */
    uint32_t hashed_size;                                 /* 已经 hash 的消息长度 */

    /* 初始化数据 buffer */
    memset(msg, 0x0U, 512U);
    memset(digest, 0x0U, 512U);
    memset(session, 0x0U, sizeof(ehsm_session_st));

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, async, cb);

    /* 打印 hash 算法、message、digest 数据。 */
    ehsm_port_printf("[Demo of %s stepwise starts.] \r\n", std_data->alg_str);
    print_hex("The std message is: \r\n    ", std_data->std_msg, std_data->std_msg_size);
    print_hex("The std digest is: \r\n    ", std_data->std_digest, std_data->std_digest_size);

    /* 将消息拷贝至 SoC 与 eHSM 的共享内存上，消息为{0，1，2，...，95}，消息总长 96 字节。 */
    memcpy(msg, std_data->std_msg, std_data->std_msg_size);
    std_digest = std_data->std_digest;

    /* 2. 调用 init API 进行 hash 算法的初始化，输入 ctx、hash 算法、eHSM 初始化分段计算的 session。*/
    ret = ehsm_hash_init(ctx, std_data->algo, session);
    ret = wait_work_done(ctx, async, cb, ret);
    ret = demo_check_val("The execution of init API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3.1 第一次调用 hash update API 进行更新，输入 ctx、明文消息、消息的字节长度，eHSM 读取明文消息并计算 digest。
         * 假设本次计算 10 个字节，累计共 10 字节。第一次调用 hash update API 之前，必须先调用一次 init API。
         * 保存中间数据的 session 指针已经在调用 init API 时保存在 ctx 中。
         */
        msg_size = 10U;
        ret = ehsm_hash_update(ctx, msg, msg_size); /* 调用 update API 更新 */
        ret = wait_work_done(ctx, async, cb, ret);
        hashed_size = msg_size;
        ret = demo_check_val("The 1st execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /** 3.2 第二次调用 hash update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算 70 个字节，累计共 70 + 10 = 80 字节。
         */
        msg_size = 70U;
        ret = ehsm_hash_update(ctx, &msg[hashed_size], msg_size); /* 调用 update API 更新 */
        ret = wait_work_done(ctx, async, cb, ret);
        ret = demo_check_val("The 2nd execution of update API ", EHSM_OK, ret);
        hashed_size += msg_size;
    }

    if (EHSM_OK == ret) {
        /**
         * 3.3 第三次调用 hash update API 更新，eHSM 支持多次更新，每次更新 eHSM 都会基于上一次更新返回的 session
         * 接着计算，假设本次计算剩余的 16 个字节，累计共 70 + 10 + 16 = 96 字节。
         */
        msg_size = std_data->std_msg_size - hashed_size;
        ret = ehsm_hash_update(ctx, &msg[hashed_size], msg_size); /* 调用 update API 更新 */
        ret = wait_work_done(ctx, async, cb, ret);
        ret = demo_check_val("The 3rd execution of update API ", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /**
         * 4. 调用 hash finish API 读取 digest，eHSM 会计算 digest 并回写到 SoC 提供的 digest buffer，首先由 SoC 输入
         * digest buffer 的长度，需要保证大于等于 INIT 时所选 hash 算法的标准 digest 长度，当 API 执行完毕，会返回实际的
         * digest size。调用 finish API 前，需要确保所有明文消息已经 update，或者无明文消息。保存中间数据的 session
         * 指针已经在调用 init API 时保存在 ctx 中。
         */
        digest_size = 100U; /* digest buffer 的长度，必须大于等于选定 hash 算法的 digest 长度 */
        ret = ehsm_hash_finish(ctx, digest, &digest_size); /* 调用 finish API 获取 digest 和实际的 digest size */
        ret = wait_work_done(ctx, async, cb, ret);
        ret = demo_check_val("The execution of finish API:", EHSM_OK, ret);

        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，需要校验生成的 digest 和返回的 digest size 是否正确 */
            ret = demo_check_data("Digest comparison", std_digest, std_digest_size, digest, digest_size);
        }
    }

    ret_str = (EHSM_OK == ret) ? "SUCCESS" : "FAILURE";
    ehsm_port_printf("[Ends with %s !!!] \r\n\r\n", ret_str);
}

/**
 * @brief Hash 算法。
 *
 * @param[in] drv_mode 驱动模式，详见 @ref ehsm_drv_mode_e。
 * @param[in] async 是否是异步调用。
 *          - true 指异步调用；
 *          - false 指同步调用；
 * @param[in] cb 回调函数，仅当 drv_mode = EHSM_DRV_MODE_INTERRUPT 且 async = true 时有效。
 */
static void hash_gen(ehsm_drv_mode_e drv_mode, bool_t async, bool_t is_cb)
{
    const demo_hash_std_st *std_data_arr = s_hash_std_data_arr;
    uint32_t cnt = sizeof(s_hash_std_data_arr) / sizeof(demo_hash_std_st);
    uint32_t i;
    ehsm_rsp_cb_func_t cb = NULL;
    static uint8_t s_drv_mode_arr[3][32] = {
        "    INTERRUPT",
        "WAIT_AND_POLL",
        "SEND_AND_PEEK",
    };

    /* 修改驱动模式。 */
    if (ehsm_driver_init_library(drv_mode) != EHSM_OK) {
        ehsm_port_printf("library init failed! \r\n");
        return;
    } else {
        ehsm_port_printf("[Driver mode      : %s.] \r\n", s_drv_mode_arr[drv_mode]);
    }

    if (async) {
        ehsm_port_printf("[Invocation Mode  : Asynchronous. ] \r\n");
    } else {
        ehsm_port_printf("[Invocation Mode  :  Synchronous. ] \r\n");
    }

    if (is_cb) {
        /* 指定 callback 函数。*/
        cb = ehsm_demo_callback;
        ehsm_port_printf("[Callback function: Valid.        ] \r\n");
    } else {
        ehsm_port_printf("[Callback function:  NULL.        ] \r\n");
    }

    for (i = 0U; i < cnt; i++) {
        /* 对指定的 hash 算法，进行哈希摘要生成的一次性计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for onepass with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hash_gen_onepass(async, cb, &std_data_arr[i]);

        /* 对指定的 hash 算法，进行哈希摘要生成的分段计算，msg = {0，1，2，..., 95}，msg_size = 96。 */
        ehsm_port_printf("[%s for stepwise with std_data[%d] starts.] \r\n", std_data_arr[i].alg_str, i);
        hash_gen_stepwise(async, cb, &std_data_arr[i]);
    }
}

/**
 * @brief hash 算法演示入口函数。包含不同的驱动模式下的 hash 算法示例。
 */
void ehsm_demo_hash_entry(void)
{
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for hash starts. ==================== "
                     "\r\n\r\n");

    /* 驱动模式：中断模式，配置同步，忽略回调函数（不起作用）。 */
    hash_gen(EHSM_DRV_MODE_INTERRUPT, false, true);
    /* 驱动模式：中断模式，配置异步，且有回调函数。 */
    hash_gen(EHSM_DRV_MODE_INTERRUPT, true, true);
    /* 驱动模式：中断模式，配置异步，且无回调函数。 */
    hash_gen(EHSM_DRV_MODE_INTERRUPT, true, false);

    /* 驱动模式：同步等待模式，忽略异步和回调函数（不起作用）。 */
    hash_gen(EHSM_DRV_MODE_WAIT_AND_POLL, true, true);

    /* 驱动模式：发送并由用户 POLL，忽略异步和回调函数（不起作用）。 */
    hash_gen(EHSM_DRV_MODE_SEND_AND_PEEK, true, true);

    /* 恢复驱动模式为：同步等待模式。 */
    if (ehsm_driver_init_library(EHSM_DRV_MODE_WAIT_AND_POLL) != EHSM_OK) {
        ehsm_port_printf("library init failed! \r\n");
        return;
    } else {
        ehsm_port_printf("[Driver mode: EHSM_DRV_MODE_WAIT_AND_POLL.] \r\n");
    }

    ehsm_port_printf("\r\n==================== eHSM demo for hash ends. ==================== \r\n\r\n");
}

