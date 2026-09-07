#include "../../crypto_include/ske/ske_gcm_gmac.h"
#include "../../crypto_include/crypto_common/utility.h"


#ifdef SUPPORT_SKE_MODE_GCM



/* function: ske gcm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     stage ---------------------- input, crypto stage
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     iv ------------------------- input, iv in bytes, must be a block
 *     is_dma_mode ---------------- input, whether dma mode flag
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU and DMA style
 *     2. only AES(128/192/256) and SM4 are supported for gcm mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. aad_bytes and c_bytes could be zero at the same time
 */
static uint32_t ske_gcm_init_internal(ske_ctx_st *ctx, ske_payload_stage_e stage, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, 
    uint16_t sp_key_idx, uint32_t aad_bytes, uint32_t c_bytes, uint32_t total_aad_bytes, uint32_t total_c_bytes, const uint8_t *iv, uint32_t is_dma_mode)
{
    uint32_t ret;

    ske_clear_cfg();

    if(0U == is_dma_mode)
    {
        ske_set_cpu_mode();
    }
    else
    {
        ske_set_dma_mode();
    }

    ske_set_payload_stage(stage);

    if((SKE_PAYLOAD_STREAM == stage) || (SKE_PAYLOAD_LAST == stage))
    {
        ske_set_aad_total_len_uint32(total_aad_bytes);
        ske_set_c_total_len_uint32(total_c_bytes);
    }
    else
    {}

    ske_set_aad_len_uint32(aad_bytes);
    ske_set_c_len_uint32(c_bytes);

    //init gcm cfg
    ret = ske_init_internal(ctx, alg, SKE_MODE_GCM, crypto, key, sp_key_idx, iv);

    return ret;
}


/* function: ske gcm mode init config(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes is supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     mac_bytes ------------------ input, byte length of mac, must be in [0,16]
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could be zero at the same time
 */
uint32_t ske_gcm_init(ske_gcm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes, uint8_t mac_bytes)
{
    uint8_t tmp[16];
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == iv))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(12U != iv_bytes)
    {
        ret = SKE_INPUT_INVALID;
    }
    else if (mac_bytes > SKE_GCM_MAX_BYTES)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if(SKE_SUCCESS == ret)
    {
        memcpy_(tmp, iv, iv_bytes);
        memset_(&tmp[iv_bytes], 0, 16U-iv_bytes);

        ctx->aad_bytes             = aad_bytes;
        ctx->c_bytes               = c_bytes;
        ctx->mac_bytes             = mac_bytes;
        ctx->current_bytes         = 0;
        ctx->ske_gcm_ctx->crypto   = crypto;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        if((0U == ctx->aad_bytes) && (0U == ctx->c_bytes))
        {
            //if aad and payload are null, use cpu mode to calc mac
            ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_STREAM, alg, crypto, key, sp_key_idx, 0U, 0U, 0U, 0U, tmp, 0U);
            if(SKE_SUCCESS == ret)
            {
                ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
                if(SKE_SUCCESS == ret)
                {
                    ske_simple_get_output_block(ctx->buf, 4);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {
            ret = ske_keep_alg_key_iv(ctx->ske_gcm_ctx, alg, SKE_MODE_GCM, key, sp_key_idx, tmp);
        }
#else
        ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_STREAM, alg, crypto, key, 
            sp_key_idx, aad_bytes, c_bytes, aad_bytes, c_bytes, tmp, 0U);       
        if(SKE_SUCCESS == ret)
        {
            if((0U == ctx->aad_bytes) && (0U == ctx->c_bytes))
            {
                ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
                if(SKE_SUCCESS == ret)
                {
                    ske_simple_get_output_block(ctx->buf, 4);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
#endif
    }

    return ret;
}


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
/* function: ske gcm mode input aad(stepwise style)
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     bytes ---------------------- input, real byte length of aad
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_gcm_init()
 *     2. if there is no aad, this function could be omitted
 *     3. please make sure all parameter are valid
 */
static uint32_t ske_gcm_update_blocks_aad_init(ske_gcm_ctx_st *ctx,  uint32_t bytes)
{
    uint32_t ret;
    ske_payload_stage_e stage;

    if(0U == ctx->current_bytes)
    {
        if((bytes == ctx->aad_bytes) && (0U == ctx->c_bytes))
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
        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);

        if((bytes == ctx->aad_bytes) && (0U == ctx->c_bytes))
        {
            stage = SKE_PAYLOAD_LAST;
        }
        else
        {
            stage = SKE_PAYLOAD_MIDDLE;
        }
    }

    //init gcm cfg
    ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, stage, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
        ctx->ske_gcm_ctx->sp_key_idx, bytes, 0U, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 0U);
    
    return ret;
}
#endif


/* function: ske gcm mode input aad(stepwise style)
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     aad ------------------------ input, aad
 *     bytes ---------------------- input, real byte length of aad
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_gcm_init()
 *     2. if there is no aad, this function could be omitted
 *     3. if the whole aad is too long, you could divide it into some sections by block, then call
 *        this function to input the sections respectively. for example, if the whole aad byte
 *        length is 65, it could be divided into 3 sections with byte length 16,32,17 respectively.
 */
static uint32_t ske_gcm_update_blocks_aad_internal(ske_gcm_ctx_st *ctx, const uint8_t *aad, uint32_t bytes)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t total_bytes = ctx->current_bytes + bytes;
    uint32_t ret;

    if (total_bytes == ctx->aad_bytes)
    {
        blocks_bytes = (bytes)&(~0x0FU); //here bytes must not be 0
        remainder_bytes = (bytes)&0x0FU;
        if(0U == remainder_bytes)
        {
            blocks_bytes -= 16U; //not overflow
            remainder_bytes = 16U;
        }
        else
        {}

        //update blocks data
        ret = ske_update_blocks_no_output(ctx->ske_gcm_ctx, aad, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            //update the last block
            uint32_clear(&ctx->buf[remainder_bytes/4U], 4U-(remainder_bytes/4U));
            memcpy_(ctx->buf, &(aad[blocks_bytes]), remainder_bytes);

            ske_set_last_block(1);
            ret = ske_update_blocks_no_output(ctx->ske_gcm_ctx, (uint8_t *)ctx->buf, 16);
            ske_set_last_block(0);
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
            ctx->current_bytes = 0;

            if(ctx->c_bytes == 0U)
            {
                //ready get mac
                ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
                if(SKE_SUCCESS == ret)
                {
                    //get mac
                    ske_simple_get_output_block(ctx->buf, ctx->ske_gcm_ctx->block_words);
                }
                else
                {}
            }
            else
            {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
                ske_get_mid_mac((uint32_t *)ctx->mid_mac, 4, 0);
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, 4);
#endif
            }
        }
        else
        {}
    }
    else
    {
        //update non last data and get mid value
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        blocks_bytes = bytes - ctx->ske_gcm_ctx->block_bytes;
        ret = ske_update_blocks_no_output(ctx->ske_gcm_ctx, aad, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
            ret = ske_update_blocks_no_output(ctx->ske_gcm_ctx, &(aad[blocks_bytes]), ctx->ske_gcm_ctx->block_bytes);
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
                ske_get_mid_mac((uint32_t *)ctx->mid_mac, 4, 0);
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, 4);
            }
            else
            {}
        }
        else
        {}
