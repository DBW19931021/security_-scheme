#include <string.h>

#include "test_types.h"
#include "version.h"

uint16_t test_cmd_echo(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    memcpy(rsp->val, cmd->val, cmd->len);
    rsp->len = cmd->len;
    return RSP_OK;
}

uint16_t test_cmd_get_server_version(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    (void)cmd;
    rsp->len = 3;
    rsp->val[0] = TEST_SYS_VER_MAJOR;
    rsp->val[1] = TEST_SYS_VER_MINOR;
    rsp->val[2] = TEST_SYS_VER_PATCH;
    return RSP_OK;
}

typedef struct {
    uint32_t addr;
    uint32_t byte;
    uint32_t size;
} share_memset_val_st;

uint16_t test_cmd_share_memset(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(share_memset_val_st)) {
        // 4 bytes addr, 4 bytes byte value, 4 bytes size
        return RSP_ERR_DATA_LENGTH;
    }

    (void)rsp;
    share_memset_val_st *val = (share_memset_val_st *)cmd->val;
    memset((void *)val->addr, (uint8_t)val->byte, val->size);
    return RSP_OK;
}

typedef struct {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint32_t size;
} share_memcpy_val_st;

uint16_t test_cmd_share_memcpy(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(share_memcpy_val_st)) {
        // 4 bytes src_addr, 4 bytes dst_addr, 4 bytes size
        return RSP_ERR_DATA_LENGTH;
    }

    (void)rsp;
    share_memcpy_val_st *val = (share_memcpy_val_st *)cmd->val;
    memcpy((void *)val->dst_addr, (const void *)val->src_addr, val->size);
    return RSP_OK;
}

typedef struct {
    uint32_t addr;
    uint16_t len;
} read_mem_val_st;

uint16_t test_cmd_read_memory(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 6) {
        // 4 bytes addr and 2 bytes length
        return RSP_ERR_DATA_LENGTH;
    }

    read_mem_val_st *val = (read_mem_val_st *)cmd->val;
    memcpy(rsp->val, (const void *)val->addr, (uint32_t)val->len);
    rsp->len = val->len;
    return RSP_OK;
}

typedef struct {
    uint32_t addr;
    uint8_t data;
} write_mem_val_st;

uint16_t test_cmd_write_memory(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len <= 4) {
        return RSP_ERR_DATA_LENGTH;
    }
    (void)rsp;
    write_mem_val_st *val = (write_mem_val_st *)cmd->val;
    memcpy((void *)val->addr, &cmd->val[4], cmd->len - 4);
    return RSP_OK;
}

uint16_t test_cmd_get_word(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 4) {
        return RSP_ERR_DATA_LENGTH;
    }

    uint32_t addr = *((uint32_t *)cmd->val);
    if ((addr & 0x03) != 0) {
        // check alignment
        return RSP_ERR_DATA_VALUE;
    }

    *((uint32_t *)rsp->val) = *((const uint32_t *)addr);
    rsp->len = 4;
    return RSP_OK;
}

uint16_t test_cmd_set_word(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != 8) {
        return RSP_ERR_DATA_LENGTH;
    }
    (void)rsp;
    uint32_t addr = *((uint32_t *)cmd->val);
    if ((addr & 0x03) != 0) {
        // check alignment
        return RSP_ERR_DATA_VALUE;
    }

    *((uint32_t *)addr) = *((const uint32_t *)&cmd->val[4]);
    return RSP_OK;
}

typedef struct {
    uint32_t write_addr;
    uint32_t write_val;
    uint32_t wait_addr;
    uint32_t wait_mask;
    uint32_t timeout_count;
} set_wait_st;

uint16_t test_cmd_set_word_and_wait(const test_cmd_st *cmd, test_rsp_st *rsp)
{
    if (cmd->len != sizeof(set_wait_st)) {
        return RSP_ERR_DATA_LENGTH;
    }

    const set_wait_st *set_wait = (const set_wait_st *)&cmd->val[0];
    if ((set_wait->write_addr & 0x03) != 0) {
        return RSP_ERR_DATA_VALUE;
    }
    if ((set_wait->wait_addr & 0x03) != 0) {
        return RSP_ERR_DATA_VALUE;
    }

    *((volatile uint32_t *)set_wait->write_addr) = set_wait->write_val;

    rsp->val[0] = 0;
    for (uint32_t i = 0; i < set_wait->timeout_count; i++) {
        if ((set_wait->wait_mask & *((volatile uint32_t *)set_wait->wait_addr)) == set_wait->wait_mask) {
            rsp->val[0] = 1;
            break;
        }
    }

    rsp->len = 1;
    return RSP_OK;
}
