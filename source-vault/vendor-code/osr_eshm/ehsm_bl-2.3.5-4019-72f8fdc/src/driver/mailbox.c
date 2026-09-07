/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mailbox.h"
#include "cpu_porting.h"
#include "component/util.h"

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
    uint32_t s2h_info[2];
    uint32_t reserv1[30];
    uint32_t h2s_info[2];
    uint32_t reserv2[30];
    uint32_t s2h_note;
    uint32_t h2s_note;
    uint32_t reserv3[2];
    uint32_t s2h_soc_int;
    uint32_t s2h_soc_int_en;
    uint32_t s2h_hsm_int;
    uint32_t s2h_hsm_int_en;
    uint32_t h2s_soc_int;
    uint32_t h2s_soc_int_en;
    uint32_t h2s_hsm_int;
    uint32_t h2s_hsm_int_en;
} mailbox_reg_st;

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
#if defined(CONFIG_UNIT_TEST)
volatile mailbox_reg_st mb_reg_array[0x10];
static inline volatile mailbox_reg_st *mailbox_get_reg_base(uint32_t channel)
{
    return (volatile mailbox_reg_st *)&mb_reg_array[channel];
}
static uint32_t g_mbox_int;
#define MAILBOX_REG_FOR_ALL_INT *((volatile uint32_t *)(&g_mbox_int))
#else
static inline volatile mailbox_reg_st *mailbox_get_reg_base(uint32_t channel)
{
    return (volatile mailbox_reg_st *)(MAILBOX_REG_BASE_ADDRESS + (MAILBOX_CHANNEL_REG_OFFSET * channel));
}
#endif

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void mailbox_init(void)
{
    volatile mailbox_reg_st *mb_reg;
    uint32_t i;

    cpu_disable_int(IRQ_TYPE_MAILBOX);
    for (i = 0; i <= MAX_MAILBOX_CHANNEL_ID; i++) {
        mb_reg = mailbox_get_reg_base(i);
        mb_reg->h2s_hsm_int_en = 0x0U;
        mb_reg->s2h_hsm_int_en = 0x0U;
    }
}

uint32_t mailbox_read(uint32_t channel, uint32_t *data, uint32_t size)
{
    uint32_t ret;
    const volatile mailbox_reg_st *mb_reg;

    if ((channel > MAX_MAILBOX_CHANNEL_ID) || (NULL == data) || (0U == size) || (size > MAILBOX_S2H_INFO_SIZE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        util_read_volatile_u32(data, &mb_reg->s2h_info[0], size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t mailbox_write(uint32_t channel, const uint32_t *data, uint32_t size)
{
    uint32_t ret;
    volatile mailbox_reg_st *mb_reg;

    if ((channel > MAX_MAILBOX_CHANNEL_ID) || (NULL == data) || (0U == size) || (size > MAILBOX_H2S_INFO_SIZE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        util_write_volatile_u32(&mb_reg->h2s_info[0], data, size);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t mailbox_read_send_notify(uint32_t channel, uint32_t *notify)
{
    uint32_t ret;
    const volatile mailbox_reg_st *mb_reg;

    if ((channel > MAX_MAILBOX_CHANNEL_ID) || (NULL == notify)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        *notify = mb_reg->h2s_note;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t mailbox_set_send_notify(uint32_t channel, uint32_t notify)
{
    uint32_t ret;
    volatile mailbox_reg_st *mb_reg;

    if (channel > MAX_MAILBOX_CHANNEL_ID) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        mb_reg->h2s_note = notify;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t mailbox_read_recv_notify(uint32_t channel, uint32_t *notify)
{
    uint32_t ret;
    const volatile mailbox_reg_st *mb_reg;

    if ((channel > MAX_MAILBOX_CHANNEL_ID) || (NULL == notify)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        *notify = mb_reg->s2h_note;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

uint32_t mailbox_clear_recv_notify(uint32_t channel, uint32_t notify)
{
    uint32_t ret;
    volatile mailbox_reg_st *mb_reg;

    if (channel > MAX_MAILBOX_CHANNEL_ID) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        mb_reg->s2h_note = notify;
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}
/**
 *
 */
