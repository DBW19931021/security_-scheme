/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "secboot.h"
#include "cpu_porting.h"
#include "schedule/schedule.h"
#include "schedule/expt_det.h"
#include "component/comm.h"
#include "component/kds.h"
#include "component/otp_key.h"
#include "component/otp_map.h"
#include "driver/otp/otp_driver.h"
#include "service/crypto_util.h"
#include "driver/emu_driver.h"
#include "driver/reg_lock.h"
#include "driver/watchdog_driver.h"
#include "driver/sysreg.h"
#include "config.h"
#include "component/util.h"
#include "debug.h"
#include "fid.h"
#include "misc_srv.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/* Firmware verification successful Version counter stored in DRAM, updated to OTP by ehsm firmware*/
#define DRAM_BASE_ADDR 0x20000000U
#ifdef CONFIG_UNIT_TEST
static uint32_t g_soc_version;
static uint32_t g_hsm_version;
#define SOC_VERSION_COUNTER_ADDR  &g_soc_version
#define EHSM_VERSION_COUNTER_ADDR &g_hsm_version
#else
#define OTP_DATA_COPY_SIZE ((OTP_SOC_VERSION_ADDRESS + OTP_VERSION_SIZE) - CONFIG_EHSM_OTP_BASE_ADDR)
#define SOC_VERSION_COUNTER_ADDR  (DRAM_BASE_ADDR + OTP_DATA_COPY_SIZE)         /* SOC version counter storage address*/
#define EHSM_VERSION_COUNTER_ADDR (SOC_VERSION_COUNTER_ADDR + OTP_VERSION_SIZE + 4U) /* ehsm version counter storage address*/
#endif
#define VERSION_COUNTER_VALID 0xA55A5AA5U

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
/*Definition the default OTP key id map */
static const otp_key_attributes_st g_otp_default_keyid_map[] = {
    { KMS_KEY_ALG_SM4, 0, EHSM_OTP_CHIP_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 1, EHSM_OTP_DEVICE_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 2, EHSM_OTP_USER_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT,
        KEY_USAGE_CREATION_TRANSP_KEY | KEY_USAGE_KEYCREATION },
    { KMS_KEY_ALG_SM2, 3, EHSM_OTP_EHSM_DEBUG_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 4, EHSM_OTP_EHSM_FW_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 5, EHSM_OTP_EHSM_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 6, EHSM_OTP_EHSM_UPG_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 7, EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 8, EHSM_OTP_EHSM_PRIVATE_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_KEYCREATION },
    { KMS_KEY_ALG_SM2, 9, EHSM_OTP_SOC_DEBUG_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 10, EHSM_OTP_SOC_FW_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 11, EHSM_OTP_SOC_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 12, EHSM_OTP_SOC_UPG_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 13, EHSM_OTP_SOC_UPG_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 14, EHSM_OTP_SOC_PRIVATE_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_KEYCREATION },
    { KMS_KEY_ALG_SM4, 15, EHSM_OTP_SECRET_KEY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_TRANSPORT },
    { KMS_KEY_ALG_SM4, 16, EHSM_OTP_USER_AUTH_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT,
        KEY_USAGE_ENCRYPT | KEY_USAGE_DECRYPT | KEY_USAGE_SIGN | KEY_USAGE_VERIFY },

};
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void secboot_wdg_cb(void)
{
    (void)expt_det_add_error(FW_ERROR_WDG_TIMEOUT);
}


static uint32_t secboot_drv_init(void)
{
    uint32_t ret;

    cpu_platform_init();

    ret = watchdog_init(secboot_wdg_cb);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        watchdog_start(DEFAULT_WDT_TIMEOUT);
    }

    return ret;
}

static uint32_t secboot_comp_init(void)
{
    uint32_t ret;
    static uint8_t ram_key_mem_addr[CONFIG_EHSM_RAM_KEY_BUFFER_MAX_SIZE];

    ret = expt_det_init();
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = comm_init();
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = sch_init();
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = kds_init(ram_key_mem_addr, CONFIG_EHSM_RAM_KEY_BUFFER_MAX_SIZE);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_register_keyid_map(
            g_otp_default_keyid_map, sizeof(g_otp_default_keyid_map) / sizeof(otp_key_attributes_st));
    }

    return ret;
}

