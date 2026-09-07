#ifndef UTILITY_H
#define UTILITY_H




#include "../common_config.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef SUPPORT_STATIC_ANALYSIS
#define CAST2UINT32(a)           ((uint32_t)(a))
#define GET_MAX_LEN(a,b)         (((a)>(b))?(a):(b))
#define GET_MIN_LEN(a,b)         (((a)>(b))?(b):(a))
#define GET_WORD_LEN(bitLen)     (((bitLen)+31u)>>5)
#define GET_BYTE_LEN(bitLen)     (((bitLen)+7u)>>3)
#else
static inline uint32_t CAST2UINT32(uint32_t a)
{
    return a;
}
#endif

/* function: get max value in a and b
 * parameters:
 *     a -------------------------- input, a value
 *     b -------------------------- input, b value
 * return: max vulue in a and b
 * caution:
 */
static inline uint32_t GET_MAX_LEN(uint32_t a, uint32_t b)
{
    return (a>b)?a:b;
}


/* function: get word length from bit length
 * parameters:
 *     bit_len -------------------- input, bit length
 * return: none
 * caution:
 */
static inline uint32_t GET_WORD_LEN(uint32_t bit_len)
{
    return (bit_len+31u)>>5;
}


/* function: get byte length from bit length
 * parameters:
 *     bit_len -------------------- input, bit length
 * return: none
 * caution:
 */
static inline uint32_t GET_BYTE_LEN(uint32_t bit_len)
{
    return (bit_len+7u)>>3;
}


/* function: whether addr is word align
 * parameters:
 *     addr -------------------- input, bit length
 * return: 1(is not word align), 0(is word align)
 * caution:
 */
static inline uint32_t check_addr_not_word_align(const void *addr)
{
    uint32_t ret = 0u;

    if(0U != (((uintptr_t)addr)&3U))
    {
        ret = 1u;
    }
    else
    {}

    return ret;
}


//APIs

#ifdef UTILITY_PRINT_BUF
extern void print_buf_U8(const uint8_t *buf, uint32_t byteLen, char *name);
extern void print_buf_U32(const uint32_t *buf, uint32_t wordLen, char *name);
extern void print_BN_buf_U32(const uint32_t *buf, uint32_t wordLen, char *name);
#endif

void memcpy_(void *dst, const void *src, uint32_t size);

void memset_(void *dst, uint8_t value, uint32_t size);

uint8_t memcmp_(const void *m1, const void *m2, uint32_t size);

void uint32_set(uint32_t *a, uint32_t value, uint32_t wordLen);

void uint32_copy(uint32_t *dst, const uint32_t *src, uint32_t wordLen);

void uint32_copy_8_words(uint32_t *dst, const uint32_t *src);

void uint32_clear(uint32_t *a, uint32_t wordLen);

void uint32_clear_8_words(uint32_t *a);

void uint32_sleep(uint32_t count, uint8_t rand_bit);

void uint32_endian_reverse(uint8_t *in, uint8_t *out, uint32_t wordLen);

#if 0
void reverse_word_array(uint8_t *in, uint32_t *out, uint32_t wordLen)
#endif

void reverse_byte_array(const uint8_t *in, uint8_t *out, uint32_t byteLen);

#if 0
void reverse_word(uint8_t *in, uint8_t *out, uint32_t bytelen);

void dma_reverse_word_array(uint32_t *in, uint32_t *out, uint32_t wordlen, uint32_t reverse_word);
#endif

void u8big_to_u32little_256bits(const uint8_t *in, uint32_t *out);

void u8big_to_u32little_256bits_self(uint32_t *a);

void u32little_to_u8big_256bits(const uint32_t *in, uint8_t *out);

void uint8_XOR(const uint8_t *A, const uint8_t *B, uint8_t *C, uint32_t byteLen);

void uint32_XOR(const uint32_t *A, const uint32_t *B, uint32_t *C, uint32_t wordLen);

uint32_t get_bit_value_by_index(const uint32_t *a, uint32_t bit_index);

uint32_t get_valid_bits(const uint32_t *a, uint32_t wordLen);

uint32_t get_valid_words(const uint32_t *a, uint32_t max_words);

uint32_t uint8_BigNum_Check_Zero(const uint8_t *a, uint32_t aByteLen);

uint32_t uint32_BigNum_Check_Zero(const uint32_t *a, uint32_t aWordLen);

uint32_t uint8_big_num_big_endian_add_little(uint8_t *a, uint32_t a_bytes, uint8_t b, 
        uint8_t is_secure);

uint32_t uint32_big_num_little_endian_add_little(uint32_t *a, uint32_t a_words, uint32_t b, 
        uint8_t is_secure);

int32_t uint32_BigNumCmp(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, uint32_t bWordLen);

uint32_t Get_Multiple2_Number(const uint32_t *a);

uint32_t Big_Div2n(uint32_t *a, uint32_t aWordLen, uint32_t n);

uint32_t Bigint_Check_1(const uint32_t *a, uint32_t aWordLen);

uint32_t Bigint_Check_p_1(const uint32_t *a, const uint32_t *p, uint32_t wordLen);

uint32_t uint32_integer_check(const uint32_t *k, const uint32_t *n, uint32_t wordLen, 
        uint32_t ret_zero, uint32_t ret_big, uint32_t ret_success);

#ifdef __cplusplus
}
#endif

#endif
