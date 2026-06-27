#ifndef ECIES_H
#define ECIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../crypto_hal/pke.h"
#include "../hash_hmac/hash.h"


#if 0
#define ECIES_SUPPORT_EC_POINT_COMPRESSED
#endif


/**************************************************************************************
 *                                 KDF                                                *
 **************************************************************************************/

typedef enum
{
    X963_KDF,
    // KDF1,
    // KDF2,
    // IKEV2_KDF,
    // TLS_KDF,
    // NIST_ONE_KDF_HASH,
    // NIST_ONE_KDF_HMAC,
    // NIST_TWO_STEP_KDF,
}E_KDF_TYPE;

typedef struct E_KDF E_KDF_BASE;
struct E_KDF
{
    uint8_t *input;                 //KDF input parameter, DH shared secret value
    uint32_t input_bytes;
    uint8_t *out1;                  //KDF output, actually output = out1||out2, out2 could be NULL
    uint32_t out1_bytes;
    uint8_t *out2;
    uint32_t out2_bytes;
    uint32_t (*kdf_fun_imp)(const E_KDF_BASE *self);
    E_KDF_TYPE kdf_type;
};

//out1||out2 = hash(input||counter||shared_info)||hash(input||counter++||shared_info)...
//counter is 4 bytes, with initial value 0x00000001
typedef struct
{
    E_KDF_BASE base;                //make sure this is the first member
    uint8_t *shared_info;           //optional, another input parameter of KDF
    uint32_t shared_info_bytes;
    hash_alg_e hash_alg;              //hash algorithm used in KDF
}e_kdf_ansi_x963_2001_ctx_st;

void kdf_base_init(E_KDF_BASE *base_ctx, E_KDF_TYPE kdf_type);

uint32_t ansi_x963_2001_kdf(hash_alg_e hash_alg, const uint8_t *Z, uint32_t Z_bytes,
        const uint8_t *shared_info, uint32_t shared_info_bytes, uint8_t *k1, uint32_t k1_bytes,
        uint8_t *k2, uint32_t k2_bytes);

uint32_t nist_sp800_56a_concatenation_kdf(hash_alg_e hash_alg, const uint8_t *Z, uint32_t Z_bytes,
        const uint8_t *other_info, uint32_t other_info_bytes, uint8_t *k1, uint32_t k1_bytes,
        uint8_t *k2, uint32_t k2_bytes);

void ansi_x963_2001_kdf_init(e_kdf_ansi_x963_2001_ctx_st *kdf_ctx, uint8_t *shared_info,
        uint32_t shared_info_bytes, hash_alg_e hash_alg);


/**************************************************************************************
 *                                 ENC                                                *
 **************************************************************************************/
#define ECIES_BLOCK_ENC_K_E_Y_MAX_BYTE_LEN   (64)   //the name is just for static analysis.

typedef enum{
    XOR_ENC,
    //AES_ENC
}ENC_TYPE;

typedef struct ENC E_ENC_BASE;

struct ENC
{
    uint8_t  *key;
    uint32_t key_bytes;
    uint8_t  *input;
    uint32_t input_bytes;
    uint8_t  *output;
    uint32_t output_bytes;
    uint32_t (*enc_fun_imp)(E_ENC_BASE *self);
    uint32_t (*dec_fun_imp)(E_ENC_BASE *self);
    ENC_TYPE enc_type;
};

typedef struct
{
    E_ENC_BASE base; //make sure this is the first member
}E_XOR_ENC_CTX;

void e_xor_enc_init(E_XOR_ENC_CTX *enc_ctx, uint8_t *input, uint32_t input_bytes);



/**************************************************************************************
 *                                 MAC                                                *
 **************************************************************************************/
#define ECIES_MAC_K_E_Y_MAX_BYTE_LEN             (128u)  //the name is just for static analysis.
#define ECIES_MAC_K_E_Y_ANSI_X963_MIN_BYTE_LEN   (10u)  //10 for ANSI-9.63-2001, and 14 for ANSI-9.63-2011-r2017
#define ECIES_MAC_MAX_BYTE_LEN                   (64u)

typedef struct E_MAC E_MAC_BASE;

