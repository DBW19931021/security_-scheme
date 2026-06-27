#ifndef PKE_H
#define PKE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "pke_common.h"
#include "../crypto_include/crypto_common/eccp_curve.h"



/***************** PKE register *******************/
#ifndef SUPPORT_STATIC_ANALYSIS
#define rPKE_CTRL           (*((volatile uint32_t *)(PKE_BASE_ADDR)))
#define rPKE_CFG            (*((volatile uint32_t *)(PKE_BASE_ADDR+0x04U)))
#define rPKE_MC_PTR         (*((volatile uint32_t *)(PKE_BASE_ADDR+0x08U)))
#define rPKE_RISR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x0CU)))
#define rPKE_IMCR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x10U)))
#define rPKE_MISR           (*((volatile uint32_t *)(PKE_BASE_ADDR+0x14U)))
#define rPKE_RT_CODE        (*((volatile uint32_t *)(PKE_BASE_ADDR+0x24U)))
#define rPKE_RAM_STATUS     (*((volatile uint32_t *)(PKE_BASE_ADDR+0x28U)))
#ifdef PKE_SEC
#define rPKE_RAND_SEED      (*((volatile uint32_t *)(PKE_BASE_ADDR+0x40U)))
#define rPKE_RC_EN          (*((volatile uint32_t *)(PKE_BASE_ADDR+0x60U)))
#define rPKE_RC_K_E_Y       (*((volatile uint32_t *)(PKE_BASE_ADDR+0x64U)))  //the name is just for static analysis.
#define rPKE_RC_D_NONCE     (*((volatile uint32_t *)(PKE_BASE_ADDR+0x68U)))
#define rPKE_RC_A_NONCE     (*((volatile uint32_t *)(PKE_BASE_ADDR+0x6CU)))
#endif
#define rPKE_VERSION        (*((volatile uint32_t *)(PKE_BASE_ADDR+0xFCU)))
#if 0
#define rPKE_A(a, step)     ((volatile uint32_t *)(PKE_BASE_ADDR+0x0400U+((a)*(step))))
#define rPKE_B(a, step)     ((volatile uint32_t *)(PKE_BASE_ADDR+0x1000U+((a)*(step))))
#endif
#else

typedef struct {
    uint32_t PKE_CTRL[1];           //0x00
    uint32_t PKE_CFG[1];            //0x04
    uint32_t PKE_MC_PTR[1];         //0x08
    uint32_t PKE_RISR[1];           //0x0C
    uint32_t PKE_IMCR[1];           //0x10
    uint32_t PKE_MISR[1];           //0x14
    uint32_t REV_1[3];
    uint32_t PKE_RT_CODE[1];        //0x24
    uint32_t PKE_RAM_STATUS[1];     //0x28
    uint32_t REV_2[5];
#ifdef PKE_SEC
    uint32_t PKE_RAND_SEED[1];      //0x40
    uint32_t REV_3[7];
    uint32_t PKE_RC_EN[1];          //0x60
    uint32_t PKE_RC_K_E_Y[1];       //0x64
    uint32_t PKE_RC_D_NONCE[1];     //0x68
    uint32_t PKE_RC_A_NONCE[1];     //0x6C
#endif
    uint32_t REV_4[35];
    uint32_t PKE_VERSION[1];        //0xFC
} pke_reg_st;

#ifdef CONFIG_UNIT_TEST
extern volatile pke_reg_st * g_pke_reg;
#else
extern volatile pke_reg_st * const g_pke_reg;
#endif

