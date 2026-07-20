#ifndef CHACHA20_POLY1305_BASE_H
#define CHACHA20_POLY1305_BASE_H

#include <string.h>
#include "../crypto_include/chacha20_poly1305_config.h"


#ifdef __cplusplus
extern "C" {
#endif


/* these two macro is for CHACHA20_POLY1305 */
#define CHACHA20_POLY1305_DMA_ENABLE                              (1U)
#define CHACHA20_POLY1305_DMA_DISABLE                             (0U)


/* some register offset */
#define CHACHA20_POLY1305_SOFT_RESET_OFFSET                       (0U)
#define chacha20_poly1305_crypto_e_OFFSET                           (2U)
#define CHACHA20_POLY1305_DMA_OFFSET                              (3U)
#define chacha20_poly1305_stage_e_OFFSET                        (0U)

/* other */
#define CHACHA20_BLOCK_BYTE_LEN                                   (64U)
#define POLY1305_TAG_BYTE_LEN                                     (16U)
#define POLY1305_CUR_TAG_BYTE_LEN                                 (17U)

/******************* ChaCha20_Poly1305 return code ********************/
#define CHACHA20_POLY1305_RT_OFFSET                               (0x700U)
#define CHACHA20_POLY1305_SUCCESS                                 (0U)
#define CHACHA20_POLY1305_BUFFER_NULL                             (CHACHA20_POLY1305_RT_OFFSET+1U)
#define CHACHA20_POLY1305_CONFIG_INVALID                          (CHACHA20_POLY1305_RT_OFFSET+2U)
#define CHACHA20_POLY1305_INPUT_INVALID                           (CHACHA20_POLY1305_RT_OFFSET+3U)
#define CHACHA20_POLY1305_VERIFY_FAIL                             (CHACHA20_POLY1305_RT_OFFSET+6U)
#define CHACHA20_POLY1305_SUSPEND                                 (CHACHA20_POLY1305_RT_OFFSET+7U)


/* chacha20_poly1305 Crypto Action */
typedef enum {
    CHACHA20_POLY1305_CRYPTO_ENCRYPT       = 0,   /*  encrypt */
    CHACHA20_POLY1305_CRYPTO_DECRYPT              /*  decrypt */
} chacha20_poly1305_crypto_e;


/* chacha20_poly1305 Stage Section */
typedef enum {
    CHACHA20_POLY1305_STAGE_ALL            = 0,
    CHACHA20_POLY1305_STAGE_INIT              ,
    CHACHA20_POLY1305_STAGE_MIDDLE            ,
    CHACHA20_POLY1305_STAGE_LAST
} chacha20_poly1305_stage_e;


typedef struct {
	uint8_t cur_tag[17];
	uint32_t cur_cnt;
	uint64_t aad_bytes;
	uint64_t payload_bytes;
    chacha20_poly1305_crypto_e crypto;
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_MUL_THREAD
    uint8_t key[32];
    uint8_t iv[8];
    uint32_t constant;
#endif
} chacha20_poly1305_st, chacha20_poly1305_dma_st;


/* fix to old project */
typedef chacha20_poly1305_st CHACHA20_POLY1305_CTX;
typedef chacha20_poly1305_dma_st CHACHA20_POLY1305_DMA_CTX;




/* APIs */
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
void chacha20_poly1305_reset(void);

void chacha20_poly1305_set_suspend_flag(void);

void chacha20_poly1305_clear_suspend_flag(void);

void chacha20_poly1305_get_suspend_status(uint32_t *ret);
#endif

void chacha20_poly1305_set_cpu_mode(void);

void chacha20_poly1305_set_dma_mode(void);

void chacha20_poly1305_set_crypto(chacha20_poly1305_crypto_e crypto);

void chacha20_poly1305_set_sec_stage(chacha20_poly1305_stage_e stage);

void chacha20_poly1305_start(void);

void chacha20_poly1305_wait_till_done(void);

void chacha20_poly1305_set_const(uint32_t constant);

void chacha20_poly1305_set_cnt(uint32_t counter);

void chacha20_poly1305_set_tag_in(const uint8_t tag_in[17]);

uint32_t chacha20_poly1305_get_current_cnt(void);

void chacha20_poly1305_get_current_tag(uint8_t tag[17]);

void chacha20_poly1305_wait_ready_data_signal(void);

void chacha20_poly1305_wait_core_idle(void);

void chacha20_poly1305_simple_set_input_word(const uint32_t *in);

void chacha20_poly1305_set_input_block(const uint32_t in[4]);

void chacha20_poly1305_get_output_block(uint32_t out[4]);

void chacha20_poly1305_dma_set_aad(const uint32_t *aad, uint32_t aad_bytes);

void chacha20_poly1305_dma_set_payload(const uint32_t *payload, uint32_t payload_bytes);

void chacha20_poly1305_dma_set_cfg(const uint32_t *in, const uint32_t *out, uint32_t bytes);

void chacha20_poly1305_set_last_mode(void);

void chacha20_poly1305_set_non_last_mode(void);

uint32_t chacha20_poly1305_get_error_status(void);

void chacha20_poly1305_set_length_basic(uint32_t index, uint32_t value);

void chacha20_poly1305_set_iv_basic(uint32_t index, uint32_t value);

void chacha20_poly1305_set_key_basic(uint32_t index, uint32_t value);

#ifdef __cplusplus
}
#endif


#endif
