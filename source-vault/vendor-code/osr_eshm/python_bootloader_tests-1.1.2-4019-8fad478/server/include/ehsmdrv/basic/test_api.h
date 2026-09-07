#ifndef TEST_MB_H
#define TEST_MB_H

#include "ehsmdrv/basic/types.h"
#include "ehsmdrv/basic/api.h"

#pragma pack(1)

#define MB_CMD_ID_TEST_READ_MEMORY  0xfe80u
#define MB_CMD_ID_TEST_WRITE_MEMORY 0xfe81u
#define MB_CMD_ID_TEST_JUMP_TO_ADDR 0xfe82u
#define MB_CMD_ID_TEST_JUMP_TO_LOOP 0xfe83u

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t ehsm_src_addr;
    uint32_t size;
    raddr_t host_dst_addr;
} mb_cmd_test_read_memory_st;

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t ehsm_dst_addr;
    uint32_t size;
    raddr_t host_src_addr;
} mb_cmd_test_write_memory_st;

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
    uint32_t addr;
} mb_cmd_test_jump_to_addr_st;

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint8_t reserved0[8];
} mb_cmd_test_jump_to_loop_st;

typedef struct {
    uint32_t ret_code; /* [offset: 0] 0x0000A55AU for success, other values for failure. */
} mb_rsp_test_result_st;

uint32_t ehsm_test_read_memory(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size);
uint32_t ehsm_test_write_memory(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size);
uint32_t ehsm_test_jump_to_addr(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint32_t addr);
uint32_t ehsm_test_jump_to_loop(EHSM_SHM ehsm_ctx_st *ctx);

#pragma pack()

#endif // TEST_MB_H
