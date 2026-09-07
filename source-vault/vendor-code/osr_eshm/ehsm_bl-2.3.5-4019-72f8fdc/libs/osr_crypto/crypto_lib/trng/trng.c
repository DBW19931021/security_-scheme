
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"



#ifdef TRNG_RO_ENTROPY

static uint32_t g_global_conifg = 0u;    //default config.


/* function: set global init config
 * parameters: 
 *     config_choice -------------- config choice, should be
 *                                  NIST_GLOBAL_CONFIG, or GM_GLOBAL_CONFIG or LATEST_GM_GLOBAL_CONFIG
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. before generating random numbers, please call this function. 
 */
uint32_t trng_set_global_init_config(uint32_t config_choice)
{
    uint32_t ret;

    if(NIST_GLOBAL_CONFIG == config_choice)
    {
        g_global_conifg = NIST_GLOBAL_CONFIG;
        ret = trng_set_nist_global_init_config(DRBG_AES_128_CTR);
    }
    else if(GM_GLOBAL_CONFIG == config_choice)
    {
        g_global_conifg = GM_GLOBAL_CONFIG;
        ret = trng_set_nist_global_init_config(DRBG_SM4_CTR);
    }
    else if(LATEST_GM_GLOBAL_CONFIG == config_choice)
    {
        g_global_conifg = LATEST_GM_GLOBAL_CONFIG;
        ret = trng_set_latest_gm_global_init_config();
    }
    else
    {
        ret = TRNG_ERROR;
    }

    return ret;
}


/* function: get rand(for internal test)
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand_internal(uint8_t *random, uint32_t bytes)
{
    return get_rand_buffer(random, bytes, get_rand_uint32_without_reseed);
}

/* function: get trng rand(without post processing)
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_trng_rand(uint8_t *random, uint32_t bytes)
{
    return get_rand_without_post_processing(random, bytes, get_rand_uint32_without_reseed);
}

/* function: get rand with fast speed(with entropy reducing, for such as clearing tmp buffer)
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand_fast(uint8_t *random, uint32_t bytes)
{
#ifdef CONFIG_TRNG_GENERATE_BY_HARDWARE
    uint32_t ret;

    if(LATEST_GM_GLOBAL_CONFIG == g_global_conifg)
    {
        ret = get_latest_gm_rand(random, bytes);
    }
    else
    {
        ret = get_rand_with_post_processing(random, bytes, get_rand_uint32_without_reseed);
    }

    return ret;
#else
    volatile uint32_t i;

    for(i=0;i<2U;i++)
    {
        memset_(random, 0, bytes);
    }

    return TRNG_SUCCESS;
#endif
}


#ifdef CONFIG_TRNG_GENERATE_BY_HARDWARE
/* function: get rand(without entropy reducing)
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand(uint8_t *random, uint32_t bytes)
{
    uint32_t ret;

    if(LATEST_GM_GLOBAL_CONFIG == g_global_conifg)
    {
        ret = get_latest_gm_rand(random, bytes);
    }
    else
    {
        ret = get_rand_with_post_processing(random, bytes, get_rand_uint32_with_reseed);
    }

    return ret;
}
#else

extern uint8_t SM3_Hash(uint8_t * message, uint32_t byteLen, uint8_t digest[32]);

static uint32_t seed=0x23ba78de;
static uint8_t sm3_buf[32];
static uint8_t buf_index=0;
//trng config flag
static uint32_t trng_cfg_flag = 0;


uint32_t get_rand_register(void)
{
    static uint32_t i=0;
    uint8_t buf[32];
    uint32_t tmp = 0;

    if(0U == trng_cfg_flag)
    {
        (void)SM3_Hash((uint8_t *)&seed, 4, sm3_buf);

        trng_cfg_flag=1;
    }
    else
    {;}

    if(buf_index<28U)
    {
        tmp = *((uint32_t *)(sm3_buf+buf_index));
        buf_index+=4U;
    }
    else if(buf_index == 28U)
    {
        tmp = *((uint32_t *)(sm3_buf+28));
        memcpy_(buf, sm3_buf, 32);
        i++;
        *((uint32_t *)(buf+16)) += 1U;
        (void)SM3_Hash(buf, 32, sm3_buf);
        buf_index=0;
    }
    else
    {
        //handle other
    }

    return tmp;
}

uint32_t get_rand(uint8_t *random, uint32_t byteLen)
{
    //uint8_t *rand_bak = rand
    //uint32_t len_bak = byteLen
    uint32_t word_len, result;
    uint8_t left_len = (uint8_t)((uint32_t)random & 0x3U);

    // if the data addr is not aligned by word
    if ((uint8_t)0 != left_len)
    {
        // wait the data is ready
        result = get_rand_register();

        if (byteLen > (4U - (uint32_t)left_len)) {
            memcpy_(random, (uint8_t *)(&result), 4U - (uint32_t)left_len);
            byteLen -= (4U - (uint32_t)left_len);
            random = &(random[(4U - left_len)]);
        }
        else
        {
            memcpy_(random, (uint8_t *)(&result), byteLen);
            //trng_disable()
            return 0;//TRNG_SUCCESS;
        }
    }

    word_len = byteLen >> 2;
    left_len = (uint8_t)(byteLen & 0x3U);

    // obtain the data by word
    while (0U != word_len--)
    {
        *((uint32_t *)random) = get_rand_register();
        random = &(random[4]);
    }

    // if the byteLen is not aligned by word
    if ((uint8_t)0 != left_len)
    {
        result = get_rand_register();
        memcpy_(random, (uint8_t *)(&result), left_len);
    }

    return 0;//TRNG_SUCCESS;
}
#endif

#endif
