.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.section .isr_vector, "a"
.global isr_vector
.global __Vectors_End
.global __Vectors_Size

isr_vector:
    .long _estack
    .long Reset_Handler
    .long NMI_Handler
    .long HardFault_Handler
    .long MemManage_Handler
    .long BusFault_Handler
    .long UsageFault_Handler
    .long 0
    .long 0
    .long 0
    .long 0
    .long SVC_Handler
    .long DebugMon_Handler
    .long 0
    .long PendSV_Handler
    .long SysTick_Handler

    /* External Interrupts */
    .long UART0_Handler
    .long Spare1_Handler
    .long UART1_Handler
    .long Spare3_Handler
    .long Spare4_Handler
    .long RTC_Handler
    .long PORT0_COMB_Handler
    .long PORT1_COMB_Handler
    .long TIMER0_Handler
    .long TIMER1_Handler
    .long DUALTIMER_HANDLER
    .long Spare11_Handler
    .long UARTOVF_Handler
    .long Spare13_Handler
    .long Spare14_Handler
    .long TSC_Handler
    .long PORT0_0_Handler
    .long PORT0_1_Handler
    .long PORT0_2_Handler
    .long PORT0_3_Handler
    .long PORT0_4_Handler
    .long PORT0_5_Handler
    .long PORT0_6_Handler
    .long PORT0_7_Handler
    .long PORT0_8_Handler
    .long PORT0_9_Handler
    .long PORT0_10_Handler
    .long PORT0_11_Handler
    .long PORT0_12_Handler
    .long PORT0_13_Handler
    .long PORT0_14_Handler
    .long PORT0_15_Handler
    .long MAILBOX_0_Handler
    .long MAILBOX_1_Handler
    .long MAILBOX_2_Handler
    .long MAILBOX_3_Handler
    .long MAILBOX_4_Handler
    .long MAILBOX_5_Handler
    .long MAILBOX_6_Handler
    .long MAILBOX_7_Handler
    .long MAILBOX_8_Handler
    .long MAILBOX_9_Handler
    .long MAILBOX_10_Handler
    .long MAILBOX_11_Handler
    .long MAILBOX_12_Handler
    .long MAILBOX_13_Handler
    .long MAILBOX_14_Handler
    .long MAILBOX_15_Handler
    .long EFLASH_Handler
    .long CORDIO0_Handler
    .long CORDIO1_Handler
    .long CORDIO2_Handler
    .long CORDIO3_Handler
    .long CORDIO4_Handler
    .long CORDIO5_Handler
    .long CORDIO6_Handler
    .long CORDIO7_Handler
    .long PORT2_COMB_Handler
    .long PORT3_COMB_Handler
    .long TRNG_Handler
    .long UART2_Handler
    .long UART3_Handler
    .long ETHERNET_Handler
    .long I2S_Handler
    .long MPS2_SPI0_Handler
    .long MPS2_SPI1_Handler
    .long MPS2_SPI2_Handler
    .long MPS2_SPI3_Handler
    .long MPS2_SPI4_Handler
    .long PORT4_COMB_Handler
    .long PORT5_COMB_Handler
    .long UART4_Handler

__Vectors_End:
.equ __Vectors_Size, __Vectors_End - isr_vector

.section .text

