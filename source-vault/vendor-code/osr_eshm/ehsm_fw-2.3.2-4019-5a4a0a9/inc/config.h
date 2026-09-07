#ifndef EHSM_FW_CONFIG_H
#define EHSM_FW_CONFIG_H

/* The CPU freqence in Hz */
#define CONFIG_EHSM_CPU_FREQ_HZ 0x29b92700u /* 700000000 */

/* The mailbox channel count */
#define CONFIG_EHSM_MB_CHANNEL_COUNT 0x10u /* 16 */

/* Whether OTP default bit value is 0 */
#define CONFIG_EHSM_OTP_DEFAULT_BIT_0 1

/* The base address of OTP area */
#define CONFIG_EHSM_OTP_BASE_ADDR 0x33000000u /* 855638016 */

/* The OTP area size in bytes */
#define CONFIG_EHSM_OTP_SIZE 0x400u /* 1024 */

/* The OTP program unit size in bytes */
#define CONFIG_EHSM_OTP_WRITE_UNIT 0x04u /* 4 */

/* Default UART baudrate */
#define CONFIG_EHSM_DEFAULT_UART_BAUDRATE 0x1c200u /* 115200 */

/* RAM buffer size in bytes to store keys */
#define CONFIG_EHSM_RAM_KEY_BUFFER_MAX_SIZE 0x1800u /* 6144 */

/* The default clock frequence for UTC timer */
#define CONFIG_EHSM_UTC_DEFAULT_FREQ 0x7a1200u /* 8000000 */

/* Whether valid field is supported for life cycle in OTP */
#define CONFIG_EHSM_HW_OTP_WITH_LIFE_CYCLE_VALID 0

/* Whether check that the OTP area is empty before write */
#define CONFIG_EHSM_OTP_CHECK_WRITE_AREA 0

/* The max code size of upgrade image supported */
#define CONFIG_EHSM_MAX_UPGRADE_FW_CODE_SIZE 0xa00000u /* 10485760 */

/* OTP key count */
#define CONFIG_EHSM_OTP_KEY_COUNT 0x11u /* 17 */

/* Whether some test commands is supported, this config must be disabled in release version! */
#define CONFIG_EHSM_TEST_CMDS_ENABLE 0

/* The ske hardware type, 0:LP, 1:HP, 2:UHP  */
#define CONFIG_EHSM_SKE_HW_TYPE 0x01u /* 1 */

/* The hash hardware type, 0:LP, 1:HP, 2:UHP */
#define CONFIG_EHSM_HASH_HW_TYPE 0x01u /* 1 */

/* The pke hardware type, 0:LP, 1:HP, 2:UHP */
#define CONFIG_EHSM_PKE_HW_TYPE 0x01u /* 1 */

/* The FIFO size in maintenance module */
#define CONFIG_EHSM_MT_FIFO_SIZE 0x04u /* 4 */

#endif /* EHSM_FW_CONFIG_H */
