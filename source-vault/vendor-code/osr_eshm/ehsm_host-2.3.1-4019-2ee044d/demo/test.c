#include <string.h>
#include "common/get_version.h"

#include "ehsmdrv/basic/port/ehsm_host_port.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/types.h"

#include "common/utils.h"
#include "common/otp_data/ehsm_demo_otp_data.h"
#include "bl_demo/bl_demo.h"
#include "fw_demo/fw_demo.h"
#include "uart_stdout.h"

#if defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE)
uint32_t demo_bl_patch_test_entry(void);
#endif /* defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE) */

#define CONFIG_BUILD_HOST_DEMO

#ifdef CONFIG_BUILD_HOST_DEMO
void ehsm_demo_test(void)
{
    if (ehsm_driver_init_library(EHSM_DRV_MODE_WAIT_AND_POLL) != EHSM_OK) {
        ehsm_port_printf("library init failed!\n");
        return;
    }

    ehsm_port_printf("ehsm demo start\n");
    uint32_t version = ehsm_driver_get_version();

    ehsm_port_printf("ehsm driver version: 0x%08x\n", version);

#if defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE)
    (void)demo_bl_patch_test_entry();
#else
    bool_t is_bl = false;

    // set OTP data
    uint8_t buf[EHSM_PORT_OTP_SIZE];
#if EHSM_PORT_OTP_DEFAULT_VALUE == 0
    memset(buf, 0, sizeof(buf));
#else
    memset(buf, 0xff, sizeof(buf));
#endif
    ehsm_port_write_otp(buf, 0, sizeof(buf));

    // reset ehsm
    demo_reset_ehsm_wait_ready();

    // wait for bl_ready or hsm ready
    while ((ehsm_port_read_reg(REG_HSM_STATUS_0) & (DEMO_SYSSTA0_HW_BOOT_DONE | DEMO_SYSSTA0_HSM_READY)) == 0) { }

    ehsm_demo_test_get_version(&is_bl);

    if (true == is_bl) {
        ehsm_bl_demo_entry();
    } else {
        ehsm_fw_demo_entry();
    }
#endif /* defined(CONFIG_HOST_BL_PATCH_TEST_ENABLE) */
}
#endif

int main(void)
{
#ifdef CONFIG_BUILD_HOST_DEMO
    /* 运行 demo */
    ehsm_demo_test();
#endif

    while (1) { }
}
