/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "cipher_srv.h"
#include "mb.h"
#include "service/crypto_util.h"
#include "kms.h"
#include "mmap.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// SYM_KEY_BUF_MAX should consider the size of AES-256 in XTS mode
#define SYM_KEY_BUF_MAX (64U)
#define SYM_IV_BUF_MAX  (16U)
// SYM_XTS_DATA_BUF_MAX is using for the CPU final API, which may contain at least 2 blocks of data
#define SYM_XTS_DATA_BUF_MAX (32U)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t (*cipher_srv_init)(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*cipher_srv_update)(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*cipher_srv_final)(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data);
    uint32_t (*cipher_srv_onepass)(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data);
} ske_cipher_srv_hdl_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static ske_cipher_srv_hdl_st non_xts_hdl;
static ske_cipher_srv_hdl_st xts_hdl;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/**
 *   @brief      Symmetric block cipher single-call handler
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] srv_hdl
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_srv_onepass(
    const mb_cmd_symm_cipher_st *cipher_cmd_data, const ske_cipher_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      Symmetric block cipher stepwise handler
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] srv_hdl
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_srv_stepwise_handler(
    const mb_cmd_symm_cipher_st *cipher_cmd_data, const ske_cipher_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

/**
 *   @brief      Get the symmetric block cipher handler
 *
 *   @param [in] cipher_mode
 *   @param [in] srv_hdl
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_get_hdl(uint8_t cipher_mode, ske_cipher_srv_hdl_st **srv_hdl)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    switch (cipher_mode) {
    case MB_SYMM_CIPHER_CIPHER_MODE_ECB:
    case MB_SYMM_CIPHER_CIPHER_MODE_CBC:
    case MB_SYMM_CIPHER_CIPHER_MODE_CFB:
    case MB_SYMM_CIPHER_CIPHER_MODE_OFB:
    case MB_SYMM_CIPHER_CIPHER_MODE_CTR: {
        *srv_hdl = &non_xts_hdl;
        break;
    }
    case MB_SYMM_CIPHER_CIPHER_MODE_XTS: {
        *srv_hdl = &xts_hdl;
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
 *   @brief      Check the parameter of symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_check_param(const mb_cmd_symm_cipher_st *cipher_cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e ske_alg;
    uint8_t block_sz;
    if (cipher_cmd_data->algorithm > MB_SYMM_CIPHER_ALGORITHM_SM4) {
        ret = EHSM_ERR_INVALID_ALGORITHM;
    } else {
        ske_alg = get_ske_alg(cipher_cmd_data->algorithm);
        block_sz = cpt_ske_get_block_byte_len(ske_alg);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((cipher_cmd_data->cipher_mode > MB_SYMM_CIPHER_CIPHER_MODE_CTR)
            || (cipher_cmd_data->cipher_mode < MB_SYMM_CIPHER_CIPHER_MODE_ECB)) {
            ret = EHSM_ERR_INVALID_CIPHER_MODE;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // XTS mode has it's own padding mode
        if ((MB_SYMM_CIPHER_CIPHER_MODE_XTS != cipher_cmd_data->cipher_mode)
            && (MB_SYMM_CIPHER_PADDING_NO_PADDING != cipher_cmd_data->padding)
            && (MB_SYMM_CIPHER_PADDING_PKCS7 != cipher_cmd_data->padding)
            && (MB_SYMM_CIPHER_PADDING_ONEWITHZEROS != cipher_cmd_data->padding)) {
            ret = EHSM_ERR_INVALID_PADDING;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_SYMM_CIPHER_CIPHER_MODE_ECB != cipher_cmd_data->cipher_mode) {
            if ((0U != cipher_cmd_data->iv_addr) && (block_sz != cipher_cmd_data->iv_size)) {
                ret = EHSM_ERR_WRONG_IV_SIZE;
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cmd_check_direction(cipher_cmd_data->direction);
    }
    return ret;
}

/**
 *   @brief      Check the output buffer size of non-xts cipher
 *
 *   @param [in] alg
 *   @param [in] padding
 *   @param [in] process_mode
 *   @param [in] input_sz
 *   @param [in] output_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t non_xts_cipher_check_output_buffer_sz(
    uint8_t alg, uint8_t direction, uint8_t padding, uint8_t process_mode, uint32_t input_sz, uint32_t output_sz)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t block_sz;
    cpt_ske_alg_e ske_alg;
    if ((MB_STREAMSTART == process_mode) || (MB_UPDATE == process_mode)) {
        if (output_sz < input_sz) {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    } else if ((MB_FINISH == process_mode) || (MB_ONE_PASS == process_mode)) {
        if (MB_SYMM_CIPHER_PADDING_NO_PADDING == padding || MB_CIPHER_DEC == direction) {
            if (output_sz < input_sz) {
                ret = EHSM_ERR_OUTPUT_OVERFLOW;
            }
        }
        // In the encryption and padding mode, the size of ouput buffer need more than input size
        else {
            ske_alg = get_ske_alg(alg);
            block_sz = cpt_ske_get_block_byte_len(ske_alg);
            if (input_sz > (UINT32_MAX - block_sz)) {
                ret = EHSM_ERR_PARAM_ERROR;
            } else {
                // we need `input_sz + (block_sz - input_sz % block_sz)` for output data
                uint32_t output_needed_sz = (input_sz + block_sz) & (uint32_t)(~(block_sz - 1));
                if (output_sz < output_needed_sz) {
                    ret = EHSM_ERR_OUTPUT_OVERFLOW;
                }
            }
        }
    } else {
        ; // No need to check the buffer size in start mode
    }
    return ret;
}

/**
 *   @brief      Check the output buffer size of xts cipher
 *
 *   @param [in] input_sz
 *   @param [in] output_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t xts_cipher_check_output_buffer_sz(uint32_t input_sz, uint32_t output_sz)
{
    /*
     * The XTS mode does not force the length of data aligned to block size,
     * and it does not support other padding mode
     */
    return (output_sz >= input_sz) ? EHSM_ERR_SW_SUCCESS : EHSM_ERR_OUTPUT_OVERFLOW;
}

