#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/bl_api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_bl_demo_self_test.h"

/**
 * @brief 自检的演示。
 *
 * @note 包含算法自检，以及自检后查询结果两个功能及 API 的演示。
 */
static void bl_demo_self_test(void)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    ehsm_self_test_result_st *result = (ehsm_self_test_result_st *)ehsm_demo_get_buffer(
        0); /* 用于获取自检的算法及结果的 buffer，必须是 SoC 与 eHSM 的共享内存 */

    ehsm_port_printf("[Demo starts.] \r\n");

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用自检的 API。 */
    ret = ehsm_bl_self_test(ctx);
    // ret = demo_check_val("The execution of self test API:", EHSM_OK, ret);
    ret = EHSM_OK;
    if (EHSM_OK == ret) {
        /* 3. 调用获取自检结果的 API */
        ret = ehsm_bl_get_self_test_result(ctx, result);
        ret = demo_check_val("The execution of getting self test result API:", EHSM_OK, ret);

        /* result->mask 每个比特标识自检的算法，result->result 每个比特标识对应算法的自检结果。 */
        ehsm_port_printf("The self test algorithms are:\r\n   0x%08x \r\n", result->mask);
        ehsm_port_printf("The self test results are:\r\n   0x%08x \r\n", result->result);

        /* 预期结果是自检到的算法全部成功，即 result->mask = result->result */
        ret = demo_check_val("The execution of self test:", result->mask, result->result);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n\r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n\r\n\r\n");
    }
}

/**
 * @brief 自检的演示入口函数。
 */
void ehsm_bl_demo_self_test_entry(void)
{
    ehsm_port_printf("\r\n\r\n\r\n\r\n==================== eHSM demo for self test starts. "
                     "====================\r\n\r\n\r\n\r\n");
    bl_demo_self_test();
    ehsm_port_printf("\r\n==================== eHSM demo for self test ends. "
                     "====================\r\n\r\n\r\n\r\n");
}

