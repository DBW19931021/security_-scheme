#ifndef ECDH_H
#define ECDH_H

#ifdef __cplusplus
extern "C" {
#endif



#include "../../crypto_hal/pke.h"


//ECDH return code
#define ECDH_SUCCESS                          PKE_SUCCESS
#define ECDH_POINTOR_NULL                     (PKE_RT_OFFSET+0x60U)
#define ECDH_INVALID_INPUT                    (PKE_RT_OFFSET+0x61U)
#define ECDH_ZERO_ALL                         (PKE_RT_OFFSET+0x62U)
#define ECDH_INTEGER_TOO_BIG                  (PKE_RT_OFFSET+0x63U)



//APIs

uint32_t ecdh_compute_key(const eccp_curve_st *curve, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf);




#ifdef ECDH_SEC

//ECDH return code(secure version)
#define ECDH_SUCCESS_S                        (0x8B9BC1E1U)
#define ECDH_ERROR_S                          (0xCBC192A3U)


uint32_t ecdh_compute_key_s(const eccp_curve_st *curve, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key, uint32_t keyByteLen, KDF_FUNC kdf);

#endif




#ifdef __cplusplus
}
#endif

#endif
