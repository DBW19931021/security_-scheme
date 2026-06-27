/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mac_srv.h"
#include "mb.h"
#include "service/crypto_util.h"
#include "kms.h"
#include "mmap.h"
#include "cpu_porting.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// The max data length in CPU mode service
#define MAC_CPU_MODE_BUF_SZ (512U)

#define MAC_KEY_BUF_MAX (32U)

#define MAC_VALUE_BUF_MAX (16U)

// The minium length of verified MAC value
#define MAC_SRV_MAC_BYTE_LEN_MIN (8U)

// GMAC only support 12 bytes IV length
#define GMAC_IV_LEN (12U)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t (*mac_srv_init)(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*mac_srv_update)(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*mac_srv_final)(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*mac_srv_onepass)(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data);
} ske_mac_srv_hdl_st;

typedef enum {
    MAC_DMA_MODE = 0U,
    MAC_CPU_MODE = 1U,
} mac_mode_e;
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static ske_mac_srv_hdl_st cmac_hdl;

static ske_mac_srv_hdl_st cbcmac_hdl;

static ske_mac_srv_hdl_st gmac_hdl;

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
 *   @brief      Get the MAC generation/verfication handler
 *
 *   @param [in] cipher_mode
 *   @param [in] srv_hdl
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_get_hdl(uint8_t cipher_mode, ske_mac_srv_hdl_st **srv_hdl)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    switch (cipher_mode) {
    case MB_MAC_CIPHER_MODE_CMAC: {
        *srv_hdl = &cmac_hdl;
        break;
    }
    case MB_MAC_CIPHER_MODE_CBC_MAC: {
        *srv_hdl = &cbcmac_hdl;
        break;
    }
    case MB_MAC_CIPHER_MODE_GMAC: {
        *srv_hdl = &gmac_hdl;
        break;
    }
    default: {
        ret = EHSM_ERR_INVALID_CIPHER_MODE;
        break;
    }
    }
    return ret;
}

