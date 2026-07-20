#include "ske_basic.h"
#include "../crypto_include/crypto_common/utility.h"
#include "../crypto_include/trng/trng.h"




#if 1
//HASH register struct 
typedef struct {
    uint32_t ctrl[1];                      /* Offset: 0x000 (W1S) SKE Control Register */
    uint32_t cfg[1];                       /* Offset: 0x004 (R/W) SKE Config Register */
    uint32_t sr[1];                        /* Offset: 0x008 (R)   SKE Status Register */
    uint32_t risr[1];                      /* Offset: 0x00C (W0C) SKE Interrupt Source Register */
    uint32_t imcr[1];                      /* Offset: 0x010 (R/W) SKE Interrupt Enable Register */
    uint32_t misr[1];                      /* Offset: 0x014 (R)   SKE Interrupt Output Register */
    uint32_t rev1[1];
    uint32_t sp[1];                        /* Offset: 0x01C (R/W) SKE Secure Port Register */
    uint32_t key1[8];                      /* Offset: 0x020 (R/W) Key1, occupy 8 words */
    uint32_t key2[8];                      /* Offset: 0x040 (R/W) Key2, occupy 8 words */
    uint32_t ske_a_len_l[1];               /* Offset: 0x060 (R/W) CCM/GCM mode AAD length low Register */
    uint32_t ske_a_len_h[1];               /* Offset: 0x064 (R/W) CCM/GCM mode AAD length high Register */
    uint32_t ske_c_len_l[1];               /* Offset: 0x068 (R/W) CCM/GCM/XTS mode plaintext/ciphertext length low Register */
    uint32_t ske_c_len_h[1];               /* Offset: 0x06C (R/W) CCM/GCM/XTS mode plaintext/ciphertext length high Register */
    uint32_t iv[4];                        /* Offset: 0x070 (R/W) Initial Vector */
    uint32_t m_din_cr[1];                  /* Offset: 0x080 (R/W) SKE Input Register */
    uint32_t rev3[3];
    uint32_t m_din[4];                     /* Offset: 0x090 (R/W) SKE Input Register, occupy 4 words */
    uint32_t rev4[4];
    uint32_t m_dout[4];                    /* Offset: 0x0B0 (R)   SKE Output Register, occupy 4 words */
    uint32_t rev5[15];
    uint32_t ske_version[1];               /* Offset: 0x0FC (R)   SKE Version Register */
    uint32_t ske_seed[36];                 /* Offset: 0x100 (R/W) SKE Seed Register, occupy 36 words */
    uint32_t ske_alarm[1];                 /* Offset: 0x190 (R)   SKE Alarm Register */
    uint32_t rev6[3];
    uint32_t mid_iv[4];                    /* Offset: 0x1A0 (R/W) SKE Middle Vector, occupy 4 words */
    uint32_t mid_mac[4];                   /* Offset: 0x1B0 (R/W) SKE Middle MAC, occupy 4 words */
    uint32_t total_a_len_l[1];             /* Offset: 0x1C0 (R/W) SKE GCM mode AAD total length low part */
    uint32_t total_a_len_h[1];             /* Offset: 0x1C4 (R/W) SKE GCM mode AAD total length high part */
    uint32_t total_c_len_l[1];             /* Offset: 0x1C8 (R/W) SKE GCM mode payload total length low part */
    uint32_t total_c_len_h[1];             /* Offset: 0x1CC (R/W) SKE GCM mode payload total length high part */
    uint32_t rev7[76];
    uint32_t dma_cr[1];                    /* Offset: 0x300 (R/W) DMA Config register */
    uint32_t dma_sr[1];                    /* Offset: 0x304 (W0C) DMA Status register */
    uint32_t dma_to[1];                    /* Offset: 0x308 (R/W) DMA Timeout Threshold register */
    uint32_t rev8[1];
    uint32_t dma_sa_l[1];                  /* Offset: 0x310 (R/W) DMA Source Address Low part register */
    uint32_t dma_sa_h[1];                  /* Offset: 0x314 (R/W) DMA Source Address High part register */
    uint32_t rev9[2];
    uint32_t dma_da_l[1];                  /* Offset: 0x320 (R/W) DMA Destination Address Low part register */
    uint32_t dma_da_h[1];                  /* Offset: 0x324 (R/W) DMA Destination Address High part register */
    uint32_t rev10[2];
    uint32_t dma_rlen[1];                  /* Offset: 0x330 (R/W) DMA read Length register */
    uint32_t dma_wlen[1];                  /* Offset: 0x334 (R/W) DMA write Length register */
    uint32_t rev11[2];
    uint32_t dma_awcc[1];                  /* Offset: 0x340 (R/W) DMA AWCC register */
    uint32_t dma_arcc[1];                  /* Offset: 0x344 (R/W) DMA ARCC register */
    uint32_t dma_llp_l[1];                 /* Offset: 0x348 (R/W) DMA List Address Low part register */
    uint32_t dma_llp_h[1];                 /* Offset: 0x34C (R/W) DMA List Address High part register */
    uint32_t dma_ost[1];                   /* Offset: 0x34C (R/W) DMA outstanding register */
    uint32_t rev_final;
} ske_reg_st;


//ske register pointer
#ifdef CONFIG_UNIT_TEST
volatile static ske_reg_st * g_ske_reg = (ske_reg_st *)(SKE_BASE_ADDR);
#else
volatile static ske_reg_st * const g_ske_reg = (ske_reg_st *)(SKE_BASE_ADDR);
#endif


