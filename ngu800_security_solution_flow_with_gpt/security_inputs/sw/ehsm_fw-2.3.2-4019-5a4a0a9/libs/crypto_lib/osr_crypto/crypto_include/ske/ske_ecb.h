#ifndef SKE_ECB_H
#define SKE_ECB_H


#include "ske.h"



#ifdef __cplusplus
extern "C" {
#endif






//APIs for user
uint32_t ske_ecb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        ske_padding_e padding);

uint32_t ske_ecb_update_blocks(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_ecb_update_including_last_block(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes);

uint32_t ske_ecb_final(ske_ctx_st *ctx);

uint32_t ske_ecb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        ske_padding_e padding, const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes);


#ifdef SKE_DMA_FUNCTION

uint32_t ske_dma_ecb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        ske_padding_e padding);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ecb_update_blocks(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t bytes, SKE_CALLBACK callback);

uint32_t ske_dma_ecb_update_including_last_block(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_ecb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_ecb_update_blocks(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback);

uint32_t ske_dma_ecb_update_including_last_block(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes,
        uint32_t *out_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_ecb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        ske_padding_e padding, uint32_t *in, uint32_t *out, uint32_t in_bytes, uint32_t *out_bytes,
        SKE_CALLBACK callback);
#endif

uint32_t ske_dma_ecb_final(ske_ctx_st *ctx);

#endif







#ifdef __cplusplus
}
#endif

#endif
