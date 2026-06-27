/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef _PMP_H_
#define _PMP_H_
/*!
 * @file    pmp.h
 */
#ifdef __cplusplus
 extern "C" {
#endif

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
__STATIC_INLINE uint8_t __PMPCFG_INDX_GET(uint32_t idx)
{
	rv_csr_t pmpcfg = 0;

	if (idx >= PMP_ENTRY_NUM) return 0;

	#if __RISCV_XLEN == 32
	    if (idx < 4) {
	        pmpcfg = read_csr(CSR_PMPCFG0);
	    } else if ((idx >=4) && (idx < 8)) {
	        idx -= 4;
	        pmpcfg = read_csr(CSR_PMPCFG1);
	    } else if ((idx >=8) && (idx < 12)) {
	        idx -= 8;
	        pmpcfg = read_csr(CSR_PMPCFG2);
	    } else if ((idx >=12) && (idx < 16)) {
	        idx -= 12;
	        pmpcfg = read_csr(CSR_PMPCFG3);
	    }

	    else if ((idx >=16) && (idx < 20)) {
	        idx -= 16;
	        pmpcfg = read_csr(CSR_PMPCFG4);
	    } else if ((idx >=20) && (idx < 24)) {
	        idx -= 20;
	        pmpcfg = read_csr(CSR_PMPCFG5);
	    } else if ((idx >=24) && (idx < 28)) {
	        idx -= 24;
	        pmpcfg = read_csr(CSR_PMPCFG6);
	    } else if ((idx >=28) && (idx < 32)) {
	        idx -= 28;
	        pmpcfg = read_csr(CSR_PMPCFG7);
	    }

	    else if ((idx >=32) && (idx < 36)) {
	        idx -= 32;
	        pmpcfg = read_csr(CSR_PMPCFG8);
	    } else if ((idx >=36) && (idx < 40)) {
	        idx -= 36;
	        pmpcfg = read_csr(CSR_PMPCFG9);
	    } else if ((idx >=40) && (idx < 44)) {
	        idx -= 40;
	        pmpcfg = read_csr(CSR_PMPCFG10);
	    } else if ((idx >=44) && (idx < 48)) {
	        idx -= 44;
	        pmpcfg = read_csr(CSR_PMPCFG11);
	    }

	    else if ((idx >=48) && (idx < 52)) {
	        idx -= 48;
	        pmpcfg = read_csr(CSR_PMPCFG12);
	    } else if ((idx >=52) && (idx < 56)) {
	        idx -= 52;
	        pmpcfg = read_csr(CSR_PMPCFG13);
	    } else if ((idx >=56) && (idx < 60)) {
	        idx -= 56;
	        pmpcfg = read_csr(CSR_PMPCFG14);
	    } else if ((idx >=60) && (idx < 64)) {
	        idx -= 60;
	        pmpcfg = read_csr(CSR_PMPCFG15);
	    }

	    idx = idx << 3;
	    return (uint8_t)((pmpcfg>>idx) & 0xFF);
	#elif __RISCV_XLEN == 64
	    if (idx < 8) {
	        pmpcfg = read_csr(CSR_PMPCFG0);
	    }
	    else if ((idx >=8) && (idx < 16)) {
	        idx -= 8;
	        pmpcfg = read_csr(CSR_PMPCFG2);
	    }
	    else if ((idx >=16) && (idx < 24)) {
	        idx -= 16;
	        pmpcfg = read_csr(CSR_PMPCFG4);
	    }
	    else if ((idx >=24) && (idx < 32)) {
	        idx -= 24;
	        pmpcfg = read_csr(CSR_PMPCFG6);
	    }
	    else if ((idx >=32) && (idx < 40)) {
	        idx -= 32;
	        pmpcfg = read_csr(CSR_PMPCFG8);
	    }
	    else if ((idx >=40) && (idx < 48)) {
	        idx -= 40;
	        pmpcfg = read_csr(CSR_PMPCFG10);
	    }
	    else if ((idx >=48) && (idx < 56)) {
	        idx -= 48;
	        pmpcfg = read_csr(CSR_PMPCFG12);
	    }
	    else if ((idx >=56) && (idx < 64)) {
	        idx -= 56;
	        pmpcfg = read_csr(CSR_PMPCFG14);
	    }

	    idx = idx << 3;
	    return (uint8_t)((pmpcfg>>idx) & 0xFF);
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
__STATIC_INLINE void __PMPCFG_INDX_SET(uint32_t idx, uint8_t pmpxcfg)
{
	rv_csr_t pmpcfgx = 0;
	if (idx >= PMP_ENTRY_NUM) return;

	#if __RISCV_XLEN == 32
	    if (idx < 4) {
	        pmpcfgx = read_csr(CSR_PMPCFG0);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG0, pmpcfgx);
	    } else if ((idx >=4) && (idx < 8)) {
	        idx -= 4;
	        pmpcfgx = read_csr(CSR_PMPCFG1);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG1, pmpcfgx);
	    } else if ((idx >=8) && (idx < 12)) {
	        idx -= 8;
	        pmpcfgx = read_csr(CSR_PMPCFG2);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG2, pmpcfgx);
	    } else if ((idx >=12) && (idx < 16)) {
	        idx -= 12;
	        pmpcfgx = read_csr(CSR_PMPCFG3);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG3, pmpcfgx);
	    }

	    else if ((idx >=16) && (idx < 20)) {
	        idx -= 16;
	        pmpcfgx = read_csr(CSR_PMPCFG4);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG4, pmpcfgx);
	    } else if ((idx >=20) && (idx < 24)) {
	        idx -= 20;
	        pmpcfgx = read_csr(CSR_PMPCFG5);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG5, pmpcfgx);
	    } else if ((idx >=24) && (idx < 28)) {
	        idx -= 24;
	        pmpcfgx = read_csr(CSR_PMPCFG6);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG6, pmpcfgx);
	    } else if ((idx >=28) && (idx < 32)) {
	        idx -= 28;
	        pmpcfgx = read_csr(CSR_PMPCFG7);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG7, pmpcfgx);
	    }

	    else if ((idx >=32) && (idx < 36)) {
	        idx -= 32;
	        pmpcfgx = read_csr(CSR_PMPCFG8);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG8, pmpcfgx);
	    } else if ((idx >=36) && (idx < 40)) {
	        idx -= 36;
	        pmpcfgx = read_csr(CSR_PMPCFG9);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG9, pmpcfgx);
	    } else if ((idx >=40) && (idx < 44)) {
	        idx -= 40;
	        pmpcfgx = read_csr(CSR_PMPCFG10);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG10, pmpcfgx);
	    } else if ((idx >=44) && (idx < 48)) {
	        idx -= 44;
	        pmpcfgx = read_csr(CSR_PMPCFG11);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG11, pmpcfgx);
	    }

	    else if ((idx >=48) && (idx < 52)) {
	        idx -= 48;
	        pmpcfgx = read_csr(CSR_PMPCFG12);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG12, pmpcfgx);
	    } else if ((idx >=52) && (idx < 56)) {
	        idx -= 52;
	        pmpcfgx = read_csr(CSR_PMPCFG13);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG13, pmpcfgx);
	    } else if ((idx >=56) && (idx < 60)) {
	        idx -= 56;
	        pmpcfgx = read_csr(CSR_PMPCFG14);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG14, pmpcfgx);
	    } else if ((idx >=60) && (idx < 64)) {
	        idx -= 60;
	        pmpcfgx = read_csr(CSR_PMPCFG15);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFUL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG15, pmpcfgx);
	    }
	#elif __RISCV_XLEN == 64
	    if (idx < 8) {
	        pmpcfgx = read_csr(CSR_PMPCFG0);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG0, pmpcfgx);
	    } else if ((idx >=8) && (idx < 16)) {
	        idx -= 8;
	        pmpcfgx = read_csr(CSR_PMPCFG2);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG2, pmpcfgx);
	    }else if ((idx >=16) && (idx < 24)) {
	        idx -= 16;
	        pmpcfgx = read_csr(CSR_PMPCFG4);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG4, pmpcfgx);
	    }else if ((idx >=24) && (idx < 32)) {
	        idx -= 24;
	        pmpcfgx = read_csr(CSR_PMPCFG6);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG6, pmpcfgx);
	    }else if ((idx >=32) && (idx < 40)) {
	        idx -= 32;
	        pmpcfgx = read_csr(CSR_PMPCFG8);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG8, pmpcfgx);
	    }else if ((idx >=40) && (idx < 48)) {
	        idx -= 40;
	        pmpcfgx = read_csr(CSR_PMPCFG10);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG10, pmpcfgx);
	    }else if ((idx >=48) && (idx < 56)) {
	        idx -= 48;
	        pmpcfgx = read_csr(CSR_PMPCFG12);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG12, pmpcfgx);
	    }else if ((idx >=56) && (idx < 64)) {
	        idx -= 56;
	        pmpcfgx = read_csr(CSR_PMPCFG14);
	        idx = idx << 3;
	        pmpcfgx = (pmpcfgx & ~(0xFFULL << idx)) | ((rv_csr_t)pmpxcfg << idx);
	        write_csr(CSR_PMPCFG14, pmpcfgx);
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
__STATIC_INLINE rv_csr_t __PMPADDR_INDX_GET(uint32_t idx)
{
	if (idx >= PMP_ENTRY_NUM) return (0-1);
    switch (idx) {
    case 0: return read_csr(CSR_PMPADDR0);
    case 1: return read_csr(CSR_PMPADDR1);
    case 2: return read_csr(CSR_PMPADDR2);
    case 3: return read_csr(CSR_PMPADDR3);
    case 4: return read_csr(CSR_PMPADDR4);
    case 5: return read_csr(CSR_PMPADDR5);
    case 6: return read_csr(CSR_PMPADDR6);
    case 7: return read_csr(CSR_PMPADDR7);
    case 8: return read_csr(CSR_PMPADDR8);
    case 9: return read_csr(CSR_PMPADDR9);
    case 10: return read_csr(CSR_PMPADDR10);
    case 11: return read_csr(CSR_PMPADDR11);
    case 12: return read_csr(CSR_PMPADDR12);
    case 13: return read_csr(CSR_PMPADDR13);
    case 14: return read_csr(CSR_PMPADDR14);
    case 15: return read_csr(CSR_PMPADDR15);

    case 16: return read_csr(CSR_PMPADDR16);
    case 17: return read_csr(CSR_PMPADDR17);
    case 18: return read_csr(CSR_PMPADDR18);
    case 19:return read_csr(CSR_PMPADDR19);
    case 20:return read_csr(CSR_PMPADDR20);
    case 21:return read_csr(CSR_PMPADDR21);
    case 22:return read_csr(CSR_PMPADDR22);
    case 23:return read_csr(CSR_PMPADDR23);
    case 24:return read_csr(CSR_PMPADDR24);
    case 25:return read_csr(CSR_PMPADDR25);
    case 26: return read_csr(CSR_PMPADDR26);
    case 27: return read_csr(CSR_PMPADDR27);
    case 28: return read_csr(CSR_PMPADDR28);
    case 29: return read_csr(CSR_PMPADDR29);
    case 30: return read_csr(CSR_PMPADDR30);
    case 31: return read_csr(CSR_PMPADDR31);

    case 32:return read_csr(CSR_PMPADDR32);
    case 33:return read_csr(CSR_PMPADDR33);
    case 34:return read_csr(CSR_PMPADDR34);
    case 35:return read_csr(CSR_PMPADDR35);
    case 36:return read_csr(CSR_PMPADDR36);
    case 37:return read_csr(CSR_PMPADDR37);
    case 38:return read_csr(CSR_PMPADDR38);
    case 39:return read_csr(CSR_PMPADDR39);
    case 40:return read_csr(CSR_PMPADDR40);
    case 41:return read_csr(CSR_PMPADDR41);
    case 42: return read_csr(CSR_PMPADDR42);
    case 43: return read_csr(CSR_PMPADDR43);
    case 44: return read_csr(CSR_PMPADDR44);
    case 45: return read_csr(CSR_PMPADDR45);
    case 46: return read_csr(CSR_PMPADDR46);
    case 47: return read_csr(CSR_PMPADDR47);


    case 48:return read_csr(CSR_PMPADDR48);
    case 49:return read_csr(CSR_PMPADDR49);
    case 50:return read_csr(CSR_PMPADDR50);
    case 51:return read_csr(CSR_PMPADDR51);
    case 52:return read_csr(CSR_PMPADDR52);
    case 53:return read_csr(CSR_PMPADDR53);
    case 54:return read_csr(CSR_PMPADDR54);
    case 55:return read_csr(CSR_PMPADDR55);
    case 56:return read_csr(CSR_PMPADDR56);
    case 57:return read_csr(CSR_PMPADDR57);
    case 58: return read_csr(CSR_PMPADDR58);
    case 59: return read_csr(CSR_PMPADDR59);
    case 60: return read_csr(CSR_PMPADDR60);
    case 61: return read_csr(CSR_PMPADDR61);
    case 62: return read_csr(CSR_PMPADDR62);
    case 63: return read_csr(CSR_PMPADDR63);
        default: return 0;
    }
}

/**
 * \brief   Set PMPADDRx by index
 * \details Write the given value to the PMPADDRx Register.
 * \param [in]    idx      PMP region index(0-63)
 * \param [in]    pmpaddr  PMPADDRx Register value to set
 */
__STATIC_INLINE void __PMPADDR_INDX_SET(uint32_t idx, rv_csr_t pmpaddr)
{
	 if (idx >= PMP_ENTRY_NUM) return;
    switch (idx) {
        case 0: write_csr(CSR_PMPADDR0, pmpaddr); break;
        case 1: write_csr(CSR_PMPADDR1, pmpaddr); break;
        case 2: write_csr(CSR_PMPADDR2, pmpaddr); break;
        case 3: write_csr(CSR_PMPADDR3, pmpaddr); break;
        case 4: write_csr(CSR_PMPADDR4, pmpaddr); break;
        case 5: write_csr(CSR_PMPADDR5, pmpaddr); break;
        case 6: write_csr(CSR_PMPADDR6, pmpaddr); break;
        case 7: write_csr(CSR_PMPADDR7, pmpaddr); break;
        case 8: write_csr(CSR_PMPADDR8, pmpaddr); break;
        case 9: write_csr(CSR_PMPADDR9, pmpaddr); break;
        case 10: write_csr(CSR_PMPADDR10, pmpaddr); break;
        case 11: write_csr(CSR_PMPADDR11, pmpaddr); break;
        case 12: write_csr(CSR_PMPADDR12, pmpaddr); break;
        case 13: write_csr(CSR_PMPADDR13, pmpaddr); break;
        case 14: write_csr(CSR_PMPADDR14, pmpaddr); break;
        case 15: write_csr(CSR_PMPADDR15, pmpaddr); break;

        case 16: write_csr(CSR_PMPADDR16, pmpaddr); break;
        case 17: write_csr(CSR_PMPADDR17, pmpaddr); break;
        case 18: write_csr(CSR_PMPADDR18, pmpaddr); break;
        case 19: write_csr(CSR_PMPADDR19, pmpaddr); break;
        case 20: write_csr(CSR_PMPADDR20, pmpaddr); break;
        case 21: write_csr(CSR_PMPADDR21, pmpaddr); break;
        case 22: write_csr(CSR_PMPADDR22, pmpaddr); break;
        case 23: write_csr(CSR_PMPADDR23, pmpaddr); break;
        case 24: write_csr(CSR_PMPADDR24, pmpaddr); break;
        case 25: write_csr(CSR_PMPADDR25, pmpaddr); break;
        case 26: write_csr(CSR_PMPADDR26, pmpaddr); break;
        case 27: write_csr(CSR_PMPADDR27, pmpaddr); break;
        case 28: write_csr(CSR_PMPADDR28, pmpaddr); break;
        case 29: write_csr(CSR_PMPADDR29, pmpaddr); break;
        case 30: write_csr(CSR_PMPADDR30, pmpaddr); break;
        case 31: write_csr(CSR_PMPADDR31, pmpaddr); break;

        case 32: write_csr(CSR_PMPADDR32, pmpaddr); break;
        case 33: write_csr(CSR_PMPADDR33, pmpaddr); break;
        case 34: write_csr(CSR_PMPADDR34, pmpaddr); break;
        case 35: write_csr(CSR_PMPADDR35, pmpaddr); break;
        case 36: write_csr(CSR_PMPADDR36, pmpaddr); break;
        case 37: write_csr(CSR_PMPADDR37, pmpaddr); break;
        case 38: write_csr(CSR_PMPADDR38, pmpaddr); break;
        case 39: write_csr(CSR_PMPADDR39, pmpaddr); break;
        case 40: write_csr(CSR_PMPADDR40, pmpaddr); break;
        case 41: write_csr(CSR_PMPADDR41, pmpaddr); break;
        case 42: write_csr(CSR_PMPADDR42, pmpaddr); break;
        case 43: write_csr(CSR_PMPADDR43, pmpaddr); break;
        case 44: write_csr(CSR_PMPADDR44, pmpaddr); break;
        case 45: write_csr(CSR_PMPADDR45, pmpaddr); break;
        case 46: write_csr(CSR_PMPADDR46, pmpaddr); break;
        case 47: write_csr(CSR_PMPADDR47, pmpaddr); break;


        case 48: write_csr(CSR_PMPADDR48, pmpaddr); break;
        case 49: write_csr(CSR_PMPADDR49, pmpaddr); break;
        case 50: write_csr(CSR_PMPADDR50, pmpaddr); break;
        case 51: write_csr(CSR_PMPADDR51, pmpaddr); break;
        case 52: write_csr(CSR_PMPADDR52, pmpaddr); break;
        case 53: write_csr(CSR_PMPADDR53, pmpaddr); break;
        case 54: write_csr(CSR_PMPADDR54, pmpaddr); break;
        case 55: write_csr(CSR_PMPADDR55, pmpaddr); break;
        case 56: write_csr(CSR_PMPADDR56, pmpaddr); break;
        case 57: write_csr(CSR_PMPADDR57, pmpaddr); break;
        case 58: write_csr(CSR_PMPADDR58, pmpaddr); break;
        case 59: write_csr(CSR_PMPADDR59, pmpaddr); break;
        case 60: write_csr(CSR_PMPADDR60, pmpaddr); break;
        case 61: write_csr(CSR_PMPADDR61, pmpaddr); break;
        case 62: write_csr(CSR_PMPADDR62, pmpaddr); break;
        case 63: write_csr(CSR_PMPADDR63, pmpaddr); break;
        default: return;
    }
}
/** @} */ /* End of Doxygen Group WMSIS_Core_PMP_Functions */

#ifdef __cplusplus
}
#endif
#endif /** _PMP_H_  */
