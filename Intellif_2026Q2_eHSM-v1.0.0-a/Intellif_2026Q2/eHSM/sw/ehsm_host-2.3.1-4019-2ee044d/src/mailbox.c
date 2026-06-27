#include "ehsmdrv/basic/mailbox.h"
#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "types_internal.h"

#define MBOX_SOCBASE               (EHSM_PORT_MAILBOX_REG_BASE_ADDR)
#define MAILBOX_CHANNEL_REG_OFFSET 0x1000UL
#define H2S_SRV_GENERAL_NOTE_BIT   ((uint32_t)0x01 << 0)

#if EHSM_PORT_MAILBOX_NOTE_WRITE_1_CLEAR
#define MBOX_CLEAR_BITS 0xffffffffu
#else
#define MBOX_CLEAR_BITS 0x00000000u
#endif

// Number of supported channels of mailbox
#define MAX_MAILBOX_CHANNEL_ID EHSM_PORT_MAILBOX_CHANNEL_COUNT

typedef struct {
    volatile uint32_t s2h_info[2];
    volatile uint32_t reserv1[30];
    volatile uint32_t h2s_info[2];
    volatile uint32_t reserv2[30];
    volatile uint32_t s2h_note;
    volatile uint32_t h2s_note;
    volatile uint32_t reserv3[2];
    volatile uint32_t s2h_soc_int;
    volatile uint32_t s2h_soc_int_en;
    volatile uint32_t s2h_hsm_int;
    volatile uint32_t s2h_hsm_int_en;
    volatile uint32_t h2s_soc_int;
    volatile uint32_t h2s_soc_int_en;
    volatile uint32_t h2s_hsm_int;
    volatile uint32_t h2s_hsm_int_en;
} mailbox_reg_st;

static inline volatile mailbox_reg_st *mailbox_get_reg_base(uint32_t channel)
{
    return (volatile mailbox_reg_st *)(MBOX_SOCBASE + (MAILBOX_CHANNEL_REG_OFFSET * channel));
}

static volatile ehsm_mb_interrupt_func_t s_mb_callback = NULL;

// must called by interrupt
void ehsm_mb_int_handler(uint32_t channel)
{
    if (channel >= MAX_MAILBOX_CHANNEL_ID) {
        return;
    }
    volatile mailbox_reg_st *mb_reg = mailbox_get_reg_base(channel);
    if (mb_reg->h2s_note && s_mb_callback) {
        raddr_t packet_addr = (uint64_t)mb_reg->h2s_info[0] | ((uint64_t)mb_reg->h2s_info[1] << 32);
        s_mb_callback(channel, packet_addr);
    }

    // clear note bit
    mb_reg->h2s_note = MBOX_CLEAR_BITS;
    mb_reg->h2s_soc_int = MBOX_CLEAR_BITS;
}

uint32_t ehsm_mb_init(ehsm_mb_interrupt_func_t func, ehsm_drv_mode_e drv_mode)
{
    uint32_t i;
    volatile mailbox_reg_st *mb_reg;

    if (func) {
        s_mb_callback = func;
    }

    for (i = 0; i < MAX_MAILBOX_CHANNEL_ID; i++) {
        mb_reg = mailbox_get_reg_base(i);
        switch (drv_mode) {
        case EHSM_DRV_MODE_INTERRUPT: {
            // enable recv interrupt
            mb_reg->h2s_soc_int_en = 0xffffffffu;

            // do not enable s2h interrupt
            mb_reg->s2h_soc_int_en = 0x0u;
            // clear note bit and interrupt bit
            mb_reg->h2s_note = MBOX_CLEAR_BITS;
            mb_reg->h2s_soc_int = MBOX_CLEAR_BITS;

            uint32_t ret = ehsm_port_enable_mailbox_int(i, ehsm_mb_int_handler);
            if (ret != EHSM_OK) {
                return ret;
            }
            break;
        }
        case EHSM_DRV_MODE_WAIT_AND_POLL:
            // do not enable recv interrupt if use poll/only_send mode
            mb_reg->h2s_soc_int_en = 0x0u;

            mb_reg->s2h_soc_int_en = 0x0u;
            // clear note bit and interrupt bit
            mb_reg->h2s_note = MBOX_CLEAR_BITS;
            mb_reg->h2s_soc_int = MBOX_CLEAR_BITS;
            break;
        case EHSM_DRV_MODE_SEND_AND_PEEK:
        default:
            // nothing to do
            break;
        }
    }

    return EHSM_OK;
}

uint32_t ehsm_mb_send(uint32_t channel, raddr_t packet_addr)
{
    if (channel >= MAX_MAILBOX_CHANNEL_ID) {
        return EHSM_ERR_PARAM_ERROR;
    }
    volatile mailbox_reg_st *mb_reg = mailbox_get_reg_base(channel);

    ehsm_port_timer_t t = ehsm_port_create_timer();
    while (mb_reg->s2h_note & H2S_SRV_GENERAL_NOTE_BIT) {
        if (ehsm_port_is_timeout(t)) {
            return EHSM_ERR_TIMEOUT;
        }
    }

    ehsm_port_flush_and_invalidate_cache();

    ehsm_port_enter_critical();
    if ((mb_reg->s2h_note & H2S_SRV_GENERAL_NOTE_BIT) == 0U) {
        mb_reg->s2h_info[0] = (uint32_t)packet_addr;
        mb_reg->s2h_info[1] = (uint32_t)(packet_addr >> 32);
        mb_reg->s2h_note = H2S_SRV_GENERAL_NOTE_BIT;

    } else {
        ehsm_port_exit_critical();
        return EHSM_ERR_BUSY;
    }
    ehsm_port_exit_critical();

    return EHSM_OK;
}

uint32_t ehsm_mb_poll(uint32_t channel)
{
    if (channel >= MAX_MAILBOX_CHANNEL_ID) {
        return EHSM_ERR_PARAM_ERROR;
    }
    volatile mailbox_reg_st *mb_reg = mailbox_get_reg_base(channel);

    // check h2s notify bit
    if ((mb_reg->h2s_note & H2S_SRV_GENERAL_NOTE_BIT) == 0) {
        return EHSM_ERR_NEED_POLL;
    }

    // response is received
    ehsm_mb_int_handler(channel);
    return EHSM_OK;
}