#define rSKE_CTRL           (*((volatile uint32_t *)(g_ske_reg->ctrl)))             /* Offset: 0x000 (W1S) SKE Control Register */
#define rSKE_CFG            (*((volatile uint32_t *)(g_ske_reg->cfg)))              /* Offset: 0x004 (R/W) SKE Config Register */
#define rSKE_SR             (*((volatile uint32_t *)(g_ske_reg->sr)))               /* Offset: 0x008 (R)   SKE Status Register */
#define rSKE_RISR           (*((volatile uint32_t *)(g_ske_reg->risr)))             /* Offset: 0x00C (W0C) SKE Interrupt Source Register */
//#define rSKE_IMCR           (*((volatile uint32_t *)(g_ske_reg->imcr)))              Offset: 0x010 (R/W) SKE Interrupt Enable Register */
//#define rSKE_MISR           (*((volatile uint32_t *)(g_ske_reg->misr)))              Offset: 0x014 (R)   SKE Interrupt Output Register */
#define rSKE_SP             (*((volatile uint32_t *)(g_ske_reg->sp)))               /* Offset: 0x01C (R/W) SKE Secure Port Register */
#define rSKE_K1_BASE        (*((volatile uint32_t *)(g_ske_reg->key1)))             /* Offset: 0x020 (R/W) Key1, occupy 8 words */
#define rSKE_K2_BASE        (*((volatile uint32_t *)(g_ske_reg->key2)))             /* Offset: 0x040 (R/W) Key2, occupy 8 words */
#define rSKE_A_LEN_L        (*((volatile uint32_t *)(g_ske_reg->ske_a_len_l)))      /* Offset: 0x060 (R/W) CCM/GCM mode AAD length low Register */
#define rSKE_A_LEN_H        (*((volatile uint32_t *)(g_ske_reg->ske_a_len_h)))      /* Offset: 0x064 (R/W) CCM/GCM mode AAD length high Register */
#define rSKE_C_LEN_L        (*((volatile uint32_t *)(g_ske_reg->ske_c_len_l)))      /* Offset: 0x068 (R/W) CCM/GCM/XTS mode plaintext/ciphertext length low Register */
#define rSKE_C_LEN_H        (*((volatile uint32_t *)(g_ske_reg->ske_c_len_h)))      /* Offset: 0x06C (R/W) CCM/GCM/XTS mode plaintext/ciphertext length high Register */
#define rSKE_IV_BASE        (*((volatile uint32_t *)(g_ske_reg->iv)))               /* Offset: 0x070 (R/W) Initial Vector, occupy 4 words */
#define rSKE_M_DIN_CR       (*((volatile uint32_t *)(g_ske_reg->m_din_cr)))         /* Offset: 0x080 (R/W) SKE Input Register */
#define rSKE_M_DIN_BASE     (*((volatile uint32_t *)(g_ske_reg->m_din)))            /* Offset: 0x090 (R/W) SKE Input Register, occupy 4 words */
#define rSKE_M_DOUT_BASE    (*((volatile uint32_t *)(g_ske_reg->m_dout)))           /* Offset: 0x0B0 (R)   SKE Output Register, occupy 4 words */
#define rSKE_VERSION        (*((volatile uint32_t *)(g_ske_reg->ske_version)))      /* Offset: 0x0FC (R)   SKE Version Register */
#define rSKE_SEED_BASE      (*((volatile uint32_t *)(g_ske_reg->ske_seed)))         /* Offset: 0x100 (R/W) SKE Seed Register, occupy 36 words */
#define rSKE_ALARM          (*((volatile uint32_t *)(g_ske_reg->ske_alarm)))        /* Offset: 0x190 (R)   SKE Alarm Register */
#define rSKE_MID_IV_BASE    (*((volatile uint32_t *)(g_ske_reg->mid_iv)))           /* Offset: 0x1A0 (R/W) SKE Middle Vector, occupy 4 words */
#define rSKE_MID_MAC_BASE   (*((volatile uint32_t *)(g_ske_reg->mid_mac)))          /* Offset: 0x1B0 (R/W) SKE Middle MAC, occupy 4 words */
#define rSKE_TOTAL_A_LEN_L  (*((volatile uint32_t *)(g_ske_reg->total_a_len_l)))    /* Offset: 0x1C0 (R/W) SKE GCM mode AAD total length low part */
#define rSKE_TOTAL_A_LEN_H  (*((volatile uint32_t *)(g_ske_reg->total_a_len_h)))    /* Offset: 0x1C4 (R/W) SKE GCM mode AAD total length high part */
#define rSKE_TOTAL_C_LEN_L  (*((volatile uint32_t *)(g_ske_reg->total_c_len_l)))    /* Offset: 0x1C8 (R/W) SKE GCM mode payload total length low part */
#define rSKE_TOTAL_C_LEN_H  (*((volatile uint32_t *)(g_ske_reg->total_c_len_h)))    /* Offset: 0x1CC (R/W) SKE GCM mode payload total length high part */
//#define rSKE_DMA_CR         (*((volatile uint32_t *)(g_ske_reg->dma_cr)))            Offset: 0x300 (R/W) DMA Config register */
//#define rSKE_DMA_SR         (*((volatile uint32_t *)(g_ske_reg->dma_sr)))            Offset: 0x304 (W0C) DMA Status register */
// #define rSKE_DMA_TO         (*((volatile uint32_t *)(g_ske_reg->dma_to)))           Offset: 0x308 (R/W) DMA Timeout Threshold register */
#define rSKE_DMA_SA_L       (*((volatile uint32_t *)(g_ske_reg->dma_sa_l)))         /* Offset: 0x310 (R/W) DMA Source Address Low part register */
#define rSKE_DMA_SA_H       (*((volatile uint32_t *)(g_ske_reg->dma_sa_h)))         /* Offset: 0x314 (R/W) DMA Source Address High part register */
#define rSKE_DMA_DA_L       (*((volatile uint32_t *)(g_ske_reg->dma_da_l)))         /* Offset: 0x320 (R/W) DMA Destination Address Low part register */
#define rSKE_DMA_DA_H       (*((volatile uint32_t *)(g_ske_reg->dma_da_h)))         /* Offset: 0x324 (R/W) DMA Destination Address High part register */
#define rSKE_DMA_R_LEN      (*((volatile uint32_t *)(g_ske_reg->dma_rlen)))         /* Offset: 0x330 (R/W) DMA read Length register */
#define rSKE_DMA_W_LEN      (*((volatile uint32_t *)(g_ske_reg->dma_wlen)))         /* Offset: 0x334 (R/W) DMA write Length register */
//#define rSKE_DMA_AWCC       (*((volatile uint32_t *)(g_ske_reg->dma_awcc)))          Offset: 0x340 (R/W) DMA AWCC register */
//#define rSKE_DMA_ARCC       (*((volatile uint32_t *)(g_ske_reg->dma_arcc)))          Offset: 0x344 (R/W) DMA ARCC register */
//#define rSKE_DMA_LLP_L      (*((volatile uint32_t *)(g_ske_reg->dma_llp_l)))         Offset: 0x348 (R/W) DMA List Address Low part register */
//#define rSKE_DMA_LLP_H      (*((volatile uint32_t *)(g_ske_reg->dma_llp_h)))         Offset: 0x34C (R/W) DMA List Address High part register */
#endif


