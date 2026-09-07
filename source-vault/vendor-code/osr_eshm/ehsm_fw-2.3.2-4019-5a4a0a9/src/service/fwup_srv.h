#ifndef APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_
#define APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "service/socvrfy_srv.h"
#include "ske/ske.h"
#include "hash_hmac/hash.h"
#include "ske/ske_cmac.h"
#include "ske/ske_gcm_gmac.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define CODE_VERIFY_ALG_RSA2048 (0U)
#define CODE_VERIFY_ALG_RSA3072 (6U)
#define CODE_VERIFY_ALG_SM2 (1U)
#define CODE_VERIFY_ALG_AES_CMAC (2U)
#define CODE_VERIFY_ALG_SM4_CMAC (3U)
#define CODE_VERIFY_ALG_INVALID (0xFFU)

// ehsm secure boot upgrade algorithm
#define CODE_UPGRADE_ALG_RSA2048 (0U)
#define CODE_UPGRADE_ALG_RSA3072 (6U)
#define CODE_UPGRADE_ALG_SM2 (1U)
#define CODE_UPGRADE_ALG_ECC_P256R1 (7U)


#define CODE_UPGRADE_ALG_AES128_CMAC (4U)
#define CODE_UPGRADE_ALG_SM4_CMAC (5U)
#define CODE_UPGRADE_ALG_INVALID (0xFFU)

#define IMAGE_DECRYPT_CODE  (0x5AU)
#define IMAGE_ANALYSIS_CODE (0x55U)

#define IMAGE_UPGRADE_BLOCK_SIZE (1U * 1024U)

#define MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW             0x00U /*  */
#define MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY  0x01U /*  */
#define MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY 0x02U /*  */

#define OTP_KEY_INVALID_TYPE (0xFFU)

#define UPGRADE_VALID_FLAG (0x71689BA2UL)
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
uint32_t fwup_srv_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data);

#endif /* APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_ */
