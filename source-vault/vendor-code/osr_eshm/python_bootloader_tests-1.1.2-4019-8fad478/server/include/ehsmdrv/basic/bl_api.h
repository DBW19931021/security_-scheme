#ifndef EHSM_BL_API_H
#define EHSM_BL_API_H

#include "types.h"
#include "api.h"

/**
 * @defgroup bl_api BL (Bootloader) 相关的API接口
 * @{
 */

/**
 * @defgroup self-test-algo 自检算法
 * @{
 */
#define EHSM_SELF_TEST_SM4_ECB    0x01
#define EHSM_SELF_TEST_SM4_CBC    0x02
#define EHSM_SELF_TEST_SM4_CFB    0x04
#define EHSM_SELF_TEST_SM4_OFB    0x08
#define EHSM_SELF_TEST_SM4_CTR    0x10
#define EHSM_SELF_TEST_SM2_ENC    0x20
#define EHSM_SELF_TEST_SM2_VERIFY 0x40
#define EHSM_SELF_TEST_SM2_EXCH   0x80
#define EHSM_SELF_TEST_SM3        0x100
#define EHSM_SELF_TEST_DES        0x200
#define EHSM_SELF_TEST_TDES       0x400
#define EHSM_SELF_TEST_AES        0x800
#define EHSM_SELF_TEST_RSA        0x1000
#define EHSM_SELF_TEST_ECC        0x2000
#define EHSM_SELF_TEST_MD5        0x4000
#define EHSM_SELF_TEST_SHA1       0x8000
#define EHSM_SELF_TEST_SHA2       0x10000
#define EHSM_SELF_TEST_SHA3       0x20000
#define EHSM_SELF_TEST_SHA256     0x40000
#define EHSM_SELF_TEST_TRNG       0x80000
/** @}*/

/**
 * @brief BL下获取版本号，见 @ref ehsm_get_version 。
 */
#define ehsm_bl_get_version(...) ehsm_get_version(__VA_ARGS__)

/**
 * @brief BL下的校验安全启动镜像，见 @ref ehsm_verify_image 。
 */
#define ehsm_bl_verify_image(...) ehsm_verify_image(__VA_ARGS__)

//#% #if CONFIG_BL_CUSTOM_ID != 0xb001
/**
 * @brief BL下的校验安全启动镜像，支持镜像头与代码分离。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] image 镜像头的存储位置
 * @param[in] image_size 镜像的大小，包括镜像头和代码区
 * @param[in] code 代码区的存储位置，如果为NULL，表示在镜像头的后面，即 `image + 1024`
 * @param[in] only_copy_code
 *           - `true` : 如果镜像是SOC FW启动镜像，将代码区解密/复制到 `image_out`的位置；如果镜像是eHSM
 * FW启动镜像，将代码区解密/复制到IRAM的启始地址
 *           - `false` : 如果镜像是SOC FW启动镜像，将镜像头+代码区解密/复制到 `image_out`的位置；如果镜像是eHSM
 * FW启动镜像，将镜像头+代码区解密/复制到IRAM的启始地址
 * @param[in] check_version 是否检查 version counter
 * @param[in] boot 校验成功后是否跳转到FW中执行，仅对eHSM镜像有效
 * @param image_out 镜像解密输出的地址，仅对SOC镜像有效，如果是eHSM镜像此参数应为 `NULL`
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_verify_image_discrete(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *image, uint32_t image_size,
    EHSM_SHM const uint8_t *code, bool_t only_copy_code, bool_t check_version, bool_t boot,
    EHSM_SHM uint8_t *image_out);

//#% #endif // CONFIG_BL_CUSTOM_ID != 0xb001

/**
 * @brief BL下校验升级固件镜像，见 @ref ehsm_upgrade_fw_image 。
 *
 */
#define ehsm_bl_upgrade_fw_image(...) ehsm_upgrade_fw_image(__VA_ARGS__)

/**
 * @brief BL下校验升级固件镜像初始化，见 @ref ehsm_upgrade_fw_image_init 。
 */
#define ehsm_bl_upgrade_fw_image_init(...) ehsm_upgrade_fw_image_init(__VA_ARGS__)

/**
 * @brief BL下校验升级固件镜像更新数据，见 @ref ehsm_upgrade_fw_image_update 。
 */
#define ehsm_bl_upgrade_fw_image_update(...) ehsm_upgrade_fw_image_finish(__VA_ARGS__)

/**
 * @brief BL下校验升级固件镜像完成，见 @ref ehsm_upgrade_fw_image_finish 。
 */
#define ehsm_bl_upgrade_fw_image_finish(...) ehsm_upgrade_fw_image_finish(__VA_ARGS__)

/**
 * @brief BL下设置串口波特率，见 @ref ehsm_set_uart_baudrate 。
 */
#define ehsm_bl_set_uart_baudrate(...) ehsm_set_uart_baudrate(__VA_ARGS__)

/**
 * @brief BL下设置HSM频率。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param hsm_freq HSM的频率，单位为Hz
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_set_hsm_freq(EHSM_SHM ehsm_ctx_st *ctx, uint32_t hsm_freq);

/**
 * @brief 启动算法自检。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_self_test(EHSM_SHM ehsm_ctx_st *ctx);

/**
 * @brief 查询算法自检结果。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] result 算法自检结果，见 @ref ehsm_self_test_result_st
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_get_self_test_result(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM ehsm_self_test_result_st *result);

/**
 * @brief BL下获取鉴权挑战值，见 @ref ehsm_get_challenge 。
 */