/* function: get ske IP version
 * parameters: none
 * return: ske IP version
 * caution:
 */
uint32_t ske_get_version(void)
{
    return rSKE_VERSION;
}


/* function: get ske driver version
 * parameters: none
 * return: ske driver version(software version)
 * caution:
 */
uint32_t ske_get_driver_version(void)
{
    //the meaning of the version(for example, if the return value is 0x23080301)
    //the first 3 bytes:  23.08.03 ---- date
    //the last byte:      01       ---- first verion on the day
    uint32_t year = 24U;
    uint32_t month = 4U;
    uint32_t day = 18U;
    uint32_t verison = 1U;

    return (year<<24U) | (month<<16U) | (day<<8U) | verison;
}


#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
/* function: reset ske
 * parameters: none
 * return: none
 * caution:
 *     1.currently use simple encrypt to reset
 */
void ske_clear(void)
{
    ske_clear_cfg();
    ske_set_cpu_mode();
    ske_set_endian_uint32();
    ske_set_c_len_uint32(16);

#if defined(SUPPORT_SKE_AES_256)
    ske_set_alg(SKE_ALG_AES_256);
#elif defined(SUPPORT_SKE_TDES_192)
    ske_set_alg(SKE_ALG_TDES_192);
#elif defined(SUPPORT_SKE_TDES_EEE_192)
    ske_set_alg(SKE_ALG_TDES_EEE_192);
#elif defined(SUPPORT_SKE_TDES_EEE_128)
    ske_set_alg(SKE_ALG_TDES_EEE_128);
#elif defined(SUPPORT_SKE_TDES_128)
    ske_set_alg(SKE_ALG_TDES_128);
#elif defined(SUPPORT_SKE_AES_192)
    ske_set_alg(SKE_ALG_AES_192);
#elif defined(SUPPORT_SKE_AES_128)
    ske_set_alg(SKE_ALG_AES_128);
#elif defined(SUPPORT_SKE_SM4)
    ske_set_alg(SKE_ALG_SM4);
#elif defined(SUPPORT_SKE_DES)
    ske_set_alg(SKE_ALG_DES);
#endif

#if defined(SUPPORT_SKE_MODE_XTS)
    ske_set_mode(SKE_MODE_XTS);
#else
    ske_set_mode(SKE_MODE_CBC);
#endif
    ske_set_crypto(SKE_CRYPTO_ENCRYPT);
    ske_set_last_block(0);
    
    //set iv
    lib_register_unlock(g_ske_reg);
    memset_((uint8_t *)(&rSKE_IV_BASE), 0, 16u);
    lib_register_lock(g_ske_reg);

    //set key
    ske_disable_secure_port();
    lib_register_unlock(g_ske_reg);
    memset_((uint8_t *)(&rSKE_K1_BASE), 0, 8u<<2);
    memset_((uint8_t *)(&rSKE_K2_BASE), 0, 8u<<2);
    lib_register_lock(g_ske_reg);

    (void)ske_expand_key();

    //set one block
    memset_((uint8_t *)(&rSKE_M_DIN_BASE), 0, 16u);
    
    //update
    rSKE_RISR = 0U;
    rSKE_CTRL = 1U;

    (void)ske_wait_till_done(WAIT_TILL_OUTPUT_READY);

    rSKE_CTRL = 2U;
    
    ske_clear_cfg();
}
#endif


/* function: ske secure port enable
 * parameters: none
 * return: none
 * caution:
 */
void ske_set_secure_port(void)
{
    MEM_VOLATILE uint32_t flag = 1U;

    lib_register_unlock(g_ske_reg);
    rSKE_SP |= flag;
    lib_register_lock(g_ske_reg);
}


/* function: clear ske CFG register
 * parameters: none
 * return: none
 * caution:
 */
void ske_clear_cfg(void)
{
    MEM_VOLATILE uint32_t flag = 0U;

    lib_register_unlock(g_ske_reg);
    rSKE_CFG = flag;
    lib_register_lock(g_ske_reg);
}


/* function: set ske to be CPU mode
 * parameters: none
 * return: none
 * caution:
 */
void ske_set_cpu_mode(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<SKE_DMA_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;
    lib_register_lock(g_ske_reg);
}


/* function: set ske to be DMA mode
 * parameters: none
 * return: none
 * caution:
 */
void ske_set_dma_mode(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<SKE_DMA_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG |= flag;
    lib_register_lock(g_ske_reg);
}


#if 0
/* function: enable ske DMA linked list function
 * parameters: none
 * return: none
 * caution:
 *     1. this works when DMA mode is enabled
 */
void ske_enable_dma_linked_list(void)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<SKE_DMA_LL_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG |= flag;
    lib_register_lock(g_ske_reg);
}
#endif


/* function: disable ske DMA linked list function
 * parameters: none
 * return: none
 * caution:
 */
void ske_disable_dma_linked_list(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<SKE_DMA_LL_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;
    lib_register_lock(g_ske_reg);
}


/* function: set the ske endian
 * parameters: none
 * return: none
 * caution:
 *     1. actually, this config works for only CPU mode now
 */
void ske_set_endian_uint32(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)3)<<SKE_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);
#ifdef SKE_REVERSE_BYTE_ORDER_IN_WORD
    MEM_VOLATILE uint32_t flag = (((uint32_t)2)<<SKE_REVERSE_BYTE_ORDER_IN_WORD_OFFSET);
#endif

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;    //clear bit[25:24], and now requires CPU is big-endian
#ifdef SKE_REVERSE_BYTE_ORDER_IN_WORD
    rSKE_CFG |= flag;    //requires CPU is little-endian, input and output reversed by hardware----ske IP
#endif
    lib_register_lock(g_ske_reg);
}


/* function: set ske encrypting or decrypting
 * parameters:
 *     crypto --------------------- input, SKE_CRYPTO_ENCRYPT or SKE_CRYPTO_DECRYPT
 * return: none
 * caution:
 *     1. please make sure crypto is valid
 */
