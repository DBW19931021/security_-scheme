
#include "../../crypto_include/pke/rsa.h"


#if defined(PKE_SEC) && defined(RSA_SEC)

#include "../../crypto_include/crypto_common/utility_sec.h"
#include "../../crypto_include/trng/trng.h"


#ifdef PKE_SEC
#define RSA_SEC_API_OPTIMIZATION
#endif



/* function: RSA_ModExp_with_pub check input
 * parameters:
 *     a -------------------------- input, pointer to the base integer
 *     e -------------------------- input, pointer to public key, the integer e, it must be less than 2^64 here.
 *     d -------------------------- input, pointer to private key, the integer d
 *     n -------------------------- input, pointer to modulus, integer n, please make sure n is odd
 *     out ------------------------ input, pointer to the output, out = a^d mod n
 *     eBitLen -------------------- input, real bit length of uint32_t big integer e, please make sure eBitLen <= 64.
 *     nBitLen -------------------- input, real bit length of uint32_t big integer n
 *     eWordLen ------------------- output, word length of e
 *     dWordLen ------------------- output, word length of d
 *     nWordLen ------------------- output, word length of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 *     2. please make sure 2 <= eBitLen <= 64.
 *     3. a and out can not point to the same buffer.
 */
static uint32_t RSA_ModExp_with_pub_check_input(const uint32_t *a, const uint32_t *e, const uint32_t *d, 
        const uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen, uint32_t *eWordLen, 
        uint32_t *dWordLen, uint32_t *nWordLen)
{
    uint32_t ret = RSA_ERROR_S;
    uint32_t dBitLen;

    if((NULL == a) || (NULL == e) || (NULL == d) || (NULL == n) || (NULL == out))
    {}
    else if((nBitLen > RSA_MAX_BIT_LEN) || (nBitLen < RSA_MIN_BIT_LEN) || (0u != (nBitLen&1u)) 
            || (eBitLen < 2u) || (eBitLen > 64u))  //(eBitLen < nBitLen) always holds
    {}
    else if(0u == (n[0] & 1u))
    {}
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        if(a == out)
        {
            ret = RSA_ERROR_S;
        }
        else
        {
            *eWordLen = GET_WORD_LEN(eBitLen);
            *nWordLen = GET_WORD_LEN(nBitLen);
            dBitLen = get_valid_bits(d, *nWordLen);
            *dWordLen = GET_WORD_LEN(dBitLen);

            if((dBitLen < 2u) || (dBitLen > nBitLen))
            {
                ret = RSA_ERROR_S;
            }
            else
            {
                ret = pke_modexp_check_input(n, d, a, out, *nWordLen, *dWordLen);
            }
        }
    }
    else
    {}

    return ret;
}


/* function: get random 0<r<n and r^(-1) mod n
 * parameters:
 *     n -------------------------- input, uint32_t big integer n, modulus, please make sure n is odd
 *     r -------------------------- output, random integer r, 0<r<n
 *     r_inv ---------------------- output, inverse of r mod n
 *     nWordLen ------------------- input, word length of n
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. r, r_inv, and n have the same word length: nWordLen = ((nBitLen+31)>>5)
 */
static uint32_t RSA_ModExp_with_pub_internal_get_r_and_r_inv(const uint32_t *n, uint32_t *r, 
        uint32_t *r_inv, uint32_t nWordLen, uint32_t nBitLen)
{
    uint32_t ret;

    //get r and r^(-1) mod n
    do {
        ret = uint32_get_rand_big_number_msb_0(r, nBitLen);
        if(0u != ret)
        {
            ret = RSA_ERROR_S;
            break;
        }
        else
        {}

        ret = pke_modinv(n, r, r_inv, nWordLen, nWordLen);
    } while(PKE_NO_MODINV == ret);

    return ret;
}


/* function: RSA_ModExp_with_pub step 1(internal API)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n
 *     e -------------------------- input, uint32_t big integer e, public key, it muist be less than 2^64 here.
 *     d -------------------------- input, uint32_t big integer d, private key
 *     n -------------------------- input, uint32_t big integer n, modulus, please make sure n is odd
 *     out ------------------------ output, out = a^d mod n
 *     cx ------------------------- output, cx = d^e mod n, just for verifying
 *     eWordLen ------------------- input, word length of e
 *     dWordLen ------------------- input, word length of d
 *     nWordLen ------------------- input, word length of n
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a, n, and out have the same word length: nWordLen = ((nBitLen+31)>>5); and eWordLen = (eBitLen+31)>>5
 *     2. a and out can not point to the same buffer.
 */
