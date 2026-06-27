
#include "../../crypto_include/hash_hmac/hash.h"
#include "../../crypto_include/hash_hmac/hmac.h"
#include "../../crypto_include/crypto_common/utility.h"


/* function: clear hw info about key
 * parameters: none
 * return: none
 * caution:
 */
void hmac_clear(void)
{
    uint32_t block_word_len;
    hash_alg_e alg;
    uint8_t i;
    const uint32_t tmp = 0;

    hash_hmac_disable_secure_port();
    hash_set_cpu_mode();
    hash_set_hmac_mode();
    hash_set_hmac_key_mode();
    hash_set_endian_uint32();
    hash_disable_dma_interruption();

#if defined(SUPPORT_HASH_SHA512)
    alg = HASH_SHA512;
#elif defined(SUPPORT_HASH_SHA384)
    alg = HASH_SHA384;
#elif defined(SUPPORT_HASH_SHA512_224)
    alg = HASH_SHA512_224;
#elif defined(SUPPORT_HASH_SHA512_256)
    alg = HASH_SHA512_256;
#elif defined(SUPPORT_HASH_SM3)
    alg = HASH_SM3;
#elif defined(SUPPORT_HASH_SHA256)
    alg = HASH_SHA256;
#elif defined(SUPPORT_HASH_SHA224)
    alg = HASH_SHA224;
#elif defined(SUPPORT_HASH_SHA1)
    alg = HASH_SHA1;
#elif defined(SUPPORT_HASH_MD5)
    alg = HASH_MD5;

#if 0
#elif defined(SUPPORT_HASH_SHA3_224)     //hardware HMAC does not support SHA3 currently.
    alg = HASH_SHA3_224;
#elif defined(SUPPORT_HASH_SHA3_256)
    alg = HASH_SHA3_256;
#elif defined(SUPPORT_HASH_SHA3_384)
    alg = HASH_SHA3_384;
#elif defined(SUPPORT_HASH_SHA3_512)
    alg = HASH_SHA3_512;
#endif

#endif

    hash_set_alg(alg);
    hash_update_config();

    hash_set_IV(alg, hash_get_iterator_word_len(alg));
    block_word_len = hash_get_block_word_len(alg);
    hash_set_hmac_key_len(block_word_len<<5);
    hash_set_hmac_key_cnt(0);
    hash_start();
    
    //input key
    for(i=0; i<block_word_len; i++)
    {
        hash_input_msg_u8((const uint8_t *)&tmp, 4);
    }

    (void)hash_wait_till_done();

    hash_clear_hmac_key_mode();
    hash_set_last_block(0);

    hash_clear_cfg();
}


/* function: HMAC key state recover, to support multiple thread mode
 * parameters:
 *     hash_alg_e ------------------- input, specific hash algorithm
 *     key_len_flag --------------- input, whether key byte length is bigger than block byte length,
 *                                         should be HMAC_KEY_NOT_LONGER_THAN_BLOCK or
 *                                         HMAC_KEY_LONGER_THAN_BLOCK
 *     K0 ------------------------- input, hmac K0
 *     block_byte_len ------------- input, block byte length of hash_alg
 *     iterator_word_len ---------- input, iterator word length of hash_alg
 *     is_sp_key ------------------ input, whether use secure port key flag
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hmac_key_state_recover(hash_alg_e alg, uint32_t key_len_flag, const uint32_t *K0, 
        uint32_t block_byte_len, uint32_t iterator_word_len, uint32_t is_sp_key, uint16_t sp_key_idx, uint32_t sp_key_bytes)
{
    uint32_t ret = HASH_SUCCESS;

    hash_set_cpu_mode();
    hash_set_hmac_mode();
    hash_set_endian_uint32();
    hash_disable_dma_interruption();
    hash_set_alg(alg);
    hash_set_hmac_key_mode();

    if(0U != is_sp_key)
    {
        hash_hmac_enable_secure_port();
        hash_update_config();

        ret = hash_hmac_sp_key_opr(sp_key_idx, sp_key_bytes<<3);
    }
    else
    {
        hash_hmac_disable_secure_port();
        hash_update_config();

        if(HMAC_K_NOT_LONGER_THAN_BLOCK == key_len_flag)
        {
            hash_set_IV(alg, iterator_word_len);
            ret = hash_hmac_key_opr_one_block(K0, block_byte_len);
        }
        else if(HMAC_K_LONGER_THAN_BLOCK == key_len_flag)
        {
            hash_set_iterator(K0, iterator_word_len);
            hash_set_hmac_key_len(0);
            hash_set_hmac_key_cnt(0);
            hash_set_last_block(1);
            hash_start();
            ret = hash_wait_till_done();
        }
        else
        {
            //handle other
        }

    }

    hash_clear_hmac_key_mode();
    hash_set_last_block(0);

    return ret;
}


/* function: init HMAC step 1, check input and conifg hardware.
 * parameters:
 *     ctx ------------------------ input, context pointer
 *     alg ------------------------ input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash_alg_e is valid
 *     2. here hmac is not for SHA3.
 */
