/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "cpu_porting.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
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
inline static uint32_t cpu_check_irq(irq_nmber_e irq)
{
    uint32_t ret;
    if ((irq >= SOC_UART0_RX_IRQn) && (irq < SOC_INT_NUM)) {
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static void cpu_exception_init(void)
{
    ;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t cpu_register_int(irq_nmber_e irq, int_trigger_type_e int_type, cpu_int_handler handler)
{
    uint32_t clic_int_attr = 0;
    uint32_t ret = cpu_check_irq(irq);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        switch (int_type) {
        case INT_LEVEL_TRIGGER:
            clic_int_attr = CLIC_INT_ATTR_LEVEL_Msk;
            break;
        case INT_POSTIVE_EDGE_TRIGGER:
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INI_ATTR_POHARITY_Msk;
            break;
        case INT_NEGTIVE_EDGE_TRIGGER:
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INT_ATTR_POLARITY_Msk;
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            // Set the interrupt attribute
            (void)clic_set_trig_type((uint8_t)irq, clic_int_attr);

            // Set the interrupt priority
            (void)clic_set_priority((uint8_t)irq, CONFIG_EHSM_DEFAULT_INT_PRIORITY);

            // Set the interrupt to vector mode
            (void)clic_set_vector_mode((uint8_t)irq, true);

            // Clear the pending of the interrupt
            (void)clic_clear_int_pending((uint8_t)irq);

            // Set the ISR
            if (NULL != handler) {
                (void)clic_int_handler_regist((uint8_t)irq, handler);
            } else {
                ;
            }
        }
    } else {
        ret = EHSM_ERR_WONG_IRQ_NUM;
    }
    return ret;
}

uint32_t cpu_enable_int(irq_nmber_e irq)
{
    uint32_t ret = cpu_check_irq(irq);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (NULL != clic_get_irq_handler((uint8_t)irq)) {
            (void)clic_int_enable((uint8_t)irq);
        } else {
            ret = EHSM_ERR_INVALID_ADDRESS;
        }
    } else {
        ret = EHSM_ERR_WONG_IRQ_NUM;
    }
    return ret;
}

uint32_t cpu_disable_int(irq_nmber_e irq)
{
    uint32_t ret = cpu_check_irq(irq);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        (void)clic_int_disable((uint8_t)irq);
    } else {
        ret = EHSM_ERR_WONG_IRQ_NUM;
    }
    return ret;
}

void cpu_enable_global_int(void)
{
    uint32_t tmp = __RV_CSR_READ(CSR_MSTATUS);
    tmp |= MSTATUS_MIE;
    __RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

void cpu_disable_global_int(void)
{
    uint32_t tmp = __RV_CSR_READ(CSR_MSTATUS);
    tmp &= ~(uint32_t)MSTATUS_MIE;
    __RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

uint32_t cpu_remap_int(const uint32_t *vector_addr)
{
    uint32_t ret;
    uint32_t addr;
    uint32_t tmp = (uint32_t)vector_addr;
    if (0U != (tmp & 0x3f)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        addr = __RV_CSR_READ(CSR_MTVT);
        addr |= ((uint32_t)vector_addr) & 0xffffffc0UL;
        __RV_CSR_WRITE(CSR_MTVT, addr);
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

uint32_t cpu_enter_critical(void)
{
    return (uint32_t)__RV_CSR_READ_CLEAR(CSR_MSTATUS, MSTATUS_MIE);
}

void cpu_exit_critical(uint32_t level)
{
    __RV_CSR_WRITE(CSR_MSTATUS, level);
}

void cpu_platform_init(void)
{
    cpu_exception_init();
    clic_init();
    cpu_enable_global_int();
}
