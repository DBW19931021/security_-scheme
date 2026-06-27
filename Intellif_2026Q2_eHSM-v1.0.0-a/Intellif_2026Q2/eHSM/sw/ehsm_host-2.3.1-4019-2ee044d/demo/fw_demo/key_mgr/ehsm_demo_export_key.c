#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_export_key.h"

#define DEMO_EXPORT_KEY_KEY_HANDLE           (0x00100000)
#define DEMO_EXPORT_KEY_TRANSPORT_KEY_HANDLE (0x00200010)
#define DEMO_EXPORT_KEY_AUTH_KEY_HANDLE      (DEMO_EXPORT_KEY_TRANSPORT_KEY_HANDLE)

typedef struct {
    const char *desc;
    ehsm_key_type_e key_type;
    bool_t is_plain_key;
    ehsm_key_part_e req_key_part;
    ehsm_key_type_e transport_key_type;
    ehsm_key_type_e auth_key_type;
    uint8_t reserved;
    uint32_t key_handle;
    const uint32_t transport_key_handle;
    const uint32_t auth_key_handle;
} demo_export_key_std_st;

/* clang-format off */
static demo_export_key_std_st s_export_key_std_data[4] = {
    {
        .desc = "Export AES128 palin key",
        .key_type = EHSM_KEY_TYPE_AES_128,
        .is_plain_key = true,
        .req_key_part = EHSM_KEY_PART_SYMM_KEY,
        .transport_key_type = EHSM_KEY_TYPE_AES_128,
        .auth_key_type = EHSM_KEY_TYPE_AES_128,
        .key_handle = DEMO_EXPORT_KEY_KEY_HANDLE,
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
    }, { /* 需要 OTP 存在具有传输权限的密钥 */
        .desc = "Export SECP256R1 keypair in ciphertext",
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .is_plain_key = true,
        .req_key_part = EHSM_KEY_PART_KEY_PAIR,
        .transport_key_type = EHSM_KEY_TYPE_AES_128,
        .auth_key_type = EHSM_KEY_TYPE_AES_128,
        .key_handle = DEMO_EXPORT_KEY_KEY_HANDLE,
        .transport_key_handle = DEMO_EXPORT_KEY_TRANSPORT_KEY_HANDLE,
        .auth_key_handle = DEMO_EXPORT_KEY_AUTH_KEY_HANDLE,
    }, {
        .desc = "Export SECP256R1 public key in plaintext",
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .is_plain_key = true,
        .req_key_part = EHSM_KEY_PART_PUBLIC_KEY,
        .transport_key_type = EHSM_KEY_TYPE_AES_128,
        .auth_key_type = EHSM_KEY_TYPE_AES_128,
        .key_handle = DEMO_EXPORT_KEY_KEY_HANDLE,
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
    }, {
        .desc = "Export SECP256R1 private key in ciphertext",
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .is_plain_key = true,
        .req_key_part = EHSM_KEY_PART_PRIVATE_KEY,
        .transport_key_type = EHSM_KEY_TYPE_SM4,
        .auth_key_type = EHSM_KEY_TYPE_AES_256,
        .key_handle = DEMO_EXPORT_KEY_KEY_HANDLE,
        .transport_key_handle = DEMO_EXPORT_KEY_TRANSPORT_KEY_HANDLE,
        .auth_key_handle = DEMO_EXPORT_KEY_AUTH_KEY_HANDLE,
    }
};
// clang-format on

