#include "../../crypto_include/ske/ske_xts.h"
#include "../../crypto_include/crypto_common/utility.h"


#ifdef SUPPORT_SKE_MODE_XTS



/* function: ske xts mode init config
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     xts_style ------------------ input, style to update t for IEEE or GM standards
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it can not be less than
 *                                  block byte length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. xts_style only works on SM4, and AES only supports IEEE standard regardless of the value of xts_style
 *     2. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX], 
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     3. key consists of key1 and key2
 *     4. c_bytes can not be less than block byte length
 */
uint32_t ske_xts_init(ske_xts_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *i, uint32_t c_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(c_bytes < 16U)
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(XTS_IEEE < xts_style)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        ctx->current_bytes = 0;
        ctx->c_bytes       = c_bytes;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ctx->ske_xts_ctx->crypto        = crypto;
        ret = ske_keep_alg_key_iv(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, key, sp_key_idx, i);
#else
        ske_clear_cfg();
        ske_set_cpu_mode();
        ske_set_c_len_uint32(c_bytes);

        ret = ske_init_internal(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, crypto, key, sp_key_idx, i);
#endif
    }
    
    return ret;
}


/* function: ske xts check parameter
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     bytes ---------------------- input, byte length of input or output.
 *     is_last_2_blocks ----------- input, whether is the last stage
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. bytes must be a multiple of block byte length.
 *     2. if the whole plaintext/ciphertext is too long, you could divide it into some sections 
 *        by block(16 bytes), then call this function to input the sections respectively. but 
 *        if ctx->c_bytes is not a multiple of block byte length, the input in here could not 
 *        contain the last two blocks of the whole input, in this case , the last 2 blocks(actually
 *        the last block is not full) are left to function ske_xts_update_including_last_2_blocks().
 */