void ske_set_crypto(ske_crypto_e crypto)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1) << SKE_CRYPTO_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;
    rSKE_CFG |= (((uint32_t)crypto) << SKE_CRYPTO_OFFSET);
    lib_register_lock(g_ske_reg);
}


/* function: set ske alg
 * parameters:
 *     ske_alg -------------------- input, ske algorithm
 * return: none
 * caution:
 *     1. please make sure ske_alg is valid
 */
void ske_set_alg(ske_alg_e alg)
{
    MEM_VOLATILE uint32_t mask = ~(0x000000FFU);
    uint32_t cfg;

    switch(alg)
    {
#ifdef SUPPORT_SKE_DES
    case SKE_ALG_DES:
        cfg = 3;
        break;
#endif

#ifdef SUPPORT_SKE_TDES_128
    case SKE_ALG_TDES_128:
#endif
#ifdef SUPPORT_SKE_TDES_192
    case SKE_ALG_TDES_192:
#endif
#if (defined(SUPPORT_SKE_TDES_128) || defined(SUPPORT_SKE_TDES_192))
        cfg = 4;
        break;
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    case SKE_ALG_TDES_EEE_128:
#endif
#ifdef SUPPORT_SKE_TDES_EEE_192
    case SKE_ALG_TDES_EEE_192:
#endif
#if (defined(SUPPORT_SKE_TDES_EEE_128) || defined(SUPPORT_SKE_TDES_EEE_192))
        cfg = 5;
        break;
#endif

#ifdef SUPPORT_SKE_AES_128
    case SKE_ALG_AES_128:
        cfg = ((1U<<6U)|(1U<<4U)|(1U));
        break;
#endif

#ifdef SUPPORT_SKE_AES_192
    case SKE_ALG_AES_192:
        cfg = (2U<<6U)|(2U<<4U)|(1U);
        break;
#endif

#ifdef SUPPORT_SKE_AES_256
    case SKE_ALG_AES_256:
        cfg = (3U<<6U)|(3U<<4U)|(1U);
        break;
#endif

#ifdef SUPPORT_SKE_SM4
    case SKE_ALG_SM4:
        cfg = 2;
        break;
#endif

    default:
        cfg = 2;  //default alg SM4
        break;
    }

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;                 // clear bit[7:0]
    rSKE_CFG |= cfg;                  // set ske alg cfg
    lib_register_lock(g_ske_reg);
}


/* function: set ske alg operation mode
 * parameters:
 *     mode ----------------------- input, operation mode
 * return: none
 * caution:
 *     1. please make sure mode is valid
 */
void ske_set_mode(ske_mode_e mode)
{
    MEM_VOLATILE uint32_t mask = ~((uint32_t)0x0000000F << SKE_MODE_OFFSET);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;                                             //clear bit [31:28]
    rSKE_CFG |= (((uint32_t)mode) << SKE_MODE_OFFSET);         //set mode
    lib_register_lock(g_ske_reg);
}


/* function: set whether ske current input data is the last data or not
 * parameters:
 *     is_last_block -------------- input, 0:no, other:yes
 * return: none
 * caution:
 *     1. just for CMAC/CCM/GCM/XTS mode
 */
void ske_set_last_block(uint32_t is_last_block)
{
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<SKE_LAST_DATA_OFFSET);
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<SKE_LAST_DATA_OFFSET);

#if 0
    if(is_last_block)
    {
        flag |= rSKE_M_DIN_CR;
        rSKE_M_DIN_CR = flag;
    }
    else
    {
        mask &= rSKE_M_DIN_CR;
        rSKE_M_DIN_CR = mask;
    }
#else
    if(0U != is_last_block)
    {
        rSKE_M_DIN_CR |= flag;
    }
    else
    {
        rSKE_M_DIN_CR &= mask;
    }
#endif
}


/* function: set ske current input data bit length
 * parameters:
 *     bytes ---------------------- input, byte length of current input data
 * return: none
 * caution:
 *     1. just for CMAC
 */
void ske_set_last_block_len(uint32_t bytes)
{
    MEM_VOLATILE uint32_t mask = ~0x000000FFU;
#if 1
    mask &= rSKE_M_DIN_CR;
    mask |= (bytes<<3);
    rSKE_M_DIN_CR = mask;
#else                        //the following style makes hardware not stable
    rSKE_M_DIN_CR &= mask;
    rSKE_M_DIN_CR |= (bytes<<3);
#endif
}


/* function: set ske seed
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_set_seed(void)
{
    uint32_t ret;

#if 1
    uint32_t tmp[SKE_SEED_WORDS];
    uint32_t i;

    ret = get_rand((uint8_t *)tmp, ((uint32_t)SKE_SEED_WORDS)<<2);
    if(TRNG_SUCCESS == ret)
    {
        for(i=0;i<SKE_SEED_WORDS;i++)
        {
            *(volatile uint32_t *)(&((&rSKE_SEED_BASE)[i])) = tmp[i];
        }

        ret = SKE_SUCCESS;
    }    
#else
    ret = get_rand((uint8_t *)&(rSKE_SEED_BASE), ((uint32_t)SKE_SEED_WORDS)<<2);
    if(TRNG_SUCCESS == ret)
    {
        ret = SKE_SUCCESS;
    }
    else
    {}
#endif

    return ret;
}


/* function: ske start to expand key or calc
 * parameters:none
 * return: none
 * caution:
 */
void ske_start(void)
{
    MEM_VOLATILE uint32_t clear_flag = 0;
    MEM_VOLATILE uint32_t start_flag = 1;

    rSKE_RISR = clear_flag;  //clear status

    rSKE_CTRL |= start_flag;
}


