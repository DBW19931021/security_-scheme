/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "comm.h"
#include "schedule/schedule.h"
#include "driver/mailbox_driver.h"
#include "driver/uart_driver.h"
#include "component/cmd_pool.h"
#include "mmap.h"
#include "dbgcmd_parser.h"
#include "schedule/expt_det.h"
#include "util.h"
#include "cpu_porting.h"
#include "types.h"
#include "mb.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define COMM_DBG_CHANNEL_NONE 0U
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
#ifndef CONFIG_UNIT_TEST
typedef struct {
    raddr_t req_addr;
    raddr_t rsp_addr;
} cmd_addr_st;
#endif
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static dbgcmdparser_st g_dbg_cmd_parser[1];
static volatile uint32_t g_dbg_channel;
static volatile uint32_t g_mb_int = 0;
static volatile uint32_t g_recv_mb_ch = 0;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static void comm_handle_gen_mbox_data(const uint32_t mb_data[MAILBOX_S2H_INFO_SIZE], uint32_t channel);
static void comm_process_dbg_data(void);
static bool_t comm_is_mb_channel(uint32_t channel);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void comm_read_mbox_data(uint32_t channel)
{
    uint32_t mb_data[MAILBOX_S2H_INFO_SIZE];
    uint32_t ret;

    ret = mailbox_read(channel, mb_data, MAILBOX_S2H_INFO_SIZE);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        comm_handle_gen_mbox_data(mb_data, channel);
    }
}

static void comm_mbox_recv_cb(uint32_t mb_ch_bitmap)
{
    // The mailbox interrupt flag stored in g_mb_int,
    // the mailbox command will be read in comm_poll
    g_mb_int |= mb_ch_bitmap;
}

static void comm_uart_recv_cb(void)
{
    uint8_t data;
    uint32_t ret;

    data = uart_getc();
    if ((g_dbg_channel == UART00) || (g_dbg_channel == COMM_DBG_CHANNEL_NONE)) {
        g_dbg_channel = UART00;
        ret = dbgcmdpars_append_data(g_dbg_cmd_parser, (uint8_t *)&data, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            // debug command will process in comm_poll
        } else if (EHSM_ERR_DBG_BUF_OVERFLOW == ret) {
            // case of EHSM_ERR_DBG_BUF_OVERFLOW
            (void)expt_det_add_error(FW_ERROR_UART_DBG_PROT_ERROR);
            (void)dbgcmdpars_init_parser(g_dbg_cmd_parser);
            g_dbg_channel = COMM_DBG_CHANNEL_NONE;
        } else {
            // case of EHSM_ERR_DBG_PROCESSING, do nothing
        }
    }
}

static uint32_t comm_send_mbox_rsp(uint32_t mbox_chl, const uint32_t *data, uint32_t size)
{
    uint32_t ret;
    uint32_t notify = 0;

    if ((mbox_chl > MAX_MAILBOX_CHANNEL_ID) || (NULL == data) || (0U == size)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = mailbox_read_send_notify(mbox_chl, &notify);
        while ((EHSM_ERR_SW_SUCCESS == ret) && ((notify & MB_DATA_NOTE_BIT) != 0UL)) {
            ret = mailbox_read_send_notify(mbox_chl, &notify);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mailbox_write(mbox_chl, data, size);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            (void)mailbox_set_send_notify(mbox_chl, MB_DATA_NOTE_BIT);
        } else {
            (void)mailbox_set_send_notify(mbox_chl, MB_DATA_NOTE_BIT | MB_CMD_WRITE_FAIL_BIT);
        }
    }
    return ret;
}

static uint32_t comm_send_dbg_rsp(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t index = 0;
    uint32_t level;
    uint32_t dbg_chl;

    level = cpu_enter_critical();
    dbg_chl = g_dbg_channel;
    (void)cpu_exit_critical(level);

    if (g_dbg_cmd_parser->rsp_size <= MAX_DBG_PROTO_SIZE) {
        if (dbg_chl == UART00) {
            while (index < g_dbg_cmd_parser->rsp_size) {
                uart_putc(g_dbg_cmd_parser->cmd_buffer[index]);
                index++;
            }
        }
    } else {
        ret = EHSM_ERR_DBG_BUF_OVERFLOW;
    }

    (void)dbgcmdpars_init_parser(g_dbg_cmd_parser);
    level = cpu_enter_critical();
    g_dbg_channel = COMM_DBG_CHANNEL_NONE;
    (void)cpu_exit_critical(level);
    return ret;
}

