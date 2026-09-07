// #% #if CONFIG_BL_HOST_INSTALL_OTP_KEY_EN
#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/bl_api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_install_otp_key.h"

#define DEMO_OTP_KEY_BASE_OFFSET      (0x118U)
#define DEMO_OTP_KEY_DATA_SIZE        (40U)
#define DEMO_ENC_OTP_KEY_HANDLE       (0x100000U)
#define DEMO_OTP_HW_CTRL_FIELD_OFFSET (0x18U)
#define DEMO_OTP_KEY_ALG_SEL          (3U << 12)
#define DEMO_OTP_KEY_ALG_AES128       (1U << 12)

typedef struct {
    const char *desc;
    ehsm_key_level_e key_level;
    ehsm_install_key_type_e key_type;
    uint16_t key_slot_id;
    bool_t is_last_key;
} demo_rand_otp_key_st;

typedef struct {
    const char *desc;
    ehsm_key_level_e key_level;
    ehsm_install_key_type_e key_type;
    uint16_t key_slot_id;
    bool_t is_last_key;
    const uint8_t *kek_key;
    uint16_t kek_key_size;
    const uint8_t *plain_key_data;
    uint32_t plain_key_data_size;
} demo_enc_otp_key_st;

// clang-format off
static demo_rand_otp_key_st s_rand_otp_key_std_data[2] = {
    {
        .desc = "DEVICE_ROOT_KEY",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_INSTALL_KEY_TYPE_SYMM,
        .key_slot_id = 1U,
        .is_last_key = false
    }, {
        .desc = "EHSM_DEBUG_KEY",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH,
        .key_slot_id = 3U,
        .is_last_key = false
    }
};
static const uint8_t s_ehsm_kek_key[16] = {
    0x4E, 0x20, 0xAC, 0x86, 0x59, 0x31, 0xF7, 0xDB, 0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
};

// static const uint8_t s_soc_kek_key[16] = {
//     0xEF, 0x9B, 0x57, 0x13, 0xCE, 0x8A, 0x46, 0x02, 0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
// };

/**
 * 以下是 SM2 公钥的明文，供外部工具验证。
 * static uint8_t s_sm2_ehsm_debug_pubkey[65] = {
 *     0x04, // 未压缩符号
 *     0x17,0x6F,0x96,0x9D,0x59,0x33,0xD5,0x3E,0xE5,0x72,0x1B,0x56,0x55,0x30,0x0D,0xD2,
 *     0x15,0x93,0x10,0x76,0xF0,0xC9,0xA8,0x00,0xE1,0xCF,0x91,0x57,0xF1,0x20,0x50,0xEF,
 *     0xF5,0x59,0x00,0xD0,0xED,0xF7,0xAC,0xAE,0x22,0xB1,0x00,0xDB,0x1D,0xC6,0x24,0x2E,
 *     0x74,0xED,0x02,0xBE,0xCC,0x47,0xC2,0x07,0x93,0x5F,0x8B,0x46,0x61,0x57,0xE0,0xFA
 * };
 */
static uint8_t s_sm2_ehsm_debug_key_plain_key_data[48] = {
    /* 32B pubkey hash */
    0xD4,0xF0,0xC2,0x29,0xE1,0x09,0x72,0x63,0x70,0x47,0x7D,0x84,0x64,0xFE,0x7C,0x72,
    0x41,0x57,0x5F,0xD7,0x32,0x1A,0xDB,0x95,0x60,0x10,0x8E,0xFF,0x3A,0x05,0xA8,0x7E,
    /* 4B CRC32 值 */
    0x21,0xDE,0x00, 0x5C,
    /* 12B 填充 0x00 */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

};

static demo_enc_otp_key_st s_enc_otp_key_std_data[1] = {
    {
        .desc = "EHSM_DEBUG_KEY",
        .key_level = EHSM_KEY_LEVEL_1,
        .key_type = EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH,
        .key_slot_id = 3U,
        .is_last_key = false,
        .kek_key = s_ehsm_kek_key,
        .kek_key_size = sizeof(s_ehsm_kek_key),
        .plain_key_data = s_sm2_ehsm_debug_key_plain_key_data,
        .plain_key_data_size = sizeof(s_sm2_ehsm_debug_key_plain_key_data)
    }
};
// clang-format on

/**
 * @brief 由 eHSM 生成并安装随机密钥到 OTP 的演示。
 * @param[in] std_data 安装随机密钥的标准数据指针，详见 @ref demo_rand_otp_key_st。
 *
 * @note
 * - 若安装的密钥是 DEVICE_ROOT_KEY 或密钥等级为一级，则需要使用 CHIP_ROOT_KEY 加密，请确保 OTP中已经存在有效的
 * CHIP_ROOT_KEY。
 * - 若安装的密钥是二级密钥，则需要使用 DEVICE_ROOT_KEY 加密，请确保 OTP中已经存在有效的 DEVICE_ROOT_KEY。
 * - 写 OTP 密钥需要 reset eHSM，才能使得 OTP 密钥被解密并更新到 KMU 中，因此安装完密钥后需要 reset eHSM。
 *
 */
static void install_rand_otp_key(const demo_rand_otp_key_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();          /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t backup_key_data[DEMO_OTP_KEY_DATA_SIZE]; /* 用于备份 slot_id 对应的 OTP 密钥数据 (含密钥属性和 CRC32 值) */
    uint8_t cur_key_data[DEMO_OTP_KEY_DATA_SIZE];    /* 用于读取当前的 OTP 密钥数据（含密钥属性和 CRC32 值） */
    uint32_t data_size = sizeof(backup_key_data);    /* OTP 密钥数据（含密钥属性和 CRC32 值）的字节长度 */
    uint32_t offset;                                 /* 用于计算密钥数据安装的 offset */

    ehsm_port_printf("[Demo of installing rand OTP key starts.] \r\n");
    ehsm_port_printf("The OTP key is %s .\r\n", std_data->desc);

    /* 备份 slot_id 对应的 OTP 密钥（含密钥属性和 CRC32 值） */
    offset = DEMO_OTP_KEY_BASE_OFFSET + (std_data->key_slot_id * data_size);
    demo_read_otp_from_soc_addr(offset, backup_key_data, data_size);

    /* 打印备份的 OTP 密钥数据 */
    print_hex("The backup 40B OTP key data before installed is: \r\n    ", backup_key_data, data_size);

    /* 0. 需要确保 slot id 对应的 OTP 密钥空间没有被写过，即保持为默认值，否则会报错 */
    memset(cur_key_data, 0x0U, data_size);
    demo_write_otp_from_soc_addr(offset, cur_key_data, data_size); /* 恢复 slot id 对应的 OTP 空间为默认值 */

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 调用由 eHSM 生成并安装随机的 API，eHSM 生成随机密钥后会将该密钥数据（含密钥属性和 CRC32 值）写入 slot_id
     * 对应的 OTP 密钥位置。 写 OTP 密钥需要 reset eHSM，才能使得 OTP 密钥被解密并更新到 KMU 中，因此安装完密钥后需要
     * reset eHSM。这里仅演示 OTP 密钥安装，不涉及 reset eHSM。
     */
    ret = ehsm_install_random_key(
        ctx, std_data->key_level, std_data->key_type, std_data->key_slot_id, std_data->is_last_key);
    ret = demo_check_val("The execution of installing rand OTP key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，读取更改后的 OTP 密钥 */
        demo_read_otp_from_soc_addr(offset, cur_key_data, data_size);

        /* 打印安装后的 OTP 密钥数据 */
        print_hex("The newer 40B OTP key data after installed is: \r\n    ", cur_key_data, data_size);
    }

    /* 通过 SoC 映射地址写 OTP 的方式恢复备份的 OTP 密钥数据（含密钥属性和 CRC32 值） */
    demo_write_otp_from_soc_addr(offset, backup_key_data, data_size);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of installing rand OTP key ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of installing rand OTP key ends with failure. !!!] \r\n\r\n");
    }
}

