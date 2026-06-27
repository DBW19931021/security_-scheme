/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "selftest.h"
#include "types.h"
#include "util.h"
#include <ske/ske.h>
#include "debug.h"
#include "crypto_lib_api.h"

/***********************************************************************************************************************
 *  VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define EHSM_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define TRNG_RETRY_COUNT   3

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct ske_gcm_self_test {
    uint8_t alg;
    uint8_t *std_in;
    uint32_t std_length;
    uint8_t *std_aad;
    uint32_t add_length;
    uint8_t *std_key;
    uint16_t key_id;
    uint8_t *std_iv;
    uint32_t iv_length;
    uint8_t *std_out;
    uint8_t *std_mac;
    uint32_t mac_length;
} ske_gcm_self_test_st;

typedef struct ske_basic_mode_self_test {
    uint8_t alg;
    uint8_t test_mode;
    const uint8_t *std_in;
    uint32_t std_length;
    const uint8_t *std_key;
    uint16_t key_id;
    const uint8_t *std_iv;
    const uint8_t *std_out;
} ske_basic_mode_self_test_st;

typedef struct ske_ccm_self_test {
    uint8_t alg;
    const uint8_t *std_in;
    uint32_t std_length;
    const uint8_t *std_key;
    uint16_t key_id;
    const uint8_t *std_nonce;
    uint32_t M;
    uint32_t L;
    const uint8_t *std_aad;
    uint32_t add_length;
    const uint8_t *std_out;
    const uint8_t *std_mac;
} ske_ccm_self_test_st;

typedef struct hash_self_test {
    uint8_t alg;
    const uint8_t *std_digest1;
    const uint8_t *std_digest2;
    const uint8_t *std_mac1;
    const uint8_t *std_mac2;
    uint32_t size;
} hash_self_test_st;

#if CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE
typedef struct esda_self_test {
    const eccp_curve_t *curve;
    const uint8_t *std_e;
    uint32_t e_length;
    const uint8_t *std_pukey;
    const uint8_t *std_prikey;
    const uint8_t *std_signature;
} esda_self_test_st;
#endif

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
const uint8_t std_in[32] = { 0x81, 0x70, 0x99, 0x44, 0xE0, 0xCB, 0x2E, 0x1D, 0xB5, 0xB0, 0xA4, 0x77, 0xD1, 0xA8, 0x53,
    0x9B, 0x0A, 0x87, 0x86, 0xE3, 0x4E, 0xAA, 0xED, 0x99, 0x30, 0x3E, 0xA6, 0x97, 0x55, 0x95, 0xB2, 0x45 };
const uint8_t std_key[32] = { 0xE0, 0x70, 0x99, 0xF1, 0xBF, 0xAF, 0xFD, 0x7F, 0x24, 0x0C, 0xD7, 0x90, 0xCA, 0x4F, 0xE1,
    0x34, 0xB4, 0x42, 0x60, 0xE1, 0x56, 0x8D, 0x9E, 0x85, 0x0A, 0x0C, 0x95, 0x37, 0x44, 0x02, 0xDE, 0x28 };
const uint8_t std_iv[16]
    = { 0xC7, 0x2B, 0x65, 0x91, 0xA0, 0xD7, 0xDE, 0x8F, 0x6B, 0x40, 0x72, 0x33, 0xAD, 0x35, 0x81, 0xD6 };

const uint8_t std_ecb_out_sm4[32] = { 0xCC, 0x62, 0x37, 0xA6, 0xA1, 0x35, 0x39, 0x75, 0xFF, 0xF5, 0xEE, 0x6A, 0xFD,
    0xD7, 0x70, 0x15, 0xE1, 0x32, 0x23, 0x1F, 0x18, 0xB8, 0xC9, 0x16, 0x07, 0x27, 0x9C, 0x6C, 0x7F, 0x8F, 0x7F, 0xF6 };
const uint8_t std_cbc_out_sm4[32] = { 0x60, 0x7A, 0xBE, 0xC9, 0xDA, 0xD7, 0x90, 0x73, 0xC7, 0x96, 0xDB, 0x34, 0x26,
    0xFD, 0x2C, 0x2F, 0x8E, 0x39, 0xC7, 0x0B, 0x60, 0xB2, 0x3D, 0xBE, 0xF3, 0xA9, 0xA5, 0x46, 0x65, 0x26, 0x41, 0xB7 };

#if CONFIG_BL_SELFTEST_ALGOMODE_CFB_ENABLE
const uint8_t std_cfb_out_sm4[32] = { 0xC1, 0x27, 0x47, 0xC7, 0x44, 0x0C, 0x9A, 0x5C, 0x7D, 0x51, 0x26, 0x0D, 0x1B,
    0xDB, 0x0D, 0x9D, 0x52, 0x59, 0xAD, 0x56, 0x05, 0xBE, 0x92, 0xD2, 0xB7, 0x62, 0xF5, 0xD7, 0x53, 0xD3, 0x12, 0x2A };
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_OFB_ENABLE
const uint8_t std_ofb_out_sm4[32] = { 0xC1, 0x27, 0x47, 0xC7, 0x44, 0x0C, 0x9A, 0x5C, 0x7D, 0x51, 0x26, 0x0D, 0x1B,
    0xDB, 0x0D, 0x9D, 0x0F, 0x0C, 0xAD, 0xA0, 0x2D, 0x18, 0x0B, 0x3C, 0x54, 0xA9, 0x87, 0x86, 0xBC, 0x6B, 0xF9, 0xFB };
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_CTR_ENABLE
const uint8_t std_ctr_out_sm4[32] = { 0xC1, 0x27, 0x47, 0xC7, 0x44, 0x0C, 0x9A, 0x5C, 0x7D, 0x51, 0x26, 0x0D, 0x1B,
    0xDB, 0x0D, 0x9D, 0xC3, 0x75, 0xCE, 0xBB, 0x63, 0x9A, 0x5B, 0x0C, 0xED, 0x64, 0x3F, 0x33, 0x80, 0x8F, 0x97, 0x40 };
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
const uint8_t std_xts_out_sm4[32] = { 0x94, 0x83, 0xE9, 0x1F, 0x12, 0xEE, 0x81, 0x81, 0x1A, 0x3C, 0x4C, 0xAB, 0xAC,
    0xF4, 0x01, 0xA3, 0x9D, 0xBC, 0x35, 0xC2, 0xE5, 0x37, 0x4D, 0x69, 0x73, 0xDB, 0x4D, 0x79, 0x32, 0x10, 0xC4, 0x27 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
const uint8_t std_ecb_out_aes128[32] = { 0x0B, 0x54, 0xE5, 0x9F, 0x47, 0x14, 0x4F, 0xD8, 0xEE, 0x43, 0x56, 0xCA, 0x0B,
    0x2D, 0x7A, 0x4B, 0x84, 0xD8, 0x17, 0x26, 0xE2, 0x8F, 0x59, 0xAD, 0x95, 0x56, 0x8C, 0x52, 0xDA, 0x98, 0x3F, 0x8D };
const uint8_t std_cbc_out_aes128[32] = { 0x2C, 0x1E, 0xD4, 0x56, 0x36, 0x2E, 0x00, 0x85, 0xA8, 0x1D, 0x8E, 0x61, 0x69,
    0xAD, 0x38, 0xB7, 0xB4, 0x42, 0x60, 0xE1, 0x56, 0x8D, 0x9E, 0x85, 0x0A, 0x0C, 0x95, 0x37, 0x44, 0x02, 0xDE, 0x28 };
#if CONFIG_BL_SELFTEST_ALGOMODE_CFB_ENABLE
const uint8_t std_cfb_out_aes128[32] = { 0x59, 0x11, 0x7E, 0xFA, 0xA2, 0x98, 0x1E, 0x95, 0xC2, 0xD0, 0x7A, 0x3E, 0xF0,
    0x7F, 0xD3, 0x17, 0x76, 0xC8, 0x33, 0xD5, 0x80, 0x43, 0x6D, 0x79, 0x67, 0x0B, 0x0A, 0x22, 0xE8, 0x9D, 0xFE, 0xDA };
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_OFB_ENABLE
const uint8_t std_ofb_out_aes128[32] = { 0x59, 0x11, 0x7E, 0xFA, 0xA2, 0x98, 0x1E, 0x95, 0xC2, 0xD0, 0x7A, 0x3E, 0xF0,
    0x7F, 0xD3, 0x17, 0x91, 0x5A, 0x7D, 0x8B, 0xB2, 0x2B, 0xE8, 0x1C, 0xCE, 0x3F, 0x06, 0x17, 0x38, 0x06, 0xA9, 0x88 };
#endif
const uint8_t std_ctr_out_aes128[32] = { 0x59, 0x11, 0x7E, 0xFA, 0xA2, 0x98, 0x1E, 0x95, 0xC2, 0xD0, 0x7A, 0x3E, 0xF0,
    0x7F, 0xD3, 0x17, 0xF4, 0x9C, 0x6F, 0x2D, 0x3A, 0x55, 0x7E, 0x07, 0xA8, 0xB0, 0x21, 0xDF, 0x11, 0x07, 0x05, 0xC4 };
const uint8_t std_xts_out_aes128[32] = { 0x15, 0xB9, 0x63, 0x88, 0x26, 0x0B, 0x79, 0x7A, 0xEE, 0xBD, 0xD8, 0xDC, 0x30,
    0xA3, 0x9B, 0x49, 0xCC, 0xA5, 0xF7, 0x21, 0x2D, 0x33, 0x70, 0xA1, 0x92, 0xD2, 0x5C, 0x62, 0x36, 0x51, 0xD9, 0xB5 };
#endif // CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE

const uint8_t std_aad[32] = { 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe,
    0xef, 0xab, 0xad, 0xda, 0xd2, 0x7C, 0xA4, 0x8E, 0x82, 0x99, 0xFC, 0x5A, 0xD3, 0xE9, 0x08, 0xB7, 0x65 };

#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
const uint8_t std_gcm_cipher_sm4[32] = { 0x07, 0xB2, 0xDD, 0x75, 0x80, 0x33, 0x65, 0x1D, 0x12, 0x67, 0xFF, 0x07, 0x94,
    0x09, 0xE5, 0x02, 0xD0, 0xB7, 0xFD, 0x1A, 0xE0, 0x6B, 0xA3, 0xD2, 0x6A, 0xAA, 0x0A, 0xB8, 0x4F, 0xE9, 0x95, 0x14 };
const uint8_t std_gcm_mac_sm4[16]
    = { 0x07, 0xAF, 0x76, 0x51, 0x14, 0x45, 0x30, 0xDD, 0x86, 0x56, 0x7B, 0xCA, 0x90, 0x74, 0x10, 0x8E };

const uint8_t std_gcm_cipher_aes128[32]
    = { 0x74, 0x13, 0x11, 0x88, 0xD1, 0x3C, 0x1E, 0x3A, 0xD8, 0xA0, 0x19, 0xC3, 0xD5, 0x38, 0x19, 0x63, 0x90, 0xFF,
          0x42, 0x3A, 0xEE, 0x45, 0x64, 0x7F, 0x74, 0x49, 0x07, 0x11, 0x49, 0xFF, 0x6C, 0xAD };
const uint8_t std_gcm_mac_aes128[16]
    = { 0x7D, 0x59, 0x63, 0xE8, 0x32, 0xA9, 0xF1, 0x13, 0xF8, 0xD5, 0x31, 0x8C, 0x1B, 0x2C, 0xDC, 0x3D };
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
const uint8_t std_ccm_cipher_sm4[32] = { 0x37, 0xDA, 0x7B, 0x2D, 0xF1, 0xC0, 0x27, 0x1B, 0xAB, 0x52, 0xB1, 0xCC, 0x72,
    0x57, 0x85, 0xA5, 0xA9, 0xC4, 0x4D, 0xE9, 0x74, 0x2E, 0x67, 0xE0, 0x81, 0xBA, 0xEF, 0x5E, 0x1B, 0x8B, 0x94, 0x07 };
const uint8_t std_ccm_mac_sm4[16]
    = { 0x46, 0x5C, 0x6C, 0x49, 0xE9, 0x21, 0xAD, 0xFC, 0x1F, 0x2C, 0x65, 0xAE, 0x0E, 0xBC, 0x6D, 0xC9 };

const uint8_t std_ccm_cipher_aes128[32]
    = { 0x2F, 0x95, 0xD2, 0x46, 0x2F, 0x8F, 0x2E, 0xCF, 0xDC, 0x13, 0x98, 0x9F, 0x87, 0x0A, 0x4B, 0x86, 0xEB, 0x1D,
          0x14, 0x34, 0xD7, 0xFB, 0x5B, 0x1C, 0xBB, 0x26, 0xF9, 0x33, 0xC9, 0x3A, 0x23, 0xE8 };
const uint8_t std_ccm_mac_aes128[16]
    = { 0xE9, 0xC5, 0x0A, 0x54, 0xFE, 0x2A, 0xC5, 0x2B, 0x55, 0x69, 0xB3, 0x9E, 0xFB, 0x21, 0x63, 0x66 };
#endif
const ske_basic_mode_self_test_st sm4_basic_test_data[] = {
    { SKE_ALG_SM4, SKE_MODE_ECB, std_in, 32, std_key, 0, std_iv, std_ecb_out_sm4 },
    { SKE_ALG_SM4, SKE_MODE_CBC, std_in, 32, std_key, 0, std_iv, std_cbc_out_sm4 },
#if CONFIG_BL_SELFTEST_ALGOMODE_CFB_ENABLE
    { SKE_ALG_SM4, SKE_MODE_CFB, std_in, 32, std_key, 0, std_iv, std_cfb_out_sm4 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_OFB_ENABLE
    { SKE_ALG_SM4, SKE_MODE_OFB, std_in, 32, std_key, 0, std_iv, std_ofb_out_sm4 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_CTR_ENABLE
    { SKE_ALG_SM4, SKE_MODE_CTR, std_in, 32, std_key, 0, std_iv, std_ctr_out_sm4 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
    { SKE_ALG_SM4, SKE_MODE_XTS, std_in, 32, std_key, 0, std_iv, std_xts_out_sm4 },
#endif
};
#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
const ske_basic_mode_self_test_st aes_basic_test_data[] = {
    { SKE_ALG_AES_128, SKE_MODE_ECB, std_in, 32, std_key, 0, std_iv, std_ecb_out_aes128 },
    { SKE_ALG_AES_128, SKE_MODE_CBC, std_in, 32, std_key, 0, std_iv, std_cbc_out_aes128 },
#if CONFIG_BL_SELFTEST_ALGOMODE_CFB_ENABLE
    { SKE_ALG_AES_128, SKE_MODE_CFB, std_in, 32, std_key, 0, std_iv, std_cfb_out_aes128 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_OFB_ENABLE
    { SKE_ALG_AES_128, SKE_MODE_OFB, std_in, 32, std_key, 0, std_iv, std_ofb_out_aes128 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_CTR_ENABLE
    { SKE_ALG_AES_128, SKE_MODE_CTR, std_in, 32, std_key, 0, std_iv, std_ctr_out_aes128 },
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
    { SKE_ALG_AES_128, SKE_MODE_XTS, std_in, 32, std_key, 0, std_iv, std_xts_out_aes128 },
#endif
};

#endif // CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE

#if CONFIG_BL_SELFTEST_ALGOFAM_SM2_ENABLE
const uint8_t std_prikey[32] = { 0x39, 0x45, 0x20, 0x8F, 0x7B, 0x21, 0x44, 0xB1, 0x3F, 0x36, 0xE3, 0x8A, 0xC6, 0xD3,
    0x9F, 0x95, 0x88, 0x93, 0x93, 0x69, 0x28, 0x60, 0xB5, 0x1A, 0x42, 0xFB, 0x81, 0xEF, 0x4D, 0xF7, 0xC5, 0xB8 };
const uint8_t std_pubkey[65] = { 0x04, 0x09, 0xF9, 0xDF, 0x31, 0x1E, 0x54, 0x21, 0xA1, 0x50, 0xDD, 0x7D, 0x16, 0x1E,
    0x4B, 0xC5, 0xC6, 0x72, 0x17, 0x9F, 0xAD, 0x18, 0x33, 0xFC, 0x07, 0x6B, 0xB0, 0x8F, 0xF3, 0x56, 0xF3, 0x50, 0x20,
    0xCC, 0xEA, 0x49, 0x0C, 0xE2, 0x67, 0x75, 0xA5, 0x2D, 0xC6, 0xEA, 0x71, 0x8C, 0xC1, 0xAA, 0x60, 0x0A, 0xED, 0x05,
    0xFB, 0xF3, 0x5E, 0x08, 0x4A, 0x66, 0x32, 0xF6, 0x07, 0x2D, 0xA9, 0xAD, 0x13 };
const uint8_t std_rand_k[32] = { 0x59, 0x27, 0x6E, 0x27, 0xD5, 0x06, 0x86, 0x1A, 0x16, 0x68, 0x0F, 0x3A, 0xD9, 0xC0,
    0x2D, 0xCC, 0xEF, 0x3C, 0xC1, 0xFA, 0x3C, 0xDB, 0xE4, 0xCE, 0x6D, 0x54, 0xB8, 0x0D, 0xEA, 0xC1, 0xBC, 0x21 };
const uint8_t std_E[] = { 0xF0, 0xB4, 0x3E, 0x94, 0xBA, 0x45, 0xAC, 0xCA, 0xAC, 0xE6, 0x92, 0xED, 0x53, 0x43, 0x82,
    0xEB, 0x17, 0xE6, 0xAB, 0x5A, 0x19, 0xCE, 0x7B, 0x31, 0xF4, 0x48, 0x6F, 0xDF, 0xC0, 0xD2, 0x86, 0x40 };
const uint8_t std_signature[64] = { 0xF5, 0xA0, 0x3B, 0x06, 0x48, 0xD2, 0xC4, 0x63, 0x0E, 0xEA, 0xC5, 0x13, 0xE1, 0xBB,
    0x81, 0xA1, 0x59, 0x44, 0xDA, 0x38, 0x27, 0xD5, 0xB7, 0x41, 0x43, 0xAC, 0x7E, 0xAC, 0xEE, 0xE7, 0x20, 0xB3, 0xB1,
    0xB6, 0xAA, 0x29, 0xDF, 0x21, 0x2F, 0xD8, 0x76, 0x31, 0x82, 0xBC, 0x0D, 0x42, 0x1C, 0xA1, 0xBB, 0x90, 0x38, 0xFD,
    0x1F, 0x7F, 0x42, 0xD4, 0x84, 0x0B, 0x69, 0xC4, 0x85, 0xBB, 0xC1, 0xAA };

//"message digest"
const uint8_t std_sm2_plain[] = { 0x65, 0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x74, 0x61,
    0x6E, 0x64, 0x61, 0x72, 0x64 };
// C1C3C2
const uint8_t std_sm2_cipher[19 + 97] = { 0x04, 0x04, 0xEB, 0xFC, 0x71, 0x8E, 0x8D, 0x17, 0x98, 0x62, 0x04, 0x32, 0x26,
    0x8E, 0x77, 0xFE, 0xB6, 0x41, 0x5E, 0x2E, 0xDE, 0x0E, 0x07, 0x3C, 0x0F, 0x4F, 0x64, 0x0E, 0xCD, 0x2E, 0x14, 0x9A,
    0x73, 0xE8, 0x58, 0xF9, 0xD8, 0x1E, 0x54, 0x30, 0xA5, 0x7B, 0x36, 0xDA, 0xAB, 0x8F, 0x95, 0x0A, 0x3C, 0x64, 0xE6,
    0xEE, 0x6A, 0x63, 0x09, 0x4D, 0x99, 0x28, 0x3A, 0xFF, 0x76, 0x7E, 0x12, 0x4D, 0xF0, 0x59, 0x98, 0x3C, 0x18, 0xF8,
    0x09, 0xE2, 0x62, 0x92, 0x3C, 0x53, 0xAE, 0xC2, 0x95, 0xD3, 0x03, 0x83, 0xB5, 0x4E, 0x39, 0xD6, 0x09, 0xD1, 0x60,
    0xAF, 0xCB, 0x19, 0x08, 0xD0, 0xBD, 0x87, 0x66, 0x21, 0x88, 0x6C, 0xA9, 0x89, 0xCA, 0x9C, 0x7D, 0x58, 0x08, 0x73,
    0x07, 0xCA, 0x93, 0x09, 0x2D, 0x65, 0x1E, 0xFA };
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
#if CONFIG_BL_SELFTEST_ALGOFAM_SM4_ENABLE
const ske_gcm_self_test_st sm4_gcm_test_data[] = {
    { SKE_ALG_SM4, (uint8_t *)std_in, 32, (uint8_t *)std_aad, 32, (uint8_t *)std_key, 0, (uint8_t *)std_iv, 12,
        (uint8_t *)std_gcm_cipher_sm4, (uint8_t *)std_gcm_mac_sm4, 16 },
};
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
const ske_gcm_self_test_st aes128_gcm_test_data[] = {
    { SKE_ALG_AES_128, (uint8_t *)std_in, 32, (uint8_t *)std_aad, 32, (uint8_t *)std_key, 0, (uint8_t *)std_iv, 12,
        (uint8_t *)std_gcm_cipher_aes128, (uint8_t *)std_gcm_mac_aes128, 16 },
};
#endif
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
const ske_ccm_self_test_st sm4_ccm_test_data[] = {
    { SKE_ALG_SM4, (uint8_t *)std_in, 32, (uint8_t *)std_key, 0, (uint8_t *)std_iv, 16, 3, (uint8_t *)std_aad, 32,
        (uint8_t *)std_ccm_cipher_sm4, (uint8_t *)std_ccm_mac_sm4 },
};
#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
const ske_ccm_self_test_st aes128_ccm_test_data[] = {
    { SKE_ALG_AES_128, (uint8_t *)std_in, 32, (uint8_t *)std_key, 0, (uint8_t *)std_iv, 16, 3, (uint8_t *)std_aad, 32,
        (uint8_t *)std_ccm_cipher_aes128, (uint8_t *)std_ccm_mac_aes128 },
};
#endif
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_RSA_ENABLE
#define SELF_CHECKING_RSA_BITS (1024)
#if (SELF_CHECKING_RSA_BITS == 1024)
// n is 0xEAC6532105DC60FF...CFF30D5FB9BFE221
const uint32_t n[] = { 0xB9BFE221, 0xCFF30D5F, 0xE16CCCD2, 0xE397CA53, 0xE27E76F9, 0xC7F8C4AE, 0x1929C590, 0xEA77C224,
    0x8A1158C4, 0x521851E7, 0x78D589A2, 0xB651F00A, 0x7F1FB804, 0xE50ACB95, 0x46666B3A, 0x0D39E5A7, 0x8A8B35AD,
    0x328CDDC5, 0xEBB5A873, 0xD4C45D10, 0x021A8D8B, 0x024A8D7E, 0xD9E8D3DE, 0x4ADD551B, 0xDCF5870E, 0xA11C9496,
    0x13B0A673, 0xBB6B624F, 0x50F3297A, 0x034A7288, 0x05DC60FF, 0xEAC65321 };
const uint32_t e[] = { 0x00010001 };
const uint32_t d[] = { 0x22A31201, 0x32414517, 0x3A752274, 0x2DCAF14B, 0x883C6938, 0x77818237, 0xFFF374A6, 0x23AC513D,
    0xD99BD905, 0xB9504D56, 0x8F19334D, 0x7A1DD991, 0x9BEFDA9A, 0x3E97DA29, 0x9B7FB00E, 0x357B050D, 0x2C2924F2,
    0x88F16019, 0x67A8DE53, 0xACE82DDA, 0x37CCFD34, 0x3CE56895, 0xCF0A5495, 0x8A1C9730, 0x7E2A24D1, 0xB36131E6,
    0x3D113C2E, 0x3EB8B61D, 0xF80256A4, 0x4FCB9FF4, 0x65798789, 0x7788F2DA };
const uint32_t p[] = { 0x88E09719, 0x1EA7A476, 0xA345B62A, 0x3361BDDD, 0xD445FA6A, 0x6F1A7691, 0xF6267649, 0xCD4E1D95,
    0x38A13B37, 0x7A1B8FF0, 0xFFE2E0DC, 0x5B88DCEF, 0x5BFF1C83, 0xA4DB0C00, 0xBA770091, 0xED630D26 };
const uint32_t q[] = { 0xA3B0AC49, 0x119CFBD7, 0x5B9C4781, 0xD7585D6E, 0xA2A8CB35, 0xD858CC7A, 0x0CD84FA1, 0x9F256E77,
    0x0510B779, 0x70F3AF31, 0x554464D6, 0x38F6F4AA, 0x1840A831, 0xEC0DEC9F, 0x81E3E97A, 0xFD2ED6F6 };
const uint32_t dp[] = { 0x10234101, 0x48D54C83, 0x7B2E16D4, 0xD5CF242F, 0x995265D9, 0xF39FC6FE, 0x34E2762D, 0x597D9777,
    0x8F1089BB, 0xAB0765DB, 0x8718DB05, 0x42BB42E1, 0xC113930C, 0xEEC5D0AA, 0x8B562865, 0x2DC8CCE0 };
const uint32_t dq[] = { 0x1CBBE1A9, 0x355D9E7B, 0xC648E880, 0xE6B54427, 0x435CED4C, 0x89092E4A, 0x8272B807, 0x9A929CAF,
    0x1CE4F1C5, 0x697889BD, 0x6FA23772, 0xAD964C3A, 0x2FA9FBB4, 0x391DB805, 0xF13A1B7B, 0x90B04CFE };
const uint32_t u[] = { 0xCD545DEF, 0x7054F2EE, 0x112844E9, 0xE1FA120D, 0x5AC1F87E, 0x7A9F0AC0, 0x6EA168A1, 0x38FA13B9,
    0x26374559, 0x4C9525E0, 0xEAAFA157, 0x07E6994E, 0x4EEC4549, 0x2CE40491, 0xDBEE37CD, 0x1A938C74 };
const uint32_t std_out[] = { 0x69BA3440, 0x154C549A, 0x4181553A, 0x67A7ADAB, 0xF594F2C8, 0x8A0C741E, 0x83E000CD,
    0x77622E49, 0xABE4A7DE, 0xC27ABB6C, 0xF34F6645, 0xA4E43153, 0x145D12C2, 0x3581A810, 0x54112D59, 0x6DFD6FB5,
    0xA23FEDEE, 0x67B0D772, 0xEBECA719, 0x886408DE, 0xBDF899EF, 0x65CB91B9, 0xCEA26FB7, 0x514FB50A, 0x722F29B8,
    0xCED6856C, 0x169D2851, 0xED6B791D, 0x43855BD5, 0x27A8F88F, 0x27AC014F, 0x57AE25F1 };
#else // RSA2048
// n is 0xA7C5D82068EC9949...3958A753BB5248B9
const uint32_t n[] = { 0xBB5248B9, 0x3958A753, 0x8374E33D, 0x361B77F2, 0xCB85F949, 0xCA8E668C, 0x66EDE655, 0x3DDEE69E,
    0x8AACE682, 0x99F46E90, 0x4492985F, 0x29F0F9C0, 0xEBF0722B, 0xE0179832, 0xCF18AE20, 0xE665C7C1, 0x0CBE0F3A,
    0x40B1FD85, 0x55F30FEC, 0x667DD0CF, 0x0445664A, 0x324C8E9A, 0x86ACD519, 0xBB6B6E72, 0xF2B5D9FE, 0x147B9465,
    0xD1A21022, 0xCBA4B564, 0xFD868F9A, 0x5FBDFE51, 0x66635C00, 0xD477C1B2, 0xD154302A, 0x60BD6712, 0x40451294,
    0xBD3D93D6, 0xB99E7133, 0x42BF8DF8, 0x481A2235, 0x4F4290A2, 0xFE90B45E, 0x892B8244, 0x0A33F5FA, 0x381E10F9,
    0xA2B41B34, 0xFCD8D1E4, 0x865B96DA, 0xF46D1760, 0xCC5085D7, 0x88EA540B, 0xED35377E, 0x07160284, 0xFA1C1603,
    0xFB6CC61F, 0x0C76B0BE, 0x56353B18, 0x3ED0F658, 0x4C9FA8B4, 0x59CC2863, 0xC2128C81, 0x371A8B13, 0x6C807C1E,
    0x68EC9949, 0xA7C5D820 };
const uint32_t e[] = { 0x00010001 };
const uint32_t d[] = { 0xBFBF5039, 0xE52AE46D, 0x52610295, 0x72AAF83F, 0x28D40FA1, 0x270A858B, 0x8DEAB989, 0x78ED0FB3,
    0x29946A0E, 0xC56355E4, 0x3F186B53, 0xD486B4B3, 0x63F56868, 0xE450D0AE, 0x1722616A, 0x77199EFC, 0x02FAEA03,
    0xBE0DA0CA, 0xB94C869D, 0xA9A94DD5, 0x17082AB9, 0xE71BEE19, 0xDDF45A1D, 0x7D1BE624, 0x65397435, 0x40B8B56E,
    0x4B591AA6, 0x13FC9A6D, 0x0FD48ABF, 0xD7B78EDB, 0x71C18BCF, 0x073EA51C, 0x6583789F, 0xFA407806, 0xEDC5285D,
    0x6DEE416C, 0x5A2C7BF6, 0x71BBAFF4, 0xEEB3A6B3, 0xA9ABFDBC, 0x8391DB1F, 0x667ACF54, 0x1687748F, 0x50ECE2F3,
    0xF7E9B284, 0x9B0EDECC, 0x3D4EE14C, 0x5C7EE93B, 0x0B048AC1, 0x2DCE686A, 0xE03B91A2, 0xECBB1EF3, 0xD88E255B,
    0xEE2863B5, 0x8C9D97F5, 0x062BC46F, 0x8BE63E86, 0x37A59DA4, 0x126551A2, 0x3B462FDE, 0xEDB1B18C, 0x5E487B35,
    0x92D2FDE8, 0x7FBA29CE };
const uint32_t p[] = { 0xD2C5399F, 0xF3EF5110, 0x04970D25, 0xE64CAEC5, 0x5D5A068D, 0xA7CE6463, 0x80E71F9D, 0x69331C79,
    0x45871E7C, 0xA0B6B1D0, 0xB5B27E5B, 0x538F84A9, 0x8DB003F7, 0x68356ACE, 0x42252C1D, 0xDD889308, 0x9D55A1E2,
    0xB51B0532, 0x6D605FBA, 0xEDAE1122, 0xB5F34AF5, 0xB552403E, 0x7CC23ED4, 0xA10075A3, 0xB88DD8E3, 0xDA3A1A7C,
    0x46942D7E, 0x718EBCF2, 0x339F1881, 0xFF79A8CA, 0x394262E7, 0xD38DFA8E };
const uint32_t q[] = { 0xC39D0EA7, 0x7C85F825, 0x479D951E, 0x65D3CC95, 0xB544E463, 0x5D704D69, 0x7D3B641D, 0xE5D7637B,
    0x3BB96DE8, 0x3CF3EAFE, 0x6E36C52E, 0xB8148229, 0xBB82F49E, 0xE6A13EFE, 0x5BA6BCA1, 0xD0C9CCF0, 0xE3B07AE5,
    0x67EEF090, 0x6B9D44C0, 0x6AF163AC, 0x53749666, 0xB90FAA34, 0x770B3AB0, 0x727A2482, 0x0633A781, 0x767780EA,
    0x1A0CBC6A, 0xEB01038D, 0x1E1FD9C1, 0xA2D9284B, 0xDED4E7A4, 0xCB052788 };
const uint32_t dp[] = { 0xDB507F89, 0x23B7FCFD, 0x5C1C714E, 0xC6D07B7A, 0x9357ABA1, 0x6739F404, 0x929A6045, 0xCD381F53,
    0x14E8EB0D, 0x0EA5CD18, 0xBFCF414E, 0x0067D48B, 0xF19AC916, 0x5A536CA3, 0xD45FA412, 0x47C0DF86, 0x0B4A9B2E,
    0xB9644313, 0xB1C99B7B, 0xFA188B68, 0x59063167, 0x9DF56E18, 0xF133AC96, 0x0A299C90, 0x61CE43C5, 0x2F6C834A,
    0x8A572FDF, 0x8EE58583, 0xE8151555, 0x4AE7E982, 0xC79D7147, 0x4CD6F8F6 };
const uint32_t dq[] = { 0x10101715, 0x0F98249C, 0xBB96FB4E, 0xE5BFD64A, 0x6BA46F1C, 0x9940C563, 0x8F40084E, 0xA37A7032,
    0xEDA54D31, 0xAE6AC537, 0x7DC043DF, 0xCA9EA769, 0x8E982024, 0xBD410FA8, 0x971F310C, 0xA5EA69CF, 0x95F7FDDA,
    0x7783167C, 0xABA3CECF, 0x0F3B41FD, 0xA45AFA22, 0x89F0CCF2, 0xE173F472, 0xB7283956, 0x257EE399, 0xDE489159,
    0xD8C9E2D1, 0x1E7364EB, 0x22564A75, 0x7EBD126C, 0x8041927F, 0x847AF297 };
const uint32_t u[] = { 0x273134B8, 0x811F3B02, 0x61385B16, 0x4940D62E, 0xAC11E62E, 0x90657139, 0xE0BA820D, 0x3517F5E3,
    0xA0152677, 0xFC65FE57, 0xCF28A8AF, 0x65E9080E, 0xD88F479E, 0xF9775F4D, 0x60BF9490, 0xB2BAEE6E, 0x5A68CD17,
    0xB549B32F, 0xF86E8A0F, 0x0376A3B5, 0xB34A8B3D, 0x93B8C51A, 0xE4A04408, 0x2250EA7A, 0xD905105E, 0x2367E8BE,
    0x8C6F6643, 0x0123C7C4, 0xE9C2A57D, 0xE3939FE8, 0xFF8A801C, 0xC99AC5E1 };
const uint32_t std_out[] = { 0x65DE5107, 0xEC26699D, 0x0D76CE00, 0x397AD40D, 0x0DBD4F97, 0xF30D4311, 0x48CA76D0,
    0x5F2723C7, 0xE3A21752, 0xCB6F36A3, 0xDA1204DB, 0x66DF3D61, 0x0334BACC, 0xEC127801, 0x90AD6D35, 0x9307AE25,
    0x4B6F3716, 0x74B04B2C, 0x61E072D6, 0x9DA1A085, 0x4E685D85, 0x640EC696, 0x6AAF6343, 0x87351877, 0x70C97CE5,
    0x7B035CED, 0xCB2C2C9F, 0x17620FA2, 0x724EBFBE, 0x9F61B6F8, 0xD58F829B, 0x2F299CC5, 0xCA5D6B61, 0x2095905B,
    0xB3C8958A, 0x36DA96E2, 0x0DD9993C, 0x8C8A76EC, 0x91B75607, 0xB25A39A3, 0x8EDB1559, 0xE5D2A164, 0x3053F1B2,
    0x4AB1E95E, 0x1EEB0CAB, 0xD6EF57D0, 0x5913D85F, 0x64EEC60E, 0xCFF42207, 0x1F8065E2, 0x6ED6A7AB, 0x705F2F9E,
    0xD67FFDE5, 0x03DAAFED, 0x3372EFF6, 0x4F58C1A8, 0x3A1B21D6, 0x1AD49793, 0x3589FF12, 0xA222521F, 0x8A78096C,
    0x749AC189, 0x411577B9, 0x7FD2D10E };
#endif
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE
const uint8_t std_digest1_sha3[28] = { 0xE6, 0x42, 0x82, 0x4C, 0x3F, 0x8C, 0xF2, 0x4A, 0xD0, 0x92, 0x34, 0xEE, 0x7D,
    0x3C, 0x76, 0x6F, 0xC9, 0xA3, 0xA5, 0x16, 0x8D, 0x0C, 0x94, 0xAD, 0x73, 0xB4, 0x6F, 0xDF };
const uint8_t std_digest2_sha3[28] = { 0x21, 0x64, 0x9A, 0xC9, 0xD0, 0x43, 0x0D, 0x74, 0xD5, 0xF5, 0x26, 0x16, 0x53,
    0xDA, 0x46, 0xC8, 0x7A, 0xC1, 0x56, 0x05, 0x40, 0xB4, 0x57, 0x91, 0x01, 0x08, 0x34, 0x61 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM3_ENABLE
const uint8_t std_mac1_sm3[32] = { 0xC7, 0x30, 0xD1, 0x1F, 0x7A, 0x09, 0xE1, 0x65, 0xDC, 0x6E, 0xD6, 0x96, 0x4B, 0x49,
    0x22, 0x52, 0xB2, 0x5C, 0x07, 0x50, 0x8B, 0x3B, 0xAA, 0xD6, 0x28, 0x42, 0xAF, 0xE9, 0x2A, 0x45, 0x79, 0x0F };
const uint8_t std_mac2_sm3[32] = { 0x4D, 0xC4, 0xD8, 0x6E, 0xD6, 0x37, 0xC8, 0x98, 0xCE, 0x59, 0x91, 0x4A, 0x7B, 0x69,
    0xEE, 0x50, 0x14, 0xDE, 0xD6, 0x24, 0xCD, 0x79, 0xD0, 0x5D, 0x49, 0x36, 0x5B, 0xF6, 0xB5, 0x13, 0x75, 0x2F };
const uint8_t std_digest1_sm3[32] = { 0x66, 0xC7, 0xF0, 0xF4, 0x62, 0xEE, 0xED, 0xD9, 0xD1, 0xF2, 0xD4, 0x6B, 0xDC,
    0x10, 0xE4, 0xE2, 0x41, 0x67, 0xC4, 0x87, 0x5C, 0xF2, 0xF7, 0xA2, 0x29, 0x7D, 0xA0, 0x2B, 0x8F, 0x4B, 0xA8, 0xE0 };
const uint8_t std_digest2_sm3[32] = { 0x4B, 0x28, 0x33, 0xC1, 0x58, 0xDD, 0x41, 0x61, 0x4B, 0x76, 0xE3, 0x7F, 0x18,
    0x88, 0x92, 0x43, 0xBD, 0x6B, 0x4A, 0x74, 0x4E, 0x36, 0xDE, 0x60, 0x92, 0x0A, 0x2F, 0x89, 0xE4, 0x09, 0xC6, 0x4E };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA256_ENABLE
const uint8_t std_mac1_sha256[32] = { 0xCA, 0x7B, 0x86, 0xA2, 0xFB, 0xF0, 0x03, 0x1A, 0xCC, 0xFC, 0x5F, 0xD5, 0x8E,
    0xD4, 0x60, 0xC3, 0xAB, 0x2A, 0xEB, 0xAF, 0x8E, 0x86, 0xCB, 0xBC, 0x18, 0xC8, 0x38, 0xBC, 0x00, 0xAE, 0x87, 0xFF };
const uint8_t std_mac2_sha256[32] = { 0xE0, 0xFC, 0x11, 0xA3, 0x1F, 0x1F, 0x2B, 0x32, 0x9E, 0x22, 0x78, 0x64, 0x90,
    0x6E, 0x9A, 0x8B, 0x39, 0xDE, 0x64, 0x7B, 0xE9, 0xE0, 0xA4, 0x56, 0xFE, 0x50, 0x9E, 0x8B, 0x63, 0xF1, 0x11, 0xAF };
const uint8_t std_digest1_sha256[32] = { 0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA, 0x41, 0x41, 0x40, 0xDE, 0x5D,
    0xAE, 0x22, 0x23, 0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C, 0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD };
const uint8_t std_digest2_sha256[32] = { 0xBC, 0xE0, 0xAF, 0xF1, 0x9C, 0xF5, 0xAA, 0x6A, 0x74, 0x69, 0xA3, 0x0D, 0x61,
    0xD0, 0x4E, 0x43, 0x76, 0xE4, 0xBB, 0xF6, 0x38, 0x10, 0x52, 0xEE, 0x9E, 0x7F, 0x33, 0x92, 0x5C, 0x95, 0x4D, 0x52 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_MD5_ENABLE
const uint8_t std_digest1_md5[16]
    = { 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0, 0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 };
const uint8_t std_digest2_md5[16]
    = { 0x7A, 0xCE, 0xDD, 0x1A, 0x84, 0xA4, 0xCF, 0xCB, 0x6E, 0x7A, 0x16, 0x00, 0x32, 0x42, 0x94, 0x5E };
const uint8_t std_mac1_md5[16]
    = { 0x95, 0x78, 0x20, 0xE8, 0xFC, 0x07, 0xC8, 0xF3, 0x00, 0xE9, 0x41, 0xDF, 0x5B, 0xEE, 0x04, 0xD7 };
const uint8_t std_mac2_md5[16]
    = { 0x03, 0x95, 0xE5, 0xE5, 0xEA, 0x2C, 0x9C, 0x42, 0x5A, 0xA5, 0x33, 0xE0, 0xED, 0xE2, 0x99, 0x06 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA1_ENABLE
const uint8_t std_digest1_sha1[20] = { 0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E, 0x25, 0x71, 0x78,
    0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D };
const uint8_t std_digest2_sha1[20] = { 0x1E, 0x66, 0x34, 0xBF, 0xAE, 0xBC, 0x03, 0x48, 0x29, 0x81, 0x05, 0x92, 0x3D,
    0x0F, 0x26, 0xE4, 0x7A, 0xA3, 0x3F, 0xF5 };
const uint8_t std_mac1_sha1[20] = { 0x45, 0x7E, 0xA2, 0x58, 0x23, 0x1A, 0x70, 0xE0, 0xBC, 0xDE, 0xE2, 0xC7, 0xAB, 0xA7,
    0xA5, 0xA8, 0x40, 0x91, 0xE6, 0xE7 };
const uint8_t std_mac2_sha1[20] = { 0x79, 0x4F, 0xA9, 0x5B, 0x09, 0x1D, 0x4B, 0x71, 0x84, 0x9B, 0x76, 0x02, 0x49, 0x58,
    0xE3, 0xCF, 0x35, 0xF3, 0x40, 0xB0 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE
const uint8_t std_mac1_sha3[28] = { 0x1A, 0xBE, 0x27, 0x9B, 0x68, 0xE8, 0x60, 0xF8, 0x98, 0x36, 0xED, 0xFD, 0x65, 0x40,
    0x01, 0xD9, 0x7B, 0x06, 0x15, 0x67, 0x3F, 0x0B, 0x73, 0x21, 0x6F, 0xEC, 0xBA, 0x30 };
const uint8_t std_mac2_sha3[28] = { 0x94, 0x70, 0x8E, 0xE1, 0x20, 0x0E, 0xD5, 0xED, 0x56, 0x7D, 0x66, 0xA1, 0x95, 0xEC,
    0x43, 0x95, 0x24, 0xD6, 0xCE, 0xA3, 0xB8, 0xE4, 0x0A, 0x7F, 0xC5, 0xEE, 0x0E, 0xE6 };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE
const uint8_t std_E_sha256[] = { 0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA, 0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE,
    0x22, 0x23, 0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C, 0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD };

const uint8_t ecc256_priKey[32] = { 0x49, 0x6E, 0x22, 0xB4, 0x35, 0x73, 0x25, 0xB7, 0xDC, 0xA4, 0x30, 0x7F, 0x96, 0xF5,
    0xE9, 0x41, 0x2D, 0x09, 0xF9, 0xC4, 0x2A, 0x4C, 0xB1, 0xDF, 0x4B, 0x71, 0xB0, 0x12, 0x26, 0x25, 0x9D, 0xCC };

const uint8_t ecc256_pubKey[64] = { 0x7B, 0x4B, 0x21, 0x1A, 0x38, 0xEC, 0xEE, 0x38, 0x32, 0x63, 0x1A, 0xEF, 0x64, 0x04,
    0x02, 0x48, 0x5E, 0x72, 0x6E, 0x08, 0x67, 0xF5, 0x1B, 0xFC, 0x9D, 0xD7, 0xEE, 0xDB, 0xD3, 0x4A, 0x83, 0xA9, 0x4E,
    0xA8, 0x19, 0x40, 0xED, 0x3A, 0xD4, 0x75, 0xB6, 0xB2, 0xC7, 0xC8, 0x8B, 0xD5, 0x9F, 0xB3, 0xC5, 0xF9, 0xC0, 0x99,
    0xF5, 0xD3, 0x55, 0xD4, 0x66, 0x6B, 0xB3, 0xFD, 0xB8, 0x23, 0x1F, 0x21 };

const uint8_t std_signature_sha256[] = { 0x4B, 0xBA, 0xA7, 0xE0, 0xF9, 0xE5, 0x01, 0x4A, 0x3F, 0xC1, 0x8E, 0x43, 0x39,
    0xA3, 0x60, 0x27, 0x79, 0x07, 0xCF, 0x20, 0xC2, 0xA3, 0x77, 0xFA, 0x75, 0xE9, 0x80, 0x83, 0x24, 0x43, 0xFF, 0x3C,
    0x7E, 0x0A, 0xDE, 0x2F, 0x8E, 0xAB, 0x6A, 0x21, 0xCD, 0xC2, 0x87, 0x68, 0xB3, 0x7C, 0x13, 0xC4, 0xE2, 0xD6, 0xFE,
    0x7A, 0x63, 0xBC, 0x31, 0x02, 0x24, 0xA3, 0x41, 0x65, 0x5F, 0x5E, 0xED, 0x2F };
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM3_ENABLE
const hash_self_test_st sm3_hash_test_data[] = {
    { HASH_SM3, std_digest1_sm3, std_digest2_sm3, std_mac1_sm3, std_mac2_sm3, 32 },
};
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA256_ENABLE
const hash_self_test_st sha256_hash_test_data[] = {
    { HASH_SHA256, std_digest1_sha256, std_digest2_sha256, std_mac1_sha256, std_mac2_sha256, 32 },
};
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_MD5_ENABLE
const hash_self_test_st md5_hash_test_data[] = {
    { HASH_MD5, std_digest1_md5, std_digest2_md5, std_mac1_md5, std_mac2_md5, 16 },
};
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA1_ENABLE
const hash_self_test_st sha1_hash_test_data[] = {
    { HASH_SHA1, std_digest1_sha1, std_digest2_sha1, std_mac1_sha1, std_mac2_sha1, 20 },
};
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE
const hash_self_test_st sha3_hash_test_data[] = {
    { HASH_SHA3_224, std_digest1_sha3, std_digest2_sha3, std_mac1_sha3, std_mac2_sha3, 28 },
};
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE
const esda_self_test_st ecdsa_test_data[] = {
    { secp256r1, std_E_sha256, 32, ecc256_pubKey, ecc256_priKey, std_signature_sha256 },
};
#endif

static uint32_t g_test_alg = 0U;
static uint32_t g_test_result = 0U;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static void selftest_set_sm4_result(cpt_ske_alg_e cpt_ske_alg_e, cpt_ske_mode_e mode, uint32_t ret);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void selftest_set_test_result(uint32_t alg_type, uint32_t ret)
{
    g_test_alg |= alg_type;
    if (EHSM_ERR_SW_SUCCESS == ret) {
        g_test_result |= alg_type;
    } else {
        g_test_result &= ~alg_type;
    }
}

static uint32_t selftest_ske_basic_mode_test_internal(cpt_ske_alg_e alg, cpt_ske_mode_e mode, const uint8_t *std_plain,
    uint32_t byteLen, const uint8_t *key, uint16_t key_id, const uint8_t *iv, const uint8_t *std_cipher)
{
    uint8_t cipher[32];
    uint8_t replain[32];
    uint32_t out_len;
    uint32_t ret;

#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
    // ENCRYPT
    if (SKE_MODE_XTS == mode) {
        ret = cpt_ske_xts_crypto(alg, SKE_CRYPTO_ENCRYPT, key, key_id, iv, (uint8_t *)std_plain, cipher, byteLen);
    } else
#endif
    {
        ret = cpt_ske_crypto(alg, mode, SKE_CRYPTO_ENCRYPT, key, key_id, iv, SKE_NO_PADDING, std_plain,
            (uint8_t *)cipher, (uint32_t)byteLen, (uint32_t *)(&out_len));
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // DECRYPT
#if CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE
        if (SKE_MODE_XTS == mode) {
            ret = cpt_ske_xts_crypto(alg, SKE_CRYPTO_DECRYPT, key, key_id, iv, cipher, replain, byteLen);
        } else
#endif
        {
            ret = cpt_ske_crypto(alg, mode, SKE_CRYPTO_DECRYPT, key, key_id, iv, SKE_NO_PADDING, (uint8_t *)cipher,
                (uint8_t *)replain, (uint32_t)byteLen, (uint32_t *)(&out_len));
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (util_memcmp(cipher, std_cipher, byteLen) || util_memcmp(replain, std_plain, byteLen)) {
                ret = EHSM_ERR_SKE_WORK_ERROR;
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}

#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
static uint32_t selftest_ske_gcm_test_internal(cpt_ske_alg_e alg, uint8_t *std_plain, uint32_t byteLen, uint8_t *key,
    uint16_t sp_key_idx, uint8_t *iv, uint8_t ivByteLen, uint8_t *aad, uint8_t aadByteLen, uint8_t *std_cipher,
    uint8_t *std_mac, uint32_t macByteLen)
{
    uint8_t cipher[32];
    uint8_t replain[32];
    uint8_t mac[16];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = cpt_ske_gcm_crypto(alg, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, iv, ivByteLen, aad, aadByteLen, std_plain,
        cipher, byteLen, mac, macByteLen);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_gcm_crypto(alg, SKE_CRYPTO_DECRYPT, key, sp_key_idx, iv, ivByteLen, aad, aadByteLen, cipher,
            replain, byteLen, mac, macByteLen);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (util_memcmp(mac, std_mac, macByteLen) || util_memcmp(cipher, std_cipher, byteLen)
                || util_memcmp(replain, std_plain, byteLen)) {
                ret = EHSM_ERR_SKE_WORK_ERROR;
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}
#endif

#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
static uint32_t selftest_ske_ccm_test_internal(cpt_ske_alg_e alg, uint8_t *std_plain, uint32_t byteLen, uint8_t *key,
    uint16_t sp_key_idx, uint8_t *nonce, uint8_t M, uint8_t L, uint8_t *aad, uint8_t aadByteLen, uint8_t *std_cipher,
    uint8_t *std_mac)
{
    uint8_t cipher[32];
    uint8_t replain[32];
    uint8_t mac[16];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // ENCRYPT
    ret = cpt_ske_ccm_crypto(
        alg, SKE_CRYPTO_ENCRYPT, key, sp_key_idx, nonce, M, L, aad, aadByteLen, std_plain, cipher, byteLen, mac);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // DECRYPT
        ret = cpt_ske_ccm_crypto(
            alg, SKE_CRYPTO_DECRYPT, key, sp_key_idx, nonce, M, L, aad, aadByteLen, cipher, replain, byteLen, mac);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (util_memcmp(mac, std_mac, M) || util_memcmp(cipher, std_cipher, byteLen)
                || util_memcmp(replain, std_plain, byteLen)) {
                ret = EHSM_ERR_SKE_WORK_ERROR;
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
static uint32_t selftest_ske_gcm_test(ske_gcm_self_test_st *gcm_t, uint32_t test_num)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    for (uint32_t i = 0; i < test_num; i++) {
        ret = selftest_ske_gcm_test_internal(gcm_t[i].alg, (uint8_t *)gcm_t[i].std_in, gcm_t[i].std_length,
            (uint8_t *)gcm_t[i].std_key, gcm_t[i].key_id, (uint8_t *)gcm_t[i].std_iv, gcm_t[i].iv_length,
            (uint8_t *)gcm_t[i].std_aad, gcm_t[i].add_length, (uint8_t *)gcm_t[i].std_out, (uint8_t *)gcm_t[i].std_mac,
            gcm_t[i].mac_length);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}
#endif
static uint32_t selftest_ske_basic_mode_test(const ske_basic_mode_self_test_st *ske_t, uint32_t test_num)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t i = 0;

    for (i = 0; i < test_num; i++) {
        ret = selftest_ske_basic_mode_test_internal((cpt_ske_alg_e)ske_t[i].alg, (cpt_ske_mode_e)ske_t[i].test_mode,
            ske_t[i].std_in, ske_t[i].std_length, ske_t[i].std_key, ske_t[i].key_id, ske_t[i].std_iv, ske_t[i].std_out);
        log_debug("ske test, %u, alg: %u, mode: %u, ret: %u\n", i, ske_t[i].alg, ske_t[i].test_mode, ret);
        selftest_set_sm4_result(ske_t[i].alg, ske_t[i].test_mode, ret);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}

#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
static uint32_t selftest_ske_ccm_test(ske_ccm_self_test_st *ccm_t, uint32_t test_num)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    for (uint32_t i = 0; i < test_num; i++) {
        ret = selftest_ske_ccm_test_internal(ccm_t[i].alg, (uint8_t *)ccm_t[i].std_in, ccm_t[i].std_length,
            (uint8_t *)ccm_t[i].std_key, ccm_t[i].key_id, (uint8_t *)ccm_t[i].std_nonce, ccm_t[i].M, ccm_t[i].L,
            (uint8_t *)ccm_t[i].std_aad, ccm_t[i].add_length, (uint8_t *)ccm_t[i].std_out, (uint8_t *)ccm_t[i].std_mac);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
static uint32_t selftest_aes_test(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = selftest_ske_basic_mode_test(aes_basic_test_data, EHSM_ARRAY_SIZE(aes_basic_test_data));
    selftest_set_test_result(EHSM_SELF_TEST_SKE_AES, ret);
#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = selftest_ske_gcm_test(aes_gcm_test_data, EHSM_ARRAY_SIZE(aes_gcm_test_data));
    }
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = selftest_ske_ccm_test(aes_ccm_test_data, EHSM_ARRAY_SIZE(aes_ccm_test_data));
    }
#endif

    return ret;
}
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM4_ENABLE
static void selftest_set_sm4_result(cpt_ske_alg_e alg, cpt_ske_mode_e mode, uint32_t ret)
{
    uint32_t alg_type = 0xFFFFFFFF;

    if (SKE_ALG_SM4 == alg) {
        switch (mode) {
        case SKE_MODE_ECB:
            alg_type = EHSM_SELF_TEST_SKE_SM4_ECB;
            break;
        case SKE_MODE_CBC:
            alg_type = EHSM_SELF_TEST_SKE_SM4_CBC;
            break;
        case SKE_MODE_CFB:
            alg_type = EHSM_SELF_TEST_SKE_SM4_CFB;
            break;
        case SKE_MODE_OFB:
            alg_type = EHSM_SELF_TEST_SKE_SM4_OFB;
            break;
        case SKE_MODE_CTR:
            alg_type = EHSM_SELF_TEST_SKE_SM4_CTR;
            break;
        default:
            break;
        }
    }

    if (0xFFFFFFFF != alg_type) {
        selftest_set_test_result(alg_type, ret);
    }
}

static uint32_t selftest_sm4_test(void)
{
    uint32_t ret;

    ret = selftest_ske_basic_mode_test(sm4_basic_test_data, EHSM_ARRAY_SIZE(sm4_basic_test_data));
#if CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = selftest_ske_gcm_test(sm4_gcm_test_data, EHSM_ARRAY_SIZE(sm4_gcm_test_data));
    }
#endif
#if CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = selftest_ske_ccm_test(sm4_ccm_test_data, EHSM_ARRAY_SIZE(sm4_ccm_test_data));
    }
#endif

    return ret;
}
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_RSA_ENABLE
static uint32_t selftest_rsa_std_data_test(void)
{
    uint32_t in[64], out[64], out2[64];
    uint32_t eBitLen = 17;
    uint32_t nBitLen = SELF_CHECKING_RSA_BITS;
    uint32_t wordLen = (nBitLen + 31) / 32;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // sign(non-CRT mode with pub)
    uint32_set(in, 0x5a5a5a5a, wordLen);
    ret = cpt_rsa_modexp_with_pub(in, e, d, n, out, eBitLen, nBitLen);
    if (EHSM_ERR_SW_SUCCESS != ret || uint32_BigNumCmp(out, wordLen, std_out, wordLen)) {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    } else {
        ret = cpt_rsa_modexp_with_pub(in, e, d, n, out, eBitLen, nBitLen);
        if (EHSM_ERR_SW_SUCCESS != ret || uint32_BigNumCmp(out, wordLen, std_out, wordLen)) {
            ret = EHSM_ERR_PKE_WORK_ERROR;
        } else {
            // sign(CRT mode with pub)
            uint32_set((uint32_t *)in, 0x5a5a5a5a, (uint32_t)wordLen);
            uint32_clear((uint32_t *)out, (uint32_t)wordLen);
            ret = cpt_rsa_crt_modexp_with_pub(in, p, q, dp, dq, u, e, out, eBitLen, nBitLen);
            if (EHSM_ERR_SW_SUCCESS != ret || uint32_BigNumCmp(out, wordLen, std_out, wordLen)) {
                ret = EHSM_ERR_PKE_WORK_ERROR;
            } else {
                ret = cpt_rsa_crt_modexp_with_pub(in, p, q, dp, dq, u, e, out, eBitLen, nBitLen);
                if (EHSM_ERR_SW_SUCCESS != ret || uint32_BigNumCmp(out, wordLen, std_out, wordLen)) {
                    ret = EHSM_ERR_PKE_WORK_ERROR;
                } else {
                    // verify
                    uint32_set(in, 0x5a5a5a5a, wordLen);
                    uint32_copy(out, std_out, wordLen);
                    ret = cpt_rsa_modexp(out, e, n, out2, eBitLen, nBitLen);
                    if (EHSM_ERR_SW_SUCCESS != ret || uint32_BigNumCmp(out2, wordLen, in, wordLen)) {
                        ret = EHSM_ERR_PKE_WORK_ERROR;
                    } else {
                        ret = EHSM_ERR_SW_SUCCESS;
                    }
                }
            }
        }
    }

    selftest_set_test_result(EHSM_SELF_TEST_PKE_RSA, ret);
    return ret;
}
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE
static uint32_t selftest_ecdsa_test(const esda_self_test_st *ecc_t, uint32_t test_num)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t signature[192];
    uint32_t i = 0;

    for (i = 0; i < test_num; i++) {
        ret = cpt_ecdsa_verify(
            ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, ecc_t[i].std_pukey, ecc_t[i].std_signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ecdsa_sign(
                ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, NULL, ecc_t[i].std_prikey, signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_ecdsa_verify(
                    ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, ecc_t[i].std_pukey, signature);
            }
        }
        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_PKE_WORK_ERROR;
            break;
        }

        ret = cpt_ecdsa_verify(
            ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, ecc_t[i].std_pukey, ecc_t[i].std_signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ecdsa_sign(
                ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, NULL, ecc_t[i].std_prikey, signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_ecdsa_verify(
                    ecc_t[i].curve, ecc_t[i].std_e, ecc_t[i].e_length, ecc_t[i].std_pukey, signature);
            }
        }

        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_PKE_WORK_ERROR;
            break;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }

    selftest_set_test_result(EHSM_SELF_TEST_PKE_ECC, ret);
    return ret;
}
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM2_ENABLE
static uint32_t selftest_sm2_encrypt_decrypt_test(void)
{
    uint8_t cipher[19 + 97];
    uint8_t replain[19];
    uint32_t MByteLen, CByteLen;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    /***************************** stdandard data test ********************************/
    // step: decrypt standard cipher by using standard private key, the plaintext should be the same with standard
    // plaintext
    MByteLen = 0;
    CByteLen = sizeof(std_sm2_cipher);

    ret = cpt_sm2_decrypt(std_sm2_cipher, CByteLen, std_prikey, SM2_C1C3C2, replain, (&MByteLen));
    if ((EHSM_ERR_SW_SUCCESS != ret) || (CByteLen != MByteLen + 97) || util_memcmp(replain, std_sm2_plain, MByteLen)) {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    } else {
        ret = cpt_sm2_encrypt(std_sm2_plain, MByteLen, NULL, std_pubkey, SM2_C1C3C2, cipher, (&CByteLen));
        if ((EHSM_ERR_SW_SUCCESS != ret) || (CByteLen != (MByteLen + 97))
            || (0 == util_memcmp(cipher + 97, std_sm2_plain, MByteLen))) {
            ret = EHSM_ERR_PKE_WORK_ERROR;
        } else {
            ret = cpt_sm2_decrypt(cipher, CByteLen, std_prikey, SM2_C1C3C2, replain, (&MByteLen));
            if ((EHSM_ERR_SW_SUCCESS == ret) && (CByteLen == MByteLen + 97)
                && (0 == util_memcmp(replain, std_sm2_plain, MByteLen))) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_PKE_WORK_ERROR;
            }
        }
    }

    selftest_set_test_result(EHSM_SELF_TEST_PKE_SM2_ENC, ret);
    return ret;
}

