
#include "trng_basic.h"
#include "../crypto_include/crypto_common/utility.h"
#include "../crypto_lib/lib_extension.h"


static uint32_t g_rand_buffer[625u + 7u];  //20000/32 = 625
static uint32_t g_buffer_used_words   = 0u;
static uint32_t g_buffer_total_words  = 0u;
static uint32_t g_buffer_regen_words  = 0u;
static uint32_t g_buffer_regen_status = TRNG_SUCCESS;


#if 1
//HASH register struct 
typedef struct {
    uint32_t cr[1];                        /* Offset: 0x000 (W1S) TRNG Control Register */
    uint32_t msel[1];                      /* Offset: 0x004 (R/W) TRNG Config Register */
    uint32_t sr[1];                        /* Offset: 0x008 (R)   TRNG Status Register */
    uint32_t dr[1];                        /* Offset: 0x00C (W0C) TRNG Interrupt Source Register */
    uint32_t reseed[1];                    /* Offset: 0x010 (R/W) TRNG Interrupt Enable Register */
    uint32_t ro_clk_en[1];                 /* Offset: 0x014 (R/W) TRNG Interrupt Enable Register */
    uint32_t ro_src_en1[1];                /* Offset: 0x018 (R/W) TRNG Interrupt Enable Register */
    uint32_t ro_src_en2[1];                /* Offset: 0x01C (R/W) TRNG Interrupt Enable Register */
    uint32_t ro_ht_cr[1];                  /* Offset: 0x020 (R/W) TRNG Interrupt Enable Register */
    uint32_t ro_ht_sr[1];                  /* Offset: 0x024 (R/W) TRNG Interrupt Enable Register */
    uint32_t rev1[2];
    uint32_t version[1];                   /* Offset: 0x030 (R/W) TRNG Interrupt Enable Register */
    uint32_t drbg_alg_hw_sr[1];            /* Offset: 0x034 (R/W) TRNG Interrupt Enable Register */
    uint32_t drbg_alg_mode_sr[1];          /* Offset: 0x038 (R/W) TRNG Interrupt Enable Register */
    uint32_t rev2[1];
    uint32_t trht_iteration_ptr[1];        /* Offset: 0x040 (R/W) TRNG Interrupt Enable Register */
    uint32_t rev3[15];
    uint32_t trbg_ht_win_cfg[1];           /* Offset: 0x080 (R/W) TRNG Interrupt Enable Register */
    uint32_t rev4[3];
    uint32_t tapt_thld_cfg[1];             /* Offset: 0x090 (R/W) TRNG Interrupt Enable Register */
    uint32_t taptnb_thld_cfg[1];           /* Offset: 0x094 (R/W) TRNG Interrupt Enable Register */
    uint32_t tpoker_thld_cfg[1];           /* Offset: 0x098 (R/W) TRNG Interrupt Enable Register */
    uint32_t truns_thld_cfg[1];            /* Offset: 0x09C (R/W) TRNG Interrupt Enable Register */
    uint32_t rev_final;
} trng_reg_st;


//ske_hp register pointer
#ifdef CONFIG_UNIT_TEST
volatile static trng_reg_st * g_trng_reg = (trng_reg_st *)(TRNG_BASE_ADDR);
#else
volatile static trng_reg_st * const g_trng_reg = (trng_reg_st *)(TRNG_BASE_ADDR);
#endif


#define rTRNG_CR              (*((volatile uint32_t *)(g_trng_reg->cr)))
#define rTRNG_MSEL            (*((volatile uint32_t *)(g_trng_reg->msel)))
#define rTRNG_SR              (*((volatile uint32_t *)(g_trng_reg->sr)))
#define rTRNG_DR              (*((volatile uint32_t *)(g_trng_reg->dr)))
#define rTRNG_RESEED          (*((volatile uint32_t *)(g_trng_reg->reseed)))
#define rRO_CLK_EN            (*((volatile uint32_t *)(g_trng_reg->ro_clk_en)))
#define rRO_SRC_EN1           (*((volatile uint32_t *)(g_trng_reg->ro_src_en1)))
#define rRO_SRC_EN2           (*((volatile uint32_t *)(g_trng_reg->ro_src_en2)))
#define rTRNG_HT_CR           (*((volatile uint32_t *)(g_trng_reg->ro_ht_cr)))
#define rTRNG_HT_SR           (*((volatile uint32_t *)(g_trng_reg->sr)))
#define rTRNG_VERSION         (*((volatile uint32_t *)(g_trng_reg->version)))
#define rDRBG_ALG_HW_SR       (*((volatile uint32_t *)(g_trng_reg->drbg_alg_hw_sr)))
#define rDRBG_ALG_MODE_SEL    (*((volatile uint32_t *)(g_trng_reg->drbg_alg_mode_sr)))
#define rTRHT_ITERATION_PTR   (*((volatile uint32_t *)(g_trng_reg->trht_iteration_ptr)))
#define rTRBG_HT_WIN_CFG      (*((volatile uint32_t *)(g_trng_reg->trbg_ht_win_cfg)))
#define rTAPT_THLD_CFG        (*((volatile uint32_t *)(g_trng_reg->tapt_thld_cfg)))
#define rTAPTNB_THLD_CFG      (*((volatile uint32_t *)(g_trng_reg->taptnb_thld_cfg)))
#define rTPOKER_THLD_CFG      (*((volatile uint32_t *)(g_trng_reg->tpoker_thld_cfg)))
#define rTRUNS_THLD_CFG       (*((volatile uint32_t *)(g_trng_reg->truns_thld_cfg)))
#endif


