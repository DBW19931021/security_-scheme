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
// #include <stdio.h>

#include "CM3DS_MPS2.h"
#include <stdio.h>

#include "host_cm3_cfg.h"

void UartStdOutInit(unsigned int buadrate)
{
    CM3DS_MPS2_UART0->BAUDDIV = CONFIG_EHSM_HW_HOST_CPU_FREQ / buadrate; // SystemCoreClock / buadrate
    CM3DS_MPS2_UART0->CTRL |= CM3DS_MPS2_UART_CTRL_TXEN_Msk;             // TX enable
    CM3DS_MPS2_UART0->CTRL |= CM3DS_MPS2_UART_CTRL_RXEN_Msk;             // RX enable
    CM3DS_MPS2_UART0->CTRL |= CM3DS_MPS2_UART_CTRL_RXIRQEN_Msk;          // RX IRQ enable
}
// Output a character
unsigned char UartPutc(unsigned char my_ch)
{
    while ((CM3DS_MPS2_UART0->STATE & 1))
        ;                           // Wait if Transmit Holding register is full
    CM3DS_MPS2_UART0->DATA = my_ch; // write to transmit holding register
    return (my_ch);
}
// Get a character
unsigned char UartGetc(void)
{
    while ((CM3DS_MPS2_UART0->STATE & 2) == 0)
        ; // Wait if Receive Holding register is empty
    return (CM3DS_MPS2_UART0->DATA);
}

void UartEndSimulation(void)
{
    UartPutc((char)0x4); // End of simulation
    while (1)
        ;
}
