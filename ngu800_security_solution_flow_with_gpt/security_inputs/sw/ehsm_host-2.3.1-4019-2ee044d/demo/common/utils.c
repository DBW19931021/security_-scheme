#include <stdio.h>
#include <string.h>
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

#define DEMO_PAUSE_WITH_ERROR

#define DEMO_OTP_REMAP_ADDR  (0x60040000)
#define DEMO_LIFCYCLE_FILTER (0x7F << 8)

#define DEMO_LIFECYCLE_TEST_MODE    (0x00000000U)
#define DEMO_LIFECYCLE_DEV_MODE     (0x42818414U)
#define DEMO_LIFECYCLE_MANU_MODE    (0x46C1A416U)
#define DEMO_LIFECYCLE_USER_MODE    (0x57C1EC96U)
#define DEMO_LIFECYCLE_DEBUG_MODE   (0xD7C5FCDEU)
#define DEMO_LIFECYCLE_DESTROY_MODE (0xFFFFFFFFU)

typedef struct {
    const char *str;
    uint32_t val;
} demo_lifecycle_st;

void demo_reset_ehsm_wait_ready(void)
{
    ehsm_port_reset_ehsm();

    /* 等待 HW BOOT DONE 和 HSM READY */
    while ((DEMO_HSM_STATUS_IN & (DEMO_SYSSTA0_BOOT_DONE | DEMO_SYSSTA0_HSM_READY)) == 0) {
        ;
    }
}

void print_hex(const char *prefix, const uint8_t *data, uint32_t size)
{
    ehsm_port_printf("%s", prefix);

    for (uint32_t i = 0; i < size; i++) {
        ehsm_port_printf("%02x", (uint32_t)data[i]);
    }
    ehsm_port_printf("\n");
}

uint32_t demo_check_val(const char *prefix, uint32_t expected, uint32_t actual)
{

    uint32_t ret;

    if (prefix) {
        ehsm_port_printf("%s \r\n", prefix);
    }

    if (expected != actual) {
        ret = actual;
        ehsm_port_printf("    failed, excepted is 0x%08x actual is 0x%08x .\r\n", expected, actual);
        // #% #ifdef DEMO_PAUSE_WITH_ERROR
        while (1) { }
        // #% #endif
    } else {
        ret = EHSM_OK;
        ehsm_port_printf("   successful. \r\n");
    }

    return ret;
}

uint32_t demo_check_data(const char *data_desc, const uint8_t *expected_data, uint32_t expected_size,
    uint8_t *actual_data, uint32_t actual_size)
{
    uint32_t ret;

    if (data_desc) {
        ehsm_port_printf("%s \r\n", data_desc);
    }

    /* 比较返回的输出数据长度和标准输出数据长度是否相等。 */
    print_hex("    The expected data is: ", expected_data, expected_size);
    print_hex("    The actual data is:   ", actual_data, actual_size);
    if (actual_size != expected_size) {
        ret = 1U;
        ehsm_port_printf("    The length comparson was failed, expeted size is %d, bur actual size is %d. \r\n",
            expected_size, actual_size);
        // #% #ifdef DEMO_PAUSE_WITH_ERROR
        while (1) { }
        // #% #endif
    } else {
        /* 比较返回的输出数据和标准输出数据是否完全一致。 */
        ret = (uint32_t)memcmp(expected_data, actual_data, expected_size);
        if (ret) {
            ehsm_port_printf("    Comparison failed. \r\n");
            // #% #ifdef DEMO_PAUSE_WITH_ERROR
            while (1) { }
            // #% #endif
        } else {
            ehsm_port_printf("    Comparison succeeded. \r\n");
        }
    }

    return ret;
}

