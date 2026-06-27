#include "speed_test.h"
#include "api.h"
#include <string.h>
#include <time.h>

static const uint32_t test_sizes[] = { 64, 256, 1024, 4096, 16 * 1024, 64 * 1024, 128 * 1024, 250 * 1024 };

static const uint32_t test_sizes_count = sizeof(test_sizes) / sizeof(test_sizes[0]);

static uint32_t test_chacha_single(speed_test_ctx_t *ctx, const char *algo_name, uint32_t data_size)
{
    if (!ctx || !algo_name)
        return 1;

    ehsm_port_printf("Testing ChaCha: %s, size: %u bytes\n", algo_name, data_size);

    uint32_t tag_size = 16;
    uint32_t constant = 0x61707865; // ChaCha20 constant

    double total_time = 0.0;
    uint32_t success_count = 0;

    // Generate ChaCha key
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_gen_key(ctx->ehsm_ctx, EHSM_KEY_TYPE_CHACHA,
        EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT | EHSM_KEY_PRIV_IMPORT_PLAIN, 0U, 0U, NULL, 0U, &key_handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("ChaCha key generation failed\n");
        return 1;
    }

    for (int loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        double start = ehsm_port_get_time_ms();

        ret = ehsm_chacha_onepass_enc(ctx->ehsm_ctx, key_handle, ehsm_port_raddr_to_addr(ctx->data2_addr), 8, // nonce
            ehsm_port_raddr_to_addr(ctx->data2_addr + 12), 16,                                                // aad
            constant, ehsm_port_raddr_to_addr(ctx->data1_addr), data_size, // plaintext
            ehsm_port_raddr_to_addr(ctx->data1_addr),                      // ciphertext
            ehsm_port_raddr_to_addr(ctx->data2_addr + 28), tag_size);      // tag

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

uint32_t test_chacha_speed(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("\n=== Starting ChaCha Speed Tests ===\n");

    const char *test_algos[] = { "CHACHA20", "XCHACHA20" };
    const uint32_t algo_count = sizeof(test_algos) / sizeof(test_algos[0]);

    int total_tests = 0;
    int failed_tests = 0;

    for (uint32_t algo_idx = 0; algo_idx < algo_count; algo_idx++) {
        for (uint32_t size_idx = 0; size_idx < test_sizes_count; size_idx++) {
            total_tests++;

            uint32_t ret = test_chacha_single(ctx, test_algos[algo_idx], test_sizes[size_idx]);

            if (ret != 0) {
                failed_tests++;
            }
        }
    }

    ehsm_port_printf("ChaCha Speed Tests completed: %d total, %d failed\n", total_tests, failed_tests);
    return failed_tests == 0 ? 0 : 1;
}