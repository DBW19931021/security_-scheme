/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mailbox_driver.h"
#include "cpu_porting.h"
#include "component/util.h"
#include "types.h"
#include "schedule/expt_det.h"
#include "sysreg.h"
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
static mailbox_recv_cb g_mailbox_cb = NULL;

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
volatile mailbox_reg_st mb_reg_array[MAX_MAILBOX_CHANNEL_ID + 1];
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

#ifdef CONFIG_UNIT_TEST
static void mailbox_int_handler(void)
#else
ATTR_INTERRUPT static void mailbox_int_handler(void)
#endif
{
    uint32_t mb_ch_bitmap;
    uint32_t chl = 0;
    volatile mailbox_reg_st *mb_reg;

    mb_ch_bitmap = MAILBOX_REG_FOR_ALL_INT;
    if (NULL != g_mailbox_cb) {
        g_mailbox_cb(mb_ch_bitmap);
    }
    for (chl = 0U; chl <= MAX_MAILBOX_CHANNEL_ID; chl++) {
        if ((mb_ch_bitmap & (0x1UL << chl)) != 0UL) {
            mb_reg = mailbox_get_reg_base(chl);
            sysreg_unlock_reg((uint32_t)mb_reg);
            mb_reg->h2s_hsm_int = 0xFFFFFFFFU;
            mb_reg->s2h_hsm_int = 0xFFFFFFFFU;
            sysreg_lock_reg((uint32_t)mb_reg);
        }
    }
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t mailbox_init(mailbox_recv_cb cb)
{
    uint32_t ret;
    volatile mailbox_reg_st *mb_reg;
    uint32_t i;

    if (NULL == cb) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpu_register_int(IRQ_TYPE_MAILBOX, INT_LEVEL_TRIGGER, (cpu_int_handler)mailbox_int_handler);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpu_enable_int(IRQ_TYPE_MAILBOX);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            g_mailbox_cb = cb;
            for (i = 0; i <= MAX_MAILBOX_CHANNEL_ID; i++) {
                mb_reg = mailbox_get_reg_base(i);
                sysreg_unlock_reg((uint32_t)mb_reg);
                mb_reg->h2s_hsm_int_en = 0x00000000U;
                mb_reg->s2h_hsm_int_en = 0x80000001U;
                sysreg_lock_reg((uint32_t)mb_reg);
                if ((mb_reg->h2s_hsm_int_en != 0x00000000U) || (mb_reg->s2h_hsm_int_en != 0x80000001U)) {
                    (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
                }
            }
        }
    }

    return ret;
}

uint32_t mailbox_read(uint32_t channel, uint32_t *data, uint32_t size)
{
    uint32_t ret;
    const volatile mailbox_reg_st *mb_reg;

    if ((channel > MAX_MAILBOX_CHANNEL_ID) || (NULL == data) || (0U == size) || (size > MAILBOX_S2H_INFO_SIZE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        mb_reg = mailbox_get_reg_base(channel);
        util_read_volatile_u32(data, mb_reg->s2h_info, size);
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
        sysreg_unlock_reg((uint32_t)mb_reg);
        util_write_volatile_u32(mb_reg->h2s_info, data, size);
        sysreg_lock_reg((uint32_t)mb_reg);

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
        sysreg_unlock_reg((uint32_t)mb_reg);
        mb_reg->h2s_note = notify;
        sysreg_lock_reg((uint32_t)mb_reg);
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
        sysreg_unlock_reg((uint32_t)mb_reg);
        mb_reg->s2h_note = notify;
        sysreg_lock_reg((uint32_t)mb_reg);
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}
/**
 *
 */
