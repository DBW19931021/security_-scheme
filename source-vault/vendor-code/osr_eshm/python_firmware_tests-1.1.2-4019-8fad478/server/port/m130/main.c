#include "uart_stdout.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "host_m130_cfg.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/mailbox.h"

#include "cpu_porting.h"

// Forward declaration for server_main
void server_main(void);

#define OSR_FPAG_FREQ (5000000)
#define MS_TICKS      (1000)

volatile uint64_t msTicks;

// System timer ISR
void SysTick_Handler(void)
{
    msTicks++;
}



// ehsm_mb_handler_init is now handled by ehsm_port_enable_mailbox_int in ehsm_host_port.c

// This function is already defined in port_m130.c, so we don't need to redefine it
// void ehsm_mb_enable_int(uint32_t channel)
// {
//     (void)cpu_enable_int(SOC_MAILBOX0_IRQn + channel);
// }

int main(void)
{
    *((volatile unsigned int *)(0x40010058)) = 0x0fa66e27; // OSR_FPGA_MAGIC
    UartStdOutInit(115200);
    printf("ehsm test system server m130 start\n");
    printf("Build Date: %s %s\r\n", __DATE__, __TIME__);


    // Call server_main instead of ehsm_demo_entry
    server_main();

    while (1) { }
}