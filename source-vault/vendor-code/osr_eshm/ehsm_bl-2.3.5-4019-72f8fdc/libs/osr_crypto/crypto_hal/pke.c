
#include "pke.h"
#include "../crypto_include/trng/trng.h"
#include "../crypto_include/crypto_common/utility.h"
#ifdef PKE_SEC
#include "../crypto_include/crypto_common/utility_sec.h"
#endif
#include "../crypto_lib/lib_extension.h"


#ifdef SUPPORT_STATIC_ANALYSIS
#ifdef CONFIG_UNIT_TEST
volatile pke_reg_st * g_pke_reg = (pke_reg_st *)PKE_BASE_ADDR;
#else
volatile pke_reg_st * const g_pke_reg = (pke_reg_st *)PKE_BASE_ADDR;
#endif
#endif


#ifdef SUPPORT_C25519
extern const uint32_t ed25519_2_power_p_minus_1_div_4[8];   //just for static analysis.
extern const uint32_t ed25519_p_minus_5_div_8[8];

//these two are for ed25519 point decoding.
//2^((p-1)/4) mod p
const uint32_t ed25519_2_power_p_minus_1_div_4[8] = 
    {0x4A0EA0B0u,0xC4EE1B27u,0xAD2FE478u,0x2F431806u,0x3DFBD7A7u,0x2B4D0099u,0x4FC1DF0Bu,0x2B832480u};
//(p-5)/8 mod p
const uint32_t ed25519_p_minus_5_div_8[8] =
    {0xFFFFFFFDu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu,0x0FFFFFFFu};
#endif

#ifdef CONFIG_UNIT_TEST
    uint32_t PKE_RAM[8*1024/4];
#endif


/* function: get pke IP version
 * parameters: none
 * return: pke IP version
 * caution:
 */
uint32_t pke_get_version(void)
{
    return rPKE_VERSION;
}


/* function: get pke driver version
 * parameters: none
 * return: pke driver version(software version)
 * caution:
 */
uint32_t pke_get_driver_version(void)
{
    //the meaning of the version(for example, if the return value is 0x23080301)
    //the first 3 bytes:  23.08.03 ---- date
    //the last byte:      01       ---- first verion on the day
    return (((uint32_t)0x24U)<<24U) | (((uint32_t)0x06U)<<16U) | (((uint32_t)0x21U)<<8U) | 0x01U;
}


/* function: wait for pke ram ready
 * parameters: none
 * return: none
 * caution:
 *     1. before use pke ram, please call this firstly.
 */
uint32_t pke_init(void)
{
    MEM_VOLATILE uint32_t flag = 1u;

    while(0u == (rPKE_RAM_STATUS & flag))
    {}

    return PKE_SUCCESS;
}


/* function: clear finished and interrupt tag
 * parameters: none
 * return: none
 * caution:
 */
void pke_clear_interrupt(void)
{
    MEM_VOLATILE uint32_t mask = ~((uint32_t)1);

#if 1
    rPKE_RISR &= mask;      //write 0 to clear
#else
    MEM_VOLATILE uint32_t flag = 1u;

    if(rPKE_RISR & flag)
    {
        rPKE_RISR &= mask;    //write 0 to clear
    }
    else
    {}
#endif
}


/* function: enable pke interrupt
 * parameters: none
 * return: none
 * caution:
 */
void pke_enable_interrupt(void)
{
    MEM_VOLATILE uint32_t flag = (uint32_t)1;

    lib_register_unlock(g_pke_reg);
    rPKE_IMCR |= flag;
    lib_register_lock(g_pke_reg);
}


/* function: disable pke interrupt
 * parameters: none
 * return: none
 * caution:
 */
void pke_disable_interrupt(void)
{
    MEM_VOLATILE uint32_t mask = ~((uint32_t)1);

    lib_register_unlock(g_pke_reg);
    rPKE_IMCR &= mask;
    lib_register_lock(g_pke_reg);
}


/* function: set operand width
 * parameters:
 *     bitLen --------------------- input, bit length of operand
 * return: uint bytes of hardware operand.
 * caution: please make sure 0 < bitLen <= OPERAND_MAX_BIT_LEN
 */
uint32_t pke_set_operand_width(uint32_t bitLen)
{
    MEM_VOLATILE uint32_t mask = ~(0x07FFFFU);
    uint32_t cfg = 0U, len;
    uint32_t step_bytes = 0U;
#if 1
    const uint32_t buf[5] = {1u,2u,4u,8u,16u};
    uint32_t i;
#endif

    len = (bitLen+255U)>>8;

#if 1
    for(i=0u; i<5u; i++)
    {
        if(len <= buf[i])
        {
            cfg = i+2u;
            step_bytes = ((uint32_t)0x08u)<<cfg;
            break;
        }
        else
        {}
    }
#else
    if(1U == len)
    {
        cfg = 2U;
        step_bytes = 0x20U;
    }
    else if(2U == len)
    {
        cfg = 3U;
        step_bytes = 0x40U;
    }
    else if(len <= 4U)
    {
        cfg = 4U;
        step_bytes = 0x80U;
    }
    else if(len <= 8U)
    {
        cfg = 5U;
        step_bytes = 0x100U;
    }
    else if(len <= 16U)
    {
        cfg = 6U;
        step_bytes = 0x200U;
    }
    else
    {
        //nothing to do, just for static analysis.
    }
#endif

    cfg = (cfg<<16)|(len<<8);

    lib_register_unlock(g_pke_reg);
    rPKE_CFG &= mask;
    rPKE_CFG |= cfg;
    lib_register_lock(g_pke_reg);

#if 0
    printf("\r\n %u, rPKE_CFG = %08x", len, rPKE_CFG);
#endif

    return step_bytes;
}


/* function: get current operand byte length
 * parameters: none
 * return: current operand byte length
 * caution: none
 */
uint32_t pke_get_operand_bytes(void)
{
    uint32_t step_bytes;
    lib_register_unlock(g_pke_reg);

#if 1
    uint32_t t = ((rPKE_CFG)>>16) & 0x07U;

    if((t > 1U) && (t < 7U))
    {
        step_bytes = ((uint32_t)0x08U)<<t;
    }
    else
    {
        step_bytes = 0x20U; //default value
    }
#else
    switch(((rPKE_CFG)>>16) & 0x07U)
    {
        case 2U:
            step_bytes = 0x20U;
            break;

        case 3U:
            step_bytes = 0x40U;
            break;

        case 4U:
            step_bytes = 0x80U;
            break;

        case 5U:
            step_bytes = 0x100U;
            break;

        case 6U:
            step_bytes = 0x200U;
            break;

        default:
            step_bytes = 0x20U;
    }
#endif

    lib_register_lock(g_pke_reg);
    return step_bytes;
}


/* function: set operation micro code
 * parameters:
 *     addr ----------------------- input, specific micro code
 * return: none
 * caution:
 */
void pke_set_microcode(uint32_t addr)
{
    lib_register_unlock(g_pke_reg);
    rPKE_MC_PTR = addr;
    lib_register_lock(g_pke_reg);
}


/* function: start pke calc
 * parameters: none
 * return: none
 * caution:
 */
void pke_start(void)
{
    MEM_VOLATILE uint32_t flag = PKE_START_CALC;

    lib_register_unlock(g_pke_reg);
    rPKE_CTRL |= flag;
    lib_register_lock(g_pke_reg);
}


/* function: return calc return code
 * parameters: none
 * return 0(success), other(error)
 * caution:
 */
uint32_t pke_check_rt_code(void)
{
    MEM_VOLATILE uint32_t mask = 0x07u;

    return rPKE_RT_CODE & mask;
}


/* function: wait till done
 * parameters: none
 * return: none
 * caution:
 */
void pke_wait_till_done(void)
{
    MEM_VOLATILE uint32_t flag = 1u;

    while(0u == (rPKE_RISR & flag))
    {}
}


/* function: set operation micro code, start hardware, wait till done, and return code
 * parameters:
 *     micro_code ----------------- input, specific micro code
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 */
uint32_t pke_set_micro_code_start_wait_return_code(uint32_t micro_code)
{
    pke_set_microcode(micro_code);

    pke_clear_interrupt();

    pke_start();

    pke_wait_till_done();

    return pke_check_rt_code();
}


/* function: ainv = a^(-1) mod modulus
 * parameters:
 *     a -------------------------- input, integer a
 *     ainv ----------------------- output, ainv = a^(-1) mod modulus
 *     modWordLen ----------------- input, word length of modulus and ainv
 *     aWordLen ------------------- input, word length of a
 *     step_bytes ----------------- input, byte length of hardware operand
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 *     1. please set hardware operand width(call function pke_set_operand_width()) before 
 *        calling this fuinction.
 *     2. please make sure the modulus is set in A0 before calling this.
 *     3. please make sure aWordLen <= modWordLen <= OPERAND_MAX_WORD_LEN and a < modulus
 */
uint32_t pke_modinv_internal(const uint32_t *a, uint32_t *ainv, uint32_t modWordLen, 
        uint32_t aWordLen, uint32_t step_bytes)
{
    uint32_t step_words = step_bytes>>2;
    uint32_t ret;

    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), a, aWordLen);                     //B1 a
    if(step_words > aWordLen)
    {
        uint32_clear(&(rPKE_B(1u,step_bytes))[aWordLen], step_words-aWordLen);
    }
    else
    {}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODINV);
    if(PKE_SUCCESS == ret)
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), ainv, modWordLen);            //A1 ainv
    }
    else if(PKE_NO_MODINV != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), modWordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), modWordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), aWordLen<<2);
#endif
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    return ret;
}


/* function: ainv = a^(-1) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     ainv ----------------------- output, ainv = a^(-1) mod modulus
 *     modWordLen ----------------- input, word length of modulus and ainv
 *     aWordLen ------------------- input, word length of a
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 *     1. please make sure aWordLen <= modWordLen <= OPERAND_MAX_WORD_LEN and a < modulus
 */
uint32_t pke_modinv(const uint32_t *modulus, const uint32_t *a, uint32_t *ainv, 
        uint32_t modWordLen, uint32_t aWordLen)
{
    uint32_t step_bytes;

    (void)pke_load_modulus(modulus, get_valid_bits(modulus,modWordLen), &step_bytes);

    return pke_modinv_internal(a, ainv, modWordLen, aWordLen, step_bytes);
}


/* function: ainv = a^(-1) mod modulus
 * parameters:
 *     a -------------------------- input, integer a
 *     ainv ----------------------- output, ainv = a^(-1) mod modulus
 * return: PKE_SUCCESS(success), other(inverse not exists or error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. please make sure the modulus is set in A0 before calling this.
 *     3. all operands are of 256 bits for SM2, SM9, etc.
 */
uint32_t pke_modinv_256bits(const uint32_t *a, uint32_t *ainv)
{
    uint32_t ret;

    pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), a);                   //B1 a

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODINV);
    if(PKE_SUCCESS == ret)
    {
        pke_read_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), ainv);            //A1 ainv
    }
    else if(PKE_NO_MODINV != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)), 32u<<1);                //clean A0,A1
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,32u)), 32u);
#endif
    }
    else
    {
        //nothing to do, just for static analysis.
    }

    return ret;
}


/* function: out = (a+b) or (a-b) or (a*b) mod modulus
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, (a+b) or (a-b) or (a*b) mod modulus
 *     wordLen -------------------- input, word length of modulus, a, b and modulus
 *     step_bytes ----------------- input, byte length of hardware operand
 *     micro_code ----------------- input, could be MICROCODE_MODADD or MICROCODE_MODSUB
 *                                         or MICROCODE_MODMUL.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width(call function pke_set_operand_width()) before 
 *        calling this fuinction.
 *     2. please make sure the modulus is set in A0 before calling this.
 *     3. if micro_code is MICROCODE_MODMUL, please make sure the pre-calculated mont parameter 
 *        H(R^2 mod modulus) is set in B0, and call micro code MICROCODE_MGMR_PRE_N0 before calling
 *        this.
 *     4. actually micro_code could be MICROCODE_INTADD or MICROCODE_INTSUB, then
 *        out = (a+b) or (a-b) without modulus and pre-calculated mont parameters.
 */
uint32_t pke_mod_add_sub_mul_internal(const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t wordLen, uint32_t step_bytes, uint32_t micro_code)
{
    uint32_t step_words = step_bytes>>2;
    uint32_t ret;

    pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), a, wordLen);          //A1 a
    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), b, wordLen);          //B1 b

    if(step_words > wordLen)
    {
        uint32_clear(&(rPKE_A(1u,step_bytes))[wordLen], step_words-wordLen);
        uint32_clear(&(rPKE_B(1u,step_bytes))[wordLen], step_words-wordLen);
    }
    else
    {}

    ret = pke_set_micro_code_start_wait_return_code(micro_code);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        if((MICROCODE_MODMUL == micro_code) || (MICROCODE_MODADD == micro_code) ||  \
            (MICROCODE_MODSUB == micro_code))
        {
            (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), wordLen<<2);
        }
        else
        {}
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), wordLen<<2);
#endif
    }
    else
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, wordLen);    //A1 result
    }

    return ret;
}


/* function: out = (a+b) or (a-b) or (a*b) mod modulus
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, (a+b) or (a-b) or (a*b) mod modulus
 *     micro_code ----------------- input, could be MICROCODE_MODADD or MICROCODE_MODSUB
 *                                         or MICROCODE_MODMUL.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. please make sure the modulus is set in A0 before calling this.
 *     3. if micro_code is MICROCODE_MODMUL, please make sure the pre-calculated mont parameter 
 *        H(R^2 mod modulus) is set in B0, and call micro code MICROCODE_MGMR_PRE_N0 before calling
 *        this.
 *     4. actually micro_code could be MICROCODE_INTADD or MICROCODE_INTSUB, then
 *        out = (a+b) or (a-b) without modulus and pre-calculated mont parameters.
 *     5. all operands are of 256 bits for SM2, SM9, etc.
 */
uint32_t pke_mod_add_sub_mul_256bits_internal(const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t micro_code)
{
    uint32_t ret;

    pke_load_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), a);          //A1 a
    pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), b);          //B1 b

    ret = pke_set_micro_code_start_wait_return_code(micro_code);
    if(PKE_SUCCESS == ret)
    {
        pke_read_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), out);    //A1 out
    }
    else
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)), 32u<<1);       //clean A0,A1
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,32u)), 32u);
#endif
    }

    return ret;
}


/* function: out = (a+b) or (a-b) mod modulus
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, (a+b) or (a-b) mod modulus
 *     micro_code ----------------- input, must be MICROCODE_MODADD or MICROCODE_MODSUB
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. please make sure micro_code is MICROCODE_MODADD or MICROCODE_MODSUB.
 *     3. all operands are of 256 bits for SM2, SM9, etc.
 */
uint32_t pke_modadd_modsub_256bits(const uint32_t *modulus, const uint32_t *a, 
        const uint32_t *b, uint32_t *out, uint32_t micro_code)
{
    pke_load_operand_256bits((uint32_t *)(rPKE_A(0u,32u)), modulus);         //A0 modulus

    return pke_mod_add_sub_mul_256bits_internal(a, b, out, micro_code);
}


