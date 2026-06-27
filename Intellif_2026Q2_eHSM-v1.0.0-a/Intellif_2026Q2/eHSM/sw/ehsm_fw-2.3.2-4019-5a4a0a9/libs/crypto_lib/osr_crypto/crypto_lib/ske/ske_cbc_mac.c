
#include "../../crypto_include/ske/ske_cbc_mac.h"
#include "../../crypto_include/crypto_common/utility.h"



#ifdef SUPPORT_SKE_MODE_CBC_MAC


/* function: ske cbc mac internal init config
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_cbc_mac_init_internal(ske_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx,
        uint8_t mac_bytes)
{
    uint32_t ret;
    uint32_t iv[4];
    uint32_t block_byte_len = ske_get_block_byte_len(alg);

    //check and keep the mac length
    if(((uint8_t)0 == mac_bytes) || (mac_bytes > block_byte_len))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //set iv zero
        uint32_clear(iv, 4);
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_keep_alg_key_iv(ctx, alg, SKE_MODE_CBC_MAC, key, sp_key_idx, (uint8_t *)iv);
#else
        ret = ske_init_internal(ctx, alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, (uint8_t *)iv);
#endif
    }

    return ret;
}


/* function: ske cbc mac init(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     padding -------------------- input, ske cbc mac padding scheme, must be SKE_NO_PADDING or SKE_ZERO_PADDING.
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_cbc_mac_init(ske_cbc_mac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, ske_padding_e padding, const uint8_t *key,
        uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ret;

    //check and keep the padding scheme and ctx->left_bytes = 0
    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((padding != SKE_NO_PADDING) && (padding != SKE_ZERO_PADDING))
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(mac_action > SKE_VERIFY_MAC)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        ctx->is_updated = 0;
        ctx->padding    = padding;
        ctx->left_bytes = 0;
        ctx->mac_action = mac_action;
        ctx->mac_bytes = mac_bytes;

#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
        ske_clear_cfg();
        ske_set_cpu_mode();
#endif
        ret = ske_cbc_mac_init_internal(ctx->ske_cbc_mac_ctx, alg, key, sp_key_idx, mac_bytes);
    }

    return ret;
}


/* function: ske cbc_mac update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes could be any value.
 */
