#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/bl_api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_bl_demo_gen_otp_key.h"

#define DEMO_BL_GEN_OTP_KEY_BASE_OFFSET      (0x70U)
#define DEMO_BL_GEN_OTP_KEY_DATA_SIZE        (40U)
#define DEMO_ENC_BL_GEN_OTP_KEY_HANDLE       (0x100000U)
#define DEMO_BL_GEN_OTP_HW_CTRL_FIELD_OFFSET (0x18U)
#define DEMO_BL_GEN_OTP_KEY_ALG_SEL          (3U << 12)
#define DEMO_BL_GEN_OTP_KEY_ALG_AES128       (1U << 12)

typedef struct {
    const char *desc;
    ehsm_key_level_e key_level;
    ehsm_bl_gen_key_type_e key_type;
} bl_demo_rand_otp_key_st;

typedef struct {
    const char *desc;
    ehsm_key_level_e key_level;
    ehsm_bl_gen_key_type_e key_type;
    const uint8_t *otp_key_data;
    uint32_t otp_key_data_size;
} bl_demo_enc_otp_key_st;

// clang-format off
static bl_demo_rand_otp_key_st s_bl_rand_otp_key_std_data[2] = {
    {
        .desc = "DEVICE_ROOT_KEY",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_BL_GEN_KEY_TYPE_SYMM,
    }, {
        .desc = "EHSM_DEBUG_KEY",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_BL_GEN_KEY_TYPE_SM2,
    }
};

// static const uint8_t s_bl_ehsm_kek_key[16] = {
//     0x4E, 0x20, 0xAC, 0x86, 0x59, 0x31, 0xF7, 0xDB, 0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
// };

// static const uint8_t s_bl_soc_kek_key[16] = {
//     0xEF, 0x9B, 0x57, 0x13, 0xCE, 0x8A, 0x46, 0x02, 0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
// };

/**
 * 以下是 SM2 公钥的明文以及 OTP 明文数据，供外部工具验证。
 * static uint8_t s_bl_sm2_ehsm_debug_pubkey[65] = {
 *     0x04, // 未压缩符号
 *     0x17,0x6F,0x96,0x9D,0x59,0x33,0xD5,0x3E,0xE5,0x72,0x1B,0x56,0x55,0x30,0x0D,0xD2,
 *     0x15,0x93,0x10,0x76,0xF0,0xC9,0xA8,0x00,0xE1,0xCF,0x91,0x57,0xF1,0x20,0x50,0xEF,
 *     0xF5,0x59,0x00,0xD0,0xED,0xF7,0xAC,0xAE,0x22,0xB1,0x00,0xDB,0x1D,0xC6,0x24,0x2E,
 *     0x74,0xED,0x02,0xBE,0xCC,0x47,0xC2,0x07,0x93,0x5F,0x8B,0x46,0x61,0x57,0xE0,0xFA
 * };
 *
 * static uint8_t s_bl_sm2_ehsm_debug_key_plain_key_data[48] = {
 *     // 32B pubkey hash
 *     0xD4,0xF0,0xC2,0x29,0xE1,0x09,0x72,0x63,0x70,0x47,0x7D,0x84,0x64,0xFE,0x7C,0x72,
 *     0x41,0x57,0x5F,0xD7,0x32,0x1A,0xDB,0x95,0x60,0x10,0x8E,0xFF,0x3A,0x05,0xA8,0x7E,
 *     // 4B CRC32 值
 *     0x21,0xDE,0x00, 0x5C,
 *     // 12B 填充 0x00
 *     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
 * };
 */

/* 加密密钥为 s_bl_ehsm_kek_key，算法为OTP KeySelAlg，CBC 模式，IV 为 16字节 0x00，加密 s_bl_sm2_ehsm_debug_key_plain_key_data */
static const uint8_t s_bl_sm2_ehsm_debug_key_cipher_key_data[48] = {
    0xBA,0xA1,0x39,0xAF,0x74,0xDE,0xD3,0xCC,0xAC,0xEE,0x99,0xB8,0x01,0x57,0x32,0xDE,
    0xD1,0x9C,0x5B,0x18,0x96,0x3D,0x52,0xB8,0x67,0xB2,0xC7,0x28,0xDA,0x65,0x1C,0xE5,
    0x63,0x13,0x53,0x10,0xB5,0x7A,0x8C,0x4D,0xC1,0xB3,0x50,0xDB,0xEF,0x92,0x1C,0xE4
};

