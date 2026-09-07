#ifndef SKE_CCM_H
#define SKE_CCM_H


#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif








typedef struct{
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t mid_iv[4];
    uint32_t mid_mac[4];
#endif
    uint32_t buf[4];
    uint32_t aad_bytes;
    uint32_t c_bytes;
    uint32_t current_bytes;
    ske_ctx_st ske_ccm_ctx[1];
    uint8_t M;
    uint8_t L;
} ske_ccm_ctx_st;


typedef ske_ccm_ctx_st SKE_CCM_CTX;
typedef ske_ccm_ctx_st ske_ccm_st;


//APIs for internal
uint32_t ske_ccm_pre_init(ske_ccm_ctx_st *ctx, ske_crypto_e crypto, const uint8_t *nonce, uint8_t M, uint8_t L, 
        uint32_t aad_bytes, uint32_t c_bytes);

uint32_t ske_ccm_get_mac(ske_ccm_ctx_st *ctx);




//APIs for user
uint32_t ske_ccm_init(ske_ccm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes);

uint32_t ske_ccm_update_aad(ske_ccm_ctx_st *ctx, const uint8_t *aad);

uint32_t ske_ccm_update_blocks(ske_ccm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_ccm_final(ske_ccm_ctx_st *ctx, uint8_t *mac);

uint32_t ske_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out, uint32_t c_bytes,
        uint8_t *mac);


#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_ccm_init(ske_ccm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes);

void ske_ccm_get_B0(const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes, uint8_t out[16]);

void ske_ccm_get_B1(const uint8_t *aad, uint32_t aad_bytes, uint32_t *aad_offset, uint8_t out[16]);


#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_blocks_B0_and_whole_aad(ske_ccm_ctx_st *ctx, uint32_t B0_aad_h, uint32_t B0_aad_l, 
        SKE_CALLBACK callback);

uint32_t ske_dma_ccm_update_blocks(ske_ccm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes, 
        uint32_t out_h, uint32_t out_l, SKE_CALLBACK callback);

uint32_t ske_dma_ccm_update_final(ske_ccm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l);
#else
uint32_t ske_dma_ccm_update_blocks_B0_and_whole_aad(ske_ccm_ctx_st *ctx, uint32_t *B0_aad, SKE_CALLBACK callback);

uint32_t ske_dma_ccm_update_blocks(ske_ccm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, 
        SKE_CALLBACK callback);

uint32_t ske_dma_ccm_update_final(ske_ccm_ctx_st *ctx, uint8_t *mac);
#endif

#endif



#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_all_blocks(ske_ccm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, SKE_CALLBACK callback);

uint32_t ske_dma_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l,
        uint32_t c_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_ccm_update_all_blocks(ske_ccm_ctx_st *ctx, uint32_t *in, uint32_t *out, SKE_CALLBACK callback);

uint32_t ske_dma_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, SKE_CALLBACK callback);
#endif

uint32_t ske_dma_ccm_final(ske_ccm_ctx_st *ctx);
#endif





#ifdef __cplusplus
}
#endif

#endif

