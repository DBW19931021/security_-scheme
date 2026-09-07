#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../../crypto_hal/hash_basic.h"



//to calculate hash or hmac
typedef enum
{
    HASH_MODE,
    HMAC_MODE
}hfe_mode_e;


//HASH status
typedef struct {
    uint32_t busy             : 1;        // calculate busy flag
} hash_status_st;


//HASH context
typedef struct
{
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];    //keep current hash iterator value for multiple thread
#endif
    uint8_t hash_buffer[HASH_BLOCK_MAX_BYTE_LEN];     //block buffer
    uint32_t total[HASH_TOTAL_LEN_MAX_WORD_LEN];      //total byte length of the whole message
    hash_status_st status;                             //hash update status, .busy=1 means doing, .busy=0 means idle
    hash_alg_e alg;                                //current hash algorithm
    hfe_mode_e hfe_mode;                                //the input message is for hash algorithm or for hmac algorithm
    uint8_t block_byte_len;
    uint8_t iterator_word_len;
    uint8_t digest_byte_len;
    uint8_t first_update_flag;                        //whether first time to update message(1:yes, 0:no)
    uint8_t finish_flag;                              //whether the whole message has been inputted(1:yes, 0:no)
} hash_ctx_st;
typedef hash_ctx_st hash_st;


#ifdef HASH_DMA_FUNCTION
//HASH DMA context
typedef struct
{
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];    //keep current hash iterator value for multiple thread
#endif

    uint32_t total[HASH_TOTAL_LEN_MAX_WORD_LEN];      //total byte length of the whole message
    HASH_CALLBACK callback;
    hash_alg_e alg;                                //current hash algorithm
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    hfe_mode_e hfe_mode;                                //the input message is for hash algorithm or for hmac algorithm
#endif
    uint8_t block_word_len;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    uint8_t iterator_word_len;
    uint8_t first_update_flag;                        //whether first time to update message(1:yes, 0:no)
#endif

    uint8_t digest_byte_len;                          //just for hmac-sha3
} hash_dma_ctx_st;
#endif


#ifdef SUPPORT_HASH_NODE
typedef struct {
    const uint8_t *msg_addr;
    uint32_t msg_bytes;
} hash_node_st;
#endif


#ifdef SUPPORT_HASH_DMA_NODE
typedef struct {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    uint32_t msg_addr_h;
    uint32_t msg_addr_l;
#else
    const uint32_t *msg_addr;
#endif
    uint32_t msg_bytes;
} hash_dma_node_st;
#endif


//fix to old project
typedef hash_status_st hash_status_t ;
typedef hfe_mode_e HFE_MODE ;
typedef hash_alg_e HASH_ALG;
typedef hash_ctx_st HASH_CTX;
typedef hash_dma_ctx_st hash_dma_st;
typedef hash_dma_ctx_st HASH_DMA_CTX;
typedef hash_node_st HASH_NODE;
typedef hash_dma_node_st HASH_DMA_NODE;


//APIs
uint32_t check_hash_alg(hash_alg_e alg);

uint8_t hash_get_block_word_len(hash_alg_e alg);

uint8_t hash_get_iterator_word_len(hash_alg_e alg);

uint8_t hash_get_digest_word_len(hash_alg_e alg);

void hash_set_IV(hash_alg_e alg, uint32_t hash_iterator_words);

uint32_t hash_total_byte_len_add_uint32(uint32_t *a, uint32_t a_words, uint32_t b);

void hash_total_bytelen_2_bitlen(uint32_t *a, uint32_t a_words);

uint32_t hash_calc_blocks(hash_ctx_st *ctx, const uint8_t *msg, uint32_t block_count);

uint32_t hash_calc_rand_len_msg(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t hash_total_byte_len_update(hash_ctx_st *ctx, uint32_t in_bytes);


uint32_t hash_init_with_iv_and_updated_length(hash_ctx_st *ctx, hash_alg_e alg, const uint32_t *iv, 
        uint32_t byte_length_h, uint32_t byte_length_l);

uint32_t hash_init(hash_ctx_st *ctx, hash_alg_e alg);

uint32_t hash_update(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t hash_final(hash_ctx_st *ctx, uint8_t *digest);

uint32_t hash(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest);

#ifdef SUPPORT_HASH_NODE
uint32_t hash_node_steps(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *digest);
#endif


#ifdef HASH_DMA_FUNCTION
uint32_t hash_dma_init_with_iv_and_updated_length(hash_dma_ctx_st *ctx, hash_alg_e alg, const uint32_t *iv, 
        uint32_t byte_length_h, uint32_t byte_length_l, HASH_CALLBACK callback);

uint32_t hash_dma_init(hash_dma_ctx_st *ctx, hash_alg_e alg, HASH_CALLBACK callback);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_update_blocks(hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes);

uint32_t hash_dma_final(hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l);

uint32_t hash_dma(hash_alg_e alg, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hash_dma_node_steps(hash_alg_e alg, const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback);
#endif
#else
uint32_t hash_dma_update_blocks(hash_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes);

uint32_t hash_dma_final(hash_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes, uint32_t *digest);

uint32_t hash_dma(hash_alg_e alg, uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback);

#ifdef SUPPORT_HASH_DMA_NODE
uint32_t hash_dma_node_steps(hash_alg_e alg, const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback);
#endif
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif


