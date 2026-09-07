/**
 * @file pmp.h
 * @brief pmp driver header file.
 * @version 1.0
 * @date 2024-03-20
 *
 * Copyright (c) 2022 - 2024, WingSemi Technology LTD.
 *
 */
#ifndef __PMP_H__
#define __PMP_H__
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "soc_def.h"
#include "csr.h"
#include "encoding.h"

#include "debug.h"

/* ===== PMP Operations ===== */
/**
 * \defgroup PMP Functions   PMP Functions
 * \ingroup  WMSIS_Core
 * \brief    Functions that related to the RISCV Phyiscal Memory Protection.
 * \details
 * Optional physical memory protection (PMP) unit provides per-hart machine-mode
 * control registers to allow physical memory access privileges (read, write, execute)
 * to be specified for each physical memory region.
 *
 * The PMP can supports region access control settings as small as four bytes.
 *
 *   @{
 */
#define PMP_ENTRY_NUM 8
#ifndef PMP_ENTRY_NUM
/* numbers of PMP entries(PMP_ENTRY_NUM) should be defined in <Device.h> */
#error "PMP_ENTRY_NUM is not defined, please check!"
#endif

/**
 * \brief   Get 8bit PMPxCFG Register by PMP entry index
 * \details Return the content of the PMPxCFG Register.
 * \param [in]    idx    PMP region index(0-63)
 * \return               PMPxCFG Register value
 */
static inline uint8_t __PMPCFG_INDX_GET(uint32_t idx)
{
    rv_csr_t pmpcfg = 0;

    if (idx >= PMP_ENTRY_NUM)
        return 0;

#if __RISCV_XLEN == 32
    if (idx < 4) {
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG0);
    } else if ((idx >= 4) && (idx < 8)) {
        idx -= 4;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG1);
    } else if ((idx >= 8) && (idx < 12)) {
        idx -= 8;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG2);
    } else if ((idx >= 12) && (idx < 16)) {
        idx -= 12;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG3);
    }

    else if ((idx >= 16) && (idx < 20)) {
        idx -= 16;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG4);
    } else if ((idx >= 20) && (idx < 24)) {
        idx -= 20;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG5);
    } else if ((idx >= 24) && (idx < 28)) {
        idx -= 24;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG6);
    } else if ((idx >= 28) && (idx < 32)) {
        idx -= 28;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG7);
    }

    else if ((idx >= 32) && (idx < 36)) {
        idx -= 32;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG8);
    } else if ((idx >= 36) && (idx < 40)) {
        idx -= 36;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG9);
    } else if ((idx >= 40) && (idx < 44)) {
        idx -= 40;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG10);
    } else if ((idx >= 44) && (idx < 48)) {
        idx -= 44;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG11);
    }

    else if ((idx >= 48) && (idx < 52)) {
        idx -= 48;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG12);
    } else if ((idx >= 52) && (idx < 56)) {
        idx -= 52;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG13);
    } else if ((idx >= 56) && (idx < 60)) {
        idx -= 56;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG14);
    } else if ((idx >= 60) && (idx < 64)) {
        idx -= 60;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG15);
    }

    idx = idx << 3;
    return (uint8_t)((pmpcfg >> idx) & 0xFF);
#elif __RISCV_XLEN == 64
    if (idx < 8) {
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG0);
    } else if ((idx >= 8) && (idx < 16)) {
        idx -= 8;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG2);
    } else if ((idx >= 16) && (idx < 24)) {
        idx -= 16;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG4);
    } else if ((idx >= 24) && (idx < 32)) {
        idx -= 24;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG6);
    } else if ((idx >= 32) && (idx < 40)) {
        idx -= 32;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG8);
    } else if ((idx >= 40) && (idx < 48)) {
        idx -= 40;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG10);
    } else if ((idx >= 48) && (idx < 56)) {
        idx -= 48;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG12);
    } else if ((idx >= 56) && (idx < 64)) {
        idx -= 56;
        pmpcfg = __RV_CSR_READ(CSR_PMPCFG14);
    }

    idx = idx << 3;
    return (uint8_t)((pmpcfg >> idx) & 0xFF);
