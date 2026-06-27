/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "bl_patch_test.h"
#include "mmap.h"
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#if defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE)
#define BL_PATCH_TEST_MARKER_ADDR       ((raddr_t)0x60019000U)
#define BL_PATCH_TEST_MARKER_STATUS_OK  (0xA5000000U)
#define BL_PATCH_TEST_MARKER_STATUS_ERR (0xE5000000U)
#define BL_PATCH_TEST_MARKER_VALUE_MASK (0x00FFFFFFU)
#define BL_PATCH_TEST_PROBE_BYTE_MASK   (0xFFU)
#define BL_PATCH_TEST_PROBE1_SHIFT      (8U)
#define BL_PATCH_TEST_PROBE2_SHIFT      (16U)

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t bl_patch_test_make_marker(void);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
__asm__(
    "    .section .text.bl_patch_test_probe,\"ax\",@progbits\n"
    "    .balign 4\n"
    "    .global bl_patch_test_probe0\n"
    "    .type bl_patch_test_probe0, @function\n"
    "bl_patch_test_probe0:\n"
    "    .option push\n"
    "    .option norvc\n"
    "    addi a0, zero, 0x11\n"
    "    jalr zero, 0(ra)\n"
    "    .option pop\n"
    "    .size bl_patch_test_probe0, . - bl_patch_test_probe0\n"
    "    .balign 4\n"
    "    .global bl_patch_test_probe1\n"
    "    .type bl_patch_test_probe1, @function\n"
    "bl_patch_test_probe1:\n"
    "    .option push\n"
    "    .option norvc\n"
    "    addi a0, zero, 0x22\n"
    "    jalr zero, 0(ra)\n"
    "    .option pop\n"
    "    .size bl_patch_test_probe1, . - bl_patch_test_probe1\n"
    "    .balign 4\n"
    "    .global bl_patch_test_probe2\n"
    "    .type bl_patch_test_probe2, @function\n"
    "bl_patch_test_probe2:\n"
    "    .option push\n"
    "    .option norvc\n"
    "    addi a0, zero, 0x33\n"
    "    jalr zero, 0(ra)\n"
    "    .option pop\n"
    "    .size bl_patch_test_probe2, . - bl_patch_test_probe2\n"
    "    .section .text\n");

static uint32_t bl_patch_test_make_marker(void)
{
    uint32_t marker;
    uint32_t probe0;
    uint32_t probe1;
    uint32_t probe2;

    probe0 = bl_patch_test_probe0() & BL_PATCH_TEST_PROBE_BYTE_MASK;
    probe1 = bl_patch_test_probe1() & BL_PATCH_TEST_PROBE_BYTE_MASK;
    probe2 = bl_patch_test_probe2() & BL_PATCH_TEST_PROBE_BYTE_MASK;

    marker = BL_PATCH_TEST_MARKER_STATUS_OK;
    marker |= probe0;
    marker |= probe1 << BL_PATCH_TEST_PROBE1_SHIFT;
    marker |= probe2 << BL_PATCH_TEST_PROBE2_SHIFT;

    return marker;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
void bl_patch_test_run(void)
{
    uint32_t ret;
    uint32_t marker;

    marker = bl_patch_test_make_marker();
    ret = mmap_write_remote_data(BL_PATCH_TEST_MARKER_ADDR, &marker, sizeof(marker));
    if (EHSM_ERR_SW_SUCCESS != ret) {
        marker = BL_PATCH_TEST_MARKER_STATUS_ERR | (marker & BL_PATCH_TEST_MARKER_VALUE_MASK);
        (void)mmap_write_remote_data(BL_PATCH_TEST_MARKER_ADDR, &marker, sizeof(marker));
    } else {
        /* Do nothing. */
    }
}
#endif /* defined(CONFIG_BL_PATCH_TEST_ENABLE) && (CONFIG_BL_PATCH_TEST_ENABLE) */
