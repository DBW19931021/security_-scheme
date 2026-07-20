/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdint.h>
#include "types.h"
#include "util.h"
#include "mmap.h"
#include "driver/sysreg.h"
#include "cpu_porting.h"
#include "crypto_api.h"
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
#include "mmap.h"
#endif
#include "fid.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define CPT_MAC_CPU_MODE_BUF_SZ (512U)
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

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/**
 * @brief convert cpt error code to ehsm error code
 * @param[in] cpt_err_code The cpt error code
 * @param[in] success_code The success code of cpt
 * @return ehsm error code
 * @note
 *     1. if cpt error code is success, return ehsm success code, otherwise return cpt error code.
 *     2. if cpt error code is not success, return cpt error code.
 */
static uint32_t cpt_err_code_to_ehsm_code(uint32_t cpt_err_code, uint32_t success_code)
{
    uint32_t ret;
    if (cpt_err_code == success_code) {
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = cpt_err_code;
    }
    return ret;
}

/**
 * @brief convert cpt error code to ehsm error code, prevent injection attacks version
 * @param[in] cpt_err_code The cpt error code
 * @param[in] success_code The success code of cpt
 * @return ehsm error code
 * @note
 *     1. if cpt error code is success, return ehsm success code, otherwise return cpt error code.
 *     2. if cpt error code is not success, return cpt error code.
 */
static uint32_t cpt_err_code_to_ehsm_code_sec(uint32_t cpt_err_code, uint32_t success_code)
{
    uint32_t ret;
    if (FID_EQ(cpt_err_code, success_code)) {
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = cpt_err_code;
    }
    return ret;
}

#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
/**
 * @brief  update ske cbc mac in cpu mode, update data from remote address.
 *
 * @param[in] msg_h The high 32bit of remote address.
 * @param[in] msg_l The low 32bit of remote address.
 * @param[in] input_sz The size of input data.
 * @param[in] ctx The ske cbc mac context.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 */
static uint32_t cbcmac_cpu_update_internal(uint32_t msg_h, uint32_t msg_l, uint32_t input_sz, ske_cbc_mac_st *ctx)
{
    uint32_t ret;
    uint8_t local_data[CPT_MAC_CPU_MODE_BUF_SZ];
    uint32_t total_sz = input_sz;
    raddr_t tmp_addr = ((raddr_t)msg_h << 32) + (raddr_t)msg_l;
    uint32_t once_sz;

    do {
        // The size of buffer local_data has been aligned to block size
        once_sz = (total_sz > CPT_MAC_CPU_MODE_BUF_SZ) ? CPT_MAC_CPU_MODE_BUF_SZ : total_sz;
        ret = mmap_read_remote_data(local_data, tmp_addr, once_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cbc_mac_update(ctx, local_data, once_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                total_sz -= once_sz;
                tmp_addr += once_sz;
            }
        }

        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }

    } while (total_sz > 0U);
    return ret;
}
#endif

/**
 * @brief get block byte length for spcific ske alg
 *
 * @param ske_alg -------------------- input, ske algorithm
 *
 * @return block byte length for ske alg
 * @note
 *     1. please make sure ske_alg is valid
 **/
uint8_t cpt_ske_get_block_byte_len(ske_alg_e ske_alg)
{
    return ske_get_block_byte_len(ske_alg);
}

/**
 * @brief set the ske context of key and secure port key index
 *
 * @param[in] ctx the ske context pointer
 * @param[in] is_sp_key true: using secure port key, false: using ram key
 * @param[in] key the key pointer, if is_sp_key is true, then key must be NULL, otherwise key must be valid.
 * @param[in] sp_key_id the secure port key index, if is_sp_key is false, then sp_key_id must be 0, otherwise sp_key_id
 * must be valid.
 * @param[in] key_bytes key in bytes
 *
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_set_ske_ctx_key(
    cpt_ske_ctx_st *ctx, bool_t is_sp_key, const uint8_t *key, uint16_t sp_key_id, uint8_t key_bytes)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    if (ctx == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (true == is_sp_key) {
            // Using secure port key
            ctx->sp_key_idx = sp_key_id;
            ctx->key = NULL;
        } else {
            // Using normal key
            if ((key == NULL) || (key_bytes > sizeof(ctx->key_buf))) {
                ret = EHSM_ERR_PARAM_ERROR;
            } else {
                util_memcpy(ctx->key_buf, key, key_bytes);
                ctx->key = (uint8_t *)ctx->key_buf;
            }
        }
    }
    return ret;
}

/**
 * @brief clear the ske context of key
 *
 * @param[in] ctx ske_ctx_st context pointer.
 *
 */
void cpt_clear_ske_ctx_key(cpt_ske_ctx_st *ctx)
{
    if (ctx != NULL) {
        util_memset(ctx->key_buf, 0U, sizeof(ctx->key_buf));
        ctx->key = NULL;
        ctx->sp_key_idx = 0U;
    }
}

/**
 * @brief ske encrypting or decrypting(CPU style, one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] mode ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes, must be a block
 * @param[in] padding padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
              SKE_ISO_7816_4_PADDING
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] in_bytes byte length of input
 * @param[out] out_bytes byte length of output
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte
 *        length, out_bytes will be the same as in_bytes.
 */
