#include <stdint.h>

#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsmdrv/basic/types.h"

#if defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE)
#define DEMO_BL_PATCH_TEST_MARKER_ADDR     (0x60019000U)
#define DEMO_BL_PATCH_TEST_MARKER_BASELINE (0xA5332211U)
#define DEMO_BL_PATCH_TEST_MARKER_PATCHED  (0xA5CCBBAAU)
#define DEMO_BL_PATCH_TEST_OTP_BASE_ADDR   (0x6007C000U)
#define DEMO_BL_PATCH_TEST_HW_CTRL_OFFSET  (0x18U)
#define DEMO_BL_PATCH_TEST_HW_ENABLE_MASK (0x0000C000U)
#if EHSM_PORT_OTP_DEFAULT_VALUE == 0
#define DEMO_BL_PATCH_TEST_HW_ENABLE_VALUE (0x00004000U)
#else
#define DEMO_BL_PATCH_TEST_HW_ENABLE_VALUE (0x00008000U)
#endif
#define DEMO_BL_PATCH_TEST_CFG_OFFSET      (0x50U)
#define DEMO_BL_PATCH_TEST_ADDR_OFFSET     (0x58U)
#define DEMO_BL_PATCH_TEST_DATA_OFFSET     (0x98U)
#define DEMO_BL_PATCH_TEST_CFG_ROW0_2      (0x15U)
// TODO patchde的函数bl_patch_test_probe0 bl_patch_test_probe1 bl_patch_test_probe2 
//地址为10000540、10000548、10000550，如果地址有变更需要相应的修改
#define DEMO_BL_PATCH_TEST_ADDR0           (0x0150U)
#define DEMO_BL_PATCH_TEST_ADDR1           (0x0152U)
#define DEMO_BL_PATCH_TEST_ADDR2           (0x0154U)
#define DEMO_BL_PATCH_TEST_DATA0           (0x0AA00513U)
#define DEMO_BL_PATCH_TEST_DATA1           (0x0BB00513U)
#define DEMO_BL_PATCH_TEST_DATA2           (0x0CC00513U)

#ifndef DEMO_BL_PATCH_TEST_EXPECTED_MARKER
#define DEMO_BL_PATCH_TEST_EXPECTED_MARKER DEMO_BL_PATCH_TEST_MARKER_PATCHED
#endif /* DEMO_BL_PATCH_TEST_EXPECTED_MARKER */

static volatile uint32_t *demo_bl_patch_test_get_marker_addr(void)
{
    volatile uint32_t *addr;

    addr = (volatile uint32_t *)(uintptr_t)DEMO_BL_PATCH_TEST_MARKER_ADDR;

    return addr;
}

static uint32_t demo_bl_patch_test_read_u32(uint32_t offset)
{
    uint32_t value;
    volatile uint32_t *addr;

    addr = (volatile uint32_t *)(uintptr_t)(DEMO_BL_PATCH_TEST_OTP_BASE_ADDR + offset);
    value = *addr;

    return value;
}

static void demo_bl_patch_test_write_u16(uint32_t offset, uint16_t value)
{
    volatile uint16_t *addr;

    addr = (volatile uint16_t *)(uintptr_t)(DEMO_BL_PATCH_TEST_OTP_BASE_ADDR + offset);
    *addr = value;
}

static void demo_bl_patch_test_write_u32(uint32_t offset, uint32_t value)
{
    volatile uint32_t *addr;

    addr = (volatile uint32_t *)(uintptr_t)(DEMO_BL_PATCH_TEST_OTP_BASE_ADDR + offset);
    *addr = value;
}

static void demo_bl_patch_test_write_marker(uint32_t marker)
{
    volatile uint32_t *addr;

    addr = demo_bl_patch_test_get_marker_addr();
    *addr = marker;
}

static uint32_t demo_bl_patch_test_read_marker(void)
{
    uint32_t marker;
    volatile uint32_t *addr;

    addr = demo_bl_patch_test_get_marker_addr();
    marker = *addr;

    return marker;
}

