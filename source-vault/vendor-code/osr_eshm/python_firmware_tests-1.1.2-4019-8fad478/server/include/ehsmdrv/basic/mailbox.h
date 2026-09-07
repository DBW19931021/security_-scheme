#ifndef EHSM_MAILBOX_H
#define EHSM_MAILBOX_H

#include "types.h"
#include "api.h"

#pragma pack(1)
typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint32_t data[39];
} ehsm_mb_cmd_st;

typedef struct {
    uint32_t ret_code;
    uint32_t data[4];
} ehsm_mb_rsp_st;

typedef struct {
    raddr_t cmd_addr;
    raddr_t rsp_addr;

} ehsm_mb_packet_st;
#pragma pack()

typedef void (*ehsm_mb_interrupt_func_t)(uint32_t channel, raddr_t packet_addr);

uint32_t ehsm_mb_init(ehsm_mb_interrupt_func_t func, ehsm_drv_mode_e drv_mode);

uint32_t ehsm_mb_send(uint32_t channel, raddr_t packet);

uint32_t ehsm_mb_poll(uint32_t channel);

void ehsm_mb_int_handler(uint32_t channel);
void ehsm_mb_enable_int(uint32_t channel);

#endif // EHSM_MAILBOX_H
