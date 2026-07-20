/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdint.h>
#include "types.h"
#include "util.h"
#include "kmu_driver.h"
#include "crypto_lib_api.h"
#if 0x1 == CRYPTO_HW_LP
#include "mmap.h"
#endif
#include "driver/sysreg.h"
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
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    uint32_t ske_ret = ske_dma_crypto(
        alg, mode, crypto, key, sp_key_idx, iv, padding, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
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
 * @brief ske cmac dma style update message including the last block(or message tail), and get the mac
 *
 * @param[in] ctx ske_cmac_dma_st context pointer
 * @param[in] msg message including the last block(or message tail)
 * @param[in] msg_bytes byte length of msg, could be 0
 * @param[out] msg_bytes cmac, occupies a block
 * @param[in] callback callback function pointer, this could be NULL, means doing nothing
 * @param[in] dma_cfg the ske dma data transfer configuration
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if the whole message length is 0, this case is supported. in this case, msg occupies a block, and
 *        please set msg_bytes to 0.
 */
uint32_t cpt_ske_dma_cmac_update_including_last_block(cpt_ske_cmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l,
    uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, cpt_ske_callback callback, uint32_t dma_cfg)
{
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_SKE);
    uint32_t ske_ret = ske_dma_cmac_update_including_last_block(ctx, msg_h, msg_l, msg_bytes, mac_h, mac_l, callback);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}

/**
 * @brief convert ske cmac cpu context to dma context
 * @param cpu_ctx the ske cmac cpu context pointer
 * @param dma_ctx the ske cmac dma context pointer
 */
void cpt_cmac_ctx_cpu_2_dma(const cpt_ske_cmac_ctx_st *cpu_ctx, cpt_ske_cmac_dma_ctx_st *dma_ctx)
{
    util_memcpy(dma_ctx->ske_cmac_ctx, cpu_ctx->ske_cmac_ctx, sizeof(cpu_ctx->ske_cmac_ctx));
    dma_ctx->mac_bytes = cpu_ctx->mac_bytes;
}


#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
/**
 * @brief ske gcm mode encrypt/decrypt(one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, key of AES(128/192/256) or SM4
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in
 *                       [1,SKE_MAX_KEY_IDX], if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] iv iv in bytes
 * @param[in] iv_bytes byte length of iv, now only 12 bytes supported
 * @param[in] aad aad, please make sure aad here is integral
 * @param[in] aad_bytes byte length of aad, it could be any value, including 0
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] c_bytes byte length of plaintext/ciphertext, it could be any value including 0
 * @param[in] mac (for decryption), output(for encryption)
 * @param[in] mac_bytes byte length of mac.
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     7. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     8. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_gcm_crypto(cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
    const uint8_t *iv, uint32_t iv_bytes, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out,
    uint32_t c_bytes, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ske_ret
        = ske_gcm_crypto(alg, crypto, key, sp_key_idx, iv, iv_bytes, aad, aad_bytes, in, out, c_bytes, mac, mac_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
/**
 * @brief ske ccm mode encrypt/decrypt(one-off style)
 *
 * @param[in] alg ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] key key in bytes, key of AES(128/192/256) or SM4
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 * @param[in] nonce  nonce in bytes, its byte lenth is 15-L
 * @param[in] M bytes of authentication field(bytes of mac)
 * @param[in] L bytes of length field(message byte length is less than 256^L)
 * @param[in] aad aad, please make sure aad here is integral
 * @param[in] aad_bytes byte length of aad, it could be any value, including 0
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @param[in] c_bytes byte length of plaintext/ciphertext, it could be any value, including 0
 * @param[in] mac (for decryption), output(for encryption)
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     7. byte length of mac is M
 *     8. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t cpt_ske_ccm_crypto(cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
    const uint8_t *nonce, uint8_t M, uint8_t L, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out,
    uint32_t c_bytes, uint8_t *mac)
{
    uint32_t ske_ret = ske_ccm_crypto(alg, crypto, key, sp_key_idx, nonce, M, L, aad, aad_bytes, in, out, c_bytes, mac);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
/**
 * @brief ske xts mode encrypting/decrypting
 * @param[in] alg  ske algorithm
 * @param[in] crypto encrypting or decrypting
 * @param[in] xts_style style to update t for IEEE or GM standards
 * @param[in] key key in bytes, key = key1||key2
 * @param[in] sp_key_idx index of secure port key, (sp_key_idx & 0x7FFF) must be in[1,SKE_MAX_KEY_IDX], if the
 *                       MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * @param[in] i i value it has the same length as blcok length
 * @param[in] in plaintext or ciphertext
 * @param[out] out ciphertext or plaintext
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. xts_style only works on SM4, and AES only supports IEEE standard regardless of the value of xts_style
 *     2. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     3. key consists of key1 and key2
 *     4. c_bytes can not be less than block byte length
 */
uint32_t cpt_ske_xts_crypto(cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, cpt_xts_style_e xts_style, const uint8_t *key,
    uint16_t sp_key_idx, const uint8_t *i, const uint8_t *in, uint8_t *out, uint32_t c_bytes)
{
    uint32_t ske_ret = ske_xts_crypto(alg, crypto, xts_style, key, sp_key_idx, i, in, out, c_bytes);
    return cpt_err_code_to_ehsm_code(ske_ret, SKE_SUCCESS);
}
#endif

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
    sysreg_ahb_dma_cfg(dma_cfg | AHB_DMA_HASH);
    uint32_t hash_ret = hash_dma_final(ctx, msg_h, msg_l, msg_bytes, digest_h, digest_l);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
}

/**
 * @brief input key and whole message, get the hmac
 *
 * @param[in] alg specific hash algorithm
 * @param[in] key key
 * @param[in] sp_key_idx index of secure port key
 * @param[in] key_bytes byte length of the key
 * @param[in] msg message
 * @param[in] msg_bytes byte length of the input message
 * @param[out] mac hmac
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure the mac buffer is sufficient
 *     2. here hmac is not for SHA3.
 *     3. if cfg not support secure port, please set key is NULL when hmac without key
 *     4. if cfg support secure port, hmac will use secure port key when parameter key is NULL,
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t cpt_hmac(cpt_hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, const uint8_t *msg,
    uint32_t msg_bytes, uint8_t *mac)
{
    uint32_t hash_ret = hmac(alg, key, sp_key_idx, key_bytes, msg, msg_bytes, mac);
    return cpt_err_code_to_ehsm_code(hash_ret, HASH_SUCCESS);
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
 * semantics). The caller must handle the completion delay independently.
 */
void cpt_trng_reseed(void)
{
    trng_reseed();
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

/* function: lib ske config secure port
 * parameters:
 *     sp_key_idx ------------------- input, secure port key id
 * return:
 * caution:
 */
uint32_t lib_ske_secure_port_config(uint32_t alg, uint16_t sp_key_idx)
{
    UNUSED(alg);
    uint32_t ret;

    ske_set_secure_port();

    ret = kmu_config(sp_key_idx, KMU_TRANSP_KEY_TO_SKE);
    if (0U != ret) {
        ret = SKE_INPUT_INVALID;
    } else {
    }

    return ret;
}
/* function: lib hash config secure port
 * parameters:
 *     sp_key_idx ------------------- input, secure port key id
 * return:
 * caution:
 */
uint32_t lib_hash_secure_port_config(uint16_t sp_key_idx)
{
    uint32_t ret;

    ret = kmu_config(sp_key_idx, KMU_TRANSP_KEY_TO_HASH);
    if (0U != ret) {
        ret = HASH_INPUT_INVALID;
    } else {
    }

    return ret;
}
