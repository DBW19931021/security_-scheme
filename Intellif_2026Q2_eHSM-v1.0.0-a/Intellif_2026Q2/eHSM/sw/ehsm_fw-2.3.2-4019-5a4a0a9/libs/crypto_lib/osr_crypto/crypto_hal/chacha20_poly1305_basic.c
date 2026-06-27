#include "chacha20_poly1305_basic.h"
#include "../crypto_include/crypto_common/utility.h"

/* HASH register struct  */
typedef struct {
    uint32_t ctrl[1];       /* Offset: 0x000 (W1S) chacha20_poly1305 Control Register */
    uint32_t cfg[1];        /* Offset: 0x004 (R/W) chacha20_poly1305 Config Register */
    uint32_t soft_rst_n[1]; /* Offset: 0x008 (W0)  chacha20_poly1305 Soft Reset Register*/
    uint32_t din_rdy[1];    /* Offset: 0x00C (R0)  chacha20_poly1305 Ready Data Register*/
    uint32_t risr[1];       /* Offset: 0x010 (W0C) chacha20_poly1305 Interrupt Source Register */
    uint32_t imcr[1];       /* Offset: 0x014 (R/W) chacha20_poly1305 Interrupt Enable Register */
    uint32_t status[1];     /* Offset: 0x018 (R)   chacha20_poly1305 Status Register */
    uint32_t err_code[1];   /* Offset: 0x01C (R)   chacha20_poly1305 Error Code Register */
    uint32_t last[1];       /* Offset: 0x020 (R/W) chacha20_poly1305 Last Data Register */
    uint32_t se_done[1];    /* Offset: 0x024 (R)   chacha20_poly1305 Secure Key Configured Register */
    uint32_t rev2[2];
    uint32_t key[8];      /* Offset: 0x030 (W)   chacha20_poly1305 Key Register */
    uint32_t iv[2];       /* Offset: 0x050 (W)   chacha20_poly1305 IV Register */
    uint32_t constant[1]; /* Offset: 0x058 (W)   chacha20_poly1305 Constant Register */
    uint32_t rev3[1];
    uint32_t length[5]; /* Offset: 0x060 (W)   chacha20_poly1305 Length Register*/
    uint32_t cnt[1];    /* Offset: 0x074 (W)   chacha20_poly1305 Counter Register */
    uint32_t rev4[2];
    uint32_t tag_in[5]; /* Offset: 0x080 (W)   chacha20_poly1305 Tag Input Register */
    uint32_t din[1];    /* Offset: 0x094 (W)   chacha20_poly1305 Data Input Register */
    uint32_t rev6[25];
    uint32_t version[1]; /* Offset: 0x0FC (R)   chacha20_poly1305 Version Register */
    uint32_t dout[4];    /* Offset: 0x100 (R)   chacha20_poly1305 Data Output Register */
    uint32_t tout[5];    /* Offset: 0x110 (R)   chacha20_poly1305 Tag Output register */
    uint32_t cout[1];    /* Offset: 0x124 (R)   chacha20_poly1305 Counter Output Register */
    uint32_t rev7[38];
    uint32_t dma_saddr_a_l[1]; /* Offset: 0x1C0 (R/W) DMA Source AAD Register */
    uint32_t dma_saddr_a_h[1]; /* Offset: 0x1C4 (R/W) DMA Source AAD Register */
    uint32_t dma_rlen_a[1];    /* Offset: 0x1C8 (R/W) DMA Reading AAD Byte Length Register */
    uint32_t rev[1];
    uint32_t dma_saddr_d_l[1]; /* Offset: 0x1D0 (R/W) DMA Source Payload Register */
    uint32_t dma_saddr_d_h[1]; /* Offset: 0x1D4 (R/W) DMA Source Payload Register */
    uint32_t dma_daddr_l[1];   /* Offset: 0x1D8 (R/W) DMA Destination Payload Register */
    uint32_t dma_daddr_h[1];   /* Offset: 0x1DC (R/W) DMA Destination Payload Register */
    uint32_t dma_len_d[1];     /* Offset: 0x1E0 (R/W) DMA Reading Payload Byte Length Register */
    uint32_t final;
} chacha20_poly1305_reg_st;

/* hash register pointer */
#ifdef CONFIG_UNIT_TEST
volatile static chacha20_poly1305_reg_st *g_chacha20_poly1305_reg
    = (chacha20_poly1305_reg_st *)CHACHA20_POLY1305_BASE_ADDR;
#else
volatile static chacha20_poly1305_reg_st *const g_chacha20_poly1305_reg
    = (chacha20_poly1305_reg_st *)CHACHA20_POLY1305_BASE_ADDR;