static uint32_t selftest_sm2_sign_verify_test(void)
{
    uint8_t signature[64];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = cpt_sm2_verify(std_E, std_pubkey, std_signature);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_sm2_sign(std_E, NULL, std_prikey, signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_sm2_verify(std_E, std_pubkey, signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_PKE_WORK_ERROR;
            }
        } else {
            ret = EHSM_ERR_PKE_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    }

    selftest_set_test_result(EHSM_SELF_TEST_PKE_SM2_VERIFY, ret);
    return ret;
}

static uint32_t selftest_sm2_key_generate_test(void)
{
    uint8_t prikey[32];
    uint8_t pubkey[65];
    uint8_t cipher[19 + 97];
    uint8_t replain[19];
    uint8_t signature[64];
    uint32_t MByteLen, CByteLen;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // step: 1. generate key pair 2. encrypt and decrypt by using key pair 3. sign and verify by using key pair
    ret = cpt_sm2_getkey(prikey, pubkey);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        MByteLen = sizeof(std_sm2_plain);
        ret = cpt_sm2_encrypt(std_sm2_plain, MByteLen, NULL, pubkey, SM2_C1C3C2, cipher, (&CByteLen));
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_sm2_decrypt(cipher, CByteLen, prikey, SM2_C1C3C2, replain, (&MByteLen));
        }

        if ((EHSM_ERR_SW_SUCCESS == ret) && (CByteLen == (MByteLen + 97))
            && (0 == util_memcmp(replain, std_sm2_plain, MByteLen))) {
            ret = cpt_sm2_sign(std_E, NULL, prikey, signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_sm2_verify(std_E, pubkey, signature);
            }
        } else {
            ret = EHSM_ERR_PKE_WORK_ERROR;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_PKE_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t sm2_key_exchange_test(void)
{
    const uint8_t secret[] =
        /* da */
        "\x0A\xF8\x4E\x14\x3E\x52\x5C\xD0"
        "\xE9\xEA\x9E\x9C\x8B\x87\xB1\xBA"
        "\x1B\xAA\xAF\x12\x94\x51\x4D\x67"
        "\xD9\xF3\xAD\x34\x38\xC5\xE4\x5B";
    const uint8_t b_public[] =
        /* pb */
        "\x04"
        "\x5A\x53\xAA\x8D\xB3\x15\x74\x94"
        "\x99\xFB\x6A\x5B\x07\x44\x6C\x4E"
        "\xE8\xC8\x87\xCA\xDD\xAA\x84\x1E"
        "\x2B\x31\x68\x71\x88\xC8\x90\x41"
        "\xF0\xF8\x6F\x14\xF2\x4A\xF3\x70"
        "\x5A\x49\xF8\xA0\xC0\x68\xC3\x88"
        "\xE7\xB9\x42\xC5\x0B\x2E\xEA\xDF"
        "\x35\x65\xDE\x96\x0F\xF3\x03\xDB";
    const uint8_t expected_ss[] =
        /* ss */
        "\x34\xAC\xCC\x6A\xDF\xBB\xB1\xCE"
        "\xF0\x96\x79\x44\x67\xD6\xD1\xB3"
        "\x51\xFF\x1D\xAA\xC2\xEC\x6A\xBE"
        "\xEA\x71\x72\x04\xFC\xED\xE5\x86"
        "\x00\xDA\xDD\xEE\xCE\x9C\x0A\xF1"
        "\xDE\xBF\xD3\xC5\xC2\x63\x8A\x87"
        "\x03\x63\x1B\x94\xC2\x34\x1E\x25"
        "\x79\xB8\xDC\xAB\x6E\xB0\x7B\xFA";
    uint16_t expected_ss_size = 64;
    const uint8_t sm2_ext_ra[] =
        /* rA */
        "\x1C\x8F\x05\x4B\xB0\x6E\x92\x8C"
        "\x94\x03\x9E\x43\x62\x10\xCA\x6F"
        "\x8A\x9B\x7A\xAD\x35\x18\x3B\x98"
        "\x07\xC6\x09\x78\x3A\x85\xC5\x13";
    /* rA_pub */
    const uint8_t sm2_ext_ra_pub[] = "\x04"
                                     "\x4B\x82\x60\x79\x74\x2B\xEA\x10"
                                     "\x91\x0C\x35\xCC\x77\x5B\x60\x5D"
                                     "\xCB\xB8\x32\x91\x79\x79\x1A\x11"
                                     "\x90\x98\x00\x1F\xB2\x64\x52\xF1"
                                     "\x5B\xDC\x04\x4C\xAC\x5F\x32\x64"
                                     "\x1A\x61\x98\xD3\x2E\xC9\x9E\x76"
                                     "\xA6\x1D\x79\xB0\xAD\x2E\x73\x77"
                                     "\xAE\xE8\x93\xC3\x79\x86\x2A\xCD";
    /* zA */
    const uint8_t sm2_ext_zA[] = "\xB6\x9E\x56\xAD\xD1\xA3\x01\xA9"
                                 "\x35\xFD\x3D\x70\x52\xD3\x64\xE9"
                                 "\xDC\x49\x8F\xD6\x0A\x2F\x58\xB7"
                                 "\xC9\xC3\x42\x0F\xDF\x2D\xDE\xC2";
    uint8_t sm2_ext_role = 0x00;
    const uint8_t sm2_ext_RB[] =
        /* RB */
        "\x04"
        "\x04\x65\x14\xF6\x8F\x6D\xC4\xC5"
        "\xAC\x93\x91\x55\xF8\x03\x00\x09"
        "\x5C\x80\x5B\x87\x83\x77\x09\x24"
        "\x9F\xD8\x02\xC5\xFB\x02\xC8\x53"
        "\xCC\x19\x82\xA4\xDF\xEE\x09\xB8"
        "\x94\xA2\xB5\x58\xDE\x86\xB9\xE0"
        "\xCD\x5F\x8A\x57\xC8\xF6\x9F\xCA"
        "\xE6\xB9\x37\x44\xA9\xEE\x6A\x9E";
    const uint8_t sm2_ext_zB[] =
        /* zB */
        "\x12\x2C\x68\x0B\x8C\xBA\xC3\x98"
        "\x60\xAA\x89\x36\xD4\x12\xAA\x58"
        "\xE2\x96\xA5\x0E\x9E\xE6\x5C\x4B"
        "\x3C\x14\xE4\xFC\x49\x67\x6C\x94";
    const uint8_t s1s2[] = "\x6C\x12\xA4\xC4\x91\x4E\xFE\x7A"
                           "\xE7\xA0\xD5\x67\x22\x0B\xAA\xB3"
                           "\xF2\xEC\x96\xD2\xAB\xAC\x6D\xC5"
                           "\x1F\xB1\xF7\xFA\xA0\x0D\xF8\xF8";
    const uint8_t sasb[] = "\xF6\xDA\xB6\x23\xD1\x63\xC6\xEB"
                           "\x50\x3F\xC9\x55\xB6\xE7\x43\xDD"
                           "\xE0\xA8\x04\x02\x98\x6D\x6D\x48"
                           "\x5E\x86\x90\x33\xA6\x2D\x9E\x26";
    uint16_t s1s2_size = 32;
    uint16_t sasb_size = 32;

    uint8_t shared_secret[64];
    uint8_t shared_s1s2[32];
    uint8_t shared_sasb[32];
    uint32_t ret;

    ret = cpt_sm2_exchangekey(sm2_ext_role, secret, b_public, sm2_ext_ra, sm2_ext_ra_pub, sm2_ext_RB, sm2_ext_zA,
        sm2_ext_zB, expected_ss_size, shared_secret, shared_s1s2, shared_sasb);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (0 == util_memcmp(expected_ss, shared_secret, expected_ss_size))
        && (0 == util_memcmp(s1s2, shared_s1s2, s1s2_size)) && (0 == util_memcmp(sasb, shared_sasb, sasb_size))) {
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = EHSM_ERR_SELFTEST_FAILED;
    }

    return ret;
}

