#include "../../crypto_include/hash_hmac/sha512.h"





#ifdef SUPPORT_HASH_SHA512


/* function: init sha512
 * parameters:
 *     ctx ------------------------ input, SHA512_CTX context pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sha512_init(SHA512_CTX *ctx)
{
    return hash_init(ctx, HASH_SHA512);
}


/* function: sha512 update message
 * parameters:
 *     ctx ------------------------ input, SHA512_CTX context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t sha512_update(SHA512_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    return hash_update(ctx, msg, msg_bytes);
}


/* function: message update done, get the sha512 digest
 * parameters:
 *     digest --------------------- output, sha512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t sha512_final(SHA512_CTX *ctx, uint8_t *digest)
{
    return hash_final(ctx, digest);
}


/* function: input whole message and get its sha512 digest
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, sha512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t sha512(const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    return hash(HASH_SHA512, msg, msg_bytes, digest);
}


#ifdef SUPPORT_HASH_NODE
/* function: input whole message and get its sha512 digest(node style)
 * parameters:
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, sha512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 */
uint32_t sha512_node_steps(const hash_node_st *node, uint32_t node_num, uint8_t *digest)
{
    return hash_node_steps(HASH_SHA512, node, node_num, digest);
}
#endif


#ifdef HASH_DMA_FUNCTION
/* function: init dma sha512
 * parameters:
 *     ctx ------------------------ input, SHA512_DMA_CTX context pointer
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t sha512_dma_init(SHA512_DMA_CTX *ctx, HASH_CALLBACK callback)
{
    return hash_dma_init(ctx, HASH_SHA512, callback);
}


/* function: dma sha512 update some message blocks
 * parameters:
 *     ctx ------------------------ input, SHA512_DMA_CTX context pointer
 *     msg ------------------------ input, message blocks
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of sha512
 *                                  block byte length(128)
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha512_dma_update_blocks(SHA512_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
{
    return hash_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
}
#else
uint32_t sha512_dma_update_blocks(SHA512_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes)
{
    return hash_dma_update_blocks(ctx, msg, msg_bytes);
}
#endif


/* function: dma sha512 final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, SHA512_DMA_CTX context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message
 *     digest --------------------- output, sha512 digest, 64 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha512_dma_final(SHA512_DMA_CTX *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t digest_h, uint32_t digest_l)
{
    return hash_dma_final(ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, digest_h, digest_l);
}
#else
uint32_t sha512_dma_final(SHA512_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest)
{
    return hash_dma_final(ctx, remainder_msg, remainder_bytes, digest);
}
#endif


/* function: dma sha512 digest calculate
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, sha512 digest, 64 bytes
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha512_dma(uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback)
{
    return hash_dma(HASH_SHA512, msg_h, msg_l, msg_bytes, digest_h, digest_l, callback);
}
#else
uint32_t sha512_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback)
{
    return hash_dma(HASH_SHA512, msg, msg_bytes, digest, callback);
}
#endif


#ifdef SUPPORT_HASH_DMA_NODE
/* function: input whole message and get its sha512 digest(dma node style)
 * parameters:
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, sha512 digest, 64 bytes
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 *     3. for every node or segment except the last, its message length must be a multiple of block length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t sha512_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback)
{
    return hash_dma_node_steps(HASH_SHA512, node, node_num, digest_h, digest_l, callback);
}
#else
uint32_t sha512_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback)
{
    return hash_dma_node_steps(HASH_SHA512, node, node_num, digest, callback);
}
#endif
#endif

#endif


#endif

