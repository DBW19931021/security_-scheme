#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_rd_wr_otp.h"

#define DEMO_OTP_BASE_ADDR (0x33000000U)

typedef struct {
    const char *desc;
    uint32_t addr;
    const uint8_t *data;
    uint32_t size;
} demo_otp_std_st;

// clang-format off
static const uint8_t s_otp_uid[20] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x14, 0x7A, 0xA0, 0xF5
};

static const uint8_t s_otp_ver_cnt[4] = {
    0x01, 0x01, 0x01, 0x01,
};

static const demo_otp_std_st s_otp_data_arr[2] = {
    {
        .desc = "UID",
        .addr = DEMO_OTP_BASE_ADDR + 0x4U,
        .data = s_otp_uid,
        .size = sizeof(s_otp_uid)
    }, {
        .desc = "Version Counter",
        .addr = DEMO_OTP_BASE_ADDR + 0x50U,
        .data = s_otp_ver_cnt,
        .size = sizeof(s_otp_ver_cnt)
    }
};
// clang-format on

/**
 * @brief 读取 OTP 数据的示例。
 * @param[in] addr eHSM 地址，OTP数据从该地址读取。
 * @param[in] size 读取数据的字节长度。读取的数据长度必须是 OTP 最小可读长度的整数倍。
 *
 */
static void read_otp(uint32_t addr, uint32_t size)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();      /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *otp_data = ehsm_demo_get_buffer(0); /* 通过 API 读取的 OTP 数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t remapped_otp_data[32];               /* 存储通过 SoC 映射地址读取的 OTP 数据 */

    /* 初始化数据 buffer */
    memset(otp_data, 0x0U, 1024U);

    ehsm_port_printf("[Demo of reading OTP starts.] \r\n");

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 read OTP API 读取 OTP，eHSM 会将数据回写到 SoC 提供的 buffer。*/
    ret = ehsm_read_otp(ctx, otp_data, addr, size);
    ret = demo_check_val("The execution of reading OTP API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，检查读出的数据是否和直接通过 SoC 映射地址读到的一致 */
        demo_read_otp_from_soc_addr(
            addr - DEMO_OTP_BASE_ADDR, remapped_otp_data, size); /* 从 SoC 映射地址读 OTP 数据 */
        ret = demo_check_data("Data comparison", remapped_otp_data, size, otp_data, size);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of reading OTP ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of reading OTP ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 写入数据到 OTP 的示例。
 * @param[in] data SoC 存储的需要写入的原始数据。
 * @param[in,out] addr eHSM 地址，数据写入到该地址。
 * @param[in] size 写入数据的字节长度。注意写入数据的长度必须是 OTP 最小可写长度的整数倍。
 *
 */
static void write_otp(const uint8_t *data, uint32_t addr, uint32_t size)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();    /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *buffer = ehsm_demo_get_buffer(0); /* 存储数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint8_t backup_otp_data[32];               /* 备份 OTP 数据的 buffer */
    uint8_t remapped_otp_data[32];             /* 存储通过 SoC 映射地址读取的 OTP 数据 */

    /* 初始化数据 buffer */
    memset(buffer, 0x0U, 1024U);

    /* 拷贝数据到 SoC 与 eHSM 的共享内存 */
    memcpy(buffer, data, size);

    /* 备份 OTP 数据，用于后面还原 */
    demo_read_otp_from_soc_addr(addr - DEMO_OTP_BASE_ADDR, backup_otp_data, size);

    ehsm_port_printf("[Demo of writing OTP starts.] \r\n");

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 write OTP API 读取 OTP，eHSM 会将数据回写到 SoC 提供的 buffer。*/
    ret = ehsm_write_otp(ctx, buffer, addr, size);
    ret = demo_check_val("The execution of writing OTP API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /*3. eHSM FW 返回成功，需要 reset eHSM 写入的数据才能改变 eHSM 内部的寄存器等状态 */
        demo_reset_ehsm_wait_ready();

        /* 通过 SoC 映射地址读取 OTP 数据，看是否写入成功 */
        demo_read_otp_from_soc_addr(
            addr - DEMO_OTP_BASE_ADDR, remapped_otp_data, size); /* 从 SoC 映射地址读 OTP 数据 */
        ret = demo_check_data("Data comparison", data, size, remapped_otp_data, size);
    }

    /* 恢复备份的 OTP 数据， reset eHSM 才能改变 eHSM 内部的寄存器等状态 */
    demo_write_otp_from_soc_addr(addr - DEMO_OTP_BASE_ADDR, backup_otp_data, size);
    demo_reset_ehsm_wait_ready();

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of writing OTP ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of writing OTP ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 读写 OTP 数据的演示入口函数。
 */
void ehsm_demo_rd_wr_otp_entry(void)
{
    const demo_otp_std_st *std_data = s_otp_data_arr;
    uint32_t cnt = sizeof(s_otp_data_arr) / sizeof(demo_otp_std_st);
    uint32_t i;

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for reding and writing OTP starts. ==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {

        /* 从 OTP 读取数据的示例 */
        ehsm_port_printf("Reading %s from OTP.\r\n", std_data[i].desc);
        read_otp(std_data[i].addr, std_data[i].size);

        /* 写数据到 OTP 的示例 */
        ehsm_port_printf("Writing %s to OTP.\r\n", std_data[i].desc);
        write_otp(std_data[i].data, std_data[i].addr, std_data[i].size);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for reding and writing OTP ends. ==================== "
                     "\r\n\r\n");
}

