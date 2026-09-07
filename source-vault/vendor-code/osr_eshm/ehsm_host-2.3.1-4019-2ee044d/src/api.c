#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/bl_api.h"
#include "ehsmdrv/basic/mailbox.h"
#include "ehsmdrv/basic/types.h"
#include "ehsmdrv/basic/version.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

#include <stddef.h>
#include <string.h>

#include "mb.h"
#include "bl_mb.h"
#include "types_internal.h"

#define EHSM_RET_CODE_OK  (0x0000A55AU)
#define EHSM_GMAC_IV_SIZE 12U

STATIC_ASSERT(sizeof(ehsm_ctx_st) >= sizeof(ehsm_ctx_intl_st));

static inline uint32_t ehsm_ret_code_remap(uint32_t ret_code)
{
    if (ret_code == EHSM_RET_CODE_OK) {
        return EHSM_OK;
    } else {
        return EHSM_ERR_FW_BASE | ret_code;
    }
}

static inline uint32_t ehsm_wait_until_responded(ehsm_ctx_intl_st *ctx_intl)
{
    volatile bool_t *responded = &ctx_intl->responded;
    ehsm_port_timer_t t = ehsm_port_create_timer();
    while (!*responded) {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        if (ehsm_port_is_timeout(t)) {
            return EHSM_ERR_TIMEOUT;
        }
    }
    ehsm_port_memory_barrier_read();
    return EHSM_OK;
}

// mailbox中断响应函数
static void ehsm_mb_int(uint32_t channel, raddr_t packet_addr)
{
    (void)channel;
    // 从 packet 成员地址计算出 ctx 的地址
    uint32_t packet_offset = offsetof(ehsm_ctx_intl_st, packet);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ehsm_port_raddr_to_addr(packet_addr - packet_offset);
    if (ctx_intl->magic != EHSM_CTX_MAGIC) {
        return;
    }

    switch (ctx_intl->cmd_buf.cmd_id) {
    case MB_CMD_ID_HASH:
    case MB_CMD_ID_HMAC:
    case MB_CMD_ID_SYMM_CIPHER:
    case MB_CMD_ID_MAC:
    case MB_CMD_ID_RSA_SIGN:
    case MB_CMD_ID_SM2_SIGN:
    case MB_CMD_ID_ECDSA:
    case MB_CMD_ID_AEAD_CCM:
    case MB_CMD_ID_AEAD_GCM:
    case MB_CMD_ID_RSA_CIPHER:
    case MB_CMD_ID_SM2_CIPHER:
    case MB_CMD_ID_EHSM_IMPORT_KEY:
    case MB_CMD_ID_EHSM_GEN_KEY:
    case MB_CMD_ID_CHACHA_CIPHER:
    case MB_CMD_ID_SM9_SIGN:
    case MB_CMD_ID_SM9_CIPHER:
        if (ctx_intl->result2 != NULL) {
            // 部分命令的结果长度要写回传入的 result2_len 指向的内存
            uint32_t size = ctx_intl->rsp_buf.data[0];
            *((uint32_t *)ctx_intl->result2) = size;
        }
        if (ctx_intl->result1 != NULL) {
            // 部分命令验证结果写回到入参 verify_result 指向的内存
            bool_t result = ctx_intl->rsp_buf.data[1] == 0;
            *((bool_t *)ctx_intl->result1) = result;
        }
        break;
    case MB_CMD_ID_EHSM_EXPORT_KEY:
        if (ctx_intl->result1 != NULL) {
            // key_data_size
            *((uint32_t *)ctx_intl->result1) = ctx_intl->rsp_buf.data[0];
        }
        if (ctx_intl->result2 != NULL) {
            // mac_size
            *((uint32_t *)ctx_intl->result2) = ctx_intl->rsp_buf.data[1];
        }
        break;
    case MB_CMD_ID_EHSM_KEY_DERIVE:
    case MB_CMD_ID_EHSM_KEY_EXCHANGE:
    case MB_CMD_ID_EHSM_SM9_EXCHG_KEY:
        if (ctx_intl->result1 != NULL) {
            // key_handle
            *((uint32_t *)ctx_intl->result1) = ctx_intl->rsp_buf.data[0];
        }
        break;
    case MB_CMD_ID_EHSM_GET_PUB_FROM_PRIV:
        if (ctx_intl->result1 != NULL) {
            // key_type
            *((ehsm_key_type_e *)ctx_intl->result1) = (ehsm_key_type_e)ctx_intl->rsp_buf.data[0];
        }
        if (ctx_intl->result2 != NULL) {
            // pub_key_size
            *((uint32_t *)ctx_intl->result2) = ctx_intl->rsp_buf.data[1];
        }
        break;
    case MB_CMD_ID_GET_UTC_TIME:
        if (ctx_intl->result1 != NULL) {
            //  utc_time
            *((uint32_t *)ctx_intl->result1) = ctx_intl->rsp_buf.data[0];
        }
        break;
    case MB_CMD_ID_CREATE_COUNTER:
    case MB_CMD_ID_READ_COUNTER:
    case MB_CMD_ID_INCREASE_COUNTER_VALUE:
        if (ctx_intl->result1 != NULL) {
            //  counter_value
            uint32_t lvalue = ctx_intl->rsp_buf.data[0];
            uint32_t hvalue = ctx_intl->rsp_buf.data[1];
            *((uint64_t *)ctx_intl->result1) = ((uint64_t)hvalue << 32) | (uint64_t)lvalue;
        }
        if (ctx_intl->result2 != NULL) {
            //  counter_id
            *((uint32_t *)ctx_intl->result2) = ctx_intl->rsp_buf.data[2];
        }
        break;

    default:
        break;
    }

    // 设置 ctx 的 responded 状态，并调用回调函数
    ehsm_port_memory_barrier_write();
    ctx_intl->responded = true;
    if (ctx_intl->callback) {
        ctx_intl->callback((ehsm_ctx_st *)ctx_intl);
    }
}

static uint32_t ehsm_send_cmd(ehsm_ctx_intl_st *ctx_intl)
{
    uint32_t ret = ehsm_mb_send(ctx_intl->mb_ch, ehsm_port_addr_to_raddr(&ctx_intl->packet));
    if (ret != EHSM_OK) {
        return ret;
    }

    switch (get_driver_mode()) {
    case EHSM_DRV_MODE_INTERRUPT:
        if (!ctx_intl->async) {
            ret = ehsm_wait_until_responded(ctx_intl);
            if (ret != EHSM_OK) {
                return ret;
            }

            return ehsm_ret_code_remap(ctx_intl->rsp_buf.ret_code);
        } else {
            return EHSM_ERR_NEED_POLL;
        }
    case EHSM_DRV_MODE_WAIT_AND_POLL:
        // 此模式忽略ctx的async标志，总是POLL直到响应
        ret = EHSM_ERR_NEED_POLL;
        ehsm_port_timer_t t = ehsm_port_create_timer();
        while (ret == EHSM_ERR_NEED_POLL) {
            if (ehsm_port_is_timeout(t)) {
                return EHSM_ERR_TIMEOUT;
            }

            ret = ehsm_mb_poll(ctx_intl->mb_ch);
        }
        if (ret == EHSM_OK) {
            return ehsm_ret_code_remap(ctx_intl->rsp_buf.ret_code);
        } else {
            return ret;
        }
    case EHSM_DRV_MODE_SEND_AND_PEEK:
        // 此模式忽略ctx的async标志，总是要求再调用 ehsm_ctx_poll来检查响应
        return EHSM_ERR_NEED_POLL;
    default:
        return EHSM_ERR_ILLEGAL_USE;
    }
}

// 复位ctx状态，可以再次调用其它功能，三段式中的update和finish不可调用此函数，因为cmd_buf中缓存了一些数据（例如session地址），其它命令都应该调用此函数
static inline void ehsm_ctx_reset(ehsm_ctx_st *ctx)
{
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    memset(&ctx_intl->result1, 0, sizeof(ehsm_ctx_intl_st) - offsetof(ehsm_ctx_intl_st, result1));
}