struct E_MAC
{
    //in this structure, msg is dynamically calculated ciphertext,
    //don't have appendix. eg: out = mac(key, msg || appendix)
    uint8_t  *key;
    uint32_t key_bytes;
    uint8_t  *msg;
    uint32_t msg_bytes;
    uint8_t  *mac;
    uint32_t mac_bytes;
    //In ECIES, set mac input = ciphertext || appendix,
    //The ciphertext is calculating and
    //the value of the appendix is determined in advance
    uint8_t  *appendix;
    uint32_t appendix_bytes;
    uint32_t (*mac_imp)(const E_MAC_BASE *self);
};

typedef struct
{
    E_MAC_BASE base;        //make sure this is the first member
    hash_alg_e   hash_alg;
}e_hmac_ctx_st;

void e_hmac_init(e_hmac_ctx_st *mac_ctx, uint8_t *key_buffer, uint32_t key_bytes, uint8_t *appendix,
        uint32_t appendix_bytes, hash_alg_e hash_alg);


typedef struct
{
    E_MAC_BASE base; //make sure this is the first member
}e_cmac_ctx_st;


//fix to old project
typedef e_hmac_ctx_st E_HMAC_CTX;
typedef e_cmac_ctx_st E_CMAC_CTX;
typedef e_kdf_ansi_x963_2001_ctx_st E_KDF_ANSI_X963_2001_CTX;

/**************************************************************************************
 *                                 ECIES  STANDARD                                    *
 **************************************************************************************/
typedef enum
{
    ANSI_X963   = 0x00000001,
    IEEE_1363a  = 0x00000002,
    ISO_18033_2 = 0x00000004,
    SECG_SEC_1  = 0x00000008,
}ecies_type_e;


typedef enum
{
    ENC_MAC_ORDER   = 1,
    MAC_ENC_ORDER,
}ecies_enc_mac_key_order_e;


typedef struct ECIES_STANDARD ECIES_STD;

struct ECIES_STANDARD
{
    ecies_type_e type_flag;
    ecies_enc_mac_key_order_e enc_mac_key_order;
    uint32_t (* get_point_len_from_ciphertext)(const eccp_curve_st *curve, const uint8_t *cipher, uint32_t *point_bytes);
    uint32_t (* point_decompress)(const eccp_curve_st *curve, const uint8_t* encode, uint32_t *x, uint32_t *y);
    uint32_t (* point_compress)(const eccp_curve_st *curve, const uint8_t *x, const uint8_t *y, EC_POINT_FORM point_form, uint8_t *result, uint32_t *r_bytes);
};

//fix to old project
typedef ecies_type_e ECIES_TYPE;
typedef ecies_enc_mac_key_order_e ECIES_ENC_MAC_KEY_ORDER;

//ECIES return code
#define ECIES_SUCCESS            PKE_SUCCESS
#define ECIES_POINTER_NULL      (PKE_RT_OFFSET+0x100U)
#define ECIES_INVALID_INPUT     (PKE_RT_OFFSET+0x101U)
#define ECIES_ZERO_ALL          (PKE_RT_OFFSET+0x102U)
#define ECIES_INTEGER_TOO_BIG   (PKE_RT_OFFSET+0x103U)
#define ECIES_ERROR             (PKE_RT_OFFSET+0x104U)



void ecies_ansi_x963_ctx_init(ECIES_STD *ctx, ecies_enc_mac_key_order_e enc_mac_key_order);



uint32_t ansi_x963_2001_ecies_encrypt(const eccp_curve_st *curve, uint8_t *msg, uint32_t msg_bytes,
        uint8_t *shared_info1, uint32_t shared_info1_bytes, uint8_t *shared_info2,
        uint32_t shared_info2_bytes, const uint8_t *sender_tmp_pri_key, const uint8_t *receiver_pub_key,
        EC_POINT_FORM point_form, hash_alg_e kdf_hash_alg, hash_alg_e mac_hash_alg,
        uint32_t mac_k_bytes, uint8_t *cipher, uint32_t *cipher_bytes);

uint32_t ansi_x963_2001_ecies_decrypt(const eccp_curve_st *curve, uint8_t *cipher, uint32_t cipher_bytes,
        const uint8_t *receiver_pri_key, uint8_t *shared_info1, uint32_t shared_info1_bytes,
        uint8_t *shared_info2, uint32_t shared_info2_bytes, hash_alg_e kdf_hash_alg,
        hash_alg_e mac_hash_alg, uint32_t mac_k_bytes, uint8_t *msg, uint32_t *msg_bytes);





#ifdef __cplusplus
}
#endif

#endif
