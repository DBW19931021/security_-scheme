/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "aead_srv.h"
#include "types.h"
#include "mmap.h"
#include "kms.h"
#include "mb.h"
#include "crypto_util.h"
#include "cpu_porting.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define SKE_SRV_AAD_BUF_SZ (128U)

// The data buffer will only be used to process the last block
#define SKE_SRV_AEAD_DATA_BUF_SZ (16U)

#define SKE_SRV_AEAD_KEY_BUF_SZ (32U)

#define SKE_SRV_AEAD_IV_BUF_SZ (32U)
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
 *   @brief      Getting the key algorithem in AEAD service
 *
 *   @param [in] alg
 *
 *   @return     uint8_t
 *
 *   @note
 */
static uint8_t aead_get_kms_alg(uint8_t alg)
{
    uint8_t kms_alg;
    switch (alg) {
    case MB_SYMM_CIPHER_ALGORITHM_AES_128: {
        kms_alg = KMS_KEY_ALG_AES_128;
        break;
    }
    case MB_SYMM_CIPHER_ALGORITHM_AES_192: {
        kms_alg = KMS_KEY_ALG_AES_192;
        break;
    }
    case MB_SYMM_CIPHER_ALGORITHM_AES_256: {
        kms_alg = KMS_KEY_ALG_AES_256;
        break;
    }
    case MB_SYMM_CIPHER_ALGORITHM_SM4: {
        kms_alg = KMS_KEY_ALG_SM4;
        break;
    }
    default: {
        // Not support other algorithm in AEAD service
        kms_alg = KMS_KEY_ALG_END;
        break;
    }
    }
    return kms_alg;
}

