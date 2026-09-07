#ifndef SKE_H
#define SKE_H


#include "../../crypto_hal/ske_basic.h"




#ifdef __cplusplus
extern "C" {
#endif


//APIs
void ske_clear_block_tail(uint32_t *in, uint32_t bytes);

uint32_t ske_check_alg(ske_alg_e ske_alg);

uint32_t ske_check_mode(ske_alg_e ske_alg, ske_mode_e ske_mode);

uint8_t ske_get_block_byte_len(ske_alg_e ske_alg);

uint8_t ske_get_key_byte_len(ske_alg_e alg);

void ske_set_iv(const uint8_t *iv, uint32_t block_bytes);

void ske_set_key(ske_alg_e alg, const uint8_t *key, uint8_t key_bytes, uint16_t key_idx);

uint32_t ske_init_internal(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key,
        uint16_t sp_key_idx, const uint8_t *iv);

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
uint32_t ske_keep_alg_key_iv(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv);
#endif

uint32_t ske_check_padding(const uint8_t *block, uint32_t block_bytes, ske_padding_e padding, uint32_t *valid_bytes);


uint32_t ske_init(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, 
        uint16_t sp_key_idx, const uint8_t *iv, ske_padding_e padding);

uint32_t ske_update_blocks(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_update_including_last_block(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes);

uint32_t ske_final(ske_ctx_st *ctx);

uint32_t ske_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes);


#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_init(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, 
        uint16_t sp_key_idx, const uint8_t *iv, ske_padding_e padding);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_update_blocks(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback);

uint32_t ske_dma_update_including_last_block(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, 
        uint32_t out_h, uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l,
        uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_update_blocks(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes, 
        SKE_CALLBACK callback);

uint32_t ske_dma_update_including_last_block(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding, uint32_t *in, uint32_t *out, uint32_t in_bytes, uint32_t *out_bytes, 
        SKE_CALLBACK callback);
#endif

uint32_t ske_dma_final(ske_ctx_st *ctx);

#endif




#ifdef __cplusplus
}
#endif

#endif

