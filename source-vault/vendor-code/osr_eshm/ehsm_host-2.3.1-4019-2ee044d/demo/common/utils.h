#ifndef EHSM_DEMO_UTILS_H
#define EHSM_DEMO_UTILS_H

#include <stdint.h>

#include "ehsmdrv/basic/types.h"
#include "ehsmdrv/basic/api.h"

#define DEMO_STATUS_BASE (0x40010000)

/* 对应硬件 o_hsm_status 31：0 */
#define DEMO_HSM_STATUS_IN          *((volatile unsigned int *)(DEMO_STATUS_BASE + 0x60))
#define DEMO_SYSSTA0_HW_BOOT_DONE   (1U << 0)
#define DEMO_SYSSTA0_HW_BOOT_ERR    (1U << 1)
#define DEMO_SYSSTA0_BOOT_DONE      (1U << 2)
#define DEMO_SYSSTA0_BOOT_ERR       (1U << 3)
#define DEMO_SYSSTA0_HSM_READY      (1U << 4)
#define DEMO_SYSSTA0_HSM_FAIL       (1U << 5)
#define DEMO_SYSSTA0_SOC_BOOT_DONE  (1U << 6)
#define DEMO_SYSSTA0_SOC_BOOT_ERR   (1U << 7)
#define DEMO_SYSSTA0_HSM_LC_TEST    (1U << 8)
#define DEMO_SYSSTA0_HSM_LC_DEV     (1U << 9)
#define DEMO_SYSSTA0_HSM_LC_MANU    (1U << 10)
#define DEMO_SYSSTA0_HSM_LC_USER    (1U << 11)
#define DEMO_SYSSTA0_HSM_LC_DEBUG   (1U << 12)
#define DEMO_SYSSTA0_HSM_LC_DESTROY (1U << 13)
#define DEMO_SYSSTA0_HSM_LC_UNDEF   (1U << 14)
#define DEMO_SYSSTA0_HSM_FW_STA0    (1U << 15)
#define DEMO_SYSSTA0_HSM_DBG_EN     (1U << 16)
#define DEMO_SYSSTA0_SOC_DBG_EN     (1U << 17)
#define DEMO_SYSSTA0_SOC_RESET      (1U << 18)
#define DEMO_SYSSTA0_SOC_CPU_RESET  (1U << 19)
#define DEMO_SYSSTA0_HSM_FW_STA1    (0x1F00000)

/* 对应硬件 o_hsm_status 63：32 */
#define DEMO_HSM_STATUS_IN1 *((volatile unsigned int *)(DEMO_STATUS_BASE + 0x64))

#define DEMO_KEY_TYPE_OTP (0x200000U)

/* OTP 密钥的逻辑 ID */
#define DEMO_OTP_CHIP_ROOT_KEY_ID        (DEMO_KEY_TYPE_OTP + 1U)
#define DEMO_OTP_DEVICE_ROOT_KEY_ID      (DEMO_KEY_TYPE_OTP + 2U)
#define DEMO_OTP_USER_ROOT_KEY_ID        (DEMO_KEY_TYPE_OTP + 3U)
#define DEMO_OTP_EHSM_DEBUG_KEY_ID       (DEMO_KEY_TYPE_OTP + 4U)
#define DEMO_OTP_EHSM_FW_VERIFY_KEY_ID   (DEMO_KEY_TYPE_OTP + 5U)
#define DEMO_OTP_EHSM_ENCRYPT_KEY_ID     (DEMO_KEY_TYPE_OTP + 6U)
#define DEMO_OTP_EHSM_UPG_ENCRYPT_KEY_ID (DEMO_KEY_TYPE_OTP + 7U)
#define DEMO_OTP_EHSM_UPG_VERIFY_KEY_ID  (DEMO_KEY_TYPE_OTP + 8U)
#define DEMO_OTP_EHSM_PRIVATE_KEY_ID     (DEMO_KEY_TYPE_OTP + 9U)
#define DEMO_OTP_SOC_DEBUG_KEY_ID        (DEMO_KEY_TYPE_OTP + 10U)
#define DEMO_OTP_SOC_FW_VERIFY_KEY_ID    (DEMO_KEY_TYPE_OTP + 11U)
#define DEMO_OTP_SOC_ENCRYPT_KEY_ID      (DEMO_KEY_TYPE_OTP + 12U)
#define DEMO_OTP_SOC_UPG_ENCRYPT_KEY_ID  (DEMO_KEY_TYPE_OTP + 13U)
#define DEMO_OTP_SOC_UPG_VERIFY_KEY_ID   (DEMO_KEY_TYPE_OTP + 14U)
#define DEMO_OTP_SOC_PRIVATE_KEY_ID      (DEMO_KEY_TYPE_OTP + 15U)
#define DEMO_OTP_SECRET_KEY_KEY_ID       (DEMO_KEY_TYPE_OTP + 16U)
#define DEMO_OTP_USER_AUTH_KEY_ID        (DEMO_KEY_TYPE_OTP + 17U)
#define DEMO_OTP_INVALID_KEY_ID          (DEMO_KEY_TYPE_OTP + 0xFFFFFU)