#else
    // TODO Add RV128 Handling
    return 0;
#endif
}

/**
 * \brief   Set 8bit PMPxCFG by pmp entry index
 * \details Set the given pmpxcfg value to the PMPxCFG Register.
 * \param [in]    idx      PMPx region index(0-63)
 * \param [in]    pmpxcfg  PMPxCFG register value to set
 */
static inline void __PMPCFG_INDX_SET(uint32_t idx, uint8_t pmpxcfg)
{
    rv_csr_t pmpcfgx = 0;
    if (idx >= PMP_ENTRY_NUM)
        return;

#if __RISCV_XLEN == 32
    if (idx < 4) {
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG0);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG0, pmpcfgx);
    } else if ((idx >= 4) && (idx < 8)) {
        idx -= 4;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG1);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG1, pmpcfgx);
    } else if ((idx >= 8) && (idx < 12)) {
        idx -= 8;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG2);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG2, pmpcfgx);
    } else if ((idx >= 12) && (idx < 16)) {
        idx -= 12;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG3);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG3, pmpcfgx);
    }

    else if ((idx >= 16) && (idx < 20)) {
        idx -= 16;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG4);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG4, pmpcfgx);
    } else if ((idx >= 20) && (idx < 24)) {
        idx -= 20;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG5);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG5, pmpcfgx);
    } else if ((idx >= 24) && (idx < 28)) {
        idx -= 24;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG6);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG6, pmpcfgx);
    } else if ((idx >= 28) && (idx < 32)) {
        idx -= 28;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG7);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG7, pmpcfgx);
    }

    else if ((idx >= 32) && (idx < 36)) {
        idx -= 32;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG8);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG8, pmpcfgx);
    } else if ((idx >= 36) && (idx < 40)) {
        idx -= 36;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG9);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG9, pmpcfgx);
    } else if ((idx >= 40) && (idx < 44)) {
        idx -= 40;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG10);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG10, pmpcfgx);
    } else if ((idx >= 44) && (idx < 48)) {
        idx -= 44;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG11);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG11, pmpcfgx);
    }

    else if ((idx >= 48) && (idx < 52)) {
        idx -= 48;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG12);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG12, pmpcfgx);
    } else if ((idx >= 52) && (idx < 56)) {
        idx -= 52;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG13);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG13, pmpcfgx);
    } else if ((idx >= 56) && (idx < 60)) {
        idx -= 56;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG14);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG14, pmpcfgx);
    } else if ((idx >= 60) && (idx < 64)) {
        idx -= 60;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG15);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG15, pmpcfgx);
    }
#elif __RISCV_XLEN == 64
    if (idx < 8) {
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG0);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG0, pmpcfgx);
    } else if ((idx >= 8) && (idx < 16)) {
        idx -= 8;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG2);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG2, pmpcfgx);
    } else if ((idx >= 16) && (idx < 24)) {
        idx -= 16;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG4);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG4, pmpcfgx);
    } else if ((idx >= 24) && (idx < 32)) {
        idx -= 24;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG6);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG6, pmpcfgx);
    } else if ((idx >= 32) && (idx < 40)) {
        idx -= 32;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG8);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG8, pmpcfgx);
    } else if ((idx >= 40) && (idx < 48)) {
        idx -= 40;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG10);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG10, pmpcfgx);
    } else if ((idx >= 48) && (idx < 56)) {
        idx -= 48;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG12);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG12, pmpcfgx);
    } else if ((idx >= 56) && (idx < 64)) {
        idx -= 56;
        pmpcfgx = __RV_CSR_READ(CSR_PMPCFG14);
        idx = idx << 3;
        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
        __RV_CSR_WRITE(CSR_PMPCFG14, pmpcfgx);
    }
#else
    // TODO Add RV128 Handling
#endif
}

/**
 * \brief   Get PMPADDRx Register by index
 * \details Return the content of the PMPADDRx Register.
 * \param [in]    idx    PMP region index(0-63)
 * \return               PMPADDRx Register value
 */
