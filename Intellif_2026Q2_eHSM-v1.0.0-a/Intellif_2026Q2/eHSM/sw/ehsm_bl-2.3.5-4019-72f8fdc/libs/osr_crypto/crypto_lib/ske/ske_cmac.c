#include "../../crypto_include/ske/ske_cmac.h"
#include "../../crypto_include/crypto_common/utility.h"




#ifdef SUPPORT_SKE_MODE_CMAC
/* function: ske cmac internal init config
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_st context pointer
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
uint32_t ske_cmac_init_internal(ske_ctx_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ret;
    uint32_t iv[4];
    uint32_t block_byte_len = ske_get_block_byte_len(alg);

    //check and keep the mac length
    if((0U == mac_bytes) || (mac_bytes > block_byte_len))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //set iv zero
        uint32_clear(iv, 4);
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_keep_alg_key_iv(ctx, alg, SKE_MODE_CMAC, key, sp_key_idx, (uint8_t *)iv);
#else
        ret = ske_init_internal(ctx, alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, (uint8_t *)iv);
#endif
    }
    
    return ret;
}


/* function: ske cmac init(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     mac_bytes ------------------ input, mac byte length, must be bigger than 1, and not bigger than block length
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_cmac_init(ske_cmac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx,
        uint8_t mac_bytes)
{
    uint32_t ret;

    //check and keep ctx->left_bytes = 0
    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(mac_action > SKE_VERIFY_MAC)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        ctx->left_bytes = 0;
        ctx->mac_action = mac_action;
        ctx->mac_bytes = mac_bytes;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
#else
        ske_clear_cfg();
        ske_set_cpu_mode();
#endif
        ret = ske_cmac_init_internal(ctx->ske_cmac_ctx, alg, key, sp_key_idx, mac_bytes);
    }
    
    return ret;
}


/* function: ske cmac update message internal interface(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure ctx is has been initialized
 *     2. please make sure msg is not null
 *     2. msg_bytes could be any value
 */