/* function: wait till ske calculating is done
 * parameters:
 *     mode ---------------------- input, SKE wait mode
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_wait_till_done(ske_wait_mode_e wait_mode)
{
    MEM_VOLATILE uint32_t finish_flag = 0;
    MEM_VOLATILE uint32_t alarm_flag = 1;
    volatile const uint32_t *reg_status = (volatile uint32_t *)&rSKE_SR;
    uint32_t ret = SKE_SUCCESS;

    switch(wait_mode)
    {
        case WAIT_TILL_EXPAND_KEY_DONE:
            finish_flag = 1;
            break;
        case WAIT_TILL_COULD_INPUT:
            finish_flag = ((uint32_t)1<<16U);
            break;
        case WAIT_TILL_OUTPUT_READY:
            finish_flag = ((uint32_t)1<<17U);
            break;
        case WAIT_TILL_CALC_DONE:
            finish_flag = 7;
            reg_status = (volatile uint32_t *)&rSKE_RISR;
            break;

        default:
            ret = SKE_INPUT_INVALID;
            break;
    }

    if(SKE_SUCCESS == ret)
    {
        while(0U == ((*reg_status) & finish_flag))
        {
            if(0U != (rSKE_ALARM & alarm_flag))
            {
                ret = SKE_ATTACK_ALARM;
                break;
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: set ske key
 * parameters:
 *     key ------------------------ input, key in word buffer
 *     idx ------------------------ input, key index, only 1 and 2 are valid
 *     key_words ------------------ input, word length of key
 * return: none
 * caution:
 *     1. if idx is 1, set key1 register, else if idx is 2, set key2 register, please
 *        make sure idx is valid
 */
void ske_set_key_uint32(const uint32_t *key, uint32_t idx, uint32_t key_words)
{
    uint32_t words = key_words;
    volatile uint32_t *key_reg;
#if 0
    int32_t i;
#endif
    
    lib_register_unlock(g_ske_reg);

    if(1U == idx)
    {
        key_reg = (volatile uint32_t *)&(rSKE_K1_BASE);
    }
    else
    {
        key_reg = (volatile uint32_t *)&(rSKE_K2_BASE);
    }

#if 0
    for (i=words; i > 0; i--)
    {
        key_reg[i-1] = key[words-i];
    }
#else
    while (0U != words)
    {
        words--;
        key_reg[words] = key[words];
    }
#endif

    lib_register_lock(g_ske_reg);
}


/* function: set ske iv
 * parameters:
 *     iv ------------------------- input, iv in word buffer
 *     block_words ---------------- input, word length of ske block
 * return: none
 * caution:
 *     1. please make sure the three parameters are valid
 */
void ske_set_iv_uint32(const uint32_t *iv, uint32_t block_words)
{
    uint32_t words = block_words;

    lib_register_unlock(g_ske_reg);

#if 0
    int32_t i;

    for (i=words; i > 0; i--)
    {
        rSKE_IV(i-1) = iv[words-i];
    }
#else
    while (0U != words)
    {
        words--;
        *((volatile uint32_t *)(&(&rSKE_IV_BASE)[words])) = iv[words];
    }
#endif

    lib_register_lock(g_ske_reg);
}


#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
/* function: set aad bits(just for ske ccm/gcm mode)
 * parameters:
 *     aad_bytes ------------------ input, byte length of aad
 * return: none
 * caution:
 *     1. this function is just for CCM/GCM mode
 */
void ske_set_aad_len_uint32(uint32_t aad_bytes)
{
    lib_register_unlock(g_ske_reg);
    rSKE_A_LEN_L = ((aad_bytes)<<3)&0xFFFFFFFFU;
    rSKE_A_LEN_H = aad_bytes>>(32-3);
    lib_register_lock(g_ske_reg);
}
#endif


#if (defined(SUPPORT_SKE_MODE_GCM))
/* function: set aad total bits(just for ske gcm mode)
 * parameters:
 *     aad_bytes ------------------ input, byte length of aad
 * return: none
 * caution:
 *     1. this function is just for GCM mode
 */
void ske_set_aad_total_len_uint32(uint32_t aad_bytes)
{
    lib_register_unlock(g_ske_reg);
    rSKE_TOTAL_A_LEN_L = ((aad_bytes)<<3)&0xFFFFFFFFU;
    rSKE_TOTAL_A_LEN_H = aad_bytes>>(32-3);
    lib_register_lock(g_ske_reg);
}
#endif


#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_XTS))
/* function: set plaintext/ciphertext bits(just for ske ccm/gcm/xts mode)
 * parameters:
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext
 * return: none
 * caution:
 *     1. this function is just for CCM/GCM/XTS mode
 */
void ske_set_c_len_uint32(uint32_t c_bytes)
{
    lib_register_unlock(g_ske_reg);
    rSKE_C_LEN_L = ((c_bytes)<<3)&0xFFFFFFFFU;
    rSKE_C_LEN_H = c_bytes>>(32-3);
    lib_register_lock(g_ske_reg);
}
#endif


#if (defined(SUPPORT_SKE_MODE_GCM))
/* function: set plaintext/ciphertext total bits(just for ske gcm mode)
 * parameters:
 *     c_bytes -------------------- input, byte length of plaintext/ciphertext
 * return: none
 * caution:
 *     1. this function is just for GCM mode
 */
void ske_set_c_total_len_uint32(uint32_t c_bytes)
{
    lib_register_unlock(g_ske_reg);
    rSKE_TOTAL_C_LEN_L = ((c_bytes)<<3)&0xFFFFFFFFU;
    rSKE_TOTAL_C_LEN_H = c_bytes>>(32-3);
    lib_register_lock(g_ske_reg);
}
#endif


/* function: set padding mode 
 * parameters:
 *     padding -------------------- input, padding mode
 * return: none
 * caution:
 */
void ske_set_padding(ske_padding_e padding)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)3)<<26);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;
    rSKE_CFG |= ((uint32_t)padding)<<26;
    lib_register_lock(g_ske_reg);
}


/* function: set plaintext/ciphertext calc stage
 * parameters:
 *     stage -------------------- input, crypto stage
 * return: none
 * caution:
 */
void ske_set_payload_stage(ske_payload_stage_e stage)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)3)<<22);

    lib_register_unlock(g_ske_reg);
    rSKE_CFG &= mask;
    rSKE_CFG |= ((uint32_t)stage)<<22;
    lib_register_lock(g_ske_reg);
}


/* function: set mid iv
 * parameters:
 *     iv -------------------- input, iv
 *     words ----------------- input, word len of iv
 * return: none
 * caution:
 */
void ske_set_mid_iv(const uint32_t *iv, uint32_t words)
{
    uint32_t i;

    for(i=0; i<words; i++)
    {
        *((volatile uint32_t *)(&(&rSKE_MID_IV_BASE)[i])) = iv[i];
    }
}


/* function: get mid iv
 * parameters:
 *     iv -------------------- input, iv
 *     words ----------------- input, word len of iv
 * return: none
 * caution:
 */
