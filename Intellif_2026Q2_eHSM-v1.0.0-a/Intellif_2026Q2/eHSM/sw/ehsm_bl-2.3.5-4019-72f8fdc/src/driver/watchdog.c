/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "watchdog.h"
#include "cpu_porting.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define WRITE_ENABLE_VALUE 0x7B3E8C5Du

#define REG_WDT_CTRL_EN       (0x1U << 0)
#define REG_WDT_CTRL_RESET_EN (0x1U << 1)
#define REG_WDT_CTRL_INT_EN   (0x1U << 2)

#define REG_WDT_CTRL_DIV_16  (0x0U << 4U)
#define REG_WDT_CTRL_DIV_32  (0x1U << 4U)
#define REG_WDT_CTRL_DIV_64  (0x2U << 4U)
#define REG_WDT_CTRL_DIV_128 (0x3U << 4U)

#define REG_WDT_CTRL_DIV_128_VALUE 128U
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t ctrl;
    uint32_t psr;
    uint32_t ldr;
    uint32_t vdr;
    uint32_t isr;
    uint32_t rcr;
    uint32_t wpt;
} wdg_reg_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static watchdog_timeout_cb g_watchdog_cb = NULL;
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static inline volatile wdg_reg_st *watchdog_get_reg_base(void)
{
    return (volatile wdg_reg_st *)(WATCHDOG_REG_BASE_ADDRESS);
}

ATTR_INTERRUPT static void watchdog_int_handler(void)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    if (NULL != g_watchdog_cb) {
        g_watchdog_cb();
    }
    wdg_reg->isr = 0U;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t watchdog_init(watchdog_timeout_cb cb)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();
    uint32_t ret;

    ret = cpu_register_int(IRQ_TYPE_WDT, INT_LEVEL_TRIGGER, (cpu_int_handler)watchdog_int_handler);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        g_watchdog_cb = cb;
        wdg_reg->wpt = WRITE_ENABLE_VALUE;
        wdg_reg->ctrl = REG_WDT_CTRL_INT_EN | REG_WDT_CTRL_DIV_128;
        wdg_reg->isr = 0U;
        wdg_reg->psr = DEFAULT_WDT_CLK_DIV;
        wdg_reg->wpt = 0U;
        cpu_enable_int(IRQ_TYPE_WDT);
    }

    return ret;
}

// the timeout unit is 10ms
void watchdog_start(uint32_t timeout)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    // auto calc div rate and initial value
    uint32_t psr = DEFAULT_WDT_CLK_DIV;
    uint32_t ldr = timeout * (APB_CLK_RATE / REG_WDT_CTRL_DIV_128_VALUE / DEFAULT_WDT_CLK_DIV / 100);
    while (ldr > 0xFFFFU) {
        ldr >>= 1;
        psr <<= 1;
    }

    uint32_t level = cpu_enter_critical();
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->ctrl &= (~REG_WDT_CTRL_EN);
    wdg_reg->psr = psr;
    wdg_reg->ldr = ldr;
    wdg_reg->vdr = 0;
    wdg_reg->ctrl |= REG_WDT_CTRL_EN;
    wdg_reg->wpt = 0;
    cpu_exit_critical(level);
}

void watchdog_feed(void)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    uint32_t level = cpu_enter_critical();
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->ldr = wdg_reg->ldr;
    wdg_reg->wpt = 0;
    cpu_exit_critical(level);
}

void watchdog_stop(void)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    uint32_t level = cpu_enter_critical();
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->ctrl &= ~REG_WDT_CTRL_EN;
    wdg_reg->wpt = 0;
    cpu_exit_critical(level);
}
/**
 *
 */
