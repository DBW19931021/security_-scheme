#include "speed_test.h"
#include "api.h"
#include <string.h>
#include <time.h>

static const uint32_t test_sizes[] = { 64, 256, 1024, 4096, 16 * 1024, 64 * 1024, 128 * 1024, 250 * 1024 };

static const uint32_t test_sizes_count = sizeof(test_sizes) / sizeof(test_sizes[0]);

static uint32_t test_mac_single(
    speed_test_ctx_t *ctx, const char *ske_algo_name, const char *mac_mode_name, uint32_t data_size)
{
    if (!ctx || !ske_algo_name || !mac_mode_name)
        return 1;

    ehsm_port_printf("Testing MAC: %s-%s, size: %u bytes\n", ske_algo_name, mac_mode_name, data_size);

    uint32_t ske_algo_id = get_algo_id_by_name(ske_algos, ske_algo_name);
    if (ske_algo_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown SKE algorithm: %s\n", ske_algo_name);
        return 1;
    }

    uint32_t mac_mode_id = get_algo_id_by_name((const algo_map_t *)mac_modes, mac_mode_name);
    if (mac_mode_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown MAC mode: %s\n", mac_mode_name);
        return 1;
    }

    // Skip incompatible combinations
    if ((strstr(ske_algo_name, "DES") || strstr(ske_algo_name, "3DES")) && (mac_mode_id == EHSM_MAC_MODE_GMAC)) {
        ehsm_port_printf("Skipping incompatible combination: %s with GMAC\n", ske_algo_name);
        return 0;
    }

    // Determine key type and MAC size
    uint32_t key_type;
    uint32_t mac_size;

    if (strcmp(ske_algo_name, "AES_128") == 0) {
        key_type = EHSM_KEY_TYPE_AES_128;
        mac_size = 16;
    } else if (strcmp(ske_algo_name, "AES_192") == 0) {
        key_type = EHSM_KEY_TYPE_AES_192;
        mac_size = 16;
    } else if (strcmp(ske_algo_name, "AES_256") == 0) {
        key_type = EHSM_KEY_TYPE_AES_256;
        mac_size = 16;
    } else if (strcmp(ske_algo_name, "SM4") == 0) {
        key_type = EHSM_KEY_TYPE_SM4;
        mac_size = 16;
    } else if (strcmp(ske_algo_name, "DES") == 0) {
        key_type = EHSM_KEY_TYPE_DES;
        mac_size = 8;
    } else if (strcmp(ske_algo_name, "3DES_128") == 0 || strcmp(ske_algo_name, "3DES_192") == 0) {
        key_type = strstr(ske_algo_name, "128") ? EHSM_KEY_TYPE_TDES_128 : EHSM_KEY_TYPE_TDES_192;
        mac_size = 8;
    } else {
        ehsm_port_printf("Unsupported SKE algorithm for MAC: %s\n", ske_algo_name);
        return 1;
    }

    double total_time = 0.0;
    uint32_t success_count = 0;

    // Generate symmetric key for MAC
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_gen_key(ctx->ehsm_ctx, key_type,
        EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY | EHSM_KEY_PRIV_IMPORT_PLAIN, 0, 0, 0, 0, &key_handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("Key generation failed for %s: 0x%08x\n", ske_algo_name, ret);
        return 1;
    }

    for (uint32_t loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        double start = ehsm_port_get_time_ms();

        if (mac_mode_id == EHSM_MAC_MODE_GMAC) {
            // GMAC needs IV
            ret = ehsm_mac_onepass_gen(ctx->ehsm_ctx, ske_algo_id, mac_mode_id, key_handle,
                ehsm_port_raddr_to_addr(ctx->data1_addr), 12, // 12-byte IV for GMAC
                ehsm_port_raddr_to_addr(ctx->data2_addr), data_size, ehsm_port_raddr_to_addr(ctx->data2_addr),
                mac_size);
        } else {
            // Other MAC modes don't need IV
            ret = ehsm_mac_onepass_gen(ctx->ehsm_ctx, ske_algo_id, mac_mode_id, key_handle, NULL, 0, // No IV
                ehsm_port_raddr_to_addr(ctx->data2_addr), data_size, ehsm_port_raddr_to_addr(ctx->data2_addr),
                mac_size);
        }

        double end = ehsm_port_get_time_ms();
        double loop_time = end - start;

        if (ret == EHSM_OK && loop_time >= 0) {
            total_time += (loop_time > 0) ? loop_time : 0.1;
            success_count++;
        }
    }

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for %s-%s\n", ske_algo_name, mac_mode_name);
        return 1;
    }

    double avg_time = total_time / success_count;
    double speed_5m = calculate_speed_mbps(data_size, avg_time);
    double speed_1g = speed_5m * 200.0;

    speed_test_result_t result = { 0 };
    snprintf(result.algo_name, sizeof(result.algo_name), "%s-%s", ske_algo_name, mac_mode_name);
    result.data_size = data_size;
    result.is_encrypt = false;
    result.loop_count = success_count;
    result.time_ms = avg_time;
    result.speed_5m_mbps = speed_5m;
    result.speed_1g_mbps = speed_1g;

    return speed_test_log_result(ctx, &result);
}

