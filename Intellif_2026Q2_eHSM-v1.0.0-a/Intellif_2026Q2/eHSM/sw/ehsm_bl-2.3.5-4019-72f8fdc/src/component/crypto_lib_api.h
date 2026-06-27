#ifndef APPLICATION_SRC_SEIP_CORE_CRYPTO_LIB_API_H_
#define APPLICATION_SRC_SEIP_CORE_CRYPTO_LIB_API_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "trng/trng.h"
#include "ske/ske.h"
#include "ske/ske_cmac.h"
#include "ske/ske_cbc.h"
#include "ske/ske_cmac.h"

#include "pke/rsa.h"

#include "pke/sm2.h"

#include "pke/ecdsa.h"

#include "hash_hmac/hash.h"

#include "ske/ske_gcm_gmac.h"
#include "hash_hmac/hmac.h"
#include "ske/ske_xts.h"
#include "ske/ske_ccm.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// define the crypto hardware type
#define CRYPTO_HW_LP  0
#define CRYPTO_HW_HP  1
#define CRYPTO_HW_UHP 2

#define EHSM_WRITE_SOC_READ  (AHB_WRITE_AXI_READ)
#define EHSM_READ_SOC_WRITE  (AHB_READ_AXI_WRITE)
#define EHSM_READ_EHSM_WRITE (AHB_READ_AND_WRITE)
#define SOC_READ_SOC_WRITE   (AXI_READ_AND_WRITE)

// define internal error code
#define EHSM_ERR_DH_ZERO_ALL        (DH_ZERO_ALL)
#define EHSM_ERR_DH_VALUE_ONE       (DH_VALUE_ONE)
#define EHSM_ERR_DH_INTEGER_TOO_BIG (DH_INTEGER_TOO_BIG)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef ske_ctx_st cpt_ske_ctx_st;
typedef ske_alg_e cpt_ske_alg_e;
typedef ske_mode_e cpt_ske_mode_e;
typedef ske_crypto_e cpt_ske_crypto_e;
typedef ske_padding_e cpt_ske_padding_e;
typedef ske_mac_e cpt_ske_mac_e;
typedef ske_cmac_ctx_st cpt_ske_cmac_ctx_st;
typedef ske_cmac_dma_st cpt_ske_cmac_dma_ctx_st;
typedef xts_style_e cpt_xts_style_e;
typedef hash_ctx_st cpt_hash_ctx_st;
typedef hash_alg_e cpt_hash_alg_e;
typedef hash_dma_ctx_st cpt_hash_dma_ctx_st;
typedef hmac_ctx_st cpt_hmac_ctx_st;
typedef hmac_dma_st cpt_hmac_dma_ctx_st;
typedef eccp_curve_st cpt_eccp_curve_st;
typedef sm2_cipher_order_e cpt_sm2_cipher_order_e;
typedef rsa_crt_private_key_st cpt_rsa_crt_private_key_st;
typedef sm2_exchange_role_e cpt_sm2_exchange_role_e;
typedef KDF_FUNC cpt_kdf_func;
typedef SKE_CALLBACK cpt_ske_callback;
typedef HASH_CALLBACK cpt_hash_callback;

typedef enum {
    MAC_RCTX_2_LCTX = 0U,
    MAC_LCTX_2_RCTX = 1U,
} mac_switch_e;

typedef enum {
    HASH_RCTX_2_LCTX = 0U,
    HASH_LCTX_2_RCTX = 1U,
} cpt_hash_switch_e;

typedef enum {
    HASH_CPU_2_DMA = 0U,
    HASH_DMA_2_CPU = 1U,
} cpt_hash_ctx_switch_e;

