
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/crypto_common/utility.h"


#ifdef SUPPORT_PBKDF2


#ifdef PBKDF2_HIGH_SPEED
/* function: pbkdf2 backup hmac ctx
 * parameters:
 *     ctx_bak -------------------- output, hmac ctx
 *     ctx ------------------------ input, hmac ctx to be backup
 *     iterator ------------------- output, if is not NULL, 
 *                                  means for SHA3, iterator of K0 XOR IPAD
 * return: 
 * caution:
 *     1. if hash algorithm is SHA3, it needs iterator, otherwise not. since hmac-sha3 is not 
 *        supported by hardware.
 */
static void pbkdf2_hmac_backup(hmac_ctx_st *ctx_bak, const hmac_ctx_st *ctx, uint32_t *iterator)
{
    memcpy_(ctx_bak, ctx, sizeof(hmac_ctx_st));

    if(NULL != iterator)
    {
        if((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512))
        {
            hash_get_iterator((uint8_t *)iterator, ctx->hash_ctx->iterator_word_len);
        }
        else
        {}
    }
    else
    {}
}
#endif


/* function: pbkdf2 recover hmac ctx
 * parameters:
 *     ctx ------------------------ output, hmac ctx to be recover
 *     ctx_bak -------------------- input, hmac ctx
 *     iterator ------------------- output, if is not NULL, 
 *                                  means for SHA3, iterator of K0 XOR IPAD
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. if hash algorithm is SHA3, it needs iterator, otherwise not. since hmac-sha3 is not 
 *        supported by hardware.
 */
static uint32_t pbkdf2_hmac_recover(hmac_ctx_st *ctx, const hmac_ctx_st *ctx_bak, const uint32_t *iterator)
{
    uint32_t ret = HASH_SUCCESS;

    memcpy_(ctx, ctx_bak, sizeof(hmac_ctx_st));

    if((ctx->hash_ctx->alg >= HASH_SHA3_224) && (ctx->hash_ctx->alg <= HASH_SHA3_512))
    {
        hash_set_cpu_mode();
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_cpu_interruption();
        hash_set_last_block(0);//set not the last block
        hash_set_alg(ctx->hash_ctx->alg);
        hash_update_config();

        hash_set_iterator(iterator, ctx->hash_ctx->iterator_word_len);
    }
    else
    {
        ret = hmac_key_state_recover(ctx->hash_ctx->alg, ctx->key_len_flag, ctx->K0,
            (uint32_t)(ctx->hash_ctx->block_byte_len), (uint32_t)(ctx->hash_ctx->iterator_word_len), ctx->is_sp_key, ctx->sp_key_idx, ctx->sp_key_bytes);
    }

    return ret;
}


/* function: pbkdf2 internal function(U = hmac(Password, Salt||counter) or U = hmac(Password, hmac))
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 *     mac ------------------------ output, hmac
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.use for pbkdf2, get U1,U2 ...
 */
static uint32_t pbkdf2_hmac_update_get_mac(hmac_ctx_st *ctx, const uint8_t *msg1, uint32_t msg1_bytes, 
    const uint8_t *msg2, uint32_t msg2_bytes, uint8_t *mac)
{
    uint32_t ret;

    ret = hash_update(ctx->hash_ctx, msg1, msg1_bytes);

    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(ctx->hash_ctx, msg2, msg2_bytes);
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        ret = hmac_final(ctx, (uint8_t *)mac);
    }
    else
    {}

    return ret;
}


/* function: pbkdf2 internal function(out = U1 xor U2 xor ... Uc)
 * parameters:
 *     ctx ------------------------ input, hmac_ctx_st context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 *     iterator ------------------- output, if is not NULL, 
 *                                  means for SHA3, iterator of K0 XOR IPAD
 *     counter  ------------------- input, counter of 4 bytes
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt, it could be 0
 *     iter ----------------------- input, iteration times
 *     out ------------------------ output, derived key
 *     out_bytes ------------------ input, byte length of derived key
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1.if open the macro PBKDF2_HIGH_SPEED, the ctx means already init, 
 */
