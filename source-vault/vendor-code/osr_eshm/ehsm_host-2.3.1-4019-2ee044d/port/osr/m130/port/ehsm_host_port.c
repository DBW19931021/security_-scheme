#include <string.h>

#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "irq_number.h"
#include "cpu_porting.h"
#include "types.h"

#include "cpu_porting.h"
#include "uart_stdout.h"

#define OSR_FPGA_FREQ (5000000)

#define OTP_BASE_ADDR     0x6007c000u
#define SYS_REG_BASE_ADDR 0x40010000u

// use RISC-V mtime
#define CLINT_REG_MEM_BASE (0x00000000 + 0x00000)       /*!< clint memory space base addr. */
#define MTIMECTRL_EN_Pos   0U                           /*!< position of mtime enable field. */
#define MTIMECTRL_EN_Msk   (0x01UL << MTIMECTRL_EN_Pos) /*!< bit mask of mtime enable field. */

typedef struct {
    volatile uint32_t MTIME_LO;    /*!< address of mtime reg. */
    volatile uint32_t MTIME_HI;    /*!< address of mtime high part reg. */
    volatile uint32_t MTIMECMP_LO; /*!< address of mtime compare low part reg. */
    volatile uint32_t MTIMECMP_HI; /*!< address of mtime compare  high part reg. */
    volatile uint32_t MTIMECTRL;   /*!< address of mtime control reg. */
    volatile uint32_t RESERVED[3];
    volatile uint32_t MSIP; /*!< address of machine software interrupt pendding control reg. */
} WING_CLINT_Type;

#define WING_CLINT ((volatile WING_CLINT_Type *)CLINT_REG_MEM_BASE)

#pragma GCC push_options
#pragma GCC optimize("O0")
// 由于寄存器地址为0，必须禁用优化，否则读写寄存器会出问题

static void init_mtime(void)
{
    WING_CLINT->MTIMECTRL |= MTIMECTRL_EN_Msk;
}

uint32_t get_time_tick(void)
{
    return WING_CLINT->MTIME_LO;
}
#pragma GCC pop_options /* 恢复之前的优化级别 */

raddr_t ehsm_port_addr_to_raddr(const void *addr)
{
    return (raddr_t)(uintptr_t)addr;
}

void *ehsm_port_raddr_to_addr(raddr_t raddr)
{
    return (void *)(uintptr_t)raddr;
}

void ehsm_port_flush_and_invalidate_cache(void)
{
    // nothing to do on OSR FPGA
}

static volatile ehsm_mailbox_isr_f g_mb_isrs[EHSM_PORT_MAILBOX_CHANNEL_COUNT] = { NULL };

#ifdef __GNUC__
#define ATTR_INTERRUPT __attribute__((interrupt, aligned(64)))
#elif defined(__ICCRISCV__)
// for IAR, alignment is set by compiler flags
#define ATTR_INTERRUPT __interrupt
#else
#define ATTR_INTERRUPT
#endif

ATTR_INTERRUPT void MAILBOX_0_Handler(void)
{
    if (g_mb_isrs[0]) {
        g_mb_isrs[0](0);
    }
}
ATTR_INTERRUPT void MAILBOX_1_Handler(void)
{
    if (g_mb_isrs[1]) {
        g_mb_isrs[1](1);
    }
}

uint32_t ehsm_port_enable_mailbox_int(uint32_t channel, ehsm_mailbox_isr_f int_func)
{
    // Only mailbox0 and mailbox1 could be used on OSR FPGA
    if (channel >= 2 || int_func == NULL) {
        return EHSM_ERR_PARAM_ERROR;
    }
    g_mb_isrs[channel] = int_func;
    switch (channel) {
    case 0:
        (void)cpu_register_int(SOC_MAILBOX0_IRQn, INT_LEVEL_TRIGGER, (cpu_int_handler)MAILBOX_0_Handler);
        cpu_enable_int(SOC_MAILBOX0_IRQn);
        break;
    case 1:
        (void)cpu_register_int(SOC_MAILBOX1_IRQn, INT_LEVEL_TRIGGER, (cpu_int_handler)MAILBOX_1_Handler);
        cpu_enable_int(SOC_MAILBOX1_IRQn);
        break;
    default:
        break;
    }
    return EHSM_OK;
}

