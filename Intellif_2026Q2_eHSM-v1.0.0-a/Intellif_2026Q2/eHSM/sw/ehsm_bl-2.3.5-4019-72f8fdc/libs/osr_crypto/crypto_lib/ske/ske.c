
#include "../../crypto_include/ske/ske.h"
#include "../../crypto_include/crypto_common/utility.h"


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
/* function: a = a + b
 * parameters:
*     a -------------------------- input, big number a, uint32_t big-endian, ske iv
 *     a_words -------------------- input, word length of a
 *     b -------------------------- input, uint32_t integer b
 *     is_secure ------------------ input, is secure implementation, 0(not), other(yes)
 * return: 0(not overflow),1(overflow)
 * caution:
 *     1. this is mainly used for ske ctr mode
 */
static uint32_t ske_counter_add_u32(uint32_t *a, uint32_t a_words, uint32_t b,
	uint8_t is_secure)
{
	uint32_t ret;

	if (b <= 0xFFU)
	{
		ret = uint8_big_num_big_endian_add_little((uint8_t *)a, a_words<<2, (uint8_t)b, 1);
	}
	else
	{
		reverse_byte_array((uint8_t *)a, (uint8_t *)a, a_words << 2);
		ret = uint32_big_num_little_endian_add_little(a, a_words, b, is_secure);
		reverse_byte_array((uint8_t *)a, (uint8_t *)a, a_words << 2);
	}

	return ret;
}
#endif


/* function: clear the last (16-bytes) of the block in(16 bytes)
 * parameters:
 *     in ------------------------- input, one block buffer(128bits, for AES/SM4 GCM, CCM mode)
 *     bytes ---------------------- input, real bytes of in, must be in[1,16]
 * return: none
 * caution:
 *     1. this function is for GCM,CCM mode of DMA.
 */
void ske_clear_block_tail(uint32_t *in, uint32_t bytes)
{
    uint32_t i;
    uint32_t tmp;

    i = bytes/4U;
    tmp = bytes&3U;

    if(0U != tmp)
    {
#ifdef SKE_CPU_BIG_ENDIAN
        in[i] &= 0xFFFFFFFF<<(32-tmp*8);
#else
        in[i] &= (((uint32_t)1)<<(tmp*8U)) - 1U;
#endif
        i++;
    }
    else
    {}

    while(i < 4U)
    {
        in[i]=0;
        i++;
    }
}


/* function: check whether the ske algorithm is valid or not
 * parameters:
 *     ske_alg -------------------- input, specific ske algorithm
 * return: SKE_SUCCESS(valid), other(invalid)
 * caution:
 */
uint32_t ske_check_alg(ske_alg_e ske_alg)
{
    uint32_t ret;

    switch(ske_alg)
    {
#ifdef SUPPORT_SKE_DES
    case SKE_ALG_DES:
#endif

#ifdef SUPPORT_SKE_TDES_128
    case SKE_ALG_TDES_128:
#endif

#ifdef SUPPORT_SKE_TDES_192
    case SKE_ALG_TDES_192:
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    case SKE_ALG_TDES_EEE_128:
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
    case SKE_ALG_TDES_EEE_192:
#endif

#ifdef SUPPORT_SKE_AES_128
    case SKE_ALG_AES_128:
#endif

#ifdef SUPPORT_SKE_AES_192
    case SKE_ALG_AES_192:
#endif

#ifdef SUPPORT_SKE_AES_256
    case SKE_ALG_AES_256:
#endif

#ifdef SUPPORT_SKE_SM4
    case SKE_ALG_SM4:
#endif
        ret = SKE_SUCCESS;
        break;

    default:
        ret = SKE_INPUT_INVALID;
        break;
    }

    return ret;
}


/* function: check whether the ske algorithm mode is valid or not
 * parameters:
 *     ske_alg -------------------- input, specific ske algorithm
 *     ske_mode ------------------- input, specific ske algorithm mode
 * return: SKE_SUCCESS(valid), other(invalid)
 * caution:
 */
uint32_t ske_check_mode(ske_alg_e ske_alg, ske_mode_e ske_mode)
{
    uint32_t ret;

    switch(ske_mode)
    {
#ifdef SUPPORT_SKE_MODE_BYPASS
    case SKE_MODE_BYPASS:
#endif

#ifdef SUPPORT_SKE_MODE_ECB
    case SKE_MODE_ECB:
#endif

#ifdef SUPPORT_SKE_MODE_CBC
    case SKE_MODE_CBC:
#endif

#ifdef SUPPORT_SKE_MODE_CFB
    case SKE_MODE_CFB:
#endif

#ifdef SUPPORT_SKE_MODE_OFB
    case SKE_MODE_OFB:
#endif

#ifdef SUPPORT_SKE_MODE_CTR
    case SKE_MODE_CTR:
#endif

#ifdef SUPPORT_SKE_MODE_CBC_MAC
    case SKE_MODE_CBC_MAC:
#endif

#ifdef SUPPORT_SKE_MODE_CMAC
    case SKE_MODE_CMAC:
#endif
        ret = SKE_SUCCESS;
        break;

    //for DES/3DES, XTS, CCM and GCM mode are not supported due to the definition or standard
#ifdef SUPPORT_SKE_MODE_XTS
    case SKE_MODE_XTS:
#endif

#ifdef SUPPORT_SKE_MODE_CCM
    case SKE_MODE_CCM:
#endif

#ifdef SUPPORT_SKE_MODE_GCM
    case SKE_MODE_GCM:
#endif

#if (defined(SUPPORT_SKE_MODE_XTS) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_GCM))
        switch(ske_alg)
        {
#ifdef SUPPORT_SKE_AES_128
        case SKE_ALG_AES_128 :
#endif

#ifdef SUPPORT_SKE_AES_192
        case SKE_ALG_AES_192 :
#endif

#ifdef SUPPORT_SKE_AES_256
        case SKE_ALG_AES_256 :
#endif

#ifdef SUPPORT_SKE_SM4
        case SKE_ALG_SM4 :
#endif

#if (defined(SUPPORT_SKE_AES_128) || defined(SUPPORT_SKE_AES_192) || defined(SUPPORT_SKE_AES_256) || defined(SUPPORT_SKE_SM4))
            ret = SKE_SUCCESS;
            break;
#endif

        default:
            ret = SKE_INPUT_INVALID;
            break;
        }

        break;
#endif

    default:
        ret = SKE_INPUT_INVALID;
        break;
    }

    return ret;
}


