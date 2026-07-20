/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "watchdog_driver.h"
#include "cpu_porting.h"
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
#define WRITE_ENABLE_VALUE 0x7B3E8C5Du

#define REG_WDT_CTRL_EN      (0x1U << 0)
#define REG_WDT_CTRL_INT_EN  (0x1U << 2)
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
#ifdef CONFIG_UNIT_TEST
volatile wdg_reg_st g_wdg_reg;
static inline volatile wdg_reg_st *watchdog_get_reg_base(void)
{
    return &g_wdg_reg;
}
#else
static inline volatile wdg_reg_st *watchdog_get_reg_base(void)
{
    return (volatile wdg_reg_st *)(WATCHDOG_REG_BASE_ADDRESS);
}
#endif

#ifdef CONFIG_UNIT_TEST
static void watchdog_int_handler(void)
#else
static ATTR_INTERRUPT void watchdog_int_handler(void)
#endif
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    if (NULL != g_watchdog_cb) {
        g_watchdog_cb();
    }

    sysreg_unlock_reg(WDT_REG_BASE);
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->isr = 0U;
    wdg_reg->wpt = 0U;
    sysreg_lock_reg(WDT_REG_BASE);
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t watchdog_init(watchdog_timeout_cb cb)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();
    uint32_t ret;

    if (NULL == cb) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpu_register_int(IRQ_TYPE_WDT, INT_LEVEL_TRIGGER, (cpu_int_handler)watchdog_int_handler);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            uint32_t cfg = 0;
            cfg |= REG_WDT_CTRL_INT_EN | REG_WDT_CTRL_DIV_128;
            g_watchdog_cb = cb;
            sysreg_unlock_reg(WDT_REG_BASE);
            wdg_reg->wpt = WRITE_ENABLE_VALUE;
            wdg_reg->ctrl = cfg;
            wdg_reg->isr = 0U;
            wdg_reg->psr = DEFAULT_WDT_CLK_DIV;
            wdg_reg->wpt = 0U;
            sysreg_lock_reg(WDT_REG_BASE);
            if ((wdg_reg->psr != DEFAULT_WDT_CLK_DIV) || (wdg_reg->isr != 0U) || (wdg_reg->ctrl != cfg)) {
                (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
            }
            ret = cpu_enable_int(IRQ_TYPE_WDT);
        }
    }

    return ret;
}

void watchdog_start(uint32_t timeout)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    // auto calc div rate and initial value
    uint32_t psr = DEFAULT_WDT_CLK_DIV;
    uint32_t ldr = timeout * (APB_CLK_RATE / REG_WDT_CTRL_DIV_128_VALUE / DEFAULT_WDT_CLK_DIV / 100U);
    while (ldr > 0xFFFFU) {
        ldr >>= 1;
        psr <<= 1;
        if (psr > 0xFFFFU) {
            (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
            break;
        }
    }

    sysreg_unlock_reg(WDT_REG_BASE);
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->ctrl &= (~REG_WDT_CTRL_EN);
    wdg_reg->psr = psr;
    wdg_reg->ldr = ldr;
    wdg_reg->vdr = 0;
    wdg_reg->ctrl |= REG_WDT_CTRL_EN;
    wdg_reg->wpt = 0;
    sysreg_lock_reg(WDT_REG_BASE);
    if (((wdg_reg->ctrl & REG_WDT_CTRL_EN) != REG_WDT_CTRL_EN) || (wdg_reg->ldr != (ldr & 0xFFFFU))) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}

void watchdog_feed(void)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    sysreg_unlock_reg(WDT_REG_BASE);
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    // Writing any value to ldr triggers hardware counter reload (watchdog feed)
    wdg_reg->ldr = wdg_reg->ldr;
    wdg_reg->wpt = 0;
    sysreg_lock_reg(WDT_REG_BASE);
}

void watchdog_stop(void)
{
    volatile wdg_reg_st *wdg_reg = watchdog_get_reg_base();

    sysreg_unlock_reg(WDT_REG_BASE);
    wdg_reg->wpt = WRITE_ENABLE_VALUE;
    wdg_reg->ctrl &= ~REG_WDT_CTRL_EN;
    wdg_reg->wpt = 0;
    sysreg_lock_reg(WDT_REG_BASE);
    if ((wdg_reg->ctrl & REG_WDT_CTRL_EN) == REG_WDT_CTRL_EN) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}
/**
 *
 */