void ske_get_mid_iv(uint32_t *iv, uint32_t words)
{
    MEM_VOLATILE uint32_t mask = ~((1U)<<1);
    MEM_VOLATILE uint32_t flag = (1U)<<1;

    uint32_t i;

    while(0U == (rSKE_SR & flag))
    {}

    for(i=0; i<words; i++)
    {
        iv[i] = *((volatile uint32_t *)(&(&rSKE_MID_IV_BASE)[i]));
    }

    rSKE_SR &= mask;  //clear
}


/* function: set mid mac
 * parameters:
 *     mac -------------------- input, mid mac
 *     words ----------------- input, word len of mac
 * return: none
 * caution:
 */
void ske_set_mid_mac(const uint32_t *mac, uint32_t words)
{
    uint32_t i;

    for(i=0; i<words; i++)
    {
        *((volatile uint32_t *)(&(&rSKE_MID_MAC_BASE)[i])) = mac[i];
    }
}


/* function: get mid mac
 * parameters:
 *     mac -------------------- input, mid mac
 *     words ----------------- input, word len of mac
 * return: none
 * caution:
 */
void ske_get_mid_mac(uint32_t *mac, uint32_t words, uint32_t clear)
{
    MEM_VOLATILE uint32_t mask = ~((1U)<<1);
    MEM_VOLATILE uint32_t flag = (1U)<<1;

    uint32_t i;

    while(0U == (rSKE_SR & flag))
    {}

    for(i=0; i<words; i++)
    {
        mac[i] = *((volatile uint32_t *)(&(&rSKE_MID_MAC_BASE)[i]));
    }

    if(0U != clear)
    {
        rSKE_SR &= mask;  //clear
    }
}


/* function: input one block
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext in word buffer
 *     block_words ---------------- input, word length of ske block
 * return: none
 * caution:
 *     1. in is a word buffer of only one block.
 */
void ske_simple_set_input_block(const uint32_t *in, uint32_t block_words)
{
    uint32_t words = block_words;
#if 0
#if 1
    int32_t i;

    for (i=words; i > 0; i--)
    {
        rSKE_M_DIN(i-1) = in[words-i];
    }
#else
    if(4 == words)      //for AES/SM4
    {
        rSKE_M_DIN(0) = in[3];
        rSKE_M_DIN(1) = in[2];
        rSKE_M_DIN(2) = in[1];
        rSKE_M_DIN(3) = in[0];
    }
    else                      //for DES
    {
        rSKE_M_DIN(0) = in[1];
        rSKE_M_DIN(1) = in[0];
    }
#endif
#else
    while(0U != words)
    {
        words--;
        *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[words])) = in[words];
    }
#endif
}


/* function: output one block
 * parameters:
 *     out ------------------------ output, one block output of ske in word buffer
 *     block_words ---------------- input, word length of ske block
 * return: none
 * caution:
 */
void ske_simple_get_output_block(uint32_t *out, uint32_t block_words)
{
    uint32_t words = block_words;
    MEM_VOLATILE uint32_t flag = 0x02;
#if 0
    int32_t i;
#endif
    
    rSKE_CTRL |= flag;   //trigger to pop

#if 0
#if 1
    for (i=words; i > 0; i--)
    {
        out[words-i] = rSKE_M_DOUT(i-1);
    }
#else
    if(4 == words)      //for AES/SM4
    {
        out[0] = rSKE_M_DOUT(3);
        out[1] = rSKE_M_DOUT(2);
        out[2] = rSKE_M_DOUT(1);
        out[3] = rSKE_M_DOUT(0);
    }
    else                      //for DES
    {
        out[0] = rSKE_M_DOUT(1);
        out[1] = rSKE_M_DOUT(0);
    }
#endif
#else
    while(0U != words)
    {
        words--;
        out[words] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[words]));
    }
#endif
}


/* function: ske expand key
 * parameters: none
 * return: SKE_SUCCESS(success), other(error)
 * caution:  1. must be called after ske_set_crypto() and ske_set_alg(), and the key is set already.
 */
uint32_t ske_expand_key(void)
{
    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1)<<SKE_UP_CFG_OFFSET);
    MEM_VOLATILE uint32_t flag = (((uint32_t)1)<<SKE_UP_CFG_OFFSET);
    uint32_t ret;

    //update cfg
    lib_register_unlock(g_ske_reg);
    rSKE_CFG |= flag;
    lib_register_lock(g_ske_reg);

    //expand key
    ske_start();
    ret = ske_wait_till_done(WAIT_TILL_EXPAND_KEY_DONE);//;WAIT_TILL_OUTPUT_READY
    if(SKE_SUCCESS == ret)
    {
        //not update cfg
        lib_register_unlock(g_ske_reg);
        rSKE_CFG &= mask;
        lib_register_lock(g_ske_reg);
    }
    else
    {}

    return ret;
}


/************************* DMA *************************/

#ifdef SKE_DMA_FUNCTION
/* function: wait till ske dma calculating is done
 * parameters:
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ske_dma_calc_wait_till_done(SKE_CALLBACK callback)
{
    MEM_VOLATILE uint32_t finish_flag = 7;
    MEM_VOLATILE uint32_t alarm_flag = 1;
    uint32_t ret = SKE_SUCCESS;

    while(0U == (rSKE_RISR & finish_flag))
    {
        if(0U != (rSKE_ALARM & alarm_flag))
        {
            ret = SKE_ATTACK_ALARM;
            break;
        }
        else if(NULL != callback)
        {
            callback();
        }
        else
        {
            //handle other
        }
    }

    return ret;
}


/* function: basic ske DMA operation
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     in_bytes ------------------- input, byte length of in, must be a multiple of block byte length
 *     out_bytes ------------------ input, byte length of out, must be a multiple of block byte length
 *     callback ------------------- callback function pointer, this could be NULL, means doing nothing
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. in_words & out_words must be multiples of block words.
 *     2. it could be without output, namely, out can be NULL, out_bytes can be 0(for input AAD, or CBC_MAC/CMAC mode)
 */
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t ske_dma_operate(uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t in_bytes, uint32_t out_bytes, SKE_CALLBACK callback)
#else
uint32_t ske_dma_operate(const uint32_t *in, const uint32_t *out, uint32_t in_bytes, uint32_t out_bytes,
        SKE_CALLBACK callback)
