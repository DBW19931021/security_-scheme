/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef __CLIC_H__
#define __CLIC_H__
#include "encoding.h"
#include "platform.h"
#include "types.h"

typedef void (*clic_int_handler)(void);

typedef enum {
    CLIC_OK,
    CLIC_PARAM_ERROR,
    CLIC_GIVEN_HANDLER_ERROR,
} clic_error_t;

#define CLIC_REG_MEM_BASE (PLF_TCM_BASE + 0x10000)

#define CLIC_REG_PTR(reg) ((volatile uint32_t *)(reg + CLIC_REG_MEM_BASE))
#define CLIC_REG_CFG      (*(CLIC_REG_PTR(0x00)))
#define CLIC_REG_TRIG     (*(CLIC_REG_PTR(0x40)))

#define CLIC_INT_X_REG(x)       (*(CLIC_REG_PTR(0x1000 + 4 * x)))
#define CLIC_INT_X_REG_IP_Pos   0
#define CLIC_INT_X_REG_IE_Pos   8
#define CLIC_INT_X_REG_ATTR_Pos 16
#define CLIC_INT_X_REG_CTRL_Pos 24

#define CLIC_CFG_NLBIT_Pos 0U
#define CLIC_CFG_NLBIT_Msk (0x0fUL << CLIC_CFG_NLBIT_Pos)

#define CLIC_INT_IE_Pos 0U
#define CLIC_INT_IE_Msk (0x1UL << CLIC_INT_IE_Pos)

#define CLIC_INT_IP_PENDING_Pos 0U
#define CLIC_INT_IP_PENDING_Msk (0x1UL << CLIC_INT_IP_PENDING_Pos)

#define CLIC_INT_ATTR_SHV_Pos 0U
#define CLIC_INT_ATTR_SHV_Msk (0x1UL << CLIC_INT_ATTR_SHV_Pos)

#define CLIC_INT_ATTR_TRIG_Pos  1U
#define CLIC_INT_ATTR_RTIG_Msk  (0x1UL << CLIC_INT_ATTR_TRIG_Pos) // 0:level 1:pulse
#define CLIC_INT_ATTR_LEVEL_Msk (0x0UL << CLIC_INT_ATTR_TRIG_Pos)

#define CLIC_INT_ATTR_POLARITY_Pos 2U
#define CLIC_INT_ATTR_POLARITY_Msk \
    (0x1UL << CLIC_INT_ATTR_POLARITY_Pos) // 0:high level/rising edge 1:low leve/falling edge
#define CLIC_INI_ATTR_POHARITY_Msk (0x0UL << CLIC_INT_ATTR_POLARITY_Pos)

#define CLIC_INT_CTRL_ALL_Pos 0U
#define CLIC_INT_CTRL_ALL_Msk (0xffUL << CLIC_INT_CTRL_ALL_Pos)

void clic_init(void);
uint32_t clic_int_handler_regist(uint8_t irq, clic_int_handler cb);
uint32_t clic_int_enable(uint8_t irq);
uint32_t clic_int_disable(uint8_t irq);
uint32_t clic_set_priority(uint8_t irq, uint8_t pri);
uint32_t clic_set_trig_type(uint8_t irq, uint32_t tri_type);
uint32_t clic_set_vector_mode(uint8_t irq, bool_t setting);
bool_t clic_get_int_pending(uint8_t irq);
uint32_t clic_clear_int_pending(uint8_t irq);
clic_int_handler clic_get_irq_handler(uint8_t irq);

#endif //__CLIC_H__
