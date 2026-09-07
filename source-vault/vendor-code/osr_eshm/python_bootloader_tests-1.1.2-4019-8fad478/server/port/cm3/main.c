#include "CM3DS_MPS2.h"
#include "uart_stdout.h"
#include <stdio.h>
#include <string.h>

#include "host_cm3_cfg.h"

#include "ehsmdrv/basic/mailbox.h"

// MAILBOX interrupt handlers are now defined in ehsm_host_port.c

void OSR_INT_Enable(IRQn_Type irq)
{
    /* Clear the pending for avoiding jump to the handler immediately */
    NVIC_ClearPendingIRQ(irq);
    /* Set priority. The priority is 0 to 255, 0 is the higest priority */
    NVIC_SetPriority(irq, 1);
    /* Enable the IRQ */
    NVIC_EnableIRQ(irq);
}

volatile uint32_t timeTicks;

// System timer ISR, one tick is 0.1 ms.
void SysTick_Handler(void)
{
    timeTicks++;
}

uint32_t stp_get_time_ms(void)
{
    return timeTicks / 10U;
}

void init_mtime(void)
{
}

uint32_t get_time_tick(void)
{
    return timeTicks;
}

uint32_t calc_time_unit(uint32_t ticks)
{
    return ticks == 0U ? 1U : ticks;
}

void ehsm_mb_enable_int(uint32_t channel)
{
    OSR_INT_Enable((IRQn_Type)((uint32_t)Mailbox0_IRQn + channel));
}

void server_main(void);

int main(void)
{
    *((volatile unsigned int *)(0x40010058)) = 0x0fa66e27; // OSR_FPGA_MAGIC

    UartStdOutInit(115200);
    printf("ehsm test system cm3 server start\n");
    printf("Build Date: %s %s\r\n", __DATE__, __TIME__);
    timeTicks = 0;
    SysTick_Config(CONFIG_EHSM_HW_HOST_CPU_FREQ / 10000U);

    server_main();

    while (1) { }

    return 0;
}
