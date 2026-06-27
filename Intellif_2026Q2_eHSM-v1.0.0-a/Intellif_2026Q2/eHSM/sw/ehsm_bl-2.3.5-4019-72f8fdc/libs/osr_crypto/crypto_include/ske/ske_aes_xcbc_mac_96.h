#ifndef AES_XCBC_MAC_96_H
#define AES_XCBC_MAC_96_H



#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint32_t k2[4];
    uint32_t k3[4];
    uint8_t block_buf[16];
    ske_ctx_st ske_xcbc_mac_ctx[1];
    uint8_t left_bytes;
    ske_mac_e mac_action;
} ske_aes_xcbc_mac_96_ctx_st;


typedef struct {
    uint32_t k2[4];
    uint32_t k3[4];
    uint8_t mid_iv[16];
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    uint32_t p_mid_iv_h;
    uint32_t p_mid_iv_l;
#else
    uint32_t *p_mid_iv;
#endif
    ske_ctx_st ske_xcbc_mac_ctx[1];
} ske_aes_xcbc_mac_96_dma_ctx_st;


typedef ske_aes_xcbc_mac_96_ctx_st SKE_AES_XCBC_MAC_96_CTX;
typedef ske_aes_xcbc_mac_96_dma_ctx_st SKE_AES_XCBC_MAC_96_DMA_CTX;


//APIs for internal
uint32_t ske_aes_xcbc_mac_96_recover_cfg(uint8_t dma_en, uint8_t *key, uint8_t *iv, uint16_t sp_key_idx);

uint32_t ske_aes_xcbc_mac_96_get_k1_k2_k3(ske_ctx_st *ctx, uint8_t *key, uint16_t sp_key_idx, uint32_t k1[4], 
        uint32_t k2[4], uint32_t k3[4]);




//APIs for user
uint32_t ske_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_ctx_st *ctx, ske_mac_e mac_action, uint8_t *key, uint16_t sp_key_idx);

uint32_t ske_aes_xcbc_mac_96_update(ske_aes_xcbc_mac_96_ctx_st *ctx, uint8_t *msg, uint32_t msg_bytes);

uint32_t ske_aes_xcbc_mac_96_final(ske_aes_xcbc_mac_96_ctx_st *ctx, uint8_t mac[12]);

uint32_t ske_aes_xcbc_mac_96(ske_mac_e mac_action, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg, uint32_t msg_bytes, 
        uint8_t mac[12]);



#ifdef SKE_DMA_FUNCTION

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint8_t *key, uint32_t mid_iv_h, 
        uint32_t mid_iv_l, uint16_t sp_key_idx);

uint32_t ske_dma_aes_xcbc_mac_96_update_blocks_excluding_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, 
        uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_aes_xcbc_mac_96_update_including_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback);

uint32_t ske_dma_aes_xcbc_mac_96(uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback);
#else
uint32_t ske_dma_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint8_t *key, uint32_t *mid_iv, 
        uint16_t sp_key_idx);

uint32_t ske_dma_aes_xcbc_mac_96_update_blocks_excluding_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, 
        uint32_t *msg, uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_aes_xcbc_mac_96_update_including_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint32_t *msg, 
        uint32_t msg_bytes, uint32_t *mac, SKE_CALLBACK callback);

uint32_t ske_dma_aes_xcbc_mac_96(uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback);
#endif


#endif





#ifdef __cplusplus
}
#endif

#endif

