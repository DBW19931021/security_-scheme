#ifndef EHSM_MAILBOX_DRIVER_H
#define EHSM_MAILBOX_DRIVER_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "config.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
typedef void (*mailbox_recv_cb)(uint32_t channel);

// Number of supported channels of mailbox
#define MAX_MAILBOX_CHANNEL_ID (CONFIG_EHSM_MB_CHANNEL_COUNT - 1U)

// Word size of register MB_H2S_INFO
#define MAILBOX_H2S_INFO_SIZE (2U)

// Word size of register MB_S2H_INFO
#define MAILBOX_S2H_INFO_SIZE (2U)

// Mailbox base register address
#define MAILBOX_REG_BASE_ADDRESS 0x30C00000UL

#ifndef CONFIG_UNIT_TEST
// register address for
#define MAILBOX_REG_FOR_ALL_INT *((volatile uint32_t *)(0x30C00480UL))
#endif

#define MAILBOX_CHANNEL_REG_OFFSET 0x1000UL

// Mailbox base register end address
#define MAILBOX_REG_BASE_END_ADDRESS (MAILBOX_REG_BASE_ADDRESS + (MAILBOX_CHANNEL_REG_OFFSET * (CONFIG_EHSM_MB_CHANNEL_COUNT)))
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t mailbox_init(mailbox_recv_cb cb);

uint32_t mailbox_read(uint32_t channel, uint32_t *data, uint32_t size);

uint32_t mailbox_write(uint32_t channel, const uint32_t *data, uint32_t size);

uint32_t mailbox_read_send_notify(uint32_t channel, uint32_t *notify);

uint32_t mailbox_read_recv_notify(uint32_t channel, uint32_t *notify);

uint32_t mailbox_clear_recv_notify(uint32_t channel, uint32_t notify);

uint32_t mailbox_set_send_notify(uint32_t channel, uint32_t notify);
#endif
