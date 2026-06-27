/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#include "encoding.h"
#include "version.h"

.section .init

.globl _start
.global default_trap_handler
.type _start,@function
.type default_trap_handler,@function

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

    la t0, default_trap_handler
    csrw CSR_MTVEC, t0
3:
    auipc a0, %pcrel_hi(__jvt_base$)
    addi a0, a0, %pcrel_lo(3b)
    csrw jvt, a0

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
1:
    /* Clear bss section */
    la a0, __bss_start
    la a1, __heap_start
    bgeu a0, a1, 1f
2:
    sw zero, (a0)
    addi a0, a0, 4
    bltu a0, a1, 2b
1:
  li a0, 0

  call    main

  .size  _start, .-_start
