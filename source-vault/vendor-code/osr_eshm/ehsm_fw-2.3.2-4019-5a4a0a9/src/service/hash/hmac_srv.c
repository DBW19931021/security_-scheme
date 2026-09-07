/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "hmac_srv.h"
#include "hash_srv.h"
#include "mb.h"
#include "mmap.h"
#include "cpu_porting.h"
#include "service/crypto_util.h"
#include "component/util.h"
#include "kms.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// The minimum length of verified MAC value
#define HMAC_SRV_MAC_BYTE_LEN_MIN (8U)

#define HMAC_K_BUF_SZ (KMS_SYM_KEY_DATA_MAX_SIZE)

#define HMAC_SRV_DATA_BUF_BYTE_SIZE (512U)

#define HMAC_SRV_MAC_BYTE_LEN_MAX (HASH_DIGEST_MAX_WORD_LEN * 4U)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef enum {
    HMAC_DMA_MODE = 0U,
    HMAC_CPU_MODE = 1U,
} hmac_mode_e;

typedef enum {
    HMAC_RCTX_2_LCTX = 0U,
    HMAC_LCTX_2_RCTX = 1U,
} hmac_switch_e;

typedef struct {
    cpt_hash_rctx_st hash_rctx;
} hmac_rctx_st;
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/*
 *  Since the hardware hmac engine will only genearte the hmac, the mac verification process will be dealed with
 * software. In that case, the calculation of HMAC in dma mode must be stored in the EHSM. However the AHB-DMA channel
 * could not write the data to ram region. So that I will use the system general register, which could be accessed by
 * AHB-DMA, to keep result of calculation temporaly. When invoking the cpt_hmac_dma API, the parameter
 * EHSM_WRITE_SOC_READ is passed to configure the DMA controller's configuration register.
 */

