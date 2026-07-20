#ifndef SM2_SEC_KMU_H
#define SM2_SEC_KMU_H



#ifdef __cplusplus
extern "C" {
#endif



//APIs
uint32_t sm2_sign_s_kmu(const uint8_t E[32], uint8_t sp_key_idx, uint8_t signature[64]);



#ifdef __cplusplus
}
#endif


#endif

