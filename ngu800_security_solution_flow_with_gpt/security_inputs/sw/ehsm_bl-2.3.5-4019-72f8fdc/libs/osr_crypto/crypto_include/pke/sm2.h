#ifndef SM2_H
#define SM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../crypto_hal/pke.h"
#include "../hash_hmac/hash.h"


#if (defined(PKE_HP) || defined(PKE_UHP))
#define SM2_HIGH_SPEED        //only available for PKE_HP, PKE_UHP
#endif


//some sm2 length
#define SM3_DIGEST_BYTE_LEN                   SM2_BYTE_LEN
#define SM2_MAX_ID_BYTE_LEN                   (0x1FFFU)   //this is from ((1U<<13)-1U)




//SM2 return code
#define SM2_SUCCESS                           PKE_SUCCESS
#define SM2_BUFFER_NULL                       (PKE_RT_OFFSET+0x40U)
#define SM2_NOT_ON_CURVE                      (PKE_RT_OFFSET+0x41U)
#define SM2_EXCHANGE_ROLE_INVALID             (PKE_RT_OFFSET+0x42U)
#define SM2_INPUT_INVALID                     (PKE_RT_OFFSET+0x43U)
#define SM2_ZERO_ALL                          (PKE_RT_OFFSET+0x44U)
#define SM2_INTEGER_TOO_BIG                   (PKE_RT_OFFSET+0x45U)
#define SM2_VERIFY_FAILED                     (PKE_RT_OFFSET+0x46U)
#define SM2_DECRYPT_VERIFY_FAILED             (PKE_RT_OFFSET+0x47U)



//SM2 key exchange role
typedef enum {
    SM2_Role_Sponsor = 0,
    SM2_Role_Responsor
} sm2_exchange_role_e;


// SM2 ciphertext order
typedef enum {
    SM2_C1C3C2   = 0,
    SM2_C1C2C3,
} sm2_cipher_order_e;



//APIs

uint32_t sm2_getZ(const uint8_t *ID, uint32_t byteLenofID, const uint8_t pubKey[65], uint8_t Z[32]);

uint32_t sm2_getE(const uint8_t *M, uint32_t byteLen, const uint8_t Z[32], uint8_t E[32]);

#if 1
#define SM2_GETE_BY_STEPS
#endif
#ifdef SM2_GETE_BY_STEPS
uint32_t sm2_getE_init(hash_ctx_st *ctx, const uint8_t Z[32]);

uint32_t sm2_getE_update(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes);

uint32_t sm2_getE_final(hash_ctx_st *ctx, uint8_t E[32]);
#endif

uint32_t sm2_get_pubkey_from_prikey(const uint8_t priKey[32], uint8_t pubKey[65]);

uint32_t sm2_getkey(uint8_t priKey[32], uint8_t pubKey[65]);

uint32_t sm2_sign(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], 
        uint8_t signature[64]);

uint32_t sm2_verify(const uint8_t E[32], const uint8_t pubKey[65], const uint8_t signature[64]);

uint32_t sm2_encrypt(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], 
        const uint8_t pubKey[65],sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen);

uint32_t sm2_decrypt(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32],
        sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen);

uint32_t sm2_exchangekey(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA);



#ifdef SM2_SEC

//SM2 return code(secure version)
#define SM2_SUCCESS_S                         (0x3E2FDB1AU)
#define SM2_ERROR_S                           (0xCBAD735EU)


uint32_t sm2_get_pubkey_from_prikey_s(const uint8_t priKey[32], uint8_t pubKey[65]);
                        
uint32_t sm2_getkey_s(uint8_t priKey[32], uint8_t pubKey[65]);
                        
uint32_t sm2_sign_s(const uint8_t E[32], const uint8_t rand_k[32], const uint8_t priKey[32], 
        uint8_t signature[64]);

uint32_t sm2_verify_s(const uint8_t E[32], const uint8_t pubKey[65], const uint8_t signature[64]);

uint32_t sm2_encrypt_s(const uint8_t *M, uint32_t MByteLen, const uint8_t rand_k[32], 
        const uint8_t pubKey[65], sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen);

uint32_t sm2_decrypt_s(const uint8_t *C, uint32_t CByteLen, const uint8_t priKey[32],
        sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen);

uint32_t sm2_exchangekey_s(sm2_exchange_role_e role,
                        const uint8_t *dA, const uint8_t *PB,
                        const uint8_t *rA, const uint8_t *RA,
                        const uint8_t *RB,
                        const uint8_t *ZA, const uint8_t *ZB,
                        uint32_t kByteLen,
                        uint8_t *KA, uint8_t *S1, uint8_t *SA);

#endif




#ifdef __cplusplus
}
#endif

#endif

