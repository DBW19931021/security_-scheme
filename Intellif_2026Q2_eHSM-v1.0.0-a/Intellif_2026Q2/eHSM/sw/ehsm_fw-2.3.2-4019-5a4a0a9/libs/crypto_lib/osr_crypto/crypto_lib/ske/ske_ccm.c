
#include "../../crypto_include/ske/ske_ccm.h"
#include "../../crypto_include/crypto_common/utility.h"



#ifdef SUPPORT_SKE_MODE_CCM



/* function: ske ccm pre init
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     crypto --------------------- input, encrypting or decrypting
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is common part for ske_ccm_init() and ske_dma_ccm_init()
 *     2. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     3. aad_bytes and c_bytes could be zero at the same time
 *     4. ctx->buf is iv for ccm
 */
uint32_t ske_ccm_pre_init(ske_ccm_ctx_st *ctx, ske_crypto_e crypto, const uint8_t *nonce, uint8_t M, uint8_t L, 
        uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t tmp, len;

    if((NULL == ctx) || (NULL == nonce))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((uint8_t)0 != (M & (uint8_t)1)) //check M(the valid candidates are 4,6,8,10,12,14,16)
    {
        ret =  SKE_INPUT_INVALID;
    }
    else if((M < (uint8_t)4) || (M > (uint8_t)16))
    {
        ret =  SKE_INPUT_INVALID;
    }
    else if((L < (uint8_t)2) || (L > (uint8_t)8)) //check L(the valid candidates are 2,3,4,5,6,7,8)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }
    
    if(SKE_SUCCESS == ret)
    {
        //check c_bytes
        tmp = c_bytes;
        len = 0;
        while(0U != tmp)
        {
            len++;

            tmp >>= 8;
        }

        if(len > L)
        {
            ret = SKE_INPUT_INVALID;
        }
        else
        {
            /***** init the ctx fields *****/
            ctx->M = M;
            ctx->L = L;

            //A0
            *((uint8_t *)ctx->buf) = (ctx->L)-(uint8_t)1;
            memcpy_((&((uint8_t *)(ctx->buf))[1]), nonce, (uint32_t)15-(ctx->L));
            memset_((&((uint8_t *)(ctx->buf))[(uint8_t)16-ctx->L]), 0, ctx->L);

            ctx->aad_bytes             = aad_bytes;
            ctx->c_bytes               = c_bytes;
            ctx->current_bytes         = 0;
            ctx->ske_ccm_ctx->crypto   = crypto;
        }
    }
    else
    {}
    
    return ret;
}


/* function: ske ccm get B0 block
 * parameters:
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     aad_bytes ------------------ input, byte length of cipher/plaintext, it could be any value, including 0
 *     out ------------------------ output, B0, occupy 16 bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU and DMA(one time) style
 *     2. only AES(128/192/256) and SM4 are supported for CCM mode
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. please make sure all parameters are valid
 */
void ske_ccm_get_B0(const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes, uint8_t out[16])
{
    uint8_t tmp[4];
    uint32_t value = c_bytes;

    //B0 flag
    out[0] = 0;
    out[0] |= (M-((uint8_t)2))/((uint8_t)2);

    out[0] <<= 3;
    out[0] |= L-((uint8_t)1);

    if(0U != aad_bytes)
    {
        out[0] |= (uint8_t)0x40;    //with aad flag
    }
    else
    {}

    //B0 nonce
    if(nonce != (&(out[1])))    //namely, if out is not ctx->buf
    {
        memcpy_(&(out[1]), nonce, 15U-(uint32_t)L);
        if((1U+15U) >= (uint32_t)L)
        {
            memset_(&(out[1U+15U-(uint32_t)L]), 0, L);
        }
        else
        {}
    }
    else
    {}

    //B0 message byte length
#ifdef SKE_CPU_BIG_ENDIAN
    memcpy_(tmp, &value, 4);
#else
    reverse_byte_array((const uint8_t *)(&value), tmp, 4);
#endif

    if(L <= (uint8_t)4)
    {
        memcpy_(&(out[(uint8_t)16-L]), &(tmp[(uint8_t)4-L]), L);
    }
    else
    {
        memcpy_(&(out[16-4]), tmp, 4);
    }
}


