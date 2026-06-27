
#include "../../crypto_include/ske/ske_cfb.h"

#ifdef SUPPORT_SKE_MODE_CFB

/* function: ske cfb init config(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 *     padding -------------------- input, padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
 *                                         SKE_ISO_7816_4_PADDING
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_cfb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding)
{
    return ske_init(ctx, alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, padding);
}


/* function: ske encryption or decryption(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 */
uint32_t ske_cfb_update_blocks(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    return ske_update_blocks(ctx, in, out, bytes);
}


/* function: ske cfb encryption or decryption for input including tail(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero;
 *        for decryption, in_bytes must be a multiple of block byte length.
 */
uint32_t ske_cfb_update_including_last_block(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    return ske_update_including_last_block(ctx, in, out, in_bytes, out_bytes);
}


/* function: ske cfb finish
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_cfb_final(ske_ctx_st *ctx)
{
    return ske_final(ctx);
}


/* function: ske cfb encrypting or decrypting(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 *     padding -------------------- input, padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
 *                                         SKE_ISO_7816_4_PADDING
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
uint32_t ske_cfb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes)
{
    return ske_crypto(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, padding, in, out, in_bytes, out_bytes);
}


#ifdef SKE_DMA_FUNCTION

uint32_t ske_dma_cfb_init(ske_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, ske_padding_e padding)
{
    return ske_dma_init(ctx, alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, padding);
}

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cfb_update_blocks(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_blocks(ctx, in_h, in_l, out_h, out_l, bytes, callback);
}

uint32_t ske_dma_cfb_update_including_last_block(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_including_last_block(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
}

uint32_t ske_dma_cfb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_crypto(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, padding, in_h, in_l, out_h, out_l, in_bytes,
            out_bytes, callback);
}
#else
uint32_t ske_dma_cfb_update_blocks(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes, 
        SKE_CALLBACK callback)
{
    return ske_dma_update_blocks(ctx, in, out, bytes, callback);
}

uint32_t ske_dma_cfb_update_including_last_block(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
{
    return ske_dma_update_including_last_block(ctx, in, out, in_bytes, out_bytes, callback);
}

uint32_t ske_dma_cfb_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, uint32_t *in, uint32_t *out, uint32_t in_bytes, uint32_t *out_bytes, 
        SKE_CALLBACK callback)
{
    return ske_dma_crypto(alg, SKE_MODE_CFB, crypto, key, sp_key_idx, iv, padding, in, out, in_bytes, 
            out_bytes, callback);
}
#endif

uint32_t ske_dma_cfb_final(ske_ctx_st *ctx)
{
    return ske_dma_final(ctx);
}

#endif

#endif

