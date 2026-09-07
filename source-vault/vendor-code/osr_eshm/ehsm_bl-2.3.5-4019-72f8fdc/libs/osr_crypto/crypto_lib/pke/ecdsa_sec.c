
#include "../../crypto_include/pke_config.h"


#if defined(PKE_SEC) && defined(ECDSA_SEC)

#include "../../crypto_include/pke/ecdsa.h"
#include "../../crypto_include/crypto_common/utility_sec.h"
#include "../../crypto_include/trng/trng.h"
#include "eccp_sec_common.h"



/* function: Generate random number r less than n
 * parameters:
 *     n -------------------------- input, big number n, U32 little-endian
 *     nBitLen -------------------- input, bit length of n
 *     nWordLen ------------------- input, word length of n
 *     r -------------------------- output, random number r less than n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. n is actually the parameter n of the elliptic curve(short weierstrass style)
 */
static uint32_t ecdsa_get_rand_less_than_n(const uint32_t *n, uint32_t nBitLen, 
        uint32_t nWordLen, uint32_t *r)
{
    uint32_t ret;
    uint32_t bits = nBitLen & 0x1Fu;

    do {
        ret = get_rand((uint8_t *)r, nWordLen<<2);
        if(TRNG_SUCCESS != ret)
        {
            break;
        }
        else
        {}

        if(0u != bits)
        {
            r[nWordLen-1u] &= (((uint32_t)1)<<bits)-1u;
        }
        else
        {}
    }while(uint32_BigNumCmp_sec(r, nWordLen, n, nWordLen) >= 0);

    if(TRNG_SUCCESS == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature s from k,dA,r step 1(internal API)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     dA ------------------------- input, private key, U32 little-endian
 *     r -------------------------- input, Signature r, U32 little-endian
 *     k -------------------------- input, random number k, U32 little-endian
 *     r1 ------------------------- output, random number r1 = r - r2, U32 little-endian
 *     r2 ------------------------- output, random number r2 = r - r1, U32 little-endian
 *     d1 ------------------------- output, random number d1 = dA - d2, U32 little-endian
 *     d2 ------------------------- output, random number d2 = dA - d1, U32 little-endian
 *     k_inv ---------------------- output, inverse of k, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_sign_get_sec_s_internal_step_1(const eccp_curve_st *curve, const uint32_t *dA, 
        const uint32_t *r, const uint32_t *k, uint32_t *r1, uint32_t *r2, uint32_t *d1, 
        uint32_t *d2, uint32_t *k_inv)
{
    uint32_t ret, tmp_step=0u;
    uint32_t t, nWordLen;

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    //make d1 < n
    ret = ecdsa_get_rand_less_than_n(curve->eccp_n, curve->eccp_n_bitLen, nWordLen, d1);
    if(PKE_SUCCESS == ret)
    {
        //make r1 < n
        ret = ecdsa_get_rand_less_than_n(curve->eccp_n, curve->eccp_n_bitLen, nWordLen, r1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n0, curve->eccp_n_bitLen);
#else
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        (void)get_rand_fast((uint8_t *)&t, 4u);

        //sleep random number
        uint32_sleep(t&0x1FFu, (uint8_t)(t >> 12));

        tmp_step = pke_get_operand_bytes();

        //d2 = (dA - d1) mod n
        ret = pke_mod_add_sub_mul_internal(dA, d1, d2, nWordLen, tmp_step, MICROCODE_MODSUB);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //sleep random number
        t >>= 16;
        uint32_sleep(t&0x1FFu, (uint8_t)(t >> 12));

        //r2 = (r - r1) mod n
        ret = pke_mod_add_sub_mul_internal(r, r1, r2, nWordLen, tmp_step, MICROCODE_MODSUB);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //s = k^(-1) mod n
        ret = pke_modinv_internal(k, k_inv, nWordLen, nWordLen, tmp_step);
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature s from k,dA,r step 2(internal API)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     e -------------------------- input, e value, U32 little-endian
 *     r1 ------------------------- input, random number r1 = r - r2, U32 little-endian
 *     r2 ------------------------- input, random number r2 = r - r1, U32 little-endian
 *     d1 ------------------------- input, random number d1 = dA - d2, U32 little-endian
 *     d2 ------------------------- input, random number d2 = dA - d1, U32 little-endian
 *     k_inv ---------------------- input, inverse of k, U32 little-endian
 *     out ------------------------ output, out = (e + r1*d1 + r2*d2)*k_inv mod n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_sign_get_sec_s_internal_step_2(const eccp_curve_st *curve, const uint32_t *e, 
        const uint32_t *r1, const uint32_t *r2, const uint32_t *d1, const uint32_t *d2, 
        const uint32_t *k_inv, uint32_t *out)
{
    uint32_t ret, tmp_step;
    uint32_t nWordLen;
    uint32_t tmp[ECCP_MAX_WORD_LEN];

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    tmp_step = pke_get_operand_bytes();

    //out = r1*d1 mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
    pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
    ret = pke_modmul_internal(r1, d1, out, nWordLen);
    if(PKE_SUCCESS == ret)
    {
        //out = (e + r1*d1) mod n
        ret = pke_mod_add_sub_mul_internal(e, out, out, nWordLen, tmp_step, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp = r2*d2 mod n
        ret = pke_modmul_internal(r2, d2, tmp, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = (e + r1*d1 + r2*d2) mod n
        ret = pke_mod_add_sub_mul_internal(tmp, out, out, nWordLen, tmp_step, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = out * (k^-1) mod n
        ret = pke_modmul_internal(k_inv, out, out, nWordLen);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)tmp, nWordLen<<2);

    return ret;
}


/* function: Generate ECDSA Signature s from k,dA,r step 3(internal API)
 * parameters:
 *     curve ---------------------- input, eccp_curve_st curve struct pointer
 *     r1 ------------------------- input, random number r1 = r - r2, U32 little-endian
 *     r2 ------------------------- input, random number r2 = r - r1, U32 little-endian
 *     d1 ------------------------- input, random number d1 = dA - d2, U32 little-endian
 *     d2 ------------------------- input, random number d2 = dA - d1, U32 little-endian
 *     k_inv ---------------------- input, inverse of k, U32 little-endian
 *     in ------------------------- input, in = (e + r1*d1 + r2*d2)*k_inv mod n
 *     s -------------------------- output, s = in + (r1*d2 + r2*d1)*k_inv = (e + dA*r)*k_inv, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_sign_get_sec_s_internal_step_3(const eccp_curve_st *curve, 
        const uint32_t *r1, const uint32_t *r2, const uint32_t *d1, const uint32_t *d2, 
        const uint32_t *k_inv, const uint32_t *in, uint32_t *s)
{
    uint32_t ret, tmp_step;
    uint32_t nWordLen;
    uint32_t tmp[ECCP_MAX_WORD_LEN];
    uint32_t tmp2[ECCP_MAX_WORD_LEN];

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    tmp_step = pke_get_operand_bytes();

    //tmp2 =  r1*d2 mod n
    ret = pke_modmul_internal(r1, d2, tmp2, nWordLen);
    if(PKE_SUCCESS == ret)
    {
        //tmp = r2*d1 mod n
        ret = pke_modmul_internal(r2, d1, tmp, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp2 = (r1*d2 + r2*d1) mod n
        ret = pke_mod_add_sub_mul_internal(tmp, tmp2, tmp2, nWordLen, tmp_step, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp2 = tmp2 * k^-1 mod n
        ret = pke_modmul_internal(k_inv, tmp2, tmp2, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //s = (in + tmp2) mod n
        ret = pke_mod_add_sub_mul_internal(in, tmp2, s, nWordLen, tmp_step, MICROCODE_MODADD);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)tmp, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)tmp2, nWordLen<<2);

    return ret;
}


/* function: Generate ECDSA Signature s from k,dA,r(secure version)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e -------------------------- input, e value, U32 little-endian
 *     k -------------------------- input, random number k, U32 little-endian
 *     dA ------------------------- input, private key, U32 little-endian
 *     r -------------------------- input, Signature r, U32 little-endian
 *     s -------------------------- output, Signature s, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. please make sure the inputs are all valid
 */
static uint32_t ecdsa_sign_get_sec_s(eccp_sec_ctx_t *ctx, const uint32_t *e, const uint32_t *k, 
        const uint32_t *dA, const uint32_t *r, uint32_t *s)
{
    uint32_t ret, tmp_step;
    uint32_t pWordLen, nWordLen;
    uint32_t r1[ECCP_MAX_WORD_LEN], r2[ECCP_MAX_WORD_LEN];
    uint32_t d1[ECCP_MAX_WORD_LEN], d2[ECCP_MAX_WORD_LEN];
    uint32_t tmp[ECCP_MAX_WORD_LEN];
    uint32_t *curve_n, *curve_n_h;
    const eccp_curve_st *curve = ctx->curve;

    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    if(nWordLen < 1u)   //just for static analysis.
    {
        ret = ECDSA_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        //actually the following pointers are the same as the corresponding fields of curve
        curve_n   = &(ctx->eccp_curve_mem[pWordLen*6u]);
        curve_n_h = &(curve_n[nWordLen]);

        ret = ecdsa_sign_get_sec_s_internal_step_1(curve, dA, r, k, r1, r2, d1, d2, s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_sign_get_sec_s_internal_step_2(curve, e, r1, r2, d1, d2, s, tmp);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_sign_get_sec_s_internal_step_3(curve, r1, r2, d1, d2, s, tmp, s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back curve paras in PKE RAM that not erased by hardware
        tmp_step = pke_get_operand_bytes();
        uint32_copy(curve_n, (uint32_t *)(rPKE_A(0u,tmp_step)), nWordLen);
        uint32_copy(curve_n_h, (uint32_t *)(rPKE_B(0u,tmp_step)), nWordLen);
    }
    else
    {
        (void)get_rand_fast((uint8_t *)s, nWordLen<<2);
    }

    (void)get_rand_fast((uint8_t *)r1, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)r2, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)d1, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)d2, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)tmp, nWordLen<<2);

    return ret;
}


/* function: Generate ECDSA Signature step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e -------------------------- input, e value, U32 little-endian
 *     k -------------------------- input, random number k, U32 little-endian
 *     dA ------------------------- input, private key, U32 little-endian
 *     x1 ------------------------- input/output, as input it is x coordinate of [k]G, as 
 *                                  output, it is another Signature s, U32 little-endian
 *     r -------------------------- output, Signature r, U32 little-endian
 *     s -------------------------- output, Signature s, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. e and dA can not be modified
 *     3. e has the same length as n(order of the curve)
 *     4. dA must be in [1, n-1]
 */
static uint32_t ecdsa_sign_uint32_s_internal_step_1(eccp_sec_ctx_t *ctx, const uint32_t *e, 
        const uint32_t *k, const uint32_t *dA, uint32_t *x1, uint32_t *r, uint32_t *s)
{
    uint32_t ret;
    uint32_t *tmp1 = x1;
    uint32_t nWordLen;
    const eccp_curve_st *curve = ctx->curve;

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    //r = x1 mod n
    uint32_clear(r, nWordLen);
    ret = pke_modadd(curve->eccp_n, x1, r, r, nWordLen);
    if(PKE_SUCCESS == ret)
    {
        //make sure r is not zero
        if(0u != uint32_BigNum_Check_Zero_sec(r, nWordLen))
        {
            ret = ECDSA_ZERO_ALL;
        }
        else
        {
            ret = ecdsa_sign_get_sec_s(ctx, e, k, dA, r, tmp1);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure s is not zero
        if(0u != uint32_BigNum_Check_Zero_sec(tmp1, nWordLen))
        {
            ret = ECDSA_ZERO_ALL;
        }
        else
        {
            //get rand for sleeping
            (void)get_rand_fast((uint8_t *)s, 1u<<2);

            //sleep between the two repeated calculations
            uint32_sleep(s[0] & 0x1Fu, (uint8_t)(s[0]>>5));

            //get s
            ret = ecdsa_sign_get_sec_s(ctx, e, k, dA, r, s);
        }
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature step 2(internal API)
 * parameters:
 *     s1 ------------------------- input, Signature s, U32 little-endian
 *     s2 ------------------------- input, another Signature s, U32 little-endian
 *     nWordLen ------------------- input, word length of s1 and s2 
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_sign_uint32_s_internal_step_2(const uint32_t *s1, const uint32_t *s2, 
        uint32_t nWordLen)
{
    uint32_t ret;
    uint32_t t;

    //get rand for sleeping
    (void)get_rand_fast((uint8_t *)&t, 1u<<2);

    //sleep random number & securely cmp
    uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
    if(0u != uint32_cmp_sec(s1, s2, nWordLen, (uint8_t)(t>>6)))
    {
        ret = ECDSA_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
        if(0u != uint32_cmp_sec(s1, s2, nWordLen, (uint8_t)(t>>6)))
        {
            ret = ECDSA_ERROR_S;
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
        uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
        if(0u != uint32_cmp_sec(s1, s2, nWordLen, (uint8_t)(t>>6)))
        {
            ret = ECDSA_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure s is not zero
        if(0u != uint32_BigNum_Check_Zero_sec(s2, nWordLen))
        {
            ret = ECDSA_ERROR_S;     //not return ECDSA_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature in U32 little-endian big integer style
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e -------------------------- input, derived from hash value, U32 little-endian
 *     k -------------------------- input, internal random integer k, U32 little-endian
 *     dA ------------------------- input, private key, U32 little-endian
 *     r -------------------------- output, signature r, U32 little-endian
 *     s -------------------------- output, signature s, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. please make sure e is in [0,n-1], dA is in [1,n-1]
 */
static uint32_t ecdsa_sign_uint32_s(eccp_sec_ctx_t *ctx, const uint32_t *e, const uint32_t *k, 
        const uint32_t *dA, uint32_t *r, uint32_t *s)
{
    uint32_t ret, tmp_step;
    uint32_t tmp1[ECCP_MAX_WORD_LEN], tmp2[ECCP_MAX_WORD_LEN];
    uint32_t k_bak[ECCP_MAX_WORD_LEN];
    uint32_t pWordLen, nWordLen = 0u, maxWordLen = 0u;
    uint32_t *curve_b, *curve_p, *curve_p_h;
    const eccp_curve_st *curve = ctx->curve;

    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    maxWordLen = GET_MAX_LEN(nWordLen,pWordLen);

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = &(ctx->eccp_curve_mem[0]);
    curve_p_h = &(curve_p[pWordLen]);
    curve_b   = &(curve_p_h[pWordLen<<1]);

    //make sure k in [1, n-1]
    ret = uint32_integer_check_sec(k, curve->eccp_n, nWordLen, ECDSA_ZERO_ALL, ECDSA_INTEGER_TOO_BIG,
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //backup k
        uint32_copy(k_bak, k, nWordLen);

        uint32_clear(tmp1, maxWordLen);  //for comparing with n later(suppose nWordLen >= pWordLen)

        //get x1
        ret = eccp_pointMul_sec(curve, k, curve->eccp_Gx, curve->eccp_Gy, tmp1, tmp2);  //y coordinate is not needed
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //check the point [k]G
        ret = eccp_check_point_internal(curve, tmp1, tmp2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back curve paras in PKE RAM that not erased by hardware
        uint32_copy(curve_b,   (uint32_t *)(rPKE_A(4u,tmp_step)), pWordLen);
        uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
        uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);

        ret = ecdsa_sign_uint32_s_internal_step_1(ctx, e, k, dA, tmp1, r, s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_sign_uint32_s_internal_step_2(tmp1, s, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //securely cmp k and backup
        if(0u != uint32_cmp_sec(k_bak, k, nWordLen, (uint8_t)(tmp2[0]>>7)))
        {
            ret = ECDSA_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS != ret)
    {
        (void)get_rand_fast((uint8_t *)r, nWordLen<<2);
        (void)get_rand_fast((uint8_t *)s, nWordLen<<2);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)k_bak, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)tmp1, maxWordLen<<2);
    (void)get_rand_fast((uint8_t *)tmp2, nWordLen<<2);

    return ret;
}


/* function: Generate ECDSA Signature(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     e_bn ----------------------- input, e value, U32 little-endian
 *     rand_k --------------------- input, random number k, U8 big-endian
 *     k -------------------------- input, random number k, U32 little-endian
 *     dA ------------------------- input, private key, U32 little-endian
 *     r -------------------------- output, Signature r, U32 little-endian
 *     s -------------------------- output, Signature s, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. e_bn and dA can not be modified
 *     3. e_bn has the same length as n(order of the SM2 curve)
 *     4. dA must be in [1, n-1]
 *     5. if you do not have rand_k, please set the parameter to be NULL, k will be generated inside.
 */
static uint32_t ecdsa_sign_s_internal(eccp_sec_ctx_t *ctx, const uint32_t *e_bn, 
        const uint8_t *rand_k, uint32_t *k, const uint32_t *dA, uint32_t *r, uint32_t *s)
{
    uint32_t ret;
    uint32_t nByteLen, nWordLen, tmpLen;
    const eccp_curve_st *curve = ctx->curve;

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    if(NULL == rand_k)
    {
        do {
            ret = get_rand((uint8_t *)k, nByteLen);
            if(TRNG_SUCCESS != ret)
            {
                ret = ECDSA_ERROR_S;
                break;
            }
            else
            {}

            //make sure k has the same bit length as n
            tmpLen = (curve->eccp_n_bitLen)&0x1Fu;
            if(0u != tmpLen)
            {
                k[nWordLen-1u] &= (((uint32_t)1)<<(tmpLen))-1u;
            }
            else
            {}

            ret = ecdsa_sign_uint32_s(ctx, e_bn, k, dA, r, s);
        } while((ECDSA_ZERO_ALL == ret) || (ECDSA_INTEGER_TOO_BIG == ret));
    }
    else
    {
        k[nWordLen - 1u] = 0u;
        reverse_byte_array(rand_k, (uint8_t *)k, nByteLen);
        ret = ecdsa_sign_uint32_s(ctx, e_bn, k, dA, r, s);
    }

    return ret;
}


/* function: Generate ECDSA Signature in byte string style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     E -------------------------- input, hash value, U8 big-endian
 *     EByteLen ------------------- input, byte length of E
 *     rand_k --------------------- input, random big integer k in signing, U8 big-endian
 *     priKey --------------------- input, private key, U8 big-endian
 *     signature ------------------ output, signature r and s, U8 big-endian
 * return:
 *     ECDSA_SUCCESS_S(success); other(error)
 * caution:
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_sign_s(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature)
{
    uint32_t ret = ECDSA_ERROR_S;
    uint32_t nByteLen = 0u, nWordLen = 0u;
    uint32_t e_bn[ECCP_MAX_WORD_LEN], k[ECCP_MAX_WORD_LEN], dA[ECCP_MAX_WORD_LEN];
    uint32_t r[ECCP_MAX_WORD_LEN], s[ECCP_MAX_WORD_LEN];
    uint16_t eccp_curve_crc16;
    const eccp_curve_st * curve1;
    eccp_sec_ctx_t ctx[1];

    if((NULL == curve) || (NULL == priKey) || (NULL == signature))
    {}
    else if(curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN)
    {}
    else
    {
        //init curve
        curve1 = eccp_curve_init(ctx, (const eccp_curve_st *)curve);
        if(NULL != curve1)
        {
            //check crc16 of curve paras
            eccp_curve_crc16 = calc_eccp_curve_crc16(curve);
            if(0u == ecc_crc16_check(curve1, eccp_curve_crc16))
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
        nByteLen = GET_BYTE_LEN(curve1->eccp_n_bitLen);
        nWordLen = GET_WORD_LEN(curve1->eccp_n_bitLen);


        //get integer e_bn from hash value E(according to SEC1-V2 2009)
        (void)ecdsa_get_e_bn(curve1->eccp_n_bitLen, nByteLen, nWordLen, E, EByteLen, e_bn);

        //make sure priKey in [1, n-1]
        dA[nWordLen - 1u] = 0u;
        reverse_byte_array(priKey, (uint8_t *)dA, nByteLen);
        ret = uint32_integer_check_sec(dA, curve1->eccp_n, nWordLen, ECDSA_ERROR_S, ECDSA_ERROR_S,
                PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_sign_s_internal(ctx, e_bn, rand_k, k, dA, r, s);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of curve paras
        if(0u != ecc_crc16_check(curve1, eccp_curve_crc16))
        {
            ret = ECDSA_ERROR_S;
        }
        else
        {
            ret = ECDSA_SUCCESS_S;
            reverse_byte_array((uint8_t *)r, signature, nByteLen);
            reverse_byte_array((uint8_t *)s, &signature[nByteLen], nByteLen);
        }
    }
    else
    {
        ret = ECDSA_ERROR_S;
    }

    eccp_curve_uninit(ctx);

    if(ECDSA_SUCCESS_S != ret)
    {
        (void)get_rand_fast((uint8_t *)signature, nByteLen<<1);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)e_bn, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)k, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)dA, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)r, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)s, nWordLen<<2);

    return ret;
}


/* function: Verify ECDSA Signature step 1(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     E -------------------------- input, hash value, U8 big-endian
 *     EByteLen ------------------- input, byte length of E
 *     signature ------------------ input, signature r and s, U8 big-endian
 *     r -------------------------- output, signature r, U32 little-endian
 *     u1 ------------------------- output, e*(s^(-1)) mod n, U32 little-endian
 *     u2 ------------------------- output, r*(s^(-1)) mod n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. ctx must be initialized by eccp_curve_init() before calling this function.
 *     2. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
static uint32_t ecdsa_verify_s_internal_step_1(const eccp_sec_ctx_t *ctx, const uint8_t *E, 
        uint32_t EByteLen, const uint8_t *signature, uint32_t *r, uint32_t *u1, uint32_t *u2)
{
    uint32_t ret, tmp_step;
    uint32_t nByteLen, nWordLen;
    uint32_t e_bn[ECCP_MAX_WORD_LEN], tmp[ECCP_MAX_WORD_LEN];
    const eccp_curve_st *curve = ctx->curve;

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    //make sure r in [1, n-1]
    r[nWordLen - 1u] = 0u;
    reverse_byte_array(signature, (uint8_t *)r, nByteLen);
    ret = uint32_integer_check(r, curve->eccp_n, nWordLen, ECDSA_ERROR_S, ECDSA_ERROR_S,
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //make sure s in [1, n-1]
        tmp[nWordLen - 1u] = 0u;
        reverse_byte_array(&signature[nByteLen], (uint8_t *)tmp, nByteLen);
        ret = uint32_integer_check(tmp, curve->eccp_n, nWordLen, ECDSA_ERROR_S, ECDSA_ERROR_S,
                PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n0, curve->eccp_n_bitLen);
#else
        ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //tmp = s^(-1) mod n
        ret = pke_modinv_internal(tmp, tmp, nWordLen, nWordLen, tmp_step);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get integer e_bn from hash value E(according to SEC1-V2 2009)
        (void)ecdsa_get_e_bn(curve->eccp_n_bitLen, nByteLen, nWordLen, E, EByteLen, e_bn);

        //u1 =  e_bn*(s^(-1)) mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(e_bn, tmp, u1, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //u2 =  r*(s^(-1)) mod n
        ret = pke_modmul_internal(r, tmp, u2, nWordLen);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)e_bn, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)tmp, nWordLen<<2);

    return ret;
}


/* function: Verify ECDSA Signature step 2(internal API)
 * parameters:
 *     ctx ------------------------ input, eccp_sec_ctx_t struct pointer
 *     pubKey --------------------- input, public key, U8 big-endian
 *     u1 ------------------------- input, e*(s^(-1)) mod n, U32 little-endian
 *     u2 ------------------------- input, r*(s^(-1)) mod n, U32 little-endian
 *     x1 ------------------------- output, x mod n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_verify_s_internal_step_2(eccp_sec_ctx_t *ctx, const uint8_t *pubKey, 
#if (defined(PKE_HP) || defined(PKE_UHP))
        const uint32_t *u1, const uint32_t *u2, uint32_t *x1)
#else
        uint32_t *u1, uint32_t *u2, uint32_t *x1)
#endif
{
    uint32_t ret, tmp_step;
    uint32_t nWordLen, pByteLen, pWordLen, maxWordLen;
    uint32_t x[ECCP_MAX_WORD_LEN], y[ECCP_MAX_WORD_LEN];
    uint32_t *curve_b, *curve_p, *curve_p_h, *curve_n, *curve_n_h;
    const eccp_curve_st *curve = ctx->curve;

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);
    maxWordLen = GET_MAX_LEN(nWordLen,pWordLen);

    //actually the following pointers are the same as the corresponding fields of curve
    curve_p   = &(ctx->eccp_curve_mem[0]);
    curve_p_h = &(curve_p[pWordLen]);
    curve_b   = &(curve_p_h[pWordLen<<1]);
    curve_n   = &(ctx->eccp_curve_mem[pWordLen*6u]);
    curve_n_h = &(curve_n[nWordLen]);

    tmp_step = pke_get_operand_bytes();

    //copy back curve paras in PKE RAM that not erased by hardware
    uint32_copy(curve_n, (uint32_t *)(rPKE_A(0u,tmp_step)), nWordLen);
    uint32_copy(curve_n_h, (uint32_t *)(rPKE_B(0u,tmp_step)), nWordLen);

    //check public key
    uint32_clear(x, maxWordLen);  //for comparing with n later(suppose nWordLen >= pWordLen)
    y[pWordLen - 1u] = 0u;
    reverse_byte_array(pubKey, (uint8_t *)x, pByteLen);
    reverse_byte_array(&pubKey[pByteLen], (uint8_t *)y, pByteLen);
    ret = eccp_check_point(curve, x, y);
    if(PKE_SUCCESS == ret)
    {
        //copy back curve paras in PKE RAM that not erased by hardware
        uint32_copy(curve_b,   (uint32_t *)(rPKE_A(4u,tmp_step)), pWordLen);
#if 0   //since p and p_h will be used by the following hardware calculation.
        uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
        uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);
#endif

#if (defined(PKE_HP) || defined(PKE_UHP))
        ret = eccp_pointMul_Shamir_safe_internal(curve, u2, x, y, u1, curve->eccp_Gx, curve->eccp_Gy, x, y);
        if(PKE_SUCCESS == ret)
        {
            //copy back curve paras in PKE RAM that not erased by hardware
            uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
            uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);
        }
        else
        {}
#else
        ret = eccp_pointMul_sec_safe_internal(curve, u2, x, y, x, y);
        if(PKE_SUCCESS == ret)
        {
            if(0u == uint32_BigNum_Check_Zero_sec(u1, nWordLen))
            {
                ret = eccp_pointMul_sec_safe_internal(curve, u1, curve->eccp_Gx, curve->eccp_Gy, u1, u2);
                if(PKE_SUCCESS == ret)
                {
                    ret = eccp_pointAdd_safe_internal(curve, x, y, u1, u2, x, y);
                }
                else
                {}

                if(PKE_SUCCESS == ret)
                {
                    ret = eccp_check_point_internal(curve, x, y);
                }
                else
                {}

                if(PKE_SUCCESS == ret)
                {
                    //copy back curve paras in PKE RAM that not erased by hardware
                    uint32_copy(curve_b,   (uint32_t *)(rPKE_A(4u,tmp_step)), pWordLen);
#if 0
                    uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
                    uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);
#endif
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            //copy back curve paras in PKE RAM that not erased by hardware
            uint32_copy(curve_p,   (uint32_t *)(rPKE_A(0u,tmp_step)), pWordLen);
            uint32_copy(curve_p_h, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen);
        }
        else
        {}
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get x1 mod n
        uint32_clear(y, nWordLen);
        ret = pke_modadd(curve->eccp_n, x, y, x1, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //copy back curve paras in PKE RAM that not erased by hardware
        uint32_copy(curve_n, (uint32_t *)(rPKE_A(0u,tmp_step)), nWordLen);
    }
    else
    {}

    (void)get_rand_fast((uint8_t *)x, maxWordLen<<2);
    (void)get_rand_fast((uint8_t *)y, maxWordLen<<2);

    return ret;
}


/* function: Verify ECDSA Signature step 3(internal API)
 * parameters:
 *     x1 ------------------------- input, x mod n, U32 little-endian
 *     r -------------------------- input, signature r, U32 little-endian
 *     nWordLen ------------------- input, word length of curve parameter n
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
static uint32_t ecdsa_verify_s_internal_step_3(const uint32_t *x1, const uint32_t *r, 
        uint32_t nWordLen)
{
    uint32_t ret, t;

    (void)get_rand_fast((uint8_t *)&t, 1u<<2);

    //sleep random number & securely cmp
    uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
    if(0u != uint32_cmp_sec(x1, r, nWordLen, (uint8_t)(t>>6)))
    {
        ret = ECDSA_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        //sleep random number & securely cmp
        t >>= 8;
        uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
        if(0u != uint32_cmp_sec(x1, r, nWordLen, (uint8_t)(t>>6)))
        {
            ret = ECDSA_ERROR_S;
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
        uint32_sleep(t & 0x1Fu, (uint8_t)(t>>5));
        if(0u != uint32_cmp_sec(x1, r, nWordLen, (uint8_t)(t>>6)))
        {
            ret = ECDSA_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Verify ECDSA Signature in byte string style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     E -------------------------- input, hash value, U8 big-endian
 *     EByteLen ------------------- input, byte length of E
 *     pubKey --------------------- input, public key, U8 big-endian
 *     signature ------------------ input, signature r and s, U8 big-endian
 * return:
 *     ECDSA_SUCCESS_S(success); other(error)
 * caution:
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_verify_s(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *pubKey, const uint8_t *signature)
{
    uint32_t ret = ECDSA_ERROR_S;
    uint32_t nWordLen = 0u;
    uint32_t r[ECCP_MAX_WORD_LEN], u1[ECCP_MAX_WORD_LEN], u2[ECCP_MAX_WORD_LEN];
    const eccp_curve_st * curve1;
    eccp_sec_ctx_t ctx[1];
    uint16_t eccp_curve_crc16;

    if((NULL == curve) || (NULL == pubKey) || (NULL == signature))
    {}
    else if(curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN)
    {}
    else
    {
        //init curve
        curve1 = eccp_curve_init(ctx, curve);
        if(NULL != curve1)
        {
            //check crc16 of curve paras
            eccp_curve_crc16 = calc_eccp_curve_crc16(curve);
            if(0u == ecc_crc16_check(curve1, eccp_curve_crc16))
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
        nWordLen = GET_WORD_LEN(curve1->eccp_n_bitLen);

        ret = ecdsa_verify_s_internal_step_1(ctx, E, EByteLen, signature, r, u1, u2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_verify_s_internal_step_2(ctx, pubKey, u1, u2, u1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_verify_s_internal_step_3(u1, r, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //check crc16 of curve paras
        if(0u != ecc_crc16_check(curve1, eccp_curve_crc16))
        {
            ret = ECDSA_ERROR_S;
        }
        else
        {
            ret = ECDSA_SUCCESS_S;
        }
    }
    else
    {
        ret = ECDSA_ERROR_S;
    }

    (void)get_rand_fast((uint8_t *)r, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)u1, nWordLen<<2);
    (void)get_rand_fast((uint8_t *)u2, nWordLen<<2);

    eccp_curve_uninit(ctx);

    return ret;
}

#endif