/**
 *   @brief      Get the key for symmetric block cipher
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
static uint32_t cipher_get_key_from_key_handle(uint32_t key_handle, uint32_t kms_alg, uint8_t dir, uint8_t *key_buf,
    uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    uint8_t req_kpart;
    kms_keydata_st keydata;

    if (MB_CIPHER_DEC == dir) {
        check_usage_bits |= KEY_USAGE_DECRYPT;
    } else {
        check_usage_bits |= KEY_USAGE_ENCRYPT;
    }
    req_kpart = KMS_KEY_PART_PRIVKEY;

    ret = kms_read_key(key_handle, key_buf, key_buf_sz, &keydata, req_kpart, check_usage_bits);

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (kms_alg == keydata.algo_id) {
            if (KMS_KEY_SRC_TYPE_RAM == keydata.src_type) {
                *key = keydata.keypair.symm.key;
                *key_sz = keydata.keypair.symm.size;
                *spk_sel = false;
            } else {
                if (keydata.keypair.symm.size <= SYM_SP_KEY_MAZ_SZ) {
                    // When using secure port key, the pointer key should be NULL
                    *key = NULL;
                    *sp_key_id = keydata.sp_key_id;
                    *spk_sel = true;
                } else {
                    // Secure port key only support the key with key size not greater than SYM_SP_KEY_MAZ_SZ
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
 *   @brief      Get the key for symmetric block cipher
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
static uint32_t cipher_get_key(const mb_cmd_symm_cipher_st *cipher_cmd_data, uint32_t kms_alg, uint8_t dir,
    uint8_t *key_buf, uint32_t key_buf_sz, uint8_t **key, uint32_t *key_sz, uint16_t *sp_key_id, bool_t *spk_sel)
{
    uint32_t ret;

    switch (cipher_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        *sp_key_id = 0U;
        *spk_sel = false;
        if (cipher_cmd_data->plain_key_size > key_buf_sz) {
            ret = EHSM_ERR_NOT_SUPPORT;
        } else {
            ret = mmap_read_remote_data(key_buf, cipher_cmd_data->plain_key_addr, cipher_cmd_data->plain_key_size);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *key = key_buf;
                *key_sz = cipher_cmd_data->plain_key_size;
            }
        }
        break;
    case MB_KEY_HANDLE:
        ret = cipher_get_key_from_key_handle(
            cipher_cmd_data->key_handle, kms_alg, dir, key_buf, key_buf_sz, key, key_sz, sp_key_id, spk_sel);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Get the initial vector for symmetric block cipher
 *
 *   @param [in] mode
 *   @param [in] r_iv
 *   @param [in] r_iv_sz
 *   @param [in] iv_buf
 *   @param [in] iv_ptr
 *   @param [in] iv_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_get_iv(
    cpt_ske_mode_e mode, raddr_t r_iv, uint32_t r_iv_sz, uint8_t *iv_buf, uint8_t **iv_ptr, uint32_t *iv_sz)
{
    uint32_t ret;
    if ((SKE_MODE_ECB == mode) || (0U == r_iv)) {
        *iv_ptr = NULL;
        // The iv_sz could be NULL, which means no need to get the size of iv
        if (NULL != iv_sz) {
            *iv_sz = 0U;
        }
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        if (r_iv_sz <= SYM_IV_BUF_MAX) {
            ret = mmap_read_remote_data(iv_buf, r_iv, r_iv_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                *iv_ptr = iv_buf;
                if (NULL != iv_sz) {
                    *iv_sz = r_iv_sz;
                }
            }

        } else {
            ret = EHSM_ERR_IV_OVERFLOW;
        }
    }
    return ret;
}

/**
 *   @brief      Upload the context struct for non-xts cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] cipher_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t push_non_xts_cipher_ctx(const mb_cmd_symm_cipher_st *cipher_cmd_data, cpt_ske_ctx_st *cipher_ctx)
{
    uint32_t ret;
    // Clear the sensitive information
    cpt_clear_ske_ctx_key(cipher_ctx);
    if (cipher_cmd_data->context_size >= sizeof(cpt_ske_ctx_st)) {
        ret = mmap_write_remote_data(cipher_cmd_data->context, cipher_ctx, sizeof(cpt_ske_ctx_st));
    } else {
        ret = EHSM_ERR_CTX_OVERFLOW;
    }
    return ret;
}

/**
 *   @brief      Get the non xts cipher context object
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] cipher_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t get_non_xts_cipher_ctx(const mb_cmd_symm_cipher_st *cipher_cmd_data, cpt_ske_ctx_st *cipher_ctx)
{
    uint32_t ret;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);
    if (cipher_cmd_data->context_size >= sizeof(cpt_ske_ctx_st)) {
        ret = mmap_read_remote_data(cipher_ctx, cipher_cmd_data->context, sizeof(cpt_ske_ctx_st));
    } else {
        ret = EHSM_ERR_CTX_OVERFLOW;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Get the key and put it to context
        ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
            &key_sz, &sp_key_id, &spk_sel);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_set_ske_ctx_key(cipher_ctx, spk_sel, key_ptr, sp_key_id, (uint8_t)key_sz);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Upload the context struct for xts cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] cipher_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t push_xts_cipher_ctx(const mb_cmd_symm_cipher_st *cipher_cmd_data, cpt_ske_xts_ctx_st *cipher_ctx)
{
    uint32_t ret;
    // Clear the sensitive information
    cpt_clear_ske_ctx_key(cpt_get_ske_xts_ske_ctx(cipher_ctx));
    if (cipher_cmd_data->context_size >= sizeof(cpt_ske_xts_ctx_st)) {
        ret = mmap_write_remote_data(cipher_cmd_data->context, cipher_ctx, sizeof(cpt_ske_xts_ctx_st));
    } else {
        ret = EHSM_ERR_CTX_OVERFLOW;
    }
    return ret;
}

/**
 *   @brief      Get the xts cipher context object
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] cipher_ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t get_xts_cipher_ctx(const mb_cmd_symm_cipher_st *cipher_cmd_data, cpt_ske_xts_ctx_st *cipher_ctx)
{
    uint32_t ret;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);
    if (cipher_cmd_data->context_size >= sizeof(cpt_ske_xts_ctx_st)) {
        ret = mmap_read_remote_data(cipher_ctx, cipher_cmd_data->context, sizeof(cpt_ske_xts_ctx_st));
    } else {
        ret = EHSM_ERR_CTX_OVERFLOW;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Get the key and put it to context
        ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
            &key_sz, &sp_key_id, &spk_sel);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_set_ske_ctx_key(cpt_get_ske_xts_ske_ctx(cipher_ctx), spk_sel, key_ptr, sp_key_id, (uint8_t)key_sz);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t cipher_srv_onepass(
    const mb_cmd_symm_cipher_st *cipher_cmd_data, const ske_cipher_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = cipher_check_param(cipher_cmd_data);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = srv_hdl->cipher_srv_onepass(cipher_cmd_data, rsp_data);
    }
    return ret;
}

static uint32_t cipher_srv_stepwise_handler(
    const mb_cmd_symm_cipher_st *cipher_cmd_data, const ske_cipher_srv_hdl_st *srv_hdl, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (cipher_cmd_data->process_mode) {
    case MB_START: {
        ret = cipher_check_param(cipher_cmd_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = srv_hdl->cipher_srv_init(cipher_cmd_data, rsp_data);
        }
        break;
    }
    case MB_UPDATE: {
        ret = srv_hdl->cipher_srv_update(cipher_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = cipher_check_param(cipher_cmd_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = srv_hdl->cipher_srv_init(cipher_cmd_data, rsp_data);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = srv_hdl->cipher_srv_update(cipher_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = srv_hdl->cipher_srv_final(cipher_cmd_data, rsp_data);
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
 *   @brief      Init service of non-xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_non_xts_init(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_ctx_st ctx;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t iv[SYM_IV_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint8_t *iv_ptr;
    cpt_ske_alg_e alg = get_ske_alg(cipher_cmd_data->algorithm);
    cpt_ske_mode_e mode = get_ske_mode(cipher_cmd_data->cipher_mode);
    cpt_ske_crypto_e crypto = (MB_CIPHER_DEC == cipher_cmd_data->direction) ? SKE_CRYPTO_DECRYPT : SKE_CRYPTO_ENCRYPT;
    cpt_ske_padding_e padding = get_ske_padding(cipher_cmd_data->padding);
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);
    ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
        &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_iv(mode, cipher_cmd_data->iv_addr, cipher_cmd_data->iv_size, iv, &iv_ptr, NULL);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_init(&ctx, alg, mode, crypto, key_ptr, sp_key_id, iv_ptr, padding);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = push_non_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }

    // No need to response data
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Update service of non-xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_non_xts_update(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_ctx_st ctx;
    uint32_t in_h = (uint32_t)(cipher_cmd_data->input_addr >> 32);
    uint32_t in_l = (uint32_t)cipher_cmd_data->input_addr;
    uint32_t out_h = (uint32_t)(cipher_cmd_data->output_addr >> 32);
    uint32_t out_l = (uint32_t)cipher_cmd_data->output_addr;

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = non_xts_cipher_check_output_buffer_sz(cipher_cmd_data->algorithm, cipher_cmd_data->direction,
        cipher_cmd_data->padding, cipher_cmd_data->process_mode, cipher_cmd_data->input_size,
        cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = get_non_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_update_blocks(
            &ctx, in_h, in_l, out_h, out_l, cipher_cmd_data->input_size, NULL, SOC_READ_SOC_WRITE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = push_non_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = cipher_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(cipher_cmd_data->input_size);
    }
    return ret;
}

/**
 *   @brief      Final service of non-xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note       If the final request in padding mode, the input_size should not be 0
 */
