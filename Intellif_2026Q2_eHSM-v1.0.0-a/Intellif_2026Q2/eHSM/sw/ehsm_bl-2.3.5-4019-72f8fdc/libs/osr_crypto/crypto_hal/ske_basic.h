#ifndef SKE_BASIC_H
#define SKE_BASIC_H


#include "../crypto_include/ske_config.h"
#include "../crypto_lib/lib_extension.h"

#ifdef __cplusplus
extern "C" {
#endif




//some register offset
#define SKE_REVERSE_BYTE_ORDER_IN_WORD_OFFSET       (24U)
#define SKE_MODE_OFFSET                             (28U)
#define SKE_CRYPTO_OFFSET                           (11U)
#define SKE_UP_CFG_OFFSET                           (12U)
#define SKE_DMA_OFFSET                              (16U)
#define SKE_DMA_LL_OFFSET                           (17U)
#define SKE_LAST_DATA_OFFSET                        (16U)


#if 0
#define SKE_SECURE_PORT_OFFSET                      (17U)
#endif


#define SKE_SEED_WORDS                                 (36U)


//SKE register struct
#if 0
#define rSKE_CTRL           (*((volatile uint32_t *)(SKE_BASE_ADDR)))                 /* Offset: 0x000 (W1S) SKE Control Register */
#define rSKE_CFG            (*((volatile uint32_t *)(SKE_BASE_ADDR+0x004U)))           /* Offset: 0x004 (R/W) SKE Config Register */
#define rSKE_SR             (*((volatile uint32_t *)(SKE_BASE_ADDR+0x008U)))           /* Offset: 0x008 (R)   SKE Status Register */
#define rSKE_RISR           (*((volatile uint32_t *)(SKE_BASE_ADDR+0x00CU)))           /* Offset: 0x00C (W0C) SKE Interrupt Source Register */
//#define rSKE_IMCR           (*((volatile uint32_t *)(SKE_BASE_ADDR+0x010U)))            Offset: 0x010 (R/W) SKE Interrupt Enable Register */
//#define rSKE_MISR           (*((volatile uint32_t *)(SKE_BASE_ADDR+0x014U)))            Offset: 0x014 (R)   SKE Interrupt Output Register */
#define rSKE_SP             (*((volatile uint32_t *)(SKE_BASE_ADDR+0x01CU)))           /* Offset: 0x01C (R/W) SKE Secure Port Register */
#define rSKE_K1_BASE        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x020U)))           /* Offset: 0x020 (R/W) Key1, occupy 8 words */
#define rSKE_K2_BASE        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x040U)))           /* Offset: 0x040 (R/W) Key2, occupy 8 words */
#define rSKE_A_LEN_L        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x060U)))           /* Offset: 0x060 (R/W) CCM/GCM mode AAD length low Register */
#define rSKE_A_LEN_H        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x064U)))           /* Offset: 0x064 (R/W) CCM/GCM mode AAD length high Register */
#define rSKE_C_LEN_L        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x068U)))           /* Offset: 0x068 (R/W) CCM/GCM/XTS mode plaintext/ciphertext length low Register */
#define rSKE_C_LEN_H        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x06CU)))           /* Offset: 0x06C (R/W) CCM/GCM/XTS mode plaintext/ciphertext length high Register */
#define rSKE_IV_BASE        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x070U)))           /* Offset: 0x070 (R/W) Initial Vector, occupy 4 words */
#define rSKE_M_DIN_CR       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x080U)))           /* Offset: 0x080 (R/W) SKE Input Register */
#define rSKE_M_DIN_BASE     (*((volatile uint32_t *)(SKE_BASE_ADDR+0x090U)))           /* Offset: 0x090 (R/W) SKE Input Register, occupy 4 words */
#define rSKE_M_DOUT_BASE    (*((volatile uint32_t *)(SKE_BASE_ADDR+0x0B0U)))           /* Offset: 0x0B0 (R)   SKE Output Register, occupy 4 words */
#define rSKE_VERSION        (*((volatile uint32_t *)(SKE_BASE_ADDR+0x0FCU)))           /* Offset: 0x0FC (R)   SKE Version Register */
#define rSKE_SEED_BASE      (*((volatile uint32_t *)(SKE_BASE_ADDR+0x100U)))           /* Offset: 0x100 (R/W) SKE Seed Register, occupy 36 words */
#define rSKE_ALARM          (*((volatile uint32_t *)(SKE_BASE_ADDR+0x190U)))           /* Offset: 0x190 (R)   SKE Alarm Register */
#define rSKE_MID_IV_BASE    (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1A0U)))           /* Offset: 0x1A0 (R/W) SKE Middle Vector, occupy 4 words */
#define rSKE_MID_MAC_BASE   (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1B0U)))           /* Offset: 0x1B0 (R/W) SKE Middle MAC, occupy 4 words */
#define rSKE_TOTAL_A_LEN_L  (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1C0U)))           /* Offset: 0x1C0 (R/W) SKE GCM mode AAD total length low part */
#define rSKE_TOTAL_A_LEN_H  (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1C4U)))           /* Offset: 0x1C0 (R/W) SKE GCM mode AAD total length high part */
#define rSKE_TOTAL_C_LEN_L  (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1C8U)))           /* Offset: 0x1C0 (R/W) SKE GCM mode payload total length low part */
#define rSKE_TOTAL_C_LEN_H  (*((volatile uint32_t *)(SKE_BASE_ADDR+0x1CCU)))           /* Offset: 0x1C0 (R/W) SKE GCM mode payload total length high part */
//#define rSKE_DMA_CR         (*((volatile uint32_t *)(SKE_BASE_ADDR+0x300U)))            Offset: 0x300 (R/W) DMA Config register */
//#define rSKE_DMA_SR         (*((volatile uint32_t *)(SKE_BASE_ADDR+0x304U)))            Offset: 0x304 (W0C) DMA Status register */
#define rSKE_DMA_TO         (*((volatile uint32_t *)(SKE_BASE_ADDR+0x308U)))           /* Offset: 0x308 (R/W) DMA Timeout Threshold register */
#define rSKE_DMA_SA_L       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x310U)))           /* Offset: 0x310 (R/W) DMA Source Address Low part register */
#define rSKE_DMA_SA_H       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x314U)))           /* Offset: 0x314 (R/W) DMA Source Address High part register */
#define rSKE_DMA_DA_L       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x320U)))           /* Offset: 0x320 (R/W) DMA Destination Address Low part register */
#define rSKE_DMA_DA_H       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x324U)))           /* Offset: 0x324 (R/W) DMA Destination Address High part register */
#define rSKE_DMA_R_LEN      (*((volatile uint32_t *)(SKE_BASE_ADDR+0x330U)))           /* Offset: 0x330 (R/W) DMA read Length register */
#define rSKE_DMA_W_LEN      (*((volatile uint32_t *)(SKE_BASE_ADDR+0x334U)))           /* Offset: 0x334 (R/W) DMA write Length register */
//#define rSKE_DMA_AWCC       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x340U)))            Offset: 0x340 (R/W) DMA AWCC register */
//#define rSKE_DMA_ARCC       (*((volatile uint32_t *)(SKE_BASE_ADDR+0x344U)))            Offset: 0x344 (R/W) DMA ARCC register */
//#define rSKE_DMA_LLP_L      (*((volatile uint32_t *)(SKE_BASE_ADDR+0x348U)))            Offset: 0x348 (R/W) DMA List Address Low part register */
//#define rSKE_DMA_LLP_H      (*((volatile uint32_t *)(SKE_BASE_ADDR+0x34CU)))            Offset: 0x34C (R/W) DMA List Address High part register */

#endif




//SKE Operation Mode
typedef enum
{
#ifdef SUPPORT_SKE_MODE_BYPASS
    SKE_MODE_BYPASS               = 0,   // BYPASS Mode
#endif

#ifdef SUPPORT_SKE_MODE_ECB
    SKE_MODE_ECB                  = 1,   // ECB Mode
#endif

#ifdef SUPPORT_SKE_MODE_XTS
    SKE_MODE_XTS                  = 2,   // XTS Mode
#endif

#ifdef SUPPORT_SKE_MODE_CBC
    SKE_MODE_CBC                  = 3,   // CBC Mode
#endif

#ifdef SUPPORT_SKE_MODE_CFB
    SKE_MODE_CFB                  = 4,   // CFB Mode
#endif

#ifdef SUPPORT_SKE_MODE_OFB
    SKE_MODE_OFB                  = 5,   // OFB Mode
#endif

#ifdef SUPPORT_SKE_MODE_CTR
    SKE_MODE_CTR                  = 6,   // CTR Mode
#endif

#ifdef SUPPORT_SKE_MODE_CMAC
    SKE_MODE_CMAC                 = 7,   // CMAC Mode
#endif

#ifdef SUPPORT_SKE_MODE_CBC_MAC
    SKE_MODE_CBC_MAC              = 8,   // CBC-MAC Mode
#endif

#ifdef SUPPORT_SKE_MODE_GCM
    SKE_MODE_GCM                  = 9,   // GCM Mode
#endif

#ifdef SUPPORT_SKE_MODE_CCM
    SKE_MODE_CCM                  = 10,  // CCM Mode
#endif

#ifdef SUPPORT_SKE_MODE_GMAC
    SKE_MODE_GMAC                 = 18,  // GMAC Mode
#endif
    SKE_MODE_INVALID              = 0xFF,
} ske_mode_e;


//SKE Crypto Action
typedef enum {
    SKE_CRYPTO_ENCRYPT       = 0,   // encrypt
    SKE_CRYPTO_DECRYPT          ,   // decrypt
} ske_crypto_e;


//SKE MAC Action
typedef enum {
    SKE_GENERATE_MAC = SKE_CRYPTO_ENCRYPT,
    SKE_VERIFY_MAC   = SKE_CRYPTO_DECRYPT,
} ske_mac_e;



//SKE Algorithm
typedef enum {
#ifdef SUPPORT_SKE_DES
    SKE_ALG_DES            = 0,      // DES
#endif

#ifdef SUPPORT_SKE_TDES_128
    SKE_ALG_TDES_128       = 1,      // TDES 128 bits key
#endif

#ifdef SUPPORT_SKE_TDES_192
    SKE_ALG_TDES_192       = 2,      // TDES 192 bits key
#endif

#ifdef SUPPORT_SKE_TDES_EEE_128
    SKE_ALG_TDES_EEE_128   = 3,      // TDES_EEE 128 bits key
#endif

#ifdef SUPPORT_SKE_TDES_EEE_192
    SKE_ALG_TDES_EEE_192   = 4,      // TDES_EEE 192 bits key
#endif

#ifdef SUPPORT_SKE_AES_128
    SKE_ALG_AES_128        = 5,      // AES 128 bits key
#endif

#ifdef SUPPORT_SKE_AES_192
    SKE_ALG_AES_192        = 6,      // AES 192 bits key
#endif

#ifdef SUPPORT_SKE_AES_256
    SKE_ALG_AES_256        = 7,      // AES 256 bits key
#endif

#ifdef SUPPORT_SKE_SM4
    SKE_ALG_SM4            = 8,      // SM4
#endif

    SKE_ALG_INVALID        = 0xFF,
} ske_alg_e;


//SKE return code
#define SKE_RT_OFFSET                  (0x500U)
#define SKE_SUCCESS                    (0U)
#define SKE_BUFFER_NULL                (SKE_RT_OFFSET + 1U)
#define SKE_INPUT_INVALID              (SKE_RT_OFFSET + 2U)
#define SKE_ATTACK_ALARM               (SKE_RT_OFFSET + 3U)
#define SKE_PADDING_ERROR              (SKE_RT_OFFSET + 4U)
#define SKE_VERIFY_ERROR               (SKE_RT_OFFSET + 5U)
#define SKE_LEN_OVERFLOW               (SKE_RT_OFFSET + 6U)
#define SKE_ERROR                      (SKE_RT_OFFSET + 10U)


//SKE padding scheme
typedef enum{
    SKE_NO_PADDING = 0,
    SKE_ANSI_X923_PADDING,    //hardware supports
    SKE_PKCS_5_7_PADDING,     //hardware supports
    SKE_ISO_7816_4_PADDING,   //hardware supports
    SKE_ZERO_PADDING,         //hardware does not support, just for CBC-MAC
    SKE_INVALID_PADDING,
} ske_padding_e;


//SKE calc wait mode
typedef enum{
    WAIT_TILL_EXPAND_KEY_DONE = 0,   //wait till ske expanding key is done
    WAIT_TILL_COULD_INPUT,           //wait till ske is waiting to input
    WAIT_TILL_OUTPUT_READY,          //wait till ske output is ready
    WAIT_TILL_CALC_DONE              //wait till ske calculating is done
} ske_wait_mode_e;


//SKE stage mode
typedef enum{
    SKE_PAYLOAD_STREAM = 0,
    SKE_PAYLOAD_HEAD,
    SKE_PAYLOAD_MIDDLE,
    SKE_PAYLOAD_LAST,
} ske_payload_stage_e;


//SKE block length
typedef struct{
#if defined(CONFIG_SKE_SUPPORT_MUL_THREAD)
    uint32_t iv[16/4];
    uint32_t key_buf[64/4];
    uint8_t *key;
    uint16_t sp_key_idx;
    ske_alg_e alg;
#endif
    uint8_t block_bytes;
    uint8_t block_words;
    ske_mode_e mode;        //affected by padding
    ske_crypto_e crypto;    //affected by padding
    ske_padding_e padding;
} ske_ctx_st;



//ske callback function type
typedef void (*SKE_CALLBACK)(void);


//fix to old project
typedef ske_mode_e SKE_MODE;
typedef ske_crypto_e SKE_CRYPTO;
typedef ske_mac_e SKE_MAC;
typedef ske_alg_e SKE_ALG;
typedef ske_payload_stage_e SKE_PAYLOARD_STAGE;
typedef ske_wait_mode_e SKE_WAIT_MODE;
typedef ske_padding_e SKE_PADDING;
typedef ske_ctx_st SKE_CTX;



//APIs

uint32_t ske_get_version(void);

uint32_t ske_get_driver_version(void);

void ske_set_secure_port(void);

void ske_disable_secure_port(void);

#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
uint32_t ske_gmac_get_mac_of_msg_empty(uint32_t *mac);

void ske_gmac_set_cfg_of_msg_not_empty(uint32_t aad_bytes);
#endif

#ifdef SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
void ske_clear(void);
#endif

void ske_clear_cfg(void);

void ske_set_cpu_mode(void);

void ske_set_dma_mode(void);

#if 0
void ske_enable_dma_linked_list(void);
#endif

void ske_disable_dma_linked_list(void);

void ske_set_endian_uint32(void);

void ske_set_crypto(ske_crypto_e crypto);

void ske_set_alg(ske_alg_e alg);

void ske_set_mode(ske_mode_e mode);

void ske_set_last_block(uint32_t is_last_block);

void ske_set_last_block_len(uint32_t bytes);

uint32_t ske_set_seed(void);

void ske_start(void);

uint32_t ske_wait_till_done(ske_wait_mode_e wait_mode);

void ske_set_key_uint32(const uint32_t *key, uint32_t idx, uint32_t key_words);

void ske_set_iv_uint32(const uint32_t *iv, uint32_t block_words);

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM))
void ske_set_aad_len_uint32(uint32_t aad_bytes);
#endif