static inline rv_csr_t __PMPADDR_INDX_GET(uint32_t idx)
{
    if (idx >= PMP_ENTRY_NUM)
        return (rv_csr_t)(0 - 1);
    switch (idx) {
    case 0:
        return __RV_CSR_READ(CSR_PMPADDR0);
    case 1:
        return __RV_CSR_READ(CSR_PMPADDR1);
    case 2:
        return __RV_CSR_READ(CSR_PMPADDR2);
    case 3:
        return __RV_CSR_READ(CSR_PMPADDR3);
    case 4:
        return __RV_CSR_READ(CSR_PMPADDR4);
    case 5:
        return __RV_CSR_READ(CSR_PMPADDR5);
    case 6:
        return __RV_CSR_READ(CSR_PMPADDR6);
    case 7:
        return __RV_CSR_READ(CSR_PMPADDR7);
    case 8:
        return __RV_CSR_READ(CSR_PMPADDR8);
    case 9:
        return __RV_CSR_READ(CSR_PMPADDR9);
    case 10:
        return __RV_CSR_READ(CSR_PMPADDR10);
    case 11:
        return __RV_CSR_READ(CSR_PMPADDR11);
    case 12:
        return __RV_CSR_READ(CSR_PMPADDR12);
    case 13:
        return __RV_CSR_READ(CSR_PMPADDR13);
    case 14:
        return __RV_CSR_READ(CSR_PMPADDR14);
    case 15:
        return __RV_CSR_READ(CSR_PMPADDR15);

    case 16:
        return __RV_CSR_READ(CSR_PMPADDR16);
    case 17:
        return __RV_CSR_READ(CSR_PMPADDR17);
    case 18:
        return __RV_CSR_READ(CSR_PMPADDR18);
    case 19:
        return __RV_CSR_READ(CSR_PMPADDR19);
    case 20:
        return __RV_CSR_READ(CSR_PMPADDR20);
    case 21:
        return __RV_CSR_READ(CSR_PMPADDR21);
    case 22:
        return __RV_CSR_READ(CSR_PMPADDR22);
    case 23:
        return __RV_CSR_READ(CSR_PMPADDR23);
    case 24:
        return __RV_CSR_READ(CSR_PMPADDR24);
    case 25:
        return __RV_CSR_READ(CSR_PMPADDR25);
    case 26:
        return __RV_CSR_READ(CSR_PMPADDR26);
    case 27:
        return __RV_CSR_READ(CSR_PMPADDR27);
    case 28:
        return __RV_CSR_READ(CSR_PMPADDR28);
    case 29:
        return __RV_CSR_READ(CSR_PMPADDR29);
    case 30:
        return __RV_CSR_READ(CSR_PMPADDR30);
    case 31:
        return __RV_CSR_READ(CSR_PMPADDR31);

    case 32:
        return __RV_CSR_READ(CSR_PMPADDR32);
    case 33:
        return __RV_CSR_READ(CSR_PMPADDR33);
    case 34:
        return __RV_CSR_READ(CSR_PMPADDR34);
    case 35:
        return __RV_CSR_READ(CSR_PMPADDR35);
    case 36:
        return __RV_CSR_READ(CSR_PMPADDR36);
    case 37:
        return __RV_CSR_READ(CSR_PMPADDR37);
    case 38:
        return __RV_CSR_READ(CSR_PMPADDR38);
    case 39:
        return __RV_CSR_READ(CSR_PMPADDR39);
    case 40:
        return __RV_CSR_READ(CSR_PMPADDR40);
    case 41:
        return __RV_CSR_READ(CSR_PMPADDR41);
    case 42:
        return __RV_CSR_READ(CSR_PMPADDR42);
    case 43:
        return __RV_CSR_READ(CSR_PMPADDR43);
    case 44:
        return __RV_CSR_READ(CSR_PMPADDR44);
    case 45:
        return __RV_CSR_READ(CSR_PMPADDR45);
    case 46:
        return __RV_CSR_READ(CSR_PMPADDR46);
    case 47:
        return __RV_CSR_READ(CSR_PMPADDR47);

    case 48:
        return __RV_CSR_READ(CSR_PMPADDR48);
    case 49:
        return __RV_CSR_READ(CSR_PMPADDR49);
    case 50:
        return __RV_CSR_READ(CSR_PMPADDR50);
    case 51:
        return __RV_CSR_READ(CSR_PMPADDR51);
    case 52:
        return __RV_CSR_READ(CSR_PMPADDR52);
    case 53:
        return __RV_CSR_READ(CSR_PMPADDR53);
    case 54:
        return __RV_CSR_READ(CSR_PMPADDR54);
    case 55:
        return __RV_CSR_READ(CSR_PMPADDR55);
    case 56:
        return __RV_CSR_READ(CSR_PMPADDR56);
    case 57:
        return __RV_CSR_READ(CSR_PMPADDR57);
    case 58:
        return __RV_CSR_READ(CSR_PMPADDR58);
    case 59:
        return __RV_CSR_READ(CSR_PMPADDR59);
    case 60:
        return __RV_CSR_READ(CSR_PMPADDR60);
    case 61:
        return __RV_CSR_READ(CSR_PMPADDR61);
    case 62:
        return __RV_CSR_READ(CSR_PMPADDR62);
    case 63:
        return __RV_CSR_READ(CSR_PMPADDR63);
    default:
        return 0;
    }
}

