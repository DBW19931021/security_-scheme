
#include "../crypto_include/crypto_common/utility.h"



/* function: memory copy, like memcpy()
 * parameters:
 *     dst ------------------------ output, output buffer
 *     src ------------------------ input, input buffer
 *     size ----------------------- input, bytes of src or dst buffer.
 * return: none
 * caution:
 *     1. please make sure neither of dst,src is NULL.
 *     2. please make sure dst buffer and src buffer do not have common part.
 */
void memcpy_(void *dst, const void *src, uint32_t size)
{
#if 0
    while(size--)
    {
        *(dst++) = *(src++);
    }
#else
    uint32_t *a_u32;
    const uint32_t *b_u32;
    uint8_t *a_u8 = (uint8_t *)dst;
    const uint8_t *b_u8 = (const uint8_t *)src;
    uint32_t i, count, tmp;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != dst) && (NULL != src))
    {
#endif
        if((0U != (((uintptr_t)dst) & 3U)) || (0U != (((uintptr_t)src) & 3U)))
        {
            for(i = 0U; i<size; i++)
            {
                a_u8[i] = b_u8[i];
            }
        }
        else
        {
            a_u32 = (uint32_t *)dst;
            b_u32 = (const uint32_t *)src;
            count = size>>2;
            for(i=0U; i<count; i++)
            {
                a_u32[i] = b_u32[i];
            }

            tmp = size&3U;
            if(0U != tmp)
            {
                a_u8 = &(a_u8[size&(~0x03U)]);
                b_u8 = &(b_u8[size&(~0x03U)]);
                for(i=0U; i<tmp; i++)
                {
                    a_u8[i] = b_u8[i];
                }
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
#endif
}


/* function: memory set, like memset()
 * parameters:
 *     dst ------------------------ output, output buffer
 *     value ---------------------- input, uint8_t value
 *     size ----------------------- input, bytes of dst buffer.
 * return: none
 * caution:
 *     1. please make sure dst is not NULL.
 */
void memset_(void *dst, uint8_t value, uint32_t size)
{
#if 0
    uint32_t i = 0u;

    for(; i<size; i++)
    {
        ((uint8_t *)dst)[i] = value;
    }
#else
    uint32_t i, count, tmp;
    uint32_t is_over = 0U;
    uint32_t bytes = size;
    uint8_t *a_u8 = (uint8_t *)dst;
    uint32_t *a_u32;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != dst)
    {
#endif
        tmp = ((uintptr_t)dst) & 3U;
        if(0U != tmp)
        {
            tmp = 4U - tmp;
            if(bytes > tmp)
            {
                for(i=0U; i<tmp; i++)
                {
                    a_u8[i] = value;
                }
                a_u8 = &a_u8[tmp];
                bytes -= tmp;
            }
            else
            {
                for(i=0U; i<bytes; i++)
                {
                    a_u8[i] = value;
                }
                is_over = 1U;
            }
        }
        else
        {}

        if(0U == is_over)
        {
            a_u32 = (uint32_t *)a_u8;
            count = bytes>>2;
            if(0U != count)
            {
                tmp = (uint32_t)value;
                tmp = (tmp<<8)|((uint32_t)value);
                tmp = (tmp<<8)|((uint32_t)value);
                tmp = (tmp<<8)|((uint32_t)value);
                uint32_set(a_u32, tmp, count);
                a_u32 = &(a_u32[count]);
            }
            else
            {}

            tmp = bytes&3U;
            if(0U != tmp)
            {
                a_u8 = (uint8_t *)a_u32;
                for(i=0; i<tmp; i++)
                {
                    a_u8[i] = value;
                }
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
#endif
}


/* function: memory compare, like memcmp()
 * parameters:
 *     m1 ------------------------- input, uint8_t buffer m1
 *     m2 ------------------------- input, uint8_t buffer m2
 *     size ----------------------- input, bytes of buffer m1 or m2.
 * return: 0(m1 = m2), other(m1 != m2)
 * caution:
 *     1. please make sure neither of m1,m2 is NULL.
 */
uint8_t memcmp_(const void *m1, const void *m2, uint32_t size)
{
    const uint8_t *p1 = (const uint8_t *)m1;
    const uint8_t *p2 = (const uint8_t *)m2;
    uint32_t bytes = size;
    uint8_t c = (uint8_t)0;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != m1) && (NULL != m2))
    {
#endif
        while(0U != bytes)
        {
            c = p1[0] - p2[0];
            if((uint8_t)0 != c)
            {
                break;
            }
            else
            {}

            p1 = &p1[1];
            p2 = &p2[1];
            bytes--;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return c;
}


/* function: set uint32 buffer
 * parameters:
 *     a -------------------------- output, output word buffer
 *     value ---------------------- input, input word value
 *     wordLen -------------------- input, word length of buffer a
 * return: none
 * caution:
 *     1. please make sure a is not NULL.
 */
void uint32_set(uint32_t *a, uint32_t value, uint32_t wordLen)
{
    uint32_t i = wordLen;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        while(0U != i)
        {
            --i;
            a[i] = value;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}

/* function: copy uint32 buffer
 * parameters:
 *     dst ------------------------ output, output word buffer 
 *     src ------------------------ input, input word buffer
 *     wordLen -------------------- input, word length of buffer dst or src
 * return: none
 * caution:
 *     1. please make sure neither of dst,src is NULL.
 */
void uint32_copy(uint32_t *dst, const uint32_t *src, uint32_t wordLen)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != dst) && (NULL != src))
    {
#endif
        if(dst != src)
        {
            for(i=0U; i<wordLen; i++)
            {
                dst[i] = src[i];
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


/* function: copy uint32 buffer of 8 words
 * parameters:
 *     dst ------------------------ output, output word buffer 
 *     src ------------------------ input, input word buffer
 * return: none
 * caution:
 *     1. please make sure neither of dst,src is NULL.
 */
void uint32_copy_8_words(uint32_t *dst, const uint32_t *src)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != dst) && (NULL != src))
    {
#endif
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
        dst[6] = src[6];
        dst[7] = src[7];
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: clear uint32 buffer
 * parameters:
 *     a -------------------------- input&output, word buffer a
 *     aWordLen ------------------- input, word length of buffer a
 * return: none
 * caution:
 *     1. please make sure a is not NULL.
 */
void uint32_clear(uint32_t *a, uint32_t wordLen)
{
    volatile uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
#if 1
        i = wordLen;
        while(0U != i)
        {
            i-=1U;
            a[i] = 0U;
        }
#else
        for(i=0U;i<wordLen;i++)
        {
            a[i] = 0U;
        }
#endif
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: clear uint32 buffer of 8 words
 * parameters:
 *     a -------------------------- input&output, word buffer a
 * return: none
 * caution:
 *     1. please make sure a is not NULL.
 */
void uint32_clear_8_words(uint32_t *a)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        a[0] = 0U;
        a[1] = 0U;
        a[2] = 0U;
        a[3] = 0U;
        a[4] = 0U;
        a[5] = 0U;
        a[6] = 0U;
        a[7] = 0U;
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: sleep for a while
 * parameters:
 *     count ---------------------- input, counter for sleeping
 * return: a uint32_t value, actually no use, ignore this
 * caution:  
 */
static uint32_t uint32_sleep1(uint32_t count)
{
    MEM_VOLATILE uint32_t a=0U, b=0U;
    MEM_VOLATILE uint32_t i;
    volatile uint32_t result=0U;

    for (i=0U; i<count; i++)
    {
        result |= (a - (b+i));
        a &= result;
    }

    return result;
}


/* function: sleep for a while
 * parameters:
 *     count ---------------------- input, counter for sleeping
 * return: a uint32_t value, actually no use, ignore this
 * caution:  
 */
static uint32_t uint32_sleep2(uint32_t count)
{
    MEM_VOLATILE uint32_t a=0U, b=0U;
    MEM_VOLATILE uint32_t i;
    volatile uint32_t result=0U;

    for (i=0U; i<count; i++)
    {
        result |= ((a+i) ^ b);
        b ^= result;
    }

    return result;
}


/* function: sleep for a while
 * parameters:
 *     count ---------------------- input, count
 *     rand_bit ------------------- input, random bit, only the LSB works
 * return: none
 * caution:  
 */
void uint32_sleep(uint32_t count, uint8_t rand_bit)
{
    if(0U == (((uint32_t)rand_bit) & 0x01U))
    {
        (void)uint32_sleep1(count);
    }
    else
    {
        (void)uint32_sleep2(count);
    }
}


#if 0
/* function: convert 0x1122334455667788 to 0x4433221188776655
 * parameters:
 *     in ------------------------- source address
 *     out ------------------------ destination address
 *     wordLen -------------------- word length of in/out
 * return: none
 * caution:
 */
void uint32_endian_reverse(uint8_t *in, uint8_t *out, uint32_t wordLen)
{
    uint8_t tmp;

    if(in == out)
    {
        while(wordLen>0U)
        {
            tmp=*in;
            in[0]=in[3];
            in[3]=tmp;
            in=&(in[1]);
            tmp=*in;
            in[0]=in[1];
            in[1]=tmp;
            wordLen--;
            in=&(in[3]);
        }
    }
    else
    {
        while(wordLen>0U)
        {
            out[0] = in[3];
            out[1] = in[2];
            out[2] = in[1];
            out[3] = in[0];
            wordLen--;
            in = &(in[4]);
            out = &(out[4]);
        }
    }
}


/* function: reverse word array
 * parameters:
 *     in ------------------------- input, input buffer
 *     out ------------------------ output, output buffer
 *     wordLen -------------------- input, word length of in or out
 * return: none
 * caution:
 *    1. in and out could point the same buffer
 */
void reverse_word_array(uint8_t *in, uint32_t *out, uint32_t wordLen)
{
    uint32_t idx, round = wordLen >> 1;
    uint32_t tmp;
    uint32_t *p_in;

    if(0U != (((uint32_t)(in))&3U))
    {
        memcpy_(out, in, wordLen<<2);
        p_in = out;
    }
    else
    {
        p_in = (uint32_t *)in;
    }

    for (idx = 0U; idx < round; idx++)
    {
        tmp = p_in[idx];
        out[idx] = p_in[wordLen - 1U - idx];
        out[wordLen - 1U - idx] = tmp;
    }

    if ((0U != (wordLen & 0x1U)) && (p_in != out))
    {
        out[round] = p_in[round];
    }
    else
    {}
}
#endif


/* function: reverse byte array
 * parameters:
 *     in ------------------------- input, input buffer
 *     out ------------------------ output, output buffer
 *     byteLen -------------------- input, byte length of in or out
 * return: none
 * caution:
 *     1. please make sure neither of in,out is NULL.
 *     2. in and out could point the same buffer
 */
void reverse_byte_array(const uint8_t *in, uint8_t *out, uint32_t byteLen)
{
    uint32_t idx, round_num = byteLen >> 1;
    uint8_t tmp;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != in) && (NULL != out))
    {
#endif
        for (idx = 0U; idx < round_num; idx++)
        {
            tmp = in[idx];
            out[idx] = in[byteLen - 1U - idx];
            out[byteLen - 1U - idx] = tmp;
        }

        if ((0U != (byteLen & 0x1U)) && (in != out))
        {
            out[round_num] = in[round_num];
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: reverse byte order in every uint32_t word
 * parameters:
 *     in ------------------------- input, input byte buffer
 *     out ------------------------ output, output word buffer
 *     bytelen -------------------- input, byte length of buffer in or out
 * return: none
 * caution:  1. byteLen must be a multiple of 4
*/
#if 0
void reverse_word(uint8_t *in, uint8_t *out, uint32_t bytelen)
{
    uint32_t i, len;
    uint8_t tmp;
    uint8_t *p = in;

    if(in == out)
    {
        while(bytelen>0)
        {
            tmp=*p;
            *p=*(p+3U);
            *(p+3U)=tmp;
            p+=1U;
            tmp=*p;
            *p=*(p+1U);
            *(p+1U)=tmp;
            bytelen-=4U;
            p+=3U;
        }
    }
    else
    {
        for (i = 0U; i < bytelen; i++)
        {
            len = i >> 2;
            len = len << 3;
            out[i] = p[len + 3U - i];
        }
    }
}
#endif

/* function: reverse word order
 * parameters:
 *     in ------------------------- input, input word buffer
 *     out ------------------------ output, output word buffer
 *     wordLen -------------------- input, word length of buffer in or out
 *     reverse_word --------------- input, whether to reverse byte order in every word, 0:no, other:yes
 * return: none
 * caution:
 *     1. in DAM mode, the memory may be accessed by words, not by bytes, this function is designed
 *        for the case
 */
#if 0
void dma_reverse_word_array(uint32_t *in, uint32_t *out, uint32_t wordLen, uint32_t reverse_word)
{
    uint32_t i, j;
    uint32_t tmp;
    uint32_t *p=out;

    if(in == out)
    {
        for(i=0U; i<wordLen; i+=4U)
        {
            for (j = 0U; j < 2U; j++)
            {
                tmp = p[j];
                p[j] = p[4U - 1U - j];
                p[4U - 1U - j] = tmp;
            }
            p+=4U;
        }
    }
    else
    {
        for(i=0U; i<wordLen; i+=4U)
        {
            p[0] = in[3];
            p[1] = in[2];
            p[2] = in[1];
            p[3] = in[0];
            p+=4U;
            in+=4U;
        }
    }

    if(0U != reverse_word)
    {
        for (i = 0U; i < wordLen; i++)
        {
            tmp = *out;
            *out = tmp&0xFFU;
            *out <<= 8;
            *out |= (tmp>>8)&0xFFU;
            *out <<= 8;
            *out |= (tmp>>16)&0xFFU;
            *out <<= 8;
            *out |= (tmp>>24)&0xFFU;

            out++;
        }
    }
    else
    {}
}
#endif


/* function: reverse byte array
 * parameters:
 *     in ------------------------- input, input buffer, 32 bytes
 *     out ------------------------ output, output buffer, 8 words
 * return: none
 * caution:
 *     1. please make sure neither of in,out is NULL.
 *     2. in and out can not point the same buffer
 *     3. this is for big number of 256 bits in SM2, SM9, etc.
 */
void u8big_to_u32little_256bits(const uint8_t *in, uint32_t *out)
{
#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != in) && (NULL != out))
    {
#endif
        out[7] = ((uint32_t)in[3])  | (((uint32_t)in[2]) << 8u)  | (((uint32_t)in[1]) << 16u)  | (((uint32_t)in[0]) << 24u);
        out[6] = ((uint32_t)in[7])  | (((uint32_t)in[6]) << 8u)  | (((uint32_t)in[5]) << 16u)  | (((uint32_t)in[4]) << 24u);
        out[5] = ((uint32_t)in[11]) | (((uint32_t)in[10]) << 8u) | (((uint32_t)in[9]) << 16u)  | (((uint32_t)in[8]) << 24u);
        out[4] = ((uint32_t)in[15]) | (((uint32_t)in[14]) << 8u) | (((uint32_t)in[13]) << 16u) | (((uint32_t)in[12]) << 24u);
        out[3] = ((uint32_t)in[19]) | (((uint32_t)in[18]) << 8u) | (((uint32_t)in[17]) << 16u) | (((uint32_t)in[16]) << 24u);
        out[2] = ((uint32_t)in[23]) | (((uint32_t)in[22]) << 8u) | (((uint32_t)in[21]) << 16u) | (((uint32_t)in[20]) << 24u);
        out[1] = ((uint32_t)in[27]) | (((uint32_t)in[26]) << 8u) | (((uint32_t)in[25]) << 16u) | (((uint32_t)in[24]) << 24u);
        out[0] = ((uint32_t)in[31]) | (((uint32_t)in[30]) << 8u) | (((uint32_t)in[29]) << 16u) | (((uint32_t)in[28]) << 24u);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: reverse byte array
 * parameters:
 *     a -------------------------- input&output, 8 words
 * return: none
 * caution:
 *     1. please make sure a is not NULL.
 *     2. this is for big number of 256 bits in SM2, SM9, etc.
 */
void u8big_to_u32little_256bits_self(uint32_t *a)
{
    uint32_t tmp;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        tmp = a[7];
        a[7] = (((uint32_t)a[0])>>24) | ((((uint32_t)a[0])>>8)&0xFF00u) | ((((uint32_t)a[0])<<8)&0xFF0000u) | (((uint32_t)a[0])<<24);
        a[0] = (tmp>>24) | ((tmp>>8)&0xFF00u) | ((tmp<<8)&0xFF0000u) | (tmp<<24);
        tmp = a[6];
        a[6] = (((uint32_t)a[1])>>24) | ((((uint32_t)a[1])>>8)&0xFF00u) | ((((uint32_t)a[1])<<8)&0xFF0000u) | (((uint32_t)a[1])<<24);
        a[1] = (tmp>>24) | ((tmp>>8)&0xFF00u) | ((tmp<<8)&0xFF0000u) | (tmp<<24);
        tmp = a[5];
        a[5] = (((uint32_t)a[2])>>24) | ((((uint32_t)a[2])>>8)&0xFF00u) | ((((uint32_t)a[2])<<8)&0xFF0000u) | (((uint32_t)a[2])<<24);
        a[2] = (tmp>>24) | ((tmp>>8)&0xFF00u) | ((tmp<<8)&0xFF0000u) | (tmp<<24);
        tmp = a[4];
        a[4] = (((uint32_t)a[3])>>24) | ((((uint32_t)a[3])>>8)&0xFF00u) | ((((uint32_t)a[3])<<8)&0xFF0000u) | (((uint32_t)a[3])<<24);
        a[3] = (tmp>>24) | ((tmp>>8)&0xFF00u) | ((tmp<<8)&0xFF0000u) | (tmp<<24);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: reverse byte array
 * parameters:
 *     in ------------------------- input, input buffer, 8 words
 *     out ------------------------ output, output buffer, 32 bytes
 * return: none
 * caution:
 *     1. please make sure neither of in,out is NULL.
 *     2. in and out can not point the same buffer
 *     3. this is for big number of 256 bits in SM2, SM9, etc.
 */
void u32little_to_u8big_256bits(const uint32_t *in, uint8_t *out)
{
#if 0
    uint32_t i, j;
    uint32_t t;

    if(out == (uint8_t *)in)
    {
        for(i = 0u; i < 4u; i++)
        {
            t = in[7u-i];
            j = 28u - (i << 2);
            out[j]      = (uint8_t)((in[i] >> 24) & 0xffu);
            out[j + 1u] = (uint8_t)((in[i] >> 16) & 0xffu);
            out[j + 2u] = (uint8_t)((in[i] >> 8) & 0xffu);
            out[j + 3u] = (uint8_t)((in[i]) & 0xffu);
            j = i<<2;
            out[j]      = (uint8_t)((t >> 24) & 0xffu);
            out[j + 1u] = (uint8_t)((t >> 16) & 0xffu);
            out[j + 2u] = (uint8_t)((t >> 8) & 0xffu);
            out[j + 3u] = (uint8_t)(t & 0xffu);
        }
    }
    else
    {
        for(i = 0u; i < 8u; i++)
        {
            j = 28u - (i << 2);
            out[j]      = (uint8_t)((in[i] >> 24) & 0xffu);
            out[j + 1u] = (uint8_t)((in[i] >> 16) & 0xffu);
            out[j + 2u] = (uint8_t)((in[i] >> 8) & 0xffu);
            out[j + 3u] = (uint8_t)((in[i]) & 0xffu);
        }
    }
#elif 0
    uint32_t i, j;

    for(i = 0u; i < 8u; i++)
    {
        j = 28u - (i << 2);
        out[j]      = (uint8_t)((in[i] >> 24) & 0xffu);
        out[j + 1u] = (uint8_t)((in[i] >> 16) & 0xffu);
        out[j + 2u] = (uint8_t)((in[i] >> 8) & 0xffu);
        out[j + 3u] = (uint8_t)((in[i]) & 0xffu);
    }
#else
#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != in) && (NULL != out))
    {
#endif
        out[28] = ((uint8_t)(in[0] >> 24));
        out[29] = ((uint8_t)(in[0] >> 16));
        out[30] = ((uint8_t)(in[0] >> 8));
        out[31] = ((uint8_t)in[0]);
        out[24] = ((uint8_t)(in[1] >> 24));
        out[25] = ((uint8_t)(in[1] >> 16));
        out[26] = ((uint8_t)(in[1] >> 8));
        out[27] = ((uint8_t)in[1]);
        out[20] = ((uint8_t)(in[2] >> 24));
        out[21] = ((uint8_t)(in[2] >> 16));
        out[22] = ((uint8_t)(in[2] >> 8));
        out[23] = ((uint8_t)in[2]);
        out[16] = ((uint8_t)(in[3] >> 24));
        out[17] = ((uint8_t)(in[3] >> 16));
        out[18] = ((uint8_t)(in[3] >> 8));
        out[19] = ((uint8_t)in[3]);
        out[12] = ((uint8_t)(in[4] >> 24));
        out[13] = ((uint8_t)(in[4] >> 16));
        out[14] = ((uint8_t)(in[4] >> 8));
        out[15] = ((uint8_t)in[4]);
        out[8]  = ((uint8_t)(in[5] >> 24));
        out[9]  = ((uint8_t)(in[5] >> 16));
        out[10] = ((uint8_t)(in[5] >> 8));
        out[11] = ((uint8_t)in[5]);
        out[4]  = ((uint8_t)(in[6] >> 24));
        out[5]  = ((uint8_t)(in[6] >> 16));
        out[6]  = ((uint8_t)(in[6] >> 8));
        out[7]  = ((uint8_t)in[6]);
        out[0]  = ((uint8_t)(in[7] >> 24));
        out[1]  = ((uint8_t)(in[7] >> 16));
        out[2]  = ((uint8_t)(in[7] >> 8));
        out[3]  = ((uint8_t)in[7]);
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
#endif
}


/* function: C = A XOR B
 * parameters:
 *     A -------------------------- input, byte buffer a
 *     B -------------------------- input, byte buffer b
 *     C -------------------------- output, C = A XOR B
 *     byteLen -------------------- input, byte length of A,B,C
 * return: none
 * caution:
 *     1. please make sure none of A,B,C is NULL.
 */
void uint8_XOR(const uint8_t *A, const uint8_t *B, uint8_t *C, uint32_t byteLen)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != A) && (NULL != B) && (NULL != C))
    {
#endif
        for(i=0U; i<byteLen; i++)
        {
            C[i] = A[i] ^ B[i];
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* function: C = A XOR B
 * parameters:
 *     A -------------------------- input, word buffer a
 *     B -------------------------- input, word buffer b
 *     C -------------------------- output, C = A XOR B
 *     byteLen -------------------- input, word length of A,B,C
 * return: none
 * caution:
 *     1. please make sure none of A,B,C is NULL.
 */
void uint32_XOR(const uint32_t *A, const uint32_t *B, uint32_t *C, uint32_t wordLen)
{
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != A) && (NULL != B) && (NULL != C))
    {
#endif
        for(i=0U; i<wordLen; i++)
        {
            C[i] = A[i] ^ B[i];
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif
}


/* Function: get aimed bit value of big integer a
 * Parameters:
 *     a -------------------------- input, big integer a
 *     bit_index ------------------ input, aimed bit location
 * Return:
 *     bit value of aimed bit
 * Caution:
 *     1. please make sure a is not NULL.
 *     2. for the LSB, bit index is 0.
 */
uint32_t get_bit_value_by_index(const uint32_t *a, uint32_t bit_index)
{
    uint32_t ret = 0u;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        if(0u != (a[(bit_index) >> 5u] & ((uint32_t)1u << (bit_index & 31u))))
        {
            ret = 1u;
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


/* Function: get real bit length of big number a of wordLen words
 * Parameters:
 *     a -------------------------- input, big integer a
 *     wordLen -------------------- input, word length of a
 * Return:
 *     real bit length of big number a
 * Caution:
 *     1. please make sure a is not NULL.
 */
uint32_t get_valid_bits(const uint32_t *a, uint32_t wordLen)
{
    uint32_t i = 0U;
    uint32_t j;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        for (i = wordLen; i > 0U; i--)
        {
            if (0U != a[i - 1U])
            {
                break;
            }
            else
            {}
        }

        if(0U != i)
        {
            for (j = 32U; j > 1U; j--)
            {
                if (0U != (a[i - 1U] & (((uint32_t)0x1) << (j - 1U))))
                {
                    break;
                }
                else
                {}
            }

            i = ((i - 1U) << 5U) + j;
        }
        else
        {}
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return i;
}


/* function: get real word lenth of big number a of max_words words
 * parameters:
 *     a -------------------------- input, big integer a
 *     max_words ------------------ input, max word length of a
 * return: real word length of big number a
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t get_valid_words(const uint32_t *a, uint32_t max_words)
{
    uint32_t ret = 0;
    uint32_t i;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        i = max_words;
        while(i > 0U)
        {
            if (0U != a[i - 1U])
            {
                ret = i;
                break;
            }
            else
            {}

            i--;
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
 * return: 0(a is not zero),1(a is all zero)
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t uint8_BigNum_Check_Zero(const uint8_t *a, uint32_t aByteLen)
{
    uint32_t i;
    uint32_t ret = 0U;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        ret = 1U;
        for(i=0U; i<aByteLen; i++)
        {
            if(0U != a[i])
            {
                ret = 0U;
                break;
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether big number or uint32_t buffer a is all zero or not
 * parameters:
 *     a -------------------------- input, big integer or word buffer a
 *     aWordLen ------------------- input, word length of a
 * return: 0(a is not zero), 1(a is all zero)
 * caution:
 *     1. please make sure a is not NULL.
 */
uint32_t uint32_BigNum_Check_Zero(const uint32_t *a, uint32_t aWordLen)
{
    uint32_t i;
    uint32_t ret = 0U;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        ret = 1U;
        for(i=0U; i<aWordLen; i++)
        {
            if(0U != a[i])
            {
                ret = 0U;
                break;
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: a = a + b
 * parameters:
 *     a -------------------------- input, big number a, uint8_t big-endian
 *     a_bytes -------------------- input, byte length of a
 *     b -------------------------- input, uint8_t integer b
 *     is_secure ------------------ input, is secure implementation, 0(not), other(yes)
 * return: 0(not overflow),1(overflow)
 * caution:
 *     1. please make sure a is not NULL.
 *     2. this is mainly used for counter++ in SKE, KDF, etc.
 */
uint32_t uint8_big_num_big_endian_add_little(uint8_t *a, uint32_t a_bytes, uint8_t b, 
        uint8_t is_secure)
{
    uint32_t ret = 0U;
    uint32_t i;
    uint8_t carry = b;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        if((uint8_t)0 != is_secure)
        {
            i = a_bytes;
            while(0U != i)
            {
                i--;
                a[i] += carry;
                if(a[i] < carry)
                {
                    carry = (uint8_t)1;
                }
                else
                {
                    carry = (uint8_t)0;
                }
            }

            ret = (uint32_t)carry;
        }
        else
        {
            i = a_bytes;
            while(0U != i)
            {
                i--;
                a[i] += carry;
                if(a[i] < carry)
                {
                    carry = (uint8_t)1;
                }
                else
                {
                    carry = (uint8_t)0;
                    break;
                }
            }

            ret = (uint32_t)carry;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: a = a + b
 * parameters:
 *     a -------------------------- input, big number a, uint32_t little-endian
 *     a_words -------------------- input, word length of a
 *     b -------------------------- input, uint32_t integer b
 *     is_secure ------------------ input, is secure implementation, 0(not), other(yes)
 * return: 0(not overflow),1(overflow)
 * caution:
 *     1. please make sure a is not NULL.
 *     2. this is mainly used for public key algorithm implementation
 */
uint32_t uint32_big_num_little_endian_add_little(uint32_t *a, uint32_t a_words, uint32_t b, 
        uint8_t is_secure)
{
    uint32_t ret = 0U;
    uint32_t i;
    uint32_t carry = b;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        if((uint8_t)0 != is_secure)
        {
            for(i = 0U; i < a_words; i++)
            {
                a[i] += carry;
    #if 0
                carry = (uint32_t)(a[i] < carry);
    #else
                if(a[i] < carry)
                {
                    carry = 1U;
                }
                else
                {
                    carry = 0U;
                }
    #endif
            }

            ret = carry;
        }
        else
        {
            for(i = 0U; i < a_words; i++)
            {
                a[i] += carry;
                if(a[i] < carry)
                {
                    carry = 1U;
                }
                else
                {
                    carry = 0U;
                    break;
                }
            }

            ret = carry;
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
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
FLAG_STATIC int32_t uint32_BigNumCmp_internal(const uint32_t *a, const uint32_t *b, uint32_t words)
{
    int32_t ret = 0;
    uint32_t i = words;

    while(0U != i)
    {
        i--;
        if(a[i] > b[i])
        {
            ret = 1;
        }
        else if(a[i] < b[i])
        {
            ret = -1;
        }
        else
        {
            //nothing to do, just for static analysis.
        }

        if(0 != ret)
        {
            break;
        }
        else
        {}
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
int32_t uint32_BigNumCmp(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, uint32_t bWordLen)
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
        else
        {
            ret = uint32_BigNumCmp_internal(a, b, a_words);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: for a = b*2^t, b is odd, get t
 * parameters:
 *     a -------------------------- big integer a
 * return:
 *     number of multiple by 2, for a
 * caution:
 *     1. please make sure a is not NULL.
 *     2. make sure a != 0
 */
uint32_t Get_Multiple2_Number(const uint32_t *a)
{
    uint32_t t, i=0U, j=0U;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        while(0U == (a[i]))
        {
            i++;
        }

        t = a[i];
        for(; j<31U; j++)
        {
            if(0U != (t&(((uint32_t)1U)<<j)))
            {
                break;
            }
            else
            {}
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return (i<<5)|j;
}


/* function: a = a/(2^n), here n<32
 * parameters:
 *     a -------------------------- big integer a
 *     a_words -------------------- word length of a
 *     n -------------------------- exponent of 2^n, n<32
 * return:
 *     word length of a = a/(2^n)
 * caution:
 *     1. please make sure a is not NULL.
 *     2. make sure a_words is real word length of a and a_words is not 0
 */
FLAG_STATIC uint32_t Big_Div2n_n_less_than_32(uint32_t *a, uint32_t a_words, uint32_t n)
{
    uint32_t ret;
    uint32_t i;

    for(i=0U; i<(a_words-1U); i++)
    {
        a[i] >>= n;
        a[i] |= (a[i+1U]<<(32U-n));
    }
    a[i] >>= n;

    if(0U == a[i])
    {
        ret = i;
    }
    else
    {
        ret = a_words;
    }

    return ret;
}


/* function: a = a/(2^n), here n>=32
 * parameters:
 *     a -------------------------- big integer a
 *     a_words -------------------- word length of a
 *     n -------------------------- exponent of 2^n, n>=32
 * return:
 *     word length of a = a/(2^n),
 * caution:
 *     1. please make sure a is not NULL.
 *     2. make sure a_words is real word length of a and a_words is not 0
 *     3. actually n could be any value
 */
FLAG_STATIC uint32_t Big_Div2n_n_not_less_than_32(uint32_t *a, uint32_t a_words, uint32_t n)
{
    uint32_t ret;
    uint32_t i, j, bits;

#if 0
    j    = n/32;
    bits = n%32;
#else
    j    = n>>5U;
    bits = n&31U;
#endif

    if(j < a_words)
    {
        for(i=0; i<(a_words-j); i++)
        {
            a[i] = a[i+j];
        }
        uint32_clear(&a[a_words-j], j);

        if(0U != bits)   //bits is in [1, 31]
        {
            ret = Big_Div2n_n_less_than_32(a, a_words-j, bits);
        }
        else             //bits is 0
        {
            ret = a_words-j;
        }
    }
    else
    {
        uint32_clear(a, a_words);
        ret = 0U;
    }

    return ret;
}


/* function: a = a/(2^n)
 * parameters:
 *     a -------------------------- big integer a
 *     aWordLen ------------------- word length of a
 *     n -------------------------- exponent of 2^n
 * return:
 *     word length of a = a/(2^n)
 * caution:
 *     1. please make sure a is not NULL.
 *     2. make sure aWordLen is real word length of a
 *     3. please make sure aWordLen*32 is not less than n
 */
uint32_t Big_Div2n(uint32_t *a, uint32_t aWordLen, uint32_t n)
{
    uint32_t ret = 0U;
    uint32_t a_words;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        a_words = get_valid_words(a, aWordLen);

        if((0U == n) || (0U == a_words))
        {
            ret = a_words;
        }
        else if(n<32U)  //now a is not zero(a_words is not zero), and n is not zero either.
        {
            ret = Big_Div2n_n_less_than_32(a, a_words, n);
        }
        else  //now a is not zero(a_words is not zero), and n is greater than 31
        {
            ret = Big_Div2n_n_not_less_than_32(a, a_words, n);
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* Function: check whether a is equal to 1 or not
 * Parameters:
 *     a ---------------- pointer to uint32_t big integer a
 *     aWordLen --------- word length of big integer a
 * Return: 1(a is 1), 0(a is not 1)
 * Caution:
 *     1. please make sure a is not NULL.
 */
uint32_t Bigint_Check_1(const uint32_t *a, uint32_t aWordLen)
{
    uint32_t i;
    uint32_t ret = 0U;

#ifdef SUPPORT_STATIC_ANALYSIS
    if(NULL != a)
    {
#endif
        if((0U == aWordLen) || (a[0] != 1U))
        {
            ret = 0U;
        }
        else
        {
            ret = 1U;
            for(i=1U; i<aWordLen; i++)
            {
                if(0U != a[i])
                {
                    ret = 0U;
                    break;
                }
                else
                {}
            }
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether a is equal to p-1 or not
 * parameters:
 *     a ---------------- pointer to uint32_t big integer a
 *     p ---------------- pointer to uint32_t big integer p, p must be odd
 *     wordLen ---------- word length of a and p
 * return: 1(a is p-1), 0(a is not p-1)
 * caution:
 *     1. please make sure neither of a,p is NULL.
 *     2. please make sure p is odd
 */
uint32_t Bigint_Check_p_1(const uint32_t *a, const uint32_t *p, uint32_t wordLen)
{
    uint32_t i;
    uint32_t ret = 0U;

#ifdef SUPPORT_STATIC_ANALYSIS
    if((NULL != a) && (NULL != p))
    {
#endif
        if((0U == wordLen) || (a[0] != (p[0] - 1U)))
        {
            ret = 0U;
        }
        else
        {
            ret = 1U;
            for(i=1U; i<wordLen; i++)
            {
                if(a[i] != p[i])
                {
                    ret = 0U;
                    break;
                }
                else
                {}
            }
        }
#ifdef SUPPORT_STATIC_ANALYSIS
    }
    else
    {}
#endif

    return ret;
}


/* function: check whether integer k is in [1, n-1]
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
uint32_t uint32_integer_check(const uint32_t *k, const uint32_t *n, uint32_t wordLen, 
        uint32_t ret_zero, uint32_t ret_big, uint32_t ret_success)
{
    uint32_t ret;

    if(0U != uint32_BigNum_Check_Zero(k, wordLen))
    {
        ret = ret_zero;
    }
    else if(uint32_BigNumCmp(k, wordLen, n, wordLen) >= 0)
    {
        ret = ret_big;
    }
    else
    {
        ret = ret_success;
    }

    return ret;
}