uint32_t cpt_ske_crypto(cpt_ske_alg_e alg, cpt_ske_mode_e mode, cpt_ske_crypto_e crypto, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *iv, cpt_ske_padding_e padding, const uint8_t *in, uint8_t *out,
    uint32_t in_bytes, uint32_t *out_bytes)
{
    uint32_t ske_ret = ske_crypto(alg, mode, crypto, key, sp_key_idx, iv, padding, in, out, in_bytes, out_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske init config(DMA style)
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] mode ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, must be a block
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes
 * @param[in] padding padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
              SKE_ISO_7816_4_PADDING
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t cpt_ske_dma_init(cpt_ske_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_mode_e mode, cpt_ske_crypto_e crypto,
    const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv, cpt_ske_padding_e padding)
{
    uint32_t ske_ret = ske_dma_init(ctx, alg, mode, crypto, key, sp_key_idx, iv, padding);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske encryption or decryption(DMA style)
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output, must be a multiple of block length
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and input/output unit is a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. bytes must be a multiple of block byte length.
 */
uint32_t cpt_ske_dma_update_blocks(cpt_ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l,
    uint32_t bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_update_blocks(ctx, in_h, in_l, out_h, out_l, bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske encryption or decryption for input including tail(DMA style)
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] in_bytes byte length of input
 * @param[out] out_bytes byte length of output
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero;
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte
 *        length, out_bytes will be the same as in_bytes.
 */
uint32_t cpt_ske_dma_update_including_last_block(cpt_ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h,
    uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_update_including_last_block(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske encrypting or decrypting(DMA style, one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] mode ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes, must be a block
 * @param[in] padding padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
              SKE_ISO_7816_4_PADDING
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] in_bytes byte length of input
 * @param[out] out_bytes byte length of output
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte
 *        length, out_bytes will be the same as in_bytes.
 */
uint32_t cpt_ske_dma_crypto(cpt_ske_alg_e alg, cpt_ske_mode_e mode, cpt_ske_crypto_e crypto, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *iv, cpt_ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h,
    uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_crypto(
        alg, mode, crypto, key, sp_key_idx, iv, padding, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske finish(DMA style)
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t cpt_ske_dma_final(cpt_ske_ctx_st *ctx)
{
    uint32_t ske_ret = ske_dma_final(ctx);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske xts mode encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 *
 * @param[in] ctx ske_xts_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks, actualy, this function is for the case that ctx->c_bytes % 16
 *        is not 0.
 */
uint32_t cpt_ske_xts_update_including_last_2_blocks(
    cpt_ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ske_ret = ske_xts_update_including_last_2_blocks(ctx, in, out, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske xts mode finish
 *
 * @param[in] ctx ske_xts_ctx_st context pointer
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this is the last step of xts calling, and it is optional
 */
uint32_t cpt_ske_xts_final(cpt_ske_xts_ctx_st *ctx)
{
    uint32_t ske_ret = ske_xts_final(ctx);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske xts mode dma style init config
 *
 * @param[in] ctx ske_xts_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, key = key1||key2
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] i i value, it has the same length as block length
 * @param[in] c_bytes byte length of plaintext/ciphertext, it can not be less than
              block byte length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 *     4. if the SKE hardware is LP version, this function will return EHSM_ERR_NOT_SUPPORT.
 */
uint32_t cpt_ske_dma_xts_init(cpt_ske_xts_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_crypto_e crypto,
    cpt_xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i, uint32_t c_bytes)
{
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
    UNUSED(ctx);
    UNUSED(alg);
    UNUSED(crypto);
    UNUSED(xts_style);
    UNUSED(key);
    UNUSED(sp_key_idx);
    UNUSED(i);
    UNUSED(c_bytes);
    uint32_t ske_ret = EHSM_ERR_NOT_SUPPORT;
#else
    uint32_t ske_ret = ske_dma_xts_init(ctx, alg, crypto, xts_style, key, sp_key_idx, i, c_bytes);
#endif
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske xts mode dma style encryption or decryption
 *
 * @param[in] ctx ske_xts_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output.
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 *     3. if the whole plaintext/ciphertext is too long, you could divide it into some sections
 *        by block(16 bytes), then call this function to input the sections respectively. but
 *        if ctx->c_bytes is not a multiple of block byte length, the input in here could not
 *        contain the last two blocks of the whole input, in this case , the last 2 blocks(actually
 *        the last block is with padding 0) are left to function
 *        ske_dma_xts_update_including_last_2_blocks().
 *     4. if the SKE hardware is LP version, this function will return EHSM_ERR_NOT_SUPPORT.
 */
uint32_t cpt_ske_dma_xts_update_blocks(cpt_ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h,
    uint32_t out_l, uint32_t bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
    UNUSED(ctx);
    UNUSED(in_h);
    UNUSED(in_l);
    UNUSED(out_h);
    UNUSED(out_l);
    UNUSED(bytes);
    UNUSED(callback);
    UNUSED(dma_cfg);
    return EHSM_ERR_NOT_SUPPORT;
#else
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_xts_update_blocks(ctx, in_h, in_l, out_h, out_l, bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
#endif
}

/**
 * @brief get the ske_ctx_st pointer of ske_xts_ctx_st
 *
 * @param[in] ctx ske_xts_ctx_st context pointer
 */
ske_ctx_st *cpt_get_ske_xts_ske_ctx(cpt_ske_xts_ctx_st *xts_ctx)
{
    return xts_ctx->ske_xts_ctx;
}

/**
* @brief ske cbc init config(CPU style)
*
* @param[in] ctx ske_ctx_st context pointer
* @param[in] alg ske algorithm
* @param[in] crypto encrypting or decrypting
* @param[in] key key in bytes
* @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
             if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
* @param[in] iv iv in bytes, must be a block
* @param[in] padding padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
             SKE_ISO_7816_4_PADDING
* @return EHSM_ERR_SW_SUCCESS(success), other(error)
* @note
*     1. input/output unit must be a block if padding is SKE_NO_PADDING.
*     2. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
*        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
*/
uint32_t cpt_ske_cbc_init(cpt_ske_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *iv, cpt_ske_padding_e padding)
{
    uint32_t ske_ret = ske_cbc_init(ctx, alg, crypto, key, sp_key_idx, iv, padding);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske encryption or decryption(CPU style)
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 */
uint32_t cpt_ske_cbc_update_blocks(cpt_ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ske_ret = ske_cbc_update_blocks(ctx, in, out, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cbc finish
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t cpt_ske_cbc_final(cpt_ske_ctx_st *ctx)
{
    uint32_t ske_ret = ske_cbc_final(ctx);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cbc mac init(CPU style)
 *
 * @param[in] ctx ske_cbc_mac_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] mac_action must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 * @param[in] padding ske cbc mac padding scheme, must be SKE_NO_PADDING or SKE_ZERO_PADDING.
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] mac_bytes mac byte length, must be bigger than 1, and not bigger than block length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t cpt_ske_cbc_mac_init(cpt_ske_cbc_mac_st *ctx, cpt_ske_alg_e alg, ske_mac_e mac_action,
    cpt_ske_padding_e padding, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_cbc_mac_init(ctx, alg, mac_action, padding, key, sp_key_idx, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cbc_mac update message(CPU style)
 *
 * @param[in] ctx ske_cbc_mac_ctx_st context pointer
 * @param[in] msg message
 * @param[in] msg_bytes byte length of message.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. msg_bytes could be any value.
 */
uint32_t cpt_ske_cbc_mac_update(cpt_ske_cbc_mac_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ske_ret = ske_cbc_mac_update(ctx, msg, msg_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cbc_mac finish, and get the mac(CPU style)
 *
 * @param[in] ctx ske_cbc_mac_ctx_st context pointer
 * @param[out] mac input(for verifying mac)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 *     2. for the case that padding is SKE_NO_PADDING, if the total length of message is not a multiple of
 *        block length, it will return error.
 */
uint32_t cpt_ske_cbc_mac_final(cpt_ske_cbc_mac_st *ctx, uint8_t *mac)
{
    uint32_t ske_ret = ske_cbc_mac_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 *   @brief      CBCMAC DMA mode context swtich to CPU mode context
 *
 *   @param [in] dma_ctx The context of cbc_mac in dma mode.
 *   @param [in] cpu_ctx The context of cbc_mac in cpu mode.
 *   @param [in] dma_data_sz The size of data updated by dma mode, unit is byte.
 *   @param [in] is_verify The flag of verify or generate
 *
 *
 *   @note cbc mac crypto hardware of lp version does not support dma mode, so the dma_ctx is cpu_ctx.
 */
void cpt_cbc_mac_ctx_dma2cpu(
    const cpt_ske_cbc_mac_dma_st *dma_ctx, cpt_ske_cbc_mac_st *cpu_ctx, uint32_t dma_data_sz, bool_t is_verify)
{
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
    UNUSED(dma_data_sz);
    UNUSED(is_verify);
    // LP版CBC MAC不支持DMA，DMA ctx 实际就是 CPU ctx
    util_memcpy(cpu_ctx, dma_ctx, sizeof(cpt_ske_cbc_mac_dma_st));
#else
    util_memcpy(cpu_ctx->ske_cbc_mac_ctx, dma_ctx->ske_cbc_mac_ctx, sizeof(cpu_ctx->ske_cbc_mac_ctx[0]));
    cpu_ctx->mac_bytes = dma_ctx->mac_bytes;
    cpu_ctx->is_updated = (uint8_t)((0U == dma_data_sz) ? 0U : 1U);
    cpu_ctx->mac_action = (is_verify) ? SKE_VERIFY_MAC : SKE_GENERATE_MAC;
    // The CBCMAC is set as default as SKE_ZERO_PADDING
    cpu_ctx->padding = SKE_ZERO_PADDING;
    // This funciton will only be called after the process of DMA update, so that the left_bytes is 0U
    cpu_ctx->left_bytes = 0U;
#endif
}

/**
 * @brief ske cbc mac init(DMA style)
 *
 * @param[in] ctx ske_cbc_dma_mac_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] mac_bytes mac byte length, must be bigger than 1, and not bigger than block length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. Due to SKE_LP hardware not support dma cbc_mac, this function will use CPU mode to init the context.
 */
uint32_t cpt_ske_dma_cbc_mac_init(
    cpt_ske_cbc_mac_dma_st *ctx, cpt_ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ske_ret;
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
    // LP版CBC MAC不支持DMA，退化为CPU模式
    ske_ret = ske_cbc_mac_init(ctx, alg, SKE_GENERATE_MAC, SKE_ZERO_PADDING, key, sp_key_idx, mac_bytes);
#else
    ske_ret = ske_dma_cbc_mac_init(ctx, alg, key, sp_key_idx, mac_bytes);
#endif
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cbc mac dma style update message blocks(excluding the last block, or the message tail)
 *
 * @param[in] ctx ske_cbc_dma_mac_ctx_st context pointer
 * @param[in] msg message of some blocks, excluding last block(or message tail)
 * @param[in] msg_bytes byte length of msg, must be a multiple of block byte length
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 *     2. if the SKE hardware is LP version, this function will use cpu mode to update the message.
 */
uint32_t cpt_ske_dma_cbc_mac_update_blocks_excluding_last_block(cpt_ske_cbc_mac_dma_st *ctx, uint32_t msg_h,
    uint32_t msg_l, uint32_t msg_bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
#if CONFIG_EHSM_SKE_HW_TYPE == CRYPTO_HW_LP
    // LP版CBC MAC不支持DMA，退化为CPU模式
    UNUSED(callback);
    UNUSED(dma_cfg);
    return cbcmac_cpu_update_internal(msg_h, msg_l, msg_bytes, ctx);
#else
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_cbc_mac_update_blocks_excluding_last_block(ctx, msg_h, msg_l, msg_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
#endif
}

/**
 * @brief get the ske context of cbc mac dam context
 *
 * @param [in] cmac_dma_ctx the context of cmac in dma mode
 */
cpt_ske_ctx_st *cpt_get_cbcmac_dma_ske_ctx(cpt_ske_cbc_mac_dma_st *cmac_dma_ctx)
{
    return cmac_dma_ctx->ske_cbc_mac_ctx;
}

/**
 * @brief get the ske context of cbc mac cpu context
 *
 * @param [in] cmac_cpu_ctx the context of cmac in cpu mode
 *
 */
cpt_ske_ctx_st *cpt_get_cbcmac_cpu_ske_ctx(cpt_ske_cbc_mac_st *cmac_cpu_ctx)
{
    return cmac_cpu_ctx->ske_cbc_mac_ctx;
}

/**
 * @brief      CBCMAC context switch function between remote context and local DMA context
 *
 * @param [in] mac_ctx_addr The address of remote context
 * @param [in] lctx The context of local DMA mode
 * @param [in] type The type of context switch, MAC_RCTX_2_LCTX or MAC_LCTX_2_RCTX
 *
 * @return     uint32_t
 *
 * @note before uploading the context to remote, the sensetive information should be cleared.
 */
uint32_t cpt_cbcmac_rctx_switch_dma_ctx(const raddr_t mac_ctx_addr, cpt_ske_cbc_mac_dma_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    cpt_ske_cbc_mac_st rctx[1];
    uint8_t *map_addr = mmap_remap_addr_u64(mac_ctx_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_ske_cbc_mac_st));
        if (MAC_RCTX_2_LCTX == type) {
            util_memcpy(lctx->ske_cbc_mac_ctx, rctx->ske_cbc_mac_ctx, sizeof(lctx->ske_cbc_mac_ctx[0]));
            lctx->mac_bytes = rctx->mac_bytes;
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_cbc_mac_ctx);

            util_memcpy(rctx->ske_cbc_mac_ctx, lctx->ske_cbc_mac_ctx, sizeof(rctx->ske_cbc_mac_ctx[0]));
            rctx->mac_bytes = lctx->mac_bytes;
            util_memcpy(map_addr, rctx, sizeof(cpt_ske_cbc_mac_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      CBCMAC context switch function between remote context and local CPU context
 *
 *   @param [in] mac_ctx_addr The address of remote context
 *   @param [in] lctx The context of local CPU mode
 *   @param [in] type The type of context switch, MAC_RCTX_2_LCTX or MAC_LCTX_2_RCTX
 *
 *   @return     uint32_t
 *
 *   @note before uploading the context to remote, the sensetive information should be cleared.
 */
uint32_t cpt_cbcmac_rctx_switch_cpu_ctx(const raddr_t mac_ctx_addr, cpt_ske_cbc_mac_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    uint8_t *map_addr = mmap_remap_addr_u64(mac_ctx_addr);
    if (NULL != map_addr) {
        if (MAC_RCTX_2_LCTX == type) {
            util_memcpy(lctx, map_addr, sizeof(cpt_ske_cbc_mac_st));
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_cbc_mac_ctx);
            util_memcpy(map_addr, lctx, sizeof(cpt_ske_cbc_mac_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      CMAC DMA mode context swtich to CPU mode context
 *
 *   @param [in] dma_ctx The context of DMA mode
 *   @param [in] cpu_ctx The context of CPU mode
 *   @param [in] is_verify The flag of verify or generate
 *
 *   @note
 */
void cpt_cmac_ctx_dma_2_cpu(const cpt_ske_cmac_dma_ctx_st *dma_ctx, cpt_ske_cmac_ctx_st *cpu_ctx, uint8_t is_verify)
{
    util_memcpy(cpu_ctx->ske_cmac_ctx, dma_ctx->ske_cmac_ctx, sizeof(cpu_ctx->ske_cmac_ctx));
    cpu_ctx->mac_bytes = dma_ctx->mac_bytes;
    cpu_ctx->mac_action = (is_verify) ? SKE_VERIFY_MAC : SKE_GENERATE_MAC;
    // This funciton will only be called after the process of DMA update, so that the left_bytes is 0U
    cpu_ctx->left_bytes = 0U;
}

/**
 * @brief ske cmac init(CPU style)
 *
 * @param[in] ctx ske_cmac_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] mac_action must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 * @param[in] mac_bytes mac byte length, must be bigger than 1, and not bigger than block length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t cpt_ske_cmac_init(cpt_ske_cmac_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_mac_e mac_action, const uint8_t *key,
    uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_cmac_init(ctx, alg, mac_action, key, sp_key_idx, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac update message(CPU style)
 *
 * @param[in] ctx ske_cmac_ctx_st context pointer
 * @param[in] msg message
 * @param[in] msg_bytes byte length of message.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure ctx is has been initialized
 *     2. msg_bytes could be any value.
 */
uint32_t cpt_ske_cmac_update(cpt_ske_cmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ske_ret = ske_cmac_update(ctx, msg, msg_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac finish, and get the mac or verify the mac(CPU style)
 *
 * @param[in] ctx ske_cmac_ctx_st context pointer
 * @param[out] mac input(for verifying mac)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure ctx is has been initialized
 *     2. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t cpt_ske_cmac_final(cpt_ske_cmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ske_ret = ske_cmac_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac(CPU style, one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] mac_action must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] msg message
 * @param[in] msg_bytes byte length of message.
 * @param[out] mac input(for verifying mac)
 * @param[in] mac_bytes mac byte length, must be bigger than 1, and not bigger than block length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes could be any value.
 *     3. if mac_action is SKE_GENERATE_MAC, mac is output. and if mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t cpt_ske_cmac(cpt_ske_alg_e alg, cpt_ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx,
    const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_cmac(alg, mac_action, key, sp_key_idx, msg, msg_bytes, mac, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac dma style init
 *
 * @param[in] ctx ske_cmac_dma_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] key key in bytes
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] mac_bytes mac byte length, must be bigger than 1, and not bigger than block length
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t cpt_ske_dma_cmac_init(
    cpt_ske_cmac_dma_ctx_st *ctx, cpt_ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_dma_cmac_init(ctx, alg, key, sp_key_idx, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac dma style update update message blocks(excluding the last block, or the message tail)
 *
 * @param[in] ctx ske_cmac_dma_st context pointer
 * @param[in] msg message of some blocks, excluding last block(or message tail)
 * @param[in] msg_bytes byte length of msg, must be a multiple of block byte length
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
uint32_t cpt_ske_dma_cmac_update_blocks_excluding_last_block(cpt_ske_cmac_dma_ctx_st *ctx, uint32_t msg_h,
    uint32_t msg_l, uint32_t msg_bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_cmac_update_blocks_excluding_last_block(ctx, msg_h, msg_l, msg_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske cmac(DMA style, one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] key key in byte buffer style
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] msg message
 * @param[in] msg_bytes byte length of message.
 * @param[out] mac mac
 * @param[in] mac_bytes byte length of mac
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes is actual byte length of message, it could be any value(including 0).
 *        (1). if msg_bytes is not 0, msg must have (msg_bytes+15)/16 blocks, if the last block is not full,
 *        please pad with zero.
 *        (2). if msg_bytes is 0, msg occupies a block.
 */
uint32_t cpt_ske_dma_cmac(cpt_ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l,
    uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, uint8_t mac_bytes, cpt_ske_callback callback, uint32_t dma_cfg)
{
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    uint32_t ske_ret = ske_dma_cmac(alg, key, sp_key_idx, msg_h, msg_l, msg_bytes, mac_h, mac_l, mac_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 *   @brief get the ske context of cmac dam context
 *
 *   @param [in] cmac_dma_ctx the context of cmac in dma mode
 */
cpt_ske_ctx_st *cpt_get_cmac_dma_ske_ctx(cpt_ske_cmac_dma_ctx_st *cmac_dma_ctx)
{
    return cmac_dma_ctx->ske_cmac_ctx;
}

/**
 *   @brief get the ske context of cmac cpu context
 *
 *   @param [in] cmac_cpu_ctx the context of cmac in cpu mode
 *
 */
cpt_ske_ctx_st *cpt_get_cmac_cpu_ske_ctx(cpt_ske_cmac_ctx_st *cmac_cpu_ctx)
{
    return cmac_cpu_ctx->ske_cmac_ctx;
}

/**
 *   @brief      CMAC context switch function between remote context and local DMA context
 *
 *   @param [in] mac_rctx_addr remote context address
 *   @param [in] lctx local context
 *   @param [in] type MAC_RCTX_2_LCTX or MAC_LCTX_2_RCTX
 *
 *   @return     uint32_t
 *
 *   @note before uploading the context to remote, the sensetive information should be cleared.
 */
uint32_t cpt_cmac_rctx_switch_dma_ctx(const raddr_t mac_rctx_addr, cpt_ske_cmac_dma_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    cpt_ske_cmac_ctx_st rctx[1];
    uint8_t *map_addr = mmap_remap_addr_u64(mac_rctx_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_ske_cmac_ctx_st));
        if (MAC_RCTX_2_LCTX == type) {
            util_memcpy(lctx->ske_cmac_ctx, rctx->ske_cmac_ctx, sizeof(lctx->ske_cmac_ctx[0]));
            lctx->mac_bytes = rctx->mac_bytes;
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_cmac_ctx);
            util_memcpy(rctx->ske_cmac_ctx, lctx->ske_cmac_ctx, sizeof(rctx->ske_cmac_ctx[0]));
            rctx->mac_bytes = lctx->mac_bytes;
            util_memcpy(map_addr, rctx, sizeof(cpt_ske_cmac_ctx_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
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
 *   @note before uploading the context to remote, the sensetive information should be cleared.
 */
uint32_t cpt_cmac_rctx_switch_cpu_ctx(const raddr_t mac_rctx_addr, cpt_ske_cmac_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    uint8_t *map_addr = mmap_remap_addr_u64(mac_rctx_addr);
    if (NULL != map_addr) {
        if (MAC_RCTX_2_LCTX == type) {
            util_memcpy(lctx, map_addr, sizeof(cpt_ske_cmac_ctx_st));
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_cmac_ctx);
            util_memcpy(map_addr, lctx, sizeof(cpt_ske_cmac_ctx_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}


/**
 * @brief ske gmac mode init config(CPU style)
 *
 * @param[in] ctx ske_gmac_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] mac_action must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 * @param[in] key key in bytes, key of AES(128/192/256) or SM4
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes
 * @param[in] iv_bytes byte length of iv, now only 12 bytes supported
 * @param[in] msg_bytes byte length of msg, it could be any value, including 0
 * @param[in] mac_bytes byte length of mac, must be in [1,16]
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GMAC mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. msg_bytes could be zero
 */
uint32_t cpt_ske_gmac_init(cpt_ske_gmac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_gmac_init(ctx, alg, mac_action, key, sp_key_idx, iv, iv_bytes, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske gmac mode input msg(stepwise style)
 *
 * @param[in] ctx ske_gmac_ctx_st context pointer
 * @param[in] msg msg
 * @param[in] bytes real byte length of msg
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after calling ske_gmac_init()
 *     2. if there is no msg, this function could be omitted
 *     3. msg_bytes could be any value
 */
uint32_t cpt_ske_gmac_update(cpt_ske_gmac_ctx_st *ctx, const uint8_t *msg, uint32_t bytes)
{
    uint32_t ske_ret = ske_gmac_update(ctx, msg, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske gmac mode finish
 *
 * @param[in] ctx ske_gmac_ctx_st context pointer
 * @param[out] mac input(for verifying mac)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes must be in [1, SKE_GCM_MAX_BYTES]
 *     3. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t cpt_ske_gmac_final(cpt_ske_gmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ske_ret = ske_gmac_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief get the ske context of gmac cpu context
 *
 * @param[in] gmac_ctx ske_gmac_ctx_st context pointer
 *
 */
cpt_ske_ctx_st *cpt_get_gmac_cpu_ske_ctx(cpt_ske_gmac_ctx_st *gmac_ctx)
{
    return gmac_ctx->ske_gcm_ctx[0].ske_gcm_ctx;
}

/**
 *   @brief      GMAC context switch function between remote context and local CPU context
 *
 *   @param [in] mac_ctx_addr remote context address
 *   @param [in] lctx local context
 *   @param [in] type MAC_RCTX_2_LCTX or MAC_LCTX_2_RCTX
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t cpt_gmac_rctx_switch_cpu_ctx(const raddr_t mac_ctx_addr, cpt_ske_gmac_ctx_st *lctx, mac_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    uint8_t *map_addr = mmap_remap_addr_u64(mac_ctx_addr);
    if (NULL != map_addr) {
        if (MAC_RCTX_2_LCTX == type) {
            util_memcpy(lctx, map_addr, sizeof(cpt_ske_gmac_ctx_st));
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_gcm_ctx[0].ske_gcm_ctx);
            util_memcpy(map_addr, lctx, sizeof(cpt_ske_gmac_ctx_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 * @brief ske gcm mode init config(CPU style)
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, key of AES(128/192/256) or SM4
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
              if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes
 * @param[in] iv_bytes byte length of iv, now only 12 bytes is supported
 * @param[in] aad_bytes byte length of aad, it could be any value, including 0
 * @param[in] c_bytes byte length of plaintext/ciphertext, it could be any value, including 0
 * @param[in] mac_bytes byte length of mac, must be in [0,16]
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could be zero at the same time
 */
uint32_t cpt_ske_gcm_init(cpt_ske_gcm_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes, uint8_t mac_bytes)
{
    uint32_t ske_ret = ske_gcm_init(ctx, alg, crypto, key, sp_key_idx, iv, iv_bytes, aad_bytes, c_bytes, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske gcm mode input aad(stepwise style)
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] aad aad
 * @param[in] bytes real byte length of aad
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after calling ske_gcm_init()
 *     2. if there is no aad, this function could be omitted
 *     3. if the whole aad is too long, you could divide it into some sections by block, then call
 *        this function to input the sections respectively. for example, if the whole aad byte
 *        length is 65, it could be divided into 3 sections with byte length 16,32,17 respectively.
 */
uint32_t cpt_ske_gcm_update_blocks_aad(cpt_ske_gcm_ctx_st *ctx, const uint8_t *aad, uint32_t bytes)
{
    uint32_t ske_ret = ske_gcm_update_blocks_aad(ctx, aad, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske gcm mode input plaintext/ciphertext
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     4. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
uint32_t cpt_ske_gcm_update_blocks(cpt_ske_gcm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ske_ret = ske_gcm_update_blocks(ctx, in, out, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske gcm mode finish
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] mac output(for encryption)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_gcm_final(cpt_ske_gcm_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ske_ret = ske_gcm_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske dma gcm mode update some plaintext/ciphertext blocks
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] in plaintext/ciphertext of some blocks
 * @param[in] in_bytes byte length of in/out
 * @param[in] out ciphertext/plaintext of some blocks
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after calling
 *        ske_dma_gcm_init(), if aad is empty
 *     or ske_dma_gcm_update_blocks_whole_aad(), if aad is not empty
 *     2. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
uint32_t cpt_ske_dma_gcm_update_blocks(cpt_ske_gcm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes,
    uint32_t out_h, uint32_t out_l, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_gcm_update_blocks(ctx, in_h, in_l, in_bytes, out_h, out_l, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske dma gcm mode finish
 *
 * @param[in] ctx ske_gcm_ctx_st context pointer
 * @param[in] mac_h the address of mac in high 32 bits output(for encryption)
 * @param[in] mac_l the address of mac in low 32 bits output(for encryption)
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_dma_gcm_update_final(cpt_ske_gcm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_gcm_update_final(ctx, mac_h, mac_l);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief get the ske context of gcm context
 *
 * @param gcm_ctx ske_gcm_ctx_st context pointer
 * @return ske_ctx_st
 */
ske_ctx_st *cpt_get_gcm_ske_ctx(ske_gcm_st *gcm_ctx)
{
    return gcm_ctx->ske_gcm_ctx;
}

/**
 * @brief      GCM context switch function between remote context and local context
 *
 * @param [in] context_addr remote context address
 * @param [in] lctx local context
 * @param [in] type GCM_RCTX_2_LCTX or GCM_LCTX_2_RCTX
 *
 * @return     uint32_t
 *
 * @note will clear the sensetive information in local context before uploading to remote context
 */
uint32_t cpt_gcm_rctx_switch_ctx(const raddr_t context_addr, ske_gcm_st *lctx, aead_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    uint8_t *map_addr = mmap_remap_addr_u64(context_addr);

    if (NULL != map_addr) {
        if (AEAD_RCTX_2_LCTX == type) {
            util_memcpy(lctx, map_addr, sizeof(ske_gcm_st));
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_gcm_ctx);
            util_memcpy(map_addr, lctx, sizeof(ske_gcm_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 * @brief ske ccm mode init config
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] alg ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, key of AES(128/192/256) or SM4
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 * @param[in] nonce nonce in bytes, its byte lenth is 15-L
 * @param[in] M bytes of authentication field(bytes of mac)
 * @param[in] L bytes of length field(message byte length is less than 256^L)
 * @param[in] aad_bytes byte length of aad, it could be any value, including 0
 * @param[in] c_bytes byte length of plaintext/ciphertext, it could be any value, including 0
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for CCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 */
uint32_t cpt_ske_ccm_init(cpt_ske_ccm_ctx_st *ctx, cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t ske_ret = ske_ccm_init(ctx, alg, crypto, key, sp_key_idx, nonce, M, L, aad_bytes, c_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske ccm mode input aad(one-off style)
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] aad aad, its length is ctx->aad_bytes, please make sure
              aad here is integral
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after calling ske_ccm_init()
 *     2. if there is no aad, this function could be omitted
 */
uint32_t cpt_ske_ccm_update_aad(cpt_ske_ccm_ctx_st *ctx, const uint8_t *aad)
{
    uint32_t ske_ret = ske_ccm_update_aad(ctx, aad);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske ccm mode input plaintext/ciphertext
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] bytes byte length of input or output
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     4. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it
 *        could be divided into 3 sections with byte length 32,16,17 respectively.
 */
uint32_t cpt_ske_ccm_update_blocks(cpt_ske_ccm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ske_ret = ske_ccm_update_blocks(ctx, in, out, bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske ccm mode finish
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] mac output(for encryption)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. byte length of mac is ctx->M
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_ccm_final(cpt_ske_ccm_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ske_ret = ske_ccm_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske dma ccm mode update some plaintext/ciphertext blocks
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] in plaintext/ciphertext of some blocks
 * @param[in] in_bytes byte length of in/out
 * @param[in] out ciphertext/plaintext of some blocks
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for DMA style
 *     2. this function must be called after calling
 *        ske_dma_ccm_init(), if aad is empty
 *     or ske_dma_ccm_update_blocks_B0_and_whole_aad(), if aad is not empty
 *     3. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
uint32_t cpt_ske_dma_ccm_update_blocks(cpt_ske_ccm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes,
    uint32_t out_h, uint32_t out_l, cpt_ske_callback callback, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_ccm_update_blocks(ctx, in_h, in_l, in_bytes, out_h, out_l, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief ske dma ccm mode finish
 *
 * @param[in] ctx ske_ccm_ctx_st context pointer
 * @param[in] mac_h the address of mac in high 32 bits output(for encryption)
 * @param[in] mac_l the address of mac in low 32 bits output(for encryption)
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. byte length of mac is ctx->M
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_dma_ccm_update_final(cpt_ske_ccm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l, uint32_t dma_cfg)
{
    uint32_t ske_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    ske_ret = ske_dma_ccm_update_final(ctx, mac_h, mac_l);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief get the ske context of ccm context
 *
 * @param gcm_ctx ske_ccm_ctx_st context pointer
 * @return ske_ctx_st
 */
ske_ctx_st *cpt_get_ccm_ske_ctx(cpt_ske_ccm_ctx_st *ccm_ctx)
{
    return ccm_ctx->ske_ccm_ctx;
}

/**
 *   @brief      CCM context switch function between remote context and local context
 *
 *   @param [in] context_addr remote context address
 *   @param [in] lctx local context
 *   @param [in] type CCM_RCTX_2_LCTX or CCM_LCTX_2_RCTX
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t cpt_ccm_rctx_switch_ctx(const raddr_t context_addr, cpt_ske_ccm_ctx_st *lctx, aead_switch_e type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t level = cpu_enter_critical();
    uint8_t *map_addr = mmap_remap_addr_u64(context_addr);

    if (NULL != map_addr) {
        if (AEAD_RCTX_2_LCTX == type) {
            util_memcpy(lctx, map_addr, sizeof(cpt_ske_ccm_ctx_st));
        } else {
            // Clear the sensetive information before uploading the ctx
            cpt_clear_ske_ctx_key(lctx->ske_ccm_ctx);
            util_memcpy(map_addr, lctx, sizeof(cpt_ske_ccm_ctx_st));
        }
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Switch the context of DMA mode between HOST and EHSM
 *
 *   @param [in] rctx_addr remote context address
 *   @param [in] lctx      local context pointer
 *   @param [in] type      context switch type
 *
 *   @return     uint32_t
 *
 *   @note
 *         This function is defined as a global function since it may be used when the SM2
 *         function need to used HASH-DMA API in streamstart function
 */
uint32_t cpt_hash_rctx_switch_dma_ctx(raddr_t rctx_addr, cpt_hash_dma_ctx_st *lctx, cpt_hash_switch_e type)
{
    uint32_t ret;
    uint32_t level = cpu_enter_critical();
    cpt_hash_rctx_st rctx[1];
    uint8_t *map_addr = mmap_remap_addr_u64(rctx_addr);

    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_hash_rctx_st));
        if (HASH_RCTX_2_LCTX == type) {
            util_memcpy(lctx->iterator, rctx->iterator, sizeof(lctx->iterator));
            util_memcpy(lctx->total, rctx->total, sizeof(lctx->total));
            lctx->alg = rctx->hash_alg;
            lctx->hfe_mode = rctx->hfe_mode;
            lctx->block_word_len = (rctx->block_byte_len >> 2);
            lctx->iterator_word_len = rctx->iterator_word_len;
            lctx->first_update_flag = rctx->first_update_flag;
            lctx->digest_byte_len = rctx->digest_byte_len;
            lctx->callback = NULL;
        } else {
            util_memcpy(rctx->iterator, lctx->iterator, sizeof(lctx->iterator));
            util_memcpy(rctx->total, lctx->total, sizeof(lctx->total));
            rctx->hash_alg = lctx->alg;
            rctx->hfe_mode = lctx->hfe_mode;
            rctx->block_byte_len = (uint8_t)(lctx->block_word_len << 2);
            rctx->iterator_word_len = lctx->iterator_word_len;
            rctx->first_update_flag = lctx->first_update_flag;
            rctx->digest_byte_len = lctx->digest_byte_len;
            util_memcpy(map_addr, rctx, sizeof(cpt_hash_rctx_st));
        }
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Switch the context of CPU mode between HOST and EHSM
 *
 *   @param [in] rctx_addr remote context address
 *   @param [in] lctx      local context pointer
 *   @param [in] type      context switch type
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t cpt_hash_rctx_switch_cpu_ctx(raddr_t rctx_addr, cpt_hash_ctx_st *lctx, cpt_hash_switch_e type)
{
    uint32_t ret;
    uint32_t level = cpu_enter_critical();
    cpt_hash_rctx_st rctx[1];
    uint8_t *map_addr = mmap_remap_addr_u64(rctx_addr);

    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_hash_rctx_st));
        if (HASH_RCTX_2_LCTX == type) {
            util_memcpy(lctx->iterator, rctx->iterator, sizeof(lctx->iterator));
            util_memcpy(lctx->hash_buffer, rctx->hash_buffer, sizeof(lctx->hash_buffer));
            util_memcpy(lctx->total, rctx->total, sizeof(lctx->total));
            lctx->alg = rctx->hash_alg;
            lctx->hfe_mode = rctx->hfe_mode;
            lctx->block_byte_len = rctx->block_byte_len;
            lctx->iterator_word_len = rctx->iterator_word_len;
            lctx->first_update_flag = rctx->first_update_flag;
            lctx->digest_byte_len = rctx->digest_byte_len;
            // The busy flag will be clear while the calling is over in the CryLib API, so it will always be 0 in ctx
            lctx->status.busy = 0U;
            // The finish flag will be set only at the beginnig of the CryLib final API, so it will always be 0 in ctx
            lctx->finish_flag = 0U;
        } else {
            util_memcpy(rctx->iterator, lctx->iterator, sizeof(lctx->iterator));
            util_memcpy(rctx->hash_buffer, lctx->hash_buffer, sizeof(lctx->hash_buffer));
            util_memcpy(rctx->total, lctx->total, sizeof(lctx->total));
            rctx->hash_alg = lctx->alg;
            rctx->hfe_mode = lctx->hfe_mode;
            rctx->block_byte_len = lctx->block_byte_len;
            rctx->iterator_word_len = lctx->iterator_word_len;
            rctx->first_update_flag = lctx->first_update_flag;
            rctx->digest_byte_len = lctx->digest_byte_len;
            util_memcpy(map_addr, rctx, sizeof(cpt_hash_rctx_st));
        }
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Switch the context between DMA and CPU mode
 *
 *   @param [in] cpu_ctx CPU context pointer
 *   @param [in] dma_ctx DMA context pointer
 *   @param [in] type    context switch type
 *
 *   @note
 *         cpt_hash_ctx_switch() does not need return code, since it will only be used in SM2-Srv once,
 *         which's situation is very simple
 */
void cpt_hash_ctx_switch(cpt_hash_ctx_st *cpu_ctx, cpt_hash_dma_ctx_st *dma_ctx, cpt_hash_ctx_switch_e type)
{
    if (HASH_CPU_2_DMA == type) {
        util_memcpy(dma_ctx->iterator, cpu_ctx->iterator, sizeof(dma_ctx->iterator));
        util_memcpy(dma_ctx->total, cpu_ctx->total, sizeof(dma_ctx->total));
        // The callback function will not be used in EHSM
        dma_ctx->callback = NULL;
        dma_ctx->alg = cpu_ctx->alg;
        dma_ctx->hfe_mode = cpu_ctx->hfe_mode;
        dma_ctx->block_word_len = (cpu_ctx->block_byte_len / 4U);
        dma_ctx->iterator_word_len = cpu_ctx->iterator_word_len;
        dma_ctx->first_update_flag = cpu_ctx->first_update_flag;
        dma_ctx->digest_byte_len = cpu_ctx->digest_byte_len;
    } else {
        util_memcpy(cpu_ctx->iterator, dma_ctx->iterator, sizeof(cpu_ctx->iterator));
        util_memcpy(cpu_ctx->total, dma_ctx->total, sizeof(cpu_ctx->total));
        // The busy signal will only be set in the CryLib function, and it will be clear before the function return
        cpu_ctx->status.busy = 0U;
        cpu_ctx->alg = dma_ctx->alg;
        cpu_ctx->hfe_mode = dma_ctx->hfe_mode;
        cpu_ctx->block_byte_len = (dma_ctx->block_word_len * 4U);
        cpu_ctx->iterator_word_len = dma_ctx->iterator_word_len;
        cpu_ctx->digest_byte_len = dma_ctx->digest_byte_len;
        cpu_ctx->first_update_flag = dma_ctx->first_update_flag;
        // The finish flag will only be set in final API, so that it will always be 0 in other function-call
        cpu_ctx->finish_flag = 0U;
    }
}

/**
 * @brief init HASH
 *
 * @param[in] ctx hash_ctx_st context pointer
 * @param[in] alg specific hash algorithm
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure alg is valid
 */
uint32_t cpt_hash_init(cpt_hash_ctx_st *ctx, cpt_hash_alg_e alg)
{
    uint32_t hash_ret = hash_init(ctx, alg);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief hash update message
 *
 * @param[in] ctx hash_ctx_st context pointer
 * @param[in] msg message
 * @param[in] msg_bytes byte length of the input message
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t cpt_hash_update(cpt_hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t hash_ret = hash_update(ctx, msg, msg_bytes);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief message update done, get the digest
 *
 * @param[in] ctx hash_ctx_st context pointer
 * @param[out] digest hash digest
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the digest buffer is sufficient
 */
uint32_t cpt_hash_final(cpt_hash_ctx_st *ctx, uint8_t *digest)
{
    uint32_t hash_ret = hash_final(ctx, digest);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief input whole message and get its digest
 *
 * @param[in] alg specific hash algorithm
 * @param[in] msg message
 * @param[in] msg_bytes byte length of the input message, it could be 0
 * @param[out] digest hash digest
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t cpt_hash(cpt_hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    uint32_t hash_ret = hash(alg, msg, msg_bytes, digest);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief dma hash update some message blocks
 *
 * @param[in] ctx hash_dma_ctx_st context pointer
 * @param[in] msg message blocks
 * @param[in] msg_bytes byte length of the input message, must be a multiple of hash block byte length
 * @param[in] dma_cfg the hash dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t cpt_hash_dma_update_blocks(
    cpt_hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t dma_cfg)
{
    uint32_t hash_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    hash_ret = hash_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief dma hash final(input the remainder message and get the digest)
 *
 * @param[in] ctx hash_dma_ctx_st context pointer
 * @param[in] remainder_msg remainder message
 * @param[in] remainder_bytes byte length of the remainder message
 * @param[out] digest hash digest
 * @param[in] dma_cfg the hash dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the four parameters are valid, and ctx is initialized
 *     2. if remainder_msg is NULL, or remainder_bytes is zero, in this case input valid,
 *        means the message is NULL.
 */
uint32_t cpt_hash_dma_final(cpt_hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes,
    uint32_t digest_h, uint32_t digest_l, uint32_t dma_cfg)
{
    uint32_t hash_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    hash_ret = hash_dma_final(ctx, msg_h, msg_l, msg_bytes, digest_h, digest_l);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief dma hash digest calculate
 *
 * @param[in] alg specific hash algorithm
 * @param[in] msg message
 * @param[in] msg_bytes byte length of the message, it could be 0
 * @param[out] digest hash digest
 * @param[in] callback callback function pointer
 * @param[in] dma_cfg the hash dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the four parameters are valid
 */
uint32_t cpt_hash_dma(cpt_hash_alg_e alg, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h,
    uint32_t digest_l, cpt_hash_callback callback, uint32_t dma_cfg)
{
    uint32_t hash_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    hash_ret = hash_dma(alg, msg_h, msg_l, msg_bytes, digest_h, digest_l, callback);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief build the dma hmac context without hash
 *
 * @param key the hmac key data, it could be NULL when is_sp_key is true
 * @param key_sz the hmac key data size
 * @param sp_key_id the sp key id, it could be 0 when is_sp_key is false
 * @param is_sp_key the key is sp key or not
 * @param ctx the dma hmac context
 * @return
 */
uint32_t cpt_build_hmac_cpu_ctx_without_hash(
    uint8_t *key, uint32_t key_sz, uint8_t sp_key_id, bool_t is_sp_key, cpt_hmac_ctx_st *ctx)
{
    uint32_t ret;
    cpt_hmac_ctx_st tmp_ctx;
    // Using the cpt_hmac_init() to recovery the K0
    if (is_sp_key == false) {
        ret = cpt_hmac_init(&tmp_ctx, ctx->hash_ctx->alg, key, 0U, key_sz);
    } else {
        ret = cpt_hmac_init(&tmp_ctx, ctx->hash_ctx->alg, NULL, sp_key_id, key_sz);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        util_memcpy(ctx->K0, tmp_ctx.K0, sizeof(tmp_ctx.K0));
        ctx->key_len_flag = tmp_ctx.key_len_flag;
        ctx->is_sp_key = tmp_ctx.is_sp_key;
        ctx->sp_key_idx = tmp_ctx.sp_key_idx;
        ctx->sp_key_bytes = tmp_ctx.sp_key_bytes;
    }
    return ret;
}

/**
 * @brief build the cpu hmac context without hash
 *
 * @param key the hmac key data, it could be NULL when is_sp_key is true
 * @param key_sz the hmac key data size
 * @param sp_key_id the sp key id, it could be 0 when is_sp_key is false
 * @param is_sp_key the key is sp key or not
 * @param ctx the cpu hmac context
 * @return
 */
uint32_t cpt_build_hmac_dma_ctx_without_hash(
    uint8_t *key, uint32_t key_sz, uint8_t sp_key_id, bool_t is_sp_key, cpt_hmac_dma_ctx_st *ctx)
{
    uint32_t ret;
    cpt_hmac_ctx_st tmp_ctx;
    // Using the cpt_hmac_init() to recovery the K0
    if (is_sp_key == false) {
        ret = cpt_hmac_init(&tmp_ctx, ctx->hash_dma_ctx->alg, key, 0U, key_sz);
    } else {
        ret = cpt_hmac_init(&tmp_ctx, ctx->hash_dma_ctx->alg, NULL, sp_key_id, key_sz);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        util_memcpy(ctx->K0, tmp_ctx.K0, sizeof(tmp_ctx.K0));
        ctx->key_len_flag = tmp_ctx.key_len_flag;
        ctx->is_sp_key = tmp_ctx.is_sp_key;
        ctx->sp_key_idx = tmp_ctx.sp_key_idx;
        ctx->sp_key_bytes = tmp_ctx.sp_key_bytes;
    }
    return ret;
}
/**
 * @brief init HMAC
 *
 * @param[in] ctx hmac_ctx_st context pointer
 * @param[in] alg specific hash algorithm
 * @param[in] key key
 * @param[in] sp_key_idx index of secure port key
 * @param[in] key_bytes byte length of key, it could be 0
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure alg is valid
 *     2. if cfg not support secure port, please set key is NULL when hmac without key
 *     3. if cfg support secure port, hmac will use secure port key when parameter key is NULL,
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t cpt_hmac_init(
    cpt_hmac_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes)
{
    uint32_t hash_ret = hmac_init(ctx, alg, key, sp_key_idx, key_bytes);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief hmac update message
 *
 * @param[in] ctx hmac_ctx_st context pointer
 * @param[in] msg message
 * @param[in] msg_bytes byte length of the input message
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t cpt_hmac_update(cpt_hmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t hash_ret = hmac_update(ctx, msg, msg_bytes);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief message update done, get the hmac
 *
 * @param[in] ctx hmac_ctx_st context pointer
 * @param[out] mac hmac
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the mac buffer is sufficient
 */
uint32_t cpt_hmac_final(cpt_hmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t hash_ret = hmac_final(ctx, mac);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief dma hmac input key and message, get the hmac
 * @param[in] alg ------------------------ input, specific hash algorithm
 * @param[in] key ------------------------ input, key
 * @param[in] sp_key_idx ----------------- input, index of secure port key
 * @param[in] key_bytes ------------------ input, key byte length
 * @param[in] msg ------------------------ input, message
 * @param[in] msg_bytes ------------------ input, byte length of the input message
 * @param[out] mac ------------------------ output, hmac
 * @param[in] callback ------------------- input, callback function pointer
 * @param[in] dma_cfg --------------------- input, the hash dma data transfer configuration, see sysreg_ahb_dma_cfg()
 * for detail.
 * @return HASH_SUCCESS(success), other(error)
 * @note
 *     1. please make sure alg is valid
 *     2. if cfg not support secure port, please set key is NULL when hmac without key
 *     3. if cfg support secure port, hmac will use secure port key when parameter key is NULL,
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t cpt_hmac_dma(cpt_hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t msg_h,
    uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, cpt_hash_callback callback, uint32_t dma_cfg)
{
    uint32_t hash_ret;
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    hash_ret = hmac_dma(alg, key, sp_key_idx, key_bytes, msg_h, msg_l, msg_bytes, mac_h, mac_l, callback);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/* @brief: dma hmac update message
 * @param[in] ctx ------------------------ input, hmac_dma_ctx_st context pointer
 * @param[in] msg ------------------------ input, message
 * @param[in] msg_bytes ------------------ input, byte length of the input message, must be a multiple of block byte
 * length of HASH
 * @param[in] dma_cfg --------------------- input, the hash dma data transfer configuration, see sysreg_ahb_dma_cfg()
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t cpt_hmac_dma_update_blocks(
    cpt_hmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t dma_cfg)
{
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    uint32_t hash_ret = hmac_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief get ECCP public key from private key, secure version(the key pair could be used in SM2/ECDSA/ECDH, etc.)
 *
 * @param[in] curve eccp_curve_st curve struct pointer
 * @param[in] priKey private key, big-endian
 * @param[out] pubKey public key, big-endian
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t cpt_eccp_get_pubkey_from_prikey(const cpt_eccp_curve_st *curve, const uint8_t *priKey, uint8_t *pubKey)
{
    uint32_t pke_ret = eccp_get_pubkey_from_prikey(curve, priKey, pubKey);
    return cpt_err_code_to_ehsm_code(pke_ret, PKE_SUCCESS);
}

/**
 * @brief get ECCP key pair, secure version(the key pair could be used in SM2/ECDSA/ECDH)
 *
 * @param[in] curve eccp_curve_st curve struct pointer
 * @param[out] priKey private key, big-endian
 * @param[out] pubKey public key, big-endian
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t cpt_eccp_getkey(const cpt_eccp_curve_st *curve, uint8_t *priKey, uint8_t *pubKey)
{
    uint32_t pke_ret = eccp_getkey(curve, priKey, pubKey);
    return cpt_err_code_to_ehsm_code(pke_ret, PKE_SUCCESS);
}

/**
 * @brief Generate ECDSA Signature in byte string style
 *
 * @param[in] curve ecc curve struct pointer, please make sure it is valid
 * @param[in] E hash value, U8 big-endian
 * @param[in] EByteLen byte length of E
 * @param[in] rand_k random big integer k in signing, U8 big-endian
 * @param[in] priKey private key, U8 big-endian
 * @param[out] signature signature r and s, U8 big-endian
 * @return
 *     ECDSA_SUCCESS_S(success); other(error)
 * @note
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t cpt_ecdsa_sign(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, const uint8_t *rand_k,
    const uint8_t *priKey, uint8_t *signature)
{
    uint32_t ecdsa_ret = ecdsa_sign(curve, E, EByteLen, rand_k, priKey, signature);
    return cpt_err_code_to_ehsm_code(ecdsa_ret, ECDSA_SUCCESS);
}

/**
 * @brief Verify ECDSA Signature in byte string style
 *
 * @param[in] curve ecc curve struct pointer, please make sure it is valid
 * @param[in] E hash value, U8 big-endian
 * @param[in] EByteLen byte length of E
 * @param[in] pubKey public key, U8 big-endian
 * @param[in] signature signature r and s, U8 big-endian
 * @return
 *     ECDSA_SUCCESS_S(success); other(error)
 * @note
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t cpt_ecdsa_verify(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, const uint8_t *pubKey,
    const uint8_t *signature)
{
    uint32_t ecdsa_ret = ecdsa_verify(curve, E, EByteLen, pubKey, signature);
    return cpt_err_code_to_ehsm_code(ecdsa_ret, ECDSA_SUCCESS);
}

/**
 * @brief out = a^e mod n
 *
 * @param[in] a uint32_t big integer a, base number, make sure a < n
 * @param[in] e uint32_t big integer e, exeponent, make sure e < n
 * @param[in] n uint32_t big integer n, modulus, make sure n is odd
 * @param[out] out out = a^e mod n
 * @param[in] eBitLen real bit length of uint32_t big integer e
 * @param[in] nBitLen real bit length of uint32_t big integer n
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 */
uint32_t cpt_rsa_modexp(
    const uint32_t *a, const uint32_t *e, const uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t rsa_ret = RSA_ModExp(a, e, n, out, eBitLen, nBitLen);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief generate RSA key (e,d,n)
 *
 * @param[out] e uint32_t big integer, RSA public key e
 * @param[out] d uint32_t big integer, RSA private key d
 * @param[out] n uint32_t big integer, RSA public module n
 * @param[in] eBitLen real bit length of e
 * @param[in] nBitLen real bit length of n
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t cpt_rsa_get_key(uint32_t *e, uint32_t *d, uint32_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t rsa_ret = RSA_GetKey(e, d, n, eBitLen, nBitLen);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief generate RSA-CRT key (e,p,q,dp,dq,u,n)
 *
 * @param[out] e uint32_t big integer, RSA public key e
 * @param[out] p uint32_t big integer, RSA private key p
 * @param[out] q uint32_t big integer, RSA private key q
 * @param[out] dp uint32_t big integer, RSA private key dp
 * @param[out] dq uint32_t big integer, RSA private key dq
 * @param[out] u uint32_t big integer, RSA private key u = q^(-1) mod p
 * @param[out] n uint32_t big integer, RSA public module n
 * @param[in] eBitLen real bit length of e
 * @param[in] nBitLen real bit length of n
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t cpt_rsa_get_crtkey(uint32_t *e, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u, uint32_t *n,
    uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t rsa_ret = RSA_GetCRTKey(e, p, q, dp, dq, u, n, eBitLen, nBitLen);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief out = a^d mod n
 *
 * @param[in] a uint32_t big integer a, base number, make sure a < n
 * @param[in] e uint32_t big integer e, public key, it muist be less than 2^64 here.
 * @param[in] d uint32_t big integer d, private key
 * @param[in] n uint32_t big integer n, modulus, please make sure n is odd
 * @param[out] out out = a^d mod n
 * @param[in] eBitLen real bit length of uint32_t big integer e, please make sure eBitLen <= 64.
 * @param[in] nBitLen real bit length of uint32_t big integer n
 * @return EHSM_ERR_SW_SUCCESS_S(success), other(error)
 * @note
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 *     2. please make sure 2 <= eBitLen <= 64.
 *     3. a and out can not point to the same buffer.
 *     4. the parameter e and eBitLen is only used for secure version, please make sure the parameter is valid.
 */
uint32_t cpt_rsa_modexp_with_pub(const uint32_t *a, const uint32_t *e, const uint32_t *d, const uint32_t *n,
    uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    UNUSED(e);
    UNUSED(eBitLen);
    uint32_t rsa_ret = RSA_ModExp(a, d, n, out, nBitLen, nBitLen);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 *
 * @param[in] a uint32_t big integer a, base number, make sure a < n=pq
 * @param[in] p uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 * @param[in] q uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 * @param[in] dp uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 * @param[in] dq uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 * @param[in] u uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 * @param[out] out out = a^d mod n
 * @param[in] nBitLen real bit length of uint32_t big integer n=pq
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. a, n and out have the same word length:((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:((nBitLen/2+31)>>5)
 *     2. the parameter e and eBitLen is only used for secure version, please make sure the parameter is valid.
 */
uint32_t cpt_rsa_crt_modexp_with_pub(const uint32_t *a, const uint32_t *p, const uint32_t *q, const uint32_t *dp,
    const uint32_t *dq, const uint32_t *u, const uint32_t *e, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    UNUSED(e);
    UNUSED(eBitLen);
    uint32_t rsa_ret = RSA_CRTModExp(a, p, q, dp, dq, u, out, nBitLen);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message digest
 *
 * @param[in] msg_hash_alg specific hash algorithm for message or Hash(message)
 * @param[in] mgf_hash_alg specific hash algorithm for MGF1
 * @param[in] salt salt
 * @param[in] salt_bytes byte length of salt
 * @param[in] msg_digest Hash(message), message is to be signed, here Hash is msg_hash_alg.
 * @param[in] d RSA private key d, (n_bits+7)/8 bytes, big-endian.
 * @param[in] n RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 * @param[in] n_bits bit length of n
 * @param[out] signature RSA signature, (n_bits+7)/8 bytes, big-endian.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t cpt_rsa_ssa_pss_sign_by_msg_digest(cpt_hash_alg_e msg_hash_alg, cpt_hash_alg_e mgf_hash_alg,
    const uint8_t *salt, uint32_t salt_bytes, const uint8_t *msg_digest, const uint8_t *d, const uint8_t *n,
    uint32_t n_bits, uint8_t *signature)
{
    uint32_t rsa_ret = rsa_ssa_pss_sign_by_msg_digest(
        msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, d, n, n_bits, signature);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message
 *
 * @param[in] msg_hash_alg specific hash algorithm for message or Hash(message)
 * @param[in] mgf_hash_alg specific hash algorithm for MGF1
 * @param[in] salt salt
 * @param[in] salt_bytes byte length of salt
 * @param[in] msg message to be signed
 * @param[in] msg_bytes byte length of message
 * @param[in] d RSA private key d, (n_bits+7)/8 bytes, big-endian.
 * @param[in] n RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 * @param[in] n_bits bit length of n
 * @param[out] signature RSA signature, (n_bits+7)/8 bytes, big-endian.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t cpt_rsa_ssa_pss_sign(cpt_hash_alg_e msg_hash_alg, cpt_hash_alg_e mgf_hash_alg, const uint8_t *salt,
    uint32_t salt_bytes, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *d, const uint8_t *n, uint32_t n_bits,
    uint8_t *signature)
{
    uint32_t rsa_ret
        = rsa_ssa_pss_sign(msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg, msg_bytes, d, n, n_bits, signature);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief RSA PKCS#1_v2.2 RSASSA-PSS-SIGN with message digest(private key is CRT style)
 *
 * @param[in] msg_hash_alg specific hash algorithm for message or Hash(message)
 * @param[in] mgf_hash_alg specific hash algorithm for MGF1
 * @param[in] salt salt
 * @param[in] salt_bytes byte length of salt
 * @param[in] msg_digest Hash(message), message is to be signed, here Hash is msg_hash_alg.
 * @param[in] d RSA-CRT private key (p,q,dp,dq,u), every field is (n_bits/2+7)/8 bytes, big-endian.
 * @param[in] n_bits bit length of n
 * @param[out] signature RSA signature, (n_bits+7)/8 bytes, big-endian.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. if no salt prepared to input, please set salt to NULL, it will be generated inside.
 *     3. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 */
uint32_t cpt_rsa_ssa_pss_crt_sign_by_msg_digest(cpt_hash_alg_e msg_hash_alg, cpt_hash_alg_e mgf_hash_alg,
    const uint8_t *salt, uint32_t salt_bytes, const uint8_t *msg_digest, const cpt_rsa_crt_private_key_st *d,
    uint32_t n_bits, uint8_t *signature)
{
    uint32_t rsa_ret = rsa_ssa_pss_crt_sign_by_msg_digest(
        msg_hash_alg, mgf_hash_alg, salt, salt_bytes, msg_digest, d, n_bits, signature);
    return cpt_err_code_to_ehsm_code(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief RSA PKCS#1_v2.2 RSASSA-PSS-VERIFY with message digest
 *
 * @param[in] msg_hash_alg specific hash algorithm for message or Hash(message)
 * @param[in] mgf_hash_alg specific hash algorithm for MGF1
 * @param[in] salt_bytes byte length of salt
 * @param[in] msg_digest Hash(message), message is to be verified, here Hash is msg_hash_alg.
 * @param[in] e RSA public key e, (e_bits+7)/8 bytes, big-endian.
 * @param[in] e_bits bit length of e
 * @param[in] n RSA modulus n, (n_bits+7)/8 bytes, big-endian.
 * @param[in] n_bits bit length of n
 * @param[in] signature RSA signature, (n_bits+7)/8 bytes, big-endian.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. it is recommended that msg_hash_alg and mgf_hash_alg are the same.
 *     2. salt_bytes should be in [0, em_bytes-digest_bytes-2], em_bytes is (em_bits+7)/8, em_bits is (n_bits-1).
 *        it is recommended to use default value, digest length of hash algorithm msg_hash_alg or mgf_hash_alg.
 *        if salt_bytes is not known, please set it to -1
 */
uint32_t cpt_rsa_ssa_pss_verify_by_msg_digest(cpt_hash_alg_e msg_hash_alg, cpt_hash_alg_e mgf_hash_alg,
    int32_t salt_bytes, const uint8_t *msg_digest, const uint8_t *e, uint32_t e_bits, const uint8_t *n, uint32_t n_bits,
    const uint8_t *signature)
{
    uint32_t rsa_ret = rsa_ssa_pss_verify_by_msg_digest(
        msg_hash_alg, mgf_hash_alg, salt_bytes, msg_digest, e, e_bits, n, n_bits, signature);
    return cpt_err_code_to_ehsm_code_sec(rsa_ret, RSA_SUCCESS);
}

/**
 * @brief Verify SM2 Signature
 *
 * @param[in] E E value, 32 bytes, big-endian
 * @param[in] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @param[in] signature Signature r and s, 64 bytes, big-endian
 * @return
 *     SM2_SUCCESS_S(success, the signature is valid); other(error or the signature is invalid)
 * @note
 */
uint32_t cpt_sm2_verify(const uint8_t *E, const uint8_t *pubKey, const uint8_t *signature)
{
    uint32_t ret = sm2_verify(E, pubKey, signature);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief Generate SM2 Signature
 *
 * @param[in] E E value, 32 bytes, big-endian
 * @param[in] rand_k random big integer k in signing, 32 bytes, big-endian,
              if you do not have this integer, please set this parameter to be NULL,
              it will be generated inside.
 * @param[in] priKey private key, 32 bytes, big-endian
 * @param[out] signature Signature r and s, 64 bytes, big-endian
 * @return
 *     SM2_SUCCESS_S(success); other(error)
 * @note
 *     1. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 */
uint32_t cpt_sm2_sign(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], uint8_t signature[64])
{
    uint32_t ret = sm2_sign(E, rand_k, priKey, signature);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief Generate SM2 public key from private key, secure version
 *
 * @param[in] priKey private key, 32 bytes, big-endian
 * @param[out] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @return
 *     SM2_SUCCESS_S(success); SM2_ERROR_S(error)
 * @note
 */
uint32_t cpt_sm2_get_pubkey_from_prikey(const uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret = sm2_get_pubkey_from_prikey(priKey, pubKey);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief Generate SM2 random Key pair, secure version
 *
 * @param[out] priKey private key, 32 bytes, big-endian
 * @param[out] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @return
 *     SM2_SUCCESS_S(success); SM2_ERROR_S(error)
 * @note
 */
uint32_t cpt_sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret = sm2_getkey(priKey, pubKey);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief SM2 Encryption
 *
 * @param[in] M plaintext, MByteLen bytes, big-endian
 * @param[in] MByteLen byte length of M
 * @param[in] rand_k random big integer k in encrypting, 32 bytes, big-endian,
              if you do not have this integer, please set this parameter to be NULL,
              it will be generated inside.
 * @param[in] pubKey public key, 65 bytes, big-endian
 * @param[in] order either SM2_C1C3C2 or SM2_C1C2C3
 * @param[out] C ciphertext, CByteLen bytes, big-endian
 * @param[out] CByteLen byte length of C, should be MByteLen+97 if success
 * @return
 *     SM2_SUCCESS_S(success); other(error)
 * @note
 *     1. M and C can be the same buffer
 *     2. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 *     3. please make sure pubKey is valid
 */
uint32_t cpt_sm2_encrypt(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], const uint8_t pubKey[65],
    cpt_sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t ret = sm2_encrypt(M, MByteLen, rand_k, pubKey, order, C, CByteLen);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief SM2 Decryption
 *
 * @param[in] C ciphertext, CByteLen bytes, big-endian
 * @param[in] CByteLen byte length of C, please make sure CByteLen>97
 * @param[in] priKey private key, 32 bytes, big-endian
 * @param[in] order either SM2_C1C3C2 or SM2_C1C2C3
 * @param[out] M plaintext, MByteLen bytes, big-endian
 * @param[out] MByteLen byte length of M, should be CByteLen-97 if success
 * @return
 *     SM2_SUCCESS_S(success); other(error)
 * @note
 *     1. M and C can be the same buffer
 */
uint32_t cpt_sm2_decrypt(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32], cpt_sm2_cipher_order_e order,
    uint8_t *M, uint32_t *MByteLen)
{
    uint32_t ret = sm2_decrypt(C, CByteLen, priKey, order, M, MByteLen);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief SM2 Key Exchange
 *
 * @param[in] role SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 * @param[in] dA local's permanent private key, 32 bytes, U8 big-endian
 * @param[in] PB peer's permanent public key, 65 bytes, U8 big-endian
 * @param[in] rA local's temporary private key, 32 bytes, U8 big-endian
 * @param[in] RA local's temporary public key, 65 bytes, U8 big-endian
 * @param[in] RB peer's temporary public key, 65 bytes, U8 big-endian
 * @param[in] ZA local's Z value, 32 bytes, U8 big-endian
 * @param[in] ZB peer's Z value, 32 bytes, U8 big-endian
 * @param[in] kByteLen byte length of output key, can not be zero
 * @param[out] KA output key
 * @param[in] callback output, sponsor's S1, or responsor's S2, 32 bytes, U8 big-endian, this is optional
 * @param[out] SA sponsor's SA, or responsor's SB, 32 bytes, U8 big-endian, this is optional
 * @return
 *     SM2_SUCCESS_S(success); other(error)
 * @note
 *     1. please make sure the inputs are valid
 *     2. S1 and SA are optional, if you don't need, please set S1 and SA as NULL
 *     3. in case that S1(S2) and SA(SB) exist, if S1=SB,S2=SA, then exchange success.
 */
uint32_t cpt_sm2_exchangekey(cpt_sm2_exchange_role_e role, const uint8_t *dA, const uint8_t *PB, const uint8_t *rA,
    const uint8_t *RA, const uint8_t *RB, const uint8_t *ZA, const uint8_t *ZB, uint32_t kByteLen, uint8_t *KA,
    uint8_t *S1, uint8_t *SA)
{
    uint32_t ret = sm2_exchangekey(role, dA, PB, rA, RA, RB, ZA, ZB, kByteLen, KA, S1, SA);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}


/**
 * @brief ECDH compute key
 *
 * @param[in] local_prikey local private key, big-endian
 * @param[in] peer_pubkey peer public key, big-endian
 * @param[out] key output key
 * @param[in] keyByteLen byte length of output key
 * @param[in] KDF KDF function to get key
 * @return
 *     ECDH_SUCCESS_S(success); other(error)
 * @note
 */
uint32_t cpt_ecdh_compute_key(const cpt_eccp_curve_st *curve, const uint8_t *local_prikey, const uint8_t *peer_pubkey,
    uint8_t *key, uint32_t keyByteLen, cpt_kdf_func kdf)
{
    uint32_t ecdh_ret = ecdh_compute_key(curve, local_prikey, peer_pubkey, key, keyByteLen, kdf);
    return cpt_err_code_to_ehsm_code(ecdh_ret, ECDH_SUCCESS);
}

/**
 * @brief get rand(without entropy reducing)
 *
 * @param[in] random byte buffer rand
 * @param[in] bytes byte length of rand
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_get_rand(uint8_t *random, uint32_t bytes)
{
    uint32_t ret = get_rand(random, bytes);
    return cpt_err_code_to_ehsm_code(ret, TRNG_SUCCESS);
}

/**
 * @brief get rand with fast speed(with entropy reducing, for such as clearing tmp buffer)
 *
 * @param[in] random byte buffer rand
 * @param[in] bytes byte length of rand
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_get_rand_fast(uint8_t *random, uint32_t bytes)
{
    uint32_t ret = get_rand_fast(random, bytes);
    return cpt_err_code_to_ehsm_code(ret, TRNG_SUCCESS);
}

/**
 * @brief reseed TRNG(works when DRBG is enabled)
 * @param none
 * @return none
 * @note
 *     1. used for DRBG
 * @warning
 *      1. Only valid when DRBG mode is enabled;
 *      2. Only triggers a write to the reseed request register, does not wait for reseed completion (no synchronization
 *         semantics). The caller must handle the completion delay independently.
 */
void cpt_trng_reseed(void)
{
    trng_reseed();
}

/**
 * @brief get TRNG alg
 * @param none
 * @return
 *     TRNG_ALG_SM4 or TRNG_ALG_AES
 * @note
 */
uint32_t cpt_trng_get_drbg_alg(void)
{
    return trng_get_drbg_alg();
}

/**
 * @brief set global init config
 * @param[in] config_choice config choice, should be NIST_GLOBAL_CONFIG, or GM_GLOBAL_CONFIG or LATEST_GM_GLOBAL_CONFIG
 * @return
 *     EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. before generating random numbers, please call this function.
 */
uint32_t cpt_trng_set_global_init_config(uint32_t config)
{
    uint32_t ret = trng_set_global_init_config(config);
    return cpt_err_code_to_ehsm_code(ret, TRNG_SUCCESS);
}

/**
 * @brief get SM2 Z value = SM3(bitLenofID||ID||a||b||Gx||Gy||Px||Py)
 *
 * @param[in] ID User ID
 * @param[in] byteLenofID byte length of ID, must be less than 2^13
 * @param[in] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @param[out] Z Z value, SM3 digest, 32 bytes
 * @return
 *     SM2_SUCCESS(success); other(error)
 * @note
 *     1. bit length of ID must be less than 2^16, thus byte length must be less than 2^13
 *     2. if ID is NULL, then replace it with sm2 default ID
 *     3. please make sure the pubKey is valid
 */
uint32_t cpt_sm2_getZ(const uint8_t *ID, uint32_t byteLenofID, const uint8_t pubKey[65], uint8_t Z[32])
{
    uint32_t ret = sm2_getZ(ID, byteLenofID, pubKey, Z);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief get SM2 E value = SM3(Z||M) (one-off style)
 *
 * @param[in] M Message
 * @param[in] byteLen byte length of M
 * @param[in] Z Z value, 32 bytes
 * @param[out] E E value, 32 bytes
 * @return
 *     SM2_SUCCESS(success); other(error)
 * @note
 */
uint32_t cpt_sm2_getE(const uint8_t *M, uint32_t byteLen, const uint8_t Z[32], uint8_t E[32])
{
    uint32_t ret = sm2_getE(M, byteLen, Z, E);
    return cpt_err_code_to_ehsm_code(ret, SM2_SUCCESS);
}

/**
 * @brief get Ed25519 public key from private key
 *
 * @param[in] prikey private key, 32 bytes, little-endian
 * @param[out] pubkey public key, 32 bytes, little-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1.
 */
uint32_t cpt_ed25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ed25519_ret = ed25519_get_pubkey_from_prikey(prikey, pubkey);
    return cpt_err_code_to_ehsm_code(ed25519_ret, EdDSA_SUCCESS);
}

/**
 * @brief generate Ed25519 random key pair
 *
 * @param[out] prikey private key, 32 bytes, little-endian
 * @param[out] pubkey public key, 32 bytes, little-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1.
 */
uint32_t cpt_ed25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t ed25519_ret = ed25519_getkey(prikey, pubkey);
    return cpt_err_code_to_ehsm_code(ed25519_ret, EdDSA_SUCCESS);
}

/**
 * @brief Ed25519 sign
 *
 * @param[in] mode Ed25519 signature mode
 * @param[in] prikey private key, 32 bytes, little-endian
 * @param[in] pubkey public key, 32 bytes, little-endian, if no pubkey, please set it to be NULL
 * @param[in] ctx 0-255 bytes
 * @param[in] ctxByteLen byte length of ctx
 * @param[in] M message, requirements are determined by mode
 * @param[in] MByteLen byte length of M, requirements are determined by mode
 * @param[out] RS signature
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. if no public key, please set pubkey to be NULL, it will be generated inside
 *     2. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL),
 *        so no need to check M and MByteLen, otherwise, M is sha512 digest, occupies 64 bytes,
 *        and MByteLen is not involved
 *     3. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     4. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     5. if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, ctx length is from 0 to 255, default
 *        length is 0, thus ctx could be empty
 */
uint32_t cpt_ed25519_sign(cpt_ed25519_mode_e mode, const uint8_t prikey[32], const uint8_t pubkey[32],
    const uint8_t *ctx, uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, uint8_t RS[64])
{
    uint32_t ed25519_ret = ed25519_sign(mode, prikey, pubkey, ctx, ctxByteLen, M, MByteLen, RS);
    return cpt_err_code_to_ehsm_code(ed25519_ret, EdDSA_SUCCESS);
}

/**
 * @brief Ed25519 verify
 *
 * @param[in] mode Ed25519 signature mode
 * @param[in] pubkey public key, 32 bytes, little-endian
 * @param[in] ctx 0-255 bytes
 * @param[in] ctxByteLen byte length of ctx
 * @param[in] M message, requirements are determined by mode
 * @param[in] MByteLen byte length of M, requirements are determined by mode
 * @param[in] RS signature, 64 bytes, little-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. if mode is not Ed25519_PH_WITH_PH_M, M could be empty(please set M to be NULL),
 *        so no need to check M and MByteLen, otherwise, M is sha512 value, occupies 64 bytes,
 *        and MByteLen is not involved
 *     2. if mode is Ed25519_DEFAULT, ctx is not involved, no need to check ctx and ctxByteLen
 *     3. if mode is Ed25519_CTX, ctx can not be empty(ctx length is from 1 to 255)
 *     4. if mode is Ed25519_PH or Ed25519_PH_WITH_PH_M, ctx length is from 0 to 255, default
 *        length is 0, thus ctx could be empty
 */
uint32_t cpt_ed25519_verify(cpt_ed25519_mode_e mode, const uint8_t pubkey[32], const uint8_t *ctx, uint8_t ctxByteLen,
    const uint8_t *M, uint32_t MByteLen, const uint8_t RS[64])
{
    uint32_t ed25519_ret = ed25519_verify(mode, pubkey, ctx, ctxByteLen, M, MByteLen, RS);
    return cpt_err_code_to_ehsm_code(ed25519_ret, EdDSA_SUCCESS);
}

/**
 * @brief get X25519 public key from private key
 *
 * @param[in] prikey private key, 32 bytes, little-endian
 * @param[out] pubkey public key, 32 bytes, little-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 */
uint32_t cpt_x25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t x25519_ret = x25519_get_pubkey_from_prikey(prikey, pubkey);
    return cpt_err_code_to_ehsm_code(x25519_ret, X25519_SUCCESS);
}

/**
 * @brief get x25519 random key pair
 *
 * @param[out] prikey private key, 32 bytes, little-endian
 * @param[out] pubkey public key, 32 bytes, little-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 */
uint32_t cpt_x25519_getkey(uint8_t prikey[32], uint8_t pubkey[32])
{
    uint32_t x25519_ret = x25519_getkey(prikey, pubkey);
    return cpt_err_code_to_ehsm_code(x25519_ret, X25519_SUCCESS);
}

/**
 * @brief  X25519 key agreement
 *
 * @param[in] local_prikey local private key, 32 bytes, little-endian
 * @param[in] peer_pubkey peer Public key, 32 bytes, little-endian
 * @param[out] key derived key
 * @param[in] keyByteLen byte length of output key
 * @param[in] kdf KDF function
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. if no KDF function, please set kdf to be NULL
 */
uint32_t cpt_x25519_compute_key(
    const uint8_t local_prikey[32], const uint8_t peer_pubkey[32], uint8_t *key, uint32_t keyByteLen, cpt_kdf_func kdf)
{
    uint32_t x25519_ret = x25519_compute_key(local_prikey, peer_pubkey, key, keyByteLen, kdf);
    return cpt_err_code_to_ehsm_code(x25519_ret, X25519_SUCCESS);
}

/**
 * @brief DH generate public key from private key.
 *
 * @param[in] dh_para dh_para_st struct pointer
 * @param[in] prikey private key
 * @param[out] pubkey public key
 * @return
 *     DH_SUCCESS(success); other(error)
 * @note
 *     1. please call dh_param_value_init() before calling this function.
 *     2. the input prikey occupies (dh_para->q_bits+7)/8 bytes
 *        the output pubkey occupies (dh_para->p_bits+7)/8 bytes
 */
uint32_t cpt_dh_generate_pubkey_from_prikey(const cpt_dh_para_st *dh_para, const uint8_t *prikey, uint8_t *pubkey)
{
    uint32_t dh_ret = dh_generate_pubkey_from_prikey(dh_para, prikey, pubkey);
    return cpt_err_code_to_ehsm_code(dh_ret, DH_SUCCESS);
}

/**
 * @brief DH compute key
 *
 * @param[in] dh_para dh_para_st struct pointer
 * @param[in] local_prikey local private key, big-endian
 * @param[in] peer_pubkey peer public key, big-endian
 * @param[out] key output key
 * @return
 *     DH_SUCCESS(success); other(error)
 * @note
 *     1. local_prikey occupies (dh_para->q_bits+7)/8 bytes
 *     2. peer_pubkey and key occupy (dh_para->p_bits+7)/8 bytes
 */
uint32_t cpt_dh_compute_key(
    const cpt_dh_para_st *dh_para, const uint8_t *local_prikey, const uint8_t *peer_pubkey, uint8_t *key)
{
    uint32_t dh_ret = dh_compute_key(dh_para, local_prikey, peer_pubkey, key);
    return cpt_err_code_to_ehsm_code(dh_ret, DH_SUCCESS);
}

/**
 * @brief pbkdf2 generate to kmu of 32Byte(using hmac as PRF)
 *
 * @param[in] hash_alg_e specific hash algorithm
 * @param[in] sp_key_idx index of secure port key(password)
 * @param[in] pwd_bytes byte length of password, it could be 0
 * @param[in] salt salt
 * @param[in] salt_bytes byte length of salt, it could be 0
 * @param[in] iter iteration times
 * @param[in] dst_key_idx index of secure dst port key(kdf)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1.only support derive 32Byte key, not support other length of derive_key
 *     2.not support SHA3(hardeare not directly support hmac-sha3), MD5, SHA1
 */
uint32_t cpt_pbkdf2_hmac_kmu(cpt_hash_alg_e hash_alg, uint16_t sp_key_idx, uint32_t pwd_bytes, const uint8_t *salt,
    uint32_t salt_bytes, uint32_t iter, uint16_t dst_key_idx)
{
    uint32_t pbkdf2_ret = pbkdf2_hmac_kmu(hash_alg, sp_key_idx, pwd_bytes, salt, salt_bytes, iter, dst_key_idx);
    return cpt_err_code_to_ehsm_code(pbkdf2_ret, HASH_SUCCESS);
}

/**
 * @brief pbkdf2 function(using hmac as PRF)
 *
 * @param[in] alg specific hash algorithm
 * @param[in] pwd password, as the key of hmac
 * @param[in] sp_key_idx index of secure port key(password)
 * @param[in] pwd_bytes byte length of password, it could be 0
 * @param[in] salt salt
 * @param[in] salt_bytes byte length of salt, it could be 0
 * @param[in] iter iteration times
 * @param[out] out derived key
 * @param[in] out_bytes byte length of derived key
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_pbkdf2_hmac(cpt_hash_alg_e alg, const uint8_t *pwd, uint16_t sp_key_idx, uint32_t pwd_bytes,
    const uint8_t *salt, uint32_t salt_bytes, uint32_t iter, uint8_t *out, uint32_t out_bytes)
{
    uint32_t pbkdf2_ret = pbkdf2_hmac(alg, pwd, sp_key_idx, pwd_bytes, salt, salt_bytes, iter, out, out_bytes);
    return cpt_err_code_to_ehsm_code(pbkdf2_ret, HASH_SUCCESS);
}

/**
 * @brief k1||k2 = ansi_x9_63_kdf(Z||counter||shared_info , k1_bytes + k2_bytes).
 *
 * @param[in] alg specific hash algorithm
 * @param[in] Z byte string
 * @param[in] Z_bytes byte length of Z
 * @param[in] shared_info shared info
 * @param[in] shared_info_bytes byte length of shared info
 * @param[in] callback output, k1 part
 * @param[in] callback input, byte length of k1
 * @param[in] callback output, k2 part
 * @param[in] callback input, byte length of k2
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure alg is valid
 *     2. k1 can not be NULL, but k2 can.
 */
uint32_t cpt_ansi_x9_63_kdf(hash_alg_e alg, const uint8_t *Z, uint32_t Z_bytes, const uint8_t *shared_info,
    uint32_t shared_info_bytes, uint8_t *k1, uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes)
{
    uint32_t kdf_ret = ansi_x9_63_kdf(alg, Z, Z_bytes, shared_info, shared_info_bytes, k1, k1_bytes, k2, k2_bytes);
    return cpt_err_code_to_ehsm_code(kdf_ret, HASH_SUCCESS);
}

/**
 * @brief get pke IP version
 *  none
 * @return pke IP version
 * @note
 */
uint32_t cpt_pke_get_version(void)
{
    return pke_get_version();
}

/**
 * @brief get pke driver version
 *  none
 * @return pke driver version(software version)
 * @note
 */
uint32_t cpt_pke_get_driver_version(void)
{
    return pke_get_driver_version();
}

/**
 * @brief get ske IP version
 *  none
 * @return ske IP version
 * @note
 */
uint32_t cpt_ske_get_version(void)
{
    return ske_get_version();
}

/**
 * @brief get ske driver version
 *  none
 * @return ske driver version(software version)
 * @note
 */
uint32_t cpt_ske_get_driver_version(void)
{
    return ske_get_driver_version();
}

/**
 * @brief get HFE IP version
 *  none
 * @return HFE IP version
 * @note
 */
uint32_t cpt_hash_get_version(void)
{
    return hash_get_version();
}

/**
 * @brief get hash driver version
 *  none
 * @return hash driver version(software version)
 * @note
 */
uint32_t cpt_hash_get_driver_version(void)
{
    return hash_get_driver_version();
}

/**
 * @brief get trng IP version
 *  none
 * @return trng IP version(hardware version)
 * @note
 */
uint32_t cpt_trng_get_version(void)
{
    return trng_get_version();
}

/**
 * @brief get trng driver version
 *  none
 * @return trng driver version(software version)
 * @note
 */
uint32_t cpt_trng_get_driver_version(void)
{
    return trng_get_driver_version();
}
