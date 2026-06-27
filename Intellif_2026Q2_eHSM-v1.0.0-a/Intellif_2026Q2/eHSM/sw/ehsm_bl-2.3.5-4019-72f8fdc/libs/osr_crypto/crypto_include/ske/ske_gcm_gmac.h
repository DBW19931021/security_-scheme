#ifndef SKE_GCM_H
#define SKE_GCM_H


#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif




#define SKE_GCM_MAX_BYTES    (16U)




typedef struct{
    ske_ctx_st ske_gcm_ctx[1];
    uint32_t buf[4];
    uint32_t aad_bytes;
    uint32_t c_bytes;
    uint32_t current_bytes;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t mid_iv[4];
    uint32_t mid_mac[4];
#endif
    uint8_t mac_bytes;
} ske_gcm_ctx_st;


typedef ske_gcm_ctx_st SKE_GCM_CTX;
typedef ske_gcm_ctx_st ske_gcm_st;


typedef struct{
    ske_gcm_ctx_st ske_gcm_ctx[1];
    uint32_t block_buf[4];
    uint8_t left_bytes;
    ske_mac_e mac_action;
} ske_gmac_ctx_st;



typedef ske_gmac_ctx_st SKE_GMAC_CTX;
typedef ske_gmac_ctx_st ske_gmac_st;


//APIs for user
uint32_t ske_gcm_init(ske_gcm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes, uint8_t mac_bytes);

uint32_t ske_gcm_update_blocks_aad(ske_gcm_ctx_st *ctx, const uint8_t *aad, uint32_t bytes);

uint32_t ske_gcm_update_blocks(ske_gcm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);

uint32_t ske_gcm_final(ske_gcm_ctx_st *ctx, uint8_t *mac);

uint32_t ske_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out, uint32_t c_bytes,
        uint8_t *mac, uint8_t mac_bytes);


#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_gcm_init(ske_gcm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes, uint8_t mac_bytes);


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_blocks_whole_aad(ske_gcm_ctx_st *ctx, uint32_t aad_h, uint32_t aad_l, 
        SKE_CALLBACK callback);

uint32_t ske_dma_gcm_update_blocks(ske_gcm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes, 
        uint32_t out_h, uint32_t out_l, SKE_CALLBACK callback);

uint32_t ske_dma_gcm_update_final(ske_gcm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l);
#else
uint32_t ske_dma_gcm_update_blocks_whole_aad(ske_gcm_ctx_st *ctx, uint32_t *aad, SKE_CALLBACK callback);

uint32_t ske_dma_gcm_update_blocks(ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, 
        SKE_CALLBACK callback);

uint32_t ske_dma_gcm_update_final(ske_gcm_ctx_st *ctx, uint8_t *mac);
#endif

#endif


#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_all_blocks(const ske_gcm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, SKE_CALLBACK callback);

uint32_t ske_dma_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, uint32_t aad_bytes, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t c_bytes, uint8_t mac_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_gcm_update_all_blocks(const ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t *out, SKE_CALLBACK callback);

uint32_t ske_dma_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, uint8_t mac_bytes,
        SKE_CALLBACK callback);
#endif

uint32_t ske_dma_gcm_final(ske_gcm_ctx_st *ctx);

#endif



#ifdef SUPPORT_SKE_MODE_GMAC
uint32_t ske_gmac_init(ske_gmac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint8_t mac_bytes);

uint32_t ske_gmac_update(ske_gmac_ctx_st *ctx, const uint8_t *msg, uint32_t bytes);

uint32_t ske_gmac_final(ske_gmac_ctx_st *ctx, uint8_t *mac);

uint32_t ske_gmac(ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes);

#ifdef SKE_DMA_FUNCTION
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, 
        uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, 
        uint8_t mac_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_gmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, 
        uint32_t *msg, uint32_t msg_bytes, uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback);
#endif
#endif

#endif




#ifdef __cplusplus
}
#endif

#endif
