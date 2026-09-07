#include <string.h>

#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "CM3DS_MPS2.h"
#include "uart_stdout.h"

#define OTP_BASE_ADDR     0x6007c000u
#define SYS_REG_BASE_ADDR 0x40010000u

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

static ehsm_mailbox_isr_f g_mb_isrs[EHSM_PORT_MAILBOX_CHANNEL_COUNT] = { NULL };

void MAILBOX_0_Handler(void)
{
    if (g_mb_isrs[0]) {
        g_mb_isrs[0](0);
    }
}
void MAILBOX_1_Handler(void)
{
    if (g_mb_isrs[1]) {
        g_mb_isrs[1](1);
    }
}

uint32_t ehsm_port_enable_mailbox_int(uint32_t channel, ehsm_mailbox_isr_f int_func)
{
    if (channel >= 2 || int_func == NULL) {
        return EHSM_ERR_PARAM_ERROR;
    }
    g_mb_isrs[channel] = int_func;
    switch (channel) {
    case 0:
        NVIC_EnableIRQ(Mailbox0_IRQn);
        break;
    case 1:
        NVIC_EnableIRQ(Mailbox1_IRQn);
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
        break;
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
    return EHSM_OK;
}