/* function: get block byte length for spcific ske alg
 * parameters:
 *     ske_alg -------------------- input, ske algorithm
 * return: block byte length for ske alg
 * caution:
 *     1. please make sure ske_alg is valid
 */
uint8_t ske_get_block_byte_len(ske_alg_e ske_alg)
{
    uint8_t byteLen;

    switch(ske_alg)
    {
#ifdef SUPPORT_SKE_DES
    case SKE_ALG_DES :
#endif

#ifdef SUPPORT_SKE_TDES_128
    case SKE_ALG_TDES_128 :
#endif

#ifdef SUPPORT_SKE_TDES_192
    case SKE_ALG_TDES_192 :
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    case SKE_ALG_TDES_EEE_128 :
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
    case SKE_ALG_TDES_EEE_192 :
#endif

#if (defined(SUPPORT_SKE_DES) ||defined(SUPPORT_SKE_TDES_128) ||defined(SUPPORT_SKE_TDES_192)   \
    ||defined(SUPPORT_SKE_TDES_EEE_128) ||defined(SUPPORT_SKE_TDES_EEE_192))
        byteLen = 8;
        break;
#endif

#ifdef SUPPORT_SKE_AES_128
    case SKE_ALG_AES_128 :
#endif

#ifdef SUPPORT_SKE_AES_192
    case SKE_ALG_AES_192 :
#endif

#ifdef SUPPORT_SKE_AES_256
    case SKE_ALG_AES_256 :
#endif

#ifdef SUPPORT_SKE_SM4
    case SKE_ALG_SM4 :
#endif

#if (defined(SUPPORT_SKE_AES_128) ||defined(SUPPORT_SKE_AES_192) ||defined(SUPPORT_SKE_AES_256) ||defined(SUPPORT_SKE_SM4))
        byteLen = 16;
        break;
#endif

    default:
        byteLen = 16;   //default alg SM4
        break;
    }

    return byteLen;
}


/* function: get key byte length for spcific ske alg
 * parameters:
 *     ske_alg -------------------- input, ske algorithm
 * return: key byte length for ske alg
 * caution:
 *     1. please make sure ske_alg is valid
 */
uint8_t ske_get_key_byte_len(ske_alg_e alg)
{
    uint8_t byte_len;

    switch(alg)
    {
#ifdef SUPPORT_SKE_DES
    case SKE_ALG_DES :
        byte_len = 8;
        break;
#endif

#ifdef SUPPORT_SKE_TDES_128
    case SKE_ALG_TDES_128 :
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    case SKE_ALG_TDES_EEE_128 :
#endif

#ifdef SUPPORT_SKE_AES_128
    case SKE_ALG_AES_128 :
#endif

#ifdef SUPPORT_SKE_SM4
    case SKE_ALG_SM4 :
#endif

#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_EEE_128) || defined(SUPPORT_SKE_AES_128) \
    ||defined(SUPPORT_SKE_SM4))
        byte_len = 16;
        break;
#endif

#ifdef SUPPORT_SKE_TDES_192
    case SKE_ALG_TDES_192 :
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
    case SKE_ALG_TDES_EEE_192 :
#endif

#ifdef SUPPORT_SKE_AES_192
    case SKE_ALG_AES_192 :
#endif

#if (defined(SUPPORT_SKE_TDES_192) || defined(SUPPORT_SKE_TDES_EEE_192) || defined(SUPPORT_SKE_AES_192))
        byte_len = 24;
        break;
#endif

#ifdef SUPPORT_SKE_AES_256
    case SKE_ALG_AES_256 :
        byte_len = 32;
        break;
#endif

    default:
        byte_len = 16;   //default alg SM4
        break;
    }

    return byte_len;
}


/* function: set ske iv
 * parameters:
 *     iv ------------------------- input, initial vector
 *     block_bytes ---------------- input, byte length of current ske block
 * return: none
 * caution:
 *     1. please make sure the inputs are valid
 */
void ske_set_iv(const uint8_t *iv, uint32_t block_bytes)
{
    uint32_t tmp[4];
    uint32_t block_words = block_bytes>>2;

#if 1
    memcpy_(tmp, iv, block_bytes);

    ske_set_iv_uint32(tmp, block_words);
#else
    if (0U != (((uint32_t)iv) & 3U))
    {
        memcpy_((uint8_t *)tmp, iv, block_bytes);
        ske_set_iv_uint32(tmp, block_words);
    }
    else
    {
        ske_set_iv_uint32((const uint32_t *)iv, block_words);
    }
#endif
}


/* function: set ske key
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     key ------------------------ input, key
 *     key_bytes ------------------ input, byte length of key
 *     key_idx -------------------- input, key index, only 1 and 2 are valid
 * return: none
 * caution:
 *     1. please make sure the inputs are valid
 */
void ske_set_key(ske_alg_e alg, const uint8_t *key, uint8_t key_bytes, uint16_t key_idx)
{
    uint32_t tmp[8];
    uint8_t tmp_key_bytes;

    memcpy_((uint8_t *)tmp, key, key_bytes);

    //for 3DES-2key, set key3=key1
    switch(alg)
    {
#ifdef SUPPORT_SKE_TDES_128
    case SKE_ALG_TDES_128 :
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    case SKE_ALG_TDES_EEE_128 :
#endif

#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_EEE_128))
        memcpy_(&tmp[4], key, 8);
        tmp_key_bytes = key_bytes + (uint8_t)8;

        break;
#endif

    default:
        tmp_key_bytes = key_bytes;

        break;
    }

    ske_set_key_uint32(tmp, key_idx, ((uint32_t)tmp_key_bytes)>>2U);
}


