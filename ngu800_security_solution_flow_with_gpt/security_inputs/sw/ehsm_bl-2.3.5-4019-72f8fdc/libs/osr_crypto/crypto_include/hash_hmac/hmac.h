#ifndef HMAC_H
#define HMAC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "hash.h"


#define HMAC_IPAD                 (0x36363636U)
#define HMAC_OPAD                 (0x5c5c5c5cU)
#define HMAC_IPAD_XOR_OPAD        (HMAC_IPAD ^ HMAC_OPAD)


//HMAC key length flag
#define HMAC_K_NOT_LONGER_THAN_BLOCK    (1U)    //1: key len <= block len
#define HMAC_K_LONGER_THAN_BLOCK        (2U)    //2: key len > block len


//HMAC context
typedef struct
{
    hash_ctx_st hash_ctx[1];
    uint32_t K0[HASH_BLOCK_MAX_WORD_LEN];
    uint32_t key_len_flag;
    uint32_t is_sp_key;
    uint32_t sp_key_bytes;
    uint16_t sp_key_idx;
} hmac_ctx_st;


//HMAC DMA context
#ifdef HASH_DMA_FUNCTION
typedef struct
{
    uint32_t K0[HASH_BLOCK_MAX_WORD_LEN];
    uint32_t key_len_flag;
    hash_dma_ctx_st hash_dma_ctx[1];
    uint32_t is_sp_key;
    uint32_t sp_key_bytes;
    uint16_t sp_key_idx;
} hmac_dma_ctx_st;
#endif

typedef hmac_ctx_st hmac_st;
typedef hmac_dma_ctx_st hmac_dma_st;
//fix to old project
typedef hmac_ctx_st HMAC_CTX;
typedef hmac_dma_ctx_st HMAC_DMA_CTX;


//APIs
void hmac_clear(void);

uint32_t hmac_key_state_recover(hash_alg_e alg, uint32_t key_len_flag, const uint32_t *K0, 
        uint32_t block_byte_len, uint32_t iterator_word_len, uint32_t is_sp_key, uint16_t sp_key_idx, uint32_t sp_key_bytes);

uint32_t hmac_init(hmac_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes);

uint32_t hmac_update(hmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
uint32_t hmac_sha3_final(hmac_ctx_st *ctx, uint8_t *mac);
#endif

uint32_t hmac_final(hmac_ctx_st *ctx, uint8_t *mac);

uint32_t hmac(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, const uint8_t *msg, 
        uint32_t msg_bytes, uint8_t *mac);

#ifdef SUPPORT_HASH_NODE
uint32_t hmac_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes,
        const hash_node_st *node, uint32_t node_num, uint8_t *mac);
#endif


#ifdef HASH_DMA_FUNCTION
uint32_t hmac_dma_init(hmac_dma_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_dma_update_blocks(hmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes);

uint32_t hmac_dma_final(hmac_dma_ctx_st *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l);

uint32_t hmac_dma(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hmac_dma_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_dma_node_st *node, uint32_t node_num, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback);
#endif
#else
uint32_t hmac_dma_update_blocks(hmac_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes);

uint32_t hmac_dma_final(hmac_dma_ctx_st *ctx, const uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *mac);

uint32_t hmac_dma(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg,
        uint32_t msg_bytes, uint32_t *mac, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hmac_dma_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_dma_node_st *node, uint32_t node_num, uint32_t *mac, HASH_CALLBACK callback);
#endif
#endif
#endif



#ifdef __cplusplus
}
#endif

#endif

