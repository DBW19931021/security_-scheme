#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_import_key.h"

#define DEMO_IMPORT_KEY_KEY_HANDLE (0x100000U)

typedef struct {
    const char *desc;
    uint32_t key_privilege;
    bool_t is_plain_key;
    uint8_t key_type;
    uint8_t part_info;
    uint8_t reserved;
    uint32_t key_handle;
    uint16_t pub_key_size;
    uint16_t priv_key_size;
    const uint8_t *key_value;
    uint32_t key_value_size;
    const uint32_t transport_key_handle;
    const uint32_t auth_key_handle;
    const uint8_t *key_signature;
    uint32_t key_signature_size;
} demo_import_key_std_st;

/* clang-format off */

static const uint8_t s_import_key_aes128_plain_key[16] = {
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xA2,0xDA,0x0F,0xC9,0x34,0xC2,0x68,0x21
};

static const uint8_t s_import_key_secp256r1_plain_keypair[64 + 32] = {
    /* 公钥，（32B x || 32B y） */
    0xB6,0xCF,0x8F,0x68,0x44,0xD0,0xBA,0x5F,0x27,0xBE,0xFD,0x4E,0x77,0x1F,0xE3,0xD4,
    0xFA,0x5C,0x9E,0xA8,0xE8,0x73,0xA2,0x1E,0x2E,0xB2,0xFE,0x14,0xAA,0xE1,0x56,0xB4,
    0x92,0x8F,0x8D,0xFA,0x59,0x6A,0xA8,0x00,0xAB,0x0B,0x34,0x08,0x8F,0x9C,0xDC,0x5F,
    0x37,0x5F,0x40,0x64,0x50,0xC4,0x84,0x48,0xBF,0xAE,0x16,0xC9,0x2B,0xA6,0x1F,0xC0,
    /* 私钥，32B */
    0xF2,0x7B,0x7E,0x05,0xF4,0xDD,0xE4,0x0D,0x3B,0x5B,0x2F,0x75,0x0B,0xFD,0x02,0xC5,
    0x6B,0x01,0xA1,0x76,0x3B,0xB9,0x17,0xF5,0x50,0x3B,0x22,0xBA,0x94,0x92,0x63,0xEF
};

static demo_import_key_std_st s_import_key_std_data[4] = {
    {
        .desc = "Import AES128 palin key, eHSM distributes key handle",
        .key_privilege =
            EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_IMPORT_CIPHER
            | EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_REMOVE,
        .is_plain_key = true,
        .key_type = EHSM_KEY_TYPE_AES_128,
        .part_info = (uint8_t)EHSM_KEY_PART_SYMM_KEY,
        .key_handle = 0xFFFFFFFF, /* 表示由 eHSM 密钥管理服务分配 key handle */
        .pub_key_size = 0,
        .priv_key_size = sizeof(s_import_key_aes128_plain_key),
        .key_value = s_import_key_aes128_plain_key,
        .key_value_size = sizeof(s_import_key_aes128_plain_key),
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
        .key_signature = NULL,
        .key_signature_size = 0,
    }, {
        .desc = "Import SECP256R1 palin keypair, eHSM distributes key handle",
        .key_privilege =
            EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_IMPORT_CIPHER
            | EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_REMOVE,
        .is_plain_key = true,
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .part_info = (uint8_t)EHSM_KEY_PART_KEY_PAIR,
        .key_handle = 0xFFFFFFFF, /* 表示由 eHSM 密钥管理服务分配 key handle */
        .pub_key_size = 64,
        .priv_key_size = 32,
        .key_value = s_import_key_secp256r1_plain_keypair,
        .key_value_size = sizeof(s_import_key_secp256r1_plain_keypair),
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
        .key_signature = NULL,
        .key_signature_size = 0,
    }, {
        .desc = "Import SECP256R1 palin public key, eHSM distributes key handle",
        .key_privilege =
            EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_IMPORT_CIPHER
            | EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_REMOVE,
        .is_plain_key = true,
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .part_info = (uint8_t)EHSM_KEY_PART_PRIVATE_KEY,
        .key_handle = 0xFFFFFFFFU, /* 表示由 eHSM 密钥管理服务分配 key handle */
        .pub_key_size = 0,
        .priv_key_size = 32,
        .key_value = &s_import_key_secp256r1_plain_keypair[64],
        .key_value_size = 32,
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
        .key_signature = NULL,
        .key_signature_size = 0,
    }, {
        .desc = "Import SECP256R1 palin private key, eHSM distributes key handle",
        .key_privilege =
            EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_IMPORT_CIPHER
            | EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_REMOVE,
        .is_plain_key = true,
        .key_type = EHSM_KEY_TYPE_ECC_SECP_256R1,
        .part_info = (uint8_t)EHSM_KEY_PART_PUBLIC_KEY,
        .key_handle = 0xFFFFFFFF, /* 表示由 eHSM 密钥管理服务分配 key handle */
        .pub_key_size = 64,
        .priv_key_size = 0,
        .key_value = s_import_key_secp256r1_plain_keypair,
        .key_value_size = 64,
        .transport_key_handle = 0xFFFFFFFF,
        .auth_key_handle = 0xFFFFFFFF,
        .key_signature = NULL,
        .key_signature_size = 0,
    }
};
// clang-format on

