#ifndef EHSM_TYPES_IP_H_
#define EHSM_TYPES_IP_H_
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define NULL ((void *)0)

/*Definition ehsm error code*/
#define EHSM_ERR_MB_SUCCESS            (0xA55AU)

#define EHSM_ERR_SW_SUCCESS            (0U)
#define EHSM_ERR_PARAM_ERROR           (1U)
#define EHSM_ERR_QUEUE_FULL            (2U)
#define EHSM_ERR_NOT_INIT              (3U)
#define EHSM_ERR_REPEATED_INIT         (4U)
#define EHSM_ERR_FREE_REPEATED         (5U)
#define EHSM_ERR_QUEUE_EMPTY           (6U)
#define EHSM_ERR_CMDPOOL_EMPTY         (7U)
#define EHSM_ERR_INVALID_ADDRESS       (8U)
#define EHSM_ERR_WONG_IRQ_NUM          (9U)
#define EHSM_ERR_NOT_SUPPORT           (10U) 
#define EHSM_ERR_INVALID_HANDLE        (11U)
#define EHSM_ERR_MISMATCH_KEY_USAGE    (12U)
#define EHSM_ERR_NOT_ALIGNED           (13U)
#define EHSM_ERR_DATA_NOT_EMPTY        (14U)
#define EHSM_ERR_INIT_DEV_FAIL         (15U)
#define EHSM_ERR_WRITE_DEV_FAIL        (16U)
#define EHSM_ERR_READ_DEV_FAIL         (17U)
#define EHSM_ERR_ERASE_DEV_FAIL        (18U)
#define EHSM_ERR_CTX_OVERFLOW          (19U)
#define EHSM_ERR_OUTPUT_OVERFLOW       (20U)
#define EHSM_ERR_HMAC_K_BUF_OCCUPIED   (21U)
#define EHSM_ERR_HMAC_INVALID_K_IDX    (22U)
#define EHSM_ERR_REMAP_FAILED          (23U)
#define EHSM_ERR_REMAP_ADDRESS_OVERFLOW (24U)
#define EHSM_ERR_MAC_LEN_WRONG_FORMAT   (25U)
#define EHSM_ERR_MAC_VRY_FAILED         (26U)
#define EHSM_ERR_INVALID_DIR            (27U)
#define EHSM_ERR_OUT_OF_MEM             (28U)
#define EHSM_ERR_DBG_BUF_OVERFLOW      (29U)
#define EHSM_ERR_DBG_CMD_INCOMPLETE    (30U)
#define EHSM_ERR_DBG_PROTO_ERROR       (31U)
#define EHSM_ERR_INVALID_CHANNEL       (32U)
#define EHSM_ERR_NO_CMD_EXEC_PERM      (33U)
#define EHSM_ERR_INVALID_CMD           (34U)
#define EHSM_ERR_SM2_DATA_BUF_OVERFLOW (35U)
#define EHSM_ERR_SM2_SIGNATURE_GEN_FAILED    (36U)
#define EHSM_ERR_SM2_SIGNATURE_VRY_FAILED    (37U)
#define EHSM_ERR_RSA_CALCULATE_FAILED        (38U)
#define EHSM_ERR_WRONG_SZ_OF_SIGNATURE       (39U)
#define EHSM_ERR_LIMIT_OF_AUTHORITY       (40U)
#define EHSM_ERR_VERIFY_KEY_INTEGRITY     (43U)
#define EHSM_ERR_EXEC_CRYPTO_LIB          (44U)
#define EHSM_ERR_MISMATCH_KEY_PERMISSION  (45U)
#define EHSM_ERR_NOT_EXIST_KEY_PART       (46U)
#define EHSM_ERR_NOT_EXIST_KEY            (47U)
#define EHSM_ERR_INVALID_KEY_ID           (48U)
#define EHSM_ERR_INVALID_ALGORITHM        (49U)
#define EHSM_ERR_DH_KEY_TOO_LEN           (50U)
#define EHSM_ERR_NOT_ALLOWED_CREATION_KEY (51U)
#define EHSM_ERR_NOT_EMPTY_KEY_HANDLE     (52U)
#define EHSM_ERR_IV_OVERFLOW              (53U)
#define EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT (54U)
#define EHSM_ERR_XTS_WRONG_DATA_LENGTH      (55U)
#define EHSM_ERR_INVALID_CIPHER_MODE        (57U)
#define EHSM_ERR_INVALID_PADDING            (58U)
#define EHSM_ERR_NOT_FREE_SPACE             (59U)
#define EHSM_ERR_DATA_CHECKSUM              (60U)
#define EHSM_ERR_NVM_WRITE_DATA             (61U)
#define EHSM_ERR_NVM_SYNC_DATA              (62U)
#define EHSM_ERR_ECDSA_SIGNATURE_GEN_FAILED (63U)
#define EHSM_ERR_ECDSA_SIGNATURE_VRY_FAILED (64U)
#define EHSM_ERR_SM2_CIPHER_ENC_FAILED      (65U)
#define EHSM_ERR_SM2_CIPHER_DEC_FAILED      (66U)
#define EHSM_ERR_INPUT_OVERFLOW             (67U)
#define EHSM_ERR_RSA_SIGNATURE_GEN_FAILED   (68U)
#define EHSM_ERR_RSA_SIGNATURE_VRY_FAILED   (69U)
#define EHSM_ERR_KEY_NOT_FOUND              (70U)
#define EHSM_ERR_WRONG_IV_SIZE              (71U)
#define EHSM_ERR_NVM_ENCRYPTION_KEY_DATA    (72U)
#define EHSM_ERR_NVM_SIGNATURE_KEY_DATA     (73U)
#define EHSM_ERR_GCM_TAG_VRY_FAILED         (74U)
#define EHSM_ERR_CCM_TAG_VRY_FAILED         (75U)
#define EHSM_ERR_KEY_UPDATE_ERROR           (76U)
#define EHSM_ERR_KEY_INVALID                (77U)
#define EHSM_ERR_KEY_NOT_AVAILABLE          (78U)
#define EHSM_ERR_KEY_EMPTY_ERROR            (79U)
#define EHSM_ERR_KEY_WRITE_PROTECTED        (80U)
#define EHSM_ERR_EHSM_LIFECYCLE_LIMIT       (81U)
#define EHSM_ERR_WRONG_CHALLENGE_TYPE       (82U)
#define EHSM_ERR_WRONG_DATA_LENGTH          (83U)
#define EHSM_ERR_WRONG_ALGORITHM            (84U)
#define EHSM_ERR_WRONG_KEY_TYPE             (85U)
#define EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH (86U)
#define EHSM_ERR_DEBUG_AUTH_FAILED          (87U)
#define EHSM_ERR_SEQUENCE                   (88U)
#define EHSM_ERR_NO_DATA_IN_MAC_FINAL       (89U)
#define EHSM_ERR_WRONG_KEY_LEVEL            (90U)
#define EHSM_ERR_OTP_KEY_INSTALL_TWICE      (91U)
#define EHSM_ERR_OTP_WRITE_CMP_ERROR        (92U)
#define EHSM_ERR_REMOVE_USER_KEY_FAILED     (93U)
#define EHSM_ERR_UTC_REG_VALUE_MISMATCH     (94U)
#define EHSM_ERR_UTC_RESYNC_FAILED          (95U)
#define EHSM_ERR_WONG_UTC_TO_IDX            (96U)
#define EHSM_ERR_COUNTER_REG_VALUE_MISMATCH (97U)
#define EHSM_ERR_UTC_TIMER_NOT_SYNC         (98U)
#define EHSM_ERR_UTC_TIMER_DISABLE          (99U)
#define EHSM_ERR_ALL_COUNTER_OCCURIED       (100U)
#define EHSM_ERR_INVALID_COUNTER_ID         (101U)
#define EHSM_ERR_COUNTER_DISABLE            (102U)
#define EHSM_ERR_COUNTER_OVERFLOW           (103U)
#define EHSM_ERR_UTC_TIMER_OVERFLOW         (104U)
#define EHSM_ERR_INVALID_DH_PQGH_SIZE       (105U)
//definition of mailbox cmd size
//The length of AEAD cmd is at least 25 word
#define MAX_CMD_RSP_WORD_LEN                (25U)
#define MAX_CMD_RSP_BYTE_LEN                (20U)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef _Bool bool_t;
typedef uint64_t raddr_t;

#define TRUE  (1U)
#define FALSE  (0U)

// TODO: change me
typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint32_t data[23];
} cmd_req_data_st;

typedef struct {
    uint32_t ret_code;
    uint32_t rsp_data_len;
    uint32_t data[5];
} cmd_rsp_data_st;


typedef struct {
    uint32_t channel;
    raddr_t tag;
    raddr_t rsp_addr;
    cmd_rsp_data_st rsp_data;
    cmd_req_data_st cmd_data;
} cmd_packet_st;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#endif