/* function: out = (a+b) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b mod modulus
 *     wordLen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a,b must be less than modulus
 *     2. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modadd(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
        uint32_t *out, uint32_t wordLen)
{
    uint32_t step_bytes;

    (void)pke_load_modulus(modulus, wordLen<<5, &step_bytes);
 
    return pke_mod_add_sub_mul_internal(a, b, out, wordLen, step_bytes, MICROCODE_MODADD);
}


/* function: out = (a-b) mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a-b mod modulus
 *     wordLen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a,b must be less than modulus
 *     2. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modsub(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
        uint32_t *out, uint32_t wordLen)
{
    uint32_t step_bytes;

    (void)pke_load_modulus(modulus, wordLen<<5, &step_bytes);
 
    return pke_mod_add_sub_mul_internal(a, b, out, wordLen, step_bytes, MICROCODE_MODSUB);
}


/* function: out = a+b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a+b
 *     wordLen -------------------- input, word length of a, b, out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a+b may overflow
 *     2. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_add(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen)
{
    uint32_t step_bytes = pke_set_operand_width(wordLen<<5);

    return pke_mod_add_sub_mul_internal(a, b, out, wordLen, step_bytes, MICROCODE_INTADD);
}


/* function: out = a-b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a-b
 *     wordLen -------------------- input, word length of a, b, out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure a > b
 *     2. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_sub(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen)
{
    uint32_t step_bytes = pke_set_operand_width(wordLen<<5);

    return pke_mod_add_sub_mul_internal(a, b, out, wordLen, step_bytes, MICROCODE_INTSUB);
}


/* function: out = a*b
 * parameters:
 *     a -------------------------- input, integer a
 *     a_wordLen ------------------ input, word length of a
 *     b -------------------------- input, integer b
 *     b_wordLen ------------------ input, word length of b
 *     out ------------------------ output, out = a*b
 *     out_wordLen----------------- input, word length of out
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure out buffer word length is bigger than (2*max_bit_len(a,b)+0x1F)>>5
 *     2. please make sure a_wordLen/b_wordLen is not bigger than OPERAND_MAX_WORD_LEN/2
 */
uint32_t pke_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t a_wordLen,
        uint32_t b_wordLen, uint32_t out_wordLen)
{
    uint32_t step_bytes, step_words;
    uint32_t ret;

#if 0
    step_bytes = pke_set_operand_width(out_wordLen<<5);    //for pke lp, pke uhp
#else
    step_bytes = pke_set_operand_width(GET_MAX_LEN(out_wordLen<<5,512u));  //for pke hp
#endif
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), a, a_wordLen);          //A1 a
    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), b, b_wordLen);          //B1 b

#ifdef SUPPORT_STATIC_ANALYSIS        //just for static analysis.
    if((step_words>a_wordLen) && (step_words>b_wordLen))
    {
#endif
        uint32_clear(&(rPKE_A(1u,step_bytes))[a_wordLen], step_words-a_wordLen);
        uint32_clear(&(rPKE_B(1u,step_bytes))[b_wordLen], step_words-b_wordLen);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_INTMUL);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), a_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), b_wordLen<<2);
#endif
    }
    else
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, out_wordLen);  //A1 result
    }

    return ret;
}


/* function: out = a*b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b
 *     ab_wordLen ----------------- input, word length of a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure out buffer word length is bigger than (2*max_bit_len(a,b)+0x1F)>>5
 *     2. please make sure ab_wordLen is not bigger than OPERAND_MAX_WORD_LEN/2
 */
#if 1
uint32_t pke_mul(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t ab_wordLen)
{
    uint32_t bitLen, tempLen;

    bitLen = get_valid_bits(a, ab_wordLen);
    tempLen = get_valid_bits(b, ab_wordLen);

    bitLen = GET_MAX_LEN(bitLen,tempLen);
    tempLen = GET_WORD_LEN(bitLen<<1);
    if(tempLen < (ab_wordLen<<1))
    {
        tempLen = (ab_wordLen<<1)-1u;
    }
    else
    {
        tempLen = (ab_wordLen<<1);
    }

    return pke_mul_internal(a, b, out, ab_wordLen, ab_wordLen, tempLen);
}
#else
uint32_t pke_mul(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t ab_wordLen)
{
    uint64_t UV;
    uint32_t i,j,*U,*V;
    uint32_t bitLen, tempLen;

    bitLen = get_valid_bits(a, ab_wordLen);
    tempLen = get_valid_bits(b, ab_wordLen);

    bitLen = GET_MAX_LEN(bitLen,tempLen);
    tempLen = GET_WORD_LEN(bitLen<<1);
    if(tempLen < (ab_wordLen<<1))
    {
        tempLen = (ab_wordLen<<1)-1u;
    }
    else
    {
        tempLen = (ab_wordLen<<1);
    }

    uint32_clear(out, tempLen);

    V = (uint32_t *)(&UV);
    U = V+1u;
    for(i=0u; i<ab_wordLen; i++)
    {
        *U = 0u;
        for(j=0u; j<ab_wordLen; j++)
        {
            UV = ((uint64_t)a[i])*b[j]+out[i+j]+(*U);
            out[i+j] = (*V);
        }
        out[i+j] = (*U);
    }

    return PKE_SUCCESS;
}
#endif


/* function: calc n0(- modulus ^(-1) mod 2^w) for modMul, and pointMul. etc.
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling, please make sure the modulus is set in rPKE_A(0)
 *     2. please make sure the modulus is odd, and word length of the modulus
 *        is not bigger than OPERAND_MAX_WORD_LEN
 *     3. the result is set in the internal register, no need to output.
 */
uint32_t pke_pre_calc_mont_N0(void)
{
    return pke_set_micro_code_start_wait_return_code(MICROCODE_MGMR_PRE_N0);
}


/* function: calc H(R^2 mod modulus) and n0'( - modulus ^(-1) mod 2^w ) for modMul,modExp, and pointMul. etc.
 *           here w is bit width of word, i,e. 32.
 * parameters:
 *     modulus -------------------- input, modulus
 *     bitLen --------------------- input, bit length of modulus
 *     H -------------------------- output, R^2 mod modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure word length of buffer H is equal to word length of modulus
 *     3. bitLen must not be bigger than OPERAND_MAX_BIT_LEN
 *     4. A1 will be corrupted after calling this function
 */
uint32_t pke_pre_calc_mont(const uint32_t *modulus, uint32_t bitLen, uint32_t *H)
{
    uint32_t step_bytes, step_words;
    uint32_t wordLen = GET_WORD_LEN(bitLen);
    uint32_t ret;

    step_bytes = pke_set_operand_width(bitLen);
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(0u,step_bytes)), modulus, wordLen);    //A0 modulus
    if(step_words > wordLen)
    {
        uint32_clear(&(rPKE_A(0u,step_bytes))[wordLen], step_words-wordLen);
        uint32_clear(&(rPKE_B(0u,step_bytes))[wordLen], step_words-wordLen);
    }
    else
    {}

    ret = pke_pre_calc_mont_N0();
    if(PKE_SUCCESS == ret)
    {
        if((256u == bitLen) || (512u == bitLen) || (1024u == bitLen) || (2048u == bitLen) || (4096u == bitLen))
        {
            ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MGMR_PRE_H_MM);
        }
        else
        {
            ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MGMR_PRE_H);
        }

        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), wordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(0u,step_bytes)), wordLen<<2);
#endif
        }
        else if(NULL != H)
        {
            pke_read_operand((uint32_t *)(rPKE_B(0u,step_bytes)), H, wordLen);                  //B0 result
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    return ret;
}


/* function: like function pke_pre_calc_mont(), but this one is without output here
 * parameters:
 *     modulus -------------------- input, modulus
 *     wordLen -------------------- input, word length of modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_pre_calc_mont_no_output(const uint32_t *modulus, uint32_t wordLen)
{
    return pke_pre_calc_mont(modulus, get_valid_bits(modulus, wordLen), NULL);
}


/* function: load modulus for hardware operation(mod_add and mod_sub, etc.)
 * parameters:
 *     modulus -------------------- input, modulus
 *     bitLen --------------------- input, bit length of modulus
 *     step_bytes ----------------- output, byte length of hardware operand
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is mainly for mod_add, mod_sub, and mod_inv, etc.
 *     2. bitLen must not be bigger than OPERAND_MAX_BIT_LEN
 */
uint32_t pke_load_modulus(const uint32_t *modulus, uint32_t bitLen, uint32_t *step_bytes)
{
    uint32_t step_words;
    uint32_t wordLen = GET_WORD_LEN(bitLen);

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if(NULL != step_bytes)
    {
#endif
        *step_bytes = pke_set_operand_width(bitLen);
        step_words = (*step_bytes)>>2;

        pke_load_operand((uint32_t *)(rPKE_A(0u,*step_bytes)), modulus, wordLen);          //A0 modulus
        if(step_words > wordLen)
        {
            uint32_clear(&(rPKE_A(0u,*step_bytes))[wordLen], step_words-wordLen);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return PKE_SUCCESS;
}


/* function: load modulus and pre-calculated mont parameters H(R^2 mod modulus) 
 *           and n0'(- modulus ^(-1) mod 2^w) for hardware operation
 * parameters:
 *     modulus -------------------- input, modulus
 *     modulus_h ------------------ input, R^2 mod modulus
 *     bitLen --------------------- input, bit length of modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. bitLen must not be bigger than OPERAND_MAX_BIT_LEN
 */
uint32_t pke_load_modulus_and_pre_monts(const uint32_t *modulus, const uint32_t *modulus_h, 
        uint32_t bitLen)
{
    uint32_t step_bytes, step_words;
    uint32_t wordLen = GET_WORD_LEN(bitLen);

    step_bytes = pke_set_operand_width(bitLen);
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(0u,step_bytes)), modulus, wordLen);          //A0 p
    pke_load_operand((uint32_t *)(rPKE_B(0u,step_bytes)), modulus_h, wordLen);        //B0 h
    if(step_words > wordLen)
    {
        uint32_clear(&(rPKE_A(0u,step_bytes))[wordLen], step_words-wordLen);
        uint32_clear(&(rPKE_B(0u,step_bytes))[wordLen], step_words-wordLen);
    }
    else
    {}

    //n0
    return pke_pre_calc_mont_N0();
}


/* function: load modulus and pre-calculated mont parameters H(R^2 mod modulus) of 256 bits
 *           and n0'(- modulus ^(-1) mod 2^w) for hardware operation
 * parameters:
 *     modulus -------------------- input, modulus
 *     modulus_h ------------------ input, R^2 mod modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. all operands are of 256 bits for SM2, SM9, etc.
 */
uint32_t pke_load_modulus_and_pre_monts_256bits(const uint32_t *modulus, const uint32_t *modulus_h)
{
#if 1
    MEM_VOLATILE uint32_t mask = ~(0x0007FFFFU);

    lib_register_unlock(g_pke_reg);
    rPKE_CFG &= mask;
    rPKE_CFG |= 0x00020100u;
    lib_register_lock(g_pke_reg);
#else
    pke_set_operand_width(256u);
#endif
    pke_load_operand_256bits((uint32_t *)(rPKE_A(0u,32u)), modulus);          //A0 p
    pke_load_operand_256bits((uint32_t *)(rPKE_B(0u,32u)), modulus_h);        //B0 h

    return pke_pre_calc_mont_N0();
}


/* function: set modulus and pre-calculated mont parameters H(R^2 mod modulus) and 
 *           n0'(- modulus ^(-1) mod 2^w) for hardware operation
 * parameters:
 *     modulus -------------------- input, modulus
 *     modulus_h ------------------ input, R^2 mod modulus
 *     bitLen --------------------- input, bit length of modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. bitLen must not be bigger than OPERAND_MAX_BIT_LEN
 */
uint32_t pke_set_modulus_and_pre_monts(const uint32_t *modulus, const uint32_t *modulus_h, 
        uint32_t bitLen)
{
    uint32_t ret;

    if(NULL == modulus_h)
    {
        ret = pke_pre_calc_mont(modulus, bitLen, NULL);
    }
    else
    {
        ret = pke_load_modulus_and_pre_monts(modulus, modulus_h, bitLen);
    }

    return ret;
}


/* function: out = a*b (mod modulus)
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b mod modulus
 *     wordLen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. a, b must be less than modulus
 *     3. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 *     4. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t pke_modmul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen)
{
    uint32_t step_bytes, step_words;
    uint32_t ret;
#if 0
    step_bytes = pke_set_operand_width(wordLen<<5);
#else
    step_bytes = pke_get_operand_bytes();
#endif
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), a, wordLen);                      //A1 a
    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), b, wordLen);                      //B1 b
    if(step_words > wordLen)
    {
        uint32_clear(&(rPKE_A(1u,step_bytes))[wordLen], step_words-wordLen);
        uint32_clear(&(rPKE_B(1u,step_bytes))[wordLen], step_words-wordLen);
    }
    else
    {}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), wordLen<<2);
#endif
    }
    else
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, wordLen);                //A1 out
    }

    return ret;
}


/* function: out = a*b mod modulus
 * parameters:
 *     modulus -------------------- input, modulus
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b
 *     out ------------------------ output, out = a*b mod modulus
 *     wordLen -------------------- input, word length of modulus, a, b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. a, b must be less than modulus
 *     3. wordLen must not be bigger than OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modmul(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t wordLen)
{
    uint32_t ret;

    ret = pke_pre_calc_mont(modulus, get_valid_bits(modulus, wordLen), NULL);
    if(PKE_SUCCESS == ret)
    {
        ret = pke_modmul_internal(a, b, out, wordLen);
    }
    else
    {}

    return ret;
}


/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 *     3. modulus must be odd
 *     4. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp_internal(const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen)
{
    uint32_t step_bytes, step_words;
    uint32_t ret;

#if 1
    step_bytes = pke_get_operand_bytes();
#else
    step_bytes = pke_set_operand_width(mod_wordLen<<5);
#endif
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), exponent, exp_wordLen);               //A2 exponent
    if(step_words > exp_wordLen)
    {
        uint32_clear(&(rPKE_A(2u,step_bytes))[exp_wordLen], step_words-exp_wordLen);
    }
    else
    {}

    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), base, mod_wordLen);                   //B1 base

    if(step_words > mod_wordLen)
    {
        uint32_clear(&(rPKE_B(1u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
    }
    else
    {}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), mod_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), mod_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), exp_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), mod_wordLen<<2);
#endif
    }
    else
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, mod_wordLen);                //A1 result
    }

    return ret;
}


/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen)
{
    uint32_t ret;

    ret = pke_pre_calc_mont(modulus, get_valid_bits(modulus, mod_wordLen), NULL);
    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_internal(exponent, base, out, mod_wordLen, exp_wordLen);
    }
    else
    {}

    return ret;
}


/* function: check input before mod exponent
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(input is valid, allow to calculate)
 *         PKE_FINISHED(mod exponent finished)
 *         other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp_check_input(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen)
{
    uint32_t ret;
    int32_t flag;

    //base should be in [0,modulus]
    flag = uint32_BigNumCmp(base, mod_wordLen, modulus, mod_wordLen);
    if(flag > 0)
    {
        ret = PKE_INVALID_INPUT;
    }
    else
    {
        //if base is 0 or n
        if((1u == uint32_BigNum_Check_Zero(base, mod_wordLen)) || (0 == flag))
        {
            if(1u == uint32_BigNum_Check_Zero(exponent, exp_wordLen))               //0^0 mod n
            {
                ret = PKE_INVALID_INPUT;
            }
            else                                                                    //if a is 0, e is not 0, the output is 0
            {
                uint32_clear(out, mod_wordLen);
                ret = PKE_FINISHED;
            }
        }
        else if(1u == uint32_BigNum_Check_Zero(exponent, exp_wordLen))              //base is in [1,modulus-1], e is 0, the output is 1
        {
            pke_set_operand_uint32_value(out, mod_wordLen, 1u);
            ret = PKE_FINISHED;
        }
        else
        {
            ret = PKE_SUCCESS;
        }
    }

    return ret;
}


/* function: mod exponent(for high level use, operands are all U8 big-endian big number), this could
 *     be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_bitLen ----------------- input, real bit length of modulus and base number
 *     exp_bitLen ----------------- input, real bit length of exponent
 *     calc_pre_monts ------------- input, if it is 0, no need to calculate the pre-calculated mont arguments
 *                                  of modulus, otherwise calculate.
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. this is for high level application or protocol to use RSA mod exponent directly. all
 *        operands of this API are U8 big-endian big number.
 *     2. modulus must be odd
 *     3. please make sure exp_bitLen <= mod_bitLen <= OPERAND_MAX_BIT_LEN
 */
