#ifndef SM9_H
#define SM9_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../crypto_hal/pke.h"
#include "../../crypto_include/ske/ske.h"
#include "../../crypto_include/hash_hmac/hash.h"



#define SM9_MAX_MSG_BYTE_LEN                  (0xFFFFFF9FU)  //(0xFFFFFFFF-0x40-0x20)
#define SM9_MAX_ENC_K2_BYTE_LEN               (0x40U) //MAC key for SM9 enc/dec to get C3, here we set maximum length to be 64 bytes.


#define SM9_FP12_EXP_COMB_PARTS               (1U)   //(2U)  //(3U)   //


//SM9 return code
#define SM9_SUCCESS                           PKE_SUCCESS
#define SM9_BUFFER_NULL                       (PKE_RT_OFFSET+0x90U)
#define SM9_NOT_ON_CURVE                      (PKE_RT_OFFSET+0x91U)
#define SM9_EXCHANGE_ROLE_INVALID             (PKE_RT_OFFSET+0x92U)
#define SM9_INPUT_INVALID                     (PKE_RT_OFFSET+0x93U)
#define SM9_ZERO_ALL                          (PKE_RT_OFFSET+0x94U)
#define SM9_INTEGER_TOO_BIG                   (PKE_RT_OFFSET+0x95U)
#define SM9_VERIFY_FAILED                     (PKE_RT_OFFSET+0x96U)
#define SM9_IN_OUT_SAME_BUFFER                (PKE_RT_OFFSET+0x97U)
#define SM9_DECRY_VERIFY_FAILED               (PKE_RT_OFFSET+0x98U)



//SM9 encryption type choice
typedef enum {
    SM9_ENC_KDF_STREAM_CIPHER = 0,
    SM9_ENC_KDF_BLOCK_CIPHER
} sm9_enc_type_e;

//available when choose SM9_ENC_KDF_BLOCK_CIPHER
#if 0
typedef enum {
    SM9_ENC_NO_PADDING = 0,
    SM9_ENC_PKCS7_PADDING
} sm9_enc_padding_e;
#else
typedef ske_padding_e sm9_enc_padding_e;
#endif

//SM9 key exchange role
typedef enum {
    SM9_Role_Sponsor = 0,
    SM9_Role_Responsor
} sm9_exchange_role_e;



//internal APIs

void sm9_fp_eccp_point_u8big_2_u32little(const uint8_t *in, uint32_t *Qx, uint32_t *Qy);

void sm9_fp_eccp_point_u32little_2_u8big(const uint32_t *Qx, const uint32_t *Qy, uint8_t *out);

void sm9_fp2_eccp_point_u8big_2_u32little(const uint8_t *in, uint32_t *Qx, uint32_t *Qy);

void sm9_fp2_eccp_point_u32little_2_u8big(const uint32_t *Qx, const uint32_t *Qy, uint8_t *out);

void sm9_fp12_u8big_2_u32little(const uint8_t *in, uint32_t *out);

void sm9_fp12_u32little_2_u8big(uint32_t *a);

void sm9_fp12_in_pke_ram_u32little_2_u8big(uint8_t *g);

uint32_t sm9_h1_h2(uint8_t tag, const uint8_t *z1, uint32_t z1ByteLen, const uint8_t *z2, 
        uint32_t z2ByteLen, uint32_t *h);



//APIs for user
uint32_t sm9_pairing_calc(const uint8_t P1[64], const uint8_t P2[128], uint8_t g[32*12]);

#if 0
uint32_t sm9_fp12_exp(uint8_t g[32*12], uint8_t r[32], uint8_t out[32*12]);
#endif

uint32_t sm9_sign_gen_mastPubKey_from_mastPriKey(const uint8_t ks[32], uint8_t Ppub_s[128]);

uint32_t sm9_sign_gen_mastKeyPair(uint8_t ks[32], uint8_t Ppub_s[128]);

uint32_t sm9_sign_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_bytes, uint8_t hid, 
        const uint8_t ks[32], uint8_t dsA[64]);

uint32_t sm9_sign(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *fp12g, 
        const uint8_t Ppub_s[128], const uint8_t dsA[64], const uint8_t r[32], 
        uint8_t h[32], uint8_t S[65]);

uint32_t sm9_verify(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *IDA, uint32_t IDA_bytes, 
        uint8_t hid, const uint8_t *fp12g, const uint8_t Ppub_s[128], const uint8_t h[32], 
        const uint8_t S[65]);


