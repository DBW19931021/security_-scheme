#ifndef SHA1_H
#define SHA1_H



#include "hash.h"



#ifdef __cplusplus
extern "C" {
#endif


#ifdef SUPPORT_HASH_SHA1


typedef hash_ctx_st SHA1_CTX;

#ifdef HASH_DMA_FUNCTION
typedef hash_dma_ctx_st SHA1_DMA_CTX;
#endif


//APIs

uint32_t sha1_init(SHA1_CTX *ctx);

uint32_t sha1_update(SHA1_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t sha1_final(SHA1_CTX *ctx, uint8_t *digest);

uint32_t sha1(const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef SUPPORT_HASH_NODE
uint32_t sha1_node_steps(const hash_node_st *node, uint32_t node_num, uint8_t *digest);
#endif


#ifdef HASH_DMA_FUNCTION
uint32_t sha1_dma_init(SHA1_DMA_CTX *ctx, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha1_dma_update_blocks(SHA1_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes);

uint32_t sha1_dma_final(SHA1_DMA_CTX *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t digest_h, uint32_t digest_l);

uint32_t sha1_dma(uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t sha1_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback);
#endif
#else
uint32_t sha1_dma_update_blocks(SHA1_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes);

uint32_t sha1_dma_final(SHA1_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t sha1_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t sha1_dma_node_steps(hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback);
#endif
#endif
#endif


#endif


#ifdef __cplusplus
}
#endif


#endif