/**
 * @brief 导入密钥的演示。
 *
 * @param[in] std_data 导入密钥的标准演示数据，详见 @ref demo_import_key_std_st
 *
 * @note 导入密钥会根据指定的 key_type 导入密钥，导入的密钥数据格式为 @ref ehsm_key_format_st，其中：
 *      - 导入明文密钥，密钥长度必须和算法匹配：
 *          - 导入对称密钥/随机密钥/HMAC 密钥，pub_key_size = 0，pri_key_size = 密钥长度。key_value 为明文密钥。
 *          - 导入非对称密钥：
 *              - 若导入密钥对，pub_key_size = 公钥明文长度，pri_key_size = 私钥明文长度。key_value =（公钥 || 私钥）。
 *              - 若导入公钥，pub_key_size = 公钥明文长度，pri_key_size = 0。key_value = 公钥。
 *              - 若导入私钥，pub_key_size = 0，pri_key_size = 私钥明文长度。key_value = 私钥。
 *      - 导入密文密钥，明文密钥长度必须和算法匹配：
 *          - 需要事先存在一个能够用于传输保护（加密）密钥。由 SoC 使用该传输保护（加密）密钥对待导入的密钥进行加密：
 *              - 加密算法由传输保护（加密）密钥的密钥类型（算法）决定。
 *              - 模式为 CBC，IV 是 16 字节全 0x00， PKCS7 填充。
 *              - 若导入的是对称密钥/随机密钥/HMAC 密钥，需要加密所有密钥，且 pub_key_size = 0，pri_key_size =
 *                  明文密钥长度。key_value = 加密后的密文数据。
 *              - 若导入的是非对称密钥：
 *                  - 若导入密钥对，仅需要加密私钥，且 pub_key_size = 公钥明文长度，pri_key_size = 私钥明文长度。
 *                      key_value =（公钥明文 || 加密后私钥密文）。
 *              - 若导入私钥，pub_key_size = 0，pri_key_size = 私钥明文长度。key_value = 加密后私钥密文。
 *      - 若设置密钥需要验证 mac 签名，则需要输入签名值（CMAC）和长度，且认证密钥需要以及存在于 eHSM
 *          且具有作为传输密钥的权限。使用的是 CMAC，算法由认证密钥的密钥类型（算法）决定。
 *      - 密文导入和验证 MAC 签名需要 eHSM 已经存在一个具有传输权限的密钥：
 *          - 可以是配置了传输权限的 OTP 密钥
 *          - 也可以是某个 OTP 密钥具有派生传输密钥的权限，并用该 OTP 密钥派生出一个具有传输权限的密钥
 *              若没有该密钥，则不支持密文导出功能。
 *
 */
static void demo_import_key(const demo_import_key_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(0); /* 输入密钥数据的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t key_data_size;                              /* 用于指定密钥数据的字节长度 */
    uint32_t key_handle = std_data->key_handle; /* 用于指定密钥句柄，若值为 0xFFFFFFFF 表示由 eHSM 分配 key handle */
    bool_t need_remove = false;                 /* 用于标记密钥是否需要移除 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("Importing for %s, key type is 0x%08x \r\n", std_data->desc, std_data->key_type);

    /* 0. 根据 @ref ehsm_key_format_st 构造密钥数据 */
    key_data->privilege = std_data->key_privilege;     /* 给导入的密钥设置权限 */
    key_data->key_type = std_data->key_type;           /* 给导入的密钥设置密钥类型（算法） */
    key_data->part_info = std_data->part_info;         /* 给导入的密钥设置部件信息 */
    key_data->pub_key_size = std_data->pub_key_size;   /* 给导入的密钥设置公钥明文长度 */
    key_data->priv_key_size = std_data->priv_key_size; /* 给导入的密钥设置私钥明文长度 */

    memcpy(key_data->key_value, std_data->key_value, std_data->key_value_size); /* 拷贝 key_value 到共享内存 */
    key_data_size = sizeof(ehsm_key_format_st) + std_data->key_value_size;      /* 计算密钥数据的总长度 */

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用导入密钥的 API。 */
    ret = ehsm_km_import_key(
        ctx, std_data->transport_key_handle, std_data->auth_key_handle, key_data, key_data_size, NULL, 0, &key_handle);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        need_remove = true; /* 标记导入的密钥需要移除 */

        if (0xFFFFFFFF == std_data->key_handle) {
            /* 3. 若指定由 eHSM 分配，打印获取到的 key_handle */
            ehsm_port_printf("The key handle is: 0x%08x \r\n", key_handle);
        } else {
            /* 3. 若指定有效 key handle，则检查返回的 key handle 是否和指定的值一致 */
            ret = demo_check_val("Comparison for key handle:", std_data->key_handle, key_handle);
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
 * @brief 导入密钥的演示入口函数。
 */
void ehsm_demo_import_key_entry(void)
{
    uint32_t i;
    demo_import_key_std_st *std_data = s_import_key_std_data;
    uint32_t cnt = sizeof(s_import_key_std_data) / sizeof(demo_import_key_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for importing key starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_import_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for importing key ends. ====================\r\n\r\n");
}