/* function: get trng IP version
 * parameters: none
 * return: trng IP version(hardware version)
 * caution:
 */
uint32_t trng_get_version(void)
{
    return rTRNG_VERSION;
}


/* function: get trng driver version
 * parameters: none
 * return: trng driver version(software version)
 * caution:
 */
uint32_t trng_get_driver_version(void)
{
    //the meaning of the version(for example, if the return value is 0x23080301)
    //the first 3 bytes:  23.08.03 ---- date
    //the last byte:      01       ---- first verion on the day
    uint32_t year = 24U;
    uint32_t month = 5U;
    uint32_t day = 20U;
    uint32_t verison = 1U;

    return (year<<24U) | (month<<16U) | (day<<8U) | verison;
}


/* function: TRNG global interruption enable
 * parameters: none
 * return: none
 * caution:
 */
void trng_global_int_enable(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1U)<<TRNG_GLOBAL_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR |= flag;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG global interruption disable
 * parameters: none
 * return: none
 * caution:
 */
void trng_global_int_disable(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1U)<<TRNG_GLOBAL_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR &= mask;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG empty-read interruption enable
 * parameters: none
 * return: none
 * caution:
 *     1. works when global interruption is enabled
 */
void trng_empty_read_int_enable(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1U)<<TRNG_READ_EMPTY_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR |= flag;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG empty-read interruption disable
 * parameters: none
 * return: none
 * caution:
 */
void trng_empty_read_int_disable(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1U)<<TRNG_READ_EMPTY_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR &= mask;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG data interruption enable
 * parameters: none
 * return: none
 * caution:
 *     1. works when global interruption is enabled
 */
void trng_data_int_enable(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1U)<<TRNG_DATA_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR |= flag;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG data interruption disable
 * parameters: none
 * return: none
 * caution:
 */
void trng_data_int_disable(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1U)<<TRNG_DATA_INT_OFFSET);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR &= mask;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG enable
 * parameters: none
 * return: none
 * caution:
 */
void trng_enable(void)
{
    MEM_VOLATILE uint32_t flag = 1U;

    lib_register_unlock(g_trng_reg);
    rTRNG_CR |= flag;
    lib_register_lock(g_trng_reg);
}


/* function: TRNG disable
 * parameters: none
 * return: none
 * caution:
 */
void trng_disable(void)
{
    MEM_VOLATILE uint32_t mask = ~((uint32_t)1U);

    lib_register_unlock(g_trng_reg);
    rTRNG_CR &= mask;
    lib_register_lock(g_trng_reg);

    //sleep for a while until the entropy is stable before enabling it.
    uint32_sleep(TRNG_DELAY_COUNTER, 0);
}


/* function: TRNG get alg
 * parameters: none
 * return: TRNG_ALG_SM4 or TRNG_ALG_AES
 * caution:
 */
uint32_t trng_get_drbg_alg(void)
{
    MEM_VOLATILE uint32_t flag = (uint32_t)1U;
    uint32_t ret;
    
    if(0U == (rDRBG_ALG_MODE_SEL & flag))
    {
        ret = TRNG_ALG_SM4;
    }
    else
    {
        ret = TRNG_ALG_AES;
    }
    
    return ret;
}


/* function: TRNG get mode
 * parameters: none
 * return: TRNG_MODE_DRBG or TRNG_MODE_NON_DRBG
 * caution:
 */
uint32_t trng_get_mode(void)
{
    MEM_VOLATILE uint32_t flag = (uint32_t)1U;
    uint32_t ret;
    
    if(0U == (rTRNG_MSEL & flag))
    {
        ret = TRNG_MODE_NON_DRBG;
    }
    else
    {
        ret = TRNG_MODE_DRBG;
    }
    
    return ret;
}
    

/* function: TRNG get mode
 * parameters: none
 * return: TRNG_DRBG_CBC or TRNG_DRBG_CTR
 * caution:
 */
uint32_t trng_get_drbg_mode(void)
{
    MEM_VOLATILE uint32_t flag = (uint32_t)1U<<4;
    uint32_t ret;
    
    if(0U == (rDRBG_ALG_MODE_SEL & flag))
    {
        ret = TRNG_DRBG_MODE_CTR;
    }
    else
    {
        ret = TRNG_DRBG_MODE_CBC;
    }
    
    return ret;
}


#ifdef TRNG_RO_ENTROPY
/* function: check if TRNG self test ready.
 * parameters:
 * return: 1(ready), 0(not ready)
 * caution:
 *     1. this works while i_skip_startup is 0. if i_skip_startup is 1, no 
 *        need to check this.
 */
uint32_t trng_if_self_test_ready(void)
{
    uint32_t ret;

    MEM_VOLATILE uint32_t flag = (((uint32_t)1U)<<TRNG_SELF_TEST_READY_OFFSET);

    if(0U != (rTRNG_SR & flag))
    {
        ret = 1U;
    }
    else
    {
        ret = 0U;
    }

    return ret;
}


