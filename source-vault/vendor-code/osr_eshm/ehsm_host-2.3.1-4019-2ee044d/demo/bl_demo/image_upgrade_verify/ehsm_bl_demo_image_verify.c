#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/bl_api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_bl_demo_image_verify.h"
#include "common/image_data/ehsm_demo_image.h"

typedef struct {
    const char *desc;
    bool_t check_version;
    const uint8_t *image;
    uint32_t image_size;
} bl_demo_image_verify_std_st;

/* clang-format off */

static bl_demo_image_verify_std_st s_bl_image_verify_std_data[4] = {
    {
        .desc = "eHSM image verify using ehsm-ehsmkey and checking version counter.",
        .check_version = true,
        .image = g_ehsm_demo_ehsm_boot_image,
        .image_size = sizeof(g_ehsm_demo_ehsm_boot_image)
    }, {
        .desc = "eHSM image verify using ehsm-ehsmkey but not checking version counter.",
        .check_version = false,
        .image = g_ehsm_demo_ehsm_boot_image,
        .image_size = sizeof(g_ehsm_demo_ehsm_boot_image)
    }, {
        .desc = "SoC image verify using soc-sockey and checking version counter.",
        .check_version = true,
        .image = g_ehsm_demo_soc_boot_image,
        .image_size = sizeof(g_ehsm_demo_soc_boot_image)
    }, {
        .desc = "SoC image verify using soc-sockey but not checking version counter.",
        .check_version = false,
        .image = g_ehsm_demo_soc_boot_image,
        .image_size = sizeof(g_ehsm_demo_soc_boot_image)
    },
};
// clang-format on

/**
 * @brief 镜像校验的演示。
 *
 * @param[in] std_data 镜像校验的标准演示数据，详见 @ref demo_image_verify_std_st
 *
 * @note 支持 Firmware 和 SoC 镜像的校验
 *
 */
static void bl_demo_image_verify(const bl_demo_image_verify_std_st *std_data)
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

    /* 2. 调用校验镜像的 API。 */
    image_size = std_data->image_size;
    memcpy(image, std_data->image, image_size);
    /**
     * - 若为 eHSM 镜像，无需设置 image_out，这里仅演示校验，因此将跳转设置为 boot = flase
     * - 若为 SoC 镜像，无跳转的说法，因此设为 false，且镜像若为加密镜像，需要设置 image_out 作为解密后的镜像输出地址
     */
    ret = ehsm_bl_verify_image(ctx, image, image_size, std_data->check_version, false, image_out);
    ret = demo_check_val("The execution of image verification API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n\r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n\r\n\r\n");
    }
}

/**
 * @brief 校验镜像的演示入口函数。
 */
void ehsm_bl_demo_image_verify_entry(void)
{
    uint32_t i;
    bl_demo_image_verify_std_st *std_data = s_bl_image_verify_std_data;
    uint32_t cnt = sizeof(s_bl_image_verify_std_data) / sizeof(bl_demo_image_verify_std_st);
    ehsm_port_printf("\r\n\r\n\r\n\r\n==================== eHSM demo for image verification starts. "
                     "====================\r\n\r\n\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        bl_demo_image_verify(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for image verification ends. "
                     "====================\r\n\r\n\r\n\r\n");
}

