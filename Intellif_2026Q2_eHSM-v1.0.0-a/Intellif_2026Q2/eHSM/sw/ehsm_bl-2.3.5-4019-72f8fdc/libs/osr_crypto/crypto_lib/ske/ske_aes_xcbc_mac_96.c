//#include <stdio.h>

#include "../../crypto_include/ske/ske_aes_xcbc_mac_96.h"
#include "../../crypto_include/crypto_common/utility.h"

#if (defined(SUPPORT_SKE_AES_XCBC_MAC_96) && defined(SUPPORT_SKE_AES_128))

#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
uint32_t ske_aes_xcbc_mac_96_recover_cfg(uint8_t dma_en, uint8_t *key, uint8_t *iv, uint16_t sp_key_idx)
{
    uint32_t ret = SKE_SUCCESS;

    ske_clear_cfg();

    if((uint8_t)0 != dma_en)
    {
        ske_set_dma_mode();
        ske_set_mode(SKE_MODE_CBC_MAC);
    }
    else
    {
        ske_set_cpu_mode();
        ske_set_mode(SKE_MODE_CBC);
    }

    ske_disable_dma_linked_list();
    ske_set_endian_uint32();
    ske_set_alg(SKE_ALG_AES_128);
    ske_set_crypto(SKE_CRYPTO_ENCRYPT);
    ske_set_last_block(0);

    ske_set_iv((uint8_t *)iv, 16);

    if(NULL != key)
    {
        ske_disable_secure_port();
        ske_set_key(SKE_ALG_AES_128, key, 16, 1);
    }
    else
    {
#ifdef SKE_SECURE_PORT_FUNCTION
        ret = lib_ske_secure_port_config(SKE_ALG_AES_128, sp_key_idx);
#endif
    }

    if(SKE_SUCCESS == ret)
    {
        ret = ske_expand_key();
    }
    else
    {}

    return ret;
}
#endif



/* function: ske get aes_xcbc_mac_96 k1, k2 and k3
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     key ------------------------ input, AES128 key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     k1 ------------------------- output, aes_xcbc_mac_96 k1
 *     k2 ------------------------- output, aes_xcbc_mac_96 k2
 *     k3 ------------------------- output, aes_xcbc_mac_96 k3
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_aes_xcbc_mac_96_get_k1_k2_k3(ske_ctx_st *ctx, uint8_t *key, uint16_t sp_key_idx, uint32_t k1[4], 
        uint32_t k2[4], uint32_t k3[4])
{
    uint32_t ret;

    ske_clear_cfg();
    ske_set_cpu_mode();
    ret = ske_init_internal(ctx, SKE_ALG_AES_128, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, NULL);
    if(SKE_SUCCESS == ret)
    {
        uint32_set(k1, 0x02020202, 4);
        ret |= ske_update_blocks_internal(ctx, (uint8_t *)k1, (uint8_t *)k2, 0x10);
        uint32_set(k1, 0x03030303, 4);
        ret |= ske_update_blocks_internal(ctx, (uint8_t *)k1, (uint8_t *)k3, 0x10);
        uint32_set(k1, 0x01010101, 4);
        ret |= ske_update_blocks_internal(ctx, (uint8_t *)k1, (uint8_t *)k1, 0x10);
    }
    else
    {}

    return ret;
}


/* function: ske aes_xcbc_mac_96 init(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_ctx_st context pointer
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_ctx_st *ctx, ske_mac_e mac_action, uint8_t *key, uint16_t sp_key_idx)
{
    uint32_t ret;
    uint32_t k1[4];

    //check and keep ctx->left_bytes = 0
    if (NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if (mac_action > SKE_VERIFY_MAC)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        ctx->left_bytes = 0;
        ctx->mac_action = mac_action;

        ret = ske_aes_xcbc_mac_96_get_k1_k2_k3(ctx->ske_xcbc_mac_ctx, key, sp_key_idx, k1, ctx->k2, ctx->k3);
        if (SKE_SUCCESS == ret)
        {
            //set iv zero
            uint32_clear((uint32_t *)ctx->block_buf, 4);

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            uint32_clear(ctx->ske_xcbc_mac_ctx->iv, 4);
            memcpy_((ctx->ske_xcbc_mac_ctx->key_buf), (uint8_t *)k1, 16);
            ctx->ske_xcbc_mac_ctx->sp_key_idx = sp_key_idx;
#endif

            ret = ske_init_internal(ctx->ske_xcbc_mac_ctx, SKE_ALG_AES_128, SKE_MODE_CBC, SKE_CRYPTO_ENCRYPT, (uint8_t *)k1, sp_key_idx, 
                    (uint8_t *)ctx->block_buf);
        }
        else
        {}
    }

    return ret;
}


/* function: ske aes_xcbc_mac_96 update message(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. msg_bytes could be any value.
 */