/* function: set RO entropy config
 * parameters:
 *     cfg ------------------------ RO entropy config, only the low 4 bits are valid, every bit
 *                                  indicates one RO entropy, the MSB is RO 4, and LSB is RO 1
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. only the low 4 bits of cfg are valid
 *     2. if the low 4 bits of cig is 0, that means to disable all RO entropy
 */
uint32_t trng_ro_entropy_config(uint8_t cfg)
{
    uint32_t ret = TRNG_SUCCESS;

    MEM_VOLATILE uint32_t mask = ~(0x0000000FU);

    if(cfg > 15U)
    {
        ret = TRNG_INVALID_INPUT;
    }
    else
    {}

    lib_register_unlock(g_trng_reg);
    rRO_CLK_EN = (rRO_CLK_EN & mask)|((uint32_t)cfg);
    lib_register_lock(g_trng_reg);

    return ret;
}


/* function: set sub RO entropy config
 * parameters:
 *     sn ------------------------- input, RO entropy source series number, must be in [1,4]
 *     value ---------------------- input, the config value of RO sn
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t trng_ro_sub_entropy_config(uint8_t sn, uint16_t cfg)
{
    MEM_VOLATILE uint32_t mask_high = ~0xFFFF0000U;
    MEM_VOLATILE uint32_t mask_low  = ~0x0000FFFFU;
    uint32_t ret = TRNG_SUCCESS;

    lib_register_unlock(g_trng_reg);

    switch(sn)
    {
        case 1:
            rRO_SRC_EN1 = (rRO_SRC_EN1 & mask_high)|(((uint32_t)cfg)<<16);
            break;

        case 2:
            rRO_SRC_EN1 = (rRO_SRC_EN1 & mask_low)|((uint32_t)cfg);
            break;

        case 3:
            rRO_SRC_EN2 = (rRO_SRC_EN2 & mask_high)|(((uint32_t)cfg)<<16);
            break;

        case 4:
            rRO_SRC_EN2 = (rRO_SRC_EN2 & mask_low)|((uint32_t)cfg);
            break;

        default:
            ret = TRNG_INVALID_INPUT;
            break;
    }

    lib_register_lock(g_trng_reg);

    return ret;
}


/* function: set TRNG mode
 * parameters:
 *     with_post_processing ------- 0:no,  other:yes
 * return: none
 * caution:
 */
void trng_set_mode(uint8_t with_post_processing)
{
    MEM_VOLATILE uint32_t mask = ~((uint32_t)1U);
    MEM_VOLATILE uint32_t flag = 1U;
    MEM_VOLATILE uint32_t clear_flag = 0x00000007U;

    lib_register_unlock(g_trng_reg);

    if((uint8_t)0 != with_post_processing)
    {
        rTRNG_MSEL |= flag;
    }
    else
    {
        rTRNG_MSEL &= mask;
    }

    rTRNG_SR |= clear_flag; //write 1 to clear

    lib_register_lock(g_trng_reg);
}


/* function: reseed TRNG(works when DRBG is enabled)
 * parameters: none
 * return: none
 * caution:
 *     1. used for DRBG
 */
void trng_reseed(void)
{
    MEM_VOLATILE uint32_t flag = 1U;
    MEM_VOLATILE uint32_t clear_flag = 0x00000007U;

    rTRNG_RESEED |= flag;

    rTRNG_SR |= clear_flag; //write 1 to clear
}


/* function: TRNG set frequency
 * parameters:
 *     freq ----------------------- input, frequency config, must be in [0,3], and
 *                                  0: 1/4 of input frequency,
 *                                  1: 1/8 ...,
 *                                  2: 1/16 ...,
 *                                  3: 1/32 ...,
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t trng_set_freq(uint8_t freq)
{
    uint32_t ret = TRNG_SUCCESS;

    MEM_VOLATILE uint32_t mask = ~(((uint32_t)0x00000003U)<<TRNG_FREQ_OFFSET);

    if(freq > 3U)
    {
        ret = TRNG_INVALID_INPUT;
    }
    else
    {}

    lib_register_unlock(g_trng_reg);
    rRO_CLK_EN = (rRO_CLK_EN & mask)|(((uint32_t)freq)<<TRNG_FREQ_OFFSET);
    lib_register_lock(g_trng_reg);

    return ret;
}


/* function: get hardware drbg alg ablility
 * parameters: none
 * return: 
 *     if (return value | DRBG_LFSR_HW_EN) is not 0, then LFSR enabled.
 *     if (return value | DRBG_AES_HW_EN) is not 0, then AES-128 enabled.
 *     if (return value | DRBG_SM4_HW_EN) is not 0, then SM4 enabled.
 * caution:
 *     1. 
 */
uint32_t trng_get_hw_drbg_alg_ability(void)
{
    return rDRBG_ALG_HW_SR;
}


