// #% #if CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN
#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_get_pub_from_priv.h"

#define DEMO_GET_PUB_FROM_PRIV_KEY_HANDLE (0x100000U)

typedef struct {
    const char *desc;
    ehsm_key_type_e key_type;
    uint32_t key_handle;
    const uint8_t *dh_param;
    uint32_t dh_param_size;
} demo_get_pub_from_priv_key_std_st;

/* clang-format off */

static const uint8_t s_get_pub_from_priv_1024_params[4U + 128U + 4U + 24U + 4U + 4U + 4U] = {
    /* p_len */
    0x80,0x00,0x00,0x00,
    /* p_value */
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xA2,0xDA,0x0F,0xC9,0x34,0xC2,0x68,0x21,
    0x8B,0x62,0xC6,0xC4,0xD1,0x1C,0xDC,0x80,0x08,0x4E,0x02,0x29,0x74,0xCC,0x67,0x8A,
    0xA6,0xBE,0x0B,0x02,0x22,0x9B,0x13,0x3B,0x79,0x08,0x4A,0x51,0xDD,0x04,0x34,0x8E,
    0xB3,0x19,0x95,0xEF,0x1B,0x43,0x3A,0xCD,0x6D,0x0A,0x2B,0x30,0x37,0x14,0x5F,0xF2,
    0x6D,0x35,0xE1,0x4F,0x45,0xC2,0x51,0x6D,0x76,0xB5,0x85,0xE4,0xC6,0x7E,0x5E,0x62,
    0xE9,0x42,0x4C,0xF4,0x6B,0xED,0x37,0xA6,0xB6,0x5C,0xFF,0x0B,0xED,0xB7,0x06,0xF4,
    0xFB,0x6B,0x38,0xEE,0xA5,0x9F,0x89,0x5A,0x11,0x24,0x9F,0xAE,0xE6,0x1F,0x4B,0x7C,
    0x51,0x66,0x28,0x49,0x81,0x53,0xE6,0xEC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    /* q_len */
    0x18,0x00,0x00,0x00,
    /* q_value */
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xA2,0xDA,0x0F,0xC9,0x34,0xC2,0x68,0x21,
    0x8B,0x62,0xC6,0xC4,0xD1,0x1C,0xDC,0x80,
    /* g_len */
    0x04,0x00,0x00,0x00,
    /* g_value */
    0x00,0x00,0x00,0x02,
    /* h_len = 0, the initialization will automatically fill 4 bytes with 0 here */
};

static demo_get_pub_from_priv_key_std_st s_get_pub_from_priv_key_std_data[3] = {
    {
        .desc = "Get ECC public key from private key.",
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .key_handle = DEMO_GET_PUB_FROM_PRIV_KEY_HANDLE,
        .dh_param = NULL,
        .dh_param_size = 0U
    }, {
        .desc = "Get SM2 public key from private key.",
        .key_type = EHSM_KEY_TYPE_SM2,
        .key_handle = DEMO_GET_PUB_FROM_PRIV_KEY_HANDLE,
        .dh_param = NULL,
        .dh_param_size = 0U
    }, {
        .desc = "Get DH-1024 public key from private key.",
        .key_type = EHSM_KEY_TYPE_DH,
        .key_handle = DEMO_GET_PUB_FROM_PRIV_KEY_HANDLE,
        .dh_param = s_get_pub_from_priv_1024_params,
        .dh_param_size = sizeof(s_get_pub_from_priv_1024_params)
    }
};
// clang-format on

/**
 * @brief 通过私钥获取公钥的演示。
 *
 * @param[in] std_data 通过私钥获取公钥的标准演示数据，详见 @ref demo_get_pub_from_priv_key_std_st
 *
 * @note 主要演示 eHSM 存在私钥，通过 API 获取公钥的操作。
 * - 对于 DH 密钥，需要指定 DH 公共参数，p，q，g 和他们的字节长度，其数据格式为
 *  (p_len || p_value || q_len || q_value || g_len || g_value || h_len(可为 0) || h_value（可选）。
 * p_len/q_len/g_len/h_len 均定为 4 字节，其值是对应的 p_value/q_value/g_value/h_value 的字节长度。
 * h_value 指乘法加速的一个预计算数，是可选的，将 h_len 的值设置为 0 表示 h_value 不存在。除 h_len
 * 可以为 0 外，其他数据长度不能为 0，且必须是 4 字节的整数倍。
 *
 */
static void demo_get_pub_from_priv_key(const demo_get_pub_from_priv_key_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();        /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *dh_param = ehsm_demo_get_buffer(0);   /* 输入 DH 公共参数的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t dh_param_size;                        /* 用于指定 DH 公共参数的数据字节长度 */
    uint8_t *pub_key = ehsm_demo_get_buffer(5);    /* 输出公钥的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t pub_key_size;                         /* 用于指定输出公钥 buffer 的数据字节长度 */
    uint32_t key_handle = std_data->key_handle;    /* 用于指定密钥句柄 */
    ehsm_key_type_e key_type = std_data->key_type; /* 用于指定生密钥类型 */
    bool_t need_remove = false;                    /* 用于标记密钥是否需要移除 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("%s \r\n", std_data->desc);

    if (EHSM_KEY_TYPE_DH == key_type) {
        /* DH 算法需要额外的 DH 参数，主要是 p，q，g 和 h（乘法加速预计算数，可选），拷贝至 SoC 与 eHSM 的共享内存 */
        dh_param_size = std_data->dh_param_size;
        memcpy(dh_param, std_data->dh_param, dh_param_size);
    } else {
        dh_param = NULL;
        dh_param_size = 0U;
    }

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 要通过私钥获取公钥，则 eHSM 需要先存在一个私钥，这里先根据密钥类型（算法）生成一个私钥。 */
    ret = ehsm_km_gen_key(ctx, key_type, EHSM_KEY_PRIV_REMOVE, 0, 0, dh_param, dh_param_size, &key_handle);
    ret = demo_check_val("The execution of generating a target key:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /* 3. 调用生成密钥的 API。 */
        pub_key_size
            = 0x1000; /* 作为输入，是公钥 buffer 的大小，必须大于等于实际公钥的字节长度；作为输出是公钥实际长度 */
        ret = ehsm_km_get_pub_from_priv(ctx, key_handle, dh_param, dh_param_size, pub_key, &pub_key_size, &key_type);
        ret = demo_check_val("The execution of get_pub_from_priv API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* 4. 比较返回的密钥类型是否和指定一致，并打印公钥 */
        ret = demo_check_val("Comparison for key type:", std_data->key_type, key_type);
        ehsm_port_printf("The actual public key size is %d. \r\n", pub_key_size);
        print_hex("The public key is: \r\n    ", pub_key, pub_key_size);
    }

    if (need_remove) {
        demo_remove_key(ctx, key_handle);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 生成密钥的演示入口函数。
 */
void ehsm_demo_get_pub_from_priv_key_entry(void)
{
    uint32_t i;
    demo_get_pub_from_priv_key_std_st *std_data = s_get_pub_from_priv_key_std_data;
    uint32_t cnt = sizeof(s_get_pub_from_priv_key_std_data) / sizeof(demo_get_pub_from_priv_key_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for getting public key from private key starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_get_pub_from_priv_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for getting public key from private key ends. "
                     "====================\r\n\r\n");
}

// #% #endif /* CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN */
