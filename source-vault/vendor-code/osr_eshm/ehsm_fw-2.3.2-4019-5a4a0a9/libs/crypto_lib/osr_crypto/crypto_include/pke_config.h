#ifndef PKE_CONFIG_H
#define PKE_CONFIG_H

#include "common_config.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *******************************    PKE config    ***********************************
 ************************************************************************************/

/*
 *function: define available PKE IP
 *caution:
 */
#define PKE_HP

/*
 *function: define secure version(hardware & software driver)
 *caution:
 */

#if defined(CONFIG_BL_SEC_LIB_ENABLE) && CONFIG_BL_SEC_LIB_ENABLE
#define PKE_SEC
#endif

/*
 *function: pke IP base address
 *caution:
 */
#ifdef CONFIG_UNIT_TEST
extern uint32_t PKE_RAM[8 * 1024 / 4];
#define PKE_BASE_ADDR (&PKE_RAM[0]) // PKE register base address
#else
#define PKE_BASE_ADDR (CRYPTO_BASE_ADDR + 0x31C00000U) // PKE register base address
#endif

/*
 *function: some PKE algorithm operand maximum bit length
 *caution:
 */
#define OPERAND_MAX_BIT_LEN (4096u)
#define ECCP_MAX_BIT_LEN    (521u)
#define RSA_MAX_BIT_LEN     OPERAND_MAX_BIT_LEN
#define DH_MAX_BIT_LEN      OPERAND_MAX_BIT_LEN

/*
 *function: supported pke algorithms
 *caution:
 */
#define SUPPORT_RSA
#define SUPPORT_ECDSA
#define SUPPORT_SM2

#ifdef SUPPORT_RSA
#define SUPPORT_RSASSA_PSS

#ifndef BUILD_EHSM_BL
#define SUPPORT_RSASSA_PKCS1_V1_5
#define SUPPORT_RSAES_OAEP
#define SUPPORT_RSAES_PKCS1_V1_5
#endif // !def BUILD_EHSM_BL
#endif

#ifndef BUILD_EHSM_BL
#define SUPPORT_DH
#define SUPPORT_ECDH
#define SUPPORT_ECIES
#define SUPPORT_SM9
#define SUPPORT_C25519
#endif // !def BUILD_EHSM_BL


#ifdef PKE_SEC

#ifdef SUPPORT_RSA
#define RSA_SEC
#endif

#ifdef SUPPORT_DH
#define DH_SEC
#endif

#ifdef SUPPORT_ECDH
#define ECDH_SEC
#endif

#ifdef SUPPORT_ECDSA
#define ECDSA_SEC
#endif

#ifdef SUPPORT_SM2
#define SM2_SEC
#endif

#ifdef SUPPORT_SM9
#define SM9_SEC
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