#else
        ret = ske_update_blocks_no_output(ctx->ske_gcm_ctx, aad, bytes);
#endif

        if(SKE_SUCCESS == ret)
        {
            ctx->current_bytes = total_bytes;
        }
        else
        {}
    }

    return ret;
}


/* function: ske gcm mode input aad(stepwise style)
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     aad ------------------------ input, aad
 *     bytes ---------------------- input, real byte length of aad
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_gcm_init()
 *     2. if there is no aad, this function could be omitted
 *     3. if the whole aad is too long, you could divide it into some sections by block, then call
 *        this function to input the sections respectively. for example, if the whole aad byte
 *        length is 65, it could be divided into 3 sections with byte length 16,32,17 respectively.
 */
uint32_t ske_gcm_update_blocks_aad(ske_gcm_ctx_st *ctx, const uint8_t *aad, uint32_t bytes)
{
    uint32_t total_bytes;
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((NULL == aad) && (0U != bytes))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        total_bytes = ctx->current_bytes + bytes;
        if ((total_bytes < bytes) || (total_bytes > ctx->aad_bytes))  // overflow
        {
            ret = SKE_INPUT_INVALID;
        }
        else if((total_bytes != ctx->aad_bytes) && (0U != (bytes & 0xFU))) //not last data and not aligned 16 bytes
        {
            ret = SKE_INPUT_INVALID;
        }
        else
        {
            //handle other
        }
    }

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_gcm_update_blocks_aad_init(ctx,  total_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_gcm_update_blocks_aad_internal(ctx, aad, bytes);
        }
#else
        ret = ske_gcm_update_blocks_aad_internal(ctx, aad, bytes);
#endif
    }

    return ret;
}


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
/* function: ske gcm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     bytes ---------------------- input, current total byte length of input or output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. please make sure all parameter are valid
 */
static uint32_t ske_gcm_update_blocks_init(ske_gcm_ctx_st *ctx, uint32_t bytes)
{
    uint32_t ret;
    ske_payload_stage_e stage;

    if(0U == ctx->current_bytes)
    {
        if(0U == ctx->aad_bytes)
        {
            if(bytes == ctx->c_bytes)
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
            if(bytes == ctx->c_bytes)
            {
                stage = SKE_PAYLOAD_LAST;
            }
            else
            {
                stage = SKE_PAYLOAD_MIDDLE;
            }
        }
    }
    else
    {
        if(bytes == ctx->c_bytes)
        {
            stage = SKE_PAYLOAD_LAST;
        }
        else
        {
            stage = SKE_PAYLOAD_MIDDLE;
        }
    }

    if((SKE_PAYLOAD_MIDDLE == stage) || (SKE_PAYLOAD_LAST == stage))
    {
        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);
    }
    else
    {}

    //init gcm cfg
    ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, stage, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
            ctx->ske_gcm_ctx->sp_key_idx, 0U, bytes, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 0U);
    
    return ret;
}
#endif


