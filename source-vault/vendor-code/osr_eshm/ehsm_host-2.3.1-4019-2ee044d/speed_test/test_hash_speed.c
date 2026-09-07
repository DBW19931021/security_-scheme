#include "speed_test.h"
#include "api.h"
#include <string.h>
#include <time.h>

static const uint32_t test_sizes[] = { 64, 256, 1024, 4096, 16 * 1024, 64 * 1024, 128 * 1024, 250 * 1024 };

static const uint32_t test_sizes_count = sizeof(test_sizes) / sizeof(test_sizes[0]);

static uint32_t test_hash_single(speed_test_ctx_t *ctx, const char *algo_name, uint32_t data_size)
{
    if (!ctx || !algo_name)
        return 1;

    uint32_t algo_id = get_algo_id_by_name(hash_algos, algo_name);
    if (algo_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown hash algorithm: %s\n", algo_name);
        return 1;
    }

    ehsm_port_printf("Testing Hash: %s, size: %u bytes\n", algo_name, data_size);

    uint32_t digest_size = 64; // Maximum digest size

    double total_time = 0.0;
    uint32_t success_count = 0;

    for (uint32_t loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        double start = ehsm_port_get_time_ms();

        uint32_t ret
            = ehsm_hash_onepass(ctx->ehsm_ctx, (ehsm_hash_algo_e)algo_id, ehsm_port_raddr_to_addr(ctx->data1_addr),
                data_size, ehsm_port_raddr_to_addr(ctx->data2_addr), &digest_size);

        double end = ehsm_port_get_time_ms();
        double loop_time = end - start;

        if (ret == EHSM_OK && loop_time >= 0) {
            total_time += (loop_time > 0) ? loop_time : 0.1;
            success_count++;
        }
    }

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for %s\n", algo_name);
        return 1;
    }

    double avg_time = total_time / success_count;
    double speed_5m = calculate_speed_mbps(data_size, avg_time);
    double speed_1g = speed_5m * 200.0;

    speed_test_result_t result = { 0 };
    strncpy(result.algo_name, algo_name, sizeof(result.algo_name) - 1);
    result.data_size = data_size;
    result.is_encrypt = false;
    result.loop_count = success_count;
    result.time_ms = avg_time;
    result.speed_5m_mbps = speed_5m;
    result.speed_1g_mbps = speed_1g;

    return speed_test_log_result(ctx, &result);
}

uint32_t test_hash_speed(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("\n=== Starting Hash Speed Tests ===\n");

    const char *test_algos[] = { "MD5", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "SM3", "SHA3_224", "SHA3_256",
        "SHA3_384", "SHA3_512" };

    const uint32_t algo_count = sizeof(test_algos) / sizeof(test_algos[0]);

    int total_tests = 0;
    int failed_tests = 0;

    for (uint32_t algo_idx = 0; algo_idx < algo_count; algo_idx++) {
        for (uint32_t size_idx = 0; size_idx < test_sizes_count; size_idx++) {
            total_tests++;

            uint32_t ret = test_hash_single(ctx, test_algos[algo_idx], test_sizes[size_idx]);

            if (ret != 0) {
                failed_tests++;
            }
        }
    }

    ehsm_port_printf("Hash Speed Tests completed: %d total, %d failed\n", total_tests, failed_tests);
    return failed_tests == 0 ? 0 : 1;
}