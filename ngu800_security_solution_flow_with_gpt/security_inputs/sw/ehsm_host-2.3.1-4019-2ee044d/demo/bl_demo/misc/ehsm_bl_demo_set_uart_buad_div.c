#include "ehsm_bl_demo_set_uart_buad_div.h"
#include "fw_demo/misc/ehsm_demo_set_uart_buad_div.h"

/**
 * @brief 设置 uart 频率分频值的演示入口函数。
 *
 * @note BL 调用的 API 只是名字和 FW 的不一样，实际指向同一个 API。
 */
void ehsm_bl_demo_set_uart_buad_div_entry(void)
{
    ehsm_demo_set_uart_buad_div_entry();
}