#endif

#define rCHACHA20_POLY1305_CTRL                      \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->ctrl))) /* Offset: 0x000 (W1S) chacha20_poly1305 Control Register */
#define rCHACHA20_POLY1305_CFG                                                                                        \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg->cfg))) /* Offset: 0x004 (R/W) chacha20_poly1305 Config Register \
                                                              */
#define rCHACHA20_POLY1305_SOFT_RST_N                \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->soft_rst_n))) /* Offset: 0x008 (W0)  chacha20_poly1305 Soft Reset Register*/
#define rCHACHA20_POLY1305_DIN_RDY                   \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->din_rdy))) /* Offset: 0x00C (R0)  chacha20_poly1305 Ready Data Register*/
#define rCHACHA20_POLY1305_RISR                      \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->risr))) /* Offset: 0x010 (W0C) chacha20_poly1305 Interrupt Source Register */
#define rCHACHA20_POLY1305_STATUS                    \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->status))) /* Offset: 0x018 (R)   chacha20_poly1305 Status Register */
#define rCHACHA20_POLY1305_ERR_CODE                  \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->err_code))) /* Offset: 0x01C (R)   chacha20_poly1305 Error Code Register */
#define rCHACHA20_POLY1305_LAST                      \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->last))) /* Offset: 0x020 (R/W) chacha20_poly1305 Last Data Register */
#define rCHACHA20_POLY1305_SE_DONE                                   \
    (*((volatile uint32_t *)(CHACHA20_POLY1305_BASE_ADDR + 0x024U))) \
            Offset : 0x024(R)chacha20_poly1305 Secure Key Configured Register * /
#define rCHACHA20_POLY1305_K_BASE                    \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->key))) /* Offset: 0x030 (W)   chacha20_poly1305 Key Register, 8 words*/
#define rCHACHA20_POLY1305_IV_BASE                   \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->iv))) /* Offset: 0x050 (W)   chacha20_poly1305 IV Register, 2 words*/
#define rCHACHA20_POLY1305_CONSTANT                  \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->constant))) /* Offset: 0x058 (W)   chacha20_poly1305 Constant Register */
#define rCHACHA20_POLY1305_LENGTH_BASE               \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->length))) /* Offset: 0x060 (W)   chacha20_poly1305 Length Register, 5 words */
#define rCHACHA20_POLY1305_CNT                       \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->cnt))) /* Offset: 0x074 (W)   chacha20_poly1305 Counter Register */
#define rCHACHA20_POLY1305_TAG_IN_BASE               \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->tag_in))) /* Offset: 0x080 (W)   chacha20_poly1305 Tag Input Register, 5 words */
#define rCHACHA20_POLY1305_DIN                       \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->din))) /* Offset: 0x094 (W)   chacha20_poly1305 Data Input Register */
#define rCHACHA20_POLY1305_VERSION                   \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->version))) /* Offset: 0x0FC (R)   chacha20_poly1305 Version Register */
#define rCHACHA20_POLY1305_DOUT_BASE                 \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dout))) /* Offset: 0x100 (R)   chacha20_poly1305 Data Output Register, 4 words */
#define rCHACHA20_POLY1305_TOUT_BASE                 \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->tout))) /* Offset: 0x110 (R)   chacha20_poly1305 Tag Output register, 5 words */
#define rCHACHA20_POLY1305_COUT                      \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->cout))) /* Offset: 0x124 (R)   chacha20_poly1305 Counter Output Register */
#define rCHACHA20_POLY1305_DMA_SADDR_L_A                                                                              \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg->dma_saddr_a_l))) /* Offset: 0x1C0 (R/W) DMA Source AAD Register \
                                                                        */
#define rCHACHA20_POLY1305_DMA_SADDR_H_A                                                                              \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg->dma_saddr_a_h))) /* Offset: 0x1C0 (R/W) DMA Source AAD Register \
                                                                        */
#define rCHACHA20_POLY1305_DMA_RLEN_A                \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_rlen_a))) /* Offset: 0x1C4 (R/W) DMA Reading AAD Byte Length Register */
#define rDMA_SADDR_L_D                               \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_saddr_d_l))) /* Offset: 0x1D0 (R/W) DMA Source Payload Register */
#define rDMA_SADDR_H_D                               \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_saddr_d_h))) /* Offset: 0x1D0 (R/W) DMA Source Payload Register */
#define rCHACHA20_POLY1305_DMA_DADDR_L_D             \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_daddr_l))) /* Offset: 0x1D4 (R/W) DMA Destination Payload Register */
#define rCHACHA20_POLY1305_DMA_DADDR_H_D             \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_daddr_h))) /* Offset: 0x1D4 (R/W) DMA Destination Payload Register */
#define rCHACHA20_POLY1305_DMA_LEN_D                 \
    (*((volatile uint32_t *)(g_chacha20_poly1305_reg \
            ->dma_len_d))) /* Offset: 0x1D8 (R/W) DMA Reading Payload Byte Length Register */

