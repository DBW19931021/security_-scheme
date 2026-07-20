/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "otp_key.h"
#include "kmu_driver.h"
#include "util.h"
#include "kms.h"

/***********************************************************************************************************************
 *  VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define OTP_LOGIC_KID_BEG (KMS_KEY_TYPE_OTP)
#define OTP_LOGIC_KID_END (OTP_LOGIC_KID_BEG + 100U)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/
static uint32_t g_keyid_map_size = 0;
static uint32_t g_max_phy_keyid = 0;
static const otp_key_attributes_st *g_keyid_map_addr = NULL;
/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t otpkey_get_keyid_map_idx(uint32_t logic_key_id, uint8_t *idx);
static uint32_t otpkey_gen_key_data(const otp_key_attributes_st *attr, uint8_t *out);
static uint32_t otpkey_check_key_usage(
    const otp_key_attributes_st *attr, uint8_t algo_type, uint8_t key_level, uint32_t key_usage);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t otpkey_get_keyid_map_idx(uint32_t logic_key_id, uint8_t *idx)
{
    uint32_t id;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    for (id = 0; id < g_keyid_map_size; id++) {
        if (logic_key_id == g_keyid_map_addr[id].otp_logic_id) {
            *idx = (uint8_t)id;
            break;
        }
    }

    if (id >= g_keyid_map_size) {
        ret = EHSM_ERR_INVALID_HANDLE;
    }

    return ret;
}

static uint32_t otpkey_check_key_usage(
    const otp_key_attributes_st *attr, uint8_t algo_type, uint8_t key_level, uint32_t key_usage)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (key_usage != KEY_USAGE_NONE) {
        if ((attr->key_usage & key_usage) != key_usage) {
            ret = EHSM_ERR_MISMATCH_KEY_USAGE;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kmu_check_key_attr(attr->otp_slot_id, algo_type, key_level);
    }

    return ret;
}

static uint32_t otpkey_gen_key_data(const otp_key_attributes_st *attr, uint8_t *out)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (attr->key_gen_type == OTP_KEY_GENERATE_TYPE_DIRECT) {
        ret = kmu_get_keyid_data(attr->otp_slot_id, out, OTP_KEY_MAX_SIZE);
    } else {
        ret = EHSM_ERR_NOT_SUPPORT;
    }

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t otpkey_register_keyid_map(const otp_key_attributes_st *keyid_map, uint32_t keyid_num)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t id;

    if ((keyid_map == NULL) || (keyid_num == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        g_keyid_map_addr = keyid_map;
        g_keyid_map_size = keyid_num;
        g_max_phy_keyid = 0;
        for (id = 0; id < g_keyid_map_size; id++) {
            if (g_keyid_map_addr[id].otp_slot_id > g_max_phy_keyid) {
                g_max_phy_keyid = g_keyid_map_addr[id].otp_slot_id;
            }
        }
    }

    return ret;
}

uint32_t otpkey_check_usage(uint32_t key_id, uint8_t algo_type, uint8_t key_level, uint32_t key_usage)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t idx = 0;

    if ((g_keyid_map_addr == NULL) || (g_keyid_map_size == 0U)) {
        ret = EHSM_ERR_NOT_INIT;
    } else if (key_usage > KEY_USAGE_PERMIT_ALL_BITS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((key_id < OTP_LOGIC_KID_BEG) || (key_id > OTP_LOGIC_KID_END)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otpkey_get_keyid_map_idx(key_id, &idx);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = otpkey_check_key_usage(&g_keyid_map_addr[idx], algo_type, key_level, key_usage);
        }
    }

    return ret;
}

uint32_t otpkey_acquire_key_attr(uint32_t key_id, otp_key_attributes_st *otp_key_attr)
{
    uint8_t idx = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((g_keyid_map_addr == NULL) || (g_keyid_map_size == 0U) || (otp_key_attr == NULL)) {
        ret = EHSM_ERR_NOT_INIT;
    } else if ((key_id < OTP_LOGIC_KID_BEG) || (key_id > OTP_LOGIC_KID_END)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otpkey_get_keyid_map_idx(key_id, &idx);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            /*only check the OTP key whether has been burned*/
            ret = kmu_check_key_attr(g_keyid_map_addr[idx].otp_slot_id, OTP_KEY_ALGO_NO_CHECK, OTP_KEY_LEVEL_NO_CHECK);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            util_memcpy(otp_key_attr, &g_keyid_map_addr[idx], sizeof(otp_key_attributes_st));
        }
    }

    return ret;
}

uint32_t otpkey_read_data(uint32_t key_id, uint8_t *out)
{
    uint8_t idx = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((g_keyid_map_addr == NULL) || (g_keyid_map_size == 0U)) {
        ret = EHSM_ERR_NOT_INIT;
    } else if ((out == NULL) || (key_id < OTP_LOGIC_KID_BEG) || (key_id > OTP_LOGIC_KID_END)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otpkey_get_keyid_map_idx(key_id, &idx);
        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_INVALID_HANDLE;
        } else {
            ret = otpkey_gen_key_data(&g_keyid_map_addr[idx], out);
        }
    }

    return ret;
}

uint32_t otpkey_get_phyid(uint32_t key_id, uint16_t *phy_id)
{
    uint8_t idx = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((g_keyid_map_addr == NULL) || (g_keyid_map_size == 0U)) {
        ret = EHSM_ERR_NOT_INIT;
    } else if ((phy_id == NULL) || (key_id < OTP_LOGIC_KID_BEG) || (key_id > OTP_LOGIC_KID_END)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otpkey_get_keyid_map_idx(key_id, &idx);
        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_INVALID_HANDLE;
        } else {
            *phy_id = g_keyid_map_addr[idx].otp_slot_id;
        }
    }

    return ret;
}

uint32_t otpkey_get_max_phyid(void)
{
    return g_max_phy_keyid;
}
