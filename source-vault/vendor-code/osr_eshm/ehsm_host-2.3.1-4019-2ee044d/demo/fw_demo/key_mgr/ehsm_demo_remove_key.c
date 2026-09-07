#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_remove_key.h"

#define DEMO_REMOVE_KEY_KEY_HANDLE (0x100000U)

/**
 * @brief 移除密钥的演示。
 *
 * @param[in] key_handle 移除密钥的密钥句柄。
 *
 * @note 被移除的密钥的 key_handle 需要有效，且该密钥具有被移除的权限，否则移除失败。
 *
 */
static void demo_remove_exsist_key(uint32_t key_handle)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("Remove key for key_handle = 0x%08x \r\n", key_handle);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 要移除密钥，需要 eHSM 先存在一个密钥，且具有被移除的权限，这里生成一个密钥作为演示 */
    ret = ehsm_km_gen_key(ctx, EHSM_KEY_TYPE_AES_128, EHSM_KEY_PRIV_REMOVE, 0, 0, NULL, 0, &key_handle);
    ret = demo_check_val("The execution of generating key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 2. 调用移除密钥的 API。 */
        ret = ehsm_km_remove_key(ctx, key_handle);
        ret = demo_check_val("The execution of removing key API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 移除密钥的演示入口函数。
 */
void ehsm_demo_remove_key_entry(void)
{
    uint32_t key_handle = DEMO_REMOVE_KEY_KEY_HANDLE;
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for removing key starts. "
                     "====================\r\n\r\n");

    demo_remove_exsist_key(key_handle);

    ehsm_port_printf("\r\n==================== eHSM demo for removing key ends. ====================\r\n\r\n");
}