/* function: ske ccm get B0 block
 * parameters:
 *     nonce ----------------------- input, nonce in bytes, its byte lenth is 15-L
 *     aad_bytes ------------------- input, byte length of aad, it could be any value, including 0
 *     aad_offset ------------------ input, necessary byte length of add head, to build B1 block
 *     out ------------------------- output, B0, occupy 16 bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU and DMA(one time) style
 *     2. if aad exists, must get B1 block, otherwise do not call this function
 *     3. please make sure all parameters are valid
 */
void ske_ccm_get_B1(const uint8_t *aad, uint32_t aad_bytes, uint32_t *aad_offset, uint8_t out[16])
{
    uint8_t tmp[4];
    uint32_t value = aad_bytes;
    uint32_t current_bytes, left_bytes;

#ifdef SKE_CPU_BIG_ENDIAN
    memcpy_(tmp, &aad_bytes, 4);
#else
    reverse_byte_array((const uint8_t *)(&value), tmp, 4);
#endif

    if(aad_bytes < (((uint32_t)1<<16)-((uint32_t)1<<8)))
    {
        memcpy_(out, &tmp[2], 2);
        current_bytes = 2;
        left_bytes = 16-2;
    }
    else
    {
        out[0] = 0xFF;
        out[1] = 0xFE;
        memcpy_(&(out[2]), tmp, 4);
        current_bytes = 6;
        left_bytes = 16-6;
    }

    if(aad_bytes <= left_bytes)
    {
        memcpy_(&(out[current_bytes]), aad, aad_bytes);
        memset_(&(out[current_bytes + aad_bytes]), 0, left_bytes - aad_bytes);
        *aad_offset = aad_bytes;
    }
    else
    {
        memcpy_(&(out[current_bytes]), aad, left_bytes);
        *aad_offset = left_bytes;
    }
}


/* function: ske ccm wait and get key
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_ccm_get_mac(ske_ccm_ctx_st *ctx)
{
    uint32_t ret;

    //get mac
    ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
    if(SKE_SUCCESS == ret)
    {
        ske_simple_get_output_block(ctx->buf, ctx->ske_ccm_ctx->block_words);
    }
    else
    {}

    return ret;
}


/* function: ske ccm mode init config
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
 *     2. only AES(128/192/256) and SM4 are supported for CCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. aad_bytes and c_bytes could be zero at the same time
 */
static uint32_t ske_ccm_init_internal(ske_ctx_st *ctx, ske_payload_stage_e stage, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, 
    uint16_t sp_key_idx, uint32_t aad_bytes, uint32_t c_bytes, const uint8_t *iv, uint32_t is_dma_mode)
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
    ske_set_aad_len_uint32(aad_bytes);
    ske_set_c_len_uint32(c_bytes);

    ret = ske_init_internal(ctx, alg, SKE_MODE_CCM, crypto, key, sp_key_idx, iv);

    return ret;
}


/* function: ske ccm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for CCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 */
uint32_t ske_ccm_init(ske_ccm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t ret;

    ret = ske_ccm_pre_init(ctx, crypto, nonce, M, L, aad_bytes, c_bytes);
    if(SKE_SUCCESS == ret)
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        if((0U == aad_bytes) && (0U == c_bytes))
        {
#ifdef CONFIG_SKE_SUPPORT_SUSPEND
            ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, alg, crypto, key, 
                sp_key_idx, 0x10U, 0U, (uint8_t *)ctx->buf, 0U);//with B0
#else
            ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, alg, crypto, key, 
                sp_key_idx, 0U, 0U, (uint8_t *)ctx->buf, 0U);
#endif
            if(SKE_SUCCESS == ret)
            {
                //get and inplut B0
                ske_ccm_get_B0(nonce, M, L, aad_bytes, c_bytes, (uint8_t *)ctx->buf);

                ske_set_last_block(1);    //last block
                ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
                ske_set_last_block(0);    //not last block

                if(SKE_SUCCESS == ret)
                {
                    ret = ske_ccm_get_mac(ctx);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {
            //caution: iv here is A0
            ret = ske_keep_alg_key_iv(ctx->ske_ccm_ctx, alg, SKE_MODE_CCM, key, sp_key_idx, (uint8_t *)ctx->buf);

            //get B0
            ske_ccm_get_B0(nonce, M, L, aad_bytes, c_bytes, (uint8_t *)ctx->buf);
        }
#else
#ifdef CONFIG_SKE_SUPPORT_SUSPEND
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, alg, ctx->ske_ccm_ctx->crypto, key, 
            sp_key_idx, 0x10U+ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->buf, 0U);//with B0
#else
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, alg, ctx->ske_ccm_ctx->crypto, key, 
            sp_key_idx, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->buf, 0U);        
