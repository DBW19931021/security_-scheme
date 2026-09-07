#ifndef KMU_H
#define KMU_H

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
#define KMU_KID_INSTALL_KEK_EHSM (0xFFU)
#define KMU_KID_INSTALL_KEK_SOC  (0xFEU)

#define KMU_TRANSP_KEY_TO_SKE    (0x00U)
#define KMU_TRANSP_KEY_TO_HASH   (0x01U)
#define KMU_TRANSP_KEY_TO_CHACHA (0x02U)
#define KMU_TRANSP_KEY_TO_PKE    (0x03U)

#define OTP_KEY_ALGO_SKE_TYPE  (0x0EU)
#define OTP_KEY_ALGO_PKE_TYPE  (0x0DU)
#define OTP_KEY_ALGO_HASH_TYPE (0x0BU)

#define OTP_KEY_LEVEL_0 (0x00U)
#define OTP_KEY_LEVEL_1 (0x0AU)
#define OTP_KEY_LEVEL_2 (0x05U)

#define OTP_KEY_LIFECYCLE_ENABLE  (0x0CU)
#define OTP_KEY_LIFECYCLE_DISABLE (0x08U)
#define OTP_KEY_LIFECYCLE_DESTROY (0x00U)

#define OTP_KEY_ATTR_NO_CHECK (0xFFU)

#define OTP_K_ATTR_LIFECYCLE_MASK   (0x0000000FU)
#define OTP_K_ATTR_LEVEL_MASK       (0x000000F0U)
#define OTP_K_ATTR_ALGO_MASK        (0x00000F00U)
#define OTP_K_ATTR_LIFECYCLE_OFFSET (0U)
#define OTP_K_ATTR_LEVEL_OFFSET     (4U)
#define OTP_K_ATTR_ALGO_OFFSET      (8U)

#define KMU_TOTAL_KEY_NUM (0x19U)

#define KMU_BASE_ADDR (0x30800000U)
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
uint32_t kmu_config(uint16_t key_id, uint8_t sec_port_sel);
uint32_t kmu_config_key_attr(uint16_t key_id, uint8_t algo_type, uint8_t key_level);
uint32_t kmu_check_key_attr(uint16_t key_id, uint8_t algo_type, uint8_t key_level);
uint32_t kmu_get_keyid_data(uint16_t key_id, uint8_t *data, uint32_t size);
void kmu_key_enable(uint32_t key_mask);
#endif
