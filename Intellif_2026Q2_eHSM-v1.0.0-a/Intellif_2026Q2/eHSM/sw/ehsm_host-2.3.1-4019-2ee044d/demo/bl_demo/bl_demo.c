#include <stdio.h>
#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/types.h"

#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "common/get_version.h"
#include "bl_demo.h"
#include "common/otp_data/ehsm_demo_otp_data.h"

#include "bl_demo/otp/ehsm_bl_demo_gen_otp_key.h"

#include "bl_demo/otp/ehsm_bl_demo_rd_wr_otp.h"

#include "bl_demo/misc/ehsm_bl_demo_rd_wr_reg.h"

#include "bl_demo/debug_auth/ehsm_bl_demo_debug_auth.h"

#include "bl_demo/image_upgrade_verify/ehsm_bl_demo_image_upgrade.h"

#include "bl_demo/image_upgrade_verify/ehsm_bl_demo_image_verify.h"

#include "bl_demo/misc/ehsm_bl_demo_set_uart_buad_div.h"

#include "bl_demo/self_test/ehsm_bl_demo_self_test.h"

void ehsm_bl_demo_entry(void)
{
    ehsm_port_printf("eHSM demo starts. \r\n\r\n");

    /* 通过 SoC 写映射后的 OTP 地址及偏移写 OTP，g_otp_data 由 fw_demo/tools/otp_data_tool/gen_otp_data.sh 生成 */
    ehsm_port_write_otp(g_otp_data, 0x0U, sizeof(g_otp_data));

    /* 重启 eHSM 使 OTP 数据同步到 eHSM 寄存器，KMU 等 */
    demo_reset_ehsm_wait_ready();

    /* 等待 eHSM HW_BOOT_DONE 和 HSM_READY */
    while ((ehsm_port_read_reg(REG_HSM_STATUS_0) & (DEMO_SYSSTA0_HW_BOOT_DONE | DEMO_SYSSTA0_HSM_READY)) == 0) { }

    ehsm_bl_demo_gen_otp_key_entry();

    ehsm_bl_demo_rd_wr_otp_entry();

    ehsm_bl_demo_rd_wr_reg_entry();

    ehsm_bl_demo_debug_auth_entry();

    ehsm_bl_demo_debug_auth_entry();

    ehsm_bl_demo_image_verify_entry();

    ehsm_bl_demo_image_upgrade_entry();

    ehsm_bl_demo_set_uart_buad_div_entry();

    ehsm_bl_demo_self_test_entry();

    ehsm_port_printf("eHSM demo ends with success !!! \r\n\r\n");
}
