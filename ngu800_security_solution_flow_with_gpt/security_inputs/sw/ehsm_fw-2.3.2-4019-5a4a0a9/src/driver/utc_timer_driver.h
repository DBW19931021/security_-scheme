#ifndef UTC_TIMER_DRIVER_H
#define UTC_TIMER_DRIVER_H

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
// UTC timer control registers
typedef struct {
    uint32_t ctrl;
    uint32_t status;
    uint32_t sec_div;
} utc_timer_ctrl_reg_st;

// UTC timer value registers
typedef struct{
    uint32_t init_sec;
    uint32_t init_cnt;
    uint32_t current_sec;
    uint32_t current_cnt;
} utc_timer_val_reg_st;

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
 *   @brief      Get the frequency of UTC timer
 *   
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_get_div_freq(void);


/**
 *   @brief      Set the frequency of UTC timer
 *   
 *   @param [in] freq
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_set_div_freq(uint32_t freq);


/**
 *   @brief      Get the enable status of UTC timer
 *   
 *   
 *   @return     bool_t
 *   
 *   @note
 */
bool_t is_utc_timer_enable(void);


/**
 *   @brief      Enable the UTC timer
 *   
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_enable(void);


/**
 *   @brief      Disable the UTC timer
 *   
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_disable(void);


/**
 *   @brief      Get the synchronized status of UTC timer
 *   
 *   
 *   @return     bool_t
 *   
 *   @note
 */
bool_t is_utc_timer_sync(void);


/**
 *   @brief      Re-sync the UTC timer
 *   
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_resync(void);


/**
 *   @brief      Get the currence value of UTC timer
 *   
 *   @param [in] sec
 *   @param [in] cnt
 *   
 *   @return     uint32_t
 *   
 *   @note
 */
uint32_t utc_timer_get_cur_val(uint32_t *sec, uint32_t *cnt);


/**
 *   @brief      Set the initial value of second
 *   
 *   @param [in] sec
 *   
 *   
 *   @note
 */
void utc_timer_set_init_val_sec(uint32_t sec);


/**
 *   @brief      Set the inital value of count
 *   
 *   @param [in] cnt
 *   
 *   
 *   @note
 */
void utc_timer_set_init_val_cnt(uint32_t cnt);

#endif /* UTC_TIMER_DRIVER_H */
