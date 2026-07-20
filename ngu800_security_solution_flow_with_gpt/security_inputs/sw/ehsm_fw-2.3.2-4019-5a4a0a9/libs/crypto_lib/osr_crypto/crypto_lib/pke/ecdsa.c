
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_ECDSA

#include "../../crypto_include/pke/ecdsa.h"
#include "../../crypto_include/crypto_common/utility.h"
#include "../../crypto_include/trng/trng.h"



/* function: Generate ECDSA Signature r
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     k -------------------------- input, internal random integer k
 *     r -------------------------- output, signature r
 *     tmp ------------------------ input, temporary buffer
 *     nWordLen ------------------- inpuit, word length of curve parameter n(order of the base point)
 *     pWordLen ------------------- inpuit, word length of curve parameter p
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t ecdsa_sign_uint32_internal_get_r(const eccp_curve_st *curve, const uint32_t *k, 
        uint32_t *r, uint32_t *tmp, uint32_t nWordLen, uint32_t pWordLen)
{
    uint32_t ret;

    //make sure k in [1, n-1]
    ret = uint32_integer_check(k, curve->eccp_n, nWordLen, ECDSA_ZERO_ALL, ECDSA_INTEGER_TOO_BIG,
            PKE_SUCCESS);
    if(PKE_SUCCESS == ret)
    {
        //get x1
#if !(defined(PKE_LP) || defined(PKE_SECURE))
        if((NULL != curve->eccp_half_Gx) && (NULL != curve->eccp_half_Gy))
        {
            ret = eccp_pointMul_base(curve, k, tmp, NULL);
        }
        else
        {
#endif
            ret = eccp_pointMul(curve, k, curve->eccp_Gx, curve->eccp_Gy, tmp, NULL);  //y coordinate is not needed
#if !(defined(PKE_LP) || defined(PKE_SECURE))
        }
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //r = x1 mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_mod(tmp, pWordLen, curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n0, nWordLen, r);
#else
        ret = pke_mod(tmp, pWordLen, curve->eccp_n, curve->eccp_n_h, nWordLen, r);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_BigNum_Check_Zero(r, nWordLen))//make sure r is not zero
        {
            ret = ECDSA_ZERO_ALL;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature s
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     e -------------------------- input, derived from hash value
 *     k -------------------------- input, internal random integer k
 *     dA ------------------------- input, private key
 *     r -------------------------- input, signature r
 *     s -------------------------- output, signature s
 *     tmp ------------------------ input, temporary buffer
 *     nWordLen ------------------- inpuit, word length of curve parameter n(order of the base point)
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. please make sure e is in [0,n-1], dA is in [1,n-1]
 */
FLAG_STATIC uint32_t ecdsa_sign_uint32_internal_get_s(const eccp_curve_st *curve, const uint32_t *e, 
        const uint32_t *k, const uint32_t *dA, const uint32_t *r, uint32_t *s, uint32_t *tmp, 
        uint32_t nWordLen)
{
    uint32_t ret, tmp_step;

#if (defined(PKE_LP) || defined(PKE_SECURE))
    ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n0, curve->eccp_n_bitLen);
#else
    ret = pke_set_modulus_and_pre_monts(curve->eccp_n, curve->eccp_n_h, curve->eccp_n_bitLen);
