#include "hash_basic.h"
#include "../crypto_include/crypto_common/utility.h"

#ifdef HASH_SEC
#include "../crypto_include/trng/trng.h"
#endif


#if 1
//HASH register struct 
typedef struct {
    uint32_t HASH_CTRL[1];                /* Offset: 0x000 (R/W)  Control register */
    uint32_t HASH_CFG[1];                 /* Offset: 0x004 (R/W)  Config register */
    uint32_t REV_1[2];
    uint32_t HASH_RISR[1];                /* Offset: 0x010 (W0C)  Resource of Interrupt Status register */
    uint32_t HASH_IMCR[1];                /* Offset: 0x014 (R/W)  Interrupt Management and Control register */
    uint32_t HASH_MISR[1];                /* Offset: 0x018 (R)    Multiple Interrupt Status register */
    uint32_t REV_2[5];
    uint32_t HASH_MSG_LEN[4];             /* Offset: 0x030 (R/W)  message total length register */
    uint32_t HASH_MSG_CNT[4];             /* Offset: 0x040 (R/W)  message been handled length register */
    uint32_t REV_3[4];
    uint32_t HASH_KEY_LEN[1];             /* Offset: 0x060 (R/W)  HMAC KEY total length register */
    uint32_t REV_4[3];
    uint32_t HASH_KEY_CNT[1];             /* Offset: 0x070 (R/W)  HMAC KEY been handled length register */
    uint32_t REV_5[15];
    uint32_t HASH_MDIN_CR[1];             /* Offset: 0x0B0 (R/W)  Data flag register */
    uint32_t REV_6[3];
    uint32_t HASH_M_DIN[1];               /* Offset: 0x0C0 (W)    Hash message Input register */
    uint32_t REV_7[14];
    uint32_t HASH_VERSION[1];             /* Offset: 0x0FC (R)    Version register */
    uint32_t HASH_IN[50];                 /* Offset: 0x100 (W)    Hash iterator Input register */
    uint32_t REV_8[14];
    uint32_t HASH_OUT[50];                /* Offset: 0x200 (R)    Output register */
    uint32_t REV_9[14];
    uint32_t HASH_SEED[36];               /* Offset: 0x300 (R)    Seed Register, 36 words */
    uint32_t REV_10[64];
    uint32_t DMA_SA_L[1];                 /* Offset: 0x490 (R/W)  DMA Source Address Low part register */
    uint32_t DMA_SA_H[1];                 /* Offset: 0x494 (R/W)  DMA Source Address High part register */
    uint32_t DMA_DA_L[1];                 /* Offset: 0x498 (R/W)  DMA Destination Address Low part register */
    uint32_t DMA_DA_H[1];                 /* Offset: 0x49c (R/W)  DMA Destination Address High part register */
    uint32_t DMA_RLEN[1];                 /* Offset: 0x4A0 (R/W)  DMA read data length register */
    uint32_t DMA_WLEN[1];                 /* Offset: 0x4A4 (R/W)  DMA write data length register */
    uint32_t DMA_AWCC[1];                 /* Offset: 0x4A8 (R/W)  DMA AWCC register */
    uint32_t DMA_ARCC[1];                 /* Offset: 0x4AC (R/W)  DMA ARCC register */
    uint32_t REV_FINAL;
} hash_reg_st;

//hash register pointer
#ifdef CONFIG_UNIT_TEST
volatile static hash_reg_st * g_hash_reg = (hash_reg_st *)HASH_BASE_ADDR;
#else
volatile static hash_reg_st * const g_hash_reg = (hash_reg_st *)HASH_BASE_ADDR;
#endif


