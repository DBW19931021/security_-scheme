#include "speed_test.h"
#include "api.h"
#include <string.h>
#include <time.h>

static const uint32_t test_sizes[] = { 64, 256, 1024, 4096, 16 * 1024, 64 * 1024, 128 * 1024, 250 * 1024 };

static const uint32_t test_sizes_count = sizeof(test_sizes) / sizeof(test_sizes[0]);

static uint32_t test_aead_single(
    speed_test_ctx_t *ctx, const char *algo_name, const char *mode_name, uint32_t data_size, bool is_encrypt)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("Testing AEAD: %s, mode: %s, size: %u, %s\n", algo_name, mode_name, data_size,
        is_encrypt ? "encrypt" : "decrypt");

    uint32_t algo_id = get_algo_id_by_name(ske_algos, algo_name);
    if (algo_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown SKE algorithm: %s\n", algo_name);
        return 1;
    }
    uint32_t mode = get_algo_id_by_name(aead_modes, mode_name);
    if (mode == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown aead mode: %s\n", mode_name);
        return 1;
    }

    // Get key type ID
    uint32_t key_type_id = get_algo_id_by_name(key_types, algo_name);
    if (key_type_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown SKE algorithm: %s\n", algo_name);
        return 1;
    }

    uint32_t tag_size = 16;
    double total_time = 0.0;
    uint32_t success_count = 0;

    // Generate AES key for AEAD
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_gen_key(ctx->ehsm_ctx, key_type_id,
        EHSM_KEY_PRIV_ENCRYPT | EHSM_KEY_PRIV_DECRYPT | EHSM_KEY_PRIV_IMPORT_PLAIN, 0U, 0U, NULL, 0U, &key_handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("AEAD key generation failed\n");
        return 1;
    }

    for (int loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        double start = ehsm_port_get_time_ms();

        if (is_encrypt) {
            ret = ehsm_aead_onepass_enc(ctx->ehsm_ctx, algo_id, mode, key_handle,
                ehsm_port_raddr_to_addr(ctx->data2_addr), 12,             // nonce
                ehsm_port_raddr_to_addr(ctx->data2_addr + 12), 16,        // aad
                ehsm_port_raddr_to_addr(ctx->data1_addr), data_size,      // plaintext
                ehsm_port_raddr_to_addr(ctx->data1_addr),                 // ciphertext
                ehsm_port_raddr_to_addr(ctx->data2_addr + 28), tag_size); // tag
        } else {
            bool_t verify_result;
            ret = ehsm_aead_onepass_dec(ctx->ehsm_ctx, algo_id, mode, key_handle,
                ehsm_port_raddr_to_addr(ctx->data2_addr), 12,            // nonce
                ehsm_port_raddr_to_addr(ctx->data2_addr + 12), 16,       // aad
                ehsm_port_raddr_to_addr(ctx->data1_addr), data_size,     // ciphertext
                ehsm_port_raddr_to_addr(ctx->data1_addr),                // plaintext
                ehsm_port_raddr_to_addr(ctx->data2_addr + 28), tag_size, // tag
                &verify_result);
        }

        double end = ehsm_port_get_time_ms();
        double loop_time = end - start;

        if (ret == EHSM_OK && loop_time >= 0) {
            total_time += (loop_time > 0) ? loop_time : 0.1;
            success_count++;
        }
    }

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for %s %s\n", algo_name, is_encrypt ? "enc" : "dec");
        return 1;
    }

    double avg_time = total_time / success_count;
    double speed_5m = calculate_speed_mbps(data_size, avg_time);
    double speed_1g = speed_5m * 200.0;

    speed_test_result_t result = { 0 };
    strncpy(result.algo_name, algo_name, sizeof(result.algo_name) - 1);
    strncpy(result.mode_name, is_encrypt ? "enc" : "dec", sizeof(result.mode_name) - 1);
    result.data_size = data_size;
    result.is_encrypt = is_encrypt;
    result.loop_count = success_count;
    result.time_ms = avg_time;
    result.speed_5m_mbps = speed_5m;
    result.speed_1g_mbps = speed_1g;

    return speed_test_log_result(ctx, &result);
}

uint32_t test_aead_speed(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("\n=== Starting AEAD Speed Tests ===\n");

    const bool encrypt_options[] = { true, false };
    const char *test_algos[] = { "AES_128", "AES_192", "AES_256", "SM4" };
    const char *test_modes[] = { "CCM", "GCM" };

    const uint32_t algo_count = sizeof(test_algos) / sizeof(test_algos[0]);
    const uint32_t encrypt_count = sizeof(encrypt_options) / sizeof(encrypt_options[0]);
    const uint32_t aead_mode_count = sizeof(test_modes) / sizeof(test_modes[0]);

    int total_tests = 0;
    int failed_tests = 0;

    for (uint32_t algo_idx = 0; algo_idx < algo_count; algo_idx++) {
        for (uint32_t size_idx = 0; size_idx < test_sizes_count; size_idx++) {
            for (uint32_t enc_idx = 0; enc_idx < encrypt_count; enc_idx++) {
                for (uint32_t mode_idx = 0; mode_idx < aead_mode_count; mode_idx++) {
                    total_tests++;

                    uint32_t ret = test_aead_single(ctx, test_algos[algo_idx], test_modes[mode_idx],
                        test_sizes[size_idx], encrypt_options[enc_idx]);

                    if (ret != 0) {
                        failed_tests++;
                    }
                }
            }
        }
    }

    ehsm_port_printf("AEAD Speed Tests completed: %d total, %d failed\n", total_tests, failed_tests);
    return failed_tests == 0 ? 0 : 1;
}