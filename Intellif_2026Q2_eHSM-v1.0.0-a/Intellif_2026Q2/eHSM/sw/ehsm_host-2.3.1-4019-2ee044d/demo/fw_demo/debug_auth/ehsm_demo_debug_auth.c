#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_debug_auth.h"
#include "common/crypto/SM2.h"
#include "common/crypto/SHA224_SHA256.h"
#include "common/crypto/ECDSA_ECDH.h"
#include "common/crypto/AES.h"
#include "common/crypto/SM4.h"

#define DEMO_DEBUG_AUTH_SIGN_KEY_HANDLE (0x100000U)

typedef struct {
    const char *desc;
    ehsm_challenge_type_e type;
    ehsm_auth_algo_e algo;
    uint32_t challenge_size;
    ehsm_key_type_e key_type; /* 用于指定计算签名的密钥的类型 */
    ehsm_key_part_e key_part; /* 用于指定计算签名的密钥的部件 */
    const uint8_t *key;
    uint32_t key_size;
    uint32_t pubkey_size;
    uint32_t prikey_size;
    const ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap;
    uint32_t soc_dbg_bitmap_size;
} demo_debug_auth_std_st;

/* clang-format off */
static const uint8_t s_debug_auth_symm_key[16] = {
    0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99
};

/**
 * SM3-SM2 签名使用的 ID，用于外部工具验证。
 * static const uint8_t s_debug_auth_sm3_sm2_id[16] = {
 *      0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38
 * };
 *
 * SM3-SM2 签名使用的公钥的 SM3 hash 值，用于外部工具验证。
 * static const uint8_t s_debug_auth_sm3_sm2_pubkey_hash[32] = {
 *      0xD4,0xF0,0xC2,0x29,0xE1,0x09,0x72,0x63,0x70,0x47,0x7D,0x84,0x64,0xFE,0x7C,0x72,
 *      0x41,0x57,0x5F,0xD7,0x32,0x1A,0xDB,0x95,0x60,0x10,0x8E,0xFF,0x3A,0x05,0xA8,0x7E
 * };
 *
 */
static const uint8_t s_debug_auth_sm3_sm2_key[65 + 32] = {
    /* 公钥 */
    0x04, /* 未压缩符号 */
    0x17, 0x6F, 0x96, 0x9D, 0x59, 0x33, 0xD5, 0x3E, 0xE5, 0x72, 0x1B, 0x56, 0x55, 0x30, 0x0D, 0xD2, 0x15, 0x93, 0x10,
    0x76, 0xF0, 0xC9, 0xA8, 0x00, 0xE1, 0xCF, 0x91, 0x57, 0xF1, 0x20, 0x50, 0xEF, 0xF5, 0x59, 0x00, 0xD0, 0xED, 0xF7,
    0xAC, 0xAE, 0x22, 0xB1, 0x00, 0xDB, 0x1D, 0xC6, 0x24, 0x2E, 0x74, 0xED, 0x02, 0xBE, 0xCC, 0x47, 0xC2, 0x07, 0x93,
    0x5F, 0x8B, 0x46, 0x61, 0x57, 0xE0, 0xFA,
    /* 私钥 */
    0x0C, 0xF7, 0x22, 0x0C, 0xF3, 0x1A, 0x8E, 0xB7, 0x09, 0xBA, 0xAA, 0xA4, 0xF5, 0xD0, 0x2E, 0x4C, 0x01, 0x47, 0x05,
    0x2C, 0x34, 0xF6, 0x4A, 0xBD, 0xBA, 0xC0, 0x88, 0x86, 0x46, 0xC7, 0x4F, 0x2B
};

static const ehsm_soc_dbg_bitmap_st s_debug_auth_soc_dbg_bitmap = {
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x1}
};

