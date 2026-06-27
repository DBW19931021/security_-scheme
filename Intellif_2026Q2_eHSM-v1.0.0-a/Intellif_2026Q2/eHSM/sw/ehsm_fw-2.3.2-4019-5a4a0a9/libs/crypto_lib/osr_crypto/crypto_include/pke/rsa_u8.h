#ifndef RSA_U8_H
#define RSA_U8_H

#ifdef __cplusplus
extern "C" {
#endif


#include "rsa.h"




//APIs

uint32_t RSA_ModExp_U8(const uint8_t *a, const uint8_t *e, const uint8_t *n, 
        uint8_t *out, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_CRTModExp_U8(const uint8_t *a, const uint8_t *p, const uint8_t *q, 
        const uint8_t *dp, const uint8_t*dq, const uint8_t *u, uint8_t *out, 
        uint32_t nBitLen);

uint32_t RSA_GetKey_U8(uint8_t *e, uint8_t *d, uint8_t *n, uint32_t eBitLen, uint32_t nBitLen);

uint32_t RSA_GetCRTKey_U8(uint8_t *e, uint8_t *p, uint8_t *q, uint8_t *dp, uint8_t *dq, uint8_t *u,
		uint8_t *n, uint32_t eBitLen, uint32_t nBitLen);


#ifdef __cplusplus
}
#endif

#endif
