#include "uart_stdout.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "host_m130_cfg.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/mailbox.h"

#include "cpu_porting.h"

#include "demo.h"

#define OSR_FPAG_FREQ (5000000)
#define MS_TICKS      (1000)

volatile uint64_t msTicks;

// System timer ISR
void SysTick_Handler(void)
{
    msTicks++;
}

void OSR_life_cycle(void)
{
    unsigned int *p = (unsigned int *)0x60040000;
    memset(p, 0, 1024);
}

void restart_ehsm(void)
{
    *((volatile unsigned int *)(0x40010058)) = 0x0fa66e27; // OSR_FPGA_MAGIC
    *((volatile unsigned int *)(0x40010008)) = 0xFFFFFFFE; // reset
    *((volatile unsigned int *)(0x40010008)) = 0xFFFFFFFF; // release
    while ((HSM_STATUS_IN & (SYSSTA0_BOOT_DONE | SYSSTA0_HSM_READY)) == 0) {
        ;
    }
}

void ehsm_mb_enable_int(uint32_t channel)
{
    (void)cpu_enable_int(SOC_MAILBOX0_IRQn + channel);
}

int main(void)
{
    UartStdOutInit(115200);
    printf("ehsm host start\n");
    OSR_life_cycle();
    restart_ehsm();

    ehsm_demo_entry();

    while (1) { }
}
