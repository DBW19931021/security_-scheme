/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef _WING_DRV_MTIMER_H_
#define _WING_DRV_MTIMER_H_

#include "mtimer_regs.h"

/*
 *  Platform configuration must define
 *  RTC source clock frequency (PLF_RTC_SRC_CLK),
 *  source type (PLF_RTC_SRC_EXTERNAL for ext),
 *  RTC frequency (PLF_RTC_TIMEBASE),
 *  RTC Timer base address (PLF_MTIMER_BASE)
 */

#ifndef PLF_MTIMER_BASE
#error Unknown PLF_MTIMER_BASE
#endif

#ifndef PLF_RTC_SRC_CLK
#define PLF_RTC_SRC_CLK PLF_SYS_CLK
#endif

#ifndef PLF_RTC_TIMEBASE
#define PLF_RTC_TIMEBASE 1000000
#endif

#define WING_PLF_TIMEBASE_DIV ((PLF_RTC_SRC_CLK) / (PLF_RTC_TIMEBASE) - 1)

#ifdef PLF_RTC_SRC_EXTERNAL
#define WING_PLF_RTC_SRC WING_MTIMER_CTRL_SE
#else
#define WING_PLF_RTC_SRC WING_MTIMER_CTRL_SI
#endif

#define WING_RTC_CTL_OFF   0
#define WING_RTC_DIV_OFF   4
#define WING_RTC_TIME_OFF  8
#define WING_RTC_TIMEH_OFF 12
#define WING_RTC_CMP_OFF   16
#define WING_RTC_CMPH_OFF  20

#define WING_RTC_CTL              (PLF_MTIMER_BASE + WING_RTC_CTL_OFF)
#define WING_RTC_DIVIDER          (PLF_MTIMER_BASE + WING_RTC_DIV_OFF)
#define WING_RTC_MTIME            (PLF_MTIMER_BASE + WING_RTC_TIME_OFF)
#define WING_RTC_MTIMEH           (PLF_MTIMER_BASE + WING_RTC_TIMEH_OFF)
#define WING_RTC_MTIMECMP         (PLF_MTIMER_BASE + WING_RTC_CMP_OFF)
#define WING_RTC_MTIMECMPH        (PLF_MTIMER_BASE + WING_RTC_CMPH_OFF)
#define WING_RTC_CTL_EN           (1 << 0)
#define WING_RTC_CTL_INTERNAL_SRC (0 << 1)
#define WING_RTC_CTL_EXTERNAL_SRC (1 << 1)

// trap causes
// interrupts
#define TRAP_CAUSE_INTERRUPT_FLAG (1UL << (__riscv_xlen - 1))
#define TRAP_CAUSE_INT_SSOFT      (1)
#define TRAP_CAUSE_INT_MSOFT      (3)
#define TRAP_CAUSE_INT_STIME      (5)
#define TRAP_CAUSE_INT_MTIME      (7)
#define TRAP_CAUSE_INT_SEXT       (9)
#define TRAP_CAUSE_INT_MEXT       (11)
// exceptions
#define TRAP_CAUSE_EXC_FETCH_ALIGN  (0)
#define TRAP_CAUSE_EXC_FETCH_ACCESS (1)
#define TRAP_CAUSE_EXC_ILLEGAL      (2)
#define TRAP_CAUSE_EXC_BREAKPOINT   (3)
#define TRAP_CAUSE_EXC_LOAD_ALIGN   (4)
#define TRAP_CAUSE_EXC_LOAD_ACCESS  (5)
#define TRAP_CAUSE_EXC_STORE_ALIGN  (6)
#define TRAP_CAUSE_EXC_STORE_ACCESS (7)
#define TRAP_CAUSE_EXC_UECALL       (8)
#define TRAP_CAUSE_EXC_SECALL       (9)
#define TRAP_CAUSE_EXC_RESERVED1    (10)
#define TRAP_CAUSE_EXC_MECALL       (11)
#define TRAP_CAUSE_EXC_FETCH_PAGE   (12)
#define TRAP_CAUSE_EXC_LOAD_PAGE    (13)
#define TRAP_CAUSE_EXC_RESERVED2    (14)
#define TRAP_CAUSE_EXC_STORE_PAGE   (15)

