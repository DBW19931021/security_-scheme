#ifndef DH_H
#define DH_H

#ifdef __cplusplus
extern "C" {
#endif



#include "../../crypto_hal/pke.h"


//DH return code
#define DH_SUCCESS                          PKE_SUCCESS
#define DH_POINTER_NULL                     (PKE_RT_OFFSET+0xA0U)
#define DH_INVALID_INPUT                    (PKE_RT_OFFSET+0xA1U)
#define DH_ZERO_ALL                         (PKE_RT_OFFSET+0xA2U)
#define DH_VALUE_ONE                        (PKE_RT_OFFSET+0xA3U)
#define DH_INTEGER_TOO_BIG                  (PKE_RT_OFFSET+0xA4U)


typedef struct
{
    uint32_t p_bits;
    uint32_t q_bits;
    uint32_t g_bits;
    uint32_t *p;
    uint32_t *p_h;
    uint32_t *q;
    uint32_t *g;
}dh_para_st;


//fix to old project
typedef dh_para_st DH_PARA;



//APIs

uint32_t dh_param_pointer_init(dh_para_st *dh_para, uint32_t *p_buf, uint32_t p_bits, uint32_t *p_h_buf, 
        uint32_t *q_buf, uint32_t q_bits, uint32_t *g_buf, uint32_t g_bits);

uint32_t dh_param_value_init(const dh_para_st *dh_para, const uint8_t *p, const uint8_t *p_h, 
        const uint8_t *q, const uint8_t *g);

uint32_t dh_check_public_key(const dh_para_st *dh_para, const uint32_t *p_minus_1, 
        const uint32_t *pubkey);

uint32_t dh_generate_pubkey_from_prikey(const dh_para_st *dh_para, const uint8_t *prikey, 
        uint8_t *pubkey);

uint32_t dh_generate_key(const dh_para_st *dh_para, uint8_t *prikey, uint8_t *pubkey);

uint32_t dh_compute_key(const dh_para_st *dh_para, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key);




#ifdef DH_SEC

//DH return code(secure version)
#define DH_SUCCESS_S                        (0x9C9BC1E3U)
#define DH_ERROR_S                          (0xFDC28CB1U)


uint32_t dh_compute_key_s(const dh_para_st *dh_para, const uint8_t *local_prikey, 
        const uint8_t *peer_pubkey, uint8_t *key);

#endif




#ifdef __cplusplus
}
#endif

#endif