static uint32_t get_enc_key_data(ehsm_ctx_st *ctx, const uint8_t *kek_key, uint16_t kek_key_size,
    const uint8_t *plain_data, uint32_t plain_data_size, uint8_t *enc_key_data, uint32_t *enc_key_data_size)
{
    uint32_t ret;
    ehsm_key_format_st *key_data
        = (ehsm_key_format_st *)ehsm_demo_get_buffer(10); /* 密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t key_data_size;                               /* 密钥数据 buffer 的长度 */
    uint32_t key_handle = DEMO_ENC_OTP_KEY_HANDLE;        /* 用于指定导入密钥的 key handle */
    uint32_t key_alg_sel;                                 /* OTP 的 HW 控制字段中用于解密 OTP 密钥的算法配置 */
    ehsm_key_type_e key_type;                             /* 根据 OTP 密钥解密算法获取的密钥类型 */
    ehsm_symm_algo_e algo;                                /* 根据 OTP 密钥解密算法获取的加密算法 */
    uint8_t *iv = ehsm_demo_get_buffer(11);               /* 加密的 IV buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t iv_size = 16U;                               /* IV 的字节长度，固定为 16 */
    uint8_t *input = ehsm_demo_get_buffer(12);            /* 用于加密的输入 buffer，必须是 SoC 与 eHSM 的共享内存 */

    /* 获取当前 OTP HW 控制字段中 OTP 密钥的解密算法 */
    demo_read_otp_from_soc_addr(DEMO_OTP_HW_CTRL_FIELD_OFFSET, (uint8_t *)&key_alg_sel, sizeof(uint32_t));

    /* 根据 OTP 密钥的解密算法获取加密的密钥类型，加密的算法 */
    if (DEMO_OTP_KEY_ALG_AES128 == (key_alg_sel & DEMO_OTP_KEY_ALG_SEL)) {
        key_type = EHSM_KEY_TYPE_AES_128;
        algo = EHSM_SYMM_ALGO_AES_128;
    } else {
        key_type = EHSM_KEY_TYPE_SM4;
        algo = EHSM_SYMM_ALGO_SM4;
    }

    /**
     * 导入明文密钥并获取 key handle，若 eHSM RAM 已经存在密钥，则可以省略这一步直接使用对应的 key handle。
     * 这里主要演示计算过程，若想了解密钥导入的具体流程，请参考密钥导入 demo。
     */
    key_data_size = sizeof(ehsm_key_format_st) + kek_key_size;
    demo_wrap_key_data((EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT), key_type, EHSM_KEY_PART_SYMM_KEY, kek_key,
        kek_key_size, 0U, kek_key_size, key_data, &key_data_size);
    ret = ehsm_km_import_key(
        ctx, EHSM_KEY_HANDLE_INVALID, EHSM_KEY_HANDLE_INVALID, key_data, key_data_size, NULL, 0U, &key_handle);
    ret = demo_check_val("The execution of importing key:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* 加密使用的 IV 为 16 字节的 0x00 */
        (void)memset(iv, 0x0U, iv_size);

        /* 将待加密的明文数据拷贝至 SoC 与 eHSM 的共享内存 */
        (void)memcpy(input, plain_data, plain_data_size);

        /* 使用 RTL key 加密明文密钥或 hash 值，密码模式为 CBC， 无需 padding */
        ret = ehsm_symm_cipher_onepass(ctx, algo, EHSM_CIPHER_MODE_CBC, EHSM_PADDING_NONE, key_handle, true, iv,
            iv_size, input, plain_data_size, enc_key_data, enc_key_data_size);
        ret = demo_check_val("The execution of encrypting plain key data:", EHSM_OK, ret);
    }

    return ret;
}