static demo_debug_auth_std_st s_debug_auth_std_data[2] = {
    {
        .desc = "SOC DEBUG",
        .type = EHSM_CHALLENGE_TYPE_SOC_DEBUG,
        .algo = EHSM_AUTH_ALGO_SM3_SM2,
        .challenge_size = 48U,
        .key_type = EHSM_KEY_TYPE_SM2,
        .key_part = EHSM_KEY_PART_KEY_PAIR,
        .key = s_debug_auth_sm3_sm2_key,
        .key_size = sizeof(s_debug_auth_sm3_sm2_key),
        .pubkey_size = 65U,
        .prikey_size = 32U,
        .soc_dbg_bitmap = &s_debug_auth_soc_dbg_bitmap,
        .soc_dbg_bitmap_size = sizeof(s_debug_auth_soc_dbg_bitmap)
    }, {
        .desc = "USER AUTH",
        .type = EHSM_CHALLENGE_TYPE_USER_AUTH,
        .algo = EHSM_AUTH_ALGO_AES128_CMAC,
        .challenge_size = 48U,
        .key_type = EHSM_KEY_TYPE_AES_128,
        .key_part = EHSM_KEY_PART_SYMM_KEY,
        .key = s_debug_auth_symm_key,
        .key_size = sizeof(s_debug_auth_symm_key),
        .pubkey_size = 0U,
        .prikey_size = 16U,
        .soc_dbg_bitmap = NULL,
        .soc_dbg_bitmap_size = 0U
    }
};
// clang-format on

static uint32_t debug_auth_sign_gen(const demo_debug_auth_std_st *std_data, uint8_t *challenge, uint32_t challenge_size,
    uint8_t *signature, uint32_t *signature_size)
{
    uint32_t ret;
    uint8_t *sm2_default_id = (uint8_t *)"1234567812345678";
    uint8_t Z[32];
    uint8_t E[32];
    uint8_t tmp_key[512];

    memcpy(tmp_key, std_data->key, std_data->key_size);

    switch (std_data->algo) {
    case EHSM_AUTH_ALGO_SM3_SM2:
        *signature_size = 64U;
        SM2_Init();
        SM2_GetZ(sm2_default_id, 16, tmp_key, Z);  // SM2 算法预计算，获取 Za
        SM2_GetE(challenge, challenge_size, Z, E); // SM2 算法预计算，获取 E
        ret = SM2_Sign(E, &tmp_key[65], signature);
        break;
    case EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1:
        *signature_size = 64U;
        SHA256_Hash(challenge, challenge_size, E);
        ret = ECDSA_sign(E, 32, &tmp_key[64], signature);
        break;
    case EHSM_AUTH_ALGO_AES128_CMAC:
        *signature_size = 16U;
        ret = AES_CMAC(tmp_key, 16, challenge, challenge_size, signature, 16);
        break;
    case EHSM_AUTH_ALGO_SM4_CMAC:
        *signature_size = 16U;
        ret = SM4_CMAC(tmp_key, challenge, challenge_size, signature, 16);
        break;
    default:
        ret = !EHSM_OK;
        break;
    }

    return ret;
}

/**
 * @brief 调试鉴权流程的演示。
 *
 * @param[in] std_data 调试鉴权的标准演示数据，详见 @ref demo_debug_auth_std_st
 *
 * @note
 * - 获取挑战值时，eHSM 会根据挑战类型生成挑战值， 根据挑战类型不同，得到的挑战值和长度也不相同：
 *      - 若为 SHE 类型，则返回 （16 字节随机数 || 16 字节 UID）；
 *      - 若为其他挑战类型，则返回（32 字节随机数 || 16 字节 UID）；
 * - 调试身份认证时，eHSM 验证签名值，认证通过后打开对应的调试端口：
 *      - 需要指定挑战类型和之前获取挑战时的挑战类型一致；
 *      - 需要指定鉴权的算法，详见 @ref ehsm_auth_algo_e，并且：
 *          - 若为公钥算法，则需要指定公钥和长度，同时将公钥 hash 值写入 OTP 对应的鉴权密钥位置；
 *          - 若为对称算法，需要将密钥写入 OTP 对应的鉴权密钥位置，若不足 32 字节需要在尾部填充 0；
 *      - 若为 SoC 鉴权调试，需要指定调试端口的配置，详见 @ref ehsm_soc_dbg_bitmap_st;
 * - 关闭调试时，eHSM 关闭对应的调试端口，需要指定挑战类型，若为 SoC 鉴权挑战，需要指定调试端口的配置，详见 @ref
 * ehsm_soc_dbg_bitmap_st。
 *
 */
