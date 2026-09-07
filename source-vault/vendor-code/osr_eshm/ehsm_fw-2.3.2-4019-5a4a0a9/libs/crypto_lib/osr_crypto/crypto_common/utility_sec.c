
#include "../crypto_include/crypto_common/utility_sec.h"


#ifdef UTILITY_SEC

#include "../crypto_include/trng/trng.h"


#define POLY                               (0x1021)

/* function: crc16 calculation, update crc value
 * parameters:
 *     in ------------------------- input, data for crc calculate
 *     bytelen -------------------- input, bytetlen of in
 *     crc  ----------------------- input, current crc value
 * return: updated crc value
 * caution:
 *     1. please make sure in is not NULL.
 */
#if 1
uint16_t crc16_calc(const uint8_t *in, uint32_t bytelen, uint16_t crc)
{
    uint32_t i;
    uint32_t tmp_crc = (uint32_t)crc;
#if 0
    const uint32_t tmp[2] = {0u, (uint32_t)POLY};
#endif

    for (i=0u; i < bytelen; i++)
    {
        tmp_crc ^= (((uint32_t)in[i]) << 8);
#if 0
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
        tmp_crc = (tmp_crc<<1) ^ tmp[(tmp_crc>>15)&0x01u];
#else
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
        tmp_crc = (0u != (tmp_crc & 0x8000u)) ? ((tmp_crc<<1)^((uint32_t)POLY)) : (tmp_crc<<1);
#endif
    }
    
    return (uint16_t)tmp_crc;
}
#else
uint16_t crc16_calc(const uint8_t *in, uint32_t bytelen, uint16_t crc)
{
    uint32_t i, j;
    uint16_t tmp_crc = crc;

    for (i = 0u; i < bytelen; i++)
    {
        tmp_crc ^= (((uint16_t)in[i]) << 8);

        for (j = 0u; j < 8u; j++)
        {
            if (0u != (tmp_crc & ((uint16_t)0x8000)))
            {
                tmp_crc = (tmp_crc << 1) ^ ((uint16_t)POLY);
            }
            else
            {
                tmp_crc <<= 1;
            }
        }
    }

    return tmp_crc;
}
#endif


/* function: get random big number a, a occupies aBitLen bits, and MSB of a is 0
 * parameters:
 *     a -------------------------- output, uint32_t big integer a
 *     aBitLen  ------------------- input, the bit length that a occupies
 * return: 0(success), other(error)
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t uint32_get_rand_big_number_msb_0(uint32_t *a, uint32_t aBitLen)
{
    uint32_t aWordLen;
    uint32_t bits;
    uint32_t ret = 1u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        if(aBitLen > 0u)
        {
            aWordLen = GET_WORD_LEN(aBitLen);
            ret = get_rand((uint8_t *)a, (aWordLen) << 2);
            if(TRNG_SUCCESS != ret)
            {
                ret = 1u;
            }
            else
            {
                bits = (aBitLen-1u) & 31u;
                if(0u != bits)
                {
                    ((volatile uint32_t *)a)[aWordLen-1u] &= (((uint32_t)1u)<<bits)-1u;
                }
                else
                {
                    ((volatile uint32_t *)a)[aWordLen-1u] = 0u;
                }

                ret = 0u;
            }
        }
        else
        {
            ret = 0u;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether big number or uint8_t buffer a is all zero or not
 * parameters:
 *     a -------------------------- input, byte buffer a
 *     aByteLen ------------------- input, byte length of a
 * return: 0(a is not zero), 1(a is all zero)
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t uint8_BigNum_Check_Zero_sec(const uint8_t *a, uint32_t aByteLen)
{
    uint32_t i, result = 0u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        for(i=0u; i<aByteLen; i++)
        {
            result |= (uint32_t)(a[i]);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

#ifdef SUPPORT_STATIC_ANALYSIS
    return (0u == result)?1u:0u;
#else
    return !result;
#endif
}


/* function: check whether big number or uint32_t buffer a is all zero or not
 * parameters:
 *     a -------------------------- input, big integer or word buffer a
 *     aWordLen ------------------- input, word length of a
 * return: 0(a is not zero), 1(a is all zero)
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t uint32_BigNum_Check_Zero_sec(const uint32_t *a, uint32_t aWordLen)
{
    uint32_t i, result = 0u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        for(i=0u; i<aWordLen; i++)
        {
            result |= a[i];
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

#ifdef SUPPORT_STATIC_ANALYSIS
    return (0u == result)?1u:0u;
#else
    return !result;
#endif
}


/* function: compare big integer a and b(same word length)
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     words ---------------------- input, real word length of a,b
 * return:
 *     0:a=b,   1:a>b,   -1: a<b
 * caution:
 *     1. please make sure neither of a,b is NULL.
 */
