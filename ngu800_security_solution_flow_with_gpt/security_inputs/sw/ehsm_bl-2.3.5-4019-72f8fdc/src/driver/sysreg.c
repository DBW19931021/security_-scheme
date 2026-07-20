/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "sysreg.h"
#include "cpu_porting.h"
#include <ske/ske.h>

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

void sysreg_get_status(uint32_t index, uint32_t *status)
{
    if ((index <= 3U) && (status != NULL)) {
        *status = *((volatile uint32_t *)(SYS_STA_REG_BASE + index * 4U));
    }
}

void sysreg_set_status(uint32_t index, uint32_t status)
{
    uint32_t write_flag_arr[] = { SYS_STA0_WRITE_FLAG, SYS_STA1_WRITE_FLAG, SYS_STA2_WRITE_FLAG, SYS_STA3_WRITE_FLAG };
    uint32_t write_flag;
    uint32_t sta;

    if (index <= 3) {
        write_flag = write_flag_arr[index];
        sta = *((volatile uint32_t *)(SYS_STA_REG_BASE + index * 4U)) | write_flag;
        *((volatile uint32_t *)(SYS_STA_REG_BASE + index * 4U)) = (sta | status);
    }
}

void sysreg_enable_hsm_dbg(void)
{
    SYS_HSM_DBG_EN_REG = SYS_HSM_DBG_EN_VALUE;
}

void sysreg_disable_hsm_dbg(void)
{
    SYS_HSM_DBG_EN_REG = 0U;
}

void sysreg_enable_soc_dbg(void)
{
    SYS_SOC_DBG_EN_REG = SYS_SOC_DBG_EN_VALUE;
}

void sysreg_disable_soc_dbg(void)
{
    SYS_SOC_DBG_EN_REG = 0U;
}

void sysreg_enable_soc_dbg_ext(uint8_t zone, uint32_t value)
{
    if (zone <= 3U) {
        uint32_t base_addr = SYS_SOC_DBG_EN_BASE_REG;

        base_addr = base_addr + ((uint32_t)zone << 2U);

        uint32_t level = cpu_enter_critical();
        *((volatile uint32_t *)(base_addr)) |= value;
        cpu_exit_critical(level);
    }
}

void sysreg_disable_soc_dbg_ext(uint8_t zone, uint32_t value)
{
    if (zone <= 3U) {
        uint32_t base_addr = SYS_SOC_DBG_EN_BASE_REG;

        base_addr = base_addr + ((uint32_t)zone << 2U);

        uint32_t level = cpu_enter_critical();
        *((volatile uint32_t *)(base_addr)) &= (~value);
        cpu_exit_critical(level);
    }
}

void sysreg_enable_bus_err(void)
{
    SYS_BUS_ERR_CFG |= SYS_BUS_SOC_ERR_EN;
    SYS_BUS_ERR_CFG |= SYS_BUS_OTP_ERR_EN;
}

uint32_t sysreg_get_life_cycle(void)
{
    return SYS_STA0_REG & SYS_STA0_LIFECYCLE_MASK;
}

cpt_ske_alg_e sysreg_get_otpkey_dec_alg(void)
{
    cpt_ske_alg_e alg;

    if (((SYS_HW_CFG_REG1 & OTP_HW_KEY_DEC_ALG_MASK) >> OTP_HW_KEY_DEC_ALG_OFFSET) == OTP_HW_KEY_DEC_ALG_AES128) {
        alg = SKE_ALG_AES_128;
    } else {
        alg = SKE_ALG_SM4;
    }

    return alg;
}

void sysreg_ahb_dma_cfg(uint32_t cfg)
{
    SYS_AHB_DMA_CFG = cfg;
}

uint8_t sysreg_get_otp_key_num(void)
{
    return SYS_HW_INF5 & SYS_HW_INF5_OTP_END_MASK;
}

void sysreg_hsm_reset(void)
{
    uint32_t rst_ctrl = SYS_RST_CTRL0;

    // Clear the lower 8 bits
    rst_ctrl &= (~0xFFU);
    rst_ctrl |= 0x5AU;
    rst_ctrl &= SYS_RST_CTRL0_HSM_RESET;
    SYS_RST_CTRL0 = rst_ctrl;
}

void sysreg_soc_reset(void)
{
    // create a rising edge
    SYS_WR_SOC_RESET = SYS_WR_SOC_RESET_CLEAR_VALUE;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    SYS_WR_SOC_RESET = SYS_WR_SOC_RESET_SET_VALUE;
}

/**
 *
 */
