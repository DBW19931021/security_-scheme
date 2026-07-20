#include <stddef.h>
#include <string.h>
#include "common/mem.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

typedef struct {
    uint8_t buffer[1024];
} ehsm_demo_buffer_st;

// 这里的内存管理仅用于demo，实际内存管理算法可能非常复杂
typedef struct {
    ehsm_ctx_st ctx;
    ehsm_session_st session;
    ehsm_demo_buffer_st buffers[999];
} ehsm_demo_share_mem_st;

#define SHARE_MEM ((ehsm_demo_share_mem_st *)(EHSM_PORT_SHARE_MEM_BASE_ADDR))

ehsm_ctx_st *ehsm_demo_get_ctx(void)
{
    return &SHARE_MEM->ctx;
}

ehsm_session_st *ehsm_demo_get_session(void)
{
    ehsm_session_st *session = &SHARE_MEM->session;
    memset((void *)session, 0x0, sizeof(ehsm_session_st));

    return session;
}

uint8_t *ehsm_demo_get_buffer(uint32_t id)
{
    uint8_t *buffer = NULL;

    if (offsetof(ehsm_demo_share_mem_st, buffers[id]) + sizeof(ehsm_demo_buffer_st) <= EHSM_PORT_SHARE_MEM_SIZE) {
        buffer = SHARE_MEM->buffers[id].buffer;
        memset((void *)buffer, 0x0, sizeof(ehsm_demo_buffer_st));
    }

    return buffer;
}
