#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stdint.h>

typedef struct {
    uint16_t cmd_id;
    uint16_t len;
    uint8_t val[65536];
} test_cmd_st;

typedef struct {
    uint32_t time;      // 由框架填充
    uint16_t rsp_id;    // 由框架填充
    uint16_t len;       // 由处理函数填充
    uint8_t val[65536]; // 由处理函数填充
} test_rsp_st;

typedef uint16_t (*cmd_proc_func_t)(const test_cmd_st *cmd, test_rsp_st *rsp);

#define RSP_OK              0
#define RSP_ERR_UNKNOWN_CMD 1
#define RSP_ERR_DATA_LENGTH 2
#define RSP_ERR_DATA_VALUE  3

#include "cmds.inl"

#endif // TEST_TYPES_H