// mie/mip bits
#define MIE_MSOFTWARE (1 << TRAP_CAUSE_INT_MSOFT)
#define MIE_MTIMER    (1 << TRAP_CAUSE_INT_MTIME)
#define MIE_MEXTERNAL (1 << TRAP_CAUSE_INT_MEXT)

#ifndef __ASSEMBLER__

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t sys_tick64_t;
typedef uint32_t sys_tick32_t;

#if __riscv_xlen == 32
typedef sys_tick32_t sys_tick_t;
#else  // __riscv_xlen  == 32
typedef sys_tick64_t sys_tick_t;
#endif // __riscv_xlen == 32

typedef struct wing_mtimer_struct {
    uint32_t ctrl;
    uint32_t div;
#if __riscv_xlen == 32
    uint32_t time;
    uint32_t timeh;
    uint32_t cmp;
    uint32_t cmph;
#else  // __riscv_xlen  == 32
    uint64_t time;
    uint64_t cmp;
#endif // __riscv_xlen == 32
} volatile wing_mtimer;

#define WING_MTIMER (*(wing_mtimer *)(PLF_MTIMER_BASE))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

static inline sys_tick_t wing_mtimer_current(void)
{
    unsigned long t;

    asm volatile("csrr %[t], time" : [t] "=r"(t));

    return (sys_tick_t)t;
}

static inline sys_tick64_t wing_mtimer_current64(void)
{
#if __riscv_xlen == 32
    uint32_t lo, hi, tmp;

    asm volatile("1:"
                 "csrr %[hi], timeh ;"
                 "csrr %[lo], time  ;"
                 "csrr %[tmp],timeh ;"
                 "bne  %[hi], %[tmp], 1b"
                 : [hi] "=&r"(hi), [lo] "=&r"(lo), [tmp] "=&r"(tmp));

    return ((sys_tick64_t)hi << 32) | lo;
#else  //  __riscv_xlen == 32
    return wing_mtimer_current();
#endif // __riscv_xlen == 32
}

// basic functions

static inline void wing_mtimer_setcmp(sys_tick64_t t)
{
#if __riscv_xlen == 32
    WING_MTIMER.cmph = ~0;
    WING_MTIMER.cmp = (uint32_t)t;
    WING_MTIMER.cmph = (uint32_t)(t >> 32);
#else  //  __riscv_xlen == 32
    WING_MTIMER.cmp = t;
#endif //  __riscv_xlen == 32
}

static inline void wing_mtimer_setcmp_offset(sys_tick64_t to)
{
    wing_mtimer_setcmp(wing_mtimer_current64() + to);
}

static inline void wing_mtimer_set(sys_tick_t t)
{
    WING_MTIMER.time = t;
}

static inline void wing_mtimer_set64(sys_tick64_t t)
{
#if __riscv_xlen == 32
    WING_MTIMER.timeh = 0;
    WING_MTIMER.time = (uint32_t)t;
    WING_MTIMER.timeh = (uint32_t)(t >> 32);
#else  //  __riscv_xlen == 32
    wing_mtimer_set(t);
#endif //  __riscv_xlen == 32
}

static inline void wing_mtimer_reset(void)
{
    // reset counter and cmp, set divisor
    WING_MTIMER.ctrl = 0;
    WING_MTIMER.div = WING_PLF_TIMEBASE_DIV;
    WING_MTIMER.time = 0;
#if __riscv_xlen == 32
    WING_MTIMER.timeh = 0;
#endif // __riscv_xlen == 32
    WING_MTIMER.cmp = ~0;
#if __riscv_xlen == 32
    WING_MTIMER.cmph = ~0;
#endif // __riscv_xlen == 32
}

static inline void wing_mtimer_enable(void)
{
    WING_MTIMER.ctrl = WING_MTIMER_CTRL_EN | WING_PLF_RTC_SRC;
}

static inline void wing_mtimer_disable(void)
{
    WING_MTIMER.ctrl = 0;
}

static inline void wing_mtimer_init(void)
{
    wing_mtimer_reset();
    wing_mtimer_enable();
}

#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif

#endif // !__ASSEMBLER__
#endif // _WING_DRV_MTIMER_H_