static bl_demo_enc_otp_key_st s_bl_enc_otp_key_std_data[1] = {
    {
        .desc = "EHSM_DEBUG_KEY with SM2",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_BL_GEN_KEY_TYPE_SM2,
        .otp_key_data = s_bl_sm2_ehsm_debug_key_cipher_key_data,
        .otp_key_data_size = sizeof(s_bl_sm2_ehsm_debug_key_cipher_key_data)
    }
};
// clang-format on

/**
 * @brief 由 eHSM 生成随机 OTP 密钥并加密后将密文数据输出到 Host 的演示。
 * @param[in] std_data 获取随机密钥的标准数据指针，详见 @ref bl_demo_rand_otp_key_st。
 *
 * @note
 * - 对于获取随机 OTP 密钥：
 *      - 若获取的密钥等级为一级，则需要使用 CHIP_ROOT_KEY 加密，请确保 OTP 中已经存在有效的
 *          CHIP_ROOT_KEY。
 *      - 若获取的密钥是二级密钥，则需要使用 DEVICE_ROOT_KEY 加密，请确保 OTP中已经存在有效的 DEVICE_ROOT_KEY。
 *      - 若获取的密钥等级为 0xFF，则表示是 DEVICE_ROOT_KEY，由 CHIP_ROOT_KEY 加密，请确保 OTP 中已经存在有效的
 *          CHIP_ROOT_KEY。
 *      - 获取到的密钥会由 eHSM 根据 OTP HW 控制字段的 KeyAlgSel 所配置的算法（AES128/SM4），使用 CBC 模式，16字节全
 *          0x00 的 IV 加密 32 字节的明文密钥数据（不足部分会填充 0x00），并对密文计算 CRC32 值后输出 36 字节数据：
 *          (32B 密文密钥 || 4B 对密文密钥计算的 CRC32 值)。
 * - 在解密/加密的过程中，涉及到的密钥需要事先存在于 eHSM。
 * - 若需要安装到 OTP，需要调用 OTP 写的 API，可以将上述数据写入 OTP 对应位置，然后重启 eHSM 生效。
 */
