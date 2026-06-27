#ifndef SYSREG_H
#define SYSREG_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include <ske/ske.h>

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define SYS_REG_BASE       (0x30000000U)
#define EMU_REG_BASE       (0x30100000U)
#define KMU_REG_BASE       (0x30800000U)
#define MB_REG_BASE        (0x30C00000U)
#define WDT_REG_BASE       (0x3F100000U)
#define CRC_REG_BASE       (0x3F600000U)
#define UTC_TIMER_REG_BASE (0x3FC00000U)
#define COUNTER_REG_BASE   (0x3FD00000U)

#define CFG_REG_BASE_ADDR (0x33400000U)
#define CFG_REG_END_ADDR  (0x33500000U)

#define SYS_STA_WRITE_ENABLE_KEY (0x65U)

#define SYS_STA_REG_BASE (SYS_REG_BASE + 0x1000U)
#ifndef CONFIG_UNIT_TEST
#define SYS_STA0_REG    *((volatile uint32_t *)(SYS_REG_BASE + 0x1000U))
#define SYS_SM_MASK_REG *((volatile uint32_t *)(SYS_REG_BASE + 0x80400U))
#define SYS_VER1_REG    *((volatile uint32_t *)(SYS_REG_BASE + 0x0084U))
#else
#define SYS_VER1_REG 0x00000000U
#endif

#define SYS_SM_MASK_ECC_UMS    ((uint32_t)1U << 23)
#define SYS_SM_MASK_ECC_1B_UMS ((uint32_t)1U << 22)

#define SYS_STA0_LIFECYCLE_MASK        ((uint32_t)0x7F << 24)
#define SYS_STA0_LIFECYCLE_MCUTEST     ((uint32_t)0x01 << 24)
#define SYS_STA0_LIFECYCLE_DEVELOP     ((uint32_t)0x01 << 25)
#define SYS_STA0_LIFECYCLE_MANUFACTURE ((uint32_t)0x01 << 26)
#define SYS_STA0_LIFECYCLE_USER        ((uint32_t)0x01 << 27)
#define SYS_STA0_LIFECYCLE_DEBUG       ((uint32_t)0x01 << 28)
#define SYS_STA0_LIFECYCLE_DESTROY     ((uint32_t)0x01 << 29)

#define SYS_STA0_HSM_READY ((uint32_t)0x01 << 20)
#define SYS_STA0_HSM_FAIL  ((uint32_t)0x01 << 21)

#define SYS_STA0_SOC_BOOT_DONE ((uint32_t)0x1U << 22)
#define SYS_STA0_SOC_BOOT_ERR  ((uint32_t)0x1U << 23)
#define SYS_STA1_HSM_DBG_EN    ((uint32_t)0x01 << 16)
#define SYS_STA1_SOC_DBG_EN    ((uint32_t)0x01 << 17)

#ifndef CONFIG_UNIT_TEST
#define SYS_FW_CFG_REG1 *((volatile uint32_t *)(SYS_REG_BASE + 0x8104U))
#endif

#define OTP_FW_UPGRADE_ALG_OFFSET 4U
#define OTP_FW_UPGRADE_ALG_MASK   ((uint32_t)0x0FU << OTP_FW_UPGRADE_ALG_OFFSET)
#define OTP_FW_VERIFY_ALG_OFFSET  0U
#define OTP_FW_VERIFY_ALG_MASK    ((uint32_t)0x0FU << OTP_FW_VERIFY_ALG_OFFSET)

#ifndef CONFIG_UNIT_TEST
#define SYS_SOC_CFG_REG0 *((volatile uint32_t *)(SYS_REG_BASE + 0x8108U))
#endif

#define OTP_SOC_UPGRADE_ALG_OFFSET 4U
#define OTP_SOC_UPGRADE_ALG_MASK   ((uint32_t)0x0FU << OTP_SOC_UPGRADE_ALG_OFFSET)
#define OTP_SOC_VERIFY_ALG_OFFSET  0U
#define OTP_SOC_VERIFY_ALG_MASK    ((uint32_t)0x0FU << OTP_SOC_VERIFY_ALG_OFFSET)

// Bit offset of SOC boot type
#define SOC_BOOT_TYPE_OFFSET     8U
#define SOC_BOOT_TYPE_MASK       ((uint32_t)0x03U << SOC_BOOT_TYPE_OFFSET)
#define SOC_BOOT_PARALLEL_VALUE1 1U
#define SOC_BOOT_PARALLEL_VALUE2 2U
#define SOC_BOOT_TYPE_SEQUENTIAL 0x0U
#define SOC_BOOT_TYPE_PARALLEL   0x1U

