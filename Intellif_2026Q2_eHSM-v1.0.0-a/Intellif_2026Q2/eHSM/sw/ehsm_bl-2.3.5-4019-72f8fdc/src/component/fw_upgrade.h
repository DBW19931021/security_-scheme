#ifndef APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_
#define APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "crypto_lib_api.h"
#include "fw_verify.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define IMAGE_VERIFY_FLAG  (0U)
#define IMAGE_UPGRADE_FLAG (1U)

#define IMAGE_UPGRADE_BLOCK_SIZE (1U * 1024U)

#define MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW             0x00U /*  */
#define MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY  0x01U /*  */
#define MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY 0x02U /*  */
#define MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH               0x03U /*  */
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

typedef struct ehsm_image {
    uint8_t process_mode;
    uint8_t *image;
    uint32_t image_size;
    uint8_t *storage;
    uint32_t storage_size;
    raddr_t host_stor_addr;
    uint8_t *ctx;
    uint32_t ctx_size;
} ehsm_image_upgrade_st, ehsm_image_verify_st;

typedef struct ehsm_upgrade_data {
    uint8_t *signature;
    uint8_t *pubkey;
} ehsm_upgrade_data_st;

typedef struct image_alg_upgrade_ctx {
    uint8_t upgrade_alg;
    uint32_t is_decrypt_code;
    cpt_ske_ctx_st ske_ctx[1];
    uint32_t upgrade_already_size;
    uint32_t upgrade_total_size;
    uint32_t enc_key_id;
    uint32_t verify_key_id;
    uint8_t image_type;
    union {
        cpt_hash_ctx_st hash_ctx[1];
        cpt_ske_cmac_ctx_st cmac_ctx[1];
    } alg_ctx;
} image_alg_upgrade_ctx_st;

typedef struct ehsm_code_data {
    uint8_t *signature;
    uint8_t *pubkey;
    uint8_t *version;
} ehsm_code_data_st;

typedef struct image_alg_code_ctx {
    uint32_t is_analysis_code;
    uint32_t code_init_ctx_flag;
    uint32_t code_already_size;
    uint32_t code_total_size;
    uint32_t code_type;
    uint8_t code_enc;
    raddr_t code_addr;
    uint32_t enc_key_id;
    uint32_t verify_key_id;
    uint8_t verify_alg;
    cpt_ske_ctx_st ske_ctx[1];
    union {
        cpt_ske_cmac_ctx_st cmac_ctx[1];
        cpt_hash_ctx_st hash_ctx[1];
    } alg_ctx;
} image_alg_code_ctx_st;

typedef struct image_ctx {
    image_alg_upgrade_ctx_st *upgrade_alg_ctx;
    image_alg_code_ctx_st *code_alg_ctx;
    ehsm_upgrade_data_st *upgrade_data;
    ehsm_code_data_st *code_data;
} image_ctx_st;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t fwupd_image_upgrade(cmd_packet_st *packet, uint8_t is_only_verify);
uint8_t get_code_key_usage(uint8_t verify_alg);
#endif /* APPLICATION_SRC_SEIP_MAILBOX_CMD_EHSM_IMAGE_UPGRADE_H_ */
