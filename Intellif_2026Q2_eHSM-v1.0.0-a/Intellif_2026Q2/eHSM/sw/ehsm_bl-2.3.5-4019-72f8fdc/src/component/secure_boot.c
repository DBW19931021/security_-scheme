/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "secure_boot.h"
#include "fw_verify.h"
#include "config.h"
#include "kms.h"
#include "otp_key.h"
#include "kmu_driver.h"
#include "selftest.h"
#include "sysreg.h"
#include "emu.h"
#include "types.h"
#include "uart.h"
#include "mailbox.h"
#include "watchdog.h"
#include "debug.h"
#include "cpu_porting.h"
#include "schedule.h"
#include "mb.h"
#include "mmap.h"
#include "util.h"
#include "randclk.h"
#include "otp_data.h"
#include "crypto_lib_api.h"
#include "otp.h"
#include "mbcmd_parser.h"
#include "version.h"
#include "fid.h"
#if defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE)
#include "bl_patch_test.h"
#endif /* defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE) */

#include <trng/trng.h>

/***********************************************************************************************************************
 *  VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

#define SM2_SIGNATURE_SIZE 64U

#define ROM_BASE_ADDR     0x10000000U
#define DRAM_BASE_ADDR    0x20000000U

#define BOOT_FW_ENABLE_FLAG 0x6F3C0A95UL
#define DRAM_OTP_INIT_SIZE  0x140U

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/
extern uint32_t _TEXT_START;
extern uint32_t _TEXT_END;
extern uint32_t _RO_DATA_START;
extern uint32_t _RO_DATA_END;

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static volatile uint32_t g_boot_fw_enable = 0U;
typedef int (*ehsm_entry)(void);
static volatile ehsm_entry g_ehsm_fw_entry = NULL;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

/*Definition the default OTP key id map */
const otp_key_attributes_st g_otp_default_keyid_map[] = {
    { KMS_KEY_ALG_SM4, 0, EHSM_OTP_CHIP_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 1, EHSM_OTP_DEVICE_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 2, EHSM_OTP_USER_ROOT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_CREATION_TRANSP_KEY },
    { KMS_KEY_ALG_SM2, 3, EHSM_OTP_EHSM_DEBUG_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 4, EHSM_OTP_EHSM_FW_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 5, EHSM_OTP_EHSM_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 6, EHSM_OTP_EHSM_UPG_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 7, EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 8, EHSM_OTP_EHSM_PRIVATE_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 9, EHSM_OTP_SOC_DEBUG_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 10, EHSM_OTP_SOC_FW_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 11, EHSM_OTP_SOC_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 12, EHSM_OTP_SOC_UPG_ENCRYPT_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM2, 13, EHSM_OTP_SOC_UPG_VERIFY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 14, EHSM_OTP_SOC_PRIVATE_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
    { KMS_KEY_ALG_SM4, 15, EHSM_OTP_SECRET_KEY_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_TRANSPORT },
    { KMS_KEY_ALG_SM4, 16, EHSM_OTP_USER_AUTH_KEY_ID, OTP_KEY_GENERATE_TYPE_DIRECT, KEY_USAGE_NONE },
};

uint32_t secboot_get_self_test_type(uint8_t verify_alg)
{
    uint32_t mode;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
        mode = EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_PKE_RSA | EHSM_SELF_TEST_SKE_AES | EHSM_SELF_TEST_HASH_SHA2;
        break;
    case CODE_VERIFY_ALG_ECC_P256R1:
        mode = EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_PKE_ECC | EHSM_SELF_TEST_SKE_AES | EHSM_SELF_TEST_HASH_SHA2;
        break;
    case CODE_VERIFY_ALG_SM2:
        mode = EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_PKE_SM2 | EHSM_SELF_TEST_SKE_SM4 | EHSM_SELF_TEST_HASH_SM3;
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
        mode = EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_SKE_AES;
        break;
    case CODE_VERIFY_ALG_SM4_CMAC:
        mode = EHSM_SELF_TEST_TRNG | EHSM_SELF_TEST_SKE_SM4;
        break;
    default:
        mode = EHSM_SELF_TEST_ALL;
        break;
    }

    return mode;
}

#define DO_NOT_TRIGGLER_FW_ERROR 0xffffffffu // used for hw error
void secboot_report_error(uint32_t emu_err)
{
    uint32_t status = 0;
    sysreg_get_status(0, &status);
    if ((status & SYS_STA0_BOOTLOADER_READY) == 0) {
        // set bootloader_err if bootloader_ready is not set
        // if bootloader_ready is set, bootloader_err will not set
        sysreg_set_status(0, SYS_STA0_BOOTLOADER_ERR);
    }
    if (emu_err != DO_NOT_TRIGGLER_FW_ERROR) {
        emu_trigger_fw_error(emu_err);
    }
    sysreg_set_status(0, SYS_STA0_BOOTLOADER_READY);

    log_flush();
    // do not block in test mode
    if (sysreg_get_life_cycle() != SYS_STA0_LIFECYCLE_MCUTEST) {
        while (1) {
            asm("wfi");
        }
    }
}