#ifdef PBKDF2_HIGH_SPEED
static uint32_t pbkdf2_hmac_calc_f(const hmac_ctx_st *ctx, hash_alg_e alg, const uint32_t *iterator, const uint8_t counter[4], 
    const uint8_t *salt, uint32_t salt_bytes, uint32_t iter, uint8_t *out, uint32_t out_bytes)
#else
static uint32_t pbkdf2_hmac_calc_f(hmac_ctx_st *ctx, hash_alg_e alg, const uint8_t *pwd, uint16_t sp_key_idx, uint32_t pwd_bytes, 
    uint32_t *iterator, uint8_t counter[4], const uint8_t *salt, uint32_t salt_bytes, uint32_t iter, uint8_t *out, uint32_t out_bytes)
#endif
{
    uint32_t result[HASH_DIGEST_MAX_WORD_LEN];
    uint32_t digest[HASH_DIGEST_MAX_WORD_LEN];
    uint32_t digest_words, digest_bytes;
    hmac_ctx_st *ctx_ptr;
    uint32_t ret = HASH_SUCCESS;
    uint32_t i,j;

#ifdef PBKDF2_HIGH_SPEED
    hmac_ctx_st ctx_tmp[1];
    ctx_ptr = ctx_tmp;
#else
    ctx_ptr = ctx;
#endif

    digest_words = hash_get_digest_word_len(alg);
    digest_bytes = digest_words<<2;

    //get U1
#ifdef PBKDF2_HIGH_SPEED
    ret = pbkdf2_hmac_recover(ctx_ptr, ctx, iterator);
#else
    ret = hmac_init(ctx, alg, pwd, sp_key_idx, pwd_bytes);
#endif

    if(HASH_SUCCESS == ret)
    {
        ret = pbkdf2_hmac_update_get_mac(ctx_ptr, salt, salt_bytes, counter, 4, (uint8_t *)result);
    }
    else
    {}

    //get U1 xor U2 xor ... xor Uc
    if(HASH_SUCCESS == ret)
    {
        uint32_copy(digest, result, digest_words);

        for(i=1U; i<iter; i++)
        {
#ifdef PBKDF2_HIGH_SPEED
            ret = pbkdf2_hmac_recover(ctx_ptr, ctx, iterator);
#else
            ret = hmac_init(ctx, alg, pwd, sp_key_idx, pwd_bytes);
#endif
            if(HASH_SUCCESS == ret)
            {
                ret = pbkdf2_hmac_update_get_mac(ctx_ptr, (const uint8_t *)digest, digest_bytes, NULL, 0, (uint8_t *)digest);
            }
            else
            {}

            if(HASH_SUCCESS == ret)
            {
                for(j=0; j<digest_words; j++)
                {
                    result[j] ^= digest[j];
                }
            }
            else
            {
                break;
            }  
        }
        
        if(HASH_SUCCESS == ret)
        {
            memcpy_(out, result, out_bytes);
        }
        else
        {}  
    }
    else
    {}

    return ret;
}