// 每个mailbox命令API在调用前应当对ctx进行检查
static inline uint32_t ehsm_check_ctx(ehsm_ctx_st *ctx)
{
    if (ctx == NULL) {
        return EHSM_ERR_PARAM_ERROR;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    if (ctx_intl->magic != EHSM_CTX_MAGIC) {
        return EHSM_ERR_CTX_INVALID;
    }
    if (!ctx_intl->inited) {
        return EHSM_ERR_CTX_NOT_INIT;
    }
    return EHSM_OK;
}

static inline void ehsm_set_cmd_id(ehsm_mb_cmd_st *cmd, uint16_t id)
{
    cmd->cmd_id = id;
    cmd->cmd_id_inv = ~id;
}

static ehsm_drv_mode_e s_drv_mode = EHSM_DRV_MODE_SEND_AND_PEEK;

ehsm_drv_mode_e get_driver_mode(void)
{
    return s_drv_mode;
}

// ---------- API impl --------------------//

uint32_t ehsm_driver_init_library(ehsm_drv_mode_e drv_mode)
{
    switch (drv_mode) {
    case EHSM_DRV_MODE_INTERRUPT:
    case EHSM_DRV_MODE_WAIT_AND_POLL:
    case EHSM_DRV_MODE_SEND_AND_PEEK:
        break;
    default:
        return EHSM_ERR_PARAM_ERROR;
    }

    s_drv_mode = drv_mode;
    uint32_t ret = ehsm_port_init(drv_mode);
    if (ret == EHSM_OK) {
        ret = ehsm_mb_init(ehsm_mb_int, drv_mode);
    }
    return ret;
}

uint32_t ehsm_driver_get_version(void)
{
    return (EHSM_DRV_VERSION_MAJOR << 16) | (EHSM_DRV_VERSION_MINOR << 8) | EHSM_DRV_VERSION_PATCH;
}

// 初始化内部ctx结构体
void ehsm_ctx_init(ehsm_ctx_st *ctx, uint8_t mb_ch, bool_t async, ehsm_rsp_cb_func_t cb)
{
    if (ctx == NULL || mb_ch >= EHSM_PORT_MAILBOX_CHANNEL_COUNT) {
        return;
    }
    memset(ctx, 0, sizeof(ehsm_ctx_st));
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->magic = EHSM_CTX_MAGIC;
    ctx_intl->mb_ch = mb_ch;
    ctx_intl->async = async;
    ctx_intl->callback = cb;

    ctx_intl->inited = true;
    ctx_intl->packet.cmd_addr = ehsm_port_addr_to_raddr(&ctx_intl->cmd_buf);
    ctx_intl->packet.rsp_addr = ehsm_port_addr_to_raddr(&ctx_intl->rsp_buf);
}

uint32_t ehsm_ctx_poll(ehsm_ctx_st *ctx)
{
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    switch (get_driver_mode()) {
    case EHSM_DRV_MODE_INTERRUPT:
        if (ctx_intl->responded) {
            return ehsm_ret_code_remap(ctx_intl->rsp_buf.ret_code);
        } else {
            return EHSM_ERR_NEED_POLL;
        }
    case EHSM_DRV_MODE_SEND_AND_PEEK: {
        uint32_t ret = ehsm_mb_poll(ctx_intl->mb_ch);
        switch (ret) {
        case EHSM_OK:
            return ehsm_ret_code_remap(ctx_intl->rsp_buf.ret_code);
        default:
            return ret;
        }
    }
    default:
        return EHSM_ERR_ILLEGAL_USE;
    }
}

uint32_t ehsm_gen_random(ehsm_ctx_st *ctx, ehsm_rng_algo_e algo, uint8_t *rand_buf, uint32_t rand_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_rng_st *cmd = (mb_cmd_rng_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RNG);

    cmd->algorithm = (uint8_t)algo;
    cmd->random_data_addr = ehsm_port_addr_to_raddr(rand_buf);
    cmd->require_size = rand_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hash_init(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_hash_st *cmd = (mb_cmd_hash_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HASH);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_START;
    cmd->hash_ctx = ehsm_port_addr_to_raddr(session);
    cmd->hash_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hash_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_hash_st *cmd = (mb_cmd_hash_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_HASH || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->hash_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;

    // 其它值如HASH算法类型、hash_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hash_finish(ehsm_ctx_st *ctx, uint8_t *digest, uint32_t *digest_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_hash_st *cmd = (mb_cmd_hash_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_HASH || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->hash_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->result2 = (void *)digest_size;
    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->data = 0;
    cmd->data_size = 0;

    cmd->digest = ehsm_port_addr_to_raddr(digest);
    if (digest_size) {
        cmd->digest_size = *digest_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hash_onepass(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, const uint8_t *msg, uint32_t msg_size,
    uint8_t *digest, uint32_t *digest_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = digest_size;

    mb_cmd_hash_st *cmd = (mb_cmd_hash_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HASH);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->digest = ehsm_port_addr_to_raddr(digest);
    if (digest_size) {
        cmd->digest_size = *digest_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_init(
    ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_hmac, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = (uint8_t)MB_START;
    cmd->direction = (uint8_t)(gen_hmac ? MB_MAC_GEN : MB_MAC_VRY);
    cmd->key_handle = key_handle;
    cmd->hmac_ctx = ehsm_port_addr_to_raddr(session);
    cmd->hmac_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_HMAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->hmac_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;

    // 其它值如HASH算法类型、key_handle、hmac_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_finish_gen(ehsm_ctx_st *ctx, uint8_t *hmac, uint32_t hmac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_HMAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->hmac_ctx == 0 || cmd->direction != MB_MAC_GEN) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->data = 0;
    cmd->data_size = 0;

    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_finish_verify(ehsm_ctx_st *ctx, const uint8_t *hmac, uint32_t hmac_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_HMAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->hmac_ctx == 0 || cmd->direction != MB_MAC_VRY) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;
    ctx_intl->result1 = (void *)verify_result;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->data = 0;
    cmd->data_size = 0;

    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_onepass_gen(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, const uint8_t *msg,
    uint32_t msg_size, uint8_t *hmac, uint32_t hmac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_GEN;
    cmd->key_handle = key_handle;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_onepass_verify(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, const uint8_t *msg,
    uint32_t msg_size, const uint8_t *hmac, uint32_t hmac_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)verify_result;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_VRY;
    cmd->key_handle = key_handle;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    uint32_t key_size, bool_t gen_hmac, EHSM_SHM ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = (uint8_t)MB_START;
    cmd->direction = (uint8_t)(gen_hmac ? MB_MAC_GEN : MB_MAC_VRY);
    cmd->hmac_ctx = ehsm_port_addr_to_raddr(session);
    cmd->hmac_ctx_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_onepass_gen_with_plain_key(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    uint32_t key_size, const uint8_t *msg, uint32_t msg_size, uint8_t *hmac, uint32_t hmac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_GEN;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_hmac_onepass_verify_with_plain_key(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    uint32_t key_size, const uint8_t *msg, uint32_t msg_size, const uint8_t *hmac, uint32_t hmac_size,
    bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)verify_result;

    mb_cmd_hmac_st *cmd = (mb_cmd_hmac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_HMAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_VRY;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->digest = ehsm_port_addr_to_raddr(hmac);
    cmd->digest_size = hmac_size;
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_init(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode, uint32_t key_handle,
    bool_t gen_mac, const uint8_t *iv, uint32_t iv_size, uint32_t mac_size, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->process_mode = MB_START;
    cmd->key_handle = key_handle;
    cmd->direction = (uint8_t)(gen_mac ? MB_MAC_GEN : MB_MAC_VRY);
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }

    // Mac的长度需要在init阶段传入
    cmd->mac_size = mac_size;
    cmd->mac_ctx = ehsm_port_addr_to_raddr(session);
    cmd->mac_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_MAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->mac_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;

    // 其它值如MAC算法类型、算法模式、key_handle、mac_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_finish_gen(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size, uint8_t *mac)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_MAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->mac_ctx == 0 || cmd->direction != MB_MAC_GEN) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;

    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_finish_verify(
    ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size, const uint8_t *mac, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_MAC || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->mac_ctx == 0 || cmd->direction != MB_MAC_VRY) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;
    cmd->process_mode = MB_FINISH;

    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_onepass_gen(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode, uint32_t key_handle,
    const uint8_t *iv, uint32_t iv_size, const uint8_t *msg, uint32_t msg_size, uint8_t *mac, uint32_t mac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_GEN;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->key_handle = key_handle;
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);
    cmd->mac_size = mac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_onepass_verify(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode, uint32_t key_handle,
    const uint8_t *iv, uint32_t iv_size, const uint8_t *msg, uint32_t msg_size, const uint8_t *mac, uint32_t mac_size,
    bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)verify_result;

    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_VRY;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->key_handle = key_handle;
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);
    cmd->mac_size = mac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_init_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t gen_mac, const uint8_t *iv, uint32_t iv_size,
    uint32_t mac_size, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->process_mode = MB_START;
    cmd->direction = (uint8_t)(gen_mac ? MB_MAC_GEN : MB_MAC_VRY);
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }

    // Mac的长度需要在init阶段传入
    cmd->mac_size = mac_size;
    cmd->mac_ctx = ehsm_port_addr_to_raddr(session);
    cmd->mac_ctx_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_onepass_gen_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, const uint8_t *iv, uint32_t iv_size, const uint8_t *msg,
    uint32_t msg_size, uint8_t *mac, uint32_t mac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_GEN;
    cmd->cipher_mode = (uint8_t)mode;
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);
    cmd->mac_size = mac_size;
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_mac_onepass_verify_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_mac_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, const uint8_t *iv, uint32_t iv_size, const uint8_t *msg,
    uint32_t msg_size, const uint8_t *mac, uint32_t mac_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)verify_result;

    mb_cmd_mac_st *cmd = (mb_cmd_mac_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_MAC);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->algorithm = (uint8_t)algo;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = MB_MAC_VRY;
    cmd->cipher_mode = (uint8_t)mode;
    // 仅有GMAC需要传入iv，且iv长度只支持12字节
    if (mode == EHSM_MAC_MODE_GMAC) {
        if (iv_size == EHSM_GMAC_IV_SIZE) {
            cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        } else {
            return EHSM_ERR_PARAM_ERROR;
        }
    }
    cmd->data = ehsm_port_addr_to_raddr(msg);
    cmd->data_size = msg_size;
    cmd->mac = ehsm_port_addr_to_raddr(mac);
    cmd->mac_size = mac_size;
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_init(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, uint32_t key_handle, bool_t enc, const uint8_t *iv, uint32_t iv_size,
    ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SYMM_CIPHER);
    cmd->process_mode = MB_START;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->padding = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->key_handle = key_handle;
    // ECB模式不需要iv
    if (mode != EHSM_CIPHER_MODE_ECB) {
        cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        cmd->iv_size = iv_size;
    }
    cmd->context = ehsm_port_addr_to_raddr(session);
    cmd->context_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_update(ehsm_ctx_st *ctx, const uint8_t *input, uint32_t input_size, uint8_t *output)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_SYMM_CIPHER || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->context == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;
    cmd->process_mode = MB_UPDATE;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    // 此处需注意output的buffer长度不小于input_size
    cmd->output_size = input_size;

    // 其它值如算法类型、算法模式、direction, key_handle、context地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_finish(
    ehsm_ctx_st *ctx, const uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_SYMM_CIPHER || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->context == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;
    ctx_intl->result2 = (void *)output_size;

    cmd->process_mode = MB_FINISH;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_onepass(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, uint32_t key_handle, bool_t enc, const uint8_t *iv, uint32_t iv_size,
    const uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SYMM_CIPHER);
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->padding = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->key_handle = key_handle;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    // ECB模式不需要iv
    if (mode != EHSM_CIPHER_MODE_ECB) {
        cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        cmd->iv_size = iv_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_init_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc, const uint8_t *iv,
    uint32_t iv_size, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SYMM_CIPHER);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->process_mode = MB_START;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->padding = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    // ECB模式不需要iv
    if (mode != EHSM_CIPHER_MODE_ECB) {
        cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        cmd->iv_size = iv_size;
    }
    cmd->context = ehsm_port_addr_to_raddr(session);
    cmd->context_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_symm_cipher_onepass_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_cipher_mode_e mode,
    ehsm_padding_mode_e padding, EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc, const uint8_t *iv,
    uint32_t iv_size, const uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_symm_cipher_st *cmd = (mb_cmd_symm_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SYMM_CIPHER);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->padding = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->cipher_mode = (uint8_t)mode;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    // ECB模式不需要iv
    if (mode != EHSM_CIPHER_MODE_ECB) {
        cmd->iv_addr = ehsm_port_addr_to_raddr(iv);
        cmd->iv_size = iv_size;
    }
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_cipher(ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, const uint8_t *input, uint32_t input_size,
    uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_sm2_cipher_st *cmd = (mb_cmd_sm2_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM2_CIPHER);
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->key_handle = key_handle;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_init(ehsm_ctx_st *ctx, uint32_t key_handle, bool_t gen_sig, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM2_SIGN);
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->key_handle = key_handle;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_SM2_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;

    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;

    // 其它值如direction、key_handle、sign_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_finish_gen(ehsm_ctx_st *ctx, uint8_t *sig, uint32_t sig_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_SM2_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_GEN) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_finish_verify(ehsm_ctx_st *ctx, const uint8_t *sig, uint32_t sig_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_SM2_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_VRY) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;
    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, bool_t use_plain_key, uint32_t key_handle,
    EHSM_SHM const uint8_t *key, bool_t gen_sig, bool_t is_digest, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uintptr_t sig_addr, uint32_t sig_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;

    if (!gen_sig) {
        ctx_intl->result1 = (void *)verify_result;
    }

    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM2_SIGN);
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = gen_sig ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->key_type = use_plain_key ? MB_PLAIN_KEY : MB_KEY_HANDLE;
    if (use_plain_key) {
        cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    } else {
        cmd->key_handle = key_handle;
    }
    cmd->msg_type = is_digest ? MB_MSG_TYPE_DIGEST : MB_MSG_TYPE_MESSAGE;
    cmd->msg_addr = ehsm_port_addr_to_raddr(input);
    cmd->msg_size = input_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr((const void *)sig_addr);
    cmd->sign_size = sig_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_cipher_with_plain_key(ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc, const uint8_t *input,
    uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_sm2_cipher_st *cmd = (mb_cmd_sm2_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM2_CIPHER);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm2_sign_init_with_plain_key(
    ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t gen_sig, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_sm2_sign_st *cmd = (mb_cmd_sm2_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM2_SIGN);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_cipher(ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, const uint8_t *input, uint32_t input_size,
    uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_rsa_cipher_st *cmd = (mb_cmd_rsa_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RSA_CIPHER);
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->key_handle = key_handle;
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_init(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_sig,
    ehsm_rsa_padding_mode_e padding, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RSA_SIGN);
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->rsa_padding_type = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->key_handle = key_handle;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_RSA_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;

    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;

    // 其它值如direction、rsa_padding_type、algorithm、key_handle、sign_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_finish_gen(ehsm_ctx_st *ctx, uint8_t *sig, uint32_t *sig_size, uint32_t salt_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_RSA_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_GEN) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;
    ctx_intl->result2 = (void *)sig_size;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    if (sig_size) {
        cmd->sign_size = *sig_size;
    }
    cmd->salt_size = salt_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_finish_verify(
    ehsm_ctx_st *ctx, const uint8_t *sig, uint32_t sig_size, uint32_t salt_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_RSA_SIGN || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_VRY) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;
    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;
    cmd->salt_size = salt_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, bool_t use_plain_key,
    uint32_t key_handle, EHSM_SHM const uint8_t *key, ehsm_rsa_padding_mode_e padding, bool_t gen_sig, bool_t is_digest,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uintptr_t sig_addr, uint32_t *sig_size,
    uint32_t salt_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    if (gen_sig) {
        ctx_intl->result2 = (void *)sig_size;
    } else {
        ctx_intl->result1 = (void *)verify_result;
    }

    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RSA_SIGN);
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = gen_sig ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->rsa_padding_type = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->key_type = use_plain_key ? MB_PLAIN_KEY : MB_KEY_HANDLE;
    if (use_plain_key) {
        cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    } else {
        cmd->key_handle = key_handle;
    }
    cmd->msg_type = is_digest ? MB_MSG_TYPE_DIGEST : MB_MSG_TYPE_MESSAGE;
    cmd->msg_addr = ehsm_port_addr_to_raddr(input);
    cmd->msg_size = input_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr((const void *)sig_addr);
    if (sig_size) {
        cmd->sign_size = *sig_size;
    }
    cmd->salt_size = salt_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_cipher_with_plain_key(ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc, const uint8_t *input,
    uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_rsa_cipher_st *cmd = (mb_cmd_rsa_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RSA_CIPHER);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_rsa_sign_init_with_plain_key(ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key,
    bool_t gen_sig, ehsm_rsa_padding_mode_e padding, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_rsa_sign_st *cmd = (mb_cmd_rsa_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_RSA_SIGN);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->rsa_padding_type = (uint8_t)padding;
    cmd->algorithm = (uint8_t)algo;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_init(
    ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, uint32_t key_handle, bool_t gen_sig, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_ECDSA);
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->algorithm = (uint8_t)algo;
    cmd->key_handle = key_handle;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_update(ehsm_ctx_st *ctx, const uint8_t *msg, uint32_t msg_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_ECDSA || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;

    cmd->process_mode = MB_UPDATE;

    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;

    // 其它值如direction、algorithm、key_handle、sign_ctx地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_finish_gen(ehsm_ctx_st *ctx, uint8_t *sig, uint32_t *sig_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_ECDSA || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_GEN) {
        return EHSM_ERR_CTX_STATUS;
    }

    ctx_intl->responded = false;
    ctx_intl->result2 = (void *)sig_size;

    cmd->process_mode = MB_FINISH;

    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    if (sig_size) {
        cmd->sign_size = *sig_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_finish_verify(ehsm_ctx_st *ctx, const uint8_t *sig, uint32_t sig_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    if (cmd->cmd_id != MB_CMD_ID_ECDSA || (cmd->process_mode != MB_START && cmd->process_mode != MB_UPDATE)
        || cmd->sign_ctx == 0 || cmd->direction != MB_SIG_VRY) {
        return EHSM_ERR_CTX_STATUS;
    }
    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;

    cmd->process_mode = MB_FINISH;
    // 注意这里要把之前update时设置的值清掉
    cmd->msg_addr = 0;
    cmd->msg_size = 0;

    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_onepass_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, bool_t use_plain_key,
    uint32_t key_handle, EHSM_SHM const uint8_t *key, bool_t gen_sig, bool_t is_digest, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uintptr_t sig_addr, uint32_t *sig_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    if (gen_sig) {
        ctx_intl->result2 = (void *)sig_size;
    } else {
        ctx_intl->result1 = (void *)verify_result;
    }

    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_ECDSA);
    cmd->process_mode = MB_ONE_PASS;
    cmd->direction = gen_sig ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->algorithm = (uint8_t)algo;
    cmd->key_type = use_plain_key ? MB_PLAIN_KEY : MB_KEY_HANDLE;
    if (use_plain_key) {
        cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    } else {
        cmd->key_handle = key_handle;
    }

    cmd->msg_type = is_digest ? MB_MSG_TYPE_DIGEST : MB_MSG_TYPE_MESSAGE;
    cmd->msg_addr = ehsm_port_addr_to_raddr(input);
    cmd->msg_size = input_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr((const void *)sig_addr);
    if (sig_size) {
        cmd->sign_size = *sig_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_ecdsa_init_with_plain_key(
    ehsm_ctx_st *ctx, ehsm_hash_algo_e algo, EHSM_SHM const uint8_t *key, bool_t gen_sig, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_ecdsa_st *cmd = (mb_cmd_ecdsa_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_ECDSA);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->process_mode = MB_START;
    cmd->direction = (gen_sig) ? MB_SIG_GEN : MB_SIG_VRY;
    cmd->algorithm = (uint8_t)algo;
    cmd->sign_ctx = ehsm_port_addr_to_raddr(session);
    cmd->sign_ctx_size = sizeof(ehsm_session_st);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm9_cipher(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, ehsm_sm9_enc_type_e enc_type,
    ehsm_sm9_padding_mode_e padding, uint8_t key2_size, uint8_t hid, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, uint32_t *output_size,
    EHSM_SHM const uint8_t *id, uint32_t id_size, EHSM_SHM const uint8_t *fp12g)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_sm9_cipher_st *cmd = (mb_cmd_sm9_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM9_CIPHER);
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->enc_type = (uint8_t)enc_type;
    cmd->padding_type = (uint8_t)padding;
    cmd->key2_size = key2_size;
    cmd->hid = hid;
    cmd->key_handle = key_handle;
    cmd->kgc_pub_key = ehsm_port_addr_to_raddr(kgc_pub_key);
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    cmd->id_addr = ehsm_port_addr_to_raddr(id);
    cmd->id_size = id_size;
    cmd->fp12g = ehsm_port_addr_to_raddr(fp12g);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm9_sign_onepass_gen(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *msg,
    uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *fp12g)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_sm9_sign_st *cmd = (mb_cmd_sm9_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM9_SIGN);
    cmd->direction = MB_SIG_GEN;
    cmd->key_handle = key_handle;
    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;
    cmd->hid = 0;
    cmd->kgc_pub_key = ehsm_port_addr_to_raddr(kgc_pub_key);
    cmd->fp12g = ehsm_port_addr_to_raddr(fp12g);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm9_sign_onepass_verify(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *msg, uint32_t msg_size,
    EHSM_SHM const uint8_t *id, uint32_t id_size, uint8_t hid, EHSM_SHM const uint8_t *kgc_pub_key,
    EHSM_SHM const uint8_t *fp12g, EHSM_SHM const uint8_t *sig, uint32_t sig_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)verify_result;

    mb_cmd_sm9_sign_st *cmd = (mb_cmd_sm9_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM9_SIGN);
    cmd->direction = MB_SIG_VRY;
    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;
    cmd->id_addr = ehsm_port_addr_to_raddr(id);
    cmd->id_size = id_size;
    cmd->hid = hid;
    cmd->kgc_pub_key = ehsm_port_addr_to_raddr(kgc_pub_key);
    cmd->fp12g = ehsm_port_addr_to_raddr(fp12g);
    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm9_cipher_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, bool_t enc,
    ehsm_sm9_enc_type_e enc_type, ehsm_sm9_padding_mode_e padding, uint8_t key2_size, uint8_t hid,
    EHSM_SHM const uint8_t *kgc_pub_key, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    uint32_t *output_size, EHSM_SHM const uint8_t *id, uint32_t id_size, EHSM_SHM const uint8_t *fp12g)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)output_size;

    mb_cmd_sm9_cipher_st *cmd = (mb_cmd_sm9_cipher_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM9_CIPHER);

    cmd->key_type = MB_PLAIN_KEY;
    cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    cmd->enc_type = (uint8_t)enc_type;
    cmd->padding_type = (uint8_t)padding;
    cmd->key2_size = key2_size;
    cmd->hid = hid;
    cmd->kgc_pub_key = ehsm_port_addr_to_raddr(kgc_pub_key);
    cmd->input_addr = ehsm_port_addr_to_raddr(input);
    cmd->input_size = input_size;
    cmd->output_addr = ehsm_port_addr_to_raddr(output);
    if (output_size) {
        cmd->output_size = *output_size;
    }
    cmd->id_addr = ehsm_port_addr_to_raddr(id);
    cmd->id_size = id_size;
    cmd->fp12g = ehsm_port_addr_to_raddr(fp12g);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_sm9_sign_onepass_gen_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    EHSM_SHM const uint8_t *msg, uint32_t msg_size, EHSM_SHM uint8_t *sig, uint32_t sig_size,
    EHSM_SHM const uint8_t *kgc_pub_key, EHSM_SHM const uint8_t *fp12g)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_sm9_sign_st *cmd = (mb_cmd_sm9_sign_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SM9_SIGN);
    cmd->key_type = MB_PLAIN_KEY;
    cmd->direction = MB_SIG_GEN;
    cmd->msg_addr = ehsm_port_addr_to_raddr(msg);
    cmd->msg_size = msg_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;
    cmd->hid = 0;
    cmd->kgc_pub_key = ehsm_port_addr_to_raddr(kgc_pub_key);
    cmd->fp12g = ehsm_port_addr_to_raddr(fp12g);
    cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_init(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, bool_t enc, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->process_mode = MB_START;
    chacha_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    chacha_cmd->constant = constant;
    chacha_cmd->key_handle = key_handle;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->input_size = 0;
    chacha_cmd->tag_size = 0;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;
    chacha_cmd->context = ehsm_port_addr_to_raddr(session);
    chacha_cmd->context_size = sizeof(ehsm_session_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_update(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_CHACHA_CIPHER) {
        mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
        if ((chacha_cmd->process_mode != MB_START && chacha_cmd->process_mode != MB_UPDATE)
            || chacha_cmd->context == 0) {
            return EHSM_ERR_CTX_STATUS;
        }

        chacha_cmd->process_mode = MB_UPDATE;
        chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        chacha_cmd->input_size = input_size;
        chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        chacha_cmd->output_size = input_size;
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    // 其它值如direction, key_handle、context地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_finish_enc(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag, uint32_t tag_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_CHACHA_CIPHER) {
        mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
        if ((chacha_cmd->process_mode != MB_START && chacha_cmd->process_mode != MB_UPDATE) || chacha_cmd->context == 0
            || chacha_cmd->direction != MB_CIPHER_ENC) {
            return EHSM_ERR_CTX_STATUS;
        }

        chacha_cmd->process_mode = MB_FINISH;
        chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        chacha_cmd->input_size = input_size;
        chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        chacha_cmd->output_size = input_size;
        chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        chacha_cmd->tag_size = tag_size;
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_finish_dec(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *input, uint32_t input_size,
    EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_CHACHA_CIPHER) {
        mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
        if ((chacha_cmd->process_mode != MB_START && chacha_cmd->process_mode != MB_UPDATE) || chacha_cmd->context == 0
            || chacha_cmd->direction != MB_CIPHER_DEC) {
            return EHSM_ERR_CTX_STATUS;
        }

        chacha_cmd->process_mode = MB_FINISH;
        chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        chacha_cmd->input_size = input_size;
        chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        chacha_cmd->output_size = input_size;
        chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        chacha_cmd->tag_size = tag_size;
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_onepass_enc(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, EHSM_SHM uint8_t *tag,
    uint32_t tag_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    // 默认字段都设为了0，这里不会重复设置
    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->process_mode = MB_ONE_PASS;
    chacha_cmd->direction = MB_CIPHER_ENC;
    chacha_cmd->key_handle = key_handle;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->constant = constant;
    chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
    chacha_cmd->input_size = input_size;
    chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
    // 此处需注意output的buffer长度不小于input_size
    chacha_cmd->output_size = input_size;
    // 此处注意tag_addr的buffer长度不小于tag_size
    chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    chacha_cmd->tag_size = tag_size;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_onepass_dec(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const uint8_t *nonce,
    uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size, uint32_t constant,
    EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output, EHSM_SHM const uint8_t *tag,
    uint32_t tag_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    // 默认字段都设为了0，这里不会重复设置
    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->process_mode = MB_ONE_PASS;
    chacha_cmd->direction = MB_CIPHER_DEC;
    chacha_cmd->key_handle = key_handle;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->constant = constant;
    chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
    chacha_cmd->input_size = input_size;
    chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
    // 此处需注意output的buffer长度不小于input_size
    chacha_cmd->output_size = input_size;
    // 此处注意tag_addr为输入TAG的地址
    chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    chacha_cmd->tag_size = tag_size;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_init_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key, uint32_t key_size,
    bool_t enc, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad, uint32_t aad_size,
    uint32_t constant, EHSM_SHM ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->key_type = MB_PLAIN_KEY;
    chacha_cmd->process_mode = MB_START;
    chacha_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
    chacha_cmd->constant = constant;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->input_size = 0;
    chacha_cmd->tag_size = 0;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;
    chacha_cmd->context = ehsm_port_addr_to_raddr(session);
    chacha_cmd->context_size = sizeof(ehsm_session_st);
    chacha_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    chacha_cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_onepass_enc_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, uint32_t constant, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM uint8_t *tag, uint32_t tag_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    // 默认字段都设为了0，这里不会重复设置
    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->key_type = MB_PLAIN_KEY;
    chacha_cmd->process_mode = MB_ONE_PASS;
    chacha_cmd->direction = MB_CIPHER_ENC;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->constant = constant;
    chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
    chacha_cmd->input_size = input_size;
    chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
    // 此处需注意output的buffer长度不小于input_size
    chacha_cmd->output_size = input_size;
    // 此处注意tag_addr的buffer长度不小于tag_size
    chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    chacha_cmd->tag_size = tag_size;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;
    chacha_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    chacha_cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_chacha_onepass_dec_with_plain_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *key,
    uint32_t key_size, EHSM_SHM const uint8_t *nonce, uint32_t nonce_size, EHSM_SHM const uint8_t *aad,
    uint32_t aad_size, uint32_t constant, EHSM_SHM const uint8_t *input, uint32_t input_size, EHSM_SHM uint8_t *output,
    EHSM_SHM const uint8_t *tag, uint32_t tag_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    // 默认字段都设为了0，这里不会重复设置
    mb_cmd_chacha_cipher_st *chacha_cmd = (mb_cmd_chacha_cipher_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)chacha_cmd, MB_CMD_ID_CHACHA_CIPHER);
    chacha_cmd->key_type = MB_PLAIN_KEY;
    chacha_cmd->process_mode = MB_ONE_PASS;
    chacha_cmd->direction = MB_CIPHER_DEC;
    chacha_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
    chacha_cmd->aad_size = aad_size;
    chacha_cmd->constant = constant;
    chacha_cmd->input_addr = ehsm_port_addr_to_raddr(input);
    chacha_cmd->input_size = input_size;
    chacha_cmd->output_addr = ehsm_port_addr_to_raddr(output);
    // 此处需注意output的buffer长度不小于input_size
    chacha_cmd->output_size = input_size;
    // 此处注意tag_addr为输入TAG的地址
    chacha_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    chacha_cmd->tag_size = tag_size;
    chacha_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
    chacha_cmd->nonce_size = nonce_size;
    chacha_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
    chacha_cmd->plain_key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_init(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode, uint32_t key_handle, bool_t enc,
    const uint8_t *nonce, uint32_t nonce_size, const uint8_t *aad, uint32_t aad_size, uint32_t data_size,
    uint32_t tag_size, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    if (mode == EHSM_AEAD_MODE_GCM) {
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->process_mode = MB_START;
        gcm_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->key_handle = key_handle;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_size = data_size;
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
        gcm_cmd->context = ehsm_port_addr_to_raddr(session);
        gcm_cmd->context_size = sizeof(ehsm_session_st);
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->process_mode = MB_START;
        ccm_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->key_handle = key_handle;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_size = data_size;
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
        ccm_cmd->context = ehsm_port_addr_to_raddr(session);
        ccm_cmd->context_size = sizeof(ehsm_session_st);
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_update(ehsm_ctx_st *ctx, const uint8_t *input, uint32_t input_size, uint8_t *output)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_GCM) {
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        if ((gcm_cmd->process_mode != MB_START && gcm_cmd->process_mode != MB_UPDATE) || gcm_cmd->context == 0) {
            return EHSM_ERR_CTX_STATUS;
        }

        gcm_cmd->process_mode = MB_UPDATE;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
    } else if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_CCM) {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        if ((ccm_cmd->process_mode != MB_START && ccm_cmd->process_mode != MB_UPDATE) || ccm_cmd->context == 0) {
            return EHSM_ERR_CTX_STATUS;
        }

        ccm_cmd->process_mode = MB_UPDATE;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    // 其它值如direction, key_handle、context地址等都缓存在ctx中，不要清掉

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_finish_enc(
    ehsm_ctx_st *ctx, const uint8_t *input, uint32_t input_size, uint8_t *output, uint8_t *tag)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_GCM) {
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        if ((gcm_cmd->process_mode != MB_START && gcm_cmd->process_mode != MB_UPDATE) || gcm_cmd->context == 0
            || gcm_cmd->direction != MB_CIPHER_ENC) {
            return EHSM_ERR_CTX_STATUS;
        }

        gcm_cmd->process_mode = MB_FINISH;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    } else if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_CCM) {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        if ((ccm_cmd->process_mode != MB_START && ccm_cmd->process_mode != MB_UPDATE) || ccm_cmd->context == 0
            || ccm_cmd->direction != MB_CIPHER_ENC) {
            return EHSM_ERR_CTX_STATUS;
        }

        ccm_cmd->process_mode = MB_FINISH;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_finish_dec(ehsm_ctx_st *ctx, const uint8_t *input, uint32_t input_size, uint8_t *output,
    const uint8_t *tag, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    ctx_intl->responded = false;

    if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_GCM) {
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        if ((gcm_cmd->process_mode != MB_START && gcm_cmd->process_mode != MB_UPDATE) || gcm_cmd->context == 0
            || gcm_cmd->direction != MB_CIPHER_DEC) {
            return EHSM_ERR_CTX_STATUS;
        }

        gcm_cmd->process_mode = MB_FINISH;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    } else if (ctx_intl->cmd_buf.cmd_id == MB_CMD_ID_AEAD_CCM) {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        if ((ccm_cmd->process_mode != MB_START && ccm_cmd->process_mode != MB_UPDATE) || ccm_cmd->context == 0
            || ccm_cmd->direction != MB_CIPHER_DEC) {
            return EHSM_ERR_CTX_STATUS;
        }

        ccm_cmd->process_mode = MB_FINISH;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
    } else {
        return EHSM_ERR_CTX_STATUS;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_onepass_enc(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode, uint32_t key_handle,
    const uint8_t *nonce, uint32_t nonce_size, const uint8_t *aad, uint32_t aad_size, const uint8_t *input,
    uint32_t input_size, uint8_t *output, uint8_t *tag, uint32_t tag_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    if (mode == EHSM_AEAD_MODE_GCM) {
        // 默认字段都设为了0，这里不会重复设置
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->process_mode = MB_ONE_PASS;
        gcm_cmd->direction = MB_CIPHER_ENC;
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->key_handle = key_handle;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        // 此处注意tag_addr的buffer长度不小于tag_size
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->process_mode = MB_ONE_PASS;
        ccm_cmd->direction = MB_CIPHER_ENC;
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->key_handle = key_handle;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        // 此处注意tag_addr的buffer长度不小于tag_size
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_onepass_dec(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode, uint32_t key_handle,
    const uint8_t *nonce, uint32_t nonce_size, const uint8_t *aad, uint32_t aad_size, const uint8_t *input,
    uint32_t input_size, uint8_t *output, const uint8_t *tag, uint32_t tag_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    if (mode == EHSM_AEAD_MODE_GCM) {
        // 默认字段都设为了0，这里不会重复设置
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->process_mode = MB_ONE_PASS;
        gcm_cmd->direction = MB_CIPHER_DEC;
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->key_handle = key_handle;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        // 此处注意tag_addr为输入TAG的地址
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->process_mode = MB_ONE_PASS;
        ccm_cmd->direction = MB_CIPHER_DEC;
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->key_handle = key_handle;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        // 此处注意tag_addr为输入TAG的地址
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_init_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, bool_t enc, const uint8_t *nonce, uint32_t nonce_size,
    const uint8_t *aad, uint32_t aad_size, uint32_t data_size, uint32_t tag_size, ehsm_session_st *session)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    if (mode == EHSM_AEAD_MODE_GCM) {
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->key_type = MB_PLAIN_KEY;
        gcm_cmd->process_mode = MB_START;
        gcm_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_size = data_size;
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
        gcm_cmd->context = ehsm_port_addr_to_raddr(session);
        gcm_cmd->context_size = sizeof(ehsm_session_st);
        gcm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        gcm_cmd->plain_key_size = key_size;
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->key_type = MB_PLAIN_KEY;
        ccm_cmd->process_mode = MB_START;
        ccm_cmd->direction = (uint8_t)(enc ? MB_CIPHER_ENC : MB_CIPHER_DEC);
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_size = data_size;
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
        ccm_cmd->context = ehsm_port_addr_to_raddr(session);
        ccm_cmd->context_size = sizeof(ehsm_session_st);
        ccm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        ccm_cmd->plain_key_size = key_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_onepass_enc_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, const uint8_t *nonce, uint32_t nonce_size, const uint8_t *aad,
    uint32_t aad_size, const uint8_t *input, uint32_t input_size, uint8_t *output, uint8_t *tag, uint32_t tag_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    if (mode == EHSM_AEAD_MODE_GCM) {
        // 默认字段都设为了0，这里不会重复设置
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->key_type = MB_PLAIN_KEY;
        gcm_cmd->process_mode = MB_ONE_PASS;
        gcm_cmd->direction = MB_CIPHER_ENC;
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        // 此处注意tag_addr的buffer长度不小于tag_size
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
        gcm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        gcm_cmd->plain_key_size = key_size;
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->key_type = MB_PLAIN_KEY;
        ccm_cmd->process_mode = MB_ONE_PASS;
        ccm_cmd->direction = MB_CIPHER_ENC;
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        // 此处注意tag_addr的buffer长度不小于tag_size
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
        ccm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        ccm_cmd->plain_key_size = key_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_aead_onepass_dec_with_plain_key(ehsm_ctx_st *ctx, ehsm_symm_algo_e algo, ehsm_aead_mode_e mode,
    EHSM_SHM const uint8_t *key, uint32_t key_size, const uint8_t *nonce, uint32_t nonce_size, const uint8_t *aad,
    uint32_t aad_size, const uint8_t *input, uint32_t input_size, uint8_t *output, const uint8_t *tag,
    uint32_t tag_size, bool_t *verify_result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    ctx_intl->result1 = (void *)verify_result;

    if (mode == EHSM_AEAD_MODE_GCM) {
        // 默认字段都设为了0，这里不会重复设置
        mb_cmd_aead_gcm_st *gcm_cmd = (mb_cmd_aead_gcm_st *)&ctx_intl->cmd_buf;
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)gcm_cmd, MB_CMD_ID_AEAD_GCM);
        gcm_cmd->key_type = MB_PLAIN_KEY;
        gcm_cmd->process_mode = MB_ONE_PASS;
        gcm_cmd->direction = MB_CIPHER_DEC;
        gcm_cmd->algorithm = (uint8_t)algo;
        gcm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        gcm_cmd->aad_size = aad_size;
        gcm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        gcm_cmd->input_size = input_size;
        gcm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        gcm_cmd->output_size = input_size;
        // 此处注意tag_addr为输入TAG的地址
        gcm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        gcm_cmd->tag_size = tag_size;
        gcm_cmd->iv_addr = ehsm_port_addr_to_raddr(nonce);
        gcm_cmd->iv_size = nonce_size;
        gcm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        gcm_cmd->plain_key_size = key_size;
    } else {
        mb_cmd_aead_ccm_st *ccm_cmd = (mb_cmd_aead_ccm_st *)&ctx_intl->cmd_buf;
        // 默认字段都设为了0，这里不会重复设置
        ehsm_set_cmd_id((ehsm_mb_cmd_st *)ccm_cmd, MB_CMD_ID_AEAD_CCM);
        ccm_cmd->key_type = MB_PLAIN_KEY;
        ccm_cmd->process_mode = MB_ONE_PASS;
        ccm_cmd->direction = MB_CIPHER_DEC;
        ccm_cmd->algorithm = (uint8_t)algo;
        ccm_cmd->aad_addr = ehsm_port_addr_to_raddr(aad);
        ccm_cmd->aad_size = aad_size;
        ccm_cmd->input_addr = ehsm_port_addr_to_raddr(input);
        ccm_cmd->input_size = input_size;
        ccm_cmd->output_addr = ehsm_port_addr_to_raddr(output);
        // 此处需注意output的buffer长度不小于input_size
        ccm_cmd->output_size = input_size;
        // 此处注意tag_addr为输入TAG的地址
        ccm_cmd->tag_addr = ehsm_port_addr_to_raddr(tag);
        ccm_cmd->tag_size = tag_size;
        ccm_cmd->nonce_addr = ehsm_port_addr_to_raddr(nonce);
        ccm_cmd->nonce_size = nonce_size;
        ccm_cmd->plain_key_addr = ehsm_port_addr_to_raddr(key);
        ccm_cmd->plain_key_size = key_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_gen_key(ehsm_ctx_st *ctx, ehsm_key_type_e key_type, uint32_t privilege, uint32_t rsa_e_bit_size,
    uint32_t hmac_key_size, const void *dh_params, uint32_t dh_params_size, uint32_t *key_handle)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)key_handle;

    mb_cmd_ehsm_gen_key_st *cmd = (mb_cmd_ehsm_gen_key_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_GEN_KEY);
    cmd->algo_id = (uint8_t)key_type;
    if (rsa_e_bit_size > 65535U || hmac_key_size > 65535U) {
        return EHSM_ERR_PARAM_ERROR;
    }
    cmd->rsa_e_bit_size = (uint16_t)rsa_e_bit_size;
    cmd->permit = privilege;
    cmd->key_size = (uint16_t)hmac_key_size;
    if (key_handle) {
        cmd->key_handle = *key_handle;
    }
    cmd->dh_common_addr = ehsm_port_addr_to_raddr(dh_params);
    cmd->dh_common_size = dh_params_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_import_key(ehsm_ctx_st *ctx, uint32_t transport_key_handle, uint32_t auth_key_handle,
    const ehsm_key_format_st *key_data, uint32_t key_data_size, const uint8_t *mac, uint32_t mac_size,
    uint32_t *key_handle)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result2 = (void *)key_handle;

    mb_cmd_ehsm_import_key_st *cmd = (mb_cmd_ehsm_import_key_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_IMPORT_KEY);
    if (key_handle) {
        cmd->key_handle = *key_handle;
    }
    cmd->transport_key_handle = transport_key_handle;
    cmd->authenticity_key_handle = auth_key_handle;
    cmd->key_data_addr = ehsm_port_addr_to_raddr(key_data);
    cmd->key_data_size = key_data_size;
    cmd->key_signature_addr = ehsm_port_addr_to_raddr(mac);
    cmd->key_signature_size = mac_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_export_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t target_key_handle, uint32_t transport_key_handle,
    uint32_t auth_key_handle, ehsm_key_part_e key_part, EHSM_SHM ehsm_key_format_st *key_data, uint32_t *key_data_size,
    EHSM_SHM uint8_t *mac, uint32_t *mac_size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)key_data_size;
    ctx_intl->result2 = (void *)mac_size;

    mb_cmd_ehsm_export_key_st *cmd = (mb_cmd_ehsm_export_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_EXPORT_KEY);
    cmd->req_key_part = (uint8_t)key_part;
    cmd->key_handle = target_key_handle;
    cmd->transport_key_handle = transport_key_handle;
    cmd->authenticity_key_handle = auth_key_handle;
    cmd->key_data_addr = ehsm_port_addr_to_raddr(key_data);
    if (key_data_size) {
        cmd->key_data_size = *key_data_size;
    }
    cmd->key_signature_addr = ehsm_port_addr_to_raddr(mac);
    if (mac_size) {
        cmd->key_signature_size = *mac_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_derive_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e hash_algo, ehsm_derive_algo_e derive_algo,
    ehsm_derive_type_e derive_type, uint32_t privilege, ehsm_key_type_e key_type, uint16_t key_size,
    uint32_t parent_key_handle, EHSM_SHM const uint8_t *salt, uint32_t salt_size, EHSM_SHM const uint8_t *password,
    uint32_t password_size, uint32_t iter_times, uint32_t *key_handle)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)key_handle;

    mb_cmd_ehsm_key_derive_st *cmd = (mb_cmd_ehsm_key_derive_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_KEY_DERIVE);

    cmd->key_deriv_func = (uint8_t)derive_algo;
    cmd->derive_type = (uint8_t)derive_type;
    cmd->hash_alg = (uint8_t)hash_algo;
    cmd->algo_id = (uint8_t)key_type;
    cmd->key_out_dir = MB_EHSM_KEY_DERIVE_KEY_OUT_DIR_KEY_DERIVE_OUTPUT_TO_KMS;
    cmd->soc_sp_id = 0;
    if (key_handle) {
        cmd->key_handle = *key_handle;
    }
    cmd->permit = privilege;
    cmd->parent_key_handle = parent_key_handle;
    cmd->salt_data_addr = ehsm_port_addr_to_raddr(salt);
    cmd->salt_data_size = salt_size;
    cmd->pw_data_addr = ehsm_port_addr_to_raddr(password);
    cmd->pw_data_size = password_size;
    cmd->itera_times = iter_times;
    cmd->key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_derive_key_to_soc(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e hash_algo,
    ehsm_derive_algo_e derive_algo, ehsm_derive_type_e derive_type, uint32_t parent_key_handle,
    EHSM_SHM const uint8_t *salt, uint32_t salt_size, uint32_t iter_times, uint8_t soc_channel_id)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_ehsm_key_derive_st *cmd = (mb_cmd_ehsm_key_derive_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_KEY_DERIVE);

    cmd->key_deriv_func = (uint8_t)derive_algo;
    cmd->derive_type = (uint8_t)derive_type;
    cmd->hash_alg = (uint8_t)hash_algo;
    cmd->algo_id = (uint8_t)EHSM_KEY_TYPE_AES_256;
    cmd->key_out_dir = MB_EHSM_KEY_DERIVE_KEY_OUT_DIR_KEY_DERIVE_OUTPUT_TO_SOC;
    cmd->soc_sp_id = soc_channel_id;
    cmd->parent_key_handle = parent_key_handle;
    cmd->salt_data_addr = ehsm_port_addr_to_raddr(salt);
    cmd->salt_data_size = salt_size;
    cmd->itera_times = iter_times;
    cmd->key_size = 32;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_exchange_key(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *rmt_pub_key, uint32_t rmt_pub_key_size,
    uint32_t privilege, ehsm_key_type_e key_type, uint32_t hmac_key_size, uint32_t local_key_handle,
    EHSM_SHM const void *dh_params, uint32_t dh_params_size, EHSM_SHM ehsm_sm2_params_st *sm2_params,
    uint32_t *key_handle)
{
    uint32_t key_size = 0;
    switch (key_type) {
    case EHSM_KEY_TYPE_DES:
        key_size = 8;
        break;
    case EHSM_KEY_TYPE_TDES_128:
    case EHSM_KEY_TYPE_AES_128:
    case EHSM_KEY_TYPE_SM4:
        key_size = 16;
        break;
    case EHSM_KEY_TYPE_AES_128_XTS:
    case EHSM_KEY_TYPE_AES_256:
    case EHSM_KEY_TYPE_SM4_XTS:
        key_size = 32;
        break;
    case EHSM_KEY_TYPE_AES_192:
        key_size = 24;
        break;
    case EHSM_KEY_TYPE_AES_192_XTS:
        key_size = 48;
        break;
    case EHSM_KEY_TYPE_AES_256_XTS:
        key_size = 64;
        break;
    case EHSM_KEY_TYPE_HMAC:
        key_size = hmac_key_size;
        break;
    case EHSM_KEY_TYPE_TDES_192:
        key_size = 24;
        break;
    case EHSM_KEY_TYPE_CHACHA:
        key_size = 32;
        break;
    default:
        return EHSM_ERR_PARAM_ERROR;
    }

    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)key_handle;

    mb_cmd_ehsm_key_exchange_st *cmd = (mb_cmd_ehsm_key_exchange_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_KEY_EXCHANGE);

    cmd->algo_id = (uint8_t)key_type;
    cmd->key_size = (uint16_t)key_size;
    if (key_handle) {
        cmd->key_handle = *key_handle;
    }
    cmd->permit = privilege;
    cmd->local_key_handle = local_key_handle;
    cmd->dh_commom_addr = ehsm_port_addr_to_raddr(dh_params);
    cmd->dh_commom_size = dh_params_size;
    cmd->remote_pubkey_data = ehsm_port_addr_to_raddr(rmt_pub_key);
    cmd->remote_pubkey_size = rmt_pub_key_size;
    cmd->sm2_param_addr = ehsm_port_addr_to_raddr(sm2_params);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_sm9_exchange_key(EHSM_SHM ehsm_ctx_st *ctx, uint32_t privilege, ehsm_key_type_e key_type, uint8_t role,
    uint32_t user_priv_key_handle, uint32_t user_tmp_key_handle, uint32_t hmac_key_size,
    ehsm_sm9_params_st *extra_params, uint32_t *key_handle)
{
    uint32_t key_size = 0;
    switch (key_type) {
    case EHSM_KEY_TYPE_DES:
        key_size = 8;
        break;
    case EHSM_KEY_TYPE_TDES_128:
    case EHSM_KEY_TYPE_AES_128:
    case EHSM_KEY_TYPE_SM4:
        key_size = 16;
        break;
    case EHSM_KEY_TYPE_AES_128_XTS:
    case EHSM_KEY_TYPE_AES_256:
    case EHSM_KEY_TYPE_SM4_XTS:
        key_size = 32;
        break;
    case EHSM_KEY_TYPE_AES_192:
        key_size = 24;
        break;
    case EHSM_KEY_TYPE_AES_192_XTS:
        key_size = 48;
        break;
    case EHSM_KEY_TYPE_AES_256_XTS:
        key_size = 64;
        break;
    case EHSM_KEY_TYPE_HMAC:
        key_size = hmac_key_size;
        break;
    case EHSM_KEY_TYPE_TDES_192:
        key_size = 24;
        break;
    case EHSM_KEY_TYPE_CHACHA:
        key_size = 32;
        break;
    default:
        return EHSM_ERR_PARAM_ERROR;
    }

    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)key_handle;

    mb_cmd_ehsm_sm9_exchg_key_st *cmd = (mb_cmd_ehsm_sm9_exchg_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_SM9_EXCHG_KEY);

    cmd->algo_id = (uint8_t)key_type;
    cmd->role = (uint8_t)role;
    cmd->user_priv_key_handle = user_priv_key_handle;
    cmd->permit = privilege;
    cmd->other_info = ehsm_port_addr_to_raddr(extra_params);
    cmd->user_tmp_key_handle = user_tmp_key_handle;
    cmd->key_size = key_size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_get_pub_from_priv(EHSM_SHM ehsm_ctx_st *ctx, uint32_t key_handle, EHSM_SHM const void *dh_params,
    uint32_t dh_params_size, EHSM_SHM uint8_t *pub_key, uint32_t *pub_key_size, ehsm_key_type_e *key_type)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)key_type;
    ctx_intl->result2 = (void *)pub_key_size;

    mb_cmd_ehsm_get_pub_from_priv_st *cmd = (mb_cmd_ehsm_get_pub_from_priv_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_GET_PUB_FROM_PRIV);

    cmd->key_handle = key_handle;
    cmd->dh_common_addr = ehsm_port_addr_to_raddr(dh_params);
    cmd->dh_common_size = dh_params_size;
    cmd->public_key_addr = ehsm_port_addr_to_raddr(pub_key);
    if (pub_key_size) {
        cmd->public_key_buffer_size = *pub_key_size;
    }

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_km_remove_key(ehsm_ctx_st *ctx, uint32_t key_handle)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_ehsm_remove_key_st *cmd = (mb_cmd_ehsm_remove_key_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_EHSM_REMOVE_KEY);
    cmd->key_handle = key_handle;

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_read_ver_st) == sizeof(mb_cmd_ehsm_read_ver_st));
STATIC_ASSERT(MB_CMD_ID_BL_READ_VER == MB_CMD_ID_EHSM_READ_VER);

uint32_t ehsm_get_version(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_version_st *version)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_read_ver_st *cmd = (mb_cmd_bl_read_ver_st *)&ctx_intl->cmd_buf;
    // 默认字段都设为了0，这里不会重复设置
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_READ_VER);
    cmd->ver_addr = ehsm_port_addr_to_raddr(version);
    cmd->ver_size = sizeof(ehsm_version_st);

    return ehsm_send_cmd(ctx_intl);
}

// STATIC_ASSERT(sizeof(mb_cmd_bl_verify_image_st) == sizeof(mb_cmd_soc_verify_st));
STATIC_ASSERT(MB_CMD_ID_BL_VERIFY_IMAGE == MB_CMD_ID_SOC_VERIFY);

uint32_t ehsm_verify_image(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *image, uint32_t image_size,
    bool_t check_version, bool_t boot, EHSM_SHM uint8_t *image_out)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_verify_image_st *cmd = (mb_cmd_bl_verify_image_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_VERIFY_IMAGE);
    cmd->image_addr = ehsm_port_addr_to_raddr(image);
    cmd->image_size = image_size;
    cmd->boot_after_verify = boot ? 1 : 0;
    cmd->check_version = check_version ? 1 : 0;
    cmd->image_out_addr = ehsm_port_addr_to_raddr(image_out);

    return ehsm_send_cmd(ctx_intl);
}


STATIC_ASSERT(sizeof(mb_cmd_bl_fw_upgrade_st) == sizeof(mb_cmd_fw_upgrade_st));
STATIC_ASSERT(MB_CMD_ID_BL_FW_UPGRADE == MB_CMD_ID_FW_UPGRADE);

uint32_t ehsm_upgrade_fw_image_ex(EHSM_SHM ehsm_ctx_st *ctx, ehsm_proc_mode_e mode, EHSM_SHM const uint8_t *input,
    uint32_t input_size, EHSM_SHM uint8_t *output)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_fw_upgrade_st *cmd = (mb_cmd_bl_fw_upgrade_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_FW_UPGRADE);

    cmd->process_mode = (uint8_t)mode;
    cmd->image_addr = ehsm_port_addr_to_raddr(input);
    cmd->image_size = input_size;
    cmd->storage_addr = ehsm_port_addr_to_raddr(output);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_set_utc_time(EHSM_SHM ehsm_ctx_st *ctx, uint32_t utc_time)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_set_utc_time_st *cmd = (mb_cmd_set_utc_time_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_SET_UTC_TIME);
    cmd->utc_time = utc_time;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_get_utc_time(EHSM_SHM ehsm_ctx_st *ctx, uint32_t *utc_time)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)utc_time;

    mb_cmd_get_utc_time_st *cmd = (mb_cmd_get_utc_time_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_GET_UTC_TIME);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_create_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t *counter_id, uint64_t *counter_value)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)counter_value;
    ctx_intl->result2 = (void *)counter_id;

    mb_cmd_create_counter_st *cmd = (mb_cmd_create_counter_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_CREATE_COUNTER);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_read_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id, uint64_t *counter_value)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)counter_value;

    mb_cmd_read_counter_st *cmd = (mb_cmd_read_counter_st *)&ctx_intl->cmd_buf;
    cmd->counter_id = counter_id;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_READ_COUNTER);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_increase_counter(
    EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id, uint64_t increase_value, uint64_t *current_value)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    ctx_intl->result1 = (void *)current_value;

    mb_cmd_increase_counter_value_st *cmd = (mb_cmd_increase_counter_value_st *)&ctx_intl->cmd_buf;
    cmd->counter_id = counter_id;
    cmd->counter_inc_l = (uint32_t)increase_value;
    cmd->counter_inc_h = (uint32_t)(increase_value >> 32);
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_INCREASE_COUNTER_VALUE);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_delete_counter(EHSM_SHM ehsm_ctx_st *ctx, uint32_t counter_id)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_delete_counter_st *cmd = (mb_cmd_delete_counter_st *)&ctx_intl->cmd_buf;
    cmd->counter_id = counter_id;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_DELETE_COUNTER);

    return ehsm_send_cmd(ctx_intl);
}


STATIC_ASSERT(sizeof(mb_cmd_bl_set_uart_baudrate_st) == sizeof(mb_cmd_set_uart_baudrate_st));
STATIC_ASSERT(MB_CMD_ID_BL_SET_UART_BAUDRATE == MB_CMD_ID_SET_UART_BAUDRATE);

uint32_t ehsm_set_uart_baudrate(EHSM_SHM ehsm_ctx_st *ctx, uint32_t baud_div)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_set_uart_baudrate_st *cmd = (mb_cmd_bl_set_uart_baudrate_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_SET_UART_BAUDRATE);
    cmd->baud_div = baud_div;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_install_random_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level,
    ehsm_install_key_type_e key_type, uint16_t key_slot_id, bool_t last_key)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_install_random_key_st *cmd = (mb_cmd_install_random_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_INSTALL_RANDOM_KEY);
    cmd->key_level = (uint8_t)key_level;
    cmd->key_type = (uint8_t)key_type;
    cmd->key_slot_id = key_slot_id;
    cmd->key_last_flag = (uint8_t)last_key;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_install_encrypted_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level,
    ehsm_install_key_type_e key_type, uint16_t key_slot_id, bool_t last_key, EHSM_SHM const uint8_t *input_data,
    uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_install_encrypt_key_st *cmd = (mb_cmd_install_encrypt_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_INSTALL_ENCRYPT_KEY);
    cmd->key_level = (uint8_t)key_level;
    cmd->key_type = (uint8_t)key_type;
    cmd->key_slot_id = key_slot_id;
    cmd->key_last_flag = (uint8_t)last_key;
    cmd->input_addr = ehsm_port_addr_to_raddr(input_data);
    cmd->input_size = size;

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_otp_read_st) == sizeof(mb_cmd_otp_read_st));
STATIC_ASSERT(MB_CMD_ID_BL_OTP_READ == MB_CMD_ID_OTP_READ);

uint32_t ehsm_read_otp(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_otp_read_st *cmd = (mb_cmd_bl_otp_read_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_OTP_READ);
    cmd->ehsm_src_addr = ehsm_src_addr;
    cmd->host_dst_addr = ehsm_port_addr_to_raddr(buf);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_otp_write_st) == sizeof(mb_cmd_otp_write_st));