#define rHASH_CTRL         (*((volatile uint32_t *)(g_hash_reg->HASH_CTRL)))
#define rHASH_CFG          (*((volatile uint32_t *)(g_hash_reg->HASH_CFG)))
#define rHASH_RISR         (*((volatile uint32_t *)(g_hash_reg->HASH_RISR)))
#define rHASH_IMCR         (*((volatile uint32_t *)(g_hash_reg->HASH_IMCR)))
#define rHASH_MSG_LEN_BASE (*((volatile uint32_t *)(g_hash_reg->HASH_MSG_LEN)))
#define rHASH_MSG_CNT_BASE (*((volatile uint32_t *)(g_hash_reg->HASH_MSG_CNT)))
#define rHASH_K_LEN        (*((volatile uint32_t *)(g_hash_reg->HASH_KEY_LEN)))
#define rHASH_K_CNT        (*((volatile uint32_t *)(g_hash_reg->HASH_KEY_CNT)))
#define rHASH_MDIN_CR      (*((volatile uint32_t *)(g_hash_reg->HASH_MDIN_CR)))
#define rHASH_M_DIN        (*((volatile uint32_t *)(g_hash_reg->HASH_M_DIN)))
#define rHASH_VERSION      (*((volatile uint32_t *)(g_hash_reg->HASH_VERSION)))
#define rHASH_IN_BASE      (*((volatile uint32_t *)(g_hash_reg->HASH_IN)))
#define rHASH_OUT_BASE     (*((volatile uint32_t *)(g_hash_reg->HASH_OUT)))
#ifdef HASH_SEC
#define rHASH_SEED_BASE    (*((volatile uint32_t *)(g_hash_reg->HASH_SEED)))
#endif
#define rHASH_DMA_SA_L     (*((volatile uint32_t *)(g_hash_reg->DMA_SA_L)))
#define rHASH_DMA_SA_H     (*((volatile uint32_t *)(g_hash_reg->DMA_SA_H)))
#define rHASH_DMA_DA_L     (*((volatile uint32_t *)(g_hash_reg->DMA_DA_L)))
#define rHASH_DMA_DA_H     (*((volatile uint32_t *)(g_hash_reg->DMA_DA_H)))
#define rHASH_DMA_RLEN     (*((volatile uint32_t *)(g_hash_reg->DMA_RLEN)))
#define rHASH_DMA_WLEN     (*((volatile uint32_t *)(g_hash_reg->DMA_WLEN)))

#endif



/* function: get HFE IP version
 * parameters: none
 * return: HFE IP version
 * caution:
 */
uint32_t hash_get_version(void)
{
    return rHASH_VERSION;
}


/* function: get hash driver version
 * parameters: none
 * return: hash driver version(software version)
 * caution:
 */
uint32_t hash_get_driver_version(void)
{
    //the meaning of the version(for example, if the return value is 0x23080301)
    //the first 3 bytes:  23.08.03 ---- date
    //the last byte:      01       ---- first verion on the day
    uint32_t year = 24U;
    uint32_t month = 3U;
    uint32_t day = 4U;
    uint32_t verison = 1U;

    return (year<<24U) | (month<<16U) | (day<<8U) | verison;
}


/* function: set hash to be CPU mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_cpu_mode(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<HASH_DMA_OFFSET);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;
    lib_register_lock(g_hash_reg);
}


/* function: set hash to be DMA mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_dma_mode(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<HASH_DMA_OFFSET);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG |= flag;
    lib_register_lock(g_hash_reg);
}


/* function: set hash mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hash_mode(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<HASH_HMAC_OFFSET);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;
    lib_register_lock(g_hash_reg);
}


/* function: set hmac mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hmac_mode(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<HASH_HMAC_OFFSET);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG |= flag;
    lib_register_lock(g_hash_reg);
}


/* function: set hmac key mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_hmac_key_mode(void)
{
    MEM_VOLATILE uint32_t flag = 1U;

    lib_register_unlock(g_hash_reg);
    rHASH_MDIN_CR |= flag;
    lib_register_lock(g_hash_reg);
}


/* function: clear hmac key mode
 * parameters: none
 * return: none
 * caution:
 */
void hash_clear_hmac_key_mode(void)
{
    MEM_VOLATILE uint32_t mask = ~1U;

    lib_register_unlock(g_hash_reg);
    rHASH_MDIN_CR &= mask;
    lib_register_lock(g_hash_reg);
}


/* function: clear hash risr register
 * parameters: none
 * return: none
 * caution:
 */
void hash_clear_risr(void)
{
    MEM_VOLATILE uint32_t mask = ~3U;

    rHASH_RISR &= mask;
}


/* function: set opreated hmac key bitlen to register
 * parameters:
 *     cnt ------------------------ input, operated key bitlen
 * return: none
 * caution:none
 */
void hash_set_hmac_key_cnt(uint32_t bitlen)
{
    lib_register_unlock(g_hash_reg);
    rHASH_K_CNT = bitlen;
    lib_register_lock(g_hash_reg);
}


