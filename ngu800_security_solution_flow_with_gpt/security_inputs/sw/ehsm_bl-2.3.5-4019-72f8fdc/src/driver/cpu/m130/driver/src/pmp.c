/**
 * @file pmp.c
 * @brief pmp driver source file.
 * @version 1.0
 * @date 2024-03-20
 *
 * Copyright (c) 2022 - 2024, WingSemi Technology LTD.
 *
 */
#include <math.h>
#include "pmp.h"
#include "platform.h"

#ifdef PMP_PRESENT

uint8_t get_one_number_of_addr(rv_csr_t address)
{
    rv_csr_t addr_tmp;
    uint8_t i;

    addr_tmp = address;

    for (i = 0; i < (sizeof(rv_csr_t) * 8); i++) {
        if (addr_tmp & 0x01) {
        } else {
            break;
        }

        addr_tmp >>= 1;
    }

    return i;
}

static uint32_t pow_1(uint32_t a, uint32_t b)
{
    uint32_t sum = 1;

    while (b > 0) {
        sum = sum * a;
        b--;
    }

    return sum;
}

static bool given_addr_has_multiple_pmp_rule(uint32_t index, rv_csr_t address)
{
    uint32_t idx;
    uint8_t match_mode;
    rv_csr_t addr_low, addr_high;

    for (idx = 0; idx < PMP_ENTRY_NUM; idx++) {
        if (idx == index) {
            continue;
        }

        pmp_addr_match_mode_get(idx, &match_mode);

        // NAPOT ranges make use of the low-order bits of the associated address register to encode the size of the
        // range
        if ((match_mode & PMP_ADDR_MATCH_MODE_SUPPORT) == PMP_NAPOT) {
            rv_csr_t addr_tmp, tmp;
            pmp_addr_get(idx, &addr_tmp);
            short ones_num = get_one_number_of_addr(addr_tmp >> 2);

            tmp = pow_1(2, (uint32_t)(ones_num + PMP_HW_G)) - 1;
            tmp = ~tmp;

            addr_low = addr_tmp & tmp;
            addr_high = addr_low + pow_1(2, (uint32_t)(ones_num + PMP_HW_G));

            if ((address >= addr_low) && (address < addr_high)) {
                return true;
            }
        }

        /* If TOR is selected, the associated address register forms the top of the address range, and the
         * preceding PMP address register forms the bottom of the address range.
         *	*/
        else if ((match_mode & PMP_ADDR_MATCH_MODE_SUPPORT) == PMP_TOR) {
            if (0 == idx) {
                addr_low = 0;
            } else {
                pmp_addr_get(idx - 1, &addr_low);
            }

            pmp_addr_get(idx, &addr_high);

            if ((address >= addr_low) && (address < addr_high)) {
                return true;
            }
        }

        // MA4:naturally aligned four-byte regions
        else if ((match_mode & PMP_ADDR_MATCH_MODE_SUPPORT) == PMP_NA4) {
            pmp_addr_get(idx, &addr_low);

            if ((address >= addr_low) && (address < (addr_low + 4))) {
                return true;
            }
        } else {
        }
    }

    return false;
}

uint8_t pmp_addr_get(uint32_t index, rv_csr_t *p_address)
{
    uint8_t res = PMP_OK;
    rv_csr_t address;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    address = __PMPADDR_INDX_GET(index);

    *p_address = address << 2;

    return res;
}

uint8_t pmp_addr_set(uint32_t index, rv_csr_t address)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg;
    rv_csr_t addr_current;
    rv_csr_t addr;

    if (address % 4) {
        // res = PMP_ADDR_NOT_ALIGNED_WARN;
#ifdef HALT_ON_WARNING
        {
            return res;
        }
#endif // HALT_ON_WARNING
    }

    if (given_addr_has_multiple_pmp_rule(index, address)) {
        res = PMP_REGION_MULTPL_RUNLE_WARN;
#ifdef HALT_ON_WARNING
        {
            return res;
        }
#endif // HALT_ON_WARNING
    }

    addr = address;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);
    // The L bit indicates that the PMP entry is locked, i.e., writes to the configuration register and associated
    // address registers are ignored
    if (pmpcfg & PMP_L) {
        addr_current = __PMPADDR_INDX_GET(index);

        if (addr_current != addr) {
            res = PMP_CONFIG_LOCKED_ERR;
        }
    } else {
        __PMPADDR_INDX_SET(index, addr);
    }

    return res;
}

uint8_t pmp_permission_get(uint32_t index, uint8_t *p_permission)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    *p_permission = pmpcfg & (PMP_R | PMP_W | PMP_X);

    return res;
}

