#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_rng.h"

/**
 * @brief 生成随机数的示例。
 * @param[in] algo 随机数后处理算法。
 * @param[in] require_size 生成随机数的字节长度。
 *
 */
static void rng_gen(ehsm_rng_algo_e algo, uint32_t require_size)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();         /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *random_data = ehsm_demo_get_buffer(0); /* 存储随机数的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    const char *algo_str = (EHSM_RNG_ALGO_AES_CTR_DRBG == algo) ? "AES_CTR_DRBG" : "SM4_CTR_DRBG";

    /* 初始化数据 buffer */
    memset(random_data, 0x0U, 512U);

    ehsm_port_printf("[Demo of RNG with %s starts.] \r\n", algo_str);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用 rng API 计算并获取随机数，eHSM 会生成随机数并回写到 SoC 提供的输出 buffer。*/
    ret = ehsm_gen_random(ctx, algo, random_data, require_size);
    ret = demo_check_val("The execution of rng API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，打印生成的随机数 */
        ehsm_port_printf("The generated %d Bytes random data is:\r\n", require_size);
        print_hex("", random_data, require_size);
        ehsm_port_printf("[Demo of RNG with %s ends with success !!!] \r\n\r\n", algo_str);

    } else {
        ehsm_port_printf("[Demo of RNG with %s ends with failure. !!!] \r\n\r\n", algo_str);
    }
}

/**
 * @brief RNG 算法演示入口函数。
 */
void ehsm_demo_rng_entry(void)
{
    uint32_t require_size_arr[4] = {
        1,
        128,
        129,
        1024,
    };
    uint32_t i;

    ehsm_port_printf("\r\n\r\n==================== eHSM demo for rng starts. ==================== "
                     "\r\n\r\n");

    for (i = 0U; i < sizeof(require_size_arr) / sizeof(require_size_arr[0]); i++) {
        /* 生成经过 AES CTR 后处理的随机数 */
        rng_gen(EHSM_RNG_ALGO_AES_CTR_DRBG, require_size_arr[i]);

        /* 生成经过 SM4 CTR 后处理的随机数 */
        rng_gen(EHSM_RNG_ALGO_SM4_CTR_DRBG, require_size_arr[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for rng ends. ==================== \r\n\r\n");
}