STATIC_ASSERT(MB_CMD_ID_BL_OTP_WRITE == MB_CMD_ID_OTP_WRITE);

uint32_t ehsm_write_otp(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_otp_write_st *cmd = (mb_cmd_bl_otp_write_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_OTP_WRITE);
    cmd->ehsm_dst_addr = ehsm_dest_addr;
    cmd->host_src_addr = ehsm_port_addr_to_raddr(src_data);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_reg_rd_st) == sizeof(mb_cmd_fw_reg_rd_st));
STATIC_ASSERT(MB_CMD_ID_BL_REG_RD == MB_CMD_ID_FW_REG_RD);

uint32_t ehsm_read_reg(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_reg_rd_st *cmd = (mb_cmd_bl_reg_rd_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_REG_RD);
    cmd->ehsm_src_addr = ehsm_src_addr;
    cmd->host_dst_addr = ehsm_port_addr_to_raddr(buf);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_reg_wr_st) == sizeof(mb_cmd_fw_reg_wr_st));
STATIC_ASSERT(MB_CMD_ID_BL_REG_WR == MB_CMD_ID_FW_REG_WR);

uint32_t ehsm_write_reg(
    EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_reg_wr_st *cmd = (mb_cmd_bl_reg_wr_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_REG_WR);
    cmd->ehsm_dst_addr = ehsm_dest_addr;
    cmd->host_src_addr = ehsm_port_addr_to_raddr(src_data);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_bl_get_random_key(
    EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level, ehsm_bl_gen_key_type_e key_type, EHSM_SHM uint8_t *key_out)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_get_random_key_st *cmd = (mb_cmd_bl_get_random_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_GET_RANDOM_KEY);
    cmd->key_level = (uint8_t)key_level;
    cmd->key_type = (uint8_t)key_type;
    cmd->key_addr = ehsm_port_addr_to_raddr(key_out);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_bl_encrypt_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level, EHSM_SHM const uint8_t *input_data,
    uint32_t size, EHSM_SHM uint8_t *key_out)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_encrypt_key_st *cmd = (mb_cmd_bl_encrypt_key_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_ENCRYPT_KEY);
    cmd->input_addr = ehsm_port_addr_to_raddr(input_data);
    cmd->input_size = size;
    cmd->key_level = (uint8_t)key_level;
    cmd->output_addr = ehsm_port_addr_to_raddr(key_out);

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_get_challenge_st) == sizeof(mb_cmd_get_challenge_st));
STATIC_ASSERT(MB_CMD_ID_BL_GET_CHALLENGE == MB_CMD_ID_GET_CHALLENGE);

