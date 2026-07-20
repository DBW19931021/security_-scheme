#include "../../crypto_include/hash_hmac/md5.h"





#ifdef SUPPORT_HASH_MD5


/* function: init md5
 * parameters:
 *     ctx ------------------------ input, MD5_CTX context pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t md5_init(MD5_CTX *ctx)
{
    return hash_init(ctx, HASH_MD5);
}


/* function: md5 update message
 * parameters:
 *     ctx ------------------------ input, MD5_CTX context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t md5_update(MD5_CTX *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    return hash_update(ctx, msg, msg_bytes);
}


/* function: message update done, get the md5 digest
 * parameters:
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t md5_final(MD5_CTX *ctx, uint8_t *digest)
{
    return hash_final(ctx, digest);
}


/* function: input whole message and get its md5 digest
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t md5(const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    return hash(HASH_MD5, msg, msg_bytes, digest);
}


#ifdef SUPPORT_HASH_NODE
/* function: input whole message and get its md5 digest(node style)
 * parameters:
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 */
uint32_t md5_node_steps(const hash_node_st *node, uint32_t node_num, uint8_t *digest)
{
    return hash_node_steps(HASH_MD5, node, node_num, digest);
}
#endif


#ifdef HASH_DMA_FUNCTION
/* function: init dma md5
 * parameters:
 *     ctx ------------------------ input, MD5_DMA_CTX context pointer
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t md5_dma_init(MD5_DMA_CTX *ctx, HASH_CALLBACK callback)
{
    return hash_dma_init(ctx, HASH_MD5, callback);
}


/* function: dma md5 update some message blocks
 * parameters:
 *     ctx ------------------------ input, MD5_DMA_CTX context pointer
 *     msg ------------------------ input, message blocks
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of md5
 *                                  block byte length(64)
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t md5_dma_update_blocks(MD5_DMA_CTX *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
{
    return hash_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
}
#else
uint32_t md5_dma_update_blocks(MD5_DMA_CTX *ctx, uint32_t *msg, uint32_t msg_bytes)
{
    return hash_dma_update_blocks(ctx, msg, msg_bytes);
}
#endif


/* function: dma md5 final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, MD5_DMA_CTX context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message
 *     digest --------------------- output, md5 digest, 16 bytes
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t md5_dma_final(MD5_DMA_CTX *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t digest_h, uint32_t digest_l)
{
    return hash_dma_final(ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, digest_h, digest_l);
}
#else
uint32_t md5_dma_final(MD5_DMA_CTX *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest)
{
    return hash_dma_final(ctx, remainder_msg, remainder_bytes, digest);
}
#endif


/* function: dma md5 digest calculate
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, md5 digest, 16 bytes
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t md5_dma(uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l, 
        HASH_CALLBACK callback)
{
    return hash_dma(HASH_MD5, msg_h, msg_l, msg_bytes, digest_h, digest_l, callback);
}
#else
uint32_t md5_dma(uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback)
{
    return hash_dma(HASH_MD5, msg, msg_bytes, digest, callback);
}
#endif


#ifdef SUPPORT_HASH_DMA_NODE
/* function: input whole message and get its md5 digest(dma node style)
 * parameters:
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, md5 digest, 16 bytes
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 *     3. for every node or segment except the last, its message length must be a multiple of block length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t md5_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback)
{
    return hash_dma_node_steps(HASH_MD5, node, node_num, digest_h, digest_l, callback);
}
#else
uint32_t md5_dma_node_steps(const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest,
        HASH_CALLBACK callback)
{
    return hash_dma_node_steps(HASH_MD5, node, node_num, digest, callback);
}
#endif
#endif

#endif


#endif

