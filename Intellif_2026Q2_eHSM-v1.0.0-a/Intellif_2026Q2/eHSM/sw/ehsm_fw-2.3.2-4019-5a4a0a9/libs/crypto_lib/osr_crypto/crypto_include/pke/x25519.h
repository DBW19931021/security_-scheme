#ifndef X25519_H
#define X25519_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../../crypto_hal/pke.h"



//X25519 return code
#define X25519_SUCCESS                        PKE_SUCCESS
#define X25519_POINTER_NULL                   (PKE_RT_OFFSET+0x70U)
#define X25519_ZERO_ALL                       (PKE_RT_OFFSET+0x71U)
#define X25519_INVALID_INPUT                  (PKE_RT_OFFSET+0x72U)
#define X25519_INVALID_OUTPUT                 (PKE_RT_OFFSET+0x73U)



//APIs for user

uint32_t x25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t x25519_getkey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t x25519_compute_key(const uint8_t local_prikey[32], const uint8_t peer_pubkey[32], 
        uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf);



#ifdef __cplusplus
}
#endif

#endif