/**
 * @brief 重启 eHSM 并等待 HW BOOT DONE 和 HSM READY。
 */
void demo_reset_ehsm_wait_ready(void);

/**
 * @brief 打印数据。
 * @param[in] prefix 打印的前缀信息。
 * @param[in] data 被打印的数据。
 * @param[in] size 被打印数据的字节长度。
 */
void print_hex(const char *prefix, const uint8_t *data, uint32_t size);

/**
 * @brief 检查 U32 的值是否和期望值相等。
 * @param[in] prefix 打印的前缀信息。
 * @param[in] expected 期望的值。
 * @param[in] actual 实际值。
 *
 * @return uint32_t 0 代表检查通过，其他代表失败。
 */
uint32_t demo_check_val(const char *prefix, uint32_t expected, uint32_t actual);

/**
 * @brief 检查实际数据及长度是否和期望数据一致。
 * @param[in] data_desc 数据描述。
 * @param[in] expected_data 期望数据。
 * @param[in] expected_size 期望数据的字节长度。
 * @param[in] actual_data 实际数据。
 * @param[in] actual_size 实际数据的字节长度。
 *
 * @return uint32_t 0 代表检查通过，其他代表失败。
 *
 */
uint32_t demo_check_data(const char *data_desc, const uint8_t *expected_data, uint32_t expected_size,
    uint8_t *actual_data, uint32_t actual_size);

/**
 * @brief 导入密钥。
 * @param[in] key_privilege 密钥的权限，详见 key-priv 密钥权限。
 * @param[in] key_type 指定密钥类型，详见 @ref ehsm_key_type_e。
 * @param[in] part_info 指定密钥部件信息，详见 @ref ehsm_key_part_e。
 * @param[in] key 导入的原始密钥：
 *  - 若为对称密钥，原始密钥为对称密钥；
 *  - 若为 SM2 密钥，原始密钥为（65 字节公钥 || 32字节私钥）；
 *  - 若为 RSA 密钥，
 *      - 非 CRT 密钥，原始密钥为（e || n || d）;
 *      - CRT 密钥，原始密钥（e || n || p || q || dp || dq || u）;
 * @param[in] key_size 导入的原始密钥的长度。
 * @param[in] pub_key_size 导入的原始公钥长度:
 *  - 若为对称密钥，公钥长度为 0；
 *  - 若为 SM2 密钥，公钥长度为 65 字节；
 *  - 若为 RSA 密钥，公钥长度为 e 的长度；
 * @param[in] priv_key_size 导入的原始私钥长度:
 *  - 若为对称密钥，私钥长度为对称密钥的长度；
 *  - 若为 SM2 密钥，私钥长度为 32 字节；
 *  - 若为 RSA 密钥，
 *      - 非 CRT 密钥，私钥长度为 d 的长度;
 *      - CRT 密钥，私钥长度为 sizof(p || q || dp || dq || u);
 * @param[in,out] key_data 封装的密钥数据 buffer，见 @ref ehsm_key_format_st。
 * @param[in,out] key_data_size
 *  - 输入时是密钥数据 buffer 的大小，必须大于等于 sizeof(ehsm_key_format_st) + pub_key_size + priv_key;
 *  - 输出时是封装后的密钥数据的字节长度，见 @ref key-format。
 */
void demo_wrap_key_data(uint8_t key_privilege, ehsm_key_type_e key_type, ehsm_key_part_e part_info, const uint8_t *key,
    uint32_t key_size, uint16_t pub_key_size, uint16_t priv_key_size, ehsm_key_format_st *key_data,
    uint32_t *key_data_size);

/**
 * @brief 根据 key handle 删除 eHSM 的密钥。
 */
void demo_remove_key(ehsm_ctx_st *ctx, uint32_t key_handle);

/**
 * @brief 通过 SoC 映射地址从 OTP 读取数据。
 * @param[in] offset 读取地址相对于 OTP 起始地址的字节偏移。
 * @param[in,out] data 接受数据的 buffer。
 * @param[in] size 指定读取数据的长度。
 */
void demo_read_otp_from_soc_addr(uint32_t offset, uint8_t *data, uint32_t size);

/**
 * @brief 通过 SoC 映射地址写数据到 OTP。
 * @param[in] offset 写入地址相对于 OTP 起始地址的字节偏移。
 * @param[in] data 待写入的数据。
 * @param[in] size 待写入数据的字节长度。
 */
void demo_write_otp_from_soc_addr(uint32_t offset, const uint8_t *data, uint32_t size);

/**
 * @brief 通过 o_hsm_status 获取当前 eHSM 的生命周期的枚举值。
 */
ehsm_lifecycle_e demo_get_lifecycle(void);

/**
 * @brief 通过 SoC 映射地址写数据到 OTP 的方式更改生命周期。
 */
void demo_chg_lifecycle_from_soc(ehsm_lifecycle_e to_lc_enum);

#endif // EHSM_DEMO_UTILS_H
