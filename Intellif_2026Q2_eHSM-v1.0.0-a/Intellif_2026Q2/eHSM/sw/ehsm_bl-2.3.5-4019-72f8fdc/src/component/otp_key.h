#ifndef SRV_OTP_KEY_H_
#define SRV_OTP_KEY_H_

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "kms.h"
#include "otp.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*Definition the key algorithm type */
#define OTP_KEY_ALGO_SKE_TYPE  (0x0EU)
#define OTP_KEY_ALGO_PKE_TYPE  (0x0DU)
#define OTP_KEY_ALGO_HASH_TYPE (0x0BU)
#define OTP_KEY_ALGO_NO_CHECK  (0xFFU)

/*Definition the key level type */
#define OTP_KEY_LEVEL_0        (0x00U)
#define OTP_KEY_LEVEL_1        (0x0AU)
#define OTP_KEY_LEVEL_2        (0x05U)
#define OTP_KEY_LEVEL_NO_CHECK (0xFFU)

/*Definition the key lifecycle type */
#define OTP_KEY_LIFECYCLE_ENABLE  (0x0CU)
#define OTP_KEY_LIFECYCLE_DISABLE (0x08U)
#define OTP_KEY_LIFECYCLE_DESTROY (0x00U)

#define OTP_KEY_DATA_SIZE (0x20U)

#define KID_USAGE_VALID_K (0xFFFFFFFFU) //

#define K_USAGE_SKE     (0x0EU)
#define K_USAGE_PKE     (0x0DU)
#define K_USAGE_HASH    (0x0BU)
#define K_USAGE_INVALID (0xFFU)

#define K_LEVEL_1        (0x0AU)
#define K_LEVEL_2        (0x05U)
#define K_LEVEL_NO_CHECK (0xFFU)

#define OTP_K_VALUE_SIZE   (32U)
#define OTP_K_CRC_SIZE     (4U)
#define OTP_K_SIZE         (OTP_K_VALUE_SIZE + OTP_K_CRC_SIZE)
#define OTP_INSTALL_K_SIZE (48U)

#define EHSM_OTP_CHIP_ROOT_KEY_ID        (KMS_KEY_TYPE_OTP + 1U)
#define EHSM_OTP_DEVICE_ROOT_KEY_ID      (KMS_KEY_TYPE_OTP + 2U)
#define EHSM_OTP_USER_ROOT_KEY_ID        (KMS_KEY_TYPE_OTP + 3U)
#define EHSM_OTP_EHSM_DEBUG_KEY_ID       (KMS_KEY_TYPE_OTP + 4U)
#define EHSM_OTP_EHSM_FW_VERIFY_KEY_ID   (KMS_KEY_TYPE_OTP + 5U)
#define EHSM_OTP_EHSM_ENCRYPT_KEY_ID     (KMS_KEY_TYPE_OTP + 6U)
#define EHSM_OTP_EHSM_UPG_ENCRYPT_KEY_ID (KMS_KEY_TYPE_OTP + 7U)
#define EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID  (KMS_KEY_TYPE_OTP + 8U)
#define EHSM_OTP_EHSM_PRIVATE_KEY_ID     (KMS_KEY_TYPE_OTP + 9U)
#define EHSM_OTP_SOC_DEBUG_KEY_ID        (KMS_KEY_TYPE_OTP + 10U)
#define EHSM_OTP_SOC_FW_VERIFY_KEY_ID    (KMS_KEY_TYPE_OTP + 11U)
#define EHSM_OTP_SOC_ENCRYPT_KEY_ID      (KMS_KEY_TYPE_OTP + 12U)
#define EHSM_OTP_SOC_UPG_ENCRYPT_KEY_ID  (KMS_KEY_TYPE_OTP + 13U)
#define EHSM_OTP_SOC_UPG_VERIFY_KEY_ID   (KMS_KEY_TYPE_OTP + 14U)
#define EHSM_OTP_SOC_PRIVATE_KEY_ID      (KMS_KEY_TYPE_OTP + 15U)
#define EHSM_OTP_SECRET_KEY_KEY_ID       (KMS_KEY_TYPE_OTP + 16U)
#define EHSM_OTP_USER_AUTH_KEY_ID        (KMS_KEY_TYPE_OTP + 17U)
#define EHSM_OTP_INVALID_KEY_ID          (KMS_KEY_TYPE_OTP + 0xFFFFFU)

#define KID_USAGE_UNUSED (0xFFFF)
#define KID_KERNEL_MAX   (EHSM_OTP_USER_AUTH_KEY_ID - KMS_KEY_TYPE_OTP)

#define OTP_KEY_MAX_SIZE (32U)

#define OTP_KEY_ADDR       (OTP_BASE_ADDR + 0x118U)
#define OTP_KEY_TOTAL_SIZE (KID_KERNEL_MAX * 40U)

#define OTP_KEY_USER_KEY_NUM 3
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*Definition the key generate type */
typedef enum { OTP_KEY_GENERATE_TYPE_DIRECT = 0, OTP_KEY_GENERATE_TYPE_DERIVE = 1 } otp_key_generate_type_e;

typedef struct otp_key_attributes_ {
    uint8_t key_algo_id;
    uint16_t otp_slot_id;
    uint32_t otp_logic_id;
    otp_key_generate_type_e key_gen_type;
    uint32_t key_usage;
} otp_key_attributes_st;

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
 * @brief Register keyid map
 *
 * @keyid_map [in] The keyid map buffer first address
 * @keyid_num [in] The key id number
 *
 **
 * @return 0 for success, negative values for error.
 */
uint32_t otpkey_register_keyid_map(const otp_key_attributes_st *keyid_map, uint32_t keyid_num);

/**
 * @brief check a otp key usage
 *
 * @logic_key_id [in] The logic key id
 * @algo_type [in] The key algorithm type will be checked
 * @key_level [in] The key level will be checked
 * @key_usage [in] The key usage will be checked
 *
 ** note: The parameter of algorithm type can be set KEY_USAGE_SKE/KEY_USAGE_PKE/KEY_USAGE_HASH
 **
 * @return 0 for success, negative values for error.
 */
uint32_t otpkey_check_usage(uint32_t key_id, uint8_t algo_type, uint8_t key_level, uint32_t key_usage);

/**
 * @brief Aquire key attribute
 *
 * @logic_key_id [in] The logic key id
 * @otp_key_attr [out] a pointer point to start buffer of otp_key_attributes_st
 *
 **
 * @return 0 for success, negative values for error.
 */
uint32_t otpkey_acquire_key_attr(uint32_t key_id, otp_key_attributes_st *otp_key_attr);

/**
 * @brief read a logic otp key data
 *
 * @logic_key_id [in] The logic key id
 * @out [out] The output the key data
 *
 ** note: 1.The buffer of "out" must be greater or equal to 32 bytes
 **
 * @return 0 for success, negative values for error.
 */
uint32_t otpkey_read_data(uint32_t key_id, uint8_t *out);

/**
 * @brief convert otp logic id to physical id
 *
 * @logic_key_id [in] The logic key id
 * @phy_id [out] The output the physical id
 **
 * @return 0 for success, negative values for error.
 */
uint32_t otpkey_get_phyid(uint32_t key_id, uint16_t *phy_id);

uint32_t otpkey_get_max_phyid(void);
#endif
