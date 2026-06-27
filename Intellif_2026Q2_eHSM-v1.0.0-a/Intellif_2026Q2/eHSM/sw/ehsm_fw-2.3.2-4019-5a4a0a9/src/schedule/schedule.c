/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "schedule.h"
#include "expt_det.h"
#include "mb.h"
#include "component/queue.h"
#include "component/comm.h"
#include "component/util.h"
#include "component/dbgcmd_parser.h"
#include "kms.h"
#include "misc_srv.h"
#include "dbgauth_srv.h"
#include "otpkinstl_srv.h"
#include "debug.h"
#include "service/rng_srv.h"
#include "service/aead/aead_srv.h"
#include "service/hash/hash_srv.h"
#include "service/hash/hmac_srv.h"
#include "service/ske/cipher_srv.h"
#include "service/ske/mac_srv.h"
#include "service/pke/ecdsa_srv.h"
#include "service/pke/rsa_srv.h"
#include "service/pke/sm2_srv.h"
#include "service/utc_timer_srv.h"
#include "service/counter_srv.h"
#include "service/socvrfy_srv.h"
#include "service/fwup_srv.h"
#include "service/chacha_srv.h"
#include "service/pke/sm9_srv.h"
#include "driver/watchdog_driver.h"
#include "driver/sysreg.h"
#include "mmap.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define WAITE_INTO_WFI_FLAG (0xA55A5AA5U)

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    queue_st queue;
    queue_element_t queue_elements[SCHEDULE_QUEUE_SIZE];
} sch_queue_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static sch_queue_st g_sch_queue[MAX_SCH_CHL];
static volatile uint32_t g_waite_into_wfi = 0U;
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
#if CONFIG_EHSM_TEST_CMDS_ENABLE
#define MB_CMD_ID_BL_READ_MEMORY  0xfe80u
#define MB_CMD_ID_BL_WRITE_MEMORY 0xfe81u
#define MB_CMD_ID_BL_JUMP_TO_ADDR 0xfe82u
#define MB_CMD_ID_BL_JUMP_TO_LOOP 0xfe83u

// FW CTRL EHSM 的 bit21 目前未使用
#define is_internal_test_cmds_enable() ((SYS_FW_CFG_REG1 & (1 << 21)) == 0)

#pragma pack(1)

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t src_addr;
    uint32_t size;
    raddr_t dest_addr;
} mb_cmd_bl_read_memory_st;

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t dest_addr;
    uint32_t size;
    raddr_t src_addr;
} mb_cmd_bl_write_memory_st;

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t addr;
} mb_cmd_bl_jump_to_addr_st;

#pragma pack()

static uint32_t mbcmdpars_handle_read_memory(cmd_packet_st *packet)
{
    if (is_internal_test_cmds_enable()) {

        mb_cmd_bl_read_memory_st cmd[1];
        (void)memcpy_((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_read_memory_st));
        mmap_write_remote_data(cmd->dest_addr, (const void *)cmd->src_addr, cmd->size);
    }
    return EHSM_ERR_SW_SUCCESS;
}

static uint32_t mbcmdpars_handle_write_memory(cmd_packet_st *packet)
{
    if (is_internal_test_cmds_enable()) {

        mb_cmd_bl_write_memory_st cmd[1];
        (void)memcpy_((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_write_memory_st));
        mmap_read_remote_data((void *)cmd->dest_addr, cmd->src_addr, cmd->size);
    }
    return EHSM_ERR_SW_SUCCESS;
}

static uint32_t mbcmdpars_jump_to_addr(cmd_packet_st *packet)
{
    if (is_internal_test_cmds_enable()) {

        mb_cmd_bl_jump_to_addr_st cmd[1];
        (void)memcpy_((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_jump_to_addr_st));
        // 强制跳转
        ((void (*)(void))cmd->addr)();
    }
    return EHSM_ERR_SW_SUCCESS;
}

static uint32_t mbcmdpars_jump_to_loop(cmd_packet_st *packet)
{
    if (is_internal_test_cmds_enable()) {
        (void)packet;
        while (1) {
            asm("nop");
        };
    }
    return EHSM_ERR_SW_SUCCESS;
}
#endif // CONFIG_EHSM_TEST_CMDS_ENABLE