#endif
{
#if (defined(CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS) && defined(CONFIG_SKE_SUPPORT_DMA_CLEAR))
    uint8_t *out_remap;
#endif
#ifndef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
    MEM_VOLATILE uint32_t flag_0 = 0;
#endif
    uint32_t ret;

    lib_register_unlock(g_ske_reg);
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS 
    rSKE_DMA_SA_L = (uint32_t)in_l;
    rSKE_DMA_DA_L = (uint32_t)out_l;
    rSKE_DMA_SA_H = (uint32_t)in_h;
    rSKE_DMA_DA_H = (uint32_t)out_h;
#else
    //src & dst addr low 32bits
    rSKE_DMA_SA_L = (uint32_t)in;
    rSKE_DMA_DA_L = (uint32_t)out;

    //src & dst addr high 32bits
    if(4U == (sizeof(uint32_t *)))
    {
        //in this case, if using (((uint64_t)in)>>32), you may get 0xFFFFFFFF, not 0 you expected!
        rSKE_DMA_SA_H = flag_0;
        rSKE_DMA_DA_H = flag_0;
    }
    else
    {
        rSKE_DMA_SA_H = (uint32_t)(((uint64_t)in)>>32);
        rSKE_DMA_DA_H = (uint32_t)(((uint64_t)out)>>32);
    }
#endif

    //data bit length
    rSKE_DMA_R_LEN = in_bytes<<3;
    rSKE_DMA_W_LEN = out_bytes<<3;

    lib_register_lock(g_ske_reg);

    ske_start();
    ret = ske_dma_calc_wait_till_done(callback);
#ifdef CONFIG_SKE_SUPPORT_DMA_CLEAR
    if(SKE_SUCCESS != ret)
    {
#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
        out_remap = lib_addr_arch32_lock_remap(out_h, out_l, 0U);
#else
        out_remap = lib_addr_arch32_lock_remap(0U, out, 0U);
#endif
        memset_(out_remap, 0U, out_bytes);

        lib_addr_arch32_unlock_remap();
    }
    else
    {}
#endif

    return ret;
}
#endif


/* function: update ske some blocks without output
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, some blocks
 *     bytes ---------------------- input, byte length of in
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the bytes is a multiple of block byte length ctx->block_bytes
 *     2. this function is called by CCM(input aad)/GCM(input aad)/CMAC/CBC-MAC mode
 */
uint32_t ske_update_blocks_no_output(const ske_ctx_st *ctx, const uint8_t *in, uint32_t bytes)
{
    MEM_VOLATILE uint32_t mask = 0xF0000000U;
    MEM_VOLATILE uint32_t flag_2 = 0x02;

    uint32_t in_word_align, is_ccm_gcm_mode;
    uint32_t tmp_buf[4];
    uint32_t i;
    uint32_t ret = SKE_SUCCESS;
    const uint8_t *current_in = in;
    
    if(0U != (((uintptr_t)in) & 3U))
    {
        in_word_align = 0;
    }
    else
    {
        in_word_align = 1;
    }

    lib_register_unlock(g_ske_reg);
    i = (rSKE_CFG & mask)>>28;
    lib_register_lock(g_ske_reg);

    switch(i)
    {
#if (defined(SUPPORT_SKE_MODE_GCM))
    case (uint32_t)SKE_MODE_GCM:
#endif

#if (defined(SUPPORT_SKE_MODE_CCM))
    case (uint32_t)SKE_MODE_CCM:
#endif

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
        is_ccm_gcm_mode = 1;
        break;
#endif

    default:   //CMAC or CBC-MAC mode
        is_ccm_gcm_mode = 0;
        break;
    }

    //input one block ---> calculating ---> output one block
    for (i = 0; i < bytes; i += ctx->block_bytes)
    {
        if(0U != in_word_align)
        {
            ske_simple_set_input_block((const uint32_t *)current_in, ctx->block_words);
        }
        else
        {
            memcpy_((uint8_t *)tmp_buf, current_in, ctx->block_bytes);
            ske_simple_set_input_block((uint32_t *)tmp_buf, ctx->block_words);
        }

        ske_start();

        if(0U != is_ccm_gcm_mode)
        {
            ret = ske_wait_till_done(WAIT_TILL_COULD_INPUT);
        }
        else              //CMAC or CBC-MAC mode
        {
            ret = ske_wait_till_done(WAIT_TILL_CALC_DONE);

            rSKE_CTRL |= flag_2;   //trigger to pop, this can not be deleted
        }

        if(SKE_SUCCESS != ret)
        {
            break;
        }
        else
        {}

        current_in = &(current_in[ctx->block_bytes]);
    }

    return ret;
}


/* function: update ske some blocks and get the same number of blocks
 * parameters:
 *     in ----------------------------- input, some blocks
 *     out ---------------------------- output, the same number of blocks
 *     blocks_num --------------------- input, blocks number of in
 *     alg_block_words ---------------- input, block word length of alg
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure in and out address are word align
 */
