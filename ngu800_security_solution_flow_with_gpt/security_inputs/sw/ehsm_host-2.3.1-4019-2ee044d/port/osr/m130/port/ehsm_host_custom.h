#ifndef EHSM_HOST_CUSTOM_H
#define EHSM_HOST_CUSTOM_H

#include <stdio.h>

#define EHSM_PORT_MAILBOX_CHANNEL_COUNT      16u
#define EHSM_PORT_MAILBOX_REG_BASE_ADDR      0x80000000u
#define EHSM_PORT_SHARE_MEM_BASE_ADDR        0x60000000u
#define EHSM_PORT_SHARE_MEM_SIZE             (256u * 1024u)
#define EHSM_PORT_OTP_SIZE                   1024
#define EHSM_PORT_MAILBOX_NOTE_WRITE_1_CLEAR 1

#define ehsm_port_printf(...) printf(__VA_ARGS__)

#endif // EHSM_HOST_CUSTOM_H