uint32_t sm9_enc_gen_mastPubKey_from_mastPriKey(const uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_enc_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_enc_gen_userPriKey(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, 
        const uint8_t ke[32], uint8_t deB[128]);

uint32_t sm9_wrap_key(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *fp12g, 
        const uint8_t Ppub_e[64], const uint8_t r[32], uint8_t C[64], uint32_t k_bytes, uint8_t *k);

uint32_t sm9_unwrap_key(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t deB[128], 
        const uint8_t C[64], uint32_t k_bytes, uint8_t *k);

uint32_t sm9_enc(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *M, 
        uint32_t M_bytes, const uint8_t *fp12g, const uint8_t Ppub_e[64],  
        const uint8_t r[32], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *C, uint32_t *C_bytes);

uint32_t sm9_dec(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t *C, uint32_t C_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *M, uint32_t *M_bytes);


uint32_t sm9_exckey_gen_mastPubKey_from_mastPriKey(const uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_exckey_gen_mastKeyPair(uint8_t ke[32], uint8_t Ppub_e[64]);

uint32_t sm9_exckey_gen_userPriKey(const uint8_t *IDA, uint32_t IDA_bytes, uint8_t hid, 
        const uint8_t ke[32], uint8_t deA[128]);

uint32_t sm9_exckey_gen_tmpPubKey_from_tmpPriKey(const uint8_t *IDB, uint32_t IDB_bytes, 
        uint8_t hid, const uint8_t Ppub_e[64], const uint8_t rA[32], uint8_t RA[64]);

uint32_t sm9_exckey_gen_tmpKeyPair(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, 
        const uint8_t Ppub_e[64], uint8_t rA[32], uint8_t RA[64]);

uint32_t sm9_exchangekey(sm9_exchange_role_e role,
                        const uint8_t *IDA, uint32_t IDA_bytes,
                        const uint8_t *IDB, uint32_t IDB_bytes,
                        const uint8_t *fp12g,
                        const uint8_t Ppub_e[64],
                        const uint8_t deA[128], const uint8_t rA[32],
                        const uint8_t RA[64], const uint8_t RB[64],
                        uint32_t k_bytes,
                        uint8_t *k, uint8_t S1[32], uint8_t SA[32]);



#ifdef SM9_SEC

//SM9 return code(secure version)
#define SM9_SUCCESS_S                         (0x5E2F7B1AU)
#define SM9_ERROR_S                           (0x2BAD785EU)


uint32_t sm9_sign_s(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *fp12g, 
        const uint8_t Ppub_s[128], const uint8_t dsA[64], const uint8_t r[32], uint8_t h[32], 
        uint8_t S[65]);

uint32_t sm9_verify_s(const uint8_t *msg, uint32_t msg_bytes, const uint8_t *IDA, uint32_t IDA_bytes, 
        uint8_t hid, const uint8_t *fp12g, const uint8_t Ppub_s[128], const uint8_t h[32], 
        const uint8_t S[65]);

uint32_t sm9_wrap_key_s(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *fp12g, 
        const uint8_t Ppub_e[64], const uint8_t r[32], uint8_t C[64], uint32_t k_bytes, uint8_t *k);

uint32_t sm9_unwrap_key_s(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t deB[128], 
        const uint8_t C[64], uint32_t k_bytes, uint8_t *k);

uint32_t sm9_enc_s(const uint8_t *IDB, uint32_t IDB_bytes, uint8_t hid, const uint8_t *M, 
        uint32_t M_bytes, const uint8_t *fp12g, const uint8_t Ppub_e[64], const uint8_t r[32], 
        sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, uint32_t K2_bytes, 
        uint8_t *C, uint32_t *C_bytes);

uint32_t sm9_dec_s(const uint8_t *IDB, uint32_t IDB_bytes, const uint8_t *C, uint32_t C_bytes, 
        const uint8_t deB[128], sm9_enc_type_e enc_type, sm9_enc_padding_e padding_type, 
        uint32_t K2_bytes, uint8_t *M, uint32_t *M_bytes);

uint32_t sm9_exchangekey_s(sm9_exchange_role_e role,
                        const uint8_t *IDA, uint32_t IDA_bytes,
                        const uint8_t *IDB, uint32_t IDB_bytes,
                        const uint8_t *fp12g,
                        const uint8_t Ppub_e[64],
                        const uint8_t deA[128], const uint8_t rA[32],
                        const uint8_t RA[64], const uint8_t RB[64],
                        uint32_t k_bytes,
                        uint8_t *k, uint8_t S1[32], uint8_t SA[32]);

#endif




#ifdef __cplusplus
}
#endif

#endif

