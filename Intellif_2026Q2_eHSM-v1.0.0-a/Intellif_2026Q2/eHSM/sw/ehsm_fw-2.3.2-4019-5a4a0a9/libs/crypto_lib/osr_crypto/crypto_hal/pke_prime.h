#ifndef PKE_PRIME_H
#define PKE_PRIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



//1:use hardware;  2:use software
#define BIGINT_DIV_CHOICE     (2U)

#if (BIGINT_DIV_CHOICE == 1U)
typedef struct {
    uint32_t low;
    uint32_t high;
}double_uint32_t;
#elif (BIGINT_DIV_CHOICE == 2U)
typedef uint32_t double_uint32_t;
#if 0
#define BIGINT_DIV_UINT32
#endif
#endif


//1:use Fermat primality test;  2:use Miller-Rabin primality test
#define PRIMALITY_TEST_CHOICE (1U)

#if (PRIMALITY_TEST_CHOICE == 1U)
#define FERMAT_ROUND          (3U)
#elif (PRIMALITY_TEST_CHOICE == 2U)
#define MILLER_RABIN_ROUND    (3U)
#endif


//prime table level(total number of small prime numbers)
#define PTL_MAX               (400U)   //the max PTL value
#define PTL_512               (400U)   //the best PTL value for prime bit length 512 (RSA1024)
#define PTL_1024              (400U)   //the best PTL value for prime bit length 1024 (RSA2048)


#define NOT_PRIME             (0xFFFFFFFFU)
#define MAYBE_PRIME           (0U)



uint32_t get_prime(uint32_t p[], uint32_t pBitLen);


#ifdef __cplusplus
}
#endif

#endif

