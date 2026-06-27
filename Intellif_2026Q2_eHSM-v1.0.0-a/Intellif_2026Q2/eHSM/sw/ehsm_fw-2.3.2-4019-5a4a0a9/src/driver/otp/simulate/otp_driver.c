/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "../otp_driver.h"
#include "component/util.h"
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
uint32_t otp_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t otp_read(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t ret;

    if ((data == NULL) || (size == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < CONFIG_EHSM_OTP_BASE_ADDR) || (size > CONFIG_EHSM_OTP_SIZE)
        || ((addr - CONFIG_EHSM_OTP_BASE_ADDR) > (CONFIG_EHSM_OTP_SIZE - size))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        (void)util_memcpy(data, (uint8_t *)addr, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t otp_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    uint32_t ret;

    if ((data == NULL) || (size == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < CONFIG_EHSM_OTP_BASE_ADDR) || (size > CONFIG_EHSM_OTP_SIZE)
        || ((addr - CONFIG_EHSM_OTP_BASE_ADDR) > (CONFIG_EHSM_OTP_SIZE - size))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((addr % CONFIG_EHSM_OTP_WRITE_UNIT) != 0U) || ((size % CONFIG_EHSM_OTP_WRITE_UNIT) != 0U)) {
        ret = EHSM_ERR_NOT_ALIGNED;
    } else {
        (void)util_memcpy((uint8_t *)addr, data, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t otp_verify(uint32_t addr, uint32_t size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t i = 0;
    uint8_t blank[CONFIG_EHSM_OTP_WRITE_UNIT];

    if (size == 0U) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((addr < CONFIG_EHSM_OTP_BASE_ADDR) || (size > CONFIG_EHSM_OTP_SIZE)
        || ((addr - CONFIG_EHSM_OTP_BASE_ADDR) > (CONFIG_EHSM_OTP_SIZE - size))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((addr % CONFIG_EHSM_OTP_WRITE_UNIT) != 0U) || ((size % CONFIG_EHSM_OTP_WRITE_UNIT) != 0U)) {
        ret = EHSM_ERR_NOT_ALIGNED;
    } else {
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0
        uint8_t init_value = 0;
#else
        uint8_t init_value = 0xff;
#endif
        (void)util_memset(blank, init_value, CONFIG_EHSM_OTP_WRITE_UNIT);

        for (i = 0; i < size; i += CONFIG_EHSM_OTP_WRITE_UNIT) {
            if (util_memcmp(blank, (uint8_t *)(addr + i), CONFIG_EHSM_OTP_WRITE_UNIT) != 0U) {
                ret = EHSM_ERR_DATA_NOT_EMPTY;
                break;
            } else {
                // nothing to do
            }
        }
    }

    return ret;
}
/**
 * @}
 */