/* function: ske check parameter
 * parameters:
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     iv ------------------------- input, iv in bytes, must be a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     2. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     3. if mode is CMAC/CBC-MAC, the iv must be a block of all zero
 *     4. if key is from user input, please make sure the argument key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
static uint32_t ske_check_param(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, const uint8_t *iv)
{
    uint32_t ret = SKE_SUCCESS;

    if(SKE_SUCCESS != ske_check_alg(alg))
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(SKE_SUCCESS != ske_check_mode(alg, mode))
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(crypto > SKE_CRYPTO_DECRYPT)
    {
        ret = SKE_INPUT_INVALID;
    }
    else if(NULL == key)   //secure port     //key idx is from 1 to SKE_MAX_KEY_IDX
    {
#ifdef SKE_SECURE_PORT_FUNCTION
        //  TODO
#else
        ret = SKE_INPUT_INVALID;
#endif
    }
    else
    {
        //handle other
    }

    if(SKE_SUCCESS == ret)
    {
#ifdef SUPPORT_SKE_MODE_BYPASS
        if((SKE_MODE_BYPASS != mode) && (SKE_MODE_ECB != mode) && (NULL == iv))
#else
        if((SKE_MODE_ECB != mode) && (NULL == iv))
#endif
        {
            ret = SKE_INPUT_INVALID;
        }
    }
    else
    {}

    return ret;
}


/* function: ske init config
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, like ECB,CBC,OFB,etc.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is common for CPU/DMA/DMA-LL
 *     2. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     3. if mode is CMAC/CBC-MAC, the iv must be a block of all zero
 *     4. if key is from user input, please make sure the argument key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_init_internal(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key,
        uint16_t sp_key_idx, const uint8_t *iv)
{
    uint32_t ret = SKE_SUCCESS;
    uint8_t key_bytes;

    if(NULL != ctx)
    {
        ret = ske_check_param(alg, mode, crypto, key, iv);
    }
    else
    {
        ret = SKE_BUFFER_NULL;
    }

    if(SKE_SUCCESS == ret)
    {
        ctx->block_bytes = ske_get_block_byte_len(alg); //keep the block length
        ctx->block_words = (ctx->block_bytes)/((uint8_t)4);

        //set iv or nonce
        ske_set_iv(iv, ctx->block_bytes);

        //config
        ske_set_endian_uint32();
        ske_set_alg(alg);
        ske_set_mode(mode);
        ske_set_crypto(crypto);
        ske_set_last_block(0);    //not affect IV

    //set key
#ifdef SKE_SECURE_PORT_FUNCTION
        if(NULL != key)    //key is from user input
#endif
        {
            ske_disable_secure_port();

            key_bytes = ske_get_key_byte_len(alg);
            ske_set_key(alg, key, key_bytes, 1);

#if defined(SUPPORT_SKE_MODE_XTS)
            if(SKE_MODE_XTS == mode)
            {
                ske_set_key(alg, &(key[key_bytes]), key_bytes, 2);
            }
            else
            {}
#endif
        }
#ifdef SKE_SECURE_PORT_FUNCTION
        else              //key is from secure port
        {
#if defined(SUPPORT_SKE_MODE_XTS)
            if(SKE_MODE_XTS == mode)
            {
                ret = lib_ske_secure_port_config(alg, sp_key_idx+(uint16_t)1);
                if(SKE_SUCCESS == ret)
                {
                    ret = ske_expand_key();
                }
                else
                {}
            }
            else
            {}
#endif
            ret = lib_ske_secure_port_config(alg, sp_key_idx);
        }

        if(SKE_SUCCESS == ret)
        {
            ret = ske_expand_key();
        }
        else
        {}
#else
        ret = ske_expand_key();
#endif
    }
    else
    {}

    return ret;
}


#if (defined(CONFIG_SKE_SUPPORT_MUL_THREAD))
/* function: keep alg key iv(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_keep_alg_key_iv(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv)
{
    uint32_t ret = SKE_SUCCESS;
    uint8_t key_bytes;

#ifdef SUPPORT_SKE_MODE_BYPASS
    if((SKE_MODE_BYPASS != mode) && (SKE_MODE_ECB != mode) && (NULL == iv))
#else
    if((SKE_MODE_ECB != mode) && (NULL == iv))
#endif
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        ctx->alg = alg;
        ctx->block_bytes = ske_get_block_byte_len(alg);
        ctx->block_words = (ctx->block_bytes)/((uint8_t)4);

        memcpy_((uint8_t *)ctx->iv, iv, ctx->block_bytes);

        if(NULL != key)
        {
            ctx->key = (uint8_t *)(ctx->key_buf);
#ifdef SUPPORT_SKE_MODE_XTS
            if(SKE_MODE_XTS == mode)
            {
                key_bytes = ske_get_key_byte_len(alg);
                memcpy_(ctx->key, key, ((uint32_t)key_bytes)<<1);
            }
            else
#endif
            {
                memcpy_(ctx->key, key, ske_get_key_byte_len(alg));
            }
        }
        else
        {
            ctx->key        = NULL;
            ctx->sp_key_idx = sp_key_idx;
        }
    }
    
    return ret;
}
#endif


/* function: ske init config(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes, must be a block
 *     padding -------------------- input, padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
 *                                         SKE_ISO_7816_4_PADDING
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must 
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_init(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, 
        uint16_t sp_key_idx, const uint8_t *iv, ske_padding_e padding)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ctx->mode    = mode;
        ctx->crypto  = crypto;
        ctx->padding = padding;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_keep_alg_key_iv(ctx, alg, mode, key, sp_key_idx, iv);
#else
        ske_clear_cfg();
        ske_set_cpu_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
        ske_set_c_len_uint32(0);  //just for XTS mode
#endif
        ske_set_padding(padding);

        ret = ske_init_internal(ctx, alg, mode, crypto, key, sp_key_idx, iv);
#endif
    }

    return ret;
}


#ifdef CONFIG_SKE_SUPPORT_MUL_THREAD
/* function: ske keep iv
 * parameters:
 *     ctx --------------------------- input, ske_ctx_st context pointer
 *     in ---------------------------- input, plaintext or ciphertext
 *     in_bytes ---------------------- input, byte length of input
 *     step -------------------------- input, step index
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. it for multiple thread
 */
