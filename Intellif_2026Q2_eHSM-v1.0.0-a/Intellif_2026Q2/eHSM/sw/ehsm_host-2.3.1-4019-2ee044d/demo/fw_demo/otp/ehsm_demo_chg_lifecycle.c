#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_chg_lifecycle.h"

/**
 * @brief 更改生命周期状态的示例。
 * @param[in] from_lc 更改前的 eHSM 生命周期值。
 * @param[in] to_lc 更改后的 eHSM 生命周期。
 *
 * @note 该函数会先将生命周期切换成 from_lc，然后演示从 from_lc 切换成 to_lc。
 * - to_lc 必须大于等于 from_lc。
 * - 从任意生命周期切换到调试模式（DEBUG_MODE）时， eHSM 会删除所有的非 OTP 密钥。
 * - API 只是写 OTP 数据，只有 reset eHSM 才能改变 eHSM 内部和生命周期相关的寄存器。
 * - 在用户模式下更改生命周期需要新进行 USER_DEBUG 鉴权，鉴权通过才允许切换生命周期，
 *      由于本示例不包含鉴权，因此从用户模式更改生命周期会报错。
 */
static void chg_lifecycle(ehsm_lifecycle_e from_lc, ehsm_lifecycle_e to_lc)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    ehsm_lifecycle_e backup_lc;             /* 用于备份 eHSM 的生命周期值 */
    ehsm_lifecycle_e cur_lc;                /* 用于读取当前的生命周期 */

    ehsm_port_printf("[Demo of changing lifecycle starts.] \r\n");

    /* 备份当前生命周期 */
    backup_lc = demo_get_lifecycle();

    /* 通过 SoC 映射地址写 OTP 的方式切换生命周期为 from_lc */
    demo_chg_lifecycle_from_soc(from_lc);
    demo_reset_ehsm_wait_ready(); /* Reset eHSM 使切换生效 */

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用更改生命周期的 API，从 from_lc 切换到 to_lc。*/
    ret = ehsm_change_lifecycle(ctx, to_lc);

    if (EHSM_LC_USER == from_lc) {
        ehsm_port_printf(
            "Change lifecycle from USER_MODE to other mode without USER_DEBUG is failed with 0x%08x\r\n", ret);
        ret = EHSM_OK;
    } else {
        ret = demo_check_val("The execution of changing lifecycle API:", EHSM_OK, ret);
        if (EHSM_OK == ret) {
            /* eHSM FW 返回成功，此时未 reset eHSM，读取并打印生命周期 */
            cur_lc = EHSM_LC_DESTORY;
            cur_lc = demo_get_lifecycle();
            ehsm_port_printf("The lifecycle before restart eHSM is: %d \r\n", (uint32_t)cur_lc);

            /* 3.  Reset eHSM 使切换生效。*/
            demo_reset_ehsm_wait_ready(); /* */

            /* 读取当前的 eHSM 生命周期，检查是否如更改成功。*/
            cur_lc = EHSM_LC_DESTORY;
            cur_lc = demo_get_lifecycle();
            ehsm_port_printf("The lifecycle after restart eHSM is: %d \r\n", (uint32_t)cur_lc);
            ret = demo_check_val("Lifecycle Comparison", (uint32_t)to_lc, (uint32_t)cur_lc);
        }
    }

    /* 通过 SoC 映射地址写 OTP 的方式恢复生命周期为 backup_lc。 */
    demo_chg_lifecycle_from_soc(backup_lc);
    demo_reset_ehsm_wait_ready(); /* Reset eHSM 使切换生效 */

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of changing lifecycle ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of changing lifecycle ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 更改 eHSM 生命周期的演示入口函数。
 */
void ehsm_demo_chg_lifecycle_entry(void)
{
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for changing lifecycle starts. "
                     "==================== "
                     "\r\n\r\n");

    /* 从测试模式（TESE_MODE）切换到其他生命周期 */
    chg_lifecycle(EHSM_LC_TEST, EHSM_LC_DEVELOP);

    /* 从开发模式（DEVELOP_MODE）切换到其他生命周期 */
    chg_lifecycle(EHSM_LC_DEVELOP, EHSM_LC_MANUFACTURE);

    /* 从制造模式（DEVELOP_MODE）切换到其他生命周期 */
    chg_lifecycle(EHSM_LC_MANUFACTURE, EHSM_LC_USER);

    /* 从用户模式（USER_MODE）切换到其他模式，预期失败，因为本示例未进行 USER_DEBUG 鉴权。 */
    chg_lifecycle(EHSM_LC_USER, EHSM_LC_DEBUG);

    ehsm_port_printf("\r\n==================== eHSM demo for changing lifecycle ends. ==================== "
                     "\r\n\r\n");
}