/**
 *   @brief      Check the parameter of MAC generation/verfication service
 *
 *   @param [in] dir
 *   @param [in] alg
 *   @param [in] mb_mac_sz
 *   @param [in] mac_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_check_mac_sz(uint8_t dir, cpt_ske_alg_e alg, uint32_t mb_mac_sz, uint8_t *mac_sz)
{
    uint32_t ret;
    uint8_t std_sz = cpt_ske_get_block_byte_len(alg);
    if (MB_MAC_GEN == dir) {
        // Make sure the output buffer size is greater than block size
        if (mb_mac_sz >= std_sz) {
            *mac_sz = std_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    } else {
        // The size of verified MAC should not less than MAC_SRV_MAC_BYTE_LEN_MIN
        if ((mb_mac_sz > std_sz) || (mb_mac_sz < MAC_SRV_MAC_BYTE_LEN_MIN)) {
            ret = EHSM_ERR_MAC_LEN_WRONG_FORMAT;
        } else {
            *mac_sz = (uint8_t)mb_mac_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    return ret;
}

/**
 *   @brief      Get the key for  MAC generation/verfication service
 *
 *   @param [in] key_handle
 *   @param [in] kms_alg
 *   @param [in] dir
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] key
 *   @param [in] key_sz
 *   @param [in] sp_key_id
 *   @param [in] spk_sel
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_get_key_from_key_handle(uint32_t key_handle, uint32_t kms_alg, uint8_t dir, uint8_t *key_buf,
    uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    uint8_t req_kpart;
    kms_keydata_st keydata;

    if (MB_MAC_GEN == dir) {
        check_usage_bits |= KEY_USAGE_SIGN;
    } else {
        check_usage_bits |= KEY_USAGE_VERIFY;
    }
    req_kpart = KMS_KEY_PART_PRIVKEY;

    ret = kms_read_key(key_handle, key_buf, key_buf_sz, &keydata, req_kpart, check_usage_bits);

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (kms_alg == keydata.algo_id) {
            if (KMS_KEY_SRC_TYPE_RAM == keydata.src_type) {
                *key = keydata.keypair.symm.key;
                *key_sz = keydata.keypair.symm.size;
                *spk_sel = false;
            }
            // Secure port key
            else {
                if (keydata.keypair.symm.size <= SYM_SP_KEY_MAZ_SZ) {
                    *key = NULL;
                    *sp_key_id = keydata.sp_key_id;
                    *spk_sel = true;
                } else {
                    ret = EHSM_ERR_NOT_SUPPORT;
                }
            }
        } else {
            ret = EHSM_ERR_INVALID_HANDLE;
        }
    }
    return ret;
}

/**
 *   @brief      Get the key for  MAC generation/verfication service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] kms_alg
 *   @param [in] dir
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] key
 *   @param [in] key_sz
 *   @param [in] sp_key_id
 *   @param [in] spk_sel
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_get_key(const mb_cmd_mac_st *mac_cmd_data, uint32_t kms_alg, uint8_t dir, uint8_t *key_buf,
    uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret;

    switch (mac_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        *sp_key_id = 0U;
        *spk_sel = false;
        if (mac_cmd_data->plain_key_size > key_buf_sz) {
            ret = EHSM_ERR_NOT_SUPPORT;
        } else {
            ret = mmap_read_remote_data(key_buf, mac_cmd_data->plain_key_addr, mac_cmd_data->plain_key_size);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *key = key_buf;
                *key_sz = mac_cmd_data->plain_key_size;
            }
        }
        break;
    case MB_KEY_HANDLE:
        ret = mac_get_key_from_key_handle(
            mac_cmd_data->key_handle, kms_alg, dir, key_buf, key_buf_sz, key, key_sz, sp_key_id, spk_sel);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Recovery the key information in context struct
 *
 *   @param [in] ske_ctx
 *   @param [in] key_handle
 *   @param [in] dir
 *   @param [in] alg
 *   @param [in] cipher_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_ctx_recovery_key(
    cpt_ske_ctx_st *ske_ctx, const mb_cmd_mac_st *mac_cmd_data, uint8_t dir, uint8_t alg, uint8_t cipher_mode)
{
    uint32_t ret;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint32_t kms_alg = get_kms_ske_alg(alg, cipher_mode);
    // Using the key fetch API to get the key again, which may reduce the effience of function-call
    ret = mac_get_key(mac_cmd_data, kms_alg, dir, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_set_ske_ctx_key(ske_ctx, spk_sel, key_ptr, sp_key_id, (uint8_t)key_sz);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      CMAC context switch function between remote context and local DMA context
 *
 *   @param [in] mac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_rctx_switch_dma_ctx(
    const mb_cmd_mac_st *mac_cmd_data, cpt_ske_cmac_dma_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_ctx_st *ske_ctx = cpt_get_cmac_dma_ske_ctx(lctx);

    ret = cpt_cmac_rctx_switch_dma_ctx(mac_cmd_data->mac_ctx, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MAC_RCTX_2_LCTX == type)) {
        ret = mac_ctx_recovery_key(
            ske_ctx, mac_cmd_data, mac_cmd_data->direction, mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    }

    return ret;
}

/**
 *   @brief      CMAC context switch function between remote context and local CPU context
 *
 *   @param [in] mac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_rctx_switch_cpu_ctx(
    const mb_cmd_mac_st *mac_cmd_data, cpt_ske_cmac_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_ctx_st *ske_ctx = cpt_get_cmac_cpu_ske_ctx(lctx);

    ret = cpt_cmac_rctx_switch_cpu_ctx(mac_cmd_data->mac_ctx, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MAC_RCTX_2_LCTX == type)) {
        ret = mac_ctx_recovery_key(
            ske_ctx, mac_cmd_data, mac_cmd_data->direction, mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    }

    return ret;
}

/**
 *   @brief      Check if the CMAC relative request could be dealed in DMA mode
 *
 *   @param [in] rctx_addr
 *   @param [in] input_sz
 *   @param [in] mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_check_dma_support(raddr_t rctx_addr, uint32_t input_sz, mac_mode_e *mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_cmac_ctx_st rctx[1];
    uint32_t level = cpu_enter_critical();
    const uint8_t *map_addr = mmap_remap_addr_u64(rctx_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_ske_cmac_ctx_st));
        // No unsolved data kept in buffer and the request data size is aligned to block size
        if ((0U == rctx->left_bytes) && (0U == (input_sz % rctx->ske_cmac_ctx[0].block_bytes))) {
            *mode = MAC_DMA_MODE;
        } else {
            *mode = MAC_CPU_MODE;
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      CMAC CPU mode internal update API
 *
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_cpu_update_internal(raddr_t input_addr, uint32_t input_sz, cpt_ske_cmac_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_data[MAC_CPU_MODE_BUF_SZ];
    uint32_t total_sz = input_sz;
    raddr_t tmp_addr = input_addr;
    uint32_t once_sz;
    do {
        // The size of buffer local_data has been aligned to block size
        once_sz = (total_sz > MAC_CPU_MODE_BUF_SZ) ? MAC_CPU_MODE_BUF_SZ : total_sz;
        ret = mmap_read_remote_data(local_data, tmp_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cmac_update(ctx, local_data, once_sz);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            total_sz -= once_sz;
            tmp_addr += once_sz;
        } else {
            total_sz = 0U;
        }
    } while (total_sz > 0U);
    return ret;
}

/**
 *   @brief      CMAC DMA mode internal update API
 *
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_dma_update_internal(raddr_t input_addr, uint32_t input_sz, cpt_ske_cmac_dma_ctx_st *ctx)
{
    uint32_t msg_h = (uint32_t)(input_addr >> 32);
    uint32_t msg_l = (uint32_t)input_addr;
    return cpt_ske_dma_cmac_update_blocks_excluding_last_block(ctx, msg_h, msg_l, input_sz, NULL, SOC_READ_SOC_WRITE);
}

/**
 *   @brief      CMAC CPU mode update API
 *
 *   @param [in] mac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_cpu_update(const mb_cmd_mac_st *mac_cmd_data)
{
    uint32_t ret;
    cpt_ske_cmac_ctx_st ctx;
    ret = cmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      CMAC DMA mode update API
 *
 *   @param [in] mac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_dma_update(const mb_cmd_mac_st *mac_cmd_data)
{
    uint32_t ret;
    cpt_ske_cmac_dma_ctx_st ctx;
    ret = cmac_rctx_switch_dma_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmac_dma_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmac_rctx_switch_dma_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      CMAC initialization service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_init(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cmac_ctx_st ctx;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    cpt_ske_mac_e dir = (MB_MAC_GEN == mac_cmd_data->direction) ? SKE_GENERATE_MAC : SKE_VERIFY_MAC;
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz;
    ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Using the CPU mode API for init service
        ret = cpt_ske_cmac_init(&ctx, alg, dir, key_ptr, sp_key_id, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // This flag is used for checking the total length of data in final stage
        ret = cmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      CMAC update service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_update(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    mac_mode_e mode;
    ret = cmac_check_dma_support(mac_cmd_data->mac_ctx, mac_cmd_data->data_size, &mode);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MAC_DMA_MODE == mode) {
            ret = cmac_dma_update(mac_cmd_data);
        } else {
            ret = cmac_cpu_update(mac_cmd_data);
        }
    }
    rsp_data->rsp_data_len = 0U;
    return ret;
}

/**
 *   @brief      CMAC final service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_final(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cmac_ctx_st ctx;
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint32_t mac_sz = mac_cmd_data->mac_size;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    if (mac_sz > MAC_VALUE_BUF_MAX) {
        ret = EHSM_ERR_MAC_LEN_WRONG_FORMAT;
    } else {
        ret = cmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (mac_cmd_data->data_size > 0U) {
            // Using CPU mode API to deal with the data in final service
            ret = cmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
        }
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cmac_final(&ctx, local_mac);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MB_MAC_GEN == mac_cmd_data->direction)) {
        ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = mac_sz;
            rsp_data->rsp_data_len = sizeof(mac_sz);
        }
    }
    return ret;
}

/**
 *   @brief      CMAC internal update function used in single-call function
 *
 *   @param [in] dma_ctx
 *   @param [in] cpu_ctx
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] dir
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_onepass_update_internal(
    cpt_ske_cmac_dma_ctx_st *dma_ctx, cpt_ske_cmac_ctx_st *cpu_ctx, raddr_t input_addr, uint32_t input_sz, uint8_t dir)
{
    uint32_t ret;
    uint32_t block_sz = dma_ctx->ske_cmac_ctx[0].block_bytes;
    uint32_t dma_data_sz;
    raddr_t tmp_addr = input_addr;
    uint32_t tmp_sz = input_sz;
    if (input_sz > 0U) {
        if (0U == (input_sz % block_sz)) {
            // Keep a block to calculate in CPU mode for padding
            dma_data_sz = tmp_sz - block_sz;
        } else {
            dma_data_sz = tmp_sz & (~(block_sz - 1U));
        }
        ret = cmac_dma_update_internal(input_addr, dma_data_sz, dma_ctx);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            cpt_cmac_ctx_dma_2_cpu(dma_ctx, cpu_ctx, dir);
            tmp_sz -= dma_data_sz;
            tmp_addr += (uint64_t)dma_data_sz;
            ret = cmac_cpu_update_internal(tmp_addr, tmp_sz, cpu_ctx);
        }
    } else {
        cpt_cmac_ctx_dma_2_cpu(dma_ctx, cpu_ctx, dir);
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

/**
 *   @brief      CMAC single-call service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cmac_onepass(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cmac_dma_ctx_st dma_ctx;
    cpt_ske_cmac_ctx_st cpu_ctx;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz = 0;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_cmac_init(&dma_ctx, alg, key_ptr, sp_key_id, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmac_onepass_update_internal(
            &dma_ctx, &cpu_ctx, mac_cmd_data->data, mac_cmd_data->data_size, mac_cmd_data->direction);
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cmac_final(&cpu_ctx, local_mac);
    }
    if ((MB_MAC_GEN == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = (uint32_t)mac_sz;
            rsp_data->rsp_data_len = sizeof(uint32_t);
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      CBCMAC context switch function between remote context and local DMA context
 *
 *   @param [in] mac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_rctx_switch_dma_ctx(
    const mb_cmd_mac_st *mac_cmd_data, cpt_ske_cbc_mac_dma_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_ctx_st *ske_ctx = cpt_get_cbcmac_dma_ske_ctx(lctx);

    ret = cpt_cbcmac_rctx_switch_dma_ctx(mac_cmd_data->mac_ctx, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MAC_RCTX_2_LCTX == type)) {
        ret = mac_ctx_recovery_key(
            ske_ctx, mac_cmd_data, mac_cmd_data->direction, mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    }

    return ret;
}

/**
 *   @brief      CBCMAC context switch function between remote context and local CPU context
 *
 *   @param [in] mac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_rctx_switch_cpu_ctx(
    const mb_cmd_mac_st *mac_cmd_data, cpt_ske_cbc_mac_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_ctx_st *ske_ctx = cpt_get_cbcmac_cpu_ske_ctx(lctx);

    ret = cpt_cbcmac_rctx_switch_cpu_ctx(mac_cmd_data->mac_ctx, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MAC_RCTX_2_LCTX == type)) {
        ret = mac_ctx_recovery_key(
            ske_ctx, mac_cmd_data, mac_cmd_data->direction, mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    }

    return ret;
}

/**
 *   @brief      Check if the CBCMAC relative request could be dealed in DMA mode
 *
 *   @param [in] rctx_addr
 *   @param [in] input_sz
 *   @param [in] mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_check_dma_support(raddr_t rctx_addr, uint32_t input_sz, mac_mode_e *mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_cbc_mac_st rctx[1];
    uint32_t level = cpu_enter_critical();
    const uint8_t *map_addr = mmap_remap_addr_u64(rctx_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_ske_cbc_mac_st));
        // No unsolved data kept in buffer and the request data size is aligned to block size
        if ((0U == rctx->left_bytes) && (0U == (input_sz % rctx->ske_cbc_mac_ctx[0].block_bytes))) {
            *mode = MAC_DMA_MODE;
        } else {
            *mode = MAC_CPU_MODE;
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      CBCMAC CPU mode internal update API
 *
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_cpu_update_internal(raddr_t input_addr, uint32_t input_sz, cpt_ske_cbc_mac_st *ctx)
{
    uint32_t ret;
    uint8_t local_data[MAC_CPU_MODE_BUF_SZ];
    uint32_t total_sz = input_sz;
    raddr_t tmp_addr = input_addr;
    uint32_t once_sz;
    do {
        // The size of buffer local_data has been aligned to block size
        once_sz = (total_sz > MAC_CPU_MODE_BUF_SZ) ? MAC_CPU_MODE_BUF_SZ : total_sz;
        ret = mmap_read_remote_data(local_data, tmp_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cbc_mac_update(ctx, local_data, once_sz);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            total_sz -= once_sz;
            tmp_addr += once_sz;
        } else {
            total_sz = 0U;
        }
    } while (total_sz > 0U);
    return ret;
}

/**
 *   @brief      CBCMAC DMA mode internal update API
 *
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_dma_update_internal(raddr_t input_addr, uint32_t input_sz, cpt_ske_cbc_mac_dma_st *ctx)
{
    uint32_t msg_h = (uint32_t)(input_addr >> 32);
    uint32_t msg_l = (uint32_t)input_addr;
    return cpt_ske_dma_cbc_mac_update_blocks_excluding_last_block(
        ctx, msg_h, msg_l, input_sz, NULL, SOC_READ_SOC_WRITE);
}

/**
 *   @brief      CBCMAC CPU mode update API
 *
 *   @param [in] mac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_cpu_update(const mb_cmd_mac_st *mac_cmd_data)
{
    uint32_t ret;
    cpt_ske_cbc_mac_st ctx;
    ret = cbcmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      CBCMAC DMA mode update API
 *
 *   @param [in] mac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_dma_update(const mb_cmd_mac_st *mac_cmd_data)
{
    uint32_t ret;
    cpt_ske_cbc_mac_dma_st ctx;
    ret = cbcmac_rctx_switch_dma_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_dma_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_rctx_switch_dma_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      CBCMAC initialization service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_init(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cbc_mac_st ctx;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    cpt_ske_mac_e dir = (MB_MAC_GEN == mac_cmd_data->direction) ? SKE_GENERATE_MAC : SKE_VERIFY_MAC;
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz;
    ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Using the CPU mode API for init service
        ret = cpt_ske_cbc_mac_init(&ctx, alg, dir, SKE_ZERO_PADDING, key_ptr, sp_key_id, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      CBCMAC update service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_update(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    mac_mode_e mode;
    ret = cbcmac_check_dma_support(mac_cmd_data->mac_ctx, mac_cmd_data->data_size, &mode);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MAC_DMA_MODE == mode) {
            ret = cbcmac_dma_update(mac_cmd_data);
        } else {
            ret = cbcmac_cpu_update(mac_cmd_data);
        }
    }
    rsp_data->rsp_data_len = 0U;
    return ret;
}

/**
 *   @brief      CBCMAC final service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_final(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cbc_mac_st ctx;
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint32_t mac_sz = mac_cmd_data->mac_size;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    if (mac_sz > MAC_VALUE_BUF_MAX) {
        ret = EHSM_ERR_MAC_LEN_WRONG_FORMAT;
    } else {
        ret = cbcmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (mac_cmd_data->data_size > 0U) {
            // Using CPU mode API to deal with the data in final service
            ret = cbcmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
        }
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cbc_mac_final(&ctx, local_mac);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MB_MAC_GEN == mac_cmd_data->direction)) {
        ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = mac_sz;
            rsp_data->rsp_data_len = sizeof(mac_sz);
        }
    }
    return ret;
}

/**
 *   @brief      CBCMAC internal update function used in single-call function
 *
 *   @param [in] dma_ctx
 *   @param [in] cpu_ctx
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] dir
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_onepass_update_internal(
    cpt_ske_cbc_mac_dma_st *dma_ctx, cpt_ske_cbc_mac_st *cpu_ctx, raddr_t input_addr, uint32_t input_sz, uint8_t dir)
{
    uint32_t ret;
    uint32_t block_sz = dma_ctx->ske_cbc_mac_ctx[0].block_bytes;
    uint32_t dma_data_sz;
    raddr_t tmp_addr = input_addr;
    uint32_t tmp_sz = input_sz;
    if (0U == (input_sz % block_sz)) {
        // Keep a block to calculate in CPU mode for padding
        dma_data_sz = tmp_sz - block_sz;
    } else {
        dma_data_sz = tmp_sz & (~(block_sz - 1U));
    }
    ret = cbcmac_dma_update_internal(input_addr, dma_data_sz, dma_ctx);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        cpt_cbc_mac_ctx_dma2cpu(dma_ctx, cpu_ctx, dma_data_sz, dir);
        tmp_sz -= dma_data_sz;
        tmp_addr += (uint64_t)dma_data_sz;
        ret = cbcmac_cpu_update_internal(tmp_addr, tmp_sz, cpu_ctx);
    }
    return ret;
}

/**
 *   @brief      CBCMAC single-call service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cbcmac_onepass(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_cbc_mac_dma_st dma_ctx;
    cpt_ske_cbc_mac_st cpu_ctx;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    // CBC-MAC does not support zero-length data
    if (0U == mac_cmd_data->data_size) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_cbc_mac_init(&dma_ctx, alg, key_ptr, sp_key_id, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cbcmac_onepass_update_internal(
            &dma_ctx, &cpu_ctx, mac_cmd_data->data, mac_cmd_data->data_size, mac_cmd_data->direction);
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cbc_mac_final(&cpu_ctx, local_mac);
    }
    if ((MB_MAC_GEN == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = (uint32_t)mac_sz;
            rsp_data->rsp_data_len = sizeof(uint32_t);
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Get the IV for GMAC
 *
 *   @param [in] r_iv
 *   @param [in] iv_buf
 *   @param [in] iv_ptr
 *   @param [in] iv_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_get_iv(raddr_t r_iv, uint8_t *iv_buf)
{
    uint32_t ret;
    if (0U != r_iv) {
        // The iv size should be 12 bytes
        ret = mmap_read_remote_data(iv_buf, r_iv, GMAC_IV_LEN);
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    return ret;
}

/**
 *   @brief      GMAC context switch function between remote context and local CPU context
 *
 *   @param [in] mac_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_rctx_switch_cpu_ctx(
    const mb_cmd_mac_st *mac_cmd_data, cpt_ske_gmac_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_ctx_st *ske_ctx = cpt_get_gmac_cpu_ske_ctx(lctx);

    ret = cpt_gmac_rctx_switch_cpu_ctx(mac_cmd_data->mac_ctx, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MAC_RCTX_2_LCTX == type)) {
        ret = mac_ctx_recovery_key(
            ske_ctx, mac_cmd_data, mac_cmd_data->direction, mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    }

    return ret;
}

/**
 *   @brief      GMAC CPU mode internal update API
 *
 *   @param [in] input_addr
 *   @param [in] input_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_cpu_update_internal(raddr_t input_addr, uint32_t input_sz, cpt_ske_gmac_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_data[MAC_CPU_MODE_BUF_SZ];
    uint32_t total_sz = input_sz;
    raddr_t tmp_addr = input_addr;
    uint32_t once_sz;
    do {
        // The size of buffer local_data has been aligned to block size
        once_sz = (total_sz > MAC_CPU_MODE_BUF_SZ) ? MAC_CPU_MODE_BUF_SZ : total_sz;
        ret = mmap_read_remote_data(local_data, tmp_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_gmac_update(ctx, local_data, once_sz);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            total_sz -= once_sz;
            tmp_addr += once_sz;
        } else {
            total_sz = 0U;
        }
    } while (total_sz > 0U);
    return ret;
}

/**
 *   @brief      GMAC CPU mode update API
 *
 *   @param [in] mac_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_cpu_update(const mb_cmd_mac_st *mac_cmd_data)
{
    uint32_t ret;
    cpt_ske_gmac_ctx_st ctx;
    ret = gmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    return ret;
}

/**
 *   @brief      GMAC initialization service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_init(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_gmac_ctx_st ctx;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t iv[GMAC_IV_LEN];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    cpt_ske_mac_e dir = (MB_MAC_GEN == mac_cmd_data->direction) ? SKE_GENERATE_MAC : SKE_VERIFY_MAC;
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz;
    ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_get_iv(mac_cmd_data->iv_addr, iv);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Make sure the data length is valid
        ret = cpt_ske_gmac_init(&ctx, alg, dir, key_ptr, sp_key_id, iv, GMAC_IV_LEN, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_LCTX_2_RCTX);
    }
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      GMAC update service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_update(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    rsp_data->rsp_data_len = 0U;
    // Only support CPU mode yet
    return gmac_cpu_update(mac_cmd_data);
}

/**
 *   @brief      GMAC final service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_final(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_gmac_ctx_st ctx;
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint32_t mac_sz = mac_cmd_data->mac_size;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    if (mac_sz > MAC_VALUE_BUF_MAX) {
        ret = EHSM_ERR_MAC_LEN_WRONG_FORMAT;
    } else {
        ret = gmac_rctx_switch_cpu_ctx(mac_cmd_data, &ctx, MAC_RCTX_2_LCTX);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (mac_cmd_data->data_size > 0U) {
            // Using CPU mode API to deal with the data in final service
            ret = gmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
        }
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_gmac_final(&ctx, local_mac);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MB_MAC_GEN == mac_cmd_data->direction)) {
        ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = mac_sz;
            rsp_data->rsp_data_len = sizeof(mac_sz);
        }
    }
    return ret;
}

/**
 *   @brief       GMAC single-call service
 *
 *   @param [in] mac_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gmac_onepass(const mb_cmd_mac_st *mac_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[MAC_KEY_BUF_MAX];
    uint8_t local_mac[MAC_VALUE_BUF_MAX];
    uint8_t iv[GMAC_IV_LEN];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_mac_e dir = (MB_MAC_GEN == mac_cmd_data->direction) ? SKE_GENERATE_MAC : SKE_VERIFY_MAC;
    cpt_ske_alg_e alg = get_ske_alg(mac_cmd_data->algorithm);
    uint32_t kms_alg = get_kms_ske_alg(mac_cmd_data->algorithm, mac_cmd_data->cipher_mode);
    uint8_t mac_sz;
    cpt_ske_gmac_ctx_st ctx;
    // Only mac-generation will response the size of generated mac value
    rsp_data->rsp_data_len = 0U;
    ret = mac_check_mac_sz(mac_cmd_data->direction, alg, mac_cmd_data->mac_size, &mac_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mac_get_key(mac_cmd_data, kms_alg, mac_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr, &key_sz,
            &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_get_iv(mac_cmd_data->iv_addr, iv);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_gmac_init(&ctx, alg, dir, key_ptr, sp_key_id, iv, GMAC_IV_LEN, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gmac_cpu_update_internal(mac_cmd_data->data, mac_cmd_data->data_size, &ctx);
    }
    if ((MB_MAC_VRY == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // No output data
        rsp_data->data[0] = 0U;
        rsp_data->rsp_data_len = sizeof(uint32_t);
        ret = mmap_read_remote_data(local_mac, mac_cmd_data->mac, mac_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // DMA mode API will write some data to HOST msg buffer, which may not be able to be written
        // It will use CPU mode API in final stage
        ret = cpt_ske_gmac_final(&ctx, local_mac);
        if ((MB_MAC_GEN == mac_cmd_data->direction) && (EHSM_ERR_SW_SUCCESS == ret)) {
            ret = mmap_write_remote_data(mac_cmd_data->mac, local_mac, mac_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = (uint32_t)mac_sz;
                rsp_data->rsp_data_len = sizeof(uint32_t);
            }
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      MAC service with single-call mode
 *
 *   @param [in] mac_cmd_data
 *   @param [in] srv_hdl
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_srv_onepass(
    const mb_cmd_mac_st *mac_cmd_data, const ske_mac_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = srv_hdl->mac_srv_onepass(mac_cmd_data, rsp_data);
    if (MB_MAC_VRY == mac_cmd_data->direction) {
        if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
            rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0U : EHSM_ERR_MAC_VRY_FAILED;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            /*
             * The result of verification will stored in response data, and the return code will only indicate
             * that if the process of calculation successfully work
             */
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    return ret;
}

