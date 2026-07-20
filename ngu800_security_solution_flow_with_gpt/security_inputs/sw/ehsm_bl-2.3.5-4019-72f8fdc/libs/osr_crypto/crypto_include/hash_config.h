#ifndef HASH_CONFIG_H
#define HASH_CONFIG_H

#include "common_config.h"

/************************************************************************************
 *******************************    HASH config    **********************************
 ************************************************************************************/

/*
 *function: hash_hp IP base address
 *caution:
 */
#define HASH_BASE_ADDR (CRYPTO_BASE_ADDR + 0x31400000U) // HASH register base address

/*
 *function: define secure version(hardware)
 *caution:
 */
#if 0
#define HASH_SEC
#endif

/*
 *function: supported hash algorithms
 *caution:
 */
#define SUPPORT_HASH_SM3
#define SUPPORT_HASH_SHA256
#ifndef BUILD_EHSM_BL
#define SUPPORT_HASH_MD5
#define SUPPORT_HASH_SHA384
#define SUPPORT_HASH_SHA512
#define SUPPORT_HASH_SHA1
#define SUPPORT_HASH_SHA224
#define SUPPORT_HASH_SHA512_224
#define SUPPORT_HASH_SHA512_256
#define SUPPORT_HASH_SHA3_224
#define SUPPORT_HASH_SHA3_256
#define SUPPORT_HASH_SHA3_384
#define SUPPORT_HASH_SHA3_512
#endif // !def BUILD_EHSM_BL

/*
 *function: support hash dma style
 *caution:
 */
#if 1
#define HASH_DMA_FUNCTION
#endif

#ifdef HASH_DMA_FUNCTION
/*
 *function: ram for hash dma
 *caution:
 *    1.just for temporary use
 */
#define HASH_DMA_RAM_BASE (0x60000000U)
#endif

/*
 *function: support multiple thread
 *caution:
 */
#if 1
#define CONFIG_HASH_SUPPORT_MUL_THREAD
#endif

/*
 *function: support node style
 *caution:
 */
#if 1
#define SUPPORT_HASH_NODE
#endif

#ifdef HASH_DMA_FUNCTION
/*
 *function: support dma node style
 *caution:
 */
#define SUPPORT_HASH_DMA_NODE
#endif

/*
 *function: support PBKDF2
 *caution:
 */
#ifndef BUILD_EHSM_BL
#define SUPPORT_PBKDF2
#endif

#ifdef SUPPORT_PBKDF2
/*
 *function: support PBKDF2 high speed
 *caution:
 */
#define PBKDF2_HIGH_SPEED
#endif

/*
 *only FW software support PBKDF2_KMU
 */
#ifdef CONFIG_EHSM_CUSTOM_ID
#define SUPPORT_PBKDF2_KMU
#endif
/*
 *function: support ANSI_x9.36_KDF
 *caution:
 */
#if 1
#define SUPPORT_ANSI_X9_63_KDF
#endif

/*
 *function: support hash reverse byte order in word
 *caution:
 *    1.endian choice, default close
 */
#if 0
#define HASH_REVERSE_BYTE_ORDER_IN_WORD
#endif

/*
 *function: support hash secure port
 *caution:
 *    1.if key is from secure port, HMAC_MAX_K_IDX is the max key index(or the number of keys),
 *      and HMAC_MAX_SP_K_SIZE is the max key byte length
 */
#ifndef BUILD_EHSM_BL
#define HMAC_SECURE_PORT_FUNCTION
#endif

/*
 *function: support hash code static analysis
 *caution:
 *    1.close the macro may improve alg speed
 */
#if 0
#define HASH_CONFIG_SUPPORT_STATIC_ANALYSIS
#endif

#if defined(HASH_DMA_FUNCTION)
/*
 *function: support DMA address with high 32bit and low 32bit and address remap
 *caution:
 *    1.please make sure dependency function rewrite on libextension model
 */
#if 1
#define CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
#endif

#endif

#endif