static uint32_t cipher_non_xts_final(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_ctx_st ctx;
    uint32_t output_sz = 0U;
    uint32_t in_h = (uint32_t)(cipher_cmd_data->input_addr >> 32);
    uint32_t in_l = (uint32_t)cipher_cmd_data->input_addr;
    uint32_t out_h = (uint32_t)(cipher_cmd_data->output_addr >> 32);
    uint32_t out_l = (uint32_t)cipher_cmd_data->output_addr;

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = non_xts_cipher_check_output_buffer_sz(cipher_cmd_data->algorithm, cipher_cmd_data->direction,
        cipher_cmd_data->padding, cipher_cmd_data->process_mode, cipher_cmd_data->input_size,
        cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = get_non_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_update_including_last_block(
            &ctx, in_h, in_l, out_h, out_l, cipher_cmd_data->input_size, &output_sz, NULL, SOC_READ_SOC_WRITE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_final(&ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = output_sz;
        rsp_data->rsp_data_len = sizeof(output_sz);
    }
    return ret;
}

/**
 *   @brief      Single-call service of non-xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_non_xts_onepass(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint32_t in_h = (uint32_t)(cipher_cmd_data->input_addr >> 32);
    uint32_t in_l = (uint32_t)cipher_cmd_data->input_addr;
    uint32_t out_h = (uint32_t)(cipher_cmd_data->output_addr >> 32);
    uint32_t out_l = (uint32_t)cipher_cmd_data->output_addr;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t iv[SYM_IV_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint8_t *iv_ptr;
    uint32_t input_sz = cipher_cmd_data->input_size;
    uint32_t output_sz = 0;
    cpt_ske_alg_e alg = get_ske_alg(cipher_cmd_data->algorithm);
    cpt_ske_mode_e mode = get_ske_mode(cipher_cmd_data->cipher_mode);
    cpt_ske_crypto_e crypto = (MB_CIPHER_DEC == cipher_cmd_data->direction) ? SKE_CRYPTO_DECRYPT : SKE_CRYPTO_ENCRYPT;
    cpt_ske_padding_e padding = get_ske_padding(cipher_cmd_data->padding);
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = non_xts_cipher_check_output_buffer_sz(cipher_cmd_data->algorithm, cipher_cmd_data->direction,
        cipher_cmd_data->padding, cipher_cmd_data->process_mode, cipher_cmd_data->input_size,
        cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
            &key_sz, &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_iv(mode, cipher_cmd_data->iv_addr, cipher_cmd_data->iv_size, iv, &iv_ptr, NULL);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_crypto(alg, mode, crypto, key_ptr, sp_key_id, iv_ptr, padding, in_h, in_l, out_h, out_l,
            input_sz, &output_sz, NULL, SOC_READ_SOC_WRITE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = output_sz;
        rsp_data->rsp_data_len = sizeof(output_sz);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Using DMA for updating the data in final service of xts cipher
 *
 *   @param [in] rdata
 *   @param [in] data_sz
 *   @param [in] output
 *   @param [in] ctx
 *   @param [in] dma_data_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t xts_dma_final_update_blocks(
    raddr_t rdata, uint32_t data_sz, raddr_t routput, cpt_ske_xts_ctx_st *ctx, uint32_t *dma_data_sz)
{
    uint32_t ret;
    uint32_t input_sz;
    uint32_t in_h = (uint32_t)(rdata >> 32);
    uint32_t in_l = (uint32_t)rdata;
    uint32_t out_h = (uint32_t)(routput >> 32);
    uint32_t out_l = (uint32_t)routput;
    uint32_t block_count = (data_sz + 15U) / 16U;
    // Make sure at least contain 2 block in the final stage or the data length is 16 bytes
    if (block_count < 2U) {
        if (data_sz == 16U) {
            input_sz = data_sz;
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_XTS_WRONG_DATA_LENGTH;
        }
    } else {
        input_sz = (block_count - 2U) * 16U;
        ret = EHSM_ERR_SW_SUCCESS;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_xts_update_blocks(ctx, in_h, in_l, out_h, out_l, input_sz, NULL, SOC_READ_SOC_WRITE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            *dma_data_sz = input_sz;
        }
    }
    return ret;
}

/**
 *   @brief      Using CPU API for calculating the last data in final service of xts cipher
 *
 *   @param [in] rdata
 *   @param [in] data_sz
 *   @param [in] routput
 *   @param [in] ctx
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t xts_cpu_final(raddr_t rdata, uint32_t data_sz, raddr_t routput, cpt_ske_xts_ctx_st *ctx)
{
    uint32_t ret;
    uint8_t local_data[SYM_XTS_DATA_BUF_MAX];
    uint8_t local_output[SYM_XTS_DATA_BUF_MAX];
    if (data_sz <= SYM_XTS_DATA_BUF_MAX) {
        ret = mmap_read_remote_data(local_data, rdata, data_sz);
    } else {
        ret = EHSM_ERR_OUT_OF_MEM;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_xts_update_including_last_2_blocks(ctx, local_data, local_output, data_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_write_remote_data(routput, local_output, data_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Clear the ctx
        ret = cpt_ske_xts_final(ctx);
    }
    return ret;
}

/**
 *   @brief      Init service of xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_xts_init(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_xts_ctx_st ctx;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t iv[SYM_IV_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint8_t *iv_ptr;
    cpt_ske_alg_e alg = get_ske_alg(cipher_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_DEC == cipher_cmd_data->direction) ? SKE_CRYPTO_DECRYPT : SKE_CRYPTO_ENCRYPT;
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);
    uint32_t input_sz = cipher_cmd_data->input_size;
    ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
        &key_sz, &sp_key_id, &spk_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_iv(SKE_MODE_XTS, cipher_cmd_data->iv_addr, cipher_cmd_data->iv_size, iv, &iv_ptr, NULL);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        /*
         *  TODO: Remove the input_sz of cpt_ske_dma_xts_init
         *  The CryLib will check the data size in cpt_ske_dma_xts_init, which should be remove in the future
         */
        input_sz = 16U;
        ret = cpt_ske_dma_xts_init(&ctx, alg, crypto, XTS_IEEE, key_ptr, sp_key_id, iv_ptr, input_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = push_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    // No need to response data
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Update service of xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_xts_update(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_xts_ctx_st ctx;
    uint32_t in_h = (uint32_t)(cipher_cmd_data->input_addr >> 32);
    uint32_t in_l = (uint32_t)cipher_cmd_data->input_addr;
    uint32_t out_h = (uint32_t)(cipher_cmd_data->output_addr >> 32);
    uint32_t out_l = (uint32_t)cipher_cmd_data->output_addr;

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = xts_cipher_check_output_buffer_sz(cipher_cmd_data->input_size, cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = get_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_xts_update_blocks(
            &ctx, in_h, in_l, out_h, out_l, cipher_cmd_data->input_size, NULL, SOC_READ_SOC_WRITE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = push_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = cipher_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(cipher_cmd_data->input_size);
    }
    return ret;
}

/**
 *   @brief      Final service of xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note       The final request should contain at least 2 blocks data as the input, since the XTS
 *               cipher need to do some special process in the last 2 block
 */
static uint32_t cipher_xts_final(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_xts_ctx_st ctx;
    raddr_t input_addr = cipher_cmd_data->input_addr;
    raddr_t output_addr = cipher_cmd_data->output_addr;
    uint32_t input_sz = cipher_cmd_data->input_size;
    uint32_t dma_data_sz;

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = xts_cipher_check_output_buffer_sz(cipher_cmd_data->input_size, cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = get_xts_cipher_ctx(cipher_cmd_data, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Update the data as could as possible in DMA mode
        ret = xts_dma_final_update_blocks(input_addr, input_sz, output_addr, &ctx, &dma_data_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            input_addr += (uint64_t)dma_data_sz;
            output_addr += (uint64_t)dma_data_sz;
            input_sz -= dma_data_sz;
        }
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) && (input_sz != 0)) {
        /*
         * The XTS DMA final API need to padding 0 in the last block,
         * so that it will only use CPU mode API in the final stage
         */
        ret = xts_cpu_final(input_addr, input_sz, output_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = cipher_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(cipher_cmd_data->input_size);
    }
    return ret;
}

/**
 *   @brief      Single-call service of xts symmetric block cipher
 *
 *   @param [in] cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t cipher_xts_onepass(const mb_cmd_symm_cipher_st *cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    cpt_ske_xts_ctx_st ctx;
    uint8_t key_buf[SYM_KEY_BUF_MAX];
    uint8_t iv[SYM_IV_BUF_MAX];
    uint8_t *key_ptr;
    uint32_t key_sz;
    uint16_t sp_key_id;
    bool_t spk_sel;
    uint8_t *iv_ptr;
    cpt_ske_alg_e alg = get_ske_alg(cipher_cmd_data->algorithm);
    cpt_ske_crypto_e crypto = (MB_CIPHER_DEC == cipher_cmd_data->direction) ? SKE_CRYPTO_DECRYPT : SKE_CRYPTO_ENCRYPT;
    uint32_t kms_alg = get_kms_ske_alg(cipher_cmd_data->algorithm, cipher_cmd_data->cipher_mode);
    raddr_t input_addr = cipher_cmd_data->input_addr;
    raddr_t output_addr = cipher_cmd_data->output_addr;
    uint32_t input_sz = cipher_cmd_data->input_size;
    uint32_t dma_data_sz;

    // No need to response while error occuring
    rsp_data->rsp_data_len = 0U;
    ret = xts_cipher_check_output_buffer_sz(cipher_cmd_data->input_size, cipher_cmd_data->output_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_key(cipher_cmd_data, kms_alg, cipher_cmd_data->direction, key_buf, sizeof(key_buf), &key_ptr,
            &key_sz, &sp_key_id, &spk_sel);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cipher_get_iv(SKE_MODE_XTS, cipher_cmd_data->iv_addr, cipher_cmd_data->iv_size, iv, &iv_ptr, NULL);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_dma_xts_init(&ctx, alg, crypto, XTS_IEEE, key_ptr, sp_key_id, iv_ptr, input_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Update the data as could as possible in DMA mode
        ret = xts_dma_final_update_blocks(input_addr, input_sz, output_addr, &ctx, &dma_data_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            input_addr += (uint64_t)dma_data_sz;
            output_addr += (uint64_t)dma_data_sz;
            input_sz -= dma_data_sz;
        }
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) && (input_sz != 0)) {
        /*
         * The XTS DMA final API need to padding 0 in the last block,
         * so that it will only use CPU mode API in the final stage.
         * That is the reason why the single-call API will use stepwise API instead
         */
        ret = xts_cpu_final(input_addr, input_sz, output_addr, &ctx);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // The size of output is as the same as the input in XTS mode
        rsp_data->data[0] = cipher_cmd_data->input_size;
        rsp_data->rsp_data_len = sizeof(cipher_cmd_data->input_size);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Initialize the non-xts cipher handler
 *
 *
 *
 *   @note
 */
static void non_xts_hdl_init(void)
{
    non_xts_hdl.cipher_srv_init = cipher_non_xts_init;
    non_xts_hdl.cipher_srv_update = cipher_non_xts_update;
    non_xts_hdl.cipher_srv_final = cipher_non_xts_final;
    non_xts_hdl.cipher_srv_onepass = cipher_non_xts_onepass;
}

/**
 *   @brief      Initialize the xts cipher handler
 *
 *
 *
 *   @note
 */
static void xts_hdl_init(void)
{
    xts_hdl.cipher_srv_init = cipher_xts_init;
    xts_hdl.cipher_srv_update = cipher_xts_update;
    xts_hdl.cipher_srv_final = cipher_xts_final;
    xts_hdl.cipher_srv_onepass = cipher_xts_onepass;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t ske_srv_cipher_init(void)
{
    // Initialize all the handlers of symmetric block cipher
    non_xts_hdl_init();
    xts_hdl_init();
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t ske_srv_cipher_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_symm_cipher_st *cipher_cmd_data;
    ske_cipher_srv_hdl_st *srv_hdl;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        cipher_cmd_data = (const mb_cmd_symm_cipher_st *)req_data;
        ret = cmd_check_process_mode(cipher_cmd_data->process_mode);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cipher_get_hdl(cipher_cmd_data->cipher_mode, &srv_hdl);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_ONE_PASS == cipher_cmd_data->process_mode) {
                ret = cipher_srv_onepass(cipher_cmd_data, srv_hdl, rsp_data);
            } else {
                ret = cipher_srv_stepwise_handler(cipher_cmd_data, srv_hdl, rsp_data);
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
