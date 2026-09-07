#ifndef MD5_H
#define MD5_H



#include "hash.h"



#ifdef __cplusplus
extern "C" {
#endif


#ifdef SUPPORT_HASH_MD5


typedef hash_ctx_st MD5_CTX;

#ifdef HASH_DMA_FUNCTION
typedef hash_dma_ctx_st MD5_DMA_CTX;
#endif


//APIs

uint32_t md5_init(MD5_CTX *ctx);

uint32_t md5_update(MD5_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t md5_final(MD5_CTX *ctx, uint8_t *digest);

uint32_t md5(const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef SUPPORT_HASH_NODE
uint32_t md5_node_steps(const hash_node_st *node, uint32_t node_num, uint8_t *digest);
#endif


#ifdef HASH_DMA_FUNCTION

uint32_t md5_dma_init(MD5_DMA_CTX *ctx, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t md5_dma_update_blocks(MD5_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes);

uint32_t md5_dma_final(MD5_DMA_CTX *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t digest_h, uint32_t digest_l);

uint32_t md5_dma(uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t md5_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback);
#endif
#else
uint32_t md5_dma_update_blocks(MD5_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes);

uint32_t md5_dma_final(MD5_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t md5_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t md5_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback);
#endif
#endif
#endif


#endif


#ifdef __cplusplus
}
#endif


#endif

