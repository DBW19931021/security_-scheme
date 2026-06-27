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
// Address offset of fieids in the inmage
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

#define CODE_VALID_FLAG (0x8E97645DUL)

// ehsm secure boot verify algorithm
#define CODE_VERIFY_ALG_RSA2048    (0U)
#define CODE_VERIFY_ALG_SM2        (1U)
#define CODE_VERIFY_ALG_AES_CMAC   (2U)
#define CODE_VERIFY_ALG_SM4_CMAC   (3U)
#define CODE_VERIFY_ALG_RSA3072    (6U)
#define CODE_VERIFY_ALG_ECC_P256R1 (7U)

// RSA public key info
#define RSA_PUBLIC_K_E_LEN     (64U)
#define RSA2048_PUBLIC_K_N_LEN (256U)
#define RSA2048_PUBLIC_K_LEN   (RSA_PUBLIC_K_E_LEN + RSA2048_PUBLIC_K_N_LEN)
#define RSA3072_PUBLIC_K_N_LEN (384U)
#define RSA3072_PUBLIC_K_LEN   (RSA_PUBLIC_K_E_LEN + RSA3072_PUBLIC_K_N_LEN)

// SM2  public key info
#define SM2_PUBLIC_K_LEN    (65U)
#define SM2_Z_SIZE          (32U)
#define SM3_BLOCK_BYTE_SIZE (64U)

#define RSA_SIGNATURE_HASH HASH_SHA256
#define RSA_PUBLIC_K_HASH  RSA_SIGNATURE_HASH
#define SM2_SIGNATURE_HASH HASH_SM3
#define SM2_PUBLIC_K_HASH  SM2_SIGNATURE_HASH

#define ECCP256R1_PUBLIC_K_LEN (64U)
#define ECC_SIGNATURE_HASH     HASH_SHA256
#define ECC_PUBLIC_K_HASH      ECC_SIGNATURE_HASH

// Image head size
#define EHSM_IMAGE_HEADER_SIZE (1024U)
// signature data size in image header
#define EHSM_IMAGE_HEADER_HASH_SIZE (EHSM_IMAGE_HEADER_SIZE - VALID_FLAG_OFFSET)
// start offset of image header signature data
#define EHSM_CODE_SIZE_RAM_OFFSET (VALID_FLAG_OFFSET)

#define IMAGE_PUBLIC_K_EXT_OFFSET INFO_RESERVED
#define IMAGE_PUBLIC_K_OFFSET     PUBLIC_K_OFFSET
#define MAX_PUBKEY_SIZE           (384U + 64U)

#define EHSM_IMAGE_TYPE_SOC_FW_USE_SOC_KEY  0x01U /*  */
#define EHSM_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY 0x02U /*  */
#define EHSM_IMAGE_CODE_PLAIN_NO            0x00U /*  */
#define EHSM_IMAGE_CODE_PLAIN_YES           0x01U /*  */

#define MB_FW_IMAGE_NAKED 0x01U /*  */

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
uint32_t socvrfy_srv_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data);
uint32_t socvrfy_get_and_verify_pubkey(const uint8_t *key_head, uint8_t *pukkey, uint32_t verify_alg, uint32_t key_id);
uint32_t socvrfy_check_fw_header(const uint8_t *header, uint8_t check_version, const uint8_t *version);
bool_t socvrfy_check_ver_cnt(const uint8_t *otp_version_counter, const uint8_t *code_version_counter);
#endif