/**
 *   @brief      Getting key for hmac service
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] key
 *   @param [in] key_sz
 *   @param [in] sp_key_idx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_get_key_from_handle(const mb_cmd_hmac_st *hmac_cmd_data, uint8_t *key_buf, uint32_t key_buf_sz,
    uint8_t **key, uint32_t *key_sz, uint8_t *sp_key_idx)
{
    uint32_t ret;
    kms_keydata_st keydata;
    uint8_t req_kpart = KMS_KEY_PART_PRIVKEY;
    uint32_t check_usage_bits = (MB_MAC_GEN == hmac_cmd_data->direction) ? KEY_USAGE_SIGN : KEY_USAGE_VERIFY;
    ret = kms_read_key(hmac_cmd_data->key_handle, key_buf, key_buf_sz, &keydata, req_kpart, check_usage_bits);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (KMS_KEY_ALG_HMAC == keydata.algo_id) {
            if (KMS_KEY_SRC_TYPE_RAM == keydata.src_type) {
                *sp_key_idx = 0U;
                *key = keydata.keypair.symm.key;
                *key_sz = keydata.keypair.symm.size;
            } else {
                *key = NULL;
                *sp_key_idx = (uint8_t)keydata.sp_key_id;
                *key_sz = keydata.keypair.symm.size;
            }
        } else {
            ret = EHSM_ERR_MISMATCH_KEY_USAGE;
        }
    }
    return ret;
}

/**
 *   @brief      Getting key for hmac service
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] key
 *   @param [in] key_sz
 *   @param [in] sp_key_idx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_get_key(const mb_cmd_hmac_st *hmac_cmd_data, uint8_t *key_buf, uint32_t key_buf_sz, uint8_t **key,
    uint32_t *key_sz, uint8_t *sp_key_idx)
{
    uint32_t ret;

    switch (hmac_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        *sp_key_idx = 0U;
        if ((0U == hmac_cmd_data->plain_key_size) || (hmac_cmd_data->plain_key_size > key_buf_sz)) {
            ret = EHSM_ERR_NOT_SUPPORT;
        } else {
            ret = mmap_read_remote_data(key_buf, hmac_cmd_data->plain_key_addr, hmac_cmd_data->plain_key_size);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *key = key_buf;
                *key_sz = hmac_cmd_data->plain_key_size;
            }
        }
        break;
    case MB_KEY_HANDLE:
        ret = hmac_get_key_from_handle(hmac_cmd_data, key_buf, key_buf_sz, key, key_sz, sp_key_idx);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Switch the context of CPU mode between HOST and EHSM
 *
 *   @param [in] rctx_addr
 *   @param [in] key_handle
 *   @param [in] dir
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_rctx_switch_cpu_ctx(
    raddr_t rctx_addr, const mb_cmd_hmac_st *hmac_cmd_data, cpt_hmac_ctx_st *lctx, hmac_switch_e type)
{
    bool_t is_sp_key;
    cpt_hash_switch_e hash_switch;
    uint8_t key_buf[HMAC_K_BUF_SZ];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_sz;
    uint8_t *key_ptr;
    uint8_t sp_key_idx = 0U;

    // 1. build the hmac hash context from rctx by using hash rctx switch API
    hash_switch = (HMAC_RCTX_2_LCTX == type) ? HASH_RCTX_2_LCTX : HASH_LCTX_2_RCTX;
    ret = cpt_hash_rctx_switch_cpu_ctx(rctx_addr, lctx->hash_ctx, hash_switch);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (HMAC_RCTX_2_LCTX == type)) {
        // 2. if the type is HMAC_RCTX_2_LCTX, build the hmac context without hash context(such as k0/key_len_flag)
        ret = hmac_get_key(hmac_cmd_data, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_idx);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            is_sp_key = (sp_key_idx == 0U) ? false : true;
            ret = cpt_build_hmac_cpu_ctx_without_hash(key_ptr, key_sz, sp_key_idx, is_sp_key, lctx);
        }
        util_memset(key_buf, 0, sizeof(key_buf));
    }

    return ret;
}

/**
 *   @brief      Switch the context of DMA mode between HOST and EHSM
 *
 *   @param [in] rctx_addr
 *   @param [in] hmac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_rctx_switch_dma_ctx(
    raddr_t rctx_addr, const mb_cmd_hmac_st *hmac_cmd_data, cpt_hmac_dma_ctx_st *lctx, hmac_switch_e type)
{
    cpt_hash_switch_e hash_switch;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // 1. build the hmac hash context from rctx by using hash rctx switch API
    hash_switch = (HMAC_RCTX_2_LCTX == type) ? HASH_RCTX_2_LCTX : HASH_LCTX_2_RCTX;
    ret = cpt_hash_rctx_switch_dma_ctx(rctx_addr, lctx->hash_dma_ctx, hash_switch);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (HMAC_RCTX_2_LCTX == type)) {
        // 2. if the type is HMAC_RCTX_2_LCTX, rebuild key state (K0/key_len_flag/sp_key) for DMA context
        bool_t is_sp_key;
        uint8_t key_buf[HMAC_K_BUF_SZ];
        uint8_t *key_ptr;
        uint32_t key_sz;
        uint8_t sp_key_idx = 0U;
        ret = hmac_get_key(hmac_cmd_data, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_idx);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            is_sp_key = (sp_key_idx == 0U) ? false : true;
            ret = cpt_build_hmac_dma_ctx_without_hash(key_ptr, key_sz, sp_key_idx, is_sp_key, lctx);
        }
        util_memset(key_buf, 0, sizeof(key_buf));
    }

    return ret;
}

/**
 *   @brief      Check if the request could be executed in DMA mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] mode
 *   @param [in] input_msg_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_check_dma_mode(raddr_t ctx_buf_addr, hmac_mode_e *mode, uint32_t input_msg_sz)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level;
    hmac_rctx_st rctx[1];
    const uint8_t *map_addr = NULL;

    level = cpu_enter_critical();
    map_addr = mmap_remap_addr_u64(ctx_buf_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(hmac_rctx_st));
        // No data cache in hash_buffer and the request data size is aligned to block size
        if ((0U == (rctx->hash_rctx.total[0] % rctx->hash_rctx.block_byte_len))
            && (0U == (input_msg_sz % rctx->hash_rctx.block_byte_len))) {
            *mode = HMAC_DMA_MODE;
        } else {
            *mode = HMAC_CPU_MODE;
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Check and return the mac size
 *
 *   @param [in] dir
 *   @param [in] alg
 *   @param [in] mb_mac_sz
 *   @param [in] hmac_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_check_mac_sz(uint8_t dir, cpt_hash_alg_e alg, uint32_t mb_mac_sz, uint32_t *hmac_sz)
{
    uint32_t ret;
    uint32_t std_sz = ((uint32_t)hash_get_digest_word_len(alg) << 2);
    if (MB_MAC_GEN == dir) {
        // Make sure the output buffer is enough for the digest
        if (mb_mac_sz >= std_sz) {
            *hmac_sz = std_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        } else if (mb_mac_sz >= HMAC_SRV_MAC_BYTE_LEN_MIN) {
            *hmac_sz = mb_mac_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    } else {
        /*
         * The length of verified MAC could not less then HMAC_SRV_MAC_BYTE_LEN_MIN,
         * greater than block size
         */
        if ((mb_mac_sz < HMAC_SRV_MAC_BYTE_LEN_MIN) || (mb_mac_sz > std_sz)) {
            ret = EHSM_ERR_MAC_LEN_WRONG_FORMAT;
        } else {
            *hmac_sz = mb_mac_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    return ret;
}

