#ifndef SKE_XTS_H
#define SKE_XTS_H



#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif


#if 0
#define XTS_FOR_INTEGRITY
#endif

typedef struct _ske_xts_st {
    ske_ctx_st ske_xts_ctx[1];
    uint32_t c_bytes;
    uint32_t current_bytes;
#ifndef CONFIG_SUPPORT_STRUCTURE_OPTIMIZATION
    uint8_t t[16]; //actually no use
#endif
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t mid_iv[4]; //actually no use
#endif
#ifndef CONFIG_SUPPORT_STRUCTURE_OPTIMIZATION
    void (*ske_xts_update_t)(struct _ske_xts_st *ctx); //actually no use
#endif
} ske_xts_ctx_st;


typedef enum
{
    XTS_IEEE  = 0,
    XTS_GM    = 1,
} xts_style_e;

typedef ske_xts_ctx_st ske_xts_st;
typedef ske_xts_ctx_st SKE_XTS_CTX;
typedef xts_style_e XTS_STYLE;


//APIs for user
uint32_t ske_xts_init(ske_xts_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *i, uint32_t c_bytes);

uint32_t ske_xts_update_blocks(ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_xts_final(ske_xts_ctx_st *ctx);

uint32_t ske_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        const uint8_t *in, uint8_t *out, uint32_t c_bytes);


#ifdef SKE_DMA_FUNCTION

uint32_t ske_dma_xts_init(ske_xts_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *i, uint32_t c_bytes);


#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_xts_update_blocks(ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t bytes, SKE_CALLBACK callback);

uint32_t ske_dma_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h,
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback);

uint32_t ske_dma_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t c_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_xts_update_blocks(ske_xts_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback);

uint32_t ske_dma_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback);

uint32_t ske_dma_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        uint32_t *in, uint32_t *out, uint32_t c_bytes, SKE_CALLBACK callback);
#endif

uint32_t ske_dma_xts_final(ske_xts_ctx_st *ctx);

#endif





#ifdef __cplusplus
}
#endif

#endif
