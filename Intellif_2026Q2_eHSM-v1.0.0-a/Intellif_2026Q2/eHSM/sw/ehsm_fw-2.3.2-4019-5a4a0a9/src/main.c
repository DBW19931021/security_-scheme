#include "secboot.h"
#include "version.h"
#include "crypto_util.h"
#include "component/util.h"

typedef struct {
    uint8_t type;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t pre_release[8];
} ehsm_ver_data_st;

#ifdef __GNUC__
__attribute__((used, section(".ver_data")))
#elif defined(__ICCRISCV__)
#pragma location = ".ver_data"
#endif
/* This version struct data will be stored in binary file and checked by release script */
static const ehsm_ver_data_st g_ver_data
    = {
          .type = 1,
          .major = EHSM_FW_VER_MAJOR,
          .minor = EHSM_FW_VER_MINOR,
          .patch = EHSM_FW_VER_PATCH,
          .pre_release = EHSM_FW_VER_PRE_RELEASE,
      };

#ifdef __GNUC__
ATTR_INTERRUPT void default_intexc_handler(void)
{
    while (1) { }
}
#endif

/*
 * TRNG initial random data is fixed/repeated after power-on when self-test is skipped.
 * Root cause: DRBG initialization with true random seed is part of TRNG HW self-test flow.
 *
 * Workaround: force a reseed and discard the initial fixed 64-byte FIFO output, so that subsequent callers always
 * receive true random data.
 *
 * Note: the return value of cpt_get_rand() is intentionally not checked here. This workaround runs before the TRNG
 * self-test and business logic. Any TRNG hardware failure will be detected and reported by the self-test and upper
 * layers that follow, avoiding redundant error handling at this stage.
 */
static void trng_discard_initial_fixed_output(void)
{
#define TRNG_DRBG_INIT_FIFO_SIZE (64U)
    uint8_t tmp_rand[TRNG_DRBG_INIT_FIFO_SIZE];
    cpt_trng_reseed();
    /* Assign to volatile to prevent the compiler from eliminating this call, even under LTO. Return value is
    intentionally discarded (see above). */
    volatile uint32_t ret = cpt_get_rand(tmp_rand, sizeof(tmp_rand));
    (void)ret;
    /* Clear buffer to avoid leaving known-pattern data on the stack. */
    util_memset(tmp_rand, 0, sizeof(tmp_rand));
}

int main(void)
{
    trng_discard_initial_fixed_output();
    // just read the version number to make EDA can watch the version
    uint32_t v;
    util_memcpy(&v, &g_ver_data, sizeof(uint32_t));
    // write it to CSR general register 1
    *((volatile uint32_t *)(SYS_GEN_REG + 4U)) = v;

    secboot_entry();
    return 0;
}
