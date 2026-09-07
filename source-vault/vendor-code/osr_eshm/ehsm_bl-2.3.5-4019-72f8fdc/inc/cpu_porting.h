#ifndef APPLICATION_PLATEFORM_PORTING_H
#define APPLICATION_PLATEFORM_PORTING_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define CPU_PMP_READ  0x01
#define CPU_PMP_WRITE 0x02
#define CPU_PMP_EXEC  0x04
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef enum int_trigger_type {
    INT_LEVEL_TRIGGER = 0x0,         /*!< Level Triggerred */
    INT_POSITIVE_EDGE_TRIGGER = 0x1, /*!< Positive/Rising Edge Triggered */
    INT_NEGATIVE_EDGE_TRIGGER = 0x2, /*!< Negative/Falling Edge Triggered */
    INT_MAX_TRIGGER = 0x3            /*!< MAX Supported Trigger Mode */
} int_trigger_type_e;

typedef enum {
    IRQ_TYPE_EMU,
    IRQ_TYPE_MAILBOX,
    IRQ_TYPE_WDT,
    IRQ_TYPE_UART,
} cpu_irq_type_e;

typedef void (*cpu_int_handler)(void);
/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t cpu_register_int(cpu_irq_type_e irq, int_trigger_type_e int_type, cpu_int_handler handler);

uint32_t cpu_enable_int(cpu_irq_type_e irq);

uint32_t cpu_disable_int(cpu_irq_type_e irq);

void cpu_enable_global_int(void);

void cpu_disable_global_int(void);

uint32_t cpu_enter_critical(void);

void cpu_exit_critical(uint32_t level);

void cpu_platform_init(void);

uint32_t cpu_get_pmp_count(void);

void cpu_config_pmp(uint32_t index, uint32_t start_addr, uint32_t size_exp, uint8_t priv);

#endif
