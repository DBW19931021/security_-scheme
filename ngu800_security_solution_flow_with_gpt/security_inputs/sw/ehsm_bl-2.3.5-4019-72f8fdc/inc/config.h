#ifndef EHSM_BL_CONFIG_H
#define EHSM_BL_CONFIG_H

/* The CPU freqence in Hz */
#define CONFIG_BL_CPU_FREQ_HZ 0x29b92700u /* 700000000 */

/* The max code size of upgrade image supported */
#define CONFIG_BL_MAX_UPGRADE_FW_CODE_SIZE 0xa00000u /* 10485760 */

/* Whether enable 3DES in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_3DES_ENABLE 0

/* Whether enable AES in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_AES_ENABLE 1

/* Whether enable DES in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_DES_ENABLE 0

/* Whether enable ECC in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_ECC_ENABLE 1

/* Whether enable MD5 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_MD5_ENABLE 0

/* Whether enable RSA in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_RSA_ENABLE 1

/* Whether enable SHA1 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SHA1_ENABLE 0

/* Whether enable SHA256 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SHA256_ENABLE 1

/* Whether enable SHA3 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SHA3_ENABLE 0

/* Whether enable SM2 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SM2_ENABLE 1

/* Whether enable SM3 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SM3_ENABLE 1

/* Whether enable SM4 in self test */
#define CONFIG_BL_SELFTEST_ALGOFAM_SM4_ENABLE 1

/* Whether enable CBC mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_CBC_ENABLE 1

/* Whether enable CCM mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_CCM_ENABLE 0

/* Whether enable CFB mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_CFB_ENABLE 1

/* Whether enable CMAC mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_CMAC_ENABLE 1

/* Whether enable CTR mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_CTR_ENABLE 1

/* Whether enable ECB mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_ECB_ENABLE 1

/* Whether enable GCM mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_GCM_ENABLE 0

/* Whether enable OFB mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_OFB_ENABLE 1

/* Whether enable XTS mode in self test */
#define CONFIG_BL_SELFTEST_ALGOMODE_XTS_ENABLE 0

/* The IROM size for bootloader */
#define CONFIG_BL_ROM_SIZE 0x10000u /* 65536 */

/* Default UART baudrate */
#define CONFIG_BL_DEFAULT_UART_BAUDRATE 0x1c200u /* 115200 */

/* The OTP area size in bytes */
#define CONFIG_BL_OTP_SIZE 0x400u /* 1024 */

/* Whether the hardware OTP ROM patch test is enabled */
#define CONFIG_BL_PATCH_TEST_ENABLE 0

/* The patch image address in host RAM */
#define CONFIG_BL_PATCH_IMAGE_HOST_ADDR 0x6000d000u /* 1610665984 */

/* The total IRAM size of eHSM */
#define CONFIG_BL_IRAM_SIZE 0x40000u /* 262144 */

#endif /* EHSM_BL_CONFIG_H */
