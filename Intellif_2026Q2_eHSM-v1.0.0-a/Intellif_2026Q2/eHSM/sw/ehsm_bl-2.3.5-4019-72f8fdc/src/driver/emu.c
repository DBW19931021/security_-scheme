/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "emu.h"
#include "cpu_porting.h"
#include "sysreg.h"


/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void emu_init(emu_err_handler handle)
{
    UNUSED(handle);
}


uint32_t emu_trigger_fw_error(uint32_t err)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (err < 32) {
        uint32_t level = cpu_enter_critical();
        EMU_FW_0_REG |= (1U << err);
        cpu_exit_critical(level);
    } else if (err < 64) {
        uint32_t level = cpu_enter_critical();
        EMU_FW_1_REG |= (1U << (err - 32U));
        cpu_exit_critical(level);
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}
/**
 *
 */
