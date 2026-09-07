#ifndef REG_LOCK_H
#define REG_LOCK_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "schedule/expt_det.h"
#include "driver/sysreg.h"
#include "driver/mailbox_driver.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
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
/*
 * The Fusa lock mechanism usage:
 *     1. Enable the function lock
 *     2. Enable the safty mechanism lock
 *     3. Umask the lock relative signal for output
 * The operation above should be put into the init function for every module.
 * Unlock the function lock before writting data to register and it should be locked after your writing operation.
 */
static inline void sysreg_enable_fun_lock(uint32_t reg_base)
{
    UNUSED(reg_base);
}

static inline void sysreg_enable_sm_lock(uint32_t reg_base)
{
    UNUSED(reg_base);
}

static inline void sysreg_umask_sm_lock(uint32_t reg_base)
{
    UNUSED(reg_base);
}

static inline void sysreg_config_sm_en(uint32_t reg_base)
{
    UNUSED(reg_base);
}

static inline void sysreg_unlock_reg(uint32_t reg_base)
{
    UNUSED(reg_base);
}

static inline void sysreg_lock_reg(uint32_t reg_base)
{
    UNUSED(reg_base);
}
#endif /* REG_LOCK_H */
