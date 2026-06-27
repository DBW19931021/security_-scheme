#ifndef SELFTEST_H
#define SELFTEST_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define EHSM_SELF_TEST_SKE_SM4_ECB (0x1U << 0)
#define EHSM_SELF_TEST_SKE_SM4_CBC (0x1U << 1)
#define EHSM_SELF_TEST_SKE_SM4_CFB (0x1U << 2)
#define EHSM_SELF_TEST_SKE_SM4_OFB (0x1U << 3)
#define EHSM_SELF_TEST_SKE_SM4_CTR (0x1U << 4)
#define EHSM_SELF_TEST_SKE_SM4                                                                                         \
    (EHSM_SELF_TEST_SKE_SM4_ECB | EHSM_SELF_TEST_SKE_SM4_CBC | EHSM_SELF_TEST_SKE_SM4_CFB | EHSM_SELF_TEST_SKE_SM4_OFB \
        | EHSM_SELF_TEST_SKE_SM4_CTR)

#define EHSM_SELF_TEST_PKE_SM2_ENC       (0x1U << 5)
#define EHSM_SELF_TEST_PKE_SM2_VERIFY    (0x1U << 6)
#define EHSM_SELF_TEST_PKE_SM2_KEY_EXCHG (0x1U << 7)
#define EHSM_SELF_TEST_PKE_SM2 \
    (EHSM_SELF_TEST_PKE_SM2_ENC | EHSM_SELF_TEST_PKE_SM2_VERIFY | EHSM_SELF_TEST_PKE_SM2_KEY_EXCHG)

#define EHSM_SELF_TEST_HASH_SM3 (0x1U << 8)

/* For symmetric DES algorithm testing */
#define EHSM_SELF_TEST_SKE_DES (0x1U << 9)
/* For symmetric TDES algorithm testing */
#define EHSM_SELF_TEST_SKE_TDES (0x1U << 10)
/* For symmetric AES algorithm testing */
#define EHSM_SELF_TEST_SKE_AES (0x1U << 11)

/* For asymmetric RSA algorithm testing */
#define EHSM_SELF_TEST_PKE_RSA (0x1U << 12)
/* For asymmetric ECC algorithms testing */
#define EHSM_SELF_TEST_PKE_ECC (0x1U << 13)

/* For hash algorithm testing */
#define EHSM_SELF_TEST_HASH_MD5  (0x1U << 14)
#define EHSM_SELF_TEST_HASH_SHA1 (0x1U << 15)
#define EHSM_SELF_TEST_HASH_SHA2 (0x1U << 16)
#define EHSM_SELF_TEST_HASH_SHA3 (0x1U << 17)

/* For trng testing */
#define EHSM_SELF_TEST_TRNG (0x1U << 18)

/* For symmetric algorithms testing */
#define EHSM_SELF_TEST_SKE \
    (EHSM_SELF_TEST_SKE_DES | EHSM_SELF_TEST_SKE_TDES | EHSM_SELF_TEST_SKE_AES | EHSM_SELF_TEST_SKE_SM4)

/* For asymmetric algorithms testing */
#define EHSM_SELF_TEST_PKE (EHSM_SELF_TEST_PKE_RSA | EHSM_SELF_TEST_PKE_ECC | EHSM_SELF_TEST_PKE_SM2)

/* For hash algorithms testing */
#define EHSM_SELF_TEST_HASH                                                                                   \
    (EHSM_SELF_TEST_HASH_MD5 | EHSM_SELF_TEST_HASH_SHA1 | EHSM_SELF_TEST_HASH_SHA2 | EHSM_SELF_TEST_HASH_SHA3 \
        | EHSM_SELF_TEST_HASH_SM3)

/* For all testing */
#define EHSM_SELF_TEST_ALL (EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_SKE | EHSM_SELF_TEST_PKE | EHSM_SELF_TEST_HASH)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void selftest_get_test_result(uint32_t *result);

uint32_t selftest_test_alg(uint32_t test_type);

#endif /* APPLICATION_SRC_SEIP_CORE_SECURE_BOOT_H_ */