uint32_t pke_modexp_U8(const uint8_t *modulus, const uint8_t *exponent, const uint8_t *base,
        uint8_t *out, uint32_t mod_bitLen, uint32_t exp_bitLen, uint32_t calc_pre_monts)
{
    uint32_t step_bytes, step_words;
    uint32_t mod_byteLen = GET_BYTE_LEN(mod_bitLen);
    uint32_t mod_wordLen = GET_WORD_LEN(mod_bitLen);
    uint32_t exp_byteLen = GET_BYTE_LEN(exp_bitLen);
    uint32_t exp_wordLen = GET_WORD_LEN(exp_bitLen);
    uint32_t ret;

    step_bytes = pke_set_operand_width(mod_bitLen);
    step_words = step_bytes>>2;

    pke_load_operand_U8((uint32_t *)(rPKE_A(0u,step_bytes)), modulus, mod_byteLen);               //A0 modulus
    if(step_words > mod_wordLen)
    {
        uint32_clear(&(rPKE_A(0u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
        uint32_clear(&(rPKE_B(1u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
    }
    else
    {}

    if(0u != calc_pre_monts)
    {
        ret = pke_pre_calc_mont((uint32_t *)(rPKE_A(0u,step_bytes)), mod_bitLen, NULL);
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        pke_load_operand_U8((uint32_t *)(rPKE_A(2u,step_bytes)), exponent, exp_byteLen);              //A2 exponent
        if(step_words > exp_wordLen)
        {
            uint32_clear(&(rPKE_A(2u,step_bytes))[exp_wordLen], step_words-exp_wordLen);
        }
        else
        {}

        pke_load_operand_U8((uint32_t *)(rPKE_B(1u,step_bytes)), base, mod_byteLen);                  //B1 base

        ret = pke_modexp_check_input(rPKE_A(0u,step_bytes), rPKE_A(2u,step_bytes), rPKE_B(1u,step_bytes), 
            rPKE_A(1u,step_bytes), mod_wordLen, exp_wordLen);
        if(PKE_FINISHED == ret)
        {
            pke_read_operand_U8((uint32_t *)(rPKE_A(1u,step_bytes)), out, mod_byteLen);                //A1 result
            ret = PKE_SUCCESS;
        }
        else if(PKE_SUCCESS == ret)
        {
            ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP);
            if(PKE_SUCCESS != ret)
            {
#ifdef PKE_SEC
                (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), mod_wordLen<<2);
                (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), mod_wordLen<<2);
                (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), exp_wordLen<<2);
                (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), mod_wordLen<<2);
#endif
            }
            else
            {
                pke_read_operand_U8((uint32_t *)(rPKE_A(1u,step_bytes)), out, mod_byteLen);                //A1 result
            }
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    return ret;
}


/* function: step 1 of pke_mod(), get high = a high part mod b
 * parameters:
 *     a -------------------------- input, integer a
 *     aWordLen ------------------- input, word length of integer
 *     b -------------------------- input, integer b, modulus
 *     bWordLen ------------------- input, word length of integer b and b_h
 *     bitLen --------------------- input, bit length of b mod 32
 *     high ----------------------- output, high = a high part mod b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. b must be odd, and please make sure bWordLen is real word length of b
 *     2. real bit length of a can not be bigger than 2*(real bit length of b), so aWordLen can 
 *        not be greater than 2*bWordLen
 *     3. pleae make sure aWordLen <= 2*OPERAND_MAX_WORD_LEN, bWordLen <= OPERAND_MAX_WORD_LEN
 */
static uint32_t pke_mod_step_1(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, 
        uint32_t bWordLen, uint32_t bitLen, uint32_t *high)
{
    uint32_t ret;
    uint32_t tmpLen;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL != a) && (NULL != high) && (aWordLen >= bWordLen) && (bWordLen > 0u))
    {
#endif
        //get high = a high part mod b
        if(0u != bitLen)
        {
            tmpLen = aWordLen-bWordLen+1u;
            uint32_copy(high, &a[bWordLen-1u], tmpLen);
            (void)Big_Div2n(high, tmpLen, bitLen);
            if(tmpLen < bWordLen)
            {
                uint32_clear(&high[tmpLen], bWordLen-tmpLen);
                ret = PKE_SUCCESS;
            }
            else if(uint32_BigNumCmp(high, bWordLen, b, bWordLen) < 0)
            {
                ret = PKE_SUCCESS;
            }
            else
            {
                ret = pke_sub(high, b, high, bWordLen);
            }
        }
        else
        {
            tmpLen = aWordLen - bWordLen;
            if(uint32_BigNumCmp(&a[bWordLen], tmpLen, b, bWordLen) >= 0)
            {
                ret = pke_sub(&a[bWordLen], b, high, bWordLen);
            }
            else
            {
                uint32_copy(high, &a[bWordLen], tmpLen);
                uint32_clear(&high[tmpLen], bWordLen-tmpLen);
                ret = PKE_SUCCESS;
            }
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {
        ret = PKE_ERROR;
    }
#endif

    return ret;
}


/* function: step 2 of pke_mod(), get high = high * 1000..000 mod b
 * parameters:
 *     b -------------------------- input, integer b, modulus
 *     b_h ------------------------ input, H parameter of b
 *     bWordLen ------------------- input, word length of integer b and b_h
 *     bBitLen -------------------- input, bit length of b
 *     bitLen --------------------- input, bit length of b mod 32
 *     t -------------------------- input, temporary buffer, at least bWrodLen words
 *     high ----------------------- input&output, high = high * 1000..000 mod b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please call pke_mod_step_1() before calling this function
 */
static uint32_t pke_mod_step_2(const uint32_t *b, const uint32_t *b_h, uint32_t bWordLen, 
        uint32_t bBitLen, uint32_t bitLen, uint32_t *t, uint32_t *high)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL != t) && (bWordLen > 0u))
    {
#endif
        //set the pre-calculated mont parameters
        ret = pke_set_modulus_and_pre_monts(b, b_h, bBitLen);
        if(PKE_SUCCESS == ret)
        {
            //get t = 1000...000 mod b
            uint32_clear(t, bWordLen);
            if(0u != bitLen)
            {
                t[bWordLen-1u] = ((uint32_t)1u)<<(bitLen);
            }
            else
            {}

            ret = pke_sub(t, b, t, bWordLen);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //get high = high * 1000..000 mod b
            ret = pke_modmul_internal(t, high, high, bWordLen);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {
        ret = PKE_ERROR;
    }
#endif

    return ret;
}


/* function: step 3 of pke_mod(), get out = a mod b
 * parameters:
 *     a -------------------------- input, integer a
 *     b -------------------------- input, integer b, modulus
 *     bWordLen ------------------- input, word length of integer b and b_h
 *     bitLen --------------------- input, bit length of b mod 32
 *     t -------------------------- input, temporary buffer, at least bWrodLen words
 *     high ----------------------- input, high = a high part * 1000..000 mod b
 *     out ------------------------ output, out = a mod b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please call pke_mod_step_2() before calling this function
 */
static uint32_t pke_mod_step_3(const uint32_t *a, const uint32_t *b, uint32_t bWordLen, 
        uint32_t bitLen, uint32_t *t, const uint32_t *high, uint32_t *out)
{
    const uint32_t *tt;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if(bWordLen > 0u)
    {
#endif
        //get tt = a low part mod b
        tt = t;
        if(0u != bitLen)
        {
            uint32_copy(t, a, bWordLen);
            t[bWordLen-1u] &= (((uint32_t)1u)<<bitLen)-1u;
            if(uint32_BigNumCmp(t, bWordLen, b, bWordLen) >= 0)
            {
                ret = pke_sub(t, b, t, bWordLen);
            }
            else
            {
                ret = PKE_SUCCESS;
            }
        }
        else
        {
            if(uint32_BigNumCmp(a, bWordLen, b, bWordLen) >= 0)
            {
                ret = pke_sub(a, b, t, bWordLen);
            }
            else
            {
                tt = a;
                ret = PKE_SUCCESS;
            }
        }

        if(PKE_SUCCESS == ret)
        {
            ret = pke_modadd(b, tt, high, out, bWordLen);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {
        ret = PKE_ERROR;
    }
#endif

    return ret;
}


/* function: c = a mod b
 * parameters:
 *     a -------------------------- input, integer a
 *     aWordLen ------------------- input, word length of integer
 *     b -------------------------- input, integer b, modulus
 *     b_h ------------------------ input, H parameter of b
 *     bWordLen ------------------- input, word length of integer b and b_h
 *     c -------------------------- output, c = a mod b
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. b must be odd, and please make sure bWordLen is real word length of b
 *     2. real bit length of a can not be bigger than 2*(real bit length of b), so aWordLen can 
 *        not be bigger than 2*bWordLen
 *     3. pleae make sure aWordLen <= 2*OPERAND_MAX_WORD_LEN, bWordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_mod(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, const uint32_t *b_h, 
        uint32_t bWordLen, uint32_t *c)
{
    uint32_t step_bytes;
    int32_t flag;
    uint32_t bBitLen, bitLen, tmpLen;
#if 0
    uint32_t t1[OPERAND_MAX_WORD_LEN], t2[OPERAND_MAX_WORD_LEN];
#else
    uint32_t *t1, *t2;
#endif
    uint32_t ret;

    flag = uint32_BigNumCmp(a, aWordLen, b, bWordLen);
    if(flag < 0)
    {
#ifdef SUPPORT_STATIC_ANALYSIS
        if(NULL == c)
        {
            ret = PKE_POINTER_NULL;
        }
        else
        {
#endif
            tmpLen = get_valid_words(a, aWordLen);
            uint32_copy(c, a, tmpLen);
            if(tmpLen < bWordLen)
            {
                uint32_clear(&c[tmpLen], bWordLen-tmpLen);
            }
            else
            {}

            ret = PKE_SUCCESS;
#ifdef SUPPORT_STATIC_ANALYSIS
        }
#endif
    }
    else if(0 == flag)
    {
        uint32_clear(c, bWordLen);
        ret = PKE_SUCCESS;
    }
    else
    {
        bBitLen = get_valid_bits(b, bWordLen);
        bitLen = bBitLen & 0x1Fu;
        step_bytes = pke_set_operand_width(bBitLen);
        t1 = (uint32_t *)(rPKE_A(1u, step_bytes));
        t2 = (uint32_t *)(rPKE_B(2u, step_bytes));

        ret = pke_mod_step_1(a, aWordLen, b, bWordLen, bitLen, t2);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_step_2(b, b_h, bWordLen, bBitLen, bitLen, t1, t2);
        }
        else
        {}
        
        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_step_3(a, b, bWordLen, bitLen, t1, t2, c);
        }
        else
        {}
    }

    return ret;
}


/********************************** ECCp functions *************************************/

/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. the output may be invalid(return PKE_NO_MODINV), even if input are all valid,
 *        it is suggested to call eccp_pointMul_Shamir_safe_internal()
 *     5. please set hardware operand width before calling this.
 *     6. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointMul_Shamir_internal(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen, nWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), P1x, pWordLen);                   //B1 P1x ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(2u,step_bytes)), P1y, pWordLen);                   //B2 P1y ---- corrupted after calculating
        pke_set_operand_uint32_value((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, 1u);      //A3 P1z ---- corrupted after calculating

        pke_load_operand((uint32_t *)(rPKE_B(5u,step_bytes)), P2x, pWordLen);                   //B5 P2x ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(6u,step_bytes)), P2y, pWordLen);                   //B6 P2y ---- corrupted after calculating

        pke_load_operand((uint32_t *)(rPKE_B(4u,step_bytes)), curve->eccp_a, pWordLen);         //B4 a   ---- if a is not 0, B4 is corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_A(4u,step_bytes)), k1, nWordLen);                    //A4 k1  ---- kept after calculating
        pke_load_operand((uint32_t *)(rPKE_A(5u,step_bytes)), k2, nWordLen);                    //A5 k2  ---- kept after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_B(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(5u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(6u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(4u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}

        if(step_words > nWordLen)
        {
            uint32_clear(&(rPKE_A(4u,step_bytes))[nWordLen], step_words-nWordLen);
            uint32_clear(&(rPKE_A(5u,step_bytes))[nWordLen], step_words-nWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_B(1u,step_bytes)), step_words, "B1");
print_BN_buf_U32((uint32_t *)(rPKE_B(2u,step_bytes)), step_words, "B2");
print_BN_buf_U32((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, "A3");
print_BN_buf_U32((uint32_t *)(rPKE_B(5u,step_bytes)), step_words, "B5");
print_BN_buf_U32((uint32_t *)(rPKE_B(6u,step_bytes)), step_words, "B6");
print_BN_buf_U32((uint32_t *)(rPKE_B(4u,step_bytes)), step_words, "B4");
print_BN_buf_U32((uint32_t *)(rPKE_A(4u,step_bytes)), step_words, "A4");
print_BN_buf_U32((uint32_t *)(rPKE_A(5u,step_bytes)), step_words, "A5");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMULF);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(2u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(5u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(6u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(4u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(4u,step_bytes)), nWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(5u,step_bytes)), nWordLen<<2);
#endif
        }
        else
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Qx, pWordLen);                //A1 Qx
            if(Qy != NULL)
            {
                pke_read_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Qy, pWordLen);            //A2 Qy
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. the output may be invalid(return PKE_NO_MODINV), even if input are all valid,
 *        it is suggested to call eccp_pointMul_Shamir_safe()
 */
uint32_t eccp_pointMul_Shamir(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_Shamir_internal(curve, k1, P1x, P1y, k2, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2), core operation
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 are both on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
FLAG_STATIC uint32_t eccp_core_pointMul_Shamir_safe_internal(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes;
    uint32_t x[ECCP_MAX_WORD_LEN], y[ECCP_MAX_WORD_LEN];
    uint32_t pWordLen, nWordLen;
    uint32_t is_k1_zero, is_k2_zero, is_finished = 0u, ret;

    ret = eccp_pointMul_Shamir_internal(curve, k1, P1x, P1y, k2, P2x, P2y, Qx, Qy);
    if(PKE_NO_MODINV == ret)
    {
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
#ifdef PKE_SEC
        is_k1_zero = uint32_BigNum_Check_Zero_sec(k1, nWordLen);
        is_k2_zero = uint32_BigNum_Check_Zero_sec(k2, nWordLen);
#else
        is_k1_zero = uint32_BigNum_Check_Zero(k1, nWordLen);
        is_k2_zero = uint32_BigNum_Check_Zero(k2, nWordLen);
#endif
        step_bytes = pke_get_operand_bytes();

        if(1u != is_k1_zero)  //k1 is not 0
        {
            ret = eccp_pointMul_internal(curve, k1, P1x, P1y, x, y);
            if(PKE_SUCCESS == ret)
            {
                if(1u == is_k2_zero)  //k1 is not 0, k2 is 0, return PKE_SUCCESS and output [k1]P1.
                {
                    uint32_copy(Qx, x, pWordLen);
                    uint32_copy(Qy, y, pWordLen);
                    is_finished = 1u;
                }
                else                  //k1 is not 0, k2 is not 0, keep.
                {}
            }
            else                      //k1 is not 0, [k1]P1 error, return error code.
            {
                is_finished = 1u;
            }
        }
        else                  //k1 is 0
        {
            if(1u == is_k2_zero)      //k1 is 0, k2 is 0, return PKE_NO_MODINV, the output point is neutral poinit(point at infinity)
            {
                is_finished = 1u;     //here ret is PKE_NO_MODINV
            }
            else                      //k1 is 0, k2 is not 0, keep.
            {}
        }

        if(0u == is_finished)   //now k2 is not 0
        {
            ret = eccp_pointMul_internal(curve, k2, P2x, P2y, (uint32_t *)(rPKE_A(1u,step_bytes)), 
                (uint32_t *)(rPKE_A(2u,step_bytes)));
            if(PKE_SUCCESS == ret)
            {
                if(1u == is_k1_zero)  //k1 is 0, k2 is not 0, return PKE_SUCCESS and output [k2]P2.
                {
                    uint32_copy(Qx, (uint32_t *)(rPKE_A(1u,step_bytes)), pWordLen);
                    uint32_copy(Qy, (uint32_t *)(rPKE_A(2u,step_bytes)), pWordLen);
                    is_finished = 1u;
                }
                else                  //k1 is not 0, k2 is not 0, keep. 
                {}
            }
            else                      //k2 is not 0, [k2]P2 error, return error code.
            {
                is_finished = 1u;
            }
        }
        else
        {}

        if(0u == is_finished)   //now k1, k2 are both not 0
        {
            ret = eccp_pointAdd_safe_internal(curve, (uint32_t *)(rPKE_A(1u,step_bytes)), 
                (uint32_t *)(rPKE_A(2u,step_bytes)), x, y, Qx, Qy);
        }
        else
        {}

#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)x, pWordLen<<2);
        (void)get_rand_fast((uint8_t *)y, pWordLen<<2);
#endif
    }
    else
    {}

    return ret;
}


/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 are both on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointMul_Shamir_safe_internal(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        ret = eccp_core_pointMul_Shamir_safe_internal(curve, k1, P1x, P1y, k2, 
                P2x, P2y, Qx, Qy);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve shamir point mul(Q = [k1]P1 + [k2]P2)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k1 ------------------------- input, scalar k1
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     k2 ------------------------- input, scalar k2
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k1,k2 in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P1,P2 are both on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_Shamir_safe(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_Shamir_safe_internal(curve, k1, P1x, P1y, k2, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point mul(Q = [k]G, here G is the curve base point)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k  ------------------------- input, scalar k1
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. the input point is base point, and please make sure curve->eccp_half_Gx
 *        and curve->eccp_half_Gy both are not NULL!
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_base(const eccp_curve_st *curve, const uint32_t *k, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes;
    uint32_t nWordLen, tmpBitLen, tmpWordLen, tmp_words;
#if 0
    k1[ECCP_MAX_WORD_LEN], k2[ECCP_MAX_WORD_LEN];
#else
    uint32_t *k1, *k2;
#endif
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == curve) || (NULL == k))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        step_bytes = pke_set_operand_width(curve->eccp_p_bitLen);
        k1 = (uint32_t *)(rPKE_A(4u,step_bytes));
        k2 = (uint32_t *)(rPKE_A(5u,step_bytes));

        //k2: low half part
        tmpBitLen = (curve->eccp_n_bitLen)>>1;
        tmpWordLen = GET_WORD_LEN(tmpBitLen);
        uint32_copy(k2, k, tmpWordLen);
        uint32_clear(&k2[tmpWordLen], nWordLen-tmpWordLen);
        tmpBitLen = tmpBitLen & 0x1Fu;
        if(0u != tmpBitLen)
        {
            k2[tmpWordLen-1u] &= (((uint32_t)1u)<<tmpBitLen)-1u;
        }
        else
        {}

        //k1: high half part
        if(0u != tmpBitLen)
        {
#ifdef SUPPORT_STATIC_ANALYSIS           //just for static analysis.
            if(nWordLen >= tmpWordLen)
            {
#endif
                tmp_words = nWordLen-tmpWordLen+1u;
                uint32_copy(k1, &k[tmpWordLen-1u], tmp_words);
                uint32_clear(&k1[tmp_words], tmpWordLen-1u);
                (void)Big_Div2n(k1, tmp_words, tmpBitLen);
#ifdef SUPPORT_STATIC_ANALYSIS
            }
            else
            {}
#endif            
        }
        else
        {
            uint32_copy(k1, &k[tmpWordLen], nWordLen-tmpWordLen);
            uint32_clear(&k1[nWordLen-tmpWordLen], tmpWordLen);
        }
        tmpBitLen = curve->eccp_n_bitLen - ((curve->eccp_n_bitLen)>>1);
        tmpWordLen = GET_WORD_LEN(tmpBitLen);
        tmpBitLen = tmpBitLen & 0x1Fu;
        if(0u != tmpBitLen)
        {
            k1[tmpWordLen-1u] &= (((uint32_t)1u)<<tmpBitLen)-1u;
        }
        else
        {}

        ret = eccp_pointMul_Shamir(curve,
            k1, curve->eccp_half_Gx, curve->eccp_half_Gy,
            k2, curve->eccp_Gx, curve->eccp_Gy,
            Qx, Qy);
        if(PKE_NO_MODINV == ret)
        {
            ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. even if the input point P is valid, the output may be infinite point, in this case
 *        it will return error.
 *     5. please set hardware operand width before calling this.
 *     6. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointMul_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen, nWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), Px, pWordLen);                     //B1 Px ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(2u,step_bytes)), Py, pWordLen);                     //B2 Py ---- corrupted after calculating
        pke_set_operand_uint32_value((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, 1u);       //A3 Pz ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(4u,step_bytes)), curve->eccp_a, pWordLen);          //B4 a  ---- if a is not 0, B4 is corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_A(4u,step_bytes)), k, nWordLen);                      //A4 k  ---- kept after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_B(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(4u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}

        if(step_words > nWordLen)
        {
            uint32_clear(&(rPKE_A(4u,step_bytes))[nWordLen], step_words-nWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_B(1u,step_bytes)), step_words, "B1");
print_BN_buf_U32((uint32_t *)(rPKE_B(2u,step_bytes)), step_words, "B2");
print_BN_buf_U32((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, "A3");
print_BN_buf_U32((uint32_t *)(rPKE_B(4u,step_bytes)), step_words, "B4");
print_BN_buf_U32((uint32_t *)(rPKE_A(4u,step_bytes)), step_words, "A4");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMUL);
        if(PKE_SUCCESS == ret)
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Qx, pWordLen);                 //A1 Qx
            if(NULL != Qy)
            {
                pke_read_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Qy, pWordLen);             //A2 Qy
            }
            else
            {}
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. even if the input point P is valid, the output may be infinite point, in this case
 *        it will return error.
 */
uint32_t eccp_pointMul(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_internal(curve, k, Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. please make sure bit length of the curve is not greater than ECCP_MAX_BIT_LEN
 *     3. even if the input point P1 and P2 are valid, it will return error in the following 2 cases.
 *        (1). P1 = P2. return PKE_NO_MODINV.
 *        (2). P1 = -P2. return PKE_NO_MODINV. actually the output point is neutral point(point at infinity)
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointAdd_internal(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        //pke_pre_calc_mont() may cover A1, so load A1(P1x) here
        pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), P1x, pWordLen);                    //A1 P1x ---- covered by output point
        pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), P1y, pWordLen);                    //A2 P1y ---- covered by output point
        pke_set_operand_uint32_value((uint32_t *)(rPKE_B(3u,step_bytes)), step_words, 1u);       //B3 P1z ---- corrupted after calculating

        pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), P2x, pWordLen);                    //B1 P2x ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(2u,step_bytes)), P2y, pWordLen);                    //B2 P2y ---- corrupted after calculating
        pke_set_operand_uint32_value((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, 1u);       //A3 P2z ---- corrupted after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_A(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_A(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(2u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_A(1u,step_bytes)), step_words, "A1");
print_BN_buf_U32((uint32_t *)(rPKE_A(2u,step_bytes)), step_words, "A2");
print_BN_buf_U32((uint32_t *)(rPKE_B(3u,step_bytes)), step_words, "B3");
print_BN_buf_U32((uint32_t *)(rPKE_B(1u,step_bytes)), step_words, "B1");
print_BN_buf_U32((uint32_t *)(rPKE_B(2u,step_bytes)), step_words, "B2");
print_BN_buf_U32((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, "A3");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PADD);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(2u,step_bytes)), pWordLen<<2);
#endif
        }
        else
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Qx, pWordLen);                 //A1 Qx
            if(NULL != Qy)
            {
                pke_read_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Qy, pWordLen);             //A2 Qy
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. please make sure bit length of the curve is not greater than ECCP_MAX_BIT_LEN
 *     3. even if the input point P1 and P2 are valid, it will return error in the following 2 cases.
 *        (1). P1 = P2. return PKE_NO_MODINV.
 *        (2). P1 = -P2. return PKE_NO_MODINV. actually the output point is neutral point(point at infinity)
 */
