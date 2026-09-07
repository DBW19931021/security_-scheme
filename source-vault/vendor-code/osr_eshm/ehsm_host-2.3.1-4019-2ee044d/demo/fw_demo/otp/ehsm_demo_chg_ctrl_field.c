#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_chg_ctrl_field.h"

#define DEMO_CTRL_FILED_SIZE (8U)

typedef struct {
    uint64_t value;
    const char *desc;
    ehsm_ctrl_field_e ctrl_field;
} demo_ctrl_field_std_st;

// clang-format off
static demo_ctrl_field_std_st s_ctrl_field_std_data[3] = {
    {
        .desc = "HW ctrl field",
        .ctrl_field = EHSM_CTRL_FIELD_HW,
        .value = 0x000040F000001000
    }, {
        .desc = "eHSM ctrl field",
        .ctrl_field = EHSM_CTRL_FIELD_EHSM,
        .value = 0x0000407700000001

    }, {
        .desc = "SoC ctrl field",
        .ctrl_field = EHSM_CTRL_FIELD_SOC,
        .value = 0x0000000000001077
    }
};
// clang-format on

/**
 * @brief 更改 OTP 控制字段的示例。
 * @param[in] desc OTP 控制字段的描述字符串。
 * @param[in] ctrl_field 需要修改的 OTP 控制字段类型，详见 @ref ehsm_ctrl_field_e。
 * @param[in] value 修改后的值：
 *  - 需要先拷贝至 SoC 与 eHSM 的共享内存，再通过 API 传参。
 *  - 更改 OTP 控制字段的时候，假设需要写入的值是 A, OTP 已经存在的值是 B，鉴于 OTP 特性，已经写过的 bit 不可以改变，
 * 那么被写入的值应该是 A|B，例如想要写入 eHSM 控制字段的是 U64 的 A = 0x10, OTP 已经写入的是 U64 的 B = 0x01，那么调
 * 用更改控制字段的 API 需要写入的值是 A|B = 0x11。
 *
 * @note 更改 OTP 控制字段后，需要 reset eHSM 才能使 eHSM 内部的寄存器同步更新。例如修改 OTP 控制端的 SoC
 * 启动算法， 从 RSA2048 修改为 AES-MAC，若不 reset eHSM，调用 soc_verify 指令或 ehsm_verify_image 校验 SoC
 * 镜像的时候，eHSM 读取内部寄存器会读到校验算法为 RSA2048 而非修改后的 AES-CMAC，导致与预期不符。此时若先 reset
 *  eHSM 即可保证 eHSM 内部寄存器也同步更新。
 *
 */
static void chg_ctrl_field(const char *desc, ehsm_ctrl_field_e ctrl_field, uint64_t value)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint64_t *buffer
        = (uint64_t *)(void *)ehsm_demo_get_buffer(1); /* 用于输入需要修改的值，必须是 SoC 和 eHSM 的共享内存 */
    uint64_t backup_value;                             /* 用于备份 OTP 控制字段的值 */
    uint64_t cur_value;                                /* 用于读取当前的 OTP 控制字段的值 */
    uint32_t ctrl_field_offset_arr[3] = { 0x18U, 0x20U, 0x28U };

    ehsm_port_printf("[Demo of changing ctrl_field starts.] \r\n");
    ehsm_port_printf("The OTP ctrl field is %s .\r\n", desc);

    /* 备份当前 OTP 控制字段 */
    demo_read_otp_from_soc_addr(
        ctrl_field_offset_arr[(uint32_t)ctrl_field], (uint8_t *)&backup_value, DEMO_CTRL_FILED_SIZE);
    cur_value = backup_value;

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用更改控制字段的 API，鉴于 OTP 特性，需要保证已经写过的 bit 不被改变，因此写入 value | cur_value */
    buffer[0] = cur_value | value;
    ret = ehsm_change_control_field(ctx, ctrl_field, buffer);
    ret = demo_check_val("The execution of changing ctrl_field API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，读取更改后的 OTP 控制字段 */
        cur_value = (uint64_t)0;
        demo_read_otp_from_soc_addr(
            ctrl_field_offset_arr[(uint32_t)ctrl_field], (uint8_t *)&cur_value, DEMO_CTRL_FILED_SIZE);
        cur_value &= value;
        ret = demo_check_data("The execution of changing ctrl_field API:", (uint8_t *)&(cur_value),
            DEMO_CTRL_FILED_SIZE, (uint8_t *)&value, DEMO_CTRL_FILED_SIZE);
    }

    /* 通过 SoC 映射地址写 OTP 的方式恢复备份的 OTP 控制字段。 */
    demo_write_otp_from_soc_addr(
        ctrl_field_offset_arr[(uint32_t)ctrl_field], (uint8_t *)&backup_value, DEMO_CTRL_FILED_SIZE);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of changing ctrl_field ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of changing ctrl_field ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 更改 eHSM OTP 控制字段的演示入口函数。
 */
void ehsm_demo_chg_ctrl_field_entry(void)
{
    uint32_t i;
    demo_ctrl_field_std_st *std_data = s_ctrl_field_std_data;
    uint32_t cnt = sizeof(s_ctrl_field_std_data) / sizeof(demo_ctrl_field_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for changing ctrl_field starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("Writting std_data[%d] to OTP ctrl field.\r\n", i);
        chg_ctrl_field(std_data[i].desc, std_data[i].ctrl_field, std_data[i].value);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for changing ctrl_field ends. ==================== "
                     "\r\n\r\n");
}

