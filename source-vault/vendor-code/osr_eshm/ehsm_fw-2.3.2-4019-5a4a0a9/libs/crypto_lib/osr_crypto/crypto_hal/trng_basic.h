/* change list
    2023.08.03
    1. add trng_get_driver_version(). 
 */
#ifndef TRNG_BASIC_H
#define TRNG_BASIC_H


#include "../crypto_include/trng_config.h"


#ifdef __cplusplus
extern "C" {
#endif



#if 0
//TRNG register address
#define rTRNG_CR              (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0000U)))
#define rTRNG_MSEL            (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0004U)))
#define rTRNG_SR              (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0008U)))
#define rTRNG_DR              (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x000CU)))
#define rTRNG_RESEED          (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0010U)))
#define rRO_CLK_EN            (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0014U)))
#define rRO_SRC_EN1           (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0018U)))
#define rRO_SRC_EN2           (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x001CU)))
#define rTRNG_HT_CR           (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0020U)))
#define rTRNG_HT_SR           (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0024U)))
#define rTRNG_VERSION         (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0030U)))
#define rDRBG_ALG_HW_SR       (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0034U)))
#define rDRBG_ALG_MODE_SEL    (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0038U)))
#define rTRHT_ITERATION_PTR   (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0040U)))
#define rTRBG_HT_WIN_CFG      (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0080U)))
#define rTAPT_THLD_CFG        (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0090U)))
#define rTAPTNB_THLD_CFG      (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0094U)))
#define rTPOKER_THLD_CFG      (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x0098U)))
#define rTRUNS_THLD_CFG       (*((volatile uint32_t *)(TRNG_BASE_ADDR + 0x009CU)))
#endif


#if 0
//TRNG freq config
#define TRNG_RO_FREQ_4                  (0U)
#define TRNG_RO_FREQ_8                  (1U)
#define TRNG_RO_FREQ_16                 (2U)
#define TRNG_RO_FREQ_32                 (3U)     //default
#endif


//TRNG action offset
#define TRNG_GLOBAL_INT_OFFSET          (24U)
#define TRNG_READ_EMPTY_INT_OFFSET      (17U)
#define TRNG_DATA_INT_OFFSET            (16U)
#define TRNG_FREQ_OFFSET                (16U)
#define TRNG_SELF_TEST_READY_OFFSET     (3U)


//hardware DRBG algorithom ability
#define DRBG_LFSR_HW_EN                       (1u<<8)
#define DRBG_AES_HW_EN                        (1u<<4)
#define DRBG_SM4_HW_EN                        (1u)


//hardware DRBG algorithom and mode config
#define DRBG_SM4_CBC                          (0x10u)     //default
#define DRBG_SM4_CTR                          (0x00u)
#define DRBG_AES_128_CTR                      (0x01u)


//global init config
#define NIST_GLOBAL_CONFIG                    (1u)
#define GM_GLOBAL_CONFIG                      (2u)
#define LATEST_GM_GLOBAL_CONFIG               (3u)


//Health Test config
#define TRRUNSEN                              ((1UL)<<8)
#define TRPOKEREN                             ((1UL)<<7)
#define TRATNBEN                              ((1UL)<<6)
#define TRCTEN                                ((1UL)<<5)
#define TRATEN                                ((1UL)<<4)
#define DRCTEN                                ((1UL)<<1)
#define DRPTEN                                ((1UL)<<0)
#define LATEST_GM_HT_CONF                     (TRRUNSEN|TRPOKEREN|TRCTEN|TRATEN|DRCTEN|DRPTEN)
#define NIST_HT_CONF                          (TRATNBEN|TRCTEN|TRATEN|DRCTEN|DRPTEN)


//thresholds for 1024-bits-window
#define APT_WIN_1024_BITS_THRESHOLD           (0x00000270u)  //624

//thresholds for 2048-bits-window
#define APTNB_WIN_2048_BITS_THRESHOLD         (0x0000004Du)  //77

//thresholds for 20000-bits-window
#define APT_WIN_20000_BITS_THRESHOLD          (0x00002823u)  //10275
#define APTNB_WIN_20000_BITS_THRESHOLD        (0x000001B8u)  //440
#define POKER_WIN_20000_BITS_THRESHOLD        (0x00180D8Du)  //1576333
#define RUNS_WIN_20000_BITS_THRESHOLD         (0x00006BC0u)  //0x6B=107,0xC0=3/4, this means 107*1000000/1048576+0.75 = 102.75




//TRNG return code
#define TRNG_RT_OFFSET                  (0x600U)
#define TRNG_SUCCESS                    (0U)
#define TRNG_BUFFER_NULL                (TRNG_RT_OFFSET + 1U)
#define TRNG_INVALID_INPUT              (TRNG_RT_OFFSET + 2U)
#define TRNG_INVALID_CONFIG             (TRNG_RT_OFFSET + 3U)
#define TRNG_HT_ERROR                   (TRNG_RT_OFFSET + 4U)
#define TRNG_TIMEOUT_ERROR              (TRNG_RT_OFFSET + 5U)
#define TRNG_ERROR                      (TRNG_RT_OFFSET + 10U)


//TRNG alg and mode, drbg_mode
#define TRNG_ALG_SM4                    (0U)
#define TRNG_ALG_AES                    (1U)
#define TRNG_MODE_DRBG                  (0U)
#define TRNG_MODE_NON_DRBG              (1U)
#define TRNG_DRBG_MODE_CTR              (0U)
#define TRNG_DRBG_MODE_CBC              (1U)

typedef uint32_t (*GET_RAND_WORDS)(uint32_t *a, uint32_t words);


//API

uint32_t trng_get_version(void);

uint32_t trng_get_driver_version(void);

void trng_global_int_enable(void);

void trng_global_int_disable(void);

void trng_empty_read_int_enable(void);

void trng_empty_read_int_disable(void);

void trng_data_int_enable(void);

void trng_data_int_disable(void);

void trng_enable(void);

void trng_disable(void);

uint32_t trng_get_drbg_alg(void);

uint32_t trng_get_mode(void);

uint32_t trng_get_drbg_mode(void);

#ifdef TRNG_RO_ENTROPY

uint32_t trng_if_self_test_ready(void);

uint32_t trng_ro_entropy_config(uint8_t cfg);

uint32_t trng_ro_sub_entropy_config(uint8_t sn, uint16_t cfg);

void trng_set_mode(uint8_t with_post_processing);

void trng_reseed(void);

uint32_t trng_set_freq(uint8_t freq);

uint32_t trng_get_hw_drbg_alg_ability(void);

uint32_t trng_drbg_alg_mode_config(uint32_t value);

uint32_t trng_ht_config(uint32_t value);

uint32_t trng_set_poker_aptnb_test_win_size(uint32_t words);

uint32_t trng_set_apt_runs_test_win_size(uint32_t words);

uint32_t trng_set_apt_test_threshold(uint32_t value);

uint32_t trng_set_apt_non_binary_test_threshold(uint32_t value);

uint32_t trng_set_poker_test_threshold(uint32_t value);

uint32_t trng_set_runs_test_threshold(uint32_t value);

uint32_t trng_set_nist_global_init_config(uint32_t drbg_alg_mode);

uint32_t trng_set_latest_gm_global_init_config(void);

uint32_t get_latest_gm_rand(uint8_t *random, uint32_t bytes);

uint32_t get_rand_uint32_without_reseed(uint32_t *a, uint32_t words);

uint32_t get_rand_uint32_with_reseed(uint32_t *a, uint32_t words);

uint32_t get_rand_buffer(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words);

uint32_t get_rand_with_post_processing(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words);
uint32_t get_rand_without_post_processing(uint8_t *random, uint32_t bytes, GET_RAND_WORDS get_rand_words);
#endif




#ifdef __cplusplus
}
#endif


#endif