static uint32_t test_hmac_single(speed_test_ctx_t *ctx, const char *hash_algo_name, uint32_t data_size)
{
    if (!ctx || !hash_algo_name)
        return 1;

    ehsm_port_printf("Testing HMAC: %s, size: %u bytes\n", hash_algo_name, data_size);

    uint32_t hash_algo_id = get_algo_id_by_name(hash_algos, hash_algo_name);
    if (hash_algo_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown hash algorithm: %s\n", hash_algo_name);
        return 1;
    }

    // Determine HMAC size based on hash algorithm
    uint32_t hmac_size;
    uint32_t hmac_key_size;

    if (strcmp(hash_algo_name, "MD5") == 0) {
        hmac_size = 16;
        hmac_key_size = 64; // HMAC-MD5 key size
    } else if (strcmp(hash_algo_name, "SHA1") == 0) {
        hmac_size = 20;
        hmac_key_size = 64; // HMAC-SHA1 key size
    } else if (strcmp(hash_algo_name, "SHA224") == 0) {
        hmac_size = 28;
        hmac_key_size = 64; // HMAC-SHA224 key size
    } else if (strcmp(hash_algo_name, "SHA256") == 0) {
        hmac_size = 32;
        hmac_key_size = 64; // HMAC-SHA256 key size
    } else if (strcmp(hash_algo_name, "SHA384") == 0) {
        hmac_size = 48;
        hmac_key_size = 128; // HMAC-SHA384 key size
    } else if (strcmp(hash_algo_name, "SHA512") == 0) {
        hmac_size = 64;
        hmac_key_size = 128; // HMAC-SHA512 key size
    } else if (strcmp(hash_algo_name, "SM3") == 0) {
        hmac_size = 32;
        hmac_key_size = 64; // HMAC-SM3 key size
    } else if (strstr(hash_algo_name, "SHA3_224") != NULL) {
        hmac_size = 28;
        hmac_key_size = 64; // HMAC-SHA3-224 key size
    } else if (strstr(hash_algo_name, "SHA3_256") != NULL) {
        hmac_size = 32;
        hmac_key_size = 64; // HMAC-SHA3-256 key size
    } else if (strstr(hash_algo_name, "SHA3_384") != NULL) {
        hmac_size = 48;
        hmac_key_size = 128; // HMAC-SHA3-384 key size
    } else if (strstr(hash_algo_name, "SHA3_512") != NULL) {
        hmac_size = 64;
        hmac_key_size = 128; // HMAC-SHA3-512 key size
    } else {
        ehsm_port_printf("Unsupported hash algorithm for HMAC: %s\n", hash_algo_name);
        return 1;
    }

    double total_time = 0.0;
    uint32_t success_count = 0;

    // Generate HMAC key
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_gen_key(ctx->ehsm_ctx, EHSM_KEY_TYPE_HMAC, EHSM_KEY_PRIV_SIGN | EHSM_KEY_PRIV_VERIFY, 0,
        hmac_key_size, 0, 0, &key_handle);
    if (ret != EHSM_OK) {
        ehsm_port_printf("HMAC key generation failed for %s: 0x%08x\n", hash_algo_name, ret);
        return 1;
    }

    for (uint32_t loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        double start = ehsm_port_get_time_ms();

        ret = ehsm_hmac_onepass_gen(ctx->ehsm_ctx, hash_algo_id, key_handle, ehsm_port_raddr_to_addr(ctx->data2_addr),
            data_size, ehsm_port_raddr_to_addr(ctx->data2_addr), hmac_size);

        double end = ehsm_port_get_time_ms();
        double loop_time = end - start;

        if (ret == EHSM_OK && loop_time >= 0) {
            total_time += (loop_time > 0) ? loop_time : 0.1;
            success_count++;
        }
    }

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for HMAC-%s\n", hash_algo_name);
        return 1;
    }

    double avg_time = total_time / success_count;
    double speed_5m = calculate_speed_mbps(data_size, avg_time);
    double speed_1g = speed_5m * 200.0;

    speed_test_result_t result = { 0 };
    snprintf(result.algo_name, sizeof(result.algo_name), "HMAC-%s", hash_algo_name);
    result.data_size = data_size;
    result.is_encrypt = false;
    result.loop_count = success_count;
    result.time_ms = avg_time;
    result.speed_5m_mbps = speed_5m;
    result.speed_1g_mbps = speed_1g;

    return speed_test_log_result(ctx, &result);
}