/* function: set total hmac key bitlen to register
 * parameters:
 *     bitlen --------------------- input, key bitlen
 * return: none
 * caution:none
 */
void hash_set_hmac_key_len(uint32_t bitlen)
{
    lib_register_unlock(g_hash_reg);
    rHASH_K_LEN = bitlen;
    lib_register_lock(g_hash_reg);
}


/* function: clear hash cfg register
 * parameters:
 * return: none
 * caution:none
 */
void hash_clear_cfg(void)
{
    lib_register_unlock(g_hash_reg);
    rHASH_CFG = 0U;
    lib_register_lock(g_hash_reg);
}


/* function: hmac key opr for one block
 * parameters:
 *     key --------------------- input, key
 *     block_byte_len ---------- input, block byte len
 * return: none
 * caution:none
 */
uint32_t hash_hmac_key_opr_one_block(const uint32_t *key, uint32_t block_byte_len)
{
    uint32_t i;
    uint32_t block_words_len = block_byte_len >> 2;

    hash_set_hmac_key_len(block_byte_len << 3);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);
    hash_start();

    for(i = 0U; i < block_words_len; i++)
    {
        rHASH_M_DIN = key[i];
    }

    return hash_wait_till_done();
}


/* function: hmac key opr for key size is bigger than one block
 * parameters:none
 * return: none
 * caution:none
 */
uint32_t hash_hmac_key_opr_longer_than_one_block(const uint8_t *key, uint32_t key_bytes)
{
    hash_set_hmac_key_len(key_bytes << 3);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);
    hash_start();
    hash_input_msg_u8(key, key_bytes);

    return hash_wait_till_done();
}


/* function: hmac secure port key opr for one block
 * parameters:
 *     sp_key_idx -------------- input, secure port key id
 *     key_bits ---------------- input, bit length of key
 * return: none
 * caution:none
 */
uint32_t hash_hmac_sp_key_opr(uint16_t sp_key_idx, uint32_t key_bits)
{
    uint32_t ret;

    hash_set_hmac_key_len(key_bits);
    hash_set_hmac_key_cnt(0);
    hash_set_last_block(1);

    hash_hmac_enable_secure_port();
    hash_update_config();

    ret = lib_hash_secure_port_config(sp_key_idx);
    if(HASH_SUCCESS == ret)
    {
        hash_start();
        ret = hash_wait_till_done();
    }
    else
    {}

    return ret;
}


/* function: set the specific hash algorithm
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: none
 * caution: 
 *     1. please make sure alg is valid
 */
void hash_set_alg(hash_alg_e alg)
{
    MEM_VOLATILE uint32_t mask = (~0x0000000FU);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;
    rHASH_CFG |= (uint32_t)alg;
    lib_register_lock(g_hash_reg);
}


#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
/* function: check whether hash_alg_e is SHA3
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: 1(yes), 0(no)
 * caution:
 *     1. please make sure hash_alg_e is valid
 */
uint32_t hash_check_whether_sha3_alg(hash_alg_e alg)
{
    uint32_t ret = 0U;

    if((alg >= HASH_SHA3_224) && (alg <= HASH_SHA3_512))
    {
        ret = 1U;
    }
    else
    {}

    return ret;
}
#endif


/* function: update hash config
 * parameters: none
 * return: none
 * caution: none
 */
void hash_update_config(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<HASH_UPDATE_CONFIG_OFFSET);
    MEM_VOLATILE uint32_t flag = ((uint32_t)1)<<HASH_UPDATE_CONFIG_OFFSET;
    MEM_VOLATILE uint32_t flag_1 = 1U;

    lib_register_unlock(g_hash_reg);
    rHASH_CFG |= flag;
    rHASH_CTRL |= flag_1;  //start
    lib_register_lock(g_hash_reg);

    (void)hash_wait_till_done();

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;
    lib_register_lock(g_hash_reg);
}

/* function: enable hash interruption in CPU mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_enable_cpu_interruption(void)
{
    MEM_VOLATILE uint32_t flag = 1U;

    rHASH_IMCR |= flag;
}


/* function: disable hash interruption in CPU mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_disable_cpu_interruption(void)
{
    MEM_VOLATILE uint32_t mask = ~1U;

    rHASH_IMCR &= mask;
}


/* function: enable hash interruption in DMA mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_enable_dma_interruption(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1) << 1);

    rHASH_IMCR |= flag;
}


/* function: disable hash interruption in DMA mode
 * parameters: none
 * return: none
 * caution: none
 */
