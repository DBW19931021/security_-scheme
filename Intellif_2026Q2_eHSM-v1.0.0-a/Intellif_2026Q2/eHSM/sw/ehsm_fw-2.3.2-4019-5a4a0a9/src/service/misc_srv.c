/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mb.h"
#include "config.h"
#include "kms.h"
#include "misc_srv.h"
#include "mmap.h"
#include "version.h"
#include "dbgauth_srv.h"
#include "she/she_key.h"
#include "she/she_util.h"
#include "component/kds.h"
#include "component/util.h"
#include "../driver/sysreg.h"
#include "driver/utc_timer_driver.h"
#include "component/otp_map.h"
#include "../driver/otp/otp_driver.h"
#include "driver/emu_driver.h"
#include "driver/uart_driver.h"
#include "component/crypto_api.h"
#include "fid.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*Definition the max size of otp read and write data*/
#define MISC_OTP_DATA_MAX_SIZE (CONFIG_EHSM_OTP_SIZE)
/*Definition the offset of otp extend area(not includes system otp chip keys)*/
#define CONFIG_EHSM_KMGR_V_OTP_EXT_K_OFF (0x118U + (CONFIG_EHSM_OTP_KEY_COUNT * OTP_KEY_SIZE))
#if CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID
/*Definition the value of life cycle valid*/
#define EHSM_LF_VALID_DATA (0x0002EC48U)
#endif
/*Definition the max size of control filed data(Mailbox command)*/
#define MB_OTP_CONTROL_FILED_BYTE_SIZE (0x08U)
/*Definition the max size of control filed data(Otp control filed)*/
#define OTP_CONTROL_FIELD_SIZE (OTP_FW_CONTROL_FIELD_EHSM_ADDRESS - OTP_HW_CONTROL_FIELD_HW_ADDRESS)

#ifdef CONFIG_UNIT_TEST
static uint32_t g_emu_reg[0x20];
#define EMU_ERR_FW0_REG      (g_emu_reg[0])
#define EMU_ERR_FW1_REG      (g_emu_reg[1])
#define EMU_HW_TRIG_0_REG    (g_emu_reg[2])
#define EMU_HW_TRIG_1_REG    (g_emu_reg[3])
#define EMU_ALARM_TRIG_0_REG (g_emu_reg[4])
#define EMU_ALARM_TRIG_1_REG (g_emu_reg[5])
#define MB_HSM_STATUS_0_REG  (g_emu_reg[6])
#define MB_HSM_STATUS_1_REG  (g_emu_reg[7])
#define EMU_INT_SENSOR_REG   (g_emu_reg[8])
#define EMU_INT_HW0_REG      (g_emu_reg[9])
#define EMU_INT_HW1_REG      (g_emu_reg[10])
#endif

#define MISC_UART_BAUDRATE_DIV_MIN (0x10U)
#define MISC_UART_BAUDRATE_DIV_MAX (0xFFFFFU)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
/*Definition the struction of life cycle value and size */
typedef struct {
    uint32_t *value;
    uint32_t size;
} misc_lf_data_st;
/*Definition the struction of life cycle information */
typedef struct {
    uint32_t otp_addr;
    uint32_t *valid_data;
    uint32_t valid_size;
    misc_lf_data_st lf_data[EHSM_LIFE_CYCLE_MAX_MODE];
} misc_otp_lf_info_st;
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
/**
 *   @brief This function compare whether two otp data are equal, and return the data difference between i and r.
 *
 *   @param[in] r The otp data that read by otp device.
 *   @param[in] i Input the data that needs to be compared with r.
 *   @param[in] s  size of data.
 *   @param[out] o output the data that needs to be write.
 *
 *   @return     0 meant two data are equal, 1 meant two data are not equal.
 *
 *   @note
 *
 */