uint8_t pmp_permission_set(uint32_t index, uint8_t permission)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg, current_perm;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);
    current_perm = pmpcfg & (PMP_R | PMP_W | PMP_X);

    if (0x00 != (permission & (~(PMP_R | PMP_W | PMP_X)))) {
        res = PMP_PARAM_ERR;
    }

    else if ((0 == (permission & PMP_R)) && (0 != (permission & PMP_W))) {
        res = PMP_PERM_COMBN_RSVD_ERR;
    }

    // The L bit indicates that the PMP entry is locked, i.e., writes to the configuration register and associated
    // address registers are ignored
    else if ((pmpcfg & PMP_L) && (current_perm != permission)) {
        res = PMP_CONFIG_LOCKED_ERR;
    } else {
        pmpcfg &= (uint8_t)(~(PMP_R | PMP_W | PMP_X));
        pmpcfg |= permission;
        __PMPCFG_INDX_SET(index, pmpcfg);
    }

    return res;
}

uint8_t pmp_addr_match_mode_get(uint32_t index, uint8_t *p_match_mode)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    *p_match_mode = (pmpcfg & PMP_ADDR_MATCH_MODE_SUPPORT);

    return res;
}

uint8_t pmp_addr_match_mode_set(uint32_t index, uint8_t match_mode)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg, current_mode;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    if (0x00 != (match_mode & (~PMP_ADDR_MATCH_MODE_SUPPORT))) {
        res = PMP_PARAM_ERR;
    }
    // The L bit indicates that the PMP entry is locked, i.e., writes to the configuration register and associated
    // address registers are ignored
    else if (pmpcfg & PMP_L) {
        current_mode = pmpcfg & PMP_ADDR_MATCH_MODE_SUPPORT;

        if (current_mode != match_mode) {
            res = PMP_CONFIG_LOCKED_ERR;
        }
    } else {
        // When G ≥ 1, the NA4 mode is not selectable.
        if ((PMP_HW_G > 0) && (PMP_NA4 == (match_mode & PMP_ADDR_MATCH_MODE_SUPPORT))) {
            res = PMP_NA4_HW_G_NOT_PAIRED_ERR;
        } else {
            if ((index > 0) && (PMP_TOR == (match_mode & PMP_ADDR_MATCH_MODE_SUPPORT))) {
                rv_csr_t addr_idx;
                rv_csr_t addr_idx_pre;

                addr_idx_pre = __PMPADDR_INDX_GET(index - 1);
                addr_idx = __PMPADDR_INDX_GET(index);

                if (addr_idx <= addr_idx_pre) {
                    res = PMP_TOR_ADDR_ERR;
                    return res;
                }
            }

            pmpcfg &= (uint8_t)(~PMP_ADDR_MATCH_MODE_SUPPORT);
            pmpcfg |= match_mode;
            __PMPCFG_INDX_SET(index, pmpcfg);
        }
    }

    return res;
}

uint8_t pmp_config_lock_get(uint32_t index, uint8_t *p_lock_status)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    *p_lock_status = pmpcfg & PMP_L;

    return res;
}

uint8_t pmp_config_lock_set(uint32_t index, uint8_t lock_status)
{
    uint8_t res = PMP_OK;
    uint8_t pmpcfg;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    if (0x00 != (lock_status & (~PMP_L))) {
        res = PMP_PARAM_ERR;
    }
    // The L bit indicates that the PMP entry is locked, i.e., writes to the configuration register and associated
    // address registers are ignored
    else if (pmpcfg & PMP_L) {
        if (0x00 == (lock_status & PMP_L)) {
            res = PMP_LOCK_CLEAR_NEED_RESET_ERR;
        }
    } else {
        pmpcfg &= (uint8_t)(~PMP_L);
        pmpcfg |= lock_status;
        __PMPCFG_INDX_SET(index, pmpcfg);
    }

    return res;
}

uint8_t pmp_config_get(uint32_t index, uint8_t *p_pmp_config)
{
    uint8_t res = PMP_OK;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    *p_pmp_config = __PMPCFG_INDX_GET(index);

    return res;
}

uint8_t pmp_config_set(uint32_t index, uint8_t pmp_config)
{
    uint8_t pmpcfg;
    uint8_t res = PMP_OK;

    if (index >= PMP_ENTRY_NUM) {
        return PMP_PARAM_ERR;
    }

    pmpcfg = __PMPCFG_INDX_GET(index);

    // The L bit indicates that the PMP entry is locked, i.e., writes to the configuration register and associated
    // address registers are ignored
    if (pmpcfg & PMP_L) {
        if (pmpcfg != pmp_config) {
            res = PMP_CONFIG_LOCKED_ERR;
        }
    } else {
        res = pmp_permission_set(index, pmp_config & (PMP_R | PMP_W | PMP_X));

        if (PMP_OK == res) {
            res = pmp_addr_match_mode_set(index, pmp_config & PMP_ADDR_MATCH_MODE_SUPPORT);
        }
        if (PMP_OK == res) {
            res = pmp_config_lock_set(index, pmp_config & PMP_L);
        }
    }

    return res;
}

#endif // PMP_PRESENT