uint32_t eccp_pointAdd(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointAdd_internal(curve, P1x, P1y, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. please make sure bit length of the curve is not greater than ECCP_MAX_BIT_LEN
 *     3. if P1 = -P2, it will return PKE_NO_MODINV. actually the output point is neutral point(point at infinity)
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointAdd_safe_internal(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
#ifndef ECCP_POINT_DOUBLE
    uint32_t step_bytes, nWordLen;
#endif
    uint32_t pWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
#ifndef ECCP_POINT_DOUBLE
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        step_bytes = pke_get_operand_bytes();
#endif

#ifdef PKE_SEC
        if(0 == uint32_BigNumCmp_sec(P1x, pWordLen, P2x, pWordLen))
        {
            if(0 == uint32_BigNumCmp_sec(P1y, pWordLen, P2y, pWordLen))
#else
        if(0 == uint32_BigNumCmp(P1x, pWordLen, P2x, pWordLen))
        {
            if(0 == uint32_BigNumCmp(P1y, pWordLen, P2y, pWordLen))
#endif
            {
#ifdef ECCP_POINT_DOUBLE
                ret = eccp_pointDouble_internal(curve, P1x, P1y, Qx, Qy);
#else
                pke_set_operand_uint32_value((uint32_t *)(rPKE_A(4u,step_bytes)), nWordLen, 2u);
                ret = eccp_pointMul_internal(curve, (uint32_t *)(rPKE_A(4u,step_bytes)), P1x, P1y, Qx, Qy);
#endif
            }
            else
            {
                ret = PKE_NO_MODINV;
            }
        }
        else
        {
            ret = eccp_pointAdd_internal(curve, P1x, P1y, P2x, P2y, Qx, Qy);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. please make sure bit length of the curve is not greater than ECCP_MAX_BIT_LEN
 *     3. if P1 = -P2, it will return PKE_NO_MODINV. actually the output point is neutral point(point at infinity)
 */
uint32_t eccp_pointAdd_safe(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointAdd_safe_internal(curve, P1x, P1y, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


#ifdef ECCP_POINT_DOUBLE
/* function: ECCP curve point double, Q=[2]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q=[2]P
 *     Qy ------------------------- output, y coordinate of point Q=[2]P
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     3. please set hardware operand width before calling this.
 *     4. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointDouble_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        //pke_pre_calc_mont() may cover A1, so load A1(Px) and other paras here
        pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Px, pWordLen);                     //A1 Px ---- covered by output point
        pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Py, pWordLen);                     //A2 Py ---- covered by output point
        pke_set_operand_uint32_value((uint32_t *)(rPKE_B(3u,step_bytes)), step_words, 1u);       //B3 Pz ---- corrupted after calculating

        pke_load_operand((uint32_t *)(rPKE_B(4u,step_bytes)), curve->eccp_a, pWordLen);          //B4 a  ---- if a is not 0, B4 is corrupted after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_A(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_A(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(4u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_A(1u,step_bytes)), step_words, "A1");
print_BN_buf_U32((uint32_t *)(rPKE_A(2u,step_bytes)), step_words, "A2");
print_BN_buf_U32((uint32_t *)(rPKE_B(3u,step_bytes)), step_words, "B3");
print_BN_buf_U32((uint32_t *)(rPKE_B(4u,step_bytes)), step_words, "B4");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PDBL);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(4u,step_bytes)), pWordLen<<2);
#endif
        }
        else
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Qx, pWordLen);                 //A1 Qx
            pke_read_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Qy, pWordLen);                 //A2 Qy
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve point double, Q=[2]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q=[2]P
 *     Qy ------------------------- output, y coordinate of point Q=[2]P
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointDouble(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointDouble_internal(curve, Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}
#endif


/* function: check whether the input point P is on ECCP curve or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     2. after calculation, A1 and A2 will be changed!
 *     3. please set hardware operand width before calling this.
 *     4. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointVerify_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        //pke_pre_calc_mont() may cover A1, so load A1(Px) and other paras here
        pke_load_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Px, pWordLen);                    //A1 Px ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Py, pWordLen);                    //A2 Py ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(4u,step_bytes)), curve->eccp_a, pWordLen);         //B4 a  ---- if a is not 0, B4 is corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_A(4u,step_bytes)), curve->eccp_b, pWordLen);         //A4 b  ---- kept after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_A(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_A(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(4u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_A(4u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_A(1u,step_bytes)), step_words, "A1");
print_BN_buf_U32((uint32_t *)(rPKE_A(2u,step_bytes)), step_words, "A2");
print_BN_buf_U32((uint32_t *)(rPKE_B(4u,step_bytes)), step_words, "B4");
print_BN_buf_U32((uint32_t *)(rPKE_A(4u,step_bytes)), step_words, "A4");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PVER);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(4u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(4u,step_bytes)), pWordLen<<2);
#endif
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: check whether the input point P is on ECCP curve or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     2. after calculation, A1 and A2 will be changed!
 */
uint32_t eccp_pointVerify(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointVerify_internal(curve, Px, Py);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: check whether the input point P is valid or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, valid), other(error or invalid)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     2. after calculation, A1 and A2 will be changed!
 *     3. please set hardware operand width before calling this.
 *     4. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_check_point_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t ret;
    uint32_t pWordLen = 0u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        if(uint32_BigNumCmp(Px, pWordLen, curve->eccp_p, pWordLen) >= 0)
        {
            ret = PKE_INTEGER_TOO_BIG;
        }
        else if(uint32_BigNumCmp(Py, pWordLen, curve->eccp_p, pWordLen) >= 0)
        {
            ret = PKE_INTEGER_TOO_BIG;
        }
        else
        {
            ret = eccp_pointVerify_internal(curve, Px, Py);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: check whether the input point P is valid or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, valid), other(error or invalid)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     2. after calculation, A1 and A2 will be changed!
 */
uint32_t eccp_check_point(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_check_point_internal(curve, Px, Py);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: get ECCP public key from private key(the key pair could be used in SM2/ECDSA/ECDH, etc.)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     priKey --------------------- input, private key, big-endian
 *     pubKey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_get_pubkey_from_prikey(const eccp_curve_st *curve, const uint8_t *priKey, uint8_t *pubKey)
{
    uint32_t ret, step_bytes;
    uint32_t nByteLen, nWordLen, pByteLen;
    uint32_t k[ECCP_MAX_WORD_LEN];
    uint32_t *x, *y;

    if((NULL == curve) || (NULL == priKey) || (NULL == pubKey))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);

        step_bytes = pke_set_operand_width(curve->eccp_p_bitLen);
        x = (uint32_t *)(rPKE_A(1u,step_bytes));
        y = (uint32_t *)(rPKE_A(2u,step_bytes));

        k[nWordLen - 1u] = 0u;   //clear if curve->eccp_n_bitLen is not a multiple of 32
        reverse_byte_array(priKey, (uint8_t *)k, nByteLen);

#ifdef SUPPORT_SM2
        if(curve == sm2_curve)
        {
            //make sure k in [1, n-2]
            ret = uint32_integer_check(k, g_sm2p256v1_n_minus_1, nWordLen, PKE_ZERO_ALL, 
                    PKE_INTEGER_TOO_BIG, PKE_SUCCESS);
        }
        else
#endif
        {
            //make sure k in [1, n-1]
            ret = uint32_integer_check(k, curve->eccp_n, nWordLen, PKE_ZERO_ALL, 
                    PKE_INTEGER_TOO_BIG, PKE_SUCCESS);
        }

        //get pubKey
        if(PKE_SUCCESS == ret)
        {
            if((NULL != curve->eccp_half_Gx) && (NULL != curve->eccp_half_Gy))
            {
                ret = eccp_pointMul_base(curve, k, x, y);
            }
            else
            {
                ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, x, y);
            }

            if(PKE_SUCCESS == ret)
            {
                reverse_byte_array((uint8_t *)x, pubKey, pByteLen);
                reverse_byte_array((uint8_t *)y, &pubKey[pByteLen], pByteLen);
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* function: get ECCP key pair(the key pair could be used in SM2/ECDSA/ECDH)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     priKey --------------------- output, private key, big-endian
 *     pubKey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_getkey(const eccp_curve_st *curve, uint8_t *priKey, uint8_t *pubKey)
{
    uint32_t tmpLen;
    uint32_t nByteLen;
    uint32_t ret;

    if((NULL == curve) || (NULL == priKey) || (NULL == pubKey))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
        do {
            ret = get_rand(priKey, nByteLen);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            //make sure k has the same bit length as n
            tmpLen = (curve->eccp_n_bitLen)&7u;
            if(0u != tmpLen)
            {
                priKey[0] &= (1u<<(tmpLen))-1u;
            }
            else
            {}

            ret = eccp_get_pubkey_from_prikey(curve, priKey, pubKey);
        } while((PKE_ZERO_ALL == ret) || (PKE_INTEGER_TOO_BIG == ret));
    }

    return ret;
}

/****************************** ECCp functions finished ********************************/


#ifdef SUPPORT_C25519
/**************************** X25519 & Ed25519 functions *******************************/

/* function: c25519 point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, c25519 curve struct pointer
 *     k -------------------------- input, scalar
 *     Pu ------------------------- input, u coordinate of point P
 *     Qu ------------------------- output, u coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be infinite point, in this case return error.
 *     3. please make sure the curve is c25519
 */
uint32_t x25519_pointMul(const mont_curve_st *curve, const uint32_t *k, const uint32_t *Pu, uint32_t *Qu)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_load_modulus_and_pre_monts_256bits(curve->p, curve->p_h);
        if(PKE_SUCCESS == ret)
        {
            pke_load_operand_256bits((uint32_t *)rPKE_A(1u,32u), Pu);                     //A1 Pu
            pke_load_operand_256bits((uint32_t *)rPKE_A(2u,32u), curve->a24);             //A2 a24
            pke_load_operand_256bits((uint32_t *)rPKE_A(4u,32u), k);                      //A4 k

            ret = pke_set_micro_code_start_wait_return_code(MICROCODE_C25519_PMUL);
            if(PKE_SUCCESS != ret)
            {
#ifdef PKE_SEC
                (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)), 32u*3u);                 //clean A0,A1,A2
                (void)get_rand_fast((uint8_t *)(rPKE_A(4u,32u)), 32u);
#endif
            }
            else
            {
                pke_read_operand_256bits((uint32_t *)rPKE_A(1u,32u), Qu);                 //A1 Qu
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* Function: decode X25519 or Ed25519 scalar for point multiplication
 * Parameters:
 *     k -------------------------- input,
 *     out ------------------------ output, big scalar in little-endian
 * Return: none
 * Caution:
 *     1. this function is for X25519 or Ed25519.
 */
void x25519_ed25519_decode_scalar(const uint8_t *k, uint8_t *out)
{
    if(k != out)
    {
        memcpy_(out, k, C25519_BYTE_LEN);
    }
    else
    {}

    //actually this is internal interface,the caller ensures out is not NULL 
#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != out)
    {
#endif
        out[0] &= (uint8_t)0xF8;                       //clear lowest 3 bits
        out[C25519_BYTE_LEN - 1u] &= (uint8_t)0x7F;    //clear highest 1 bit
        out[C25519_BYTE_LEN - 1u] |= (uint8_t)0x40;    //set second highest bit as 1
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: mod exponent, this is for ed25519 point decoding.
 * parameters:
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 *     3. modulus must be odd
 *     4. all operands are of 256 bits for ed25519, etc.
 */
uint32_t pke_modexp_256bits_internal(const uint32_t *exponent, const uint32_t *base, uint32_t *out)
{
    uint32_t ret;

    pke_load_operand_256bits((uint32_t *)(rPKE_A(2u,32u)), exponent);               //A2 exponent
    pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), base);                   //B1 base

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)),32u*3u);                    //clean A0,A1,A2
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,32u)),32u);
#endif
    }
    else
    {
        pke_read_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), out);                //A1 result
    }

    return ret;
}


