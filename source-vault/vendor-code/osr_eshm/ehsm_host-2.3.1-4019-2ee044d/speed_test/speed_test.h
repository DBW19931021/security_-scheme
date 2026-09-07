#ifndef SPEED_TEST_H
#define SPEED_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/types.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"

#define MAX_ALGO_NAME_LEN  32
#define MAX_MODE_NAME_LEN  16
#define MAX_LOG_PATH_LEN   256
#define MAX_TEST_DATA_SIZE (250 * 1024)
#define DEFAULT_TEST_LOOPS 5

#define UNKNOWN_ALG_ID 0xFFFFFFFF

typedef enum {
    SPEED_TEST_SKE = 0,
    SPEED_TEST_HASH,
    SPEED_TEST_PKE,
    SPEED_TEST_MAC,
    SPEED_TEST_AEAD,
    SPEED_TEST_CHACHA,
    SPEED_TEST_MAX
} speed_test_type_e;

typedef struct {
    char algo_name[MAX_ALGO_NAME_LEN];
    char mode_name[MAX_MODE_NAME_LEN];
    uint32_t data_size;
    bool is_encrypt;
    uint32_t loop_count;
    double time_ms;
    double speed_5m_mbps;
    double speed_1g_mbps;
} speed_test_result_t;

typedef struct {
    uint32_t test_count;
    uint32_t pass_count;
    uint32_t fail_count;
    raddr_t data1_addr;
    raddr_t data2_addr;
    ehsm_ctx_st *ehsm_ctx;
    ehsm_session_st session;
} speed_test_ctx_t;

typedef struct {
    const char *name;
    uint32_t id;
} algo_map_t;

typedef struct {
    const char *name;
    uint32_t id;
} mode_map_t;

extern const algo_map_t ske_algos[];
extern const mode_map_t ske_modes[];
extern const algo_map_t hash_algos[];
extern const algo_map_t key_types[];
extern const algo_map_t aead_modes[];
extern const mode_map_t mac_modes[];

int speed_test_init(speed_test_ctx_t *ctx, speed_test_type_e type);
int speed_test_cleanup(speed_test_ctx_t *ctx);
uint32_t speed_test_log_result(speed_test_ctx_t *ctx, const speed_test_result_t *result);
int speed_test_setup_ehsm(void);
double calculate_speed_mbps(uint32_t data_size, double time_ms);
double calculate_speed_tps(double time_ms);
void get_current_timestamp(char *buffer, size_t size);
const char *get_algo_name_by_id(const algo_map_t *map, uint32_t id);
uint32_t get_algo_id_by_name(const algo_map_t *map, const char *name);

uint32_t test_ske_speed(speed_test_ctx_t *ctx);
uint32_t test_hash_speed(speed_test_ctx_t *ctx);
int test_pke_speed(speed_test_ctx_t *ctx);
uint32_t test_pke_cipher_speed(speed_test_ctx_t *ctx);
uint32_t test_mac_speed(speed_test_ctx_t *ctx);
uint32_t test_aead_speed(speed_test_ctx_t *ctx);
uint32_t test_chacha_speed(speed_test_ctx_t *ctx);

#endif // SPEED_TEST_H