/* function: set drbg alg and mode config
 * parameters:
 *     value ---------------------- input, config value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_drbg_alg_mode_config(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rDRBG_ALG_MODE_SEL = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set health rest config
 * parameters:
 *     value ---------------------- input, config value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_ht_config(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rTRNG_HT_CR = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set word length of poker and aptnb test window
 * parameters:
 *     words ---------------------- input, word length of the window
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_poker_aptnb_test_win_size(uint32_t words)
{
    lib_register_unlock(g_trng_reg);
    rTRBG_HT_WIN_CFG = (rTRBG_HT_WIN_CFG & 0x03FFU)|(words<<16);
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set word length of apt and runs test window
 * parameters:
 *     words ---------------------- input, word length of the window
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_apt_runs_test_win_size(uint32_t words)
{
    lib_register_unlock(g_trng_reg);
    rTRBG_HT_WIN_CFG = (rTRBG_HT_WIN_CFG & (0x03FFUL<<16))|words;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set threshold of apt test(i.e. frequency monobit test)
 * parameters:
 *     value ---------------------- input, threshold value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_apt_test_threshold(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rTAPT_THLD_CFG = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set threshold of apt_non_binary test
 * parameters:
 *     value ---------------------- input, threshold value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_apt_non_binary_test_threshold(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rTAPTNB_THLD_CFG = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set threshold of poker test
 * parameters:
 *     value ---------------------- input, threshold value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_poker_test_threshold(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rTPOKER_THLD_CFG = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: set threshold of runs test
 * parameters:
 *     value ---------------------- input, threshold value
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please do this when TRNG_CR.TRNGEN is 0.
 */
uint32_t trng_set_runs_test_threshold(uint32_t value)
{
    lib_register_unlock(g_trng_reg);
    rTRUNS_THLD_CFG = value;
    lib_register_lock(g_trng_reg);

    return TRNG_SUCCESS;
}


/* function: nist global init config
 * parameters:
 *     drbg_alg_mode -------------- input, drbg algorithm and mode
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. drbg_alg_mode should be DRBG_SM4_CTR or DRBG_AES_128_CTR
 */
uint32_t trng_set_nist_global_init_config(uint32_t drbg_alg_mode)
{
    trng_disable();

    rTRNG_SR |= 7u;    //clear

    //set mode(with post-processing)
    trng_set_mode(1u);

    //set alg and mode in post-processing
    (void)trng_drbg_alg_mode_config(drbg_alg_mode);

    //HT config
    (void)trng_ht_config(NIST_HT_CONF);

    (void)trng_set_poker_aptnb_test_win_size(2048/32);
    (void)trng_set_apt_runs_test_win_size(1024/32);

    (void)trng_set_apt_test_threshold(APT_WIN_1024_BITS_THRESHOLD);
    (void)trng_set_apt_non_binary_test_threshold(APTNB_WIN_2048_BITS_THRESHOLD);

    trng_enable();

#if 0
    reg_scan_test();
#endif

    return TRNG_SUCCESS;
}