/* function: ske gcm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     4. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it 
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
static uint32_t ske_gcm_update_blocks_internal(ske_gcm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t total_bytes = ctx->current_bytes + bytes;
    uint32_t ret;

    if (total_bytes == ctx->c_bytes)
    {
        blocks_bytes = (bytes)&(~0x0FU); //here bytes must not be 0
        remainder_bytes = (bytes)&(0x0FU);
        if(0U == remainder_bytes)
        {
            blocks_bytes -= 16U; //not overflow
            remainder_bytes = 16U;
        }
        else
        {}

        ret = ske_update_blocks_internal(ctx->ske_gcm_ctx, in, out, blocks_bytes);
        //get mid
        if(SKE_SUCCESS == ret)
        {
            //the last block
            memcpy_(ctx->buf, &(in[blocks_bytes]), remainder_bytes);
            memset_(&(((uint8_t *)ctx->buf)[remainder_bytes]), 0, 16U-remainder_bytes);

            ske_set_last_block(1);
            ret = ske_update_blocks_internal(ctx->ske_gcm_ctx, (uint8_t *)ctx->buf, (uint8_t *)ctx->buf, 16);
            ske_set_last_block(0);

            if(SKE_SUCCESS == ret)
            {
                memcpy_(&(out[blocks_bytes]), ctx->buf, remainder_bytes);
                //ready get mac
                ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
            }
            else
            {}

            if(SKE_SUCCESS == ret)
            {
                //get mac
                ske_simple_get_output_block(ctx->buf, ctx->ske_gcm_ctx->block_words);
            }
            else
            {}
        }
        else
        {}
    }
    else
    {
        //update non last data and get mid value
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        blocks_bytes = bytes - ctx->ske_gcm_ctx->block_bytes;
        ret = ske_update_blocks_internal(ctx->ske_gcm_ctx, in, out, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
            ret = ske_update_blocks_internal(ctx->ske_gcm_ctx, &(in[blocks_bytes]), &(out[blocks_bytes]), ctx->ske_gcm_ctx->block_bytes);
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
                ske_get_mid_mac((uint32_t *)ctx->mid_mac, 4, 0);
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, 4);
            }
            else
            {}
        }
        else
        {}
#else
        ret = ske_update_blocks_internal(ctx->ske_gcm_ctx, in, out, bytes);
#endif
    }

    if(SKE_SUCCESS == ret)
    {
        ctx->current_bytes = total_bytes;
    }
    else
    {}
        
    return ret;
}


/* function: ske gcm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext, this function could be omitted
 *     3. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     4. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it 
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
uint32_t ske_gcm_update_blocks(ske_gcm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t total_bytes;
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(((NULL == in) || (NULL == out)) && (0U != bytes))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        //handle other
    }

    if(SKE_SUCCESS == ret)
    {
        total_bytes = ctx->current_bytes + bytes;
        if ((total_bytes < bytes) || (total_bytes > ctx->c_bytes))  // overflow
        {
            ret = SKE_INPUT_INVALID;
        }
        else if((total_bytes != ctx->c_bytes) && (0U != (bytes & 0xFU))) //not last data and not aligned 16 bytes
        {
            ret = SKE_INPUT_INVALID;
        }
        else
        {
            //handle other
        }
    }
    else
    {}

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret =ske_gcm_update_blocks_init(ctx, total_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_gcm_update_blocks_internal(ctx, in, out, bytes);
        }
#else
        ret = ske_gcm_update_blocks_internal(ctx, in, out, bytes);
#endif
    }
    else
    {}

    return ret;
}


/* function: ske gcm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     mac ------------------------ input(for decryption), output(for encryption)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_gcm_final(ske_gcm_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        if(SKE_CRYPTO_ENCRYPT == ctx->ske_gcm_ctx->crypto)
        {
            memcpy_(mac, ctx->buf, ctx->mac_bytes);
        }
        else
        {
            ret = memcmp_(mac, ctx->buf, ctx->mac_bytes);
            if(0U != ret)
            {
                ret = SKE_VERIFY_ERROR;
            }
            else
            {}
        }

#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
        ske_clear();
#endif
        memset_((uint8_t *)ctx, 0, sizeof(ske_gcm_ctx_st));
    }

    return ret;
}


/* function: ske gcm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad ------------------------ input, aad, please make sure aad here is integral
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value,
 *                                  including 0
 *     mac ------------------------ input(for decryption), output(for encryption)
 *     mac_bytes ------------------ input, byte length of mac.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     7. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     8. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out, uint32_t c_bytes,
        uint8_t *mac, uint8_t mac_bytes)
{
    ske_gcm_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_gcm_init(ctx, alg, crypto, key, sp_key_idx, iv, iv_bytes, aad_bytes, c_bytes, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_gcm_update_blocks_aad(ctx, aad, aad_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_gcm_update_blocks(ctx, in, out, c_bytes);
            if(SKE_SUCCESS == ret)
            {
                ret = ske_gcm_final(ctx, mac);
            }
            else
            {}
        }
        else
        {}
    }
    else
    {}

    return ret;
}




#ifdef SUPPORT_SKE_MODE_GMAC
/* function: ske gmac mode init config(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_gmac_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     mac_bytes ------------------ input, byte length of mac, must be in [1,16]
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GMAC mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. msg_bytes could be zero
 */
uint32_t ske_gmac_init(ske_gmac_ctx_st *ctx, ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint8_t mac_bytes)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ctx->mac_action = mac_action;
        ctx->left_bytes = (uint8_t)0;

        ret = ske_gcm_init(ctx->ske_gcm_ctx, alg, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, iv, iv_bytes, 0xFFFFFFFF, 0, mac_bytes);
    }

    return ret;
}