static bool_t comm_is_mb_channel(uint32_t channel)
{
    bool_t ret;

    if ((channel >= MAILBOX00) && (channel <= MAILBOX15)) {
        ret = true;
    } else {
        ret = false;
    }

    return ret;
}

static void comm_process_dbg_data(void)
{
    uint32_t ret;
    cmd_packet_st *packet;
    uint32_t error;
    uint32_t level;
    uint32_t dbg_chl;

    level = cpu_enter_critical();
    dbg_chl = g_dbg_channel;
    (void)cpu_exit_critical(level);

    if (dbg_chl == UART00) {
        error = FW_ERROR_UART_DBG_PROT_ERROR;

        ret = dbgcmdpars_recv_complete(g_dbg_cmd_parser);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmdpool_alloc(&packet);
        } else if (EHSM_ERR_DBG_CMD_INCOMPLETE == ret) {
            // debug cmd received incomplete do nothing
        } else {
            (void)expt_det_add_error(error);
            level = cpu_enter_critical();
            g_dbg_channel = COMM_DBG_CHANNEL_NONE;
            (void)cpu_exit_critical(level);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = dbgcmdpars_trans_to_packet(g_dbg_cmd_parser, packet);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                level = cpu_enter_critical();
                g_dbg_channel = COMM_DBG_CHANNEL_NONE;
                (void)cpu_exit_critical(level);
                (void)cmdpool_free(packet);
            }
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            packet->channel = dbg_chl;
            packet->type = PACKET_TYPE_DBG_AUTH;
            ret = sch_add_cmd(packet);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                level = cpu_enter_critical();
                g_dbg_channel = COMM_DBG_CHANNEL_NONE;
                (void)cpu_exit_critical(level);
                (void)dbgcmdpars_init_parser(g_dbg_cmd_parser);
                (void)cmdpool_free(packet);
            }
        }
    }
}


// check mailbox command size
#if MB_MAX_COMMAND_SIZE > (MAX_CMD_RSP_WORD_LEN * 4)
#error MAX_CMD_RSP_WORD_LEN is too small!
#endif

static void comm_init_packet(cmd_packet_st *packet, const cmd_addr_st *cmd_addr, uint32_t channel, raddr_t cmd_tag)
{
    uint32_t ret;
    uint32_t mb_data[MAILBOX_S2H_INFO_SIZE];
    uint32_t notify;
    raddr_t cmd_tag_b = 0;

    if ((NULL != packet) && (NULL != cmd_addr)) {
        ret = mmap_read_remote_data((void *)&packet->cmd_data, cmd_addr->req_addr, MAX_CMD_RSP_WORD_LEN * 4U);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            packet->rsp_addr = cmd_addr->rsp_addr;
            packet->tag = cmd_tag;
            packet->channel = COMM_TYPE_MAILBOX + channel;
            packet->type = PACKET_TYPE_MB;
            ret = sch_add_cmd(packet);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // Mailbox data has been read and added into schedule successfully, clear the bitmap
                g_recv_mb_ch &= (~((uint32_t)1u << channel));
                (void)mailbox_clear_recv_notify(channel, MB_DATA_NOTE_BIT);
                ret = mailbox_read_recv_notify(channel, &notify);
                if ((EHSM_ERR_SW_SUCCESS == ret) && ((notify & MB_DATA_NOTE_BIT) != 0UL)) {
                    ret = mailbox_read(channel, mb_data, MAILBOX_S2H_INFO_SIZE);
                    util_memcpy(&cmd_tag_b, &mb_data[0], sizeof(raddr_t));
                }

                if ((EHSM_ERR_SW_SUCCESS == ret) && (packet->tag == cmd_tag_b)) {
                    (void)expt_det_add_error(FW_ERROR_MB_NOTE_CLR_FAILED);
                }
            } else {
                (void)cmdpool_free(packet);
            }
        } else {
            (void)mailbox_set_send_notify(channel, MB_DATA_NOTE_BIT | MB_CMD_READ_FAIL_BIT);
            (void)cmdpool_free(packet);
        }
    }
}