uint32_t test_mac_speed(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("\n=== Starting MAC Speed Tests ===\n");

    // Test algorithms and modes as per Python test file
    const char *test_ske_algos[] = { "AES_128", "AES_192", "AES_256", "SM4", "DES", "3DES_128", "3DES_192" };
    const char *test_mac_modes[] = { "CMAC", "CBC_MAC", "GMAC" };

    // HMAC test algorithms (hash algorithms)
    const char *test_hash_algos[] = { "MD5", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "SM3", "SHA3_224",
        "SHA3_256", "SHA3_384", "SHA3_512" };

    const uint32_t ske_algo_count = sizeof(test_ske_algos) / sizeof(test_ske_algos[0]);
    const uint32_t mac_mode_count = sizeof(test_mac_modes) / sizeof(test_mac_modes[0]);
    const uint32_t hash_algo_count = sizeof(test_hash_algos) / sizeof(test_hash_algos[0]);

    int total_tests = 0;
    int failed_tests = 0;

    // Test symmetric encryption MAC modes (CMAC, CBC_MAC, GMAC)
    for (uint32_t ske_idx = 0; ske_idx < ske_algo_count; ske_idx++) {
        for (uint32_t mac_idx = 0; mac_idx < mac_mode_count; mac_idx++) {
            for (uint32_t size_idx = 0; size_idx < test_sizes_count; size_idx++) {
                total_tests++;

                uint32_t ret
                    = test_mac_single(ctx, test_ske_algos[ske_idx], test_mac_modes[mac_idx], test_sizes[size_idx]);

                if (ret != 0) {
                    failed_tests++;
                }
            }
        }
    }

    // Test HMAC modes (based on hash algorithms)
    for (uint32_t hash_idx = 0; hash_idx < hash_algo_count; hash_idx++) {
        for (uint32_t size_idx = 0; size_idx < test_sizes_count; size_idx++) {
            total_tests++;

            uint32_t ret = test_hmac_single(ctx, test_hash_algos[hash_idx], test_sizes[size_idx]);

            if (ret != 0) {
                failed_tests++;
            }
        }
    }

    ehsm_port_printf("MAC Speed Tests completed: %d total, %d failed\n", total_tests, failed_tests);
    return failed_tests == 0 ? 0 : 1;
}