// #% #if CONFIG_HOST_DEREIVE_KEY_EN
#include <stdio.h>
#include <string.h>

#include "common/mem.h"

#include "ehsmdrv/basic/api.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "ehsm_demo_derive_key.h"

#define DEMO_DEREIVE_KEY_PARRENT_KEY_HANDLE (0x100000U)
#define DEMO_DEREIVE_KEY_KEY_HANDLE         (DEMO_DEREIVE_KEY_PARRENT_KEY_HANDLE + 1U)

uint32_t ehsm_km_derive_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_hash_algo_e hash_algo, ehsm_derive_algo_e derive_algo,
    ehsm_derive_type_e derive_type, uint32_t privilege, ehsm_key_type_e key_type, uint16_t key_size,
    uint32_t parent_key_handle, EHSM_SHM const uint8_t *salt, uint32_t salt_size, EHSM_SHM const uint8_t *password,
    uint32_t password_size, uint32_t iter_times, uint32_t *key_handle);

typedef struct {
    const char *desc;
    ehsm_hash_algo_e hash_algo;
    ehsm_derive_algo_e derive_algo;
    ehsm_derive_type_e derive_type;
    uint32_t privilege;
    ehsm_key_type_e key_type;
    uint16_t key_size;
    uint32_t parent_key_handle;
    const uint8_t *salt;
    uint32_t salt_size;
    const uint8_t *password;
    uint32_t password_size;
    uint32_t iter_times;
    uint32_t key_handle;
} demo_derive_key_std_st;

/* clang-format off */
/* 只有 PBKDF2 才可以选择 password 方式生成密钥 */
static const uint8_t s_derive_key_password[32] = {
    0xC0,0x6B,0xF3,0xBF,0x71,0xD1,0xEF,0xEE,0xCE,0xD3,0xDC,0x35,0xF2,0x27,0x87,0x37,
    0x30,0x8E,0x8A,0x16,0x8A,0x3E,0xE9,0xF6,0x7E,0x1A,0xDD,0x1B,0xCC,0x25,0x79,0x3F,
};

/* 只有 PBKDF2 才可以选择 salt */
static const uint8_t s_derive_key_std_salt[64] = {
    0x0A,0x11,0x79,0x19,0xB7,0x18,0x95,0x6E,0xF3,0x9B,0x17,0x6C,0xA3,0xBC,0xB4,0x41,
    0x23,0xD0,0xFC,0x16,0x70,0x34,0xAA,0xED,0xD7,0x91,0x58,0x71,0x99,0x32,0x44,0xA9,
    0x8A,0x74,0xA1,0x20,0xA9,0x9E,0x12,0x98,0xE9,0x76,0xE0,0xA5,0xF5,0xCC,0x55,0xA4,
    0x4D,0x2E,0x67,0xDC,0x29,0x5F,0x9C,0x8D,0x8D,0xC4,0xA9,0x89,0xBD,0xDE,0xCB,0xE0
};

static demo_derive_key_std_st s_derive_key_std_data[3] = {
    {
        .desc = "KDFX963 HASH SHA2-256 uses parrent key to generate AES256 key",
        .hash_algo = EHSM_HASH_ALGO_SHA256,
        .derive_algo = EHSM_DERIVE_ALGO_KDFX963,
        .derive_type = EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
        .privilege = EHSM_KEY_PRIV_KEY_CREATION | EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT | EHSM_KEY_PRIV_REMOVE,
        .key_type = EHSM_KEY_TYPE_AES_256,
        .key_size = 32,
        .parent_key_handle = DEMO_DEREIVE_KEY_PARRENT_KEY_HANDLE,
        .salt = NULL,
        .salt_size = 0U,
        .password = NULL,
        .password_size = 0U,
        .iter_times = 0U,
        .key_handle = 0xFFFFFFFFU, /* 表示由 eHSM 密钥管理服务分配 key handle */
    }, {
        .desc = "PBKDF2 HASH SM3 with 64B salt uses password to generate SM4 key",
        .hash_algo = EHSM_HASH_ALGO_SM3,
        .derive_algo = EHSM_DERIVE_ALGO_PBKDF2,
        .derive_type = EHSM_DERIVE_TYPE_PASSWORD,
        .privilege = EHSM_KEY_PRIV_KEY_CREATION | EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT | EHSM_KEY_PRIV_REMOVE,
        .key_type = EHSM_KEY_TYPE_SM4,
        .key_size = 16,
        .parent_key_handle = 0U,
        .salt = s_derive_key_std_salt,
        .salt_size = sizeof(s_derive_key_std_salt),
        .password = s_derive_key_password,
        .password_size = sizeof(s_derive_key_password),
        .iter_times = 2000U,
        .key_handle = DEMO_DEREIVE_KEY_KEY_HANDLE,
    }, {
        .desc = "PBKDF2 HASH SHA256 with 64B salt uses parrent key to generate HMAC 64B key",
        .hash_algo = EHSM_HASH_ALGO_SHA256,
        .derive_algo = EHSM_DERIVE_ALGO_PBKDF2,
        .derive_type = EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
        .privilege = EHSM_KEY_PRIV_KEY_CREATION | EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_REMOVE,
        .key_type = EHSM_KEY_TYPE_HMAC,
        .key_size = 64,
        .parent_key_handle = DEMO_DEREIVE_KEY_PARRENT_KEY_HANDLE,
        .salt = s_derive_key_std_salt,
        .salt_size = sizeof(s_derive_key_std_salt),
        .password = NULL,
        .password_size = 0U,
        .iter_times = 1000U,
        .key_handle = DEMO_DEREIVE_KEY_KEY_HANDLE,
    }
};
// clang-format on

