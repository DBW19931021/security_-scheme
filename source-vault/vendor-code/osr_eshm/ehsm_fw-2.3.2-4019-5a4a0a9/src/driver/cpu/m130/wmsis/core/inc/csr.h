/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef CPU_CSR_H
#define CPU_CSR_H

#ifndef __ASSEMBLER__
#include"riscv_bits.h"
/**
 * \brief CSR operation Macro for csrr instruction.
 * \details
 * Read the content of csr register to v and return it
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \return the CSR register value
 */
#define RV_CSR_READ(csr)                                      \
    ({                                                          \
        register unsigned int v;                                  \
        asm volatile("csrr %0, " STRINGIFY(csr)               \
                     : "=r"(v)                                \
                     :                                          \
                     : "memory");                               \
        v;                                                    \
    })

/**
 * \brief CSR operation Macro for csrw instruction.
 * \details
 * Write the content of val to csr register
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   value to store into the CSR register
 */
#define RV_CSR_WRITE(csr, val)                                \
    ({                                                          \
        register unsigned int v = (unsigned int)(val);                \
        asm volatile("csrw " STRINGIFY(csr) ", %0"            \
                     :                                          \
                     : "rK"(v)                                \
                     : "memory");                               \
    })

/**
 * \brief CSR operation Macro for csrrc instruction.
 * \details
 * Read the content of csr register to v,
 * then set csr register to be v & ~val, then return v
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrrc instruction
 * \return the CSR register value before written
 */
#define RV_CSR_READ_CLEAR(csr, val)                           \
    ({                                                          \
        register unsigned int v = (unsigned int)(val);                \
        asm volatile("csrrc %0, " STRINGIFY(csr) ", %1"       \
                     : "=r"(v)                                \
                     : "rK"(v)                                \
                     : "memory");                               \
        v;                                                    \
    })

#endif // !__ASSEMBLER__
#endif // CPU_CSR_H
