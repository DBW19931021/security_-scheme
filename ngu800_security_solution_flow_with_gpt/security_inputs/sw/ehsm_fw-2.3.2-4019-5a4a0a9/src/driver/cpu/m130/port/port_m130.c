/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "cpu_porting.h"
#include "driver/emu_driver.h"
#include "csr.h"
#include "clic.h"
#include "irq_number.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define IRQ_NUM_INVALID 0xFFFFU

#define CONFIG_EHSM_DEFAULT_INT_PRIORITY (0xFF)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t cpu_get_irq_number(cpu_irq_type_e irq)
{
    uint32_t Irq_n = (uint32_t)IRQ_NUM_INVALID;

    switch (irq) {
    case IRQ_TYPE_EMU:
        Irq_n = (uint32_t)SOC_EMU_IRQn;
        break;
    case IRQ_TYPE_MAILBOX:
        Irq_n = (uint32_t)SOC_MAILBOX_IRQn;
        break;
    case IRQ_TYPE_WDT:
        Irq_n = (uint32_t)SOC_WDT_IRQn;
        break;
    case IRQ_TYPE_UART:
        Irq_n = (uint32_t)SOC_UART_IRQn;
        break;
    default:
        Irq_n = (uint32_t)IRQ_NUM_INVALID;
        break;
    }

    return Irq_n;
}

static void cpu_exception_init(void)
{
    RV_CSR_WRITE(CSR_MTVEC, &emu_exception_handler);
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t cpu_register_int(cpu_irq_type_e irq, int_trigger_type_e int_type, cpu_int_handler handler)
{
    uint32_t clic_int_attr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t irq_num = cpu_get_irq_number(irq);
    if (irq_num == IRQ_NUM_INVALID) {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (int_type == INT_LEVEL_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_LEVEL_Msk;
        } else if (int_type == INT_POSTIVE_EDGE_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INI_ATTR_POHARITY_Msk;
        } else if (int_type == INT_NEGTIVE_EDGE_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INT_ATTR_POLARITY_Msk;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            // Set the interrupt attribute
            (void)clic_set_trig_type((uint8_t)irq_num, clic_int_attr);

            // Set the interrupt priority
            (void)clic_set_priority((uint8_t)irq_num, CONFIG_EHSM_DEFAULT_INT_PRIORITY);

            // Set the interrupt to vector mode
            (void)clic_set_vector_mode((uint8_t)irq_num, true);

            // Clear the pending of the interrupt
            (void)clic_clear_int_pending((uint8_t)irq_num);

            // Set the ISR
            if (NULL != handler) {
                (void)clic_int_handler_regist((uint8_t)irq_num, handler);
            } else {
                ;
            }
        }
    } else {
        ret = EHSM_ERR_WONG_IRQ_NUM;
    }
    return ret;
}

uint32_t cpu_enable_int(cpu_irq_type_e irq)
{
    uint32_t ret = EHSM_ERR_WONG_IRQ_NUM;
    uint32_t irq_num = cpu_get_irq_number(irq);
    if (irq_num != IRQ_NUM_INVALID) {
        if (NULL != clic_get_irq_handler((uint8_t)irq_num)) {
            (void)clic_int_enable((uint8_t)irq_num);
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_INVALID_ADDRESS;
        }
    }
    return ret;
}

uint32_t cpu_disable_int(cpu_irq_type_e irq)
{
    uint32_t ret = EHSM_ERR_WONG_IRQ_NUM;
    uint32_t irq_num = cpu_get_irq_number(irq);
    if (irq_num != IRQ_NUM_INVALID) {
        (void)clic_int_disable((uint8_t)irq_num);
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

void cpu_enable_global_int(void)
{
    uint32_t tmp = RV_CSR_READ(CSR_MSTATUS);
    tmp |= MSTATUS_MIE;
    RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

void cpu_disable_global_int(void)
{
    uint32_t tmp = RV_CSR_READ(CSR_MSTATUS);
    tmp &= ~MSTATUS_MIE;
    RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

uint32_t cpu_enter_critical(void)
{
#ifdef __ICCRISCV__
    // IAR does not support the instruction `csrrc a0, mstatus, 0x08` extended
	// from RV_CSR_READ_CLEAR, use x5 to transfer here.
    asm volatile ("addi x5, x0, 0x08");
    asm volatile ("csrrc a0, mstatus, x5");
    asm volatile ("ret");
    return 0;
#else
    return (uint32_t)RV_CSR_READ_CLEAR(CSR_MSTATUS, MSTATUS_MIE);
#endif
}

void cpu_exit_critical(uint32_t level)
{
    RV_CSR_WRITE(CSR_MSTATUS, level);
}

void cpu_platform_init(void)
{

    cpu_exception_init();
    clic_init();
    cpu_enable_global_int();
}
