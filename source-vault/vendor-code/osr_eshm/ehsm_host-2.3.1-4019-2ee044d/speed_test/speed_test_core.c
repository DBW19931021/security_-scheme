#include "speed_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

const algo_map_t ske_algos[] = { { "AES_128", EHSM_SYMM_ALGO_AES_128 }, { "AES_192", EHSM_SYMM_ALGO_AES_192 },
    { "AES_256", EHSM_SYMM_ALGO_AES_256 }, { "SM4", EHSM_SYMM_ALGO_SM4 }, { "DES", EHSM_SYMM_ALGO_DES },
    { "3DES_128", EHSM_SYMM_ALGO_TDES_128 }, { "3DES_192", EHSM_SYMM_ALGO_TDES_192 }, { NULL, 0 } };

const mode_map_t ske_modes[] = { { "ECB", EHSM_CIPHER_MODE_ECB }, { "CBC", EHSM_CIPHER_MODE_CBC },
    { "CFB", EHSM_CIPHER_MODE_CFB }, { "OFB", EHSM_CIPHER_MODE_OFB }, { "CTR", EHSM_CIPHER_MODE_CTR },
    { "XTS", EHSM_CIPHER_MODE_XTS }, { NULL, 0 } };

const algo_map_t hash_algos[] = { { "MD5", EHSM_HASH_ALGO_MD5 }, { "SHA1", EHSM_HASH_ALGO_SHA1 },
    { "SHA224", EHSM_HASH_ALGO_SHA224 }, { "SHA256", EHSM_HASH_ALGO_SHA256 }, { "SHA384", EHSM_HASH_ALGO_SHA384 },
    { "SHA512", EHSM_HASH_ALGO_SHA512 }, { "SM3", EHSM_HASH_ALGO_SM3 }, { "SHA3_224", EHSM_HASH_ALGO_SHA3_224 },
    { "SHA3_256", EHSM_HASH_ALGO_SHA3_256 }, { "SHA3_384", EHSM_HASH_ALGO_SHA3_384 },
    { "SHA3_512", EHSM_HASH_ALGO_SHA3_512 }, { NULL, 0 } };

const algo_map_t key_types[] = { { "DES", EHSM_KEY_TYPE_DES }, { "3DES_128", EHSM_KEY_TYPE_TDES_128 },
    { "3DES_192", EHSM_KEY_TYPE_TDES_192 }, { "AES_128", EHSM_KEY_TYPE_AES_128 }, { "AES_192", EHSM_KEY_TYPE_AES_192 },
    { "AES_256", EHSM_KEY_TYPE_AES_256 }, { "SM4", EHSM_KEY_TYPE_SM4 }, { "SM2", EHSM_KEY_TYPE_SM2 },
    { "RSA_1024", EHSM_KEY_TYPE_RSA_1024 }, { "RSA_2048", EHSM_KEY_TYPE_RSA_2048 },
    { "RSA_3072", EHSM_KEY_TYPE_RSA_3072 }, { "RSA_4096", EHSM_KEY_TYPE_RSA_4096 },
    { "RSA_1024_CRT", EHSM_KEY_TYPE_RSA_1024_CRT }, { "RSA_2048_CRT", EHSM_KEY_TYPE_RSA_2048_CRT },
    { "RSA_3072_CRT", EHSM_KEY_TYPE_RSA_3072_CRT }, { "RSA_4096_CRT", EHSM_KEY_TYPE_RSA_4096_CRT },
    { "ECC_BP_160R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1 }, { "ECC_BP_192R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1 },
    { "ECC_SECP_224R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1 }, { "ECC_SECP_256K1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1 },
    { "ECC_BP_320R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1 }, { "ECC_BP_384R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1 },
    { "ECC_BP_512R1", EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1 }, { "ECC_SECP_192R1", EHSM_KEY_TYPE_ECC_SECP_192R1 },
    { "ECC_BP_224R1", EHSM_KEY_TYPE_ECC_SECP_224R1 }, { "ECC_BP_256R1", EHSM_KEY_TYPE_ECC_SECP_256R1 },
    { "ECC_BP_384R1", EHSM_KEY_TYPE_ECC_SECP_384R1 }, { "ECC_BP_512R1", EHSM_KEY_TYPE_ECC_SECP_521R1 },
    { "ED25519", EHSM_KEY_TYPE_ED25519 }, { "X25519", EHSM_KEY_TYPE_X25519 }, { "SM4_XTS", EHSM_KEY_TYPE_SM4_XTS },
    { "AES_128_XTS", EHSM_KEY_TYPE_AES_128_XTS }, { "AES_192_XTS", EHSM_KEY_TYPE_AES_192_XTS },
    { "AES_256_XTS", EHSM_KEY_TYPE_AES_256_XTS }, { "CHACHA", EHSM_KEY_TYPE_CHACHA }, { "HMAC", EHSM_KEY_TYPE_HMAC },
    { "ECC_SECP_160K1", EHSM_KEY_TYPE_ECC_SECP_160K1 }, { "ECC_SECP_224K1", EHSM_KEY_TYPE_ECC_SECP_224K1 },
    { "ECC_SECP_256K1", EHSM_KEY_TYPE_ECC_SECP_256K1 }, { NULL, 0 } };