/* function: ske gmac mode input msg internal(stepwise style)
 * parameters:
 *     ctx ------------------------ input, ske_gmac_ctx_st context pointer
 *     msg ------------------------ input, msg
 *     bytes ---------------------- input, real byte length of msg
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
static uint32_t ske_gmac_update_internal(ske_gmac_ctx_st *ctx, const uint8_t *msg, uint32_t bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t blocks_bytes, current_bytes = bytes;;
    uint8_t fill_bytes, remainder_bytes;
    const uint8_t *current_msg = msg;
    uint8_t is_finished = (uint8_t)0;

    //if one block left, process it
    if(16U == ctx->left_bytes)
    {
        ret = ske_gcm_update_blocks_aad(ctx->ske_gcm_ctx, (uint8_t *)ctx->block_buf, 16U);
        if(SKE_SUCCESS == ret)
        {
            ctx->left_bytes = 0;
        }
        else
        {}
    }
    else
    {}

    if((SKE_SUCCESS == ret) && ((uint8_t)0 != ctx->left_bytes))
    {
        //padding
        fill_bytes = 16U - ctx->left_bytes;
        if(current_bytes <= fill_bytes)
        {
            memcpy_(&(((uint8_t *)ctx->block_buf)[ctx->left_bytes]), current_msg, current_bytes);
            ctx->left_bytes += (uint8_t)current_bytes;

            is_finished = (uint8_t)1;
        }
        else
        {
            memcpy_(&(((uint8_t *)ctx->block_buf)[ctx->left_bytes]), current_msg, fill_bytes);
            ret = ske_gcm_update_blocks_aad(ctx->ske_gcm_ctx, (uint8_t *)ctx->block_buf, 16U);
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

    if((SKE_SUCCESS == ret) && ((uint8_t)0 == is_finished))
    {
        //now current_bytes is not 0, so blocks_bytes and remainder_bytes can not be 0 at the same time
        //process some blocks
        blocks_bytes = (current_bytes/16U)*16U;
        remainder_bytes = (uint8_t)(current_bytes & 15U);

        //process remainder
        if(0U == remainder_bytes)
        {
            blocks_bytes -= 16U;
            remainder_bytes = 16U;
        }
        else
        {}

        ret = ske_gcm_update_blocks_aad(ctx->ske_gcm_ctx, current_msg, blocks_bytes);

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

    return ret;
}


/* function: ske gmac mode input msg(stepwise style)
 * parameters:
 *     ctx ------------------------ input, ske_gmac_ctx_st context pointer
 *     msg ------------------------ input, msg
 *     bytes ---------------------- input, real byte length of msg
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_gmac_init()
 *     2. if there is no msg, this function could be omitted
 *     3. msg_bytes could be any value
 */
uint32_t ske_gmac_update(ske_gmac_ctx_st *ctx, const uint8_t *msg, uint32_t bytes)
{
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(0U != bytes)
    {
        ret = ske_gmac_update_internal(ctx, msg, bytes);
    }
    else
    {
        //hanle other
    }

    return ret;
}


/* function: ske gmac mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gmac_ctx_st context pointer
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes must be in [1, SKE_GCM_MAX_BYTES]
 *     3. if ctx->mac_action is SKE_GENERATE_MAC, mac is output. and if ctx->mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_gmac_final(ske_gmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ctx->ske_gcm_ctx->aad_bytes = ctx->ske_gcm_ctx->current_bytes + ctx->left_bytes;
        if(0U == ctx->ske_gcm_ctx->aad_bytes)
        {
#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
            ret = ske_gcm_init(ctx->ske_gcm_ctx, ctx->ske_gcm_ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->ske_gcm_ctx->key, ctx->ske_gcm_ctx->ske_gcm_ctx->sp_key_idx,
                (uint8_t *)ctx->ske_gcm_ctx->ske_gcm_ctx->iv, 12U, 0U, 0U, ctx->ske_gcm_ctx->mac_bytes);
#else
            ret = ske_gmac_get_mac_of_msg_empty(ctx->ske_gcm_ctx->buf);
#endif
        }
        else
        {
#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
            ske_gmac_set_cfg_of_msg_not_empty(ctx->ske_gcm_ctx->aad_bytes);
#endif
            ret = ske_gcm_update_blocks_aad(ctx->ske_gcm_ctx, (uint8_t *)ctx->block_buf, ctx->left_bytes);
        }

        if(SKE_SUCCESS == ret)
        {
            if(SKE_GENERATE_MAC == ctx->mac_action)
            {
                memcpy_(mac, ctx->ske_gcm_ctx->buf, ctx->ske_gcm_ctx->mac_bytes);
            }
            else
            {
                ret = memcmp_(mac, ctx->ske_gcm_ctx->buf, CAST2UINT32(ctx->ske_gcm_ctx->mac_bytes));
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
        memset_((uint8_t *)ctx, 0, sizeof(ske_gmac_ctx_st));
    }

    return ret;
}


/* function: ske gmac mode(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     mac_action ----------------- input, must be SKE_GENERATE_MAC or SKE_VERIFY_MAC
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     msg ------------------------ input, msg, please make sure msg here is integral
 *     msg_bytes ------------------ input, byte length of msg, it could be any value, including 0
 *     mac ------------------------ output(for generating mac), input(for verifying mac)
 *     mac_bytes ------------------ input, byte length of mac.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. msg_bytes could be zero
 *     6. mac_bytes must be in [1, SKE_GCM_MAX_BYTES]
 *     7. if mac_action is SKE_GENERATE_MAC, mac is output. and if mac_action is SKE_VERIFY_MAC,
 *        mac is input, return value SKE_SUCCESS means the mac is valid, otherwise mac is invalid.
 */
