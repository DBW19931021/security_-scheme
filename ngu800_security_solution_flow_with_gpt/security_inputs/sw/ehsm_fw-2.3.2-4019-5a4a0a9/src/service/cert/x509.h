#ifndef X509
#define X509
#include "component/crypto_api.h"
#include "asn1.h"
#include "types.h"

typedef mbedtls_asn1_buf mbedtls_x509_buf;

/**
 * Domain-parameter identifiers: curve, subgroup, and generator.
 *
 * \note Only curves over prime fields are supported.
 *
 * \warning This library does not support validation of arbitrary domain
 * parameters. Therefore, only standardized domain parameters from trusted
 * sources should be used. See mbedtls_ecp_group_load().
 */
/* Note: when adding a new curve:
 * - Add it at the end of this enum, otherwise you'll break the ABI by
 *   changing the numerical value for existing curves.
 * - Increment MBEDTLS_ECP_DP_MAX below if needed.
 * - Update the calculation of MBEDTLS_ECP_MAX_BITS below.
 * - Add the corresponding MBEDTLS_ECP_DP_xxx_ENABLED macro definition to
 *   mbedtls_config.h.
 * - List the curve as a dependency of MBEDTLS_ECP_C and
 *   MBEDTLS_ECDSA_C if supported in check_config.h.
 * - Add the curve to the appropriate curve type macro
 *   MBEDTLS_ECP_yyy_ENABLED above.
 * - Add the necessary definitions to ecp_curves.c.
 * - Add the curve to the ecp_supported_curves array in ecp.c.
 * - Add the curve to applicable profiles in x509_crt.c.
 * - Add the curve to applicable presets in ssl_tls.c.
 */
typedef enum {
    MBEDTLS_ECP_DP_NONE = 0,       /*!< Curve not defined. */
    MBEDTLS_ECP_DP_SECP192R1,      /*!< Domain parameters for the 192-bit curve defined by FIPS 186-4 and SEC1. */
    MBEDTLS_ECP_DP_SECP224R1,      /*!< Domain parameters for the 224-bit curve defined by FIPS 186-4 and SEC1. */
    MBEDTLS_ECP_DP_SECP256R1,      /*!< Domain parameters for the 256-bit curve defined by FIPS 186-4 and SEC1. */
    MBEDTLS_ECP_DP_SECP384R1,      /*!< Domain parameters for the 384-bit curve defined by FIPS 186-4 and SEC1. */
    MBEDTLS_ECP_DP_SECP521R1,      /*!< Domain parameters for the 521-bit curve defined by FIPS 186-4 and SEC1. */
    MBEDTLS_ECP_DP_BP256R1,        /*!< Domain parameters for 256-bit Brainpool curve. */
    MBEDTLS_ECP_DP_BP384R1,        /*!< Domain parameters for 384-bit Brainpool curve. */
    MBEDTLS_ECP_DP_BP512R1,        /*!< Domain parameters for 512-bit Brainpool curve. */
    MBEDTLS_ECP_DP_CURVE25519,     /*!< Domain parameters for Curve25519. */
    MBEDTLS_ECP_DP_SECP192K1,      /*!< Domain parameters for 192-bit "Koblitz" curve. */
    MBEDTLS_ECP_DP_SECP224K1,      /*!< Domain parameters for 224-bit "Koblitz" curve. */
    MBEDTLS_ECP_DP_SECP256K1,      /*!< Domain parameters for 256-bit "Koblitz" curve. */
    MBEDTLS_ECP_DP_CURVE448,       /*!< Domain parameters for Curve448. */
} mbedtls_ecp_group_id;


/**
 * \brief     Supported message digests.
 *
 * \warning   MD5 and SHA-1 are considered weak message digests and
 *            their use constitutes a security risk. We recommend considering
 *            stronger message digests instead.
 *
 */
/* Note: these are aligned with the definitions of PSA_ALG_ macros for hashes,
 * in order to enable an efficient implementation of conversion functions.
 * This is tested by md_to_from_psa() in test_suite_md. */
typedef enum {
    MBEDTLS_MD_NONE=0,    /**< None. */
    MBEDTLS_MD_MD5=0x03,       /**< The MD5 message digest. */
    MBEDTLS_MD_RIPEMD160=0x04, /**< The RIPEMD-160 message digest. */
    MBEDTLS_MD_SHA1=0x05,      /**< The SHA-1 message digest. */
    MBEDTLS_MD_SHA224=0x08,    /**< The SHA-224 message digest. */
    MBEDTLS_MD_SHA256=0x09,    /**< The SHA-256 message digest. */
    MBEDTLS_MD_SHA384=0x0a,    /**< The SHA-384 message digest. */
    MBEDTLS_MD_SHA512=0x0b,    /**< The SHA-512 message digest. */
    MBEDTLS_MD_SHA3_224=0x10,  /**< The SHA3-224 message digest. */
    MBEDTLS_MD_SHA3_256=0x11,  /**< The SHA3-256 message digest. */
    MBEDTLS_MD_SHA3_384=0x12,  /**< The SHA3-384 message digest. */
    MBEDTLS_MD_SHA3_512=0x13,  /**< The SHA3-512 message digest. */
} mbedtls_md_type_t;


/**
 * \brief          Public key types
 */
typedef enum {
    MBEDTLS_PK_NONE=0,
    MBEDTLS_PK_RSA,
    MBEDTLS_PK_ECKEY,
    MBEDTLS_PK_ECKEY_DH,
    MBEDTLS_PK_ECDSA,
    MBEDTLS_PK_RSA_ALT,
    MBEDTLS_PK_RSASSA_PSS,
    MBEDTLS_PK_OPAQUE,
} mbedtls_pk_type_t;

typedef struct {
    uint32_t e_size;
    uint32_t n_size;
    uint8_t  *e;
    uint8_t  *n;
} crypto_rsa_pubkey_st;

typedef struct {
    uint32_t size;
    uint8_t *key;
    mbedtls_ecp_group_id ec_grp_id;
} crypto_ecc_pubkey_st;

typedef union {
    crypto_rsa_pubkey_st rsa_key;
    crypto_ecc_pubkey_st ecc_key;
} crypto_pubkey_u;

typedef struct {
    mbedtls_pk_type_t pk_alg;
    crypto_pubkey_u pubkey;
} crypto_pubkey_st;

uint32_t mbedtls_x509_get_sig_alg(const mbedtls_x509_buf *sig_oid, const mbedtls_x509_buf *sig_params,
                             mbedtls_md_type_t *md_alg, mbedtls_pk_type_t *pk_alg,
                             void **sig_opts);
// uint32_t mbedtls_rsa_parse_pubkey(crypto_rsa_pubkey_st *rsa_key, const unsigned char *key, size_t keylen);
uint32_t mbedtls_pk_parse_subpubkey(unsigned char **p, const unsigned char *end,
                               crypto_pubkey_st *pk);
uint32_t mbedtls_parse_sign(unsigned char **p,
                         const unsigned char *end, mbedtls_pk_type_t pk_alg,
                         unsigned char *sign, size_t buf_size);
uint32_t mbedtls_parse_hash_value(
    unsigned char *digest_info, uint32_t info_size, unsigned char **hash_value, uint32_t *hash_size, cpt_hash_alg_e *md_alg);
#endif