/**
 * \brief   Set PMPADDRx by index
 * \details Write the given value to the PMPADDRx Register.
 * \param [in]    idx      PMP region index(0-63)
 * \param [in]    pmpaddr  PMPADDRx Register value to set
 */
static inline void __PMPADDR_INDX_SET(uint32_t idx, rv_csr_t pmpaddr)
{
    if (idx >= PMP_ENTRY_NUM)
        return;
    switch (idx) {
    case 0:
        __RV_CSR_WRITE(CSR_PMPADDR0, pmpaddr);
        break;
    case 1:
        __RV_CSR_WRITE(CSR_PMPADDR1, pmpaddr);
        break;
    case 2:
        __RV_CSR_WRITE(CSR_PMPADDR2, pmpaddr);
        break;
    case 3:
        __RV_CSR_WRITE(CSR_PMPADDR3, pmpaddr);
        break;
    case 4:
        __RV_CSR_WRITE(CSR_PMPADDR4, pmpaddr);
        break;
    case 5:
        __RV_CSR_WRITE(CSR_PMPADDR5, pmpaddr);
        break;
    case 6:
        __RV_CSR_WRITE(CSR_PMPADDR6, pmpaddr);
        break;
    case 7:
        __RV_CSR_WRITE(CSR_PMPADDR7, pmpaddr);
        break;
    case 8:
        __RV_CSR_WRITE(CSR_PMPADDR8, pmpaddr);
        break;
    case 9:
        __RV_CSR_WRITE(CSR_PMPADDR9, pmpaddr);
        break;
    case 10:
        __RV_CSR_WRITE(CSR_PMPADDR10, pmpaddr);
        break;
    case 11:
        __RV_CSR_WRITE(CSR_PMPADDR11, pmpaddr);
        break;
    case 12:
        __RV_CSR_WRITE(CSR_PMPADDR12, pmpaddr);
        break;
    case 13:
        __RV_CSR_WRITE(CSR_PMPADDR13, pmpaddr);
        break;
    case 14:
        __RV_CSR_WRITE(CSR_PMPADDR14, pmpaddr);
        break;
    case 15:
        __RV_CSR_WRITE(CSR_PMPADDR15, pmpaddr);
        break;

    case 16:
        __RV_CSR_WRITE(CSR_PMPADDR16, pmpaddr);
        break;
    case 17:
        __RV_CSR_WRITE(CSR_PMPADDR17, pmpaddr);
        break;
    case 18:
        __RV_CSR_WRITE(CSR_PMPADDR18, pmpaddr);
        break;
    case 19:
        __RV_CSR_WRITE(CSR_PMPADDR19, pmpaddr);
        break;
    case 20:
        __RV_CSR_WRITE(CSR_PMPADDR20, pmpaddr);
        break;
    case 21:
        __RV_CSR_WRITE(CSR_PMPADDR21, pmpaddr);
        break;
    case 22:
        __RV_CSR_WRITE(CSR_PMPADDR22, pmpaddr);
        break;
    case 23:
        __RV_CSR_WRITE(CSR_PMPADDR23, pmpaddr);
        break;
    case 24:
        __RV_CSR_WRITE(CSR_PMPADDR24, pmpaddr);
        break;
    case 25:
        __RV_CSR_WRITE(CSR_PMPADDR25, pmpaddr);
        break;
    case 26:
        __RV_CSR_WRITE(CSR_PMPADDR26, pmpaddr);
        break;
    case 27:
        __RV_CSR_WRITE(CSR_PMPADDR27, pmpaddr);
        break;
    case 28:
        __RV_CSR_WRITE(CSR_PMPADDR28, pmpaddr);
        break;
    case 29:
        __RV_CSR_WRITE(CSR_PMPADDR29, pmpaddr);
        break;
    case 30:
        __RV_CSR_WRITE(CSR_PMPADDR30, pmpaddr);
        break;
    case 31:
        __RV_CSR_WRITE(CSR_PMPADDR31, pmpaddr);
        break;

    case 32:
        __RV_CSR_WRITE(CSR_PMPADDR32, pmpaddr);
        break;
    case 33:
        __RV_CSR_WRITE(CSR_PMPADDR33, pmpaddr);
        break;
    case 34:
        __RV_CSR_WRITE(CSR_PMPADDR34, pmpaddr);
        break;
    case 35:
        __RV_CSR_WRITE(CSR_PMPADDR35, pmpaddr);
        break;
    case 36:
        __RV_CSR_WRITE(CSR_PMPADDR36, pmpaddr);
        break;
    case 37:
        __RV_CSR_WRITE(CSR_PMPADDR37, pmpaddr);
        break;
    case 38:
        __RV_CSR_WRITE(CSR_PMPADDR38, pmpaddr);
        break;
    case 39:
        __RV_CSR_WRITE(CSR_PMPADDR39, pmpaddr);
        break;
    case 40:
        __RV_CSR_WRITE(CSR_PMPADDR40, pmpaddr);
        break;
    case 41:
        __RV_CSR_WRITE(CSR_PMPADDR41, pmpaddr);
        break;
    case 42:
        __RV_CSR_WRITE(CSR_PMPADDR42, pmpaddr);
        break;
    case 43:
        __RV_CSR_WRITE(CSR_PMPADDR43, pmpaddr);
        break;
    case 44:
        __RV_CSR_WRITE(CSR_PMPADDR44, pmpaddr);
        break;
    case 45:
        __RV_CSR_WRITE(CSR_PMPADDR45, pmpaddr);
        break;
    case 46:
        __RV_CSR_WRITE(CSR_PMPADDR46, pmpaddr);
        break;
    case 47:
        __RV_CSR_WRITE(CSR_PMPADDR47, pmpaddr);
        break;

    case 48:
        __RV_CSR_WRITE(CSR_PMPADDR48, pmpaddr);
        break;
    case 49:
        __RV_CSR_WRITE(CSR_PMPADDR49, pmpaddr);
        break;
    case 50:
        __RV_CSR_WRITE(CSR_PMPADDR50, pmpaddr);
        break;
    case 51:
        __RV_CSR_WRITE(CSR_PMPADDR51, pmpaddr);
        break;
    case 52:
        __RV_CSR_WRITE(CSR_PMPADDR52, pmpaddr);
        break;
    case 53:
        __RV_CSR_WRITE(CSR_PMPADDR53, pmpaddr);
        break;
    case 54:
        __RV_CSR_WRITE(CSR_PMPADDR54, pmpaddr);
        break;
    case 55:
        __RV_CSR_WRITE(CSR_PMPADDR55, pmpaddr);
        break;
    case 56:
        __RV_CSR_WRITE(CSR_PMPADDR56, pmpaddr);
        break;
    case 57:
        __RV_CSR_WRITE(CSR_PMPADDR57, pmpaddr);
        break;
    case 58:
        __RV_CSR_WRITE(CSR_PMPADDR58, pmpaddr);
        break;
    case 59:
        __RV_CSR_WRITE(CSR_PMPADDR59, pmpaddr);
        break;
    case 60:
        __RV_CSR_WRITE(CSR_PMPADDR60, pmpaddr);
        break;
    case 61:
        __RV_CSR_WRITE(CSR_PMPADDR61, pmpaddr);
        break;
    case 62:
        __RV_CSR_WRITE(CSR_PMPADDR62, pmpaddr);
        break;
    case 63:
        __RV_CSR_WRITE(CSR_PMPADDR63, pmpaddr);
        break;
    default:
        return;
    }
}
/** @} */ /* End of Doxygen Group WMSIS_Core_PMP_Functions */