void secboot_set_self_test_emu_err(void)
{
    uint32_t result[2];
    uint32_t failed_type;

    selftest_get_test_result(result);

    failed_type = result[1] & ~result[0];
    if ((failed_type & EHSM_SELF_TEST_HASH) != 0) {
        emu_trigger_fw_error(FW_ERROR_SELFTEST_HASH_FAIL);
    }

    if ((failed_type & EHSM_SELF_TEST_PKE) != 0) {
        emu_trigger_fw_error(FW_ERROR_SELFTEST_PKE_FAIL);
    }

    if ((failed_type & EHSM_SELF_TEST_SKE) != 0) {
        emu_trigger_fw_error(FW_ERROR_SELFTEST_SKE_FAIL);
    }

    if ((failed_type & EHSM_SELF_TEST_TRNG) != 0) {
        emu_trigger_fw_error(FW_ERROR_SELFTEST_TRNG_FAIL);
    }
    // call emu_trigger_fw_error before will not hold CPU, and secboot_report_error will hold CPU
    log_error("self test failed\n");
    secboot_report_error(FW_ERROR_SECBOOT_SELFTEST_FAIL);
}

uint32_t secboot_do_self_test(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t test_type;
    uint32_t type = sysreg_get_self_test_type();

    if (OTP_FW_SELF_TEST_ALL == type) {
        ret = selftest_test_alg(EHSM_SELF_TEST_ALL);
    } else if ((OTP_FW_SELF_TEST_BASIC1 == type) || (OTP_FW_SELF_TEST_BASIC2 == type)) {
        test_type = secboot_get_self_test_type(sysreg_get_soc_verify_alg());
        test_type |= secboot_get_self_test_type(sysreg_get_fw_verify_alg());
        ret = selftest_test_alg(test_type);
    } else {
    }

    if (EHSM_ERR_SW_SUCCESS != ret) {
        secboot_set_self_test_emu_err();
    }

    return ret;
}

void wdg_timeout_cb(void)
{
    log_error("wdt out\n");
    secboot_report_error(FW_ERROR_WDG_TIMEOUT);
}

typedef struct {
    uint32_t start_addr;
    uint16_t size_exp;
    uint8_t priv;
} ehsm_pmp_config_item_st;



STATIC_ASSERT(OTP_FW_RANDCLK_1_CLK_8_CYCLES == RANDCLK_MODE_GATE_ONE_CLK_PER_8_CYCLE);
STATIC_ASSERT(OTP_FW_RANDCLK_1_CLK_16_CYCLES == RANDCLK_MODE_GATE_ONE_CLK_PER_16_CYCLE);
STATIC_ASSERT(OTP_FW_RANDCLK_1_CLK_32_CYCLES == RANDCLK_MODE_GATE_ONE_CLK_PER_32_CYCLE);

void secboot_start_randclk(uint8_t type)
{
    uint32_t seed = 0;

    switch (type) {
    case OTP_FW_RANDCLK_1_CLK_8_CYCLES:
    case OTP_FW_RANDCLK_1_CLK_16_CYCLES:
    case OTP_FW_RANDCLK_1_CLK_32_CYCLES:
        (void)cpt_get_rand((uint8_t *)&seed, sizeof(uint32_t));
        randclk_start(seed, type);
        break;
    default:
        // nothing to do
        break;
    }
}

void secboot_sensor_rsp(uint8_t response)
{
    log_error("sensor rsp: 0x%x\n", response);

    if ((response & (1U << SENSOR_RSP_STOP_IP_CORE_INDEX)) != 0) {
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_CLEAR_RAM_K_INDEX)) != 0) {
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_CLEAR_NVM_K_INDEX)) != 0) {
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_CLEAR_OTP_K_INDEX)) != 0) {
        otpdata_delete_all_keys();
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_RESET_EHSM_INDEX)) != 0) {
        sysreg_hsm_reset();
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_RESET_SOC_INDEX)) != 0) {
        sysreg_soc_reset();
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_REV1_INDEX)) != 0) {
    } else {
        ;
    }

    if ((response & (1U << SENSOR_RSP_REV2_INDEX)) != 0) {
    } else {
        ;
    }
}