static uint8_t misc_data_cmp(const uint8_t *r, const uint8_t *i, uint32_t s, uint8_t *o)
{
    uint8_t ret = 0;

    if (util_memcmp(r, i, s) != 0U) {
        ret = 1;
    } else {
        ret = 0;
    }

    if (o != NULL) {
        util_memcpy(o, i, s);
    }

    return ret;
}
/**
 *   @brief      This function loop check with the smallest otp write byte, only write on the input data is inconsistent
 * with the stored data
 *
 *   @param[in] addr  The address of OTP to be write.
 *   @param[in] data  a pointer point to OTP data .
 *   @param[in] size  size of data.
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
inline static uint32_t misc_loop_write_otp_unit(uint32_t addr, const uint8_t *data, uint32_t size)
{
    uint32_t idx = 0;
    bool_t loop_exit = FALSE;
    uint8_t w_buf[CONFIG_EHSM_OTP_WRITE_UNIT];
    uint8_t tmp_buf[CONFIG_EHSM_OTP_WRITE_UNIT];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t cmp_ret = 0;

    for (idx = 0; idx < size; idx += CONFIG_EHSM_OTP_WRITE_UNIT) {
        ret = otp_read(addr + idx, tmp_buf, CONFIG_EHSM_OTP_WRITE_UNIT);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            cmp_ret = misc_data_cmp(tmp_buf, &data[idx], CONFIG_EHSM_OTP_WRITE_UNIT, w_buf);
            if (cmp_ret != 0U) {
                ret = otp_write(addr + idx, w_buf, CONFIG_EHSM_OTP_WRITE_UNIT);
                if (ret != EHSM_ERR_SW_SUCCESS) {
                    loop_exit = TRUE;
                }
            }
        } else {
            loop_exit = TRUE;
        }

        if (loop_exit == TRUE) {
            break;
        }
    }

    return ret;
}
/**
 *   @brief      This function read otp data and check it whether same with input data
 *
 *   @param[in] addr  The address of OTP to be checked.
 *   @param[in] data  a pointer point to OTP data .
 *   @param[in] size  size of data.
 *   @param[out] flag  output the data check result.
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
inline static uint32_t misc_check_otp_data(uint32_t addr, const uint8_t *data, uint32_t size, bool_t *flag)
{
    uint32_t idx = 0;
    bool_t same_flag = TRUE;
    uint8_t tmp_buf[CONFIG_EHSM_OTP_WRITE_UNIT];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t cmp_ret = 0;

    *flag = FALSE;

    for (idx = 0; idx < size; idx += CONFIG_EHSM_OTP_WRITE_UNIT) {
        ret = otp_read(addr + idx, tmp_buf, CONFIG_EHSM_OTP_WRITE_UNIT);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            cmp_ret = misc_data_cmp(tmp_buf, &data[idx], CONFIG_EHSM_OTP_WRITE_UNIT, NULL);
            if (cmp_ret != 0U) {
                same_flag = FALSE;
                ret = EHSM_ERR_OTP_WRITE_CMP_ERROR;
            }
        } else {
            same_flag = FALSE;
        }

        if (same_flag == FALSE) {
            break;
        }
    }

    *flag = same_flag;

    return ret;
}

/**
 *   @brief This function write data to OTP, before write will do check OTP data firstly. read OTP data and compare with
 * data which need to write, if both data are same not need to write again. else doing write process.
 *
 *   @param[in] otp_addr  The address of OTP to be write.
 *   @param[in] data  a pointer point OTP data which need to write.
 *   @param[in] size  size of data.
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
uint32_t misc_write_otp_data(uint32_t otp_addr, const uint8_t *data, uint32_t size)
{
    bool_t data_is_same = FALSE;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((otp_addr < CONFIG_EHSM_OTP_BASE_ADDR) || (size % CONFIG_EHSM_OTP_WRITE_UNIT != 0)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((size > CONFIG_EHSM_OTP_SIZE)
        || ((otp_addr - CONFIG_EHSM_OTP_BASE_ADDR) > (CONFIG_EHSM_OTP_SIZE - size))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // 1. write data to otp area.
        ret = misc_loop_write_otp_unit(otp_addr, data, size);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        // 2. if otp write success, read back and check again
        ret = misc_check_otp_data(otp_addr, data, size, &data_is_same);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (data_is_same != TRUE) {
                ret = EHSM_ERR_OTP_WRITE_CMP_ERROR;
            }
        }
    }

    return ret;
}
/**
 *   @brief      Check if the constraints for otp write have passed
 *
 *   @param[in] cmd_data The otp write command struct buffer
 *
 *   @note       1.The device life cycle must be allowed.
 */
