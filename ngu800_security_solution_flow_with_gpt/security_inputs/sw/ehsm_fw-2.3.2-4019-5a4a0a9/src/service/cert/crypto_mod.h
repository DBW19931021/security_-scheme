#ifndef CRYPTO_MOD_H
#define CRYPTO_MOD_H
#include "types.h"
#include "oid.h"
#include "x509.h"

#define CRYPTO_AUTH_VERIFY_ONLY          1
#define CRYPTO_HASH_CALC_ONLY            2
#define CRYPTO_AUTH_VERIFY_AND_HASH_CALC 3

#define CRYPTO_RSA_KEY_E_LEN  64U
#define CRYPTO_RSA_KEY_N_LEN  512U

#define CRYPTO_ECC_KEY_LEN  65U
/* Return values */
enum crypto_ret_value {
    CRYPTO_SUCCESS = 0,
    CRYPTO_ERR_INIT,
    CRYPTO_ERR_HASH,
    CRYPTO_ERR_SIGNATURE,
    CRYPTO_ERR_DECRYPTION,
    CRYPTO_ERR_UNKNOWN
};

#define CRYPTO_MAX_IV_SIZE  16U
#define CRYPTO_MAX_TAG_SIZE 16U

#define MBEDTLS_MD_MAX_SIZE         64  /* longest known is SHA512 */

/* Decryption algorithm */
enum crypto_dec_algo { CRYPTO_GCM_DECRYPT = 0 };

/* Message digest algorithm */
enum crypto_md_algo {
    CRYPTO_MD_SHA256,
    CRYPTO_MD_SHA384,
    CRYPTO_MD_SHA512,
    CRYPTO_MD_SM3,
};

/* Maximum size as per the known stronger hash algorithm i.e.SHA512 */
#define CRYPTO_MD_MAX_SIZE 64U

void crypto_mod_init(void);

uint32_t crypto_mod_verify_signature(void *data_ptr, uint32_t data_len, void *sig_ptr, uint32_t sig_len,
    void *sig_alg_ptr, uint32_t sig_alg_len, void *pk_ptr, uint32_t pk_len);

uint32_t crypto_mod_verify_hash(void *data_ptr, uint32_t data_len, void *digest_info_ptr, uint32_t digest_info_len, cpt_hash_alg_e md_alg);

uint32_t crypto_mod_calc_hash(
    mbedtls_md_type_t alg, void *data_ptr, uint32_t data_len, unsigned char output[CRYPTO_MD_MAX_SIZE]);

uint32_t crypto_mod_convert_pk(void *full_pk_ptr, uint32_t full_pk_len, void **hashed_pk_ptr, uint32_t *hashed_pk_len);

uint32_t crypto_mod_auth_decrypt(enum crypto_dec_algo dec_algo, void *data_ptr, uint32_t len, const void *key,
    uint32_t key_len, uint32_t key_flags, const void *iv, uint32_t iv_len, const void *tag, uint32_t tag_len);

#endif /* CRYPTO_MOD_H */