/* function: Ed25519 decode point step 1
 * parameters:
 *     y -------------------------- input, y coordinate, 8 words
 *     t -------------------------- input, temporary buffer, 8 words
 *     u -------------------------- output, u = y^2 - 1
 *     v -------------------------- output, v = d*y^2 + 1
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
static uint32_t ed25519_decode_point_internal_step_1(const uint32_t *y, uint32_t *t, uint32_t *u, 
        uint32_t *v)
{
    uint32_t ret;

    ret = pke_mod_add_sub_mul_256bits_internal(y, y, u, MICROCODE_MODMUL);               //u = y^2
    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(ed25519->d, u, v, MICROCODE_MODMUL);  //v = d*y^2
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != u[0])
        {
            u[0] -= 1u;                                                                  //u = y^2 - 1
        }
        else
        {
            pke_set_operand_uint32_value(t, Ed25519_WORD_LEN, 1u);           
            ret = pke_mod_add_sub_mul_256bits_internal(u, t, u, MICROCODE_MODSUB);       //u = y^2 - 1
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#ifdef SUPPORT_STATIC_ANALYSIS
        if(v[0] != (~0u))    //actually v < n, and n is odd, so this is redundant, just for static analysis.
        {
#endif
            //v = d*y^2 + 1, v may be equal to p, but p*p mod p is 0 by hardware, this does not affect the following.
            //so no need to execute ret = pke_mod_add_sub_mul_256bits_internal(v, t, v, MICROCODE_MODADD).
            v[0] += 1u;                                                                  //v = d*y^2 + 1
#ifdef SUPPORT_STATIC_ANALYSIS
        }
        else
        {}
#endif 
    }
    else
    {}

    return ret;
}


/* function: Ed25519 decode point step 2
 * parameters:
 *     u -------------------------- input, u = y^2 - 1, 8 words
 *     v -------------------------- input, v = d*y^2 + 1, 8 words
 *     out1 ----------------------- output, out1 = u*v^3, 8 words
 *     out2 ----------------------- output, out2 = u*v^7, 8 words
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please call ed25519_decode_point_internal_step_1() before calling this function
 */
static uint32_t ed25519_decode_point_internal_step_2(const uint32_t *u, const uint32_t *v, 
        uint32_t *out1, uint32_t *out2)
{
    uint32_t ret;

#if 0
    ret = pke_mod_add_sub_mul_256bits_internal(v, v, out1, MICROCODE_MODMUL);             //out1 = v^2
    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(v, out1, out2, MICROCODE_MODMUL);      //out2 = v^3
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(out2, u, out1, MICROCODE_MODMUL);      //out1 = u*v^3
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(v, out2, out2, MICROCODE_MODMUL);      //out2 = v^4
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(out2, out1, out2, MICROCODE_MODMUL);   //out2 = u*v^7
    }
    else
    {}
#else
    pke_load_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), v);              //A1 v
    pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), v);              //B1 v
    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);      //A1 = v^2
    if(PKE_SUCCESS == ret)
    {
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);  //A1 = v^3
        pke_load_operand_256bits(out2, (uint32_t *)(rPKE_A(1u,32u)));       //out2 = v^3
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), u);          //B1 u
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);  //A1 = u*v^3 
        pke_load_operand_256bits(out1, (uint32_t *)(rPKE_A(1u,32u)));       //out1 = u*v^3 
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        pke_load_operand_256bits((uint32_t *)(rPKE_A(1u,32u)), v);
        pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), out2);       //out2 = v^3
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);  //A1 = v^4
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        pke_load_operand_256bits((uint32_t *)(rPKE_B(1u,32u)), out1);       //out1 = u*v^3 
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODMUL);  //A1 = u*v^7
        pke_load_operand_256bits(out2, (uint32_t *)(rPKE_A(1u,32u)));       //out2 = u*v^7
    }
    else
    {}

    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)), 32u<<1);           //clean A0,A1
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,32u)), 32u);
#endif
    }
#endif

    return ret;
}


/* function: Ed25519 decode point step 3
 * parameters:
 *     v -------------------------- input, v = d*y^2 + 1, 8 words
 *     x1 ------------------------- input, x1 = u*v^3, 8 words
 *     x2 ------------------------- input, x2 = u*v^7, 8 words
 *     out1 ----------------------- output, out1 = x = (u*v^3)*(u*v^7 )^((p-5)/8), 8 words
 *     out2 ----------------------- output, out2 = v*x^2, 8 words
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please call ed25519_decode_point_internal_step_2() before calling this function
 */
static uint32_t ed25519_decode_point_internal_step_3(const uint32_t *v, const uint32_t *x1, 
        const uint32_t *x2, uint32_t *out1, uint32_t *out2)
{
    uint32_t ret;

    //get out2 := (u*v^7 )^((p-5)/8)
#if 1
    ret = pke_modexp_256bits_internal(ed25519_p_minus_5_div_8, x2, out2);
#elif 0
    ret = pke_modexp_internal(ed25519_p_minus_5_div_8, x2, out2, Ed25519_WORD_LEN, Ed25519_WORD_LEN);
#else
    //out = (p-5)/8
    uint32_copy_8_words(out2, ed25519->p);
    out2[0] -= 5u;
    Big_Div2n(out2, Ed25519_WORD_LEN, 3u);

    ret = pke_modexp_internal(out2, x2, out2, Ed25519_WORD_LEN, Ed25519_WORD_LEN);
#endif

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(x1, out2, out1, MICROCODE_MODMUL);        //out1 := x := (u*v^3)*(u*v^7 )^((p-5)/8)
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(out1, out1, out2, MICROCODE_MODMUL);       //out2 = x^2
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_mod_add_sub_mul_256bits_internal(out2, v, out2, MICROCODE_MODMUL);         //out2 = v*x^2
    }
    else
    {}

    return ret;
}


/* function: Ed25519 decode point step 4
 * parameters:
 *     v_x_2 ---------------------- input, v*x^2, 8 words
 *     u -------------------------- input, u = y^2 - 1, 8 words
 *     t -------------------------- input, temporary buffer, 8 words
 *     x -------------------------- input&output, root x, 8 words
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please call ed25519_decode_point_internal_step_3() before calling this function
 *     2. if this function returns PKE_SUCCESS, the output x make sense.
 */
static uint32_t ed25519_decode_point_internal_step_4(const uint32_t *v_x_2, const uint32_t *u, 
        uint32_t *t, uint32_t *x)
{
    uint32_t ret;
#if 0
    uint32_t t2[Ed25519_WORD_LEN];
#endif

    if(0 == uint32_BigNumCmp(v_x_2, Ed25519_WORD_LEN, u, Ed25519_WORD_LEN))         //if v x^2 = u (mod p), x is a square root.
    {
        ret = PKE_SUCCESS;
    }
    else
    {
        ret = pke_sub(ed25519->p, u, t, Ed25519_WORD_LEN);                          //t = -u mod p
        if(PKE_SUCCESS == ret)
        {
            if(0 == uint32_BigNumCmp(v_x_2, Ed25519_WORD_LEN, t, Ed25519_WORD_LEN))
            {
                //get t := x*(2^((p-1)/4))
#if 1
                ret = pke_mod_add_sub_mul_256bits_internal(x, ed25519_2_power_p_minus_1_div_4, x, MICROCODE_MODMUL);
#else
                //t = (p-1)/4
                uint32_copy(t, ed25519->p, Ed25519_WORD_LEN);
                t[0] -= 1u;
                Big_Div2n(t, Ed25519_WORD_LEN, 2u);

                //t2 = 2
                pke_set_operand_uint32_value_256bits(t2, 2u);

                //t = 2^((p-1)/4)
                ret = pke_modexp_internal(t, t2, t, Ed25519_WORD_LEN, Ed25519_WORD_LEN);
                if(PKE_SUCCESS != ret)
                {
                    return ret;
                }
                else
                {}

                ret = pke_mod_add_sub_mul_256bits_internal(x, t, x, MICROCODE_MODMUL);     //x := x*(2^((p-1)/4))
#endif
            }
            else
            {
                ret = PKE_INVALID_INPUT;   //root not exist
            }
        }
        else
        {}
    }

    return ret;
}


/* function: Ed25519 decode point step 5
 * parameters:
 *     x -------------------------- input, may be a root, 8 words
 *     t -------------------------- input, temporary buffer, 8 words
 *     x_0 ------------------------ input, real LSB of x
 *     out_x ---------------------- output, root x, 32 bytes
 * return: PKE_SUCCESS(find out a root), other(no root)
 * caution:
 *     1. please call ed25519_decode_point_internal_step_4() before calling this function
 */
static uint32_t ed25519_decode_point_internal_step_5(const uint32_t *x, uint32_t *t, 
        uint32_t x_0, uint8_t *out_x)
{
    uint32_t ret = PKE_SUCCESS;

    //if x=0 and x is odd, decode fail
    if((1u == uint32_BigNum_Check_Zero(x, Ed25519_WORD_LEN)) && (1u == x_0))
    {
        ret = PKE_INVALID_INPUT;
    }
    else
    {
        //get out_x
        if((x[0]&1u) == x_0)
        {
            memcpy_(out_x, (const uint8_t *)x, Ed25519_BYTE_LEN);
        }
        else
        {
            ret = pke_mod_add_sub_mul_256bits_internal(ed25519->p, x, t, MICROCODE_INTSUB);  //t = -x mod p
            if(PKE_SUCCESS == ret)
            {
                memcpy_(out_x, (uint8_t *)t, Ed25519_BYTE_LEN);
            }
            else
            {}
        }
    }

    return ret;
}


/* function: Ed25519 decode point
 * parameters:
 *     in_y ----------------------- input, encoded Ed25519 point
 *     out_x ---------------------- output, x coordinate of input point
 *     out_y ---------------------- output, y coordinate of input point
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t ed25519_decode_point_internal(const uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32])
{
    uint32_t u[Ed25519_WORD_LEN],v[Ed25519_WORD_LEN],t[Ed25519_WORD_LEN],t2[Ed25519_WORD_LEN];
    uint32_t t3[Ed25519_WORD_LEN];
    uint32_t ret = PKE_SUCCESS;

#ifdef SUPPORT_STATIC_ANALYSIS   //just for static analysis.
    if((NULL == in_y) || (NULL == out_y))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //get y
        memcpy_((uint8_t *)u, in_y, Ed25519_BYTE_LEN);
        u[Ed25519_WORD_LEN-1u] &= 0x7FFFFFFFu;

        //make sure y < prime p
        if(uint32_BigNumCmp(u, Ed25519_WORD_LEN, ed25519->p, Ed25519_WORD_LEN) >= 0)
        {
            ret = PKE_INVALID_INPUT;
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_decode_point_internal_step_1(u, t, u, v);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_decode_point_internal_step_2(u, v, t, t2);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_decode_point_internal_step_3(v, t, t2, t, t3);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_decode_point_internal_step_4(t3, u, t2, t);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_decode_point_internal_step_5(t, v, ((uint32_t)(in_y[Ed25519_BYTE_LEN-1u]))>>7, out_x);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //get out_y
            memcpy_(out_y, in_y, Ed25519_BYTE_LEN);
            out_y[Ed25519_BYTE_LEN-1u] &= ((uint8_t)0x7F);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: Ed25519 decode point
 * parameters:
 *     in_y ----------------------- input, encoded Ed25519 point
 *     out_x ---------------------- output, x coordinate of input point
 *     out_y ---------------------- output, y coordinate of input point
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t ed25519_decode_point(const uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32])
{
    uint32_t ret;

#if 0
    ret = pke_set_modulus_and_pre_monts(ed25519->p, ed25519->p_h, ed25519->p_bitLen);
#else
    ret = pke_load_modulus_and_pre_monts_256bits(ed25519->p, ed25519->p_h);
#endif
    if(PKE_SUCCESS == ret)
    {
        ret = ed25519_decode_point_internal(in_y, out_x, out_y);
    }
    else
    {}

    return ret;
}


/* function: edwards25519 curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could not be zero now.
 *     5. please set hardware operand width 256u before calling this.
 *     6. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t ed25519_pointMul_internal(const edward_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_A(1u,32u), Px);                    //A1 Px
        pke_load_operand_256bits((uint32_t *)rPKE_A(2u,32u), Py);                    //A2 Py
        pke_load_operand_256bits((uint32_t *)rPKE_A(3u,32u), curve->d);              //A3 d
        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,32u), k);                     //A4 k

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_Ed25519_PMUL);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(0u,32u)), 32u*3u);                //clean A0,A1,A2
            (void)get_rand_fast((uint8_t *)(rPKE_B(0u,32u)), 32u);
            (void)get_rand_fast((uint8_t *)(rPKE_A(4u,32u)), 32u);
#endif
        }
        else
        {
            pke_read_operand_256bits((uint32_t *)rPKE_A(1u,32u), Qx);                //A1 Qx
            if(NULL != Qy)
            {
                pke_read_operand_256bits((uint32_t *)rPKE_A(2u,32u), Qy);            //A2 Qy
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: edwards25519 curve point mul(random point), Q=[k]P
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. even if the input point P is valid, the output may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. k could not be zero now.
 */
