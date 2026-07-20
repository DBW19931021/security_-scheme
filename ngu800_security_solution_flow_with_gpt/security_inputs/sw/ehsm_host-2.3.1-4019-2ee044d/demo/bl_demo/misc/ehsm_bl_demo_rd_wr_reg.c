#include "ehsm_bl_demo_rd_wr_reg.h"
#include "fw_demo/misc/ehsm_demo_rd_wr_reg.h"

/**
 * @brief 读写 REG 数据的演示入口函数。
 *
 * @note BL 调用的 API 只是名字和 FW 的不一样，实际指向同一个 API。
 */
void ehsm_bl_demo_rd_wr_reg_entry(void)
{
    ehsm_demo_rd_wr_reg_entry();
}