uint32_t ehsm_get_challenge(EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e challenge_type, EHSM_SHM uint8_t *output)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_get_challenge_st *cmd = (mb_cmd_bl_get_challenge_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_GET_CHALLENGE);
    cmd->type = (uint8_t)challenge_type;
    cmd->addr = ehsm_port_addr_to_raddr(output);

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_debug_auth_st) == sizeof(mb_cmd_debug_auth_st));
STATIC_ASSERT(MB_CMD_ID_BL_DEBUG_AUTH == MB_CMD_ID_DEBUG_AUTH);

uint32_t ehsm_debug_auth(EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e challenge_type, ehsm_auth_algo_e algo,
    EHSM_SHM const uint8_t *sig, uint32_t sig_size, EHSM_SHM const uint8_t *pub_key, uint32_t pub_key_size,
    EHSM_SHM const ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_debug_auth_st *cmd = (mb_cmd_bl_debug_auth_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_DEBUG_AUTH);
    cmd->alg = (uint8_t)algo;
    cmd->type = (uint8_t)challenge_type;
    cmd->pub_addr = ehsm_port_addr_to_raddr(pub_key);
    cmd->pub_size = pub_key_size;
    cmd->sign_addr = ehsm_port_addr_to_raddr(sig);
    cmd->sign_size = sig_size;
    if (soc_dbg_bitmap) {
        cmd->soc_dbg_bitmap_addr = ehsm_port_addr_to_raddr(&soc_dbg_bitmap->bitmaps[0]);
    }
    cmd->soc_dbg_bitmap_size = sizeof(ehsm_soc_dbg_bitmap_st) / sizeof(uint32_t);

    return ehsm_send_cmd(ctx_intl);
}