typedef struct {
    uint32_t total[HASH_TOTAL_LEN_MAX_WORD_LEN];
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];
    uint8_t hash_buffer[HASH_BLOCK_MAX_BYTE_LEN];
    cpt_hash_alg_e hash_alg;
    hfe_mode_e hfe_mode;
    uint8_t block_byte_len;
    uint8_t digest_byte_len;
    uint8_t iterator_word_len;
    uint8_t first_update_flag;
} cpt_hash_rctx_st;
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

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
    uint32_t in_bytes, uint32_t *out_bytes);

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
    uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, cpt_ske_callback callback, uint32_t dma_cfg);

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
    uint16_t sp_key_idx, const uint8_t *iv, cpt_ske_padding_e padding);

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
uint32_t cpt_ske_cbc_update_blocks(cpt_ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

/**
 * @brief ske cbc finish
 *
 * @param[in] ctx ske_ctx_st context pointer
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t cpt_ske_cbc_final(cpt_ske_ctx_st *ctx);

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
    uint16_t sp_key_idx, uint8_t mac_bytes);

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
uint32_t cpt_ske_cmac_update(cpt_ske_cmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

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
uint32_t cpt_ske_cmac_final(cpt_ske_cmac_ctx_st *ctx, uint8_t *mac);

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
    const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes);

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
    uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, cpt_ske_callback callback, uint32_t dma_cfg);

/**
 * @brief convert ske cmac cpu context to dma context
 * @param cpu_ctx the ske cmac cpu context pointer
 * @param dma_ctx the ske cmac dma context pointer
 */
void cpt_cmac_ctx_cpu_2_dma(const cpt_ske_cmac_ctx_st *cpu_ctx, cpt_ske_cmac_dma_ctx_st *dma_ctx);


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
    uint32_t c_bytes, uint8_t *mac, uint8_t mac_bytes);
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
    uint32_t c_bytes, uint8_t *mac);
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
/**
 * @brief ske xts mode encrypting/decrypting
 *
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
    uint16_t sp_key_idx, const uint8_t *i, const uint8_t *in, uint8_t *out, uint32_t c_bytes);
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
void cpt_hash_ctx_switch(cpt_hash_ctx_st *cpu_ctx, cpt_hash_dma_ctx_st *dma_ctx, cpt_hash_ctx_switch_e type);

/**
 * @brief init HASH
 *
 * @param[in] ctx hash_ctx_st context pointer
 * @param[in] alg specific hash algorithm
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 *     1. please make sure alg is valid
 */
uint32_t cpt_hash_init(cpt_hash_ctx_st *ctx, cpt_hash_alg_e alg);

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
uint32_t cpt_hash_update(cpt_hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

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
uint32_t cpt_hash_final(cpt_hash_ctx_st *ctx, uint8_t *digest);

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
uint32_t cpt_hash(cpt_hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

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
    uint32_t digest_h, uint32_t digest_l, uint32_t dma_cfg);

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
    uint32_t msg_bytes, uint8_t *mac);

/**
 * @brief Generate ECDSA Signature in byte string style
 *
 * @param[in] curve ecc curve struct pointer, please make sure it is valid
 * @param[in] E hash value, U8 big-endian
 * @param[in] EByteLen byte length of E
 * @param[in] rand_k random big integer k in signing, U8 big-endian
 * @param[in] priKey private key, U8 big-endian
 * @param[out] signature signature r and s, U8 big-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t cpt_ecdsa_sign(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, const uint8_t *rand_k,
    const uint8_t *priKey, uint8_t *signature);

/**
 * @brief Verify ECDSA Signature in byte string style
 *
 * @param[in] curve ecc curve struct pointer, please make sure it is valid
 * @param[in] E hash value, U8 big-endian
 * @param[in] EByteLen byte length of E
 * @param[in] pubKey public key, U8 big-endian
 * @param[in] signature signature r and s, U8 big-endian
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t cpt_ecdsa_verify(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, const uint8_t *pubKey,
    const uint8_t *signature);

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
    const uint32_t *a, const uint32_t *e, const uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

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
    uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

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
    const uint32_t *dq, const uint32_t *u, const uint32_t *e, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

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
    const uint8_t *signature);

/**
 * @brief Verify SM2 Signature
 *
 * @param[in] E E value, 32 bytes, big-endian
 * @param[in] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @param[in] signature Signature r and s, 64 bytes, big-endian
 * @return
 *     EHSM_ERR_SW_SUCCESS(success, the signature is valid); other(error or the signature is invalid)
 * @note
 */
uint32_t cpt_sm2_verify(const uint8_t *E, const uint8_t *pubKey, const uint8_t *signature);

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
 *     EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 */
uint32_t cpt_sm2_sign(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], uint8_t signature[64]);