#if (defined(SUPPORT_SKE_MODE_GCM))
void ske_set_aad_total_len_uint32(uint32_t aad_bytes);
#endif

#if (defined(SUPPORT_SKE_MODE_GCM) || defined(SUPPORT_SKE_MODE_CCM) || defined(SUPPORT_SKE_MODE_XTS))
void ske_set_c_len_uint32(uint32_t c_bytes);
#endif

#if (defined(SUPPORT_SKE_MODE_GCM))
void ske_set_c_total_len_uint32(uint32_t c_bytes);
#endif

void ske_set_padding(ske_padding_e padding);

void ske_set_payload_stage(ske_payload_stage_e stage);

void ske_set_mid_iv(const uint32_t *iv, uint32_t words);

void ske_get_mid_iv(uint32_t *iv, uint32_t words);

void ske_set_mid_mac(const uint32_t *mac, uint32_t words);

void ske_get_mid_mac(uint32_t *mac, uint32_t words, uint32_t clear);

void ske_simple_set_input_block(const uint32_t *in, uint32_t block_words);

void ske_simple_get_output_block(uint32_t *out, uint32_t block_words);

uint32_t ske_expand_key(void);

void dma_set_time_out_threshold(uint16_t cycle_threshold);

#ifdef SKE_DMA_FUNCTION
uint32_t ske_dma_calc_wait_till_done(SKE_CALLBACK callback);

#ifdef CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS 
uint32_t ske_dma_operate(uint32_t in_h, uint32_t in_l, uint32_t out_h, uint32_t out_l, 
        uint32_t in_bytes, uint32_t out_bytes, SKE_CALLBACK callback);
#else
uint32_t ske_dma_operate(const uint32_t *in, const uint32_t *out, uint32_t in_bytes, uint32_t out_bytes,
        SKE_CALLBACK callback);
#endif
#endif


uint32_t ske_update_blocks_no_output(const ske_ctx_st *ctx, const uint8_t *in, uint32_t bytes);

uint32_t ske_update_blocks_align(const uint32_t *in, uint32_t *out, uint32_t blocks_num, uint32_t alg_block_words);

uint32_t ske_update_blocks_internal(const ske_ctx_st *ctx, const uint8_t *in, uint8_t *out, uint32_t bytes);


#ifdef SUPPORT_SKE_MODE_GMAC
uint32_t ske_gmac_update_blocks_internal(uint8_t *in, uint32_t bytes);
#endif


#ifdef __cplusplus
}
#endif


#endif