/* function: pbkdf2 function(using hmac as PRF)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     pwd ------------------------ input, password, as the key of hmac
 *     sp_key_idx ----------------- input, index of secure port key(password)
 *     pwd_bytes ------------------ input, byte length of password, it could be 0
 *     salt ----------------------- input, salt
 *     salt_bytes ----------------- input, byte length of salt, it could be 0
 *     iter ----------------------- input, iteration times
 *     out ------------------------ output, derived key
 *     out_bytes ------------------ input, byte length of derived key
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t pbkdf2_hmac(hash_alg_e alg, const uint8_t *pwd, uint16_t sp_key_idx, uint32_t pwd_bytes, 
        const uint8_t *salt, uint32_t salt_bytes, uint32_t iter, uint8_t *out, uint32_t out_bytes)
{
    uint32_t digest_words, digest_bytes, tmp_bytes;
    uint8_t counter[4] = {0,0,0,1};
    uint32_t actual_pwd_bytes = pwd_bytes;
    uint32_t actual_salt_bytes = salt_bytes;
    uint32_t remainder_out_bytes = out_bytes;
    uint8_t *current_out = out;
    uint32_t ret = HASH_SUCCESS;

#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD)) && (defined(SUPPORT_HASH_SHA3_224) || \
    defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512)))
    uint32_t iterator[HASH_ITERATOR_MAX_WORD_LEN];
#endif

    hmac_ctx_st ctx[1];

#ifdef PBKDF2_HIGH_SPEED
    hmac_ctx_st ctx_bak[1];
#endif

    if(HASH_SUCCESS != check_hash_alg(alg))
    {
        ret = HASH_INPUT_INVALID;
    }
    else if(NULL == out)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
        //handle other
    }

    if(HASH_SUCCESS == ret)
    {
#ifndef HMAC_SECURE_PORT_FUNCTION
        if(NULL == pwd)
        {
            actual_pwd_bytes = 0;
        }
        else
        {}
#endif

        if(NULL == salt)
        {
            actual_salt_bytes = 0;
        }
        else
        {}

        digest_words = hash_get_digest_word_len(alg);
        digest_bytes = 4U*digest_words;

#ifdef PBKDF2_HIGH_SPEED
        ret = hmac_init(ctx, alg, pwd, sp_key_idx, actual_pwd_bytes);
        if(HASH_SUCCESS == ret)
        {
#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD)) && (defined(SUPPORT_HASH_SHA3_224) || \
        defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512)))
            pbkdf2_hmac_backup(ctx_bak, ctx, iterator);
#else
            pbkdf2_hmac_backup(ctx_bak, ctx, NULL);
#endif
#endif
            while(0U != remainder_out_bytes)
            {
                if(remainder_out_bytes > digest_bytes)
                {
                    tmp_bytes = digest_bytes;
                }
                else
                {
                    tmp_bytes = remainder_out_bytes;
                }

#ifdef PBKDF2_HIGH_SPEED
#if ((!defined(CONFIG_HASH_SUPPORT_MUL_THREAD)) && (defined(SUPPORT_HASH_SHA3_224) || \
            defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512)))
                ret = pbkdf2_hmac_calc_f(ctx_bak, alg, iterator, counter, salt, actual_salt_bytes, iter, current_out, tmp_bytes);
#else
                ret = pbkdf2_hmac_calc_f(ctx_bak, alg, NULL, counter, salt, actual_salt_bytes, iter, current_out, tmp_bytes);
#endif
#else
                ret = pbkdf2_hmac_calc_f(ctx, alg, pwd, sp_key_idx, actual_pwd_bytes, 
                          NULL, counter, salt, actual_salt_bytes, iter, current_out, tmp_bytes);
#endif
                if(HASH_SUCCESS == ret)
                {
                    //add 1
                    (void)uint8_big_num_big_endian_add_little(counter, 4, 1, 1);

                    current_out = &(current_out[tmp_bytes]);
                    remainder_out_bytes -= tmp_bytes;
                }
                else
                {
                   break;
                }
            }
#ifdef PBKDF2_HIGH_SPEED
        }
#endif
    }
    else
    {}

    return ret;
}
#endif



#ifdef SUPPORT_ANSI_X9_63_KDF
#ifdef SUPPORT_HASH_NODE

static uint32_t ansi_x9_63_kdf_digest_remainder(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *counter, 
        uint8_t *k, uint32_t k_bytes, uint8_t *digest, uint32_t *digest_remainder_bytes);

static uint32_t ansi_x9_63_kdf_internal_check_zero(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, uint8_t *out, uint32_t out_bytes, uint32_t check_whether_zero, uint8_t *zero_check);

static uint32_t ansi_x9_63_kdf_parameter_check(const hash_node_st *node, const uint8_t *out, uint32_t alg_digest_bytes);


static uint32_t ansi_x9_63_kdf_parameter_check(const hash_node_st *node, const uint8_t *out, uint32_t alg_digest_bytes)
{
    uint32_t ret = HASH_SUCCESS;

    if(0U == alg_digest_bytes)
    {
        ret = HASH_INPUT_INVALID;
    }
    else if(NULL == out)
    {
        ret = HASH_INPUT_INVALID;
    }
    else if(NULL == node)
    {
        ret = HASH_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    return ret;
}


/* function: out = ansi_x9_63_kdf(msg1||msg2||...||counter||..., out_bytes).
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     node ----------------------- input, hash_node_st struct pointer
 *     node_num ------------------- input, number of hash_node_st, or number of message pieces
 *     counter_idx ---------------- input, index of the counter of 4 bytes in node array
 *     out ------------------------ output, out
 *     out_bytes ------------------ input, byte length of output
 *     check_whether_zero --------- input, 0(not check whether kdf output is zero or not),
 *                                         other(check whether kdf output is zero or not).
 *     zero_check ----------------- input&output, result of out whether is all zero
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. node[counter_idx] holds counter of 4 bytes.
 *     3. here out_bytes must be a multiple of alg digest byte length.
 */