static void comm_handle_gen_mbox_data(const uint32_t mb_data[MAILBOX_S2H_INFO_SIZE], uint32_t channel)
{
    cmd_addr_st cmd_addr;
    uint32_t ret;
    raddr_t addr = 0;
    cmd_packet_st *packet;

    if (false == sch_is_cache_full(COMM_TYPE_MAILBOX + channel)) {
        util_memcpy(&addr, &mb_data[0], sizeof(raddr_t));
        ret = mmap_read_remote_data((void *)&cmd_addr, addr, sizeof(cmd_addr_st));
        if ((EHSM_ERR_SW_SUCCESS == ret) && (cmd_addr.req_addr != 0U) && (cmd_addr.rsp_addr != 0U)) {
            ret = cmdpool_alloc(&packet);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                comm_init_packet(packet, (const cmd_addr_st *)&cmd_addr, channel, addr);
            }
        } else {
            (void)mailbox_set_send_notify(channel, MB_DATA_NOTE_BIT | MB_CMD_READ_FAIL_BIT);
        }
    }
    return;
}

static uint32_t comm_send_gen_mbox_rsp(const cmd_packet_st *packet)
{
    uint32_t ret;
    uint32_t level;
    uint8_t *remap_addr;
    uint32_t mbox_chl;
    uint32_t tag[2];

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (packet->rsp_data.rsp_data_len > sizeof(packet->rsp_data.data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        level = cpu_enter_critical();
        remap_addr = mmap_remap_addr_u64(packet->rsp_addr);
        if (NULL != remap_addr) {
            util_memcpy(remap_addr, &packet->rsp_data.ret_code, 4);
            util_memcpy((void *)&remap_addr[4], packet->rsp_data.data, packet->rsp_data.rsp_data_len);
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_REMAP_FAILED;
        }
        (void)cpu_exit_critical(level);

        mbox_chl = packet->channel - COMM_TYPE_MAILBOX;
        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy(tag, &packet->tag, sizeof(tag));
            ret = comm_send_mbox_rsp(mbox_chl, (const uint32_t *)tag, 2);
        }
    }

    return ret;
}

static void comm_poll_mb_chl(void)
{
    uint32_t i = 0U;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t notify;

    for (i = 0U; i <= MAX_MAILBOX_CHANNEL_ID; i++) {
        if ((g_recv_mb_ch & (0x1UL << i)) != 0UL) {
            ret = mailbox_read_recv_notify(i, &notify);
            if ((EHSM_ERR_SW_SUCCESS == ret) && ((notify & MB_DATA_NOTE_BIT) != 0UL)) {
                (void)expt_det_reset_error(FW_ERROR_MB_INT_NO_DATA);
                comm_read_mbox_data(i);
            } else {
                g_recv_mb_ch &= (~((uint32_t)1u << i));
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    (void)expt_det_add_error(FW_ERROR_MB_INT_NO_DATA);
                } else {
                    /*No action required*/
                }
            }
        }
    }
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t comm_init(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = cmdpool_init();
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = uart_init(comm_uart_recv_cb, CONFIG_EHSM_CPU_FREQ_HZ / CONFIG_EHSM_DEFAULT_UART_BAUDRATE);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mailbox_init(comm_mbox_recv_cb);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = dbgcmdpars_init_parser(g_dbg_cmd_parser);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        g_dbg_channel = COMM_DBG_CHANNEL_NONE;
    }

    return ret;
}

uint32_t comm_send_rsp(cmd_packet_st *packet)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (packet->channel == UART00) {
        ret = comm_send_dbg_rsp();
    } else if (comm_is_mb_channel(packet->channel)) {
        if (packet->type == PACKET_TYPE_MB) {
            ret = comm_send_gen_mbox_rsp(packet);
        } else {
            ret = comm_send_dbg_rsp();
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (NULL != packet) {
        util_memset(packet, 0, sizeof(cmd_packet_st));
        (void)cmdpool_free(packet);
    }

    return ret;
}

void comm_poll(void)
{
    uint32_t mb_int = 0;
    uint32_t level;

    level = cpu_enter_critical();
    mb_int = g_mb_int;
    g_mb_int = 0;
    g_recv_mb_ch |= mb_int;
    (void)cpu_exit_critical(level);
    if (g_recv_mb_ch != 0U) {
        comm_poll_mb_chl();
    }
    comm_process_dbg_data();
}
/**
 *
 */