static uint32_t misc_chk_otp_write_limit(const mb_cmd_otp_write_st *cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if (cmd_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if ((cmd_data->ehsm_dst_addr < (CONFIG_EHSM_OTP_BASE_ADDR + CONFIG_EHSM_KMGR_V_OTP_EXT_K_OFF))
            && FID_EQ(SYS_STA0_LIFECYCLE_MANUFACTURE, life_cycle)) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
        else if ((!FID_EQ(SYS_STA0_LIFECYCLE_MCUTEST, life_cycle) && !FID_EQ(SYS_STA0_LIFECYCLE_DEVELOP, life_cycle)
                     && !FID_EQ(SYS_STA0_LIFECYCLE_MANUFACTURE, life_cycle))) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
        else {
#if CONFIG_EHSM_OTP_CHECK_WRITE_AREA
            ret = otp_verify(cmd_data->ehsm_dst_addr, cmd_data->size);
#else
            // nothing to do
#endif
        }
    }

    return ret;
}

/**
 *   @brief Write otp data mailbox command handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note The device life cycle and write address must be allowed.
 *
 */
static uint32_t misc_write_otp_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t data[MISC_OTP_DATA_MAX_SIZE];
    const mb_cmd_otp_write_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_otp_write_st *)req_data;
        if (cmd_data->size > MISC_OTP_DATA_MAX_SIZE) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else {
            ret = mmap_read_remote_data(data, cmd_data->host_src_addr, cmd_data->size);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = misc_chk_otp_write_limit(cmd_data);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = misc_write_otp_data((uint32_t)cmd_data->ehsm_dst_addr, data, cmd_data->size);
    }

    return ret;
}
/**
 *   @brief Read otp data mailbox command handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note The device life cycle and write address must be allowed.
 *
 */
static uint32_t misc_read_otp_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t data[MISC_OTP_DATA_MAX_SIZE];
    const mb_cmd_otp_read_st *cmd_data = NULL;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_otp_read_st *)req_data;
        if (cmd_data->size > MISC_OTP_DATA_MAX_SIZE) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else if ((cmd_data->size > CONFIG_EHSM_OTP_SIZE) || (cmd_data->ehsm_src_addr < CONFIG_EHSM_OTP_BASE_ADDR)
            || ((cmd_data->ehsm_src_addr - CONFIG_EHSM_OTP_BASE_ADDR) > (CONFIG_EHSM_OTP_SIZE - cmd_data->size))) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else if (!FID_EQ(SYS_STA0_LIFECYCLE_MCUTEST, life_cycle) && !FID_EQ(SYS_STA0_LIFECYCLE_DEVELOP, life_cycle)
            && !FID_EQ(SYS_STA0_LIFECYCLE_MANUFACTURE, life_cycle)) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
        else {
            ret = otp_read((uint32_t)cmd_data->ehsm_src_addr, data, cmd_data->size);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_write_remote_data(cmd_data->host_dst_addr, data, cmd_data->size);
    }

    return ret;
}
/**
 *   @brief Write register data mailbox command handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note The device life cycle and write address must be allowed.
 *
 */
static uint32_t misc_read_reg_handle(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_fw_reg_rd_st *cmd_data = NULL;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_fw_reg_rd_st *)req_data;
        if (FID_EQ(SYS_STA0_LIFECYCLE_MCUTEST, life_cycle) || FID_EQ(SYS_STA0_LIFECYCLE_DEVELOP, life_cycle)) {
            if ((cmd_data->ehsm_src_addr >= CFG_REG_BASE_ADDR) && (cmd_data->ehsm_src_addr <= CFG_REG_END_ADDR)
                && (cmd_data->size <= (CFG_REG_END_ADDR - cmd_data->ehsm_src_addr))) {
                ret = mmap_write_remote_data(
                    cmd_data->host_dst_addr, (uint8_t *)((uint32_t)cmd_data->ehsm_src_addr), cmd_data->size);
            } else {
                ret = EHSM_ERR_OUT_OF_MEM;
            }
        } else {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    }

    return ret;
}
/**
 *   @brief Write register data mailbox command handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note The device life cycle and write address must be allowed.
 *
 */