/**
 * @brief 安装加密后的密文密钥到 OTP 的演示。
 * @param[in] std_data 安装密文密钥的标准数据指针，详见 @ref demo_enc_otp_key_st。
 *
 * @note
 * - 该功能需要提前使用 RTL 密钥加密并生成对应的数据：
 *      - 对明文密钥进行加密，若已经加密，可以忽略这一步骤。
 *      - 待安装的明文 OTP 密钥：
 *           - 若为一级密钥用 eHSM RTL key 加密；
 *           - 若为二级密钥用 SoC RTL key 加密；
 *      - 加密的算法为 OTP HW 控制字段的 KeyAlgSel 所指定的算法（AES 或 SM4），密码模式为 CBC 模式，IV 是 16 字节全
 *          0x00，待加密的明文数据为 48 字节的 （32B plain_key(or hash) || 4B CRC32 值 || 12B 0x0 填充）拼接起来。
 *      - 加密后的密文长度也为 48 字节。
 * - 调用安装密文 OTP 密钥的 API，eHSM 首先解密传入 48B 的密文数据,然后：
 *      - 再次加密：
 *          - 根据密钥等级：
 *              - 若安装的是一级密钥，使用对应的 CHIP_ROOT_KEY 加密；
 *              - 若安装的是二级密钥，使用对应的 DEVICE_ROOT_KEY 加密；
 *          - 加密的算法为 OTP HW 控制字段的 KeyAlgSel 所指定的算法（AES 或 SM4），密码模式为 ECB
 *              模式，再次加密时的明文 数据为 32 字节的 plain key(or hash)；
 *          - 对再次加密的密文数据计算 CRC32 值，及 CRC32_VAL = CRC32(再次加密 plain key 的密文)；
 *          - 最后写入 OTP 的 40 字节数据为 （4B 密钥属性|| 32B 再次加密的 plain key 的密文 || 4B CRC32 值）；
 *              写入 OTP 的密钥数据只有在 reset eHSM 后才会被重新解密并更新到 KMU，因此安装完密钥后需要 reset
 *              eHSM。这里仅演示 OTP 密钥安装，不涉及 reset eHSM。
 *      - 总结起来就是写入 OTP 的数据为 40 字节的下列数据拼接：
 *           (4B 密钥属性 || 32B RE_ENC(ROOT_KEY, DEC(RTL_KEY, 48B enc_key_data)) || 4B CRC32(32B RE_ENC 数据)。
 */