uint32_t ske_gmac(ske_alg_e alg, ske_mac_e mac_action, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, const uint8_t *msg, uint32_t msg_bytes, uint8_t *mac, uint8_t mac_bytes)
{
    ske_gmac_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_gmac_init(ctx, alg, mac_action, key, sp_key_idx, iv, iv_bytes, mac_bytes);
    if(SKE_SUCCESS == ret)
    {
        //for gmac, message is aad in gcm
        ret = ske_gmac_update(ctx, msg, msg_bytes);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_gmac_final(ctx, mac);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif



#ifdef SKE_DMA_FUNCTION
/* function: ske dma gcm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes is supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     mac_bytes ------------------ input, byte length of mac, must be in [0,16]
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could be zero at the same time
 */
uint32_t ske_dma_gcm_init(ske_gcm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *iv, uint32_t iv_bytes, uint32_t aad_bytes, uint32_t c_bytes, uint8_t mac_bytes)
{
    uint32_t ret;
    uint8_t tmp[16];

    if((NULL == ctx) || (NULL == iv))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if(12U != iv_bytes)
    {
        ret = SKE_INPUT_INVALID;
    }
    else if ((aad_bytes > 0x1fffffffU) || (c_bytes > 0x1fffffffU))
    {
        ret = SKE_INPUT_INVALID;
    }
    else if (mac_bytes > SKE_GCM_MAX_BYTES)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        memcpy_(tmp, iv, iv_bytes);
        memset_(&tmp[iv_bytes], 0, 16U-iv_bytes);

        ctx->aad_bytes             = aad_bytes;
        ctx->c_bytes               = c_bytes;
        ctx->mac_bytes             = mac_bytes;
        ctx->current_bytes         = 0;
        ctx->ske_gcm_ctx->crypto   = crypto;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_keep_alg_key_iv(ctx->ske_gcm_ctx, alg, SKE_MODE_GCM, key, sp_key_idx, (uint8_t *)tmp);
#else
        ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_STREAM, alg, crypto, key, 
            sp_key_idx, aad_bytes, c_bytes, aad_bytes, c_bytes, tmp, 1U);    
#endif
    }

    return ret;
}


#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
/* function: ske dma gcm mode input whole aad
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     aad ------------------------ input, AAD, here AAD is whole aad, and may with padding 0
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. here AAD is whole aad, and may padded some zeros to make AAD length a multiple of block length
 *     2. if there is no aad, this function could be omitted
 *     3. this function must be called after calling ske_dma_gcm_init()
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_blocks_whole_aad(ske_gcm_ctx_st *ctx, uint32_t aad_h, uint32_t aad_l, 
        SKE_CALLBACK callback)
#else
uint32_t ske_dma_gcm_update_blocks_whole_aad(ske_gcm_ctx_st *ctx, uint32_t *aad, SKE_CALLBACK callback)
#endif
{
    uint32_t aad_bytes;
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    else if((0U == aad_h) && (0U == aad_l))
#else
    else if(NULL == aad)
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != ctx->aad_bytes))
    {
        aad_bytes = (ctx->aad_bytes + 15U)&(~0x0FU);

        //init gcm cfg
        ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_HEAD, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
            ctx->ske_gcm_ctx->sp_key_idx, aad_bytes, 0U, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 1U);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret =  ske_dma_operate(aad_h, aad_l, 0, 0, aad_bytes, 0, callback);
#else
            ret =  ske_dma_operate(aad, NULL, aad_bytes, 0, callback);
#endif
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
                ske_get_mid_mac((uint32_t *)ctx->mid_mac, 4, 0);
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, 4);
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* function: ske dma gcm mode update some plaintext/ciphertext blocks
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     in ------------------------- input, plaintext/ciphertext of some blocks
 *     in_bytes ------------------- input, byte length of in/out
 *     out ------------------------ input, ciphertext/plaintext of some blocks
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling 
 *        ske_dma_gcm_init(), if aad is empty
 *     or ske_dma_gcm_update_blocks_whole_aad(), if aad is not empty
 *     2. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it 
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_blocks(ske_gcm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes, 
        uint32_t out_h, uint32_t out_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_gcm_update_blocks(ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, 
        SKE_CALLBACK callback)
#endif
{
    uint32_t total_bytes;
    uint32_t dma_in_bytes, remainder_bytes;
    uint32_t ret = SKE_SUCCESS;
    ske_payload_stage_e stage;
    uint8_t *out_remap;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    else if(((0U == in_h) && (0U == in_l)) || ((0U == out_h) && (0U == out_l)))
#else
    else if((NULL == in) || (NULL == out))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        total_bytes = ctx->current_bytes + in_bytes;
        if ((total_bytes < in_bytes) || (total_bytes > ctx->c_bytes))  // overflow
        {
            ret = SKE_INPUT_INVALID;
        }
        else if((total_bytes != ctx->c_bytes) && (0U != (in_bytes & 0xFU))) //not last data and not aligned 16 bytes
        {
            ret = SKE_INPUT_INVALID;
        }
        else
        {
            //handle other
        }
    }

    if((SKE_SUCCESS == ret) && (0U != in_bytes))
    {
        remainder_bytes = in_bytes & (0x0FU);
        dma_in_bytes = (in_bytes +15U) & (~0x0FU);

        if((0U == ctx->aad_bytes) && (0U == ctx->current_bytes))    //here ctx->current_bytes is just a flag
        {
            ctx->current_bytes = in_bytes;
            stage = SKE_PAYLOAD_HEAD;
        }
        else
        {
            stage = SKE_PAYLOAD_MIDDLE;
            ctx->current_bytes += in_bytes;
            ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
            ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);
        }

        //init gcm cfg
        ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, stage, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
            ctx->ske_gcm_ctx->sp_key_idx, 0, in_bytes, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 1U);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, dma_in_bytes, dma_in_bytes, callback);
#else
            ret = ske_dma_operate(in, out, dma_in_bytes, dma_in_bytes, callback);
#endif
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
                //since hardware does not clear the remainder part
                if(0U != remainder_bytes)
                {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                    out_remap = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
                    out_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
                    memset_(&(out_remap[dma_in_bytes-16U+remainder_bytes]), 0, 16U-remainder_bytes);

                    lib_addr_arch32_unlock_remap();
                }
                else
                {}

                ske_get_mid_mac((uint32_t *)ctx->mid_mac, 4, 0);
                ske_get_mid_iv((uint32_t *)ctx->mid_iv, 4);
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


#if 0
/* function: ske dma gcm mode update plaintext/ciphertext with the last block(or the tail part)
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     in ------------------------- input, plaintext/ciphertext with the last block(or the tail part)
 *     in_bytes ------------------- input, byte length of in/out
 *     out ------------------------ input, ciphertext/plaintext with the last block(or the tail part) + mac
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling 
 *        ske_dma_gcm_init(), if aad is empty and in begins with plaintext/ciphertext head
 *     or ske_dma_gcm_update_blocks_whole_aad(), if aad is not empty and in begins with plaintext/ciphertext head
 *     or ske_dma_gcm_update_blocks_excluding_last_block(), if in does not begin with plaintext/ciphertext head
 *     2. the mac is behind the output ciphertext/plaintext tail
 */
