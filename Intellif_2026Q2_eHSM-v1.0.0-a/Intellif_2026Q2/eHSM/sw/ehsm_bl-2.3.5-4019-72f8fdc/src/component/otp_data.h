#ifndef OTP_DATA_H
#define OTP_DATA_H

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
#define OTP_BASE_ADDRESS          (OTP_BASE_ADDR)
#define OTP_LIFE_CYCLE_ADDRESS    (OTP_BASE_ADDRESS + 0x00U)
#define OTP_UID_ADDRESS           (OTP_BASE_ADDRESS + 0x04U)
#define OTP_HW_CONTROL_FIELD      (OTP_BASE_ADDRESS + 0x18U)
#define OTP_FW_CONTROL_FIELD_EHSM (OTP_BASE_ADDRESS + 0x20U)
#define OTP_FW_CONTROL_FIELD_SOC  (OTP_BASE_ADDRESS + 0x28U)
#define OTP_EHSM_VERSION_ADDRESS  (OTP_BASE_ADDRESS + 0x30U)
#define OTP_SOC_VERSION_ADDRESS   (OTP_BASE_ADDRESS + 0x40U)
#define OTP_K_ATTR_ADDRESS(a)     (OTP_BASE_ADDRESS + 0x118U + ((a) * 10U * 4U))
#define OTP_K_ADDRESS(a)          (OTP_BASE_ADDRESS + 0x11CU + ((a) * 10U * 4U))
#define OTP_K_CRC(a)              (OTP_BASE_ADDRESS + 0x13CU + ((a) * 10U * 4U))

#define OTP_VERSION_LENGTH (16U)
#define OTP_KEY_SIZE       40U

#define CONFIG_EHSM_HW_LIFE_CYCLE_TEST_MODE        0x00000000
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEVELOP_MODE     0x42818414
#define CONFIG_EHSM_HW_LIFE_CYCLE_MANUFACTURE_MODE 0x46C1A416
#define CONFIG_EHSM_HW_LIFE_CYCLE_USER_MODE        0x57C1EC96
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEBUG_MODE       0xD7C5FCDE
#define CONFIG_EHSM_HW_LIFE_CYCLE_DESTROY_MODE     0xFFFFFFFF
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

uint32_t otpdata_get_soc_ver_cnt(uint8_t *version);

uint32_t otpdata_get_hsm_ver_cnt(uint8_t *version);

uint32_t otpdata_write_hsm_ver_cnt(uint8_t *version);

uint32_t otpdata_write_soc_ver_cnt(uint8_t *version);

uint32_t otpdata_write_lifecycle(uint32_t lifecycle);

uint32_t otpdata_delete_key(uint32_t key_id);

uint32_t otpdata_delete_all_keys(void);

#endif