void hash_disable_dma_interruption(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1) << 1);

    rHASH_IMCR &= mask;
}


/* function: set dma output bytes length
 * parameters:
 *     wlen ------------------------ input, byte length of the written data for hash hardware
 * return: none
 * caution: none
 */
void hash_set_dma_output_len(uint32_t bytes)
{
    lib_register_unlock(g_hash_reg);
    rHASH_DMA_WLEN = bytes;
    lib_register_lock(g_hash_reg);
}


/* function: set the tag whether current block is the last message block or not
 * parameters:
 *     tag ------------------------ input, 0(no), other(yes) 
 * return: none
 * caution: 
 *     1. if it is the last block, please config rHASH_MSG_LEN,
 *        then the hardware will do the padding and post-processing.
 */
void hash_set_last_block(uint32_t tag)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1) << HASH_LAST_BLOCK_OFFSET);
    MEM_VOLATILE uint32_t flag =  (((uint32_t)1) << HASH_LAST_BLOCK_OFFSET);

    lib_register_unlock(g_hash_reg);

    if(0U != tag)     //current block is the last one of the message
    {
        rHASH_MDIN_CR |= flag;
    }
    else        //current block is not the last one of the message
    {
        rHASH_MDIN_CR &= mask;
    }

    lib_register_lock(g_hash_reg);
}



/* function: set the hash endian
 * parameters: none
 * return: none
 * caution:
 */
void hash_set_endian_uint32(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)3)<<HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);
#ifdef HASH_REVERSE_BYTE_ORDER_IN_WORD
    MEM_VOLATILE uint32_t flag =  (((uint32_t)2)<<HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);
#endif

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;     //clear bit[9:8], and now CPU is big-endian

#ifdef HASH_REVERSE_BYTE_ORDER_IN_WORD
    rHASH_CFG |= flag;     //CPU is little-endian, input and output reversed by hardware----HFE IP
#endif
    lib_register_lock(g_hash_reg);
}


/* function: enable hmac secure port
 * parameters:
 *     sp_key_idx ----------------- input, index of secure port key
 * return: none
 * caution:
 */
void hash_hmac_enable_secure_port(void)
{
#ifdef HMAC_SECURE_PORT_FUNCTION
    MEM_VOLATILE uint32_t flag   = ((uint32_t)1)<<HASH_HMAC_SECURE_PORT_OFFSET;

    lib_register_unlock(g_hash_reg);
    rHASH_CFG |= flag;
    lib_register_lock(g_hash_reg);
#endif
}


/* function: disable hmac secure port
 * parameters:
 * return: none
 * caution:
 */
void hash_hmac_disable_secure_port(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<HASH_HMAC_SECURE_PORT_OFFSET);

    lib_register_unlock(g_hash_reg);
    rHASH_CFG &= mask;
    lib_register_lock(g_hash_reg);
}


/* function: get current HASH iterator value
 * parameters:
 *     iterator ------------------- output, current hash iterator
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 */
void hash_get_iterator(uint8_t *iterator, uint32_t hash_iterator_words)
{
    uint32_t temp;
    uint32_t i;

    if(0U != (((uintptr_t)iterator) & 3U))
    {
        for (i = 0U; i < hash_iterator_words; i++) //for the case that iterator is not aligned by word
        {
            temp = *((volatile uint32_t *)(&((&rHASH_OUT_BASE)[i])));
            memcpy_((&(iterator[(i<<2)])), (&temp), 4);
        }
    }
    else
    {
        for (i = 0U; i < hash_iterator_words; i++)
        {
            ((uint32_t *)iterator)[i] = *((volatile uint32_t *)(&((&rHASH_OUT_BASE)[i])));
        }
    }
}


/* function: input current iterator value
 * parameters:
 *     iterator ------------------- input, hash iterator value
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 *     1. iterator must be word aligned
 */
void hash_set_iterator(const uint32_t *iterator, uint32_t hash_iterator_words)
{
    uint32_t i;

    if(NULL != iterator)
    {
        for (i = 0U; i < hash_iterator_words; i++)
        {
            *((volatile uint32_t *)(&((&rHASH_IN_BASE)[i]))) = iterator[i];
        }
    }
    else    //for SHA3 init, iv is zero of 1600 bits
    {
        for (i = 0U; i < hash_iterator_words; i++)
        {
            *((volatile uint32_t *)(&((&rHASH_IN_BASE)[i]))) = 0;
        }
    }
}