static uint32_t misc_write_reg_handle(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_fw_reg_wr_st *cmd_data = NULL;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_fw_reg_wr_st *)req_data;
        if (FID_EQ(SYS_STA0_LIFECYCLE_MCUTEST, life_cycle) || FID_EQ(SYS_STA0_LIFECYCLE_DEVELOP, life_cycle)) {
            if ((cmd_data->ehsm_dst_addr >= CFG_REG_BASE_ADDR) && (cmd_data->ehsm_dst_addr <= CFG_REG_END_ADDR)
                && (cmd_data->size <= (CFG_REG_END_ADDR - cmd_data->ehsm_dst_addr))) {
                ret = mmap_read_remote_data(
                    (uint8_t *)((uint32_t)cmd_data->ehsm_dst_addr), cmd_data->host_src_addr, cmd_data->size);
            } else {
                ret = EHSM_ERR_OUT_OF_MEM;
            }
        } else {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    }

    return ret;
}
/**
 *   @brief      This function convert the device life cycle to ehsm_lifecycle_e
 *
 *   @param[in] dev_lc the life cycle of device defined
 *
 *   @return     The converted life cycle
 */
static uint32_t misc_life_cycle_convert(uint32_t dev_lc)
{
    uint32_t lifecycle = EHSM_LIFE_CYCLE_UNNORMAL_MODE;

    switch (dev_lc) {
    case SYS_STA0_LIFECYCLE_MCUTEST: {
        lifecycle = EHSM_LIFE_CYCLE_TEST_MODE;
    } break;

    case SYS_STA0_LIFECYCLE_DEVELOP: {
        lifecycle = EHSM_LIFE_CYCLE_DEV_MODE;
    } break;

    case SYS_STA0_LIFECYCLE_MANUFACTURE: {
        lifecycle = EHSM_LIFE_CYCLE_MANU_MODE;
    } break;

    case SYS_STA0_LIFECYCLE_USER: {
        lifecycle = EHSM_LIFE_CYCLE_USER_MODE;
    } break;

    case SYS_STA0_LIFECYCLE_DEBUG: {
        lifecycle = EHSM_LIFE_CYCLE_DEBUG_MODE;
    } break;

    case SYS_STA0_LIFECYCLE_DESTROY: {
        lifecycle = EHSM_LIFE_CYCLE_DESTORY_MODE;
    } break;

    default: {
        lifecycle = EHSM_LIFE_CYCLE_UNNORMAL_MODE;
    } break;
    }

    return lifecycle;
}
/**
 *   @brief      delete all user key data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 */
static uint32_t misc_delete_all_keys(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = kds_remove_all_key();

    return ret;
}


/**
 *   @brief      Check if the constraints for switching lifecycle have passed
 *
 *   @param[in] chg_lf_type The value of lifecycle type
 *
 *   @note       1.The device life cycle must be allowed.
 *               2.The ehsm RAM/NVM key will be deleted if lifecycle change to debug mode.
 */
