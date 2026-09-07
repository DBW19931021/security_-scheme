#include "strans_pro.h"
#include "uart_hal.h"
#include "host_m130_cfg.h"
#include <stdint.h>

#define UART1_BASE_ADDR 0x50005000UL

//RX interrupt bit
#define UART_RX_INT_BIT                         ((uint32_t)0x01U << 1)
//RX enable bit
#define UART_RX_ENABLE_BIT                      (uint32_t)(0x01U << 1)
//RX interrupt enable bit
#define UART_RX_INT_ENABLE_BIT                  (uint32_t)(0x01U << 3)
//RX overflow interrupt enable bit
#define UART_RX_OFLOW_INT_ENABLE_BIT            (uint32_t)(0x01U << 5)
//TX enable bit
#define UART_TX_ENABLE_BIT                      (uint32_t)(0x01U << 0)
//TX buffer empty bit
#define UART_TX_BUF_EMPTY_BIT                    ((uint32_t)0x01U << 0)


typedef struct {
    uint32_t data;
    uint32_t status;
    uint32_t ctrl;
    uint32_t int_status;
    uint32_t baudrate;
} uart_reg_st;



static inline volatile uart_reg_st* uart_get_reg_base(void)
{
    return (volatile uart_reg_st*)UART1_BASE_ADDR;
}


#define UART_REG_BASE uart_get_reg_base()


uint32_t uart_init(void)
{
    UART_REG_BASE->baudrate = CONFIG_EHSM_HW_HOST_CPU_FREQ / 115200;
    UART_REG_BASE->ctrl = 0U;
    UART_REG_BASE->int_status = 0xFF;
    UART_REG_BASE->ctrl |= (UART_RX_ENABLE_BIT | UART_TX_ENABLE_BIT);
    return STP_OK;
}

uint32_t uart_send_byte(uint8_t val)
{
    while ((UART_REG_BASE->status & 1)) { } // Wait if Transmit Holding register is full
    UART_REG_BASE->data = val;             // write to transmit holding register

    return STP_OK;
}

uint32_t uart_peek_byte(uint8_t *buf)
{
    if ((UART_REG_BASE->status & 2) == 0) {
        return STP_ERR_PEEK_NO_DATA;
    } else {
        uint32_t val = UART_REG_BASE->data;
        *buf = (uint8_t)val;
        return STP_OK;
    }
}

void uart_deinit(void)
{
    UART_REG_BASE->ctrl = 0;
}
