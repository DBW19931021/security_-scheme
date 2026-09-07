#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/test_api.h"
#include "ehsmdrv/basic/types.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

#include "mb.h"
#include "types_internal.h"

uint32_t ehsm_test_read_memory(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint8_t *buf, uint32_t ehsm_src_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_test_read_memory_st *cmd = (mb_cmd_test_read_memory_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_TEST_READ_MEMORY);
    cmd->ehsm_src_addr = ehsm_src_addr;
    cmd->host_dst_addr = ehsm_port_addr_to_raddr(buf);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_test_write_memory(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM const uint8_t *src_data, uint32_t ehsm_dest_addr, uint32_t size)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_test_write_memory_st *cmd = (mb_cmd_test_write_memory_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_TEST_WRITE_MEMORY);
    cmd->ehsm_dst_addr = ehsm_dest_addr;
    cmd->host_src_addr = ehsm_port_addr_to_raddr(src_data);
    cmd->size = size;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_test_jump_to_addr(EHSM_SHM ehsm_ctx_st *ctx, EHSM_SHM uint32_t addr)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_test_jump_to_addr_st *cmd = (mb_cmd_test_jump_to_addr_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_TEST_WRITE_MEMORY);
    cmd->addr = addr;

    return ehsm_send_cmd(ctx_intl);
}

uint32_t ehsm_test_jump_to_loop(EHSM_SHM ehsm_ctx_st *ctx)
{
    uint32_t ret = ehsm_check_ctx(ctx);
    if (ret != EHSM_OK) {
        return ret;
    }

    ehsm_ctx_reset(ctx);
    ehsm_ctx_intl_st *ctx_intl = (ehsm_ctx_intl_st *)ctx;

    mb_cmd_test_jump_to_loop_st *cmd = (mb_cmd_test_jump_to_loop_st *)&ctx_intl->cmd_buf;
    ehsm_set_cmd_id((ehsm_mb_cmd_st *)cmd, MB_CMD_ID_TEST_JUMP_TO_LOOP);

    return ehsm_send_cmd(ctx_intl);
}