const algo_map_t aead_modes[] = { { "CCM", EHSM_AEAD_MODE_CCM }, { "GCM", EHSM_AEAD_MODE_GCM }, { NULL, 0 } };

const mode_map_t mac_modes[] = { { "CMAC", EHSM_MAC_MODE_CMAC }, { "CBC_MAC", EHSM_MAC_MODE_CBC_MAC },
    { "GMAC", EHSM_MAC_MODE_GMAC }, { NULL, 0 } };

void get_current_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y%m%d-%H%M%S", t);
}

const char *get_algo_name_by_id(const algo_map_t *map, uint32_t id)
{
    if (!map)
        return "UNKNOWN";

    for (int i = 0; map[i].name != NULL; i++) {
        if (map[i].id == id) {
            return map[i].name;
        }
    }
    return "UNKNOWN";
}

uint32_t get_algo_id_by_name(const algo_map_t *map, const char *name)
{
    if (!map || !name)
        return UNKNOWN_ALG_ID;

    for (int i = 0; map[i].name != NULL; i++) {
        if ((strcmp(map[i].name, name) == 0) && (strlen(map[i].name) == strlen(name))) {
            return map[i].id;
        }
    }
    return UNKNOWN_ALG_ID;
}

double calculate_speed_mbps(uint32_t data_size, double time_ms)
{
    if (time_ms <= 0.0)
        return 0.0;
    return (data_size * 8.0 * 1000.0) / (time_ms * 1000000.0);
}

double calculate_speed_tps(double time_ms)
{
    if (time_ms <= 0.0)
        return 0.0;
    return 1000.0 / time_ms;
}

int speed_test_init(speed_test_ctx_t *ctx, speed_test_type_e type)
{
    if (!ctx)
        return -1;

    memset(ctx, 0, sizeof(speed_test_ctx_t));

    const char *type_names[] = { "ske_speed", "hash_speed", "pke_speed", "mac_speed", "aead_speed", "chacha_speed" };

    if (type >= SPEED_TEST_MAX) {
        ehsm_port_printf("Invalid test type: %d\n", type);
        return -1;
    }

    switch (type) {
    case SPEED_TEST_SKE:
        ehsm_port_printf("=== SKE Speed Test Header ===\n");
        ehsm_port_printf(
            "SKE algorithm, mode, enc/dec, data size (bytes), time (ms), speed 5MHz (mbps), speed 1GHz (mbps)\n");
        break;
    case SPEED_TEST_HASH:
        ehsm_port_printf("=== HASH Speed Test Header ===\n");
        ehsm_port_printf("HASH algorithm, data size (bytes), time (ms), speed 5MHz (mbps), speed 1GHz (mbps)\n");
        break;
    case SPEED_TEST_PKE:
        ehsm_port_printf("=== PKE Speed Test Header ===\n");
        ehsm_port_printf("PKE algorithm, mode, time (ms), speed 5MHz (tps), speed 1GHz (tps)\n");
        break;
    case SPEED_TEST_MAC:
        ehsm_port_printf("=== MAC Speed Test Header ===\n");
        ehsm_port_printf("MAC algorithm, data size (bytes), time (ms), speed 5MHz (mbps), speed 1GHz (mbps)\n");
        break;
    case SPEED_TEST_AEAD:
        ehsm_port_printf("=== AEAD Speed Test Header ===\n");
        ehsm_port_printf(
            "AEAD algorithm, enc/dec, data size (bytes), time (ms), speed 5MHz (mbps), speed 1GHz (mbps)\n");
        break;
    case SPEED_TEST_CHACHA:
        ehsm_port_printf("=== ChaCha Speed Test Header ===\n");
        ehsm_port_printf("ChaCha algorithm, data size (bytes), time (ms), speed 5MHz (mbps), speed 1GHz (mbps)\n");
        break;
    default:
        break;
    }

    ctx->ehsm_ctx
        = (ehsm_ctx_st *)EHSM_PORT_SHARE_MEM_BASE_ADDR; /* 用于 SoC API 存储 Mailbox 指令、返回值和配置的 buffer */
    ehsm_ctx_init(ctx->ehsm_ctx, 0, false, NULL);
    ctx->data1_addr = EHSM_PORT_SHARE_MEM_BASE_ADDR + 1024;
    ctx->data2_addr = EHSM_PORT_SHARE_MEM_BASE_ADDR + 1024 + 250 * 1024;

    ehsm_port_printf("Speed test initialized for %s\n", type_names[type]);
    return 0;
}

