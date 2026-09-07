/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#include "csr.h"
#include "clic.h"
#include "platform.h"
#include "irq_number.h"

#ifdef CLIC_INT_SUPPORT
static clic_int_handler s_handler_array[SOC_INT_NUM] __attribute__((aligned(64)));

extern void default_trap_handler(void);
void clic_init(void)
{
    uint32_t tmp;
#ifdef SMCLICSHV_EXTENTION_SUPPORT
    // enable clic vector mode

    __RV_CSR_WRITE(CSR_MTVEC, default_trap_handler);
    // set handler base
    tmp = ((uint32_t)s_handler_array) & 0xffffffc0;
    __RV_CSR_WRITE(CSR_MTVT, tmp);
#endif // SMCLICSHV_EXTENTION_SUPPORT

    // set level/priority bit:level 4  priority 8-4
    tmp = CLIC_REG_CFG;
    tmp &= ~CLIC_CFG_NLBIT_Msk;
    tmp |= 4;
    CLIC_REG_CFG = tmp;
}

uint32_t clic_int_handler_regist(uint8_t irq, clic_int_handler cb)
{
    uint32_t err_ret = CLIC_OK;

    if (NULL == cb) {
        err_ret = CLIC_GIVEN_HANDLER_ERROR;
        return err_ret;
    }

    s_handler_array[irq] = cb;

    return err_ret;
}

uint32_t clic_int_enable(uint8_t irq)
{
    uint32_t err_ret = CLIC_OK;

    CLIC_INT_X_REG(irq) |= CLIC_INT_IE_Msk << CLIC_INT_X_REG_IE_Pos;

    return err_ret;
}

uint32_t clic_int_disable(uint8_t irq)
{
    uint32_t err_ret = CLIC_OK;

    CLIC_INT_X_REG(irq) &= ~(CLIC_INT_IE_Msk << CLIC_INT_X_REG_IE_Pos);

    return err_ret;
}

uint32_t clic_set_priority(uint8_t irq, uint8_t pri)
{
    uint32_t err_ret = CLIC_OK;

    CLIC_INT_X_REG(irq) &= ~(CLIC_INT_CTRL_ALL_Msk << CLIC_INT_X_REG_CTRL_Pos);
    CLIC_INT_X_REG(irq) |= ((uint32_t)pri << CLIC_INT_X_REG_CTRL_Pos);

    return err_ret;
}

uint32_t clic_set_trig_type(uint8_t irq, uint32_t tri_type)
{
    uint32_t err_ret = CLIC_OK;

    if (tri_type & (~(CLIC_INT_ATTR_RTIG_Msk | CLIC_INT_ATTR_POLARITY_Msk))) {
        err_ret = CLIC_PARAM_ERROR;
        return err_ret;
    }

    CLIC_INT_X_REG(irq) &= ~((CLIC_INT_ATTR_RTIG_Msk | CLIC_INT_ATTR_POLARITY_Msk) << CLIC_INT_X_REG_ATTR_Pos);
    CLIC_INT_X_REG(irq) |= (tri_type << CLIC_INT_X_REG_ATTR_Pos);

    return err_ret;
}

uint32_t clic_set_vector_mode(uint8_t irq, bool_t setting)
{
    uint32_t err_ret = CLIC_OK;

#ifdef SMCLICSHV_EXTENTION_SUPPORT
    CLIC_INT_X_REG(irq) &= ~(CLIC_INT_ATTR_SHV_Msk << CLIC_INT_X_REG_ATTR_Pos);

    if (setting) {
        CLIC_INT_X_REG(irq) |= (CLIC_INT_ATTR_SHV_Msk << CLIC_INT_X_REG_ATTR_Pos);
    }
#endif // SMCLICSHV_EXTENTION_SUPPORT

    return err_ret;
}

bool_t clic_get_int_pending(uint8_t irq)
{
    bool_t ret = false;
    uint8_t tmp;

    tmp = (uint8_t)CLIC_INT_X_REG(irq);

    if (tmp & (CLIC_INT_IP_PENDING_Msk << CLIC_INT_X_REG_IP_Pos)) {
        ret = true;
    }

    return ret;
}

uint32_t clic_clear_int_pending(uint8_t irq)
{
    uint32_t err_ret = CLIC_OK;

    CLIC_INT_X_REG(irq) &= (~(CLIC_INT_IP_PENDING_Msk << CLIC_INT_X_REG_IP_Pos));

    return err_ret;
}

clic_int_handler clic_get_irq_handler(uint8_t irq)
{
    return s_handler_array[irq];
}
#endif // CLIC_INT_SUPPORT