void demo_wrap_key_data(uint8_t key_privilege, ehsm_key_type_e key_type, ehsm_key_part_e part_info, const uint8_t *key,
    uint32_t key_size, uint16_t pub_key_size, uint16_t priv_key_size, ehsm_key_format_st *key_data,
    uint32_t *key_data_size)
{
    uint32_t expected_size = sizeof(ehsm_key_format_st) + key_size;

    /* 封装密钥数据，示例代码，不考虑密钥相关的数据错误，只检查 key data buffer 的大小，避免溢出。 */
    if (*key_data_size >= expected_size) {
        /* 初始化数据 buffer */
        *key_data_size = expected_size;
        memset(key_data, 0x0U, *key_data_size);
        /* 将密钥数据拷贝到 eHSM 与 SoC 的共享内存。 */
        memcpy(key_data->key_value, key, key_size);

        /* 封装密钥数据 */
        key_data->privilege = key_privilege; /* 配置密钥的权限。 */
        key_data->privilege
            |= EHSM_KEY_PRIV_IMPORT_PLAIN | EHSM_KEY_PRIV_REMOVE; /* 配置为允许导入明文密钥，方便更新，允许移除 */
        key_data->key_type = key_type;                            /* 配置密钥的类型。 */
        key_data->part_info = part_info;                          /* 配置密钥部件类型。 */
        key_data->pub_key_size = pub_key_size;                    /* 公钥长度。 */
        key_data->priv_key_size = priv_key_size;                  /* 私钥长度。 */
    } else {
        ehsm_port_printf("Key data buffer is too small, expcted min size is %d, but actual is %d.\r\n", expected_size,
            *key_data_size);
    }
}

void demo_remove_key(ehsm_ctx_st *ctx, uint32_t key_handle)
{
    uint32_t ret;

    if (NULL != ctx && 0xFFFFFFFF != key_handle && 0 != key_handle) {
        ret = ehsm_km_remove_key(ctx, key_handle);
        ret = demo_check_val("The execution of removing key API:", EHSM_OK, ret);
        if (EHSM_OK != ret) {
            while (1) { }
        } else {
            ehsm_port_printf("Removing key for key handle = 0x%08x is successful. \r\n", key_handle);
        }
    }
}

void demo_read_otp_from_soc_addr(uint32_t offset, uint8_t *data, uint32_t size)
{
    (void)ehsm_port_read_otp(data, offset, size);
}

void demo_write_otp_from_soc_addr(uint32_t offset, const uint8_t *data, uint32_t size)
{
    (void)ehsm_port_write_otp(data, offset, size);
}

ehsm_lifecycle_e demo_get_lifecycle(void)
{
    uint32_t hsm_status = (uint32_t)(DEMO_HSM_STATUS_IN & DEMO_LIFCYCLE_FILTER);
    ehsm_lifecycle_e lc_enum;

    switch (hsm_status) {
    case DEMO_SYSSTA0_HSM_LC_TEST:
        lc_enum = EHSM_LC_TEST;
        break;
    case DEMO_SYSSTA0_HSM_LC_DEV:
        lc_enum = EHSM_LC_DEVELOP;
        break;
    case DEMO_SYSSTA0_HSM_LC_MANU:
        lc_enum = EHSM_LC_MANUFACTURE;
        break;
    case DEMO_SYSSTA0_HSM_LC_USER:
        lc_enum = EHSM_LC_USER;
        break;
    case DEMO_SYSSTA0_HSM_LC_DEBUG:
        lc_enum = EHSM_LC_DEBUG;
        break;
    case DEMO_SYSSTA0_HSM_LC_DESTROY:
        lc_enum = EHSM_LC_DESTORY;
        break;
    default:
        lc_enum = EHSM_LC_DESTORY;
        break;
    }
    return lc_enum;
}

void demo_chg_lifecycle_from_soc(ehsm_lifecycle_e to_lc_enum)
{
    uint32_t to_lc_u32;
    ehsm_lifecycle_e cur_lc_enum = demo_get_lifecycle();

    if (to_lc_enum != cur_lc_enum) {
        switch (to_lc_enum) {
        case EHSM_LC_TEST:
            to_lc_u32 = DEMO_LIFECYCLE_TEST_MODE;
            break;
        case EHSM_LC_DEVELOP:
            to_lc_u32 = DEMO_LIFECYCLE_DEV_MODE;
            break;
        case EHSM_LC_MANUFACTURE:
            to_lc_u32 = DEMO_LIFECYCLE_MANU_MODE;
            break;
        case EHSM_LC_USER:
            to_lc_u32 = DEMO_LIFECYCLE_USER_MODE;
            break;
        case EHSM_LC_DEBUG:
            to_lc_u32 = DEMO_LIFECYCLE_DEBUG_MODE;
            break;
        case EHSM_LC_DESTORY:
            to_lc_u32 = DEMO_LIFECYCLE_DESTROY_MODE;
            break;
        default:
            to_lc_u32 = DEMO_LIFECYCLE_TEST_MODE;
            break;
        }

        demo_write_otp_from_soc_addr(0U, (uint8_t *)(void *)&to_lc_u32, 4U);
    }
}
