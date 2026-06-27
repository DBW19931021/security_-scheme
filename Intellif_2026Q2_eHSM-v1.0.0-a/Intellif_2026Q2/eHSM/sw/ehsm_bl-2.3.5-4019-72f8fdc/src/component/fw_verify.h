#ifndef FW_VERIFY_H
#define FW_VERIFY_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "config.h"
#include "mb.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define SIGNATURE_OFFSET       (0U)                                          // 256byte
#define PUBLIC_K_OFFSET        (256U)                                        // 320byte
#define IV_OFFSET              (256U + 320U)                                 // 16byte
#define VALID_FLAG_OFFSET      (256U + 320U + 16U)                           // 4byte
#define IMAGE_TYPE_OFFSET      (256U + 320U + 16U + 4U)                      // 1byte
#define PLAIN_FLAG_OFFSET      (256U + 320U + 16U + 4U + 1U)                 // 1byte
#define NAKED_FLAG_OFFSET      (256U + 320U + 16U + 4U + 1U + 1U)            // 1byte
#define CODE_SIZE_OFFSET       (256U + 320U + 16U + 4U + 3U + 5U)            // 4byte
#define VERSION_COUNTER_OFFSET (256U + 320U + 16U + 4U + 3U + 5U + 4U)       // 32byte
#define INFO_RESERVED          (256U + 320U + 16U + 4U + 3U + 5U + 4U + 16U) // 400byte

#define CODE_VALID_FLAG    (0x8E97645DUL)
#define UPGRADE_VALID_FLAG (0x71689BA2UL)

#define CODE_VERIFY_ALG_RSA2048    (0U)
#define CODE_VERIFY_ALG_SM2        (1U)
#define CODE_VERIFY_ALG_AES_CMAC   (2U)
#define CODE_VERIFY_ALG_SM4_CMAC   (3U)
#define CODE_VERIFY_ALG_RSA3072    (6U)
#define CODE_VERIFY_ALG_ECC_P256R1 (7U)
#define CODE_VERIFY_ALG_INVALID    (0xFFU)

// ehsm secure boot upgrade algorithm
#define CODE_UPGRADE_ALG_RSA2048    (0U)
#define CODE_UPGRADE_ALG_SM2        (1U)
#define CODE_UPGRADE_ALG_AES_GCM    (2U)
#define CODE_UPGRADE_ALG_SM4_GCM    (3U)
#define CODE_UPGRADE_ALG_AES_CMAC   (4U)
#define CODE_UPGRADE_ALG_RSA3072    (6U)
#define CODE_UPGRADE_ALG_ECC_P256R1 (7U)
#define CODE_UPGRADE_ALG_SM4_CMAC   (5U)
#define CODE_UPGRADE_ALG_INVALID    (0xFFU)

#define MAX_SIGNATURE_SIZE (384U)
#define MAX_PUBKEY_SIZE    (384U + 64U)

#define RSA_PUBLIC_K_E_LEN     (64U)
#define RSA2048_PUBLIC_K_N_LEN (256U)
#define RSA2048_PUBLIC_K_LEN   (RSA_PUBLIC_K_E_LEN + RSA2048_PUBLIC_K_N_LEN)
#define RSA3072_PUBLIC_K_N_LEN (384U)
#define RSA3072_PUBLIC_K_LEN   (RSA_PUBLIC_K_E_LEN + RSA3072_PUBLIC_K_N_LEN)

#define ECCP256R1_PUBLIC_K_LEN (64U)

#define SM2_PUBLIC_K_LEN    (65U)
#define SM2_Z_SIZE          (32U)
#define SM3_BLOCK_BYTE_SIZE (64U)

#define RSA_SIGNATURE_HASH HASH_SHA256
#define RSA_PUBLIC_K_HASH  RSA_SIGNATURE_HASH

#define ECC_SIGNATURE_HASH HASH_SHA256
#define ECC_PUBLIC_K_HASH  ECC_SIGNATURE_HASH

#define SM2_SIGNATURE_HASH HASH_SM3
#define SM2_PUBLIC_K_HASH  SM2_SIGNATURE_HASH

#define EHSM_CODE_IRAM_BASE                (0x10800000UL)
#define EHSM_CODE_NVM_BASE                 (0x10400000UL)
#define EHSM_INTERNAL_NVM_IMAGE_ADDR_MAGIC ((raddr_t)0xFFFFFFFFFFFFFFFFULL)

#define EHSM_VERSION_COUNTER_LEN (OTP_VERSION_LENGTH)
#define SOC_VERSION_COUNTER_LEN  (OTP_VERSION_LENGTH)

// image head parameter offset
#define EHSM_CODE_INFO_SIZE          (1024U)
#define IMAGE_INFO_SIZE              (EHSM_CODE_INFO_SIZE)
#define IMAGE_SIGNATURE_OFFSET       SIGNATURE_OFFSET
#define IMAGE_PUBLIC_K_OFFSET        PUBLIC_K_OFFSET
#define IMAGE_IV_OFFSET              IV_OFFSET
#define IMAGE_VALID_FLAG_OFFSET      VALID_FLAG_OFFSET
#define IMAGE_CODE_SIZE_OFFSET       CODE_SIZE_OFFSET
#define IMAGE_VERSION_COUNTER_OFFSET VERSION_COUNTER_OFFSET
#define IMAGE_RESERVED               INFO_RESERVED
#define IMAGE_PUBLIC_K_EXT_OFFSET    INFO_RESERVED
//
#define IMAGE_INFO_HASH_SIZE       (IMAGE_INFO_SIZE - IMAGE_VALID_FLAG_OFFSET)
#define IMAGE_CODE_SIZE_RAM_OFFSET IMAGE_VALID_FLAG_OFFSET

#define EHSM_CODE_INFO_HASH_SIZE  (EHSM_CODE_INFO_SIZE - VALID_FLAG_OFFSET)
#define EHSM_CODE_SIZE_RAM_OFFSET (VALID_FLAG_OFFSET) // 4byte

#define IMAGE_DECRYPT_CODE  (0x5AU)
#define IMAGE_ANALYSIS_CODE (0x55U)

#define MB_BL_VERIFY_IMAGE_IMAGE_TYPE_EHSM_FW             0x00U /*  */
#define MB_BL_VERIFY_IMAGE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY  0x01U /*  */
#define MB_BL_VERIFY_IMAGE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY 0x02U /*  */
#define MB_BL_VERIFY_IMAGE_CODE_PLAIN_NO                  0x00U /*  */
#define MB_BL_VERIFY_IMAGE_CODE_PLAIN_YES                 0x01U /*  */

#define MB_BL_IMAGE_NAKED 0x01U /*  */

#define SYS_GEN_REG (0x300F0800U)
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
uint32_t fwverify_verify_image(cmd_packet_st *packet);

uint32_t fwverify_get_and_verify_pubkey(uint8_t *key_head, uint8_t *pubkey, uint32_t verify_alg, uint32_t key_id);

bool_t fwverify_check_ver_cnt(uint8_t *otp_version_counter, uint8_t *code_version_counter);

bool_t fwverify_header_is_valid(uint8_t *head);

uint32_t fwverify_vry_soc_fw(mb_cmd_bl_verify_image_st *cmd);

uint32_t fwverify_vry_hsm_fw(mb_cmd_bl_verify_image_st *cmd);

uint32_t fwverify_check_fw_header(uint8_t *header, uint8_t check_version, uint8_t *version);

uint32_t fw_verify_load_patch_image(raddr_t image_addr, uint32_t *entry_addr);
#endif