static uint32_t RSA_ModExp_with_pub_internal_step_1(const uint32_t *a, const uint32_t *e, 
        const uint32_t *d, const uint32_t *n, uint32_t *out, uint32_t *cx, uint32_t eWordLen, 
        uint32_t dWordLen, uint32_t nWordLen, uint32_t nBitLen)
{
    uint32_t ret;
    uint32_t r[RSA_MAX_WORD_LEN];

    //get r and r^(-1) mod n
    ret = RSA_ModExp_with_pub_internal_get_r_and_r_inv(n, r, cx, nWordLen, nBitLen);
    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_pre_calc_mont(n, nBitLen, NULL, NULL);
#else
        ret = pke_pre_calc_mont(n, nBitLen, NULL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //cx = (r^(-1))^e mod n
        ret = pke_modexp_internal(e, cx, cx, nWordLen, eWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //cx = a*(r^(-1))^e mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(a, cx, cx, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_with_pub(n, d, e, cx, cx, nWordLen, dWordLen, eWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get output
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
        ret = pke_modmul_internal(r, cx, out, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_internal(e, out, cx, nWordLen, eWordLen);
    }
    else
    {}

    return ret;
}


/* function: RSA_ModExp_with_pub step 2(internal API)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n
 *     cx ------------------------- input, cx = d^e mod n, just for verifying
 *     nWordLen ------------------- input, word length of a, cx
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
static uint32_t RSA_ModExp_with_pub_internal_step_2(const uint32_t *a, const uint32_t *cx, 
        uint32_t nWordLen)
{
    uint32_t ret;
    uint32_t t;

    //check output three or more times
    (void)get_rand_fast((uint8_t *)&t, 4u);

    if(0u != uint32_cmp_sec(cx, a, nWordLen, (uint8_t)(t)))
    {
        ret = RSA_ERROR_S;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        uint32_sleep(nWordLen|((t>>1)&0xFFu), (uint8_t)(t>>9));

        if(0u != uint32_cmp_sec(cx, a, nWordLen, (uint8_t)(t>>10)))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        uint32_sleep(nWordLen|((t>>11)&0xFFu), (uint8_t)(t>>19));

        if(0u != uint32_cmp_sec(cx, a, nWordLen, (uint8_t)(t>>20)))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}

        uint32_sleep(nWordLen|((t>>21)&0xFFu), (uint8_t)(t>>29));
    }
    else
    {}

    return ret;
}


/* function: out = a^d mod n
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n
 *     e -------------------------- input, uint32_t big integer e, public key, it muist be less than 2^64 here.
 *     d -------------------------- input, uint32_t big integer d, private key
 *     n -------------------------- input, uint32_t big integer n, modulus, please make sure n is odd
 *     out ------------------------ output, out = a^d mod n
 *     eBitLen  ------------------- input, real bit length of uint32_t big integer e, please make sure eBitLen <= 64.
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n
 * return: RSA_SUCCESS_S(success), other(error)
 * caution:
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 *     2. please make sure 2 <= eBitLen <= 64.
 *     3. a and out can not point to the same buffer.
 */
uint32_t RSA_ModExp_with_pub(const uint32_t *a, const uint32_t *e, const uint32_t *d, 
        const uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t ret, is_over = 0u;
    uint32_t r[RSA_MAX_WORD_LEN];
    uint32_t cx[RSA_MAX_WORD_LEN];
    uint32_t eWordLen, dWordLen, nWordLen = 0U;

    ret = RSA_ModExp_with_pub_check_input(a, e, d, n, out, eBitLen, nBitLen, &eWordLen, 
            &dWordLen, &nWordLen);
    if(PKE_FINISHED == ret)
    {
        is_over = 1u;
        ret = PKE_SUCCESS;
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        ret = RSA_ModExp_with_pub_internal_step_1(a, e, d, n, out, cx, eWordLen, dWordLen, nWordLen, nBitLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //check output three or more times
        ret = RSA_ModExp_with_pub_internal_step_2(a, cx, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = RSA_SUCCESS_S;
    }
    else
    {
        ret = RSA_ERROR_S;

        (void)get_rand_fast((uint8_t *)out, nWordLen << 2);
    }

    (void)get_rand_fast((uint8_t *)r,  nWordLen << 2);
    (void)get_rand_fast((uint8_t *)cx, nWordLen << 2);

    return ret;
}



/* function: RSA_CRTModExp_with_pub check input
 * parameters:
 *     a -------------------------- input, pointer to base number a
 *     p -------------------------- input, pointer to prime number p
 *     q -------------------------- input, pointer to prime number q
 *     dp ------------------------- input, pointer to dp = e^(-1) mod (p-1)
 *     dq ------------------------- input, pointer to dq = e^(-1) mod (q-1)
 *     u -------------------------- input, pointer to u = q^(-1) mod p
 *     e -------------------------- input, pointer to public key e
 *     out ------------------------ input/output, pointer to out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 *     eBitLen  ------------------- input, real bit length of uint32_t big integer e, please make sure 2 <= eBitLen <= 64.
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 *     n -------------------------- output, n=p*q
 *     eWordLen ------------------- output, word length of e
 *     nWordLen ------------------- output, word length of n
 *     pBitLen -------------------- output, bit length of p,q
 *     pWordLen ------------------- output, word length of p,q,dp,dq,u
 *     is_over -------------------- output, indicate whether over
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure 2 <= eBitLen <= 64.
 *     2. a and out can not point to the same buffer
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_check_input(const uint32_t *a, const uint32_t *p, 
        const uint32_t *q, const uint32_t *dp, const uint32_t*dq, const uint32_t *u, 
        const uint32_t *e, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen, uint32_t *n, 
        uint32_t *eWordLen, uint32_t *nWordLen, uint32_t *pBitLen, uint32_t *pWordLen, 
        uint32_t *is_over)
{
    uint32_t ret = RSA_ERROR_S;
    int32_t flag;

    *is_over = 0u;

    if((NULL == a) || (NULL == p) || (NULL == q) || (NULL == dp) || (NULL == dq) || (NULL == u) || (NULL == e) || (NULL == out))
    {}
    else if(a == out)
    {}
    else if((nBitLen > RSA_MAX_BIT_LEN) || (nBitLen < RSA_MIN_BIT_LEN) || (0u != (nBitLen&1u)) || (eBitLen < 2u) || (eBitLen > 64u))
    {}
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        *eWordLen = GET_WORD_LEN(eBitLen);
        *nWordLen = GET_WORD_LEN(nBitLen);
        *pBitLen  = nBitLen>>1;
        *pWordLen = GET_WORD_LEN(*pBitLen);

        //get tmp_n = p*q
        ret = pke_mul_internal(p, q, n, *pWordLen, *pWordLen, *nWordLen);
        if((PKE_SUCCESS != ret) || (0u == (n[0] & 1u)))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //a should be in [0,n]
        flag = uint32_BigNumCmp(a, *nWordLen, n, *nWordLen);
        if(flag > 0)
        {
            ret = RSA_ERROR_S;
        }
        else if((1u == uint32_BigNum_Check_Zero(a, *nWordLen)) || (0 == flag))
        {
            //if a is 0 or n, the output is 0
            uint32_clear(out, *nWordLen);
            *is_over = 1u;
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


/* function: get random r and masked_a = a*(r^(-1))^e mod n
 * parameters:
 *     n -------------------------- input, uint32_t big integer, RSA public modulus
 *     a -------------------------- input, base number
 *     e -------------------------- input, RSA public key e, here e < 2^64
 *     r -------------------------- output, random number r < n, occupies pWordLen words
 *     masked_a ------------------- output, masked_a = a*(r^(-1))^e mod n
 *     pWordLen ------------------- input, word length of r
 *     nBitLen -------------------- input, bit length of n
 *     nWordLen ------------------- input, word length of n
 *     n_step --------------------- input, step of n
 *     eWordLen ------------------- input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_r_and_masked_a(const uint32_t *n, 
        const uint32_t *a, const uint32_t *e, uint32_t *r, uint32_t *masked_a, 
        uint32_t pWordLen, uint32_t nBitLen, uint32_t nWordLen, uint32_t n_step, 
        uint32_t eWordLen)
{
    uint32_t ret;

    ret = pke_pre_calc_mont(n, nBitLen, NULL);
    if(PKE_SUCCESS == ret)
    {
        //get r and r^(-1) mod n
        do {
            ret = get_rand((uint8_t *)r, pWordLen << 2);
            if(TRNG_SUCCESS != ret)
            {
                ret = RSA_ERROR_S;
                break;
            }
            else
            {}

#ifndef RSA_SEC_API_OPTIMIZATION
            ret = pke_modinv_internal(r, masked_a, nWordLen, pWordLen, n_step);
#else
            ret = pke_modinv_internal(r, (uint32_t *)(rPKE_B(1u,n_step)), nWordLen, pWordLen, n_step);
#endif
        } while(PKE_NO_MODINV == ret);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get (r^(-1))^e mod n
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modexp_internal(e, masked_a, masked_a, nWordLen, eWordLen);
#else
        ret = pke_modexp_internal(e, (uint32_t *)(rPKE_B(1u,n_step)), (uint32_t *)(rPKE_A(1u,n_step)), nWordLen, eWordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //masked_a = a*(r^(-1))^e mod n
#if (defined(PKE_LP) || defined(PKE_SECURE))
        pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modmul_internal(masked_a, a, masked_a, nWordLen);
#else
        ret = pke_modmul_internal((uint32_t *)(rPKE_A(1u,n_step)), a, masked_a, nWordLen);
#endif
    }
    else
    {}

    return ret;
}


/* function: get 64bit random k, y, and make sure k < y, and y is odd 
 * parameters:
 *     k -------------------------- output, 64bit random k, k<y
 *     y -------------------------- output, 64bit random y, k<y, and y is odd
 *     pWordLen ------------------- input, word length of k, y buffer
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_k_y(uint32_t *k, uint32_t *y, uint32_t pWordLen)
{
    uint32_t ret;
    int32_t flag = 0;

    do {
        ret = get_rand((uint8_t *)k, 16u);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {
            ret = RSA_ERROR_S;
            break;
        }

        flag = uint32_BigNumCmp_sec(k, 2u, &k[2u], 2u);
    } while(0 == flag);

    if(PKE_SUCCESS == ret)
    {
        if(1 == flag)
        {
            uint32_copy(y, k, 2u);
            uint32_copy(k, &k[2u], 2u);
        }
        else
        {
            uint32_copy(y, &k[2u], 2u);
            uint32_copy(k, k, 2u);      //this is redundant
        }

        y[0] |= 0x01u;

        uint32_clear(&k[2u], pWordLen-2u);
        uint32_clear(&y[2u], pWordLen-2u);
    }
    else
    {}

    return ret;
}


/* function: get masked_dp = dp+k*h1+k*(p-1-h1) = dp+k(p-1)
 * parameters:
 *     p -------------------------- input, uint32_t big integer, RSA prime p
 *     dp ------------------------- input, uint32_t big integer, dp = e^(-1) mod (p-1)
 *     k -------------------------- input, random number of 64 bits, occupies pWordLen words
 *     h1 ------------------------- input, temporary buffer of pWordLen words
 *     masked_dp ------------------ output, masked_dp = dp+k*h1+k*(p-1-h1) = dp+k(p-1)
 *     pBitLen -------------------- input, bit length of p
 *     pWordLen ------------------- input, word length of p
 *     p_step --------------------- input, step of p
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_masked_dp(const uint32_t *p, 
        const uint32_t *dp, const uint32_t *k, uint32_t *h1, uint32_t *masked_dp, 
        uint32_t pBitLen, uint32_t pWordLen, uint32_t p_step)
{
    uint32_t ret, tmp_step;
    uint32_t temp_WordLen = pWordLen + 2u;

    //get random big number h1 < p (and h1 < p-1)
    ret = uint32_get_rand_big_number_msb_0(h1, pBitLen);
    if(0u != ret)
    {
        ret = RSA_ERROR_S;
    }
    else
    {
        //get k*h1
        ret = pke_mul_internal(h1, k, masked_dp, pWordLen, pWordLen, temp_WordLen);
    }

    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //h1 = (p-1) - h1
#if 0
        p[0] -= 1u;
        (void)pke_sub(p, h1, h1, pWordLen);
        p[0] |= 1u;
#else
        uint32_copy((uint32_t *)(rPKE_A(1u,p_step)), p, pWordLen);
        *(uint32_t *)(rPKE_A(1u,p_step)) &= (~0x01u);
        ret = pke_sub((uint32_t *)(rPKE_A(1u,p_step)), h1, h1, pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_mul_internal(h1, k, tmpp, pWordLen, pWordLen, temp_WordLen);
#else
        ret = pke_mul_internal(h1, k, (uint32_t *)(rPKE_A(1u,tmp_step)), pWordLen, pWordLen, temp_WordLen);
#endif
    }
    else
    {}
#endif

    if(PKE_SUCCESS == ret)
    {
        //get k*h1+k*(p-1-h1)
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_add(tmpp, masked_dp, tmpp, temp_WordLen);
#else
        ret = pke_add((uint32_t *)(rPKE_A(1u,tmp_step)), masked_dp, (uint32_t *)(rPKE_A(1u,tmp_step)), temp_WordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get k*h1+k*(p-1-h1)+dp = dp+k(p-1)
#ifndef RSA_SEC_API_OPTIMIZATION
        uint32_copy(masked_dp, dp, pWordLen);
        uint32_clear(masked_dp+pWordLen, 2u);
        ret = pke_add(tmpp, masked_dp, masked_dp, temp_WordLen);
#else
        uint32_copy((uint32_t *)(rPKE_B(1u,tmp_step)), dp, pWordLen);
        uint32_clear(&(rPKE_B(1u,tmp_step))[pWordLen], 2u);
        ret = pke_add((uint32_t *)(rPKE_A(1u,tmp_step)), (uint32_t *)(rPKE_B(1u,tmp_step)), masked_dp, temp_WordLen);
#endif
    }
    else
    {}

    return ret;
}


/* function: get masked_p = p1*y+(p-p1)*y = py
 * parameters:
 *     p -------------------------- input, uint32_t big integer, RSA prime p
 *     y -------------------------- input, random odd number of 64 bits, occupies pWordLen words
 *     p1 ------------------------- output, p1, occupies pWordLen words
 *     p2 ------------------------- output, p-p1, occupies pWordLen words
 *     masked_p ------------------- output, masked_p = p1*y+(p-p1)*y = py
 *     pBitLen -------------------- input, bit length of p
 *     pWordLen ------------------- input, word length of p
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_masked_p(const uint32_t *p, const uint32_t *y, 
        uint32_t *p1, uint32_t *p2, uint32_t *masked_p, uint32_t pBitLen, uint32_t pWordLen)
{
    uint32_t ret, tmp_step;
    uint32_t temp_WordLen = pWordLen + 2u;

    //dp + k(p-1) < (k+1)(p-1) <= y(p-1) < py

    //get random big number p1 < p
    ret = uint32_get_rand_big_number_msb_0(p1, pBitLen);
    if(0u != ret)
    {
        ret = RSA_ERROR_S;
    }
    else
    {
        //get p1*y
        ret = pke_mul_internal(p1, y, masked_p, pWordLen, pWordLen, temp_WordLen);
    }

    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //get p2 = p-p1
        ret = pke_sub(p, p1, p2, pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get (p-p1)y
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_mul_internal(p2, y, tmp_buf, pWordLen, pWordLen, temp_WordLen);
#else
        ret = pke_mul_internal(p2, y, (uint32_t *)(rPKE_A(1u,tmp_step)), pWordLen, pWordLen, temp_WordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get p1*y + (p-p1)y = py
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_add(tmp_buf, tmpp, tmpp, temp_WordLen);
#else
        ret = pke_add((uint32_t *)(rPKE_A(1u,tmp_step)), masked_p, masked_p, temp_WordLen);
#endif
    }
    else
    {}

    return ret;
}


/* function: get m1 or m2, i.e. out = (masked_base^masked_exp mod masked_p) mod p
 * parameters:
 *     p -------------------------- input, uint32_t big integer, RSA prime p
 *     pBitLen -------------------- input, bit length of p
 *     pWordLen ------------------- input, word length of p
 *     p_step --------------------- input, step of p
 *     masked_p ------------------- input, masked p = py, occupies pWordLen+2 words
 *     masked_exp ----------------- input, masked exponent dp = dp+k(p-1), occupies pWordLen+2 words
 *     masked_base ---------------- input, masked base number, occupies nWordLen words
 *     masked_pWordLen ------------ output, real word length of masked_p
 *     masked_baseWordLen --------- input, word length of masked_base
 *     out ------------------------ output, occupies pWordLen words
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. masked_exp will be erased after calling this function
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_m1_or_m2(const uint32_t *p, uint32_t pBitLen, 
        uint32_t pWordLen, uint32_t p_step, const uint32_t *masked_p, uint32_t *masked_exp, 
        const uint32_t *masked_base, uint32_t *masked_pWordLen, uint32_t masked_baseWordLen, 
        uint32_t *out)
{
    uint32_t ret, tmp_step;
    uint32_t bitLen;

    bitLen = get_valid_bits(masked_p, pWordLen+2u);
    *masked_pWordLen = GET_WORD_LEN(bitLen);
#if (defined(PKE_LP) || defined(PKE_SECURE))
    ret = pke_pre_calc_mont(masked_p, bitLen, NULL, NULL);
#else
    ret = pke_pre_calc_mont(masked_p, bitLen, NULL);
#endif
    if(PKE_SUCCESS == ret)
    {
        //tmp_buf = cx mod masked_p
        tmp_step = pke_get_operand_bytes();
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_mod(masked_base, masked_baseWordLen, masked_p, (uint32_t *)(rPKE_B(0u,tmp_step)), *masked_pWordLen, tmp_buf);
#else
        ret = pke_mod(masked_base, masked_baseWordLen, masked_p, (uint32_t *)(rPKE_B(0u,tmp_step)), *masked_pWordLen, (uint32_t *)(rPKE_B(1u,tmp_step)));
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //masked_exp = tmp_buf^masked_exp mod masked_p
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modexp_ladder_internal(masked_exp, tmp_buf, masked_exp, *masked_pWordLen, *masked_pWordLen);  //masked_exp < masked_p
#else
        ret = pke_modexp_ladder_internal(masked_exp, (uint32_t *)(rPKE_B(1u,tmp_step)), masked_exp, *masked_pWordLen, *masked_pWordLen);  //masked_exp < masked_p
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_pre_calc_mont(p, pBitLen, NULL, NULL);
#else
        ret = pke_pre_calc_mont(p, pBitLen, NULL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = masked_exp mod p
        ret = pke_mod(masked_exp, *masked_pWordLen, p, (uint32_t *)(rPKE_B(0u,p_step)), pWordLen, out);
    }
    else
    {}

    return ret;
}


/* function: calculate m, i.e. m = (masked_base^masked_exp mod masked_p) mod p
 * parameters:
 *     k -------------------------- input/output, as input, this is 64bit random k, k<y, as output, this is p2<p, occupies pWordLen words
 *     y -------------------------- input, 64bit random y, k<y, and y is odd, this is a tmporary buffer, occupies pWordLen words
 *     p -------------------------- input, uint32_t big integer, RSA prime p
 *     dp ------------------------- input, uint32_t big integer, dp = e^(-1) mod (p-1)
 *     masked_p ------------------- output, masked p = py, occupies pWordLen+2 words
 *     masked_dp ------------------ output, masked_dp = dp+k*h1+k*(p-1-h1) = dp+k(p-1)
 *     masked_base ---------------- input, masked base number, occupies nWordLen words
 *     p1 ------------------------- output, p1<p, occupies pWordLen words
 *     m -------------------------- output, m1 or m2, occupies pWordLen words
 *     masked_pWordLen ------------ output, real word length of masked_p
 *     pBitLen -------------------- input, bit length of p
 *     pWordLen ------------------- input, word length of p
 *     p_step --------------------- input, step of p
 *     nWordLen ------------------- input, word length of masked_base
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. as output, k = p2 = p-p1
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_calc_m1_or_m2(uint32_t *k, uint32_t *y, const uint32_t *p, 
        const uint32_t *dp, uint32_t *masked_p, uint32_t *masked_dp, const uint32_t *masked_base, 
        uint32_t *p1, uint32_t *m, uint32_t *masked_pWordlen, uint32_t pBitLen, uint32_t pWordLen, 
        uint32_t p_step, uint32_t nWordLen)
{
    uint32_t ret;

    //get 64bit random k, y, and make sure k < y, and y is odd
    ret = RSA_CRTModExp_with_pub_get_k_y(k, y, pWordLen);
    if(PKE_SUCCESS == ret)
    {
        //get dp1 = dp + k*h1 + k*h2 = dp + k(p-1)
        ret = RSA_CRTModExp_with_pub_get_masked_dp(p, dp, k, p1, masked_dp, pBitLen, pWordLen, p_step);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get py = p1*y + p2*y = py
        ret = RSA_CRTModExp_with_pub_get_masked_p(p, y, p1, k, masked_p, pBitLen, pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get m1
        ret = RSA_CRTModExp_with_pub_get_m1_or_m2(p, pBitLen, pWordLen, p_step, masked_p, masked_dp, 
                masked_base, masked_pWordlen, nWordLen, m);
    }
    else
    {}

    return ret;
}


/* function: RSA_CRTModExp_with_pub_get_result step 1
 * parameters:
 *     m1 ------------------------- input, uint32_t big integer masked_base^dp mod p, occupies (pWordLen+2) words
 *     m2 ------------------------- input, uint32_t big integer masked_base^dq mod q, occupies (pWordLen+2) words
 *     u -------------------------- input, uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     masked_p ------------------- input, uint32_t big integer py, (pWordLen+2u) words
 *     t1 ------------------------- input, temporary buffer t1, (pWordLen+2u) words
 *     t2 ------------------------- input, temporary buffer t2, (pWordLen+2u) words
 *     t3 ------------------------- input, temporary buffer t3, (pWordLen+2u) words
 *     masked_pWordLen ------------ input, real word length of masked_p
 *     pBitLen -------------------- input, bit length of prime p
 *     pWordLen ------------------- input, word length of prime p
 *     p_step --------------------- input, step of p
 *     n_step --------------------- input, step of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. output is in B1
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_result_step_1(uint32_t *m1, uint32_t *m2, 
        const uint32_t *u, const uint32_t *p, const uint32_t *masked_p, uint32_t *t1, 
        uint32_t *t2, uint32_t *t3, uint32_t masked_pWordLen, uint32_t pBitLen, 
        uint32_t pWordLen, uint32_t p_step, uint32_t n_step)
{
    uint32_t ret, tmp_step;
    uint32_t bitLen, uWordLen;

    //get random big number t1 < u
    bitLen = get_valid_bits(u, pWordLen);
    uWordLen = GET_WORD_LEN(bitLen);
    ret = uint32_get_rand_big_number_msb_0(t1, bitLen);
    if(0u != ret)
    {
        ret = RSA_ERROR_S;
    }
    else
    {
        //u = t1 + t2
        uint32_clear(&t1[uWordLen], masked_pWordLen - uWordLen);
        uint32_clear(&t2[uWordLen], masked_pWordLen - uWordLen);
        ret = pke_sub(u, t1, t2, uWordLen);
    }

    if(PKE_SUCCESS == ret)
    {
        //t3 = (m1 - m2) mod masked_p
        uint32_clear(&m1[pWordLen], masked_pWordLen - pWordLen);
        uint32_clear(&m2[pWordLen], masked_pWordLen - pWordLen);
        ret = pke_modsub(masked_p, m1, m2, t3, masked_pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        tmp_step = pke_get_operand_bytes();

        //t1 = t3*t1 mod masked_p
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modmul(masked_p, t1, t3, t1, masked_pWordLen);
#else
        ret = pke_modmul((uint32_t *)(rPKE_A(0u,tmp_step)), t1, t3, t1, masked_pWordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //t2 = t3*t2 mod masked_p
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modmul_internal(t2, t3, t2, masked_pWordLen);
#else
        ret = pke_modmul_internal(t2, (uint32_t *)(rPKE_B(1u,tmp_step)), (uint32_t *)(rPKE_A(1u,tmp_step)), masked_pWordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //t1 = m*u mod masked_p
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modadd(masked_p, t2, t1, t1, tempp_WordLen);
#else
        ret = pke_mod_add_sub_mul_internal((uint32_t *)(rPKE_A(1u,tmp_step)), t1, t1, masked_pWordLen, tmp_step, MICROCODE_MODADD);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_pre_calc_mont(p, pBitLen, NULL, NULL);
#else
        ret = pke_pre_calc_mont(p, pBitLen, NULL);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //get t1 mod p
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_mod(t1, masked_pWordLen, p, (uint32_t *)(rPKE_B(0u,p_step)), pWordLen, cx);
#else
        ret = pke_mod(t1, masked_pWordLen, p, (uint32_t *)(rPKE_B(0u,p_step)), pWordLen, (uint32_t *)(rPKE_B(1u,n_step)));
#endif
    }
    else
    {}

    return  ret;
}


/* function: RSA_CRTModExp_with_pub_get_result step 2
 * parameters:
 *     q1 ------------------------- input, uint32_t big integer, less than prime q
 *     q2 ------------------------- input, uint32_t big integer, q-q1
 *     m2 ------------------------- input, masked_a^dq mod q
 *     r -------------------------- input, random big number r, occupies pWordLen words
 *     n -------------------------- input, RSA public modulus
 *     t -------------------------- output, result of RSA_CRTModExp_with_pub(), occupies nWordLen words
 *     pWordLen ------------------- input, word length of prime p
 *     nWordLen ------------------- input, word length of n
 *     n_step --------------------- input, step of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. input m is in B1, occupies pWordLen words
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_result_step_2(const uint32_t *q1, const uint32_t *q2, 
        const uint32_t *m2, uint32_t *r, const uint32_t *n, uint32_t *t, uint32_t pWordLen, 
        uint32_t nWordLen, uint32_t n_step)
{
    uint32_t ret;

    //input m is in B1
#ifndef RSA_SEC_API_OPTIMIZATION
    ret = pke_mul_internal(q1, cx, t, pWordLen, pWordLen, nWordLen);
#else
    ret = pke_mul_internal(q1, (uint32_t *)(rPKE_B(1u,n_step)), t, pWordLen, pWordLen, nWordLen);
#endif
    if(PKE_SUCCESS == ret)
    {
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_mul_internal(q2, cx, cx, pWordLen, pWordLen, nWordLen);
#else
        ret = pke_mul_internal(q2, (uint32_t *)(rPKE_B(1u,n_step)), (uint32_t *)(rPKE_A(1u,n_step)), pWordLen, pWordLen, nWordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_add(cx, t, t, nWordLen);
#else
        ret = pke_add((uint32_t *)(rPKE_A(1u,n_step)), t, (uint32_t *)(rPKE_A(1u,n_step)), nWordLen);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#ifndef RSA_SEC_API_OPTIMIZATION
        uint32_copy(cx, m2, pWordLen);
        uint32_clear(&cx[pWordLen], nWordLen - pWordLen);
        ret = pke_add(tmp_buf, cx, tmp_buf, nWordLen);
#else
        uint32_copy((uint32_t *)(rPKE_B(1u,n_step)), m2, pWordLen);
        uint32_clear(&(rPKE_B(1u,n_step))[pWordLen], nWordLen - pWordLen);
        ret = pke_add((uint32_t *)(rPKE_A(1u,n_step)), (uint32_t *)(rPKE_B(1u,n_step)), (uint32_t *)(rPKE_B(1u,n_step)), nWordLen);
#endif
    }
    else
    {}

    //get output
    if(PKE_SUCCESS == ret)
    {
        uint32_clear(&r[pWordLen], nWordLen - pWordLen);
#ifndef RSA_SEC_API_OPTIMIZATION
        ret = pke_modmul(n, r, tmp_buf, tmp_buf, nWordLen);
#else
        ret = pke_modmul(n, r, (uint32_t *)(rPKE_B(1u,n_step)), t, nWordLen);
#endif
    }
    else
    {}

    return ret;
}


/* function: RSA_CRTModExp_with_pub get result
 * parameters:
 *     m1 ------------------------- input, uint32_t big integer masked_base^dp mod p, occupies (pWordLen+2) words
 *     m2 ------------------------- input, uint32_t big integer masked_base^dq mod q, occupies (pWordLen+2) words
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     q1 ------------------------- input, random q1<q
 *     q2 ------------------------- input, random q2=q-q1
 *     masked_p ------------------- input, uint32_t big integer py, (pWordLen+2u) words
 *     t1 ------------------------- input, temporary buffer t1, (pWordLen+2u) words
 *     t2 ------------------------- input, temporary buffer t2, (pWordLen+2u) words
 *     out ------------------------ output, the result, nWordLen words
 *     n -------------------------- input, RSA public modulus
 *     r -------------------------- input, random big number r, occupies pWordLen words
 *     masked_pWordLen ------------ input, real word length of masked_p
 *     pBitLen -------------------- input, bit length of prime p
 *     pWordLen ------------------- input, word length of prime p
 *     p_step --------------------- input, step of p
 *     nWordLen ------------------- input, word length of n
 *     n_step --------------------- input, step of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_get_result(uint32_t *m1, uint32_t *m2, const uint32_t *p, 
        const uint32_t *u, const uint32_t * q1, const uint32_t *q2, const uint32_t *masked_p, 
        uint32_t *t1, uint32_t *t2, uint32_t *out, const uint32_t *n, uint32_t *r, 
        uint32_t masked_pWordLen, uint32_t pBitLen, uint32_t pWordLen, uint32_t p_step, 
        uint32_t nWordLen, uint32_t n_step)
{
    uint32_t ret;

    //get m
    ret = RSA_CRTModExp_with_pub_get_result_step_1(m1, m2, u, p, masked_p, t1, t2, out,
        masked_pWordLen, pBitLen, pWordLen, p_step, n_step);
    if(PKE_SUCCESS == ret)
    {
        //get result
        ret = RSA_CRTModExp_with_pub_get_result_step_2(q1, q2, m2, r, n, out, pWordLen, 
                nWordLen, n_step);
    }
    else
    {}

    return ret;
}


/* function: RSA_CRTModExp_with_pub check output
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n=pq
 *     e -------------------------- input, uint32_t big integer e, public key, it is less than 2^64.
 *     result --------------------- input, result of RSA_CRTModExp_with_pub 
 *     n_step --------------------- input, step of n
 *     nWordLen ------------------- input, word length of n
 *     eWordLen ------------------- input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. 
 */
FLAG_STATIC uint32_t RSA_CRTModExp_with_pub_check_output(const uint32_t *a, const uint32_t *e, 
        const uint32_t *result, uint32_t n_step, uint32_t nWordLen, uint32_t eWordLen)
{
    uint32_t ret;
    uint32_t tmp;

    //check output three or more times
#ifndef RSA_SEC_API_OPTIMIZATION
    ret = pke_modexp_internal(e, result, t, nWordLen, eWordLen);
#else
    ret = pke_modexp_internal(e, result, (uint32_t *)(rPKE_A(1u,n_step)), nWordLen, eWordLen);
#endif
    if(PKE_SUCCESS == ret)
    {
        (void)get_rand_fast((uint8_t *)&tmp, 4u);

        if(0u != uint32_cmp_sec((uint32_t *)(rPKE_A(1u,n_step)), a, nWordLen, (uint8_t)tmp))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}

        uint32_sleep(nWordLen|((tmp>>1)&0xFFu), (uint8_t)(tmp>>9));
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_cmp_sec((uint32_t *)(rPKE_A(1u,n_step)), a, nWordLen, (uint8_t)(tmp>>10)))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}

        uint32_sleep(nWordLen|((tmp>>11)&0xFFu), (uint8_t)(tmp>>19));
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != uint32_cmp_sec((uint32_t *)(rPKE_A(1u,n_step)), a, nWordLen, (uint8_t)(tmp>>20)))
        {
            ret = RSA_ERROR_S;
        }
        else
        {}

        uint32_sleep(nWordLen|((tmp>>21)&0xFFu), (uint8_t)(tmp>>29));
    }
    else
    {}

    return ret;
}