static void demo_bl_patch_test_program_hw_enable(void)
{
    uint32_t hw_ctrl;

    hw_ctrl = demo_bl_patch_test_read_u32(DEMO_BL_PATCH_TEST_HW_CTRL_OFFSET);
    hw_ctrl &= ~DEMO_BL_PATCH_TEST_HW_ENABLE_MASK;
    hw_ctrl |= DEMO_BL_PATCH_TEST_HW_ENABLE_VALUE;
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_HW_CTRL_OFFSET, hw_ctrl);
}

static void demo_bl_patch_test_program_cfg(void)
{
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_CFG_OFFSET, DEMO_BL_PATCH_TEST_CFG_ROW0_2);
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_CFG_OFFSET + 4U, 0U);
}

static void demo_bl_patch_test_program_addr(void)
{
    demo_bl_patch_test_write_u16(DEMO_BL_PATCH_TEST_ADDR_OFFSET, DEMO_BL_PATCH_TEST_ADDR0);
    demo_bl_patch_test_write_u16(DEMO_BL_PATCH_TEST_ADDR_OFFSET + 2U, DEMO_BL_PATCH_TEST_ADDR1);
    demo_bl_patch_test_write_u16(DEMO_BL_PATCH_TEST_ADDR_OFFSET + 4U, DEMO_BL_PATCH_TEST_ADDR2);
}

static void demo_bl_patch_test_program_data(void)
{
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_DATA_OFFSET, DEMO_BL_PATCH_TEST_DATA0);
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_DATA_OFFSET + 4U, DEMO_BL_PATCH_TEST_DATA1);
    demo_bl_patch_test_write_u32(DEMO_BL_PATCH_TEST_DATA_OFFSET + 8U, DEMO_BL_PATCH_TEST_DATA2);
}

static void demo_bl_patch_test_program_otp(void)
{
    demo_bl_patch_test_program_hw_enable();
    demo_bl_patch_test_program_cfg();
    demo_bl_patch_test_program_addr();
    demo_bl_patch_test_program_data();
}

uint32_t demo_bl_patch_test_entry(void)
{
    uint32_t marker;
    uint32_t ret;

    ehsm_port_printf("BL hardware OTP ROM patch test start.\r\n");
    ehsm_port_printf("    baseline marker: 0x%08x\r\n", DEMO_BL_PATCH_TEST_MARKER_BASELINE);
    ehsm_port_printf("    patched marker:  0x%08x\r\n", DEMO_BL_PATCH_TEST_MARKER_PATCHED);
    ehsm_port_printf("    expected marker: 0x%08x\r\n", DEMO_BL_PATCH_TEST_EXPECTED_MARKER);
    ehsm_port_printf("    OTP HW Patch Enable: 0x%08x\r\n", DEMO_BL_PATCH_TEST_HW_ENABLE_VALUE);
    ehsm_port_printf("    OTP Patch Cfg:  0x%08x\r\n", DEMO_BL_PATCH_TEST_CFG_ROW0_2);
    ehsm_port_printf("    OTP Patch Addr: 0x%04x 0x%04x 0x%04x\r\n", DEMO_BL_PATCH_TEST_ADDR0,
        DEMO_BL_PATCH_TEST_ADDR1, DEMO_BL_PATCH_TEST_ADDR2);
    ehsm_port_printf("    OTP Patch Data: 0x%08x 0x%08x 0x%08x\r\n", DEMO_BL_PATCH_TEST_DATA0,
        DEMO_BL_PATCH_TEST_DATA1, DEMO_BL_PATCH_TEST_DATA2);

    demo_bl_patch_test_write_marker(0U);
    demo_bl_patch_test_program_otp();
    demo_reset_ehsm_wait_ready();
    marker = demo_bl_patch_test_read_marker();

    ret = demo_check_val("BL hardware OTP ROM patch marker:", DEMO_BL_PATCH_TEST_EXPECTED_MARKER, marker);

    return ret;
}
#endif /* defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE) */
