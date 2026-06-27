#ifndef SHA3_384_H
#define SHA3_384_H



#include "hash.h"



#ifdef __cplusplus
extern "C" {
#endif


#ifdef SUPPORT_HASH_SHA3_384


typedef hash_ctx_st SHA3_384_CTX;

#ifdef HASH_DMA_FUNCTION
typedef hash_dma_ctx_st SHA3_384_DMA_CTX;
#endif


//APIs

uint32_t sha3_384_init(SHA3_384_CTX *ctx);

uint32_t sha3_384_update(SHA3_384_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t sha3_384_final(SHA3_384_CTX *ctx, uint8_t *digest);

uint32_t sha3_384(const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef SUPPORT_HASH_NODE
uint32_t sha3_384_node_steps(const hash_node_st *node, uint32_t node_num, uint8_t *digest);
#endif


#ifdef HASH_DMA_FUNCTION
uint32_t sha3_384_dma_init(SHA3_384_DMA_CTX *ctx, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha3_384_dma_update_blocks(SHA3_384_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes);

uint32_t sha3_384_dma_final(SHA3_384_DMA_CTX *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t digest_h, uint32_t digest_l);

uint32_t sha3_384_dma(uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t sha3_384_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback);
#endif
#else
uint32_t sha3_384_dma_update_blocks(SHA3_384_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes);

uint32_t sha3_384_dma_final(SHA3_384_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest);

uint32_t sha3_384_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t sha3_384_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback);
#endif
#endif
#endif


#endif


#ifdef __cplusplus
}
#endif


#endif

