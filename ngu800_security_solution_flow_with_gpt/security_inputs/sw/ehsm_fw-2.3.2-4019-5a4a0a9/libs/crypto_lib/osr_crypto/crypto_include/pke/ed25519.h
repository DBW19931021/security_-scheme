#ifndef Ed25519_H
#define Ed25519_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../../crypto_hal/pke.h"



//Ed25519 mode
typedef enum
{
    Ed25519_DEFAULT = 0,
    Ed25519_CTX,
    Ed25519_PH,
    Ed25519_PH_WITH_PH_M,
}ed25519_mode_e;

//fix to old project
typedef ed25519_mode_e Ed25519_MODE;


//Ed25519 return code
#define EdDSA_SUCCESS                         PKE_SUCCESS
#define EdDSA_POINTOR_NULL                    (PKE_RT_OFFSET+0x80U)
#define EdDSA_INVALID_INPUT                   (PKE_RT_OFFSET+0x81U)
#define EdDSA_VERIFY_FAIL                     (PKE_RT_OFFSET+0x82U)



//APIs


uint32_t ed25519_get_pubkey_from_prikey(const uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t ed25519_getkey(uint8_t prikey[32], uint8_t pubkey[32]);

uint32_t ed25519_sign(ed25519_mode_e mode, const uint8_t prikey[32], const uint8_t pubkey[32], 
        const uint8_t *ctx, uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, 
        uint8_t RS[64]);

uint32_t ed25519_verify(ed25519_mode_e mode, const uint8_t pubkey[32], const uint8_t *ctx, 
        uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, const uint8_t RS[64]);



#ifdef __cplusplus
}
#endif

#endif