static void demo_debug_auth(const demo_debug_auth_std_st *std_data)
{
    uint32_t ret;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();       /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *challenge = ehsm_demo_get_buffer(0); /* 获取挑战值的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint8_t *pubkey = ehsm_demo_get_buffer(1);    /* 用于输入公钥的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t pubkey_size;                         /* 公钥的长度 */
    uint8_t *signature = ehsm_demo_get_buffer(2); /* 用于输入签名值的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t signature_size = 0U;
    ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap_buffer = (ehsm_soc_dbg_bitmap_st *)ehsm_demo_get_buffer(
        3); /* SoC 鉴权时用于输入调试端口配置的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    ehsm_soc_dbg_bitmap_st *soc_dbg_bitmap_ptr = NULL; /* 指向调试端口配置数据的指针 */
    uint8_t backup_otp[1024];                          /* 用于备份 OTP 数据 */

    ehsm_port_printf("[Demo starts.] \r\n");

    /* 备份 OTP 数据 */
    demo_read_otp_from_soc_addr(0U, backup_otp, sizeof(backup_otp));

    /* 0. 预先安装鉴权密钥到 OTP，若已经安装对应的鉴权密钥，请忽略这一步，本 demo 在 demo.c 已经安装了对应的 OTP 数据 */
    // demo_write_otp_from_soc_addr(0U, s_debug_auth_otp_data, sizeof(s_debug_auth_otp_data));
    // demo_reset_ehsm_wait_ready(); /* reset eHSM 生效 */

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    /* 2. 调用获取挑战值的 API，根据挑战类型获取挑战值。 */
    ret = ehsm_get_challenge(ctx, std_data->type, challenge);
    ret = demo_check_val("The execution of getting challenge API:", EHSM_OK, ret);

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，打印获取到的挑战值 */
        print_hex("The challenge is: \r\n    ", challenge, std_data->challenge_size);

        /* 3. 根据采用的算法，对获取的挑战值进行签名，密钥（或其 hash 值）应预先写入 OTP，也可采用外部工具生成签名 */
        ret = debug_auth_sign_gen(std_data, challenge, std_data->challenge_size, signature, &signature_size);
        ret = demo_check_val("The execution of sign gen:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* eHSM FW 返回成功，打印获取到的挑战值 */
        print_hex("The signature is: \r\n    ", signature, signature_size);

        if ((EHSM_AUTH_ALGO_AES128_CMAC == std_data->algo) || (EHSM_AUTH_ALGO_SM4_CMAC == std_data->algo)) {
            /* 对称算法无需公钥 */
            pubkey = NULL;
            pubkey_size = 0U;
        } else {
            /* 公钥算法需要拷贝公钥到 SoC 与 eHSM 的共享内存 */
            pubkey_size = std_data->pubkey_size;
            (void)memcpy(pubkey, std_data->key, pubkey_size);
        }

        if (EHSM_CHALLENGE_TYPE_SOC_DEBUG == std_data->type) {
            /* SoC 鉴权需要指定调试端口配置，拷贝调试端口配置到 SoC 与 eHSM 的共享内存 */
            soc_dbg_bitmap_ptr = soc_dbg_bitmap_buffer;
            (void)memcpy(soc_dbg_bitmap_ptr, std_data->soc_dbg_bitmap, std_data->soc_dbg_bitmap_size);
        } else {
            soc_dbg_bitmap_ptr = NULL;
        }

        /* 4. 调用调试鉴权的 API */
        ret = ehsm_debug_auth(
            ctx, std_data->type, std_data->algo, signature, signature_size, pubkey, pubkey_size, soc_dbg_bitmap_ptr);
        ret = demo_check_val("The execution of debug auth API:", EHSM_OK, ret);
    }

    if (EHSM_OK == ret) {
        /* 5. 调用关闭调试的 API，关闭调试端口 */
        ret = ehsm_close_debug(ctx, std_data->type, soc_dbg_bitmap_ptr);
        ret = demo_check_val("The execution of close debug API:", EHSM_OK, ret);
    }

    /* 恢复备份的 OTP 数据，若自行安装了 OTP 密钥和对应的数据，需要恢复 OTP 数据，本 demo 无需该操作。 */
    // demo_write_otp_from_soc_addr(0U, backup_otp, sizeof(backup_otp));

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 调试鉴权的演示入口函数。
 */
void ehsm_demo_debug_auth_entry(void)
{
    uint32_t i;
    demo_debug_auth_std_st *std_data = s_debug_auth_std_data;
    uint32_t cnt = sizeof(s_debug_auth_std_data) / sizeof(demo_debug_auth_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for debugging auth starts. "
                     "==================== "
                     "\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_debug_auth(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for debugging auth ends. "
                     "==================== "
                     "\r\n\r\n");
}

