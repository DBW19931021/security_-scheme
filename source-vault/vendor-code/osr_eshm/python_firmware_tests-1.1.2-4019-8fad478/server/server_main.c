#include "strans_pro.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "test_types.h"

uint16_t dispatch_cmd(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->cmd_id >= CMD_MAX_COUNT) {
        return RSP_ERR_UNKNOWN_CMD;
    }
    return g_test_cmd_table[cmd->cmd_id](cmd, rsp);
}

uint32_t get_time_tick(void);
uint32_t calc_time_unit(uint32_t tick);
void init_mtime(void);

test_cmd_st g_cmd = { 0 };
test_rsp_st g_rsp = { 0 };

void server_main(void)
{
    uint32_t ret = stp_init(0, NULL);
    if (ret != STP_OK) {
        printf("init failed\n");
        return;
    }
    init_mtime();
    while (1) {
        memset(&g_cmd, 0, offsetof(test_cmd_st, val));
        memset(&g_rsp, 0, offsetof(test_rsp_st, val));

        ret = stp_recv((uint8_t *)&g_cmd, offsetof(test_cmd_st, val));
        if (ret != STP_OK) {
            continue;
        }
        ret = stp_recv(g_cmd.val, g_cmd.len);
        if (ret != STP_OK) {
            continue;
        }
        if (g_cmd.cmd_id < CMD_MAX_COUNT) {
           printf("cmd: %s, len: %u\n", g_test_cmd_names[g_cmd.cmd_id], g_cmd.len);
        }
        uint32_t start = get_time_tick();
        uint16_t rsp_code = dispatch_cmd(&g_cmd, &g_rsp);
        g_rsp.time = calc_time_unit(get_time_tick() - start);
        g_rsp.rsp_id = rsp_code;
        printf("rsp: %u, ret: %u\n", g_rsp.rsp_id, g_rsp.val[0]);

        stp_send((const uint8_t *)&g_rsp, offsetof(test_rsp_st, val) + g_rsp.len);
    }
}
// Default timing backend for RISC-V mtime. Ports can override these weak symbols.
#if defined(__riscv) || defined(__riscv__)
#define CLINT_REG_MEM_BASE (0x00000000 + 0x00000) /*!< clint memory space base addr. */
#define MTIMECTRL_EN_Pos 0U                           /*!< position of mtime enable field. */
#define MTIMECTRL_EN_Msk (0x01UL << MTIMECTRL_EN_Pos) /*!< bit mask of mtime enable field. */

typedef struct
{
    volatile uint32_t MTIME_LO;    /*!< address of mtime reg. */
    volatile uint32_t MTIME_HI;    /*!< address of mtime high part reg. */
    volatile uint32_t MTIMECMP_LO; /*!< address of mtime compare low part reg. */
    volatile uint32_t MTIMECMP_HI; /*!< address of mtime compare  high part reg. */
    volatile uint32_t MTIMECTRL;   /*!< address of mtime control reg. */
    volatile uint32_t RESERVED[3];
    volatile uint32_t MSIP;        /*!< address of machine software interrupt pendding control reg. */
} WING_CLINT_Type;

#define WING_CLINT ((volatile WING_CLINT_Type *)CLINT_REG_MEM_BASE)

#pragma GCC push_options
#pragma GCC optimize ("O0")
// 由于寄存器地址为0，必须禁用优化，否则读写寄存器会出问题

__attribute__((weak)) uint32_t get_time_tick(void)
{
    return WING_CLINT->MTIME_LO;
}

__attribute__((weak)) void init_mtime(void)
{
    WING_CLINT->MTIMECTRL |= MTIMECTRL_EN_Msk;
}

#pragma GCC pop_options    /* 恢复之前的优化级别 */

__attribute__((weak)) uint32_t calc_time_unit(uint32_t ticks)
{
    // mtime clock is CPU freq / 32 = 156250 Hz
    // 0.1ms = 15.625 mtime clock
    // x / 15.625 = x * 8 / 125
    // 计算 (8 * 2^32) / 125 = 274877906
    const uint64_t multiplier = ((uint64_t)8 << 32) / 125;

    // 使用64位乘法避免溢出，然后右移32位
    return (uint32_t)(((uint64_t)ticks * multiplier) >> 32);
}
#else
__attribute__((weak)) uint32_t get_time_tick(void)
{
    return 0U;
}

__attribute__((weak)) void init_mtime(void)
{
}

__attribute__((weak)) uint32_t calc_time_unit(uint32_t ticks)
{
    return ticks;
}
#endif

// 不需要非常准确，用于协议判断超时
// 使用 weak 属性，允许 port 提供自己的实现
__attribute__((weak)) uint32_t stp_get_time_ms(void)
{
    uint32_t tick = get_time_tick();
    return tick >> 7;
}
