/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#define WINGSEMI_M130

#ifdef WINGSEMI_M130
#define CLIC_INT_SUPPORT
#define SMCLICSHV_EXTENTION_SUPPORT
#endif // WINGSEMI_M130

#ifndef PLF_SYS_CLK
#define PLF_SYS_CLK 20000000
#endif

// memory configuration
#define PLF_MTIMER_BASE (0x00490000)
#define PLF_MMIO_BASE   (0x80000000)
#define PLF_TCM_BASE    (0x00000000)

#define PMP_PRESENT
// #define PMP_NAPOT_ONLY
#define PMP_ENTRY_NUM 8
#define PMP_HW_G      3
#endif /* PLATFORM_H */
