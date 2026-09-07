/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2010-2017  ARM Limited or its affiliates.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *      Checked In          : $Date: 2013-04-10 15:14:20 +0100 (Wed, 10 Apr 2013) $
 *
 *      Revision            : $Revision: 243501 $
 *
 *      Release Information : CM3DesignStart-r0p0-01rel0
 *-----------------------------------------------------------------------------
 */

 /*

 UART functions for retargetting

 */

#include <stdio.h>
#include <stdint.h>

#include "host_m130_cfg.h"

#include "uart_stdout.h"
#define UART_BASE_ADDR 0x50004000UL;

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
    return (volatile uart_reg_st*)UART_BASE_ADDR;
}


#define UART_REG_BASE uart_get_reg_base()

void UartStdOutInit(unsigned int baudrate)
{
    UART_REG_BASE->baudrate = CONFIG_EHSM_HW_HOST_CPU_FREQ / baudrate;
    UART_REG_BASE->ctrl = 0U;
    UART_REG_BASE->int_status = 0xFF;
    UART_REG_BASE->ctrl |= (UART_RX_ENABLE_BIT | UART_RX_INT_ENABLE_BIT | UART_RX_OFLOW_INT_ENABLE_BIT);
    UART_REG_BASE->ctrl |= UART_TX_ENABLE_BIT;
}
// Output a character
unsigned char UartPutc(unsigned char my_ch)
{
    while ((UART_REG_BASE->status & UART_TX_BUF_EMPTY_BIT) == UART_TX_BUF_EMPTY_BIT){
    }
    UART_REG_BASE->data = my_ch;
  return (my_ch);
}
// Get a character
unsigned char UartGetc(void)
{
    uint8_t c = (uint8_t)(UART_REG_BASE->data);

    return c;
}