/**
 *   @brief      Hmac update service in DMA mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] hmac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_dma_update(
    raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t msg_sz, const mb_cmd_hmac_st *hmac_cmd_data)
{
    uint32_t ret;
    cpt_hmac_dma_ctx_st dma_ctx;
    uint32_t msg_l = (uint32_t)msg_addr;
    uint32_t msg_h = (uint32_t)(msg_addr >> 32);
    ret = hmac_rctx_switch_dma_ctx(ctx_buf_addr, hmac_cmd_data, &dma_ctx, HMAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hmac_dma_update_blocks(&dma_ctx, msg_h, msg_l, msg_sz, SOC_READ_SOC_WRITE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = hmac_rctx_switch_dma_ctx(ctx_buf_addr, hmac_cmd_data, &dma_ctx, HMAC_LCTX_2_RCTX);
        }
    }
    return ret;
}

/**
 *   @brief      Hmac internal update service in CPU mode
 *
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] cpu_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_cpu_update_internal(raddr_t msg_addr, uint32_t msg_sz, cpt_hmac_ctx_st *cpu_ctx)
{
    uint32_t ret;
    uint32_t once_sz;
    uint32_t buf_block_sz;
    raddr_t remain_msg_addr = msg_addr;
    uint32_t remain_sz = msg_sz;
    uint8_t local_msg[HMAC_SRV_DATA_BUF_BYTE_SIZE];

    // When calcation's size is aligned to block size, the calling will be most effeicient
    buf_block_sz = (sizeof(local_msg) / cpu_ctx->hash_ctx->block_byte_len) * cpu_ctx->hash_ctx->block_byte_len;
    do {
        once_sz = (remain_sz > buf_block_sz) ? buf_block_sz : remain_sz;
        ret = mmap_read_remote_data(local_msg, remain_msg_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hmac_update(cpu_ctx, local_msg, once_sz);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            remain_msg_addr += (uint64_t)once_sz;
            remain_sz -= once_sz;
        } else {
            remain_sz = 0U;
        }
    } while (remain_sz > 0U);
    return ret;
}

/**
 *   @brief      Hmac update service in CPU mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] key_handle
 *   @param [in] dir
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_cpu_update(
    raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t msg_sz, const mb_cmd_hmac_st *hmac_cmd_data)
{
    uint32_t ret;
    cpt_hmac_ctx_st cpu_ctx;
    ret = hmac_rctx_switch_cpu_ctx(ctx_buf_addr, hmac_cmd_data, &cpu_ctx, HMAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = hmac_srv_cpu_update_internal(msg_addr, msg_sz, &cpu_ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = hmac_rctx_switch_cpu_ctx(ctx_buf_addr, hmac_cmd_data, &cpu_ctx, HMAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      Hmac final service in CPU mode
 *
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] cpu_ctx
 *   @param [in] mac
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_cpu_final(raddr_t msg_addr, uint32_t msg_sz, cpt_hmac_ctx_st *cpu_ctx, uint8_t *mac)
{
    uint32_t ret;
    if (msg_sz > 0U) {
        ret = hmac_srv_cpu_update_internal(msg_addr, msg_sz, cpu_ctx);
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hmac_final(cpu_ctx, mac);
    }
    return ret;
}

/**
 *   @brief      Hmac stepwise initialization service
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_stepwise_init(const mb_cmd_hmac_st *hmac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_hmac_ctx_st cpu_ctx;
    uint8_t key_buf[HMAC_K_BUF_SZ];
    uint32_t key_sz;
    uint8_t *key_ptr = NULL;
    uint8_t sp_key_idx = 0U;
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    cpt_hash_alg_e alg = get_lib_hash_alg(hmac_cmd_data->algorithm);
    ret = hmac_get_key(hmac_cmd_data, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_idx);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // The init service will use CPU mode, which could provide the complete ctx of hmac_rctx_st
        ret = cpt_hmac_init(&cpu_ctx, alg, key_ptr, sp_key_idx, key_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = hmac_rctx_switch_cpu_ctx(hmac_cmd_data->hmac_ctx, hmac_cmd_data, &cpu_ctx, HMAC_LCTX_2_RCTX);
    }
    util_memset(key_buf, 0, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Hmac stepwise update service
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_stepwise_update(const mb_cmd_hmac_st *hmac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    hmac_mode_e mode;
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    ret = hmac_check_dma_mode(hmac_cmd_data->hmac_ctx, &mode, hmac_cmd_data->data_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (HMAC_DMA_MODE == mode) {
            /*
             * If the buffer in HOST ctx contains any data or the request data size is not aligned to block size,
             * the update service will choose CPU mode
             */
            ret = hmac_srv_dma_update(
                hmac_cmd_data->hmac_ctx, hmac_cmd_data->data, hmac_cmd_data->data_size, hmac_cmd_data);
        } else {
            ret = hmac_srv_cpu_update(
                hmac_cmd_data->hmac_ctx, hmac_cmd_data->data, hmac_cmd_data->data_size, hmac_cmd_data);
        }
    }
    return ret;
}

