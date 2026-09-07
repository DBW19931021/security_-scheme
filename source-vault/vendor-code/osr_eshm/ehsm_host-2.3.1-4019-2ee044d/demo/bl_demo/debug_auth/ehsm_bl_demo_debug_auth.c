#include "ehsm_bl_demo_debug_auth.h"
#include "fw_demo/debug_auth/ehsm_demo_debug_auth.h"

/**
 * @brief 调试鉴权的演示入口函数。
 *
 * @note BL 调用的 API 只是名字和 FW 的不一样，实际指向同一个 API。
 */
void ehsm_bl_demo_debug_auth_entry(void)
{
    ehsm_demo_debug_auth_entry();
}