static void ske_keep_iv(ske_ctx_st *ctx, const uint8_t *in, uint32_t in_bytes, uint32_t step)
{
    if(1U == step)
    {
        if((SKE_MODE_CBC == ctx->mode) || (SKE_MODE_CFB == ctx->mode))
        {
            if(SKE_CRYPTO_DECRYPT == ctx->crypto)
            {
                memcpy_(ctx->iv, &(in[in_bytes - ctx->block_bytes]), ctx->block_bytes);
            }
            else
            {}
        }
        else if (SKE_MODE_OFB == ctx->mode)
        {
            memcpy_(ctx->iv, &(in[in_bytes - ctx->block_bytes]), ctx->block_bytes);
        }
        else
        {
            //handle other
        }
    }
    else
    {}
    
    if(2U == step)
    {
        if((SKE_MODE_CBC == ctx->mode) || (SKE_MODE_CFB == ctx->mode))
        {
            if(SKE_CRYPTO_ENCRYPT == ctx->crypto)
            {
                memcpy_((uint8_t *)ctx->iv, &(in[in_bytes-ctx->block_bytes]), ctx->block_bytes);
            }
            else
            {}
        }
        else if(SKE_MODE_OFB == ctx->mode)
        {
            uint8_XOR((uint8_t *)(ctx->iv), &in[in_bytes-ctx->block_bytes], (uint8_t *)(ctx->iv), ctx->block_bytes);
        }
        else
        {
            //handle other
        }

        if(SKE_MODE_CTR == ctx->mode)
        {
            (void)ske_counter_add_u32(ctx->iv, ctx->block_words, 
                (in_bytes/(uint32_t)ctx->block_bytes), 1);
        }
        else
        {}
    }
    else
    {}
}
#endif


/* function: ske encryption or decryption(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output.
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and input/output unit must be a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. bytes must be a multiple of block byte length.
 */
uint32_t ske_update_blocks(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t ret = SKE_SUCCESS;

    if (NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if ((NULL == in) || (NULL == out))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if (0U != (bytes & (CAST2UINT32(ctx->block_bytes) - 1U)))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ske_clear_cfg();
        ske_set_cpu_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
        ske_set_c_len_uint32(0);  //just for XTS mode
#endif
        ske_set_padding(ctx->padding);

        ret = ske_init_internal(ctx, ctx->alg, ctx->mode, ctx->crypto, ctx->key, ctx->sp_key_idx, (uint8_t *)ctx->iv);
        if(SKE_SUCCESS == ret)
        {
            ske_keep_iv(ctx, in, bytes, 1U);

            //enc/dec some blocks
            ret = ske_update_blocks_internal(ctx, in, out, bytes);
            if(SKE_SUCCESS == ret)
            {
                ske_keep_iv(ctx, out, bytes, 2U);
            }
            else
            {}
        }
        else
        {}
#else
        ret = ske_update_blocks_internal(ctx, in, out, bytes);
#endif
    }

    return ret;
}


/* function: check the last plaintext block after ansi x923 padding
 * parameters:
 *     block ---------------------- input, last plaintext block after padding
 *     block_bytes ---------------- input, block byte length
 *     valid_bytes ---------------- output, valid plaintext byte length of the last block after padding
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure block_bytes is valid
 */
static uint32_t ske_check_ansi_x923_padding(const uint8_t *block, uint32_t block_bytes, uint32_t *valid_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    int32_t i, idx, tmp1;
    uint32_t padding_bytes;

    i = ((int32_t)block_bytes) - 1;

    padding_bytes = block[i];
    if((0U == padding_bytes) || (padding_bytes > block_bytes))
    {
        ret = SKE_PADDING_ERROR;
    }
    else
    {
        idx = (int32_t)block_bytes - (int32_t)padding_bytes - 1;
        --i;
        for(; i>idx; i--)
        {
            if((uint8_t)0 != block[i])
            {
                ret = SKE_PADDING_ERROR;
                break;
            }
            else
            {}
        }

        if(SKE_SUCCESS == ret)
        {
            tmp1 = idx + 1;
            *valid_bytes = (uint32_t)tmp1;
        }
        else
        {}
    }

    return ret;
}


/* function: check the last plaintext block after pkcs5_7 padding
 * parameters:
 *     block ---------------------- input, last plaintext block after padding
 *     block_bytes ---------------- input, block byte length
 *     valid_bytes ---------------- output, valid plaintext byte length of the last block after padding
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure block_bytes is valid
 */
static uint32_t ske_check_pkcs5_7_padding(const uint8_t *block, uint32_t block_bytes, uint32_t *valid_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    int32_t i, idx, tmp;
    uint32_t padding_bytes;

    i = ((int32_t)block_bytes) - 1;

    padding_bytes = block[i];
    if((0U == padding_bytes) || (padding_bytes > block_bytes))
    {
        ret = SKE_PADDING_ERROR;
    }
    else
    {
        tmp = (int32_t)block_bytes - (int32_t)padding_bytes - 1;
        idx = (int32_t)tmp;
        --i;
        for(; i>idx; i--)
        {
            if(block[i] != padding_bytes)
            {
                ret = SKE_PADDING_ERROR;
                break;
            }
            else
            {}
        }

        if(SKE_SUCCESS == ret)
        {
            tmp = idx + 1;
            *valid_bytes = (uint32_t)tmp;
        }
        else
        {}
    }

    return ret;
}


