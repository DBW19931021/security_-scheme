
#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_RSA

#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_hal/pke_prime.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility.h"


#ifdef SUPPORT_STATIC_ANALYSIS
//functions defined and used only in this file(just for static analysis).

#endif


//this macro is for RSA key generation.
#define RSA_GET_K_E_Y_AGAIN      (~(RSA_SUCCESS))
#define RSA_GET_K_E_Y_SUCCESS    (RSA_SUCCESS)


/* function: out = a^e mod n
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n
 *     e -------------------------- input, uint32_t big integer e, exeponent, make sure e < n
 *     n -------------------------- input, uint32_t big integer n, modulus, make sure n is odd
 *     out ------------------------ output, out = a^e mod n
 *     eBitLen  ------------------- input, real bit length of uint32_t big integer e
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 */
uint32_t RSA_ModExp(const uint32_t *a, const uint32_t *e, const uint32_t *n, uint32_t *out, 
        uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t eWordLen;
    uint32_t nWordLen;
    uint32_t ret;

    if((NULL == a) || (NULL == e) || (NULL == n) || (NULL == out))
    {
        ret = RSA_BUFFER_NULL;
    }
    else if((nBitLen > RSA_MAX_BIT_LEN) || (eBitLen > nBitLen) || (nBitLen < RSA_MIN_BIT_LEN))
    {
        ret = RSA_INPUT_INVALID;
    }
    else if(0u == (n[0] & 1u))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        ret = RSA_SUCCESS;
    }

    if(RSA_SUCCESS == ret)
    {
        eWordLen = GET_WORD_LEN(eBitLen);
        nWordLen = GET_WORD_LEN(nBitLen);
        ret = pke_modexp_check_input(n, e, a, out, nWordLen, eWordLen);
        if(PKE_FINISHED == ret)
        {
            ret = RSA_SUCCESS;
        }
        else if(PKE_SUCCESS == ret)
        {
#if (defined(PKE_LP) || defined(PKE_SECURE))
            ret = pke_pre_calc_mont(n, nBitLen, NULL, NULL);
#else
            ret = pke_pre_calc_mont(n, nBitLen, NULL);
#endif
            ret = pke_modexp_internal(e, a, out, nWordLen, eWordLen);
            if(PKE_SUCCESS == ret)
            {
                ret = RSA_SUCCESS;
            }
            else
            {}
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


/* function: RSA_CRTModExp check input
 * parameters:
 *     a -------------------------- input, pointer to uint32_t big integer a, base number, make sure a < n=pq
 *     p -------------------------- input, pointer to uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, pointer to uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, pointer to uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, pointer to uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, pointer to uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     out ------------------------ input, pointer to out = a^d mod n
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 *     nWordLen ------------------- output, word length of n
 *     pBitLen -------------------- output, bit length of p,q
 *     pWordLen ------------------- output, word length of p,q,dp,dq,u
 *     tmp ------------------------ output, n=pq
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a, n and out have the same word length:nWordLen = ((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:pWordLen = ((nBitLen/2+31)>>5)
 */
FLAG_STATIC uint32_t RSA_CRTModExp_check_input(const uint32_t *a, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t *dq, const uint32_t *u, const uint32_t *out, uint32_t nBitLen, 
        uint32_t *nWordLen, uint32_t *pBitLen, uint32_t *pWordLen, uint32_t *tmp)
{
    uint32_t ret;

    if((NULL == a) || (NULL == p) || (NULL == q) || (NULL == dp) || (NULL == dq) || (NULL == u) || (NULL == out))
    {
        ret = RSA_BUFFER_NULL;
    }
    else if(nBitLen > RSA_MAX_BIT_LEN)
    {
        ret = RSA_INPUT_TOO_LONG;
    }
    else if(nBitLen < RSA_MIN_BIT_LEN)
    {
        ret = RSA_INPUT_INVALID;
    }
    else if(0u != (nBitLen & 1u))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        *nWordLen = GET_WORD_LEN(nBitLen);
        *pBitLen  = nBitLen>>1;
        *pWordLen = GET_WORD_LEN(*pBitLen);

        //get n = p*q
        ret = pke_mul(p, q, tmp, *pWordLen);
    }

    return ret;
}


/* function: RSA_CRTModExp step 1(internal API)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n=pq
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     m1 ------------------------- output, m1 = ((a^dp mod p) - (a^dq mod q)) mod p
 *     m2 ------------------------- output, m2 = a^dq mod q
 *     pBitLen -------------------- input, bit length of p,q
 *     pWordLen ------------------- input, word length of p,q,dp,dq,u
 *     nWordLen ------------------- input, word length of a,n=pq
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a, n and out have the same word length:nWordLen = ((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:pWordLen = ((nBitLen/2+31)>>5)
 */
FLAG_STATIC uint32_t RSA_CRTModExp_internal_step_1(const uint32_t *a, const uint32_t *p, 
        const uint32_t *q, const uint32_t *dp, const uint32_t *dq, uint32_t *m1, uint32_t *m2, 
        uint32_t pBitLen, uint32_t pWordLen, uint32_t nWordLen)
{
    uint32_t ret;
    uint32_t *tmp_out;
    uint32_t tmp_step = 0u;

    //do pke_pre_calc_mont() first, because a may be less than p or q, then pke_mod() will not
    //call pke_pre_calc_mont() inside, but pke_modexp_internal() needs the output of pke_pre_calc_mont().

    //m2 = (a) mod q
#if (defined(PKE_LP) || defined(PKE_SECURE))
    ret = pke_pre_calc_mont(q, pBitLen, NULL, NULL);
#else
    ret = pke_pre_calc_mont(q, pBitLen, NULL);
#endif
    if(PKE_SUCCESS == ret)
    {
        //get the pBitLen step
        tmp_step = pke_get_operand_bytes();

#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_mod(a, nWordLen, q, (uint32_t *)(rPKE_A(3u,tmp_step)), (uint32_t *)(rPKE_B(4u,tmp_step)), pWordLen, m2);
#else
        ret = pke_mod(a, nWordLen, q, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen, m2);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //m2 = (a)^dq mod q
        ret = pke_modexp_internal(dq, m2, m2, pWordLen, pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //m1 = (a) mod p
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
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_mod(a, nWordLen, p, (uint32_t *)(rPKE_A(3u,tmp_step)), (uint32_t *)(rPKE_B(4u,tmp_step)), pWordLen, m1);
#else
        ret = pke_mod(a, nWordLen, p, (uint32_t *)(rPKE_B(0u,tmp_step)), pWordLen, m1);
#endif
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //m1 = (a)^dp mod p
        ret = pke_modexp_internal(dp, m1, m1, pWordLen, pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        tmp_out = (uint32_t *)(rPKE_B(0u,tmp_step));
#else
        tmp_out = (uint32_t *)(rPKE_B(1u,tmp_step));
#endif

        //m1 = (m1-m2) mod p
        if(uint32_BigNumCmp(m2, pWordLen, p, pWordLen) >= 0)
        {
            //if m2 >= p, get tmp_out = m2 mod p
            ret = pke_mod_add_sub_mul_internal(m2, p, tmp_out, pWordLen, tmp_step, MICROCODE_INTSUB);
            if(PKE_SUCCESS == ret)
            {
                ret = pke_mod_add_sub_mul_internal(m1, tmp_out, m1, pWordLen, tmp_step, MICROCODE_MODSUB);
            }
            else
            {}
        }
        else
        {
            ret = pke_mod_add_sub_mul_internal(m1, m2, m1, pWordLen, tmp_step, MICROCODE_MODSUB);
        }
    }
    else
    {}

    return ret;
}


/* function: RSA_CRTModExp step 2(internal API)
 * parameters:
 *     u -------------------------- input, uint32_t big integer u, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     m1 ------------------------- input, m1 = ((a^dp mod p) - (a^dq mod q)) mod p
 *     m2 ------------------------- input, m2 = a^dq mod q
 *     out ------------------------ output, out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 *     pWordLen ------------------- input, word length of p,q,dp,dq,u
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 *     nWordLen ------------------- input, word length of a,n=pq
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. a, n and out have the same word length:nWordLen = ((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:pWordLen = ((nBitLen/2+31)>>5)
 */
FLAG_STATIC uint32_t RSA_CRTModExp_internal_step_2(const uint32_t *u, const uint32_t *q, 
        uint32_t *m1, const uint32_t *m2, uint32_t *out, uint32_t pWordLen, uint32_t nBitLen, 
        uint32_t nWordLen)
{
    uint32_t ret;
    uint32_t tmp_step = 0u;

    //m1 = h = u*(m1-m2) mod p
#if 1
#if (defined(PKE_LP) || defined(PKE_SECURE))
    pke_set_exe_cfg(PKE_EXE_CFG_ALL_NON_MONT);
#endif
    ret = pke_modmul_internal(m1, u, m1, pWordLen);
#else
    ret = pke_modmul(p, m1, u, m1, pWordLen);
#endif
    if(PKE_SUCCESS == ret)
    {
        //store the nBitLen step
        tmp_step = pke_set_operand_width(nBitLen);

        //A1 = hq
        ret = pke_mul(m1, q, (uint32_t *)(rPKE_A(1u,tmp_step)), pWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //out = m2+hq
        uint32_copy((uint32_t *)(rPKE_B(1u,tmp_step)), m2, pWordLen);
        uint32_clear(&(rPKE_B(1u,tmp_step))[pWordLen], nWordLen-pWordLen);
        ret = pke_add((uint32_t *)(rPKE_A(1u,tmp_step)), (uint32_t *)(rPKE_B(1u,tmp_step)), out, nWordLen);
    }
    else
    {}

    return ret;
}


/* function: out = a^d mod n, here d represents RSA CRT private key (p,q,dp,dq,u)
 * parameters:
 *     a -------------------------- input, uint32_t big integer a, base number, make sure a < n=pq
 *     p -------------------------- input, uint32_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint32_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, uint32_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, uint32_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, uint32_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     out ------------------------ output, out = a^d mod n
 *     nBitLen  ------------------- input, real bit length of uint32_t big integer n=pq
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. a, n and out have the same word length:((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:((nBitLen/2+31)>>5)
 */
uint32_t RSA_CRTModExp(const uint32_t *a, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t *dq, const uint32_t *u, uint32_t *out, 
        uint32_t nBitLen)
{
    uint32_t ret, is_over = 0u;
    uint32_t buf[RSA_MAX_WORD_LEN];
    uint32_t *m1 = buf;
    uint32_t *m2 = &buf[RSA_MAX_WORD_LEN>>1];
    uint32_t nWordLen;
    uint32_t pBitLen;
    uint32_t pWordLen;
    int32_t flag;

    ret = RSA_CRTModExp_check_input(a, p, q, dp, dq, u, out, nBitLen, &nWordLen, 
            &pBitLen, &pWordLen, buf);
    if(PKE_SUCCESS == ret)
    {
        //a should be in [0,n]
        flag = uint32_BigNumCmp(a, nWordLen, buf, nWordLen);
        if(flag > 0)
        {
            ret = RSA_INPUT_INVALID;
        }
        else if((1u == uint32_BigNum_Check_Zero(a, nWordLen)) || (0 == flag))  //if a is 0 or n, the output is 0
        {
            uint32_clear(out, nWordLen);
            is_over = 1u;
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        ret = RSA_CRTModExp_internal_step_1(a, p, q, dp, dq, m1, m2, pBitLen, pWordLen, nWordLen);
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (0u == is_over))
    {
        ret = RSA_CRTModExp_internal_step_2(u, q, m1, m2, out, pWordLen, nBitLen, nWordLen);
        if(PKE_SUCCESS == ret)
        {
            is_over = 1;
        }
        else
        {}
    }
    else
    {}

    if((PKE_SUCCESS == ret) && (1u == is_over))
    {
        ret = RSA_SUCCESS;
    }
    else
    {}

    return ret;
}


/* function: get big odd integer e of eBitLen
 * parameters:
 *     e -------------------------- input, uint32_t big odd integer e
 *     eBitLen  ------------------- input, bit length of uint32_t big odd integer e
 * return: PKE_SUCCESS(success), other(error: eBitLen<2 or failure of TRNG)
 * caution:
 *     1. eBitLen must be greater than 1
 */
FLAG_STATIC uint32_t RSA_Get_E1(uint32_t *e, uint32_t eBitLen)
{
    uint32_t bits = 0u;
    uint32_t eWordLen = 0u;
    uint32_t ret;

    if((eBitLen < 2u) || (eBitLen > RSA_MAX_BIT_LEN))    //just for static analysis.
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        eWordLen = (eBitLen+0x1Fu)>>5;

        ret = get_rand((uint8_t *)e, eWordLen<<2);
        if(TRNG_SUCCESS == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        bits = eBitLen & 31u;

        if(0u != bits)
        {
#if 0
            e[eWordLen - 1u] <<= (32u - bits);
            e[eWordLen - 1u] |= 0x80000000u;
            e[eWordLen - 1u] >>= (32u - bits);
#else
            e[eWordLen - 1u] &= (((uint32_t)1u)<<(bits))-1u;
            e[eWordLen - 1u] |= ((uint32_t)1u)<<(bits - 1u);
#endif
        }
        else
        {
            e[eWordLen - 1u] |= 0x80000000u;
        }

        e[0] |= 0x01u;          //make e odd
    }
    else
    {}

    return ret;
}


/* function: get big odd integer e of eBitLen, satisfies e < fai_n of bitLen
 * parameters:
 *     e -------------------------- input, uint32_t big odd integer e
 *     fai_n ---------------------- input, uint32_t big even integer fai_n
 *     bitLen   ------------------- input, bit length of uint32_t big odd integer e and n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. eBitLen must be greater than 65
 *     2. fai_n can not be 1000000000...000000
 */
FLAG_STATIC uint32_t RSA_Get_E2(uint32_t *e, const uint32_t *fai_n, uint32_t bitLen)
{
    uint32_t ret;
    uint32_t i, bits;
    uint8_t j;

    if(bitLen < 66u)
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        ret = RSA_Get_E1(e, bitLen);
    }

    if(PKE_SUCCESS == ret)
    {
        bits = bitLen - 1u;
        i = (bits+0x1Fu)>>5;        //i is 1 plus the word index of the word where the second highest bit is located
        j = (uint8_t)(bits&31u);    //j is the bit length up to the second highest bit in the targeted word
        if((uint8_t)0 == j)
        {
            j = (uint8_t)32;
        }
        else
        {}

        while(i>0u)
        {
            e[i-1u] &= (uint32_t)(~(((uint32_t)1u)<<(j-((uint8_t)1))));
            if(uint32_BigNumCmp(e, i, fai_n, i) < 0)       //if e < n
            {
                break;
            }
            else
            {}

            j--;
            if(((uint8_t)0) == j)       //j is 0, switch to the next word
            {
                i--;
                j = (uint8_t)32;
            }
            else
            {}
        }

        //fail, because fai_n is 1000000000...000000
    }
    else
    {}

    return ret;
}


/* function: judge whether big integer a is equal to 0x5a5a5a5a5a...5a or not
 * parameters:
 *     a -------------------------- input, uint32_t big integer a
 *     aBitLen -------------------- input, real bit length of a
 * return: 0(a==0x5a5a5a5a5a...5a), 1(a!=0x5a5a5a5a5a...5a)
 * caution:
 *     1. aBitLen can not be 0
 *     2. if aBitLen%32 != 0, then the highest word of a should be 0
 */
FLAG_STATIC uint32_t CheckValue_0x5a5a5a5a(const uint32_t *a, uint32_t aBitLen)
{
    uint32_t ret = 0u;
    uint32_t i, wordLen = aBitLen>>5;

    if(0u != (aBitLen & 0x1Fu))
    {
        if(a[wordLen] != 0u)
        {
            ret = 1u;
        }
        else
        {}
    }
    else
    {}

    if(0u == ret)
    {
        for(i=0; i<wordLen; i++)
        {
            if(a[i] != 0x5a5a5a5au)
            {
                ret = 1u;
                break;
            }
            else
            {}
        }
    }
    else
    {}

    return ret;
}


/* function: generate p,q,and fai(n)=(p-1)(q-1)
 * parameters:
 *     p -------------------------- output, uint32_t big integer, prime p
 *     q -------------------------- output, uint32_t big integer, prime q
 *     fai_n ---------------------- output, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     pBitLen  ------------------- input, bit length of p,q
 *     pWordLen  ------------------ input, word length of p,q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. here fai(n) occupies (pBitLen*2+31)>>5 words
 */
FLAG_STATIC uint32_t rsa_keygen_get_p_q_fai_n(uint32_t *p, uint32_t *q, uint32_t *fai_n, 
        uint32_t pBitLen, uint32_t pWordLen)
{
    uint32_t ret;

    ret = get_prime(p, pBitLen);
    if(MAYBE_PRIME == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = get_prime(q, pBitLen);
        if(MAYBE_PRIME == ret)
        {
            ret = PKE_SUCCESS;
        }
        else
        {}
    }
    else
    {}

#ifdef SUPPORT_STATIC_ANALYSIS
    if(PKE_SUCCESS == ret)
    {
        //this check is redundant, just for static analysis
        if((0u == (p[0] & 1u)) || (0u == (q[0] & 1u)))
        {
            ret = RSA_ERROR;
        }
        else
        {}
    }
    else
    {}
#endif

    if(PKE_SUCCESS == ret)
    {
        p[0]--;                                 // p=p-1
        q[0]--;                                 // q=q-1
        ret = pke_mul(p, q, fai_n, pWordLen);   // get fai(n)=(p-1)(q-1)
        if(PKE_SUCCESS == ret)
        {
            p[0]++;
            q[0]++;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: generate e,d from fai(n)=(p-1)(q-1), this is internal API.
 * parameters:
 *     fai_n ---------------------- input, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     d -------------------------- output, uint32_t big integer, RSA private key d
 *     nBitLen  ------------------- input, bit length of n
 *     nWordLen  ------------------ input, word length of n
 *     eBitLen  ------------------- input, bit length of e
 *     eWordLen  ------------------ input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 *     2. d = e^(-1) mod fai(n), and d occupies nWordLen words
 *     3. if return value is PKE_NO_MODINV, d does not exist
 */
FLAG_STATIC uint32_t rsa_keygen_get_e_d_internal(const uint32_t *fai_n, uint32_t *e, uint32_t *d, 
        uint32_t nBitLen, uint32_t nWordLen, uint32_t eBitLen, uint32_t eWordLen)
{
    uint32_t ret;

    if(17u == eBitLen)
    {
        e[0] = 65537u;
        ret = PKE_SUCCESS;
    }
    else if(5u == eBitLen)
    {
        e[0] = 17u;
        ret = PKE_SUCCESS;
    }
    else if(2u == eBitLen)
    {
        e[0] = 3u;
        ret = PKE_SUCCESS;
    }
    else if(eBitLen == nBitLen)
    {
        ret = RSA_Get_E2(e, fai_n, eBitLen);
    }
    else
    {
        ret = RSA_Get_E1(e, eBitLen);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modinv(fai_n, e, d, nWordLen, eWordLen);
    }
    else
    {}

    return ret;
}


/* function: generate e,d from fai(n)=(p-1)(q-1)
 * parameters:
 *     fai_n ---------------------- input, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     d -------------------------- output, uint32_t big integer, RSA private key d
 *     nBitLen  ------------------- input, bit length of n
 *     nWordLen  ------------------ input, word length of n
 *     eBitLen  ------------------- input, bit length of e
 *     eWordLen  ------------------ input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 *     2. d = e^(-1) mod fai(n), and d occupies nWordLen words
 *     3. if return value is PKE_NO_MODINV, d does not exist
 */
FLAG_STATIC uint32_t rsa_keygen_get_e_d(const uint32_t *fai_n, uint32_t *e, uint32_t *d, 
        uint32_t nBitLen, uint32_t nWordLen, uint32_t eBitLen, uint32_t eWordLen)
{
    uint32_t ret;
    uint32_t count;

    /****************************************
    * if ret is PKE_NO_MODINV, d doesn't exist, that means :
    * 1. e is prime, and e divide fai(n) 
    * 2. e is not prime, and e, fai(n) have common divisor.
    *****************************************/
    if((17u == eBitLen) || (5u == eBitLen) || (2u == eBitLen))
    {
        ret = rsa_keygen_get_e_d_internal(fai_n, e, d, nBitLen, nWordLen, eBitLen, eWordLen);
    }
    else
    {
        for(count = 0u; count<7u; count++)
        {
            ret = rsa_keygen_get_e_d_internal(fai_n, e, d, nBitLen, nWordLen, eBitLen, eWordLen);
#if 1
            if(PKE_NO_MODINV != ret)
            {
                break;
            }
            else
            {}
#else
            if(PKE_NO_MODINV == ret)
            {
                continue;
            }
            else
            {
                break;
            }
#endif
        }
    }

    return ret;
}


/* function: generate n and check the RSA key pair
 * parameters:
 *     e -------------------------- input, uint32_t big integer, RSA public key e
 *     d -------------------------- input, uint32_t big integer, RSA private key d
 *     p -------------------------- input, uint32_t big integer, prime p
 *     q -------------------------- input, uint32_t big integer, prime q
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eWordLen  ------------------ input, word length of e
 *     pWordLen  ------------------ input, word length of p,q
 *     nBitLen  ------------------- input, bit length of n
 *     nWordLen  ------------------ input, word length of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if return value is PKE_NO_MODINV, this means p or q may not be prime.
 */
FLAG_STATIC uint32_t rsa_keygen_final(const uint32_t *e, const uint32_t *d, const uint32_t *p, 
        const uint32_t *q, uint32_t *n, uint32_t eWordLen, uint32_t pWordLen, uint32_t nBitLen, 
        uint32_t nWordLen)
{
    uint32_t ret, tmp_step;
    uint32_t *in, *out;

    tmp_step = pke_set_operand_width(nBitLen);

    in = (uint32_t *)(rPKE_B(1u,tmp_step));
#if (defined(PKE_LP) || defined(PKE_SECURE))
    out = (uint32_t *)(rPKE_A(2u,tmp_step));
#else
    out = (uint32_t *)(rPKE_A(1u,tmp_step));
#endif

    //get n = pq
    ret = pke_mul(p, q, n, pWordLen);
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
        //Encryption test
#ifdef SUPPORT_STATIC_ANALYSIS
        //the last two conditional expressions are just for static analysis
        if((0u != (nBitLen & 0x1Fu)) && (nWordLen >= 1u) && (nWordLen <= RSA_MAX_WORD_LEN))
#else
        if(0u != (nBitLen & 0x1Fu))
#endif
        {
            in[nWordLen-1u]=0u;
        }
        else
        {}

        uint32_set(in, 0x5a5a5a5au, nBitLen>>5);

        ret = pke_modexp_internal(e, in, out, nWordLen, eWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = pke_modexp_internal(d, out, out, nWordLen, nWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != CheckValue_0x5a5a5a5a(out, nBitLen))
        {
            ret = PKE_NO_MODINV;   //this means p or q may not be prime.
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: generate RSA key (e,d,n)
 * parameters:
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     d -------------------------- output, uint32_t big integer, RSA private key d
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t RSA_GetKey(uint32_t *e, uint32_t *d, uint32_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t ret;
    uint32_t buf[RSA_MAX_WORD_LEN];
    uint32_t *p = buf;
    uint32_t *q = &buf[RSA_MAX_WORD_LEN>>1];
    uint32_t pBitLen, pWordLen, eWordLen, nWordLen;

    if((NULL == e) || (NULL == d) || (NULL == n))
    {
        ret = RSA_BUFFER_NULL;
    }
    else if((0u != (nBitLen&1u)) || (nBitLen < RSA_MIN_BIT_LEN) || (nBitLen > RSA_MAX_BIT_LEN))  //nBitLen can not be odd
    {
        ret = RSA_INPUT_INVALID;
    }
    else if((eBitLen<2u) || (eBitLen>nBitLen))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        eWordLen = GET_WORD_LEN(eBitLen);
        nWordLen = GET_WORD_LEN(nBitLen);
        pBitLen = nBitLen>>1;
        pWordLen = GET_WORD_LEN(pBitLen);

        /****************************************
        * if ret is PKE_NO_MODINV, that means :
        * 1. e, fai(n) have common divisor.
        * 2. p or q is not prime.
        * 3. p = q.
        *****************************************/
        do {
            ret = rsa_keygen_get_p_q_fai_n(p, q, n, pBitLen, pWordLen);
            if(PKE_SUCCESS == ret)
            {
                ret = rsa_keygen_get_e_d(n, e, d, nBitLen, nWordLen, eBitLen, eWordLen);
            }
            else
            {}

            if(PKE_SUCCESS == ret)
            {
                ret = rsa_keygen_final(e, d, p, q, n, eWordLen, pWordLen, nBitLen, nWordLen);
            }
            else
            {}
        } while(PKE_NO_MODINV == ret);

        if(PKE_SUCCESS == ret)
        {
            ret = RSA_SUCCESS;
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* function: RSA-CRT generate p-1,q-1,and fai(n)=(p-1)(q-1)
 * parameters:
 *     p_minus_1 ------------------ output, uint32_t big integer, prime p minus 1
 *     q_minus_1 ------------------ output, uint32_t big integer, prime q minus 1
 *     fai_n ---------------------- output, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     eBitLen  ------------------- input, bit length of e
 *     pBitLen  ------------------- input, bit length of p,q
 *     pWordLen  ------------------ input, word length of p,q
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if eBitLen == (pBitLen*2), fai(n) will be calculated, and fai(n) occupies 
 *        (pBitLen*2+31)>>5 words
 *     2. if return value is PKE_NO_MODINV, that means p=q, need to regenerate key pair
 */
FLAG_STATIC uint32_t rsa_crt_keygen_get_p_minus_1_q_minus_1_fai_n(uint32_t *p_minus_1, uint32_t *q_minus_1, 
        uint32_t *fai_n, uint32_t eBitLen, uint32_t pBitLen, uint32_t pWordLen)
{
    uint32_t ret;
    uint32_t i, tmp;
    int32_t flag;

    ret = get_prime(p_minus_1, pBitLen);
    if(MAYBE_PRIME == ret)
    {
        ret = PKE_SUCCESS;
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        ret = get_prime(q_minus_1, pBitLen);
        if(MAYBE_PRIME == ret)
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
        flag = uint32_BigNumCmp(p_minus_1, pWordLen, q_minus_1, pWordLen);   // make p > q, to get u = q^(-1) mod p conveniently
        if(flag == -1)
        {
            for(i=0; i<pWordLen; i++)
            {
                tmp = p_minus_1[i];
                p_minus_1[i] = q_minus_1[i];
                q_minus_1[i] = tmp;
            }
        }
        else if(flag == 0)
        {
            ret = PKE_NO_MODINV;  //p=q, need to regenerate key pair
        }
        else
        {
            //nothing to do, just for static analysis.
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        p_minus_1[0]--;                                     //get p-1
        q_minus_1[0]--;                                     //get q-1
        if(eBitLen == (pBitLen<<1))
        {
            ret = pke_mul(p_minus_1, q_minus_1, fai_n, pWordLen);   // get fai(n)=(p-1)(q-1)
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* Function: generate RSA-CRT key (dp,dq,u) from (e,p-1,q-1)
 * Parameters:
 *     e -------------------------- input, uint32_t big integer, RSA public key e
 *     p_minus_1 ------------------ input, uint32_t big integer, RSA private key p minus 1
 *     q_minus_1 ------------------ input, uint32_t big integer, RSA private key q minus 1
 *     dp-------------------------- output, uint32_t big integer, RSA private key dp
 *     dq-------------------------- output, uint32_t big integer, RSA private key dq
 *     u -------------------------- output, uint32_t big integer, RSA private key u = q^(-1) mod p
 *     eWordLen  ------------------ input, real word length of e
 *     pWordLen  ------------------ input, real word length of p_minus_1,q_minus_1,dp,dq,u
 * Return: PKE_SUCCESS(success), other(error)
 * Caution:
 *     1. if the returned value is PKE_SUCCESS, p_minus_1 and q_minus_1 will be p and q 
 *        respectively.
 */
FLAG_STATIC uint32_t rsa_crt_keygen_get_dp_dq_u_from_e_p_minus_1_q_minus_1(const uint32_t *e, 
        uint32_t *p_minus_1, uint32_t *q_minus_1, uint32_t *dp, uint32_t *dq, uint32_t *u, 
        uint32_t eWordLen, uint32_t pWordLen)
{
    uint32_t ret;

    // dp = e^(-1) mod (p-1)
    if(uint32_BigNumCmp(e, eWordLen, p_minus_1, pWordLen) > 0)
    {
#if (defined(PKE_LP) || defined(PKE_SECURE))
        ret = pke_mod(e, eWordLen, p_minus_1, NULL, NULL, pWordLen, u);
#else
        ret = pke_mod(e, eWordLen, p_minus_1, NULL, pWordLen, u);
#endif
        if(PKE_SUCCESS == ret)
        {
            ret = pke_modinv(p_minus_1, u, dp, pWordLen, pWordLen);
        }
        else
        {}
    }
    else
    {
        ret = pke_modinv(p_minus_1, e, dp, pWordLen, eWordLen);
    }

    if(PKE_SUCCESS == ret)
    {
        // dq = e^(-1) mod (q-1)
        if(uint32_BigNumCmp(e, eWordLen, q_minus_1, pWordLen) > 0)
        {
#if (defined(PKE_LP) || defined(PKE_SECURE))
            ret = pke_mod(e, eWordLen, q_minus_1, NULL, NULL, pWordLen, u);
#else
            ret = pke_mod(e, eWordLen, q_minus_1, NULL, pWordLen, u);
#endif
            if(PKE_SUCCESS == ret)
            {
                ret = pke_modinv(q_minus_1, u, dq, pWordLen, pWordLen);
            }
            else
            {}
        }
        else
        {
            ret = pke_modinv(q_minus_1, e, dq, pWordLen, eWordLen);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        p_minus_1[0]++;
        q_minus_1[0]++;

        // u = q^(-1) mod p
        ret = pke_modinv(p_minus_1, q_minus_1, u, pWordLen, pWordLen);
    }
    else
    {}

    return ret;
}


/* function: generate e,(p,q,dq,dq,u) from p-1, q-1, and fai(n)=(p-1)(q-1), this is internal API.
 * parameters:
 *     fai_n ---------------------- input, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     p_minus_1 ------------------ input, uint32_t big integer, (p-1)
 *     q_minus_1 ------------------ input, uint32_t big integer, (q-1)
 *     dp ------------------------- output, uint32_t big integer, RSA private key dp
 *     dq ------------------------- output, uint32_t big integer, RSA private key dq
 *     u -------------------------- output, uint32_t big integer, RSA private key u
 *     nBitLen  ------------------- input, bit length of n
 *     pWordLen  ------------------ input, word length of p,dq,q,dq,u
 *     eBitLen  ------------------- input, bit length of e
 *     eWordLen  ------------------ input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 *     2. fai(n) is used when (eBitLen=nBitLen), and fai(n) occupies nWordLen words
 *     3. if return value is PKE_NO_MODINV, (p,q,dp,dq,u) does not exist
 *     4. if rerurn value is PKE_SUCCESS, p_minus_1 and q_minus_1 will be p and q respectively
 */
FLAG_STATIC uint32_t rsa_crt_keygen_get_e_p_q_dp_dq_u_internal(const uint32_t *fai_n, uint32_t *e, 
        uint32_t *p_minus_1, uint32_t *q_minus_1, uint32_t *dp, uint32_t *dq, uint32_t *u, 
        uint32_t nBitLen, uint32_t pWordLen, uint32_t eBitLen, uint32_t eWordLen)
{
    uint32_t ret;

    if(17u == eBitLen)
    {
        e[0] = 65537u;
        ret = PKE_SUCCESS;
    }
    else if(5u == eBitLen)
    {
        e[0] = 17u;
        ret = PKE_SUCCESS;
    }
    else if(2u == eBitLen)
    {
        e[0] = 3u;
        ret = PKE_SUCCESS;
    }
    else if(eBitLen == nBitLen)
    {
        ret = RSA_Get_E2(e, fai_n, eBitLen);
    }
    else
    {
        ret = RSA_Get_E1(e, eBitLen);
    }

    if(PKE_SUCCESS == ret)
    {
        ret = rsa_crt_keygen_get_dp_dq_u_from_e_p_minus_1_q_minus_1(e, p_minus_1, \
                q_minus_1, dp, dq, u, eWordLen, pWordLen);
    }
    else
    {}

    return ret;
}


/* function: generate e and (p,q,dq,dq,u)
 * parameters:
 *     fai_n ---------------------- input, uint32_t big integer, fai(n)=(p-1)(q-1)
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     p_minus_1 ------------------ input, uint32_t big integer, (p-1)
 *     q_minus_1 ------------------ input, uint32_t big integer, (q-1)
 *     dp ------------------------- output, uint32_t big integer, RSA private key dp
 *     dq ------------------------- output, uint32_t big integer, RSA private key dq
 *     u -------------------------- output, uint32_t big integer, RSA private key u
 *     nBitLen  ------------------- input, bit length of n
 *     pWordLen  ------------------ input, word length of p,dq,q,dq,u
 *     eBitLen  ------------------- input, bit length of e
 *     eWordLen  ------------------ input, word length of e
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 *     2. fai(n) is used when (eBitLen=nBitLen), and fai(n) occupies nWordLen words
 *     3. if return value is PKE_NO_MODINV, (p,q,dp,dq,u) does not exist
 *     4. if rerurn value is PKE_SUCCESS, p_minus_1 and q_minus_1 will be p and q respectively
 */
FLAG_STATIC uint32_t rsa_crt_keygen_get_e_p_q_dp_dq_u(const uint32_t *fai_n, uint32_t *e, 
        uint32_t *p_minus_1, uint32_t *q_minus_1, uint32_t *dp, uint32_t *dq, uint32_t *u, 
        uint32_t nBitLen, uint32_t pWordLen, uint32_t eBitLen, uint32_t eWordLen)
{
    uint32_t ret;
    uint32_t count;

    /****************************************
    * if ret is PKE_NO_MODINV, (p,q,dp,dq,u) doesn't exist, that means :
    * 1. e is prime, and e divide fai(n) 
    * 2. e is not prime, and e, fai(n) have common divisor.
    *****************************************/
    if((17u == eBitLen) || (5u == eBitLen) || (2u == eBitLen))
    {
        ret = rsa_crt_keygen_get_e_p_q_dp_dq_u_internal(fai_n, e, p_minus_1, q_minus_1, dp, dq, u, nBitLen, pWordLen, eBitLen, eWordLen);
    }
    else
    {
        for(count = 0u; count<7u; count++)
        {
            ret = rsa_crt_keygen_get_e_p_q_dp_dq_u_internal(fai_n, e, p_minus_1, q_minus_1, dp, dq, u, nBitLen, pWordLen, eBitLen, eWordLen);
#if 1
            if(PKE_NO_MODINV != ret)
            {
                break;
            }
            else
            {}
#else
            if(PKE_NO_MODINV == ret)
            {
                continue;
            }
            else
            {
                break;
            }
#endif
        }
    }

    return ret;
}


/* function: generate n and check the RSA-CRT key pair
 * parameters:
 *     e -------------------------- input, uint32_t big integer, RSA public key e
 *     p -------------------------- input, uint32_t big integer, prime p
 *     q -------------------------- input, uint32_t big integer, prime q
 *     dp-------------------------- input, uint32_t big integer, RSA private key dp
 *     dq-------------------------- input, uint32_t big integer, RSA private key dq
 *     u -------------------------- input, uint32_t big integer, RSA private key u = q^(-1) mod p
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eWordLen  ------------------ input, word length of e
 *     pWordLen  ------------------ input, word length of p,q
 *     nBitLen  ------------------- input, bit length of n
 *     nWordLen  ------------------ input, word length of n
 * return: PKE_SUCCESS(success), other(error)
 * caution:
 *     1. if return value is PKE_NO_MODINV, this means p or q may not be prime.
 */
FLAG_STATIC uint32_t rsa_crt_keygen_final(const uint32_t *e, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t *dq, const uint32_t *u, uint32_t *n, uint32_t eWordLen, 
        uint32_t pWordLen, uint32_t nBitLen, uint32_t nWordLen)
{
    uint32_t ret;
    uint32_t buf[RSA_MAX_WORD_LEN];

    //get n = pq
    ret = pke_mul(p, q, n, pWordLen);
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
        //Encryption test
#ifdef SUPPORT_STATIC_ANALYSIS
        //the last two conditional expressions are just for static analysis
        if((0u != (nBitLen & 0x1Fu)) && (nWordLen >= 1u) && (nWordLen <= RSA_MAX_WORD_LEN))
#else
        if(0u != (nBitLen & 0x1Fu))
#endif
        {
            buf[nWordLen-1u]=0u;
        }
        else
        {}

        uint32_set(buf, 0x5a5a5a5au, nBitLen>>5);

        ret = pke_modexp_internal(e, buf, buf, nWordLen, eWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u == CheckValue_0x5a5a5a5a(buf, nBitLen))
        {
            ret = PKE_NO_MODINV;   //to make sure buf is changed
        }
        else
        {
            ret = RSA_CRTModExp(buf, p, q, dp, dq, u, buf, nBitLen);
        }
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        if(0u != CheckValue_0x5a5a5a5a(buf, nBitLen))
        {
            ret = PKE_NO_MODINV;   //this means p or q may not be prime.
        }
        else
        {}
    }
    else
    {}

    return ret;
}


/* Function: generate RSA-CRT key (e,p,q,dp,dq,u,n)
 * Parameters:
 *     e -------------------------- output, uint32_t big integer, RSA public key e
 *     p -------------------------- output, uint32_t big integer, RSA private key p
 *     q -------------------------- output, uint32_t big integer, RSA private key q
 *     dp-------------------------- output, uint32_t big integer, RSA private key dp
 *     dq-------------------------- output, uint32_t big integer, RSA private key dq
 *     u -------------------------- output, uint32_t big integer, RSA private key u = q^(-1) mod p
 *     n -------------------------- output, uint32_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * Return: RSA_SUCCESS(success), other(error)
 * Caution:
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t RSA_GetCRTKey(uint32_t *e, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u,
        uint32_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
    uint32_t ret;
    uint32_t pBitLen, pWordLen, eWordLen, nWordLen;

    if((NULL == e) || (NULL == p) || (NULL == q) || (NULL == dp) || (NULL == dq) || (NULL == u) || (NULL == n))
    {
        ret = RSA_BUFFER_NULL;
    }
    else if((0u != (nBitLen&1u)) || (nBitLen < RSA_MIN_BIT_LEN) || (nBitLen > RSA_MAX_BIT_LEN))  //nBitLen can not be odd
    {
        ret = RSA_INPUT_INVALID;
    }
    else if((eBitLen<2u) || (eBitLen>nBitLen))
    {
        ret = RSA_INPUT_INVALID;
    }
    else
    {
        ret = PKE_SUCCESS;
    }

    if(PKE_SUCCESS == ret)
    {
        eWordLen = GET_WORD_LEN(eBitLen);
        nWordLen = GET_WORD_LEN(nBitLen);
        pBitLen = nBitLen>>1;
        pWordLen = GET_WORD_LEN(pBitLen);

        /****************************************
        * if ret is PKE_NO_MODINV, that means :
        * 1. e, fai(n) have common divisor.
        * 2. p or q is not prime.
        * 3. p = q.
        *****************************************/
        do {
            ret = rsa_crt_keygen_get_p_minus_1_q_minus_1_fai_n(p, q, n, eBitLen, pBitLen, pWordLen);
            if(PKE_SUCCESS == ret)
            {
                ret = rsa_crt_keygen_get_e_p_q_dp_dq_u(n, e, p, q, dp, dq, u, nBitLen, pWordLen, eBitLen, eWordLen);
            }
            else
            {}

            if(PKE_SUCCESS == ret)
            {
                ret = rsa_crt_keygen_final(e, p, q, dp, dq, u, n, eWordLen, pWordLen, nBitLen, nWordLen);
            }
            else
            {}
        } while(PKE_NO_MODINV == ret);

        if(PKE_SUCCESS == ret)
        {
            ret = RSA_SUCCESS;
        }
        else
        {}
    }

    return ret;
}

#endif