/**
 * @brief Generate SM2 random Key pair, secure version
 *
 * @param[out] priKey private key, 32 bytes, big-endian
 * @param[out] pubKey public key(0x04 + x + y), 65 bytes, big-endian
 * @return EHSM_ERR_SW_SUCCESS(success); SM2_ERROR_S(error)
 * @note
 */
uint32_t cpt_sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65]);

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
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. M and C can be the same buffer
 *     2. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 *     3. please make sure pubKey is valid
 */
uint32_t cpt_sm2_encrypt(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], const uint8_t pubKey[65],
    cpt_sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen);

/**
 * @brief SM2 Decryption
 *
 * @param[in] C ciphertext, CByteLen bytes, big-endian
 * @param[in] CByteLen byte length of C, please make sure CByteLen>97
 * @param[in] priKey private key, 32 bytes, big-endian
 * @param[in] order either SM2_C1C3C2 or SM2_C1C2C3
 * @param[out] M plaintext, MByteLen bytes, big-endian
 * @param[out] MByteLen byte length of M, should be CByteLen-97 if success
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. M and C can be the same buffer
 */
uint32_t cpt_sm2_decrypt(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32], cpt_sm2_cipher_order_e order,
    uint8_t *M, uint32_t *MByteLen);

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
 * @return EHSM_ERR_SW_SUCCESS(success); other(error)
 * @note
 *     1. please make sure the inputs are valid
 *     2. S1 and SA are optional, if you don't need, please set S1 and SA as NULL
 *     3. in case that S1(S2) and SA(SB) exist, if S1=SB,S2=SA, then exchange success.
 */
uint32_t cpt_sm2_exchangekey(cpt_sm2_exchange_role_e role, const uint8_t *dA, const uint8_t *PB, const uint8_t *rA,
    const uint8_t *RA, const uint8_t *RB, const uint8_t *ZA, const uint8_t *ZB, uint32_t kByteLen, uint8_t *KA,
    uint8_t *S1, uint8_t *SA);

/**
 * @brief get rand(without entropy reducing)
 *
 * @param[in] random byte buffer rand
 * @param[in] bytes byte length of rand
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_get_rand(uint8_t *random, uint32_t bytes);

/**
 * @brief get rand with fast speed(with entropy reducing, for such as clearing tmp buffer)
 *
 * @param[in] random byte buffer rand
 * @param[in] bytes byte length of rand
 * @return EHSM_ERR_SW_SUCCESS(success), other(error)
 * @note
 */
uint32_t cpt_get_rand_fast(uint8_t *random, uint32_t bytes);

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
void cpt_trng_reseed(void);

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
uint32_t cpt_sm2_getZ(const uint8_t *ID, uint32_t byteLenofID, const uint8_t pubKey[65], uint8_t Z[32]);

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
uint32_t cpt_sm2_getE(const uint8_t *M, uint32_t byteLen, const uint8_t Z[32], uint8_t E[32]);

/**
 * @brief get pke IP version
 *  none
 * @return pke IP version
 * @note
 */
uint32_t cpt_pke_get_version(void);

/**
 * @brief get pke driver version
 *  none
 * @return pke driver version(software version)
 * @note
 */
uint32_t cpt_pke_get_driver_version(void);

/**
 * @brief get ske IP version
 *  none
 * @return ske IP version
 * @note
 */
uint32_t cpt_ske_get_version(void);

/**
 * @brief get ske driver version
 *  none
 * @return ske driver version(software version)
 * @note
 */
uint32_t cpt_ske_get_driver_version(void);

/**
 * @brief get HFE IP version
 *  none
 * @return HFE IP version
 * @note
 */
uint32_t cpt_hash_get_version(void);

/**
 * @brief get hash driver version
 *  none
 * @return hash driver version(software version)
 * @note
 */
uint32_t cpt_hash_get_driver_version(void);

/**
 * @brief get trng IP version
 *  none
 * @return trng IP version(hardware version)
 * @note
 */
uint32_t cpt_trng_get_version(void);

/**
 * @brief get trng driver version
 *  none
 * @return trng driver version(software version)
 * @note
 */
uint32_t cpt_trng_get_driver_version(void);

#endif /* APPLICATION_SRC_SEIP_CORE_CRYPTO_LIB_API_H_ */
