#ifndef RSA_H
#define RSA_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../../crypto_hal/pke.h"






#if defined(SUPPORT_RSASSA_PSS)
#include "../hash_hmac/hash.h"
#endif



//RSA return code
#define RSA_SUCCESS                           PKE_SUCCESS
#define RSA_BUFFER_NULL                       (PKE_RT_OFFSET+0x30U)
#define RSA_INPUT_TOO_LONG                    (PKE_RT_OFFSET+0x31U)
#define RSA_INPUT_INVALID                     (PKE_RT_OFFSET+0x32U)
#define RSA_ERROR                             (PKE_RT_OFFSET+0x33U)



//APIs

uint32_t RSA_ModExp(const uint32_t *a, const uint32_t *e, const uint32_t *n, uint32_t *out, 
        uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_CRTModExp(const uint32_t *a, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t*dq, const uint32_t *u, uint32_t *out, 
        uint32_t nBitLen);

uint32_t RSA_GetKey(uint32_t *e, uint32_t *d, uint32_t *n, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_GetCRTKey(uint32_t *e, uint32_t *p, uint32_t *q, uint32_t *dp, uint32_t *dq, uint32_t *u,
        uint32_t *n, uint32_t eBitLen, uint32_t nBitLen);



#ifdef RSA_SEC

//RSA return code(secure version)
#define RSA_SUCCESS_S                         (0x3AEBA318U)
#define RSA_ERROR_S                           (0x45DF3DAEU)


uint32_t RSA_ModExp_with_pub(const uint32_t *a, const uint32_t *e, const uint32_t *d, 
        const uint32_t *n, uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_CRTModExp_with_pub(const uint32_t *a, const uint32_t *p, const uint32_t *q, 
        const uint32_t *dp, const uint32_t*dq, const uint32_t *u, const uint32_t *e,
        uint32_t *out, uint32_t eBitLen, uint32_t nBitLen);

#endif




typedef struct {
    uint8_t *p;
    uint8_t *q;
    uint8_t *dp;
    uint8_t *dq;
    uint8_t *u;//qinv
} rsa_crt_private_key_st;

//fix to old project
typedef rsa_crt_private_key_st RSA_CRT_PRIVATE_KEY;

#if (defined(SUPPORT_RSASSA_PSS) || defined(SUPPORT_RSAES_OAEP))
void rsa_pkcs1_mgf1_counter_add(uint8_t *counter, uint32_t bytes, uint8_t b);

uint32_t rsa_pkcs1_mgf1_with_xor_in(hash_alg_e hash_alg, const uint8_t *seed, uint32_t seed_bytes, 
        const uint8_t *in, uint8_t *out, uint32_t mask_bytes);
#endif


#ifdef SUPPORT_RSASSA_PSS
uint32_t rsa_ssa_pss_sign_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, const uint8_t *d, const uint8_t *n, uint32_t n_bits, 
        uint8_t *signature);

uint32_t rsa_ssa_pss_sign(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *d, const uint8_t *n, 
        uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pss_crt_sign_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg_digest, const rsa_crt_private_key_st *d, uint32_t n_bits, 
        uint8_t *signature);

uint32_t rsa_ssa_pss_crt_sign(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *salt, 
        uint32_t salt_bytes, const uint8_t *msg, uint32_t msg_bytes, const rsa_crt_private_key_st *d, 
        uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pss_verify_by_msg_digest(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, int32_t salt_bytes,
        const uint8_t *msg_digest, const uint8_t *e, uint32_t e_bits, const uint8_t *n, uint32_t n_bits, 
        const uint8_t *signature);

uint32_t rsa_ssa_pss_verify(hash_alg_e msg_hash_alg, hash_alg_e mgf_hash_alg, int32_t salt_bytes, 
        const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, uint32_t e_bits, const uint8_t *n, 
        uint32_t n_bits, const uint8_t *signature);
#endif


#ifdef SUPPORT_RSASSA_PKCS1_V1_5
uint32_t emsa_pkcs_v1_5_encode_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
    uint8_t *em, uint32_t em_bytes);

uint32_t emsa_pkcs_v1_5_encode(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes);

uint32_t rsa_ssa_pkcs1_v1_5_sign_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
        const uint8_t *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pkcs1_v1_5_crt_sign_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest,
        const rsa_crt_private_key_st *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pkcs1_v1_5_sign(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const uint8_t *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pkcs1_v1_5_crt_sign(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const rsa_crt_private_key_st *d, const uint8_t *n, uint32_t n_bits, uint8_t *signature);

uint32_t rsa_ssa_pkcs1_v1_5_verify_by_msg_digest(hash_alg_e alg, const uint8_t *msg_digest, const uint8_t *e, 
    uint32_t e_bits, const uint8_t *n, uint32_t n_bits, const uint8_t *signature);

uint32_t rsa_ssa_pkcs1_v1_5_verify(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, 
        const uint8_t *e, uint32_t e_bits, const uint8_t *n, uint32_t n_bits, const uint8_t *signature);
#endif



#ifdef SUPPORT_RSAES_OAEP
uint32_t eme_oaep_encode_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes);

uint32_t eme_oaep_encode(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, uint8_t *em, uint32_t em_bytes);

uint32_t eme_oaep_decode_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes);

uint32_t eme_oaep_decode(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes);

uint32_t rsa_es_oaep_enc_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, 
        uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher);

uint32_t rsa_es_oaep_enc(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, const uint8_t *seed, const uint8_t *msg, uint32_t msg_bytes, const uint8_t *e, 
        uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher);

uint32_t rsa_es_oaep_dec_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher);

uint32_t rsa_es_oaep_dec(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher);

uint32_t rsa_es_oaep_crt_dec_by_label_digest(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label_digest, 
        uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher);

uint32_t rsa_es_oaep_crt_dec(hash_alg_e label_hash_alg, hash_alg_e mgf_hash_alg, const uint8_t *label, 
        uint32_t label_bytes, uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher);
#endif



#ifdef SUPPORT_RSAES_PKCS1_V1_5
uint32_t eme_pkcs1_v1_5_encode(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *ps, uint8_t *em, uint32_t em_bytes);

uint32_t eme_pkcs1_v1_5_decode(uint8_t *msg, uint32_t *msg_bytes, const uint8_t *em, uint32_t em_bytes);

uint32_t rsa_es_pkcs1_v1_5_enc(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *ps, const uint8_t *e, 
    uint32_t e_bits, const uint8_t *n, uint32_t n_bits, uint8_t *cipher);

uint32_t rsa_es_pkcs1_v1_5_dec(uint8_t *msg, uint32_t *msg_bytes, const uint8_t *d, 
        const uint8_t *n, uint32_t n_bits, const uint8_t *cipher);

uint32_t rsa_es_pkcs1_v1_5_crt_dec(uint8_t *msg, uint32_t *msg_bytes, const rsa_crt_private_key_st *d,
        uint32_t n_bits, const uint8_t *cipher);
#endif



#ifdef __cplusplus
}
#endif

#endif