static uint32_t ske_cbc_mac_update_internal(ske_cbc_mac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint8_t is_finished = (uint8_t)0;
    uint32_t ret = SKE_SUCCESS;
    uint32_t blocks_bytes;
    const uint8_t *current_msg = msg;
    uint32_t current_bytes = msg_bytes;
    uint8_t fill_bytes, remainder_bytes;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint8_t is_update_block = (uint8_t)0;
#endif

    ctx->is_updated = 1;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    if((ctx->left_bytes + msg_bytes) >= ctx->ske_cbc_mac_ctx->block_bytes)
    {
        is_update_block = (uint8_t)1;

        ske_clear_cfg();
        ske_set_cpu_mode();
        ret = ske_init_internal(ctx->ske_cbc_mac_ctx, ctx->ske_cbc_mac_ctx->alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cbc_mac_ctx->key, ctx->ske_cbc_mac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cbc_mac_ctx->iv);
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
#endif
        if((uint8_t)0 != ctx->left_bytes)
        {
            fill_bytes = ctx->ske_cbc_mac_ctx->block_bytes - ctx->left_bytes;
            if(current_bytes < fill_bytes)
            {
                //ctx->block_buf = ctx->block_buf || msg, and finish calc
                memcpy_((&(((uint8_t *)ctx->block_buf)[ctx->left_bytes])), msg, current_bytes);
                ctx->left_bytes += (uint8_t)current_bytes;

                is_finished = (uint8_t)1;
            }
            else
            {
                memcpy_((&(((uint8_t *)ctx->block_buf)[ctx->left_bytes])), current_msg, fill_bytes);
                ret = ske_update_blocks_no_output(ctx->ske_cbc_mac_ctx, (uint8_t *)ctx->block_buf, ctx->ske_cbc_mac_ctx->block_bytes);
                if(SKE_SUCCESS == ret)
                {
                    //calc remainder msg length
                    ctx->left_bytes = 0;
                    current_msg = &(current_msg[fill_bytes]);
                    current_bytes -= fill_bytes;
                }
                else
                {}
            }
        }
        else
        {}

        if(((uint8_t)0 == is_finished) && (SKE_SUCCESS == ret))
        {
            //update blocks
            blocks_bytes = (current_bytes/ctx->ske_cbc_mac_ctx->block_bytes)*ctx->ske_cbc_mac_ctx->block_bytes;

            ret = ske_update_blocks_no_output(ctx->ske_cbc_mac_ctx, current_msg, blocks_bytes);
            if(SKE_SUCCESS == ret)
            {
                //hold the remainder
                remainder_bytes = (uint8_t)(current_bytes % ctx->ske_cbc_mac_ctx->block_bytes);
                if((uint8_t)0 != remainder_bytes)
                {
                    memcpy_((uint8_t *)ctx->block_buf, &(current_msg[blocks_bytes]), remainder_bytes);
                    ctx->left_bytes = remainder_bytes;
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        if(0U != is_update_block)
        {
            ske_simple_get_output_block((uint32_t *)ctx->ske_cbc_mac_ctx->iv, ctx->ske_cbc_mac_ctx->block_words);
        }
        else
        {}
    }
#endif

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)

#endif

    return ret;
}


/* function: ske cbc_mac update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes could be any value.
 */
uint32_t ske_cbc_mac_update(ske_cbc_mac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(0U == msg_bytes)
    {
        ret =  SKE_SUCCESS;
    }
    else if(NULL == msg)
    {
        ret =  SKE_BUFFER_NULL;
    }
    else
    {
        ret = ske_cbc_mac_update_internal(ctx, msg, msg_bytes);
    }
    
    return ret;
}


/* function: ske cbc_mac finish, and get the mac(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_mac_ctx_st context pointer
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 *     2. for the case that padding is SKE_NO_PADDING, if the total length of message is not a multiple of
 *        block length, it will return error.
 */
uint32_t ske_cbc_mac_final(ske_cbc_mac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((uint8_t)0 == ctx->is_updated)   //no input, it is not valid
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if(SKE_SUCCESS == ret)
    {
        if((uint8_t)0 == ctx->left_bytes)
        {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            memcpy_((uint8_t *)ctx->block_buf, (uint8_t *)ctx->ske_cbc_mac_ctx->iv, ctx->ske_cbc_mac_ctx->block_bytes);
#else
            ske_simple_get_output_block(ctx->block_buf, ctx->ske_cbc_mac_ctx->block_words);
#endif
        }
        else
        {
            if(SKE_ZERO_PADDING == ctx->padding)
            {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
                ske_clear_cfg();
                ske_set_cpu_mode();
                ret = ske_init_internal(ctx->ske_cbc_mac_ctx, ctx->ske_cbc_mac_ctx->alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, 
                        ctx->ske_cbc_mac_ctx->key, ctx->ske_cbc_mac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cbc_mac_ctx->iv);
                if(SKE_SUCCESS == ret)
                {
#endif
                    memset_((&(((uint8_t *)ctx->block_buf)[ctx->left_bytes])), 0, CAST2UINT32(ctx->ske_cbc_mac_ctx->block_bytes) - CAST2UINT32(ctx->left_bytes));
                    ret = ske_update_blocks_internal(ctx->ske_cbc_mac_ctx, (uint8_t *)ctx->block_buf, 
                        (uint8_t *)ctx->block_buf, ctx->ske_cbc_mac_ctx->block_bytes);
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
                }
                else
                {}
#endif
            }
            else
            {
                ret = SKE_PADDING_ERROR;
            }
        }

        if(SKE_SUCCESS == ret)
        {
            if(SKE_GENERATE_MAC == ctx->mac_action)
            {
                memcpy_(mac, (uint8_t *)ctx->block_buf, ctx->mac_bytes);
            }
            else
            {
                ret = memcmp_(mac, (uint8_t *)ctx->block_buf, CAST2UINT32(ctx->mac_bytes));
                if(0U != ret)
                {
                    ret = SKE_VERIFY_ERROR;
                }
                else
                {}
            }
        }
        else
        {}

#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_cbc_mac_ctx_st));
    }
    
    return ret;
}


/* function: ske cbc mac(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     padding -------------------- input, ske cbc mac padding scheme, must be SKE_NO_PADDING or SKE_ZERO_PADDING.
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes can not be 0
 *     3. if mac_action is SKE_GENERATE_MAC, mac is output. and if mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 *     4. for the case that padding is SKE_NO_PADDING, if the total length of message is not a multiple of
 *        block length, it will return error.
 */
uint32_t ske_cbc_mac(ske_alg_e alg, ske_mac_e mac_action, ske_padding_e padding, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ret;
    ske_cbc_mac_ctx_st ctx[1];

    ret = ske_cbc_mac_init(ctx, alg, mac_action, padding, key, sp_key_idx, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_cbc_mac_update(ctx, msg, msg_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_cbc_mac_final(ctx, mac);
        }
        else
        {}
    }
    else
    {}

    return ret;
}


#ifdef SKE_DMA_FUNCTION
/* function: ske cbc mac init(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_dma_cbc_mac_init(ske_cbc_dma_mac_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ctx->mac_bytes = mac_bytes;

#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
#endif
        ret = ske_cbc_mac_init_internal(ctx->ske_cbc_mac_ctx, alg, key, sp_key_idx, mac_bytes);   
    }

    return ret;
}


/* function: ske cbc mac dma style update message blocks(excluding the last block, or the message tail)
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     msg ------------------------ input, message of some blocks, excluding last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, must be a multiple of block byte length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_dma_mac_ctx_st *ctx, uint32_t msg_h,
        uint32_t msg_l, uint32_t msg_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_cbc_mac_update_blocks_excluding_last_block(ske_cbc_dma_mac_ctx_st *ctx, const uint32_t *msg,
        uint32_t msg_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(0U == msg_bytes)
    {
        ret = SKE_SUCCESS;
    }
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    else if((0U == msg_h) && (0U == msg_l))
#else
    else if(NULL == msg)
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(0U != (msg_bytes & (ctx->ske_cbc_mac_ctx->block_bytes - (uint32_t)1)))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != msg_bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_set_payload_stage(SKE_PAYLOAD_MIDDLE);
        ske_set_mid_iv(ctx->ske_cbc_mac_ctx->iv, ctx->ske_cbc_mac_ctx->block_words);
        ret = ske_init_internal(ctx->ske_cbc_mac_ctx, ctx->ske_cbc_mac_ctx->alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cbc_mac_ctx->key, ctx->ske_cbc_mac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cbc_mac_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
            ske_set_last_block_len(ctx->ske_cbc_mac_ctx->block_bytes);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(msg_h, msg_l, 0, 0, msg_bytes, 0, callback);
#else
            ret = ske_dma_operate(msg, NULL, msg_bytes, 0, callback);
#endif
            if(SKE_SUCCESS == ret)
            {
                ske_get_mid_iv(ctx->ske_cbc_mac_ctx->iv, ctx->ske_cbc_mac_ctx->block_words);
            }
            else
            {}
        }
        else
        {}
#else
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_operate(msg_h, msg_l, 0, 0, msg_bytes, 0, callback);
#else
        ret = ske_dma_operate(msg, NULL, msg_bytes, 0, callback);
#endif
#endif
    }

    return ret;
}


/* function: ske cbc mac dma style update message including the last block(or message tail), and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     msg ------------------------ input, message including the last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, can not be 0
 *     mac ------------------------ output, cbc mac, occupies a block
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the actual message length msg_bytes is not a multiple of block length, please make sure the
 *        last block(or message tail) is padded with 0 already.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cbc_mac_update_including_last_block(ske_cbc_dma_mac_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_cbc_mac_update_including_last_block(ske_cbc_dma_mac_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback)
#endif
{
    uint32_t tmp_bytes;
    uint32_t remainder_bytes;
    uint32_t ret;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == msg_h) && (0U == msg_l)) || ((0U == mac_h) && (0U == mac_l)))
#else
    if((NULL == ctx) || (NULL == msg) || (NULL == mac))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(0U == msg_bytes)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_set_payload_stage(SKE_PAYLOAD_LAST);
        ske_set_mid_iv(ctx->ske_cbc_mac_ctx->iv, ctx->ske_cbc_mac_ctx->block_words);
        ret = ske_init_internal(ctx->ske_cbc_mac_ctx, ctx->ske_cbc_mac_ctx->alg, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cbc_mac_ctx->key, ctx->ske_cbc_mac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cbc_mac_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
#endif
            remainder_bytes = msg_bytes & ((uint32_t)ctx->ske_cbc_mac_ctx->block_bytes - 1U);
            if(0U == remainder_bytes)
            {
                remainder_bytes = ctx->ske_cbc_mac_ctx->block_bytes;
            }
            else
            {}

            //set the last block message length
            ske_set_last_block(1);
            ske_set_last_block_len(remainder_bytes);

            tmp_bytes = (msg_bytes + ctx->ske_cbc_mac_ctx->block_bytes - 1U)/ctx->ske_cbc_mac_ctx->block_bytes;
            tmp_bytes *= ctx->ske_cbc_mac_ctx->block_bytes;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(msg_h, msg_l, mac_h, mac_l, tmp_bytes, 
                ctx->ske_cbc_mac_ctx->block_bytes, callback);
#else
            ret = ske_dma_operate(msg, mac, tmp_bytes, ctx->ske_cbc_mac_ctx->block_bytes, callback);
#endif
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
        else
        {}
#endif
#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_cbc_dma_mac_ctx_st));
    }

    return ret;
}


/* function: ske cbc mac, dma style(DMA style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of msg
 *     mac ------------------------ output, mac, occupies a block
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes can not be 0
 *     3. if the actual message length msg_bytes is not a multiple of block length, please make sure the last block
 *        is padded with 0 already
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cbc_mac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l,
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, uint8_t mac_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_cbc_mac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;
    ske_cbc_dma_mac_ctx_st ctx[1];

    ret = ske_dma_cbc_mac_init(ctx, alg, key, sp_key_idx, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_cbc_mac_update_including_last_block(ctx, msg_h, msg_l, msg_bytes, mac_h, mac_l, callback);
#else
        ret = ske_dma_cbc_mac_update_including_last_block(ctx, msg, msg_bytes, mac, callback);
#endif
    }
    else
    {}
    
    return ret;
}
#endif


#ifdef SKE_DMA_LL_FUNCTION
/* function: ske cbc mac dma style init
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_dma_ll_cbc_mac_init(ske_cbc_dma_mac_ctx_st *ctx, ske_alg_e alg, uint8_t *key, uint16_t sp_key_idx, 
        uint8_t mac_bytes)
{
    if(NULL == ctx)
    {
        return SKE_BUFFER_NULL;
    }
    else
    {
        ctx->is_updated = 0;
    }

    ske_set_dma_mode();
    ske_enable_dma_linked_list();

    return ske_cbc_mac_init_internal((ske_cbc_mac_ctx_st *)ctx, alg, key, sp_key_idx, mac_bytes);
}


/* function: ske cbc mac dma style update message in blocks
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     in ------------------------- input, message in blocks
 *     out ------------------------ output, must has the same length as in
 *     words ---------------------- input, word length of in or out, must be a multiple of block word length
 *     llp ------------------------ input, DMA linked list node
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. here the length unit is ske block length, so please make sure input are some blocks, if the actual
 *        message length is not a multiple of block length, please make sure the last block is padded.
 */
uint32_t ske_dma_ll_cbc_mac_update_blocks(ske_cbc_dma_mac_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t words,
        dma_ll_st *llp)
{
    if((NULL == ctx) || (NULL == in) || (NULL == out))
    {
        return SKE_BUFFER_NULL;
    }
    else if(words & (ctx->ske_cbc_mac_ctx->block_words - 1))
    {
        return SKE_INPUT_INVALID;
    }
    else
    {
        ctx->is_updated = 1;
    }

    ske_dma_ll_operate(in, out, words, llp);

    while(0 != llp->next_llp)
    {
        llp = (dma_ll_st *)(llp->next_llp);
    }

    //keep the last output block
    uint32_copy(ctx->tmp_output_block, ((uint32_t *)(llp->dst_addr)) + (((llp->last_len)&0x7FFFFFFF)>>5) - ctx->ske_cbc_mac_ctx->block_words,
            ctx->ske_cbc_mac_ctx->block_words);

    return SKE_SUCCESS;
}


/* function: ske cbc mac dma ll style finish, and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cbc_dma_mac_ctx_st context pointer
 *     mac ------------------------ output, mac
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_dma_ll_cbc_mac_final(ske_cbc_dma_mac_ctx_st *ctx, uint8_t *mac)
{
    return ske_dma_cbc_mac_final(ctx, mac);
}


/* function: ske cbc mac, dma ll style(one-off)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     in ------------------------- input, message
 *     out ------------------------ just for temporary output, must have the same length as in
 *     in_words ------------------- input, word length of in and out, must be a multiple of block length.
 *     llp ------------------------ input, DMA linked list node
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. in_words must be a multiple of block length
 *     3. if the actual message length is not a multiple of block length, please make sure the last block is
 *        padded
 */
uint32_t ske_dma_ll_cbc_mac(ske_alg_e alg, uint8_t *key, uint16_t sp_key_idx, uint32_t *in, uint32_t *out,
        uint32_t in_words, dma_ll_st *llp, uint8_t *mac, uint8_t mac_bytes)
{
    uint32_t ret;
    ske_cbc_dma_mac_ctx_st ctx[1];

    ret = ske_dma_ll_cbc_mac_init(ctx, alg, key, sp_key_idx, mac_bytes);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {}

    ret = ske_dma_ll_cbc_mac_update_blocks(ctx, in, out, in_words, llp);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {}

    ret = ske_dma_ll_cbc_mac_final(ctx, mac);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {
        return SKE_SUCCESS;
    }
}
#endif
#endif


