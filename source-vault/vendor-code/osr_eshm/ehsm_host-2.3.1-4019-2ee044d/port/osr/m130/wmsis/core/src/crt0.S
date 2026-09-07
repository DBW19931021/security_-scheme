/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#include "encoding.h"

.macro DECLARE_INT_HANDLER  INT_HDL_NAME
#if defined(__riscv_xlen) && (__riscv_xlen == 32)
    .word \INT_HDL_NAME
#else
    .dword \INT_HDL_NAME
#endif
.endm


	.section .vtable

    .weak eclic_msip_handler
    .weak eclic_mtip_handler

    .globl vector_base
    .type vector_base, @object

vector_base:
    DECLARE_INT_HANDLER     default_intexc_handler          /* 0: Reserved, default handler for vector table remapped cases */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 1: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 2: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 3: Machine software interrupt */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 4: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 5: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 6: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 7: Machine timer interrupt */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 8: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 9: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 10: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 11: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 12: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 13: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 14: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 15: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 16: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 17: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 18: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler        	/* 19: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 20: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 21: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 22: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler        	/* 23: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 24: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 25: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 26: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler        	/* 27: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 28: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 29: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 30: Reserved */
    DECLARE_INT_HANDLER     default_intexc_handler        	/* 31: Reserved */

    DECLARE_INT_HANDLER     default_intexc_handler        	/* 32: Interrupt 32 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 32: Interrupt 33 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 34: Interrupt 34 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 35: Interrupt 35 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 36: Interrupt 36 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 37: Interrupt 37 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 38: Interrupt 38 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 39: Interrupt 39 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 40: Interrupt 40 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 41: Interrupt 41 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 42: Interrupt 42 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 43: Interrupt 43 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 44: Interrupt 44 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 45: Interrupt 45 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 46: Interrupt 46 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 47: Interrupt 47 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 48: Interrupt 48 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 49: Interrupt 49 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 50: Interrupt 50 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 51: Interrupt 51 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 52: Interrupt 52 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 53: Interrupt 53 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 54: Interrupt 54 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 55: Interrupt 55 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 56: Interrupt 56 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 57: Interrupt 57 */
    DECLARE_INT_HANDLER     default_intexc_handler        	/* 58: Interrupt 58 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 59: Interrupt 59 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 60: Interrupt 60 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 61: Interrupt 61 */
    DECLARE_INT_HANDLER     default_intexc_handler			/* 62: Interrupt 62 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 63: Interrupt 63 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 64: Interrupt 64 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 65: Interrupt 65 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 66: Interrupt 66 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 67: Interrupt 67 */

    DECLARE_INT_HANDLER     uart_hander          		    /* 68: Interrupt 68 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 69: Interrupt 69 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 70: Interrupt 70 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 71: Interrupt 71 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 72: Interrupt 72 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 73: Interrupt 73 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 74: Interrupt 74 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 75: Interrupt 75 */

#ifdef CONFIG_EHSM_HW_UTC_TIME
    DECLARE_INT_HANDLER     utc_timer_handler               /* 76: Interrupt 76 */
#else
    DECLARE_INT_HANDLER     default_intexc_handler
#endif
    DECLARE_INT_HANDLER     default_intexc_handler          /* 77: Interrupt 77 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 74: Interrupt 74 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 75: Interrupt 75 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 76: Interrupt 76 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 77: Interrupt 77 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 78: Interrupt 78 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 79: Interrupt 79 */


#ifdef CONFIG_EHSM_HW_COUNTER
    DECLARE_INT_HANDLER     counter_handler                 /* 80: Interrupt 80 */
#else
    DECLARE_INT_HANDLER     default_intexc_handler
#endif
    DECLARE_INT_HANDLER     default_intexc_handler          /* 81: Interrupt 81 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 82: Interrupt 82 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 83: Interrupt 83 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 84: Interrupt 84 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 85: Interrupt 85 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 86: Interrupt 86 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 87: Interrupt 87 */

    DECLARE_INT_HANDLER     mailbox_handler        	        /* 88: Interrupt 88 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 89: Interrupt 89 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 90: Interrupt 90 */
    DECLARE_INT_HANDLER     default_intexc_handler          /* 91: Interrupt 91 */

    DECLARE_INT_HANDLER     default_intexc_handler          /* 92: Interrupt 92 */

    .section .init

    .globl _start
    .type _start,@function

/**
 * Reset Handler called on controller reset
 */
_start:
    /* Initialize GP and Stack Pointer SP */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la sp, __StackTop

    la t0, default_intexc_handler
    csrw CSR_MTVEC, t0
1:
    /* Load data section */
    la a0, _data_lma
    la a1, _data
    /* If data vma=lma, no need to copy */
    beq a0, a1, 1f
    la a2, _edata
    bgeu a1, a2, 1f
2:
    lw t0, (a0)
    sw t0, (a1)
    addi a0, a0, 4
    addi a1, a1, 4
    bltu a1, a2, 2b
#ifndef EHSM_RUN_SIM
1:
    /* Clear bss section */
    la a0, __bss_start
    la a1, __heap_start
    bgeu a0, a1, 1f
2:
    sw zero, (a0)
    addi a0, a0, 4
    bltu a0, a1, 2b
#endif
1:
  li a0, 0
  li a1, 0

  call    main

  .size  _start, .-_start

.align 6
.global default_intexc_handler
Undef_Handler:
default_intexc_handler:
1:
    j 1b