/* function: latest gm global init config
 * parameters: none
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
uint32_t trng_set_latest_gm_global_init_config(void)
{
    trng_disable();

    rTRNG_SR |= 7u;

    //set mode(with post-processing)
    trng_set_mode(1u);

    //set SM4 and CBC in post-processing
    (void)trng_drbg_alg_mode_config(DRBG_SM4_CBC);

    //HT config
    (void)trng_ht_config(LATEST_GM_HT_CONF);

    (void)trng_set_poker_aptnb_test_win_size(20000/32);
    (void)trng_set_apt_runs_test_win_size(20000/32);

    (void)trng_set_poker_test_threshold(POKER_WIN_20000_BITS_THRESHOLD);
    (void)trng_set_apt_test_threshold(APT_WIN_20000_BITS_THRESHOLD);
    (void)trng_set_runs_test_threshold(RUNS_WIN_20000_BITS_THRESHOLD);

    //buffer is empty
    g_buffer_used_words   = 0u;
    g_buffer_total_words  = 0u;
    g_buffer_regen_words  = 0u;
    g_buffer_regen_status = TRNG_SUCCESS;

    trng_enable();

#if 0
    reg_scan_test();
#endif

    return TRNG_SUCCESS;
}


/* function: wait till trng data is ready
 * parameters: 
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t wait_till_DT_ready(void)
{
    uint32_t ret = TRNG_ERROR;
    MEM_VOLATILE uint32_t DT_ready_flag = 2U;
    MEM_VOLATILE uint32_t HT_error_flag = 1U;
    MEM_VOLATILE uint32_t clear_flag = 7U;
    volatile uint32_t cnt = 0u;

    do {
        if(0U != (rTRNG_SR & DT_ready_flag))
        {
            ret = TRNG_SUCCESS;
            break;
        }
        else if(0U != (rTRNG_SR & HT_error_flag))
        {
#ifdef TRNG_DEBUG
            printf("\r\n *****************HT error  ------- \r\n");
#endif
            trng_disable();
            rTRNG_SR |= clear_flag;  //clear (alarm) status
            trng_enable();

            ret = TRNG_HT_ERROR;
        }
        else
        {
            cnt += 1U;
            if (cnt > TRNG_TIMEOUT_COUNTER_THRESHOLD)
            {
#ifdef TRNG_DEBUG
                printf("\r\n *****************TIMEOUT error  ------- \r\n");
#endif
                trng_disable();
                rTRNG_SR |= clear_flag;  //clear (alarm) status
                trng_enable();

                ret = TRNG_TIMEOUT_ERROR;
            }
            else
            {}
        }
    } while(TRNG_ERROR == ret);

    return ret;
}


/* function: get random numbers and store in the 20000-bit buffer(latest gm standard)
 * parameters: 
 *     kept_words ----------------- random number words in 20000-bit buffer before calling this function
 *     regen_total_words ---------- random number words in 20000-bit buffer after calling this function
 *     read_last_8_words ---------- read the last 8 words in trng data register or not read.
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. both kept_words and regen_total_words must be multiples of 8.
 *     2. please make sure that 0 <= kept_words <= regen_total_words <= 624.
 *     3. if regen_total_words < 624, regen_total_words shoule be equla to TRHT_ITERATION_PTR.TRHT_ITERATION_PTR 
 *        after calling this function, if regen_total_words is 624, TRHT_ITERATION_PTR.TRHT_ITERATION_PTR is 624
 *        or 632 after calling this function.
 *     4. if regen_total_words is 624, g_buffer_total_words will be 624 or 632 after calling this function. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_gen(uint32_t kept_words, uint32_t regen_total_words, 
        uint32_t read_last_8_words)
{
    uint32_t ret;
    MEM_VOLATILE uint32_t DT_ready_flag = 2U;
    uint32_t i;

#ifdef TRNG_DEBUG
    printf("\r\n %d, %d", kept_words, regen_total_words);
#endif

    ret = TRNG_SUCCESS;
    for(i=kept_words; i<regen_total_words; i+=8u)
    {
        ret = wait_till_DT_ready();
        if(TRNG_SUCCESS == ret)
        {
            g_rand_buffer[i+0u] = rTRNG_DR;
            g_rand_buffer[i+1u] = rTRNG_DR;
            g_rand_buffer[i+2u] = rTRNG_DR;
            g_rand_buffer[i+3u] = rTRNG_DR;
            g_rand_buffer[i+4u] = rTRNG_DR;
            g_rand_buffer[i+5u] = rTRNG_DR;
            g_rand_buffer[i+6u] = rTRNG_DR;
            g_rand_buffer[i+7u] = rTRNG_DR;

            rTRNG_SR |= DT_ready_flag;  //clear

#if 0
            trng_reseed();
#endif
        }
        else
        {
            break;
        }
    }

    if((TRNG_SUCCESS == ret) && (624u == regen_total_words))
    {
        ret = wait_till_DT_ready();
        if(TRNG_SUCCESS == ret)
        {
#ifdef TRNG_DEBUG
            printf("\r\n rTRHT_ITERATION_PTR = %d ------- \r\n", rTRHT_ITERATION_PTR & 0x3FFU);
#endif
            if(0u != read_last_8_words)
            {
                if(625U == (rTRHT_ITERATION_PTR & 0x3FFU))
                {
                    g_rand_buffer[i+0u] = rTRNG_DR;
                    g_rand_buffer[i+1u] = rTRNG_DR;
                    g_rand_buffer[i+2u] = rTRNG_DR;
                    g_rand_buffer[i+3u] = rTRNG_DR;
                    g_rand_buffer[i+4u] = rTRNG_DR;
                    g_rand_buffer[i+5u] = rTRNG_DR;
                    g_rand_buffer[i+6u] = rTRNG_DR;
                    g_rand_buffer[i+7u] = rTRNG_DR;

                    rTRNG_SR |= DT_ready_flag;  //clear

#if 0
                    trng_reseed();
#endif

                    g_buffer_total_words = 624u+8u;
                }
                else
                {
                    g_buffer_total_words = 624u;
                }
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


/* function: get full 20000-bit random numbers and store in the 20000 bit buffer(latest gm standard)
 * parameters: none
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_full(void)
{
    uint32_t ret;

    g_buffer_used_words  = 0u;
    g_buffer_total_words = 0u;

    ret = get_latest_gm_rand_buffer_gen(0, 624u, 1);

#ifdef TRNG_DEBUG
    print_buf_U8((uint8_t *)g_rand_buffer, g_buffer_total_words<<2, "g_rand_buffer");
#endif

    return ret;
}


/* function: get random numbers case 1(latest gm standard)
 * parameters: 
 *     regen_words ---------------- random number words to generate for case 1
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_regen_case_1_less(uint32_t regen_words)
{
    uint32_t ret;

    if(TRNG_SUCCESS == g_buffer_regen_status)
    {
        ret = get_latest_gm_rand_buffer_gen(g_buffer_regen_words, g_buffer_regen_words + regen_words, 0u);
        if(TRNG_SUCCESS == ret)
        {
            g_buffer_regen_words += regen_words;
        }
        else
        {
            g_buffer_regen_words  = 0u;
            g_buffer_regen_status = ret;
        }
    }
    else
    {}

    return TRNG_SUCCESS;
}


/* function: get random numbers case 2(latest gm standard)
 * parameters: none
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_regen_case_2_equal(void)
{
    uint32_t ret;

    if(TRNG_SUCCESS == g_buffer_regen_status)
    {
        ret = get_latest_gm_rand_buffer_gen(g_buffer_regen_words, 624u, 1u);
        if(TRNG_SUCCESS != ret)
        {
            g_buffer_total_words = 0u;
            g_buffer_regen_status = ret;
        }
        else
        {
#ifdef TRNG_DEBUG
            print_buf_U8((uint8_t *)g_rand_buffer, g_buffer_total_words<<2, "g_rand_buffer");
#endif
        }
    }
    else
    {
        g_buffer_total_words = 0u;
    }

    g_buffer_used_words  = 0u;
    g_buffer_regen_words = 0u;

    return TRNG_SUCCESS;
}


/* function: get random numbers case 3(latest gm standard)
 * parameters: none
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_regen_case_3_greater(void)
{
    uint32_t ret;

    if(TRNG_SUCCESS == g_buffer_regen_status)
    {
        ret = get_latest_gm_rand_buffer_gen(g_buffer_regen_words, 624u, 1u);
        if(TRNG_SUCCESS != ret)
        {
            g_buffer_total_words  = 0u;
        }
        else
        {
#ifdef TRNG_DEBUG
            print_buf_U8((uint8_t *)g_rand_buffer, g_buffer_total_words<<2, "g_rand_buffer");
#endif
        }
    }
    else
    {
        g_buffer_total_words  = 0u;
        ret = g_buffer_regen_status;
        g_buffer_regen_status = TRNG_SUCCESS;
    }

    g_buffer_used_words  = 0u;
    g_buffer_regen_words = 0u;

    return ret;
}


/* function: get random numbers case 4(latest gm standard)
 * parameters: 
 *     regen_words ---------------- random number words to generate for case 4
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_buffer_regen_case_4_remainder(uint32_t regen_words)
{
    uint32_t ret;

    ret = get_latest_gm_rand_buffer_gen(0, regen_words, 0u);
    if(TRNG_SUCCESS == ret)
    {
        g_buffer_regen_words = regen_words;
    }
    else
    {
        g_buffer_regen_words  = 0u;
        g_buffer_regen_status = ret;
    }

    return ret;
}


/* function: get random numbers for internal calling(latest gm standard)
 * parameters: 
 *     random ----------------------- output, random number in byte style
 *     words ---------------------- input, random number words
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. the output random number is words*4 bytes.
 */
