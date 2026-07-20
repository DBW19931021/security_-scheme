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
#endif //WINGSEMI_M130

#ifndef PLF_SYS_CLK
#define PLF_SYS_CLK      (0x20000000U)
#endif

// memory configuration
#define PLF_TCM_BASE     (0x00000000U)

#endif /* PLATFORM_H */