static uint32_t ansi_x9_63_kdf_internal_check_zero(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, uint8_t *out, uint32_t out_bytes, uint32_t check_whether_zero, uint8_t *zero_check)
{
    uint32_t ret = HASH_SUCCESS;

    ret = ansi_x9_63_kdf_internal(alg,node, node_num, counter, out, out_bytes);
    if(0U != check_whether_zero)
    {
        if(0U != uint8_BigNum_Check_Zero(out, out_bytes))
        {
            //here secure scheme
            (*zero_check) = (uint8_t)0;        
        }
        else
        {
            (*zero_check) = (uint8_t)1;
        }
    }
    else
    {}

    return ret;
}


/* function: out = ansi_x9_63_kdf(msg1||msg2||...||counter||... , out_bytes),        ---- if in is NULL
 *           out = ansi_x9_63_kdf(msg1||msg2||...||counter||... , out_bytes) XOR in, ---- if in is not NULL
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     node ------------------ input, hash_node_st struct pointer
 *     node_num ------------------- input, number of hash_node_st, or number of message pieces
 *     counter_idx ---------------- input, index of the counter of 4 bytes in node array
 *     in ------------------------- input, input message, if no input message, please set this para as NULL
 *     out ------------------------ output, if in is NULL, out = kdf(...), otherwise out = kdf(...) XOR in.
 *     out_bytes ------------------ input, byte length of out, if in is not NULL, this is also byte length of in
 *     check_whether_zero --------- input, 0(not check whether kdf output is zero or not),
 *                                         other(check whether kdf output is zero or not).
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. node[counter_idx] holds counter of 4 bytes.
 *     3. if in is NULL, out = kdf(...), otherwise out = kdf(...) XOR in.
 *     4. for ansi x9.63, initial counter is 1; for PKCS#1 RSA MGF, initial counter is 0.
 */