#endif
        if(SKE_SUCCESS == ret)
        {
            //get and inplut B0
            ske_ccm_get_B0(nonce, M, L, aad_bytes, c_bytes, (uint8_t *)ctx->buf);

            if(0U == ctx->aad_bytes)
            {
                ske_set_last_block(1);    //last block
            }
            else
            {}

            ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
            if(SKE_SUCCESS == ret)
            {
                if(0U == ctx->aad_bytes)
                {
                    ske_set_last_block(0);    //not last block

                    if(0U == ctx->c_bytes)
                    {
                        ret = ske_ccm_get_mac(ctx);
                    }
                    else
                    {}
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
    else
    {}

    return ret;
}


/* function: ske ccm mode input aad (without B0)
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     aad ------------------------ input, aad, its length is ctx->aad_bytes, please make sure
 *                                         aad here is integral
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_ccm_init()
 *     2. if there is no aad, this function could be omitted
 */
static uint32_t ske_ccm_update_aad_without_B0(ske_ccm_ctx_st *ctx, const uint8_t *aad)
{
    uint32_t ret;
    const uint8_t *current_aad = aad;
    uint32_t aad_bytes, aad_offset;
    uint32_t blocks_bytes, remainder_bytes;
    uint8_t is_finished = (uint8_t)0;

    //input B1,B2...
    aad_bytes = ctx->aad_bytes;

    /******** get and input B1 ********/
    ske_ccm_get_B1(current_aad, aad_bytes, &aad_offset, (uint8_t *)ctx->buf);

    aad_bytes -= aad_offset;

    current_aad = &(current_aad[aad_offset]);
    if(0U == aad_bytes)
    {
        ske_set_last_block(1);    //last block
    }
    else
    {}

    ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
    if(SKE_SUCCESS == ret)
    {
        if(0U == aad_bytes)
        {
            ske_set_last_block(0);    //not last block
            ctx->current_bytes = 0;

            if(0U == ctx->c_bytes)
            {
                ret = ske_ccm_get_mac(ctx);
            }
            else
            {}

            is_finished = (uint8_t)1;
        }
        else
        {}

        if((uint8_t)0 == is_finished)
        {
            /******** input B2,B3... ********/
            blocks_bytes = (aad_bytes)&(~0x0FU);  //assume that ctx->ske_ccm_ctx->block_bytes is 16
            remainder_bytes = (aad_bytes)&(0x0FU);
            if(0U == remainder_bytes)
            {
                blocks_bytes -= 16U;

                remainder_bytes = 16U;
            }
            else
            {}

            ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, current_aad, blocks_bytes);
            if(SKE_SUCCESS == ret)
            {
                memcpy_(ctx->buf, &(current_aad[blocks_bytes]), remainder_bytes);
                memset_(&(((uint8_t *)(ctx->buf))[remainder_bytes]), 0, ctx->ske_ccm_ctx->block_bytes - remainder_bytes);
                ske_set_last_block(1);    //last block
                ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
                ske_set_last_block(0);    //not last block

                if((SKE_SUCCESS == ret) && (0U == ctx->c_bytes))
                {
                    ret = ske_ccm_get_mac(ctx); 
                }
                else
                {}
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


/* function: ske ccm mode input aad(one-off style)
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     aad ------------------------ input, aad, its length is ctx->aad_bytes, please make sure
 *                                         aad here is integral
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_ccm_init()
 *     2. if there is no aad, this function could be omitted
 */
uint32_t ske_ccm_update_aad(ske_ccm_ctx_st *ctx, const uint8_t *aad)
{
    uint32_t ret = SKE_SUCCESS;
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ske_payload_stage_e stage;
#endif

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((NULL == aad) && (0U != ctx->aad_bytes))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    //now aad is not NULL, and ctx->aad_bytes is not 0
    if((SKE_SUCCESS == ret) && (0U != ctx->aad_bytes))
    {
        //init ccm cfg
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        if(0U == ctx->c_bytes)
        {
            stage = SKE_PAYLOAD_STREAM;
        }
        else
        {
            stage = SKE_PAYLOAD_HEAD;
        }

#ifdef CONFIG_SKE_SUPPORT_SUSPEND
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, stage, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, 0x10U+ctx->aad_bytes, 0U, (uint8_t *)ctx->ske_ccm_ctx->iv, 0U);
#else
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, stage, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, ctx->aad_bytes, 0U, (uint8_t *)ctx->ske_ccm_ctx->iv, 0U);
#endif

        if(SKE_SUCCESS == ret)
        {
            //input B0
            ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
            //update aad
            ret = ske_ccm_update_aad_without_B0(ctx, aad);
        }
        else
        {}

        if((SKE_SUCCESS == ret) && (0U != ctx->c_bytes))
        {
            ske_get_mid_mac(ctx->mid_mac, 4, 0);
            ske_get_mid_iv(ctx->mid_iv, 4);
        }
        else
        {}
#else
        //update aad
        ret = ske_ccm_update_aad_without_B0(ctx, aad);
#endif
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        ctx->current_bytes = 0;
    }
    else
    {}

    return ret;
}


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
/* function: ske ccm init cfg before update ciphertext/plaintext
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     bytes ---------------------- input, byte length of input or output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after the whole aad is inputted(if aad exists)
 *     2. if there is no plaintext/ciphertext(bytes = 0), this function could be omitted
 *     3. please make sure all parameter are valid
 */
static uint32_t ske_ccm_update_blocks_init(ske_ccm_ctx_st *ctx, uint32_t bytes)
{
    uint32_t total_bytes = ctx->current_bytes + bytes; //ctx is valid
    uint32_t ret;
    uint32_t aad_bytes = 0U;
    ske_payload_stage_e stage;

    if((0U == ctx->current_bytes) && (0U == ctx->aad_bytes))
    {
        if(total_bytes == ctx->c_bytes)
        {
            stage = SKE_PAYLOAD_STREAM;
        }
        else
        {
            stage = SKE_PAYLOAD_HEAD;
        }
#ifdef CONFIG_SKE_SUPPORT_SUSPEND
        aad_bytes = 0x10U;//B0
#endif
    }
    else
    {}

    if((0U != ctx->current_bytes) || (0U != ctx->aad_bytes))
    {
        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);

        if(total_bytes == ctx->c_bytes)
        {
            stage = SKE_PAYLOAD_LAST;
        }
        else
        {
            stage = SKE_PAYLOAD_MIDDLE;
        }
    }
    else
    {}

    ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, stage, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, aad_bytes, bytes, (uint8_t *)ctx->ske_ccm_ctx->iv, 0U);
    if(SKE_SUCCESS == ret)
    {
        if((SKE_PAYLOAD_STREAM == stage) || (SKE_PAYLOAD_HEAD == stage))
        {
            //input B0
            ske_set_last_block(1);
            ret = ske_update_blocks_no_output(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, ctx->ske_ccm_ctx->block_bytes);
            ske_set_last_block(0);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif


/* function: ske ccm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
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
 *        could be divided into 3 sections with byte length 32,16,17 respectively.
 */
static uint32_t ske_ccm_update_blocks_internal(ske_ccm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t total_bytes = ctx->current_bytes + bytes; //ctx is valid
    uint32_t ret;

    if(total_bytes == ctx->c_bytes)
    {
        blocks_bytes = bytes & (~0x0FU); //here bytes must not be 0
        remainder_bytes = bytes & (0x0FU);
        if(0U == remainder_bytes)
        {
            blocks_bytes -= 16U;
            remainder_bytes = 16U;
        }
        else
        {}

        ret = ske_update_blocks_internal(ctx->ske_ccm_ctx, in, out, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            //the last block
            memcpy_(ctx->buf, &(in[blocks_bytes]), remainder_bytes);
            memset_(&(((uint8_t *)(ctx->buf))[remainder_bytes]), 0, 16U-remainder_bytes);

            ske_set_last_block(1);
            ret = ske_update_blocks_internal(ctx->ske_ccm_ctx, (uint8_t *)ctx->buf, (uint8_t *)ctx->buf, 16);
            ske_set_last_block(0);
            if(SKE_SUCCESS == ret)
            {
                memcpy_(&(out[blocks_bytes]), ctx->buf, remainder_bytes);

                ret = ske_ccm_get_mac(ctx);
            }
            else
            {}
        }
        else
        {}
    }
    else
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        blocks_bytes = bytes - ctx->ske_ccm_ctx->block_bytes;
        ret = ske_update_blocks_internal(ctx->ske_ccm_ctx, in, out, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
            ret = ske_update_blocks_internal(ctx->ske_ccm_ctx, &(in[blocks_bytes]), &(out[blocks_bytes]), ctx->ske_ccm_ctx->block_bytes);
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
        ret = ske_update_blocks_internal(ctx->ske_ccm_ctx, in, out, bytes);
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


/* function: ske ccm mode input plaintext/ciphertext
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
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
 *        could be divided into 3 sections with byte length 32,16,17 respectively.
 */
uint32_t ske_ccm_update_blocks(ske_ccm_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
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
        ret = ske_ccm_update_blocks_init(ctx, bytes);//init ccm cfg
        if(SKE_SUCCESS == ret)
        {
            ret = ske_ccm_update_blocks_internal(ctx, in, out, bytes);
        }
        else
        {}
#else
        ret = ske_ccm_update_blocks_internal(ctx, in, out, bytes);
#endif        
    }
    else
    {}

    return ret;
}


/* function: ske ccm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     mac ------------------------ input(for decryption), output(for encryption)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. byte length of mac is ctx->M
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_ccm_final(ske_ccm_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        if(SKE_CRYPTO_ENCRYPT == ctx->ske_ccm_ctx->crypto)
        {
            memcpy_(mac, ctx->buf, ctx->M);
        }
        else
        {
            ret= memcmp_(mac, ctx->buf, ctx->M);
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
        memset_(ctx, 0, sizeof(ske_ccm_ctx_st));
    }

    return ret;
}


/* function: ske ccm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad ------------------------ input, aad, please make sure aad here is integral
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     mac ------------------------ input(for decryption), output(for encryption)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for CPU style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     7. byte length of mac is M
 *     8. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
uint32_t ske_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, const uint8_t *aad, uint32_t aad_bytes, const uint8_t *in, uint8_t *out, uint32_t c_bytes,
        uint8_t *mac)
{
    ske_ccm_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_ccm_init(ctx, alg, crypto, key, sp_key_idx, nonce, M, L, aad_bytes, c_bytes);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_ccm_update_aad(ctx, aad);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_ccm_update_blocks(ctx, in, out, c_bytes);
            if(SKE_SUCCESS == ret)
            {
                ret = ske_ccm_final(ctx, mac);
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




#ifdef SKE_DMA_FUNCTION
/* function: ske dma ccm mode init config
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for CCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could not be zero at the same time due to hardware implementation
 */
uint32_t ske_dma_ccm_init(ske_ccm_ctx_st *ctx, ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx,
        const uint8_t *nonce, uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t c_bytes)
{
    uint32_t b1_aad_start_offset;
    uint32_t tmp;
    uint32_t ret;

    ret = ske_ccm_pre_init(ctx, crypto, nonce, M, L, aad_bytes, c_bytes);
    if(SKE_SUCCESS == ret)
    {
        //get aad start offset in B1
        if(0U == aad_bytes)
        {
            b1_aad_start_offset = 0U;  //only B0
        }
        else if(aad_bytes < (((uint32_t)1<<16)-((uint32_t)1<<8)))
        {
            b1_aad_start_offset = 2U;
        }
        else
        {
            b1_aad_start_offset = 6U;
        }

        //set total block length except plaintext/ciphertext, this is due to the hardware requires
        tmp = (b1_aad_start_offset + 16U);//B0 + aad_bytes

        if((ctx->aad_bytes+tmp+15U) > ctx->aad_bytes) //no overflow
        {
            ctx->aad_bytes = (ctx->aad_bytes+tmp+15U)&(~0x0FU);
        }
        else
        {
            ret = SKE_LEN_OVERFLOW;
        }

        if(SKE_SUCCESS == ret)
        {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
            //caution: iv here is A0    
            ret = ske_keep_alg_key_iv(ctx->ske_ccm_ctx, alg, SKE_MODE_CCM, key, sp_key_idx, (uint8_t *)ctx->buf);
#else
            ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, alg, ctx->ske_ccm_ctx->crypto, key, 
                sp_key_idx, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->buf, 1U);
#endif
        }
        else
        {}
    }
    else
    {}

    return ret;
}


#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
/* function: ske dma ccm mode update B0 and whole AAD part
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     B0_aad --------------------- input, B0 + AAD, here AAD is whole aad with prefix, and may be with padding 0
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. here AAD is whole aad with prefix(the prefix is in B1 ahead of aad), and may pad some zeros to make AAD 
 *        length a multiple of block length
 *     3. this function must be called after calling ske_dma_ccm_init()
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_blocks_B0_and_whole_aad(ske_ccm_ctx_st *ctx, uint32_t B0_aad_h, uint32_t B0_aad_l, 
        SKE_CALLBACK callback)
#else
uint32_t ske_dma_ccm_update_blocks_B0_and_whole_aad(ske_ccm_ctx_st *ctx, uint32_t *B0_aad, SKE_CALLBACK callback)
#endif
{
    uint32_t ret;

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == B0_aad_h) && (0U == B0_aad_l)))
#else
    if((NULL == ctx) || (NULL == B0_aad))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_HEAD, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, ctx->aad_bytes, 0U, (uint8_t *)ctx->ske_ccm_ctx->iv, 1U);
        if(SKE_SUCCESS == ret)
        {
            ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(B0_aad_h, B0_aad_l, 0U, 0U, ctx->aad_bytes, 0, callback);
#else
            ret = ske_dma_operate(B0_aad, NULL, ctx->aad_bytes, 0, callback);
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


/* function: ske dma ccm mode update some plaintext/ciphertext blocks
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     in ------------------------- input, plaintext/ciphertext of some blocks
 *     in_bytes ------------------- input, byte length of in/out
 *     out ------------------------ input, ciphertext/plaintext of some blocks
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. this function must be called after calling 
 *        ske_dma_ccm_init(), if aad is empty
 *     or ske_dma_ccm_update_blocks_B0_and_whole_aad(), if aad is not empty
 *     3. if the whole plaintext/ciphertext is too long, you could divide it by block(16 bytes),
 *        and if the whole plaintext/ciphertext byte length is not a multiple of 16, please make
 *        sure the last section contains the tail, then call this function to input the sections
 *        respectively. for example, if the whole plaintext/ciphertext byte length is 65, it 
 *        could be divided into 3 sections with byte length 48,16,1 respectively.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_blocks(ske_ccm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t in_bytes, 
        uint32_t out_h, uint32_t out_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_ccm_update_blocks(ske_ccm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, uint32_t *out, 
        SKE_CALLBACK callback)
#endif
{
    uint32_t total_bytes;
    uint32_t dma_in_bytes, remainder_bytes;
    uint32_t ret = SKE_SUCCESS;
    uint8_t *out_remap;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if((0U == ctx->c_bytes) && (0U != in_bytes))
    {
        ret = SKE_INPUT_INVALID;
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
        ctx->current_bytes += in_bytes;

        remainder_bytes = in_bytes & (0x0FU);
        dma_in_bytes = (in_bytes +15U) & (~0x0FU);

        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);

        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_MIDDLE, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, 0U, in_bytes, (uint8_t *)ctx->ske_ccm_ctx->iv, 1U);

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
/* function: ske dma ccm mode update plaintext/ciphertext with the last block(or the tail part)
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     in ------------------------- input, plaintext/ciphertext with the last block(or the tail part)
 *     in_bytes ------------------- input, byte length of in/out
 *     out ------------------------ input, ciphertext/plaintext with the last block(or the tail part) + mac
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. this function must be called after calling ske_dma_ccm_update_blocks_B0_and_whole_aad() or
 *        ske_dma_ccm_update_blocks_excluding_last_block()
 *     3. the mac is behind the output ciphertext/plaintext tail
  */
uint32_t ske_dma_ccm_update_blocks_including_last_block(ske_ccm_ctx_st *ctx, uint32_t *in, uint32_t in_bytes, 
        uint32_t *out, SKE_CALLBACK callback)
{
    uint32_t c_blocks_bytes;
    uint32_t ret;

    if((NULL == ctx) || (NULL == in) || (NULL == out))
    {
        return SKE_BUFFER_NULL;
    }
    else if(0 == in_bytes)
    {
        return SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    ske_clear_cfg();
    ske_set_dma_mode();
    ske_set_aad_len_uint32(0);
    ske_set_c_len_uint32(in_bytes);
    ske_set_payload_stage(SKE_PAYLOAD_LAST);
    ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
    ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);

    ret = ske_init_internal(ctx->ske_ccm_ctx, ctx->ske_ccm_ctx->alg, SKE_MODE_CCM, ctx->crypto, ctx->ske_ccm_ctx->key, 
        ctx->ske_ccm_ctx->sp_key_idx, (uint8_t *)ctx->ske_ccm_ctx->iv);
    if(SKE_SUCCESS != ret)
    {
        return ret;
    }
    else
    {}

    in_bytes = (in_bytes+15)&(~0x0F);

    ske_set_last_block(1);
    ret = ske_dma_operate(in, out, in_bytes, in_bytes+16, callback);
    ske_set_last_block(0);
    if(SKE_SUCCESS == ret)
    {
        if(ctx->c_bytes & 0x0F)
        {
            ske_clear_block_tail(out + (in_bytes>>2) - 4, (ctx->c_bytes & 0x0F));
        }
        else
        {}

        if(16 != ctx->M)
        {
            ske_clear_block_tail(out + (in_bytes>>2), ctx->M);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif


/* function: ske dma ccm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     mac ------------------------ input(for decryption), output(for encryption)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after aad and plaintext/ciphertext both are inputted.
 *     2. byte length of mac is ctx->M
 *     3. for encryption, mac is output; and for decryption, mac is input, if returns SKE_SUCCESS
 *        that means certification passed, otherwise not.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_final(ske_ccm_ctx_st *ctx, uint32_t mac_h, uint32_t mac_l)
#else
uint32_t ske_dma_ccm_update_final(ske_ccm_ctx_st *ctx, uint8_t *mac)
#endif
{
    uint32_t tmp[4];
    uint32_t ret;
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
        ske_set_mid_mac((uint32_t *)ctx->mid_mac, 4);
        ske_set_mid_iv((uint32_t *)ctx->mid_iv, 4);

        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_LAST, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, 0U, 0U, (uint8_t *)ctx->ske_ccm_ctx->iv, 0U);
        if(SKE_SUCCESS == ret)
        {
            ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
            ske_simple_get_output_block((uint32_t *)tmp, 4);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            current_mac = lib_addr_arch32_lock_remap(mac_h, mac_l, 0U);
#else
            current_mac = lib_addr_arch32_lock_remap(0U, (uint32_t)mac, 0U);
#endif
            if(SKE_CRYPTO_ENCRYPT == ctx->ske_ccm_ctx->crypto)
            {
                memcpy_(current_mac, (uint8_t *)tmp, ctx->M);
            }
            else
            {
                ret= memcmp_(current_mac, (uint8_t *)tmp, ctx->M);
                if(0U != ret)
                {
                    ret = SKE_VERIFY_ERROR;
                }
                else
                {}
            }

            lib_addr_arch32_unlock_remap();
            memset_(ctx, 0, sizeof(ske_ccm_ctx_st));
        }
        else
        {}
    }

    return ret;
}
#endif


/* function: ske dma ccm mode input B0+aad+plaintext/ciphertext, get ciphertext/plaintext+mac
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 *     in ------------------------- input, aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+mac
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function must be called after calling ske_dma_ccm_init()
 *     2. if aad exists, it must be some blocks, if not, please pad it with 0
 *     3. plaintext/ciphertext must be some blocks, if not, please pad it with 0
 *     4. the output ciphertext/plaintext has the same number of blocks as the input plaintext/ciphertext,  
 *        and followed by one block, it is mac with padding 0 if necessary, so is the second last blcok if 
 *        necessary(ciphertext/plaintext)
 *     5. please make sure B0+aad+plaintext/ciphertext is integral
 *     6. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_update_all_blocks(ske_ccm_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, SKE_CALLBACK callback)
#else
uint32_t ske_dma_ccm_update_all_blocks(ske_ccm_ctx_st *ctx, uint32_t *in, uint32_t *out, SKE_CALLBACK callback)
#endif
{
    uint32_t c_blocks_bytes;
    uint32_t in_bytes, out_bytes;
    uint32_t ret;
    uint8_t *current_out;
    uint32_t remainder_bytes;

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
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_ccm_init_internal(ctx->ske_ccm_ctx, SKE_PAYLOAD_STREAM, ctx->ske_ccm_ctx->alg, ctx->ske_ccm_ctx->crypto, ctx->ske_ccm_ctx->key, 
            ctx->ske_ccm_ctx->sp_key_idx, ctx->aad_bytes, ctx->c_bytes, (uint8_t *)ctx->ske_ccm_ctx->iv, 1U);
        if(SKE_SUCCESS == ret)
        {
#else
            ret = SKE_SUCCESS;
#endif
            if((ctx->c_bytes+0x0FU) >= 0x0FU)
            {
                c_blocks_bytes = (ctx->c_bytes+15U)&(~(0x0FU));//no overflow
            }
            else
            {
                ret = SKE_LEN_OVERFLOW;
            }

            in_bytes = ctx->aad_bytes+c_blocks_bytes;
            out_bytes = c_blocks_bytes+16U;

            //read bit len register is 32bit
            if((SKE_SUCCESS == ret) && (in_bytes >= c_blocks_bytes) && (out_bytes > c_blocks_bytes) && 
                (in_bytes <= 0x1FFFFFFFU) && (out_bytes <= 0x1FFFFFFFU)) //check whether overflow
            {
                ske_set_last_block(1);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                ret = ske_dma_operate(in_h, in_l, out_h, out_l, ctx->aad_bytes + c_blocks_bytes,
                    c_blocks_bytes+16U, callback);
#else
                ret = ske_dma_operate(in, out, ctx->aad_bytes + c_blocks_bytes,
                    c_blocks_bytes+16U, callback);
#endif
                ske_set_last_block(0);
            }
            else
            {}

            if(SKE_SUCCESS == ret)  //clear useless data
            {
                remainder_bytes = (ctx->c_bytes & 0x0FU);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                current_out = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
                current_out = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);        
#endif
                if(0U != remainder_bytes)
                {
                    memset_(&(current_out[c_blocks_bytes-16U+remainder_bytes]), 0, 16U-remainder_bytes);
                }
                else
                {}

                if((uint8_t)16 != ctx->M)
                {
                    memset_(&(current_out[c_blocks_bytes+(uint32_t)ctx->M]), 0, 16U-(uint32_t)ctx->M);
                }
                else
                {}

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


/* function: ske dma ccm mode finish
 * parameters:
 *     ctx ------------------------ input, ske_ccm_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is optional
 */
uint32_t ske_dma_ccm_final(ske_ccm_ctx_st *ctx)
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
        memset_(ctx, 0, sizeof(ske_ccm_ctx_st));
        ret = SKE_SUCCESS;
    }

    return ret;
}


/* function: ske dma ccm mode encrypt/decrypt(one-off style)
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, key of AES(128/192/256) or SM4
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     nonce ---------------------- input, nonce in bytes, its byte lenth is 15-L
 *     M -------------------------- input, bytes of authentication field(bytes of mac)
 *     L -------------------------- input, bytes of length field(message byte length is less than 256^L)
 *     aad_bytes ------------------ input, byte length of aad, it could be any value, including 0
 *     in ------------------------- input, B0+aad+plaintext/ciphertext
 *     out ------------------------ output, ciphertext/plaintext+mac
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext, it could be any value, including 0
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is for DMA style
 *     2. only AES(128/192/256) and SM4 are supported for GCM mode
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. valid M is {4,6,8,10,12,14,16}, and valid L is {2,3,4,5,6,7,8},
 *     5. aad_bytes and c_bytes could be zero at the same time
 *     6. if aad exists, it must be some blocks, if not, please pad it with 0
 *     7. plaintext/ciphertext must be some blocks, if not, please pad it with 0
 *     8. the output ciphertext/plaintext has the same number of blocks as the input plaintext/ciphertext, 
 *        and followed by one block, it is mac with padding 0 if necessary, so is the second last blcok if 
 *        necessary(ciphertext/plaintext)
 *     9. please make sure B0+aad+plaintext/ciphertext is integral
 *     10. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l,
        uint32_t c_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_ccm_crypto(ske_alg_e alg, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *nonce,
        uint8_t M, uint8_t L, uint32_t aad_bytes, uint32_t *in, uint32_t *out, uint32_t c_bytes, SKE_CALLBACK callback)
#endif
{
    ske_ccm_ctx_st ctx[1];
    uint32_t ret;

    ret = ske_dma_ccm_init(ctx, alg, crypto, key, sp_key_idx, nonce, M, L, aad_bytes, c_bytes);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_ccm_update_all_blocks(ctx, in_h, in_l, out_h, out_l, callback);
#else
        ret = ske_dma_ccm_update_all_blocks(ctx, in, out, callback);
#endif
        if(SKE_SUCCESS == ret)
        {
            ret = ske_dma_ccm_final(ctx);
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