#endif
    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //tmp =  r*dA mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(r, dA, tmp, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //tmp = e + r*dA mod n
        ret = pke_mod_add_sub_mul_internal(e, tmp, tmp, nWordLen, tmp_step, MICROCODE_MODADD);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //s = k^(-1) mod n
        ret = pke_modinv_internal(k, s, nWordLen, nWordLen, tmp_step);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //s = (k^(-1))*(e + r*dA) mod n
        ret = pke_modmul_internal(s, tmp, s, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure s is not zero
        if(0u != uint32_BigNum_Check_Zero(s, nWordLen))
        {
            ret = ECDSA_ZERO_ALL;
        }
        else
        {
            ret = PKE_SUCCESS;
        }
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature in U32 little-endian big integer style
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     e -------------------------- input, derived from hash value
 *     k -------------------------- input, internal random integer k
 *     dA ------------------------- input, private key
 *     r -------------------------- output, signature r
 *     s -------------------------- output, signature s
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. please make sure e is in [0,n-1], dA is in [1,n-1]
 */
FLAG_STATIC uint32_t ecdsa_sign_uint32(const eccp_curve_st *curve, const uint32_t *e, 
        const uint32_t *k, const uint32_t *dA, uint32_t *r, uint32_t *s)
{
    uint32_t ret;
    uint32_t nWordLen, pWordLen;
    uint32_t tmp[ECCP_MAX_WORD_LEN];

    if(curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN)
    {
        ret = ECDSA_INVALID_INPUT;
    }
    else
    {
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
        pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

        ret = ecdsa_sign_uint32_internal_get_r(curve, k, r, tmp, nWordLen, pWordLen);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_sign_uint32_internal_get_s(curve, e, k, dA, r, s, tmp, nWordLen);
    }
    else
    {}

    return ret;
}


/* function: Generate ECDSA Signature(internal API)
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     e_bn ----------------------- input, e value, U32 little-endian
 *     rand_k --------------------- input, random number k, U8 big-endian
 *     k -------------------------- input, random number k, U32 little-endian
 *     dA ------------------------- input, private key, U32 little-endian
 *     r -------------------------- output, Signature r, U32 little-endian
 *     s -------------------------- output, Signature s, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. if you do not have rand_k, please set the parameter to be NULL, k will be generated inside.
 */
FLAG_STATIC uint32_t ecdsa_sign_internal(const eccp_curve_st *curve, const uint32_t *e_bn, 
        const uint8_t *rand_k, uint32_t *k, const uint32_t *dA, uint32_t *r, uint32_t *s)
{
    uint32_t ret;
    uint32_t nByteLen, nWordLen, tmpLen;

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    if(NULL == rand_k)
    {
        do {
            ret = get_rand((uint8_t *)k, nByteLen);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {
                //make sure k has the same bit length as n
                tmpLen = (curve->eccp_n_bitLen)&0x1Fu;
                if(0u != tmpLen)
                {
                    k[nWordLen-1u] &= (((uint32_t)1)<<(tmpLen))-1u;
                }
                else
                {}
            }

            ret = ecdsa_sign_uint32(curve, e_bn, k, dA, r, s);
        } while((ECDSA_ZERO_ALL == ret) || (ECDSA_INTEGER_TOO_BIG == ret));
    }
    else
    {
        k[nWordLen - 1u] = 0u;
        reverse_byte_array(rand_k, (uint8_t *)k, nByteLen);
        ret = ecdsa_sign_uint32(curve, e_bn, k, dA, r, s);
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
 *     ECDSA_SUCCESS(success); other(error)
 * caution:
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_sign(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature)
{
    uint32_t nByteLen, nWordLen;
    uint32_t e_bn[ECCP_MAX_WORD_LEN], k[ECCP_MAX_WORD_LEN], dA[ECCP_MAX_WORD_LEN];
    uint32_t r[ECCP_MAX_WORD_LEN], s[ECCP_MAX_WORD_LEN];
    uint32_t ret;

    if((NULL == curve) || (NULL == priKey) || (NULL == signature))
    {
        ret = ECDSA_POINTOR_NULL;
    }
    else if(curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN)
    {
        ret = ECDSA_INVALID_INPUT;
    }
    else
    {
        ret = PKE_SUCCESS;

        nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

        //get integer e_bn from hash value E(according to SEC1-V2 2009)
        (void)ecdsa_get_e_bn(curve->eccp_n_bitLen, nByteLen, nWordLen, E, EByteLen, e_bn);

#if 0
        //get e_bn = e_bn mod n, i.e., make sure e_bn in [0, n-1]
        if(uint32_BigNumCmp(e_bn, nWordLen, curve->eccp_n, nWordLen) >= 0)
        {
            ret = pke_sub(e_bn, curve->eccp_n, e_bn, nWordLen);
        }
        else
        {}
#endif
    }

    if(PKE_SUCCESS == ret)
    {
        //make sure priKey in [1, n-1]
        dA[nWordLen - 1u] = 0u;
        reverse_byte_array(priKey, (uint8_t *)dA, nByteLen);
        ret = uint32_integer_check(dA, curve->eccp_n, nWordLen, ECDSA_ZERO_ALL, ECDSA_INTEGER_TOO_BIG,
            PKE_SUCCESS);
        if(PKE_SUCCESS == ret)
        {
            ret = ecdsa_sign_internal(curve, e_bn, rand_k, k, dA, r, s);
        }
        else
        {}

        if(PKE_SUCCESS == ret)
        {
            reverse_byte_array((uint8_t *)r, signature, nByteLen);
            reverse_byte_array((uint8_t *)s, &signature[nByteLen], nByteLen);

            ret = ECDSA_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: Verify ECDSA Signature step 1(internal API)
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     E -------------------------- input, hash value, U8 big-endian
 *     EByteLen ------------------- input, byte length of E
 *     signature ------------------ input, signature r and s, U8 big-endian
 *     r -------------------------- output, signature r, U32 little-endian
 *     u1 ------------------------- output, e*(s^(-1)) mod n, U32 little-endian
 *     u2 ------------------------- output, r*(s^(-1)) mod n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
FLAG_STATIC uint32_t ecdsa_verify_internal_step_1(const eccp_curve_st *curve, const uint8_t *E, 
        uint32_t EByteLen, const uint8_t *signature, uint32_t *r, uint32_t *u1, uint32_t *u2)
{
    uint32_t ret = ECDSA_INVALID_INPUT;
    uint32_t tmp_step;
    uint32_t nByteLen, nWordLen;
    uint32_t e_bn[ECCP_MAX_WORD_LEN], tmp[ECCP_MAX_WORD_LEN];

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    //make sure r in [1, n-1]
    if(nWordLen > 0u)   //just for static analysis.
    {
        r[nWordLen - 1u] = 0u;
        reverse_byte_array(signature, (uint8_t *)r, nByteLen);
        ret = uint32_integer_check(r, curve->eccp_n, nWordLen, ECDSA_ZERO_ALL, ECDSA_INTEGER_TOO_BIG,
                PKE_SUCCESS);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //make sure s in [1, n-1]
        tmp[nWordLen - 1u] = 0u;
        reverse_byte_array(&signature[nByteLen], (uint8_t *)tmp, nByteLen);
        ret = uint32_integer_check(tmp, curve->eccp_n, nWordLen, ECDSA_ZERO_ALL, ECDSA_INTEGER_TOO_BIG,
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

    return ret;
}

/* function: Verify ECDSA Signature step 2(internal API)
 * parameters:
 *     curve ---------------------- input, ecc curve struct pointer, please make sure it is valid
 *     pubKey --------------------- input, public key, U8 big-endian
 *     u1 ------------------------- input, e*(s^(-1)) mod n, U32 little-endian
 *     u2 ------------------------- input, r*(s^(-1)) mod n, U32 little-endian
 *     x1 ------------------------- output, x mod n, U32 little-endian
 * return:
 *     PKE_SUCCESS(success); other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t ecdsa_verify_internal_step_2(const eccp_curve_st *curve, const uint8_t *pubKey, 
#if (defined(PKE_HP) || defined(PKE_UHP))
        const uint32_t *u1, const uint32_t *u2, uint32_t *x1)
#else
        uint32_t *u1, uint32_t *u2, uint32_t *x1)
#endif
{
    uint32_t ret = ECDSA_INVALID_INPUT;
    uint32_t nWordLen, pByteLen, pWordLen;
    uint32_t x[ECCP_MAX_WORD_LEN], y[ECCP_MAX_WORD_LEN];

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

    //check public key
    if(pWordLen > 0u)   //just for static analysis.
    {
        x[pWordLen - 1u] = 0u;
        y[pWordLen - 1u] = 0u;
        reverse_byte_array(pubKey, (uint8_t *)x, pByteLen);
        reverse_byte_array(&pubKey[pByteLen], (uint8_t *)y, pByteLen);
        ret = eccp_check_point(curve, x, y);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_HP) || defined(PKE_UHP))
        ret = eccp_pointMul_Shamir_safe_internal(curve,
                u2, x, y,
                u1, curve->eccp_Gx, curve->eccp_Gy,
                x, y);
#else
        ret = eccp_pointMul_internal(curve, u2, x, y, x, y);
        if(PKE_SUCCESS == ret)
        {
            if(0u == uint32_BigNum_Check_Zero(u1, nWordLen))
            {
                ret = eccp_pointMul_internal(curve, u1, curve->eccp_Gx, curve->eccp_Gy, u1, u2);
                if(PKE_SUCCESS == ret)
                {
                    ret = eccp_pointAdd_safe_internal(curve, x, y, u1, u2, x, y);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //x = x1 mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_mod(x, pWordLen, curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n0, nWordLen, x1);
#else
        ret = pke_mod(x, pWordLen, curve->eccp_n, curve->eccp_n_h, nWordLen, x1);
#endif
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
 *     ECDSA_SUCCESS(success); other(error)
 * caution:
 *     1. the method of getting big integer e_bn from hash value E is based on SEC1 V2.
 */
uint32_t ecdsa_verify(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *pubKey, const uint8_t *signature)
{
    uint32_t ret;
    uint32_t nWordLen = 0u;
    uint32_t r[ECCP_MAX_WORD_LEN], u1[ECCP_MAX_WORD_LEN], u2[ECCP_MAX_WORD_LEN];

    if((NULL == curve) || (NULL == pubKey) || (NULL == signature))
    {
        ret = ECDSA_POINTOR_NULL;
    }
    else if(curve->eccp_p_bitLen > ECCP_MAX_BIT_LEN)
    {
        ret = ECDSA_INVALID_INPUT;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

        ret = ecdsa_verify_internal_step_1(curve, E, EByteLen, signature, r, u1, u2);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = ecdsa_verify_internal_step_2(curve, pubKey, u1, u2, u1);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0 != uint32_BigNumCmp(u1, nWordLen, r, nWordLen))
        {
            ret = ECDSA_VERIFY_FAILED;
        }
        else
        {
            ret = ECDSA_SUCCESS;
        }
    }
    else
    {}

    return ret;
}

#endif

