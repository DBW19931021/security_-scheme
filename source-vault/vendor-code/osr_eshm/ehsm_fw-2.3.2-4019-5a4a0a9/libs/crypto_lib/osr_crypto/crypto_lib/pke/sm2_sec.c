
#include "../../crypto_include/pke_config.h"


#if (defined(SUPPORT_SM2) && defined(SM2_SEC))

#include "./sm2_internal.h"
#include "../../crypto_include/pke/sm2.h"
#include "../../crypto_include/hash_hmac/hash_kdf.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility_sec.h"
#include "eccp_sec_common.h"



#define SM2_SEC_SIGN_COUNTER        (0x64D0B4FEU)
#define SM2_SEC_SIGN_COUNTER1       (0x3A53F102U)
#define SM2_SEC_SIGN_COUNTER2       (0x9F758C1EU)
#define SM2_SEC_DEC_COUNTER         (0x157A396AU)
#define SM2_SEC_EXC_COUNTER         (0xF5264A87U)



/* function: Generate SM2 public key from private key, secure version
 * parameters:
 *     priKey --------------------- input, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS_S(success); SM2_ERROR_S(error)
 * caution:
 */
uint32_t sm2_get_pubkey_from_prikey_s(const uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

    if((NULL == priKey) || (NULL == pubKey))
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = eccp_get_pubkey_from_prikey_s(sm2_curve, priKey, &pubKey[1]);
        if(PKE_SUCCESS == ret)
        {
            pubKey[0] = POINT_UNCOMPRESSED;

            ret = SM2_SUCCESS_S;
        }
        else
        {
            ret = SM2_ERROR_S;
        }
    }

    return ret;
}


/* function: Generate SM2 random Key pair, secure version
 * parameters:
 *     priKey --------------------- output, private key, 32 bytes, big-endian
 *     pubKey --------------------- output, public key(0x04 + x + y), 65 bytes, big-endian
 * return:
 *     SM2_SUCCESS_S(success); SM2_ERROR_S(error)
 * caution:
 */
uint32_t sm2_getkey_s(uint8_t priKey[32], uint8_t pubKey[65])
{
    uint32_t ret;

    if((NULL == priKey) || (NULL == pubKey))
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = eccp_getkey_s(sm2_curve, priKey, &pubKey[1]);
        if(PKE_SUCCESS == ret)
        {
            pubKey[0] = POINT_UNCOMPRESSED;

            ret = SM2_SUCCESS_S;
        }
        else
        {
            ret = SM2_ERROR_S;
        }
    }

    return ret;
}


/* function: Generate SM2 Signature s from k,dA,r step 1(internal API)
 * parameters:
 *     dA[8]  --------------------- input, private key, 8 words, little-endian
 *     rand1[8]  ------------------ input, random number 1, 8 words, little-endian
 *     rand2[8] ------------------- output, random number 2, 8 words, little-endian
 *     out[8] --------------------- output, out = (dA+1)*rand2, 8 words, little-endian
 *     tmp[8] --------------------- input, temporary buffer, 8 words
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. please make sure the inputs are all valid
 */
