#ifndef HASH_BASIC_H
#define HASH_BASIC_H



#include "../crypto_include/hash_config.h"
#include "../crypto_lib/lib_extension.h"



#ifdef __cplusplus
extern "C" {
#endif



#ifdef HMAC_SECURE_PORT_FUNCTION
#define HMAC_MAX_K_IDX                             (8U)   //if key is from secure port, the max key index(or the number of keys)
#define HMAC_MAX_SP_K_SIZE                         (64U)  //for secure port key, max bytes of one key
#endif



//some register offset
#define HASH_ATTACK_ALARM_OFFSET                     (3U)
#define HASH_HMAC_OFFSET                             (4U)
#define HASH_HMAC_SECURE_PORT_OFFSET                 (5U)
#define HASH_REVERSE_BYTE_ORDER_IN_WORD_OFFSET       (8U)
#define HASH_UPDATE_CONFIG_OFFSET                    (12U)
#define HASH_DMA_OFFSET                              (16U)
#define HASH_LAST_BLOCK_OFFSET                       (16U)

#ifdef HASH_SEC
#define HASH_SEED_WORDS                              (36U)
#endif


#if 1
#else

#define rHASH_CTRL         (*((volatile uint32_t *)(HASH_BASE_ADDR)))                  /* Offset: 0x000 (R/W)  Control register */
#define rHASH_CFG          (*((volatile uint32_t *)(HASH_BASE_ADDR+0x004U)))           /* Offset: 0x004 (R/W)  Config register */
#define rHASH_RISR         (*((volatile uint32_t *)(HASH_BASE_ADDR+0x010U)))           /* Offset: 0x010 (W0C)  Resource of Interrupt Status register */
#define rHASH_IMCR         (*((volatile uint32_t *)(HASH_BASE_ADDR+0x014U)))           /* Offset: 0x014 (R/W)  Interrupt Management and Control register */
//#define rHASH_MISR         (*((volatile uint32_t *)(HASH_BASE_ADDR+0x018U)))            Offset: 0x018 (R)    Multiple Interrupt Status register */
#define rHASH_MSG_LEN_BASE (*((volatile uint32_t *)(HASH_BASE_ADDR+0x030U)))           /* Offset: 0x030 (R/W)  message total length register, 4 words */
#define rHASH_MSG_CNT_BASE (*((volatile uint32_t *)(HASH_BASE_ADDR+0x040U)))           /* Offset: 0x040 (R/W)  message been handled length register, 4 words */
#define rHASH_K_LEN        (*((volatile uint32_t *)(HASH_BASE_ADDR+0x060U)))           /* Offset: 0x060 (R/W)  HMAC KEY total length register */
#define rHASH_K_CNT        (*((volatile uint32_t *)(HASH_BASE_ADDR+0x070U)))           /* Offset: 0x070 (R/W)  HMAC KEY been handled length register */
#define rHASH_MDIN_CR      (*((volatile uint32_t *)(HASH_BASE_ADDR+0x0B0U)))           /* Offset: 0x0B0 (R/W)  Data flag register */
#define rHASH_M_DIN        (*((volatile uint32_t *)(HASH_BASE_ADDR+0x0C0U)))           /* Offset: 0x0C0 (W)    Hash message Input register */
#define rHASH_VERSION      (*((volatile uint32_t *)(HASH_BASE_ADDR+0x0FCU)))           /* Offset: 0x0FC (R)    Version register */
#define rHASH_IN_BASE      (*((volatile uint32_t *)(HASH_BASE_ADDR+0x100U)))           /* Offset: 0x100 (W)    Hash iterator Input register, 50 words */
#define rHASH_OUT_BASE     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x200U)))           /* Offset: 0x200 (R)    Output register, 50 words */
#ifdef HASH_SEC
#define rHASH_SEED_BASE    (*((volatile uint32_t *)(HASH_BASE_ADDR+0x300U)))           /* Offset: 0x100 (R/W)  Seed Register, 36 words */
#endif
#define rHASH_DMA_SA_L     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x490U)))           /* Offset: 0x490 (R/W)  DMA Source Address Low part register */
#define rHASH_DMA_SA_H     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x494U)))           /* Offset: 0x494 (R/W)  DMA Source Address High part register */
#define rHASH_DMA_DA_L     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x498U)))           /* Offset: 0x498 (R/W)  DMA Destination Address Low part register */
#define rHASH_DMA_DA_H     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x49CU)))           /* Offset: 0x49c (R/W)  DMA Destination Address High part register */
#define rHASH_DMA_RLEN     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x4A0U)))           /* Offset: 0x4A0 (R/W)  DMA read data length register */
#define rHASH_DMA_WLEN     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x4A4U)))           /* Offset: 0x4A4 (R/W)  DMA write data length register */
//#define rHASH_DMA_AWCC     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x4A8U)))            Offset: 0x4A8 (R/W)  DMA AWCC register */
//#define rHASH_DMA_ARCC     (*((volatile uint32_t *)(HASH_BASE_ADDR+0x4ACU)))            Offset: 0x4AC (R/W)  DMA ARCC register */

#endif



//HASH max length
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
#define HASH_DIGEST_MAX_WORD_LEN       (16U)
#define HASH_BLOCK_MAX_WORD_LEN        (36U)
#elif (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) ||defined(SUPPORT_HASH_SHA512_224) ||defined(SUPPORT_HASH_SHA512_256))
#define HASH_DIGEST_MAX_WORD_LEN       (16U)
#define HASH_BLOCK_MAX_WORD_LEN        (32U)
#else
#define HASH_DIGEST_MAX_WORD_LEN       (8U)
#define HASH_BLOCK_MAX_WORD_LEN        (16U)
#endif

#define HASH_BLOCK_MAX_BYTE_LEN        (HASH_BLOCK_MAX_WORD_LEN<<2)

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
#define HASH_ITERATOR_MAX_WORD_LEN     (50U)
#else
#define HASH_ITERATOR_MAX_WORD_LEN     HASH_DIGEST_MAX_WORD_LEN
#endif

#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) ||defined(SUPPORT_HASH_SHA512_224) ||defined(SUPPORT_HASH_SHA512_256))
#define HASH_TOTAL_LEN_MAX_WORD_LEN     (4U)
#else
#define HASH_TOTAL_LEN_MAX_WORD_LEN     (2U)
#endif



//HASH algorithm definition
typedef enum {
#ifdef SUPPORT_HASH_SM3
    HASH_SM3                      = 0,
#endif

#ifdef SUPPORT_HASH_MD5
    HASH_MD5                      = 1,
#endif

#ifdef SUPPORT_HASH_SHA256
    HASH_SHA256                   = 2,
#endif

#ifdef SUPPORT_HASH_SHA384
    HASH_SHA384                   = 3,
#endif

#ifdef SUPPORT_HASH_SHA512
    HASH_SHA512                   = 4,
#endif

#ifdef SUPPORT_HASH_SHA1
    HASH_SHA1                     = 5,
#endif

#ifdef SUPPORT_HASH_SHA224
    HASH_SHA224                   = 6,
#endif

#ifdef SUPPORT_HASH_SHA512_224
    HASH_SHA512_224               = 7,
#endif

#ifdef SUPPORT_HASH_SHA512_256
    HASH_SHA512_256               = 8,
#endif

    HASH_SHA3_START               = 9,
#ifdef SUPPORT_HASH_SHA3_224
    HASH_SHA3_224                 = 9,
#endif

#ifdef SUPPORT_HASH_SHA3_256
    HASH_SHA3_256                 = 10,
#endif

#ifdef SUPPORT_HASH_SHA3_384
    HASH_SHA3_384                 = 11,
#endif

#ifdef SUPPORT_HASH_SHA3_512
    HASH_SHA3_512                 = 12,
#endif

    HASH_INVALID_ALG              = 0xFF,
} hash_alg_e;


//HASH return code
#define HASH_RT_OFFSET                        (0x400U)
#define HASH_SUCCESS                          (0U)
#define HASH_BUFFER_NULL                      (HASH_RT_OFFSET + 1U)
#define HASH_INPUT_INVALID                    (HASH_RT_OFFSET + 2U)
#define HASH_LEN_OVERFLOW                     (HASH_RT_OFFSET + 3U)
#define HASH_OUTPUT_ZERO_ALL                  (HASH_RT_OFFSET + 4U)

#if (defined(HASH_SEC) || (defined(HASH_CONFIG_SUPPORT_STATIC_ANALYSIS)))
#define HASH_ATTACK_ALARM                     (HASH_RT_OFFSET + 5U)
#endif

#define HASH_ERROR                            (HASH_RT_OFFSET + 10U)



//hash callback function type
typedef void (*HASH_CALLBACK)(void);



//APIs
uint32_t hash_get_version(void);

uint32_t hash_get_driver_version(void);

void hash_set_cpu_mode(void);

void hash_set_dma_mode(void);

void hash_set_hash_mode(void);

void hash_set_hmac_mode(void);

void hash_set_hmac_key_mode(void);

void hash_clear_hmac_key_mode(void);

void hash_clear_risr(void);

void hash_set_hmac_key_cnt(uint32_t bitlen);

void hash_set_hmac_key_len(uint32_t bitlen);

void hash_clear_cfg(void);

uint32_t hash_hmac_key_opr_one_block(const uint32_t *key, uint32_t block_byte_len);

uint32_t hash_hmac_key_opr_longer_than_one_block(const uint8_t *key, uint32_t key_bytes);

uint32_t hash_hmac_sp_key_opr(uint16_t sp_key_idx, uint32_t key_bits);

void hash_set_alg(hash_alg_e alg);

uint32_t hash_check_whether_sha3_alg(hash_alg_e alg);

void hash_update_config(void);

void hash_enable_cpu_interruption(void);

void hash_disable_cpu_interruption(void);

void hash_enable_dma_interruption(void);

void hash_disable_dma_interruption(void);

void hash_set_dma_output_len(uint32_t bytes);

void hash_set_last_block(uint32_t tag);

void hash_set_endian_uint32(void);

void hash_hmac_disable_secure_port(void);

void hash_get_iterator(uint8_t *iterator, uint32_t hash_iterator_words);

void hash_set_iterator(const uint32_t *iterator, uint32_t hash_iterator_words);

void hash_set_msg_len(uint32_t bytelen);

void hash_set_msg_total_bit_len(const uint32_t *msg_total_bits, uint32_t block_byte_len);

#ifdef HASH_SEC
uint32_t hash_set_seed(void);
#endif

void hash_start(void);

uint32_t hash_wait_till_done(void);

void hash_input_msg_u8(const uint8_t *msg, uint32_t msg_bytes);

void hash_hmac_enable_secure_port(void);

uint32_t hash_dma_wait_till_done(HASH_CALLBACK callback);

#ifdef HASH_DMA_FUNCTION
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_operate(uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, uint32_t inByteLen, 
        HASH_CALLBACK callback);
#else
uint32_t hash_dma_operate(const uint32_t *in, const uint32_t *out, uint32_t inByteLen, HASH_CALLBACK callback);
#endif
#endif




#ifdef __cplusplus
}
#endif

#endif 