/**
 *   @brief      MAC service with stepwise mode
 *
 *   @param [in] mac_cmd_data
 *   @param [in] srv_hdl
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t mac_srv_stepwise_handler(
    const mb_cmd_mac_st *mac_cmd_data, const ske_mac_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (mac_cmd_data->process_mode) {
    case MB_START: {
        ret = srv_hdl->mac_srv_init(mac_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = srv_hdl->mac_srv_update(mac_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = srv_hdl->mac_srv_init(mac_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = srv_hdl->mac_srv_update(mac_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = srv_hdl->mac_srv_final(mac_cmd_data, rsp_data);
        if (MB_MAC_VRY == mac_cmd_data->direction) {
            if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
                rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0U : EHSM_ERR_MAC_VRY_FAILED;
                rsp_data->rsp_data_len += sizeof(uint32_t);
                /*
                 * The result of verification will stored in response data, and the return code will only indicate
                 * that if the process of calculation successfully work
                 */
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
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
 *   @brief      CMAC handler initialization function
 *
 *
 *
 *   @note
 */
static void cmac_hdl_init(void)
{
    cmac_hdl.mac_srv_init = cmac_init;
    cmac_hdl.mac_srv_update = cmac_update;
    cmac_hdl.mac_srv_final = cmac_final;
    cmac_hdl.mac_srv_onepass = cmac_onepass;
}