static uint32_t hmac_init_step1(hash_alg_e alg, const uint8_t *key, uint32_t key_bytes)
{
    uint32_t ret = HASH_SUCCESS;

    if(alg >= HASH_SHA3_START)
    {
        ret = HASH_INPUT_INVALID;
    }
    else if(0U != (key_bytes & 0xE0000000U))  //bit length overflow
    {
        ret = HASH_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if(HASH_SUCCESS == ret)
    {
        if(NULL != key)   //key is from user input
        {
            hash_hmac_disable_secure_port();
        }
        else
        {}

        hash_set_cpu_mode();
        hash_set_hmac_mode();
        hash_set_hmac_key_mode();
        hash_set_endian_uint32();
        hash_disable_dma_interruption();
        hash_set_alg(alg);
        hash_update_config();
    }
    else
    {}

    return ret;
}


/* function: init HMAC step 2, conifg key(get K0), etc.
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 *     block_byte_len ------------- input, block byte length of alg
 *     iterator_word_len ---------- input, iterator word length of alg
 *     digest_byte_len ------------ input, digest byte length of alg
 *     key_len_flag --------------- output, whether key byte length is bigger than block byte length,
 *                                         should be HMAC_KEY_NOT_LONGER_THAN_BLOCK or
 *                                         HMAC_KEY_LONGER_THAN_BLOCK
 *     K0 ------------------------- output, hmac K0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. please make K0 cleared before calling
 *     3. here hmac is not for SHA3.
 */
static uint32_t hmac_init_step2(hash_alg_e alg, const uint8_t *key, uint32_t key_bytes, uint32_t block_byte_len, 
        uint32_t iterator_word_len, uint32_t digest_byte_len, uint32_t *key_len_flag, uint32_t *K0, uint16_t sp_key_idx)
{
    uint32_t ret = HASH_SUCCESS;

    if(NULL != key)   //key is from user input
    {
        hash_set_IV(alg, iterator_word_len);

        //get K0 and back-up K0
        if(key_bytes <= block_byte_len)
        {
            //K0 = (key)||000...00
            *key_len_flag = HMAC_K_NOT_LONGER_THAN_BLOCK;
            memcpy_((uint8_t *)(K0), key, key_bytes);
            ret = hash_hmac_key_opr_one_block(K0, block_byte_len);
        }
        else
        {
            //K0 = hash(key)||000...00
            *key_len_flag = HMAC_K_LONGER_THAN_BLOCK;
            ret = hash_hmac_key_opr_longer_than_one_block(key, key_bytes);
            if(HASH_SUCCESS == ret)
            {
                hash_get_iterator(((uint8_t *)(K0)), digest_byte_len>>2);
            }
            else
            {}
        }
    }
    else      //key is from secure port
    {
#ifdef HMAC_SECURE_PORT_FUNCTION
        ret = hash_hmac_sp_key_opr(sp_key_idx, key_bytes<<3);
#endif
    }

    hash_clear_hmac_key_mode();
    hash_set_last_block(0);

    return ret;
}


#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
/* function: init HMAC(for sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     alg ------------------- input, specific sha3 hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid(sha3 hash algorithm)
 */
static uint32_t hmac_sha3_init(hmac_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint32_t key_bytes)
{
    uint32_t block_byte_len, digest_byte_len;
    uint32_t i, ret = HASH_SUCCESS;

    hash_hmac_disable_secure_port();

    block_byte_len = CAST2UINT32(hash_get_block_word_len(alg))<<2;
    digest_byte_len = CAST2UINT32(hash_get_digest_word_len(alg))<<2;

    //get K0
    if(key_bytes <= block_byte_len)
    {
        memcpy_(ctx->K0, key, key_bytes);
        memset_(&(((uint8_t *)(ctx->K0))[key_bytes]), 0, block_byte_len - key_bytes);
    }
    else
    {
        //K0 = hash(key)||000..00
        ret = hash_init(ctx->hash_ctx, alg);
        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(ctx->hash_ctx, key, key_bytes);
            if(HASH_SUCCESS == ret)
            {
                ret = hash_final(ctx->hash_ctx, (uint8_t *)(ctx->K0));
            }
            else
            {}
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            memset_(&(((uint8_t *)(ctx->K0))[digest_byte_len]), 0, block_byte_len - digest_byte_len);
        }
        else
        {}
    }

    if(HASH_SUCCESS == ret)
    {
        //get K0 ^ ipad
        digest_byte_len = block_byte_len/4U;
        for(i=0; i<digest_byte_len; i++)
        {
            ctx->K0[i] ^= HMAC_IPAD;
        }

        ret = hash_init(ctx->hash_ctx, alg);
        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(ctx->hash_ctx, (uint8_t *)(ctx->K0), block_byte_len);
        }
        else
        {}

        if(HASH_SUCCESS != ret)
        {
            memset_((uint8_t *)ctx, 0, sizeof(hmac_ctx_st));
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif


/* function: init HMAC
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of key, it could be 0
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. if cfg not support secure port, please set key is NULL when hmac without key
 *     3. if cfg support secure port, hmac will use secure port key when parameter key is NULL, 
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t hmac_init(hmac_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes)
{
    uint32_t ret = HASH_SUCCESS;
    uint32_t actual_key_bytes = key_bytes;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
        //clear and init the context
        memset_((uint8_t *)ctx, 0, sizeof(hmac_ctx_st));
    }

    if(HASH_SUCCESS == ret)
    {
#ifndef HMAC_SECURE_PORT_FUNCTION
        if(NULL == key)
        {
            actual_key_bytes = 0;
        }
        else
        {}

        ctx->is_sp_key = 0U;
#else
        if(NULL == key)
        {
            ctx->is_sp_key = 1U;
            ctx->sp_key_bytes = key_bytes;
            ctx->sp_key_idx = sp_key_idx;
        }
        else
        {
            ctx->is_sp_key = 0U;
        }
#endif
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((alg >= HASH_SHA3_224) && (alg <= HASH_SHA3_512))
        {
            if(NULL == key)
            {
                ret = HASH_INPUT_INVALID; //IP not diretlly support hmac_sha3
            }
            else
            {
                ret = hmac_sha3_init(ctx, alg, key, actual_key_bytes);
            }
        }
        else
        {
#endif
            ret = hmac_init_step1(alg, key, actual_key_bytes);
            if(HASH_SUCCESS == ret)
            {
                ctx->hash_ctx->hfe_mode          = HMAC_MODE;
                ctx->hash_ctx->alg          = alg;
                ctx->hash_ctx->block_byte_len    = hash_get_block_word_len(alg)<<2;
                ctx->hash_ctx->iterator_word_len = hash_get_iterator_word_len(alg);
                ctx->hash_ctx->digest_byte_len   = hash_get_digest_word_len(alg)<<2;
                ctx->hash_ctx->status.busy       = 0U;
                ctx->hash_ctx->first_update_flag = (uint8_t)1;
                ctx->hash_ctx->finish_flag       = (uint8_t)0;

                ret = hmac_init_step2(alg, key, actual_key_bytes, ctx->hash_ctx->block_byte_len,
                ctx->hash_ctx->iterator_word_len, ctx->hash_ctx->digest_byte_len,
                    &ctx->key_len_flag, ctx->K0, ctx->sp_key_idx);
            }
            else
            {}
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
    }

    return ret;
}


/* function: hmac update message(for non sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 *     2. this is not for sha3
 */
static uint32_t hmac_non_sha3_update(hmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = HASH_SUCCESS;
    uint32_t count;
    uint8_t left, fill;
    uint8_t is_finished = (uint8_t)0;
    const uint8_t *current_msg = msg;
    uint32_t remainder_msg_bytes = msg_bytes;

    ctx->hash_ctx->status.busy = 1U;    //start to update processing

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    ret = hmac_key_state_recover(ctx->hash_ctx->alg, ctx->key_len_flag, ctx->K0,
            ctx->hash_ctx->block_byte_len, ctx->hash_ctx->iterator_word_len, ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
    if(HASH_SUCCESS == ret)
    {
#endif
        left = (uint8_t)(ctx->hash_ctx->total[0] % (ctx->hash_ctx->block_byte_len));     //byte length of valid message left in block buffer
        fill = (ctx->hash_ctx->block_byte_len) - left;                        //byte length that block buffer need to fill a block

        //update total byte length
        if(0U != hash_total_byte_len_add_uint32(ctx->hash_ctx->total, CAST2UINT32(ctx->hash_ctx->block_byte_len)/32U, msg_bytes))
        {
            ret = HASH_LEN_OVERFLOW;
        }
        else
        {}

        if(((uint8_t)0 != left) && (HASH_SUCCESS == ret))
        {
            if(msg_bytes >= fill)
            {
                memcpy_(&(ctx->hash_ctx->hash_buffer[left]), current_msg, fill);
                ret = hash_calc_blocks(ctx->hash_ctx, ctx->hash_ctx->hash_buffer, 1);
                if(HASH_SUCCESS == ret)
                {
                    remainder_msg_bytes -= fill;
                    current_msg = &(current_msg[fill]);
                }
                else
                {}
            }
            else
            {
                memcpy_(&ctx->hash_ctx->hash_buffer[left], current_msg, msg_bytes);
                is_finished = (uint8_t)1;
            }
        }
        else
        {}

        if((HASH_SUCCESS == ret) && ((uint8_t)0 == is_finished))
        {
            //process some blocks
            count = remainder_msg_bytes/(ctx->hash_ctx->block_byte_len);
            if(0U != count)
            {
                ret = hash_calc_blocks(ctx->hash_ctx, current_msg, count);
            }
            else
            {}

            if(HASH_SUCCESS == ret)
            {
                //process the remainder
                remainder_msg_bytes = remainder_msg_bytes % (ctx->hash_ctx->block_byte_len);
                if(0U != remainder_msg_bytes)
                {
                    current_msg = &(current_msg[(ctx->hash_ctx->block_byte_len)*count]);
                    memcpy_(ctx->hash_ctx->hash_buffer, current_msg, remainder_msg_bytes);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    }
    else
    {}
#endif

    ctx->hash_ctx->status.busy = 0U;    //update end, status becomes idle

    return ret;
}


/* function: hmac update message
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hmac_update(hmac_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = HASH_SUCCESS;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else if((NULL == msg) || (0U == msg_bytes))
    {
        //nothing to do
    }
    else
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512))
        {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            //IP not diretlly support hmac_sha3
            hash_hmac_disable_secure_port();
#endif
            ret = hash_update(ctx->hash_ctx, msg, msg_bytes);
        }
        else
        {
#endif
            ret = hmac_non_sha3_update(ctx, msg, msg_bytes);
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
    }

    return ret;
}


#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
/* function: message update done, get the hmac(for sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the mac buffer is sufficient
 */
uint32_t hmac_sha3_final(hmac_ctx_st *ctx, uint8_t *mac)
{
    hash_alg_e alg;
    uint32_t block_word_len, digest_word_len;
    uint32_t i, ret = HASH_SUCCESS;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        alg = ctx->hash_ctx->alg;
        digest_word_len = hash_get_digest_word_len(alg);

        //set mac as hash((K0^ipad)||message)
        //caution: here context will be cleaned up
        ret = hash_final(ctx->hash_ctx, mac);
        if(HASH_SUCCESS == ret)
        {
            //get K0 ^ opad
            block_word_len = hash_get_block_word_len(alg);
            for(i=0; i<block_word_len; i++)
            {
                ctx->K0[i] ^= HMAC_IPAD_XOR_OPAD;
            }

            ret = hash_init(ctx->hash_ctx, alg);
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            ret = hash_update(ctx->hash_ctx, (uint8_t *)(ctx->K0), ctx->hash_ctx->block_byte_len);
            if(HASH_SUCCESS == ret)
            {
                ret = hash_update(ctx->hash_ctx, mac, ctx->hash_ctx->digest_byte_len);
                if(HASH_SUCCESS == ret)
                {
                    ret = hash_final(ctx->hash_ctx, mac);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}

        //clear mac if error exists
        if(HASH_SUCCESS != ret)
        {
            memset_(mac, 0, digest_word_len<<2);
        }
        else
        {
            //clear the context
            memset_((uint8_t *)ctx, 0, sizeof(hmac_ctx_st));
        }
    }

    return ret;
}
#endif


/* function: message update done, get the hmac
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the mac buffer is sufficient
 */
uint32_t hmac_final(hmac_ctx_st *ctx, uint8_t *mac)
{
    uint32_t ret;
    uint8_t tmp;

    if((NULL == ctx) || (NULL == mac))
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
        //for hmac-sha3
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512))
        {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            //IP not diretlly support hmac_sha3
            hash_hmac_disable_secure_port();
#endif
            ret = hmac_sha3_final(ctx, mac);
        }
        else
        {
#endif

       //for hmac-non-sha3
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            ret = hmac_key_state_recover(ctx->hash_ctx->alg, ctx->key_len_flag, ctx->K0,
                        ctx->hash_ctx->block_byte_len, ctx->hash_ctx->iterator_word_len, ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
            if(HASH_SUCCESS == ret)
            {
#endif
                ctx->hash_ctx->finish_flag = (uint8_t)1;     //the last block calc

                //get the byte length of the remainder msg(less than one block)
                tmp = (uint8_t)(ctx->hash_ctx->total[0] % (ctx->hash_ctx->block_byte_len));

                //set total msg bit length
                hash_total_bytelen_2_bitlen(ctx->hash_ctx->total, CAST2UINT32(ctx->hash_ctx->block_byte_len)/32U);
                hash_set_msg_total_bit_len(ctx->hash_ctx->total, ctx->hash_ctx->block_byte_len);

                //input the remainder msg(less than one block)
                ret = hash_calc_rand_len_msg(ctx->hash_ctx, ctx->hash_ctx->hash_buffer, tmp);
                if(HASH_SUCCESS == ret)
                {
                    //get the hash result
                    hash_get_iterator(mac, CAST2UINT32(ctx->hash_ctx->digest_byte_len)>>2);
                }
                else
                {}
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            }
            else
            {}
#endif
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
        // clear the context for non-sha3
        memset_((uint8_t *)ctx, 0, sizeof(hmac_ctx_st));
    }

    return ret;
}


/* function: input key and whole message, get the hmac
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of the key
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the mac buffer is sufficient
 *     2. here hmac is not for SHA3.
 *     3. if cfg not support secure port, please set key is NULL when hmac without key
 *     4. if cfg support secure port, hmac will use secure port key when parameter key is NULL, 
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t hmac(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, const uint8_t *msg, 
        uint32_t msg_bytes, uint8_t *mac)
{
    hmac_ctx_st ctx[1];
    uint32_t ret;

    ret = hmac_init(ctx, alg, key, sp_key_idx, key_bytes);
    if(HASH_SUCCESS == ret)
    {
        ret = hmac_update(ctx, msg, msg_bytes);
        if(HASH_SUCCESS == ret)
        {
            ret = hmac_final(ctx, mac);
        }
        else
        {}
    }
    else
    {}

    return ret;
}


#ifdef SUPPORT_HASH_NODE
/* function: input key and whole message, get the hmac(node style)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, byte length of the key
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the mac buffer is sufficient
 *     2. here hmac is not for SHA3.
 *     3. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 */
uint32_t hmac_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_node_st *node, uint32_t node_num, uint8_t *mac)
{
    hmac_ctx_st ctx[1];
    uint32_t i, ret;

    ret = hmac_init(ctx, alg, key, sp_key_idx, key_bytes);
    if(HASH_SUCCESS == ret)
    {
        for(i=0U; i<node_num; i++)
        {
            ret = hmac_update(ctx, node[i].msg_addr, node[i].msg_bytes);
            if(HASH_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hmac_final(ctx, mac);
    }
    else
    {}

    return ret;
}
#endif


#ifdef HASH_DMA_FUNCTION
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
/* function: init dma hmac(for sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     alg ------------------------ input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid(sha3 hash algorithm)
 */
static uint32_t hmac_sha3_dma_init(hmac_dma_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, 
        uint32_t key_bytes, HASH_CALLBACK callback)
{
    uint32_t ret;
    uint32_t actual_key_bytes = key_bytes;
    hmac_ctx_st tmp_ctx[1];

    hash_disable_dma_interruption();

    ret = hmac_sha3_init(tmp_ctx, alg, key, actual_key_bytes);
    if(HASH_SUCCESS == ret)
    {
        ctx->hash_dma_ctx->alg        = alg;
        ctx->hash_dma_ctx->block_word_len  = (tmp_ctx->hash_ctx->block_byte_len)/((uint8_t)4);
        ctx->hash_dma_ctx->digest_byte_len = hash_get_digest_word_len(alg)<<2;
        ctx->hash_dma_ctx->callback        = callback;
        uint32_copy(ctx->hash_dma_ctx->total, tmp_ctx->hash_ctx->total, CAST2UINT32(ctx->hash_dma_ctx->block_word_len)>>3);
        memcpy_((uint8_t *)(ctx->K0), (uint8_t *)(tmp_ctx->K0), CAST2UINT32(ctx->hash_dma_ctx->block_word_len)<<2);

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        ctx->hash_dma_ctx->hfe_mode          = HASH_MODE; //since the hardware requires.
        ctx->hash_dma_ctx->first_update_flag = (uint8_t)0;
        ctx->hash_dma_ctx->iterator_word_len = hash_get_iterator_word_len(alg);
        memcpy_((uint8_t *)(ctx->hash_dma_ctx->iterator), (uint8_t *)(tmp_ctx->hash_ctx->iterator), 
                CAST2UINT32(ctx->hash_dma_ctx->iterator_word_len)<<2);
#else
#if 0
//        hash_set_hash_mode() 
//        hash_set_endian_uint32() 
//        hash_disable_dma_interruption() 
//        hash_set_last_block(0) 
//        hash_set_alg(alg) 
//        hash_update_config() 
#endif
        hash_clear_risr();
        hash_set_dma_output_len(0);
        hash_set_dma_mode();
#endif
    }
    else
    {}

    memset_((uint8_t *)tmp_ctx, 0, sizeof(hmac_ctx_st));

    return ret;
}


/* function: dma hmac update message(for sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of block byte length of HASH
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t hmac_sha3_dma_update_blocks(hmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
#else
static uint32_t hmac_sha3_dma_update_blocks(hmac_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes)
#endif
{
    uint32_t ret;

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    ret = hash_dma_update_blocks(ctx->hash_dma_ctx, msg_h, msg_l, msg_bytes);
#else
    ret = hash_dma_update_blocks(ctx->hash_dma_ctx, msg, msg_bytes);
#endif

    return ret;
}


/* function: dma hmac message update done, get the hmac(for sha3)
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the last message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t hmac_sha3_dma_final(hmac_dma_ctx_st *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l)
#else
static uint32_t hmac_sha3_dma_final(hmac_dma_ctx_st *ctx, const uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *mac)
#endif
{
    hash_alg_e alg;
    uint32_t block_byte_len, digest_byte_len;
    uint32_t i;
    uint32_t ret;
    hash_ctx_st tmp_ctx[1];
    uint8_t *mac_remap;

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((0U == mac_h) && (0U == mac_l))
#else
    if(NULL == mac)
#endif
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
        //backup before ctx->hash_dma_ctx cleaned
        alg = ctx->hash_dma_ctx->alg;
        block_byte_len = CAST2UINT32(ctx->hash_dma_ctx->block_word_len)<<2;
        digest_byte_len = CAST2UINT32(ctx->hash_dma_ctx->digest_byte_len);

        //get K0 ^ opad before ctx->hash_dma_ctx cleaned
        for(i=0; i<ctx->hash_dma_ctx->block_word_len; i++)
        {
            ctx->K0[i] ^= HMAC_IPAD_XOR_OPAD;
        }

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = hash_dma_final(ctx->hash_dma_ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, mac_h, mac_l);
#else
        ret = hash_dma_final(ctx->hash_dma_ctx, remainder_msg, remainder_bytes, mac);
#endif
        if(HASH_SUCCESS == ret)
        {
            ret = hash_init(tmp_ctx, alg);
            if(HASH_SUCCESS == ret)
            {
                ret = hash_update(tmp_ctx, (uint8_t *)(ctx->K0), block_byte_len);
            }
            else
            {}
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            mac_remap = lib_addr_arch32_lock_remap(mac_h, mac_l, 0U);
#else
            mac_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)mac, 0U);
#endif
            //tmp_iterator may not be accessed by bytes
            memcpy_(ctx->K0, mac_remap, digest_byte_len);
            ret = hash_update(tmp_ctx, (uint8_t *)(ctx->K0), digest_byte_len);
            if(HASH_SUCCESS == ret)
            {
                ret = hash_final(tmp_ctx, mac_remap);
            }
            else
            {}
            
            lib_addr_arch32_unlock_remap();
        }
        else
        {}
    }

    memset_((uint8_t *)ctx, 0, sizeof(hmac_dma_ctx_st));
    memset_((uint8_t *)tmp_ctx, 0, sizeof(hash_ctx_st));

    return ret;
}
#endif


/* function: init dma hmac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. here hmac is not for SHA3.
 */
static uint32_t hmac_non_sha3_dma_init(hmac_dma_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes, HASH_CALLBACK callback)
{
    uint32_t ret;

    ret = hmac_init_step1(alg, key, key_bytes);
    if(HASH_SUCCESS == ret)
    {
        //clear and init the context
        memset_((uint8_t *)ctx, 0, sizeof(hmac_dma_ctx_st));

        ctx->hash_dma_ctx->alg          = alg;
        ctx->hash_dma_ctx->block_word_len    = hash_get_block_word_len(alg);
        ctx->hash_dma_ctx->callback          = callback;
        ctx->sp_key_idx                     = sp_key_idx;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        ctx->hash_dma_ctx->hfe_mode          = HMAC_MODE;
        ctx->hash_dma_ctx->iterator_word_len = hash_get_iterator_word_len(alg);
        ctx->hash_dma_ctx->first_update_flag = (uint8_t)1;
#endif

        ret = hmac_init_step2(alg, key, key_bytes, CAST2UINT32(ctx->hash_dma_ctx->block_word_len)<<2,
                hash_get_iterator_word_len(alg), CAST2UINT32(hash_get_digest_word_len(alg))<<2,
                &ctx->key_len_flag, ctx->K0, ctx->sp_key_idx);
        if(HASH_SUCCESS == ret)
        {
            hash_set_dma_mode();
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: dma hmac update message
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of block byte length of HASH
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 *     2. here hmac is not for SHA3.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t hmac_non_sha3_dma_update_blocks(hmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
#else
static uint32_t hmac_non_sha3_dma_update_blocks(hmac_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes)
#endif
{
    uint32_t ret;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    ret = hmac_key_state_recover(ctx->hash_dma_ctx->alg, ctx->key_len_flag, ctx->K0, 
                CAST2UINT32(ctx->hash_dma_ctx->block_word_len)<<2, ctx->hash_dma_ctx->iterator_word_len, ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
    if(HASH_SUCCESS == ret)
    {
        hash_set_dma_mode();
        hash_set_last_block(0);
        hash_set_dma_output_len(0);
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = hash_dma_update_blocks(ctx->hash_dma_ctx, msg_h, msg_l, msg_bytes);
#else
        ret = hash_dma_update_blocks(ctx->hash_dma_ctx, msg, msg_bytes);
#endif
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    }
    else
    {}
#endif

    return ret;
}


/* function: dma hmac message update done, get the hmac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the last message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 *     2. here hmac is not for SHA3.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t hmac_non_sha3_dma_final(hmac_dma_ctx_st *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l)
#else
static uint32_t hmac_non_sha3_dma_final(hmac_dma_ctx_st *ctx, const uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *mac)
#endif
{
    uint32_t ret;

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((0U == mac_h) && (0U == mac_l))
#else
    if(NULL == mac)
#endif
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        ret = hmac_key_state_recover(ctx->hash_dma_ctx->alg, ctx->key_len_flag, ctx->K0, 
                    CAST2UINT32(ctx->hash_dma_ctx->block_word_len)<<2, ctx->hash_dma_ctx->iterator_word_len, ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
        if(HASH_SUCCESS == ret)
        {
            hash_set_dma_mode();
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_final(ctx->hash_dma_ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, mac_h, mac_l);
#else
            ret = hash_dma_final(ctx->hash_dma_ctx, remainder_msg, remainder_bytes, mac);
#endif
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        }
        else
        {}
#endif
    }

    memset_((uint8_t *)ctx, 0, sizeof(hmac_dma_ctx_st));

    return ret;
}


/* function: init dma hmac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     alg ------------------- input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. if cfg not support secure port, please set key is NULL when hmac without key
 *     3. if cfg support secure port, hmac will use secure port key when parameter key is NULL, 
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
uint32_t hmac_dma_init(hmac_dma_ctx_st *ctx, hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, 
        uint32_t key_bytes, HASH_CALLBACK callback)
{
    uint32_t ret = HASH_SUCCESS;
    uint32_t actual_key_bytes = key_bytes;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
#ifndef HMAC_SECURE_PORT_FUNCTION
        if(NULL == key)
        {
            actual_key_bytes = 0;
        }
        else
        {}

        ctx->is_sp_key = 0U;
#else
        if(NULL == key)
        {
            ctx->is_sp_key = 1U;
            ctx->sp_key_bytes = key_bytes;
            ctx->sp_key_idx = sp_key_idx;
        }
        else
        {
            ctx->is_sp_key = 0U;
        }
#endif
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((alg >= HASH_SHA3_224) && (alg <= HASH_SHA3_512))
        {
            if(NULL == key)
            {
                ret = HASH_INPUT_INVALID; //IP not diretlly support hmac_sha3
            }
            else
            {
                ret = hmac_sha3_dma_init(ctx, alg, key, actual_key_bytes, callback);
            }
        }
        else
        {
#endif
            ret = hmac_non_sha3_dma_init(ctx, alg, key, sp_key_idx, actual_key_bytes, callback);
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
    }

    return ret;
}


/* function: dma hmac update message
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of block byte length of HASH
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_dma_update_blocks(hmac_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
#else
uint32_t hmac_dma_update_blocks(hmac_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes)
#endif
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((ctx->hash_dma_ctx->alg >= HASH_SHA3_224) && (ctx->hash_dma_ctx->alg <= HASH_SHA3_512))
        {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            //IP not diretlly support hmac_sha3
            hash_hmac_disable_secure_port();
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_sha3_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
#else
            ret = hmac_sha3_dma_update_blocks(ctx, msg, msg_bytes);
#endif
        }
        else
        {
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_non_sha3_dma_update_blocks(ctx, msg_h, msg_l, msg_bytes);
#else
            ret = hmac_non_sha3_dma_update_blocks(ctx, msg, msg_bytes);
#endif
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
    }
    
    return ret;
}


/* function: dma hmac message update done, get the hmac
 * parameters:
 *     ctx ------------------------ input, hmac_dma_ctx_st context pointer
 *     remainder_msg -------------- input, message
 *     remainder_bytes ------------ input, byte length of the last message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_dma_final(hmac_dma_ctx_st *ctx, uint32_t remainder_msg_h, uint32_t remainder_msg_l, 
        uint32_t remainder_bytes, uint32_t mac_h, uint32_t mac_l)
#else
uint32_t hmac_dma_final(hmac_dma_ctx_st *ctx, const uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *mac)
#endif
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        if((ctx->hash_dma_ctx->alg >= HASH_SHA3_224) && (ctx->hash_dma_ctx->alg <= HASH_SHA3_512))
        {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            //IP not diretlly support hmac_sha3
            hash_hmac_disable_secure_port();
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_sha3_dma_final(ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, mac_h, mac_l);
#else
            ret = hmac_sha3_dma_final(ctx, remainder_msg, remainder_bytes, mac);
#endif
        }
        else
        {
#endif
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_non_sha3_dma_final(ctx, remainder_msg_h, remainder_msg_l, remainder_bytes, mac_h, mac_l);
#else
            ret = hmac_non_sha3_dma_final(ctx, remainder_msg, remainder_bytes, mac);
#endif
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || \
    defined(SUPPORT_HASH_SHA3_512))
        }
#endif
    }

    return ret;
}


/* function: dma hmac input key and message, get the hmac
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     key ------------------------ input, key
 *     sp_key_idx ----------------- input, index of secure port key
 *     key_bytes ------------------ input, key byte length
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. if cfg not support secure port, please set key is NULL when hmac without key
 *     3. if cfg support secure port, hmac will use secure port key when parameter key is NULL, 
 *        please set key is not NULL and key_bytes is 0 when hmac without key
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_dma(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t msg_h, 
        uint32_t msg_l, uint32_t msg_bytes, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback)
#else
uint32_t hmac_dma(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, uint32_t *msg, 
        uint32_t msg_bytes, uint32_t *mac, HASH_CALLBACK callback)
#endif
{
    uint32_t ret;
    hmac_dma_ctx_st ctx[1];

    ret = hmac_dma_init(ctx, alg, key, sp_key_idx, key_bytes, callback);
    if(HASH_SUCCESS == ret)
    {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = hmac_dma_final(ctx, msg_h, msg_l, msg_bytes, mac_h, mac_l);
#else
        ret = hmac_dma_final(ctx, msg, msg_bytes, mac);
#endif
    }
    else
    {}

    return ret;
}


#ifdef SUPPORT_HASH_DMA_NODE
/* function: dma hmac input key and message, get the hmac(node style)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
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
 *     2. please make sure alg is valid
 *     3. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 *     4. for every node or segment except the last, its message length must be a multiple of block length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hmac_dma_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
        const hash_dma_node_st *node, uint32_t node_num, uint32_t mac_h, uint32_t mac_l, HASH_CALLBACK callback)
#else
uint32_t hmac_dma_node_steps(hash_alg_e alg, const uint8_t *key, uint16_t sp_key_idx, uint32_t key_bytes, 
		const hash_dma_node_st *node, uint32_t node_num, uint32_t *mac, HASH_CALLBACK callback)
#endif
{
    uint32_t i, ret;
    hmac_dma_ctx_st ctx[1];

    ret = hmac_dma_init(ctx, alg, key, sp_key_idx, key_bytes, callback);
    if(HASH_SUCCESS == ret)
    {
        for(i=0; i<(node_num-1U); i++)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_dma_update_blocks(ctx, node[i].msg_addr_h, node[i].msg_addr_l, node[i].msg_bytes);
#else
            ret = hmac_dma_update_blocks(ctx, node[i].msg_addr, node[i].msg_bytes);
#endif
            if(HASH_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }

        if(HASH_SUCCESS == ret)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hmac_dma_final(ctx, node[i].msg_addr_h, node[i].msg_addr_l, node[i].msg_bytes, mac_h, mac_l);
#else
            ret = hmac_dma_final(ctx, node[i].msg_addr, node[i].msg_bytes, mac);
#endif
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