uint32_t ske_dma_gcm_update_blocks_including_last_block(ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, 
        uint32_t *out, SKE_CALLBACK callback)
{
    uint32_t ret;
    ske_payload_stage_e stage;

    if(NULL == ctx)
    {
        return SKE_BUFFER_NULL;
    }
    else if(0 == in_bytes)
    {
        return SKE_INPUT_INVALID;
    }
    if((NULL == in) || (NULL == out))
    {
        return SKE_BUFFER_NULL;
    }
    else
    {}

    if((0 == ctx->aad_bytes) && (0 == ctx->current_bytes))
    {
        stage = SKE_PAYLOAD_STREAM;
    }
    else
    {
        stage = SKE_PAYLOAD_LAST;
        ske_set_mid_mac(ctx->mid_mac, 4);
        ske_set_mid_iv(ctx->mid_iv, 4);
    }

    ske_clear_cfg();
    ske_set_dma_mode();
    ske_set_payload_stage(stage);
    ske_set_aad_len_uint32(0);
    ske_set_c_len_uint32(in_bytes);
    ske_set_aad_total_len_uint32(ctx->aad_bytes);
    ske_set_c_total_len_uint32(ctx->c_bytes);

    ret = ske_init_internal(ctx->ske_gcm_ctx, ctx->ske_gcm_ctx->alg, SKE_MODE_GCM, ctx->crypto, 
            ctx->ske_gcm_ctx->key, ctx->ske_gcm_ctx->sp_key_idx, (uint8_t *)ctx->ske_gcm_ctx->iv);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {}

    in_bytes = (in_bytes + 15)&(~0x0FU);

    ske_set_last_block(1);
    ret = ske_dma_operate(in, out, in_bytes, in_bytes + 16, callback);
    ske_set_last_block(0);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        if(ctx->c_bytes & 0x0F)
        {
            ske_clear_block_tail(out + (in_bytes>>2) - 4, (ctx->c_bytes & 0x0F));
        }
        else
        {}

        if(16 != ctx->mac_bytes)
        {
            ske_clear_block_tail(out + (in_bytes>>2), ctx->mac_bytes);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif

/* function: ske dma gcm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     mac ------------------------ input(for decryption), output(for encryption)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. mac_bytes could be 0, but not bigger than SKE_GCM_MAX_BYTES
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_final(ske_gcm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l)
#else
uint32_t ske_dma_gcm_update_final(ske_gcm_ctx_st *ctx, uint8_t *mac)
#endif
{
    uint32_t ret;
    uint32_t tmp[4];
    uint8_t *current_mac;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == mac_h) && (0U == mac_l)))
#else
    if((NULL == ctx) || (NULL == mac))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        if((0U == ctx->aad_bytes) && (0U == ctx->c_bytes))
        {
            uint32_clear(ctx->mid_mac, 4);
            ske_set_mid_iv((uint32_t *)ctx->ske_gcm_ctx->iv, 4);
        }
        else
        {
            ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);
        }

        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);

        //init gcm cfg
        ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_LAST, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
            ctx->ske_gcm_ctx->sp_key_idx, 0, 0, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 0U);
        if(SKE_SUCCESS == ret)
        {
            //ready get mac
            ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
            if(SKE_SUCCESS == ret)
            {
                //get mac
                ske_simple_get_output_block((uint32_t *)tmp, 4);
            }
            else
            {}
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_mac = lib_addr_arch32_lock_remap(mac_h, mac_l, 0U);
#else
            current_mac = lib_addr_arch32_lock_remap(0U, (uint32_t)mac, 0U);
#endif
            if(SKE_CRYPTO_ENCRYPT == ctx->ske_gcm_ctx->crypto)
            {
                memcpy_(current_mac, (uint8_t *)tmp, ctx->mac_bytes);
            }
            else
            {
                ret= memcmp_((uint8_t *)current_mac, (uint8_t *)tmp, CAST2UINT32(ctx->mac_bytes));
                if(0U != ret)
                {
                    ret = SKE_VERIFY_ERROR;
                }
                else
                {}
            }

            lib_addr_arch32_unlock_remap();
        }
        else
        {}

        memset_((uint8_t *)ctx, 0, sizeof(ske_gcm_ctx_st));
    }

    return ret;
}
#endif

#if 0
uint32_t ske_dma_gcm_update_blocks_head(ske_gcm_ctx_st *ctx, uint32_t *aad_and_in, uint32_t aad_and_in_bytes, uint32_t *out, SKE_CALLBACK callback)
{
    uint32_t aad_bytes_padding;
    uint32_t ret;

    aad_bytes_padding = (ctx->aad_bytes + 15)&(~0x0FU);

    ske_set_aad_len_uint32(ctx->aad_bytes);
    ske_set_c_len_uint32(aad_and_in_bytes-aad_bytes_padding);
    ret =  ske_dma_operate(aad_and_in, out, aad_and_in_bytes, aad_and_in_bytes - aad_bytes_padding, callback);

    return ret;
}

uint32_t ske_dma_gcm_update_blocks_middle(ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, SKE_CALLBACK callback)
{
    uint32_t ret;

    ske_set_aad_len_uint32(0);
    ske_set_c_len_uint32(in_bytes);
    ret = ske_dma_operate(in, out, in_bytes, in_bytes, callback);

    return ret;
}

uint32_t ske_dma_gcm_update_blocks_tail(ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, SKE_CALLBACK callback)
{
    uint32_t ret;

    //*
    ske_set_aad_len_uint32(0);
    ske_set_c_len_uint32(in_bytes);
    ske_set_aad_total_len_uint32(ctx->aad_bytes);
    ske_set_c_total_len_uint32(ctx->c_bytes);//*/

    in_bytes = (in_bytes + 15)&(~0x0FU);

    ske_set_last_block(1);
    ret = ske_dma_operate(in, out, in_bytes, in_bytes + 16, callback);
    ske_set_last_block(0);

    if(SKE_SUCCESS == ret)  //clear useless data
    {
        if(ctx->c_bytes & 0x0F)
        {
            ske_clear_block_tail(out + (in_bytes>>2) - 4, (ctx->c_bytes & 0x0F));
        }
        else
        {}

        if(16 != ctx->mac_bytes)
        {
            ske_clear_block_tail(out + (in_bytes>>2), ctx->mac_bytes);
        }
        else
        {}
    }
    else
    {}

    return ret;

}
#endif


/* function: ske dma gcm mode input aad+plaintext/ciphertext, get ciphertext/plaintext+mac
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 *     in ------------------------- input, aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+mac
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_dma_gcm_init()
 *     2. if aad exists, it must be some blocks, if not, please pad it with 0
 *     3. plaintext/ciphertext must be some blocks, if not, please pad it with 0
 *     4. the output ciphertext/plaintext has the same number of blocks as the input plaintext/ciphertext,  
 *        and followed by one block, it is mac with padding 0 if necessary, so is the second last blcok if 
 *        necessary(ciphertext/plaintext)
 *     5. please make sure aad+plaintext/ciphertext is integral
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_update_all_blocks(const ske_gcm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_gcm_update_all_blocks(const ske_gcm_ctx_st *ctx, uint32_t *in, uint32_t *out, SKE_CALLBACK callback)
#endif
{
    uint32_t aad_blocks_bytes;
    uint32_t c_blocks_bytes;
    uint32_t in_bytes, out_bytes;
    uint32_t remainder_bytes;
    uint32_t ret = SKE_SUCCESS;
    uint8_t * current_out;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == in_h) && (0U == in_l)) || ((0U == out_h) && (0U == out_l)))
#else
    if((NULL == ctx) || (NULL == in) || (NULL == out))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        if(((ctx->aad_bytes+0x0FU) >= 0x0FU) && ((ctx->c_bytes+0x0FU) >= 0x0FU))// on overflow
        {
            aad_blocks_bytes = ((ctx->aad_bytes)+0x0FU)&(~(0x0FU)); //((ctx->aad_bytes+15U)/16U)*16U
            c_blocks_bytes   = ((ctx->c_bytes)+0x0FU)&(~(0x0FU));   //((ctx->c_bytes+15U)/16U)*16U
        }
        else
        {
            ret = SKE_LEN_OVERFLOW;
        }

        in_bytes = aad_blocks_bytes + c_blocks_bytes;
        out_bytes = c_blocks_bytes+16U;

        if((SKE_SUCCESS == ret) && (in_bytes >= c_blocks_bytes) && (out_bytes > c_blocks_bytes) && 
            (in_bytes <= 0x1FFFFFFFU) && (out_bytes <= 0x1FFFFFFFU)) //check whether overflow
        {
            ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, aad_blocks_bytes + c_blocks_bytes, 
                    c_blocks_bytes+16U, callback);
#else
            ret = ske_dma_operate(in, out, aad_blocks_bytes + c_blocks_bytes, c_blocks_bytes+16U,
                    callback);
#endif
            ske_set_last_block(0);
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_out = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
            current_out = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
            remainder_bytes = ctx->c_bytes & 0x0FU;
            if(0U != remainder_bytes)//clear useless data
            {
                memset_(&current_out[c_blocks_bytes-16U+remainder_bytes], 0, 16U-remainder_bytes);
            }
            else
            {}

            if(16U != ctx->mac_bytes)//clear useless mac
            {
                memset_(&current_out[c_blocks_bytes+(uint32_t)ctx->mac_bytes], 0, 16U-(uint32_t)ctx->mac_bytes);
            }
            else
            {}

            lib_addr_arch32_unlock_remap();
        }
        else
        {}
    }

    return ret;
}


/* function: ske dma gcm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_gcm_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is optional
 */
uint32_t ske_dma_gcm_final(ske_gcm_ctx_st *ctx)
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
        memset_((uint8_t *)ctx, 0, sizeof(ske_gcm_ctx_st));
        ret = SKE_SUCCESS;
    }

    return ret;
}


