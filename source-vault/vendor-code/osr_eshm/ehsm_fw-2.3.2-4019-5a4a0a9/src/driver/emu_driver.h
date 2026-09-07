#ifndef EHSM_EMU_DRIVER_H
#define EHSM_EMU_DRIVER_H

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
#define EMU_REG_STATUS_OFFSET      (0U)
#define EMU_REG_INT_PENDING_OFFSET (0x20U)
#define EMU_REG_INT_ENABLE_OFFSET  (0x40U)


#define EMU_REG_ERR_TRIG_LOCK_OFFSET (0x180U)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t sensor;
    uint32_t soc_err;
    uint32_t hw_err_0;
    uint32_t hw_err_1;
    uint32_t fw_err_0;
    uint32_t fw_err_1;
} emu_status_reg_st;

typedef struct {
    uint32_t sensor;
    uint32_t soc_err;
    uint32_t hw_err_0;
    uint32_t hw_err_1;
} emu_int_reg_st;

typedef struct {
    uint32_t sensor;
    uint32_t soc_err;
    uint32_t hw_err_0;
    uint32_t hw_err_1;
} emu_int_en_reg_st;


/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

/**
 *   @brief      EMU initialization function
 *
 *
 *
 *   @note
 */
uint32_t emu_init(void);

/**
 *   @brief      EMU error trigger(excl. CPU exception)
 *
 *   @param [in] err
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t emu_trigger_fw_error(uint32_t err);

/**
 *   @brief      EMU exception handler
 *
 *
 *
 *   @note
 */
ATTR_INTERRUPT void emu_exception_handler(void);

/**
 * @brief enable or disable EMU interrupt.
 *
 * @param enable
 *  - true: enable EMU interrupt
 *  - false: disable EMU interrupt
 */
void emu_enable_hw_int(bool_t enable);

/**
 * @brief unlock or lock EMU ERR_TRIG_LOCK.
 *
 * @param unlock
 *  - true: unlock EMU ERR_TRIG_LOCK
 *  - false: lock EMU ERR_TRIG_LOCK
 */
void emu_unlock_err_trig(bool_t unlock);
#endif