/* function: check the last plaintext block after pkcs5_7 padding
 * parameters:
 *     block ---------------------- input, last plaintext block after padding
 *     block_bytes ---------------- input, block byte length
 *     valid_bytes ---------------- output, valid plaintext byte length of the last block after padding
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure block_bytes is valid
 */
static uint32_t ske_check_iso_7816_4_padding(const uint8_t *block, uint32_t block_bytes, uint32_t *valid_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t i;

    i = block_bytes - 1U;
    while(0U != i)
    {
        if((uint8_t)0 != block[i])
        {
            break;
        }
        else
        {}
        
        i--;
    }

    if((uint8_t)0x80 != block[i])
    {
        ret = SKE_PADDING_ERROR;
    }
    else
    {
        *valid_bytes = (uint32_t)i;
    }

    return ret;
}


/* function: check the last plaintext block after padding
 * parameters:
 *     block ---------------------- input, last plaintext block after padding
 *     block_bytes ---------------- input, block byte length
 *     padding -------------------- input, padding scheme, must be SKE_ANSI_X923_PADDING, or SKE_PKCS_5_7_PADDING, 
 *                                  or SKE_ISO_7816_4_PADDING
 *     valid_bytes ---------------- output, valid plaintext byte length of the last block after padding
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_check_padding(const uint8_t *block, uint32_t block_bytes, ske_padding_e padding, uint32_t *valid_bytes)
{
    uint32_t ret;

    if(SKE_ANSI_X923_PADDING == padding)
    {
        ret = ske_check_ansi_x923_padding(block, block_bytes, valid_bytes);
    }
    else if(SKE_PKCS_5_7_PADDING == padding)
    {
        ret = ske_check_pkcs5_7_padding(block, block_bytes, valid_bytes);
    }
    else if(SKE_ISO_7816_4_PADDING == padding)
    {
        ret = ske_check_iso_7816_4_padding(block, block_bytes, valid_bytes);
    }
    else
    {
        ret = SKE_INPUT_INVALID;
    }

    return ret;
}


/* function: ske encryption or decryption for input including tail(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 */
static uint32_t ske_update_last_blocks_with_padding_enc(const ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t tmp_buf[4];
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t blocks_bytes, remainder_bytes;

    remainder_bytes = in_bytes & (CAST2UINT32(ctx->block_bytes) - 1U);

    if(0U != remainder_bytes)
    {
        blocks_bytes = in_bytes - remainder_bytes;
    }
    else
    {
        blocks_bytes = in_bytes - ctx->block_bytes;
    }

    ret = ske_update_blocks_internal(ctx, current_in, current_out, blocks_bytes);
    if(SKE_SUCCESS == ret)
    {
        current_in = &(current_in[blocks_bytes]);
        current_out = &current_out[blocks_bytes];

        ske_set_last_block(1);
        if(0U != remainder_bytes)
        {
            ske_set_last_block_len(remainder_bytes);
            ret = ske_update_blocks_internal(ctx, current_in, current_out, ctx->block_bytes);
            if(SKE_SUCCESS == ret)
            {
                *out_bytes = blocks_bytes + ctx->block_bytes;
            }
            else
            {}
        }
        else
        {
            ske_set_last_block_len(ctx->block_bytes);
            ret = ske_update_blocks_internal(ctx, current_in, current_out, ctx->block_bytes);
            if(SKE_SUCCESS == ret)
            {
                ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
                if(SKE_SUCCESS == ret)
                {
                    ske_simple_get_output_block(tmp_buf, ctx->block_words);
                    memcpy_(&(current_out[ctx->block_bytes]), (uint8_t *)tmp_buf, ctx->block_bytes);
                    *out_bytes = blocks_bytes + (CAST2UINT32(ctx->block_bytes)<<1U);
                }
                else
                {}
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: ske encryption or decryption for input including tail(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 */
static uint32_t ske_update_last_blocks_with_padding_dec(const ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    uint32_t ret = SKE_SUCCESS;
    uint32_t tmp_buf[4];
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t blocks_bytes, remainder_bytes;

    remainder_bytes = in_bytes & (CAST2UINT32(ctx->block_bytes) - 1U);

    if(0U != remainder_bytes)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        blocks_bytes = in_bytes - ctx->block_bytes;
        if(0U != blocks_bytes)
        {
            ret = ske_update_blocks_internal(ctx, current_in, current_out, blocks_bytes);
            if(SKE_SUCCESS == ret)
            {
                current_in = &(current_in[blocks_bytes]);
                current_out = &(current_out[blocks_bytes]);
            }
            else
            {}
        }
        else
        {}

        if(SKE_SUCCESS == ret)
        {
            ret = ske_update_blocks_internal(ctx, current_in, (uint8_t *)tmp_buf, ctx->block_bytes);
            if(SKE_SUCCESS == ret)
            {
                ret = ske_check_padding((uint8_t *)tmp_buf, ctx->block_bytes, ctx->padding, out_bytes);
                if(SKE_SUCCESS == ret)
                {
                    memcpy_(current_out, (uint8_t *)tmp_buf, *out_bytes);
                    *out_bytes += blocks_bytes;
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

    return ret;
}


/* function: ske encryption or decryption for input including tail(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
static uint32_t ske_update_last_blocks_without_padding(const ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    uint32_t ret;
    uint32_t tmp_buf[4];
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t blocks_bytes, remainder_bytes;
    
    remainder_bytes = in_bytes & (CAST2UINT32(ctx->block_bytes) - 1U);

    if(((SKE_MODE_ECB == ctx->mode) || (SKE_MODE_CBC == ctx->mode)) && (0U != remainder_bytes))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //now for ECB/CBC, in_bytes is a multiple of block byte length(except 0),
        //but for CFB/OFB/CTR, in_bytes could be any value(except 0), for message tail, use stream style
        blocks_bytes = in_bytes - remainder_bytes;
        ret = ske_update_blocks_internal(ctx, current_in, current_out, blocks_bytes);
        if(SKE_SUCCESS == ret)
        {
            if(0U != remainder_bytes)
            {
                current_in = &(current_in[blocks_bytes]);
                current_out = &(current_out[blocks_bytes]);

                ret = ske_update_blocks_internal(ctx, current_in, (uint8_t *)tmp_buf, ctx->block_bytes);
                if(SKE_SUCCESS == ret)
                {
                    memcpy_(current_out, (uint8_t *)tmp_buf, remainder_bytes);
                }
                else
                {}
            }
            else
            {}

            if(SKE_SUCCESS == ret)
            {
                *out_bytes = in_bytes;
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* function: ske encryption or decryption for input including tail(CPU style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
uint32_t ske_update_including_last_block(ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes)
{
    uint32_t ret = SKE_SUCCESS;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else if (0U == in_bytes)
    {
        *out_bytes = 0;
    }
    else if((NULL == in) || (NULL == out))
    {
        ret = SKE_BUFFER_NULL;
    }
    else if (SKE_ISO_7816_4_PADDING < ctx->padding)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != in_bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ske_clear_cfg();
        ske_set_cpu_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
        ske_set_c_len_uint32(0);  //just for XTS mode
#endif
        ske_set_padding(ctx->padding);

        ret = ske_init_internal(ctx, ctx->alg, ctx->mode, ctx->crypto, ctx->key, ctx->sp_key_idx, (uint8_t *)ctx->iv);
        if(SKE_SUCCESS == ret)
        {
#endif
            if(SKE_NO_PADDING != ctx->padding)
            {
                if(SKE_CRYPTO_ENCRYPT == ctx->crypto)
                {
                    ret = ske_update_last_blocks_with_padding_enc(ctx, in, out, in_bytes, out_bytes);
                }
                else
                {
                    ret = ske_update_last_blocks_with_padding_dec(ctx, in, out, in_bytes, out_bytes);
                }
            }
            else  //SKE_NO_PADDING
            {
                ret = ske_update_last_blocks_without_padding(ctx, in, out, in_bytes, out_bytes);
            }
        }

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    }
#endif

    return ret;
}


/* function: ske finish
 * parameters: 
 *     ctx ------------------------ input, ske_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_final(ske_ctx_st *ctx)
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
        memset_((uint8_t *)ctx, 0, sizeof(ske_ctx_st));

        ret = SKE_SUCCESS;
    }

    return ret;
}


/* function: ske encrypting or decrypting(CPU style, one-off style)
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
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must 
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
uint32_t ske_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, const uint8_t *iv,
        ske_padding_e padding, const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes)
{
    uint32_t ret;
    ske_ctx_st ctx[1];

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ret = ske_init(ctx, alg, mode, crypto, key, sp_key_idx, iv, padding);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_update_including_last_block(ctx, in, out, in_bytes, out_bytes);
    }
    else
    {}
#else
    ske_clear_cfg();
    ske_set_cpu_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
    ske_set_c_len_uint32(0);  //just for XTS mode
#endif
    ske_set_padding(padding);

    ctx->mode    = mode;
    ctx->crypto  = crypto;
    ctx->padding = padding;

    ret = ske_init_internal(ctx, alg, mode, crypto, key, sp_key_idx, iv);
    if(SKE_SUCCESS == ret)
    {
        ret = ske_update_including_last_block(ctx, in, out, in_bytes, out_bytes);
    }
    else
    {}
#endif
    
    return ret;
}


#ifdef SKE_DMA_FUNCTION
/* function: ske init config(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     alg ------------------------ input, ske algorithm
 *     mode ----------------------- input, ske algorithm operation mode, just for ECB/CBC/CFB/OFB/CTR.
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes, must be a block
 *     sp_key_idx ----------------- input, index of secure port key, (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX],
 *                                  if the MSB(sp_key_idx) is 1, that means using low 128bit of the 256bit key
 *     iv ------------------------- input, iv in bytes
 *     padding -------------------- input, padding scheme, should be SKE_NO_PADDING/SKE_ANSI_X923_PADDING/SKE_PKCS_5_7_PADDING/
 *                                         SKE_ISO_7816_4_PADDING
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must 
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 */
uint32_t ske_dma_init(ske_ctx_st *ctx, ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, 
        uint16_t sp_key_idx, const uint8_t *iv, ske_padding_e padding)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        ctx->mode    = mode;
        ctx->crypto  = crypto;
        ctx->padding = padding;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ret = ske_keep_alg_key_iv(ctx, alg, mode, key, sp_key_idx, iv);
#else
        ske_clear_cfg();
        ske_set_dma_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
        ske_set_c_len_uint32(0);  //just for XTS mode
#endif
        ske_disable_dma_linked_list();

        ske_set_padding(padding);
        ret = ske_init_internal(ctx, alg, mode, crypto, key, sp_key_idx, iv);
#endif
    }
    
    return ret;
}


/* function: ske encryption or decryption(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output, must be a multiple of block length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and input/output unit is a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. bytes must be a multiple of block byte length.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t ske_dma_update_blocks_internal(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback)
#else
static uint32_t ske_dma_update_blocks_internal(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes, 
        SKE_CALLBACK callback)
#endif
{
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    const uint8_t *in_remap;
#endif
    uint32_t ret;

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ske_clear_cfg();
    ske_set_dma_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
    ske_set_c_len_uint32(0);  //just for XTS mode
#endif
    ske_disable_dma_linked_list();
    ske_set_padding(ctx->padding);

    ret = ske_init_internal(ctx, ctx->alg, ctx->mode, ctx->crypto, ctx->key, ctx->sp_key_idx, (uint8_t *)ctx->iv);
    if(SKE_SUCCESS == ret)
    {
        /************* keep info for iv **************/
        if((SKE_MODE_CBC == ctx->mode) || (SKE_MODE_CFB == ctx->mode))
        {
            if(SKE_CRYPTO_DECRYPT == ctx->crypto)
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                in_remap = lib_addr_arch32_lock_remap(in_h, in_l, 1U);
#else
                in_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)in, 1U);
#endif
                memcpy_(ctx->iv, &(in_remap[bytes - ctx->block_bytes]), ctx->block_bytes);

                lib_addr_arch32_unlock_remap();
            }
            else
            {}
        }
        else if (SKE_MODE_OFB == ctx->mode)
        {

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            in_remap = lib_addr_arch32_lock_remap(in_h, in_l, 1U);
#else
            in_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)in, 1U);
#endif
            memcpy_(ctx->iv, &(in_remap[bytes - (ctx->block_bytes)]), ctx->block_bytes);
            
            lib_addr_arch32_unlock_remap();
        }
        else
        {
            //handle other
        }

        //enc/dec some blocks
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_operate(in_h, in_l, out_h, out_l, bytes, bytes, callback);
#else
        ret = ske_dma_operate(in, out, bytes, bytes, callback);
#endif
    }
    else
    {}

    if(SKE_SUCCESS == ret)
    {
        /************* keep iv **************/
        if((SKE_MODE_CBC == ctx->mode) || (SKE_MODE_CFB == ctx->mode))
        {
            if(SKE_CRYPTO_ENCRYPT == ctx->crypto)
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                in_remap = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
                in_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
                memcpy_(ctx->iv, &(in_remap[bytes - ctx->block_bytes]), ctx->block_bytes);

                lib_addr_arch32_unlock_remap();
            }
            else
            {}
        }
        else if(SKE_MODE_OFB == ctx->mode)
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            in_remap = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
            in_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
            uint8_XOR((const uint8_t *)ctx->iv, &in_remap[bytes - ctx->block_bytes], (uint8_t *)ctx->iv, ctx->block_bytes);

            lib_addr_arch32_unlock_remap();
        }
        else if(SKE_MODE_CTR == ctx->mode)
        {
            (void)ske_counter_add_u32(ctx->iv, ctx->block_words, 
                (bytes/(uint32_t)ctx->block_bytes), 1);
        }
        else
        {
            //handle other
        }
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
      
    return ret;
}