/**
 *   @brief      CBCMAC handler initialization function
 *
 *
 *
 *   @note
 */
static void cbcmac_hdl_init(void)
{
    cbcmac_hdl.mac_srv_init = cbcmac_init;
    cbcmac_hdl.mac_srv_update = cbcmac_update;
    cbcmac_hdl.mac_srv_final = cbcmac_final;
    cbcmac_hdl.mac_srv_onepass = cbcmac_onepass;
}

/**
 *   @brief      GMAC handler initialization function
 *
 *
 *
 *   @note
 */
static void gmac_hdl_init(void)
{
    gmac_hdl.mac_srv_init = gmac_init;
    gmac_hdl.mac_srv_update = gmac_update;
    gmac_hdl.mac_srv_final = gmac_final;
    gmac_hdl.mac_srv_onepass = gmac_onepass;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t ske_srv_mac_init(void)
{
    // Initialize all the handlers of MAC service
    cmac_hdl_init();
    cbcmac_hdl_init();
    gmac_hdl_init();
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t ske_srv_mac_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_mac_st *mac_cmd_data;
    ske_mac_srv_hdl_st *srv_hdl;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        mac_cmd_data = (const mb_cmd_mac_st *)req_data;
        ret = cmd_check_direction(mac_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode(mac_cmd_data->process_mode);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mac_get_hdl(mac_cmd_data->cipher_mode, &srv_hdl);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_ONE_PASS == mac_cmd_data->process_mode) {
                ret = mac_srv_onepass(mac_cmd_data, srv_hdl, rsp_data);
            } else {
                ret = mac_srv_stepwise_handler(mac_cmd_data, srv_hdl, rsp_data);
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