static uint32_t misc_chk_chg_lfcycle_limit(uint32_t chg_lf_type)
{
    bool_t user_auth_result = FALSE;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t dev_life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(dev_life_cycle, life_cycle2)) {
        fid_panic();
    }
    if ((chg_lf_type < EHSM_LIFE_CYCLE_TEST_MODE) || (chg_lf_type > EHSM_LIFE_CYCLE_DESTORY_MODE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (misc_life_cycle_convert(dev_life_cycle) >= chg_lf_type) {
        // not support rollback life cycle
        ret = EHSM_ERR_NOT_SUPPORT;
    } else {
        // nothing to do
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Check the user authentication result if current life cycle is user mode
        if (FID_EQ(SYS_STA0_LIFECYCLE_USER, dev_life_cycle)) {
            user_auth_result = dbgauth_srv_acquire_user_auth_result();
            if (TRUE != user_auth_result) {
                ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
            }
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (chg_lf_type == EHSM_LIFE_CYCLE_DEBUG_MODE) {
            // Delete all firmware RAM/NVM key if change life cycle to debug mode
            ret = misc_delete_all_keys();
            if (ret != EHSM_ERR_SW_SUCCESS) {
                ret = EHSM_ERR_REMOVE_USER_KEY_FAILED;
            }
        }
    }

    return ret;
}
/**
 *   @brief      This function change firmware life cycle
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note       1.The device life cycle must be allowed.
 *               2.The ehsm RAM/NVM key will be deleted if lifecycle change to debug mode.
 */
static uint32_t misc_change_lifecycle_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_change_lifecycle_st *cmd_data = NULL;
#if CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID
    uint32_t g_lf_valid_data[] = { 0x0002EC48U };
#endif
/*Definition the value of lifecycle mode*/
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0
    uint32_t g_lf_test_mode_data[] = { 0x00000000U };
    uint32_t g_lf_develop_mode_data[] = { 0x42818414U };
    uint32_t g_lf_manu_mode_data[] = { 0x46C1A416U };
    uint32_t g_lf_user_mode_data[] = { 0x57C1EC96U };
    uint32_t g_lf_debug_mode_data[] = { 0xD7C5FCDEU };
    uint32_t g_lf_destroy_mode_data[] = { 0xFFFFFFFFU };
#else
    uint32_t g_lf_test_mode_data[] = { 0xFFFFFFFFU };
    uint32_t g_lf_develop_mode_data[] = { 0xBD7E7BEBU };
    uint32_t g_lf_manu_mode_data[] = { 0xB93E5BE9U };
    uint32_t g_lf_user_mode_data[] = { 0xA83E1369U };
    uint32_t g_lf_debug_mode_data[] = { 0x283A0321U };
    uint32_t g_lf_destroy_mode_data[] = { 0x00000000U };
#endif
    /*Definition the instantiated life cycle data*/
    misc_otp_lf_info_st g_lf_info = { .otp_addr = OTP_LIFE_CYCLE_ADDRESS,
#if CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID
        .valid_data = g_lf_valid_data,
        .valid_size = sizeof(g_lf_valid_data),
#else
        .valid_data = NULL,
        .valid_size = 0x00,
#endif
        .lf_data = {
            { NULL, 0 },
            { g_lf_test_mode_data, sizeof(g_lf_test_mode_data) },
            { g_lf_develop_mode_data, sizeof(g_lf_develop_mode_data) },
            { g_lf_manu_mode_data, sizeof(g_lf_manu_mode_data) },
            { g_lf_user_mode_data, sizeof(g_lf_user_mode_data) },
            { g_lf_debug_mode_data, sizeof(g_lf_debug_mode_data) },
            { g_lf_destroy_mode_data, sizeof(g_lf_destroy_mode_data) },
        } };

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_change_lifecycle_st *)req_data;
        ret = misc_chk_chg_lfcycle_limit(cmd_data->type);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
#if CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID
        // Write life cycle valid data if exist
        if (g_lf_info.valid_data != NULL) {
            ret = misc_write_otp_data(g_lf_info.otp_addr, (uint8_t *)g_lf_info.valid_data, g_lf_info.valid_size);
        }

        // Write life cycle value
        if (EHSM_ERR_SW_SUCCESS == ret)
#endif
        {
            ret = misc_write_otp_data(g_lf_info.otp_addr + g_lf_info.valid_size,
                (uint8_t *)g_lf_info.lf_data[cmd_data->type].value, g_lf_info.lf_data[cmd_data->type].size);
        }
    }

    return ret;
}
/**
 *   @brief      This function change control field
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note       The device life cycle must be allowed.
 *
 */