/* function: out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, please make sure a < n=pq
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     e -------------------------- input, uint32_t big integer e, public key, it must be less than 2^64 here.
 *     out ------------------------ output, out = a^d mod n
 *     eBitLen  ------------------- input, real bit length of uint32_t big integer e, please make sure 2 <= eBitLen <= 64.
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 * return: RSA_SUCCESS_S(success), other(error)
 * caution:
 *     1. a and out have the same word length:((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:((nBitLen/2+31)>>5)
 *     2. please make sure 2 <= eBitLen <= 64.
 *     3. a and out can not point to the same buffer
 */
uint32_t RSA_CRTModExp_with_pub(const uint32_t *a, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t*dq, const uint32_t *u, const uint32_t *e,
        uint32_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t ret;
    //caution: y and k both occupy RSA_MAX_WORD_LEN/2 words, but tmp_buf is also used for mod py operations,
    //it need RSA_MAX_WORD_LEN/2 + 2 words, to keep k, so tmp_buf needs RSA_MAX_WORD_LEN + 2 words.
    uint32_t tmp_buf[RSA_MAX_WORD_LEN + 2u];
#if 1
    uint32_t *tmp_n = out;
    uint32_t *y = tmp_buf;
    uint32_t *k = &tmp_buf[(RSA_MAX_WORD_LEN>>1) + 2u];
#else
    uint32_t tmp_n[RSA_MAX_WORD_LEN];
    uint32_t y[(RSA_MAX_WORD_LEN>>1)];
    uint32_t k[(RSA_MAX_WORD_LEN>>1)];
#endif
    uint32_t h1[(RSA_MAX_WORD_LEN>>1)];
    uint32_t dp1[(RSA_MAX_WORD_LEN>>1) + 2u];
    uint32_t py[(RSA_MAX_WORD_LEN>>1) + 2u];
    uint32_t qy[(RSA_MAX_WORD_LEN>>1) + 2u];
    uint32_t r[(RSA_MAX_WORD_LEN>>1)+RSA_MAX_WORD_LEN];
    uint32_t m1[(RSA_MAX_WORD_LEN>>1) + 2u];
#if 1
    uint32_t *cx = &r[RSA_MAX_WORD_LEN>>1];
    uint32_t *m2 = qy;
#else
    uint32_t cx[RSA_MAX_WORD_LEN];
    uint32_t m2[(RSA_MAX_WORD_LEN>>1) + 2u];
#endif

    uint32_t pBitLen;
    uint32_t eWordLen, nWordLen = 0u, pWordLen = 0u;
    uint32_t py_WordLen, qy_WordLen;
    uint32_t p_step, n_step;
    uint32_t is_over = 0u;

    ret = RSA_CRTModExp_with_pub_check_input(a, p, q, dp, dq, u, e, out, eBitLen, nBitLen, 
            tmp_n, &eWordLen, &nWordLen, &pBitLen, &pWordLen, &is_over);
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get hardware step
        n_step = pke_set_operand_width(nBitLen);
        p_step = pke_set_operand_width(pBitLen);

        //get r and cx = a*(r^(-1))^e mod n
        ret = RSA_CRTModExp_with_pub_get_r_and_masked_a(tmp_n, a, e, r, cx, 
                pWordLen, nBitLen, nWordLen, n_step, eWordLen);
    }
    else
    {}