static uint32_t ske_xts_check_param(const ske_xts_ctx_st *ctx, uint32_t bytes, uint32_t is_last_2_blocks)
{
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        //last stage
        if(0U != is_last_2_blocks) 
        {
            if(bytes < ctx->ske_xts_ctx->block_bytes)
            {
                //bytes length must be greater than one block
                ret = SKE_INPUT_INVALID;
            }
            else
            {}

#ifdef XTS_FOR_INTEGRITY
            //now bytes > ctx->ske_xts_ctx->block_bytes, and (bytes & 0x0F) != 0
            if(ctx->current_bytes + bytes != ctx->c_bytes)
            {
                ret = SKE_INPUT_INVALID;
            }
            else
            {}
#endif
        }
        else
        {
            //middle stage
#ifdef XTS_FOR_INTEGRITY
            if(ctx->c_bytes & 0x0F)
            {
                if((ctx->c_bytes - 16 - (ctx->c_bytes & 0x0F)) < (ctx->current_bytes + bytes))
                {
                    ret =  SKE_INPUT_INVALID;
                }
                else
                {}
            }
            else
            {
                if(ctx->c_bytes < (ctx->current_bytes + bytes))
                {
                    ret = SKE_INPUT_INVALID;
                }
                else
                {}
            }
#endif
            if(0U != (bytes & (CAST2UINT32(ctx->ske_xts_ctx->block_bytes) - 1U)))
            {
                //bytes must be a multiple of block byte length
                ret =  SKE_INPUT_INVALID;
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: ske xts mode encryption or decryption
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 *     3. if the whole plaintext/ciphertext is too long, you could divide it into some sections 
 *        by block(16 bytes), then call this function to input the sections respectively. but 
 *        if ctx->c_bytes is not a multiple of block byte length, the input in here could not 
 *        contain the last two blocks of the whole input, in this case , the last 2 blocks(actually
 *        the last block is not full) are left to function ske_xts_update_including_last_2_blocks().
 */
uint32_t ske_xts_update_blocks(ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
#ifdef XTS_FOR_INTEGRITY
    uint32_t total_bytes;
#endif
    uint32_t ret = SKE_SUCCESS;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t blocks_bytes;
    ske_payload_stage_e stage;
#endif

    if((NULL == in) || (NULL == out))
    {
        ret =  SKE_BUFFER_NULL;
    }
    else
    {
        ret = ske_xts_check_param(ctx, bytes, 0U);
    }

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
#ifdef XTS_FOR_INTEGRITY
        total_bytes = ctx->current_bytes + bytes;
        if(0 == ctx->current_bytes)
        {
            if(total_bytes == ctx->c_bytes)
            {
                stage = SKE_PAYLOAD_STREAM;
            }
            else
            {
                stage = SKE_PAYLOAD_HEAD;
            }
        }
        else
        {
            ske_set_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
            if(total_bytes == ctx->c_bytes)
            {
                stage = SKE_PAYLOAD_LAST;
            }
            else
            {
                stage = SKE_PAYLOAD_MIDDLE;
            }
        }
#else
        if(0U == ctx->current_bytes)
        {
            stage = SKE_PAYLOAD_HEAD;
        }
        else
        {
            ske_set_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
            stage = SKE_PAYLOAD_MIDDLE;
        }
#endif
        //init xts cfg 
        ske_clear_cfg();
        ske_set_cpu_mode();
        ske_set_payload_stage(stage);
        ske_set_c_len_uint32(bytes);

        ret = ske_init_internal(ctx->ske_xts_ctx, ctx->ske_xts_ctx->alg, SKE_MODE_XTS, ctx->ske_xts_ctx->crypto, 
                ctx->ske_xts_ctx->key, ctx->ske_xts_ctx->sp_key_idx, (uint8_t *)ctx->ske_xts_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
            blocks_bytes = bytes - ctx->ske_xts_ctx->block_bytes;
            ret = ske_update_blocks_internal(ctx->ske_xts_ctx, in, out, blocks_bytes);
            if(SKE_SUCCESS == ret)
            {
                ske_set_last_block(1);
                ret = ske_update_blocks_internal(ctx->ske_xts_ctx, &(in[blocks_bytes]), &(out[blocks_bytes]), 
                        ctx->ske_xts_ctx->block_bytes);
                ske_set_last_block(0);
                if(SKE_SUCCESS == ret)
                {
#ifdef XTS_FOR_INTEGRITY
                    if((SKE_PAYLOAD_HEAD == stage) || (SKE_PAYLOAD_MIDDLE == stage))
                    {
                        ske_get_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
                    }
                    else
                    {}
#else
                    ske_get_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
#endif
                }
                else
                {}
#else
                ret = ske_update_blocks_internal(ctx->ske_xts_ctx, in, out, bytes);
#endif
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            }
            else
            {}
        }
        else
        {}
#endif
        if(SKE_SUCCESS == ret)
        {
            ctx->current_bytes += bytes;
        }
        else
        {}
    }

    return ret;
}


/* function: ske xts mode encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks, actualy, this function is for the case that ctx->c_bytes % 16
 *        is not 0.
 */
static uint32_t ske_xts_update_last_data(ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t blocks_bytes;
    uint32_t tmp_buf[4];
    uint32_t ret = SKE_SUCCESS;
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t current_bytes = bytes;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ske_payload_stage_e stage;
#endif

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    if(0U == ctx->current_bytes)
    {
         //means never update any message before, use one-time style
        stage = SKE_PAYLOAD_STREAM;
    }
    else
    {
        ske_set_mid_iv(ctx->mid_iv, ctx->ske_xts_ctx->block_words);
        stage = SKE_PAYLOAD_LAST;
    }

    //init xts cfg
    ske_clear_cfg();
    ske_set_cpu_mode();
    ske_set_payload_stage(stage);
    ske_set_c_len_uint32(current_bytes);

    ret = ske_init_internal(ctx->ske_xts_ctx, ctx->ske_xts_ctx->alg, SKE_MODE_XTS, ctx->ske_xts_ctx->crypto, 
            ctx->ske_xts_ctx->key, ctx->ske_xts_ctx->sp_key_idx, (uint8_t *)ctx->ske_xts_ctx->iv);
    if(SKE_SUCCESS == ret)
    {
#endif
        //process blocks,
        blocks_bytes = (current_bytes&(~0x0FU)) - ctx->ske_xts_ctx->block_bytes;
        if(0U != blocks_bytes)
        {
            ret = ske_update_blocks_internal(ctx->ske_xts_ctx, current_in, current_out, blocks_bytes);
            if(SKE_SUCCESS == ret)
            {
                //remainder message
                current_in = &(current_in[blocks_bytes]);
                current_out = &(current_out[blocks_bytes]);
                current_bytes -= blocks_bytes;
            }
            else
            {}
        }
        else
        {}

        //process remainder 2 blocks
        memcpy_(tmp_buf, current_in, 16);
        ske_simple_set_input_block(tmp_buf, ctx->ske_xts_ctx->block_words);
        ske_start();

        ske_set_last_block(1);    //last block

        //input the last block, no need to clear the remainder buffer
        memcpy_(tmp_buf, &(current_in[16U]), current_bytes-16U);

        ske_simple_set_input_block(tmp_buf, ctx->ske_xts_ctx->block_words);

        ske_start();
        ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
        if(SKE_SUCCESS == ret)
        {
            //get the last 2 blocks output
            ske_simple_get_output_block(tmp_buf, ctx->ske_xts_ctx->block_words);
            memcpy_(current_out, tmp_buf, 16);

            ske_simple_get_output_block(tmp_buf, ctx->ske_xts_ctx->block_words);
            memcpy_(&(current_out[16U]), tmp_buf, current_bytes-16U);
        }
        else
        {}
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    }
#endif

    return ret;
}


/* function: ske xts mode encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks, actualy, this function is for the case that ctx->c_bytes % 16
 *        is not 0.
 */
uint32_t ske_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ret = SKE_SUCCESS;

    if(0U == (bytes & (CAST2UINT32(ctx->ske_xts_ctx->block_bytes) - 1U)))
    {
        ret = ske_xts_update_blocks(ctx, in, out, bytes);
    }
    else
    {
        if((NULL == in) || (NULL == out))
        {
            ret =  SKE_BUFFER_NULL;
        }
        else
        {
            ret = ske_xts_check_param(ctx, bytes, 1U);
        }

        if(SKE_SUCCESS == ret)
        {
            ret = ske_xts_update_last_data(ctx, in, out, bytes);
        }
        else
        {}
    }

    return ret;
}


/* function: ske xts mode finish
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is the last step of xts calling, and it is optional
 */
uint32_t ske_xts_final(ske_xts_ctx_st *ctx)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_(ctx, 0, sizeof(ske_xts_ctx_st));

        ret = SKE_SUCCESS;
    }

    return ret;
}


