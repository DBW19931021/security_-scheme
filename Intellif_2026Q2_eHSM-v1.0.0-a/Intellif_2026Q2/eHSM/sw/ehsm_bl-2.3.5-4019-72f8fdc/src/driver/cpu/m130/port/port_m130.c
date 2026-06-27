/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "cpu_porting.h"
#include "csr.h"
#include "clic.h"
#include "irq_number.h"
#include "pmp.h"
#include "types.h"

#define BASE_ADDR_SHIFT 2

#define IRQ_NUM_INVALID 0xFFFFU

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define CONFIG_BL_DEFAULT_INT_PRIORITY (0xFF)
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
extern void secboot_cpu_excpt_handler(uint32_t cause, uint32_t epc, uint32_t *regs);

ATTR_INTERRUPT void default_trap_handler(void)
{
    uint32_t mcause = __RV_CSR_READ(CSR_MCAUSE);
    uint32_t mepc = __RV_CSR_READ(CSR_MEPC);
    uint32_t regs_addr;
    asm volatile("mv %0, sp" : "=r"(regs_addr));
    secboot_cpu_excpt_handler(mcause, mepc, (uint32_t *)regs_addr);
}

static uint32_t cpu_get_irq_number(cpu_irq_type_e irq)
{
    switch (irq) {
    case IRQ_TYPE_EMU:
        return SOC_EMU_IRQn;
    case IRQ_TYPE_MAILBOX:
        return SOC_MAILBOX_IRQn;
    case IRQ_TYPE_WDT:
        return SOC_WDT_IRQn;
    case IRQ_TYPE_UART:
        return SOC_UART_IRQn;
    default:
        return IRQ_NUM_INVALID;
    }
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t cpu_register_int(cpu_irq_type_e irq, int_trigger_type_e int_type, cpu_int_handler handler)
{
    uint32_t clic_int_attr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t irq_num = cpu_get_irq_number(irq);
    if (irq_num == IRQ_NUM_INVALID || NULL == handler) {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (int_type == INT_LEVEL_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_LEVEL_Msk;
        } else if (int_type == INT_POSITIVE_EDGE_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INI_ATTR_POHARITY_Msk;
        } else if (int_type == INT_NEGATIVE_EDGE_TRIGGER) {
            clic_int_attr = CLIC_INT_ATTR_RTIG_Msk | CLIC_INT_ATTR_POLARITY_Msk;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            // Set the interrupt attribute
            uint32_t clic_ret = clic_set_trig_type((uint8_t)irq_num, clic_int_attr);

            // Set the interrupt priority
            if (CLIC_OK == clic_ret) {
                clic_ret = clic_set_priority((uint8_t)irq_num, CONFIG_BL_DEFAULT_INT_PRIORITY);
            }

            // Set the interrupt to vector mode
            if (CLIC_OK == clic_ret) {
                clic_ret = clic_set_vector_mode((uint8_t)irq_num, true);
            }

            // Clear the pending of the interrupt
            if (CLIC_OK == clic_ret) {
                clic_ret = clic_clear_int_pending((uint8_t)irq_num);
            }

            // Set the ISR
            if (CLIC_OK == clic_ret) {
                clic_ret = clic_int_handler_regist((uint8_t)irq_num, handler);
            }

            if (CLIC_OK != clic_ret) {
                ret = EHSM_ERR_INIT_DEV_FAIL;
            }
        }
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
    uint32_t tmp = __RV_CSR_READ(CSR_MSTATUS);
    tmp |= MSTATUS_MIE;
    __RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

void cpu_disable_global_int(void)
{
    uint32_t tmp = __RV_CSR_READ(CSR_MSTATUS);
    tmp &= (uint32_t)(~MSTATUS_MIE);
    __RV_CSR_WRITE(CSR_MSTATUS, tmp);
}

uint32_t cpu_enter_critical(void)
{
#ifdef __ICCRISCV__
    // IAR不支持__RV_CSR_READ_CLEAR扩展出来的指令 `csrrc a0, mstatus, 0x08`，这里使用t0转一下
    uint32_t level;
    asm volatile("li t0, 0x08\n\t"
                 "csrrc %0, mstatus, t0"
        : "=r"(level)
        :
        : "t0");
    return level;
#else
    return (uint32_t)__RV_CSR_READ_CLEAR(CSR_MSTATUS, MSTATUS_MIE);
#endif
}

void cpu_exit_critical(uint32_t level)
{
    __RV_CSR_WRITE(CSR_MSTATUS, level);
}

void cpu_config_pmp(uint32_t pmp_index, uint32_t start_addr, uint32_t size_exp, uint8_t priv)
{
    if (pmp_index >= cpu_get_pmp_count()) {
        return;
    }
    if (size_exp < 3U || size_exp > 33U) {
        return;
    }

    uint32_t addr_mask = (1U << (size_exp - (uint32_t)PMP_SHIFT)) - 1U;
    uint32_t addr = (start_addr >> BASE_ADDR_SHIFT) | (addr_mask);
    uint32_t ret = pmp_addr_set(pmp_index, addr);

    if (PMP_OK != ret) {
        log_error("pmp_a err: %d, %x\n", ret, addr);
        return;
    }
    ret = pmp_config_set(pmp_index, PMP_NAPOT | PMP_L | priv);
    if (PMP_OK != ret) {
        log_error("pmp_c err: %d\n", ret);
        return;
    }
}

uint32_t cpu_get_pmp_count(void)
{
    return 8;
}

void cpu_platform_init(void)
{

    clic_init();
    cpu_enable_global_int();
}
