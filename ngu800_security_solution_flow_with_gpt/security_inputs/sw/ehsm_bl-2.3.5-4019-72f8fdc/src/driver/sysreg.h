#ifndef SYSREG_H
#define SYSREG_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include <ske/ske.h>
#include "config.h"
#include "crypto_lib_api.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// Definition of boot type
#define SOC_BOOT_TYPE_SEQUENTIAL 0x0U
#define SOC_BOOT_TYPE_PARALLEL   0x1U

// Bit offset of SOC boot type
#define SOC_BOOT_TYPE_OFFSET 4U
#define SOC_BOOT_TYPE_MASK   ((uint32_t)0x03U << SOC_BOOT_TYPE_OFFSET)

#define SYS_REG_BASE     0x30000000UL
#define SYS_STA_REG_BASE (SYS_REG_BASE + 0x1000U)
#define SYS_STA0_REG     *((volatile uint32_t *)(SYS_REG_BASE + 0x1000U))
#define SYS_STA1_REG     *((volatile uint32_t *)(SYS_REG_BASE + 0x1004U))
#define SYS_STA2_REG     *((volatile uint32_t *)(SYS_REG_BASE + 0x1008U))
#define SYS_STA3_REG     *((volatile uint32_t *)(SYS_REG_BASE + 0x100CU))
#define SYS_VER1_REG     *((volatile uint32_t *)(SYS_REG_BASE + 0x0084U))
#define SYS_SOC_STA_REG  *((volatile uint32_t *)(SYS_REG_BASE + 0x2000U))
#define SYS_HW_INFO0_REG *((volatile uint32_t *)(SYS_REG_BASE + 0x0000U))

#define SYS_HW_INFO0_DEFINE_FPGA ((uint32_t)0x01U << 30)

#define SYS_STA0_WRITE_FLAG            (0x65U)
#define SYS_STA0_LIFECYCLE_MCUTEST     ((uint32_t)0x01U << 24)
#define SYS_STA0_LIFECYCLE_MASK        ((uint32_t)0x7FU << 24)
#define SYS_STA0_LIFECYCLE_DEVELOP     ((uint32_t)0x01U << 25)
#define SYS_STA0_LIFECYCLE_MANUFACTURE ((uint32_t)0x01U << 26)
#define SYS_STA0_LIFECYCLE_USER        ((uint32_t)0x01U << 27)
#define SYS_STA0_LIFECYCLE_DEBUG       ((uint32_t)0x01U << 28)
#define SYS_STA0_LIFECYCLE_DESTROY     ((uint32_t)0x01U << 29)
#define SYS_STA0_LIFECYCLE_UNNORMAL    ((uint32_t)0x01U << 30)

#define SYS_STA0_BOOTLOADER_READY (1U << 18)
#define SYS_STA0_BOOTLOADER_ERR   (1U << 19)
#define SYS_STA0_SOC_VERIFY_DONE  (1U << 22)
#define SYS_STA0_SOC_VERIFY_ERR   (1U << 23)

#define SYS_STA1_WRITE_FLAG (0x93U)
#define SYS_STA2_WRITE_FLAG (0xA6U)
#define SYS_STA3_WRITE_FLAG (0xC9U)

#define SYS_FW_CFG_REG1     *((volatile uint32_t *)(SYS_REG_BASE + 0x8100U))
#define SYS_FW_CFG_REG2     *((volatile uint32_t *)(SYS_REG_BASE + 0x8104U))
#define SYS_FW_CFG_REG1_PRT ((volatile uint32_t *)(SYS_REG_BASE + 0x8100U))
#define SYS_FW_CFG_REG2_PRT ((volatile uint32_t *)(SYS_REG_BASE + 0x8104U))

#define SYS_SOC_CFG_REG1     *((volatile uint32_t *)(SYS_REG_BASE + 0x8108U))
#define SYS_SOC_CFG_REG2     *((volatile uint32_t *)(SYS_REG_BASE + 0x810CU))
#define SYS_SOC_CFG_REG1_PRT ((volatile uint32_t *)(SYS_REG_BASE + 0x8108U))

