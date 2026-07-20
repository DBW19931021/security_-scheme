
#include "../crypto_include/pke_config.h"


#if (defined(SUPPORT_SM2) && defined(SM2_SEC))

#include "sm2_sec_kmu.h"
#include "../crypto_include/crypto_common/utility_sec.h"
#include "pke/eccp_sec_common.h"
#include "../crypto_include/pke/sm2.h"
#include "../crypto_include/trng/trng.h"



#define SM2_SEC_SIGN_COUNTER        (0x64D0B4FEU)
#define SM2_SEC_SIGN_COUNTER1       (0x3A53F102U)
#define SM2_SEC_SIGN_COUNTER2       (0x9F758C1EU)
#define SM2_SEC_DEC_COUNTER         (0x157A396AU)
#define SM2_SEC_EXC_COUNTER         (0xF5264A87U)

#ifdef CONFIG_UNIT_TEST
uint32_t g_unit_ram[400];
#define EHSM_UNIT_BASE   (&g_unit_ram[0])
#define rPKE_SEQ           (*((volatile uint32_t *)(EHSM_UNIT_BASE + 1)))//PKE Operation Sequence Register
#define rPKE_ENDIAN_CTRL   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 2)))//PKE Control Register
#define rPKE_KEY_ADDR      (*((volatile uint32_t *)(EHSM_UNIT_BASE + 3)))//PKE Key Target Address Register
#define rPKE_TRNG_ADDR     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 4)))//PKE TRNG Target Address Register
#define rPKE_SOURCE_ADDR   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 5)))//PKE Private DMA Source Address Register
#define rPKE_TARGET_ADDR   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 6)))//PKE Private DMA Target Address Register
#define rPKE_TRNG_LENGTH   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 7)))//PKE TRNG Length Register
#define rPKE_MOVE_LENGTH   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 8)))//PKE Move Length Register
#define rPKE_TRNG_TRIG     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 9)))//PKE Transport TRNG Trigger Register
#define rPKE_MOVE_TRIG     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 10)))//PKE Transport TRNG Trigger Register
#define rPKE_CLC_TRIG      (*((volatile uint32_t *)(EHSM_UNIT_BASE + 11)))//PKE PKE Clear PKE RAM Registe
#define rPKE_FLAG          (*((volatile uint32_t *)(EHSM_UNIT_BASE + 12)))//PKE Flag Register
#else
#define EHSM_UNIT_BASE   (0x31C00000U)
#define rPKE_SEQ           (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0000)))//PKE Operation Sequence Register
#define rPKE_ENDIAN_CTRL   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0008)))//PKE Control Register
#define rPKE_KEY_ADDR      (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0010)))//PKE Key Target Address Register
#define rPKE_TRNG_ADDR     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0040)))//PKE TRNG Target Address Register
#define rPKE_SOURCE_ADDR   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0100)))//PKE Private DMA Source Address Register
#define rPKE_TARGET_ADDR   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0104)))//PKE Private DMA Target Address Register
#define rPKE_TRNG_LENGTH   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0110)))//PKE TRNG Length Register
#define rPKE_MOVE_LENGTH   (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0118)))//PKE Move Length Register
#define rPKE_TRNG_TRIG     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0200)))//PKE Transport TRNG Trigger Register
#define rPKE_MOVE_TRIG     (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0210)))//PKE Transport TRNG Trigger Register
#define rPKE_CLC_TRIG      (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0220)))//PKE PKE Clear PKE RAM Registe
#define rPKE_FLAG          (*((volatile uint32_t *)(EHSM_UNIT_BASE + 0xC0280)))//PKE Flag Register
#endif



// #define STANDALONE_IP_DEBUG

#ifdef CONFIG_UNIT_TEST
uint32_t g_unit_ram2[20*32];
#define PKE_RAM_SIGN_BUFFER_OFFSET  (&g_unit_ram2[0])//for hp ramA:0x400-0x0E00, ramB:0x1000-0x1A00
#else
#define PKE_RAM_SIGN_BUFFER_OFFSET  (0x1700U)//for hp ramA:0x400-0x0E00, ramB:0x1000-0x1A00
#endif

void set_key_addr(uint32_t *addr)
{
#ifdef STANDALONE_IP_DEBUG
#else
    rPKE_KEY_ADDR = (uint32_t)(uintptr_t)addr;
#endif
}