#ifndef CONFIG_UNIT_TEST
#define SYS_SOC_DBG_EN_REG *((volatile uint32_t *)(SYS_REG_BASE + 0x1100U))
#endif
#define SYS_SOC_DBG_EN_VALUE 0x6F3C0A95U

#define SYS_SOC_DBG_EN_BASE_REG (SYS_REG_BASE + 0x1104U)

#ifndef CONFIG_UNIT_TEST
#define SYS_HSM_DBG_EN_REG *((volatile uint32_t *)(SYS_REG_BASE + 0x1080U))
#endif
#define SYS_HSM_DBG_EN_VALUE 0x265C1A93U

#ifndef CONFIG_UNIT_TEST
/* AHB_DMA */
#define AHB_WRITE_AXI_READ  (0x00U)
#define AHB_READ_AXI_WRITE  (0x01U << 1U)
#define AHB_READ_AND_WRITE  (0x02U << 1U)
#define AXI_READ_AND_WRITE  (0x03U << 1U)
#define AHB_AXI_CONFIG_MASK (0x03U << 1U)

#define AHB_DMA_SKE  (0U)
#define AHB_DMA_HASH (1U)

#define SYS_AHB_DMA_CFG *((volatile uint32_t *)(SYS_REG_BASE + 0x6000U))
#endif

#define SYS_HW_INF5_OTP_END_MASK ((uint32_t)0xFF)
#ifndef CONFIG_UNIT_TEST
#define SYS_HW_INF5 *((volatile uint32_t *)(SYS_REG_BASE + 0x14U))
#endif

#define SYS_GEN_REG_BASE (SYS_REG_BASE + 0xF0800U)

#ifndef CONFIG_UNIT_TEST
#define SYS_HW_CONTROL1_REG *((volatile uint32_t *)(SYS_REG_BASE + 0x8080U))
#endif
#define OTP_CTRL0_K_ALG_SEL_MASK   ((uint32_t)0x03 << 12)
#define OTP_CTRL0_K_ALG_SEL_AES128 ((uint32_t)0x01 << 12)

#ifndef CONFIG_UNIT_TEST
#define MB_HSM_STATUS_0_REG *((volatile uint32_t *)(MB_REG_BASE + 0x0400U))
#define MB_HSM_STATUS_1_REG *((volatile uint32_t *)(MB_REG_BASE + 0x0404U))

#define EMU_INT_SENSOR_REG   *((volatile uint32_t *)(EMU_REG_BASE + 0x20U))
#define EMU_INT_HW0_REG      *((volatile uint32_t *)(EMU_REG_BASE + 0x28U))
#define EMU_INT_HW1_REG      *((volatile uint32_t *)(EMU_REG_BASE + 0x2CU))
#define EMU_ERR_FW0_REG      *((volatile uint32_t *)(EMU_REG_BASE + 0x10U))
#define EMU_ERR_FW1_REG      *((volatile uint32_t *)(EMU_REG_BASE + 0x14U))
#define EMU_HW_TRIG_0_REG    *((volatile uint32_t *)(EMU_REG_BASE + 0x140U))
#define EMU_HW_TRIG_1_REG    *((volatile uint32_t *)(EMU_REG_BASE + 0x144U))
#endif

#define EMU_ERR_TRIG_UNLOCK_VALUE (0x965F0CBAU)
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
uint32_t sysreg_get_soc_boot_type(void);

uint8_t sysreg_get_soc_verify_alg(void);

uint8_t sysreg_get_fw_upd_alg(void);

uint8_t sysreg_get_fw_verify_alg(void);

void sysreg_get_status(uint32_t index, uint32_t *status);

void sysreg_set_status(uint32_t index, uint32_t status);

void sysreg_enable_hsm_dbg(void);

void sysreg_disable_hsm_dbg(void);

void sysreg_enable_soc_dbg(void);

void sysreg_enable_soc_dbg_ext(uint8_t zone, uint32_t value);

void sysreg_disable_soc_dbg(void);

void sysreg_disable_soc_dbg_ext(uint8_t zone, uint32_t value);

uint32_t sysreg_get_life_cycle(void);

uint32_t sysreg_get_otpkey_dec_alg(void);

void sysreg_ahb_dma_cfg(uint32_t cfg);

uint8_t sysreg_get_soc_upd_alg(void);

uint8_t sysreg_get_otp_key_num(void);

void sysreg_set_gen_reg(uint8_t zone, uint32_t value);
#endif