static void bl_get_rand_otp_key(const bl_demo_rand_otp_key_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();     /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *key_out = ehsm_demo_get_buffer(0); /* 用于存储密文密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */

    ehsm_port_printf("[Demo of getting rand OTP key starts.] \r\n");
    ehsm_port_printf("The OTP key is %s. \r\n", std_data->desc);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用由 eHSM 生成并安装随机的 API，eHSM 生成随机密钥后会将该密钥数据（含密钥属性和 CRC32 值）写入 slot_id
     * 对应的 OTP 密钥位置。 写 OTP 密钥需要 reset eHSM，才能使得 OTP 密钥被解密并更新到 KMU 中，因此安装完密钥后需要
     * reset eHSM。这里仅演示 OTP 密钥安装，不涉及 reset eHSM。
     */
    ret = ehsm_bl_get_random_key(ctx, std_data->key_level, std_data->key_type, key_out);
    ret = demo_check_val("The execution of getting rand OTP key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 打印获取的 OTP 密钥数据 */
        print_hex("The newer 36B OTP key data after getting is: \r\n    ", key_out, 36U);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of getting rand OTP key ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of getting rand OTP key ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 由 eHSM 解密外部密文密钥并生成 OTP 密文数据后输出到 Host 的演示。
 * @param[in] std_data 获取密钥的标准数据指针，详见 @ref bl_demo_enc_otp_key_st。
 *
 * @note
 * - 对于被 KEK 加密的密文密钥，eHSM 会先根据密钥等级，使用对应的 KEK 解密，获得明文，并比较 CRC32
 * 值，之后再根据密钥等级 使用 CHIP_ROOT_KEY 或 DEVICE_ROOT_KEY 加密，生成 OTP 迷们密钥，比返回给 Host
 * - 首先需要 Host 通过 eHSM_KEK_KEY（加密一级密钥） 或 SoC_KEK_KEY（加密二级密钥） 加密
 *      （32B 明文 OTP 密钥 || 4B CRC32(32B 明文 OTP 密钥) || 12B 0x00），加密使用 OTP
 *      硬件控制字段的 KeyAlgSel 算法（AES128 或 SM4），CBC 模式，16 字节全 0x00 的 IV。
 * - eHSM 对被 KEK 密钥加密的 48 字节数据进行解密，解密后，计算 CRC32 值并比对，比对成功，
 *      eHSM 根据密钥等级使用 CHIP_ROOT_KEY（加密一级密钥） 或 DEVICE_ROOT_KEY（加密二级密钥）
 *      加密 32 字节 OTP 密钥，并对密文计算 CRC32(32B OTP 密钥密文)，最后将 36B 数据
 *      （32B OTP 密钥密文 || 4B CRC32(32B OTP 密钥密文)）返回给 Host。
 * - 在解密/加密的过程中，涉及到的密钥需要事先存在于 eHSM。
 * - 若 Host 需要的话，调用 OTP 写可以将 36B 的数据写入 OTP 对应位置。
 */
static void bl_enc_otp_key(const bl_demo_enc_otp_key_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx(); /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *enc_key_data
        = ehsm_demo_get_buffer(0); /* 用于存储 KEK 加密的密文密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t enc_key_data_size;    /* 密文密钥的字节长度 */
    uint8_t *returned_key_data
        = ehsm_demo_get_buffer(2); /* 用于存储返回的 OTP 密文密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */

    ehsm_port_printf("[Demo of encrypting OTP key starts.] \r\n");
    ehsm_port_printf("The OTP key is %s .\r\n", std_data->desc);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用安装密文 OTP 密钥的 API，输入 KEK 密钥加密的 48 字节数据。
     */
    enc_key_data_size = std_data->otp_key_data_size;
    memcpy(enc_key_data, std_data->otp_key_data, enc_key_data_size);
    ret = ehsm_bl_encrypt_key(ctx, std_data->key_level, enc_key_data, enc_key_data_size, returned_key_data);
    ret = demo_check_val("The execution of encrypting OTP key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 打印返回的 36 字节 OTP 密钥数据 */
        print_hex("The newer 36B OTP key data is: \r\n    ", returned_key_data, 36);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of encrypting OTP key ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of encrypting OTP key ends with failure. !!!] \r\n\r\n");
    }
}
/**
 * @brief 由 eHSM 生成随机 OTP 密钥密文数据并返回 Host 的演示入口函数。
 */
void bl_get_rand_otp_key_entry(void)
{
    uint32_t i;
    bl_demo_rand_otp_key_st *std_data = s_bl_rand_otp_key_std_data;
    uint32_t cnt = sizeof(s_bl_rand_otp_key_std_data) / sizeof(bl_demo_rand_otp_key_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for getting rand OTP key starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("std_data[%d].\r\n", i);
        bl_get_rand_otp_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for getting rand OTP key ends. ==================== "
                     "\r\n\r\n");
}

/**
 * @brief 由 eHSM 解密外部密文密钥并生成 OTP 密文数据后输出到 Host 的演示入口函数。
 */
void bl_enc_otp_key_entry(void)
{
    uint32_t i;
    bl_demo_enc_otp_key_st *std_data = s_bl_enc_otp_key_std_data;
    uint32_t cnt = sizeof(s_bl_enc_otp_key_std_data) / sizeof(bl_demo_enc_otp_key_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for encrypting OTP key starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("std_data[%d].\r\n", i);
        bl_enc_otp_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for encrypting OTP key ends. ==================== "
                     "\r\n\r\n");
}

/**
 * @brief 生成 OTP 密钥的演示入口函数。
 */
void ehsm_bl_demo_gen_otp_key_entry(void)
{
    /* 由 eHSM 生成 OTP 随机密钥加密后输出到 Host */
    bl_get_rand_otp_key_entry();

    /* 由 eHSM 解密外部密文密钥并生成 OTP 密文数据后输出到 Host */
    bl_enc_otp_key_entry();
}

