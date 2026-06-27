#include "../../crypto_include/chacha20_poly1305/chacha20_poly1305.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_lib/lib_extension.h"

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
extern volatile uint32_t g_chacha20_poly1305_suspend_flag;
#endif

 /* function: update payload blocks and get the same number of blocks (DMA mode)
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, bytes length of input or output
 * return: SKE_SUCCESS(success), other(error)
 */
FLAG_STATIC uint32_t chacha20_poly1305_dma_operate(const uint32_t *in, const uint32_t *out, uint32_t bytes)
{
    
    uint32_t ret;

	if(((NULL == in) && (0U != bytes)) || ((NULL == out) && (0U != bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else
	{
        chacha20_poly1305_dma_set_cfg(in, out, bytes);

        chacha20_poly1305_start();
        chacha20_poly1305_wait_core_idle();

        ret = chacha20_poly1305_get_error_status();

        if(0U == ret)
        {
            ret = CHACHA20_POLY1305_SUCCESS;
        }
        else
        {
            ret = CHACHA20_POLY1305_CONFIG_INVALID;
        }
    }

    return ret;
}


/* function: update chacha20_poly1305 some blocks without output
 * parameters:
 *     in ------------------------- input, data
 *     words ---------------------- input, word length of in
 * return: none
 * caution:
 *     1. use for all aad input and get current tag
 */
FLAG_STATIC void chacha20_poly1305_set_input_word(const uint8_t *in, uint64_t words)
{
    
	uint32_t in_word_align;
	uint32_t tmp_in[1];
    const uint8_t *current_in = in;
	uint64_t i;

	if(0U != check_addr_not_word_align(current_in))
	{
		in_word_align = 0;
	}
	else
	{
		in_word_align = 1;
	}

	for (i = 0; i < words; i++)
	{
		if(0U != in_word_align)
		{
            chacha20_poly1305_wait_ready_data_signal();
			chacha20_poly1305_simple_set_input_word((const uint32_t *)current_in);
		}
		else
		{
			memcpy_(tmp_in, current_in, 4);

            chacha20_poly1305_wait_ready_data_signal();
			chacha20_poly1305_simple_set_input_word(tmp_in);
		}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
        if(0U != g_chacha20_poly1305_suspend_flag)
        {
            chacha20_poly1305_reset();
            break;
        }
        else
        {}
#endif
		current_in = &current_in[4u];
	}
}


/* function: set whether chacha20_poly1305 next input data is the last word data or not (CPU mode)
 * parameters:
 *     is_last_block -------------- input, 0:no, other:yes
 * return: none
 * caution:
 */
FLAG_STATIC void chacha20_poly1305_set_last_word(uint32_t is_last_word)
{
    
	if(0U != is_last_word)
	{
		chacha20_poly1305_set_last_mode();
	}
	else
	{
		chacha20_poly1305_set_non_last_mode();
	}
}


/* function:chacha20_poly1305 get err code
 * parameters:
 * return: none
 *     error -------------- output, err code
 * caution:
 */
FLAG_STATIC uint32_t chacha20_poly1305_get_error(void)
{
    
	uint32_t ret = chacha20_poly1305_get_error_status();

	if(0U == ret)
	{
		ret = CHACHA20_POLY1305_SUCCESS;
	}
	else
	{
		ret = CHACHA20_POLY1305_CONFIG_INVALID;
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 set length and set aad last 0
 * parameters:
 *     aad_bytes ---------------------- input, byte length of aad
 *     payload_bytes ------------------ input, byte length of payload
 * return: none
 * caution:
 */
FLAG_STATIC void chacha20_poly1305_set_length_clean_aad_last(uint64_t aad_bytes, uint64_t payload_bytes)
{
    
    uint32_t tmp = (uint32_t)(payload_bytes & 3U);

	if ((0U == tmp) && (0U != payload_bytes))
	{
		tmp = 4U;
	}
    else
    {}

    chacha20_poly1305_set_length_basic(4U, tmp);

    tmp = ((uint32_t)aad_bytes & 0xFFFFFFFFU);
    chacha20_poly1305_set_length_basic(0U, tmp);
    tmp = (uint32_t)(aad_bytes >> 32);
    chacha20_poly1305_set_length_basic(1U, tmp);
    tmp = ((uint32_t)payload_bytes & 0xFFFFFFFFU);
    chacha20_poly1305_set_length_basic(2U, tmp);
    tmp = (uint32_t)(payload_bytes >> 32);
    chacha20_poly1305_set_length_basic(3U, tmp);
}


/* function: chacha20_poly1305 set length
 * parameters:
 *     aad_bytes ---------------------- input, byte length of aad
 * return: none
 * caution:
 */
FLAG_STATIC void chacha20_poly1305_set_aad_length(uint64_t aad_bytes)
{
    
	uint32_t last_aad = (uint32_t)(aad_bytes & 3U);

	if ((0U == last_aad) && (0U != aad_bytes))
	{
		last_aad = 4U;
	}
    else
    {}

	chacha20_poly1305_set_length_clean_aad_last(aad_bytes, 0U);
	
    chacha20_poly1305_set_length_basic(4U, (last_aad << 3));
}


/* function: chacha20_poly1305 set key
 * parameters:
 *     key ------------------------ input, key in word buffer, occupies 32 bytes
 * return: none
 * caution:
 */
FLAG_STATIC void chacha20_poly1305_set_key(const uint8_t *key)
{
    
    uint32_t i = 8U;
    uint32_t tmp[8];
    const uint32_t *key_ptr;

    if(0U != check_addr_not_word_align(key))
    {
        memcpy_(tmp, key, 32);
        key_ptr = tmp;
    }
    else
    {
        key_ptr = (const uint32_t *)key;
    }

    while (0U != i)
    {
        i--;
        
        chacha20_poly1305_set_key_basic(i, key_ptr[i]);
    }
}


/* function: chacha20_poly1305 set nonce
 * parameters:
 *     nonce ---------------------- input, nonce in word buffer, occupies 8bytes
 * return: none
 * caution:
 */
FLAG_STATIC void chacha20_poly1305_set_iv(const uint8_t *iv)
{
    
    uint32_t tmp[2];
    const uint32_t *iv_ptr;

    if(0U != check_addr_not_word_align(iv))
    {
        memcpy_(tmp, iv, 8U);
        iv_ptr = tmp;
    }
    else
    {
        iv_ptr = (const uint32_t *)iv;
    }

    chacha20_poly1305_set_iv_basic(0U, iv_ptr[0]);
    chacha20_poly1305_set_iv_basic(1U, iv_ptr[1]);
}


/* function: update payload blocks and get the same number of blocks internal
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_word_align ---------------input, input address word align flag
 *     out_word_align --------------input, input address word align flag
 *     blocks ----------------------input, blocks number
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input data does not contain last word data
 */
FLAG_STATIC void chacha20_poly1305_update_blocks_internal(const uint8_t *in, uint8_t *out, uint32_t in_word_align, uint32_t out_word_align, uint64_t blocks)
{
    
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t i, tmp[4];

    if((0U != in_word_align) && (0U != out_word_align))
    {
        for(i=0; i<blocks; i++)
        {
            chacha20_poly1305_wait_ready_data_signal();
            chacha20_poly1305_set_input_block((const uint32_t *)current_in);
            chacha20_poly1305_wait_till_done();
            chacha20_poly1305_get_output_block((uint32_t *)current_out);

            current_in = &current_in[16];
            current_out = &current_out[16];
        }
    }
    else
    {
        for(i = 0; i < blocks; i++)
        {
            if(0U != in_word_align)
            {
                chacha20_poly1305_wait_ready_data_signal();
                chacha20_poly1305_set_input_block((const uint32_t *)current_in);
            }
            else
            {
                memcpy_((uint8_t *)tmp, current_in, 16);
                chacha20_poly1305_wait_ready_data_signal();
                chacha20_poly1305_set_input_block(tmp);
            }

            chacha20_poly1305_wait_till_done();

            if(0U != out_word_align)
            {
                chacha20_poly1305_get_output_block((uint32_t *)current_out);
            }
            else
            {
                chacha20_poly1305_get_output_block(tmp);
                memcpy_(current_out, (uint8_t *)tmp, 16);
            }

            current_in = &current_in[16];
            current_out = &current_out[16];
        }
    }
}


/* function: update payload blocks and get the same number of blocks
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     remainder ------------------ input, bytes length of remaining data
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input data must contain last word data
 */
FLAG_STATIC uint32_t chacha20_poly1305_update_last_block(const uint8_t *in, uint8_t *out, uint32_t remainder_bytes)
{
    
	uint64_t i, rounds;
	uint32_t tmp[4];
	uint32_t last_block_bytes = remainder_bytes & 15U;
    const uint8_t *current_in = in;
    uint8_t *current_out = out;
    uint32_t msg_remainder;

	rounds = (remainder_bytes+3U)>>2;
	msg_remainder = remainder_bytes & 3U;
	
	if(0U == msg_remainder)
	{
		msg_remainder = 4U;
	}


	if(0U == last_block_bytes)
	{
		last_block_bytes = 16U;
	}
	
	for(i=1; i <= (rounds-1U); i++)
	{
		chacha20_poly1305_set_input_word(current_in, 1);
		current_in = &current_in[4];

		if(0U == (i & 3U))
		{
			chacha20_poly1305_wait_till_done();
            
            if(0U != check_addr_not_word_align(current_out))
            {
                chacha20_poly1305_get_output_block(tmp);
                memcpy_(current_out, (uint8_t *)tmp, 16);
            }
            else
            {
    			chacha20_poly1305_get_output_block((uint32_t *)current_out);
            }
			current_out = &current_out[16];
		}
	}

	chacha20_poly1305_set_last_word(1);
	memcpy_((uint8_t *)tmp, current_in, msg_remainder);
	chacha20_poly1305_set_input_word((const uint8_t *)tmp, 1);
	
	chacha20_poly1305_wait_core_idle();
    
	chacha20_poly1305_get_output_block((uint32_t *)tmp);
	memcpy_(current_out, (uint8_t *)tmp, last_block_bytes);

	return chacha20_poly1305_get_error();
}


/* function: update payload blocks and get the same number of blocks
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, bytes length of input or output, must be multiples of 64
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. the input data does not contain last word data
 */
FLAG_STATIC uint32_t chacha20_poly1305_update_blocks(const uint8_t *in, uint8_t *out, uint64_t bytes)
{
    
	uint32_t in_word_align, out_word_align, ret;
	uint64_t rounds = bytes>>4;

	if(0U != ((uint32_t)bytes & 63U))
	{
		ret = CHACHA20_POLY1305_INPUT_INVALID;
	}
	else
	{
        if(0U != check_addr_not_word_align(in))
        {
            in_word_align = 0;
        }
        else
        {
            in_word_align = 1;
        }

        if(0U != check_addr_not_word_align(out))
        {
            out_word_align = 0;
        }
        else
        {
            out_word_align = 1;
        }

        chacha20_poly1305_update_blocks_internal(in, out, in_word_align, out_word_align, rounds);

        ret = CHACHA20_POLY1305_SUCCESS;
    }

	return ret;
}


/* function: chacha20_poly1305 init config
 * parameters:
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key, occupies 32 bytes
 *     constant ------------------- input, constant of nonce
 *     iv ------------------------- input, iv of nonce, occupies 8 bytes
 *     stage ---------------------- input, data interleave stage
 *     dma_en --------------------- input, input, for DMA mode(not 0) or not(0)
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. this function is common for CPU/DMA
 */
FLAG_STATIC uint32_t chacha20_poly1305_init_internal(chacha20_poly1305_crypto_e crypto, const uint8_t *key, uint32_t constant, 
	const uint8_t *iv, chacha20_poly1305_stage_e stage, uint32_t dma_en)
{
    
	uint32_t ret;

	if((NULL == key) || (NULL == iv))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else if(CHACHA20_POLY1305_STAGE_LAST < stage)
	{
		ret = CHACHA20_POLY1305_INPUT_INVALID;
	}
	else if(CHACHA20_POLY1305_CRYPTO_DECRYPT < crypto)
	{
		ret = CHACHA20_POLY1305_INPUT_INVALID;
	}
	else
	{
		chacha20_poly1305_wait_core_idle();

		if(CHACHA20_POLY1305_DMA_ENABLE == dma_en)
		{
			chacha20_poly1305_set_dma_mode();
		}
		else
		{
			chacha20_poly1305_set_cpu_mode();
		}
		
		chacha20_poly1305_set_crypto(crypto);
		chacha20_poly1305_set_sec_stage(stage);
		chacha20_poly1305_set_key(key);
		chacha20_poly1305_set_const(constant);
		chacha20_poly1305_set_iv(iv);

		ret = CHACHA20_POLY1305_SUCCESS;
	}
	
  	return ret;
}


FLAG_STATIC uint32_t chacha20_poly1305_update_aad(chacha20_poly1305_st *ctx, const uint8_t *aad , uint64_t aad_bytes)
{
    
	uint32_t ret, tmp[1] = {0};
    const uint8_t *u8_ptr;
	uint64_t words;
	uint32_t remainder_bytes = (uint32_t)(aad_bytes & 3U);

    if(0U == remainder_bytes)
    {
        remainder_bytes = 4U;
    }
    else
    {}

    if(0U != aad_bytes)
    {
        words = (aad_bytes+3U)>>2;

        chacha20_poly1305_set_aad_length(aad_bytes);
        chacha20_poly1305_start();
        chacha20_poly1305_set_last_word(0);
        if(words > 1U)
        {
            chacha20_poly1305_set_input_word(aad, words - 1U);
        }
        else
        {}

        /* last aad word */
        chacha20_poly1305_set_last_word(1);

        u8_ptr = &aad[((words-1U)<<2)];
        memcpy_((uint8_t *)tmp, u8_ptr, remainder_bytes);
        chacha20_poly1305_set_input_word((const uint8_t *)tmp, 1);

        chacha20_poly1305_wait_core_idle();
        ret = chacha20_poly1305_get_error();
        if(CHACHA20_POLY1305_SUCCESS == ret)
        {
            chacha20_poly1305_get_current_tag(ctx->cur_tag);
        }
        else
        {}
    }
    else
    {
        ret = CHACHA20_POLY1305_SUCCESS;
    }

    return ret;
}


FLAG_STATIC uint32_t chacha20_poly1305_gen_verify_tag(chacha20_poly1305_st *ctx, uint8_t tag[16])
{
    uint32_t ret = CHACHA20_POLY1305_SUCCESS;

    chacha20_poly1305_get_current_tag(ctx->cur_tag);

    if(CHACHA20_POLY1305_CRYPTO_ENCRYPT == ctx->crypto)
    {
        memcpy_(tag, ctx->cur_tag, 16);
    }
    else
    {
        ret = memcmp_(tag, ctx->cur_tag, 16);
        if(0U != ret)
        {
            ret = CHACHA20_POLY1305_VERIFY_FAIL;
        }
        else
        {}
    }

    return ret;
}


FLAG_STATIC uint32_t chacha20_poly1305_check_param(const chacha20_poly1305_st *ctx, const uint8_t *payload_in, 
	const uint8_t *payload_out, uint64_t payload_bytes)
{
    
    uint32_t ret;

    if(0U != ((uint32_t)payload_bytes & 63U))
	{
		ret = CHACHA20_POLY1305_INPUT_INVALID;
	}
	else if((NULL == ctx) || ((NULL == payload_in) && (0U != payload_bytes)) || ((NULL == payload_out) && (0U != payload_bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
    else
    {
        ret = CHACHA20_POLY1305_SUCCESS;
    }

    return ret;
}


/* function: chacha20_poly1305 init (CPU style)
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_st context pointer
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key, occupies 32 bytes
 *     constant ------------------- input, constant of nonce
 *     iv ------------------------- input, iv of nonce
 *     aad ------------------------ input, all aad
 *     aad_bytes ------------------ input, byte length of aad
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t chacha20_poly1305_init(chacha20_poly1305_st *ctx, chacha20_poly1305_crypto_e crypto, const uint8_t *key, uint32_t constant, 
	const uint8_t iv[8], const uint8_t *aad , uint64_t aad_bytes)
{
	uint32_t ret;

	if((NULL == ctx) || ((NULL == aad) && (0U != aad_bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else
	{
		ctx->cur_cnt = 1;
		ctx->payload_bytes = 0;
		ctx->aad_bytes = aad_bytes;
		memset_(ctx->cur_tag, 0, 17);

		ret = chacha20_poly1305_init_internal(crypto, key, constant, iv, CHACHA20_POLY1305_STAGE_INIT, CHACHA20_POLY1305_DMA_DISABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
			ctx->crypto = crypto;
			memcpy_(ctx->key, key, 32);
			memcpy_(ctx->iv, iv, 8);
			ctx->constant = constant;
#else
			ctx->crypto = crypto;
#endif
            ret = chacha20_poly1305_update_aad(ctx, aad, aad_bytes);
		}
        else
        {}
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 update message excluding the last data (CPU mode)
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_st context pointer
 *     payload_in ----------------- input, payload of some blocks, excluding last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload, must be a multiple of 64, could be 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the whole payload length is 0, this case is supported. in this case, payload_in and
 *        payload_out could set NULL
 */
uint32_t chacha20_poly1305_update_excluding_last_data(chacha20_poly1305_st *ctx, const uint8_t *payload_in, 
	uint8_t *payload_out, uint64_t payload_bytes)
{
    
	uint32_t ret;
	uint64_t rounds = (payload_bytes+63U)>>6;
    const uint8_t *in = payload_in;
        uint8_t *out = payload_out;
	uint32_t remainder_bytes = (uint32_t)(payload_bytes & 63U);

    ret = chacha20_poly1305_check_param(ctx, payload_in, payload_out, payload_bytes);

    if((CHACHA20_POLY1305_SUCCESS == ret) && (0U != payload_bytes))
	{
		ctx->payload_bytes += payload_bytes;

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD	
		ret = chacha20_poly1305_init_internal(ctx->crypto, ctx->key, ctx->constant, ctx->iv, CHACHA20_POLY1305_STAGE_MIDDLE, CHACHA20_POLY1305_DMA_DISABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#else
			chacha20_poly1305_set_sec_stage(CHACHA20_POLY1305_STAGE_MIDDLE);
#endif
			chacha20_poly1305_set_length_clean_aad_last(ctx->aad_bytes, payload_bytes);
			chacha20_poly1305_set_cnt(ctx->cur_cnt);
			chacha20_poly1305_set_tag_in(ctx->cur_tag);

			chacha20_poly1305_start();
			chacha20_poly1305_set_last_word(0);
			
			if(rounds > 1U)
			{
				ret = chacha20_poly1305_update_blocks(in, out, payload_bytes - 64U);
				if(CHACHA20_POLY1305_SUCCESS == ret)
				{
					in = &in[payload_bytes - 64U];
					out = &out[payload_bytes - 64U];
				}
				else
				{}
			}
			else
			{}

			if(0U == remainder_bytes)
			{
				remainder_bytes = 64U;
			}
			else
			{}

			ret = chacha20_poly1305_update_last_block(in, out, remainder_bytes);	
			if(CHACHA20_POLY1305_SUCCESS == ret)
			{
				ctx->cur_cnt = chacha20_poly1305_get_current_cnt();	
				chacha20_poly1305_get_current_tag(ctx->cur_tag);
			}
			else
			{}
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD	
		}
		else
		{}
#endif
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 update message including the last data internal api
 * parameters:
 *     payload_in ----------------- input, payload, including last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t chacha20_poly1305_update_last_data_internal(const uint8_t *payload_in, uint8_t *payload_out, uint64_t payload_bytes)
{
    
    uint64_t blocks;
    uint32_t remainder_bytes = (uint32_t)(payload_bytes & 63U);
    uint32_t ret = CHACHA20_POLY1305_SUCCESS;
    const uint8_t *in = payload_in;
    uint8_t *out = payload_out;

    if((0U == remainder_bytes) && ((uint64_t)0 != payload_bytes))
    {
        remainder_bytes = 64U;
    }
    else
    {}

    if(63U <= (payload_bytes+63U))
    {
        blocks = (payload_bytes+63U)>>6;
    }
    else
    {
        blocks = ((payload_bytes)>>6)+1U;
    }

    chacha20_poly1305_start();
    if(0U != payload_bytes)
    {
        chacha20_poly1305_set_last_word(0);
        ret = chacha20_poly1305_update_blocks(in, out, payload_bytes - remainder_bytes);
        if(CHACHA20_POLY1305_SUCCESS == ret)
        {
            in = &in[((blocks-1U)<<6)];
            out = &out[((blocks-1U)<<6)];

            ret = chacha20_poly1305_update_last_block(in, out, remainder_bytes);
        }else{}
    }
    else
    {
        chacha20_poly1305_wait_core_idle();
    }

    return ret;
}


/* function: chacha20_poly1305 update message including the last data (CPU mode), and get the tag
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_st context pointer
 *     payload_in ----------------- input, payload, including last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload
 *     tag ------------------------ output(for encryption), input(for decryption), occupies 16 bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the all payload length is 0, this case is supported. in this case, payload_in and
 *        payload_out could set NULL and payload_bytes should be 0
 */
uint32_t chacha20_poly1305_update_including_last_data(chacha20_poly1305_st *ctx, const uint8_t *payload_in, 
	uint8_t *payload_out, uint64_t payload_bytes, uint8_t tag[16])
{
	uint32_t ret;

	if((NULL == ctx) || ((NULL == payload_in) && ((uint64_t)0 != payload_bytes)) || ((NULL == payload_out) && ((uint64_t)0 != payload_bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else
	{
		ctx->payload_bytes += payload_bytes;

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		ret = chacha20_poly1305_init_internal(ctx->crypto, ctx->key, ctx->constant, ctx->iv, CHACHA20_POLY1305_STAGE_LAST, CHACHA20_POLY1305_DMA_DISABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#else
    		chacha20_poly1305_set_sec_stage(CHACHA20_POLY1305_STAGE_LAST);
#endif
			chacha20_poly1305_set_cnt(ctx->cur_cnt);
			chacha20_poly1305_set_tag_in(ctx->cur_tag);
			chacha20_poly1305_set_length_clean_aad_last(ctx->aad_bytes, ctx->payload_bytes);

            ret = chacha20_poly1305_update_last_data_internal(payload_in, payload_out, payload_bytes);
			if(CHACHA20_POLY1305_SUCCESS == ret)
			{
                ret = chacha20_poly1305_gen_verify_tag(ctx, tag);
			}
            else
            {}
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		}
		else
		{}
#endif
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 (CPU style, one-off style)
 * parameters:
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     constant ------------------- input, constant of nonce
 *     iv ------------------------- input, iv of nonce
 *     aad ------------------------ input, key in bytes
 *     aad_bytes ------------------ input, byte length of aad
 *     payload_in ----------------- input, payload of some blocks, excluding last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload, could be 0
 *     tag ------------------------ output, chacha20_poly1305 tag, occupies 16 bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. aad_bytes and payload_bytes could be any value. if length bytes is 0, could set pointer is NULL
 */
uint32_t chacha20_poly1305(chacha20_poly1305_crypto_e crypto, const uint8_t key[32], uint32_t constant, 
	const uint8_t iv[8], const uint8_t *aad , uint64_t aad_bytes, const uint8_t *payload_in, uint8_t *payload_out, uint64_t payload_bytes, uint8_t tag[16])
{
	uint32_t ret;
	chacha20_poly1305_st ctx[1];
	
	ret = chacha20_poly1305_init(ctx, crypto, key, constant, iv, aad, aad_bytes);
	if(CHACHA20_POLY1305_SUCCESS == ret)
	{
		ret = chacha20_poly1305_update_including_last_data(ctx, payload_in, payload_out, payload_bytes, tag);
	}
	else
	{}

	return ret;
}



#ifdef CHACHA20_POLY1305_DMA_FUNCTION
/* function: chacha20_poly1305 init (DMA style)
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_dma_st context pointer
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     constant ------------------- input, constant of nonce
 *     iv ------------------------- input, iv of nonce
 *     aad ------------------------ input, key in bytes
 *     aad_bytes ------------------ input, byte length of aad
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. in dma mode, the byte length of aad occupies 32 bit
 */
uint32_t chacha20_poly1305_dma_init(chacha20_poly1305_dma_st *ctx, chacha20_poly1305_crypto_e crypto, const uint8_t key[32], 
	uint32_t constant, const uint8_t iv[8], const uint32_t *aad , uint32_t aad_bytes)
{
	uint32_t ret;

	if((NULL == ctx) || ((NULL == aad) && (0U != aad_bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else
	{
		ctx->cur_cnt = 1;
		ctx->aad_bytes = aad_bytes;
		ctx->payload_bytes = 0;
		memset_(ctx->cur_tag, 0, 17);

		ret = chacha20_poly1305_init_internal(crypto, key, constant, iv, CHACHA20_POLY1305_STAGE_INIT, CHACHA20_POLY1305_DMA_ENABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
			ctx->crypto = crypto;
			memcpy_(ctx->key, key, 32);
			memcpy_(ctx->iv, iv, 8);
			ctx->constant = constant;
#else
			ctx->crypto = crypto;
#endif

			if(0U != aad_bytes)
			{
				chacha20_poly1305_set_aad_length(aad_bytes);
				chacha20_poly1305_dma_set_aad(aad, aad_bytes);
				chacha20_poly1305_dma_set_payload(NULL, 0);
				
				chacha20_poly1305_start();
				chacha20_poly1305_wait_core_idle();
				ret = chacha20_poly1305_get_error();
				if(CHACHA20_POLY1305_SUCCESS == ret)
				{
					chacha20_poly1305_get_current_tag(ctx->cur_tag);
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

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 update message excluding the last data (DMA mode)
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_dma_st context pointer
 *     payload_in ----------------- input, payload of some blocks, excluding last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte of payload, must be a multiple of 64, could be 0
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the whole payload length is 0, this case is supported. in this case, payload_in and
 *        payload_out could set NULL
 */
uint32_t chacha20_poly1305_dma_update_excluding_last_data(chacha20_poly1305_dma_st *ctx, const uint32_t *payload_in, const uint32_t *payload_out, 
	uint32_t payload_bytes)
{
    
	uint32_t ret;

    ret = chacha20_poly1305_check_param(ctx, (const uint8_t *)payload_in, (const uint8_t *)payload_out, payload_bytes);

	if((CHACHA20_POLY1305_SUCCESS == ret) && (0U != payload_bytes))
	{
		ctx->payload_bytes += payload_bytes;

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		ret = chacha20_poly1305_init_internal(ctx->crypto, ctx->key, ctx->constant, ctx->iv, CHACHA20_POLY1305_STAGE_MIDDLE, CHACHA20_POLY1305_DMA_ENABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#else
			chacha20_poly1305_set_sec_stage(CHACHA20_POLY1305_STAGE_MIDDLE);
#endif
			chacha20_poly1305_set_length_clean_aad_last(ctx->aad_bytes, payload_bytes);
			chacha20_poly1305_set_tag_in(ctx->cur_tag);
			chacha20_poly1305_set_cnt(ctx->cur_cnt);
			
			ret = chacha20_poly1305_dma_operate(payload_in, payload_out, payload_bytes);
			if(CHACHA20_POLY1305_SUCCESS == ret)
			{
				ctx->cur_cnt = chacha20_poly1305_get_current_cnt();
				chacha20_poly1305_get_current_tag(ctx->cur_tag);
			}
			else
			{}
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		}
#endif
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 update message including the last data (DMA mode), and get the tag
 * parameters:
 *     ctx ------------------------ input, chacha20_poly1305_dma_st context pointer
 *     payload_in ----------------- input, payload, including last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload
 *     tag ------------------------ output(for encryption), input(for decryption), occupies 16 bytes
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. if the all payload length is 0, this case is supported. in this case, payload_in and
 *        payload_out could set NULL and payload_bytes should be 0
 */
uint32_t chacha20_poly1305_dma_update_including_last_data(chacha20_poly1305_dma_st *ctx, const uint32_t *payload_in,
	const uint32_t *payload_out, uint32_t payload_bytes, uint8_t tag[16])
{
	uint32_t ret;

	if((NULL == ctx) || ((NULL == payload_out) && (0U != payload_bytes)) || ((NULL == payload_out) && (0U != payload_bytes)))
	{
		ret = CHACHA20_POLY1305_BUFFER_NULL;
	}
	else
	{
		ctx->payload_bytes += payload_bytes;

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		ret = chacha20_poly1305_init_internal(ctx->crypto, ctx->key, ctx->constant, ctx->iv, CHACHA20_POLY1305_STAGE_LAST, CHACHA20_POLY1305_DMA_ENABLE);
		if(CHACHA20_POLY1305_SUCCESS == ret)
		{
#else
			chacha20_poly1305_set_sec_stage(CHACHA20_POLY1305_STAGE_LAST);
#endif
			chacha20_poly1305_set_length_clean_aad_last(ctx->aad_bytes, ctx->payload_bytes);
			chacha20_poly1305_set_tag_in(ctx->cur_tag);
			chacha20_poly1305_set_cnt(ctx->cur_cnt);

			ret = chacha20_poly1305_dma_operate(payload_in, payload_out, payload_bytes);
			if(CHACHA20_POLY1305_SUCCESS == ret)
			{
				ctx->cur_cnt = chacha20_poly1305_get_current_cnt();
				chacha20_poly1305_get_current_tag(ctx->cur_tag);

				if(CHACHA20_POLY1305_CRYPTO_ENCRYPT == ctx->crypto)
				{
					memcpy_(tag, ctx->cur_tag, 16);
				}
				else
				{
					ret = memcmp_(tag, ctx->cur_tag, 16);
					if(0U != ret)
					{
						ret = CHACHA20_POLY1305_VERIFY_FAIL;
					}
					else
					{}
				}
			}
			else
			{}
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
		}
		else
		{}
#endif
	}

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
    chacha20_poly1305_get_suspend_status(&ret);
#endif
	return ret;
}


/* function: chacha20_poly1305 (DMA style, one-off style)
 * parameters:
 *     crypto --------------------- input, encrypting or decrypting
 *     key ------------------------ input, key in bytes
 *     constant ------------------- input, constant of nonce
 *     iv ------------------------- input, iv of nonce
 *     aad ------------------------ input, key in bytes
 *     aad_bytes ------------------ input, byte length of aad
 *     payload_in ----------------- input, payload of some blocks, excluding last block(or message tail)
 *     payload_out ---------------- output, ciphertext or plaintext
 *     payload_bytes -------------- input, byte length of payload, could be 0
 *     tag ------------------------ output, chacha20_poly1305 tag, occupies 4 words
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. aad_bytes and payload_bytes could be any value. if length bytes is 0, could set pointer is NULL
 *     2. in dma mode, the byte length of aad or payload occupies 32 bit
 */
uint32_t chacha20_poly1305_dma(chacha20_poly1305_crypto_e crypto, const uint8_t key[32], uint32_t constant, 
	const uint8_t iv[8], const uint32_t *aad , uint32_t aad_bytes, const uint32_t *payload_in, const uint32_t *payload_out, uint32_t payload_bytes, uint8_t tag[16])
{
	uint32_t ret;
	chacha20_poly1305_dma_st ctx[1];

	ret = chacha20_poly1305_dma_init(ctx, crypto, key, constant, iv, aad, aad_bytes);
	if(CHACHA20_POLY1305_SUCCESS == ret)
	{
        ret = chacha20_poly1305_dma_update_including_last_data(ctx, payload_in, payload_out, payload_bytes, tag);
	}
	else
	{}

	return ret;
}
#endif