#ifdef CONFIG_UNIT_TEST
#define rCHACHA20_POLY1305_RST (rCHACHA20_POLY1305_CTRL)
#else
#define rCHACHA20_POLY1305_RST (*((volatile uint32_t *)(SYSTEM_BASE_ADDR + 0x3310U)))
#endif

#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
extern volatile uint32_t g_chacha20_poly1305_suspend_flag;
volatile uint32_t g_chacha20_poly1305_suspend_flag = 0U;

/* function: chacha20_poly1305 reset
 * parameters: none
 * return:
 * caution:
 */
void chacha20_poly1305_reset(void)
{

    rCHACHA20_POLY1305_RST = (0x5A);  /* active low */
    rCHACHA20_POLY1305_RST = (0x15A); /* active high */
}

/* function: chacha20_poly1305 set suspend flag
 * parameters: none
 * return:
 * caution:
 */
void chacha20_poly1305_set_suspend_flag(void)
{

    g_chacha20_poly1305_suspend_flag = ~0U;
}

/* function: chacha clear suspend flag
 * parameters: none
 * return:
 * caution:
 */
void chacha20_poly1305_clear_suspend_flag(void)
{

    g_chacha20_poly1305_suspend_flag = 0U;
}

/* function: chacha get suspend status
 * parameters: none
 * return:
 * caution:
 */
void chacha20_poly1305_get_suspend_status(uint32_t *ret)
{

    if (0U != g_chacha20_poly1305_suspend_flag) {
        *ret = CHACHA20_POLY1305_SUSPEND;
    } else {
    }
}
#endif

/* function: set chacha20_poly1305 to be CPU mode
 * parameters: none
 * return: none
 * caution:
 */
void chacha20_poly1305_set_cpu_mode(void)
{

    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1) << CHACHA20_POLY1305_DMA_OFFSET);

    rCHACHA20_POLY1305_CFG &= mask;
}

/* function: set chacha20_poly1305 to be DMA mode
 * parameters: none
 * return: none
 * caution:
 */
void chacha20_poly1305_set_dma_mode(void)
{

    MEM_VOLATILE uint32_t flag = (((uint32_t)1) << CHACHA20_POLY1305_DMA_OFFSET);

    rCHACHA20_POLY1305_CFG |= flag;
}

/* function: chacha20_poly1305 set crypto mode
 * parameters:
 *     crypto --------------------- input, dec or enc
 * return: none
 * caution:
 */
void chacha20_poly1305_set_crypto(chacha20_poly1305_crypto_e crypto)
{

    MEM_VOLATILE uint32_t mask = ~(((uint32_t)1) << chacha20_poly1305_crypto_e_OFFSET);

    rCHACHA20_POLY1305_CFG &= mask;
    rCHACHA20_POLY1305_CFG |= (((uint32_t)crypto) << chacha20_poly1305_crypto_e_OFFSET);
}

/* function: chacha20_poly1305 set sec_stage
 * parameters:
 *     stage --------------------- input, data interleave stage
 * return: error code
 * caution:
 */
void chacha20_poly1305_set_sec_stage(chacha20_poly1305_stage_e stage)
{

    MEM_VOLATILE uint32_t mask = ~(0x00000003U << chacha20_poly1305_stage_e_OFFSET);

    rCHACHA20_POLY1305_CFG &= mask;                                                    /* clear bit [0:1] */
    rCHACHA20_POLY1305_CFG |= (((uint32_t)stage) << chacha20_poly1305_stage_e_OFFSET); /* set stage */
}

/* function: chacha20_poly1305 start
 * parameters:
 *     stage --------------------- input, 0: calculate one-time, other: step-by-step
 * return: none
 * caution:
 */
void chacha20_poly1305_start(void)
{

    MEM_VOLATILE uint32_t start_flag = 1U;

    rCHACHA20_POLY1305_CTRL |= start_flag;
}

/* function: wait till chacha20_poly1305 calculating is done
 * parameters:
 * return: none
 * caution:
 */