#define rPKE_CTRL             (*((volatile uint32_t *)(g_pke_reg->PKE_CTRL)))
#define rPKE_CFG              (*((volatile uint32_t *)(g_pke_reg->PKE_CFG)))
#define rPKE_MC_PTR           (*((volatile uint32_t *)(g_pke_reg->PKE_MC_PTR)))
#define rPKE_RISR             (*((volatile uint32_t *)(g_pke_reg->PKE_RISR)))
#define rPKE_IMCR             (*((volatile uint32_t *)(g_pke_reg->PKE_IMCR)))
#define rPKE_MISR             (*((volatile uint32_t *)(g_pke_reg->PKE_MISR)))
#define rPKE_RT_CODE          (*((volatile uint32_t *)(g_pke_reg->PKE_RT_CODE)))
#define rPKE_RAM_STATUS       (*((volatile uint32_t *)(g_pke_reg->PKE_RAM_STATUS)))
#define rPKE_RAND_SEED        (*((volatile uint32_t *)(g_pke_reg->PKE_RAND_SEED)))
#define rPKE_RC_EN            (*((volatile uint32_t *)(g_pke_reg->PKE_RC_EN)))
#define rPKE_RC_K_E_Y         (*((volatile uint32_t *)(g_pke_reg->PKE_RC_K_E_Y)))
#define rPKE_RC_D_NONCE       (*((volatile uint32_t *)(g_pke_reg->PKE_RC_D_NONCE)))
#define rPKE_RC_A_NONCE       (*((volatile uint32_t *)(g_pke_reg->PKE_RC_A_NONCE)))
#define rPKE_VERSION          (*((volatile uint32_t *)(g_pke_reg->PKE_VERSION)))
#endif

/*********** PKE register action offset ************/
#define PKE_START_CALC                        (1U)


/***************** PKE microcode ******************/
#define MICROCODE_PDBL                        (0x04U)
#define MICROCODE_PADD                        (0x08U)
#define MICROCODE_PVER                        (0x0CU)
#define MICROCODE_PMUL                        (0x10U)
#define MICROCODE_MODEXP                      (0x14U)
#define MICROCODE_MODMUL                      (0x18U)
#define MICROCODE_MODINV                      (0x1CU)
#define MICROCODE_MODADD                      (0x20U)
#define MICROCODE_MODSUB                      (0x24U)
#define MICROCODE_MGMR_PRE_H                  (0x28U)
#define MICROCODE_INTMUL                      (0x2CU)
#define MICROCODE_Ed25519_PMUL                (0x30U)
#define MICROCODE_Ed25519_PADD                (0x34U)
#define MICROCODE_C25519_PMUL                 (0x38U)
#define MICROCODE_MODRES                      (0x3CU)
#define MICROCODE_INTADD                      (0x40U)
#define MICROCODE_INTSUB                      (0x44U)
#define MICROCODE_PMULF                       (0x48U)
#define MICROCODE_MGMR_PRE_H_MM               (0x4CU)
#define MICROCODE_MGMR_PRE_N0                 (0x50U)
#ifdef PKE_SEC
#define MICROCODE_PMUL_SEC                    (0x54U)
#define MICROCODE_MODEXP_WITH_PUB_K_E_Y       (0x58U)  //the name is just for static analysis.
#define MICROCODE_MODEXP_MGMR_LADDER          (0x5CU)
#endif
#ifdef SUPPORT_SM9
#if 1//defined(PKE_SEC)
#define MICROCODE_SM9_FP2_PMUL_SEC            (0x60U)  //only one version
#endif
#define MICROCODE_SM9_FP2_PADD                (0x68U)
#define MICROCODE_SM9_FP2_PDBL                (0x6CU)
#if 1//defined(PKE_SEC)
#define MICROCODE_SM9_FP12_MEXP_SEC           (0x70U)  //only one version
#endif
#define MICROCODE_SM9_FP12_MMUL               (0x74U)
#define MICROCODE_SM9_PAIRING                 (0x78U)
#endif


/*********** some PKE algorithm operand length ************/
#define OPERAND_MAX_WORD_LEN                  ((OPERAND_MAX_BIT_LEN+31u)>>5)

#define ECCP_MAX_BYTE_LEN                     ((ECCP_MAX_BIT_LEN+7u)>>3)
#define ECCP_MAX_WORD_LEN                     ((ECCP_MAX_BIT_LEN+31u)>>5)

#define C25519_BYTE_LEN                       (256U/8U)
#define C25519_WORD_LEN                       (256U/32U)

#define Ed25519_BYTE_LEN                      C25519_BYTE_LEN
#define Ed25519_WORD_LEN                      C25519_WORD_LEN

