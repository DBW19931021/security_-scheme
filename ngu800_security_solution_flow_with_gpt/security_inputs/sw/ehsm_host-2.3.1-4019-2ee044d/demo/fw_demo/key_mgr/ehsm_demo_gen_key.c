#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_gen_key.h"

#define DEMO_GEN_KEY_KEY_HANDLE (0x100000U)

typedef struct {
    const char *desc;
    ehsm_key_type_e key_type;
    uint16_t rsa_e_bit_size;
    uint16_t hmac_key_size;
    uint32_t key_handle;
    uint32_t key_privilege;
    const uint8_t *dh_param;
    uint32_t dh_param_size;
} demo_gen_key_std_st;

/* clang-format off */

static const uint8_t s_dh_1024_params[4U + 128U + 4U + 24U + 4U + 4U + 4U] = {
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

static demo_gen_key_std_st s_gen_key_std_data[3] = {
    {
        .desc = "AES128 non XTS key, eHSM distributes key handle",
        .key_type = EHSM_KEY_TYPE_AES_128,
        .rsa_e_bit_size =(uint16_t)0U,
        .hmac_key_size = (uint16_t)0U,
        .key_handle = 0xFFFFFFFFU, /* 表示由 eHSM 密钥管理服务分配 key handle */
        .key_privilege = EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT
            | EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_REMOVE,
        .dh_param = NULL,
        .dh_param_size = 0U
    }, {
        .desc = "HMAC 32B key, Host specifis key handle",
        .key_type = EHSM_KEY_TYPE_HMAC,
        .rsa_e_bit_size = 0U,
        .hmac_key_size = (uint16_t)32U,
        .key_handle = DEMO_GEN_KEY_KEY_HANDLE, /* 表示指定密钥句柄 */
        .key_privilege = EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT
            | EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_REMOVE,
        .dh_param = NULL,
        .dh_param_size = 0U
    }, {
        .desc = "DH 1024 key, Host specifis key handle",
        .key_type = EHSM_KEY_TYPE_DH,
        .rsa_e_bit_size = (uint16_t)0U,
        .hmac_key_size = (uint16_t)0U,
        .key_handle = DEMO_GEN_KEY_KEY_HANDLE, /* 表示指定密钥句柄 */
        .key_privilege = EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT
            | EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_REMOVE,
        .dh_param = s_dh_1024_params,
        .dh_param_size = sizeof(s_dh_1024_params)
    }
};
// clang-format on

/**
 * @brief 生成密钥的演示。
 *
 * @param[in] std_data 生成密钥的标准演示数据，详见 @ref demo_gen_key_std_st
 *
 * @note 生成密钥会根据指定的 key_type 生成随机密钥，HMAC 密钥。
 * - 对于 RSA 密钥，需要指定 rsa_e_bit_size，即公钥 e 的比特长度。
 * - 对于 HMAC 密钥，需要指定 key_size，即生成密钥的字节长度。
 * - 对于 DH 密钥，需要指定 DH 公共参数，p，q，g 和他们的字节长度，其数据格式为
 *  (p_len || p_value || q_len || q_value || g_len || g_value || h_len(可为 0) || h_value（可选）。
 * p_len/q_len/g_len/h_len 均定为 4 字节，其值是对应的 p_value/q_value/g_value/h_value 的字节长度。
 * h_value 指乘法加速的一个预计算数，是可选的，将 h_len 的值设置为 0 表示 h_value 不存在。除 h_len
 * 可以为 0 外，其他数据长度不能为 0，且必须是 4 字节的整数倍。
 * - key handle 可以由两种选择：
 *      - Host 在 API 传参为有效的 key handle，具体参考固件 TRM 文档描述。
 *      - Host 在 API 传参为 0xFFFFFFFF，将由 eHSM 密钥管理服务分配 key handle。
 *
 */
static void demo_gen_key(const demo_gen_key_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();      /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *dh_param = ehsm_demo_get_buffer(0); /* 输入 DH 公共参数的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t dh_param_size;                      /* 用于指定 DH 公共参数的数据字节长度 */
    uint32_t key_handle = std_data->key_handle;  /* 用于指定密钥句柄，若值为 0xFFFFFFFF 表示由 eHSM 分配 key handle */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("Generating for %s, key type is 0x%08x \r\n", std_data->desc, std_data->key_type);

    if (EHSM_KEY_TYPE_DH == std_data->key_type) {
        /* DH 算法需要额外的 DH 参数，主要是 p，q，g 和 h（乘法加速预计算数，可选），拷贝至 SoC 与 eHSM 的共享内存 */
        dh_param_size = std_data->dh_param_size;
        memcpy(dh_param, std_data->dh_param, dh_param_size);
    } else {
        dh_param_size = 0U;
    }

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用生成密钥的 API。 */
    ret = ehsm_km_gen_key(ctx, std_data->key_type, std_data->key_privilege, std_data->rsa_e_bit_size,
        std_data->hmac_key_size, dh_param, dh_param_size, &key_handle);
    ret = demo_check_val("The execution of generating key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        if (0xFFFFFFFF == std_data->key_handle) {
            /* 3. 若指定由 eHSM 分配，打印获取到的 key_handle */
            ehsm_port_printf("The key handle is: 0x%08x \r\n", key_handle);
        } else {
            /* 3. 若指定有效 key handle，则检查返回的 key handle 是否和指定的值一致 */
            ret = demo_check_val("Comparison for key handle:", std_data->key_handle, key_handle);
        }
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
void ehsm_demo_gen_key_entry(void)
{
    uint32_t i;
    demo_gen_key_std_st *std_data = s_gen_key_std_data;
    uint32_t cnt = sizeof(s_gen_key_std_data) / sizeof(demo_gen_key_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for getting public key from private key starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_gen_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for getting public key from private key ends. "
                     "====================\r\n\r\n");
}