/**
 *   @brief      Hmac stepwise final service
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_stepwise_final(const mb_cmd_hmac_st *hmac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_hmac_ctx_st cpu_ctx;
    cpt_hash_alg_e alg;
    uint32_t mac_sz;
    uint8_t local_mac[HMAC_SRV_MAC_BYTE_LEN_MAX];
    uint8_t expect_mac[HMAC_SRV_MAC_BYTE_LEN_MAX];
    // hmac generation will reset the rsp_data_len
    rsp_data->rsp_data_len = 0U;
    ret = hmac_rctx_switch_cpu_ctx(hmac_cmd_data->hmac_ctx, hmac_cmd_data, &cpu_ctx, HMAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        alg = cpu_ctx.hash_ctx->alg;
        ret = hmac_check_mac_sz(hmac_cmd_data->direction, alg, hmac_cmd_data->digest_size, &mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        /*
         *  The hmac final service will use CPU mode directly,
         *  because most 3rd-party API will not give an input in final stage
         */
        ret = hmac_cpu_final(hmac_cmd_data->data, hmac_cmd_data->data_size, &cpu_ctx, local_mac);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_MAC_GEN == hmac_cmd_data->direction) {
            ret = mmap_write_remote_data(hmac_cmd_data->digest, local_mac, mac_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = mac_sz;
                rsp_data->rsp_data_len += sizeof(mac_sz);
            }
        } else {
            // No output data
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            ret = mmap_read_remote_data(expect_mac, hmac_cmd_data->digest, mac_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = util_memcmp(expect_mac, local_mac, mac_sz);
                if (0U == ret) {
                    rsp_data->data[1] = 0U;
                } else {
                    rsp_data->data[1] = EHSM_ERR_MAC_VRY_FAILED;
                }
                rsp_data->rsp_data_len += sizeof(uint32_t);
                /*
                 * The result of verification will stored in response data, and the return code will only indicate
                 * that if the process of calculation successfully work
                 */
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
    }
    return ret;
}