/* function: ske encryption or decryption(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, byte length of input or output, must be a multiple of block length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and input/output unit is a block
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. bytes must be a multiple of block byte length.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_update_blocks(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, uint32_t out_h, 
        uint32_t out_l, uint32_t bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_update_blocks(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t bytes, 
        SKE_CALLBACK callback)
#endif
{
    uint32_t ret = SKE_SUCCESS;

    if (NULL == ctx)
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
    else if(0U != (bytes & (CAST2UINT32(ctx->block_bytes) - 1U)))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != bytes))
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_update_blocks_internal(ctx, in_h, in_l, out_h, out_l, bytes, callback);
#else
        ret = ske_dma_update_blocks_internal(ctx, in, out, bytes, callback);
#endif
    }
    else
    {}

    return ret;
}


/* function: ske encryption or decryption for input including tail(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t ske_dma_update_last_blocks_with_padding(const ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, 
        uint32_t out_h, uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
#else
static uint32_t ske_dma_update_last_blocks_with_padding(const ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t tmp_bytes, remainder_bytes;
    const uint8_t *out_remap;
    uint32_t ret;

    remainder_bytes = in_bytes & (CAST2UINT32(ctx->block_bytes) - 1U);

    if(SKE_CRYPTO_ENCRYPT == ctx->crypto)
    {
        ske_set_last_block(1);
        if(0U != remainder_bytes)
        {
            ske_set_last_block_len(remainder_bytes);
            tmp_bytes = ((in_bytes+(ctx->block_bytes - (uint32_t)1))/ctx->block_bytes)*(ctx->block_bytes);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, tmp_bytes, tmp_bytes, callback);
#else
            ret = ske_dma_operate(in, out, tmp_bytes, tmp_bytes, callback);
#endif
            if(SKE_SUCCESS == ret)
            {
                *out_bytes = tmp_bytes;
            }
            else
            {}
        }
        else
        {
            ske_set_last_block_len(ctx->block_bytes);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, in_bytes, in_bytes+ctx->block_bytes, callback);
#else
            ret = ske_dma_operate(in, out, in_bytes, in_bytes+ctx->block_bytes, callback);
#endif
            if(SKE_SUCCESS == ret)
            {
                *out_bytes = in_bytes + (ctx->block_bytes);
            }
            else
            {}
        }
    }
    else
    {
        if(0U != remainder_bytes)
        {
            ret = SKE_INPUT_INVALID;
        }
        else
        {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = ske_dma_operate(in_h, in_l, out_h, out_l, in_bytes, in_bytes, callback);
#else
            ret = ske_dma_operate(in, out, in_bytes, in_bytes, callback);
#endif
            if(SKE_SUCCESS == ret)
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                out_remap = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
                out_remap = lib_addr_arch32_lock_remap(0U, (uint32_t)out, 0U);
#endif
                ret = ske_check_padding(&(out_remap[in_bytes - ctx->block_bytes]), ctx->block_bytes, ctx->padding, out_bytes);

                lib_addr_arch32_unlock_remap();
                if(SKE_SUCCESS == ret)
                {
                    *out_bytes += in_bytes - ctx->block_bytes;
                }
                else
                {}
            }
            else
            {}
        }
    }

    return ret;
}


/* function: ske encryption or decryption for input including tail(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t ske_dma_update_last_blocks_without_padding(const ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, 
        uint32_t out_h, uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
#else
static uint32_t ske_dma_update_last_blocks_without_padding(const ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
#endif
{
    uint32_t tmp_bytes, remainder_bytes;
    uint32_t ret;

    remainder_bytes = in_bytes & (CAST2UINT32(ctx->block_bytes) - 1U);

    if(((SKE_MODE_ECB == ctx->mode) || (SKE_MODE_CBC == ctx->mode)) && (0U != remainder_bytes))
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //now for ECB/CBC, in_bytes is a multiple of block byte length(except 0),
        //but for CFB/OFB/CTR, in_bytes could be any value(except 0), for message tail, use stream style
        tmp_bytes = ((in_bytes+(ctx->block_bytes - (uint32_t)1))/ctx->block_bytes)*(ctx->block_bytes);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_operate(in_h, in_l, out_h, out_l, tmp_bytes, tmp_bytes, callback);
#else
        ret = ske_dma_operate(in, out, tmp_bytes, tmp_bytes, callback);
#endif
        if(SKE_SUCCESS == ret)
        {
            *out_bytes = in_bytes;
        }
        else
        {}
    }

    return ret;
}


/* function: ske encryption or decryption for input including tail(DMA style)
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of input
 *     out_bytes ------------------ output, byte length of output
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is designed for ECB/CBC/CFB/OFB/CTR modes, and the input includes tail
 *     2. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     3. if with padding scheme, for encryption, in_bytes could be any integer except zero; 
 *        for decryption, in_bytes must be a multiple of block byte length.
 *     4. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_update_including_last_block(ske_ctx_st *ctx, uint32_t in_h, uint32_t in_l, 
        uint32_t out_h, uint32_t out_l, uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_update_including_last_block(ske_ctx_st *ctx, uint32_t *in, uint32_t *out, uint32_t in_bytes, 
        uint32_t *out_bytes, SKE_CALLBACK callback)
#endif
{    
    uint32_t ret = SKE_SUCCESS;

    if((NULL == ctx) || (NULL == out_bytes))
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
    else if (SKE_ISO_7816_4_PADDING < ctx->padding)
    {
        ret = SKE_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

    if((SKE_SUCCESS == ret) && (0U != in_bytes))
    {
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        ske_clear_cfg();
        ske_set_dma_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
        ske_set_c_len_uint32(0);  //just for XTS mode
#endif
        ske_disable_dma_linked_list();
        ske_set_padding(ctx->padding);

        ret = ske_init_internal(ctx, ctx->alg, ctx->mode, ctx->crypto, ctx->key, ctx->sp_key_idx, (uint8_t *)ctx->iv);
        if(SKE_SUCCESS == ret)
        {
#endif
            if(SKE_NO_PADDING != ctx->padding)
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                ret = ske_dma_update_last_blocks_with_padding(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
#else
                ret = ske_dma_update_last_blocks_with_padding(ctx, in, out, in_bytes, out_bytes, callback);
#endif
            }
            else  //SKE_NO_PADDING
            {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
                ret = ske_dma_update_last_blocks_without_padding(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
#else
                ret = ske_dma_update_last_blocks_without_padding(ctx, in, out, in_bytes, out_bytes, callback);
#endif
            }
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
        }
#endif
    }
    else
    {}

    return ret;
}


/* function: ske finish(DMA style)
 * parameters: 
 *     ctx ------------------------ input, ske_ctx_st context pointer
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if encryption or decryption is done, please call this(optional)
 */