#define SOC_BOOT_SEQUENTIAL_VALUE1 0U
#define SOC_BOOT_PARALLEL_VALUE1   1U
#define SOC_BOOT_PARALLEL_VALUE2   2U
#define SOC_BOOT_SEQUENTIAL_VALUE2 3U

#define OTP_SOC_UPGRADE_ALG_OFFSET 4U
#define OTP_SOC_UPGRADE_ALG_MASK   ((uint32_t)0x0FU << OTP_SOC_UPGRADE_ALG_OFFSET)
#define OTP_SOC_VERIFY_ALG_OFFSET  0U
#define OTP_SOC_VERIFY_ALG_MASK    ((uint32_t)0x0FU << OTP_SOC_VERIFY_ALG_OFFSET)

#define OTP_FW_UPGRADE_ALG_OFFSET 4U
#define OTP_FW_UPGRADE_ALG_MASK   ((uint32_t)0x0FU << OTP_FW_UPGRADE_ALG_OFFSET)
#define OTP_FW_VERIFY_ALG_OFFSET  0U
#define OTP_FW_VERIFY_ALG_MASK    ((uint32_t)0x0FU << OTP_FW_VERIFY_ALG_OFFSET)

#define OTP_SELF_TEST_DISABLE_FLAG0 1U
#define OTP_SELF_TEST_DISABLE_FLAG1 2U
#define OTP_SELF_TEST_DISABLE_MASK  3U

#define SYS_SOC_DBG_EN_REG   *((volatile uint32_t *)(SYS_REG_BASE + 0x1100U))
#define SYS_SOC_DBG_EN_VALUE 0x6F3C0A95UL

#define SYS_SOC_DBG_EN_BASE_REG (SYS_REG_BASE + 0x1104U)

#define SYS_HSM_DBG_EN_REG   *((volatile uint32_t *)(SYS_REG_BASE + 0x1080U))
#define SYS_HSM_DBG_EN_VALUE 0x265C1A93UL

#define SYS_BUS_ERR_CFG    *((volatile uint32_t *)(SYS_REG_BASE + 0x5000))
#define SYS_BUS_SOC_ERR_EN ((uint32_t)0x01U << 0)
#define SYS_BUS_OTP_ERR_EN ((uint32_t)0x01U << 4)

/* AHB_DMA */
#define AHB_WRITE_AXI_READ  (0x00U)
#define AHB_READ_AXI_WRITE  (0x01U << 1U)
#define AHB_READ_AND_WRITE  (0x02U << 1U)
#define AXI_READ_AND_WRITE  (0x03U << 1U)
#define AHB_AXI_CONFIG_MASK (0x03U << 1U)

#define AHB_DMA_SKE  (0U)
#define AHB_DMA_HASH (1U)

#define SYS_AHB_DMA_CFG *((volatile uint32_t *)(SYS_REG_BASE + 0x6000))

#define SYS_SM_MASK_REG        *((volatile uint32_t *)(SYS_REG_BASE + 0x80400U))
#define SYS_SM_MASK_ECC_UMS    (1U << 23)
#define SYS_SM_MASK_ECC_1B_UMS (1U << 22)

#define OTP_FW_SELF_TEST_OFFSET 6U
#define OTP_FW_SELF_TEST_MASK   (0x3U << 6U)
#define OTP_FW_SELF_TEST_OFF    (0x0U)
#define OTP_FW_SELF_TEST_BASIC1 (0x1U)
#define OTP_FW_SELF_TEST_BASIC2 (0x2U)
#define OTP_FW_SELF_TEST_ALL    (0x3U)

#define OTP_FW_VERIFY_OFFSET  10U
#define OTP_SOC_VERIFY_OFFSET 10U

#define OTP_PATCH_ENABLE_OFFSET 8U
#define OTP_PATCH_ENABLE_MASK   (0x3U << 8U)

#define OTP_WDT_LEVEL_OFFSET 10U
#define OTP_WDT_LEVEL_MASK   (0xFU << 10U)

#define OTP_LOG_EN_OFFSET 14U
#define OTP_LOG_EN_MASK   (0x1U << 14U)

#define OTP_FW_IN_NVM_OFFSET 16U
#define OTP_FW_IN_NVM_MASK   (0xFU << 16U)