/**
 * @brief 导出密钥的演示。
 *
 * @param[in] std_data 导出密钥的标准演示数据，详见 @ref demo_export_key_std_st
 *
 * @note 导出密钥会根据指定的 key_type 导出密钥，导出的密钥数据格式为 @ref ehsm_key_format_st，其中：
 *      - 导出明文密钥，密钥长度必须和算法匹配：
 *          - 导出对称密钥/随机密钥/HMAC 密钥，pub_key_size = 0，pri_key_size = 密钥长度。key_value 为明文密钥。
 *          - 导出非对称密钥:
 *              - 若导出密钥对，pub_key_size = 公钥明文长度，pri_key_size = 私钥明文长度。key_value =（公钥 || 私钥）。
 *              - 若导出公钥，pub_key_size = 公钥明文长度，pri_key_size = 0。key_value =公钥。
 *              - 若导出私钥，pub_key_size = 0，pri_key_size = 私钥明文长度。key_value = 私钥。
 *      - 导出密文密钥，明文密钥长度必须和算法匹配：
 *          - 需要事先存在一个能够用于传输保护（加密）的 auth 密钥。由 SoC 使用该 auth 密钥对待导出的密钥进行加密：
 *              - 解密算法由传输保护（加密）密钥的密钥类型（算法）决定。
 *              - 模式为 CBC，IV 是 16 字节全 0x00， PKCS7 填充。
 *              - 若导出的是对称密钥/随机密钥/HMAC 密钥，需要加密所有密钥，且 pub_key_size = 0，pri_key_size =
 *                  明文密钥长度。key_value = 加密后的密文数据。
 *              - 若导出的是非对称密钥，仅需要加密私钥：
 *                  - 若导出的是密钥对，pub_key_size = 公钥明文长度，pri_key_size = 私钥明文长度。
 *                      key_value =（公钥明文 || 加密后私钥密文）。
 *                  - 若导出的是私钥，pub_key_size = 0，pri_key_size = 私钥明文长度。
 *                      key_value = 加密后私钥密文。
 *      - 若设置密钥需要导出 MAC 签名，则认证密钥需要以及存在于 eHSM 且具有作为传输密钥的权限。使用的是 CMAC，
 *          算法由认证密钥的密钥类型（算法）决定。
 *      - 密文导出和生成 MAC 签名需要 eHSM 已经存在一个具有传输权限的密钥：
 *          - 可以是配置了传输权限的 OTP 密钥
 *          - 也可以是某个 OTP 密钥具有派生传输密钥的权限，并用该 OTP 密钥派生出一个具有传输权限的密钥
 *              若没有该密钥，则不支持密文导出功能。
 */
static void demo_export_key(const demo_export_key_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 输入密钥数据的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 用于指定密钥数据的 buffer 的字节长度 */
    uint8_t *mac = ehsm_demo_get_buffer(5);              /* 输出 mac 的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t mac_size;                                   /* 用于指定 mac 的 buffer 字节长度 */
    uint32_t key_handle = std_data->key_handle;          /* 用于指定导出的密钥句柄 */
    uint32_t priviledge;                                 /* 用于指定密钥权限 */
    bool_t need_remove = false;                          /* 用于标记密钥是否需要移除 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("exporting for %s. \r\n", std_data->desc);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 先生成一个密钥，或者保证 eHSM 已经存在一个具有被导出权限的密钥 */
    priviledge = EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT | EHSM_KEY_PRIV_EXPORT_PLAIN
        | EHSM_KEY_PRIV_EXPORT_CIPHER | EHSM_KEY_PRIV_REMOVE;
    ret = ehsm_km_gen_key(ctx, std_data->key_type, priviledge, 0, 0, NULL, 0, &key_handle);
    ret = demo_check_val("The execution of generating a target key:", EHSM_OK, ret);
    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        /* 3. 调用导出密钥的 API。 */
        key_data_size = 0x1000; /* 作为输入是 buffer 的大小，作为输出是返回的密钥数据的总长度 */

        /* 若需要生成认证 MAC */
        if (0xFFFFFFFF != std_data->auth_key_handle) {
            mac_size = 16;
        } else {
            mac = NULL;
            mac_size = 0;
        }

        ret = ehsm_km_export_key(ctx, key_handle, std_data->transport_key_handle, std_data->auth_key_handle,
            std_data->req_key_part, key_data, &key_data_size, mac, &mac_size);
        ret = demo_check_val("The execution of exporting key API:", EHSM_OK, ret);

        if (EHSM_OK == ret) {
            /* 3. 打印获取到的 密钥数据 */
            print_hex("The head of key data is: \r\n    ", (uint8_t *)key_data, sizeof(ehsm_key_format_st));
            print_hex("The key value of key data is: \r\n    ", key_data->key_value,
                key_data_size - sizeof(ehsm_key_format_st));
        }
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
 * @brief 导出密钥的演示入口函数。
 */
void ehsm_demo_export_key_entry(void)
{
    uint32_t i;
    demo_export_key_std_st *std_data = s_export_key_std_data;
    uint32_t cnt = sizeof(s_export_key_std_data) / sizeof(demo_export_key_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for exporting key starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_export_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for exporting key ends. ====================\r\n\r\n");
}