uint32_t ske_update_blocks_align(const uint32_t *in, uint32_t *out, uint32_t blocks_num, uint32_t alg_block_words)
{
    MEM_VOLATILE uint32_t flag_0 = 0x00;
    MEM_VOLATILE uint32_t flag_1 = 0x01;
    MEM_VOLATILE uint32_t flag_2 = 0x02;
    const uint32_t *in_ptr_u32 = in;
    uint32_t *out_ptr_u32 = out;
    uint32_t ret = SKE_SUCCESS;
    uint32_t i;

#if 1
    if(4U == alg_block_words) //for AES/SM4
    {
        for (i = 0; i < blocks_num; i++)
        {
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[0])) = ((const uint32_t *)in_ptr_u32)[0];
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[1])) = ((const uint32_t *)in_ptr_u32)[1];
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[2])) = ((const uint32_t *)in_ptr_u32)[2];
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[3])) = ((const uint32_t *)in_ptr_u32)[3];

            rSKE_RISR = flag_0;  //clear
            rSKE_CTRL = flag_1;

            ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
            if(SKE_SUCCESS != ret)
            {
                uint32_clear(out, blocks_num*alg_block_words);
                break;
            }
            else
            {}

            rSKE_CTRL = flag_2;   //trigger to pop
                
            ((uint32_t *)out_ptr_u32)[0] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[0]));
            ((uint32_t *)out_ptr_u32)[1] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[1]));
            ((uint32_t *)out_ptr_u32)[2] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[2]));
            ((uint32_t *)out_ptr_u32)[3] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[3]));

            in_ptr_u32 = &(in_ptr_u32[alg_block_words]);
            out_ptr_u32 = &(out_ptr_u32[alg_block_words]);
        }
    }
    else //for DES/3DES
    {
        for (i = 0; i < blocks_num; i++)
        {
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[0])) = ((const uint32_t *)in_ptr_u32)[0];
            *((volatile uint32_t *)(&(&rSKE_M_DIN_BASE)[1])) = ((const uint32_t *)in_ptr_u32)[1];

            rSKE_RISR = flag_0;  //clear
            rSKE_CTRL = flag_1;

            ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
            if(SKE_SUCCESS != ret)
            {
                uint32_clear(out, blocks_num*alg_block_words);
                break;
            }
            else
            {}

            rSKE_CTRL = flag_2;   //trigger to pop
            ((uint32_t *)out_ptr_u32)[0] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[0]));
            ((uint32_t *)out_ptr_u32)[1] = *((volatile uint32_t *)(&(&rSKE_M_DOUT_BASE)[1]));

            in_ptr_u32 = &(in_ptr_u32[alg_block_words]);
            out_ptr_u32 = &(out_ptr_u32[alg_block_words]);
        }
    }
#else
    for (i = 0; i < blocks_num; i++)
    {
        ske_simple_set_input_block((uint32_t *)in, block_words);

        ske_start();

        ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
        if(SKE_SUCCESS != ret)
        {
            memset_(out, 0, blocks_num*(block_words<<2));
            break;
        }
        else
        {}

        ske_simple_get_output_block((uint32_t *)out_ptr_u32, block_words);

        in_ptr_u32 = &(in_ptr_u32[block_words]);
        out_ptr_u32 = &(out_ptr_u32[block_words]);
    }
#endif

    return ret;
}


/* function: update ske some blocks and get the same number of blocks
 * parameters:
 *     ctx ------------------------ input, ske_ctx_st context pointer
 *     in ------------------------- input, some blocks
 *     out ------------------------ output, the same number of blocks
 *     bytes ---------------------- input, byte length of in
 * return: SKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the bytes is a multiple of block byte length ctx->block_bytes
 */
uint32_t ske_update_blocks_internal(const ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes)
{
    uint32_t in_word_align, out_word_align;
    uint32_t tmp_buf[4];
    uint32_t i, round_num = bytes/ctx->block_bytes;
    uint32_t block_bytes = ctx->block_bytes;
    uint32_t ret = SKE_SUCCESS;

    uint8_t *out_ptr_u8 = NULL;
    const uint8_t *in_ptr_u8 = NULL;
    uint32_t *out_ptr_u32 = NULL;
    const uint32_t *in_ptr_u32 = NULL;
    uint32_t block_words = block_bytes>>2;

    if(0U != (((uintptr_t)in) & 3U))
    {
        in_word_align = 0;
        in_ptr_u8 = in;
    }
    else
    {
        in_word_align = 1;
        in_ptr_u32 = (const uint32_t *)in;
    }

    if(0U != (((uintptr_t)out) & 3U))
    {
        out_word_align = 0;
        out_ptr_u8 = out;
    }
    else
    {
        out_word_align = 1;
        out_ptr_u32 = (uint32_t *)out;
    }

    if((1U == in_word_align) && (1U == out_word_align))
    {
        ret = ske_update_blocks_align(in_ptr_u32, out_ptr_u32, round_num, ctx->block_words);
    }
    else
    {
        //input one block ---> calculating ---> output one block
        for (i = 0; i < round_num; i++)
        {
            if(0U != in_word_align)
            {
                ske_simple_set_input_block(in_ptr_u32, ctx->block_words);
                in_ptr_u32 = &(in_ptr_u32[block_words]);
            }
            else
            {
                memcpy_((uint8_t *)tmp_buf, in_ptr_u8, block_bytes);
                ske_simple_set_input_block(tmp_buf, ctx->block_words);
                in_ptr_u8 = &(in_ptr_u8[block_bytes]);
            }

            ske_start();

            ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
            if(SKE_SUCCESS != ret)
            {
                memset_(out, 0, bytes);
                break;
            }
            else
            {}

            if(0U != out_word_align)
            {
                ske_simple_get_output_block((uint32_t *)out_ptr_u32, ctx->block_words);
                out_ptr_u32 = &(out_ptr_u32[block_words]);
            }
            else
            {
                ske_simple_get_output_block(tmp_buf, ctx->block_words);
                memcpy_(out_ptr_u8, (uint8_t *)tmp_buf, block_bytes);
                out_ptr_u8 = &(out_ptr_u8[block_bytes]);
            }
        }
    }

    return ret;
}


/* function: disable ske secure port
 * parameters: none
 * return: none
 * caution:
 */
void ske_disable_secure_port(void)
{
    MEM_VOLATILE uint32_t mask = ~1U;

    lib_register_unlock(g_ske_reg);
    rSKE_SP &= mask;
    lib_register_lock(g_ske_reg);
}


#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
uint32_t ske_gmac_get_mac_of_msg_empty(uint32_t *mac)
{
    uint32_t ret;

    lib_register_unlock(g_ske_reg);
    ske_set_aad_total_len_uint32(0U);
    ske_set_c_total_len_uint32(0U);
    ske_set_aad_len_uint32(0U);
    ske_set_c_len_uint32(0U);
    lib_register_lock(g_ske_reg);
    ret = ske_expand_key();
    
    if(SKE_SUCCESS == ret)
    {
        ret = ske_wait_till_done(WAIT_TILL_OUTPUT_READY);
    }

    if(SKE_SUCCESS == ret)
    {
        ske_simple_get_output_block(mac, 4);
    }

    return ret;
}

void ske_gmac_set_cfg_of_msg_not_empty(uint32_t aad_bytes)
{
    lib_register_unlock(g_ske_reg);
    ske_set_aad_total_len_uint32(aad_bytes);
    ske_set_c_total_len_uint32(0U);
    ske_set_aad_len_uint32(aad_bytes);
    ske_set_c_len_uint32(0U);
    lib_register_lock(g_ske_reg);
}
#endif
