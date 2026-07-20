/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "schedule.h"
#include "mailbox.h"
#include "uart.h"
#include "emu.h"
#include "cpu_porting.h"
#include "mmap.h"
#include "util.h"
#include "mbcmd_parser.h"
#include "watchdog.h"
#include "secure_boot.h"
#include "debug.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define DBG_CHANNEL_NONE 0x0U
#define DBG_CHANNEL_UART 0x1U

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
static uint32_t sch_send_mbox_rsp(uint32_t mbox_chl, const uint32_t *data, uint32_t size, uint32_t s2h_notify);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

static uint32_t sch_send_mbox_rsp(uint32_t mbox_chl, const uint32_t *data, uint32_t size, uint32_t s2h_notify)
{
    uint32_t ret;
    uint32_t notify = 0;

    if ((mbox_chl > MAX_MAILBOX_CHANNEL_ID) || (NULL == data) || (0U == size)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = mailbox_read_send_notify(mbox_chl, &notify);
        while ((EHSM_ERR_SW_SUCCESS == ret) && ((notify & s2h_notify) != 0UL)) {
            ret = mailbox_read_send_notify(mbox_chl, &notify);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mailbox_write(mbox_chl, data, size);
        }
        (void)mailbox_set_send_notify(mbox_chl, s2h_notify);
    }
    return ret;
}

static uint32_t sch_send_gen_mbox_rsp(cmd_packet_st *packet)
{
    uint32_t ret;
    uint32_t level;
    uint8_t *remap_addr;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        level = cpu_enter_critical();
        remap_addr = mmap_remap_addr_u64(packet->rsp_addr);
        if (NULL != remap_addr) {
            util_memcpy(remap_addr, &packet->rsp_data.ret_code, 4);
            if (packet->rsp_data.rsp_data_len > 0 && packet->rsp_data.rsp_data_len <= sizeof(packet->rsp_data.data)) {
                util_memcpy((void *)&remap_addr[4], packet->rsp_data.data, packet->rsp_data.rsp_data_len);
            }
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_REMAP_FAILED;
        }
        (void)cpu_exit_critical(level);

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = sch_send_mbox_rsp(packet->channel, (const uint32_t *)&packet->tag, 2, MB_DATA_NOTE_BIT);
        }
    }

    return ret;
}

static void sch_handle_packet(cmd_packet_st *packet, const cmd_addr_st *cmd_addr, uint32_t channel)
{
    uint32_t ret;

    if ((NULL != packet) && (NULL != cmd_addr)) {
        ret = mmap_read_remote_data((void *)&packet->cmd_data, cmd_addr->req_addr, MAX_CMD_DATA_SIZE * 4U);
        packet->rsp_addr = cmd_addr->rsp_addr;
        packet->channel = channel;
        if (EHSM_ERR_SW_SUCCESS == ret) {
            packet->rsp_data.ret_code = mbcmdpars_parse_cmd(packet);
            if (packet->rsp_data.ret_code == EHSM_ERR_SW_SUCCESS) {
                packet->rsp_data.ret_code = EHSM_ERR_MB_SUCCESS;
            }
        } else {
            packet->rsp_data.ret_code = ret;
        }
        ret = sch_send_gen_mbox_rsp(packet);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            log_error("mbox rsp send fail: 0x%x\n", ret);
        }
    }
}

static void sch_handle_mb_data(uint32_t mb_data[MB_INFO_SIZE], uint32_t mb_chl)
{
    uint32_t ret;
    raddr_t addr = 0;
    cmd_addr_st cmd_addr;
    cmd_packet_st packet;

    util_memset(&packet, 0, sizeof(cmd_packet_st));
    watchdog_feed();
    util_memcpy(&addr, &mb_data[0], sizeof(raddr_t));
    ret = mmap_read_remote_data((void *)&cmd_addr, addr, sizeof(cmd_addr_st));
    if (EHSM_ERR_SW_SUCCESS == ret) {
        packet.tag = addr;
        sch_handle_packet(&packet, (const cmd_addr_st *)&cmd_addr, mb_chl);
    }
    watchdog_feed();
}

static void sch_poll_mb_chl(uint32_t mb_chl)
{
    uint32_t notify;
    uint32_t ret;
    uint32_t mb_data[MB_INFO_SIZE];

    ret = mailbox_read_recv_notify(mb_chl, &notify);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((notify & MB_DATA_NOTE_BIT) != 0UL) {
            ret = mailbox_read(mb_chl, mb_data, MB_INFO_SIZE);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                sch_handle_mb_data(mb_data, mb_chl);
            }
            (void)mailbox_clear_recv_notify(mb_chl, MB_DATA_NOTE_BIT);
        }
    }
}


/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t sch_start(void)
{
    uint32_t mb_chl = 0;

    while (true) {
        watchdog_feed();
        for (mb_chl = 0; mb_chl <= MAX_MB_CHL; mb_chl++) {
            sch_poll_mb_chl(mb_chl);
        }

        secboot_boot_fw();
    }
}

/**
 *
 */