/* function: ske xts mode encrypting/decrypting
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     xts_style ------------------ input, style to update t for IEEE or GM standards
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as blcok length
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, actual byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. xts_style only works on SM4, and AES only supports IEEE standard regardless of the value of xts_style
 *     2. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX], 
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     3. key consists of key1 and key2
 *     4. c_bytes can not be less than block byte length
 */
uint32_t ske_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        const uint8_t *in, uint8_t *out, uint32_t c_bytes)
{
    ske_xts_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_xts_init(ctx, alg, crypto, xts_style, key, sp_key_idx, i, c_bytes);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_xts_update_including_last_2_blocks(ctx, in, out, c_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_xts_final(ctx);
        }
        else
        {}
    }
    else
    {}

    return ret;
}





#ifdef SKE_DMA_FUNCTION

/* function: ske xts mode dma style init config
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it can not be less than
 *                                  block byte length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX], 
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
uint32_t ske_dma_xts_init(ske_xts_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *i, uint32_t c_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(c_bytes < 16U)
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(XTS_IEEE < xts_style)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        ctx->current_bytes = 0;
        ctx->c_bytes       = c_bytes;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ctx->ske_xts_ctx->crypto        = crypto;

        ret = ske_keep_alg_key_iv(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, key, sp_key_idx, i);
#else
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
        ske_set_c_len_uint32(c_bytes);

        ret = ske_init_internal(ctx->ske_xts_ctx, alg, SKE_MODE_XTS, crypto, key, sp_key_idx, i);
#endif
    }

    return ret;
}


/* function: ske xts mode dma style encryption or decryption
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. bytes must be a multiple of block byte length.
 *     3. if the whole plaintext/ciphertext is too long, you could divide it into some sections 
 *        by block(16 bytes), then call this function to input the sections respectively. but 
 *        if ctx->c_bytes is not a multiple of block byte length, the input in here could not 
 *        contain the last two blocks of the whole input, in this case , the last 2 blocks(actually
 *        the last block is with padding 0) are left to function 
 *        ske_dma_xts_update_including_last_2_blocks().
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_xts_update_blocks(ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_xts_update_blocks(ske_xts_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback)
#endif
{
    uint32_t ret = SKE_SUCCESS;
#ifdef XTS_FOR_INTEGRITY
    uint32_t total_bytes;
#endif
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ske_payload_stage_e stage;
#endif

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if(((0U == in_h) && (0U == in_l)) || ((0U == out_h) && (0U == out_l)))
#else
    if((NULL == in) || (NULL == out))
#endif
    {
        ret =  SKE_BUFFER_NULL;
    }
    else
    {
        ret = ske_xts_check_param(ctx, bytes, 0U);
    }

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
#ifdef XTS_FOR_INTEGRITY
        if(0 == ctx->current_bytes)
        {
            if(total_bytes == ctx->c_bytes)
            {
                stage = SKE_PAYLOAD_STREAM;
            }
            else
            {
                stage = SKE_PAYLOAD_HEAD;
            }
        }
        else
        {
            ske_set_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
            if(total_bytes == ctx->c_bytes)
            {
                stage = SKE_PAYLOAD_LAST;
            }
            else
            {
                stage = SKE_PAYLOAD_MIDDLE;
            }
        }
#else
        if(0U == ctx->current_bytes)
        {
            stage = SKE_PAYLOAD_HEAD;
        }
        else
        {
            ske_set_mid_iv(ctx->mid_iv, ctx->ske_xts_ctx->block_words);
            stage = SKE_PAYLOAD_MIDDLE;
        }
#endif
        //init xts cfg
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
        ske_set_payload_stage(stage);
        ske_set_c_len_uint32(bytes);

        ret = ske_init_internal(ctx->ske_xts_ctx, ctx->ske_xts_ctx->alg, SKE_MODE_XTS, ctx->ske_xts_ctx->crypto, 
                ctx->ske_xts_ctx->key, ctx->ske_xts_ctx->sp_key_idx, (uint8_t *)ctx->ske_xts_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, bytes, bytes, callback);
#else
            ret = ske_dma_operate(in, out, bytes, bytes, callback);
#endif
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
#ifdef XTS_FOR_INTEGRITY
                if((SKE_PAYLOAD_HEAD == stage) || (SKE_PAYLOAD_MIDDLE == stage))
                {
                    ske_get_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
                }
                else
                {}
#else
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
#endif
            }
            else
            {}
        }
        else
        {}
#else
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, bytes, bytes, callback);
#else
            ret = ske_dma_operate(in, out, bytes, bytes, callback);
#endif
#endif
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        ctx->current_bytes += bytes;
    }
    else
    {}

    return ret;
}


/* function: ske xts mode dma style encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks(the last block is with padding 0), actualy, this function
 *        is for the case that ctx->c_bytes % 16 is not 0.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t ske_dma_xts_update_last_data(ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h,
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback)
#else
static uint32_t ske_dma_xts_update_last_data(ske_xts_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback)
#endif
{
    uint32_t tmp_len;
    uint8_t * current_out;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ske_payload_stage_e stage;
#endif
    uint32_t ret;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    if(0U == ctx->current_bytes)
    {
        //means never update any message before, use one-time style
        stage = SKE_PAYLOAD_STREAM;
    }
    else
    {
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, ctx->ske_xts_ctx->block_words);
        stage = SKE_PAYLOAD_LAST;
    }

    //init xts cfg
    ske_clear_cfg();
    ske_set_dma_mode();
    ske_disable_dma_linked_list();
    ske_set_payload_stage(stage);
    ske_set_c_len_uint32(bytes);

    ret = ske_init_internal(ctx->ske_xts_ctx, ctx->ske_xts_ctx->alg, SKE_MODE_XTS, ctx->ske_xts_ctx->crypto, 
            ctx->ske_xts_ctx->key, ctx->ske_xts_ctx->sp_key_idx, (uint8_t *)ctx->ske_xts_ctx->iv);
    if(SKE_SUCCESS == ret)
    {
#endif
        //input must be a multiple of block byte length
        tmp_len = (bytes+0x0FU) & (~0x0FU);
        ske_set_last_block(1);    //last block
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_operate(in_h, in_l, out_h, out_l, tmp_len, tmp_len, callback);
#else
        ret = ske_dma_operate(in, out, tmp_len, tmp_len, callback);
#endif
        ske_set_last_block(0);
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    }
#endif

    if(SKE_SUCCESS == ret)
    {
        //clear useless data
        tmp_len = bytes & 15U;
        if(0U != tmp_len)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_out = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
            current_out = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
            memset_(&(current_out[bytes]), 0U, 16U-tmp_len);

            lib_addr_arch32_unlock_remap();
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: ske xts mode dma style encryption or decryption(for the case that ctx->c_bytes % 16 is not 0)
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     2. input must contain the last 2 blocks(the last block is with padding 0), actualy, this function
 *        is for the case that ctx->c_bytes % 16 is not 0.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h,
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_xts_update_including_last_2_blocks(ske_xts_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes,
        SKE_CALLBACK callback)
#endif
{
    uint32_t ret;

    if(0U == (bytes & (CAST2UINT32(ctx->ske_xts_ctx->block_bytes) - 1U)))
    {
        //the whole message bytes is a multiple of block byte length
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_xts_update_blocks(ctx, in_h, in_l, out_h, out_l, bytes, callback);
#else
        ret = ske_dma_xts_update_blocks(ctx, in, out, bytes, callback);
#endif
    }
    else
    {
        //the whole message bytes is not a multiple of block byte length
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        if(((0U == in_h) && (0U == in_l)) || ((0U == out_h) && (0U == out_l)))
#else
        if((NULL == in) || (NULL == out))
#endif
        {
            ret =  SKE_BUFFER_NULL;
        }
        else
        {
            ret = ske_xts_check_param(ctx, bytes, 1U);
        }

        if(SKE_SUCCESS == ret)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_xts_update_last_data(ctx, in_h, in_l, out_h, out_l, bytes, callback);
#else
            ret = ske_dma_xts_update_last_data(ctx, in, out, bytes, callback);
#endif
        }
        else
        {}
    }

    return ret;
}


/* function: ske xts mode dma style finish
 * parameters:
 *     ctx ------------------------ input, ske_xts_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is the last step of xts dma style calling, and it is optional
 */
uint32_t ske_dma_xts_final(ske_xts_ctx_st *ctx)
{
    return ske_xts_final(ctx);
}


/* function: ske xts mode dma style encrypting/decrypting
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key = key1||key2
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     i -------------------------- input, i value, it has the same length as block length
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, actual byte length of input or output.
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX], 
 *        actually, sp_key_idx is reserved at present, please input key directly
 *     2. key consists of key1 and key2
 *     3. c_bytes can not be less than block byte length
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t c_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_xts_crypto(ske_alg_e alg, ske_crypto_e crypto, xts_style_e xts_style, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *i,
        uint32_t *in, uint32_t *out, uint32_t c_bytes, SKE_CALLBACK callback)
#endif
{
    ske_xts_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_dma_xts_init(ctx, alg, crypto, xts_style, key, sp_key_idx, i, c_bytes);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_xts_update_including_last_2_blocks(ctx, in_h, in_l, out_h, out_l, c_bytes, callback);
#else
        ret = ske_dma_xts_update_including_last_2_blocks(ctx, in, out, c_bytes, callback);
#endif
        if(SKE_SUCCESS == ret)
        {
            ret = ske_xts_final(ctx);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif



#endif

