#ifndef ECCP_SEC_COMMON_H
#define ECCP_SEC_COMMON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "../../crypto_include/crypto_common/eccp_curve.h"



#if defined(SUPPORT_ANDERS_1024_1)
#define ECCP_WORD_COUNT      (32U)
#elif defined(SUPPORT_BN638)
#define ECCP_WORD_COUNT      (20U)
#elif defined(SUPPORT_SECP521R1)
#define ECCP_WORD_COUNT      (17U)
#elif defined(SUPPORT_BRAINPOOLP512R1)
#define ECCP_WORD_COUNT      (16U)
#elif defined(SUPPORT_SECP384R1)
#define ECCP_WORD_COUNT      (12U)
#elif defined(SUPPORT_BRAINPOOLP320R1)
#define ECCP_WORD_COUNT      (10U)
#else
#define ECCP_WORD_COUNT      (8U)
#endif


typedef struct {
#if (defined(PKE_LP) || defined(PKE_SECURE))
    uint32_t eccp_curve_mem[(ECCP_WORD_COUNT << 3) + 2];
#else
    uint32_t eccp_curve_mem[(ECCP_WORD_COUNT << 3) + (ECCP_WORD_COUNT << 1)];
#endif

    eccp_curve_st curve[1];
    uint8_t final;
}eccp_sec_ctx_t;




uint32_t is_eccp_curve_defined_internal(const eccp_curve_st *curve);

uint16_t calc_eccp_curve_crc16(const eccp_curve_st *curve);

uint32_t ecc_crc16_check(const eccp_curve_st *curve, uint16_t curve_crc16);

eccp_curve_st * eccp_curve_init(eccp_sec_ctx_t *ctx, const eccp_curve_st *curve);

void eccp_curve_uninit(eccp_sec_ctx_t *ctx);




#ifdef __cplusplus
}
#endif

#endif