/**
 * @brief 派生密钥的演示。
 *
 * @param[in] std_data 派生密钥的标准演示数据，详见 @ref demo_derive_key_std_st
 *
 * @note
 * - 根据指定的 key_type 生成对称密钥或HMAC 密钥，详见 @ref ehsm_key_type_e：
 *      - 对于对称密钥，key_size 必须和 key_type 匹配，例如指定 key_type = EHSM_KEY_TYPE_AES_256，则 key_size 必须为32。
 *      - 对于 HMAC 密钥，则 key_type = EHSM_KEY_TYPE_HMAC，key_size 最大值为 512。
 * - 可以指定使用的 KDF 算法，derive_algo，详见 @ref ehsm_derive_algo_e：
 *      - 使用 X963 算法：
 *          - 派生类型必须是使用父密钥来派生，即 derive_type = EHSM_DERIVE_TYPE_FROM_PARENT_KEY，eHSM 内部必须存在有效的
 *              父密钥，且改父密钥具有派生密钥的权限。且父密钥的密钥类型必须和当前的指定的密钥类型一致。
 *          - 无需盐，salt 必须为 NULL， salt_size = 0。
 *          - 不支持使用密码派生，即 password = NULL, password_size = 0。
 *      - 使用 PBKDF2 算法：
 *          - 派生类型可选通过父密钥或密码来派生，详见 @ref ehsm_key_derive_type_e：
 *              - 若选择通过父密钥派生，即 derive_type = EHSM_DERIVE_TYPE_FROM_PARENT_KEY，eHSM 内部必须存在有效的
 *              父密钥，且改父密钥具有派生密钥的权限。且父密钥的密钥类型必须和当前的指定的密钥类型一致。
 *              - 若选择通过密码派生，即 derive_type = EHSM_DERIVE_TYPE_PASSWORD，则 password 必须有效，长度不能为 0。
 *          - 必须指定 salt 和 salt_size。
 *          - 必须指定 iter_times，即迭代次数。
 * - key handle 可以有两种选择：
 *      - Host 在 API 传参为有效的 key handle，具体参考固件 TRM 文档描述。
 *      - Host 在 API 传参为 0xFFFFFFFF，将由 eHSM 密钥管理服务分配 key handle。
 *
 */
