#ifndef SKE_CONFIG_H
#define SKE_CONFIG_H


#include "common_config.h"






/************************************************************************************
 ********************************    SKE config    **********************************
 ************************************************************************************/

/*
 *function: define available SKE IP
 *caution:
 */
#define SKE_HP


/*
 *function: ske IP base address
 *caution:
 */
#define SKE_BASE_ADDR            (CRYPTO_BASE_ADDR + 0x31800000U)


/*
 *function: supported ske algorithms
 *caution:
 *    1.choose at least one algorithm
 */
#ifndef BUILD_EHSM_BL
#define SUPPORT_SKE_DES
#define SUPPORT_SKE_TDES_128
#define SUPPORT_SKE_TDES_192
#define SUPPORT_SKE_TDES_EEE_128
#define SUPPORT_SKE_TDES_EEE_192
#define SUPPORT_SKE_AES_192
#endif

#define SUPPORT_SKE_AES_128
#define SUPPORT_SKE_AES_256
#define SUPPORT_SKE_SM4


/*
 *function: supported ske mode
 *caution:
 *    1.GMAC is specialization of GCM mode
 *    2.hardware does not support AES_XCBC_MAC_96 directly
 *    3.5 basic mode(ECB/CBC/CFB/OFB/CTR) should be open
 */
#define SUPPORT_SKE_MODE_BYPASS
#define SUPPORT_SKE_MODE_ECB
#define SUPPORT_SKE_MODE_CBC
#define SUPPORT_SKE_MODE_CFB
#define SUPPORT_SKE_MODE_OFB
#define SUPPORT_SKE_MODE_CTR
#define SUPPORT_SKE_MODE_XTS
#define SUPPORT_SKE_MODE_GCM
#ifdef SUPPORT_SKE_MODE_GCM
#define SUPPORT_SKE_MODE_GMAC
#endif
#define SUPPORT_SKE_MODE_CMAC
#define SUPPORT_SKE_MODE_CBC_MAC
#define SUPPORT_SKE_MODE_CCM
#define SUPPORT_SKE_AES_XCBC_MAC_96


/*
 *function: support ske dma style
 *caution:
 */
#if 1
#define SKE_DMA_FUNCTION
#endif


#ifdef SKE_DMA_FUNCTION
/*
 *function: ram for ske dma
 *caution:
 *    1.just for temporary use
 */
#define SKE_DMA_RAM_BASE         (0x60000000U)
#endif


/*
 *function: support multiple thread
 *caution:
 */
#if 1
#define CONFIG_SKE_SUPPORT_MUL_THREAD
#endif


/*
 *function: whether for standalone ip or firmware
 *caution:
 *   1. differnt choices affect some structure members
 *   2. if open the macro, will reduces stack cost
 *   3. if use for firmware, should closed this macro
 */
#if 1
#ifndef CONFIG_SKE_SUPPORT_MUL_THREAD
#define CONFIG_SUPPORT_STRUCTURE_OPTIMIZATION
#endif
#endif


/*
 *function: support algorithm use on hardware suspend version
 *caution:
 */
#if 1
#define CONFIG_SKE_SUPPORT_SUSPEND
#endif


/*
 *function: support ske clear hardware sensitive information 
            after the whole calculation is over
 *caution:
 *    1.close default 
 */
#if 0
#define SKE_CONFIG_CLEAR_HARDWARE_INFORMATION
#endif


/*
 *function: support ske secure port
 *caution:
 *    1.if key is from secure port, SKE_MAX_K_IDX is the max key index(or the number of keys)
 */
#define SKE_SECURE_PORT_FUNCTION
#ifdef SKE_SECURE_PORT_FUNCTION
#define SKE_MAX_K_IDX                                   (31U)  //max key idx
#endif


#ifdef SKE_DMA_FUNCTION
/*
 *function: support clear output when dma operate fail
 *caution:
 *    1.close default 
 */
#if 0
#define CONFIG_SKE_SUPPORT_DMA_CLEAR
#endif
#endif


/*
 *function: support ske reverse byte order in word
 *caution: 
 *    1.close default 
 */
#if 0
#define SKE_REVERSE_BYTE_ORDER_IN_WORD
#endif


#ifdef SKE_DMA_FUNCTION
/*
 *function: support DMA address with high 32bit and low 32bit
 *caution:
 */
#if 1
#define CONFIG_SKE_SUPPORT_DMA_HIGH_LOW_ADDRESS
#endif
#endif


#endif