#define OTP_FW_RANDCLK_OFFSET          13U
#define OTP_FW_RANDCLK_MASK            (0x3U << OTP_FW_RANDCLK_OFFSET)
#define OTP_FW_RANDCLK_1_CLK_32_CYCLES 0x1u
#define OTP_FW_RANDCLK_1_CLK_16_CYCLES 0x2u
#define OTP_FW_RANDCLK_1_CLK_8_CYCLES  0x3u
#define OTP_FW_RANDCLK_DISABLE         0x0u

#define SYS_HW_INF5_OTP_END_MASK ((uint32_t)0xFF)
#define SYS_HW_INF5              *((volatile uint32_t *)(SYS_REG_BASE + 0x14U))

#define rSYS_STA(a) *((volatile uint32_t *)(SYS_REG_BASE + 0x1000U + (4U * (a))))

#define SYS_HW_CFG_REG1 *((volatile uint32_t *)(SYS_REG_BASE + 0x8080U))
#define SYS_HW_CFG_REG2 *((volatile uint32_t *)(SYS_REG_BASE + 0x8084U))

#define OTP_HW_KEY_DEC_ALG_OFFSET 12U
#define OTP_HW_KEY_DEC_ALG_MASK   ((uint32_t)0x3U << OTP_HW_KEY_DEC_ALG_OFFSET)
#define OTP_HW_KEY_DEC_ALG_AES128 0x1U

#define SYS_SM_TRIG *((volatile uint32_t *)(SYS_REG_BASE + 0x80800))

#define SYS_RST_CTRL0           *((volatile uint32_t *)(SYS_REG_BASE + 0x3200))
#define SYS_RST_CTRL0_HSM_RESET (~(0x1U << 31U))

#define SYS_WR_SOC_RESET             *((volatile uint32_t *)(SYS_REG_BASE + 0x1280))
#define SYS_WR_SOC_RESET_SET_VALUE   0x5C28639DU
#define SYS_WR_SOC_RESET_CLEAR_VALUE 0xAE1F05B7U

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/

// 所有控制字段按FW-CTRL-EHSM 和 FW—CTRL-SOC 共128bits统一排列
#define OTP_FW_SELF_TEST_LEVEL_OFFSET 6
#define OTP_FW_SELF_TEST_LEVEL_WIDTH  2

#define OTP_FW_EHSM_BOOT_VERIFY_ACTIVELY_OFFSET 10
#define OTP_FW_EHSM_BOOT_VERIFY_ACTIVELY_WIDTH  1


#define OTP_FW_RANDOM_CLK_LEVEL_OFFSET 13
#define OTP_FW_RANDOM_CLK_LEVEL_WIDTH  2

#define OTP_FW_EHSM_BOOT_VERIFY_ALG_OFFSET 32
#define OTP_FW_EHSM_BOOT_VERIFY_ALG_WIDTH  4

#define OTP_FW_EHSM_UP_VERIFY_ALG_OFFSET 36
#define OTP_FW_EHSM_UP_VERIFY_ALG_WIDTH  4

#define OTP_FW_PATCH_ENABLE_OFFSET 40
#define OTP_FW_PATCH_ENABLE_WIDTH  2

#define OTP_FW_WDT_LEVEL_OFFSET 42
#define OTP_FW_WDT_LEVEL_WIDTH  4

#define OTP_FW_UART_LOG_SWITCH_OFFSET 46
#define OTP_FW_UART_LOG_SWITCH_WIDTH  1

#define OTP_FW_BOOT_FROM_NVM_OFFSET 48
#define OTP_FW_BOOT_FROM_NVM_WIDTH  4

#define OTP_FW_SOC_BOOT_VERIFY_ALG_OFFSET 64
#define OTP_FW_SOC_BOOT_VERIFY_ALG_WIDTH  4

#define OTP_FW_SOC_UP_VERIFY_ALG_OFFSET 68
#define OTP_FW_SOC_UP_VERIFY_ALG_WIDTH  4

#define OTP_FW_SOC_BOOT_VERIFY_ACTIVELY_OFFSET 74
#define OTP_FW_SOC_BOOT_VERIFY_ACTIVELY_WIDTH  1