/**
 *   @brief      Getting the key in AEAD service
 *
 *   @param [in] key_handle
 *   @param [in] alg
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
static uint32_t aead_get_key(uint32_t key_handle, uint8_t alg, uint8_t dir, uint8_t *key_buf, uint32_t key_buf_sz,
    uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    uint8_t req_kpart = KMS_KEY_PART_PRIVKEY;
    kms_keydata_st keydata;
    uint8_t kms_alg = aead_get_kms_alg(alg);
    if (MB_CIPHER_ENC == dir) {
        check_usage_bits |= KEY_USAGE_ENCRYPT;
    } else {
        check_usage_bits |= KEY_USAGE_DECRYPT;
    }
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
                // Only support AES 128 secure port key
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
 *   @brief      Getting the key for CCM service
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] alg
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
static uint32_t aead_ccm_get_key(const mb_cmd_aead_ccm_st *ccm_cmd_data, uint8_t alg, uint8_t dir, uint8_t *key_buf,
    uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (ccm_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        *sp_key_id = 0;
        *spk_sel = false;
        if (ccm_cmd_data->plain_key_size > key_buf_sz) {
            ret = EHSM_ERR_NOT_SUPPORT;
        } else {
            ret = mmap_read_remote_data(key_buf, ccm_cmd_data->plain_key_addr, ccm_cmd_data->plain_key_size);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *key = key_buf;
                *key_sz = ccm_cmd_data->plain_key_size;
            }
        }
        break;
    case MB_KEY_HANDLE:
        ret = aead_get_key(ccm_cmd_data->key_handle, alg, dir, key_buf, key_buf_sz, key, key_sz, sp_key_id, spk_sel);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Getting the key for GCM service
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] alg
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
static uint32_t aead_gcm_get_key(const mb_cmd_aead_gcm_st *gcm_cmd_data, uint8_t alg, uint8_t dir, uint8_t *key_buf,
    uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (gcm_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        *sp_key_id = 0;
        *spk_sel = false;
        if (gcm_cmd_data->plain_key_size > key_buf_sz) {
            ret = EHSM_ERR_NOT_SUPPORT;
        } else {
            ret = mmap_read_remote_data(key_buf, gcm_cmd_data->plain_key_addr, gcm_cmd_data->plain_key_size);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *key = key_buf;
                *key_sz = gcm_cmd_data->plain_key_size;
            }
        }
        break;
    case MB_KEY_HANDLE:
        ret = aead_get_key(gcm_cmd_data->key_handle, alg, dir, key_buf, key_buf_sz, key, key_sz, sp_key_id, spk_sel);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Recovery the key in CCM context
 *
 *   @param [in] ske_ctx
 *   @param [in] ccm_cmd_data
 *   @param [in] dir
 *   @param [in] alg
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t aead_ccm_ctx_recovery_key(
    cpt_ske_ctx_st *ske_ctx, const mb_cmd_aead_ccm_st *ccm_cmd_data, uint8_t dir, uint8_t alg)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;

    ret = aead_ccm_get_key(ccm_cmd_data, alg, dir, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_set_ske_ctx_key(ske_ctx, spk_sel, key_ptr, sp_key_id, (uint8_t)key_sz);
    }
    return ret;
}

/**
 *   @brief      Recovery the key in GCM context
 *
 *   @param [in] ske_ctx
 *   @param [in] gcm_cmd_data
 *   @param [in] dir
 *   @param [in] alg
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t aead_gcm_ctx_recovery_key(
    cpt_ske_ctx_st *ske_ctx, const mb_cmd_aead_gcm_st *gcm_cmd_data, uint8_t dir, uint8_t alg)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;

    ret = aead_gcm_get_key(gcm_cmd_data, alg, dir, key_buf, sizeof(key_buf), &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_set_ske_ctx_key(ske_ctx, spk_sel, key_ptr, sp_key_id, (uint8_t)key_sz);
    }
    return ret;
}

/**
 *   @brief      Getting the IV in AEAD service
 *
 *   @param [in] r_iv
 *   @param [in] iv_sz
 *   @param [in] iv_buf
 *   @param [in] iv_buf_sz
 *   @param [in] iv_ptr
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t aead_get_iv(raddr_t r_iv, uint32_t iv_sz, uint8_t *iv_buf, uint32_t iv_buf_sz, uint8_t **iv_ptr)
{
    uint32_t ret;
    if (iv_sz <= iv_buf_sz) {
        ret = mmap_read_remote_data(iv_buf, r_iv, iv_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            *iv_ptr = iv_buf;
        }
    } else {
        ret = EHSM_ERR_OUT_OF_MEM;
    }

    return ret;
}

/**
 *   @brief      Check the output buffer size of AEAD service
 *
 *   @param [in] process_mode
 *   @param [in] input_sz
 *   @param [in] output_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t aead_check_output_buf_sz(uint8_t process_mode, uint32_t input_sz, uint32_t output_sz)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    if (MB_START != process_mode) {
        ret = (input_sz <= output_sz) ? EHSM_ERR_SW_SUCCESS : EHSM_ERR_OUTPUT_OVERFLOW;
    }
    return ret;
}

/**
 *   @brief      GCM context switch function between remote context and local context
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_rctx_switch_ctx(
    const mb_cmd_aead_gcm_st *gcm_cmd_data, cpt_ske_gcm_ctx_st *lctx, aead_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ske_ctx_st *ske_ctx = cpt_get_gcm_ske_ctx(lctx);

    ret = cpt_gcm_rctx_switch_ctx(gcm_cmd_data->context, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (AEAD_RCTX_2_LCTX == type)) {
        ret = aead_gcm_ctx_recovery_key(ske_ctx, gcm_cmd_data, gcm_cmd_data->direction, gcm_cmd_data->algorithm);
    }

    return ret;
}

/**
 *   @brief      CCM context switch function between remote context and local context
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] lctx
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_rctx_switch_ctx(
    const mb_cmd_aead_ccm_st *ccm_cmd_data, cpt_ske_ccm_ctx_st *lctx, aead_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ske_ctx_st *ske_ctx = cpt_get_ccm_ske_ctx(lctx);

    ret = cpt_ccm_rctx_switch_ctx(ccm_cmd_data->context, lctx, type);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (AEAD_RCTX_2_LCTX == type)) {
        // Recovery the key relative information in context
        ret = aead_ccm_ctx_recovery_key(ske_ctx, ccm_cmd_data, ccm_cmd_data->direction, ccm_cmd_data->algorithm);
    }

    return ret;
}

/**
 *   @brief      GCM update associated data service
 *
 *   @param [in] raad
 *   @param [in] aad_sz
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_update_aad(raddr_t raad, cpt_ske_gcm_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_aad[SKE_SRV_AAD_BUF_SZ];
    if (ctx->aad_bytes <= sizeof(local_aad)) {
        ret = mmap_read_remote_data(local_aad, raad, ctx->aad_bytes);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_gcm_update_blocks_aad(ctx, local_aad, ctx->aad_bytes);
        }
    } else {
        ret = EHSM_ERR_OUT_OF_MEM;
    }
    return ret;
}

/**
 *   @brief      GCM DMA update internal API
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_update_payload(raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, cpt_ske_gcm_ctx_st *ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t in_h = (uint32_t)(rin_addr >> 32);
    uint32_t in_l = (uint32_t)rin_addr;
    uint32_t out_h = (uint32_t)(rout_addr >> 32);
    uint32_t out_l = (uint32_t)rout_addr;

    if ((input_sz % ctx->ske_gcm_ctx[0].block_bytes) != 0U) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        ret = cpt_ske_dma_gcm_update_blocks(ctx, in_h, in_l, input_sz, out_h, out_l, NULL, SOC_READ_SOC_WRITE);
    }

    return ret;
}

/**
 *   @brief      GCM update the last block
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_updata_last_block(raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, cpt_ske_gcm_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_in[SKE_SRV_AEAD_DATA_BUF_SZ];
    uint8_t local_out[SKE_SRV_AEAD_DATA_BUF_SZ];
    ret = mmap_read_remote_data(local_in, rin_addr, input_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_gcm_update_blocks(ctx, local_in, local_out, input_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_write_remote_data(rout_addr, local_out, input_sz);
    }
    return ret;
}

/**
 *   @brief      GCM internal final API
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] rmac
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_final_internal(
    raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, raddr_t rmac, cpt_ske_gcm_ctx_st *ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    uint8_t local_mac[SKE_SRV_AEAD_DATA_BUF_SZ];
    uint32_t mac_h = (uint32_t)(rmac >> 32);
    uint32_t mac_l = (uint32_t)rmac;
    raddr_t tmp_in_addr = rin_addr;
    raddr_t tmp_out_addr = rout_addr;
    uint32_t dma_data_sz = (input_sz & ~((uint32_t)ctx->ske_gcm_ctx[0].block_bytes - 1U));
    uint32_t last_block_sz = input_sz - dma_data_sz;
    cpt_ske_crypto_e dir = ctx->ske_gcm_ctx[0].crypto;
    // ctx->mac_bytes will be clear in cpt_ske_gcm_final()
    uint32_t mac_sz = ctx->mac_bytes;
    if (dma_data_sz > 0U) {
        // Using DMA API to deal with the aligned blocks
        ret = gcm_update_payload(tmp_in_addr, dma_data_sz, tmp_out_addr, ctx);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            tmp_in_addr += (uint64_t)dma_data_sz;
            tmp_out_addr += (uint64_t)dma_data_sz;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Using CPU API to deal with the last block
        if (last_block_sz > 0U) {
            ret = gcm_updata_last_block(tmp_in_addr, last_block_sz, tmp_out_addr, ctx);
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Generate or verify the tag
        if (last_block_sz > 0U) {
            if (SKE_CRYPTO_DECRYPT == dir) {
                ret = mmap_read_remote_data(local_mac, rmac, mac_sz);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_ske_gcm_final(ctx, local_mac);
            }
            if ((SKE_CRYPTO_ENCRYPT == dir) && (EHSM_ERR_SW_SUCCESS == ret)) {
                ret = mmap_write_remote_data(rmac, local_mac, mac_sz);
            }
        } else {
            if (0U == ctx->current_bytes) {
                if (SKE_CRYPTO_DECRYPT == dir) {
                    ret = mmap_read_remote_data(local_mac, rmac, mac_sz);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = cpt_ske_gcm_final(ctx, local_mac);
                }
                if ((SKE_CRYPTO_ENCRYPT == dir) && (EHSM_ERR_SW_SUCCESS == ret)) {
                    ret = mmap_write_remote_data(rmac, local_mac, mac_sz);
                }
            } else {
                ret = cpt_ske_dma_gcm_update_final(ctx, mac_h, mac_l, SOC_READ_SOC_WRITE);
            }
        }
    }
    return ret;
}

/**
 *   @brief      GCM init stepwise service
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_init(const mb_cmd_aead_gcm_st *gcm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t iv[SKE_SRV_AEAD_IV_BUF_SZ];
    uint8_t *iv_ptr;
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_gcm_ctx_st ctx;
    uint32_t iv_sz = gcm_cmd_data->iv_size;
    cpt_ske_alg_e ske_alg = get_ske_alg(gcm_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_ENC == gcm_cmd_data->direction) ? SKE_CRYPTO_ENCRYPT : SKE_CRYPTO_DECRYPT;
    ret = aead_gcm_get_key(gcm_cmd_data, gcm_cmd_data->algorithm, gcm_cmd_data->direction, key_buf, sizeof(key_buf),
        &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // iv only support 12 byte in GCM
        if (12U == iv_sz) {
            ret = aead_get_iv(gcm_cmd_data->iv_addr, iv_sz, iv, sizeof(iv), &iv_ptr);
        } else {
            ret = EHSM_ERR_WRONG_IV_SIZE;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // The tag_size will not greater then the max of uint8_t
        ret = cpt_ske_gcm_init(&ctx, ske_alg, crypto, key_ptr, sp_key_id, iv_ptr, iv_sz, gcm_cmd_data->aad_size,
            gcm_cmd_data->input_size, (uint8_t)gcm_cmd_data->tag_size);
    }
    if ((gcm_cmd_data->aad_size > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = gcm_update_aad(gcm_cmd_data->aad_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gcm_rctx_switch_ctx(gcm_cmd_data, &ctx, AEAD_LCTX_2_RCTX);
    }
    rsp_data->rsp_data_len = 0U;
    return ret;
}

/**
 *   @brief      GCM update stepwise service
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_update(const mb_cmd_aead_gcm_st *gcm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_gcm_ctx_st ctx;
    ret = gcm_rctx_switch_ctx(gcm_cmd_data, &ctx, AEAD_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gcm_update_payload(gcm_cmd_data->input_addr, gcm_cmd_data->input_size, gcm_cmd_data->output_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gcm_rctx_switch_ctx(gcm_cmd_data, &ctx, AEAD_LCTX_2_RCTX);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = gcm_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(gcm_cmd_data->input_size);
    }
    return ret;
}

/**
 *   @brief      GCM final stepwise service
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_final(const mb_cmd_aead_gcm_st *gcm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_gcm_ctx_st ctx;
    rsp_data->rsp_data_len = 0U;
    ret = gcm_rctx_switch_ctx(gcm_cmd_data, &ctx, AEAD_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = gcm_final_internal(gcm_cmd_data->input_addr, gcm_cmd_data->input_size, gcm_cmd_data->output_addr,
            gcm_cmd_data->tag_addr, &ctx);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
        rsp_data->data[0] = gcm_cmd_data->input_size;
        rsp_data->rsp_data_len += sizeof(gcm_cmd_data->input_size);
        if (MB_CIPHER_DEC == gcm_cmd_data->direction) {
            rsp_data->data[1] = (SKE_VERIFY_ERROR == ret) ? EHSM_ERR_GCM_TAG_VRY_FAILED : 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
        }
        /*
         * The result of verification will stored in response data, and the return code will only indicate
         * that if the process of calculation successfully work
         */
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

