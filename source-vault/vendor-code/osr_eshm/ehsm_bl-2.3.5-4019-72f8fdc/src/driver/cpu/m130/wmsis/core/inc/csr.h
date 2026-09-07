/**
 * Copyright (c) 2022 - 2023, WingSemi Technology LTD.
 *
 * All rights reserved.
 */
#ifndef __CSR_H__
#define __CSR_H__

#ifndef __ASSEMBLER__
#include "riscv_bits.h"
#include "encoding.h"

/**
 * \brief CSR operation Macro for csrrw instruction.
 * \details
 * Read the content of csr register to __v,
 * then write content of val into csr register, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   value to store into the CSR register
 * \return the CSR register value before written
 */
#define __RV_CSR_SWAP(csr, val)                                                              \
    ({                                                                                       \
        register unsigned int __v = (unsigned long)(val);                                    \
        asm volatile("csrrw %0, " STRINGIFY(csr) ", %1" : "=r"(__v) : "rK"(__v) : "memory"); \
        __v;                                                                                 \
    })

/**
 * \brief CSR operation Macro for csrr instruction.
 * \details
 * Read the content of csr register to __v and return it
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \return the CSR register value
 */
#define __RV_CSR_READ(csr)                                                 \
    ({                                                                     \
        register unsigned int __v;                                         \
        asm volatile("csrr %0, " STRINGIFY(csr) : "=r"(__v) : : "memory"); \
        __v;                                                               \
    })

/**
 * \brief CSR operation Macro for csrw instruction.
 * \details
 * Write the content of val to csr register
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   value to store into the CSR register
 */
#define __RV_CSR_WRITE(csr, val)                                              \
    ({                                                                        \
        register unsigned int __v = (unsigned int)(val);                      \
        asm volatile("csrw " STRINGIFY(csr) ", %0" : : "rK"(__v) : "memory"); \
    })

/**
 * \brief CSR operation Macro for csrrs instruction.
 * \details
 * Read the content of csr register to __v,
 * then set csr register to be __v | val, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrrs instruction
 * \return the CSR register value before written
 */
#define __RV_CSR_READ_SET(csr, val)                                                          \
    ({                                                                                       \
        register unsigned int __v = (unsigned int)(val);                                     \
        asm volatile("csrrs %0, " STRINGIFY(csr) ", %1" : "=r"(__v) : "rK"(__v) : "memory"); \
        __v;                                                                                 \
    })

/**
 * \brief CSR operation Macro for csrs instruction.
 * \details
 * Set csr register to be csr_content | val
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrs instruction
 */
#define __RV_CSR_SET(csr, val)                                                \
    ({                                                                        \
        register unsigned int __v = (unsigned int)(val);                      \
        asm volatile("csrs " STRINGIFY(csr) ", %0" : : "rK"(__v) : "memory"); \
    })

/**
 * \brief CSR operation Macro for csrrc instruction.
 * \details
 * Read the content of csr register to __v,
 * then set csr register to be __v & ~val, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrrc instruction
 * \return the CSR register value before written
 */
#define __RV_CSR_READ_CLEAR(csr, val)                                                        \
    ({                                                                                       \
        register unsigned int __v = (unsigned int)(val);                                     \
        asm volatile("csrrc %0, " STRINGIFY(csr) ", %1" : "=r"(__v) : "rK"(__v) : "memory"); \
        __v;                                                                                 \
    })

/**
 * \brief CSR operation Macro for csrc instruction.
 * \details
 * Set csr register to be csr_content & ~val
 * \param csr   CSR macro definition defined in
 *              \ref WMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrc instruction
 */
#define __RV_CSR_CLEAR(csr, val)                                              \
    ({                                                                        \
        register unsigned int __v = (unsigned int)(val);                      \
        asm volatile("csrc " STRINGIFY(csr) ", %0" : : "rK"(__v) : "memory"); \
    })

#endif // !__ASSEMBLER__
#endif // __CSR_H__
