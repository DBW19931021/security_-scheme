/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "emu_driver.h"
#include "reg_lock.h"
#include "cpu_porting.h"
#include "sysreg.h"
#include "types.h"
#include "schedule/expt_det.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#ifdef CONFIG_UNIT_TEST
uint32_t g_test_sys_reg[2];
#define SYS_STA0_REG    g_test_sys_reg[0]
#define SYS_SM_MASK_REG g_test_sys_reg[1]
#endif
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

/**
 *   @brief      Get the EMU interrupt enable register address
 *
 *
 *   @return     volatile*
 *
 *   @note
 */
static inline volatile emu_int_en_reg_st *emu_get_int_en_reg_base(void)
{
    return (volatile emu_int_en_reg_st *)(EMU_REG_BASE + EMU_REG_INT_ENABLE_OFFSET);
}

/**
 *   @brief      Get the EMU interrupt pendig register address
 *
 *
 *   @return     volatile*
 *
 *   @note
 */
static inline volatile emu_int_reg_st *emu_get_int_reg_base(void)
{
    return (volatile emu_int_reg_st *)(EMU_REG_BASE + EMU_REG_INT_PENDING_OFFSET);
}


/**
 *   @brief      Get the EMU status register address
 *
 *
 *   @return     volatile*
 *
 *   @note
 */
static inline volatile emu_status_reg_st *emu_get_status_reg_base(void)
{
    return (volatile emu_status_reg_st *)(EMU_REG_BASE + EMU_REG_STATUS_OFFSET);
}

static inline volatile uint32_t *emu_get_err_trig_lock_reg_base(void)
{
    return (volatile uint32_t *)(EMU_REG_BASE + EMU_REG_ERR_TRIG_LOCK_OFFSET);
}

static void enter_hold_status(void)
{
#ifndef CONFIG_UNIT_TEST
    while (1) {
        asm volatile("wfi");
    }
#endif
}

static uint32_t get_mcause_value(void)
{
    uint32_t mcause = 0U;
#ifndef CONFIG_UNIT_TEST
    asm volatile("csrr %0, mcause" : "=r"(mcause));
#endif
    return mcause;
}

/**
 *   @brief      EMU ISR
 */
static ATTR_INTERRUPT void emu_int_handler(void)
{
    // Trap and wait for the reset
    enter_hold_status();
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
ATTR_INTERRUPT void emu_exception_handler(void)
{
    uint32_t mcause = get_mcause_value();
    // Move the low 4 bit to MSB and mask the low 28 bit
    sysreg_unlock_reg(EMU_REG_BASE);
    volatile emu_status_reg_st *emu_status = emu_get_status_reg_base();
    uint32_t emu_fw_0_val = emu_status->fw_err_0;
    // Clear the 28 - 31 bits
    emu_fw_0_val &= 0x0FFFFFFFU;
    // Set mcause to 28 - 31 bits in fw_err_0
    emu_status->fw_err_0 = (emu_fw_0_val | (mcause << 28));
    sysreg_lock_reg(EMU_REG_BASE);

    if (emu_status->fw_err_0 != (emu_fw_0_val | (mcause << 28))) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
    // Trap and wait for the reset
    enter_hold_status();
}

uint32_t emu_init(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    volatile emu_int_en_reg_st *emu_int_en_reg = emu_get_int_en_reg_base();
    volatile emu_int_reg_st *emu_int_reg = emu_get_int_reg_base();
    // unmask ecc alarm
    SYS_SM_MASK_REG |= (SYS_SM_MASK_ECC_UMS | SYS_SM_MASK_ECC_1B_UMS);

    // The safty mechanism has been eanbed in the init function of the boot module.

    sysreg_unlock_reg(EMU_REG_BASE);
    // Clear all the pandding
    emu_int_reg->hw_err_0 = 0xFFFFFFFFU;
    emu_int_reg->hw_err_1 = 0xFFFFFFFFU;
    emu_int_reg->sensor = 0xFFFFFFFFU;
    emu_int_reg->soc_err = 0xFFFFFFFFU;
    // Enable all the interrupt

    // The lsb 8 bits is mem_ecc_1b_xxx error, which will be recovered by HW.
    // So we ignore these errors.
    emu_int_en_reg->hw_err_0 = 0xFFFFFF00U;
    // bit 32 ~ 34 of TRNG warnings are ignored
    emu_int_en_reg->hw_err_1 = 0xFFFFFFF8U;
    emu_int_en_reg->sensor = 0x0;
    emu_int_en_reg->soc_err = 0x0;

    if ((emu_int_en_reg->hw_err_0 != 0xFFFFFF00U) || (emu_int_en_reg->hw_err_1 != 0xFFFFFFF8U)
        || (emu_int_en_reg->sensor != 0x0) || (emu_int_en_reg->soc_err != 0x0)) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }


    sysreg_lock_reg(EMU_REG_BASE);
    ret = cpu_register_int(IRQ_TYPE_EMU, INT_LEVEL_TRIGGER, (cpu_int_handler)emu_int_handler);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpu_enable_int(IRQ_TYPE_EMU);
    }

    return ret;
}