static uint32_t misc_change_controlfield_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t control_field_buf[OTP_CONTROL_FIELD_SIZE];
    uint32_t dev_life_cycle = sysreg_get_life_cycle();
    uint8_t control_field_value[MB_OTP_CONTROL_FILED_BYTE_SIZE];
    const mb_cmd_change_control_field_st *cmd_data = NULL;
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(dev_life_cycle, life_cycle2)) {
        fid_panic();
    }
    util_memset(control_field_buf, 0x00, sizeof(control_field_buf));
    /*Definition the control field type and address map*/
    const uint32_t ctrl_fd_addr_map[EHSM_CONTROL_FIELD_TYPE_SOC + 1]
        = { OTP_HW_CONTROL_FIELD_HW_ADDRESS, OTP_FW_CONTROL_FIELD_EHSM_ADDRESS, OTP_FW_CONTROL_FIELD_SOC_ADDRESS };

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_change_control_field_st *)req_data;
        if (cmd_data->size != MB_OTP_CONTROL_FILED_BYTE_SIZE) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else {
            // Read control field data from host address
            ret = mmap_read_remote_data(control_field_value, cmd_data->value, cmd_data->size);
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (cmd_data->type > (uint8_t)EHSM_CONTROL_FIELD_TYPE_SOC) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else if ((FID_EQ(SYS_STA0_LIFECYCLE_USER, dev_life_cycle))
            || (FID_EQ(SYS_STA0_LIFECYCLE_DEBUG, dev_life_cycle))) {
            // Not support to setting control field if firmware current in either user or manufacture mode
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else if ((FID_EQ(SYS_STA0_LIFECYCLE_MANUFACTURE, dev_life_cycle))
            && (cmd_data->type != (uint8_t)EHSM_CONTROL_FIELD_TYPE_SOC)) {
            // Only support set soc field if firmware current life cycle is manufacture mode
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else {
            util_memcpy(control_field_buf, control_field_value, cmd_data->size);
            ret = misc_write_otp_data(ctrl_fd_addr_map[cmd_data->type], control_field_buf, OTP_CONTROL_FIELD_SIZE);
        }
    }

    return ret;
}
/**
 *   @brief      This function handler read firmware version data
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
static uint32_t misc_read_version_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    mb_ehsm_version_st version[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_read_ver_st *cmd_data = NULL;
#ifdef CONFIG_UNIT_TEST
    char ctrl_fd_addr_map[8] = { 'a', 'l', 'p', 'h', 'a', '.', '5' };
#else
    /*Definition the version counter string*/
    const char *g_version_string = EHSM_FW_VER_PRE_RELEASE;
#endif

    util_memset(version, 0, sizeof(mb_ehsm_version_st));
    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_read_ver_st *)req_data;
        if (cmd_data->ver_size < sizeof(mb_ehsm_version_st)) {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        } else if (cmd_data->ver_addr == 0U) {
            ret = EHSM_ERR_INVALID_ADDRESS;
        } else {
            // nothing to do
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            version->type = EHSM_FW_TYPE;
            version->ver_major = EHSM_FW_VER_MAJOR;
            version->ver_minor = EHSM_FW_VER_MINOR;
            version->ver_patch = EHSM_FW_VER_PATCH;

            for (uint32_t i = 0U; i < 8U; i++) {
                if (g_version_string[i] == '\0') {
                    break;
                }
                version->ver_pre_release[i] = (uint8_t)g_version_string[i];
            }

            version->pke_engine_ver = cpt_pke_get_version();
            version->pke_lib_ver = cpt_pke_get_driver_version();
            version->ske_engine_ver = cpt_ske_get_version();
            version->ske_lib_ver = cpt_ske_get_driver_version();
            version->hash_engine_ver = cpt_hash_get_version();
            version->hash_lib_ver = cpt_hash_get_driver_version();
            version->trng_engine_ver = cpt_trng_get_version();
            version->trng_lib_ver = cpt_trng_get_driver_version();
            version->hw_ver = SYS_VER1_REG;
            ret = otp_read((CONFIG_EHSM_OTP_BASE_ADDR + 4U), version->uid, sizeof(version->uid));
            if (ret == EHSM_ERR_SW_SUCCESS) {
                ret = mmap_write_remote_data(cmd_data->ver_addr, version, sizeof(mb_ehsm_version_st));
            }
        }
    }

    return ret;
}


