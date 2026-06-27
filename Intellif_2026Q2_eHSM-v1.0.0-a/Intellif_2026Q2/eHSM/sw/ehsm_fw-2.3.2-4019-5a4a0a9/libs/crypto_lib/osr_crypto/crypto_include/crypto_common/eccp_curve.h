#ifndef ECCP_CURVE_H
#define ECCP_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../crypto_hal/pke_common.h"

// sample ecc GF(p) curve
#define SUPPORT_SECP256R1

#ifndef BUILD_EHSM_BL
#define SUPPORT_BRAINPOOLP160R1
#define SUPPORT_BRAINPOOLP192R1
#define SUPPORT_BRAINPOOLP224R1
#define SUPPORT_BRAINPOOLP256R1
#if (ECCP_MAX_BIT_LEN >= 320)
#define SUPPORT_BRAINPOOLP320R1
#endif
#if (ECCP_MAX_BIT_LEN >= 384)
#define SUPPORT_BRAINPOOLP384R1
#endif
#if (ECCP_MAX_BIT_LEN >= 512)
#define SUPPORT_BRAINPOOLP512R1
#endif
#define SUPPORT_SECP160R1
#define SUPPORT_SECP160R2
#define SUPPORT_SECP192R1
#define SUPPORT_SECP224R1
#if (ECCP_MAX_BIT_LEN >= 384)
#define SUPPORT_SECP384R1
#endif
#if (ECCP_MAX_BIT_LEN >= 521)
#define SUPPORT_SECP521R1
#endif
#define SUPPORT_SECP160K1
#define SUPPORT_SECP192K1
#define SUPPORT_SECP224K1
#define SUPPORT_SECP256K1
#define SUPPORT_BN256
#if (ECCP_MAX_BIT_LEN >= 638)
#define SUPPORT_BN638
#endif
#if (ECCP_MAX_BIT_LEN >= 1024)
#define SUPPORT_ANDERS_1024_1
#endif
#endif // !def BUILD_EHSM_BL

// eccp curve struct
typedef struct
{
    uint32_t eccp_p_bitLen;        //bit length of prime p
    uint32_t eccp_n_bitLen;        //bit length of order n
    const uint32_t *eccp_p;        //prime p
    const uint32_t *eccp_p_h;
#if (defined(PKE_LP) || defined(PKE_SECURE) || defined(PKE_UHP_ECC))
    const uint32_t *eccp_p_n0;
#endif
    const uint32_t *eccp_a;
    const uint32_t *eccp_b;
    const uint32_t *eccp_Gx;
    const uint32_t *eccp_Gy;
    const uint32_t *eccp_n;        //order of curve or point(Gx,Gy)
    const uint32_t *eccp_n_h;
#if (defined(PKE_LP) || defined(PKE_SECURE) || defined(PKE_UHP_ECC))
    const uint32_t *eccp_n_n0;
#endif
#if (defined(PKE_HP) || defined(PKE_UHP) || defined(PKE_UHP_ECC))
    const uint32_t *eccp_half_Gx;
    const uint32_t *eccp_half_Gy;
#endif
} eccp_curve_st;


//fix to old project
typedef eccp_curve_st eccp_curve_t;


#ifdef SUPPORT_BRAINPOOLP160R1
extern const eccp_curve_st brainpoolp160r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP192R1
extern const eccp_curve_st brainpoolp192r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP224R1
extern const eccp_curve_st brainpoolp224r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP256R1
extern const eccp_curve_st brainpoolp256r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP320R1
extern const eccp_curve_st brainpoolp320r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP384R1
extern const eccp_curve_st brainpoolp384r1[1];
#endif

#ifdef SUPPORT_BRAINPOOLP512R1
extern const eccp_curve_st brainpoolp512r1[1];
#endif

#ifdef SUPPORT_SECP160R1
extern const eccp_curve_st secp160r1[1];
#endif

#ifdef SUPPORT_SECP160R2
extern const eccp_curve_st secp160r2[1];
#endif

#ifdef SUPPORT_SECP192R1
extern const eccp_curve_st secp192r1[1];
#endif

#ifdef SUPPORT_SECP224R1
extern const eccp_curve_st secp224r1[1];
#endif

#ifdef SUPPORT_SECP256R1
extern const eccp_curve_st secp256r1[1];
#endif

#ifdef SUPPORT_SECP384R1
extern const eccp_curve_st secp384r1[1];
#endif

#ifdef SUPPORT_SECP521R1
extern const eccp_curve_st secp521r1[1];
#endif

#ifdef SUPPORT_SECP160K1
extern const eccp_curve_st secp160k1[1];
#endif

#ifdef SUPPORT_SECP192K1
extern const eccp_curve_st secp192k1[1];
#endif

#ifdef SUPPORT_SECP224K1
extern const eccp_curve_st secp224k1[1];
#endif

#ifdef SUPPORT_SECP256K1
extern const eccp_curve_st secp256k1[1];
#endif

#ifdef SUPPORT_BN256
extern const eccp_curve_st bn256[1];
#endif

#ifdef SUPPORT_BN638
extern const eccp_curve_st bn638[1];
#endif

#ifdef SUPPORT_ANDERS_1024_1
extern const eccp_curve_st anders_1024_1[1];
#endif

#ifdef SUPPORT_SM2
extern const eccp_curve_st sm2_curve[1];
extern const uint32_t g_sm2p256v1_n_minus_1[8];
#endif

#ifdef SUPPORT_SM9
extern const eccp_curve_st sm9_curve[1];
#endif


/********* Curve25519 struct *********/
typedef struct
{
    uint32_t p_bitLen;        //bit length of prime p
    uint32_t n_bitLen;        //bit length of order n
    const uint32_t *p;
    const uint32_t *p_h;
#if (defined(PKE_LP) || defined(PKE_SECURE))
    const uint32_t *p_n0;
#endif
    const uint32_t *a24;            //(A-2)/4
    const uint32_t *u;
    const uint32_t *v;
    const uint32_t *n;              //order of curve or point(Gx,Gy)
    const uint32_t *n_h;
#if (defined(PKE_LP) || defined(PKE_SECURE))
    const uint32_t *n_n0;
#endif
    const uint32_t *cofactor;
} mont_curve_st;

//fix to old project
typedef mont_curve_st mont_curve_t;


/********* Edward Curve 25519 struct *********/
typedef struct
{
    uint32_t p_bitLen;        //bit length of prime p
    uint32_t n_bitLen;        //bit length of order n
    const uint32_t *p;
    const uint32_t *p_h;
#if (defined(PKE_LP) || defined(PKE_SECURE))
    const uint32_t *p_n0;
#endif
    const uint32_t *d;
    const uint32_t *Gx;
    const uint32_t *Gy;
    const uint32_t *n;              //order of curve or point(Gx,Gy)
    const uint32_t *n_h;
#if (defined(PKE_LP) || defined(PKE_SECURE))
    const uint32_t *n_n0;
#endif
    const uint32_t *cofactor;
} edward_curve_st;

//fix to old project
typedef edward_curve_st edward_curve_t;

#ifdef SUPPORT_C25519
extern const mont_curve_st c25519[1];
extern const edward_curve_st ed25519[1];
#endif



#ifdef __cplusplus
}
#endif

#endif