#define RSA_MAX_WORD_LEN                      ((RSA_MAX_BIT_LEN+31u)>>5)
#define RSA_MAX_BYTE_LEN                      ((RSA_MAX_BIT_LEN+7u)>>3)
#define RSA_MIN_BIT_LEN                       (512U)

#define DH_MAX_WORD_LEN                       ((DH_MAX_BIT_LEN+31u)>>5)
#define DH_MAX_BYTE_LEN                       ((DH_MAX_BIT_LEN+7u)>>3)
#define DH_MIN_BIT_LEN                        (512U)

#define SM2_BIT_LEN                           (256U)
#define SM2_BYTE_LEN                          (32U)
#define SM2_STEPS                             SM2_BYTE_LEN
#define SM2_WORD_LEN                          (8U)

#define SM9_BASE_BIT_LEN                      (256U)
#define SM9_BASE_BYTE_LEN                     (SM9_BASE_BIT_LEN/8U)
#define SM9_STEPS                             SM9_BASE_BYTE_LEN
#define SM9_BASE_WORD_LEN                     (SM9_BASE_BIT_LEN/32U)


/******************* PKE return code ********************/
#define PKE_RT_OFFSET                         (0x700U)
#define PKE_SUCCESS                           (0U)
#define PKE_STOP                              (PKE_RT_OFFSET + 1U)
#define PKE_NO_MODINV                         (PKE_RT_OFFSET + 2U)
#define PKE_NOT_ON_CURVE                      (PKE_RT_OFFSET + 3U)
#define PKE_INVALID_MC                        (PKE_RT_OFFSET + 4U)
#define PKE_ZERO_ALL                          (PKE_RT_OFFSET + 5U)  //for ECCP input check
#define PKE_INTEGER_TOO_BIG                   (PKE_RT_OFFSET + 6U)  //for ECCP input check
#define PKE_INVALID_INPUT                     (PKE_RT_OFFSET + 7U)
#define PKE_FINISHED                          (PKE_RT_OFFSET + 8U)
#define PKE_POINTER_NULL                      (PKE_RT_OFFSET + 9U)
#define PKE_ERROR                             (PKE_RT_OFFSET + 10U)


/* function: get PKE RAM A slot
 * parameters:
 *     index ---------------------- input, index of slot(0,1,2,...)
 *     step ----------------------- input, byte length of slot
 * return: uint32_t pointer to the slot
 * caution:
 *     1. 
 */
static inline uint32_t *rPKE_A(uint32_t index, uint32_t step)
{
#ifdef CONFIG_UNIT_TEST
    return (uint32_t *)(&(PKE_RAM[(0x0400U+(index*step))/4]));
#else
    return (uint32_t *)(PKE_BASE_ADDR+0x0400U+(index*step));
#endif
}


/* function: get PKE RAM B slot
 * parameters:
 *     index ---------------------- input, index of slot(0,1,2,...)
 *     step ----------------------- input, byte length of slot
 * return: uint32_t pointer to the slot
 * caution:
 *     1. 
 */
static inline uint32_t *rPKE_B(uint32_t index, uint32_t step)
{
#ifdef CONFIG_UNIT_TEST
    return (uint32_t *)(&(PKE_RAM[(0x1000U+(index*step))/4]));
#else
    return (uint32_t *)(PKE_BASE_ADDR+0x1000U+(index*step));
#endif
}


//APIs

uint32_t *rPKE_A(uint32_t index, uint32_t step);

uint32_t *rPKE_B(uint32_t index, uint32_t step);

uint32_t pke_get_version(void);

uint32_t pke_get_driver_version(void);

uint32_t pke_init(void);

void pke_clear_interrupt(void);

void pke_enable_interrupt(void);

void pke_disable_interrupt(void);

uint32_t pke_set_operand_width(uint32_t bitLen);

uint32_t pke_get_operand_bytes(void);

void pke_set_microcode(uint32_t addr);

void pke_start(void);

uint32_t pke_check_rt_code(void);

void pke_wait_till_done(void);

uint32_t pke_set_micro_code_start_wait_return_code(uint32_t micro_code);