uint32_t ed25519_pointMul(const edward_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        ret = pke_load_modulus_and_pre_monts_256bits(curve->p, curve->p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_pointMul_internal(curve, k, Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: edwards25519 point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. the output point may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 *     4. please set hardware operand width 256u before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t ed25519_pointAdd_internal(const edward_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_A(1u,32u), P1x);                      //A1 P1x
        pke_load_operand_256bits((uint32_t *)rPKE_A(2u,32u), P1y);                      //A2 P1y
        pke_load_operand_256bits((uint32_t *)rPKE_B(1u,32u), P2x);                      //B1 P2x
        pke_load_operand_256bits((uint32_t *)rPKE_B(2u,32u), P2y);                      //B2 P2y
        pke_load_operand_256bits((uint32_t *)rPKE_A(3u,32u), curve->d);                 //A3 d

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_Ed25519_PADD);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(1u,32u)), 32u<<1);                   //clean A1,A2
            (void)get_rand_fast((uint8_t *)(rPKE_B(0u,32u)), 32u*3u);                   //clean B0,B1,B2
#endif
        }
        else
        {
            pke_read_operand_256bits((uint32_t *)rPKE_A(1u,32u), Qx);                   //A1 Qx
            pke_read_operand_256bits((uint32_t *)rPKE_A(2u,32u), Qy);                   //A2 Qy
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: edwards25519 point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, edwards25519 curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. the output point may be neutral point (0, 1), it is valid
 *     3. please make sure the curve is edwards25519
 */
uint32_t ed25519_pointAdd(const edward_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        ret = pke_load_modulus_and_pre_monts_256bits(curve->p, curve->p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = ed25519_pointAdd_internal(curve, P1x, P1y, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}

/**************************** X25519 & Ed25519 finished ********************************/
#endif


#ifdef SUPPORT_SM9
/**************************** SM9 functions *******************************/

/* function: sm9 GF(p^2) add
 * parameters:
 *     a -------------------------- input, element in GF(p^2)
 *     b -------------------------- input, element in GF(p^2)
 *     out ------------------------ input, out = a+b, also an element in GF(p^2)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. please make sure the modulus is set in A0 before calling this.
 */
uint32_t sm9_fp2_add_internal(const uint32_t *a, const uint32_t *b, uint32_t *out)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == a) || (NULL == b) || (NULL == out))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
#if 1
        ret = pke_mod_add_sub_mul_256bits_internal(a, b, out, MICROCODE_MODADD);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&a[SM9_BASE_WORD_LEN], &b[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], MICROCODE_MODADD);
        }
        else
        {}
#else
        ret = pke_modadd(sm9_curve->eccp_p, a, b, out, SM9_BASE_WORD_LEN);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_modadd(sm9_curve->eccp_p, &a[SM9_BASE_WORD_LEN], &b[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], SM9_BASE_WORD_LEN);
        }
        else
        {}
#endif
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) sub
 * parameters:
 *     a -------------------------- input, an element in GF(p^2)
 *     b -------------------------- input, an element in GF(p^2)
 *     out ------------------------ input, out = a-b, also an element in GF(p^2)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. please make sure the modulus is set in A0 before calling this.
 */
uint32_t sm9_fp2_sub_internal(const uint32_t *a, const uint32_t *b, uint32_t *out)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == a) || (NULL == b) || (NULL == out))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
#if 1
        ret = pke_mod_add_sub_mul_256bits_internal(a, b, out, MICROCODE_MODSUB);
        if(PKE_SUCCESS != ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&a[SM9_BASE_WORD_LEN], &b[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], MICROCODE_MODSUB);
        }
        else
        {}
#else
        ret = pke_modsub(sm9_curve->eccp_p, a, b, out, SM9_BASE_WORD_LEN);
        if(PKE_SUCCESS != ret)
        {
            ret = pke_modsub(sm9_curve->eccp_p, &a[SM9_BASE_WORD_LEN], &b[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], SM9_BASE_WORD_LEN);
        }
        else
        {}
#endif
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) mul
 * parameters:
 *     a -------------------------- input, an element in GF(p^2)
 *     b -------------------------- input, an element in GF(p^2)
 *     out ------------------------ input, out = a*b, also an element in GF(p^2)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 *     3. a and out can not point the same buffer
 *     4. b and out can not point the same buffer
 */
uint32_t sm9_fp2_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out)
{
    uint32_t tmp[8];
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == a) || (NULL == b) || (NULL == out))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        ret = pke_mod_add_sub_mul_256bits_internal(a, b, out, MICROCODE_MODMUL);
        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&a[SM9_BASE_WORD_LEN], &b[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], MICROCODE_MODMUL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&out[SM9_BASE_WORD_LEN], &out[SM9_BASE_WORD_LEN], 
                &out[SM9_BASE_WORD_LEN], MICROCODE_MODADD);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(out, &out[SM9_BASE_WORD_LEN], out, MICROCODE_MODSUB);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&a[SM9_BASE_WORD_LEN], b, &out[SM9_BASE_WORD_LEN], 
                MICROCODE_MODMUL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(a, &b[SM9_BASE_WORD_LEN], tmp, MICROCODE_MODMUL);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = pke_mod_add_sub_mul_256bits_internal(&out[SM9_BASE_WORD_LEN], tmp, 
                &out[SM9_BASE_WORD_LEN], MICROCODE_MODADD);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 get ECC-GF(p^2) output point from pke ram
 * parameters:
 *     Qx ------------------------- output, x coordinate of point Q, an element in GF(p^2)
 *     Qy ------------------------- output, y coordinate of point Q, an element in GF(p^2)
 * return: none
 * caution:
 *     1. the output Qx, Qy both are 16 words.
 *     2. this is for sm9_fp2_pointMul_s(), sm9_fp2_pointAdd(), sm9_fp2_pointDouble, etc.
 */
void sm9_fp2_get_eccp_output_point_from_pke_ram(uint32_t *Qx, uint32_t *Qy)
{
    if(NULL != Qx)
    {
        pke_read_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), Qx);                      //A4 Qx0
        pke_read_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &Qx[SM9_BASE_WORD_LEN]);  //A5 Qx1
    }
    else
    {}

    if(NULL != Qy)
    {
        pke_read_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), Qy);                      //A6 Qy0
        pke_read_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &Qy[SM9_BASE_WORD_LEN]);  //A7 Qy1
    }
    else
    {}
}


/* function: sm9 GF(p^2) curve point mul(random point), Q=[k]P(secure version)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of base point
 *     2. please make sure input point P is on the curve
 *     3. even if the input point P is valid, the output may be infinite point, in this case
 *        it will return error.
 *     4. please set hardware operand width 256u before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t sm9_fp2_pointMul_s_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == curve) || (NULL == Px) || (NULL == Py))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_B(4u,SM9_STEPS), Px);                         //B4 Px0
        pke_load_operand_256bits((uint32_t *)rPKE_B(5u,SM9_STEPS), &Px[SM9_BASE_WORD_LEN]);     //B5 Px1
        pke_load_operand_256bits((uint32_t *)rPKE_B(6u,SM9_STEPS), Py);                         //B6 Py0
        pke_load_operand_256bits((uint32_t *)rPKE_B(7u,SM9_STEPS), &Py[SM9_BASE_WORD_LEN]);     //B7 Py1
        pke_load_operand_256bits((uint32_t *)rPKE_B(3u,SM9_STEPS), curve->eccp_b);              //B3 b   ---- kept after calculating
        pke_load_operand_256bits((uint32_t *)rPKE_B(1u,SM9_STEPS), curve->eccp_n);              //B1 n   ---- corrupted after calculating
        pke_load_operand_256bits((uint32_t *)rPKE_A(3u,SM9_STEPS), k);                          //A3 k   ---- kept after calculating
#if 0
        print_BN_buf_U32((uint32_t *)rPKE_A(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "A0 p");
        print_BN_buf_U32((uint32_t *)rPKE_B(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "B0 p_h");
        print_BN_buf_U32((uint32_t *)rPKE_B(4u,SM9_STEPS), SM9_BASE_WORD_LEN, "B4 Px0");
        print_BN_buf_U32((uint32_t *)rPKE_B(5u,SM9_STEPS), SM9_BASE_WORD_LEN, "B5 Px1");
        print_BN_buf_U32((uint32_t *)rPKE_B(6u,SM9_STEPS), SM9_BASE_WORD_LEN, "B6 Py0");
        print_BN_buf_U32((uint32_t *)rPKE_B(7u,SM9_STEPS), SM9_BASE_WORD_LEN, "B7 Py1");
        print_BN_buf_U32((uint32_t *)rPKE_B(3u,SM9_STEPS), SM9_BASE_WORD_LEN, "B3 b");
        print_BN_buf_U32((uint32_t *)rPKE_B(1u,SM9_STEPS), SM9_BASE_WORD_LEN, "B1 n");
        print_BN_buf_U32((uint32_t *)rPKE_A(3u,SM9_STEPS), SM9_BASE_WORD_LEN, "A3 k");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_FP2_PMUL_SEC);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_B(4u,SM9_STEPS), SM9_BASE_BYTE_LEN<<2);         //clean B4,B5,B6,B7
            (void)get_rand_fast((uint8_t *)rPKE_A(3u,SM9_STEPS), SM9_BASE_BYTE_LEN);
#endif
        }
        else
        {
            (void)sm9_fp2_get_eccp_output_point_from_pke_ram(Qx, Qy);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) curve point mul(random point), Q=[k]P(secure version)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of base point
 *     2. please make sure input point P is on the curve
 *     3. even if the input point P is valid, the output may be infinite point, in this case
 *        it will return error.
 */
uint32_t sm9_fp2_pointMul_s(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == curve))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_p, curve->eccp_p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_pointMul_s_internal(curve, k, Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) curve point add, Q=P1+P2
 * parameters:
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. even if the input point P1 and P2 are valid, the output may be infinite point,
 *        in this case it will return error.
 *     3. please set hardware operand width 256u before calling this.
 *     4. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t sm9_fp2_pointAdd_internal(const uint32_t *P1x, const uint32_t *P1y, const uint32_t *P2x, 
        const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == P1x) || (NULL == P1y) || (NULL == P2x) || (NULL == P2y))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), P1x);                        //A4 P1x0
        pke_load_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &P1x[SM9_BASE_WORD_LEN]);    //A5 P1x1
        pke_load_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), P1y);                        //A6 P1y0
        pke_load_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &P1y[SM9_BASE_WORD_LEN]);    //A7 P1y1
        pke_load_operand_256bits((uint32_t *)rPKE_B(4u,SM9_STEPS), P2x);                        //B4 P2x0
        pke_load_operand_256bits((uint32_t *)rPKE_B(5u,SM9_STEPS), &P2x[SM9_BASE_WORD_LEN]);    //B5 P2x1
        pke_load_operand_256bits((uint32_t *)rPKE_B(6u,SM9_STEPS), P2y);                        //B6 P2y0
        pke_load_operand_256bits((uint32_t *)rPKE_B(7u,SM9_STEPS), &P2y[SM9_BASE_WORD_LEN]);    //B7 P2y1

#if 0
print_BN_buf_U32((uint32_t *)rPKE_A(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "A0 p");
print_BN_buf_U32((uint32_t *)rPKE_B(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "B0 p_h");
print_BN_buf_U32((uint32_t *)rPKE_A(4u,SM9_STEPS), SM9_BASE_WORD_LEN, "A4 P1x0");
print_BN_buf_U32((uint32_t *)rPKE_A(5u,SM9_STEPS), SM9_BASE_WORD_LEN, "A5 P1x1");
print_BN_buf_U32((uint32_t *)rPKE_A(6u,SM9_STEPS), SM9_BASE_WORD_LEN, "A6 P1y0");
print_BN_buf_U32((uint32_t *)rPKE_A(7u,SM9_STEPS), SM9_BASE_WORD_LEN, "A7 P1y1");
print_BN_buf_U32((uint32_t *)rPKE_B(4u,SM9_STEPS), SM9_BASE_WORD_LEN, "B4 P2x0");
print_BN_buf_U32((uint32_t *)rPKE_B(5u,SM9_STEPS), SM9_BASE_WORD_LEN, "B5 P2x1");
print_BN_buf_U32((uint32_t *)rPKE_B(6u,SM9_STEPS), SM9_BASE_WORD_LEN, "B6 P2y0");
print_BN_buf_U32((uint32_t *)rPKE_B(7u,SM9_STEPS), SM9_BASE_WORD_LEN, "B7 P2y1");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_FP2_PADD);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_B(4u,SM9_STEPS), SM9_BASE_BYTE_LEN<<2);         //clean B4,B5,B6,B7
            (void)get_rand_fast((uint8_t *)rPKE_A(3u,SM9_STEPS), SM9_BASE_BYTE_LEN);
#endif
        }
        else
        {
            (void)sm9_fp2_get_eccp_output_point_from_pke_ram(Qx, Qy);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) curve point add, Q=P1+P2
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     P1x ------------------------ input, x coordinate of point P1
 *     P1y ------------------------ input, y coordinate of point P1
 *     P2x ------------------------ input, x coordinate of point P2
 *     P2y ------------------------ input, y coordinate of point P2
 *     Qx ------------------------- output, x coordinate of point Q=P1+P2
 *     Qy ------------------------- output, y coordinate of point Q=P1+P2
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P1 and P2 are both on the curve
 *     2. even if the input point P1 and P2 are valid, the output may be infinite point,
 *        in this case it will return error.
 */