uint32_t ske_aes_xcbc_mac_96_update(ske_aes_xcbc_mac_96_ctx_st *ctx, uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t blocks_bytes;
    uint8_t *current_msg = msg;
    uint32_t current_msg_bytes = msg_bytes;
    uint8_t fill_bytes, remainder_bytes;
    uint8_t is_finished = (uint8_t)0;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint8_t is_update_block = (uint8_t)0;
#endif

    if (NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if (0U == msg_bytes)
    {
        ret = SKE_SUCCESS;
    }
    else if (NULL == msg)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(ctx->left_bytes > ctx->ske_xcbc_mac_ctx->block_bytes)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        if(ctx->ske_xcbc_mac_ctx->block_bytes < (msg_bytes + ctx->left_bytes))
        {
            is_update_block = 1;
            ret = ske_aes_xcbc_mac_96_recover_cfg(0, (uint8_t *)ctx->ske_xcbc_mac_ctx->key_buf, 
                    (uint8_t *)ctx->ske_xcbc_mac_ctx->iv, ctx->ske_xcbc_mac_ctx->sp_key_idx);
        }
        else
        {}

        if (SKE_SUCCESS == ret)
        {
#endif
            //if one block left, process it
            if (ctx->ske_xcbc_mac_ctx->block_bytes == ctx->left_bytes)
            {
                ret = ske_update_blocks_no_output(ctx->ske_xcbc_mac_ctx, ctx->block_buf, ctx->ske_xcbc_mac_ctx->block_bytes);
                if (SKE_SUCCESS == ret)
                {
                    ctx->left_bytes = 0;
                }
                else
                {}
            }
            else
            {}

            //padding
            if(SKE_SUCCESS == ret)
            {
                if ((uint8_t)0 != ctx->left_bytes)
                {
                    fill_bytes = ctx->ske_xcbc_mac_ctx->block_bytes - ctx->left_bytes;
                    if (current_msg_bytes <= fill_bytes)
                    {
                        memcpy_(&(ctx->block_buf[ctx->left_bytes]), current_msg, current_msg_bytes);
                        ctx->left_bytes += (uint8_t)current_msg_bytes;

                        is_finished = (uint8_t)1;
                    }
                    else
                    {
                        memcpy_(&(ctx->block_buf[ctx->left_bytes]), current_msg, fill_bytes);
                        ret = ske_update_blocks_no_output(ctx->ske_xcbc_mac_ctx, ctx->block_buf, ctx->ske_xcbc_mac_ctx->block_bytes);
                        if (SKE_SUCCESS == ret)
                        {
                            ctx->left_bytes = 0;
                            current_msg = &(current_msg[fill_bytes]);
                            current_msg_bytes -= fill_bytes;
                        }
                        else
                        {}
                    }
                }
                else
                {}
            }
            else
            {}

            //now current_msg_bytes is not 0, so blocks_bytes and remainder_bytes can not be 0 at the same time
            if(((uint8_t)0 == is_finished) && (SKE_SUCCESS == ret))
            {
                //process some blocks
                blocks_bytes = (current_msg_bytes / ctx->ske_xcbc_mac_ctx->block_bytes) * ctx->ske_xcbc_mac_ctx->block_bytes;
                remainder_bytes = (uint8_t)(current_msg_bytes % CAST2UINT32(ctx->ske_xcbc_mac_ctx->block_bytes));

                //process remainder
                if ((uint8_t)0 != remainder_bytes)
                {
                    ret = ske_update_blocks_no_output(ctx->ske_xcbc_mac_ctx, current_msg, blocks_bytes);
                    if (SKE_SUCCESS == ret)
                    {
                        memcpy_(ctx->block_buf, &(current_msg[blocks_bytes]), remainder_bytes);
                        ctx->left_bytes = remainder_bytes;
                    }
                    else
                    {}
                }
                else
                {
                    blocks_bytes -= ctx->ske_xcbc_mac_ctx->block_bytes;
                    ret = ske_update_blocks_no_output(ctx->ske_xcbc_mac_ctx, current_msg, blocks_bytes);
                    if (SKE_SUCCESS == ret)
                    {
                        memcpy_(ctx->block_buf, &(current_msg[blocks_bytes]), ctx->ske_xcbc_mac_ctx->block_bytes);
                        ctx->left_bytes = ctx->ske_xcbc_mac_ctx->block_bytes;
                    }
                    else
                    {}
                }

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
                if(((uint8_t)0 != is_update_block) && (SKE_SUCCESS == ret))
                {
                    ske_simple_get_output_block((uint32_t *)ctx->ske_xcbc_mac_ctx->iv, 4);//since CBC encrypt
                }
                else
                {}
#endif
            }
            else
            {}
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
#endif
    }

    return ret;
}



/* function: ske aes_xcbc_mac_96 finish, and get the mac or verify the mac(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_ctx_st context pointer
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_aes_xcbc_mac_96_final(ske_aes_xcbc_mac_96_ctx_st *ctx, uint8_t mac[12])
{
    uint32_t tmp[4];
    uint32_t ret;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_aes_xcbc_mac_96_recover_cfg(0, (uint8_t *)ctx->ske_xcbc_mac_ctx->key_buf, 
                (uint8_t *)ctx->ske_xcbc_mac_ctx->iv, ctx->ske_xcbc_mac_ctx->sp_key_idx);
        if (SKE_SUCCESS == ret)
        {
#endif
            if (ctx->ske_xcbc_mac_ctx->block_bytes == ctx->left_bytes)
            {
                uint32_XOR((uint32_t *)ctx->block_buf, (uint32_t *)ctx->k2, (uint32_t *)ctx->block_buf, ctx->ske_xcbc_mac_ctx->block_words);
                ret = ske_update_blocks_internal(ctx->ske_xcbc_mac_ctx, ctx->block_buf, (uint8_t *)tmp, ctx->ske_xcbc_mac_ctx->block_bytes);
            }
            else
            {
                ctx->block_buf[ctx->left_bytes] = 0x80;
                memset_(&(ctx->block_buf[ctx->left_bytes + (uint8_t)1]), 0, CAST2UINT32(ctx->ske_xcbc_mac_ctx->block_bytes)-1U-CAST2UINT32(ctx->left_bytes));
                uint32_XOR((uint32_t *)ctx->block_buf, (uint32_t *)ctx->k3, (uint32_t *)ctx->block_buf, ctx->ske_xcbc_mac_ctx->block_words);
                ret = ske_update_blocks_internal(ctx->ske_xcbc_mac_ctx, ctx->block_buf, (uint8_t *)tmp, ctx->ske_xcbc_mac_ctx->block_bytes);
            }

            if(SKE_SUCCESS == ret)
            {
                if (SKE_GENERATE_MAC == ctx->mac_action)
                {
                    memcpy_(mac, (uint8_t *)tmp, 0x0C);
                }
                else
                {
                    ret = (uint8_t)memcmp_(mac, (uint8_t *)tmp, 0x0C);
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

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
        else
        {}
#endif

#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_aes_xcbc_mac_96_ctx_st));
    }

    uint32_clear(tmp, 4);

    return ret;
}


/* function: ske xcbc_mac_96(CPU style, one-off style)
 * parameters:
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message.
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes could be any value.
 *     3. if mac_action is SKE_GENERATE_MAC, mac is output. and if mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_aes_xcbc_mac_96(ske_mac_e mac_action, uint8_t *key, uint16_t sp_key_idx, uint8_t *msg, uint32_t msg_bytes, 
        uint8_t mac[12])
{
    ske_aes_xcbc_mac_96_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_aes_xcbc_mac_96_init(ctx, mac_action, key, sp_key_idx);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_aes_xcbc_mac_96_update(ctx, msg, msg_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_aes_xcbc_mac_96_final(ctx, mac);
        }
        else
        {}
    }
    else
    {}

    return ret;
}


#ifdef SKE_DMA_FUNCTION

/* function: ske aes-xcbc-mac-96 dma style init
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_dma_ctx_st context pointer
 *     key ------------------------ input, key in bytes
 *     mid_iv --------------------- output, for dma output, occupies a block
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint8_t *key, uint32_t mid_iv_h, 
        uint32_t mid_iv_l, uint16_t sp_key_idx)
#else
uint32_t ske_dma_aes_xcbc_mac_96_init(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint8_t *key, uint32_t *mid_iv, 
        uint16_t sp_key_idx)
#endif
{
    uint32_t ret;
    uint32_t k1[4];

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if ((NULL == ctx) || ((0U == mid_iv_h) && (0U == mid_iv_l)))
#else
    if ((NULL == ctx) || (NULL == mid_iv))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ret = ske_aes_xcbc_mac_96_get_k1_k2_k3(ctx->ske_xcbc_mac_ctx, key, sp_key_idx, k1, ctx->k2, ctx->k3);
        if (SKE_SUCCESS == ret)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ctx->p_mid_iv_h = mid_iv_h;
            ctx->p_mid_iv_l = mid_iv_l;
#else
            ctx->p_mid_iv = mid_iv;
#endif

            //set iv zero
            uint32_clear((uint32_t *)ctx->mid_iv, 4);

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            memcpy_((ctx->ske_xcbc_mac_ctx->key_buf), (uint8_t *)k1, 16);
            ctx->ske_xcbc_mac_ctx->sp_key_idx = sp_key_idx;
#else
            ske_clear_cfg();
            ske_set_dma_mode();
            ske_disable_dma_linked_list();

            ret = ske_init_internal(ctx->ske_xcbc_mac_ctx, SKE_ALG_AES_128, SKE_MODE_CBC_MAC, SKE_CRYPTO_ENCRYPT, (uint8_t *)k1, 
                sp_key_idx, (uint8_t *)ctx->mid_iv);
#endif
        }
        else
        {}
    }

    return ret;
}


/* function: ske aes-xcbc-mac-96 dma style update update message blocks(excluding the last block, or the message tail)
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_dma_ctx_st context pointer
 *     msg ------------------------ input, message of some blocks, excluding last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, must be a multiple of block byte length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input msg must be some blocks, and excludes the last block(or message tail)
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_aes_xcbc_mac_96_update_blocks_excluding_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, 
        uint32_t msg_h,  uint32_t msg_l, uint32_t msg_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_aes_xcbc_mac_96_update_blocks_excluding_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, 
        uint32_t *msg, uint32_t msg_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;
    uint8_t *mid_iv;

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
    else if(0U != (msg_bytes & (CAST2UINT32(ctx->ske_xcbc_mac_ctx->block_bytes) - 1U)))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_aes_xcbc_mac_96_recover_cfg(1, (uint8_t *)ctx->ske_xcbc_mac_ctx->key_buf, (uint8_t *)ctx->mid_iv, 
                ctx->ske_xcbc_mac_ctx->sp_key_idx);
        if (SKE_SUCCESS == ret)
        {
#endif
            ske_set_last_block(1);
            ske_set_last_block_len(ctx->ske_xcbc_mac_ctx->block_bytes);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(msg_h, msg_l, ctx->p_mid_iv_h, ctx->p_mid_iv_l, msg_bytes, 
                ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#else
            ret = ske_dma_operate(msg, (uint32_t *)ctx->p_mid_iv, msg_bytes, 
                ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#endif
            if(SKE_SUCCESS == ret)
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                mid_iv = lib_addr_arch32_lock_remap(ctx->p_mid_iv_h, ctx->p_mid_iv_l, 0U);
#else
                mid_iv = lib_addr_arch32_lock_remap(0U, (uint32_t)ctx->p_mid_iv, 0U);
#endif
                memcpy_((ctx->mid_iv), mid_iv, ctx->ske_xcbc_mac_ctx->block_bytes);
                
                lib_addr_arch32_unlock_remap();
            }
            else
            {}
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
#endif
    }

    return ret;
}


/* function: ske aes-xcbc-mac-96 dma style update message including the last block(or message tail), and get the mac
 * parameters:
 *     ctx ------------------------ input, ske_aes_xcbc_mac_96_dma_ctx_st context pointer
 *     msg ------------------------ input, message including the last block(or message tail)
 *     msg_bytes ------------------ input, byte length of msg, could be 0
 *     mac ------------------------ output, occupies 12 bytes
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the whole message length is 0, this case is supported. in this case, msg occupies a block, and
 *        please set msg_bytes to 0.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_aes_xcbc_mac_96_update_including_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_aes_xcbc_mac_96_update_including_last_block(ske_aes_xcbc_mac_96_dma_ctx_st *ctx, uint32_t *msg, 
        uint32_t msg_bytes, uint32_t *mac, SKE_CALLBACK callback)
#endif
{
    uint32_t remainder_bytes;
    uint32_t block_num;
    uint32_t ret = SKE_SUCCESS;
    uint8_t *current_msg;
    uint8_t *current_mac;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == msg_h) && (0U == msg_l)) || ((0U == mac_h && (0U == mac_l))))
#else
    if((NULL == ctx) || (NULL == msg) || (NULL == mac))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        remainder_bytes = msg_bytes & 15U;
        block_num = msg_bytes / 16U;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_aes_xcbc_mac_96_recover_cfg(1, (uint8_t *)ctx->ske_xcbc_mac_ctx->key_buf, 
                (uint8_t *)ctx->mid_iv, ctx->ske_xcbc_mac_ctx->sp_key_idx);
        if (SKE_SUCCESS == ret)
        {
#endif
            ske_set_last_block(1);
            ske_set_last_block_len(ctx->ske_xcbc_mac_ctx->block_bytes);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_msg = lib_addr_arch32_lock_remap(msg_h, msg_l, 1U);
#else
            current_msg = lib_addr_arch32_lock_remap(0U, (uint32_t)msg, 1U);
#endif
            if((0U == remainder_bytes) && ((uint32_t)0 != msg_bytes))
            {
                if(0U != (block_num - 1U))
                {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                    ret = ske_dma_operate(msg_h, msg_l, ctx->p_mid_iv_h, ctx->p_mid_iv_l,
                            (block_num - 1U) * ctx->ske_xcbc_mac_ctx->block_bytes, ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#else
                    ret = ske_dma_operate(msg, (uint32_t *)ctx->p_mid_iv, 
                            (block_num - 1U) * ctx->ske_xcbc_mac_ctx->block_bytes, ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#endif
                }
                else
                {}

                if (SKE_SUCCESS == ret)
                {
                    uint8_XOR(&(current_msg[(block_num - 1U) * ctx->ske_xcbc_mac_ctx->block_bytes]), (uint8_t *)ctx->k2, ctx->mid_iv,
                            ctx->ske_xcbc_mac_ctx->block_bytes);
                }
                else
                {}
            }
            else
            {
                if(0U != block_num)
                {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                    ret = ske_dma_operate(msg_h, msg_l, ctx->p_mid_iv_h, ctx->p_mid_iv_l,
                            block_num * ctx->ske_xcbc_mac_ctx->block_bytes, ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#else
                    ret = ske_dma_operate(msg, (uint32_t *)ctx->p_mid_iv, 
                            block_num * ctx->ske_xcbc_mac_ctx->block_bytes, ctx->ske_xcbc_mac_ctx->block_bytes, callback);
#endif
                }
                else
                {}

                if (SKE_SUCCESS == ret)
                {
                    memcpy_(ctx->mid_iv, (uint8_t *)(&(current_msg[block_num * ctx->ske_xcbc_mac_ctx->block_bytes])), remainder_bytes);
                    memset_((uint8_t *)&(ctx->mid_iv[remainder_bytes]), 0x80, 1);
                    memset_((uint8_t *)&(ctx->mid_iv[remainder_bytes + 1U]), 0, CAST2UINT32(ctx->ske_xcbc_mac_ctx->block_bytes) - 1U - remainder_bytes);
                    uint32_XOR((uint32_t *)ctx->mid_iv, (uint32_t *)ctx->k3, (uint32_t *)ctx->mid_iv, ctx->ske_xcbc_mac_ctx->block_words);
                }
                else
                {}
            }

            lib_addr_arch32_unlock_remap();

            if(SKE_SUCCESS == ret)
            {
                //last block (CPU mode)
                ske_set_cpu_mode();
                ske_set_mode(SKE_MODE_CBC);
                //ske_set_iv((uint8_t *)ctx->mid_iv, 16);   //hardware holds the iv

                ret = ske_update_blocks_internal(ctx->ske_xcbc_mac_ctx, (uint8_t *)ctx->mid_iv, (uint8_t *)ctx->mid_iv, ctx->ske_xcbc_mac_ctx->block_bytes);
                if (SKE_SUCCESS == ret)
                {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                    current_mac = lib_addr_arch32_lock_remap(mac_h, mac_l, 0U);
#else
                    current_mac = lib_addr_arch32_lock_remap(0U, (uint32_t)mac, 0U);
#endif
                    memcpy_(current_mac, ctx->mid_iv, 0x0C);

                    lib_addr_arch32_unlock_remap();
                }
                else
                {}
            }
            else
            {}

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
#endif

#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_aes_xcbc_mac_96_dma_ctx_st));
    }

    return ret;
}


/* function: ske dma_aes_xcbc_mac_96(DMA style, one-off style)
 * parameters:
 *     key ------------------------ input, key in byte buffer style
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of message
 *     mac ------------------------ output, occupies a block, but actual valid output is 12 bytes
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     2. msg_bytes is actual byte length of message, it could be any value(including 0).
 *        (1). if msg_bytes is not 0, msg must have (msg_bytes+15)/16 blocks.
 *        (2). if msg_bytes is 0, msg occupies a block.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_aes_xcbc_mac_96(uint8_t *key, uint16_t sp_key_idx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_aes_xcbc_mac_96(uint8_t *key, uint16_t sp_key_idx, uint32_t *msg, uint32_t msg_bytes,
        uint32_t *mac, SKE_CALLBACK callback)
#endif
{
    ske_aes_xcbc_mac_96_dma_ctx_st ctx[1];
    uint32_t ret;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    ret = ske_dma_aes_xcbc_mac_96_init(ctx, key, mac_h, mac_l, sp_key_idx);
#else
    ret = ske_dma_aes_xcbc_mac_96_init(ctx, key, mac, sp_key_idx);
#endif
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_aes_xcbc_mac_96_update_including_last_block(ctx, msg_h, msg_l, msg_bytes, mac_h, mac_l, callback);
#else
        ret = ske_dma_aes_xcbc_mac_96_update_including_last_block(ctx, msg, msg_bytes, mac, callback);
#endif
    }
    else
    {}

    return ret;
}
#endif

#endif