FLAG_STATIC int32_t uint32_BigNumCmp_sec_internal(const uint32_t *a, const uint32_t *b, uint32_t words)
{
    int32_t ret = 0;
    uint32_t i;

    i = words;
    while(i > 0u)
    {
        i--;
        if(a[i] > b[i])
        {
            if(0 == ret)
            {
                ret = 1;
            }
            else
            {}
        }
        else if(a[i] < b[i])
        {
            if(0 == ret)
            {
                ret = -1;
            }
            else
            {}
        }
        else
        {
            //just for static analysis.
        }
    }

    return ret;
}


/* function: compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     aWordLen ------------------- input, word length of a
 *     b -------------------------- input, big integer b
 *     bWordLen ------------------- input, word length of b
 * return:
 *     0:a=b,   1:a>b,   -1: a<b
 * caution:
 *     1. please make sure neither of a,b is NULL.
 */
int32_t uint32_BigNumCmp_sec(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, uint32_t bWordLen)
{
    uint32_t a_words, b_words;
    int32_t ret = 0;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != a) && (NULL != b))
    {
#endif
        a_words = get_valid_words(a, aWordLen);
        b_words = get_valid_words(b, bWordLen);

        if(a_words > b_words)
        {
            ret = 1;
        }
        else if(a_words < b_words)
        {
            ret = -1;
        }
        else   //a_words == b_words
        {
            ret = uint32_BigNumCmp_sec_internal(a, b, a_words);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 *     1. please make sure neither of a,b is NULL.
 */
static uint32_t uint32_cmp1_sec(const uint32_t *a, const uint32_t *b, uint32_t wordLen)
{
    uint32_t i, result = 1u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != a) && (NULL != b))
    {
#endif
        result = 0u;
        for(i=0u;i<wordLen;i++)
        {
            result |= (a[i] - b[i]);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return result;
}


/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 *     1. please make sure neither of a,b is NULL.
 */
static uint32_t uint32_cmp2_sec(const uint32_t *a, const uint32_t *b, uint32_t wordLen)
{
    uint32_t i, result = 1u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != a) && (NULL != b))
    {
#endif
        result = 0u;
        for(i=0u;i<wordLen;i++)
        {
            result |= (a[i] ^ b[i]);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return result;
}


/* function: securely compare big integer a and b
 * parameters:
 *     a -------------------------- input, big integer a
 *     b -------------------------- input, big integer b
 *     wordLen -------------------- input, word length of a and b
 * return: 0(a=b), other(a!=b)
 * caution:
 *     1. please make sure neither of a,b is NULL.
 */
uint32_t uint32_cmp_sec(const uint32_t *a, const uint32_t *b, uint32_t wordLen, uint8_t rand_bit)
{
    uint32_t ret;

    if(0u == (((uint32_t)rand_bit) & 0x01u))
    {
        ret = uint32_cmp1_sec(a, b, wordLen);
    }
    else
    {
        ret = uint32_cmp2_sec(a, b, wordLen);
    }

    return ret;
}


/* function: check whether integer k is in [1, n-1]. secure version
 * parameters:
 *     k -------------------------- input, big number k
 *     n -------------------------- input, big number n
 *     wordLen -------------------- input, word length of k and n
 * return:
 *     ret_zero ------------------- k is zero
 *     ret_big -------------------- k is greater/bigger than or equal to n
 *     ret_success ---------------- k is in [1, n-1]
 * caution:
 */
uint32_t uint32_integer_check_sec(const uint32_t *k, const uint32_t *n, uint32_t wordLen, 
        uint32_t ret_zero, uint32_t ret_big, uint32_t ret_success)
{
    uint32_t ret;

    if(0u != uint32_BigNum_Check_Zero_sec(k, wordLen))
    {
        ret = ret_zero;
    }
    else if(uint32_BigNumCmp_sec(k, wordLen, n, wordLen) >= 0)
    {
        ret = ret_big;
    }
    else
    {
        ret = ret_success;
    }

    return ret;
}


/* function: counter += 1u
 * parameters:
 *     counter -------------------- input, pointer to counter
 * return: counter+1u
 * caution:
 */
uint32_t counter_add_one(volatile uint32_t *counter)
{
    uint32_t ret = 0u;

    counter[0] += 1u;
    if(counter[0] < 1u)
    {
        ret = counter[0];
    }
    else
    {
        ret += counter[0];
    }

    return ret;
}

#endif

