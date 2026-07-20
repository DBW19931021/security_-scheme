#include "pke_common.h"

#include "../crypto_include/crypto_common/utility.h"




/* function: load input operand to baseaddr
 * parameters:
 *     baseaddr ------------------- output, destination data
 *     data ----------------------- input, source data
 *     wordLen -------------------- input, word length of data
 * return: none
 * caution:
 *     1. operands are both U32 little-endian
 */
void pke_load_operand(uint32_t *baseaddr, const uint32_t *data, uint32_t wordLen)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != baseaddr) && (NULL != data))
    {
#endif
        if(baseaddr != data)
        {
            for (i = 0; i < wordLen; i++)
            {
                ((volatile uint32_t *)baseaddr)[i] = data[i];
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: load input operand of 256 bits to baseaddr
 * parameters:
 *     baseaddr ------------------- output, destination data
 *     data ----------------------- input, source data
 * return: none
 * caution:
 *     1. operands are both U32 little-endian
 *     2. operands are both of 256 bits for SM2, SM9, etc.
 */
void pke_load_operand_256bits(uint32_t *baseaddr, const uint32_t *data)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != baseaddr) && (NULL != data))
    {
#endif
        *((volatile uint32_t *)(&baseaddr[0])) = data[0];
        *((volatile uint32_t *)(&baseaddr[1])) = data[1];
        *((volatile uint32_t *)(&baseaddr[2])) = data[2];
        *((volatile uint32_t *)(&baseaddr[3])) = data[3];
        *((volatile uint32_t *)(&baseaddr[4])) = data[4];
        *((volatile uint32_t *)(&baseaddr[5])) = data[5];
        *((volatile uint32_t *)(&baseaddr[6])) = data[6];
        *((volatile uint32_t *)(&baseaddr[7])) = data[7];
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: get result operand from baseaddr
 * parameters:
 *     baseaddr ------------------- input, source data
 *     data ----------------------- output, destination data
 *     wordLen -------------------- input, word length of data
 * return: none
 * caution:
 *     1. operands are both U32 little-endian
 */
void pke_read_operand(const uint32_t *baseaddr, uint32_t *data, uint32_t wordLen)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != baseaddr) && (NULL != data))
    {
#endif
        if(baseaddr != data)
        {
            for (i = 0; i < wordLen; i++)
            {
                data[i] = baseaddr[i];
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: get result operand of 256 bits from baseaddr
 * parameters:
 *     baseaddr ------------------- input, source data
 *     data ----------------------- output, destination data
 * return: none
 * caution:
 *     1. operands are both U32 little-endian
 *     2. operands are both of 256 bits for SM2, SM9, etc.
 */
void pke_read_operand_256bits(const uint32_t *baseaddr, uint32_t *data)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != baseaddr) && (NULL != data))
    {
#endif
        data[0] = baseaddr[0];
        data[1] = baseaddr[1];
        data[2] = baseaddr[2];
        data[3] = baseaddr[3];
        data[4] = baseaddr[4];
        data[5] = baseaddr[5];
        data[6] = baseaddr[6];
        data[7] = baseaddr[7];
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: load input operand(U8 big-endian) to baseaddr
 * parameters:
 *     baseaddr ------------------- output, destination data
 *     data ----------------------- input, source data, U8 big-endian
 *     byteLen -------------------- input, byte length of data
 * return: none
 * caution:
 */
void pke_load_operand_U8(uint32_t *baseaddr, const uint8_t *data, uint32_t byteLen)
{
    uint32_t *dst;
    uint32_t bytes;
    uint32_t t, i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != data)
    {
#endif
        if((uint8_t *)baseaddr != data)
        {
            dst = baseaddr;
            bytes = byteLen;

            for(; bytes>3u; bytes-=4u)
            {
                t  = (uint32_t)(data[bytes - 1u]);
                t |= ((uint32_t)(data[bytes - 2u]))<<8;
                t |= ((uint32_t)(data[bytes - 3u]))<<16;
                t |= ((uint32_t)(data[bytes - 4u]))<<24;

                *((volatile uint32_t *)(dst)) = t;

                dst = &dst[1];
            }

            if(0U != bytes)
            {
                t = 0U;
                for(i=0U; i < bytes; i++)
                {
                    t |= ((uint32_t)(data[bytes - 1u - i]))<<(i<<3);
                }

                *((volatile uint32_t *)(dst)) = t;
            }
            else
            {}
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: get result operand(U8 big-endian) from baseaddr
 * parameters:
 *     baseaddr ------------------- input, source data
 *     data ----------------------- output, destination data, U8 big-endian
 *     byteLen -------------------- input, byte length of data
 * return: none
 * caution:
 */
void pke_read_operand_U8(const uint32_t *baseaddr, uint8_t *data, uint32_t byteLen)
{
    const uint32_t *src;
    uint32_t bytes;
    uint32_t t, i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != data)
    {
#endif
        if((const uint8_t *)baseaddr != data)
        {
            src = baseaddr;
            bytes = byteLen;

            for (; bytes>3u; bytes-=4u)
            {
                t = *(src);

                data[bytes - 1u] = (uint8_t)((t)&0xFFu);
                data[bytes - 2u] = (uint8_t)((t>>8)&0xFFu);
                data[bytes - 3u] = (uint8_t)((t>>16)&0xFFu);
                data[bytes - 4u] = (uint8_t)((t>>24)&0xFFu);

                src = &src[1];
            }

            if(0U != bytes)
            {
                t = *(src);

                for(i=0U; i < bytes; i++)
                {
                    data[bytes - 1u - i] = (uint8_t)((t>>(i<<3))&0xFFu);
                }
            }
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: set operand with an uint32_t value
 * parameters:
 *     baseaddr ------------------- output, operand
 *     wordLen -------------------- input, word length of operand
 *     b -------------------------- input, uint32_t value b
 * return: none
 * caution:
 *     1. wordLen can not be 0
 */
void pke_set_operand_uint32_value(uint32_t *baseaddr, uint32_t wordLen, uint32_t b)
{
    uint32_t i = wordLen;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != baseaddr)
    {
#endif
        while(i > 1U)
        {
            i--;
            *((volatile uint32_t *)(&baseaddr[i])) = 0U;
        }

        *((volatile uint32_t *)(&baseaddr[0])) = b;
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: set operand of 256 bits with an uint32_t value
 * parameters:
 *     baseaddr ------------------- output, operand
 *     wordLen -------------------- input, word length of operand
 *     b -------------------------- input, uint32_t value b
 * return: none
 * caution:
 *     1. operand is U32 little-endian
 *     2. operand is of 256 bits for SM2, SM9, etc.
 */
void pke_set_operand_uint32_value_256bits(uint32_t *baseaddr, uint32_t b)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != baseaddr)
    {
#endif
        *((volatile uint32_t *)(&baseaddr[0])) = b;
        *((volatile uint32_t *)(&baseaddr[1])) = 0u;
        *((volatile uint32_t *)(&baseaddr[2])) = 0u;
        *((volatile uint32_t *)(&baseaddr[3])) = 0u;
        *((volatile uint32_t *)(&baseaddr[4])) = 0u;
        *((volatile uint32_t *)(&baseaddr[5])) = 0u;
        *((volatile uint32_t *)(&baseaddr[6])) = 0u;
        *((volatile uint32_t *)(&baseaddr[7])) = 0u;
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: check whether k is equal to (n-1), here n is odd.
 * parameters:
 *     k -------------------------- input, big number k
 *     n -------------------------- input, big number n
 *     words ---------------------- input, word length of k,n
 * return: 0(k is n-1), other(k is not n-1)
 * caution:
 *     1. n must be odd
 */
uint32_t is_k_equal_to_n_minus_1(const uint32_t *k, const uint32_t *n, uint32_t words)
{
    uint32_t ret = 1u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != k) && (NULL != n))
    {
#endif
        if(k[0] == (n[0]-1u))
        {
            ret = 0u;

            if(words > 1u)
            {
                if(0 != uint32_BigNumCmp(&k[1], words-1u, &n[1], words-1u))
                {
                    ret = 1u;
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}