void ehsm_port_reset_ehsm(void)
{
    *((volatile uint32_t *)(SYS_REG_BASE_ADDR + 0x58)) = 0x0fa66e27u; // OSR_FPGA_MAGIC
    *((volatile uint32_t *)(SYS_REG_BASE_ADDR + 0x08)) = 0xfffffffeu; // reset
    *((volatile uint32_t *)(SYS_REG_BASE_ADDR + 0x08)) = 0xffffffffu; // release
}

static volatile uint32_t *get_reg_addr(ehsm_reg_e reg)
{
    uint32_t addr = 0;
    switch (reg) {
    case REG_HSM_STATUS_0:
        addr = 0x60;
        break;
    case REG_HSM_STATUS_1:
        addr = 0x64;
        break;
    case REG_HSM_ERR_SENSOR:
        addr = 0x68;
        break;
    case REG_HSM_ERR_HW_0:
        addr = 0x6c;
        break;
    case REG_HSM_ERR_HW_1:
        addr = 0x70;
        break;
    case REG_HSM_ERR_FW_0:
        addr = 0x74;
        break;
    case REG_HSM_ERR_FW_1:
        addr = 0x78;
        break;
    case REG_HSM_FUSA_ALARM_0:
        addr = 0x80;
        break;
    case REG_HSM_FUSA_ALARM_1:
        addr = 0x84;
        break;
    case REG_SOC_DBG_EN_0:
        addr = 0x88;
        break;
    case REG_SOC_DBG_EN_1:
        addr = 0x8c;
        break;
    case REG_SOC_DBG_EN_2:
        addr = 0x90;
        break;
    case REG_SOC_DBG_EN_3:
        addr = 0x94;
        break;
    case REG_SOC_SENSOR:
        addr = 0x54;
        break;
    default:
        return NULL;
    }

    return (volatile uint32_t *)(SYS_REG_BASE_ADDR + addr);
}

uint32_t ehsm_port_read_reg(ehsm_reg_e reg)
{
    volatile uint32_t *reg_addr = get_reg_addr(reg);
    if (reg_addr != NULL) {
        return *reg_addr;
    } else {
        return 0;
    }
}

void ehsm_port_write_reg(ehsm_reg_e reg, uint32_t val)
{
    volatile uint32_t *reg_addr = get_reg_addr(reg);
    if (reg_addr != NULL) {
        *reg_addr = val;
    }
}

uint32_t ehsm_port_read_otp(uint8_t *buf, uint32_t offset, uint32_t len)
{
    if (offset + len > EHSM_PORT_OTP_SIZE) {
        return EHSM_ERR_PARAM_ERROR;
    }
    memcpy(buf, (void *)(OTP_BASE_ADDR + offset), len);
    return EHSM_OK;
}

uint32_t ehsm_port_write_otp(const uint8_t *otp_data, uint32_t offset, uint32_t len)
{
    if (offset + len > EHSM_PORT_OTP_SIZE) {
        return EHSM_ERR_PARAM_ERROR;
    }
    memcpy((void *)(OTP_BASE_ADDR + offset), otp_data, len);
    return EHSM_OK;
}

ehsm_port_timer_t ehsm_port_create_timer(void)
{
    return 0;
}

bool_t ehsm_port_is_timeout(ehsm_port_timer_t timer)
{
    (void)timer;
    return false;
}

uint32_t ehsm_port_init(ehsm_drv_mode_e mode)
{
    (void)mode;
    UartStdOutInit(115200);
    cpu_platform_init();
    init_mtime();
    return EHSM_OK;
}

double ehsm_port_get_time_ms(void)
{
    return ((double)get_time_tick() / ((double)OSR_FPGA_FREQ / 32.0)) * 1000.0;
}

void ehsm_port_memory_barrier_write(void)
{
    __asm volatile("fence w, w" ::: "memory");
}

void ehsm_port_memory_barrier_read(void)
{
    __asm volatile("fence r, r" ::: "memory");
}

void ehsm_port_enter_critical(void)
{
    __asm volatile("csrc mstatus, 0x8" ::: "memory");
}

void ehsm_port_exit_critical(void)
{
    __asm volatile("csrs mstatus, 0x8" ::: "memory");
}
