#ifndef HASH_KDF_H
#define HASH_KDF_H



#include "hmac.h"



#ifdef __cplusplus
extern "C" {
#endif



//APIs

uint32_t pbkdf2_hmac(hash_alg_e alg, const uint8_t *pwd, uint16_t sp_key_idx, uint32_t pwd_bytes, 
        const uint8_t *salt, uint32_t salt_bytes, uint32_t iter, uint8_t *out, uint32_t out_bytes);


#ifdef SUPPORT_ANSI_X9_63_KDF
#ifdef SUPPORT_HASH_NODE
uint32_t ansi_x9_63_kdf_node_with_xor_in(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, const uint8_t *in, uint8_t *out, uint32_t out_bytes, uint32_t check_whether_zero);

uint32_t ansi_x9_63_kdf_internal(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, 
        uint8_t *counter, uint8_t *key, uint32_t key_bytes);

uint32_t ansi_x9_63_kdf_node(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *counter, 
        uint8_t *k1, uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes);

uint32_t ansi_x9_63_kdf(hash_alg_e alg, const uint8_t *Z, uint32_t Z_bytes, const uint8_t *shared_info, 
        uint32_t shared_info_bytes, uint8_t *k1, uint32_t k1_bytes, uint8_t *k2, uint32_t k2_bytes);
#endif
#endif


#ifdef __cplusplus
}
#endif


#endif