/**
 *   @brief      GCM single-call service
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_srv_onepass(const mb_cmd_aead_gcm_st *gcm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t iv[SKE_SRV_AEAD_IV_BUF_SZ];
    uint8_t *iv_ptr;
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_gcm_ctx_st ctx;
    uint32_t iv_sz = gcm_cmd_data->iv_size;
    cpt_ske_alg_e ske_alg = get_ske_alg(gcm_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_ENC == gcm_cmd_data->direction) ? SKE_CRYPTO_ENCRYPT : SKE_CRYPTO_DECRYPT;
    rsp_data->rsp_data_len = 0U;

    ret = aead_gcm_get_key(gcm_cmd_data, gcm_cmd_data->algorithm, gcm_cmd_data->direction, key_buf, sizeof(key_buf),
        &key_ptr, &key_sz, &sp_key_id, &spk_sel);

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // iv only support 12 byte in GCM
        if (12U == iv_sz) {
            ret = aead_get_iv(gcm_cmd_data->iv_addr, iv_sz, iv, sizeof(iv), &iv_ptr);
        } else {
            ret = EHSM_ERR_WRONG_IV_SIZE;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_gcm_init(&ctx, ske_alg, crypto, key_ptr, sp_key_id, iv_ptr, iv_sz, gcm_cmd_data->aad_size,
            gcm_cmd_data->input_size, (uint8_t)gcm_cmd_data->tag_size);
    }
    /*
     * If gcm_cmd_data->aad_size and gcm_cmd_data->input_size are all zero, tag has been calculate in ske_gcm_init,
     * so we must call cpt_ske_gcm_final to get or verify tag directly, and we cannot call ske_dma_gcm_update_final,
     * becase ctx->ske_gcm_ctx has not been initialization in ske_gcm_init
     */
    if ((0U == gcm_cmd_data->aad_size) && (0U == gcm_cmd_data->input_size)) {
        if (EHSM_ERR_SW_SUCCESS == ret) {
            uint8_t local_mac[SKE_SRV_AEAD_DATA_BUF_SZ];
            if (SKE_CRYPTO_DECRYPT == crypto) {
                ret = mmap_read_remote_data(local_mac, gcm_cmd_data->tag_addr, gcm_cmd_data->tag_size);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = cpt_ske_gcm_final(&ctx, local_mac);
                }
            } else {
                ret = cpt_ske_gcm_final(&ctx, local_mac);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = mmap_write_remote_data(gcm_cmd_data->tag_addr, local_mac, gcm_cmd_data->tag_size);
                }
            }
        }
    } else {
        if ((gcm_cmd_data->aad_size > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
            ret = gcm_update_aad(gcm_cmd_data->aad_addr, &ctx);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = gcm_final_internal(gcm_cmd_data->input_addr, gcm_cmd_data->input_size, gcm_cmd_data->output_addr,
                gcm_cmd_data->tag_addr, &ctx);
        }
    }

    if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
        rsp_data->data[0] = gcm_cmd_data->input_size;
        rsp_data->rsp_data_len += sizeof(gcm_cmd_data->input_size);
        if (MB_CIPHER_DEC == gcm_cmd_data->direction) {
            rsp_data->data[1] = (SKE_VERIFY_ERROR == ret) ? EHSM_ERR_GCM_TAG_VRY_FAILED : 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
        }
        /*
         * The result of verification will stored in response data, and the return code will only indicate
         * that if the process of calculation successfully work
         */
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

/**
 *   @brief      GCM stepwise handler
 *
 *   @param [in] gcm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gcm_srv_stepwise_handler(const mb_cmd_aead_gcm_st *gcm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (gcm_cmd_data->process_mode) {
    case MB_START: {
        ret = gcm_init(gcm_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = gcm_update(gcm_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = gcm_init(gcm_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = gcm_update(gcm_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = gcm_final(gcm_cmd_data, rsp_data);
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
 *   @brief      CCM update associated data service
 *
 *   @param [in] raad
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_update_aad(raddr_t raad, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_aad[SKE_SRV_AAD_BUF_SZ];
    if (ctx->aad_bytes <= sizeof(local_aad)) {
        ret = mmap_read_remote_data(local_aad, raad, ctx->aad_bytes);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_ccm_update_aad(ctx, local_aad);
        }
    } else {
        ret = EHSM_ERR_OUT_OF_MEM;
    }
    return ret;
}

/**
 *   @brief      CCM DMA update internal API
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_update_payload(raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t in_h = (uint32_t)(rin_addr >> 32);
    uint32_t in_l = (uint32_t)rin_addr;
    uint32_t out_h = (uint32_t)(rout_addr >> 32);
    uint32_t out_l = (uint32_t)rout_addr;

    if ((input_sz % ctx->ske_ccm_ctx[0].block_bytes) != 0U) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        ret = cpt_ske_dma_ccm_update_blocks(ctx, in_h, in_l, input_sz, out_h, out_l, NULL, SOC_READ_SOC_WRITE);
    }

    return ret;
}

/**
 *   @brief      CCM update the last block
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_updata_single_block(raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_in[SKE_SRV_AEAD_DATA_BUF_SZ];
    uint8_t local_out[SKE_SRV_AEAD_DATA_BUF_SZ];
    ret = mmap_read_remote_data(local_in, rin_addr, input_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_ccm_update_blocks(ctx, local_in, local_out, input_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_write_remote_data(rout_addr, local_out, input_sz);
    }
    return ret;
}

/**
 *   @brief      CCM final update first block API
 *
 *   @param [in] rin_addr
 *   @param [in] first_block_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_final_first_block_update(
    raddr_t *rin_addr, uint32_t first_block_sz, raddr_t *rout_addr, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret;
    // Update the first block with CPU mode for updating the B0
    ret = ccm_updata_single_block(*rin_addr, first_block_sz, *rout_addr, ctx);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        *rin_addr += (uint64_t)first_block_sz;
        *rout_addr += (uint64_t)first_block_sz;
    }
    return ret;
}

/**
 *   @brief      CCM final DMA update API
 *
 *   @param [in] rin_addr
 *   @param [in] dma_data_sz
 *   @param [in] rout_addr
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_final_dma_update(
    raddr_t *rin_addr, uint32_t dma_data_sz, raddr_t *rout_addr, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret;
    ret = ccm_update_payload(*rin_addr, dma_data_sz, *rout_addr, ctx);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        *rin_addr += (uint64_t)dma_data_sz;
        *rout_addr += (uint64_t)dma_data_sz;
    }
    return ret;
}

/**
 *   @brief      CCM internal final API
 *
 *   @param [in] rin_addr
 *   @param [in] input_sz
 *   @param [in] rout_addr
 *   @param [in] rmac
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_final_internal(
    raddr_t rin_addr, uint32_t input_sz, raddr_t rout_addr, raddr_t rmac, cpt_ske_ccm_ctx_st *ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t local_mac[SKE_SRV_AEAD_DATA_BUF_SZ];
    uint32_t mac_h = (uint32_t)(rmac >> 32);
    uint32_t mac_l = (uint32_t)rmac;
    raddr_t tmp_in_addr = rin_addr;
    raddr_t tmp_out_addr = rout_addr;
    uint32_t first_block_sz;
    uint32_t dma_data_sz;
    uint32_t last_block_sz;
    // ctx->M will be clear in cpt_ske_ccm_final()
    uint32_t mac_sz = ctx->M;
    cpt_ske_crypto_e dir = ctx->ske_ccm_ctx[0].crypto;
    uint32_t total_sz = input_sz;
    uint32_t total_proc_w_o_b0_sz = 0;
    /*
     * If the CCM update requist without aad, and the data size is aligned to block size,
     * it should need to update the B0 with CPU update API first
     */
    if ((0U == ctx->current_bytes) && (0U == ctx->aad_bytes)) {
        first_block_sz = (total_sz > ctx->ske_ccm_ctx[0].block_bytes) ? ctx->ske_ccm_ctx[0].block_bytes : total_sz;
        total_sz -= first_block_sz;
    } else {
        first_block_sz = 0U;
    }

    dma_data_sz = (total_sz & ~((uint32_t)ctx->ske_ccm_ctx[0].block_bytes - 1U));
    last_block_sz = total_sz - dma_data_sz;

    if (0U != first_block_sz) {
        ret = ccm_final_first_block_update(&tmp_in_addr, first_block_sz, &tmp_out_addr, ctx);
    }

    if ((dma_data_sz > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // Using DMA API to deal with the aligned blocks
        ret = ccm_final_dma_update(&tmp_in_addr, dma_data_sz, &tmp_out_addr, ctx);
    }

    if ((last_block_sz > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
        // Using CPU API to deal with the last block
        ret = ccm_updata_single_block(tmp_in_addr, last_block_sz, tmp_out_addr, ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {

        total_proc_w_o_b0_sz = ctx->current_bytes;
        // If ctx->aad_bytes = 0，the first block data is update for CPU mode, it should not be counted in
        // total_proc_w_o_b0_sz
        if ((0U == ctx->aad_bytes) && (total_proc_w_o_b0_sz >= ctx->ske_ccm_ctx[0].block_bytes)) {
            total_proc_w_o_b0_sz -= ctx->ske_ccm_ctx[0].block_bytes;
        }
        // Generate or verify the tag
        // If the last block is updated with DMA API, it should use cpt_ske_dma_ccm_update_final to generate tag
        if ((total_proc_w_o_b0_sz >= ctx->ske_ccm_ctx[0].block_bytes)
            && (total_proc_w_o_b0_sz % ctx->ske_ccm_ctx[0].block_bytes == 0U)) {
            ret = cpt_ske_dma_ccm_update_final(ctx, mac_h, mac_l, SOC_READ_SOC_WRITE);
        } else {
            if (SKE_CRYPTO_DECRYPT == dir) {
                ret = mmap_read_remote_data(local_mac, rmac, mac_sz);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // If the last block is updated with CPU API, it should use cpt_ske_ccm_final to generate tag
                ret = cpt_ske_ccm_final(ctx, local_mac);
            }
            if ((SKE_CRYPTO_ENCRYPT == dir) && (EHSM_ERR_SW_SUCCESS == ret)) {
                ret = mmap_write_remote_data(rmac, local_mac, mac_sz);
            }
        }
    }
    return ret;
}

/**
 *   @brief      CCM init stepwise service
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_init(const mb_cmd_aead_ccm_st *ccm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t iv[SKE_SRV_AEAD_IV_BUF_SZ];
    uint8_t *iv_ptr;
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_ccm_ctx_st ctx;
    uint32_t iv_sz = ccm_cmd_data->nonce_size;
    uint8_t L;
    uint8_t M;
    cpt_ske_alg_e ske_alg = get_ske_alg(ccm_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_ENC == ccm_cmd_data->direction) ? SKE_CRYPTO_ENCRYPT : SKE_CRYPTO_DECRYPT;
    // CCM nonce_size must be in [7, 13] per NIST SP 800-38C
    if ((iv_sz < 7U) || (iv_sz > 13U)) {
        ret = EHSM_ERR_WRONG_NONCE_SIZE;
    } else {
        L = 15U - (uint8_t)iv_sz;
        M = (uint8_t)ccm_cmd_data->tag_size;
        ret = aead_ccm_get_key(ccm_cmd_data, ccm_cmd_data->algorithm, ccm_cmd_data->direction, key_buf, sizeof(key_buf),
            &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = aead_get_iv(ccm_cmd_data->nonce_addr, iv_sz, iv, sizeof(iv), &iv_ptr);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_ccm_init(
            &ctx, ske_alg, crypto, key_ptr, sp_key_id, iv, M, L, ccm_cmd_data->aad_size, ccm_cmd_data->input_size);
    }
    if ((ccm_cmd_data->aad_size > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = ccm_update_aad(ccm_cmd_data->aad_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = ccm_rctx_switch_ctx(ccm_cmd_data, &ctx, AEAD_LCTX_2_RCTX);
    }
    rsp_data->rsp_data_len = 0U;
    return ret;
}

/**
 *   @brief      CCM update stepwise service
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_update(const mb_cmd_aead_ccm_st *ccm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_ccm_ctx_st ctx;
    uint32_t first_block_sz;
    uint32_t input_sz = ccm_cmd_data->input_size;
    raddr_t tmp_in_addr = ccm_cmd_data->input_addr;
    raddr_t tmp_out_addr = ccm_cmd_data->output_addr;
    ret = ccm_rctx_switch_ctx(ccm_cmd_data, &ctx, AEAD_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        /*
         * If the CCM update requist without aad, and the data size is aligned to block size,
         * it should need to update the B0 with CPU update API first
         */
        if ((0U == ctx.current_bytes) && (0U == ctx.aad_bytes)) {
            first_block_sz = (input_sz > ctx.ske_ccm_ctx[0].block_bytes) ? ctx.ske_ccm_ctx[0].block_bytes : input_sz;
        } else {
            first_block_sz = 0U;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (0U != first_block_sz) {
            // Update the first block with CPU mode for updating the B0
            ret = ccm_updata_single_block(tmp_in_addr, first_block_sz, tmp_out_addr, &ctx);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                tmp_in_addr += first_block_sz;
                tmp_out_addr += first_block_sz;
                input_sz -= first_block_sz;
            }
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = ccm_update_payload(tmp_in_addr, input_sz, tmp_out_addr, &ctx);
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = ccm_rctx_switch_ctx(ccm_cmd_data, &ctx, AEAD_LCTX_2_RCTX);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = ccm_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(ccm_cmd_data->input_size);
    }
    return ret;
}

/**
 *   @brief      CCM final stepwise service
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_final(const mb_cmd_aead_ccm_st *ccm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_ccm_ctx_st ctx;
    rsp_data->rsp_data_len = 0U;
    ret = ccm_rctx_switch_ctx(ccm_cmd_data, &ctx, AEAD_RCTX_2_LCTX);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = ccm_final_internal(ccm_cmd_data->input_addr, ccm_cmd_data->input_size, ccm_cmd_data->output_addr,
            ccm_cmd_data->tag_addr, &ctx);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
        rsp_data->data[0] = ccm_cmd_data->input_size;
        rsp_data->rsp_data_len += sizeof(ccm_cmd_data->input_size);
        if (MB_CIPHER_DEC == ccm_cmd_data->direction) {
            rsp_data->data[1] = (SKE_VERIFY_ERROR == ret) ? EHSM_ERR_CCM_TAG_VRY_FAILED : 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
        }
        /*
         * The result of verification will stored in response data, and the return code will only indicate
         * that if the process of calculation successfully work
         */
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

/**
 *   @brief      CCM single-call service
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_srv_onepass(const mb_cmd_aead_ccm_st *ccm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[SKE_SRV_AEAD_KEY_BUF_SZ];
    uint8_t iv[SKE_SRV_AEAD_IV_BUF_SZ];
    uint8_t *iv_ptr;
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    cpt_ske_ccm_ctx_st ctx;
    uint32_t iv_sz = ccm_cmd_data->nonce_size;
    uint8_t L;
    uint8_t M;
    cpt_ske_alg_e ske_alg = get_ske_alg(ccm_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_ENC == ccm_cmd_data->direction) ? SKE_CRYPTO_ENCRYPT : SKE_CRYPTO_DECRYPT;
    rsp_data->rsp_data_len = 0U;
    // CCM nonce_size must be in [7, 13] per NIST SP 800-38C
    if ((iv_sz < 7U) || (iv_sz > 13U)) {
        ret = EHSM_ERR_WRONG_NONCE_SIZE;
    } else {
        L = 15U - (uint8_t)iv_sz;
        M = (uint8_t)ccm_cmd_data->tag_size;
        ret = aead_ccm_get_key(ccm_cmd_data, ccm_cmd_data->algorithm, ccm_cmd_data->direction, key_buf, sizeof(key_buf),
            &key_ptr, &key_sz, &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = aead_get_iv(ccm_cmd_data->nonce_addr, iv_sz, iv, sizeof(iv), &iv_ptr);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_ccm_init(
            &ctx, ske_alg, crypto, key_ptr, sp_key_id, iv, M, L, ccm_cmd_data->aad_size, ccm_cmd_data->input_size);
    }
    if ((ccm_cmd_data->aad_size > 0U) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = ccm_update_aad(ccm_cmd_data->aad_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = ccm_final_internal(ccm_cmd_data->input_addr, ccm_cmd_data->input_size, ccm_cmd_data->output_addr,
            ccm_cmd_data->tag_addr, &ctx);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) || (SKE_VERIFY_ERROR == ret)) {
        rsp_data->data[0] = ccm_cmd_data->input_size;
        rsp_data->rsp_data_len += sizeof(ccm_cmd_data->input_size);
        if (MB_CIPHER_DEC == ccm_cmd_data->direction) {
            rsp_data->data[1] = (SKE_VERIFY_ERROR == ret) ? EHSM_ERR_CCM_TAG_VRY_FAILED : 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
        }
        /*
         * The result of verification will stored in response data, and the return code will only indicate
         * that if the process of calculation successfully work
         */
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

/**
 *   @brief      CCM stepwise handler
 *
 *   @param [in] ccm_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ccm_srv_stepwise_handler(const mb_cmd_aead_ccm_st *ccm_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (ccm_cmd_data->process_mode) {
    case MB_START: {
        ret = ccm_init(ccm_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = ccm_update(ccm_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = ccm_init(ccm_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = ccm_update(ccm_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = ccm_final(ccm_cmd_data, rsp_data);
        break;
    }
    default: {
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t aead_srv_gcm_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t aead_srv_gcm_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_aead_gcm_st *gcm_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        gcm_cmd_data = (const mb_cmd_aead_gcm_st *)req_data;
        // Check the direction first
        ret = cmd_check_direction(gcm_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode(gcm_cmd_data->process_mode);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = aead_check_output_buf_sz(
                gcm_cmd_data->process_mode, gcm_cmd_data->input_size, gcm_cmd_data->output_size);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_ONE_PASS == gcm_cmd_data->process_mode) {
                ret = gcm_srv_onepass(gcm_cmd_data, rsp_data);
            } else {
                ret = gcm_srv_stepwise_handler(gcm_cmd_data, rsp_data);
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

uint32_t aead_srv_ccm_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t aead_srv_ccm_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_aead_ccm_st *ccm_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        ccm_cmd_data = (const mb_cmd_aead_ccm_st *)req_data;
        // Check the direction first
        ret = cmd_check_direction(ccm_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode(ccm_cmd_data->process_mode);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = aead_check_output_buf_sz(
                ccm_cmd_data->process_mode, ccm_cmd_data->input_size, ccm_cmd_data->output_size);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_ONE_PASS == ccm_cmd_data->process_mode) {
                ret = ccm_srv_onepass(ccm_cmd_data, rsp_data);
            } else {
                ret = ccm_srv_stepwise_handler(ccm_cmd_data, rsp_data);
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
