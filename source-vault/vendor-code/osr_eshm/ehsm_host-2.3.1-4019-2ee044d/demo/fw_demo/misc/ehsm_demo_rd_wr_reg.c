#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_rd_wr_reg.h"

#define DEMO_REG_BASE_ADDR (0x33400000U)

typedef struct {
    const char *desc;
    uint32_t addr;
    const uint8_t *data;
    uint32_t size;
} demo_reg_std_st;

// clang-format off
static const uint8_t s_reg_data_0[20] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x14, 0x7A, 0xA0, 0xF5
};

static const uint8_t s_reg_data_1[4] = {
    0x01, 0x01, 0x01, 0x01,
};

static const demo_reg_std_st s_reg_data_arr[2] = {
    {
        .desc = "REG data 0",
        .addr = DEMO_REG_BASE_ADDR + 0x4U,
        .data = s_reg_data_0,
        .size = sizeof(s_reg_data_0)
    }, {
        .desc = "REG data 1",
        .addr = DEMO_REG_BASE_ADDR + 0x50U,
        .data = s_reg_data_1,
        .size = sizeof(s_reg_data_1)
    }
};
// clang-format on

/**
 * @brief 读取 REG 数据的示例。
 * @param[in] addr eHSM 地址，REG 数据从该地址读取。
 * @param[in] size 读取数据的字节长度。读取的数据长度必须是 REG 最小可读长度的整数倍。
 *
 */
static void read_reg(uint32_t addr, uint32_t size)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();      /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *reg_data = ehsm_demo_get_buffer(0); /* 通过 API 读取的 REG 数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */

    /* 初始化数据 buffer */
    memset(reg_data, 0x0U, 1024U);

    ehsm_port_printf("[Demo of reading REG starts.] \r\n");

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 read REG API 读取 REG，eHSM 会将数据回写到 SoC 提供的 buffer。*/
    ret = ehsm_read_reg(ctx, reg_data, addr, size);
    ret = demo_check_val("The execution of reading REG API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，打印读取的数据 */
        print_hex("The REG data is \r\n    ", reg_data, size);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of reading REG ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of reading REG ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 写入数据到 REG 的示例。
 * @param[in] data SoC 存储的需要写入的原始数据。
 * @param[in] addr eHSM 地址，数据写入到该地址。
 * @param[in] size 写入数据的字节长度。注意写入数据的长度必须是 REG 最小可写长度的整数倍。
 *
 */
static void write_reg(const uint8_t *data, uint32_t addr, uint32_t size)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();      /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *reg_data = ehsm_demo_get_buffer(0); /* 存储数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t backup_reg_data[128];                /* 用于备份 REG 数据 */

    ehsm_port_printf("[Demo of writing REG starts.] \r\n");

    /* 初始化数据 buffer */
    memset(reg_data, 0x0U, 1024U);

    /* 读取并打印当前 REG 地址对应的数据，并备份 */
    ret = ehsm_read_reg(ctx, reg_data, addr, size);
    memcpy(backup_reg_data, reg_data, size);
    print_hex("The REG data before being written to is \r\n    ", backup_reg_data, size);

    /* 拷贝数据到 SoC 与 eHSM 的共享内存 */
    memset(reg_data, 0x0U, 1024U);
    memcpy(reg_data, data, size);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 write REG API 读取 REG，eHSM 会将数据回写到 SoC 提供的 buffer。*/
    ret = ehsm_write_reg(ctx, reg_data, addr, size);
    ret = demo_check_val("The execution of writing REG API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，打印读取的数据 */
        print_hex("The REG data after being written to is \r\n    ", reg_data, size);
    }

    /* 恢复备份的 REG 数据， reset eHSM 才能改变 eHSM 内部的寄存器等状态 */
    memset(reg_data, 0x0U, 1024U);
    memcpy(reg_data, backup_reg_data, size);
    ret = ehsm_write_reg(ctx, reg_data, addr, size);
    ret = demo_check_val("The execution of recovering backup REG data:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of writing REG ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of writing REG ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 读写 REG 数据的演示入口函数。
 */
void ehsm_demo_rd_wr_reg_entry(void)
{
    const demo_reg_std_st *std_data = s_reg_data_arr;
    uint32_t cnt = sizeof(s_reg_data_arr) / sizeof(demo_reg_std_st);
    uint32_t i;

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for reding and writing REG starts. ==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {

        /* 从 REG 读取数据的示例 */
        ehsm_port_printf("Reading %s from REG.\r\n", std_data[i].desc);
        read_reg(std_data[i].addr, std_data[i].size);

        /* 写数据到 REG 的示例 */
        ehsm_port_printf("Writing %s to REG.\r\n", std_data[i].desc);
        write_reg(std_data[i].data, std_data[i].addr, std_data[i].size);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for reding and writing REG ends. ==================== "
                     "\r\n\r\n");
}