/* function: ske dma gcm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     in ------------------------- input, aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+mac
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     mac_bytes ------------------ input, byte length of mac, must be in [0,16]
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. aad_bytes and c_bytes could not be zero at the same time
 *     6. if aad exists, it must be some blocks, if not, please pad it with 0
 *     7. plaintext/ciphertext must be some blocks, if not, please pad it with 0
 *     8. the output ciphertext/plaintext has the same number of blocks as the input plaintext/ciphertext,  
 *        and followed by one block, it is mac with padding 0 if necessary, so is the second last blcok if 
 *        necessary(ciphertext/plaintext)
 *     9. please make sure aad+plaintext/ciphertext is integral
 *     10. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, uint32_t aad_bytes, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t c_bytes, uint8_t mac_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_gcm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        uint32_t iv_bytes, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, uint8_t mac_bytes,
        SKE_CALLBACK callback)
#endif
{
    ske_gcm_ctx_st ctx[1];
    uint32_t ret;
    uint8_t *current_out;

    if((0U == aad_bytes) && (0U == c_bytes))
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        current_out = lib_addr_arch32_lock_remap(out_h, out_l, 0U);    //output mac
#else
        current_out = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);    //output mac
#endif
        ret = ske_gcm_crypto(alg, crypto, key, sp_key_idx, iv, iv_bytes, NULL, 0U, NULL, 
            current_out, 0U, current_out, mac_bytes);

        lib_addr_arch32_unlock_remap();
    }
    else
    {
        ret = ske_dma_gcm_init(ctx, alg, crypto, key, sp_key_idx, iv, iv_bytes, aad_bytes, c_bytes, mac_bytes);
        if(SKE_SUCCESS == ret)
        {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            //init gcm cfg
            ret = ske_gcm_init_internal(ctx->ske_gcm_ctx, SKE_PAYLOAD_STREAM, ctx->ske_gcm_ctx->alg, ctx->ske_gcm_ctx->crypto, ctx->ske_gcm_ctx->key, 
                ctx->ske_gcm_ctx->sp_key_idx, aad_bytes, c_bytes, aad_bytes, c_bytes, (uint8_t *)ctx->ske_gcm_ctx->iv, 1U);
            if(SKE_SUCCESS == ret)
            {
#endif
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                //input is zero_padding(aad) + zero_padding(payload)
                ret = ske_dma_gcm_update_all_blocks(ctx, in_h, in_l, out_h, out_l, callback);
#else
                ret = ske_dma_gcm_update_all_blocks(ctx, in, out, callback);
#endif
                if(SKE_SUCCESS == ret)
                {
                    ret = ske_dma_gcm_final(ctx);
                }
                else
                {}

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            }
#endif
        }
        else
        {}
    }

    return ret;
}


#ifdef SUPPORT_SKE_MODE_GMAC
/* function: ske dma gmac mode(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     iv_bytes ------------------- input, byte length of iv, now only 12 bytes supported
 *     msg ------------------------ input, msg, please make sure aad here is integral
 *     msg_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     mac ------------------------ output(for generating mac)
 *     mac_bytes ------------------ input, byte length of mac.
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. iv must be 12 bytes here
 *     5. msg_bytes could be zero
 *     6. mac_bytes must be in [1, SKE_GCM_MAX_BYTES]
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_gmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, 
        uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, 
        uint8_t mac_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_gmac(ske_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv, uint32_t iv_bytes, 
        uint32_t *msg, uint32_t msg_bytes, uint32_t *mac, uint8_t mac_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    //for gmac, message is aad in gcm
    ret = ske_dma_gcm_crypto(alg, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, iv, iv_bytes, 
        msg_bytes, msg_h, msg_l, mac_h, mac_l, 0U, mac_bytes, callback);
#else
    ret = ske_dma_gcm_crypto(alg, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, iv, iv_bytes, 
        msg_bytes, msg, mac, 0, mac_bytes, callback);
#endif

    return ret;
}
#endif

#endif


#endif


