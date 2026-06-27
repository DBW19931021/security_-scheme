#ifndef HASH_PBKDF2_KMU_H
#define HASH_PBKDF2_KMU_H



#include "hmac.h"
#include "../../crypto_include/crypto_common/utility.h"



#ifdef __cplusplus
extern "C" {
#endif



//APIs
uint32_t pbkdf2_hmac_kmu(hash_alg_e hash_alg, uint16_t sp_key_idx, uint32_t pwd_bytes, const uint8_t *salt, 
    uint32_t salt_bytes, uint32_t iter, uint16_t dst_key_idx);



#ifdef __cplusplus
}
#endif


#endif