typedef enum {
    PMP_OK,

    PMP_RES_START_ERR,
    PMP_PARAM_ERR,                 // Given value not available
    PMP_NA4_HW_G_NOT_PAIRED_ERR,   // When G >= 1, the NA4 mode is not selectable
    PMP_LOCK_CLEAR_NEED_RESET_ERR, // config lock can be cleared only by reset
    PMP_CONFIG_LOCKED_ERR,         // when config locked,config and addr change not allowed
    PMP_TOR_ADDR_ERR,              // tor addr match mode,addr_X shoud larger than  addr_X-1
    PMP_PERM_COMBN_RSVD_ERR,       // permission combinations with R=0 and W=1 are reserved

    PMP_RES_START_WARN,
    PMP_ADDR_NOT_ALIGNED_WARN,    // pmp addr should 4 byte aligned
    PMP_REGION_MULTPL_RUNLE_WARN, // given has multiple pmp rule setting
} pmp_drc_res_t;

struct pmp_region_s {
    uint16_t index;
    uint32_t address;

    bool read_permission;
    bool write_permission;
    bool excute_permission;

    uint8_t address_match_mode;

    bool lock_bit;
};

#if defined(HALT_ON_WARNING)
#define PMP_ERROR_CHECK(ERR_CODE)                                                                    \
    do {                                                                                             \
        if (ERR_CODE != PMP_OK) {                                                                    \
            if (ERR_CODE < PMP_RES_START_WARN) {                                                     \
                printf("Error code:0x%x,function:%s,line:%d\n", ERR_CODE, __FUNCTION__, __LINE__);   \
                while (1)                                                                            \
                    ;                                                                                \
            } else {                                                                                 \
                printf("Warning code:0x%x,function:%s,line:%d\n", ERR_CODE, __FUNCTION__, __LINE__); \
                while (1)                                                                            \
                    ;                                                                                \
            }                                                                                        \
        }                                                                                            \
    } while (0)