uint32_t pke_modinv_internal(const uint32_t *a, uint32_t *ainv, uint32_t modWordLen, 
        uint32_t aWordLen, uint32_t step_bytes);

uint32_t pke_modinv(const uint32_t *modulus, const uint32_t *a, uint32_t *ainv, 
        uint32_t modWordLen, uint32_t aWordLen);

uint32_t pke_modinv_256bits(const uint32_t *a, uint32_t *ainv);

uint32_t pke_mod_add_sub_mul_internal(const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t wordLen, uint32_t step_bytes, uint32_t micro_code);

uint32_t pke_mod_add_sub_mul_256bits_internal(const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t micro_code);

uint32_t pke_modadd_modsub_256bits(const uint32_t *modulus, const uint32_t *a, 
        const uint32_t *b, uint32_t *out, uint32_t micro_code);

uint32_t pke_modadd(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
        uint32_t *out, uint32_t wordLen);

uint32_t pke_modsub(const uint32_t *modulus, const uint32_t *a, const uint32_t *b,
        uint32_t *out, uint32_t wordLen);

uint32_t pke_add(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen);

uint32_t pke_sub(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen);

uint32_t pke_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t a_wordLen, 
        uint32_t b_wordLen, uint32_t out_wordLen);

uint32_t pke_mul(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t ab_wordLen);

uint32_t pke_pre_calc_mont_N0(void);

uint32_t pke_pre_calc_mont(const uint32_t *modulus, uint32_t bitLen, uint32_t *H);

uint32_t pke_pre_calc_mont_no_output(const uint32_t *modulus, uint32_t wordLen);

uint32_t pke_load_modulus(const uint32_t *modulus, uint32_t bitLen, uint32_t *step_bytes);

uint32_t pke_load_modulus_and_pre_monts(const uint32_t *modulus, const uint32_t *modulus_h, 
        uint32_t bitLen);

uint32_t pke_load_modulus_and_pre_monts_256bits(const uint32_t *modulus, const uint32_t *modulus_h);

uint32_t pke_set_modulus_and_pre_monts(const uint32_t *modulus, const uint32_t *modulus_h, 
        uint32_t bitLen);

uint32_t pke_modmul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out, uint32_t wordLen);

uint32_t pke_modmul(const uint32_t *modulus, const uint32_t *a, const uint32_t *b, 
        uint32_t *out, uint32_t wordLen);

uint32_t pke_modexp_internal(const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen);

uint32_t pke_modexp(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen);

uint32_t pke_modexp_check_input(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen);

uint32_t pke_modexp_U8(const uint8_t *modulus, const uint8_t *exponent, const uint8_t *base,
        uint8_t *out, uint32_t mod_bitLen, uint32_t exp_bitLen, uint32_t calc_pre_monts);

uint32_t pke_mod(const uint32_t *a, uint32_t aWordLen, const uint32_t *b, const uint32_t *b_h, 
        uint32_t bWordLen, uint32_t *c);

