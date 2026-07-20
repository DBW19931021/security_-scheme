/**
 * @file soc_def.h
 * @brief soc define header file.
 * @version 1.0
 * @date 2024-04-07
 *
 * Copyright (c) 2022 - 2024, WingSemi Technology LTD.
 *
 */
#ifndef __SOC_DEF__
#define __SOC_DEF__
#include "platform.h"

#define PLF_SYS_CLK                20000000 /*!< system clock frequency. */
#define PLF_INTERNAL_TIMER_REF_CLK 5000000  /*!< reference clock frequency for timer. */
#define PLF_UART_CLK               PLF_SYS_CLK

// memory configuration
#define PLF_MEM_BASE  (0x00000000) /*!< memory space address base. */
#define PLF_PBUS_BASE (0x30000000) /*!< pbus space address base. */
#endif                             //__SOC_DEF__
