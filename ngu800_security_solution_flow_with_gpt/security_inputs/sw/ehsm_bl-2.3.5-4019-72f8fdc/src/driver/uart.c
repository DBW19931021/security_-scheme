/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "uart.h"
#include "cpu_porting.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define UART_SEND_ASYNC_MODE_ENABLE 0
#define UART_NEED_INTERRPT          (0x0 || UART_SEND_ASYNC_MODE_ENABLE)

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
// TX interrupt enable bit
#define UART_TX_INT_ENABLE_BIT (uint32_t)(0x01U << 2)
// TX buffer full bit
#define UART_TX_BUF_FULL_BIT ((uint32_t)0x01U << 0)

#define UART_TX_INT_BIT ((uint32_t)0x01U << 0)

#define UART_RX_BUF_FULL_BIT ((uint32_t)0x01U << 1)

#define UART_TX_OFLOW_INT_ENABLE_BIT (uint32_t)(0x01U << 4)
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

#if UART_SEND_ASYNC_MODE_ENABLE
#define UART_TX_BUF_SIZE 512
static volatile uint32_t uart_tx_front = 0;
static volatile uint32_t uart_tx_end = 0;
static volatile uint8_t uart_tx_buf[UART_TX_BUF_SIZE];
#endif // UART_SEND_ASYNC_MODE_ENABLE

#if UART_NEED_INTERRPT
ATTR_INTERRUPT static void uart_int_handler(void)
{
    uint32_t int_status = UART_REG_BASE->int_status;

#if UART_SEND_ASYNC_MODE_ENABLE
    if ((int_status & UART_TX_INT_BIT) && !(UART_REG_BASE->status & UART_TX_BUF_FULL_BIT)) {
        // send data if buf is not empty
        if (uart_tx_front != uart_tx_end) {
            UART_REG_BASE->data = uart_tx_buf[uart_tx_end];
            uart_tx_end = (uart_tx_end + 1) % UART_TX_BUF_SIZE;
        }
    }
#endif // UART_SEND_ASYNC_MODE_ENABLE

    UART_REG_BASE->int_status = 0xFFU;
}
#endif // UART_NEED_INTERRPT

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t uart_init(uart_recv_cb cb, uint32_t baudrate_div)
{
    uint32_t ret;

    if (NULL == cb) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {

#if UART_NEED_INTERRPT
        ret = cpu_register_int(IRQ_TYPE_UART, INT_POSITIVE_EDGE_TRIGGER, (cpu_int_handler)uart_int_handler);
#else
        ret = EHSM_ERR_SW_SUCCESS;
#endif
        if (EHSM_ERR_SW_SUCCESS == ret) {
            g_uart_cb = cb;

            UART_REG_BASE->ctrl = 0U;
            UART_REG_BASE->int_status = 0xFF;

            UART_REG_BASE->ctrl |= UART_TX_ENABLE_BIT;
#if UART_SEND_ASYNC_MODE_ENABLE
            UART_REG_BASE->ctrl |= UART_TX_INT_ENABLE_BIT;
#endif
            uart_set_baudrate(baudrate_div);
#if UART_NEED_INTERRPT
            cpu_enable_int(IRQ_TYPE_UART);
#endif
        }
    }

    return ret;
}

#if UART_SEND_ASYNC_MODE_ENABLE
static void uart_try_tx_none_int(void)
{
    if ((UART_REG_BASE->status & UART_TX_BUF_FULL_BIT) == 0) {
        if (UART_REG_BASE->status & UART_TX_BUF_FULL_BIT) {
            // double check
            return;
        }
        uint8_t c = uart_tx_buf[uart_tx_end];
        uart_tx_end = (uart_tx_end + 1) % UART_TX_BUF_SIZE;
        UART_REG_BASE->data = c;
    }
}
#endif // UART_SEND_ASYNC_MODE_ENABLE

#if UART_SEND_ASYNC_MODE_ENABLE
void uart_putc(uint8_t c)
{
    while ((uart_tx_front + 1) % UART_TX_BUF_SIZE == uart_tx_end) {
        uart_try_tx_none_int();
    }

    uart_tx_buf[uart_tx_front] = c;
    uart_tx_front = (uart_tx_front + 1) % UART_TX_BUF_SIZE;
    uart_try_tx_none_int();
}
#else  // !UART_SEND_ASYNC_MODE_ENABLE
void uart_putc(uint8_t c)
{
    while ((UART_REG_BASE->status & UART_TX_BUF_FULL_BIT) != 0) { }
    UART_REG_BASE->data = c;
}
#endif // UART_SEND_ASYNC_MODE_ENABLE

void uart_flush(void)
{
#if UART_SEND_ASYNC_MODE_ENABLE
    while (uart_tx_end != uart_tx_front) {
        uart_try_tx_none_int();
    }
#endif
    while ((UART_REG_BASE->status & UART_TX_BUF_FULL_BIT) != 0) { }
}

uint8_t uart_getc(void)
{
    uint8_t c = (uint8_t)(UART_REG_BASE->data);

    return c;
}

void uart_set_baudrate(uint32_t baudrate_div)
{
    UART_REG_BASE->baudrate = baudrate_div;
}

void uart_deinit(void)
{
    UART_REG_BASE->int_status = 0xFFU;
    UART_REG_BASE->ctrl = 0;
#if UART_NEED_INTERRPT
    cpu_disable_int(IRQ_TYPE_UART);
#endif
}
/**
 *
 */