.thumb
.thumb_func
.align 2
.global Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:

  /* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

    ldr r0, =SystemInit
    blx r0
    ldr r0, =main
    bx r0

LoopForever:
  b LoopForever

.size Reset_Handler, . - Reset_Handler

.thumb_func
.align 2
.global NMI_Handler
.weak NMI_Handler
.type NMI_Handler, %function
NMI_Handler:
    b .
.size NMI_Handler, . - NMI_Handler

.thumb_func
.align 2
.global HardFault_Handler
.weak HardFault_Handler
.type HardFault_Handler, %function
HardFault_Handler:
    b .
.size HardFault_Handler, . - HardFault_Handler

.thumb_func
.align 2
.global MemManage_Handler
.weak MemManage_Handler
.type MemManage_Handler, %function
MemManage_Handler:
    b .
.size MemManage_Handler, . - MemManage_Handler

.thumb_func
.align 2
.global BusFault_Handler
.weak BusFault_Handler
.type BusFault_Handler, %function
BusFault_Handler:
    b .
.size BusFault_Handler, . - BusFault_Handler

.thumb_func
.align 2
.global UsageFault_Handler
.weak UsageFault_Handler
.type UsageFault_Handler, %function
UsageFault_Handler:
    b .
.size UsageFault_Handler, . - UsageFault_Handler

.thumb_func
.align 2
.global SVC_Handler
.weak SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
    b .
.size SVC_Handler, . - SVC_Handler

.thumb_func
.align 2
.global DebugMon_Handler
.weak DebugMon_Handler
.type DebugMon_Handler, %function
DebugMon_Handler:
    b .
.size DebugMon_Handler, . - DebugMon_Handler

.thumb_func
.align 2
.global PendSV_Handler
.weak PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    b .
.size PendSV_Handler, . - PendSV_Handler

.thumb_func
.align 2
.global SysTick_Handler
.weak SysTick_Handler
.type SysTick_Handler, %function
SysTick_Handler:
    b .
.size SysTick_Handler, . - SysTick_Handler

.thumb_func
.align 2
.global MAILBOX_Handler
.weak MAILBOX_Handler
.type MAILBOX_Handler, %function
MAILBOX_Handler:
    b .
.size MAILBOX_Handler, . - MAILBOX_Handler

.thumb_func
.align 2
.global Default_Handler
.weak Default_Handler
.type Default_Handler, %function
Default_Handler:
    .weak UART0_Handler
    .weak Spare1_Handler
    .weak UART1_Handler
    .weak Spare3_Handler
    .weak Spare4_Handler
    .weak RTC_Handler
    .weak PORT0_COMB_Handler
    .weak PORT1_COMB_Handler
    .weak TIMER0_Handler
    .weak TIMER1_Handler
    .weak DUALTIMER_HANDLER
    .weak Spare11_Handler
    .weak UARTOVF_Handler
    .weak Spare13_Handler
    .weak Spare14_Handler
    .weak TSC_Handler
    .weak PORT0_0_Handler
    .weak PORT0_1_Handler
    .weak PORT0_2_Handler
    .weak PORT0_3_Handler
    .weak PORT0_4_Handler
    .weak PORT0_5_Handler
    .weak PORT0_6_Handler
    .weak PORT0_7_Handler
    .weak PORT0_8_Handler
    .weak PORT0_9_Handler
    .weak PORT0_10_Handler
    .weak PORT0_11_Handler
    .weak PORT0_12_Handler
    .weak PORT0_13_Handler
    .weak PORT0_14_Handler
    .weak PORT0_15_Handler
    .weak MAILBOX_0_Handler
    .weak MAILBOX_1_Handler
    .weak MAILBOX_2_Handler
    .weak MAILBOX_3_Handler
    .weak MAILBOX_4_Handler
    .weak MAILBOX_5_Handler
    .weak MAILBOX_6_Handler
    .weak MAILBOX_7_Handler
    .weak MAILBOX_8_Handler
    .weak MAILBOX_9_Handler
    .weak MAILBOX_10_Handler
    .weak MAILBOX_11_Handler
    .weak MAILBOX_12_Handler
    .weak MAILBOX_13_Handler
    .weak MAILBOX_14_Handler
    .weak MAILBOX_15_Handler
    .weak EFLASH_Handler
    .weak CORDIO0_Handler
    .weak CORDIO1_Handler
    .weak CORDIO2_Handler
    .weak CORDIO3_Handler
    .weak CORDIO4_Handler
    .weak CORDIO5_Handler
    .weak CORDIO6_Handler
    .weak CORDIO7_Handler
    .weak PORT2_COMB_Handler
    .weak PORT3_COMB_Handler
    .weak TRNG_Handler
    .weak UART2_Handler
    .weak UART3_Handler
    .weak ETHERNET_Handler
    .weak I2S_Handler
    .weak MPS2_SPI0_Handler
    .weak MPS2_SPI1_Handler
    .weak MPS2_SPI2_Handler
    .weak MPS2_SPI3_Handler
    .weak MPS2_SPI4_Handler
    .weak PORT4_COMB_Handler
    .weak PORT5_COMB_Handler
    .weak UART4_Handler

UART0_Handler:
Spare1_Handler:
UART1_Handler:
Spare3_Handler:
Spare4_Handler:
RTC_Handler:
PORT0_COMB_Handler:
PORT1_COMB_Handler:
TIMER0_Handler:
TIMER1_Handler:
DUALTIMER_HANDLER:
Spare11_Handler:
UARTOVF_Handler:
Spare13_Handler:
Spare14_Handler:
TSC_Handler:
PORT0_0_Handler:
PORT0_1_Handler:
PORT0_2_Handler:
PORT0_3_Handler:
PORT0_4_Handler:
PORT0_5_Handler:
PORT0_6_Handler:
PORT0_7_Handler:
PORT0_8_Handler:
PORT0_9_Handler:
PORT0_10_Handler:
PORT0_11_Handler:
PORT0_12_Handler:
PORT0_13_Handler:
PORT0_14_Handler:
PORT0_15_Handler:
MAILBOX_0_Handler:
MAILBOX_1_Handler:
MAILBOX_2_Handler:
MAILBOX_3_Handler:
MAILBOX_4_Handler:
MAILBOX_5_Handler:
MAILBOX_6_Handler:
MAILBOX_7_Handler:
MAILBOX_8_Handler:
MAILBOX_9_Handler:
MAILBOX_10_Handler:
MAILBOX_11_Handler:
MAILBOX_12_Handler:
MAILBOX_13_Handler:
MAILBOX_14_Handler:
MAILBOX_15_Handler:
EFLASH_Handler:
CORDIO0_Handler:
CORDIO1_Handler:
CORDIO2_Handler:
CORDIO3_Handler:
CORDIO4_Handler:
CORDIO5_Handler:
CORDIO6_Handler:
CORDIO7_Handler:
PORT2_COMB_Handler:
PORT3_COMB_Handler:
TRNG_Handler:
UART2_Handler:
UART3_Handler:
ETHERNET_Handler:
I2S_Handler:
MPS2_SPI0_Handler:
MPS2_SPI1_Handler:
MPS2_SPI2_Handler:
MPS2_SPI3_Handler:
MPS2_SPI4_Handler:
PORT4_COMB_Handler:
PORT5_COMB_Handler:
UART4_Handler:
    b .
.size Default_Handler, . - Default_Handler


.end