uint32_t eccp_pointMul_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir_internal(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir_safe_internal(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_Shamir_safe(const eccp_curve_st *curve,
        const uint32_t *k1, const uint32_t *P1x, const uint32_t *P1y,
        const uint32_t *k2, const uint32_t *P2x, const uint32_t *P2y,
        uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_base(const eccp_curve_st *curve, const uint32_t *k, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointAdd_internal(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointAdd(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointAdd_safe_internal(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);
        
uint32_t eccp_pointAdd_safe(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

#if 0
#define ECCP_POINT_DOUBLE   //not recommended to define
#endif
#ifdef ECCP_POINT_DOUBLE
uint32_t eccp_pointDouble_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointDouble(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy);
#endif

uint32_t eccp_pointVerify_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t eccp_pointVerify(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t eccp_check_point_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t eccp_check_point(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t eccp_get_pubkey_from_prikey(const eccp_curve_st *curve, const uint8_t *priKey, uint8_t *pubKey);

uint32_t eccp_getkey(const eccp_curve_st *curve, uint8_t *priKey, uint8_t *pubKey);


#ifdef SUPPORT_C25519
uint32_t x25519_pointMul(const mont_curve_st *curve, const uint32_t *k, const uint32_t *Pu, uint32_t *Qu);

void x25519_ed25519_decode_scalar(const uint8_t *k, uint8_t *out);

uint32_t pke_modexp_256bits_internal(const uint32_t *exponent, const uint32_t *base, uint32_t *out);

uint32_t ed25519_decode_point_internal(const uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32]);

uint32_t ed25519_decode_point(const uint8_t in_y[32], uint8_t out_x[32], uint8_t out_y[32]);

uint32_t ed25519_pointMul_internal(const edward_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t ed25519_pointMul(const edward_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t ed25519_pointAdd_internal(const edward_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

uint32_t ed25519_pointAdd(const edward_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);
#endif


#ifdef SUPPORT_SM9
uint32_t sm9_fp2_add_internal(const uint32_t *a, const uint32_t *b, uint32_t *out);

uint32_t sm9_fp2_sub_internal(const uint32_t *a, const uint32_t *b, uint32_t *out);

uint32_t sm9_fp2_mul_internal(const uint32_t *a, const uint32_t *b, uint32_t *out);
                   
void sm9_fp2_get_eccp_output_point_from_pke_ram(uint32_t *Qx, uint32_t *Qy);

uint32_t sm9_fp2_pointMul_s_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t sm9_fp2_pointMul_s(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t sm9_fp2_pointAdd_internal(const uint32_t *P1x, const uint32_t *P1y, const uint32_t *P2x, 
        const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

uint32_t sm9_fp2_pointAdd(const eccp_curve_st *curve, const uint32_t *P1x, const uint32_t *P1y, 
        const uint32_t *P2x, const uint32_t *P2y, uint32_t *Qx, uint32_t *Qy);

#define SM9_ECCP_POINT_DOUBLE   //not recommended to define
#ifdef SM9_ECCP_POINT_DOUBLE
uint32_t sm9_fp2_pointDouble_internal(const uint32_t *Px, const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t sm9_fp2_pointDouble(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py, 
        uint32_t *Qx, uint32_t *Qy);
#endif

uint32_t sm9_fp2_pointVerify_internal(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t sm9_fp2_pointVerify(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

uint32_t sm9_fp2_check_point_internal(const eccp_curve_st *curve, const uint32_t *Px, 
        const uint32_t *Py);

uint32_t sm9_fp2_check_point(const eccp_curve_st *curve, const uint32_t *Px, const uint32_t *Py);

void sm9_fp12_get_output_from_pke_ram(uint32_t *ff);

uint32_t sm9_fp12_exp_s(const uint32_t *f, const uint32_t *k, uint32_t *ff);

uint32_t sm9_fp12_mul(const uint32_t *f1, const uint32_t *f2, uint32_t *ff);

uint32_t sm9_pairing(const uint32_t *Px, const uint32_t *Py, const uint32_t *Qx, 
        const uint32_t *Qy, uint32_t *ff);
#endif


#ifdef PKE_SEC

uint32_t pke_sec_init(void);

uint32_t pke_sec_uninit(void);

uint32_t pke_modexp_ladder_internal(const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen);

uint32_t pke_modexp_ladder(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *base,
        uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen);

uint32_t pke_modexp_with_pub(const uint32_t *modulus, const uint32_t *exponent, const uint32_t *pub, 
        const uint32_t *base, uint32_t *out, uint32_t mod_wordLen, uint32_t exp_wordLen, 
        uint32_t pub_wordLen);

uint32_t eccp_pointMul_sec_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_sec_safe_internal(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_pointMul_sec(const eccp_curve_st *curve, const uint32_t *k, const uint32_t *Px, 
        const uint32_t *Py, uint32_t *Qx, uint32_t *Qy);

uint32_t eccp_get_pubkey_from_prikey_s(const eccp_curve_st *curve, const uint8_t *priKey, 
        uint8_t *pubKey);

uint32_t eccp_getkey_s(const eccp_curve_st *curve, uint8_t *priKey, uint8_t *pubKey);
#endif


#ifdef __cplusplus
}
#endif

#endif

