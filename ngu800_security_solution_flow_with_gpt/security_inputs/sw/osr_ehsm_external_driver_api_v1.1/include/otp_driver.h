#ifndef OTP_DRIVER_H
#define OTP_DRIVER_H

#include <stdint.h>
#include "types.h"

/**
 * @brief 初始化OTP驱动模块。
 *
 * @retval EHSM_ERR_SW_SUCCESS 初始化成功
 * @retval EHSM_ERR_INIT_DEV_FAIL 初始化OTP设备失败
 */
uint32_t custom_otp_init(void);

/**
 * @brief 从OTP中读取数据。
 *
 * @param addr OTP源地址
 * @param data 存放读取数据的RAM地址
 * @param size 读取的数据长度，以字节为单位
 *
 * @retval EHSM_ERR_SW_SUCCESS 读取数据成功
 * @retval EHSM_ERR_PARAM_ERROR 传入参数错误
 * @retval EHSM_ERR_READ_DEV_FAIL 硬件问题导致读取数据失败
 */
uint32_t custom_otp_read(uint32_t addr, uint8_t *data, uint32_t size);

/**
 * @brief 向OTP中写入数据。
 *
 * 此函数负责向OTP写入数据，如果 `data` 中的某些bit会使得OTP中的值从写入后的值
 * 向OTP默认值变化，应当返回合适的错误码，否则写入应当成功（即OTP的值只能单向变
 * 化）
 *
 * @param addr OTP目标地址
 * @param data RAM中的源数据地址
 * @param size 写入的数据长度，以字节为单位
 *
 * @retval EHSM_ERR_SW_SUCCESS 写入数据成功
 * @retval EHSM_ERR_PARAM_ERROR 传入参数错误
 * @retval EHSM_ERR_WRITE_DEV_FAIL 硬件问题导致写入数据失败
 */
uint32_t custom_otp_write(uint32_t addr, const uint8_t *data, uint32_t size);

#endif // OTP_DRIVER_H