/* function: set rHASH_MSG_LEN and rHASH_MSG_CNT
 * parameters:
 *     bytelen ------------------- input, msg byte length
 * return: none
 * caution:none
 */
void hash_set_msg_len(uint32_t bytelen)
{
    MEM_VOLATILE uint32_t flag = 0U;

    lib_register_unlock(g_hash_reg);

    *((volatile uint32_t *)(&((&rHASH_MSG_LEN_BASE)[0]))) = bytelen << 3;
    *((volatile uint32_t *)(&((&rHASH_MSG_LEN_BASE)[1]))) = bytelen>>(32-3);
    *((volatile uint32_t *)(&((&rHASH_MSG_LEN_BASE)[2]))) = flag;
    *((volatile uint32_t *)(&((&rHASH_MSG_LEN_BASE)[3]))) = flag;

    *((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[0]))) = flag;
    *((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[1]))) = flag;
    *((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[2]))) = flag;
    *((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[3]))) = flag;

    lib_register_lock(g_hash_reg);
}


/* function: set the total bit length of the whole message for HASH algorithm of MD structure
 * parameters:
 *     msg_total_bits ------------- input, total bit length of the whole message
 *     words ---------------------- input, word length of array msg_total_bits
 * return: none
 * caution:
 *     1. this is needed by padding for HASH algorithm of MD structure, not for SHA3
 */
void hash_set_msg_total_bit_len(const uint32_t *msg_total_bits, uint32_t block_byte_len)
{
    MEM_VOLATILE uint32_t mask_1 = 0xFFFFFE00U;
    MEM_VOLATILE uint32_t mask_2 = 0xFFFFFC00U;
    uint32_t words = HASH_BLOCK_MAX_WORD_LEN/8U;

    lib_register_unlock(g_hash_reg);

    while(0U != words)
    {
        words--;
        *((volatile uint32_t *)(&((&rHASH_MSG_LEN_BASE)[words]))) = msg_total_bits[words];
        *((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[words]))) = msg_total_bits[words];
    }

    if(64U == block_byte_len)
    {
        (*((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[0])))) &= mask_1;
    }
    else
    {
        (*((volatile uint32_t *)(&((&rHASH_MSG_CNT_BASE)[0])))) &= mask_2;
    }

    lib_register_lock(g_hash_reg);
}


#ifdef HASH_SEC
/* function: set hash_hp seed
 * parameters: none
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hash_set_seed(void)
{
    uint32_t ret;

    ret = get_rand((uint8_t *)&(rHASH_SEED(0U)), ((uint32_t)HASH_SEED_WORDS)<<2);
    if(TRNG_SUCCESS != ret)
    {
        return ret;
    }
    else
    {
        return HASH_SUCCESS;
    }
}
#endif


/* function: start HASH iteration calc
 * parameters: none
 * return: none
 * caution:
 */
void hash_start(void)
{
    MEM_VOLATILE uint32_t clear_flag = 0U;
    MEM_VOLATILE uint32_t start_flag = 1U;

    lib_register_unlock(g_hash_reg);
    rHASH_RISR = clear_flag;
    rHASH_CTRL |= start_flag;
    lib_register_lock(g_hash_reg);
}


/* function: wait till done
 * parameters: none
 * return: none
 * caution:
 */
uint32_t hash_wait_till_done(void)
{

#if (defined(HASH_SEC) || (defined(HASH_CONFIG_SUPPORT_STATIC_ANALYSIS)))
    MEM_VOLATILE uint32_t alarm_flag = (1U<<HASH_ATTACK_ALARM_OFFSET);
#endif
    MEM_VOLATILE uint32_t finish_flag = 1U;
    uint32_t ret = HASH_SUCCESS;

    while(0U != (rHASH_CTRL & finish_flag))
    {}

#if (defined(HASH_SEC) || (defined(HASH_CONFIG_SUPPORT_STATIC_ANALYSIS)))
    if(0U != (rHASH_RISR & alarm_flag))
    {
        ret = HASH_ATTACK_ALARM;
    }
    else
    {}
#endif

    return ret;
}


