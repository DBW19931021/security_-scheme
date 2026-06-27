/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "sysreg.h"
#include <ske/ske.h>
#include "reg_lock.h"
#include "schedule/expt_det.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

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
#ifdef CONFIG_UNIT_TEST
static uint32_t g_sys_soc_cfg_reg[2];
#define SYS_SOC_CFG_REG0 *((volatile uint32_t *)(&g_sys_soc_cfg_reg[0]))
#define SYS_SOC_CFG_REG1 *((volatile uint32_t *)(&g_sys_soc_cfg_reg[1]))

static uint32_t g_sys_fw_cfg_reg[2];
#define SYS_FW_CFG_REG0 *((volatile uint32_t *)(&g_sys_fw_cfg_reg[0]))
#define SYS_FW_CFG_REG1 *((volatile uint32_t *)(&g_sys_fw_cfg_reg[1]))

static uint32_t g_sys_hsm_dbg_en;
#define SYS_HSM_DBG_EN_REG *((volatile uint32_t *)(&g_sys_hsm_dbg_en))

static uint32_t g_sys_soc_dbg_en;
#define SYS_SOC_DBG_EN_REG *((volatile uint32_t *)(&g_sys_soc_dbg_en))

static uint32_t g_sys_soc_dbg_en;
#define SYS_SOC_DBG_EN_REG *((volatile uint32_t *)(&g_sys_soc_dbg_en))

static uint32_t g_sys_soc_dbg_en_base[4];

static uint32_t g_sys_sta_reg[4];
#define SYS_STA0_REG *((volatile uint32_t *)(&g_sys_sta_reg[0]))
#define SYS_STA1_REG *((volatile uint32_t *)(&g_sys_sta_reg[1]))
#define SYS_STA2_REG *((volatile uint32_t *)(&g_sys_sta_reg[2]))
#define SYS_STA3_REG *((volatile uint32_t *)(&g_sys_sta_reg[3]))

static uint32_t g_dma_cfg_reg;
#define SYS_AHB_DMA_CFG *((volatile uint32_t *)(&g_dma_cfg_reg))

static uint32_t g_hw_cfg_reg;
#define SYS_HW_CONTROL1_REG *((volatile uint32_t *)(&g_hw_cfg_reg))

static uint32_t g_hw_inf5;
#define SYS_HW_INF5 *((volatile uint32_t *)(&g_hw_inf5))

static uint32_t g_sys_gen_reg[16];
#endif
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
uint32_t sysreg_get_soc_boot_type(void)
{
    uint32_t boot_value;
    uint32_t boot_type;

    boot_value = (SYS_SOC_CFG_REG0 & SOC_BOOT_TYPE_MASK) >> SOC_BOOT_TYPE_OFFSET;
    if ((boot_value == SOC_BOOT_PARALLEL_VALUE1) || (boot_value == SOC_BOOT_PARALLEL_VALUE2)) {
        boot_type = SOC_BOOT_TYPE_PARALLEL;
    } else {
        boot_type = SOC_BOOT_TYPE_SEQUENTIAL;
    }

    return boot_type;
}

uint8_t sysreg_get_soc_verify_alg(void)
{
    return (uint8_t)((SYS_SOC_CFG_REG0 & OTP_SOC_VERIFY_ALG_MASK) >> OTP_SOC_VERIFY_ALG_OFFSET);
}

uint8_t sysreg_get_soc_upd_alg(void)
{
    return (uint8_t)((SYS_SOC_CFG_REG0 & OTP_SOC_UPGRADE_ALG_MASK) >> OTP_SOC_UPGRADE_ALG_OFFSET);
}

uint8_t sysreg_get_fw_upd_alg(void)
{
    return (uint8_t)((SYS_FW_CFG_REG1 & OTP_FW_UPGRADE_ALG_MASK) >> OTP_FW_UPGRADE_ALG_OFFSET);
}

uint8_t sysreg_get_fw_verify_alg(void)
{
    return (uint8_t)((SYS_FW_CFG_REG1 & OTP_FW_VERIFY_ALG_MASK) >> OTP_FW_VERIFY_ALG_OFFSET);
}

void sysreg_get_status(uint32_t index, uint32_t *status)
{
#ifdef CONFIG_UNIT_TEST
    *status = g_sys_sta_reg[index];
#else
    *status = *((volatile uint32_t *)(SYS_STA_REG_BASE + (index * 4U)));
#endif
}