/**
 *   @brief      Hmac stepwise service handler
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_stepwise_handler(const mb_cmd_hmac_st *hmac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (hmac_cmd_data->process_mode) {
    case MB_START: {
        ret = hmac_srv_stepwise_init(hmac_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = hmac_srv_stepwise_update(hmac_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = hmac_srv_stepwise_init(hmac_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = hmac_srv_stepwise_update(hmac_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = hmac_srv_stepwise_final(hmac_cmd_data, rsp_data);
        break;
    }
    default: {
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }
    }
    return ret;
}

/**
 *   @brief      Hmac single-call service handler
 *
 *   @param [in] hmac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hmac_srv_onepass(const mb_cmd_hmac_st *hmac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t expect_mac[HMAC_SRV_MAC_BYTE_LEN_MAX];
    uint8_t local_mac[HMAC_SRV_MAC_BYTE_LEN_MAX];
    uint32_t mac_h;
    uint32_t mac_l;
    uint32_t msg_h = (uint32_t)(hmac_cmd_data->data >> 32);
    uint32_t msg_l = (uint32_t)hmac_cmd_data->data;
    uint32_t msg_sz = hmac_cmd_data->data_size;
    uint32_t mac_sz;
    uint8_t key_buf[KMS_SYM_KEY_DATA_MAX_SIZE];
    uint32_t key_sz;
    uint8_t *key_ptr = NULL;
    uint8_t sp_key_idx = 0U;
    cpt_hash_alg_e alg = get_lib_hash_alg(hmac_cmd_data->algorithm);
    rsp_data->rsp_data_len = 0U;
    ret = hmac_check_mac_sz(hmac_cmd_data->direction, alg, hmac_cmd_data->digest_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = hmac_get_key(hmac_cmd_data, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_idx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_MAC_GEN == hmac_cmd_data->direction) {
            mac_h = 0U;
            // Since the AHB-DMA could not access the RAM, it will use the system general register to store the result
            // temporarily, then copy exactly mac_sz bytes to host to support truncated output
            mac_l = SYS_GEN_REG;
            ret = cpt_hmac_dma(
                alg, key_ptr, sp_key_idx, key_sz, msg_h, msg_l, msg_sz, mac_h, mac_l, NULL, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // SYS_GEN_REG does not support byte read/write, use 4-byte aligned copy
                util_memcpy(local_mac, (uint8_t *)mac_l, (mac_sz + 3U) & ~3U);
                ret = mmap_write_remote_data(hmac_cmd_data->digest, local_mac, mac_sz);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = mac_sz;
                rsp_data->rsp_data_len += sizeof(mac_sz);
            }
        } else {
            // No output data
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);

            mac_h = 0U;
            // Since the AHB-DMA could not access the RAM, it will use the system general register to store the result
            // temporarily
            mac_l = SYS_GEN_REG;
            ret = mmap_read_remote_data(expect_mac, hmac_cmd_data->digest, mac_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hmac_dma(
                    alg, key_ptr, sp_key_idx, key_sz, msg_h, msg_l, msg_sz, mac_h, mac_l, NULL, EHSM_WRITE_SOC_READ);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // The system genral register is not support byte read/write operation, which is implemented in
                // util_memcmp()
                util_memcpy(local_mac, (uint8_t *)mac_l, (mac_sz + 3U) & ~3U);
                ret = util_memcmp(local_mac, expect_mac, mac_sz);
                if (0U == ret) {
                    rsp_data->data[1] = 0U;
                } else {
                    rsp_data->data[1] = EHSM_ERR_MAC_VRY_FAILED;
                }
                rsp_data->rsp_data_len += sizeof(uint32_t);
                /*
                 * The result of verification will stored in response data, and the return code will only indicate
                 * that if the process of calculation successfully work
                 */
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
    }
    util_memset(key_buf, 0, sizeof(key_buf));
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t hmac_srv_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t hmac_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_hmac_st *hmac_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        hmac_cmd_data = (const mb_cmd_hmac_st *)(req_data);
        ret = cmd_check_direction(hmac_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (HASH_INVALID_ALG == get_lib_hash_alg(hmac_cmd_data->algorithm)) {
                ret = EHSM_ERR_INVALID_ALGORITHM;
            } else {
                ret = cmd_check_process_mode(hmac_cmd_data->process_mode);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (MB_ONE_PASS == hmac_cmd_data->process_mode) {
                    ret = hmac_srv_onepass(hmac_cmd_data, rsp_data);
                } else {
                    ret = hmac_srv_stepwise_handler(hmac_cmd_data, rsp_data);
                }
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
