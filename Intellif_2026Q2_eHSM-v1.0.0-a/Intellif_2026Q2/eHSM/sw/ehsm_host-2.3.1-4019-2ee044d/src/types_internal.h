#ifndef EHSM_TYPES_INTERNAL_H
#define EHSM_TYPES_INTERNAL_H

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/mailbox.h"
#include "ehsmdrv/basic/types.h"

// usage: macro static assert
#define _EXPAND_NAME(x)        _EXPAND_NAME2(x, __LINE__)
#define _EXPAND_NAME2(x, line) _EXPAND_NAME3(x, line)
#define _EXPAND_NAME3(x, line) x##line

#define STATIC_ASSERT(expr)                        \
    enum _EXPAND_NAME(_static_assert_enum) {       \
        _EXPAND_NAME(_VAL) = 1 / (uint32_t)(expr), \
    }

#define EHSM_CTX_MAGIC 0x5aa5aa55U

typedef struct {
    uint32_t magic;
    uint8_t mb_ch; // mailbox channel
    bool_t async;  // async
    bool_t inited;

    ehsm_rsp_cb_func_t callback;
    ehsm_mb_packet_st packet;

    // The content below will be cleared on each call
    void *result1;
    void *result2;
    volatile bool_t responded;        // received response, shared with ISR
    uint8_t _res[4 - sizeof(bool_t)]; // make cmd_buf 4 bytes aligned
    ehsm_mb_cmd_st cmd_buf;
    ehsm_mb_rsp_st rsp_buf;
} ehsm_ctx_intl_st;

ehsm_drv_mode_e get_driver_mode(void);

#endif // EHSM_TYPES_INTERNAL_H