void set_key_endian(uint32_t idx)
{
#ifdef STANDALONE_IP_DEBUG
#else
    rPKE_ENDIAN_CTRL = idx;
#endif
}

void pke_dma_ram_move_start_and_wait_done(void)
{
#ifdef STANDALONE_IP_DEBUG
#else
    rPKE_MOVE_TRIG = 1U;
    while(0U != rPKE_MOVE_TRIG)
    {}
#endif
}

void pke_dma_trng_move_start_and_wait_done(void)
{
#ifdef STANDALONE_IP_DEBUG
#else
    rPKE_TRNG_TRIG = 1U;
    while(0U != rPKE_TRNG_TRIG)
    {}
#endif
}

void pke_dma_kmu_wait_done(void)
{
#ifdef STANDALONE_IP_DEBUG
#else
    while(0U == (rPKE_FLAG & 1U))
    {}
#endif
}

void pke_dma_clear(void)
{
#ifdef STANDALONE_IP_DEBUG
#else
    rPKE_CLC_TRIG = 1U;
    while(0U != rPKE_CLC_TRIG)
    {}
#endif
}

void pke_dma_ram_move(uint32_t *dst, uint32_t *src, uint32_t valid_wordlen, uint32_t total_wordlen)
{
#ifdef STANDALONE_IP_DEBUG
    valid_wordlen++;
    total_wordlen++;
   uint32_copy((uint32_t *)((uint32_t)dst+PKE_BASE_ADDR), (uint32_t *)((uint32_t)src+PKE_BASE_ADDR), total_wordlen);
   uint32_clear((uint32_t *)((uint32_t)dst+PKE_BASE_ADDR+valid_wordlen<<2), total_wordlen - valid_wordlen);
    // uint32_copy(dst, src, total_wordlen);
    // dst += valid_wordlen<<2;
    // uint32_clear(dst, total_wordlen - valid_wordlen);
#else
    rPKE_SOURCE_ADDR = (uint32_t)(uintptr_t)src;
    rPKE_TARGET_ADDR = (uint32_t)(uintptr_t)dst;
    rPKE_MOVE_LENGTH = ((total_wordlen<<8)|(valid_wordlen));
    pke_dma_ram_move_start_and_wait_done();
#endif
}

void pke_dma_trng_gen(uint32_t *src, uint32_t valid_wordlen, uint32_t total_wordlen)
{
#ifdef STANDALONE_IP_DEBUG
    uint8_t rand_k[32] = {0x44,0x26,0x0C,0x8D,0xB3,0xB3,0xDD,0x73,0xD9,0xB0,0x12,0x07,0x96,0x1B,0x18,0xCA,0xEE,0x1D,0xB7,0xDD,0x1C,0x59,0x77,0x41,0x9A,0x9D,0x68,0xF3,0x44,0x33,0x22,0x11};
    if(0x18C0 == (uint32_t)src)
    {
        memcpy_((void *)((uint32_t)src + PKE_BASE_ADDR), rand_k, 32);
    }
    else if(0x1900 == (uint32_t)src)
    {
        uint32_clear_8_words((uint32_t *)((uint32_t)src + PKE_BASE_ADDR));
        uint32_set((uint32_t *)((uint32_t)src + PKE_BASE_ADDR), 0x22222222, valid_wordlen+1);
    }
    else
    {
        uint32_clear_8_words((uint32_t *)((uint32_t)src + PKE_BASE_ADDR));
        uint32_set((uint32_t *)((uint32_t)src + PKE_BASE_ADDR), 0x11111111, valid_wordlen+1);
    }
#else
    rPKE_TRNG_ADDR = (uint32_t)(uintptr_t)src;
    rPKE_TRNG_LENGTH = ((total_wordlen<<8)|(valid_wordlen));
    pke_dma_trng_move_start_and_wait_done();
#endif
}


#ifdef STANDALONE_IP_DEBUG
static uint32_t kmu_config(uint16_t key_id, uint8_t in_port_sel)
{
    uint8_t dA_little[32] = {0x44,0x26,0x0C,0x8D,0xB3,0xB3,0xDD,0x73,0xD9,0xB0,0x12,0x07,0x96,0x1B,0x18,0xCA,0xEE,0x1D,0xB7,0xDD,0x1C,0x59,0x77,0x41,0x9A,0x9D,0x68,0xF3,0xA1,0x81,0xD4,0xB6};
    uint32_copy_8_words((uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + PKE_BASE_ADDR + 12*SM2_BYTE_LEN), (uint32_t *)dA_little);
}
#else
uint32_t kmu_config(uint16_t key_id, uint8_t in_port_sel); 
#endif