uint32_t sm9_fp2_pointAdd(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_p, curve->eccp_p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_pointAdd_internal(P1x, P1y, P2x, P2y, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


#ifdef SM9_ECCP_POINT_DOUBLE
/* function: sm9 GF(p^2) curve point double, Q=[2]P
 * parameters:
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q=[2]P
 *     Qy ------------------------- output, y coordinate of point Q=[2]P
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 *     2. please set hardware operand width 256u before calling this.
 *     3. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t sm9_fp2_pointDouble_internal(const uint32_t *Px, const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == Px) || (NULL == Py))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //pke_pre_calc_mont() may cover A1, so load A1(Px) and other paras here
        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), Px);                         //A4 Px0
        pke_load_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &Px[SM9_BASE_WORD_LEN]);     //A5 Px1
        pke_load_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), Py);                         //A6 Py0
        pke_load_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &Py[SM9_BASE_WORD_LEN]);     //A7 Py1
#if 0
print_BN_buf_U32((uint32_t *)rPKE_A(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "A0 p");
print_BN_buf_U32((uint32_t *)rPKE_B(0u,SM9_STEPS), SM9_BASE_WORD_LEN, "B0 p_h");
print_BN_buf_U32((uint32_t *)rPKE_A(4u,SM9_STEPS), SM9_BASE_WORD_LEN, "A4 Px0");
print_BN_buf_U32((uint32_t *)rPKE_A(5u,SM9_STEPS), SM9_BASE_WORD_LEN, "A5 Px1");
print_BN_buf_U32((uint32_t *)rPKE_A(6u,SM9_STEPS), SM9_BASE_WORD_LEN, "A6 Py0");
print_BN_buf_U32((uint32_t *)rPKE_A(7u,SM9_STEPS), SM9_BASE_WORD_LEN, "A7 Py1");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_FP2_PDBL);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_A(4u,SM9_STEPS), SM9_BASE_BYTE_LEN<<2);         //clean A4,A5,A6,A7
#endif
        }
        else
        {
            (void)sm9_fp2_get_eccp_output_point_from_pke_ram(Qx, Qy);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^2) curve point double, Q=[2]P
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q=[2]P
 *     Qy ------------------------- output, y coordinate of point Q=[2]P
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure input point P is on the curve
 */
uint32_t sm9_fp2_pointDouble(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_p, curve->eccp_p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_pointDouble_internal(Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}
#endif


/* function: sm9 GF(p^2) curve point verify
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t sm9_fp2_pointVerify_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t t1[SM9_BASE_WORD_LEN<<1];
    uint32_t t2[SM9_BASE_WORD_LEN<<1];
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        ret = sm9_fp2_mul_internal(Px, Px, t1);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_mul_internal(Px, t1, t2);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //t1 = b
            uint32_clear_8_words(t1);
            uint32_copy_8_words(&t1[SM9_BASE_WORD_LEN], curve->eccp_b);

            ret = sm9_fp2_add_internal(t1, t2, t1);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_mul_internal(Py, Py, t2);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
#ifdef PKE_SEC
            if(0 != uint32_BigNumCmp_sec(t1, SM9_BASE_WORD_LEN<<1, t2, SM9_BASE_WORD_LEN<<1))
#else
            if(0 != uint32_BigNumCmp(t1, SM9_BASE_WORD_LEN<<1, t2, SM9_BASE_WORD_LEN<<1))
#endif
            {
                ret = PKE_NOT_ON_CURVE;
            }
            else
            {
                ret = PKE_SUCCESS;
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}

/* function: sm9 GF(p^2) curve point verify
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. 
 */
uint32_t sm9_fp2_pointVerify(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_p, curve->eccp_p_h);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_pointVerify_internal(curve, Px, Py);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: check sm9 GF(p^2) curve point P is valid or not(internal API)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. please set hardware operand width 256u before calling this.
 *     2. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t sm9_fp2_check_point_internal(const eccp_curve_st *curve, const uint32_t *Px, 
        const uint32_t *Py)
{
    uint32_t ret;
    uint32_t pWordLen = 0u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        if(uint32_BigNumCmp(Px, pWordLen, curve->eccp_p, pWordLen) >= 0)
        {
            ret = PKE_INTEGER_TOO_BIG;
        }
        else
        {
            ret = PKE_SUCCESS;
        }

        if(PKE_SUCCESS == ret)
        {
            if(uint32_BigNumCmp(&Px[pWordLen], pWordLen, curve->eccp_p, pWordLen) >= 0)
            {
                ret = PKE_INTEGER_TOO_BIG;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            if(uint32_BigNumCmp(Py, pWordLen, curve->eccp_p, pWordLen) >= 0)
            {
                ret = PKE_INTEGER_TOO_BIG;
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            if(uint32_BigNumCmp(&Py[pWordLen], pWordLen, curve->eccp_p, pWordLen) >= 0)
            {
                ret = PKE_INTEGER_TOO_BIG;
            }
            else
            {
                ret = sm9_fp2_pointVerify_internal(curve, Px, Py);
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: check sm9 GF(p^2) curve point P is valid or not
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. 
 */
uint32_t sm9_fp2_check_point(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = sm9_fp2_check_point_internal(curve, Px, Py);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 get GF(p^12) output from pke ram
 * parameters:
 *     ff ------------------------- output, an element in GF(p^12)
 * return: none
 * caution:
 *     1. the output ff is 8*12=96 words.
 *     2. this is for sm9_fp12_exp_s(), sm9_fp12_mul(), sm9_pairing(), etc.
 */
void sm9_fp12_get_output_from_pke_ram(uint32_t *ff)
{
    if(NULL != ff)
    {
        pke_read_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS),  ff );                        //A4 ff0
        pke_read_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS),  &ff[SM9_BASE_WORD_LEN]);     //A5 ff1
        pke_read_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS),  &ff[2u*SM9_BASE_WORD_LEN]);  //A6 ff2
        pke_read_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS),  &ff[3u*SM9_BASE_WORD_LEN]);  //A7 ff3
        pke_read_operand_256bits((uint32_t *)rPKE_A(8u,SM9_STEPS),  &ff[4u*SM9_BASE_WORD_LEN]);  //A8 ff4
        pke_read_operand_256bits((uint32_t *)rPKE_A(9u,SM9_STEPS),  &ff[5u*SM9_BASE_WORD_LEN]);  //A9 ff5
        pke_read_operand_256bits((uint32_t *)rPKE_A(10u,SM9_STEPS), &ff[6u*SM9_BASE_WORD_LEN]);  //A10 ff6
        pke_read_operand_256bits((uint32_t *)rPKE_A(11u,SM9_STEPS), &ff[7u*SM9_BASE_WORD_LEN]);  //A11 ff7
        pke_read_operand_256bits((uint32_t *)rPKE_A(12u,SM9_STEPS), &ff[8u*SM9_BASE_WORD_LEN]);  //A12 ff8
        pke_read_operand_256bits((uint32_t *)rPKE_A(13u,SM9_STEPS), &ff[9u*SM9_BASE_WORD_LEN]);  //A13 ff9
        pke_read_operand_256bits((uint32_t *)rPKE_A(14u,SM9_STEPS), &ff[10u*SM9_BASE_WORD_LEN]); //A14 ff10
        pke_read_operand_256bits((uint32_t *)rPKE_A(15u,SM9_STEPS), &ff[11u*SM9_BASE_WORD_LEN]); //A15 ff11
    }
    else
    {}
}


/* function: sm9 GF(p^12) exponent operation, ff = f^k(secure version)
 * parameters:
 *     f -------------------------- input, an element in GF(p^12)
 *     k -------------------------- input, exponent, 256bits
 *     ff ------------------------- output, ff = f^(k), also an element in GF(p^12)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure the modulus and the pre-calculated mont arguments
 *        of modulus are located in the right address.
 */
uint32_t sm9_fp12_exp_s(const uint32_t *f, const uint32_t *k, uint32_t *ff)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == f)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), f);                          //A4 f0
        pke_load_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &f[SM9_BASE_WORD_LEN]);      //A5 f1
        pke_load_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), &f[2u*SM9_BASE_WORD_LEN]);   //A6 f2
        pke_load_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &f[3u*SM9_BASE_WORD_LEN]);   //A7 f3

        pke_load_operand_256bits((uint32_t *)rPKE_A(8u,SM9_STEPS),  &f[4u*SM9_BASE_WORD_LEN]);  //A8 f4
        pke_load_operand_256bits((uint32_t *)rPKE_A(9u,SM9_STEPS),  &f[5u*SM9_BASE_WORD_LEN]);  //A9 f5
        pke_load_operand_256bits((uint32_t *)rPKE_A(10u,SM9_STEPS), &f[6u*SM9_BASE_WORD_LEN]);  //A10 f6
        pke_load_operand_256bits((uint32_t *)rPKE_A(11u,SM9_STEPS), &f[7u*SM9_BASE_WORD_LEN]);  //A11 f7

        pke_load_operand_256bits((uint32_t *)rPKE_A(12u,SM9_STEPS), &f[8u*SM9_BASE_WORD_LEN]);  //A12 f8
        pke_load_operand_256bits((uint32_t *)rPKE_A(13u,SM9_STEPS), &f[9u*SM9_BASE_WORD_LEN]);  //A13 f9
        pke_load_operand_256bits((uint32_t *)rPKE_A(14u,SM9_STEPS), &f[10u*SM9_BASE_WORD_LEN]); //A14 f10
        pke_load_operand_256bits((uint32_t *)rPKE_A(15u,SM9_STEPS), &f[11u*SM9_BASE_WORD_LEN]); //A15 f11

        pke_load_operand_256bits((uint32_t *)rPKE_A(1u,SM9_STEPS), k);                          //A1 k

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_FP12_MEXP_SEC);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_A(4u,SM9_STEPS), SM9_BASE_BYTE_LEN*12u);        //clean A4,A5,A6,...,A15
            (void)get_rand_fast((uint8_t *)rPKE_A(1u,SM9_STEPS), SM9_BASE_BYTE_LEN);
#endif
        }
        else
        {
            (void)sm9_fp12_get_output_from_pke_ram(ff);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 GF(p^12) mul, ff = f1*f2
 * parameters:
 *     f1 ------------------------- input, an element in GF(p^12)
 *     f2 ------------------------- input, an element in GF(p^12)
 *     ff ------------------------- output, ff = f1*f2, also an element in GF(p^12)
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure the modulus and the pre-calculated mont arguments
 *        of modulus are located in the right address.
 */
uint32_t sm9_fp12_mul(const uint32_t *f1, const uint32_t *f2, uint32_t *ff)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == f1) || (NULL == f2))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), f1);                           //A4 f1_0
        pke_load_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &f1[SM9_BASE_WORD_LEN]);       //A5 f1_1
        pke_load_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), &f1[2u*SM9_BASE_WORD_LEN]);    //A6 f1_2
        pke_load_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &f1[3u*SM9_BASE_WORD_LEN]);    //A7 f1_3

        pke_load_operand_256bits((uint32_t *)rPKE_A(8u,SM9_STEPS),  &f1[4u*SM9_BASE_WORD_LEN]);   //A8 f1_4
        pke_load_operand_256bits((uint32_t *)rPKE_A(9u,SM9_STEPS),  &f1[5u*SM9_BASE_WORD_LEN]);   //A9 f1_5
        pke_load_operand_256bits((uint32_t *)rPKE_A(10u,SM9_STEPS), &f1[6u*SM9_BASE_WORD_LEN]);   //A10 f1_6
        pke_load_operand_256bits((uint32_t *)rPKE_A(11u,SM9_STEPS), &f1[7u*SM9_BASE_WORD_LEN]);   //A11 f1_7

        pke_load_operand_256bits((uint32_t *)rPKE_A(12u,SM9_STEPS), &f1[8u*SM9_BASE_WORD_LEN]);   //A12 f1_8
        pke_load_operand_256bits((uint32_t *)rPKE_A(13u,SM9_STEPS), &f1[9u*SM9_BASE_WORD_LEN]);   //A13 f1_9
        pke_load_operand_256bits((uint32_t *)rPKE_A(14u,SM9_STEPS), &f1[10u*SM9_BASE_WORD_LEN]);  //A14 f1_10
        pke_load_operand_256bits((uint32_t *)rPKE_A(15u,SM9_STEPS), &f1[11u*SM9_BASE_WORD_LEN]);  //A15 f1_11

        pke_load_operand_256bits((uint32_t *)rPKE_B(4u,SM9_STEPS), f2);                           //B4 f2_0
        pke_load_operand_256bits((uint32_t *)rPKE_B(5u,SM9_STEPS), &f2[SM9_BASE_WORD_LEN]);       //B5 f2_1
        pke_load_operand_256bits((uint32_t *)rPKE_B(6u,SM9_STEPS), &f2[2u*SM9_BASE_WORD_LEN]);    //B6 f2_2
        pke_load_operand_256bits((uint32_t *)rPKE_B(7u,SM9_STEPS), &f2[3u*SM9_BASE_WORD_LEN]);    //B7 f2_3

        pke_load_operand_256bits((uint32_t *)rPKE_B(8u,SM9_STEPS),  &f2[4u*SM9_BASE_WORD_LEN]);   //B8 f2_4
        pke_load_operand_256bits((uint32_t *)rPKE_B(9u,SM9_STEPS),  &f2[5u*SM9_BASE_WORD_LEN]);   //B9 f2_5
        pke_load_operand_256bits((uint32_t *)rPKE_B(10u,SM9_STEPS), &f2[6u*SM9_BASE_WORD_LEN]);   //B10 f2_6
        pke_load_operand_256bits((uint32_t *)rPKE_B(11u,SM9_STEPS), &f2[7u*SM9_BASE_WORD_LEN]);   //B11 f2_7

        pke_load_operand_256bits((uint32_t *)rPKE_B(12u,SM9_STEPS), &f2[8u*SM9_BASE_WORD_LEN]);   //B12 f2_8
        pke_load_operand_256bits((uint32_t *)rPKE_B(13u,SM9_STEPS), &f2[9u*SM9_BASE_WORD_LEN]);   //B13 f2_9
        pke_load_operand_256bits((uint32_t *)rPKE_B(14u,SM9_STEPS), &f2[10u*SM9_BASE_WORD_LEN]);  //B14 f2_10
        pke_load_operand_256bits((uint32_t *)rPKE_B(15u,SM9_STEPS), &f2[11u*SM9_BASE_WORD_LEN]);  //B15 f2_11

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_FP12_MMUL);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_A(4u,SM9_STEPS), SM9_BASE_BYTE_LEN*12u);          //clean A4,A5,A6,...,A15
            (void)get_rand_fast((uint8_t *)rPKE_B(4u,SM9_STEPS), SM9_BASE_BYTE_LEN*12u);          //clean B4,B5,B6,...,B15
            (void)get_rand_fast((uint8_t *)rPKE_B(1u,SM9_STEPS),  SM9_BASE_BYTE_LEN);
#endif
        }
        else
        {
            (void)sm9_fp12_get_output_from_pke_ram(ff);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: sm9 pairing
 * parameters:
 *     Px ------------------------- input, x coordinate of GF(p) point P
 *     Py ------------------------- input, y coordinate of GF(p) point P
 *     Qx ------------------------- input, x coordinate of GF(p^2) point Q
 *     Qy ------------------------- input, y coordinate of GF(p^2) point Q
 *     ff ------------------------- output, an element in GF(p^12)
 * return: PKE_SUCCESS(success, on the curve), other(error or not on the curve)
 * caution:
 *     1. before calling this function, please make sure the modulus and the pre-calculated mont arguments
 *        of modulus are located in the right address.
 */
uint32_t sm9_pairing(const uint32_t *Px, const uint32_t *Py, const uint32_t *Qx, 
        const uint32_t *Qy, uint32_t *ff)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == Qx) || (NULL == Qy))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        uint32_clear_8_words((uint32_t *)rPKE_A(1u,SM9_STEPS));                             //A1 (6t+2) ---- corrupted after calculating
        ((uint32_t *)rPKE_A(1u,SM9_STEPS))[0] = 0x0215D93EU;
        ((uint32_t *)rPKE_A(1u,SM9_STEPS))[1] = 0x40000000U;
        ((uint32_t *)rPKE_A(1u,SM9_STEPS))[2] = 0x02U;

        pke_load_operand_256bits((uint32_t *)rPKE_A(2u,SM9_STEPS), Px);                     //A2 Px
        pke_load_operand_256bits((uint32_t *)rPKE_A(3u,SM9_STEPS), Py);                     //A3 Py

        pke_load_operand_256bits((uint32_t *)rPKE_A(4u,SM9_STEPS), Qx);                     //A4 Qx0
        pke_load_operand_256bits((uint32_t *)rPKE_A(5u,SM9_STEPS), &Qx[SM9_BASE_WORD_LEN]); //A5 Qx1

        pke_load_operand_256bits((uint32_t *)rPKE_A(6u,SM9_STEPS), Qy);                     //A6 Qy0
        pke_load_operand_256bits((uint32_t *)rPKE_A(7u,SM9_STEPS), &Qy[SM9_BASE_WORD_LEN]); //A7 Qy1

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_SM9_PAIRING);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)rPKE_A(2u,SM9_STEPS), SM9_BASE_BYTE_LEN*6u);     //clean A2,A3,...,A7
#endif
        }
        else
        {
            (void)sm9_fp12_get_output_from_pke_ram(ff);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}