/* function: DMA wait till done
 * parameters:
 *     callback ------------------- input, callback function pointer
 * return: none
 * caution:
 */
uint32_t hash_dma_wait_till_done(HASH_CALLBACK callback)
{
#if (defined(HASH_SEC) || (defined(HASH_CONFIG_SUPPORT_STATIC_ANALYSIS)))
    MEM_VOLATILE uint32_t alarm_flag = (1U<<HASH_ATTACK_ALARM_OFFSET);
#endif
    MEM_VOLATILE uint32_t finish_flag = 1U;
    uint32_t ret = HASH_SUCCESS;

    while(0U != (rHASH_CTRL & finish_flag))
    {
        if(NULL != callback)
        {
            callback();
        }
        else
        {}
    }

#if (defined(HASH_SEC) || (defined(HASH_CONFIG_SUPPORT_STATIC_ANALYSIS)))
    if(0U != (rHASH_RISR & alarm_flag))
    {
        ret = HASH_ATTACK_ALARM;
    }
    else
    {}
#endif

    return ret;
}


/* function: input message(at most a block)
 * parameters:
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of msg, can not be greater than block bytes
 * return: none
 * caution:
 *     1. msg_bytes can not be greater than block bytes
 */
void hash_input_msg_u8(const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t msg_words = msg_bytes>>2;
    uint32_t remainder_bytes = msg_bytes&(3U);
    const uint8_t* current_msg = msg;
    uint32_t tmp = 0U;

    if(0U != (((uintptr_t)current_msg) & 3U))
    {
        while(0U != msg_words)
        {
            memcpy_((&tmp), current_msg, 4);
            rHASH_M_DIN = tmp;
            current_msg = &(current_msg[4]);
            msg_words--;
        }
    }
    else
    {
        while(0U != msg_words)
        {
            //here current_msg is aligned word
            rHASH_M_DIN = *((uint32_t *)current_msg);
            current_msg = &(current_msg[4]);
            msg_words--;
        }
    }

    if(0U != remainder_bytes)
    {
        tmp = 0U;
        memcpy_(&tmp, current_msg, remainder_bytes);
        rHASH_M_DIN = tmp;
    }
    else
    {}
}


#ifdef HASH_DMA_FUNCTION
/* function: basic HASH DMA operation
 * parameters:
 *     in ------------------------- input, message of some blocks, or message including the last byte(last block)
 *     out ------------------------ output, hash digest or hmac.
 *     inByteLen ------------------ input, actual byte length of input msg
 *     callback ------------------- input, callback function pointer
 * return: none
 * caution:
 *     1. for DMA operation, the unit of input and output is 4 words, so, please make sure the buffer
 *        out is sufficient.
 *     2. if just to input message, not to get digest or hmac, please set para out to be NULL and WLEN to be 0.
 *        if to get the digest or hmac, para out can not be NULL, and please set WLEN to be digest length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_operate(uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t inByteLen, 
        HASH_CALLBACK callback)
#else
uint32_t hash_dma_operate(const uint32_t *in, const uint32_t *out, uint32_t inByteLen, HASH_CALLBACK callback)
#endif
{
    lib_register_unlock(g_hash_reg);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    rHASH_DMA_SA_L = (uint32_t)in_l;
    rHASH_DMA_DA_L = (uint32_t)out_l;
    rHASH_DMA_SA_H = (uint32_t)in_h;
    rHASH_DMA_DA_H = (uint32_t)out_h;
#else
    //set src and dst addr
    rHASH_DMA_SA_L = (uint32_t)(((uint64_t)in)&0xFFFFFFFFU);
    rHASH_DMA_DA_L = (uint32_t)(((uint64_t)out)&0xFFFFFFFFU);
    if(sizeof(uint32_t *) != 4U)
    {
        rHASH_DMA_SA_H = (uint32_t)(((uint64_t)in)>>32);
        rHASH_DMA_DA_H = (uint32_t)(((uint64_t)out)>>32);
    }
    else
    {
        rHASH_DMA_SA_H = 0U;
        rHASH_DMA_DA_H = 0U;
    }
#endif

    //data byte length
    rHASH_DMA_RLEN = inByteLen;

    lib_register_lock(g_hash_reg);

    hash_start();

    return hash_dma_wait_till_done(callback);
}

#endif
