#ifndef COUNTER_DRIVER_H
#define COUNTER_DRIVER_H

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

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
// Counter base register
typedef struct {
    uint32_t ctrl;
    uint32_t rev;
    uint32_t int_pending;
    uint32_t int_en;
    uint32_t cur_val_h;
    uint32_t cur_val_l;
    uint32_t inc_val_h;
    uint32_t inc_val_l;
    uint32_t to_h;
    uint32_t to_l;
} counter_base_reg_st;

// Counter status relative register
typedef struct {
    uint32_t status;
    uint32_t int_pending_bitmap;
} counter_status_reg_st;

typedef enum {
    COUNTER_INC_BY_FW = 0U,
    COUNTER_INC_BY_HW = 1U,
} counter_inc_mode_e;

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
 *   @brief      Set the process mode of counter
 *   
 *   @param [in] idx
 *   @param [in] mode
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t counter_set_mode(uint8_t idx, counter_inc_mode_e mode);


/**
 *   @brief      Get the enable status of counter
 *   
 *   @param [in] idx
 *   
 *   @return     bool_t
 *   
 *   @note
 */
bool_t is_counter_enable(uint8_t idx);


/**
 *   @brief      Enable the counter
 *   
 *   @param [in] idx
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t counter_enable(uint8_t idx);


/**
 *   @brief      Disable the counter
 *   
 *   @param [in] idx
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t counter_disable(uint8_t idx);


/**
 *   @brief      Synchonize the increase value of counter
 *   
 *   @param [in] idx
 *   
 *   
 *   @note
 */
void counter_sync_inc_val(uint8_t idx);


/**
 *   @brief      Get the current value of counter
 *   
 *   @param [in] idx
 *   @param [in] v_h
 *   @param [in] v_l
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t counter_get_current_val(uint8_t idx, uint32_t *v_h, uint32_t *v_l);


/**
 *   @brief      Set the increase value of high 32 bits
 *   
 *   @param [in] idx
 *   @param [in] inc_h
 *   
 *   
 *   @note
 */
void counter_set_inc_h(uint8_t idx, uint32_t inc_h);


/**
 *   @brief      Set the increase value of low 32 bits
 *   
 *   @param [in] idx
 *   @param [in] inc_l
 *   
 *   
 *   @note
 */
void counter_set_inc_l(uint8_t idx, uint32_t inc_l);


/**
 *   @brief      Get the busy status of all counters
 *   
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t counter_get_status(void);

#endif /* COUNTER_DRIVER_H */
