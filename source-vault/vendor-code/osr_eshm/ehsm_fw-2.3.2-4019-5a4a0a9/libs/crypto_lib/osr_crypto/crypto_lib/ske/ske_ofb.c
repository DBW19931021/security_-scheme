

#include "../../crypto_include/ske/ske_ofb.h"




#ifdef SUPPORT_SKE_MODE_OFB


uint32_t ske_ofb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding)
{
    return ske_init(ctx, alg, SKE_MODE_OFB, crypto, key, sp_key_idx, iv, padding);
}

uint32_t ske_ofb_update_blocks(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    return ske_update_blocks(ctx, in, out, bytes);
}

uint32_t ske_ofb_update_including_last_block(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    return ske_update_including_last_block(ctx, in, out, in_bytes, out_bytes);
}

uint32_t ske_ofb_final(ske_ctx_st *ctx)
{
    return ske_final(ctx);
}

uint32_t ske_ofb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes)
{
    return ske_crypto(alg, SKE_MODE_OFB, crypto, key, sp_key_idx, iv, padding, in, out, in_bytes, out_bytes);
}


#ifdef SKE_DMA_FUNCTION

uint32_t ske_dma_ofb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, ske_padding_e padding)
{
    return ske_dma_init(ctx, alg, SKE_MODE_OFB, crypto, key, sp_key_idx, iv, padding);
}

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ofb_update_blocks(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_blocks(ctx, in_h, in_l, out_h, out_l, bytes, callback);
}

uint32_t ske_dma_ofb_update_including_last_block(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_including_last_block(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
}

uint32_t ske_dma_ofb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_crypto(alg, SKE_MODE_OFB, crypto, key, sp_key_idx, iv, padding, in_h, in_l, out_h, out_l, 
            in_bytes, out_bytes, callback);
}
#else
uint32_t ske_dma_ofb_update_blocks(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes, 
        SKE_CALLBACK callback)
{
    return ske_dma_update_blocks(ctx, in, out, bytes, callback);
}

uint32_t ske_dma_ofb_update_including_last_block(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_including_last_block(ctx, in, out, in_bytes, out_bytes, callback);
}

uint32_t ske_dma_ofb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, uint32_t *in, uint32_t *out, uint32_t in_bytes, uint32_t *out_bytes, 
        SKE_CALLBACK callback)
{
    return ske_dma_crypto(alg, SKE_MODE_OFB, crypto, key, sp_key_idx, iv, padding, in, out, in_bytes, 
            out_bytes, callback);
}
#endif

uint32_t ske_dma_ofb_final(ske_ctx_st *ctx)
{
    return ske_dma_final(ctx);
}
#endif


#endif