#if 1
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get m1
        ret = RSA_CRTModExp_with_pub_calc_m1_or_m2(k, y, p, dp, py, dp1, cx, h1, m1, 
                &py_WordLen, pBitLen, pWordLen, p_step, nWordLen);
    }
    else
    {}
#else
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get 64bit random k, y, and make sure k < y, and y is odd
        ret = RSA_CRTModExp_with_pub_get_k_y(k, y, pWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get dp1 = dp + k*h1 + k*h2 = dp + k(p-1)
        ret = RSA_CRTModExp_with_pub_get_masked_dp(p, dp, k, h1, dp1, pBitLen, pWordLen, p_step);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //py = p1*y + p2*y = py
        ret = RSA_CRTModExp_with_pub_get_masked_p(p, y, h1, k, py, pBitLen, pWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get m1
        ret = RSA_CRTModExp_with_pub_get_m1_or_m2(p, pBitLen, pWordLen, p_step, tmpp, dp1, 
                cx, &py_WordLen, nWordLen, m1);
    }
    else
    {}
#endif

#if 1
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        ret = RSA_CRTModExp_with_pub_calc_m1_or_m2(k, y, q, dq, qy, dp1, cx, h1, m2, 
                &qy_WordLen, pBitLen, pWordLen, p_step, nWordLen);
    }
    else
    {}
