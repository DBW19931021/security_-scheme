/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "otp_data.h"
#include "fw_verify.h"
#include "util.h"
#include "otp_key.h"
#include "sysreg.h"

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

uint32_t otpdata_get_soc_ver_cnt(uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == version) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)version, SOC_VERSION_COUNTER_LEN);
    }

    return ret;
}

uint32_t otpdata_get_hsm_ver_cnt(uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == version) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otp_read(OTP_EHSM_VERSION_ADDRESS, (uint8_t *)version, EHSM_VERSION_COUNTER_LEN);
    }

    return ret;
}

uint32_t otpdata_write_hsm_ver_cnt(uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == version) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otp_write(OTP_EHSM_VERSION_ADDRESS, (uint8_t *)version, EHSM_VERSION_COUNTER_LEN);
    }

    return ret;
}

uint32_t otpdata_write_soc_ver_cnt(uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == version) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otp_write(OTP_SOC_VERSION_ADDRESS, (uint8_t *)version, SOC_VERSION_COUNTER_LEN);
    }

    return ret;
}

uint32_t otpdata_write_lifecycle(uint32_t lifecycle)
{
    return otp_write(OTP_LIFE_CYCLE_ADDRESS, (uint8_t *)&lifecycle, sizeof(lifecycle));
}

uint32_t otpdata_delete_key(uint32_t key_id)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t buffer[OTP_KEY_SIZE];

    if (key_id > KID_KERNEL_MAX) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memset(buffer, 0xFF, sizeof(buffer));
        // set key lifecycle to destroy
        ret = otp_read(OTP_K_ATTR_ADDRESS(key_id), buffer, 4);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            buffer[0] |= 0x0F;
            ret = otp_write(OTP_K_ATTR_ADDRESS(key_id), buffer, OTP_KEY_SIZE);
        }
    }

    return ret;
}

uint32_t otpdata_delete_all_keys(void)
{
    uint8_t buffer[OTP_KEY_SIZE * KID_KERNEL_MAX];
    uint32_t size = OTP_KEY_SIZE * (otpkey_get_max_phyid() + 1);

    if (size > sizeof(buffer)) {
        return EHSM_ERR_PARAM_ERROR;
    }

    util_memset(buffer, 0xFF, sizeof(buffer));
    return otp_write(OTP_K_ATTR_ADDRESS(0), buffer, size);
}

/**
 *
 */
