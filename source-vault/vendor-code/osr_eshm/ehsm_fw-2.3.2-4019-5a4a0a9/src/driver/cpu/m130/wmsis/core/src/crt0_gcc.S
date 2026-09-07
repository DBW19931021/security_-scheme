/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#include "encoding.h"


.section .init

.globl _start
.global default_intexc_handler
.type _start,@function
.type default_intexc_handler,@function

/**
 * Reset Handler called on controller reset
 */
_start: /* 程序入口点，复位后首先执行此处指令 */
    /* --- 初始化全局指针 (GP) 和堆栈指针 (SP) --- */
    .option push        /* 保存当前汇编器选项 */
    .option norelax     /* 禁用链接器松弛优化（确保 GP 正确加载） */
    la gp, __global_pointer$  /* 加载全局指针地址（链接脚本定义的全局变量基址）*/
    .option pop         /* 恢复之前的汇编器选项 */
    la sp, __StackTop   /* 加载堆栈顶地址（堆栈从高地址向低地址增长）*/

    /* --- 设置默认中断/异常处理程序 --- */
    la t0, default_intexc_handler  /* 将异常处理函数地址加载到 t0 */
    csrw CSR_MTVEC, t0            /* 写入机器模式中断向量基址寄存器 */

1:  /* 标签 1：下一步的开始 */
    /* --- 加载数据段（将初始数据从 Flash 复制到 RAM）--- */
    la a0, _data_lma       /* a0 = 数据段在 Flash 中的起始地址（LMA: Load Memory Address） */
    la a1, _data           /* a1 = 数据段在 RAM 中的目标地址（VMA: Virtual Memory Address） */
    beq a0, a1, 1f         /* 如果 LMA 和 VMA 相同（无需复制），跳转到下方标签 1 */
    la a2, _edata          /* a2 = 数据段在 RAM 中的结束地址（_edata 来自链接脚本） */
    bgeu a1, a2, 1f        /* 如果目标地址已超过结束地址（数据段为空），跳转到标签 1 */

2:  /* 标签 2：循环复制数据 */
    lw t0, (a0)        /* 从 Flash（a0）加载 4 字节数据到临时寄存器 t0 */
    sw t0, (a1)        /* 将 t0 的数据存入 RAM（a1） */
    addi a0, a0, 4     /* 源地址 a0 += 4（移动到下一个字） */
    addi a1, a1, 4     /* 目标地址 a1 += 4 */
    bltu a1, a2, 2b    /* 如果 a1 < a2（未到结束地址），跳回标签 2 继续循环 */

1:  /* 此处标签 1结束数据段复制 */

    /* --- 清零 BSS 段（初始化未初始化的全局变量为0） --- */
    la a0, __bss_start      /* a0 = BSS 段起始地址 */
    la a1, __heap_start     /* a1 = BSS 段结束地址（通常 BSS 段末尾是堆的起始）*/
    bgeu a0, a1, 1f        /* 如果起始地址 >= 结束地址（BSS 段为空），跳转到标签1 */

2:  /* 标签 2：循环清零内存 */
    sw zero, (a0)       /* 将当前地址（a0）的 4 字节写入零 */
    addi a0, a0, 4      /* a0 += 4（处理下一个字）*/
    bltu a0, a1, 2b     /* 如果 a0 < a1（未到结束地址），跳回标签 2 循环 */

1:  /* 此处标签 1结束 BSS 段清零 */

    li a0, 0          /* 准备 main 函数参数（a0 = argc = 0，RISC-V 调用规范第一个参数存入 a0）*/
    call main         /* 调用用户主函数，程序将跳转到用户代码 */ 

  .size  _start, .-_start