/**
 *   @brief      This function get the EMU status information.
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
static uint32_t misc_get_emu_status_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    mb_ehsm_emu_status_st status;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_get_emu_st *cmd_data = NULL;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_get_emu_st *)req_data;
        if (cmd_data->emu_size < sizeof(mb_ehsm_emu_status_st)) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else {
            status.o_hsm_status[0] = MB_HSM_STATUS_0_REG;
            status.o_hsm_status[1] = MB_HSM_STATUS_1_REG;
            status.o_hsm_err_sensor = EMU_INT_SENSOR_REG;
            status.o_hsm_err_hw[0] = EMU_INT_HW0_REG;
            status.o_hsm_err_hw[1] = EMU_INT_HW1_REG;
            status.o_hsm_err_fw[0] = EMU_ERR_FW0_REG;
            status.o_hsm_err_fw[1] = EMU_ERR_FW1_REG;

            ret = mmap_write_remote_data(cmd_data->emu_addr, &status, sizeof(mb_ehsm_emu_status_st));
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->rsp_data_len = 4;
                rsp_data->data[0] = sizeof(mb_ehsm_emu_status_st);
            }
        }
    }

    return ret;
}


#if CONFIG_EHSM_TEST_CMDS_ENABLE
static uint32_t misc_otp_write_for_test_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t data[MISC_OTP_DATA_MAX_SIZE];
    const mb_cmd_otp_write_for_test_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_otp_write_for_test_st *)req_data;
        if (cmd_data->size > MISC_OTP_DATA_MAX_SIZE) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else {
            ret = mmap_read_remote_data(data, cmd_data->host_src_addr, cmd_data->size);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otp_write(cmd_data->ehsm_dst_addr, data, cmd_data->size);
    }

    return ret;
}
#endif // CONFIG_EHSM_TEST_CMDS_ENABLE

/**
 *   @brief      This function for handle setting uart baudrate command
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     EHSM_ERR_SW_SUCCESS for success, otherwise error.
 *
 *   @note
 *
 */
static uint32_t misc_set_baudrate_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_set_uart_baudrate_st *cmd_data = NULL;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_set_uart_baudrate_st *)req_data;

        if ((cmd_data->baud_div > MISC_UART_BAUDRATE_DIV_MAX) || (cmd_data->baud_div < MISC_UART_BAUDRATE_DIV_MIN)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            uart_set_baudrate(cmd_data->baud_div);
        }
    }

    return ret;
}

/**
 *   @brief      misc service processing function entry
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t misc_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (req_data->cmd_id) {
        case MB_CMD_ID_OTP_WRITE:
            ret = misc_write_otp_handler(req_data, rsp_data);
            break;
        case MB_CMD_ID_OTP_READ:
            ret = misc_read_otp_handler(req_data, rsp_data);
            break;
        case MB_CMD_ID_FW_REG_WR:
            ret = misc_write_reg_handle(req_data, rsp_data);
            break;
        case MB_CMD_ID_FW_REG_RD:
            ret = misc_read_reg_handle(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_READ_VER:
            ret = misc_read_version_handler(req_data, rsp_data);
            break;
        case MB_CMD_ID_CHANGE_LIFECYCLE:
            ret = misc_change_lifecycle_handler(req_data, rsp_data);
            break;
        case MB_CMD_ID_CHANGE_CONTROL_FIELD:
            ret = misc_change_controlfield_handler(req_data, rsp_data);
            break;
        case MB_CMD_ID_GET_EMU:
            ret = misc_get_emu_status_handler(req_data, rsp_data);
            break;


        case MB_CMD_ID_SET_UART_BAUDRATE:
            ret = misc_set_baudrate_handler(req_data, rsp_data);
            break;
#if CONFIG_EHSM_TEST_CMDS_ENABLE
        case MB_CMD_ID_OTP_WRITE_FOR_TEST:
            ret = misc_otp_write_for_test_handler(req_data, rsp_data);
            break;
#endif // CONFIG_EHSM_TEST_CMDS_ENABLE
        default:
            ret = EHSM_ERR_INVALID_CMD;
            break;
        }
    }

    return ret;
}