STATIC_ASSERT(sizeof(mb_cmd_bl_close_debug_st) == sizeof(mb_cmd_close_debug_st));
STATIC_ASSERT(MB_CMD_ID_BL_CLOSE_DEBUG == MB_CMD_ID_CLOSE_DEBUG);

uint32_t ehsm_close_debug(
    EHSM_SHM ehsm_ctx_st *ctx, ehsm_challenge_type_e type, EHSM_SHM const ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_close_debug_st *cmd = (mb_cmd_bl_close_debug_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_CLOSE_DEBUG);
    cmd->type = (uint8_t)type;
    if (soc_dbg_bitmap) {
        cmd->soc_dbg_bitmap_addr = ehsm_port_addr_to_raddr(&soc_dbg_bitmap->bitmaps[0]);
    }
    cmd->soc_dbg_bitmap_size = sizeof(ehsm_soc_dbg_bitmap_st) / sizeof(uint32_t);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_bl_self_test(EHSM_SHM ehsm_ctx_st *ctx)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_self_test_st *cmd = (mb_cmd_bl_self_test_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_SELF_TEST);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_bl_get_self_test_result(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_self_test_result_st *result)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_read_self_test_result_st *cmd = (mb_cmd_bl_read_self_test_result_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_READ_SELF_TEST_RESULT);
    cmd->result_addr = ehsm_port_addr_to_raddr(result);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_bl_fw_auth(EHSM_SHM ehsm_ctx_st *ctx, uint8_t type, const uint8_t arg[16],
    EHSM_SHM const uint8_t *auth_data, EHSM_SHM uint8_t *out_addr)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_bl_fw_auth_st *cmd = (mb_cmd_bl_fw_auth_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_BL_FW_AUTH);
    cmd->type = type;
    if (arg == NULL || auth_data == NULL) {
        return EHSM_ERR_PARAM_ERROR;
    }
    memcpy(cmd->arg, arg, 16);
    memcpy(cmd->auth_data, auth_data, 32);
    cmd->out_addr = ehsm_port_addr_to_raddr(out_addr);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_enter_wfi(EHSM_SHM ehsm_ctx_st *ctx)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_enter_wfi_st *cmd = (mb_cmd_enter_wfi_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_ENTER_WFI);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_get_emu_status(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_emu_status_st *status_buf)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_get_emu_st *cmd = (mb_cmd_get_emu_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_GET_EMU);
    cmd->emu_addr = ehsm_port_addr_to_raddr(status_buf);
    cmd->emu_size = sizeof(ehsm_emu_status_st);

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_change_lifecycle(EHSM_SHM ehsm_ctx_st *ctx, ehsm_lifecycle_e lifecycle)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_change_lifecycle_st *cmd = (mb_cmd_change_lifecycle_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_CHANGE_LIFECYCLE);
    cmd->type = (uint32_t)lifecycle;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_change_control_field(EHSM_SHM ehsm_ctx_st *ctx, ehsm_ctrl_field_e field, EHSM_SHM const uint64_t *value)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;
    mb_cmd_change_control_field_st *cmd = (mb_cmd_change_control_field_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_CHANGE_CONTROL_FIELD);

    cmd->size = sizeof(uint64_t);
    cmd->type = (uint8_t)field;
    cmd->value = ehsm_port_addr_to_raddr(value);

    return ehsm_send_cmd(ctx_intl);
}
