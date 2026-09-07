#ifndef EHSM_HOST_PORT_H
#define EHSM_HOST_PORT_H

/*
 * 此文件定义了移植配置和接口，将此ehsm_host库移植到新的平台时，需要提供所需的
 * 宏定义和接口实现。
 */

#include "types.h"
#include "ehsmdrv/basic/types.h"

#include "ehsm_host_custom.h"

/* Forward declaration */
typedef enum ehsm_drv_mode ehsm_drv_mode_e;

/* mailbox channel 数量 */
#ifndef EHSM_PORT_MAILBOX_CHANNEL_COUNT
#error EHSM_PORT_MAILBOX_CHANNEL_COUNT must be defined!
#endif

/* Mailbox寄存器基地址，每个channel的寄存器空间为0x1000，寄存器空间应当是
 * non-cacheable的。 */
#ifndef EHSM_PORT_MAILBOX_REG_BASE_ADDR
#error EHSM_PORT_MAILBOX_REG_BASE_ADDR must be defined!
#endif

/* Mailbox里的note寄存器是写0清0还是写1清0，此功能为软模拟预留，实际硬件是写1清0 */
#ifndef EHSM_PORT_MAILBOX_NOTE_WRITE_1_CLEAR
#error EHSM_PORT_MAILBOX_NOTE_WRITE_1_CLEAR must be defined!
#endif

/* 共享地址空间基地址，此地址用于demo中的共享内存申请 */
#ifndef EHSM_PORT_SHARE_MEM_BASE_ADDR
#error EHSM_PORT_SHARE_MEM_BASE_ADDR must be defined!
#endif

/* 共享地址空间大小 */
#ifndef EHSM_PORT_SHARE_MEM_SIZE
#error EHSM_PORT_SHARE_MEM_SIZE must be defined!
#endif

/* eHSM OTP空间的大小 */
#ifndef EHSM_PORT_OTP_SIZE
#error EHSM_PORT_OTP_SIZE must be defined!
#endif

/* OTP的默认值是0还是1 */
#ifndef EHSM_PORT_OTP_DEFAULT_VALUE
#error EHSM_PORT_OTP_DEFAULT_VALUE must be defined!
#endif

/**
 * @brief 初始化移植平台。
 *
 * @return EHSM_OK表示成功，其它值表示失败
 */
uint32_t ehsm_port_init(int drv_mode);

/**
 * @brief 将逻辑地址转换为硬件地址。
 *
 * 用于将当前进程的逻辑地址转换为硬件地址并放到mailbox命令中，此硬件地址是eHSM
 * 可以识别的地址。
 *
 * @param addr 当前进程的逻辑地址
 * @return 硬件地址
 */
raddr_t ehsm_port_addr_to_raddr(const void *addr);

/**
 * @brief 将硬件地址转换为当前进程的逻辑地址。
 *
 * @param raddr 硬件地址
 * @return 逻辑地址
 */
void *ehsm_port_raddr_to_addr(raddr_t raddr);

/**
 * @brief 将CPU cache数据刷入RAM并失效cache。
 *
 * 将CPU cache中的数据刷入RAM然后将cache失效，确保cache数据写入RAM并且从RAM中读
 * 取最新数据，同时要考虑增加合适内存屏障操作避免CPU乱序执行引起的问题。
 * 实现者可以考虑将一段有限的地址空间作为共享内存，这样仅针对有限空间刷cache可
 * 提高性能；或者将共享内存空间配置为non-cacheable，这样此函数可以什么都不做。
 * 此函数会在每次发送Mailbox命令之前调用
 */
void ehsm_port_flush_and_invalidate_cache(void);

/**
 * @brief Mailbox中断回调函数原型。
 * @param channel 触发中断的mailbox channel ID
 */
typedef void (*ehsm_mailbox_isr_f)(uint32_t channel);

/**
 * @brief 使能指定mailbox channel的中断，并注册回调函数。
 *
 *  当中断触发时，传入的int_func必须被调用，且参数为对应channel值。
 *
 * @param channel Mailbox channel ID
 * @param int_func 中断回调函数
 * @return EHSM_OK 表示成功，其它值为错误码
 */