static uint32_t sch_check_cmd_auth(uint32_t comm_chl, uint32_t cmd_id)
{
    uint32_t ret;

    if ((comm_chl == UART00) && (cmd_id != DBG_CMD_ID)) {
        ret = EHSM_ERR_NO_CMD_EXEC_PERM;
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

static uint32_t sch_trans_sch_chl(uint32_t comm_chl)
{
    uint32_t ret_chl;
    uint32_t comm_type;
    uint32_t mbox_chl;

    if (comm_chl == UART00) {
        ret_chl = 0;
    } else {
        comm_type = comm_chl & 0xF0000000U;
        if (COMM_TYPE_MAILBOX == comm_type) {
            mbox_chl = comm_chl & 0xFFFFFFFU;
            if (mbox_chl < CONFIG_EHSM_MB_CHANNEL_COUNT) {
                ret_chl = mbox_chl + 1U;
            } else {
                ret_chl = INVALID_SCH_CHL;
            }
        } else {
            ret_chl = INVALID_SCH_CHL;
        }
    }

    return ret_chl;
}

static uint32_t sch_process_srv(cmd_packet_st *packet)
{
    uint32_t ret;
    uint16_t cmd_id_inv;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_id_inv = ~packet->cmd_data.cmd_id_inv;
        if (packet->cmd_data.cmd_id != cmd_id_inv) {
            ret = EHSM_ERR_INVALID_CMD;
        } else {
            switch (packet->cmd_data.cmd_id) {
            case MB_CMD_ID_EHSM_GEN_KEY:
            case MB_CMD_ID_EHSM_IMPORT_KEY:
            case MB_CMD_ID_EHSM_EXPORT_KEY:
            case MB_CMD_ID_EHSM_KEY_DERIVE:
            case MB_CMD_ID_EHSM_KEY_EXCHANGE:
            case MB_CMD_ID_EHSM_GET_PUB_FROM_PRIV:
            case MB_CMD_ID_EHSM_REMOVE_KEY:
                ret = kms_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_RNG:
                ret = rng_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_MAC:
                ret = ske_srv_mac_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_HASH:
                ret = hash_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_HMAC:
                ret = hmac_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_SM2_CIPHER:
                ret = pke_srv_sm2_cipher_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_SM2_SIGN:
                ret = pke_srv_sm2_sign_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_RSA_CIPHER:
                ret = pke_srv_rsa_cipher_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_RSA_SIGN:
                ret = pke_srv_rsa_sign_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_ECDSA:
                ret = pke_srv_ecdsa_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_SYMM_CIPHER:
                ret = ske_srv_cipher_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_AEAD_GCM:
                ret = aead_srv_gcm_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_AEAD_CCM:
                ret = aead_srv_ccm_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_GET_EMU:
            case MB_CMD_ID_OTP_READ:
            case MB_CMD_ID_OTP_WRITE:
            case MB_CMD_ID_FW_REG_WR:
            case MB_CMD_ID_FW_REG_RD:
            case MB_CMD_ID_EHSM_READ_VER:
            case MB_CMD_ID_CHANGE_LIFECYCLE:
            case MB_CMD_ID_SET_UART_BAUDRATE:
            case MB_CMD_ID_CHANGE_CONTROL_FIELD:
#if CONFIG_EHSM_TEST_CMDS_ENABLE
            case MB_CMD_ID_OTP_WRITE_FOR_TEST:
#endif // CONFIG_EHSM_TEST_CMDS_ENABLE
                ret = misc_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_AUTH:
            case MB_CMD_ID_GET_CHALLENGE:
            case MB_CMD_ID_CLOSE_DEBUG:
                ret = dbgauth_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_INSTALL_RANDOM_KEY:
            case MB_CMD_ID_INSTALL_ENCRYPT_KEY:
                ret = otpkinstl_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case DBG_CMD_ID:
                ret = dbgcmdpars_srv_handler(packet);
                break;
            case MB_CMD_ID_SOC_VERIFY:
                ret = socvrfy_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_FW_UPGRADE:
                ret = fwup_srv_handler(&packet->cmd_data, &packet->rsp_data);
                break;
            case MB_CMD_ID_ENTER_WFI:
                g_waite_into_wfi = WAITE_INTO_WFI_FLAG;
                ret = EHSM_ERR_SW_SUCCESS;
                break;
#if CONFIG_EHSM_TEST_CMDS_ENABLE
            case MB_CMD_ID_BL_READ_MEMORY:
                ret = mbcmdpars_handle_read_memory(packet);
                break;
            case MB_CMD_ID_BL_WRITE_MEMORY:
                ret = mbcmdpars_handle_write_memory(packet);
                break;
            case MB_CMD_ID_BL_JUMP_TO_ADDR:
                ret = mbcmdpars_jump_to_addr(packet);
                break;
            case MB_CMD_ID_BL_JUMP_TO_LOOP:
                ret = mbcmdpars_jump_to_loop(packet);
                break;
#endif // CONFIG_EHSM_TEST_CMDS_ENABLE
            default:
                ret = EHSM_ERR_INVALID_CMD;
                break;
            }
        }
    }

    return ret;
}

static void sch_check_wfi_mode(void)
{
#ifndef CONFIG_UNIT_TEST
    if (WAITE_INTO_WFI_FLAG == g_waite_into_wfi) {
        log_debug("enter wfi\n");
        watchdog_stop();
        asm volatile("wfi\n\t");
        watchdog_start(DEFAULT_WDT_TIMEOUT);
        log_debug("exit wfi\n");
        g_waite_into_wfi = 0U;
    }
#endif
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t sch_init(void)
{
    uint32_t index;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    for (index = 0; index < MAX_SCH_CHL; index++) {
        ret = queue_init(&g_sch_queue[index].queue, g_sch_queue[index].queue_elements, SCHEDULE_QUEUE_SIZE);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            break;
        }
    }

    return ret;
}

void sch_start(void)
{
    uint32_t sch_chl;
    uint32_t sch_stop = 0;
    queue_element_t queue_data;
    uint32_t ret;
    cmd_packet_st *cmd = NULL;
    bool_t handle_cmd = false;

    log_debug("sch_start\n");
    while (sch_stop == 0U) {
        for (sch_chl = 0; sch_chl < MAX_SCH_CHL; sch_chl++) {
            ret = queue_pop(&g_sch_queue[sch_chl].queue, &queue_data);

            if (EHSM_ERR_SW_SUCCESS == ret) {
                (void)util_memcpy((void *)&cmd, &queue_data, sizeof(queue_element_t));
                ret = expt_det_check_before(cmd);
                if (EHSM_ERR_SW_SUCCESS != ret) {
                    cmd->rsp_data.ret_code = ret;
                    (void)comm_send_rsp(cmd);
                    (void)expt_det_check_after();
                }
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = sch_check_cmd_auth(cmd->channel, cmd->cmd_data.cmd_id);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    handle_cmd = true;
                    cmd->rsp_data.ret_code = sch_process_srv(cmd);
                } else {
                    cmd->rsp_data.ret_code = ret;
                }

                if (EHSM_ERR_SW_SUCCESS == cmd->rsp_data.ret_code) {
                    cmd->rsp_data.ret_code = EHSM_ERR_MB_SUCCESS;
                }
                (void)comm_send_rsp(cmd);
                (void)expt_det_check_after();
            }
        }
        if (false == handle_cmd) {
            (void)expt_det_check_idle();
            sch_check_wfi_mode();
        }
        comm_poll();
        handle_cmd = false;
    }
}

uint32_t sch_add_cmd(cmd_packet_st *packet)
{
    uint32_t ret;
    uint32_t sch_chl;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        sch_chl = sch_trans_sch_chl(packet->channel);
        if (INVALID_SCH_CHL == sch_chl) {
            ret = EHSM_ERR_INVALID_CHANNEL;
        } else {
            ret = queue_push(&g_sch_queue[sch_chl].queue, (queue_element_t)packet);
        }
    }

    return ret;
}

bool_t sch_is_cache_full(uint32_t comm_chl)
{
    uint32_t sch_chl;
    bool_t ret;

    sch_chl = sch_trans_sch_chl(comm_chl);
    if (INVALID_SCH_CHL == sch_chl) {
        ret = true;
    } else {
        ret = queue_is_full(&g_sch_queue[sch_chl].queue);
    }

    return ret;
}

/**
 *
 */
