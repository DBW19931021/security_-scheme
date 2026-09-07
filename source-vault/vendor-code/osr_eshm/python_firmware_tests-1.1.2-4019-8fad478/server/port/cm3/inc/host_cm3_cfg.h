#ifndef EHSM_INTCFG_IP_H
#define EHSM_INTCFG_IP_H

/**
 * @brief configuration life cycle mode value
 */
#define CONFIG_EHSM_HW_LIFE_CYCLE_TEST_MODE        0xFFFFFFFF
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEVELOP_MODE     0xBD7E7BEB
#define CONFIG_EHSM_HW_LIFE_CYCLE_MANUFACTURE_MODE 0xB93E5BE9
#define CONFIG_EHSM_HW_LIFE_CYCLE_USER_MODE        0xA83E1369
#define CONFIG_EHSM_HW_LIFE_CYCLE_DEBUG_MODE       0x283A0321
#define CONFIG_EHSM_HW_LIFE_CYCLE_DESTROY_MODE     0x00000000

#define STATUS_BASE          (0x40010000)
#define SYSSTA0_HW_BOOT_DONE (1U << 0)
#define SYSSTA0_HW_BOOT_ERR  (1U << 1)
#define SYSSTA0_BOOT_DONE    (1U << 2)
#define SYSSTA0_BOOT_ERR     (1U << 3)
#define SYSSTA0_HSM_READY    (1U << 4)

#define HSM_STATUS_IN  *((volatile unsigned int *)(STATUS_BASE + 0x60))
#define HSM_STATUS_IN1 *((volatile unsigned int *)(STATUS_BASE + 0x64))

#define CONFIG_EHSM_HW_HOST_CPU_FREQ            (5000000U)

#endif /* EHSM_INTCFG_IP_H */