uint32_t ehsm_port_enable_mailbox_int(uint32_t channel, ehsm_mailbox_isr_f int_func);

/**
 * @brief 复位eHSM。
 */
void ehsm_port_reset_ehsm(void);

/**
 * @brief eHSM寄存器编号，用于读写。
 */
typedef enum {
    REG_HSM_STATUS_0,     ///< 对应 eHSM o_hsm_status[31:0]
    REG_HSM_STATUS_1,     ///< 对应 eHSM o_hsm_status[64:32]
    REG_HSM_ERR_SENSOR,   ///< 对应 eHSM o_hsm_err_seneor[31:0]
    REG_HSM_ERR_HW_0,     ///< 对应 eHSM o_hsm_err_hw[31:0]
    REG_HSM_ERR_HW_1,     ///< 对应 eHSM o_hsm_err_hw[64:32]
    REG_HSM_ERR_FW_0,     ///< 对应 eHSM o_hsm_err_fw[31:0]
    REG_HSM_ERR_FW_1,     ///< 对应 eHSM o_hsm_err_fw[64:32]
    REG_HSM_FUSA_ALARM_0, ///< 对应 eHSM o_hsm_fusa_alarm[31:0]
    REG_HSM_FUSA_ALARM_1, ///< 对应 eHSM o_hsm_fusa_alarm[64:32]
    REG_SOC_DBG_EN_0,     ///< 对应 eHSM o_soc_dbg_en_128b[31:0]
    REG_SOC_DBG_EN_1,     ///< 对应 eHSM o_soc_dbg_en_128b[63:32]
    REG_SOC_DBG_EN_2,     ///< 对应 eHSM o_soc_dbg_en_128b[95:64]
    REG_SOC_DBG_EN_3,     ///< 对应 eHSM o_soc_dbg_en_128b[127:96]
    REG_SOC_SENSOR,       ///< 输入eHSM的传感器告警寄存器，可写入以触发eHSM的操作
} ehsm_reg_e;

/**
 * @brief 读取eHSM寄存器值。
 *
 * @param reg 寄存器编号。
 * @return 寄存器的值。
 */
uint32_t ehsm_port_read_reg(ehsm_reg_e reg);

/**
 * @brief 写入eHSM寄存器值。
 *
 * @note 目前仅支持 `REG_SOC_SENSOR` 寄存器。
 *
 * @param reg 寄存器编号。
 * @param val 寄存器的值。
 */
void ehsm_port_write_reg(ehsm_reg_e reg, uint32_t val);

/**
 * @brief 读取eHSM OTP空间数据。
 * @param buf 存储读取数据的buffer
 * @param offset 起始偏移
 * @param len 读取长度
 * @return EHSM_OK表示成功，其它值为错误码
 */
uint32_t ehsm_port_read_otp(uint8_t *buf, uint32_t offset, uint32_t len);

/**
 * @brief 写入eHSM OTP空间数据。
 * @param buf 存储待写入数据的buffer
 * @param offset 起始偏移
 * @param len 写入长度
 * @return EHSM_OK表示成功，其它值为错误码
 */
uint32_t ehsm_port_write_otp(const uint8_t *otp_data, uint32_t offset, uint32_t len);

/**
 * @brief 计时器的ID。
 *
 * 对于简单的实现，可以直接使用当前的系统时钟值，因此这里定义为64位无符号整数。
 */
typedef uint64_t ehsm_port_timer_t;

/**
 * @brief 创建一个计时器。
 *
 * @return  计时器ID（可能仅是当前系统时钟值）
 */
ehsm_port_timer_t ehsm_port_create_timer(void);

/**
 * @brief 查看计时器是否超时。
 *
 * 由实现者判断多少时间是超时，ehsm_host库仅使用这个判断结果来决定操作是否继续
 * 进行。
 *
 * @param timer 计时器ID（可能仅是创建时的系统时钟值）
 * @return  是否超时
 */
bool_t ehsm_port_is_timeout(ehsm_port_timer_t timer);

/**
 * @brief 提供一个printf相同的ehsm_port_printf调用宏，用来打印数据。
 */
#ifndef ehsm_port_printf
#define ehsm_port_printf(...)
#endif

#endif