static uint32_t sm2_sign_get_sec_s_internal_step_1(const uint32_t dA[8], const uint32_t rand1[8], 
        uint32_t rand2[8], uint32_t out[8], uint32_t tmp[8], volatile uint32_t *count)
{
    uint32_t ret;

    (void)counter_add_one(count);

    //out = (dA - rand1) mod n
    ret = pke_mod_add_sub_mul_256bits_internal(dA, rand1, out, MICROCODE_MODSUB);

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //sleep random number
        uint32_sleep(tmp[1]&0x1FFu, (uint8_t)(tmp[1] >> 24));

        (void)counter_add_one(count);

        //out = (dA - rand1 + 1) mod n
        pke_set_operand_uint32_value_256bits(tmp, 1u);
        ret = pke_mod_add_sub_mul_256bits_internal(out, tmp, out, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        uint32_clear_8_words(rand2);
        ret = get_rand((uint8_t *)rand2, 8u);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //out = (dA - rand1 + 1)*rand2 mod n
        ret = pke_mod_add_sub_mul_256bits_internal(out, rand2, out, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //tmp = rand1*rand2 mod n
        ret = pke_mod_add_sub_mul_256bits_internal(rand1, rand2, tmp, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //out = (out+tmp) mod n = ((dA+1)*rand2) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(out, tmp, out, MICROCODE_MODADD);
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature s from k,dA,r step 2(internal API)
 * parameters:
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     r[8] ----------------------- input, Signature r, 8 words, little-endian
 *     w[8] ----------------------- input, w = (dA+1)*rand2, 8 words, little-endian
 *     rand2[8] ------------------- input, random number 2, 8 words, little-endian
 *     s[8]   --------------------- output, Signature s, 8 words, little-endian
 *     tmp[8] --------------------- input, temporary buffer, 8 words
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. please make sure the inputs are all valid
 */
static uint32_t sm2_sign_get_sec_s_internal_step_2(const uint32_t k[8], const uint32_t r[8], 
        uint32_t w[8], const uint32_t rand2[8], uint32_t s[8], uint32_t tmp[8], 
        volatile uint32_t *count)
{
    uint32_t ret;

    (void)counter_add_one(count);

    //w = (w)^(-1) mod n = ((dA+1)*rand2)^(-1) mod n
    ret = pke_modinv_256bits(w, w);

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //tmp = (k+r) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(k, r, tmp, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //tmp = (k+r)*(rand2) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(tmp, rand2, tmp, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //tmp = (w)*(tmp) mod n = (((dA+1)*t2)^(-1))*((k+r)*(t2)) mod n = ((dA+1)^(-1))*(k+r) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(w, tmp, tmp, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //s = (tmp - r) mod n = (((dA+1)^(-1))*(k+r) - r) mod n = ((dA+1)^(-1))*(k+r-(dA+1)r) mod n = ((dA+1)^(-1))*(k-r*dA) mod n
        ret = pke_mod_add_sub_mul_256bits_internal(tmp, r, s, MICROCODE_MODSUB);
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature s from k,dA,r(secure version)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     k[8]   --------------------- input, random number k, 8 words, little-endian
 *     dA[8]  --------------------- input, private key, 8 words, little-endian
 *     r[8]   --------------------- input, Signature r, 8 words, little-endian
 *     s[8]   --------------------- output, Signature s, 8 words, little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. please make sure the inputs are all valid
 *     3. s = ((1+dA)^(-1))*(k-r*dA) mod n
 */
static uint32_t sm2_sign_get_sec_s(eccp_sec_ctx_t *ctx, const uint32_t k[8], const uint32_t dA[8], 
        const uint32_t r[8], uint32_t s[8])
{
    uint32_t ret;
#if 0
    uint32_t tmp1[SM2_WORD_LEN], tmp2[SM2_WORD_LEN], t1[SM2_WORD_LEN], t2[SM2_WORD_LEN];
#else
    uint32_t buf[SM2_WORD_LEN<<2];
#ifndef SUPPORT_STATIC_ANALYSIS
#define  tmp1     ((uint32_t *)(&buf[0]))
#define  tmp2     ((uint32_t *)(&buf[SM2_WORD_LEN]))
#define  t1       ((uint32_t *)(&buf[SM2_WORD_LEN<<1]))
#define  t2       ((uint32_t *)(&buf[SM2_WORD_LEN*3u]))
#else
    uint32_t *tmp1 = buf;
    uint32_t *tmp2 = &buf[SM2_WORD_LEN];
    uint32_t *t1   = &buf[SM2_WORD_LEN<<1];
    uint32_t *t2   = &buf[SM2_WORD_LEN*3u];
#endif
#endif
    volatile uint32_t count = SM2_SEC_SIGN_COUNTER1;
    uint32_t *curve_n, *curve_n_h;
    const eccp_curve_st *curve;

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_n   = &(ctx->eccp_curve_mem[SM2_WORD_LEN*6u]);
    curve_n_h = &(ctx->eccp_curve_mem[SM2_WORD_LEN*7u]);

    //make t1 < n
    do {
        ret = get_rand((uint8_t *)t1, SM2_BYTE_LEN);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {
            break;
        }
    }while(uint32_BigNumCmp_sec(t1, SM2_WORD_LEN, curve->eccp_n, SM2_WORD_LEN) >= 0);

    if(PKE_SUCCESS == ret)
    {
        (void)get_rand_fast((uint8_t *)tmp2, 8u);

        (void)counter_add_one(&count);

        //sleep random number
        uint32_sleep(tmp2[0]&0x1FFu, (uint8_t)(tmp2[0] >> 24));

        (void)counter_add_one(&count);

#if 0
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_n, curve->eccp_n_h);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_sign_get_sec_s_internal_step_1(dA, t1, t2, tmp1, tmp2, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_sign_get_sec_s_internal_step_2(k, r, tmp1, t2, s, tmp2, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_n_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
    }
    else
    {}

    if((count != (SM2_SEC_SIGN_COUNTER1 + 0x0FU)) || (PKE_SUCCESS != ret))
    {
        ret = PKE_ERROR;
        (void)get_rand_fast((uint8_t *)s, SM2_BYTE_LEN);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)buf, sizeof(buf));

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  tmp1
#undef  tmp2
#undef  t1
#undef  t2
#endif

    return ret;
}


/* function: Generate SM2 Signature r and s with rand k step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e[8] ----------------------- input, e value, 8 words, little-endian
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     dA[8] ---------------------- input, private key, 8 words, little-endian
 *     x1[8] ---------------------- input/output, as input it is x coordinate of [k]G, as 
 *                                  output, it is another Signature s, 8 words, little-endian
 *     r[8] ----------------------- output, Signature r, 8 words, little-endian
 *     s[8] ----------------------- output, Signature s, 8 words, little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. e and dA can not be modified
 *     3. e has the same length as n(order of the SM2 curve)
 *     4. dA must be in [1, n-2]
 */
static uint32_t sm2_sign_with_k_s_internal_step_1(eccp_sec_ctx_t *ctx, const uint32_t e[8], 
        const uint32_t k[8], const uint32_t dA[8], uint32_t x1[8], uint32_t r[8], uint32_t s[8], 
        volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t *tmp1 = x1;
    const eccp_curve_st *curve = ctx->curve;
    uint32_t *curve_n = &(ctx->eccp_curve_mem[SM2_WORD_LEN*6u]);

    //r = e + x1 mod n
    ret = pke_modadd_modsub_256bits(curve->eccp_n, e, x1, r, MICROCODE_MODADD);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //make sure r is not zero
        if(0u != uint32_BigNum_Check_Zero_sec(r, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            (void)counter_add_one(count);

            //tmp1 = r + k mod n
            ret = pke_mod_add_sub_mul_256bits_internal(r, k, tmp1, MICROCODE_MODADD);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2_curve->n
        uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM2_STEPS)));

        (void)counter_add_one(count);

        //make sure r+k is not n
        if(0u != uint32_BigNum_Check_Zero_sec(tmp1, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            (void)counter_add_one(count);

            //get tmp1 = s
            ret = sm2_sign_get_sec_s(ctx, k, dA, r, tmp1);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        if(0u != uint32_BigNum_Check_Zero_sec(tmp1, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            (void)counter_add_one(count);

            //get rand for sleeping
            (void)get_rand_fast((uint8_t *)s, 1u<<2);

            //sleep between the two repeated calculations
            uint32_sleep(s[0] & 0x0Fu, (uint8_t)(s[0]>>4));

            //get s again
            ret = sm2_sign_get_sec_s(ctx, k, dA, r, s);
        }
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature r and s with rand k step 2(internal API)
 * parameters:
 *     s1[8] ---------------------- input, Signature s, 8 words, little-endian
 *     s2[8] ---------------------- input, another Signature s, 8 words, little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t sm2_sign_with_k_s_internal_step_2(const uint32_t s1[8], const uint32_t s2[8], 
        volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t t;

    //get rand for sleeping
    (void)get_rand_fast((uint8_t *)&t, 1u<<2);

    (void)counter_add_one(count);

    //sleep random number & securely cmp
    uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
    if(0u != uint32_cmp_sec(s1, s2, SM2_WORD_LEN, (uint8_t)(t>>5)))
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
        if(0u != uint32_cmp_sec(s1, s2, SM2_WORD_LEN, (uint8_t)(t>>5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
        if(0u != uint32_cmp_sec(s1, s2, SM2_WORD_LEN, (uint8_t)(t>>5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //make sure s is not zero
        if(0u != uint32_BigNum_Check_Zero_sec(s2, SM2_WORD_LEN))
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Generate SM2 Signature r and s with rand k
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e[8] ----------------------- input, e value, 8 words, little-endian
 *     k[8] ----------------------- input, random number k, 8 words, little-endian
 *     dA[8] ---------------------- input, private key, 8 words, little-endian
 *     r[8] ----------------------- output, Signature r, 8 words, little-endian
 *     s[8] ----------------------- output, Signature s, 8 words, little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. e and dA can not be modified
 *     3. e has the same length as n(order of the SM2 curve)
 *     4. dA must be in [1, n-2]
 */
static uint32_t sm2_sign_with_k_s(eccp_sec_ctx_t *ctx, const uint32_t e[8], const uint32_t k[8], 
        const uint32_t dA[8], uint32_t r[8], uint32_t s[8])
{
    uint32_t ret;
#if 0
    uint32_t k_bak[SM2_WORD_LEN], tmp1[SM2_WORD_LEN];
#else
    uint32_t buf[SM2_WORD_LEN*2u];
#ifndef SUPPORT_STATIC_ANALYSIS
#define  k_bak    ((uint32_t *)(&buf[0]))
#define  tmp1     ((uint32_t *)(&buf[SM2_WORD_LEN]))
#else
uint32_t *k_bak = buf;
uint32_t *tmp1  = &buf[SM2_WORD_LEN];
#endif
#endif
    volatile uint32_t count = SM2_SEC_SIGN_COUNTER2;
    uint32_t *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);

    //make sure k in [1, n-1]
    ret = uint32_integer_check_sec(k, curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG, 
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //backup k
        uint32_copy_8_words(k_bak, k);

        (void)counter_add_one(&count);

#ifdef SM2_SEC
        ret = eccp_pointMul_sec(curve, k, curve->eccp_Gx, curve->eccp_Gy, tmp1, NULL);
#else
        ret = eccp_pointMul_base(curve, k, tmp1, NULL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //check the point [k]G
        ret = eccp_check_point_internal(curve, (uint32_t *)(rPKE_A(1u,SM2_STEPS)), (uint32_t *)(rPKE_A(2u,SM2_STEPS)));
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        (void)counter_add_one(&count);

        ret = sm2_sign_with_k_s_internal_step_1(ctx, e, k, dA, tmp1, r, s, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_sign_with_k_s_internal_step_2(tmp1, s, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)get_rand_fast((uint8_t *)tmp1, 1u<<2);

        (void)counter_add_one(&count);

        //securely cmp k and backup
        if(0u != uint32_cmp_sec(k_bak, k, SM2_WORD_LEN, (uint8_t)(tmp1[0])))
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

    if((count != (SM2_SEC_SIGN_COUNTER2 + 0x10U)) || (PKE_SUCCESS != ret))
    {
        ret = PKE_ERROR;
        (void)get_rand_fast((uint8_t *)r, SM2_BYTE_LEN);
        (void)get_rand_fast((uint8_t *)s, SM2_BYTE_LEN);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)buf, sizeof(buf));

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  k_bak
#undef  tmp1
#endif

    return ret;
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
static uint32_t sm2_sign_s_internal(eccp_sec_ctx_t *ctx, const uint32_t e_bn[8], const uint8_t rand_k[32], 
        uint32_t k[8], const uint32_t dA[8], uint32_t r[8], uint32_t s[8])
{
    uint32_t ret;

    if(NULL == rand_k)
    {
        do {
            ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);
            if(TRNG_SUCCESS != ret)
            {
                ret = SM2_ERROR_S;
                break;
            }
            else
            {}

            ret = sm2_sign_with_k_s(ctx, e_bn, k, dA, r, s);
        } while((SM2_ZERO_ALL == ret) || (SM2_INTEGER_TOO_BIG == ret));
    }
    else
    {
        u8big_to_u32little_256bits(rand_k, k);
        ret = sm2_sign_with_k_s(ctx, e_bn, k, dA, r, s);
    }

    return ret;
}


/* function: Generate SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     rand_k[32] ----------------- input, random big integer k in signing, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     signature[64] -------------- output, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution:
 *     1. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 */
uint32_t sm2_sign_s(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], 
        uint8_t signature[64])
{
#if 0
    uint32_t e_bn[SM2_WORD_LEN], k[SM2_WORD_LEN], dA[SM2_WORD_LEN], r[SM2_WORD_LEN], s[SM2_WORD_LEN];
#else
    uint32_t buf[SM2_WORD_LEN*5u];
#ifndef SUPPORT_STATIC_ANALYSIS
#define  e_bn     ((uint32_t *)(&buf[0]))
#define  k        ((uint32_t *)(&buf[SM2_WORD_LEN]))
#define  dA       ((uint32_t *)(&buf[SM2_WORD_LEN<<1]))
#define  r        ((uint32_t *)(&buf[SM2_WORD_LEN*3u]))
#define  s        ((uint32_t *)(&buf[SM2_WORD_LEN<<2]))
#else
    uint32_t *e_bn = buf;
    uint32_t *k    = &buf[SM2_WORD_LEN];
    uint32_t *dA   = &buf[SM2_WORD_LEN<<1];
    uint32_t *r    = &buf[SM2_WORD_LEN*3u];
    uint32_t *s    = &buf[SM2_WORD_LEN<<2];
#endif
#endif

    uint32_t ret = SM2_ERROR_S;
    uint16_t eccp_curve_crc16 = 0;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    volatile uint32_t count = SM2_SEC_SIGN_COUNTER;


    if((NULL == E) || (NULL == priKey) || (NULL == signature))
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

        //make sure priKey in [1, n-2]
        u8big_to_u32little_256bits(priKey, dA);
        ret = uint32_integer_check_sec(dA, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ERROR_S, 
                SM2_ERROR_S, PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);
        ret = sm2_sign_s_internal(ctx, e_bn, rand_k, k, dA, r, s);
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

    (void)get_rand_fast((uint8_t *)buf, sizeof(buf));

#ifndef SUPPORT_STATIC_ANALYSIS
#undef  e_bn
#undef  k
#undef  dA
#undef  r
#undef  s
#endif

    return ret;
}


/* function: Verify SM2 Signature step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     pubKey[65] ----------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     signature[64] -------------- input, Signature r and s, 64 bytes, big-endian
 *     r -------------------------- output, first part of the signature, 8 words, little-endian
 *     R -------------------------- output, (e+x1) mod n, it should be equal to r if the signature is valid
 *     tmp ------------------------ input, temporary buffer, 4*8 words
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 */
static uint32_t sm2_verify_s_internal_step_1(eccp_sec_ctx_t *ctx, const uint8_t E[32], 
        const uint8_t pubKey[65], const uint8_t signature[64], uint32_t r[8], 
        uint32_t R[8], uint32_t tmp[32])
{
    uint32_t ret;
    uint32_t t[SM2_WORD_LEN], s[SM2_WORD_LEN];
    uint32_t *e_bn = t;
    uint32_t *curve_b, *curve_p, *curve_p_h, *curve_n;
    const eccp_curve_st *curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);
    curve_n   = &(curve_b[SM2_WORD_LEN*3u]);

    //make sure r in [1, n-1]
    u8big_to_u32little_256bits(signature, r);
    ret = uint32_integer_check(r, curve->eccp_n, SM2_WORD_LEN, SM2_ERROR_S, SM2_ERROR_S, 
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //make sure s in [1, n-1]
        u8big_to_u32little_256bits(&signature[SM2_BYTE_LEN], s);
        ret = uint32_integer_check(s, curve->eccp_n, SM2_WORD_LEN, SM2_ERROR_S, SM2_ERROR_S, 
                PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //t = (r+s) mod n
        (void)pke_set_operand_width(256u);
        ret = pke_modadd_modsub_256bits(curve->eccp_n, r, s, t, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back curve->eccp_n
        uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM2_STEPS)));

        //if t is 0, refuse the signature
        if(0u != uint32_BigNum_Check_Zero(t, SM2_WORD_LEN))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            //get PA and check PA
            u8big_to_u32little_256bits(&pubKey[1u], &tmp[2u*SM2_WORD_LEN]);
            u8big_to_u32little_256bits(&pubKey[1u+SM2_BYTE_LEN], &tmp[3u*SM2_WORD_LEN]);
            ret = eccp_check_point(curve, &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN]);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
#endif

#ifdef SM2_HIGH_SPEED
        ret = eccp_pointMul_Shamir_safe_internal(curve,
                                    s, curve->eccp_Gx, curve->eccp_Gy,
                                    t, &tmp[2u*SM2_WORD_LEN], &tmp[3u*SM2_WORD_LEN],
                                    tmp, NULL);
#else
        //[s]G
        ret = eccp_pointMul_internal(curve, s, curve->eccp_Gx, curve->eccp_Gy, tmp, tmp+SM2_WORD_LEN);
        if(PKE_SUCCESS == ret)
        {
            //[t]PA
            ret = eccp_pointMul_internal(curve, t, tmp+2u*SM2_WORD_LEN, tmp+3u*SM2_WORD_LEN, tmp+2u*SM2_WORD_LEN,
                                tmp+3u*SM2_WORD_LEN);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //[s]G + [t]PA
            ret = eccp_pointAdd_safe_internal(curve, tmp, tmp+SM2_WORD_LEN, tmp+2u*SM2_WORD_LEN, tmp+3u*SM2_WORD_LEN,
                            tmp, NULL);
        }
        else
        {}
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        //get e
        u8big_to_u32little_256bits(E, e_bn);

        //tmp = e + x1 mod n
        ret = pke_modadd_modsub_256bits(curve->eccp_n, e_bn, tmp, R, MICROCODE_MODADD);
        if(PKE_SUCCESS == ret)
        {
            //copy back curve->eccp_n
            uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        }
        else
        {}
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)t, sizeof(t));
    (void)get_rand_fast((uint8_t *)s, sizeof(s));

    return ret;
}


/* function: Verify SM2 Signature step 2(internal API)
 * parameters:
 *     r -------------------------- input, first part of the signature, 8 words, little-endian
 *     R -------------------------- input, (e+x1) mod n, it should be equal to r if the signature is valid
 * return:
 *     PKE_SUCCESS(success, the signature is valid); other(error or the signature is invalid)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 */
static uint32_t sm2_verify_s_internal_step_2(const uint32_t r[8], const uint32_t R[8])
{
    uint32_t ret;
    uint32_t t;

    (void)get_rand_fast((uint8_t *)&t, 1u<<2);

    //sleep random number & securely cmp
    uint32_sleep(t & 0x0Fu, (uint8_t)(t >> 4));
    if(0u != uint32_cmp_sec(R, r, SM2_WORD_LEN, (uint8_t)(t >> 5)))
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t >> 4));
        if(0u != uint32_cmp_sec(R, r, SM2_WORD_LEN, (uint8_t)(t >> 5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t>> 4));
        if(0u != uint32_cmp_sec(R, r, SM2_WORD_LEN, (uint8_t)(t >> 5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Verify SM2 Signature
 * parameters:
 *     E[32] ---------------------- input, E value, 32 bytes, big-endian
 *     pubKey[65] ----------------- input, public key(0x04 + x + y), 65 bytes, big-endian
 *     signature[64] -------------- input, Signature r and s, 64 bytes, big-endian
 * return:
 *     SM2_SUCCESS_S(success, the signature is valid); other(error or the signature is invalid)
 * caution:
 */
uint32_t sm2_verify_s(const uint8_t E[32], const uint8_t pubKey[65], const uint8_t signature[64])
{
    uint32_t ret = SM2_ERROR_S;
    uint32_t r[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<2];
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16 = 0;

    if((NULL == E) || (NULL == pubKey) || (NULL == signature) || (POINT_UNCOMPRESSED != pubKey[0]))
    {}
    else
    {
        //init sm2 curve
        curve = eccp_curve_init(ctx, sm2_curve);
        if(NULL != curve)
        {
            //check crc16 of sm2 paras
            if(0U == ecc_crc16_check(curve, eccp_curve_crc16))
            {
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
        ret = sm2_verify_s_internal_step_1(ctx, E, pubKey, signature, r, tmp, tmp);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_verify_s_internal_step_2(r, tmp);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm2 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            ret = SM2_SUCCESS_S;
        }
    }
    else
    {
        ret = SM2_ERROR_S;
    }

    (void)get_rand_fast((uint8_t *)r, sizeof(r));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));

    eccp_curve_uninit(ctx);

    return ret;
}


/* function: SM2 Encryption with rand k(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     k -------------------------- input, random number k, 8 words, little-endian
 *     pubkey_x ------------------- input, x coordinate of public key point, 8 words, little-endian
 *     pubkey_y ------------------- input, y coordinate of public key point, 8 words, little-endian
 *     C1 ------------------------- output, C1 part of ciphertext, 65 bytes, big-endian
 *     C2 ------------------------- output, C2 part of ciphertext, CByteLen-97 bytes, big-endian
 *     C3 ------------------------- output, C3 part of ciphertext, 32 bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. please make sure pubkey_x and pubkey_y are valid
 */
static uint32_t sm2_encrypt_with_k_s_internal(eccp_sec_ctx_t *ctx, const uint8_t *M, uint32_t MByteLen, 
        const uint32_t *k, const uint32_t *pubkey_x, const uint32_t *pubkey_y,
        uint8_t *C1, uint8_t *C2, uint8_t *C3, uint32_t *CByteLen)
{
    uint32_t ret;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t xy[SM2_WORD_LEN<<1];
    uint32_t *curve_p, *curve_p_h;
    const eccp_curve_st *curve;

    hash_node_st digest_node[3];  //since M and C may point the same address, please do not initialize hash_node here.

    curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);

    //make sure k in [1, n-1]
    ret = uint32_integer_check_sec(k, curve->eccp_n, SM2_WORD_LEN, SM2_ZERO_ALL, SM2_INTEGER_TOO_BIG, 
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get [k]G
#ifdef SM2_SEC
        ret = eccp_pointMul_sec(curve, k, curve->eccp_Gx, curve->eccp_Gy, xy, &xy[SM2_WORD_LEN]);
#else
        ret = eccp_pointMul_base(curve, k, xy, &xy[SM2_WORD_LEN]);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //output C1
        C1[0] = POINT_UNCOMPRESSED;
        u32little_to_u8big_256bits(xy, &C1[1u]);
        u32little_to_u8big_256bits(&xy[SM2_WORD_LEN], &C1[1u+SM2_BYTE_LEN]);

        //get [k]PB
        ret = eccp_pointMul_sec_safe_internal(curve, k, pubkey_x, pubkey_y, xy, &xy[SM2_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        //get x2||y2
        u8big_to_u32little_256bits_self(xy);
        u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);

        //get C3
        digest_node[0].msg_addr  = (uint8_t *)xy;
        digest_node[0].msg_bytes = SM2_BYTE_LEN;
        digest_node[1].msg_addr  = M;
        digest_node[1].msg_bytes = MByteLen;
        digest_node[2].msg_addr  = (uint8_t *)(&xy[SM2_WORD_LEN]);
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
        ret = hash_node_steps(HASH_SM3, digest_node, 3u, C3);
        if(HASH_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get C2
#if 0
        digest_node[0].msg_addr  = (uint8_t *)xy;
#endif
        digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
        digest_node[1].msg_addr  = counter;
        digest_node[1].msg_bytes = 4u;
        ret = ansi_x9_63_kdf_node_with_xor_in(HASH_SM3, digest_node, 2u, counter_buf, M, C2, MByteLen, 1u);
        if(HASH_SUCCESS == ret)
        {
            CByteLen[0] = (MByteLen+1u+(3u*SM2_BYTE_LEN));
            ret = PKE_SUCCESS;
        }
        else if(HASH_OUTPUT_ZERO_ALL == ret)
        {
            ret = SM2_ZERO_ALL;
        }
        else
        {
            ret = SM2_ERROR_S;
        }
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)xy, SM2_BYTE_LEN<<1);
    (void)get_rand_fast((uint8_t *)digest_node, sizeof(digest_node));

    return ret;
}


/* function: SM2 Encryption with rand k
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     rand_k[32] ----------------- input, random big integer k in encrypting, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     pubkey_x ------------------- input, x coordinate of public key point, 8 words, little-endian
 *     pubkey_y ------------------- input, y coordinate of public key point, 8 words, little-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. M and C can be the same buffer
 *     3. please make sure pubkey_x and pubkey_y are valid
 */
static uint32_t sm2_encrypt_with_k_s(eccp_sec_ctx_t *ctx, const uint8_t *M, uint32_t MByteLen, 
        const uint8_t rand_k[32], const uint32_t *pubkey_x, const uint32_t *pubkey_y,
        sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t ret;
    uint32_t k[SM2_WORD_LEN];
    const uint8_t *plain = M;
    uint8_t *C2, *C3;
    uint32_t i;

    C2 = &C[1u+(2u*SM2_BYTE_LEN) + ((SM2_C1C2C3 == order)?0u:SM2_BYTE_LEN)];
    C3 = &C[1u+(2u*SM2_BYTE_LEN) + ((SM2_C1C2C3 == order)?MByteLen:0u)];

    //not support M and C crossing, but support M = C
    ret = PKE_SUCCESS;
    if(M > C)
    {
        if((&C[MByteLen+1u+(3u*SM2_BYTE_LEN)]) > M)
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else if(M < C)
    {
        if((&M[MByteLen]) > C)
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else  //M = C
    {
        //move M to C2, and now M = C2
        i = MByteLen;
        while(i > 0u)
        {
            i--;
            C2[i] = M[i];
        }

        plain = C2;
    }

    if(PKE_SUCCESS == ret)
    {
        if(NULL == rand_k)
        {
            do {
                ret = get_rand((uint8_t *)k, SM2_BYTE_LEN);
                if(TRNG_SUCCESS != ret)
                {
                    ret = SM2_ERROR_S;
                    break;
                }
                else
                {}

                ret = sm2_encrypt_with_k_s_internal(ctx, plain, MByteLen, k, pubkey_x, pubkey_y,
                    C, C2, C3, CByteLen);
            } while((SM2_ZERO_ALL == ret) || (SM2_INTEGER_TOO_BIG == ret));
        }
        else
        {
            u8big_to_u32little_256bits(rand_k, k);
            ret = sm2_encrypt_with_k_s_internal(ctx, plain, MByteLen, k, pubkey_x, pubkey_y,
                C, C2, C3, CByteLen);
        }
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)k, SM2_BYTE_LEN);

    return ret;
}


/* function: SM2 Encryption check input
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     pubKey[65] ----------------- input, public key, 65 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. please make sure pubKey is valid
 */
static uint32_t sm2_encrypt_s_check_input(const uint8_t *M, uint32_t MByteLen, const uint8_t pubKey[65], 
        sm2_cipher_order_e order, const uint8_t *C, const uint32_t *CByteLen)
{
    uint32_t ret;

    if((NULL == M) || (NULL == pubKey) || (NULL == C) || (NULL == CByteLen))
    {
        ret = SM2_ERROR_S;
    }
    else if(0U == MByteLen)
    {
        ret = SM2_ERROR_S;
    }
    else if(order > SM2_C1C2C3)
    {
        ret = SM2_ERROR_S;
    }
    else if(POINT_UNCOMPRESSED != pubKey[0])
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    return ret;
}


/* function: SM2 Encryption
 * parameters:
 *     M -------------------------- input, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- input, byte length of M
 *     rand_k[32] ----------------- input, random big integer k in encrypting, 32 bytes, big-endian,
 *                                  if you do not have this integer, please set this parameter to be NULL,
 *                                  it will be generated inside.
 *     pubKey[65] ----------------- input, public key, 65 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     C -------------------------- output, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- output, byte length of C, should be MByteLen+97 if success
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 *     2. if you do not have rand_k, please set the parameter to be NULL, it will be generated inside.
 *     3. please make sure pubKey is valid
 */
uint32_t sm2_encrypt_s(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], 
        const uint8_t pubKey[65], sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t pubkey_x[SM2_WORD_LEN],pubkey_y[SM2_WORD_LEN];
    uint32_t *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve;
    uint16_t eccp_curve_crc16 = 0;
    uint32_t ret;
    eccp_sec_ctx_t ctx[1];

    ret = sm2_encrypt_s_check_input(M, MByteLen,  pubKey, order, C, CByteLen);
    if(PKE_SUCCESS == ret)
    {
        *CByteLen = 0u;

        //init sm2 curve
        curve = eccp_curve_init(ctx, sm2_curve);
        if(NULL == curve)
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            //check crc16 of sm2 paras
            if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
            {
                ret = SM2_ERROR_S;
            }
            else
            {}
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        curve_p   = ctx->eccp_curve_mem;
        curve_p_h = &(curve_p[SM2_WORD_LEN]);
        curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);

        u8big_to_u32little_256bits(&pubKey[1u], pubkey_x);
        u8big_to_u32little_256bits(&pubKey[1u+SM2_BYTE_LEN], pubkey_y);
        ret = eccp_check_point(curve, pubkey_x, pubkey_y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2 paras that not covered by output
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        ret = sm2_encrypt_with_k_s(ctx, M, MByteLen, rand_k, pubkey_x, pubkey_y, order, C, CByteLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of sm2 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            ret = SM2_SUCCESS_S;
        }
    }
    else
    {
        ret = SM2_ERROR_S;
    }

    if(SM2_SUCCESS_S != ret)
    {
        (void)get_rand_fast((uint8_t *)C, 1u+(2u*SM2_BYTE_LEN)+SM2_BYTE_LEN+MByteLen);
    }
    else
    {}

    eccp_curve_uninit(ctx);
    (void)get_rand_fast((uint8_t *)pubkey_x, SM2_BYTE_LEN);
    (void)get_rand_fast((uint8_t *)pubkey_y, SM2_BYTE_LEN);

    return ret;
}


/* function: SM2 Decryption check input
 * parameters:
 *     C -------------------------- input, pointer to ciphertext
 *     CByteLen ------------------- input, byte length of C, please make sure CByteLen>97
 *     priKey --------------------- input, pointer to private key
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- input, pointer to plaintext
 *     MByteLen ------------------- input, pointer to byte length of M
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 */
static uint32_t sm2_decrypt_s_check_input(const uint8_t *C, uint32_t CByteLen, 
        const uint8_t priKey[32], sm2_cipher_order_e order, const uint8_t *M, 
        const uint32_t *MByteLen, volatile uint32_t *count)
{
    uint32_t ret = SM2_ERROR_S;

    if((NULL == C) || (NULL == priKey) || (NULL == M) || (NULL == MByteLen))
    {}
    else if(CByteLen <= (1u+(3u*SM2_BYTE_LEN)))    //97 = 1+3*ECCP_BYTELEN
    {}
    else if(order > SM2_C1C2C3)
    {}
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //not support M and C crossing, but support M = C
        if(M > C)
        {
            if((&C[CByteLen]) > M)
            {
                ret = SM2_ERROR_S;
            }
            else
            {}
        }
        else if(M < C)
        {
            if((&M[CByteLen-1u-(3u*SM2_BYTE_LEN)]) > C)
            {
                ret = SM2_ERROR_S;
            }
            else
            {}
        }
        else  //M = C
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Decryption step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     C1 ------------------------- input, C1 part of ciphertext, 65 bytes, big-endian
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     xy ------------------------- output, x & y coordinates of [priKey]C1, 8+8 words, little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 */
static uint32_t sm2_decrypt_s_internal_step_1(eccp_sec_ctx_t *ctx, const uint8_t *C1, 
        const uint8_t priKey[32], uint32_t *xy, volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t dA[SM2_WORD_LEN];
    uint32_t *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);

    (void)counter_add_one(count);

    //make sure C1 is on the SM2 curve
    u8big_to_u32little_256bits(&C1[1u], xy);
    u8big_to_u32little_256bits(&C1[1u+SM2_BYTE_LEN], &xy[SM2_WORD_LEN]);
    ret = eccp_check_point(curve, xy, &xy[SM2_WORD_LEN]);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
#endif

        (void)counter_add_one(count);

        //make sure priKey in [1, n-2]
        u8big_to_u32little_256bits(priKey, dA);
        ret = uint32_integer_check_sec(dA, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ERROR_S, 
                SM2_ERROR_S, PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //[dA]C1
        ret = eccp_pointMul_sec_safe_internal(curve, dA, xy, &xy[SM2_WORD_LEN], xy, &xy[SM2_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)dA, sizeof(dA));

    return ret;
}


/* function: SM2 Decryption step 2(internal API)
 * parameters:
 *     xy ------------------------- input, x & y coordinates of [priKey]C1, 8+8 words, little-endian
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- input, byte length of C, please make sure CByteLen>97
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- output, plaintext, should be (CByteLen-97) bytes if success, big-endian
 *     digest --------------------- output, HASH(x||M||y)
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t sm2_decrypt_s_internal_step_2(uint32_t *xy, const uint8_t *C, uint32_t CByteLen, 
        sm2_cipher_order_e order, uint8_t *M, uint32_t *digest, volatile uint32_t *count)
{
    uint32_t ret;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t i, temLen;
    uint8_t C3_buf[SM2_BYTE_LEN];
    const uint8_t *C2;
    const uint8_t *C3;
    hash_node_st digest_node[3];

    (void)counter_add_one(count);

    digest_node[0].msg_addr  = (uint8_t *)xy;
    digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
    digest_node[1].msg_addr  = counter;
    digest_node[1].msg_bytes = 4u;
    digest_node[2].msg_addr  = (uint8_t *)(&xy[SM2_WORD_LEN]);
    digest_node[2].msg_bytes = SM2_BYTE_LEN;

    temLen = CByteLen-1u-(3u*SM2_BYTE_LEN);

    C2 = &C[1u+(2u*SM2_BYTE_LEN) +((SM2_C1C2C3 == order)?0u:SM2_BYTE_LEN)];
    C3 = &C[1u+(2u*SM2_BYTE_LEN) +((SM2_C1C2C3 == order)?temLen:0u)];

    if(M == C)  //M = C
    {
        //keep C3
        memcpy_(C3_buf, C3, SM2_BYTE_LEN);
        C3 = C3_buf;

        //move C2 to M, and now M = C2
        for(i=0; i<temLen; i++)
        {
            M[i] = C2[i];
        }

        C2 = M;
    }
    else
    {}

    u8big_to_u32little_256bits_self(xy);
    u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);
    ret = ansi_x9_63_kdf_node_with_xor_in(HASH_SM3, digest_node, 2u, counter_buf, C2, M, temLen, 1u);
    if(HASH_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

#if 0
        digest_node[0].msg_addr  = (uint8_t *)xy;
#endif
        digest_node[0].msg_bytes = SM2_BYTE_LEN;
        digest_node[1].msg_addr  = (uint8_t *)M;
        digest_node[1].msg_bytes = temLen;
#if 0
        digest_node[2].msg_addr  = (uint8_t *)(xy+SM2_WORD_LEN);
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
#endif
        ret = hash_node_steps(HASH_SM3, digest_node, 3u, (uint8_t *)digest);
        if(HASH_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        memcpy_((uint8_t *)xy, C3, SM2_BYTE_LEN);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)C3_buf, sizeof(C3_buf));
    (void)get_rand_fast((uint8_t *)digest_node, sizeof(digest_node));

    return ret;
}


/* function: SM2 Decryption step 3(internal API)
 * parameters:
 *     C3 ------------------------- input, C3 part of ciphertext, 32 bytes, big-endian
 *     digest --------------------- input, HASH(x||M||y)
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t sm2_decrypt_s_internal_step_3(const uint32_t *C3, const uint32_t *digest, 
        volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t t;

    (void)get_rand_fast((uint8_t *)(&t), 1u<<2);

    //sleep random number & securely cmp
    uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
    if(0u != uint32_cmp_sec(C3, digest, SM2_WORD_LEN, (uint8_t)(t>>5)))
    {
        ret = SM2_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
        if(0u != uint32_cmp_sec(C3, digest, SM2_WORD_LEN, (uint8_t)(t>>5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x0Fu, (uint8_t)(t>>4));
        if(0u != uint32_cmp_sec(C3, digest, SM2_WORD_LEN, (uint8_t)(t>>5)))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Decryption(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- input, byte length of C, please make sure CByteLen>97
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- output, plaintext, should be (CByteLen-97) bytes if success, big-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 */
static uint32_t sm2_decrypt_s_internal(eccp_sec_ctx_t *ctx, const uint8_t *C, uint32_t CByteLen, 
        const uint8_t priKey[32], sm2_cipher_order_e order, uint8_t *M, 
        volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t xy[SM2_WORD_LEN<<1];
    uint32_t digest[SM2_WORD_LEN];

    ret = sm2_decrypt_s_internal_step_1(ctx, C, priKey, xy, count);
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_decrypt_s_internal_step_2(xy, C, CByteLen, order, M, digest, count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_decrypt_s_internal_step_3(xy, digest, count);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)xy, sizeof(xy));
    (void)get_rand_fast((uint8_t *)digest, sizeof(digest));

    return ret;
}


/* function: SM2 Decryption
 * parameters:
 *     C -------------------------- input, ciphertext, CByteLen bytes, big-endian
 *     CByteLen ------------------- input, byte length of C, please make sure CByteLen>97
 *     priKey[32] ----------------- input, private key, 32 bytes, big-endian
 *     order ---------------------- input, either SM2_C1C3C2 or SM2_C1C2C3
 *     M -------------------------- output, plaintext, MByteLen bytes, big-endian
 *     MByteLen ------------------- output, byte length of M, should be CByteLen-97 if success
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution:
 *     1. M and C can be the same buffer
 */
uint32_t sm2_decrypt_s(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32],
        sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen)
{
    uint32_t ret;
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    volatile uint32_t count = SM2_SEC_DEC_COUNTER;
    uint16_t eccp_curve_crc16 = 0;

    ret = sm2_decrypt_s_check_input(C, CByteLen, priKey, order, M, MByteLen, &count);
    if(PKE_SUCCESS == ret)
    {
        *MByteLen = 0u;

        (void)counter_add_one(&count);

        //init sm2 curve
        curve = eccp_curve_init(ctx, sm2_curve);
        if(NULL == curve)
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            (void)counter_add_one(&count);

            //check crc16 of sm2 paras
            if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
            {
                ret = SM2_ERROR_S;
            }
            else
            {}
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_decrypt_s_internal(ctx, C, CByteLen, priKey, order, M, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //check crc16 of sm2 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        if(count != (SM2_SEC_DEC_COUNTER + 0x0EU))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            *MByteLen = CByteLen-1u-(3u*SM2_BYTE_LEN);

            ret = SM2_SUCCESS_S;
        }
    }
    else
    {
        ret = SM2_ERROR_S;
    }

    if(SM2_SUCCESS_S != ret)
    {
        if(CByteLen > (1u+(3u*SM2_BYTE_LEN)))
        {
            (void)get_rand_fast(M, CByteLen-1u-(3u*SM2_BYTE_LEN));
        }

    }
    else
    {}

    eccp_curve_uninit(ctx);

    return ret;
}


/* function: SM2 Key Exchange check input
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     dA ------------------------- input, pointer to local's permanent private key, 32 bytes, U8 big-endian
 *     PB ------------------------- input, pointer to peer's permanent public key, 65 bytes, U8 big-endian
 *     rA ------------------------- input, pointer to local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, pointer to local's temporary public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, pointer to peer's temporary public key, 65 bytes, U8 big-endian
 *     ZA ------------------------- input, pointer to local's Z value, 32 bytes, U8 big-endian
 *     ZB ------------------------- input, pointer to peer's Z value, 32 bytes, U8 big-endian
 *     kByteLen ------------------- input, byte length of output key, can not be zero
 *     KA ------------------------- input, pointer to the output key
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_check_input(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        const uint8_t *KA)
{
    uint32_t ret = SM2_ERROR_S;

    if((NULL == dA) || (NULL == PB) || (NULL == rA) || (NULL == RA) || (NULL == RB))
    {}
    else if((NULL == ZA) || (NULL == ZB) || (NULL == KA))
    {}
    else if(role > SM2_Role_Responsor)
    {}
    else if(0u == kByteLen)
    {}
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if((POINT_UNCOMPRESSED != PB[0]) || (POINT_UNCOMPRESSED != RA[0]) || (POINT_UNCOMPRESSED != RB[0]))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     rA ------------------------- input, local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     out1 ----------------------- output, x1*r1, 8 words, U32 little-endian
 *     out2 ----------------------- output, x1*r2, r2 = rA-d1 mod n, 8 words, U32 little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_internal_setp_1(eccp_sec_ctx_t *ctx, const uint8_t *rA, 
        const uint8_t *RA, uint32_t *out1, uint32_t *out2, volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t x1[SM2_WORD_LEN], t1[SM2_WORD_LEN];
    uint32_t *curve_b, *curve_p, *curve_p_h;
#if 0
    uint32_t *curve_n, *curve_n_h;
#endif
    const eccp_curve_st *curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);
#if 0
    curve_n   = &(curve_b[SM2_WORD_LEN*3u]);
    curve_n_h = &(curve_n[SM2_WORD_LEN]);
#endif

    (void)counter_add_one(count);

    u8big_to_u32little_256bits(&RA[1u], x1);
    u8big_to_u32little_256bits(&RA[1u+SM2_BYTE_LEN], t1);
    ret = eccp_check_point(curve, x1, t1);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        //get x1
        uint32_clear(&x1[SM2_WORD_LEN>>1], SM2_WORD_LEN>>1);
        x1[(SM2_WORD_LEN>>1)-1u] |= 0x80000000u;

        //make sure rA in [1, n-2]
        u8big_to_u32little_256bits(rA, t1);
        ret = uint32_integer_check_sec(t1, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ERROR_S, 
                SM2_ERROR_S, PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = get_rand((uint8_t *)out1, 12u);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //sleep random number
        uint32_sleep(out1[2]&0x1FFu, (uint8_t)(out1[2] >> 24));

        uint32_clear(&out1[2u], SM2_WORD_LEN-2u);

        (void)counter_add_one(count);
#if 0
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_n, curve->eccp_n_h);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = pke_mod_add_sub_mul_256bits_internal(t1, out1, out2, MICROCODE_MODSUB);
    }
    else
    {}


    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = pke_mod_add_sub_mul_256bits_internal(out1, x1, out1, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = pke_mod_add_sub_mul_256bits_internal(out2, x1, out2, MICROCODE_MODMUL);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)x1, sizeof(x1));
    (void)get_rand_fast((uint8_t *)t1, sizeof(t1));

    return ret;
}


/* function: SM2 Key Exchange step 2(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     dA ------------------------- input, local's permanent private key, 32 bytes, U8 big-endian
 *     in1 ------------------------ input, x1*r1, 8 words, U32 little-endian
 *     in2 ------------------------ input, x1*r2, r2 = rA-d1 mod n, 8 words, U32 little-endian
 *     tA ------------------------- output, dA+x1*rA, 8 words, U32 little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_internal_setp_2(eccp_sec_ctx_t *ctx, const uint8_t *dA, 
        const uint32_t *in1, const uint32_t *in2, uint32_t *tA, volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t tmp[SM2_WORD_LEN];
#if 0
    uint32_t *curve_b, *curve_p, *curve_p_h;
#endif
    uint32_t *curve_n, *curve_n_h;

    //actually the following pointers are the same as the corresponding fields of curve
#if 0
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);
#endif
    curve_n   = &(ctx->eccp_curve_mem[SM2_WORD_LEN*6u]);
    curve_n_h = &(ctx->eccp_curve_mem[SM2_WORD_LEN*7u]);

    (void)counter_add_one(count);

    //make sure dA in [1, n-2]
    u8big_to_u32little_256bits(dA, tmp);
    ret = uint32_integer_check_sec(tmp, g_sm2p256v1_n_minus_1, SM2_WORD_LEN, SM2_ERROR_S, 
            SM2_ERROR_S, PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = pke_mod_add_sub_mul_256bits_internal(in1, tmp, tmp, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //t1 = (dA + x1*rA) mod n, and it must not be 0
        ret = pke_mod_add_sub_mul_256bits_internal(in2, tmp, tA, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2_curve->n, sm2_curve->n_h
        uint32_copy_8_words(curve_n, (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_n_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));

        (void)get_rand_fast((uint8_t *)tmp, 4u);
        uint32_sleep(tmp[0]&0x1FFu, (uint8_t)(tmp[0] >> 12));

        if(0u != uint32_BigNum_Check_Zero_sec(tA, SM2_WORD_LEN))
        {
            ret = SM2_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));

    return ret;
}


/* function: SM2 Key Exchange step 3(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     PB ------------------------- input, peer's permanent public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     tA ------------------------- input, tA, 8 words, U32 little-endian
 *     x -------------------------- output, x coordinate of the output point U, 8 words, U32 little-endian
 *     y -------------------------- output, y coordinate of the output point U, 8 words, U32 little-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_internal_setp_3(eccp_sec_ctx_t *ctx, const uint8_t *PB, 
        const uint8_t *RB, const uint32_t *tA, uint32_t *x, uint32_t *y, 
        volatile uint32_t *count)
{
    uint32_t ret;
    uint32_t x2[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<1];
    uint32_t *curve_b, *curve_p, *curve_p_h;
#if 0
    uint32_t *curve_n, *curve_n_h;
#endif
    const eccp_curve_st *curve = ctx->curve;

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = ctx->eccp_curve_mem;
    curve_p_h = &(curve_p[SM2_WORD_LEN]);
    curve_b   = &(curve_p_h[SM2_WORD_LEN<<1]);
#if 0
    curve_n   = &(curve_b[SM2_WORD_LEN*3u]);
    curve_n_h = &(curve_n[SM2_WORD_LEN]);
#endif

    (void)counter_add_one(count);

    //make sure RB on the SM2 curve
    u8big_to_u32little_256bits(&RB[1u], tmp);
    u8big_to_u32little_256bits(&RB[1u+SM2_BYTE_LEN], &tmp[SM2_WORD_LEN]);
    ret = eccp_check_point(curve, tmp, &tmp[SM2_WORD_LEN]);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
#endif

        uint32_copy(x2, tmp, SM2_WORD_LEN>>1);
        uint32_clear(&x2[SM2_WORD_LEN>>1], SM2_WORD_LEN>>1);
        x2[(SM2_WORD_LEN>>1)-1u] |= 0x80000000u;
    }
    else
    {}

#ifdef SM2_SEC
    if(PKE_SUCCESS == ret)
    {
        ret = eccp_pointMul_sec_safe_internal(curve, x2, tmp, &tmp[SM2_WORD_LEN], tmp, &tmp[SM2_WORD_LEN]);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        u8big_to_u32little_256bits(&PB[1u], x);
        u8big_to_u32little_256bits(&PB[1u+SM2_BYTE_LEN], y);
        ret = eccp_check_point_internal(curve, x, y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_b,   (uint32_t *)(rPKE_A(4u,SM2_STEPS)));
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
#endif

        ret = eccp_pointAdd_safe_internal(curve, tmp, &tmp[SM2_WORD_LEN], x, y, x, y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = eccp_pointMul_sec_safe_internal(curve, tA, x, y, x, y);
    }
    else
    {}
#else
    if(PKE_SUCCESS == ret)
    {
        //x1 = tA*x2 mod n
#if 0
        ret = pke_set_modulus_and_pre_mont(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#else
        ret = pke_load_modulus_and_pre_monts_256bits(curve->eccp_n, curve->eccp_n_h);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = pke_mod_add_sub_mul_256bits_internal(tA, x2, x2, MICROCODE_MODMUL);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        u8big_to_u32little_256bits(PB+1u, x);
        u8big_to_u32little_256bits(PB+1u+SM2_BYTE_LEN, y);

        (void)counter_add_one(&count);

        //[tA]PB +[tA*x2 mod n]RB
        ret = eccp_pointMul_Shamir_safe(curve,
                                    tA, x, y,
                                    x2, tmp, &tmp[SM2_WORD_LEN],
                                    x, y);
    }
    else
    {}
#endif

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        //copy back sm2 curve paras in PKE RAM that not erased by hardware
        uint32_copy_8_words(curve_p,   (uint32_t *)(rPKE_A(0u,SM2_STEPS)));
        uint32_copy_8_words(curve_p_h, (uint32_t *)(rPKE_B(0u,SM2_STEPS)));
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)x2, sizeof(x2));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));

    return ret;
}


/* function: SM2 Key Exchange step 1,2,3(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     rA ------------------------- input, local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     dA ------------------------- input, local's permanent private key, 32 bytes, U8 big-endian
 *     PB ------------------------- input, peer's permanent public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     t1 ------------------------- input, temporary buffer, 8 words
 *     tmp ------------------------ output, x||y, point U, 16 words
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_internal_setp_1_2_3(eccp_sec_ctx_t *ctx, const uint8_t *rA, 
        const uint8_t *RA, const uint8_t *dA, const uint8_t *PB, const uint8_t *RB, 
        uint32_t *t1, uint32_t *tmp, volatile uint32_t *count)
{
    uint32_t ret;

    ret = sm2_exchangekey_s_internal_setp_1(ctx, rA, RA, tmp, &tmp[SM2_WORD_LEN], count);
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_2(ctx, dA, tmp, &tmp[SM2_WORD_LEN], t1, count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_3(ctx, PB, RB, t1, tmp, &tmp[SM2_WORD_LEN], count);
    }
    else
    {}

    return ret;
}


/* function: SM2 Key Exchange step 4(internal API)
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     xy ------------------------- input, x||y, x and y coordiantes of the point U, 8+8=16 words, U32 little-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     ZA ------------------------- input, local's Z value, 32 bytes, U8 big-endian
 *     ZB ------------------------- input, peer's Z value, 32 bytes, U8 big-endian
 *     kByteLen ------------------- input, byte length of output key, can not be zero
 *     KA ------------------------- output, output key
 *     S1 ------------------------- output, sponsor's S1, or responsor's S2, 32 bytes, U8 big-endian
 *     SA ------------------------- output, sponsor's SA, or responsor's SB, 32 bytes, U8 big-endian
 *     count ---------------------- output, counter 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution: 
 *     1. 
 */
static uint32_t sm2_exchangekey_s_internal_setp_4(sm2_exchange_role_e role,
                        uint32_t *xy, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA, volatile uint32_t *count)
{
    uint32_t ret;
    uint8_t counter_buf[4] = {0,0,0,1};
    const uint8_t *counter = counter_buf;
    uint32_t tmp[SM2_WORD_LEN];
    
    hash_node_st digest_node[5];

    //xU||yU
    u8big_to_u32little_256bits_self(xy);
    u8big_to_u32little_256bits_self(&xy[SM2_WORD_LEN]);

    digest_node[0].msg_addr  = (uint8_t *)xy;
    digest_node[0].msg_bytes = SM2_BYTE_LEN<<1;
    if(SM2_Role_Sponsor == role)
    {
        digest_node[1].msg_addr  = ZA;
        digest_node[2].msg_addr  = ZB;
    }
    else
    {
        digest_node[1].msg_addr  = ZB;
        digest_node[2].msg_addr  = ZA;
    }
    digest_node[1].msg_bytes = SM2_BYTE_LEN;
    digest_node[2].msg_bytes = SM2_BYTE_LEN;
    digest_node[3].msg_addr  = counter;
    digest_node[3].msg_bytes = 4u;

    //KA
    ret = ansi_x9_63_kdf_node(HASH_SM3, digest_node, 4u, counter_buf, KA, kByteLen, NULL, 0u);
    if((HASH_SUCCESS == ret) && (NULL != S1) && (NULL != SA))
    {
        //t1 := hash(xu||ZA||ZB||x1||y1||x2||y2)
        digest_node[0].msg_addr  = (uint8_t *)xy;
        digest_node[0].msg_bytes = SM2_BYTE_LEN;

        if(SM2_Role_Sponsor == role)
        {
            digest_node[1].msg_addr  = ZA;
            digest_node[2].msg_addr  = ZB;
            digest_node[3].msg_addr  = &RA[1u];
            digest_node[4].msg_addr  = &RB[1u];
        }
        else
        {
            digest_node[1].msg_addr  = ZB;
            digest_node[2].msg_addr  = ZA;
            digest_node[3].msg_addr  = &RB[1u];
            digest_node[4].msg_addr  = &RA[1u];
        }

        digest_node[1].msg_bytes = SM2_BYTE_LEN;
        digest_node[2].msg_bytes = SM2_BYTE_LEN;
        digest_node[3].msg_bytes = SM2_BYTE_LEN<<1;
        digest_node[4].msg_bytes = SM2_BYTE_LEN<<1;

        ret = hash_node_steps(HASH_SM3, digest_node, 5u, (uint8_t *)tmp);
        if(HASH_SUCCESS == ret)
        {
            //get SA := hash(0x03||yu||t1)
            ((uint8_t *)(xy))[SM2_BYTE_LEN-1u] = (uint8_t)0x03;
            digest_node[0].msg_addr  = &(((uint8_t *)(xy))[SM2_BYTE_LEN-1u]);
            digest_node[0].msg_bytes = SM2_BYTE_LEN+1u;
            digest_node[1].msg_addr  = (uint8_t *)tmp;
            digest_node[1].msg_bytes = SM2_BYTE_LEN;
            if(SM2_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)SA);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)S1);
            }
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            //get S1 := hash(0x02||yu||t1)
            ((uint8_t *)(xy))[SM2_BYTE_LEN-1u] = (uint8_t)0x02;
            if(SM2_Role_Sponsor == role)
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)S1);
            }
            else
            {
                ret = hash_node_steps(HASH_SM3, digest_node, 2u, (uint8_t *)SA);
            }
        }
        else
        {
            (void)get_rand_fast((uint8_t *)KA, kByteLen);
            (void)get_rand_fast((uint8_t *)S1, SM2_BYTE_LEN);
            (void)get_rand_fast((uint8_t *)SA, SM2_BYTE_LEN);
        }
    }
    else
    {}

    if(HASH_SUCCESS == ret)
    {
        (void)counter_add_one(count);

        ret = PKE_SUCCESS;
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));
    (void)get_rand_fast((uint8_t *)digest_node, sizeof(digest_node));

    return ret;
}


/* function: SM2 Key Exchange
 * parameters:
 *     role ----------------------- input, SM2_Role_Sponsor - sponsor, SM2_Role_Responsor - responsor
 *     dA ------------------------- input, local's permanent private key, 32 bytes, U8 big-endian
 *     PB ------------------------- input, peer's permanent public key, 65 bytes, U8 big-endian
 *     rA ------------------------- input, local's temporary private key, 32 bytes, U8 big-endian
 *     RA ------------------------- input, local's temporary public key, 65 bytes, U8 big-endian
 *     RB ------------------------- input, peer's temporary public key, 65 bytes, U8 big-endian
 *     ZA ------------------------- input, local's Z value, 32 bytes, U8 big-endian
 *     ZB ------------------------- input, peer's Z value, 32 bytes, U8 big-endian
 *     kByteLen ------------------- input, byte length of output key, can not be zero
 *     KA ------------------------- output, output key
 *     S1 ------------------------- output, sponsor's S1, or responsor's S2, 32 bytes, U8 big-endian, this is optional
 *     SA ------------------------- output, sponsor's SA, or responsor's SB, 32 bytes, U8 big-endian, this is optional
 * return:
 *     SM2_SUCCESS_S(success); other(error)
 * caution: 
 *     1. please make sure the inputs are valid
 *     2. S1 and SA are optional, if you don't need, please set S1 and SA as NULL
 *     3. in case that S1(S2) and SA(SB) exist, if S1=SB,S2=SA, then exchange success.
 */
uint32_t sm2_exchangekey_s(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA)
{
    uint32_t ret;
    uint32_t t1[SM2_WORD_LEN], tmp[SM2_WORD_LEN<<1];
    const eccp_curve_st *curve;
    eccp_sec_ctx_t ctx[1];
    volatile uint32_t count = SM2_SEC_EXC_COUNTER;
    uint16_t eccp_curve_crc16 = 0;

    ret = sm2_exchangekey_s_check_input(role, dA, PB, rA, RA, RB, ZA, ZB, kByteLen, KA);
    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //init sm2 curve
        curve = eccp_curve_init(ctx, sm2_curve);
        if(NULL == curve)
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            (void)counter_add_one(&count);

            //check crc16 of sm2 paras
            if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
            {
                ret = SM2_ERROR_S;
            }
            else
            {}
        }
    }
    else
    {}

#if 1
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_1_2_3(ctx, rA, RA, dA, PB, RB, t1, tmp, &count);
    }
    else
    {}
#else
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_1(ctx, rA, RA, tmp, &tmp[SM2_WORD_LEN], &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_2(ctx, dA, tmp, &tmp[SM2_WORD_LEN], t1, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_3(ctx, PB, RB, t1, tmp, &tmp[SM2_WORD_LEN], &count);
    }
    else
    {}
#endif
    if(PKE_SUCCESS == ret)
    {
        ret = sm2_exchangekey_s_internal_setp_4(role, tmp, RA, RB, ZA, ZB, kByteLen,
                        KA, S1, SA, &count);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)counter_add_one(&count);

        //check crc16 of sm2 paras
        if(0U != ecc_crc16_check(curve, eccp_curve_crc16))
        {
            ret = SM2_ERROR_S;
        }
        else
        {
            (void)counter_add_one(&count);

            if(count != (SM2_SEC_EXC_COUNTER + 0x16U))
            {
                ret = SM2_ERROR_S;
            }
            else
            {
                ret = SM2_SUCCESS_S;
            }
        }
    }
    else
    {
        ret = SM2_ERROR_S;
    }

    (void)get_rand_fast((uint8_t *)t1, sizeof(t1));
    (void)get_rand_fast((uint8_t *)tmp, sizeof(tmp));

    eccp_curve_uninit(ctx);

    return ret;
}
#endif