uint32_t emu_trigger_fw_error(uint32_t err)
{
    uint32_t set_value = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    volatile emu_status_reg_st *emu_status_reg = emu_get_status_reg_base();
    if (err < 28U) {
        set_value = ((uint32_t)0x1U << err);
    } else if ((err >= 32U) && (err < 64U)) {
        set_value = ((uint32_t)0x1U << (err - 32U));
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        sysreg_unlock_reg(EMU_REG_BASE);
        if (err < 28U) {
            emu_status_reg->fw_err_0 = set_value;
        } else {
            emu_status_reg->fw_err_1 = set_value;
        }
        sysreg_lock_reg(EMU_REG_BASE);
    }

    return ret;
}

void emu_enable_hw_int(bool_t enable)
{
    volatile emu_int_en_reg_st *emu_int_en_reg = emu_get_int_en_reg_base();
    volatile emu_int_reg_st *emu_int_reg = emu_get_int_reg_base();
    // unlock
    sysreg_unlock_reg(EMU_REG_BASE);

    // Clear the interrupt if have
    emu_int_reg->hw_err_0 = 0xFFFFFFFFU;
    emu_int_reg->hw_err_1 = 0xFFFFFFFFU;
    emu_int_reg->sensor = 0xFFFFFFFFU;
    emu_int_reg->soc_err = 0xFFFFFFFFU;
    if (enable) {

        emu_int_en_reg->hw_err_0 = 0xFFFFFF00U;
        // bit 32 ~ 34 of TRNG warnings are ignored
        emu_int_en_reg->hw_err_1 = 0xFFFFFFF8U;
        emu_int_en_reg->sensor = 0xFFFFFFFFU;
        emu_int_en_reg->soc_err = 0xFFFFFFFFU;
    } else {
        emu_int_en_reg->hw_err_0 = 0x0U;
        emu_int_en_reg->hw_err_1 = 0x0U;
        emu_int_en_reg->sensor = 0x0U;
        emu_int_en_reg->soc_err = 0x0U;
    }
    // lock
    sysreg_lock_reg(EMU_REG_BASE);

    if (enable) {
        if ((emu_int_en_reg->hw_err_0 != 0xFFFFFF00U) || (emu_int_en_reg->hw_err_1 != 0xFFFFFFF8U)
            || (emu_int_en_reg->sensor != 0xFFFFFFFFU) || (emu_int_en_reg->soc_err != 0xFFFFFFFFU)) {
            (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
        }
    } else {
        if ((emu_int_en_reg->hw_err_0 != 0x0U) || (emu_int_en_reg->hw_err_1 != 0x0U) || (emu_int_en_reg->sensor != 0x0U)
            || (emu_int_en_reg->soc_err != 0x0U)) {
            (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
        }
    }
}

void emu_unlock_err_trig(bool_t unlock)
{
    uint32_t value = 0;
    volatile uint32_t *err_trig_lock = emu_get_err_trig_lock_reg_base();
    if (unlock) {
        value = EMU_ERR_TRIG_UNLOCK_VALUE;
    } else {
        value = 0;
    }

    sysreg_unlock_reg(EMU_REG_BASE);
    *err_trig_lock = value;
    sysreg_lock_reg(EMU_REG_BASE);

    if (*err_trig_lock != value) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}

/**
 *
 */
