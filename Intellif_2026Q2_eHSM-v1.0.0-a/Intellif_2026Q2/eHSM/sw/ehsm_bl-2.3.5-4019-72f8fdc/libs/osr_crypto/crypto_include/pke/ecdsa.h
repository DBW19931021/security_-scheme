#ifndef ECDSA_H
#define ECDSA_H

#ifdef __cplusplus
extern "C" {
#endif



#include "../../crypto_hal/pke.h"



//ECDSA return code
#define ECDSA_SUCCESS                         PKE_SUCCESS
#define ECDSA_POINTOR_NULL                    (PKE_RT_OFFSET+0x50U)
#define ECDSA_INVALID_INPUT                   (PKE_RT_OFFSET+0x51U)
#define ECDSA_ZERO_ALL                        (PKE_RT_OFFSET+0x52U)
#define ECDSA_INTEGER_TOO_BIG                 (PKE_RT_OFFSET+0x53U)
#define ECDSA_VERIFY_FAILED                   (PKE_RT_OFFSET+0x54U)




//APIs
uint32_t ecdsa_get_e_bn(uint32_t n_bits, uint32_t n_bytes, uint32_t n_words, 
        const uint8_t *e, uint32_t e_bytes, uint32_t *e_bn);
        
uint32_t ecdsa_sign(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature);

uint32_t ecdsa_verify(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *pubKey, const uint8_t *signature);





#ifdef ECDSA_SEC

//ECDSA return code(secure version)
#define ECDSA_SUCCESS_S                       (0x7D5FEB14U)
#define ECDSA_ERROR_S                         (0xB4C0BC5AU)


uint32_t ecdsa_sign_s(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature);
    
uint32_t ecdsa_verify_s(const eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen, 
        const uint8_t *pubKey, const uint8_t *signature);

#endif




#ifdef __cplusplus
}
#endif

#endif
