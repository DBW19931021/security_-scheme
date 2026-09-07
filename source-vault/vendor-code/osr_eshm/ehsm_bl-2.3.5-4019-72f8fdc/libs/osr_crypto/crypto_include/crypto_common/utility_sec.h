#ifndef UTILITY_SEC_H
#define UTILITY_SEC_H




#include "../common_config.h"
#include "utility.h"

#ifdef __cplusplus
extern "C" {
#endif



uint16_t crc16_calc(const uint8_t *in, uint32_t bytelen, uint16_t crc);

uint32_t uint32_get_rand_big_number_msb_0(uint32_t *a, uint32_t aBitLen);

uint32_t uint8_BigNum_Check_Zero_sec(const uint8_t *a, uint32_t aByteLen);

uint32_t uint32_BigNum_Check_Zero_sec(const uint32_t *a, uint32_t aWordLen);

int32_t uint32_BigNumCmp_sec(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, uint32_t bWordLen);

uint32_t uint32_cmp_sec(const uint32_t *a, const uint32_t *b, uint32_t wordLen, uint8_t rand_bit);

uint32_t uint32_integer_check_sec(const uint32_t *k, const uint32_t *n, uint32_t wordLen, 
        uint32_t ret_zero, uint32_t ret_big, uint32_t ret_success);

uint32_t counter_add_one(volatile uint32_t *counter);

#ifdef __cplusplus
}
#endif

#endif