void sysreg_set_status(uint32_t index, uint32_t status)
{
    uint32_t orig_value;

    if (index < 4U) {
#ifdef CONFIG_UNIT_TEST
        orig_value = g_sys_sta_reg[index];
#else
        orig_value = *((volatile uint32_t *)(SYS_STA_REG_BASE + (index * 4U)));
#endif
        /* SYS_STA register write-enable key, must be OR'd before writing */
        orig_value |= SYS_STA_WRITE_ENABLE_KEY;
        orig_value |= status;
#ifdef CONFIG_UNIT_TEST
        g_sys_sta_reg[index] = orig_value;
#else
        sysreg_unlock_reg(SYS_REG_BASE);
        *((volatile uint32_t *)(SYS_STA_REG_BASE + (index * 4U))) = orig_value;
        sysreg_lock_reg(SYS_REG_BASE);
        if ((*((volatile uint32_t *)(SYS_STA_REG_BASE + (index * 4U))) & status) != status) {
            (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
        }
#endif
    }
}

void sysreg_enable_hsm_dbg(void)
{
    sysreg_unlock_reg(SYS_REG_BASE);
    SYS_HSM_DBG_EN_REG = SYS_HSM_DBG_EN_VALUE;
    sysreg_lock_reg(SYS_REG_BASE);
}

void sysreg_disable_hsm_dbg(void)
{
    sysreg_unlock_reg(SYS_REG_BASE);
    SYS_HSM_DBG_EN_REG = 0U;
    sysreg_lock_reg(SYS_REG_BASE);
}

void sysreg_enable_soc_dbg(void)
{
    sysreg_unlock_reg(SYS_REG_BASE);
    SYS_SOC_DBG_EN_REG = SYS_SOC_DBG_EN_VALUE;
    sysreg_lock_reg(SYS_REG_BASE);
}

void sysreg_disable_soc_dbg(void)
{
    sysreg_unlock_reg(SYS_REG_BASE);
    SYS_SOC_DBG_EN_REG = 0U;
    sysreg_lock_reg(SYS_REG_BASE);
}

void sysreg_enable_soc_dbg_ext(uint8_t zone, uint32_t value)
{
#ifdef CONFIG_UNIT_TEST
    g_sys_soc_dbg_en_base[zone] |= value;
#else
    uint32_t base_addr = SYS_SOC_DBG_EN_BASE_REG;

    base_addr = base_addr + ((uint32_t)zone << 2U);

    sysreg_unlock_reg(SYS_REG_BASE);
    *((volatile uint32_t *)(base_addr)) |= value;
    sysreg_lock_reg(SYS_REG_BASE);
#endif
}

void sysreg_disable_soc_dbg_ext(uint8_t zone, uint32_t value)
{
#ifdef CONFIG_UNIT_TEST
    g_sys_soc_dbg_en_base[zone] &= (~value);
#else
    uint32_t base_addr = SYS_SOC_DBG_EN_BASE_REG;

    base_addr = base_addr + ((uint32_t)zone << 2U);
    sysreg_unlock_reg(SYS_REG_BASE);
    *((volatile uint32_t *)(base_addr)) &= (~value);
    sysreg_lock_reg(SYS_REG_BASE);
#endif
}

uint32_t sysreg_get_life_cycle(void)
{
    return SYS_STA0_REG & SYS_STA0_LIFECYCLE_MASK;
}

uint32_t sysreg_get_otpkey_dec_alg(void)
{
    return SYS_HW_CONTROL1_REG & OTP_CTRL0_K_ALG_SEL_MASK;
}

void sysreg_ahb_dma_cfg(uint32_t cfg)
{
    sysreg_unlock_reg(SYS_REG_BASE);
    SYS_AHB_DMA_CFG = cfg;
    sysreg_lock_reg(SYS_REG_BASE);
    if (SYS_AHB_DMA_CFG != cfg) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}

uint8_t sysreg_get_otp_key_num(void)
{
    return (uint8_t)(SYS_HW_INF5 & SYS_HW_INF5_OTP_END_MASK);
}

void sysreg_set_gen_reg(uint8_t zone, uint32_t value)
{
#ifdef CONFIG_UNIT_TEST
    g_sys_gen_reg[zone] = value;
#else
    uint32_t base_addr = SYS_GEN_REG_BASE;

    base_addr = base_addr + ((uint32_t)zone << 2U);
    sysreg_unlock_reg(SYS_REG_BASE);
    *((volatile uint32_t *)(base_addr)) = value;
    sysreg_lock_reg(SYS_REG_BASE);
    if (*((volatile uint32_t *)(base_addr)) != value) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
#endif
}
/**
 *
 */