void chacha20_poly1305_wait_till_done(void)
{

    MEM_VOLATILE uint32_t finish_flag = 1U;
    MEM_VOLATILE uint32_t clear_flag = 0U;

    while (0U == (rCHACHA20_POLY1305_RISR & finish_flag)) {
#ifdef CONFIG_CHACHA20_POLY1305_SUPPORT_TASK_DROP
        if (0U != g_chacha20_poly1305_suspend_flag) {
            break;
        } else {
        }
#endif
    }

    rCHACHA20_POLY1305_RISR = clear_flag; /* write 0 to clear */
}

/* function: chacha20_poly1305 set constant
 * parameters:
 *     constant ---------------------- input, constant of nonce
 * return: none
 * caution:
 */
void chacha20_poly1305_set_const(uint32_t constant)
{

    rCHACHA20_POLY1305_CONSTANT = constant;
}

/* function: chacha20_poly1305 set cnt
 * parameters:
 *     count ---------------------- input, counter
 * return: none
 * caution:
 */
void chacha20_poly1305_set_cnt(uint32_t counter)
{

    rCHACHA20_POLY1305_CNT = counter;
}

/* function: chacha20_poly1305 set tag_in
 * parameters:
 *     tag_in ---------------------- input, tag in byte buffer
 * return: none
 * caution:
 */
void chacha20_poly1305_set_tag_in(const uint8_t tag_in[17])
{
    uint32_t i = 4U;
    uint32_t tmp[4];

    if (0U != check_addr_not_word_align(tag_in)) {
        memcpy_(tmp, tag_in, 16);
        while (0U != i) {
            i--;
            (&rCHACHA20_POLY1305_TAG_IN_BASE)[i] = ((uint32_t *)tmp)[i];
        }
    } else {
        while (0U != i) {
            i--;
            (&rCHACHA20_POLY1305_TAG_IN_BASE)[i] = ((const uint32_t *)tag_in)[i];
        }
    }

    ((&rCHACHA20_POLY1305_TAG_IN_BASE)[4]) = (uint32_t)tag_in[16];
}

/* function:chacha20_poly1305 get current counter
 * parameters:none
 * return: none
 * caution:
 */
uint32_t chacha20_poly1305_get_current_cnt(void)
{

    return rCHACHA20_POLY1305_COUT;
}

/* function:chacha20_poly1305 get current tag
 * parameters:
 *     tag -------------- output, current tag
 * return: none
 * caution:
 */
void chacha20_poly1305_get_current_tag(uint8_t tag[17])
{
    uint32_t tmp[4];
    tmp[0] = (&(rCHACHA20_POLY1305_TOUT_BASE))[0];
    tmp[1] = (&(rCHACHA20_POLY1305_TOUT_BASE))[1];
    tmp[2] = (&(rCHACHA20_POLY1305_TOUT_BASE))[2];
    tmp[3] = (&(rCHACHA20_POLY1305_TOUT_BASE))[3];
    memcpy_(tag, tmp, 16);
    tag[16] = (uint8_t)((&(rCHACHA20_POLY1305_TOUT_BASE))[4] & 3U);
}

/* function: wait till chacha20_poly1305 can config data
 * parameters:none
 * return: none
 * caution:
 */
void chacha20_poly1305_wait_ready_data_signal(void)
{

    MEM_VOLATILE uint32_t finish_flag = 1;

    while (0U == (rCHACHA20_POLY1305_DIN_RDY & finish_flag)) { }
}

/* function: wait till core is idle
 * parameters:none
 * return: none
 * caution:
 */
void chacha20_poly1305_wait_core_idle(void)
{

    MEM_VOLATILE uint32_t busy_flag = 1;
    MEM_VOLATILE uint32_t clear_flag = 0;

    while (0U != (rCHACHA20_POLY1305_STATUS & busy_flag)) { }

    rCHACHA20_POLY1305_RISR = clear_flag;
}

/* function: set one word data
 * parameters:
 *     in -------------- input, pointer to data, occupies one word
 * return: none
 * caution:
 */
void chacha20_poly1305_simple_set_input_word(const uint32_t *in)
{

    rCHACHA20_POLY1305_DIN = in[0];
}

/* function: set 4 words input data as one block
 * parameters:
 *     in -------------- input, pointer to data
 * return: none
 * caution:
 */
void chacha20_poly1305_set_input_block(const uint32_t in[4])
{
    rCHACHA20_POLY1305_DIN = in[0];
    rCHACHA20_POLY1305_DIN = in[1];
    rCHACHA20_POLY1305_DIN = in[2];
    rCHACHA20_POLY1305_DIN = in[3];
}

/* function: get 4 words output data
 * parameters:
 * return: none
 * caution:
 */