int speed_test_cleanup(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return -1;

    ehsm_port_printf("\n=== Speed Test Summary ===\n");
    ehsm_port_printf("Total tests: %u\n", ctx->test_count);
    ehsm_port_printf("Passed: %u\n", ctx->pass_count);
    ehsm_port_printf("Failed: %u\n", ctx->fail_count);

    return 0;
}

uint32_t speed_test_log_result(speed_test_ctx_t *ctx, const speed_test_result_t *result)
{
    if (!ctx || !result)
        return 1;

    ctx->test_count++;

    if (result->time_ms > 0) {
        ctx->pass_count++;

        if (strlen(result->mode_name) > 0) {
            ehsm_port_printf("%s, %s, %s, %u, %lu.%lu, %lu.%lu, %lu.%lu\n", result->algo_name, result->mode_name,
                result->is_encrypt ? "enc" : "dec", result->data_size, (uint32_t)result->time_ms,
                (uint32_t)((uint32_t)(result->time_ms * 1000) % 1000), (uint32_t)result->speed_5m_mbps,
                (uint32_t)((uint32_t)(result->speed_5m_mbps * 1000) % 1000), (uint32_t)result->speed_1g_mbps,
                (uint32_t)((uint32_t)(result->speed_1g_mbps * 1000) % 1000));
        } else {
            ehsm_port_printf("%s, %u,%lu.%lu, %lu.%lu, %lu.%lu\n", result->algo_name, result->data_size,
                (uint32_t)result->time_ms, (uint32_t)((uint32_t)(result->time_ms * 1000) % 1000),
                (uint32_t)result->speed_5m_mbps, (uint32_t)((uint32_t)(result->speed_5m_mbps * 1000) % 1000),
                (uint32_t)result->speed_1g_mbps, (uint32_t)((uint32_t)(result->speed_1g_mbps * 1000) % 1000));
        }

        ehsm_port_printf("[%s speed] algo: %s", strlen(result->mode_name) > 0 ? "cipher" : "hash", result->algo_name);

        if (strlen(result->mode_name) > 0) {
            ehsm_port_printf(", mode: %s, %s", result->mode_name, result->is_encrypt ? "enc" : "dec");
        }

        ehsm_port_printf(", size: %u bytes, time: %lu.%lu ms, speed: %lu.%lu mbps (5M), %lu.%lu mbps (1G)\n",
            result->data_size, (uint32_t)result->time_ms, (uint32_t)((uint32_t)(result->time_ms * 1000) % 1000),
            (uint32_t)result->speed_5m_mbps, (uint32_t)((uint32_t)(result->speed_5m_mbps * 1000) % 1000),
            (uint32_t)result->speed_1g_mbps, (uint32_t)((uint32_t)(result->speed_1g_mbps * 1000) % 1000));
    } else {
        ctx->fail_count++;
        ehsm_port_printf("Test failed for %s %s\n", result->algo_name, result->mode_name);
    }

    return 0;
}

int speed_test_setup_ehsm(void)
{
    uint32_t ret = ehsm_driver_init_library(EHSM_DRV_MODE_WAIT_AND_POLL);
    if (ret != EHSM_OK) {
        ehsm_port_printf("Failed to initialize eHSM library: 0x%08x\n", ret);
        return -1;
    }

    ehsm_port_printf("eHSM setup completed successfully\n");
    return 0;
}