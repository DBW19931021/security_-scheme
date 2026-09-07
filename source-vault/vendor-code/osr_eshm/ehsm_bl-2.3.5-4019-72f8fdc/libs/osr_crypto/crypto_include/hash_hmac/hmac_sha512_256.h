#ifndef HMAC_SHA512_256_H
#define HMAC_SHA512_256_H



#include "hmac.h"



#ifdef __cplusplus
extern "C" {
#endif


#ifdef SUPPORT_HASH_SHA512_256


typedef hmac_ctx_st HMAC_SHA512_256_CTX;

#ifdef HASH_DMA_FUNCTION
typedef hmac_dma_ctx_st HMAC_SHA512_256_DMA_CTX;
#endif


//APIs

uint32_t hmac_sha512_256_init(HMAC_SHA512_256_CTX *ctx, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes);

uint32_t hmac_sha512_256_update(HMAC_SHA512_256_CTX *ctx, const  uint8_t *msg, uint32_t msg_bytes);

uint32_t hmac_sha512_256_final(HMAC_SHA512_256_CTX *ctx, uint8_t *mac);

uint32_t hmac_sha512_256(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, const  uint8_t *msg, 
        uint32_t msg_bytes, uint8_t *mac);

#ifdef SUPPORT_HASH_NODE
uint32_t hmac_sha512_256_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_node_st *node, uint32_t node_num, uint8_t *mac);
#endif


#ifdef HASH_DMA_FUNCTION
uint32_t hmac_sha512_256_dma_init(HMAC_SHA512_256_DMA_CTX *ctx, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_sha512_256_dma_update_blocks(HMAC_SHA512_256_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes);

uint32_t hmac_sha512_256_dma_final(HMAC_SHA512_256_DMA_CTX *ctx, uint32_t remainder_msg_h, 
        uint32_t remainder_msg_l, uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l);

uint32_t hmac_sha512_256_dma(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hmac_sha512_256_dma_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const  hash_dma_node_st *node, uint32_t node_num, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback);
#endif
#else
uint32_t hmac_sha512_256_dma_update_blocks(HMAC_SHA512_256_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes);

uint32_t hmac_sha512_256_dma_final(HMAC_SHA512_256_DMA_CTX *ctx, uint32_t *remainder_msg, 
        uint32_t remainder_bytes, uint32_t *mac);

uint32_t hmac_sha512_256_dma(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, 
        uint32_t msg_bytes, uint32_t *mac, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hmac_sha512_256_dma_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const  hash_dma_node_st *node, uint32_t node_num, uint32_t *mac, HASH_CALLBACK callback);
#endif
#endif
#endif


#endif


#ifdef __cplusplus
}
#endif


#endif

