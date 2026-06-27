/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef __MTIMER_REGS_H__
#define __MTIMER_REGS_H__

// MTimer memory mapped registers (offset)
#define WING_MTIMER_CTRL   (0)
#define WING_MTIMER_DIV    (4)
#define WING_MTIMER_MTIME  (8)
#define WING_MTIMER_MTIMEH (12)
#define WING_MTIMER_CMP    (16)
#define WING_MTIMER_CMPH   (20)
// MTimer control register bits
#define WING_MTIMER_CTRL_EN (1 << 0)
#define WING_MTIMER_CTRL_SI (0 << 1)
#define WING_MTIMER_CTRL_SE (1 << 1)

#endif // __MTIMER_REGS_H__
