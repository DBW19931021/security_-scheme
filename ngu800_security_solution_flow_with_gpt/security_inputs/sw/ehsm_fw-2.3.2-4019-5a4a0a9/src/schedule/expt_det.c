/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "expt_det.h"
#include "mb.h"
#include "driver/watchdog_driver.h"
#include "driver/emu_driver.h"
#include "component/util.h"
#include "hash_hmac/hash.h"
#include "ske/ske.h"
#include "ske/ske_gcm_gmac.h"
#include "ske/ske_ccm.h"
#include "ske/ske_cmac.h"
#include "ske/ske_xts.h"
#include "driver/reg_lock.h"
#include "component/dbgcmd_parser.h"
#include "crypto_util.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/* FW action after trigger emu error */
#define FW_ACTION_NORMAL 0U
#define FW_ACTION_LOOP   1U

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint16_t error_code;
    uint16_t trigger_level;
    uint16_t action;
} error_mgr_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static uint16_t g_error_times[FW_ERROR_MAX];

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t expt_det_init(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ret = emu_init();

    return ret;
}

uint32_t expt_det_check_before(cmd_packet_st *cmd)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    UNUSED(cmd);
    return ret;
}

void expt_det_check_after(void)
{
    watchdog_feed();
}

void expt_det_check_idle(void)
{
    watchdog_feed();
}

uint32_t expt_det_add_error(uint32_t code)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    bool_t do_action = false;
    uint32_t i = 0U;
    uint32_t num = 0;
    const error_mgr_st *error_mgr = NULL;
    bool_t loop = true;

    static const error_mgr_st g_error_mgr[] = {
        // config for FW_ERROR_SECBOOT_SELFTEST_FAIL
        { .error_code = FW_ERROR_SECBOOT_SELFTEST_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_CODE_INV
        { .error_code = FW_ERROR_SECBOOT_CODE_INV, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_VERSION_FAIL
        { .error_code = FW_ERROR_SECBOOT_VERSION_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_KEY_INV
        { .error_code = FW_ERROR_SECBOOT_KEY_INV, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_PUBKEY_INV
        { .error_code = FW_ERROR_SECBOOT_PUBKEY_INV,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_VERIFY_FAIL
        { .error_code = FW_ERROR_SECBOOT_VERIFY_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_MOV_CODE_FAIL
        { .error_code = FW_ERROR_SECBOOT_MOV_CODE_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_AIG_INV
        { .error_code = FW_ERROR_SECBOOT_AIG_INV, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_WORK_FAIL
        { .error_code = FW_ERROR_SECBOOT_WORK_FAIL, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SECBOOT_SYS_ERR
        { .error_code = FW_ERROR_SECBOOT_SYS_ERR, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_NO_OTP_SOC_KEY
        { .error_code = FW_ERROR_NO_OTP_SOC_KEY, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_NO_OTP_HSM_KEY
        { .error_code = FW_ERROR_NO_OTP_HSM_KEY, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SELFTEST_HASH_FAIL
        { .error_code = FW_ERROR_SELFTEST_HASH_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SELFTEST_SKE_FAIL
        { .error_code = FW_ERROR_SELFTEST_SKE_FAIL, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SELFTEST_PKE_FAIL
        { .error_code = FW_ERROR_SELFTEST_PKE_FAIL, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SELFTEST_TRNG_FAIL
        { .error_code = FW_ERROR_SELFTEST_TRNG_FAIL,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        // config for FW_ERROR_SELFTEST_HW_FAIL
        { .error_code = FW_ERROR_SELFTEST_HW_FAIL, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_TRNG_HT_FAIL
        { .error_code = FW_ERROR_TRNG_HT_FAIL, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_OTP_KEY_CRC_ERR
        { .error_code = FW_ERROR_OTP_KEY_CRC_ERR, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_MB_INT_NO_DATA
        { .error_code = FW_ERROR_MB_INT_NO_DATA, .trigger_level = FW_ERROR_TRIG_TIMES_03, .action = FW_ACTION_NORMAL },
        // config for FW_ERROR_MB_NOTE_CLR_FAILED
        { .error_code = FW_ERROR_MB_NOTE_CLR_FAILED,
            .trigger_level = FW_ERROR_TRIG_TIMES_03,
            .action = FW_ACTION_NORMAL },
        // config for FW_ERROR_MB_DBG_PROT_ERROR
        { .error_code = FW_ERROR_MB_DBG_PROT_ERROR,
            .trigger_level = FW_ERROR_TRIG_TIMES_03,
            .action = FW_ACTION_NORMAL },
        // config for FW_ERROR_UART_DBG_PROT_ERROR
        { .error_code = FW_ERROR_UART_DBG_PROT_ERROR,
            .trigger_level = FW_ERROR_TRIG_TIMES_03,
            .action = FW_ACTION_NORMAL },
        // config for FW_ERROR_UART_INT_CLR_FAILED
        { .error_code = FW_ERROR_UART_INT_CLR_FAILED,
            .trigger_level = FW_ERROR_TRIG_TIMES_03,
            .action = FW_ACTION_NORMAL },
        // config for FW_ERROR_WDG_TIMEOUT
        { .error_code = FW_ERROR_WDG_TIMEOUT, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_BL_VERIFY_FAILED
        { .error_code = FW_ERROR_BL_VERIFY_FAILED, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        // config for FW_ERROR_REG_CFG_FAILED
        { .error_code = FW_ERROR_REG_CFG_FAILED, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
        { .error_code = FW_ERROR_FAULT_INJECTION_DETECTED,
            .trigger_level = FW_ERROR_TRIG_TIMES_01,
            .action = FW_ACTION_LOOP },
        { .error_code = FW_ERROR_OTP_WRITE_FAILED, .trigger_level = FW_ERROR_TRIG_TIMES_01, .action = FW_ACTION_LOOP },
    };

    num = sizeof(g_error_mgr) / sizeof(error_mgr_st);
    for (i = 0U; i < num; i++) {
        if (g_error_mgr[i].error_code == code) {
            error_mgr = &g_error_mgr[i];
            break;
        }
    }

    if ((NULL == error_mgr) || (code >= FW_ERROR_MAX)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (g_error_times[code] < UINT16_MAX) {
            g_error_times[code]++;
        }
        if (g_error_times[code] >= error_mgr->trigger_level) {
            ret = emu_trigger_fw_error(code);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                do_action = true;
            }
        }
    }

    if (true == do_action) {
        g_error_times[code] = 0;
        if (FW_ACTION_LOOP == error_mgr->action) {
            while (loop) { }
        }
    }

    return ret;
}

uint32_t expt_det_reset_error(uint32_t code)
{
    uint32_t ret;

    if (code >= FW_ERROR_MAX) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        g_error_times[code] = 0;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

/**
 *
 */
