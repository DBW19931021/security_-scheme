/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "uart_driver.h"
#include "cpu_porting.h"
#include "reg_lock.h"
#include "types.h"
#include "schedule/expt_det.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// RX interrupt bit
#define UART_RX_INT_BIT ((uint32_t)0x01U << 1)
// RX enable bit
#define UART_RX_ENABLE_BIT (uint32_t)(0x01U << 1)
// RX interrupt enable bit
#define UART_RX_INT_ENABLE_BIT (uint32_t)(0x01U << 3)
// RX overflow interrupt enable bit
#define UART_RX_OFLOW_INT_ENABLE_BIT (uint32_t)(0x01U << 5)
// TX enable bit
#define UART_TX_ENABLE_BIT (uint32_t)(0x01U << 0)
// TX buffer empty bit
#define UART_TX_BUF_EMPTY_BIT ((uint32_t)0x01U << 0)

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint32_t data;
    uint32_t status;
    uint32_t ctrl;
    uint32_t int_status;
    uint32_t baudrate;
} uart_reg_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static uart_recv_cb g_uart_cb = NULL;

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static inline volatile uart_reg_st *uart_get_reg_base(void)
{
    return (volatile uart_reg_st *)UART_BASE_ADDR;
}
#ifdef CONFIG_UNIT_TEST
volatile uart_reg_st g_uart_reg_1;
volatile uart_reg_st *g_uart_reg = &g_uart_reg_1;
#define UART_REG_BASE g_uart_reg
#else
#define UART_REG_BASE uart_get_reg_base()
#endif

#ifdef CONFIG_UNIT_TEST
static void uart_int_handler(void)
#else
static ATTR_INTERRUPT void uart_int_handler(void)
#endif
{
    if ((UART_REG_BASE->int_status & UART_RX_INT_BIT) == UART_RX_INT_BIT) {
        if (NULL != g_uart_cb) {
            g_uart_cb();
        }
        UART_REG_BASE->int_status |= UART_RX_INT_BIT;
    } else {
        UART_REG_BASE->int_status = 0xFFU;
    }
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t uart_init(uart_recv_cb cb, uint32_t baudrate_div)
{
    uint32_t ret;

    if (NULL == cb) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        sysreg_unlock_reg(UART_BASE_ADDR);
        ret = cpu_register_int(IRQ_TYPE_UART, INT_POSTIVE_EDGE_TRIGGER, (cpu_int_handler)uart_int_handler);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            g_uart_cb = cb;

            UART_REG_BASE->ctrl = 0U;
            UART_REG_BASE->int_status = 0xFF;
            UART_REG_BASE->ctrl |= (UART_RX_ENABLE_BIT | UART_RX_INT_ENABLE_BIT | UART_RX_OFLOW_INT_ENABLE_BIT);
            UART_REG_BASE->ctrl |= UART_TX_ENABLE_BIT;
            uint32_t cfg = 0;
            cfg |= (UART_RX_ENABLE_BIT | UART_RX_INT_ENABLE_BIT | UART_RX_OFLOW_INT_ENABLE_BIT | UART_TX_ENABLE_BIT);
            if (UART_REG_BASE->ctrl != cfg) {
                (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
            }
            uart_set_baudrate(baudrate_div);
            (void)cpu_enable_int(IRQ_TYPE_UART);
        }
        sysreg_lock_reg(UART_BASE_ADDR);
    }

    return ret;
}

void uart_putc(uint8_t c)
{
    while ((UART_REG_BASE->status & UART_TX_BUF_EMPTY_BIT) == UART_TX_BUF_EMPTY_BIT) { }
    UART_REG_BASE->data = c;
}

uint8_t uart_getc(void)
{
    uint8_t c = (uint8_t)(UART_REG_BASE->data);

    return c;
}

void uart_set_baudrate(uint32_t baudrate_div)
{
    UART_REG_BASE->baudrate = baudrate_div;
    if (UART_REG_BASE->baudrate != baudrate_div) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}
/**
 *
 */
