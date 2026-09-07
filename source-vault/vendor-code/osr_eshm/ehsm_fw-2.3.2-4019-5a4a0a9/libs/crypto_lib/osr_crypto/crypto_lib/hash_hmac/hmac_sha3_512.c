#include "../../crypto_include/hash_hmac/hmac_sha3_512.h"





#ifdef SUPPORT_HASH_SHA3_512


/* function: init hmac-sha3_512
 * parameters:
 *     ctx ------------------------ input, HMAC_SHA3_512_CTX context pointer
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_sha3_512_init(HMAC_SHA3_512_CTX *ctx, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes)
{
    return hmac_init(ctx, HASH_SHA3_512, key, sp_key_idx, key_bytes);
}


/* function: hmac-sha3_512 update message
 * parameters:
 *     ctx ------------------------ input, HMAC_SHA3_512_CTX context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_sha3_512_update(HMAC_SHA3_512_CTX *ctx, const  uint8_t *msg, uint32_t msg_bytes)
{
    return hmac_update(ctx, msg, msg_bytes);
}


/* function: message update done, get the hmac
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the mac buffer is sufficient
 */
uint32_t hmac_sha3_512_final(HMAC_SHA3_512_CTX *ctx, uint8_t *mac)
{
    return hmac_final(ctx, mac);
}


/* function: input key and whole message, get the hmac
 * parameters:
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of the key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the mac buffer is sufficient
 */
uint32_t hmac_sha3_512(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, const  uint8_t *msg, 
        uint32_t msg_bytes, uint8_t *mac)
{
    return hmac(HASH_SHA3_512, key, sp_key_idx, key_bytes, msg, msg_bytes, mac);
}


#ifdef SUPPORT_HASH_NODE
/* function: input key and whole message, get the hmac(node style)
 * parameters:
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of the key
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the mac buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 */
uint32_t hmac_sha3_512_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_node_st *node, uint32_t node_num, uint8_t *mac)
{
    return hmac_node_steps(HASH_SHA3_512, key, sp_key_idx, key_bytes, node, node_num, mac);
}
#endif


#ifdef HASH_DMA_FUNCTION
/* function: init dma hmac-sha3_512
 * parameters:
 *     ctx ------------------------ input, HMAC_SHA3_512_DMA_CTX context pointer
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_sha3_512_dma_init(HMAC_SHA3_512_DMA_CTX *ctx, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes, HASH_CALLBACK callback)
{
    return hmac_dma_init(ctx, HASH_SHA3_512, key, sp_key_idx, key_bytes, callback);
}


/* function: dma hmac-sha3_512 update message
 * parameters:
 *     ctx ------------------------ input, HMAC_SHA3_512_DMA_CTX context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of block byte length
 *                                  of SHA3_512(72)
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_sha3_512_dma_update_blocks(HMAC_SHA3_512_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes)
{
    return hmac_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
}
#else
uint32_t hmac_sha3_512_dma_update_blocks(HMAC_SHA3_512_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes)
{
    return hmac_dma_update_blocks(ctx, msg, msg_bytes);
}
#endif


/* function: dma hmac-sha3_512 message update done, get the hmac
 * parameters:
 *     ctx ------------------------ input, HMAC_SHA3_512_DMA_CTX context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the remainder message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_sha3_512_dma_final(HMAC_SHA3_512_DMA_CTX *ctx, uint32_t remainder_msg_h, 
        uint32_t remainder_msg_l, uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l)
{
    return hmac_dma_final(ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, mac_h, mac_l);
}
#else
uint32_t hmac_sha3_512_dma_final(HMAC_SHA3_512_DMA_CTX *ctx, uint32_t *remainder_msg, 
        uint32_t remainder_bytes, uint32_t *mac)
{
    return hmac_dma_final(ctx, remainder_msg, remainder_bytes, mac);
}
#endif


/* function: dma hmac-sha3_512 input key and message, get the hmac
 * parameters:
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_sha3_512_dma(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback)
{
    return hmac_dma(HASH_SHA3_512, key, sp_key_idx, key_bytes, msg_h, msg_l, msg_bytes, mac_h, 
            mac_l, callback);
}
#else
uint32_t hmac_sha3_512_dma(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, 
        uint32_t msg_bytes, uint32_t *mac, HASH_CALLBACK callback)
{
    return hmac_dma(HASH_SHA3_512, key, sp_key_idx, key_bytes, msg, msg_bytes, mac, callback);
}
#endif


#ifdef SUPPORT_HASH_DMA_NODE
/* function: dma hmac input key and message, get the hmac(node style)
 * parameters:
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     mac ------------------------ output, hmac
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 *     3. for every node or segment except the last, its message length must be a multiple of block length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_sha3_512_dma_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const  hash_dma_node_st *node, uint32_t node_num, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback)
{
    return hmac_dma_node_steps(HASH_SHA3_512, key, sp_key_idx, key_bytes, node, node_num, mac_h, mac_l, 
            callback);
}
#else
uint32_t hmac_sha3_512_dma_node_steps(const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const  hash_dma_node_st *node, uint32_t node_num, uint32_t *mac, HASH_CALLBACK callback)
{
    return hmac_dma_node_steps(HASH_SHA3_512, key, sp_key_idx, key_bytes, node, node_num, mac, callback);
}
#endif
#endif

#endif


#endif