static void install_enc_otp_key(const demo_enc_otp_key_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();           /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *enc_key_data = ehsm_demo_get_buffer(0);  /* 用于存储密文密钥数据的 buffer，必须是 SoC 与 eHSM 的共享内存 */
    uint32_t enc_key_data_size;                       /* 密文密钥的字节长度 */
    uint8_t backup_otp_data[DEMO_OTP_KEY_DATA_SIZE];  /* 用于备份 slot_id 对应的 OTP 密钥数据 (含密钥属性和 CRC32 值) */
    uint8_t cur_otp_data[DEMO_OTP_KEY_DATA_SIZE];     /* 用于读取当前的 OTP 密钥数据（含密钥属性和 CRC32 值） */
    uint32_t otp_data_size = sizeof(backup_otp_data); /* OTP 密钥数据（含密钥属性和 CRC32 值）的字节长度 */
    uint32_t otp_offset;                              /* 用于计算密钥数据安装相对于 OTP 起始地址的 offset */

    ehsm_port_printf("[Demo of installing encrypted OTP key starts.] \r\n");
    ehsm_port_printf("The OTP key is %s .\r\n", std_data->desc);

    /* 备份 slot_id 对应的 OTP 密钥（含密钥属性和 CRC32 值） */
    otp_offset = DEMO_OTP_KEY_BASE_OFFSET + (std_data->key_slot_id * otp_data_size);
    demo_read_otp_from_soc_addr(otp_offset, backup_otp_data, otp_data_size);

    /* 打印备份的 OTP 密钥数据 */
    print_hex("The backup 40B OTP key data before installed is: \r\n    ", backup_otp_data, otp_data_size);

    /* 0. 需要确保 slot id 对应的 OTP 密钥空间没有被写过，即保持为默认值，否则会报错 */
    memset(cur_otp_data, 0x0U, otp_data_size);
    demo_write_otp_from_soc_addr(otp_offset, cur_otp_data, otp_data_size); /* 恢复 slot id 对应的 OTP 空间为默认值 */

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /**
     * 2. 对明文密钥进行加密，若已经加密，可以忽略这一步骤。
     * - 待安装的 OTP 密钥：
     *      - 若为一级密钥用 eHSM RTL key 加密；
     *      - 若为二级密钥用 SoC RTL key 加密；
     * - 加密的算法为 OTP HW 控制字段的 KeyAlgSel 所指定的算法（AES 或 SM4），密码模式为 CBC 模式，待加密的明文数据为
     * 48 字节的 （32B plain_key(or hash) || 4B CRC32 值 || 12B 0x0 填充）拼接起来。
     * - 加密后的密文长度也为 48 字节
     */
    enc_key_data_size = std_data->plain_key_data_size;
    ret = get_enc_key_data(ctx, std_data->kek_key, std_data->kek_key_size, std_data->plain_key_data,
        std_data->plain_key_data_size, enc_key_data, &enc_key_data_size);
    ret = demo_check_val("The execution of importing key API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /**
         * 3. 调用安装密文 OTP 密钥的 API，eHSM 首先解密传入的密文数据,然后：
         * - 再次加密：
         *      - 根据密钥等级：
         *          - 若安装的是一级密钥，使用对应的 CHIP_ROOT_KEY 加密；
         *          - 若安装的是二级密钥，使用对应的 DEVICE_ROOT_KEY 加密；
         *      - 加密的算法为 OTP HW 控制字段的 KeyAlgSel 所指定的算法（AES 或 SM4），密码模式为 ECB
         *          模式，再次加密时的明文 数据为 32 字节的 plain key(or hash)；
         *      - 对再次加密的密文数据计算 CRC32 值，及 CRC32_VAL = CRC32(再次加密 plain key 的密文)；
         * - 最后写入 OTP 的 40 字节数据为 （4B 密钥属性|| 32B 再次加密的 plain key 的密文 || 4B CRC32 值）；
         * 写入 OTP 的密钥数据只有在 reset eHSM 后才会被重新解密并更新到 KMU，因此安装完密钥后需要 reset
         * eHSM。这里仅演示 OTP 密钥安装，不涉及 reset eHSM。
         * - 总结起来就是写入 OTP 的数据为 40 字节的下列数据拼接：
         *      (4B 密钥属性 || 32B RE_ENC(ROOT_KEY, DEC(RTL_KEY, 48B enc_key_data)) || 4B CRC32(32B RE_ENC 数据)。
         */
        ret = ehsm_install_encrypted_key(ctx, std_data->key_level, std_data->key_type, std_data->key_slot_id,
            std_data->is_last_key, enc_key_data, enc_key_data_size);
        ret = demo_check_val("The execution of installing encrypted OTP key API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，读取更改后的 OTP 密钥 */
        demo_read_otp_from_soc_addr(otp_offset, cur_otp_data, otp_data_size);

        /* 打印安装后的 OTP 密钥数据 */
        print_hex("The newer 40B OTP key data after installed is: \r\n    ", cur_otp_data, otp_data_size);
    }

    /* 通过 SoC 映射地址写 OTP 的方式恢复备份的 OTP 密钥数据（含密钥属性和 CRC32 值） */
    demo_write_otp_from_soc_addr(otp_offset, backup_otp_data, otp_data_size);

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo of installing encrypted OTP key ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo of installing encrypted OTP key ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 由 eHSM 生成随机密钥并写入 OTP 的演示入口函数。
 */
void install_rand_otp_key_entry(void)
{
    uint32_t i;
    demo_rand_otp_key_st *std_data = s_rand_otp_key_std_data;
    uint32_t cnt = sizeof(s_rand_otp_key_std_data) / sizeof(demo_rand_otp_key_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for installing rand OTP key starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("std_data[%d].\r\n", i);
        install_rand_otp_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for installing rand OTP key ends. ==================== "
                     "\r\n\r\n");
}

/**
 * @brief 安装外部密文密钥到 OTP 的演示入口函数。
 */
void install_enc_otp_key_entry(void)
{
    uint32_t i;
    demo_enc_otp_key_st *std_data = s_enc_otp_key_std_data;
    uint32_t cnt = sizeof(s_enc_otp_key_std_data) / sizeof(demo_enc_otp_key_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for installing encrypted OTP key starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("std_data[%d].\r\n", i);
        install_enc_otp_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for installing encrypted OTP key ends. ==================== "
                     "\r\n\r\n");
}

/**
 * @brief 安装 OTP 密钥的演示入口函数。
 */
void demo_install_otp_key_entry(void)
{
    /* 由 eHSM 生成随机密钥并安装到 OTP */
    install_rand_otp_key_entry();

    /* 安装外部密文密钥到 OTP */
    install_enc_otp_key_entry();
}

// #% #endif /* 0x1 */
