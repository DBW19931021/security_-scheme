#include "speed_test.h"
#include <string.h>
#include "uart_stdout.h"
#include "host_m130_cfg.h"

#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/mailbox.h"

#include "cpu_porting.h"

static uint32_t run_test_by_type(const char *test_type)
{
    speed_test_ctx_t ctx;
    uint32_t ret = 0;

    if (strcmp(test_type, "ske") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_SKE) != 0) {
            return 1;
        }
        ret = test_ske_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "hash") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_HASH) != 0) {
            return 1;
        }
        ret = test_hash_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "pke") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_PKE) != 0) {
            return 1;
        }
        ret = test_pke_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "pke-cipher") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_PKE) != 0) {
            return 1;
        }
        ret = test_pke_cipher_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "mac") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_MAC) != 0) {
            return 1;
        }
        ret = test_mac_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "aead") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_AEAD) != 0) {
            return 1;
        }
        ret = test_aead_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else if (strcmp(test_type, "chacha") == 0) {
        if (speed_test_init(&ctx, SPEED_TEST_CHACHA) != 0) {
            return 1;
        }
        ret = test_chacha_speed(&ctx);
        speed_test_cleanup(&ctx);

    } else {
        ehsm_port_printf("Unknown test type: %s\n", test_type);
        return 1;
    }

    return ret;
}

void reset_ehsm_wait_ready(void)
{
    ehsm_port_reset_ehsm();

#define STATUS_BASE (0x40010000)

/* 对应硬件 o_hsm_status 31：0 */
#define HSM_STATUS_IN     *((volatile unsigned int *)(STATUS_BASE + 0x60))
#define SYSSTA0_BOOT_DONE (1U << 2)
#define SYSSTA0_HSM_READY (1U << 4)

    /* 等待 HW BOOT DONE 和 HSM READY */
    while ((HSM_STATUS_IN & (SYSSTA0_BOOT_DONE | SYSSTA0_HSM_READY)) == 0) {
        ;
    }
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

uint32_t main(int argc, char *argv[])
{
    uint32_t ret = 0;

    if (speed_test_setup_ehsm() != 0) {
        ehsm_port_printf("Failed to setup eHSM\n");
        return 1;
    }
    ehsm_port_printf("eHSM Speed Test Starting...\n");
    // set OTP data
    uint8_t buf[EHSM_PORT_OTP_SIZE];
#if EHSM_PORT_OTP_DEFAULT_VALUE == 0
    memset(buf, 0, sizeof(buf));
#else
    memset(buf, 0xff, sizeof(buf));
#endif
    ehsm_port_write_otp(buf, 0, sizeof(buf));

    // reset ehsm
    reset_ehsm_wait_ready();

    const char *all_types[] = { "ske", "hash", "pke", "pke-cipher", "mac", "aead"
#if 0x0
        ,
        "chacha"
#endif
    };
    int all_count = sizeof(all_types) / sizeof(all_types[0]);

    for (int i = 0; i < all_count; i++) {
        ehsm_port_printf("\n=== Running %s tests ===\n", all_types[i]);
        if (run_test_by_type(all_types[i]) != 0) {
            ret = 1;
        }
    }

    if (ret == 0) {
        ehsm_port_printf("\nAll speed tests completed successfully!\n");
    } else {
        ehsm_port_printf("\nSome speed tests failed!\n");
    }

    return ret == 0 ? 0 : 1;
}