uint32_t ske_dma_final(ske_ctx_st *ctx)
{
    uint32_t ret;

    if(NULL == ctx)
    {
        ret = SKE_BUFFER_NULL;
    }
    else
    {
        memset_((uint8_t *)ctx, 0, sizeof(ske_ctx_st));
        ret = SKE_SUCCESS;
    }

    return ret;
}


/* function: ske encrypting or decrypting(DMA style, one-off style)
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
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if mode is ECB, then there is no iv, in this case iv could be NULL
 *     2. this function is designed for ECB/CBC/CFB/OFB/CTR modes, for ECB/CBC, input/output unit must 
 *        be a block if padding is SKE_NO_PADDING.
 *     3. if key is from user input, please make sure key is not NULL(now sp_key_idx is useless),
 *        otherwise, key is from secure port, and (sp_key_idx & 0x7FFF) must be in [1,SKE_MAX_KEY_IDX]
 *     4. to save memory, in and out could be the same buffer, in this case, the output will
 *        cover the input.
 *     5. if without padding scheme, for ECB/CBC, in_bytes must be a multiple of block byte 
 *        length, out_bytes will be the same as in_bytes.
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding, uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l,
        uint32_t in_bytes, uint32_t *out_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_crypto(ske_alg_e alg, ske_mode_e mode, ske_crypto_e crypto, const uint8_t *key, uint16_t sp_key_idx, 
        const uint8_t *iv, ske_padding_e padding, uint32_t *in, uint32_t *out, uint32_t in_bytes, uint32_t *out_bytes, 
        SKE_CALLBACK callback)
#endif
{
    uint32_t ret;
    ske_ctx_st ctx[1];

#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    ret = ske_dma_init(ctx, alg, mode, crypto, key, sp_key_idx, iv, padding);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_update_including_last_block(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
#else
        ret = ske_dma_update_including_last_block(ctx, in, out, in_bytes, out_bytes, callback);
#endif
    }
    else
    {}
#else
    ske_clear_cfg();
    ske_set_dma_mode();
#if defined(SUPPORT_SKE_MODE_XTS)
    ske_set_c_len_uint32(0);  //just for XTS mode
#endif
    ske_disable_dma_linked_list();

    ske_set_padding(padding);

    ctx->mode    = mode;
    ctx->crypto  = crypto;
    ctx->padding = padding;

    ret = ske_init_internal(ctx, alg, mode, crypto, key, sp_key_idx, iv);
    if(SKE_SUCCESS == ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        ret = ske_dma_update_including_last_block(ctx, in_h, in_l, out_h, out_l, in_bytes, out_bytes, callback);
#else
        ret = ske_dma_update_including_last_block(ctx, in, out, in_bytes, out_bytes, callback);
#endif
    }
    else
    {}
#endif

    return ret;
}
#endif