void secboot_uart_recv_cb(void)
{
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#if FID_LEVEL != FID_LEVEL_NONE
// function for fid-lib
void fid_panic_callback(void)
{
    secboot_report_error(FW_ERROR_FAULT_INJECTION_DETECTED);
}
#if FID_LEVEL == FID_LEVEL_HIGH
void fid_delay_get_random(uint8_t *buf, uint32_t size)
{
    (void)cpt_get_rand(buf, size);
}
#endif
#endif
// handler for CPU exception
void secboot_cpu_excpt_handler(uint32_t cause, uint32_t epc, uint32_t *regs)
{
    UNUSED(regs);
    UNUSED(epc);
    UNUSED(cause);
    log_debug("epc: 0x%08x, cause: %x \n", epc, cause);
    secboot_report_error(FW_ERROR_CPU_EXCEPTION);
}

void secboot_check_hw_version(void)
{
    uint32_t hw_ver = SYS_VER1_REG;

    log_debug("hw ver: %04x, custom: %04x\n", hw_ver & 0xffff, hw_ver >> 16);

    if (
        // custom id must be same
        (hw_ver >> 16) != 0x4019 ||
        ((int8_t)((hw_ver & 0xf000U) >> 12)) != EHSM_BL_HW_VER_MAJOR   // major version must be same
        || ((int8_t)((hw_ver & 0x0f00U) >> 8)) != EHSM_BL_HW_VER_MINOR // minor version must be same
        || ((int8_t)((hw_ver & 0x00f0U) >> 4)) < EHSM_BL_HW_VER_PATCH  // patch version must >= dependence
        || ((int8_t)(((hw_ver & 0x00f0U) >> 4)) == EHSM_BL_HW_VER_PATCH
            && (int8_t)((hw_ver & 0x000fU)) < EHSM_BL_HW_VER_PR) // pre-release version must >= dependence
    ) {
        log_debug("version mismatch\n");
        emu_trigger_fw_error(FW_WARN_VERSION_MISMATCH);
    }
}

uint32_t secboot_get_uart_baudrate_div(void)
{
#define OSR_FPGA_FREQ 5000000U
    if (sysreg_is_osr_fpga()) {
        return OSR_FPGA_FREQ / CONFIG_BL_DEFAULT_UART_BAUDRATE;
    } else {
        return CONFIG_BL_CPU_FREQ_HZ / CONFIG_BL_DEFAULT_UART_BAUDRATE;
    }
#undef OSR_FPGA_FREQ
}

void secboot_init_sensor(void)
{
}

void secboot_load_patch(void)
{
}

void secboot_check_uid(void)
{
    if (sysreg_get_life_cycle() != SYS_STA0_LIFECYCLE_MCUTEST) {
        uint8_t buf[OTP_UID_LEN + 4]; // 4 bytes crc32
        if (otp_read(OTP_UID_ADDRESS, buf, OTP_UID_LEN + 4) != EHSM_ERR_SW_SUCCESS) {
            return;
        }
        uint32_t crc32 = util_crc32(buf, OTP_UID_LEN, 0xFFFFFFFFU);
        if (crc32 != get_uint32(&buf[OTP_UID_LEN])) {
            emu_trigger_fw_error(FW_WARN_UID_CRC_MISMATCH);
        }
    }
}

uint32_t secboot_entry(void)
{
    cpu_platform_init();

    // 前DRAM_OTP_INIT_SIZE个字节用于存放OTP的数据，VERSION COUNTER、自检结果的数据，先进行初始化
    util_memset((void *)DRAM_BASE_ADDR, 0x0, DRAM_OTP_INIT_SIZE);
    // OTP driver必须在任何访问OTP的操作之前初始化
    if (otp_init() != EHSM_ERR_SW_SUCCESS) {
        secboot_report_error(FW_ERROR_INIT_OTP_FAILED);
    }

#if defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE)
    bl_patch_test_run();
#endif /* defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE) */

    uint32_t wdt_level = sysreg_get_wdt_level();
    if (wdt_level > 15U) {
        wdt_level = 15U;
    }
    if (wdt_level > 0) {
        watchdog_init(wdg_timeout_cb);
        watchdog_start(1U << wdt_level);
    }

    if (sysreg_is_log_enable()) {
        uart_init(secboot_uart_recv_cb, secboot_get_uart_baudrate_div());
    }

    log_debug("bl_start\n");
    log_debug("wdt_level: %u\n", wdt_level);

    secboot_check_hw_version();


    // should register otp key map first, patch needs it
    otpkey_register_keyid_map(g_otp_default_keyid_map, sizeof(g_otp_default_keyid_map) / sizeof(otp_key_attributes_st));
    secboot_load_patch();
    secboot_init_sensor();

    emu_init(NULL);

    mailbox_init();
    secboot_start_randclk(sysreg_get_randclk_type());

    secboot_check_uid();
    (void)secboot_do_self_test();
    watchdog_feed();

    sysreg_set_status(0, SYS_STA0_BOOTLOADER_READY);
    log_debug("bl_done\n");
    sch_start();
    while (1) { }
}

void secboot_boot_fw(void)
{

    if (FID_EQ(g_boot_fw_enable, BOOT_FW_ENABLE_FLAG)) {
        watchdog_stop();
        cpu_disable_global_int();
        log_debug("bt fw\n");
        // do not flush uart, disable uart directly
        uart_deinit();
        // recover DMA configuration, see OSR Standard eHSM TRM for default value
        sysreg_ahb_dma_cfg(AXI_READ_AND_WRITE | AHB_DMA_SKE);
        {
        }
        if (g_ehsm_fw_entry != NULL) {
            g_ehsm_fw_entry();
        }
    }
}

void secboot_enable_boot_fw(uint32_t addr)
{
    g_boot_fw_enable = BOOT_FW_ENABLE_FLAG;
    g_ehsm_fw_entry = (ehsm_entry)(addr);
}