FLAG_STATIC uint32_t get_latest_gm_rand_by_word_internal(uint8_t *random, uint32_t words)
{
    uint32_t ret, is_over = 0u, regen_words, remainder_words;
    uint32_t words_t = words;
    uint8_t *rand_p = random;

    if(0u == g_buffer_total_words)    //initial case, or the buffer is empty due to HT/TIMEOUT error.
    {
        g_buffer_used_words = 0u;
        g_buffer_regen_words = 0u;

        if(TRNG_SUCCESS != g_buffer_regen_status)
        {
            is_over = 1u;
            ret = g_buffer_regen_status;
            g_buffer_regen_status = TRNG_SUCCESS;
        }
        else
        {
            ret = get_latest_gm_rand_buffer_full();
        }
    }
    else
    {
        ret = TRNG_SUCCESS;
    }

    if((0u == is_over) && (TRNG_SUCCESS == ret))
    {
        remainder_words = g_buffer_total_words - g_buffer_used_words;
        if(remainder_words > words_t)
        {
            memcpy_(rand_p, (uint8_t *)&g_rand_buffer[g_buffer_used_words], words_t<<2);

            regen_words = (g_buffer_used_words + words_t - g_buffer_regen_words) & (~0x07u);
            (void)get_latest_gm_rand_buffer_regen_case_1_less(regen_words);

            g_buffer_used_words += words_t;
            is_over = 1u;
            ret = TRNG_SUCCESS;
        }
        else if(remainder_words == words_t)
        {
            memcpy_(rand_p, (uint8_t *)&g_rand_buffer[g_buffer_used_words], remainder_words<<2);

            (void)get_latest_gm_rand_buffer_regen_case_2_equal();

            is_over = 1u;
            ret = TRNG_SUCCESS;
        }
        else
        {
            memcpy_(rand_p, (uint8_t *)&g_rand_buffer[g_buffer_used_words], remainder_words<<2);
            words_t -= remainder_words;
            rand_p = &rand_p[remainder_words<<2];

            ret = get_latest_gm_rand_buffer_regen_case_3_greater();
        }
    }
    else
    {}

    if((0u == is_over) && (TRNG_SUCCESS == ret))
    {
        while(words_t >= g_buffer_total_words)
        {
            memcpy_(rand_p, (uint8_t *)g_rand_buffer, g_buffer_total_words<<2);
            words_t -= g_buffer_total_words;
            rand_p = &rand_p[g_buffer_total_words<<2];
            ret = get_latest_gm_rand_buffer_full();
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }

        if((TRNG_SUCCESS == ret) && (words_t != 0u))
        {
            memcpy_(rand_p, (uint8_t *)g_rand_buffer, words_t<<2);

            (void)get_latest_gm_rand_buffer_regen_case_4_remainder(words_t & (~0x07u));

            g_buffer_used_words = words_t;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: get random numbers for internal calling(latest gm standard)
 * parameters: 
 *     random ----------------------- output, random number in byte style
 *     bytes ---------------------- input, random number bytes
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t get_latest_gm_rand_internal(uint8_t *random, uint32_t bytes)
{
    uint32_t ret, tmp[1];
    uint32_t words, remainder_bytes;

    words = bytes/4u;
    if(words != 0u)
    {
        ret = get_latest_gm_rand_by_word_internal(random, words);
    }
    else
    {
        ret = TRNG_SUCCESS;
    }

    if(TRNG_SUCCESS == ret)
    {
        remainder_bytes = bytes & 3u;
        if(remainder_bytes != 0u)
        {
            ret = get_latest_gm_rand_by_word_internal((uint8_t *)tmp, 1u);
            if(TRNG_SUCCESS == ret)
            {
                memcpy_(&random[words<<2], (uint8_t *)tmp, remainder_bytes);
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: get random numbers(latest gm standard)
 * parameters: 
 *     random ----------------------- output, random number in byte style
 *     bytes ---------------------- input, random number bytes
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
uint32_t get_latest_gm_rand(uint8_t *random, uint32_t bytes)
{
    uint32_t ret = TRNG_SUCCESS;
    MEM_VOLATILE uint32_t ro_entropy_mask = (0x0000000FU);
    volatile uint32_t errorCnt = TRNG_ERROR_COUNTER_THRESHOLD;

    //check input parameters
    if(NULL == random)
    {
        ret = TRNG_BUFFER_NULL;
    }
    else
    {}

    if((TRNG_SUCCESS == ret) && (0U != bytes))
    {
        lib_register_unlock(g_trng_reg);

        //make sure trng and ro are enabled
        if(0U == (rRO_CLK_EN & ro_entropy_mask))
        {
            ret = TRNG_INVALID_CONFIG;
        }
        else
        {}

        lib_register_lock(g_trng_reg);

        while(0U != errorCnt)
        {
            ret = get_latest_gm_rand_internal(random, bytes);
            if (TRNG_SUCCESS == ret)
            {
                break;
            }
            else
            {}

            errorCnt -= 1U;
        }
    }
    else
    {}

    return ret;
}


/* function: get some rand words(without reseed)
 * parameters:
 *     a -------------------------- output, random words
 *     words ---------------------- input, word number of output, must be in [1, 8]
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the two parameters are valid
 */
uint32_t get_rand_uint32_without_reseed(uint32_t *a, uint32_t words)
{
    MEM_VOLATILE uint32_t DT_ready_flag = 2U;
    MEM_VOLATILE uint32_t HT_error_flag = 1U;
    MEM_VOLATILE uint32_t clear_flag = 7U;
    volatile uint32_t tmp, cnt = 0;
    uint32_t ret = TRNG_SUCCESS;
    uint32_t i;

    while(0U == (rTRNG_SR & DT_ready_flag))
    {
        if(0U != (rTRNG_SR & HT_error_flag))
        {
            trng_disable();
            rTRNG_SR |= clear_flag;  //clear (alarm) status
            trng_enable();

            ret = TRNG_HT_ERROR;
        }
        else
        {
            cnt += 1U;
            if (cnt > TRNG_TIMEOUT_COUNTER_THRESHOLD)
            {
                trng_disable();
                rTRNG_SR |= clear_flag;  //clear (alarm) status
                trng_enable();

                ret = TRNG_TIMEOUT_ERROR;
            }
            else
            {}
        }

        if(TRNG_SUCCESS != ret)
        {
            break;
        }
        else
        {}
    }

    if(TRNG_SUCCESS == ret)
    {
        if(8u == words)
        {
            a[0] = rTRNG_DR;
            a[1] = rTRNG_DR;
            a[2] = rTRNG_DR;
            a[3] = rTRNG_DR;
            a[4] = rTRNG_DR;
            a[5] = rTRNG_DR;
            a[6] = rTRNG_DR;
            a[7] = rTRNG_DR;
        }
        else
        {
            for(i=0; i<words; i++)
            {
                a[i] = rTRNG_DR;
            }

            for(; i<8u; i++)     //read 8 words.
            {
                tmp = rTRNG_DR;
            }
        }

        rTRNG_SR |= DT_ready_flag;  //clear

        //if now HT error
        if(0U != (rTRNG_SR & HT_error_flag))
        {
            trng_disable();
            rTRNG_SR |= clear_flag;  //clear (alarm) status
            trng_enable();

            ret = TRNG_HT_ERROR;
        }
        else
        {}
    }
    else
    {}
    (void)tmp; // disable gcc warning (set but not used)

    return ret;
}


/* function: get some rand words(with post-processing and reseed)
 * parameters:
 *     a -------------------------- output, random words
 *     words ---------------------- input, word number of output, must be in [1, 8]
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the two parameters are valid
 */
uint32_t get_rand_uint32_with_reseed(uint32_t *a, uint32_t words)
{
    uint32_t ret = get_rand_uint32_without_reseed(a, words);

    if(TRNG_SUCCESS == ret)
    {
        trng_reseed();  //for next generation.
    }
    else
    {};

    return ret;
}


/* function: get rand buffer(internal basis interface)
 * parameters:
 *     random ----------------------- input, byte buffer rand, address must be word 
 *     bytes ---------------------- input, byte length of rand
 *     get_rand_words ------------- input, function pointer to get some random words(at most 8 words)
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
FLAG_STATIC uint32_t get_rand_buffer_internal(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words)
{
    uint32_t tmp, tmp_len, remainder_bytes = bytes;
    uint32_t ret = TRNG_SUCCESS, count;
    uint8_t *a = random;
    uint32_t is_finished = 0U;

    tmp = ((uintptr_t)a) & 3U;
    if(0U != tmp) //a address is not aligned word
    {
        tmp_len = 4U-tmp;

        ret = get_rand_words(&tmp, 1);
        if(TRNG_SUCCESS == ret)
        {
            if(remainder_bytes > tmp_len)
            {
                memcpy_(a, (uint8_t *)(&tmp), tmp_len);
                a = &a[tmp_len];//now a address is aligned word
                remainder_bytes -= tmp_len;
            }
            else
            {
                memcpy_(a, (uint8_t *)(&tmp), tmp_len);
                is_finished = 1U;
            }
        }
        else
        {}
    }
    else
    {}

    if((TRNG_SUCCESS == ret) && (0U == is_finished))
    {
        tmp = remainder_bytes>>2;
        while(0U != tmp)
        {
            if(0u != (tmp&(~7U)))    //tmp>=8U
            {
                count = 8U;
            }
            else
            {
                count = tmp;
            }

            ret = get_rand_words((uint32_t *)a, count);//now a address is aligned word
            if(TRNG_SUCCESS == ret)
            {
                a = &(a[count<<2]);
                tmp -= count;
            }
            else
            {
                break;
            }
        }

        remainder_bytes &= 3U;

        if((TRNG_SUCCESS == ret) && (0U != remainder_bytes))
        {
            ret = get_rand_words(&tmp, 1);
            if(TRNG_SUCCESS == ret)
            {
                memcpy_(a, (uint8_t *)(&tmp), remainder_bytes);
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


/* function: get rand buffer(internal basis interface)
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 *     get_rand_words ------------- input, function pointer to get some random words(at most 8 words)
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand_buffer(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words)
{
    MEM_VOLATILE uint32_t enable_flag = 1U;
    MEM_VOLATILE uint32_t ro_entropy_mask = (0x0000000FU);
    uint32_t ret = TRNG_SUCCESS;

    //check input parameters
    if((NULL == random) || (NULL == get_rand_words))
    {
        ret = TRNG_BUFFER_NULL;
    }
    else
    {}

    if(TRNG_SUCCESS == ret)
    {
        lib_register_unlock(g_trng_reg);

        //make sure trng and ro are enabled
        if(0U == (rTRNG_CR & enable_flag))
        {
            ret = TRNG_INVALID_CONFIG;
        }
        else if(0U == (rRO_CLK_EN & ro_entropy_mask))
        {
            ret = TRNG_INVALID_CONFIG;
        }
        else
        {
            //handle other
        }

        lib_register_lock(g_trng_reg);
    }
    else
    {}

    if((TRNG_SUCCESS == ret) && (0U != bytes))
    {
        ret = get_rand_buffer_internal(random, bytes, get_rand_words);
    }
    else
    {}

    if(TRNG_SUCCESS != ret)
    {
        memset_(random, 0, bytes);
    }
    else
    {}

#ifdef TRNG_POKER_TEST
    if(TRNG_SUCCESS == ret)
    {
        poker_test(rand, bytes);
    }
#endif

    return ret;
}


/* function: get rand with post processing
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 *     get_rand_words ------------- input, function pointer to get some random words(at most 8 words)
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand_with_post_processing(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words)
{
    MEM_VOLATILE uint32_t flag = 0;
    volatile uint32_t errorCnt = TRNG_ERROR_COUNTER_THRESHOLD;
    uint32_t ret = TRNG_ERROR;

    lib_register_unlock(g_trng_reg);

    //with post-processing
    if(flag == rTRNG_MSEL)
    {
        trng_disable();
        trng_set_mode(1);
        trng_enable();
    }
    else
    {}

    lib_register_lock(g_trng_reg);

    while(0U != errorCnt)
    {
	    errorCnt -= 1U;
        ret = get_rand_buffer(random, bytes, get_rand_words);
        if ((TRNG_HT_ERROR == ret)||(TRNG_TIMEOUT_ERROR == ret))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return ret;
}

/* function: get rand without post processing
 * parameters:
 *     random ----------------------- input, byte buffer rand
 *     bytes ---------------------- input, byte length of rand
 *     get_rand_words ------------- input, function pointer to get some random words(at most 8 words)
 * return: TRNG_SUCCESS(success), other(error)
 * caution:
 */
uint32_t get_rand_without_post_processing(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words)
{
    MEM_VOLATILE uint32_t flag = 0;
    volatile uint32_t errorCnt = TRNG_ERROR_COUNTER_THRESHOLD;
    uint32_t ret = TRNG_ERROR;

    lib_register_unlock(g_trng_reg);

    //without post-processing
    if(flag != rTRNG_MSEL)
    {
        trng_disable();
        trng_set_mode(0);
        trng_enable();
    }
    else
    {}

    lib_register_lock(g_trng_reg);

    while(0U != errorCnt)
    {
        errorCnt -= 1U;
        ret = get_rand_buffer(random, bytes, get_rand_words);
        if ((TRNG_HT_ERROR == ret)||(TRNG_TIMEOUT_ERROR == ret))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return ret;
}

#endif
