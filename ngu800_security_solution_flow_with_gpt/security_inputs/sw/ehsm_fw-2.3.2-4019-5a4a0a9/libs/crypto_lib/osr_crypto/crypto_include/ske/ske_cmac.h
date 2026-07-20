#ifndef SKE_CMAC_H
#define SKE_CMAC_H



#include "ske.h"


#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
#ifndef CONFIG_SUPPORT_STRUCTURE_OPTIMIZATION
#if (defined(SUPPORT_SKE_TDES_128))
    uint8_t k1[16]; //acctually no use
    uint8_t k2[16]; //acctually no use
#endif
#endif
    ske_ctx_st ske_cmac_ctx[1];
    uint32_t block_buf[4];
    uint8_t mac_bytes;
    uint8_t left_bytes;
    ske_mac_e mac_action;
} ske_cmac_ctx_st;

typedef struct {
#ifndef CONFIG_SUPPORT_STRUCTURE_OPTIMIZATION
#if (defined(SUPPORT_SKE_TDES_128))
    uint8_t k1[16]; //acctually no use
    uint8_t k2[16]; //acctually no use
#endif
#endif
    ske_ctx_st ske_cmac_ctx[1];
    uint8_t mac_bytes;
} ske_cmac_dma_st;

typedef ske_cmac_ctx_st ske_cmac_st;
typedef ske_cmac_ctx_st SKE_CMAC_CTX;
typedef ske_cmac_dma_st SKE_CMAC_DMA_CTX;


//APIs for internal
uint32_t ske_cmac_init_internal(ske_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes);




//APIs for user
uint32_t ske_cmac_init(ske_cmac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx,
        uint8_t mac_bytes);

uint32_t ske_cmac_update(ske_cmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t ske_cmac_final(ske_cmac_ctx_st *ctx,uint8_t *mac);

uint32_t ske_cmac(ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *msg, uint32_t msg_bytes, 
        uint8_t *mac, uint8_t mac_bytes);


#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_cmac_init(ske_cmac_dma_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_cmac_update_including_last_block(ske_cmac_dma_st *ctx, uint32_t msg_h, uint32_t msg_l,
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback);

uint32_t ske_dma_cmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, uint8_t mac_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_st *ctx, uint32_t *msg,
        uint32_t msg_bytes, SKE_CALLBACK callback);

uint32_t ske_dma_cmac_update_including_last_block(ske_cmac_dma_st *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback);

uint32_t ske_dma_cmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback);
#endif


#endif







#ifdef __cplusplus
}
#endif

#endif

