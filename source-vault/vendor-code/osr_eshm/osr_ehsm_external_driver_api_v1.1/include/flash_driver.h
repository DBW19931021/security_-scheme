#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#include <stdint.h>
#include "types.h"

/**
 * @brief 初始化FLASH驱动模块。
 *
 * @retval EHSM_ERR_SW_SUCCESS 初始化成功
 * @retval EHSM_ERR_INIT_DEV_FAIL 初始化FLASH设备失败
 */
uint32_t custom_flash_init(void);

/**
 * @brief 从FLASH中读取数据。
 *
 * @param addr FLASH源地址
 * @param data 存放读取数据的RAM地址
 * @param size 读取的数据长度，以字节为单位
 *
 * @retval EHSM_ERR_SW_SUCCESS 读取数据成功
 * @retval EHSM_ERR_PARAM_ERROR 传入参数错误
 * @retval EHSM_ERR_READ_DEV_FAIL 硬件问题导致读取数据失败
 */
uint32_t custom_flash_read(uint32_t addr, uint8_t *data, uint32_t size);

/**
 * @brief 向FLASH中写入数据。
 *
 * 此函数实现向FLASH中写入数据的功能，包括数据对齐，及页数据的备份、擦除、更新都由此函数负责。
 *
 * @param addr FLASH目标地址
 * @param data RAM中的源数据地址
 * @param size 写入的数据长度，以字节为单位
 *
 * @retval EHSM_ERR_SW_SUCCESS 写入数据成功
 * @retval EHSM_ERR_PARAM_ERROR 传入参数错误
 * @retval EHSM_ERR_WRITE_DEV_FAIL 硬件问题导致写入数据失败
 * @retval EHSM_ERR_ERASE_DEV_FAIL 硬件问题导致擦除失败
 */
uint32_t custom_flash_write(uint32_t addr, const uint8_t *data, uint32_t size);

#endif // FLASH_DRIVER_H
