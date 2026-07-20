/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "util.h"
#include "crypto_common/utility_sec.h"
#include "fid.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

static uint8_t util_hextochar_internal(uint8_t tmp)
{
    tmp &= 0x0fU;
    if (tmp >= 10U) {
        tmp = tmp - 0x0aU + 0x41U;
    } else {
        tmp = tmp + 0x30U;
    }

    return tmp;
}

static uint8_t util_chartohex_internal(uint8_t tmp)
{
    if ((tmp >= 0x30U) && (tmp <= 0x39U)) {
        tmp -= (uint8_t)0x30U;
    } else if ((tmp >= 0x41U) && (tmp <= 0x46U)) {
        tmp -= (uint8_t)0x37U;
    } else if ((tmp >= 0x61U) && (tmp <= 0x66U)) {
        tmp -= (uint8_t)0x57U;
    } else {
        tmp = 0xFFU;
    }

    return tmp;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void *util_memmove(void *dest, const void *src, uint32_t size)
{
    void *ret = dest;

    if ((dest != NULL) && (src != NULL)) {
        if ((uintptr_t)dest > (uintptr_t)src) {
            while (size--) {
                *((uint8_t *)dest + size) = *((const uint8_t *)src + size);
            }
        } else {
            while (size--) {
                *((uint8_t *)dest) = *((const uint8_t *)src);
                dest = (uint8_t *)dest + 1;
                src = (const uint8_t *)src + 1;
            }
        }
    }

    return ret;
}

uint32_t util_crc32(const uint8_t *addr, uint32_t num, uint32_t crc)
{
    int32_t i;
    uint32_t tmp_crc = crc;
    for (uint32_t j = 0u; j < num; j++) {
        tmp_crc = tmp_crc ^ (((uint32_t)(addr[j])) << 24u);
        for (i = 0; i < 8; i++) {
            if (0u != (tmp_crc & 0x80000000u)) {
                tmp_crc = (tmp_crc << 1u) ^ (0x04C11DB7u);
            } else {
                tmp_crc <<= 1u;
            }
        }
    }
    return (tmp_crc);
}

uint32_t get_uint32(const uint8_t *p)
{
#ifndef DATA_BIG_ENDIAN
    return (((uint32_t)p[3]) << 24u) | (((uint32_t)p[2]) << 16u) | (((uint32_t)p[1]) << 8u) | ((uint32_t)p[0]);
#else
    return (((uint32_t)p[0]) << 24u) | (((uint32_t)p[1]) << 16u) | (((uint32_t)p[2]) << 8u) | (((uint32_t)p[3]));
#endif
}

void set_uint32(uint8_t *p, uint32_t data)
{
#ifndef DATA_BIG_ENDIAN
    p[0] = (uint8_t)data;
    p[1] = (uint8_t)(data >> 8u);
    p[2] = (uint8_t)(data >> 16u);
    p[3] = (uint8_t)(data >> 24u);
#else
    p[3] = (uint8_t)data;
    p[2] = (uint8_t)(data >> 8u);
    p[1] = (uint8_t)(data >> 16u);
    p[0] = (uint8_t)(data >> 24u);
#endif
}

uint32_t util_data_sec_double_check(
    const uint8_t *data, const uint8_t *expect_data, uint32_t byte_len, uint32_t error_sw, uint32_t expect_sw)
{
    uint32_t ret;
    uint32_t cmp_data[64];
    uint32_t exp_data[64];

    if (byte_len > sizeof(cmp_data)) {
        return error_sw;
    }

    (void)util_memcpy((uint8_t *)cmp_data, data, byte_len);
    (void)util_memcpy((uint8_t *)exp_data, expect_data, byte_len);

    fid_delay();
    if (0u != uint32_cmp_sec(cmp_data, exp_data, byte_len >> 2, 0u)) {
        ret = error_sw;
    } else {
        fid_delay();
        if (0u != uint32_cmp_sec(cmp_data, exp_data, byte_len >> 2, 0u)) {
            ret = error_sw;
        } else {
            ret = expect_sw;
        }
    }

    return ret;
}

void util_hextochar(const uint8_t *in, uint16_t len, uint8_t *out)
{
    uint8_t tmp[256 * 2];

    if (len > 256U) {
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        tmp[i * 2U] = util_hextochar_internal(in[i] >> 4U);
        tmp[(i * 2U) + 1U] = util_hextochar_internal(in[i]);
    }

    util_memcpy(out, tmp, ((uint32_t)len * 2U));
}

void util_chartohex(const uint8_t *in, uint16_t len, uint8_t *out)
{
    uint8_t tmp[(1024 + 1) / 2];

    if (len > 1024U) {
        return;
    }

    for (uint16_t i = 0; i < len;) {
        tmp[i / 2U] = 0;
        tmp[i / 2U] = util_chartohex_internal(in[i]);

        if ((i + 1U) < len) {
            tmp[i / 2U] = (uint8_t)(tmp[i / 2U] << 4U) + util_chartohex_internal(in[i + 1U]);
        }
        i += 2U;
    }

    util_memcpy(out, tmp, (((uint32_t)len + 1U) / 2U));
}

/**
 *
 */
