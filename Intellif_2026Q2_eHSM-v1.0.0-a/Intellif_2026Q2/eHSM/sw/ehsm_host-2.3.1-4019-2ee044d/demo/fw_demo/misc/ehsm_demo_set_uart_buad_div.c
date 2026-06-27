#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_set_uart_buad_div.h"

/**
 * @brief 设置 uart 频率分频值的演示。
 *
 * @param[in] buad_div uart 频率分频值，指CPU 频率值除以UART 波特率值的结果。例如假设 CPU 频率为 10 Mhz，
 *                      若想要设置 uart 频率为 115200，则 buad_div = 10 Mhz / 115200 = 86。
 *
 */
static void demo_set_uart_buad_div(uint32_t buad_div)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("The uart buadrate division is %d.\r\n", buad_div);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用设置 uart 频率分频值的 API。 */
    ret = ehsm_set_uart_baudrate(ctx, buad_div);
    ret = demo_check_val("The execution of settinguart buadrate division API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n\r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n\r\n\r\n");
    }
}

/**
 * @brief 设置 uart 频率分频值的演示入口函数。
 *
 */
void ehsm_demo_set_uart_buad_div_entry(void)
{
    uint32_t buad_div_arr[2] = { 56000, 115200 };
    uint32_t i;

    ehsm_port_printf(
        "\r\n\r\n==================== eHSM demo for setting uart buadrate division starts. ==================== "
        "\r\n\r\n");

    for (i = 0U; i < sizeof(buad_div_arr) / sizeof(buad_div_arr[0]); i++) {

        demo_set_uart_buad_div(buad_div_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for setting uart buadrate division ends. ==================== "
                     "\r\n\r\n");
}

