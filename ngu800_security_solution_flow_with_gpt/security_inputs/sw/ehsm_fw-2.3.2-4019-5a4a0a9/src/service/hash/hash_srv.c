/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "hash_srv.h"
#include "mb.h"
#include "mmap.h"
#include "cpu_porting.h"
#include "component/util.h"
#include "crypto_util.h"
#include "component/crypto_api.h"
#include "driver/sysreg.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

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
static uint32_t hash_check_dma_mode(raddr_t ctx_buf_addr, hash_mode_e *mode, uint32_t input_msg_sz)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level;
    cpt_hash_rctx_st rctx[1];
    // Disable global interrupt
    level = cpu_enter_critical();
    const uint8_t *map_addr = mmap_remap_addr_u64(ctx_buf_addr);

    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_hash_rctx_st));
        // No cache in hash_buffer and the request data size is aligned to block size
        if ((0U == (rctx->total[0] % rctx->block_byte_len)) && (0U == (input_msg_sz % rctx->block_byte_len))) {
            *mode = HASH_DMA_MODE;
        } else {
            *mode = HASH_CPU_MODE;
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    // Restore the value of global interrupt configuration
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Hash update service in CPU mode
 *
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] cpu_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_cpu_update(raddr_t msg_addr, uint32_t msg_sz, cpt_hash_ctx_st *cpu_ctx)
{
    uint32_t ret;
    uint32_t once_sz;
    uint32_t buf_block_sz;
    raddr_t remain_msg_addr = msg_addr;
    uint32_t remain_sz = msg_sz;
    uint8_t local_msg[HASH_SRV_DATA_BUF_BYTE_SIZE];

    // When calcation's size is aligned to block size, the calling will be most effeicient
    buf_block_sz = (sizeof(local_msg) / cpu_ctx->block_byte_len) * cpu_ctx->block_byte_len;
    do {
        once_sz = (remain_sz > buf_block_sz) ? buf_block_sz : remain_sz;
        ret = mmap_read_remote_data(local_msg, remain_msg_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(cpu_ctx, local_msg, once_sz);
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
 *   @brief      Hash final service in CPU mode
 *
 *   @param [in] msg_addr
 *   @param [in] msg_sz
 *   @param [in] cpu_ctx
 *   @param [in] digest
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_cpu_final(raddr_t msg_addr, uint32_t msg_sz, cpt_hash_ctx_st *cpu_ctx, uint8_t *digest)
{
    uint32_t ret;
    if (msg_sz > 0U) {
        ret = hash_cpu_update(msg_addr, msg_sz, cpu_ctx);
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_final(cpu_ctx, digest);
    }
    return ret;
}

/**
 *   @brief      Hash stepwise initialization service
 *
 *   @param [in] hash_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_srv_stepwise_init(const mb_cmd_hash_st *hash_cmd_data, cmd_rsp_data_st *rsp_data)
{
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    cpt_hash_alg_e alg = get_lib_hash_alg(hash_cmd_data->algorithm);
    return hash_srv_general_init(hash_cmd_data->hash_ctx, alg);
}

/**
 *   @brief      Hash stepwise update service
 *
 *   @param [in] hash_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_srv_stepwise_update(const mb_cmd_hash_st *hash_cmd_data, cmd_rsp_data_st *rsp_data)
{
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    return hash_srv_general_update(hash_cmd_data->hash_ctx, hash_cmd_data->data, hash_cmd_data->data_size);
}

/**
 *   @brief      Hash stepwise final service
 *
 *   @param [in] hash_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_srv_stepwise_final(const mb_cmd_hash_st *hash_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint32_t digest_sz;
    uint8_t local_digest[HASH_BLOCK_MAX_BYTE_LEN];
    /*
     *  The hash final service will use CPU mode directly,
     *  because most 3rd-party API will not give an input in final stage
     */
    ret = hash_srv_general_final(
        hash_cmd_data->hash_ctx, hash_cmd_data->data, hash_cmd_data->data_size, local_digest, &digest_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Make sure the output buffer is enough for the digest
        if (hash_cmd_data->digest_size >= digest_sz) {
            ret = mmap_write_remote_data(hash_cmd_data->digest, local_digest, digest_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = digest_sz;
                rsp_data->rsp_data_len = sizeof(digest_sz);
            }
        } else {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    }
    return ret;
}

/**
 *   @brief      Hash stepwise service handler
 *
 *   @param [in] hash_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_srv_stepwise_handler(const mb_cmd_hash_st *hash_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (hash_cmd_data->process_mode) {
    case MB_START: {
        ret = hash_srv_stepwise_init(hash_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = hash_srv_stepwise_update(hash_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = hash_srv_stepwise_init(hash_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = hash_srv_stepwise_update(hash_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = hash_srv_stepwise_final(hash_cmd_data, rsp_data);
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
 *   @brief      Hash single-call service handler
 *
 *   @param [in] hash_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t hash_srv_onepass(const mb_cmd_hash_st *hash_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_hash_alg_e alg = get_lib_hash_alg(hash_cmd_data->algorithm);
    uint32_t output_sz = (uint32_t)hash_get_digest_word_len(alg) << 2;
    // Make sure the output buffer is enough for the digest
    if (hash_cmd_data->digest_size >= output_sz) {
        ret = hash_srv_general_onepass(alg, hash_cmd_data->data, hash_cmd_data->data_size, hash_cmd_data->digest, NULL,
            &output_sz, DIGEST_WRITE_TO_HOST);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = output_sz;
            rsp_data->rsp_data_len = sizeof(output_sz);
        }
    } else {
        ret = EHSM_ERR_OUTPUT_OVERFLOW;
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t hash_srv_using_ahb_dma(void)
{
    // The DMA relative register will be configured as AXI-DMA-read and AHB-DMA-write
    return crypto_using_ahb_dma(CRYPTO_HASH_USING_AHB_DMA_REG_VAL);
}

// The function hash_srv_release_ahb_dma() should be used with function hash_srv_using_ahb_dma()
void hash_srv_release_ahb_dma(uint32_t origin_val)
{
    crypto_release_ahb_dma(origin_val);
}

uint32_t hash_srv_cpu_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz)
{
    uint32_t ret;
    cpt_hash_ctx_st cpu_ctx;
    ret = cpt_hash_rctx_switch_cpu_ctx(ctx_buf_addr, &cpu_ctx, HASH_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = hash_cpu_update(msg_addr, input_sz, &cpu_ctx);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_rctx_switch_cpu_ctx(ctx_buf_addr, &cpu_ctx, HASH_LCTX_2_RCTX);
        }
    }
    return ret;
}

uint32_t hash_srv_dma_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz)
{
    uint32_t ret;
    cpt_hash_dma_ctx_st dma_ctx;
    uint32_t msg_l = (uint32_t)msg_addr;
    uint32_t msg_h = (uint32_t)(msg_addr >> 32);
    ret = cpt_hash_rctx_switch_dma_ctx(ctx_buf_addr, &dma_ctx, HASH_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_dma_update_blocks(&dma_ctx, msg_h, msg_l, input_sz, SOC_READ_SOC_WRITE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_rctx_switch_dma_ctx(ctx_buf_addr, &dma_ctx, HASH_LCTX_2_RCTX);
        }
    }
    return ret;
}

uint32_t hash_srv_general_init(raddr_t ctx_buf_addr, cpt_hash_alg_e alg)
{
    uint32_t ret;
    cpt_hash_ctx_st cpu_ctx;
    // The init service will use CPU mode, which could provide the complete ctx of cpt_hash_rctx_st
    ret = cpt_hash_init(&cpu_ctx, alg);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_rctx_switch_cpu_ctx(ctx_buf_addr, &cpu_ctx, HASH_LCTX_2_RCTX);
    }
    return ret;
}

uint32_t hash_srv_general_update(raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz)
{
    uint32_t ret;
    hash_mode_e mode;
    ret = hash_check_dma_mode(ctx_buf_addr, &mode, input_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        /*
         * If the buffer in HOST ctx contains any data or the request data size is not aligned to block size,
         * the update service will choose CPU mode
         */
        if (HASH_DMA_MODE == mode) {
            ret = hash_srv_dma_update(ctx_buf_addr, msg_addr, input_sz);
        } else {
            ret = hash_srv_cpu_update(ctx_buf_addr, msg_addr, input_sz);
        }
    }
    return ret;
}

uint32_t hash_srv_general_final(
    raddr_t ctx_buf_addr, raddr_t msg_addr, uint32_t input_sz, uint8_t *digest, uint32_t *digest_sz)
{
    uint32_t ret;
    cpt_hash_ctx_st cpu_ctx;
    uint32_t tmp_digest_sz = 0U;
    ret = cpt_hash_rctx_switch_cpu_ctx(ctx_buf_addr, &cpu_ctx, HASH_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Since the ctx may be cleared in the CryLib final funcion, the digest size should be kept before calling the
        // function
        tmp_digest_sz = cpu_ctx.digest_byte_len;
        // The CPU mode final API can be re-used in PKE-Srv module
        ret = hash_cpu_final(msg_addr, input_sz, &cpu_ctx, digest);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        *digest_sz = tmp_digest_sz;
    }
    return ret;
}

uint32_t hash_srv_general_onepass(cpt_hash_alg_e alg, raddr_t msg_addr, uint32_t input_sz, raddr_t digest_raddr,
    uint8_t *digest_lptr, uint32_t *digest_sz, hash_output_mode_e mode)
{
    uint32_t ret;
    uint32_t msg_l = (uint32_t)msg_addr;
    uint32_t msg_h = (uint32_t)(msg_addr >> 32);
    uint32_t rdigest_l;
    uint32_t rdigest_h;
    uint32_t output_sz;
    // Consider the efficience of single-call service, it will use DMA mode
    if (0U == msg_addr) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else if (DIGEST_KEEP_IN_LOCAL == mode) {
        if ((NULL != digest_lptr) && (digest_sz != NULL)) {
            output_sz = (uint32_t)hash_get_digest_word_len(alg) << 2;
            /*
             * The DMA update service will use the system general register, which can be accessed by AHB-DMA,
             * to store the digest temporarily
             */
            ret = cpt_hash_dma(alg, msg_h, msg_l, input_sz, 0U, SYS_GEN_REG, NULL, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                util_memcpy(digest_lptr, (uint8_t *)SYS_GEN_REG, output_sz);
                *digest_sz = output_sz;
            }
        } else {
            ret = EHSM_ERR_INVALID_ADDRESS;
        }
    } else {
        rdigest_l = (uint32_t)digest_raddr;
        rdigest_h = (uint32_t)(digest_raddr >> 32);
        ret = cpt_hash_dma(alg, msg_h, msg_l, input_sz, rdigest_h, rdigest_l, NULL, SOC_READ_SOC_WRITE);
    }
    return ret;
}

uint32_t hash_srv_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t hash_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_hash_st *hash_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        hash_cmd_data = (const mb_cmd_hash_st *)(req_data);
        if (HASH_INVALID_ALG == get_lib_hash_alg(hash_cmd_data->algorithm)) {
            ret = EHSM_ERR_INVALID_ALGORITHM;
        } else {
            ret = cmd_check_process_mode(hash_cmd_data->process_mode);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_ONE_PASS == hash_cmd_data->process_mode) {
                ret = hash_srv_onepass(hash_cmd_data, rsp_data);
            } else {
                ret = hash_srv_stepwise_handler(hash_cmd_data, rsp_data);
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}
