#ifndef EHSM_EXCEPTION_DETECTOR_H
#define EHSM_EXCEPTION_DETECTOR_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/* Error Code Definition*/

/* Error Code For Secure Boot */
#define FW_ERROR_SECBOOT_SELFTEST_FAIL 0U
#define FW_ERROR_SECBOOT_CODE_INV      1U
#define FW_ERROR_SECBOOT_VERSION_FAIL  2U
#define FW_ERROR_SECBOOT_KEY_INV       3U
#define FW_ERROR_SECBOOT_PUBKEY_INV    4U
#define FW_ERROR_SECBOOT_VERIFY_FAIL   5U
#define FW_ERROR_SECBOOT_MOV_CODE_FAIL 6U
#define FW_ERROR_SECBOOT_AIG_INV       7U
#define FW_ERROR_SECBOOT_WORK_FAIL     8U
#define FW_ERROR_SECBOOT_SYS_ERR       9U

/* Error Code For OTP Key */
#define FW_ERROR_NO_OTP_SOC_KEY 14U
#define FW_ERROR_NO_OTP_HSM_KEY 15U

/* Error Code For Self Test */
#define FW_ERROR_SELFTEST_HASH_FAIL 16U
#define FW_ERROR_SELFTEST_SKE_FAIL  17U
#define FW_ERROR_SELFTEST_PKE_FAIL  18U
#define FW_ERROR_SELFTEST_TRNG_FAIL 19U
#define FW_ERROR_SELFTEST_HW_FAIL   20U

/* Error Code For TRNG health test failed */
#define FW_ERROR_TRNG_HT_FAIL    24U
#define FW_ERROR_OTP_KEY_CRC_ERR 27U

/* Error Code For Mailbox */
#define FW_ERROR_MB_INT_NO_DATA     32U
#define FW_ERROR_MB_NOTE_CLR_FAILED 33U
#define FW_ERROR_MB_DBG_PROT_ERROR  34U

/* Error Code For Uart */
#define FW_ERROR_UART_DBG_PROT_ERROR 35U
#define FW_ERROR_UART_INT_CLR_FAILED 36U

/* Error Code For Watchdog timeout */
#define FW_ERROR_WDG_TIMEOUT 37U

/* Error Code For HASH STL failed */
#define FW_ERROR_HASH_STL_FAILED 38U

/* Error Code For SKE STL failed */
#define FW_ERROR_SKE_STL_FAILED 39U

/* Error Code For CHACHA STL failed */
#define FW_ERROR_CHACHA_STL_FAILED 40U

/* Error bootloader code verify failed */
#define FW_ERROR_BL_VERIFY_FAILED 44U

/* Error register config failed */
#define FW_ERROR_REG_CFG_FAILED 45U

/* Stack check failed */
#define FW_ERROR_STACK_CHECK_FAILED 46U

#define FW_ERROR_FAULT_INJECTION_DETECTED 47U

#define FW_ERROR_OTP_WRITE_FAILED 48U

#define FW_ERROR_MAX 49U

#define FW_ERROR_TRIG_TIMES_01 1U
#define FW_ERROR_TRIG_TIMES_03 3U

/* Error Code bit For STL */
#define STL_ERROR_BIT_SHA1        0U
#define STL_ERROR_BIT_MD5         1U
#define STL_ERROR_BIT_SM3         2U
#define STL_ERROR_BIT_SHA256      3U
#define STL_ERROR_BIT_SHA512      4U
#define STL_ERROR_BIT_SHA3        5U
#define STL_ERROR_BIT_AES128_CBC  8U
#define STL_ERROR_BIT_AES128_OFB  9U
#define STL_ERROR_BIT_AES128_GCM  10U
#define STL_ERROR_BIT_AES128_CCM  11U
#define STL_ERROR_BIT_AES256_CMAC 12U
#define STL_ERROR_BIT_AES256_XTS  13U
#define STL_ERROR_BIT_AES256_CFB  14U
#define STL_ERROR_BIT_AES256_CTR  15U
#define STL_ERROR_BIT_SM4         16U
#define STL_ERROR_BIT_DES         17U
#define STL_ERROR_BIT_CHACHA      18U
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t expt_det_init(void);

uint32_t expt_det_check_before(cmd_packet_st *cmd);

void expt_det_check_after(void);

void expt_det_check_idle(void);

uint32_t expt_det_add_error(uint32_t code);

uint32_t expt_det_reset_error(uint32_t code);
#endif // EHSM_EXCEPTION_DETECTOR_H