static uint32_t selftest_sm2_test(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // step: 1. test encrypt and decrypt 2. test sign and verify 3. key generate 4. key exchange(reserved)
    ret = selftest_sm2_encrypt_decrypt_test();
    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = selftest_sm2_sign_verify_test();
        if (ret == EHSM_ERR_SW_SUCCESS) {
            if ((EHSM_ERR_SW_SUCCESS == selftest_sm2_key_generate_test())
                && (EHSM_ERR_SW_SUCCESS == sm2_key_exchange_test())) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_SELFTEST_FAILED;
            }
            selftest_set_test_result(EHSM_SELF_TEST_PKE_SM2_KEY_EXCHG, ret);
        }
    }

    return ret;
}
#endif /* CONFIG_BL_SELFTEST_ALGOFAM_SM2_ENABLE */
static uint32_t selftest_hash_test_internal(
    cpt_hash_alg_e hash_alg, const uint8_t *std_digest1, const uint8_t *std_digest2, uint32_t digest_byte_len)
{
    uint8_t message[100];
    uint8_t digest[32];
    uint32_t i;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    util_memcpy(message, "abc", 3);

    ret = cpt_hash(hash_alg, message, 3, digest);
    if ((EHSM_ERR_SW_SUCCESS == ret) && (0 == util_memcmp(digest, std_digest1, digest_byte_len))) {
        for (i = 0; i < 100; i++) {
            message[i] = (uint8_t)i;
        }

        ret = cpt_hash(hash_alg, message, 100, digest);
        if ((EHSM_ERR_SW_SUCCESS == ret) && (0 == util_memcmp(digest, std_digest2, digest_byte_len))) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_HASH_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_HASH_WORK_ERROR;
    }

    return ret;
}
static uint32_t selftest_hmac_test_internal(
    cpt_hash_alg_e hash_alg, const uint8_t *std_mac1, const uint8_t *std_mac2, uint32_t digest_byte_len)
{
    uint16_t key_id = 0;
    uint8_t key[64];
    uint8_t message[100];
    uint8_t mac[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t i, block_byte_len;

    block_byte_len = (uint32_t)hash_get_block_word_len(hash_alg) << 2U;

    util_memcpy(message, "abc", 3);
    for (i = 0; i < 64; i++)
        key[i] = (uint8_t)i;

    ret = cpt_hmac(hash_alg, key, key_id, 3, message, 3, mac);
    if (EHSM_ERR_SW_SUCCESS == ret && (0 == util_memcmp(mac, std_mac1, digest_byte_len))) {
        for (i = 0; i < 100; i++) {
            message[i] = (uint8_t)i;
        }
        ret = cpt_hmac(hash_alg, key, key_id, block_byte_len, message, 100, mac);
        if (EHSM_ERR_SW_SUCCESS == ret && (0 == util_memcmp(mac, std_mac2, digest_byte_len))) {
            return EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_HASH_WORK_ERROR;
        }
    } else {
        ret = EHSM_ERR_HASH_WORK_ERROR;
    }

    return ret;
}

static uint32_t selftest_hash_test(const hash_self_test_st *hash_t, uint32_t test_num, bool_t do_hmac)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t i = 0;

    for (i = 0; i < test_num; i++) {
        ret = selftest_hash_test_internal(hash_t[i].alg, hash_t[i].std_digest1, hash_t[i].std_digest2, hash_t[i].size);
        // SHA3 not support HMAC
        if ((do_hmac == true) && (EHSM_ERR_SW_SUCCESS == ret)) {
            ret = selftest_hmac_test_internal(hash_t[i].alg, hash_t[i].std_mac1, hash_t[i].std_mac2, hash_t[i].size);
        }
        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}

#if CONFIG_BL_SELFTEST_ALGOFAM_SM3_ENABLE
static uint32_t selftest_sm3_test(void)
{
    uint32_t ret;

    ret = selftest_hash_test(sm3_hash_test_data, EHSM_ARRAY_SIZE(sm3_hash_test_data), true);
    selftest_set_test_result(EHSM_SELF_TEST_HASH_SM3, ret);

    return ret;
}
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_SHA256_ENABLE
static uint32_t selftest_sha256_test(void)
{
    uint32_t ret;

    ret = selftest_hash_test(sha256_hash_test_data, EHSM_ARRAY_SIZE(sha256_hash_test_data), true);
    selftest_set_test_result(EHSM_SELF_TEST_HASH_SHA2, ret);

    return ret;
}

#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_MD5_ENABLE
static uint32_t selftest_md5_test(void)
{
    uint32_t ret;

    ret = selftest_hash_test(md5_hash_test_data, EHSM_ARRAY_SIZE(md5_hash_test_data), true);
    selftest_set_test_result(EHSM_SELF_TEST_HASH_MD5, ret);

    return ret;
}
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_SHA1_ENABLE
static uint32_t selftest_sha1_test(void)
{
    uint32_t ret;

    ret = selftest_hash_test(sha1_hash_test_data, EHSM_ARRAY_SIZE(sha1_hash_test_data), true);
    selftest_set_test_result(EHSM_SELF_TEST_HASH_SHA1, ret);

    return ret;
}
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE
static uint32_t selftest_sha3_test()
{
    uint32_t ret;

    ret = selftest_hash_test(sha3_hash_test_data, EHSM_ARRAY_SIZE(sha3_hash_test_data), false);
    selftest_set_test_result(EHSM_SELF_TEST_HASH_SHA3, ret);

    return ret;
}
#endif

#define POKER_RAND_BYTE 1280 // 10240 bits, 1280=32*40
#define POKER_LENGTH    32U  // 32 bytes every round
#define POKER_ROUND     40U  // 40 rounds

static uint32_t selftest_trng_test(void)
{
    uint16_t count1[16];
    uint16_t count2[256];
    uint32_t sum;
    uint8_t rand[POKER_LENGTH];
    uint16_t i;
    uint8_t j, k, tmp;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    memset_((uint8_t *)count1, 0, 2 * 16);
    memset_((uint8_t *)count2, 0, 2 * 256);

    for (i = 0; i < (uint16_t)POKER_ROUND; i++) {
        if (EHSM_ERR_SW_SUCCESS != cpt_get_rand(rand, POKER_LENGTH)) {
            ret = EHSM_ERR_TRNG_WORK_ERROR;
            break;
        }

        for (j = 0; j < (uint16_t)POKER_LENGTH; j++) {
            for (k = 0U; k < 2U; k++) {
                if (k == 1U) {
                    tmp = (rand[j] >> 4U);
                } else {
                    (tmp = (rand[j] & 0x0FU));
                }
                count1[tmp]++;
            }
            count2[rand[j]]++;
        }
    }

    if (EHSM_ERR_TRNG_WORK_ERROR != ret) {
        /****************** m=4 ******************/
        sum = 0;
        for (i = 0; i < 16U; i++) {
            sum += ((uint32_t)count1[i]) * count1[i];
        }
        if ((409600U <= sum) && (sum <= 414492U)) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }

        /****************** m=8 ******************/
        if (EHSM_ERR_SW_SUCCESS != ret) {
            sum = 0;
            for (i = 0; i < 256U; i++) {
                sum += ((uint32_t)count2[i]) * count2[i];
            }
            if ((6400U <= sum) && (sum <= 7952U)) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_SELFTEST_FAILED;
            }
        }
    }
    selftest_set_test_result(EHSM_SELF_TEST_TRNG, ret);
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t selftest_test_alg(uint32_t test_type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (test_type & EHSM_SELF_TEST_TRNG) {
        ret = EHSM_ERR_SELFTEST_FAILED;
        for (uint32_t i = 0; i < TRNG_RETRY_COUNT; i++) {
            if (EHSM_ERR_SW_SUCCESS == selftest_trng_test()) {
                ret = EHSM_ERR_SW_SUCCESS;
                break;
            }
        }
    }
#if CONFIG_BL_SELFTEST_ALGOFAM_DES_ENABLE
    if (test_type & EHSM_SELF_TEST_SKE_DES) { }
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_3DES_ENABLE
    if (test_type & EHSM_SELF_TEST_SKE_TDES) { }
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE
    if (test_type & EHSM_SELF_TEST_SKE_AES) {
        if (EHSM_ERR_SW_SUCCESS != selftest_aes_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_SM4_ENABLE
    if (test_type & EHSM_SELF_TEST_SKE_SM4) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sm4_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif
#if CONFIG_BL_SELFTEST_ALGOFAM_RSA_ENABLE
    if (test_type & EHSM_SELF_TEST_PKE_RSA) {
        if (EHSM_ERR_SW_SUCCESS != selftest_rsa_std_data_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE
    if (test_type & EHSM_SELF_TEST_PKE_ECC) {
        if (EHSM_ERR_SW_SUCCESS != selftest_ecdsa_test(ecdsa_test_data, EHSM_ARRAY_SIZE(ecdsa_test_data))) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM2_ENABLE
    if (test_type & EHSM_SELF_TEST_PKE_SM2) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sm2_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_MD5_ENABLE
    if (test_type & EHSM_SELF_TEST_HASH_MD5) {
        if (EHSM_ERR_SW_SUCCESS != selftest_md5_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA1_ENABLE
    if (test_type & EHSM_SELF_TEST_HASH_SHA1) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sha1_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA256_ENABLE
    if (test_type & EHSM_SELF_TEST_HASH_SHA2) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sha256_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE
    if (test_type & EHSM_SELF_TEST_HASH_SHA3) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sha3_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

#if CONFIG_BL_SELFTEST_ALGOFAM_SM3_ENABLE
    if (test_type & EHSM_SELF_TEST_HASH_SM3) {
        if (EHSM_ERR_SW_SUCCESS != selftest_sm3_test()) {
            ret = EHSM_ERR_SELFTEST_FAILED;
        }
    }
#endif

    return ret;
}

void selftest_get_test_result(uint32_t *result)
{
    if (result != NULL) {
        result[0] = g_test_result;
        result[1] = g_test_alg;
    }
}
/**
 * @}
 */