static uint32_t ske_cmac_update_internal(ske_cmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint8_t is_finished = (uint8_t)0;
    uint32_t blocks_bytes;
    uint8_t fill_bytes, remainder_bytes;
    const uint8_t *current_msg = msg;
    uint32_t current_bytes = msg_bytes;

    //if one block left, process it
    if(ctx->ske_cmac_ctx->block_bytes == ctx->left_bytes)
    {
        ret = ske_update_blocks_no_output(ctx->ske_cmac_ctx, (uint8_t *)ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);
        if(SKE_SUCCESS == ret)
        {
            ctx->left_bytes = 0;
        }
        else
        {}
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        //padding
        if((uint8_t)0 != ctx->left_bytes)
        {
            fill_bytes = ctx->ske_cmac_ctx->block_bytes - ctx->left_bytes;
            if(current_bytes <= fill_bytes)
            {
                memcpy_(&(((uint8_t *)ctx->block_buf)[ctx->left_bytes]), current_msg, current_bytes);
                ctx->left_bytes += (uint8_t)current_bytes;

                is_finished = (uint8_t)1;
            }
            else
            {
                memcpy_(&(((uint8_t *)ctx->block_buf)[ctx->left_bytes]), current_msg, fill_bytes);
                ret = ske_update_blocks_no_output(ctx->ske_cmac_ctx, (uint8_t *)ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);
                if(SKE_SUCCESS == ret)
                {
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

        if((SKE_SUCCESS == ret) && ((uint8_t)0 == is_finished))
        {
            //now current_bytes is not 0, so blocks_bytes and remainder_bytes can not be 0 at the same time
            //process some blocks
            blocks_bytes = (current_bytes/ctx->ske_cmac_ctx->block_bytes)*ctx->ske_cmac_ctx->block_bytes;
            remainder_bytes = (uint8_t)(current_bytes % ctx->ske_cmac_ctx->block_bytes);

            //process remainder
            if(0U != remainder_bytes)
            {
                ret = ske_update_blocks_no_output(ctx->ske_cmac_ctx, current_msg, blocks_bytes);
            }
            else
            {
                blocks_bytes -= ctx->ske_cmac_ctx->block_bytes;
                remainder_bytes = ctx->ske_cmac_ctx->block_bytes;
                ret = ske_update_blocks_no_output(ctx->ske_cmac_ctx, current_msg, blocks_bytes);
            }

            if(SKE_SUCCESS == ret)
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

    return ret;
}


/* function: ske cmac update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure ctx is has been initialized
 *     2. msg_bytes could be any value.
 */
uint32_t ske_cmac_update(ske_cmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = SKE_SUCCESS;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t current_bytes = msg_bytes;
    uint8_t is_update_block = 0;
#endif

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
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != msg_bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        //recover cbcmac hardware environment
        if((ctx->left_bytes + current_bytes) > ctx->ske_cmac_ctx->block_bytes)
        {
            is_update_block = 1;
            ske_clear_cfg();
            ske_set_cpu_mode();
            ret = ske_init_internal(ctx->ske_cmac_ctx, ctx->ske_cmac_ctx->alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, 
                    ctx->ske_cmac_ctx->key, ctx->ske_cmac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cmac_ctx->iv);
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
#endif
            ret = ske_cmac_update_internal(ctx, msg, msg_bytes);

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            //hold the mid-iv
            if(((uint8_t)0 != is_update_block) && (SKE_SUCCESS == ret))
            {
                ske_simple_get_output_block(ctx->ske_cmac_ctx->iv, ctx->ske_cmac_ctx->block_words);
            }
            else
            {}
        }
#endif
    }
    
    return ret;
}


/* function: ske cmac finish, and get the mac or verify the mac(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_ctx_st context pointer
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure ctx is has been initialized
 *     2. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_cmac_final(ske_cmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        //recover cbcmac hardware environment
        ske_clear_cfg();
        ske_set_cpu_mode();
        ret = ske_init_internal(ctx->ske_cmac_ctx, ctx->ske_cmac_ctx->alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cmac_ctx->key, ctx->ske_cmac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cmac_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
#endif
            ske_set_last_block(1);
            ske_set_last_block_len(ctx->left_bytes);

            if(ctx->ske_cmac_ctx->block_bytes == ctx->left_bytes)
            {
                ret = ske_update_blocks_internal(ctx->ske_cmac_ctx, (uint8_t *)ctx->block_buf, (uint8_t *)ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);
            }
            else
            {
                ((uint8_t *)ctx->block_buf)[ctx->left_bytes] = 0x80;
                memset_(&(((uint8_t *)ctx->block_buf)[ctx->left_bytes+(uint8_t)1]), 0, CAST2UINT32(ctx->ske_cmac_ctx->block_bytes)-1U-ctx->left_bytes);
                ret = ske_update_blocks_internal(ctx->ske_cmac_ctx, (uint8_t *)ctx->block_buf, (uint8_t *)ctx->block_buf, ctx->ske_cmac_ctx->block_bytes);
            }
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
        else
        {}
#endif
        //generate or verify mac
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
        memset_((uint8_t *)ctx, 0, sizeof(ske_cmac_ctx_st));
    }

    return ret;
}


/* function: ske cmac(CPU style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
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
 *     2. msg_bytes could be any value.
 *     3. if mac_action is SKE_GENERATE_MAC, mac is output. and if mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_cmac(ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *msg, uint32_t msg_bytes,
        uint8_t *mac, uint8_t mac_bytes)
{
    ske_cmac_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_cmac_init(ctx, alg, mac_action, key, sp_key_idx, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_cmac_update(ctx, msg, msg_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_cmac_final(ctx, mac);
        }
        else
        {}
    }
    else
    {}

    return ret;
}



#ifdef SKE_DMA_FUNCTION
/* function: ske cmac dma style init
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_st context pointer
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
uint32_t ske_dma_cmac_init(ske_cmac_dma_st *ctx, ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint8_t mac_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
#endif
        ctx->mac_bytes = mac_bytes;

        ret = ske_cmac_init_internal(ctx->ske_cmac_ctx, alg, key, sp_key_idx, mac_bytes);
    }

    return ret;
}


/* function: ske cmac dma style update update message blocks(excluding the last block, or the message tail)
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_st context pointer
 *     msg ------------------------ input, message of some blocks, excluding last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, must be a multiple of block byte length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, SKE_CALLBACK callback)
 #else
uint32_t ske_dma_cmac_update_blocks_excluding_last_block(ske_cmac_dma_st *ctx, uint32_t *msg,
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
    else if(0U != (msg_bytes & (CAST2UINT32(ctx->ske_cmac_ctx->block_bytes) - 1U)))
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
        //recover cbcmac hardware environment
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
        ske_set_payload_stage(SKE_PAYLOAD_MIDDLE);
        ske_set_mid_iv(ctx->ske_cmac_ctx->iv, ctx->ske_cmac_ctx->block_words);
        ret = ske_init_internal(ctx->ske_cmac_ctx, ctx->ske_cmac_ctx->alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cmac_ctx->key, ctx->ske_cmac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cmac_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
            ske_set_last_block_len(ctx->ske_cmac_ctx->block_bytes);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(msg_h, msg_l, 0, 0, msg_bytes, 0, callback);
#else
            ret = ske_dma_operate(msg, NULL, msg_bytes, 0, callback);
#endif
        }
        else
        {}

        //hold the mid-v
        if(SKE_SUCCESS == ret)
        {
            ske_get_mid_iv(ctx->ske_cmac_ctx->iv, ctx->ske_cmac_ctx->block_words);
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


/* function: ske cmac dma style update message including the last block(or message tail), and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_cmac_dma_st context pointer
 *     msg ------------------------ input, message including the last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, could be 0
 *     mac ------------------------ output, cmac, occupies a block
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the whole message length is 0, this case is supported. in this case, msg occupies a block, and
 *        please set msg_bytes to 0.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cmac_update_including_last_block(ske_cmac_dma_st *ctx, uint32_t msg_h, uint32_t msg_l,
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_cmac_update_including_last_block(ske_cmac_dma_st *ctx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;
    uint32_t tmp_bytes;
    uint32_t remainder_bytes;
    uint32_t current_msg_bytes = msg_bytes;
    uint8_t *current_msg;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == msg_h) && (0U == msg_l)) || ((0U == mac_h) && (0U == mac_l)))
#else
    if((NULL == ctx) || (NULL == msg) || (NULL == mac))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        //recover cbcmac hardware environment
        ske_clear_cfg();
        ske_set_dma_mode();
        ske_disable_dma_linked_list();
        ske_set_payload_stage(SKE_PAYLOAD_LAST);
        ske_set_mid_iv(ctx->ske_cmac_ctx->iv, ctx->ske_cmac_ctx->block_words);
        ret = ske_init_internal(ctx->ske_cmac_ctx, ctx->ske_cmac_ctx->alg, SKE_MODE_CMAC, SKE_CRYPTO_ENCRYPT, 
                ctx->ske_cmac_ctx->key, ctx->ske_cmac_ctx->sp_key_idx, (uint8_t *)ctx->ske_cmac_ctx->iv);
        if(SKE_SUCCESS == ret)
        {
#endif

#if 0
            //get last block length
            remainder_bytes = current_msg_bytes & (ctx->ske_cmac_ctx->block_bytes - 1);
            if(0 == current_msg_bytes)
            {
                current_msg_bytes = ctx->ske_cmac_ctx->block_bytes;
            }
            else if(0 == remainder_bytes)
            {
                remainder_bytes = ctx->ske_cmac_ctx->block_bytes;
            }
            else
            {
                //handle other
            }
#else
            //get last block length and pad
            //padded by software, not hardware, do not delete this padding action
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_msg = lib_addr_arch32_lock_remap(msg_h, msg_l, 1U);
#else
            current_msg = lib_addr_arch32_lock_remap(0U, (uint32_t)msg, 1U);
#endif
            if(0U == current_msg_bytes)
            {
                //msg = 80||00..00, occupy one block
                current_msg[0] = 0x80;
                memset_(&current_msg[1], 0, (CAST2UINT32(ctx->ske_cmac_ctx->block_bytes) - 1U));

                current_msg_bytes = 1;
                remainder_bytes = 1;
            }
            else if(0U != (current_msg_bytes & (CAST2UINT32(ctx->ske_cmac_ctx->block_bytes) - 1U)))
            {
                //msg = msg || 00..00, occupy one block
                remainder_bytes = current_msg_bytes & (CAST2UINT32(ctx->ske_cmac_ctx->block_bytes) - 1U);
                memset_(&current_msg[current_msg_bytes], 0x80, 1);
                memset_(&current_msg[current_msg_bytes + 1U], 0, CAST2UINT32(ctx->ske_cmac_ctx->block_bytes) - 1U - remainder_bytes);
            }
            else
            {
                remainder_bytes = ctx->ske_cmac_ctx->block_bytes;
            }
            
            lib_addr_arch32_unlock_remap();
#endif
            //set the last block message length
            ske_set_last_block(1);
            ske_set_last_block_len(remainder_bytes);

            tmp_bytes = (current_msg_bytes + ctx->ske_cmac_ctx->block_bytes - 1U)/ctx->ske_cmac_ctx->block_bytes;
            tmp_bytes *= ctx->ske_cmac_ctx->block_bytes;
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(msg_h, msg_l, mac_h, mac_l, tmp_bytes, ctx->ske_cmac_ctx->block_bytes, 
                callback);
#else
            ret = ske_dma_operate(msg, mac, tmp_bytes, ctx->ske_cmac_ctx->block_bytes, callback);
#endif
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
        else
        {}
#endif
#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_cmac_dma_st));
    }

    return ret;
}


/* function: ske cmac(DMA style, one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 *     mac ------------------------ output, mac
 *     mac_bytes ------------------ input, byte length of mac
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes is actual byte length of message, it could be any value(including 0).
 *        (1). if msg_bytes is not 0, msg must have (msg_bytes+15)/16 blocks, if the last block is not full,
 *        please pad with zero.
 *        (2). if msg_bytes is 0, msg occupies a block.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_cmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, uint8_t mac_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_cmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;
    ske_cmac_dma_st ctx[1];

    ret = ske_dma_cmac_init(ctx, alg, key, sp_key_idx, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_cmac_update_including_last_block(ctx, msg_h, msg_l, msg_bytes, mac_h, mac_l, callback);
#else
        ret = ske_dma_cmac_update_including_last_block(ctx, msg, msg_bytes, mac, callback);
#endif
    }
    else
    {}

    return ret;
}
#endif

#endif