uint32_t ansi_x9_63_kdf_node_with_xor_in(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, const uint8_t *in, uint8_t *out, uint32_t out_bytes, uint32_t check_whether_zero)
{
    uint8_t digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;
    uint32_t remainder_bytes;
    uint32_t round_num;
    uint32_t i, ret = HASH_SUCCESS;
    uint8_t zero_check = 0;
    uint8_t is_zero = 0;

    ret = ansi_x9_63_kdf_parameter_check(node, out, digest_bytes);
    
    if((0U != out_bytes) && (HASH_SUCCESS == ret))
    {
        round_num = out_bytes/digest_bytes;

        for(i = 0U; i < round_num; i++)
        {
            if(NULL == in)
            {
                ret = ansi_x9_63_kdf_internal_check_zero(alg, node, node_num, counter, &out[i*digest_bytes], 
                    digest_bytes, check_whether_zero, &is_zero);

                zero_check |= is_zero;
            }
            else
            {
                ret = ansi_x9_63_kdf_internal_check_zero(alg, node, node_num, counter, digest, digest_bytes, 
                    check_whether_zero, &is_zero);

                zero_check |= is_zero;

                if(HASH_SUCCESS == ret)
                {
                    uint8_XOR(&in[i*digest_bytes], digest, &out[i*digest_bytes], digest_bytes);
                }
                else
                {}
            }

            if(HASH_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }

        remainder_bytes = out_bytes - (round_num*digest_bytes);
        if((HASH_SUCCESS == ret) && (0U != remainder_bytes))
        {
            ret = ansi_x9_63_kdf_internal_check_zero(alg, node, node_num, counter, digest, digest_bytes, 
                check_whether_zero, &is_zero);

            zero_check |= is_zero;

            if(HASH_SUCCESS == ret)
            {
                if(NULL == in)
                {
                    memcpy_(&out[i*digest_bytes], digest, remainder_bytes);
                }
                else
                {
                    uint8_XOR(&in[i*digest_bytes], digest, &out[i*digest_bytes], remainder_bytes);
                }
            }
            else
            {}
        }

        if((HASH_SUCCESS == ret) && (0U != check_whether_zero) && (0U == zero_check))
        {
            memset_(out, 0, out_bytes);
            ret = HASH_OUTPUT_ZERO_ALL;
        }
        else
        {}
    }

    return ret;
}


/* function: key = ansi_x9_63_kdf(msg1||msg2||...||counter||..., key_bytes).
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 *     node ------------------ input, hash_node_st struct pointer
 *     node_num ------------------- input, number of hash_node_st, or number of message pieces
 *     counter_idx ---------------- input, index of the counter of 4 bytes in node array
 *     key ------------------------ output, key
 *     key_bytes ------------------ input, byte length of output key
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. node[counter_idx] holds counter of 4 bytes.
 *     3. here key_bytes must be a multiple of alg digest byte length.
 */
uint32_t ansi_x9_63_kdf_internal(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, uint8_t *key, uint32_t key_bytes)
{
    uint32_t digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;
    uint32_t round_num;
    uint32_t i, ret = HASH_SUCCESS;
    
    ret = ansi_x9_63_kdf_parameter_check(node, key, digest_bytes);

    if(HASH_SUCCESS == ret)
    {
        round_num = key_bytes/digest_bytes;

        for(i = 0U; i < round_num; i++)
        {
            ret = hash_node_steps(alg, node, node_num, &(key[(i*digest_bytes)]));
            if(HASH_SUCCESS == ret)
            {
                (void)uint8_big_num_big_endian_add_little(counter, 4, 1, 1);
            }
            else
            {}
        }
    }
    
    return ret;
}


/* function: get k = ansi_x9_63_kdf(msg1||msg2||...||counter||... , k_bytes)
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 *     node ------------------ input, hash_node_st struct pointer
 *     node_num ------------------- input, number of hash_node_st, or number of message pieces
 *     counter_idx ---------------- input, index of the counter of 4 bytes in node array
 *     k -------------------------- output, key
 *     k_bytes -------------------- input, byte length of k
 *     digest --------------------- input, digest buffer
 *     digest_remainder_bytes ----- output, remainder bytes length of digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. node[counter_idx] holds counter of 4 bytes.
 */
static uint32_t ansi_x9_63_kdf_digest_remainder(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *counter, 
        uint8_t *k, uint32_t k_bytes, uint8_t *digest, uint32_t *digest_remainder_bytes)
{
    uint32_t blocks_bytes, remainder_bytes;
    uint32_t digest_bytes;
    uint32_t ret;

    digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;
    blocks_bytes = (k_bytes/digest_bytes)*digest_bytes;

    ret = ansi_x9_63_kdf_internal(alg, node, node_num, counter, k, blocks_bytes);
    if(HASH_SUCCESS == ret)
    {
        remainder_bytes = k_bytes - blocks_bytes;
        if(0U != remainder_bytes)
        {
            ret = ansi_x9_63_kdf_internal(alg, node, node_num, counter, digest, digest_bytes);
            if(HASH_SUCCESS == ret)
            {
                memcpy_(&(k[blocks_bytes]), digest, remainder_bytes);

                if(NULL != digest_remainder_bytes)
                {
                    *digest_remainder_bytes = digest_bytes - remainder_bytes;
                }
                else
                {}
            }
            else
            {}
        }
        else
        {
            if(NULL != digest_remainder_bytes)
            {
                *digest_remainder_bytes = 0U;
            }
            else
            {}
        }
    }

    return ret;

}


/* function: k1||k2 = ansi_x9_63_kdf(msg1||msg2||...||counter||... , k1_bytes + k2_bytes).
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 *     node ------------------ input, hash_node_st struct pointer
 *     node_num ------------------- input, number of hash_node_st, or number of message pieces
 *     counter_idx ---------------- input, index of the counter of 4 bytes in node array
 *     k1 ------------------------- output, k1 part
 *     k1_bytes ------------------- input, byte length of k1
 *     k2 ------------------------- output, k2 part
 *     k2_bytes ------------------- input, byte length of k2
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. node[counter_idx] holds counter of 4 bytes.
 *     3. k1 can not be NULL, but k2 can.
 */
uint32_t ansi_x9_63_kdf_node(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *counter, 
        uint8_t *k1, uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes)
{
    uint8_t digest[HASH_DIGEST_MAX_WORD_LEN<<2];
    uint32_t digest_bytes = (uint32_t)hash_get_digest_word_len(alg)<<2;
    uint8_t *current_k2 = k2;
    uint32_t remainder_k2_bytes = k2_bytes;
    uint8_t is_finished = (uint8_t)0;
    uint32_t ret = HASH_SUCCESS;
    uint32_t digest_remainder_bytes;

    ret = ansi_x9_63_kdf_parameter_check(node, k1, digest_bytes);
    
    if(HASH_SUCCESS == ret)
    {
        //get k1
        ret = ansi_x9_63_kdf_digest_remainder(alg, node, node_num, counter, k1, k1_bytes, digest, &digest_remainder_bytes);

        //get k2
        if((HASH_SUCCESS == ret) && (NULL != k2))
        {
            if(k2_bytes <= digest_remainder_bytes)
            {
                memcpy_(k2, &(digest[digest_bytes - digest_remainder_bytes]), k2_bytes);

                is_finished = (uint8_t)1;
            }
            else
            {
                memcpy_(k2, &(digest[digest_bytes - digest_remainder_bytes]), digest_remainder_bytes);
                current_k2 = &(k2[digest_remainder_bytes]);
                remainder_k2_bytes -= digest_remainder_bytes;
            }

            if((uint8_t)0 == is_finished)
            {
                ret = ansi_x9_63_kdf_digest_remainder(alg, node, node_num, counter, current_k2, remainder_k2_bytes, digest, NULL);
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* function: k1||k2 = ansi_x9_63_kdf(Z||counter||shared_info , k1_bytes + k2_bytes).
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     Z -------------------------- input, byte string
 *     Z_bytes -------------------- input, byte length of Z
 *     shared_info ---------------- input, shared info
 *     shared_info_bytes ---------- input, byte length of shared info
 *     k1 ------------------------- output, k1 part
 *     k1_bytes ------------------- input, byte length of k1
 *     k2 ------------------------- output, k2 part
 *     k2_bytes ------------------- input, byte length of k2
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. k1 can not be NULL, but k2 can.
 */
uint32_t ansi_x9_63_kdf(hash_alg_e alg, const uint8_t *Z, uint32_t Z_bytes, const uint8_t *shared_info, 
        uint32_t shared_info_bytes, uint8_t *k1, uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes)
{
    uint8_t counter[4] = {0x00,0x00,0x00,0x01};      //init count = 1
    hash_node_st node[3];

    node[0].msg_addr  = Z;
    node[0].msg_bytes = Z_bytes;
    node[1].msg_addr  = counter;
    node[1].msg_bytes = 4U;
    node[2].msg_addr  = shared_info;
    node[2].msg_bytes = shared_info_bytes;

    return ansi_x9_63_kdf_node(alg, node, 3, counter, k1, k1_bytes, k2, k2_bytes);
}
#endif
#endif

