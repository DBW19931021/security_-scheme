#ifndef UTIL_H
#define UTIL_H

#include "types.h"
#include "crypto_common/utility.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void util_memcpy(void *dest, const void *src, uint32_t size)
{
    memcpy_(dest, src, size);
}

static inline void util_memset(void *dest, uint8_t val, uint32_t size)
{
    memset_(dest, val, size);
}

static inline uint32_t util_memcmp(const void *p1, const void *p2, uint32_t size)
{
    return memcmp_(p1, p2, size);
}

static inline void util_u32_copy(uint32_t *dest, const uint32_t *src, uint32_t size)
{
    uint32_copy(dest, src, size);
}

static inline void util_read_volatile_u32(uint32_t *dest, const volatile uint32_t *src, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

static inline void util_write_volatile_u32(volatile uint32_t *dest, const uint32_t *src, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

uint32_t get_uint32(const uint8_t *p);

void *util_memmove(void *dest, const void *src, uint32_t size);
uint32_t util_crc32(const uint8_t *addr, uint32_t num, uint32_t crc);
uint32_t util_data_sec_double_check(
    const uint8_t *data, const uint8_t *expect_data, uint32_t byte_len, uint32_t error_sw, uint32_t expect_sw);
void util_chartohex(const uint8_t *in, uint16_t len, uint8_t *out);
void util_hextochar(const uint8_t *in, uint16_t len, uint8_t *out);
#ifdef __cplusplus
}
#endif

#endif // UTIL_H