static void demo_derive_key(const demo_derive_key_std_st *std_data)
{
    uint32_t ret = EHSM_OK;
    ehsm_ctx_st *ctx = ehsm_demo_get_ctx();      /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    uint8_t *salt = ehsm_demo_get_buffer(0);     /* 输入 salt 的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t salt_size;                          /* 用于指定 salt 的字节长度 */
    uint8_t *password = ehsm_demo_get_buffer(0); /* 输入 password 的 buffer，必须是 SoC 和 eHSM 的共享内存 */
    uint32_t password_size;                      /* 用于指定 password 的字节长度 */
    uint32_t key_handle = std_data->key_handle;  /* 用于指定密钥句柄，若为 0xFFFFFFFF 表示由 eHSM 分配 key handle */
    uint32_t parent_key_handle = std_data->parent_key_handle; /* 用于父密钥句柄 */
    uint32_t hmac_key_size;                                   /* 用于指定生成父密钥为 HAMC 密钥时的密钥长度 */
    bool_t key_need_remove = false;                           /* 用于标记派生出来的密钥是否需要移除 */
    bool_t parent_key_need_remove = false;                    /* 用于标记父密钥是否需要移除 */

    ehsm_port_printf("[Demo starts.] \r\n");
    /* 打印标准数据信息 */
    ehsm_port_printf("%s, key type is 0x%08x \r\n", std_data->desc, std_data->key_type);

    /* 1. 所有 API 的调用都必须先初始化 ctx，指定 Mailbox 通道，配置同步/异步，异步模式的回调函数（可为空）。*/
    ehsm_ctx_init(ctx, 1, false, NULL);

    if (EHSM_DERIVE_TYPE_FROM_PARENT_KEY == std_data->derive_type) {
        /* 2. 使用父密钥来派生，需要 eHSM 内部已经存在有效的父密钥，且具有派生密钥的权限，无需 password。 */
        password = NULL;
        password_size = 0U;

        /* 这里先生成一个父密钥，密钥类型与派生指定的一致。若要了解生成密钥的具体过程，详见生成密钥的示例。 */
        if (EHSM_KEY_TYPE_HMAC == std_data->key_type) {
            /* 指定派生 HMAC 密钥，因此需要导入一个 HAMC 密钥 */
            hmac_key_size = std_data->key_size;
        } else {
            /* 指定派生非 HMAC 密钥，即对称密钥，因此需要导入一个对称密钥，无需指定 key_size */
            hmac_key_size = 0U;
        }
        ret = ehsm_km_gen_key(
            ctx, std_data->key_type, EHSM_KEY_PRIV_KEY_CREATION, 0U, hmac_key_size, NULL, 0U, &parent_key_handle);
        ret = demo_check_val("The execution of generaring key API:", EHSM_OK, ret);

        if (EHSM_OK == ret) {
            parent_key_need_remove = true; /* 标记父密钥需要移除 */

            /* 检查返回的父密钥的 key handle 是否和指定的值一致 */
            ret = demo_check_val("Comparison for parent key handle:", std_data->key_handle, key_handle);
        }
    } else {
        /* 2. PBKDF2 可选通过 password 来派生密钥，若选择该方式，拷贝 password 至 SoC 与 eHSM 的共享内存 */
        password_size = std_data->password_size;
        memcpy(password, std_data->password, password_size);
        print_hex("The password is: \r\n    ", std_data->password, password_size);
    }

    if (EHSM_OK == ret) {
        if (EHSM_DERIVE_ALGO_PBKDF2 == std_data->derive_algo) {
            /* PBKDF2 需要额外的 salt，拷贝 salt 至 SoC 与 eHSM 的共享内存 */
            salt_size = std_data->salt_size;
            memcpy(salt, std_data->salt, salt_size);
            print_hex("The salt is: \r\n    ", std_data->salt, salt_size);
        } else {
            /* X963 无需盐值 */
            salt = NULL;
            salt_size = 0U;
        }
    }

    if (EHSM_OK == ret) {
        /* 3. 调用派生密钥的 API。 */
        ret = ehsm_km_derive_key(ctx, std_data->hash_algo, std_data->derive_algo, std_data->derive_type,
            std_data->privilege, std_data->key_type, std_data->key_size, parent_key_handle, salt, salt_size, password,
            password_size, std_data->iter_times, &key_handle);
        ret = demo_check_val("The execution of defiving key API:", EHSM_OK, ret);

        if (EHSM_OK == ret) {
            parent_key_need_remove = true; /* 标记派生出来的密钥需要移除 */

            if (0xFFFFFFFF == std_data->key_handle) {
                /* 3. 若指定由 eHSM 分配，打印获取到的 key_handle */
                ehsm_port_printf("The key handle is: 0x%08x \r\n", key_handle);
            } else {
                /* 3. 若指定有效 key handle，则检查返回的 key handle 是否和指定的值一致 */
                ret = demo_check_val("Comparison for derived key handle:", std_data->key_handle, key_handle);
            }
        }
    }

    if (parent_key_need_remove) {
        /* 移除父密钥 */
        demo_remove_key(ctx, key_handle);
    }

    if (key_need_remove) {
        /* 移除派生出来的密钥 */
        demo_remove_key(ctx, key_handle);
    }

    if (EHSM_OK == ret) {
        ehsm_port_printf("[Demo ends with success !!!] \r\n\r\n");
    } else {
        ehsm_port_printf("[Demo ends with failure. !!!] \r\n\r\n");
    }
}

/**
 * @brief 派生密钥的演示入口函数。
 */
void ehsm_demo_derive_key_entry(void)
{
    uint32_t i;
    demo_derive_key_std_st *std_data = s_derive_key_std_data;
    uint32_t cnt = sizeof(s_derive_key_std_data) / sizeof(demo_derive_key_std_st);
    ehsm_port_printf("\r\n\r\n==================== eHSM demo for deriving key starts. "
                     "====================\r\n\r\n");

    for (i = 0U; i < cnt; i++) {
        ehsm_port_printf("demo for std_data[%d].\r\n", i);
        demo_derive_key(&std_data[i]);
    }

    ehsm_port_printf("\r\n==================== eHSM demo for deriving key ends. ====================\r\n\r\n");
}

// #% #endif /* CONFIG_HOST_DEREIVE_KEY_EN */
