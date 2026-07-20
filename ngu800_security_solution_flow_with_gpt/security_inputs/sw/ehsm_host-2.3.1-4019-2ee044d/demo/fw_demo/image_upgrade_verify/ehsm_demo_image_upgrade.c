#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_image_upgrade.h"
#include "common/image_data/ehsm_demo_image.h"

typedef struct {
    const char *desc;
    const uint8_t *image;
    uint32_t image_size;
} demo_image_upgrade_std_st;

/* clang-format off */

static demo_image_upgrade_std_st s_image_upgrade_std_data[2] = {
    {
        .desc = "eHSM image upgrade using ehsm-ehsmkey.",
        .image = g_ehsm_demo_ehsm_upgrade_image,
        .image_size = sizeof(g_ehsm_demo_ehsm_upgrade_image)
    }, {
        .desc = "SoC image upgrade using soc-sockey.",
        .image = g_ehsm_demo_soc_upgrade_image,
        .image_size = sizeof(g_ehsm_demo_soc_upgrade_image)
    },
};
// clang-format on

/**
 * @brief 固件镜像升级演示。
 *
 * @param[in] std_data 镜像升级的标准演示数据，详见 @ref demo_image_upgrade_std_st
 *
 * @note Firmware 支持 eHSM/SoC 镜像的升级。
 *
 */
static void demo_image_upgrade(const demo_image_upgrade_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();       /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *image = ehsm_demo_get_buffer(0);     /* 输入镜像的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t image_size;                          /* 用于指定镜像的数据字节长度 */
    uint8_t *image_out = ehsm_demo_get_buffer(5); /* 输出解密后的镜像的 buffer，必须是 SoC 和 eHSM 的共享内存 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("%s.\r\n", std_data->desc);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用升级镜像的 API。 */
    image_size = std_data->image_size;
    memcpy(image, std_data->image, image_size);

    /* 需要设置 image_out 作为解密后的安全启动镜像输出地址 */
    ret = ehsm_upgrade_fw_image(ctx, image, image_size, image_out);
    ret = demo_check_val("The execution image upgrade API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 固件镜像升级三段式演示。
 *
 * @param[in] std_data 镜像升级的标准演示数据，详见 @ref demo_image_upgrade_std_st
 *
 * @note Firmware 支持 eHSM/SoC 镜像的升级。
 *
 */
static void demo_image_upgrade_steps(const demo_image_upgrade_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();       /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *image = ehsm_demo_get_buffer(0);     /* 输入镜像的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t image_size;                          /* 用于指定镜像的数据字节长度 */
    uint8_t *image_out = ehsm_demo_get_buffer(5); /* 输出解密后的镜像的 buffer，必须是 SoC 和 eHSM 的共享内存 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("%s.\r\n", std_data->desc);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用升级镜像的 API。 */
    image_size = std_data->image_size;
    memcpy(image, std_data->image, image_size);

    uint32_t image_header_size = 1024;
    uint32_t offset = 0;
    uint32_t out_offset = 0;

    /* 3. 初始化 */
    ret = ehsm_upgrade_fw_image_init(ctx, image + offset, image_header_size, image_out);
    ret = demo_check_val("The execution image upgrade API:", EHSM_OK, ret);
    offset += image_header_size;

    /* 4. 分块更新数据 */
    while (offset < image_size) {
        uint32_t update_size = image_size - offset;
        if (update_size > 1024) {
            update_size = 1024;
        }
        
        if (offset + update_size < image_size) {
            // 非最后一块
            ret = ehsm_upgrade_fw_image_update(ctx, image + offset, update_size, image_out + out_offset);
        } else {
            // 最后一块
            ret = ehsm_upgrade_fw_image_finish(ctx, image + offset, update_size, image_out + out_offset);
        }
        ret = demo_check_val("The execution image upgrade API:", EHSM_OK, ret);
        offset += update_size;
        out_offset += update_size;
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}
/**
 * @brief 升级镜像的演示入口函数。
 */
void ehsm_demo_image_upgrade_entry(void)
{
    uint32_t i;
    demo_image_upgrade_std_st *std_data = s_image_upgrade_std_data;
    uint32_t cnt = sizeof(s_image_upgrade_std_data) / sizeof(demo_image_upgrade_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for image upgrade starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_image_upgrade(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for image upgrade ends. "
                     "====================\r\n\r\n");

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for image upgrade 3 steps starts. "
                     "====================\r\n\r\n");
    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_image_upgrade_steps(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for image upgrade 3 steps ends. "
                     "====================\r\n\r\n");
}