static inline uint32_t sysreg_read_fw_ctrl_bits(uint32_t off, uint32_t width, bool_t abs)
{
    // 注意，此实现要求定义控制字段时，单个字段不能跨32bit边界
    uint32_t index = off / 32U;
    uint32_t bit_off = off % 32U;
    uint32_t mask = (1U << width) - 1U;
    const volatile uint32_t *reg_base = (const volatile uint32_t *)(SYS_REG_BASE + 0x8100U);
    uint32_t reg_val = reg_base[index];

    UNUSED(abs);

    return (reg_val >> bit_off) & mask;
}

#define sysreg_get_soc_verify_alg() \
    (uint8_t)sysreg_read_fw_ctrl_bits(OTP_FW_SOC_BOOT_VERIFY_ALG_OFFSET, OTP_FW_SOC_BOOT_VERIFY_ALG_WIDTH, 1)

#define sysreg_get_soc_upd_alg() \
    (uint8_t)sysreg_read_fw_ctrl_bits(OTP_FW_SOC_UP_VERIFY_ALG_OFFSET, OTP_FW_SOC_UP_VERIFY_ALG_WIDTH, 1)

#define sysreg_get_fw_upd_alg() \
    (uint8_t)sysreg_read_fw_ctrl_bits(OTP_FW_EHSM_UP_VERIFY_ALG_OFFSET, OTP_FW_EHSM_UP_VERIFY_ALG_WIDTH, 1)

#define sysreg_get_fw_verify_alg() \
    (uint8_t)sysreg_read_fw_ctrl_bits(OTP_FW_EHSM_BOOT_VERIFY_ALG_OFFSET, OTP_FW_EHSM_BOOT_VERIFY_ALG_WIDTH, 1)

#define sysreg_get_self_test_type() \
    sysreg_read_fw_ctrl_bits(OTP_FW_SELF_TEST_LEVEL_OFFSET, OTP_FW_SELF_TEST_LEVEL_WIDTH, 0)



#define sysreg_get_randclk_type() \
    (uint8_t)(sysreg_read_fw_ctrl_bits(OTP_FW_RANDOM_CLK_LEVEL_OFFSET, OTP_FW_RANDOM_CLK_LEVEL_WIDTH, 0))

#define sysreg_get_wdt_level() sysreg_read_fw_ctrl_bits(OTP_FW_WDT_LEVEL_OFFSET, OTP_FW_WDT_LEVEL_WIDTH, 0)

#define sysreg_is_log_enable() \
    (sysreg_read_fw_ctrl_bits(OTP_FW_UART_LOG_SWITCH_OFFSET, OTP_FW_UART_LOG_SWITCH_WIDTH, 0) ? false : true)


/* ------------------------------------------------------- */

void sysreg_get_status(uint32_t index, uint32_t *status);

void sysreg_set_status(uint32_t index, uint32_t status);

void sysreg_enable_hsm_dbg(void);

void sysreg_disable_hsm_dbg(void);

void sysreg_enable_soc_dbg(void);

void sysreg_enable_soc_dbg_ext(uint8_t zone, uint32_t value);

void sysreg_disable_soc_dbg(void);

void sysreg_disable_soc_dbg_ext(uint8_t zone, uint32_t value);

void sysreg_enable_bus_err(void);

uint32_t sysreg_get_life_cycle(void);

cpt_ske_alg_e sysreg_get_otpkey_dec_alg(void);

void sysreg_ahb_dma_cfg(uint32_t cfg);

uint8_t sysreg_get_otp_key_num(void);

void sysreg_hsm_reset(void);

void sysreg_soc_reset(void);

#define OSR_FPGA_TEST_ENV_MAGIC 0xfa66e27U

// 1. 当前的HW FPGA标记已设置
// 2. SYS_SOC_STA 目前未使用，OSR内部使用时传入一个MAGIC值以标明这是OSR FPGA环境
#define sysreg_is_osr_fpga() \
    ((SYS_HW_INFO0_REG & SYS_HW_INFO0_DEFINE_FPGA) != 0 && SYS_SOC_STA_REG == OSR_FPGA_TEST_ENV_MAGIC)

#endif