static uint32_t secboot_srv_init(void)
{
    crypto_srv_init();
    return EHSM_ERR_SW_SUCCESS;
}

static uint32_t secboot_init_hsm(void)
{
    uint32_t ret;

    ret = secboot_drv_init();
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = secboot_comp_init();
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = secboot_srv_init();
    }
    watchdog_feed();

    return ret;
}

static void secboot_update_ver_cnt(void)
{
    uint32_t ver_in_otp[OTP_VERSION_SIZE / 4U];
    uint32_t ver_in_ram[OTP_VERSION_SIZE / 4U];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (*(uint32_t *)(EHSM_VERSION_COUNTER_ADDR) == VERSION_COUNTER_VALID) {
        ret = otp_read(OTP_EHSM_VERSION_ADDRESS, (uint8_t *)ver_in_otp, OTP_VERSION_SIZE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy(ver_in_ram, (uint8_t *)(EHSM_VERSION_COUNTER_ADDR + 4U), OTP_VERSION_SIZE);
            /* Randomize timing to prevent fault injection using otp_read bus timing as trigger */
            fid_delay();
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0 == 0
            if (uint32_BigNumCmp(ver_in_ram, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) < 0) {
#else
            if (uint32_BigNumCmp(ver_in_ram, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) > 0) {
#endif
                ret = misc_write_otp_data(OTP_EHSM_VERSION_ADDRESS, (const uint8_t *)ver_in_ram, OTP_VERSION_SIZE);
                if (EHSM_ERR_SW_SUCCESS != ret) {
                    (void)expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED);
                }
            }
        } else {
            (void)expt_det_add_error(FW_ERROR_SECBOOT_SYS_ERR);
        }
    }

    if (*(uint32_t *)(SOC_VERSION_COUNTER_ADDR) == VERSION_COUNTER_VALID) {
        ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)ver_in_otp, OTP_VERSION_SIZE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy(ver_in_ram, (uint8_t *)(SOC_VERSION_COUNTER_ADDR + 4U), OTP_VERSION_SIZE);
            /* Randomize timing to prevent fault injection using otp_read bus timing as trigger */
            fid_delay();
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0 == 0
            if (uint32_BigNumCmp(ver_in_ram, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) < 0) {
#else
            if (uint32_BigNumCmp(ver_in_ram, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) > 0) {
#endif
                ret = misc_write_otp_data(OTP_SOC_VERSION_ADDRESS, (const uint8_t *)ver_in_ram, OTP_VERSION_SIZE);
                if (EHSM_ERR_SW_SUCCESS != ret) {
                    (void)expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED);
                }
            }
        } else {
            (void)expt_det_add_error(FW_ERROR_SECBOOT_SYS_ERR);
        }
    }
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#if FID_LEVEL != FID_LEVEL_NONE
// function for fid-lib
void fid_panic_callback(void)
{
    (void)expt_det_add_error(FW_ERROR_FAULT_INJECTION_DETECTED);
}
#if FID_LEVEL == FID_LEVEL_HIGH
void fid_delay_get_random(uint8_t *buf, uint32_t size)
{
    (void)cpt_get_rand_fast(buf, size);
}
#endif
#endif
void secboot_entry(void)
{
    uint32_t ret;

    ret = secboot_init_hsm();
    log_debug("secboot_entry\n");
    if (EHSM_ERR_SW_SUCCESS == ret) {
        secboot_update_ver_cnt();
        log_debug("hsm ready\n");
        sysreg_set_status(0, SYS_STA0_HSM_READY);
        sch_start();
    } else {
        watchdog_stop();
        sysreg_set_status(0, SYS_STA0_HSM_FAIL);
    }
    while (1) { }
}
/**
 *
 */
