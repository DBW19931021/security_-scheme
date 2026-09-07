#ifndef STRANS_PRO_H
#define STRANS_PRO_H

#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef bool bool_t;
#define FALSE 0
#define TRUE  1

enum {
    STP_OK = 0,
    STP_ERR_PARAM = 1,
    STP_ERR_TIMEOUT = 2,
    STP_ERR_PEEK_NO_DATA = 3,
    STP_ERR_CRC = 4,
    STP_ERR_RESYNC = 5,
    STP_ERR_HAL = 6,
};

/**
 * @brief 字符串输出回调函数类型
 *
 * @param str 输出的字符串内容
 * @param len 字符串长度
 */
typedef void (*stp_string_output_cb)(const char *str, uint32_t len);

/**
 * @brief 协议初始化
 *
 * @param is_client 设备角色，TRUE为客户端，FALSE为服务端
 * @param string_cb 字符串输出回调函数，NULL表示忽略字符串帧
 * @return 0表示成功
 */
uint32_t stp_init(bool_t is_client, stp_string_output_cb string_cb);

/**
 * @brief 协议关闭
 */
void stp_deinit(void);

/**
 * @brief 发送指定长度的数据
 *
 * 数据可能缓存在底层buffer，需要调用 stp_flush 才能保证发送所有数据
 *
 * @param data 放数据的内存地址
 * @param size 要发送的长度
 * @return 0表示成功
 */
uint32_t stp_send(const uint8_t *data, uint32_t size);

/**
 * @brief 将缓存中的发送数据全部发出。
 *
 * 由于可能使用缓存提速，当发送完所有数据准备接收数据时，应该先调用此函数将可能缓存的数据全都发送。
 *
 * @return 0表示成功
 */
uint32_t stp_flush(void);

/**
 * @brief 接收指定长度的数据。
 *
 * @param buf 存放接收数据的缓存地址，必须足够长
 * @param size 接收的数据长度
 *
 * @return 0表示成功
 */
uint32_t stp_recv(uint8_t *buf, uint32_t size);

/**
 * @brief 手动重置通信状态（供测试使用）
 * 
 * 根据设备角色重置到初始模式：
 * - 客户端：发送模式
 * - 服务端：接收模式
 */
void stp_reset_comm(void);

/**
 * @brief 发送字符串帧
 *
 * @param data 字符串数据
 * @param size 数据长度
 * @return 0表示成功
 */
uint32_t stp_send_string(const uint8_t *data, uint32_t size);

/**
 * @brief 获取时间戳，以毫秒为单位
 *
 * @note 此函数由底层适配提供
 *
 * @return 时间戳
 */
uint32_t stp_get_time_ms(void);

#endif // STRANS_PRO_H