/**************************** SM9 finished ********************************/
#endif



#ifdef PKE_SEC
/*********************************** sec functions *************************************/

/* function: pke sec init
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t pke_sec_init(void)
{
    uint32_t rand_buf[4];
    uint32_t ret;

    (void)pke_init();

    ret = get_rand((uint8_t *)&rand_buf, 16u);
    if(TRNG_SUCCESS != ret)
    {
        ret = PKE_STOP;
    }
    else
    {
        lib_register_unlock(g_pke_reg);
        rPKE_RAND_SEED  = rand_buf[0];

        rPKE_RC_EN      = 0u;
        rPKE_RC_K_E_Y   = rand_buf[1];
        rPKE_RC_D_NONCE = rand_buf[2];
        rPKE_RC_A_NONCE = rand_buf[3];
        rPKE_RC_EN      = 1u;
        lib_register_lock(g_pke_reg);

        ret = PKE_SUCCESS;
    }

    return ret;
}


/* function: pke sec uninit
 * parameters: none
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 */
uint32_t pke_sec_uninit(void)
{
    lib_register_unlock(g_pke_reg);
    rPKE_RC_EN = 0u;
    lib_register_lock(g_pke_reg);

    return PKE_SUCCESS;
}


/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please set hardware operand width before calling this.
 *     2. before calling this function, please make sure the pre-calculated mont arguments 
 *        of modulus are located in the right address.
 *     3. modulus must be odd
 *     4. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp_ladder_internal(const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen)
{
    uint32_t step_bytes, step_words;
    uint32_t ret;

#if 1
    step_bytes = pke_get_operand_bytes();
#else
    step_bytes = pke_set_operand_width(mod_wordLen<<5);
#endif
    step_words = step_bytes>>2;

    pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), exponent, exp_wordLen);              //A2 exponent
    if(step_words > exp_wordLen)
    {
        uint32_clear(&(rPKE_A(2u,step_bytes))[exp_wordLen], step_words-exp_wordLen);
    }
    else
    {}

    pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), base, mod_wordLen);                  //B1 base
    if(step_words > mod_wordLen)
    {
        uint32_clear(&(rPKE_B(1u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
    }
    else
    {}

    ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP_MGMR_LADDER);
    if(PKE_SUCCESS != ret)
    {
#ifdef PKE_SEC
        (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), mod_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), mod_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), exp_wordLen<<2);
        (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), mod_wordLen<<2);
#endif
    }
    else
    {
        pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, mod_wordLen);                //A1 result
    }

    return ret;
}


/* function: mod exponent, this could be used for rsa encrypting,decrypting,signing,verifing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. modulus must be odd
 *     2. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 */
uint32_t pke_modexp_ladder(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen)
{
    uint32_t ret;

    ret = pke_pre_calc_mont(modulus, mod_wordLen<<5, NULL);
    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_ladder_internal(exponent, base, out, mod_wordLen, exp_wordLen);
    }
    else
    {}

    return ret;
}


/* function: mod exponent with private key and public key, this could be used for rsa decrypting,signing.
 * parameters:
 *     modulus -------------------- input, modulus
 *     exponent ------------------- input, exponent, actually private key d
 *     pub ------------------------ input, public key e
 *     base ----------------------- input, base number
 *     out ------------------------ output, out = base^(exponent) mod modulus
 *     mod_wordLen ---------------- input, word length of modulus and base number
 *     exp_wordLen ---------------- input, word length of exponent
 *     pub_wordLen ---------------- input, word length of pub
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. before calling this function, please make sure the pre-calculated mont arguments 
 *        of modulus are located in the right address.
 *     2. modulus must be odd
 *     3. please make sure exp_wordLen <= mod_wordLen <= OPERAND_MAX_WORD_LEN
 *     4. please make sure pub_wordLen <= 2
 *     5. please make sure value of exponent should be bigger than 1
 */
uint32_t pke_modexp_with_pub(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *pub, 
        const uint32_t *base, uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen, 
        uint32_t pub_wordLen)
{
    uint32_t step_bytes, step_words;
    uint32_t exp_words, exp_bitLen = get_valid_bits(exponent, exp_wordLen);
    uint32_t ret;

    step_bytes = pke_set_operand_width(mod_wordLen<<5);
    step_words = step_bytes>>2;

    ret = uint32_get_rand_big_number_msb_0((uint32_t *)(rPKE_A(3u,step_bytes)), exp_bitLen);          //A3 d2
    if(0u == ret)
    {
        exp_words = GET_WORD_LEN(exp_bitLen);
        pke_load_operand((uint32_t *)(rPKE_A(2u,step_bytes)), exponent, exp_words);                   //A2 exponent
        if(step_words > exp_words)
        {
            uint32_clear(&(rPKE_A(2u,step_bytes))[exp_words], step_words-exp_words);
            uint32_clear(&(rPKE_A(3u,step_bytes))[exp_words], step_words-exp_words);
        }
        else
        {}

        pke_load_operand((uint32_t *)(rPKE_B(2u,step_bytes)), pub, pub_wordLen);                      //B2 pub
        if(step_words > pub_wordLen)
        {
            uint32_clear(&(rPKE_B(2u,step_bytes))[pub_wordLen], step_words-pub_wordLen);
        }
        else
        {}

        pke_load_operand((uint32_t *)(rPKE_A(0u,step_bytes)), modulus, mod_wordLen);                  //A0 modulus
        pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), base, mod_wordLen);                     //B1 base
        if(step_words > mod_wordLen)
        {
            uint32_clear(&(rPKE_A(0u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
            uint32_clear(&(rPKE_B(1u,step_bytes))[mod_wordLen], step_words-mod_wordLen);
        }
        else
        {}

        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_MODEXP_WITH_PUB_K_E_Y);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_A(0u,step_bytes)), mod_wordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(1u,step_bytes)), mod_wordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(2u,step_bytes)), exp_words<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(3u,step_bytes)), exp_words<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), mod_wordLen<<2);
#endif
        }
        else
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), out, mod_wordLen);                  //A1 result
        }
    }
    else
    {}

    return ret;
}


/* function: ECCP curve sec point mul, Q=[k]P, P is a random point on curve, here k can not be n-1.
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-2], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointMul_sec_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t step_bytes, step_words;
    uint32_t pWordLen, nWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == curve) || (NULL == k))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

        step_bytes = pke_get_operand_bytes();
        step_words = step_bytes>>2;

        pke_load_operand((uint32_t *)(rPKE_B(1u,step_bytes)), Px, pWordLen);                    //B1 Px    ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(2u,step_bytes)), Py, pWordLen);                    //B2 Py    ---- corrupted after calculating
        pke_set_operand_uint32_value((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, 1u);      //A3 Pz    ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(3u,step_bytes)), curve->eccp_b, pWordLen);         //B3 b     ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(4u,step_bytes)), curve->eccp_a, pWordLen);         //B4 a     ---- if a is not 0, B4 is corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_B(5u,step_bytes)), curve->eccp_n, nWordLen);         //B5 order ---- corrupted after calculating
        pke_load_operand((uint32_t *)(rPKE_A(4u,step_bytes)), k, nWordLen);                     //A4 k     ---- corrupted after calculating

        if(step_words > pWordLen)
        {
            uint32_clear(&(rPKE_B(1u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(2u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(3u,step_bytes))[pWordLen], step_words-pWordLen);
            uint32_clear(&(rPKE_B(4u,step_bytes))[pWordLen], step_words-pWordLen);
        }
        else
        {}

        if(step_words > nWordLen)
        {
            uint32_clear(&(rPKE_B(5u,step_bytes))[nWordLen], step_words-nWordLen);
            uint32_clear(&(rPKE_A(4u,step_bytes))[nWordLen], step_words-nWordLen);
        }
        else
        {}
#if 0
print_BN_buf_U32((uint32_t *)(rPKE_B(1u,step_bytes)), step_words, "B1");
print_BN_buf_U32((uint32_t *)(rPKE_B(2u,step_bytes)), step_words, "B2");
print_BN_buf_U32((uint32_t *)(rPKE_A(3u,step_bytes)), step_words, "A3");
print_BN_buf_U32((uint32_t *)(rPKE_B(3u,step_bytes)), step_words, "B3");
print_BN_buf_U32((uint32_t *)(rPKE_B(4u,step_bytes)), step_words, "B4");
print_BN_buf_U32((uint32_t *)(rPKE_B(5u,step_bytes)), step_words, "B5");
print_BN_buf_U32((uint32_t *)(rPKE_A(4u,step_bytes)), step_words, "A4");
#endif
        ret = pke_set_micro_code_start_wait_return_code(MICROCODE_PMUL_SEC);
        if(PKE_SUCCESS != ret)
        {
#ifdef PKE_SEC
            (void)get_rand_fast((uint8_t *)(rPKE_B(1u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(2u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(3u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(4u,step_bytes)), pWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_B(5u,step_bytes)), nWordLen<<2);
            (void)get_rand_fast((uint8_t *)(rPKE_A(4u,step_bytes)), nWordLen<<2);
#endif
        }
        else
        {
            pke_read_operand((uint32_t *)(rPKE_A(1u,step_bytes)), Qx, pWordLen);                //A1 Qx
            if(NULL != Qy)
            {
                pke_read_operand((uint32_t *)(rPKE_A(2u,step_bytes)), Qy, pWordLen);            //A2 Qy
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve sec point mul, Q=[k]P, P is a random point on curve
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 *     4. please set hardware operand width before calling this.
 *     5. before calling this function, please make sure the modulus and the pre-calculated mont 
 *        arguments of modulus are located in the right address.
 */
uint32_t eccp_pointMul_sec_safe_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t pWordLen, nWordLen;
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL == curve) || (NULL == k))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

        //for k = n-1, the hardware does not support(it return code is PKE_NO_MODINV), so here check it.
        //actually, now R1 = [n]G, R0 = [n-1]G, but it can not get y coordinate of output point Q since
        //R1 can not be represented in affine coordinates.
        if(0u == is_k_equal_to_n_minus_1(k, curve->eccp_n, nWordLen))
        {
            uint32_copy(Qx, Px, pWordLen);
            ret = pke_sub(curve->eccp_p, Py, Qy, pWordLen);
        }
        else
        {
            ret = eccp_pointMul_sec_internal(curve, k, Px, Py, Qx, Qy);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: ECCP curve sec point mul, Q=[k]P, P is a random point on curve
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     k -------------------------- input, scalar
 *     Px ------------------------- input, x coordinate of point P
 *     Py ------------------------- input, y coordinate of point P
 *     Qx ------------------------- output, x coordinate of point Q
 *     Qy ------------------------- output, y coordinate of point Q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure k in [1,n-1], n is order of ECCP curve
 *     2. please make sure input point P is on the curve
 *     3. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_pointMul_sec(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy)
{
    uint32_t ret;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL == curve)
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
#endif
        //set ecc_p, ecc_p_h, etc.
        ret = pke_set_modulus_and_pre_monts(curve->eccp_p, curve->eccp_p_h, curve->eccp_p_bitLen);
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_sec_safe_internal(curve, k, Px, Py, Qx, Qy);
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
#endif

    return ret;
}


/* function: get ECCP public key from private key, secure version(the key pair could be used in SM2/ECDSA/ECDH, etc.)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     priKey --------------------- input, private key, big-endian
 *     pubKey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_get_pubkey_from_prikey_s(const eccp_curve_st *curve, const uint8_t *priKey, 
        uint8_t *pubKey)
{
    uint32_t ret, step_bytes, t;
    uint32_t nByteLen, nWordLen = 0u, pByteLen, pWordLen = 0u;
    uint32_t k[ECCP_MAX_WORD_LEN];
    uint32_t *x, *y;

    if((NULL == curve) || (NULL == priKey) || (NULL == pubKey))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

        step_bytes = pke_set_operand_width(curve->eccp_p_bitLen);
        x = (uint32_t *)(rPKE_A(5u,step_bytes));   //can not be A1, since pke_sub erases A1 when k=n-1
        y = (uint32_t *)(rPKE_A(2u,step_bytes));

        k[nWordLen - 1u] = 0u;   //clear if curve->eccp_n_bitLen is not a multiple of 32
        reverse_byte_array(priKey, (uint8_t *)k, nByteLen);

#ifdef SUPPORT_SM2
        if(curve == sm2_curve)
        {
            //make sure k in [1, n-2]
            ret = uint32_integer_check_sec(k, g_sm2p256v1_n_minus_1, nWordLen, PKE_ZERO_ALL, 
                    PKE_INTEGER_TOO_BIG, PKE_SUCCESS);
        }
        else
#endif
        {
            //make sure k in [1, n-1]
            ret = uint32_integer_check_sec(k, curve->eccp_n, nWordLen, PKE_ZERO_ALL, 
                    PKE_INTEGER_TOO_BIG, PKE_SUCCESS);
        }

        //get pubKey
        if(PKE_SUCCESS == ret)
        {
            ret = eccp_pointMul_sec(curve, k, curve->eccp_Gx, curve->eccp_Gy, x, y);
        }
        else
        {}
    }

    //check curve parameters
    if(PKE_SUCCESS == ret)
    {
        (void)get_rand_fast((uint8_t *)(&t), 1u<<2);

        if(1u == uint32_BigNum_Check_Zero_sec(curve->eccp_a, pWordLen))
        {
            if(0u != uint32_cmp_sec(curve->eccp_a, (rPKE_B(4u,step_bytes)), pWordLen, (uint8_t)(t)))
            {
                ret = PKE_ERROR;
            }
            else
            {}
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_cmp_sec(curve->eccp_p, (rPKE_A(0u,step_bytes)), pWordLen, (uint8_t)(t>>1)))
        {
            ret = PKE_ERROR;
        }
        else if(0u != uint32_cmp_sec(curve->eccp_p_h, (rPKE_B(0u,step_bytes)), pWordLen, (uint8_t)(t>>2)))
        {
            ret = PKE_ERROR;
        }
        else
        {
            reverse_byte_array((uint8_t *)x, pubKey, pByteLen);
            reverse_byte_array((uint8_t *)y, &pubKey[pByteLen], pByteLen);
        }
    }

    (void)get_rand_fast((uint8_t *)k, nWordLen<<2);

    return ret;
}


/* function: get ECCP key pair, secure version(the key pair could be used in SM2/ECDSA/ECDH)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     priKey --------------------- output, private key, big-endian
 *     pubKey --------------------- output, public key, big-endian
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure bit length of the curve is not bigger than ECCP_MAX_BIT_LEN
 */
uint32_t eccp_getkey_s(const eccp_curve_st *curve, uint8_t *priKey, uint8_t *pubKey)
{
    uint32_t tmpLen;
    uint32_t nByteLen;
    uint32_t ret;

    if((NULL == curve) || (NULL == priKey) || (NULL == pubKey))
    {
        ret = PKE_POINTER_NULL;
    }
    else
    {
        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
        do {
            ret = get_rand(priKey, nByteLen);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            //make sure k has the same bit length as n
            tmpLen = (curve->eccp_n_bitLen)&7u;
            if(0u != tmpLen)
            {
                priKey[0] &= (1u<<(tmpLen))-1u;
            }
            else
            {}

            ret = eccp_get_pubkey_from_prikey_s(curve, priKey, pubKey);
        } while((PKE_ZERO_ALL == ret) || (PKE_INTEGER_TOO_BIG == ret));
    }

    return ret;
}

#endif