void pke_dma_ram_move_sm2(uint32_t *dst, uint32_t *src)
{
    pke_dma_ram_move(dst, src, SM2_WORD_LEN-1, SM2_WORD_LEN-1);
}

/* function: get PKE RAM A slot
 * parameters:
 *     index ---------------------- input, index of slot(0,1,2,...)
 *     step ----------------------- input, byte length of slot
 * return: uint32_t pointer to the slot
 * caution:
 *     1. 
 */
uint32_t *rPKE_A_OFFSET(uint32_t index, uint32_t step)
{
    return (uint32_t *)(uintptr_t)(0x0400U+(index*step));
}


/* function: get PKE RAM B slot
 * parameters:
 *     index ---------------------- input, index of slot(0,1,2,...)
 *     step ----------------------- input, byte length of slot
 * return: uint32_t pointer to the slot
 * caution:
 *     1. 
 */
uint32_t *rPKE_B_OFFSET(uint32_t index, uint32_t step)
{
    return (uint32_t *)(uintptr_t)(0x1000U+(index*step));
}

/* function: Generate SM2 Signature(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e_bn[8] -------------------- input, e value, 8 words, little-endian
 *     rand_k[8] ------------------ input, random number k, 32 bytes, big-endian
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     dA[8] ---------------------- input, private key, 8 words, little-endian
 *     r[8] ----------------------- output, Signature r, 8 words, little-endian
 *     s[8] ----------------------- output, Signature s, 8 words, little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. e_bn and dA can not be modified
 *     3. e_bn has the same length as n(order of the SM2 curve)
 *     4. dA must be in [1, n-2]
 *     5. if you do not have rand_k, please set the parameter to be NULL, k will be generated inside.
 */
