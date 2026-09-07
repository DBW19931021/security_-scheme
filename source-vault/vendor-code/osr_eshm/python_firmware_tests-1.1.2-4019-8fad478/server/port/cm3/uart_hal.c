#include "strans_pro.h"
#include "uart_hal.h"
#include "CM3DS_MPS2.h"
#include "host_cm3_cfg.h"

uint32_t uart_init()
{
    CM3DS_MPS2_UART1->BAUDDIV = CONFIG_EHSM_HW_HOST_CPU_FREQ / 115200U;
    CM3DS_MPS2_UART1->CTRL = CM3DS_MPS2_UART_CTRL_TXEN_Msk | CM3DS_MPS2_UART_CTRL_RXEN_Msk; // TX RX enable
    return STP_OK;
}

uint32_t uart_send_byte(uint8_t val)
{
    while ((CM3DS_MPS2_UART1->STATE & 1)) { } // Wait if Transmit Holding register is full
    CM3DS_MPS2_UART1->DATA = val;             // write to transmit holding register

    return STP_OK;
}

uint32_t uart_peek_byte(uint8_t *buf)
{
    if ((CM3DS_MPS2_UART1->STATE & 2) == 0) {
        return STP_ERR_PEEK_NO_DATA;
    } else {
        uint32_t val = CM3DS_MPS2_UART1->DATA;
        *buf = (uint8_t)val;
        return STP_OK;
    }
}

void uart_deinit(void)
{
    CM3DS_MPS2_UART1->CTRL = 0;
}
