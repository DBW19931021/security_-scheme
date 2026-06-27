#ifndef EHSM_DEMO_MEM_H
#define EHSM_DEMO_MEM_H

#include "ehsmdrv/basic/api.h"

ehsm_ctx_st *ehsm_demo_get_ctx(void);
ehsm_session_st *ehsm_demo_get_session(void);

// 每个1K
uint8_t* ehsm_demo_get_buffer(uint32_t id);

#endif // EHSM_DEMO_MEM_H