static uint32_t sm2_sign_s_kmu_internal(eccp_sec_ctx_t *ctx, const uint32_t e_bn[8], uint8_t kmu_key_idx, uint32_t signature_r[8], uint32_t signature_s[8])
{
    uint32_t *eccp_p, *eccp_p_h, *eccp_Gx, *eccp_Gy, *eccp_a, *eccp_b, *eccp_n, *eccp_n_h;
    uint32_t *rand_k, *rand1, *rand2, *pz, *dA, *out, *tmp, *value_one, *r, *e;
    uint32_t *result = (uint32_t *)(0x19C0);//ramB last64B base address
    uint32_t ret;
    // uint32_t *debug_out = (uint32_t *)(0x1980);

    eccp_p             = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 0*SM2_BYTE_LEN);
    eccp_p_h           = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 1*SM2_BYTE_LEN);
    eccp_Gx            = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 2*SM2_BYTE_LEN);
    eccp_Gy            = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 3*SM2_BYTE_LEN);
    eccp_a             = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 4*SM2_BYTE_LEN);
    eccp_b             = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 5*SM2_BYTE_LEN);
    eccp_n             = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 6*SM2_BYTE_LEN);
    eccp_n_h           = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 7*SM2_BYTE_LEN);
    rand_k             = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 8*SM2_BYTE_LEN);
    rand1              = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 9*SM2_BYTE_LEN);
    rand2              = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 10*SM2_BYTE_LEN);
    pz                 = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 11*SM2_BYTE_LEN);
    dA                 = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 12*SM2_BYTE_LEN);
    r                  = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 13*SM2_BYTE_LEN);
    out                = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 14*SM2_BYTE_LEN);
    tmp                = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 15*SM2_BYTE_LEN);
    value_one          = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 16*SM2_BYTE_LEN);
    e                  = (uint32_t *)(PKE_RAM_SIGN_BUFFER_OFFSET + 17*SM2_BYTE_LEN);

    //prepare curves param
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_p+PKE_BASE_ADDR), ctx->curve->eccp_p);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_p_h+PKE_BASE_ADDR), ctx->curve->eccp_p_h);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_Gx+PKE_BASE_ADDR), ctx->curve->eccp_Gx);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_Gy+PKE_BASE_ADDR), ctx->curve->eccp_Gy);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_a+PKE_BASE_ADDR), ctx->curve->eccp_a);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_b+PKE_BASE_ADDR), ctx->curve->eccp_b);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_n+PKE_BASE_ADDR), ctx->curve->eccp_n);
    uint32_copy_8_words((uint32_t *)((uintptr_t)eccp_n_h+PKE_BASE_ADDR), ctx->curve->eccp_n_h);
    uint32_copy_8_words((uint32_t *)((uintptr_t)e+PKE_BASE_ADDR), e_bn);
    pke_set_operand_uint32_value((uint32_t *)((uintptr_t)pz+PKE_BASE_ADDR), SM2_WORD_LEN, 1u);
    uint32_clear_8_words((uint32_t *)((uintptr_t)value_one+PKE_BASE_ADDR));
    *((uint32_t *)((uintptr_t)value_one+PKE_BASE_ADDR)) = 0x01;

    //transmit privkey to pke ram from kmu
    set_key_addr(dA);
    set_key_endian(0);//u32 little endian
    (void)kmu_config(kmu_key_idx, 3);
    pke_dma_kmu_wait_done();

    //now cpu can't not read and write pke ram
    //step: set mgmr pre value of p
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(0u,SM2_BYTE_LEN)), eccp_p);
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(0u,SM2_BYTE_LEN)), eccp_p_h);
    pke_pre_calc_mont_N0();

    //get rand k, t1, t2
    pke_dma_trng_gen(rand_k, SM2_WORD_LEN-2, SM2_WORD_LEN-1);
    pke_dma_trng_gen(rand1, SM2_WORD_LEN-2, SM2_WORD_LEN-1);
    pke_dma_trng_gen(rand2, SM2_WORD_LEN-2, SM2_WORD_LEN-1);

    //[k](Gx,Gy), r=Px
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), eccp_Gx);      //B1 Px
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(2u,32u)), eccp_Gy);      //B2 Py
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(3u,32u)), pz);           //A3 Pz
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(3u,32u)), eccp_b);       //B3 b ---- corrupted after calculating
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(4u,32u)), eccp_a);       //B4 a---- if a is not 0, B4 is corrupted after calculating.
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(5u,32u)), eccp_n);       //B5 order---- corrupted after calculating
    pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(4u,32u)), rand_k);       //A4 k---- corrupted after calculating
    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMUL_SEC);
    pke_dma_ram_move_sm2(r, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 Qx, save r

    if(PKE_SUCCESS == ret)
    {
        //r=r+e mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(0u,32u)), eccp_n);//A0 modulus
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), e);          //A1 a--e
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), r);          //B1 b--r
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODADD);
        pke_dma_ram_move_sm2(r, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));      //A1 out--r
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //set mgmr pre value of n
        rPKE_CFG &= (~(0x0007FFFFU));
        rPKE_CFG |= 0x00020100u;
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(0u,32u)), eccp_n);   //A0 p--eccp_n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(0u,32u)), eccp_n_h); //B0 h--eccp_n_h
        pke_pre_calc_mont_N0();
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = (dA - t1) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), dA);            //A1 a--dA
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), rand1);         //B1 b--t1
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODSUB);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 --out
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = (out + 1) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), out);            //A1 --out
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), value_one);      //B1 --1
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODADD);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 --out
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //out = (out)*t2 mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), out);            //A1 a--out
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), rand2);          //B1 b--t2
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 --out
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //tmp = t1*t2 mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), rand1);          //A1 a--t1
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), rand2);          //B1 b--t2
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);
        pke_dma_ram_move_sm2(tmp, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 --tmp
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //out = (out+tmp) mod n = ((dA+1)*rand2) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), out);            //A1 --out
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), tmp);            //B1 --tmp
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODADD);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 --out
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //out = (out)^(-1) mod n = ((dA+1)*rand2)^(-1) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), out);            //B1 a --out
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODINV);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 out = ainv --(out)^(-1)
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //tmp = (k+r) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), rand_k);         //A1 a --k
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), r);              //B1 b --r
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODADD);
        pke_dma_ram_move_sm2(tmp, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 out--tmp
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        //tmp = tmp*(t2) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), tmp);            //A1 a--tmp
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), rand2);          //B1 b--t2
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);
        pke_dma_ram_move_sm2(tmp, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 out--tmp
    }
    else
    {}

    
    if(PKE_SUCCESS == ret)
    {
        //tmp = (out)*(tmp) mod n = (((dA+1)*t2)^(-1))*((k+r)*(t2)) mod n = ((dA+1)^(-1))*(k+r) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), out);            //A1 a--out
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), tmp);            //B1 b--tmp
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);
        pke_dma_ram_move_sm2(tmp, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));            //A1 out--tmp
    }
    else
    {}

    
    if(PKE_SUCCESS == ret)
    {
        //s = (tmp - r) mod n = (((dA+1)^(-1))*(k+r) - r) mod n = ((dA+1)^(-1))*(k+r-(dA+1)r) mod n = ((dA+1)^(-1))*(k-r*dA) mod n
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_A_OFFSET(1u,32u)), tmp);           //A1 a--tmp
        pke_dma_ram_move_sm2((uint32_t *)(rPKE_B_OFFSET(1u,32u)), r);             //B1 b--r
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODSUB);
        pke_dma_ram_move_sm2(out, (uint32_t *)(rPKE_A_OFFSET(1u,32u)));           //A1 out--s
    }
    else
    {}

    
    if(PKE_SUCCESS == ret)
    {
        //output r,s and clear pke ram
        pke_dma_ram_move_sm2(result, r);
        pke_dma_ram_move_sm2(&result[(32/4)], out);
        // pke_dma_ram_move_sm2(debug_out, dA);
    }
    else
    {}

    pke_dma_clear();

    if(PKE_SUCCESS == ret)
    {
        memcpy_(signature_r, (void *)(PKE_BASE_ADDR+(uintptr_t)result), 32);
        memcpy_(signature_s, (void *)(PKE_BASE_ADDR+(uintptr_t)result+32), 32);
        // print_buf_U8((void *)(PKE_BASE_ADDR+(uint32_t)debug_out), 32, "dA");
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     sp_key_idx ----------------- input, index of secure port key(privkey)
 *     signature[64] -------------- output, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution:
 */
uint32_t sm2_sign_s_kmu(const uint8_t E[32], uint8_t sp_key_idx, uint8_t signature[64])
{
    uint32_t buf[SM2_WORD_LEN*3u];
    uint32_t *e_bn = buf;
    uint32_t *r    = &buf[SM2_WORD_LEN];
    uint32_t *s    = &buf[SM2_WORD_LEN*2];

    uint32_t ret = SM2_ERROR_S;
    uint16_t eccp_curve_crc16 = 0;
    const eccp_curve_t *curve;
    eccp_sec_ctx_t ctx[1];
    volatile uint32_t count = SM2_SEC_SIGN_COUNTER;

    if((NULL == E) || (NULL == signature))
    {}
    else
    {
        (void)counter_add_one(&count);

        //init sm2 curve
        curve = eccp_curve_init(ctx, sm2_curve);
        if(NULL != curve)
        {
            (void)counter_add_one(&count);

            //check crc16 of sm2 paras
            if(0U == ecc_crc16_check(curve, eccp_curve_crc16))
            {
                (void)counter_add_one(&count);
                ret = PKE_SUCCESS;
            }
            else
            {}
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //get e
        u8big_to_u32little_256bits(E, e_bn);

        (void)counter_add_one(&count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);
        ret = sm2_sign_s_kmu_internal(ctx, e_bn, sp_key_idx, r, s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

#ifndef SUPPORT_STATIC_ANALYSIS
        if(0u == (((uint32_t)signature) & 3u))
        {
            u8big_to_u32little_256bits((uint8_t *)r, (uint32_t *)signature);
            u8big_to_u32little_256bits((uint8_t *)s, (uint32_t *)(&signature[SM2_BYTE_LEN]));
        }
        else
#endif
        {
            u32little_to_u8big_256bits(r, signature);
            u32little_to_u8big_256bits(s, &signature[SM2_BYTE_LEN]);
        }

        (void)counter_add_one(&count);

        //check crc16 of sm2 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            (void)counter_add_one(&count);
        }
    }
    else
    {}

    if((count != (SM2_SEC_SIGN_COUNTER + 0x09U)) || (PKE_SUCCESS != ret))
    {
        ret = SM2_ERROR_S;
        (void)get_rand_fast((uint8_t *)signature, SM2_BYTE_LEN<<1);
    }
    else
    {
        ret = SM2_SUCCESS_S;
    }

    eccp_curve_uninit(ctx);

    return ret;
}

#endif