#else
#define PMP_ERROR_CHECK(ERR_CODE)                                                                    \
    do {                                                                                             \
        if (ERR_CODE != PMP_OK) {                                                                    \
            if (ERR_CODE < PMP_RES_START_WARN) {                                                     \
                printf("Error code:0x%x,function:%s,line:%d\n", ERR_CODE, __FUNCTION__, __LINE__);   \
                while (1)                                                                            \
                    ;                                                                                \
            } else {                                                                                 \
                printf("Warning code:0x%x,function:%s,line:%d\n", ERR_CODE, __FUNCTION__, __LINE__); \
            }                                                                                        \
        }                                                                                            \
    } while (0)
#endif //

#define PMP_ADDR_MATCH_MODE_SUPPORT (PMP_TOR | PMP_NA4 | PMP_NAPOT)

uint8_t get_one_number_of_addr(rv_csr_t address);

uint8_t pmp_addr_get(uint32_t index, rv_csr_t *p_address);
uint8_t pmp_addr_set(uint32_t index, rv_csr_t address);

uint8_t pmp_permission_get(uint32_t index, uint8_t *p_permission);
uint8_t pmp_permission_set(uint32_t index, uint8_t permission);

uint8_t pmp_addr_match_mode_get(uint32_t index, uint8_t *p_match_mode);
uint8_t pmp_addr_match_mode_set(uint32_t index, uint8_t match_mode);

uint8_t pmp_config_lock_get(uint32_t index, uint8_t *p_lock_status);
uint8_t pmp_config_lock_set(uint32_t index, uint8_t lock_status);

uint8_t pmp_config_get(uint32_t index, uint8_t *p_pmp_config);
uint8_t pmp_config_set(uint32_t index, uint8_t pmp_config);

#endif /** __PMP_H__  */