#else
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get 64bit random k, y, and make sure k < y, and y is odd
        ret = RSA_CRTModExp_with_pub_get_k_y(k, y, pWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get dq1 = dq + k*h1 + k*h2 = dq + k(q-1)
        ret = RSA_CRTModExp_with_pub_get_masked_dp(q, dq, k, h1, dp1, pBitLen, pWordLen, p_step);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get qy = q1*y + q2*y = qy
        ret = RSA_CRTModExp_with_pub_get_masked_p(q, y, h1, k, qy, pBitLen, pWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get m2
        ret = RSA_CRTModExp_with_pub_get_m1_or_m2(q, pBitLen, pWordLen, p_step, tmpq, dp1, 
                cx, &qy_WordLen, nWordLen, m2);
    }
    else
    {}
#endif

#if 1
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get result
        ret = RSA_CRTModExp_with_pub_get_result(m1, m2, p, u, k, h1, py, dp1, cx, 
                tmp_buf, tmp_n, r, py_WordLen, pBitLen, pWordLen, p_step, nWordLen, n_step);
    }
    else
    {}
#else
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get m
        ret = RSA_CRTModExp_with_pub_get_result_step_1(m1, m2, u, p, py, dp1, cx, tmp_buf,
            py_WordLen, pBitLen, pWordLen, p_step, n_step);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //get result
        ret = RSA_CRTModExp_with_pub_get_result_step_2(k, h1, m2, r, tmp_n, tmp_buf, pWordLen, 
                nWordLen, n_step);
    }
    else
    {}
#endif
    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //check output three or more times
        ret = RSA_CRTModExp_with_pub_check_output(a, e, tmp_buf, n_step, nWordLen, eWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        //output result
        uint32_copy(out, tmp_buf, nWordLen);
        is_over = 1u;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = RSA_SUCCESS_S;
    }
    else
    {
        ret = RSA_ERROR_S;
    }

    (void)get_rand_fast((uint8_t *)y,   (pWordLen+2u)<<2);
    (void)get_rand_fast((uint8_t *)k,   (pWordLen)<<2);
    (void)get_rand_fast((uint8_t *)h1,  (pWordLen)<<2);
    (void)get_rand_fast((uint8_t *)dp1, (pWordLen+2u)<<2);
    (void)get_rand_fast((uint8_t *)py,  (pWordLen+2u)<<2);
    (void)get_rand_fast((uint8_t *)qy,  (pWordLen+2u)<<2);
    (void)get_rand_fast((uint8_t *)r,   (pWordLen)<<2);
    (void)get_rand_fast((uint8_t *)cx,  (nWordLen)<<2);
    (void)get_rand_fast((uint8_t *)m1,  (pWordLen+2u)<<2);

    return ret;
}

#endif

