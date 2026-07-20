#ifndef SKE_CBC_MAC_H
#define SKE_CBC_MAC_H



#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    ske_ctx_st ske_cbc_mac_ctx[1];
    uint32_t block_buf[4];
    uint8_t mac_bytes;
    uint8_t is_updated;
    uint8_t left_bytes;
    ske_padding_e padding;
    ske_mac_e mac_action;
} ske_cbc_mac_ctx_st;


typedef struct {
    ske_ctx_st ske_cbc_mac_ctx[1];
    uint8_t mac_bytes;
} ske_cbc_dma_mac_ctx_st;


typedef ske_cbc_mac_ctx_st ske_cbc_mac_st;
typedef ske_cbc_dma_mac_ctx_st ske_cbc_mac_dma_st;
typedef ske_cbc_mac_ctx_st SKE_CBC_MAC_CTX;
typedef ske_cbc_dma_mac_ctx_st SKE_CBC_MAC_DMA_CTX;


//APIs for internal
uint32_t ske_cbc_mac_init_internal(ske_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx,
        uint8_t mac_bytes);



//APIs for user
uint32_t ske_cbc_mac_init(ske_cbc_mac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, ske_padding_e padding, const uint8_t *key,
        uint16_t sp_key_idx, uint8_t mac_bytes);

uint32_t ske_cbc_mac_update(ske_cbc_mac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t ske_cbc_mac_final(ske_cbc_mac_ctx_st *ctx, uint8_t *mac);

uint32_t ske_cbc_mac(ske_alg_e alg, ske_mac_e mac_action, ske_padding_e padding, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes);


#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_cbc_mac_init(ske_cbc_dma_mac_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_dma_mac_ctx_st *ctx, uint32_t msg_h,
        uint32_t msg_l, uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_cbc_mac_update_including_last_block(ske_cbc_dma_mac_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback);

uint32_t ske_dma_cbc_mac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l,
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, uint8_t mac_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_dma_mac_ctx_st *ctx, const uint32_t *msg,
        uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_cbc_mac_update_including_last_block(ske_cbc_dma_mac_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback);

uint32_t ske_dma_cbc_mac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback);
#endif

#endif






#ifdef __cplusplus
}
#endif

#endif

