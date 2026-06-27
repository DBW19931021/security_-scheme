/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "sysreg.h"
#include "kmu_driver.h"
#include "component/util.h"
#include "schedule/expt_det.h"
#include "reg_lock.h"

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
typedef struct {
    uint32_t ctrl;
    uint8_t pad1[0x8];
    uint32_t ex_ch;
    uint8_t pad2[0x70];
    uint32_t derive_trig_bit;
    uint8_t pad3[0x3C];
    uint32_t key_en;
    uint8_t pad4[0x1F3C];
    uint32_t buff[256][8];
    uint32_t attr[256];
} kmu_reg_st;

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
#ifdef CONFIG_UNIT_TEST
volatile kmu_reg_st g_kmu_reg;
static inline volatile kmu_reg_st *kmu_get_reg_base(void)
{
    return &g_kmu_reg;
}
#else
static inline volatile kmu_reg_st *kmu_get_reg_base(void)
{
    return (volatile kmu_reg_st *)(KMU_BASE_ADDR);
}
#endif

static inline uint32_t kmu_check_param(uint8_t algo_type, uint8_t key_level)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((algo_type == OTP_KEY_ALGO_SKE_TYPE) || (algo_type == OTP_KEY_ALGO_PKE_TYPE)
        || (algo_type == OTP_KEY_ALGO_HASH_TYPE) || (algo_type == OTP_KEY_ATTR_NO_CHECK)) {
        if ((key_level == OTP_KEY_LEVEL_0) || (key_level == OTP_KEY_LEVEL_1) || (key_level == OTP_KEY_LEVEL_2)
            || (key_level == OTP_KEY_ATTR_NO_CHECK)) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}

static inline uint32_t kmu_verify_attr_field(uint16_t key_id, uint32_t mask, uint8_t offset, uint8_t value)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const volatile kmu_reg_st *kmu_reg = kmu_get_reg_base();

    if (value != OTP_KEY_ATTR_NO_CHECK) {
        if (((kmu_reg->attr[key_id] & mask) >> offset) != value) {
            ret = EHSM_ERR_MISMATCH_KEY_USAGE;
        }
    }

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t kmu_config(uint16_t key_id, uint8_t sec_port_sel)
{
    uint32_t value = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    volatile kmu_reg_st *kmu_reg = kmu_get_reg_base();

    if (sec_port_sel > KMU_TRANSP_KEY_TO_PKE) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if ((key_id < KMU_TOTAL_KEY_NUM) || (key_id == KMU_KID_INSTALL_KEK_EHSM)
            || (key_id == KMU_KID_INSTALL_KEK_SOC)) {
            value = (((uint32_t)((uint32_t)key_id & 0xFFU)) << 16U)
                | (((uint32_t)((uint32_t)sec_port_sel & 0x0FU)) << 4U);
            sysreg_unlock_reg(KMU_REG_BASE);
            kmu_reg->ctrl = 0U;
            kmu_reg->ctrl = value;
            kmu_reg->ctrl |= 1U; // Must be last set start_key_transport for hardware limit
            sysreg_lock_reg(KMU_REG_BASE);
            if ((kmu_reg->ctrl & value) != value) {
                (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
            }
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    // 这里成功返回0，因为这个函数是供cryptolib调用的，cryptolib里判断0为成功
    ret = (EHSM_ERR_SW_SUCCESS == ret) ? 0 : ret;

    return ret;
}

uint32_t kmu_config_derive_key(uint16_t key_id)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t otp_last_key_num = (uint16_t)sysreg_get_otp_key_num();
    volatile kmu_reg_st *kmu_reg = kmu_get_reg_base();

    if ((key_id >= otp_last_key_num) && (key_id < KMU_TOTAL_KEY_NUM)) {
        sysreg_unlock_reg(KMU_REG_BASE);
        // Config derive key to kmu which slot
        kmu_reg->ctrl = 0U;
        kmu_reg->ctrl = ((uint32_t)((uint32_t)key_id & 0xFFU)) << 16U;

        // Triger derived key starting from HASH transpot to kmu ram
        kmu_reg->derive_trig_bit = 1U;
        sysreg_lock_reg(KMU_REG_BASE);
        if (kmu_reg->ctrl != (((uint32_t)((uint32_t)key_id & 0xFFU)) << 16U)) {
            (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    return ret;
}

uint32_t kmu_check_key_attr(uint16_t key_id, uint8_t algo_type, uint8_t key_level)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t vf_algo_ret = EHSM_ERR_SW_SUCCESS;
    uint32_t vf_lv_ret = EHSM_ERR_SW_SUCCESS;
    uint32_t vf_lf_ret = EHSM_ERR_SW_SUCCESS;

    ret = kmu_check_param(algo_type, key_level);
    if (ret != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (key_id >= KMU_TOTAL_KEY_NUM) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        vf_algo_ret = kmu_verify_attr_field(key_id, OTP_K_ATTR_ALGO_MASK, OTP_K_ATTR_ALGO_OFFSET, algo_type);
        vf_lv_ret = kmu_verify_attr_field(key_id, OTP_K_ATTR_LEVEL_MASK, OTP_K_ATTR_LEVEL_OFFSET, key_level);
        vf_lf_ret = kmu_verify_attr_field(
            key_id, OTP_K_ATTR_LIFECYCLE_MASK, OTP_K_ATTR_LIFECYCLE_OFFSET, OTP_KEY_LIFECYCLE_ENABLE);

        if ((vf_algo_ret == EHSM_ERR_SW_SUCCESS) && (vf_lv_ret == EHSM_ERR_SW_SUCCESS)
            && (vf_lf_ret == EHSM_ERR_SW_SUCCESS)) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_MISMATCH_KEY_USAGE;
        }
    }

    return ret;
}

uint32_t kmu_get_keyid_data(uint16_t key_id, uint8_t *data, uint32_t size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const volatile kmu_reg_st *kmu_reg = kmu_get_reg_base();

    if ((data == NULL) || (size == 0U) || (size > 32U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (key_id >= KMU_TOTAL_KEY_NUM) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        uint32_t buf[8];
        util_read_volatile_u32(buf, kmu_reg->buff[key_id], 8);
        util_memcpy(data, buf, size);
        util_memset(buf, 0, sizeof(buf));
    }

    return ret;
}