#define ehsm_bl_get_challenge(...) ehsm_get_challenge(__VA_ARGS__)

/**
 * @brief BL下调试鉴权，见 @ref ehsm_debug_auth 。
 */
#define ehsm_bl_debug_auth(...) ehsm_debug_auth(__VA_ARGS__)

/**
 * @brief BL下关闭调试功能，见 @ref ehsm_close_debug 。
 */
#define ehsm_bl_close_debug(...) ehsm_close_debug(__VA_ARGS__)

/**
 * @brief 国密固件认证。
 *
 * @note 需要先执行 @ref ehsm_bl_get_challenge 。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] type 认证类型，见 @ref ehsm_self_test_result_st
 * @param[in] arg 当类型为 1 时，arg 表示用户密钥 ID；当类型为 2 时，表示新的版本计数器；否则忽略
 * @param[in] auth_data 认证数据。认证数据，表示随机数据 Ra||Rb 的 SM4‑CBC 加密结果，Ra 由 bl_get_challenge 命令生成，Rb
 * 由主机生成。Ra 和Rb 均为 16 字节
 * @param[out] out_addr OTP 输出 buffer，可以为 NULL，当不为 NULL 时，应当至少有720 字节空间
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_fw_auth(EHSM_SHM ehsm_ctx_st *ctx, uint8_t type, const uint8_t arg[16],
    EHSM_SHM const uint8_t *auth_data, EHSM_SHM uint8_t *out_addr);

//#% #if CONFIG_BL_CUSTOM_ID == 0xb001

/**
 * @brief 获取SOCID
 *
 * 实际获取到的SOCID为16字节。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[out] socid 保存SOCID的buffer地址，可用空间不能小于16
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_get_socid(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *socid);

//#% #endif // CONFIG_BL_CUSTOM_ID == 0xb001

typedef enum {
    EHSM_BL_GEN_KEY_TYPE_SYMM = 1,
    EHSM_BL_GEN_KEY_TYPE_SM2 = 2,
    EHSM_BL_GEN_KEY_TYPE_ECC_P256R1 = 4,
} ehsm_bl_gen_key_type_e;

/**
 * @brief 生成可写入OTP的被ROOT KEY加密的密钥值及相应的CRC。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_level 密钥级别，见 @ref ehsm_key_level_e
 * @param[in] key_type 生成的密钥类型 见 @ref ehsm_bl_gen_key_type_e
 * @param[out] key_out 输出加密密钥值和CRC32的地址，空间不能小于36字节
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_get_random_key(
    EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level, ehsm_bl_gen_key_type_e key_type, EHSM_SHM uint8_t *key_out);

/**
 * @brief 加密外部提供的密钥值，输出被对应ROOT KEY加密的密钥值及相应的CRC。
 *
 * 此命令内部使用 `CHIP RTL KEK EHSM` 或 `CHIP RTL KEK SOC` 密钥对输入值进行 CBC (IV=全0）解密，然后使用对应的 `CHIP
 * ROOT KEY` 或 `DEVICE ROOT KEY` 对密钥明文进行加密，并计算其CRC32值。
 *
 * @param[in,out] ctx ehsm context 地址，需事先调用过 @ref ehsm_ctx_init 初始化
 * @param[in] key_level 密钥级别，见 @ref ehsm_key_level_e
 * @param[in] input_data 密钥值或公钥HASH值，根据 `key_level` 的取值，被 `CHIP RTL KEK EHSM` 或 `CHIP RTL KEK SOC`
 * 加密过（CBC模式，IV=全0，算法由OTP HW-CTRL配置中的KeyAlgSel决定是AES128还是SM4）
 * @param[in] size `input_data` 的字节长度
 * @param[out] key_out 输出加密密钥值和CRC32的地址，空间不能小于36字节
 *
 * @retval EHSM_OK 任务成功完成
 * @retval EHSM_ERR_NEED_POLL 任务未完成
 * @retval 其它值 任务发生错误
 */
uint32_t ehsm_bl_encrypt_key(EHSM_SHM ehsm_ctx_st *ctx, ehsm_key_level_e key_level, EHSM_SHM const uint8_t *input_data,
    uint32_t size, EHSM_SHM uint8_t *key_out);

/**
 * @brief BL下读取CFG寄存器值，见 @ref ehsm_read_reg 。
 */
#define ehsm_bl_read_reg(...) ehsm_read_reg(__VA_ARGS__)

/**
 * @brief BL下写入CFG寄存器值，见 @ref ehsm_write_reg 。
 */
#define ehsm_bl_write_reg(...) ehsm_write_reg(__VA_ARGS__)

/**
 * @brief BL下读取OTP值，见 @ref ehsm_read_otp 。
 */
#define ehsm_bl_read_otp(...) ehsm_read_otp(__VA_ARGS__)

/**
 * @brief BL下写入OTP值，见 @ref ehsm_write_otp 。
 */
#define ehsm_bl_write_otp(...) ehsm_write_otp(__VA_ARGS__)

// #% #if CONFIG_BL_INJECT_ERR_ENABLE
/**
 * @brief BL下向 eHSM 注入错误并可在 SOC 端观察到对应的错误信号。
 */
#define ehsm_bl_inject_error(...) ehsm_inject_error(__VA_ARGS__)
// #% #endif // CONFIG_BL_INJECT_ERR_ENABLE

/** @} */

#endif // EHSM_BL_API_H
