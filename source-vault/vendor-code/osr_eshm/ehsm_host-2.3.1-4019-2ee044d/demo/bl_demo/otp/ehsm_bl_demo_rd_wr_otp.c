#include "ehsm_bl_demo_rd_wr_otp.h"
#include "fw_demo/otp/ehsm_demo_rd_wr_otp.h"

/**
 * @brief 读写 OTP 数据的演示入口函数。
 *
 * @note BL 调用的 API 只是名字和 FW 的不一样，实际指向同一个 API。
 */
void ehsm_bl_demo_rd_wr_otp_entry(void)
{
    ehsm_demo_rd_wr_otp_entry();
}

