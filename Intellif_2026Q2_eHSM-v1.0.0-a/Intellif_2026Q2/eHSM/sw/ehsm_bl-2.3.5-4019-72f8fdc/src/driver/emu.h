#ifndef EMU_H
#define EMU_H

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
// clang-format off
#define EMU_HW_0_REG *((volatile uint32_t *)(0x30100008U))
#define EMU_HW_1_REG *((volatile uint32_t *)(0x3010000CU))
#define EMU_FW_0_REG *((volatile uint32_t *)(0x30100010U))
#define EMU_FW_1_REG *((volatile uint32_t *)(0x30100014U))


#define EMU_HW_TRIG_0_REG *((volatile uint32_t *)(0x30100140U))
#define EMU_HW_TRIG_1_REG *((volatile uint32_t *)(0x30100144U))

#define EMU_INT_SENSOR            *((volatile uint32_t *)(0x30100020U))
#define EMU_INT_SOC_ERR           *((volatile uint32_t *)(0x30100024U))
#define EMU_INT_HW_0              *((volatile uint32_t *)(0x30100028U))
#define EMU_INT_HW_1              *((volatile uint32_t *)(0x3010002CU))
#define EMU_INTEN_SENSOR          *((volatile uint32_t *)(0x30100040U))
#define EMU_INTEN_SOC_ERR         *((volatile uint32_t *)(0x30100044U))
#define EMU_INTEN_HW_0            *((volatile uint32_t *)(0x30100048U))
#define EMU_INTEN_HW_1            *((volatile uint32_t *)(0x3010004CU))
#define EMU_ERR_SENSOR            *((volatile uint32_t *)(0x30100000U))
#define EMU_ERR_TRIG_LOCK         *((volatile uint32_t *)(0x30100180U))

#define EMU_ERR_TRIG_UNLOCK_VALUE (0x965F0CBAU)
#define EMU_ERR_TRIG_LOCK_VALUE   (0x0U)

#define SENSOR_IN_MAX_BIT_SIZE        32U
#define SENSOR_RSP_STOP_IP_CORE_INDEX 0U
#define SENSOR_RSP_CLEAR_RAM_K_INDEX  1U
#define SENSOR_RSP_CLEAR_NVM_K_INDEX  2U
#define SENSOR_RSP_CLEAR_OTP_K_INDEX  3U
#define SENSOR_RSP_RESET_EHSM_INDEX   4U
#define SENSOR_RSP_RESET_SOC_INDEX    5U
#define SENSOR_RSP_REV1_INDEX         6U
#define SENSOR_RSP_REV2_INDEX         7U

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

/* Error Code For HASH STL failed */
#define FW_ERROR_SKE_STL_FAILED 39U

/* Error bootloader code verify failed */
#define FW_ERROR_BL_VERIFY_FAILED 40U

#define FW_ERROR_CPU_EXCEPTION 41U

#define FW_ERROR_LOAD_PATCH_FAILED 42U

#define FW_ERROR_INIT_OTP_FAILED 43U

#define FW_ERROR_FAULT_INJECTION_DETECTED 44U

#define FW_WARN_UID_CRC_MISMATCH 62U
#define FW_WARN_VERSION_MISMATCH 63U

#define FW_ERROR_MAX 64U

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef void (*emu_err_handler)(void);

typedef struct {
    uint32_t pad;
} emu_status_st;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void emu_init(emu_err_handler handle);

uint32_t emu_trigger_fw_error(uint32_t err);

void emu_enable_hw_int(bool_t enable);
// clang-format on
#endif