void chacha20_poly1305_get_output_block(uint32_t out[4])
{
    out[0] = (&(rCHACHA20_POLY1305_DOUT_BASE))[0];
    out[1] = (&(rCHACHA20_POLY1305_DOUT_BASE))[1];
    out[2] = (&(rCHACHA20_POLY1305_DOUT_BASE))[2];
    out[3] = (&(rCHACHA20_POLY1305_DOUT_BASE))[3];
}

#ifdef CHACHA20_POLY1305_DMA_FUNCTION
/* function: basic chacha20_poly1305 DMA set source address and byte length of aad
 * parameters:
 *     aad ------------------------- input, source address of aad
 *     aad_bytes ------------------- output, byte length of aad
 * return: none
 * caution:
 */
void chacha20_poly1305_dma_set_aad(const uint32_t *aad, uint32_t aad_bytes)
{

    rCHACHA20_POLY1305_DMA_SADDR_H_A = (uint32_t)((uint64_t)(uintptr_t)aad >> 32);
    rCHACHA20_POLY1305_DMA_SADDR_L_A = (uint32_t)(uintptr_t)aad;

    rCHACHA20_POLY1305_DMA_RLEN_A = aad_bytes;
}

/* function: basic chacha20_poly1305 DMA set source address and byte length of payload
 * parameters:
 * 	payload ------------------------- input, source address of payload
 * 	payload_bytes ------------------- output, byte length of payload
 * return: none
 * caution:
 */
void chacha20_poly1305_dma_set_payload(const uint32_t *payload, uint32_t payload_bytes)
{

    rDMA_SADDR_H_D = (uint32_t)((uint64_t)(uintptr_t)payload >> 32);
    rDMA_SADDR_L_D = (uint32_t)(uintptr_t)payload;

    rCHACHA20_POLY1305_DMA_LEN_D = payload_bytes;
}

/* function:chacha20_poly1305 dma set cfg
 * parameters:
 *     in ------------------------- input, plaintext or ciphertext
 *     out ------------------------ output, ciphertext or plaintext
 *     bytes ---------------------- input, bytes length of input or output
 * return: none
 * caution:
 */
void chacha20_poly1305_dma_set_cfg(const uint32_t *in, const uint32_t *out, uint32_t bytes)
{

    rDMA_SADDR_H_D = (uint32_t)((uint64_t)(uintptr_t)in >> 32);
    rCHACHA20_POLY1305_DMA_DADDR_H_D = (uint32_t)((uint64_t)(uintptr_t)out >> 32);

    /* src & dst addr */
    rDMA_SADDR_L_D = (uint32_t)(uintptr_t)in;
    rCHACHA20_POLY1305_DMA_DADDR_L_D = (uint32_t)(uintptr_t)out;

    /* data byte length */
    rCHACHA20_POLY1305_DMA_RLEN_A = 0;
    rCHACHA20_POLY1305_DMA_LEN_D = bytes;
}

/* function:chacha20_poly1305 set last mode
 * parameters:
 * return: none
 * caution:
 */
void chacha20_poly1305_set_last_mode(void)
{

    MEM_VOLATILE uint32_t flag = 1;

    rCHACHA20_POLY1305_LAST |= flag;
}

/* function:chacha20_poly1305 set non last mode
 * parameters:
 * return: none
 * caution:
 */
void chacha20_poly1305_set_non_last_mode(void)
{

    MEM_VOLATILE uint32_t mask = 0U;

    rCHACHA20_POLY1305_LAST &= mask;
}

/* function:chacha20_poly1305 get err code status
 * parameters:
 * return: none
 * caution:
 */
uint32_t chacha20_poly1305_get_error_status(void)
{

    return rCHACHA20_POLY1305_ERR_CODE;
}

/* function:chacha20_poly1305 set length
 * parameters:
 *     index ---------------------- input, length register index
 *     value ---------------------- input, value
 * caution:
 */
void chacha20_poly1305_set_length_basic(uint32_t index, uint32_t value)
{

    (&rCHACHA20_POLY1305_LENGTH_BASE)[index] = value;
}

/* function:chacha20_poly1305 set iv
 * parameters:
 *     index ---------------------- input, length register index
 *     value ---------------------- input, value
 * caution:
 */
void chacha20_poly1305_set_iv_basic(uint32_t index, uint32_t value)
{

    (&rCHACHA20_POLY1305_IV_BASE)[index] = value;
}

/* function:chacha20_poly1305 set key
 * parameters:
 *     index ---------------------- input, length register index
 *     value ---------------------- input, value
 * caution:
 */
void chacha20_poly1305_set_key_basic(uint32_t index, uint32_t value)
{

    (&rCHACHA20_POLY1305_K_BASE)[index] = value;
}
#endif
