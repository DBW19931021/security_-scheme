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
#ifndef NULL
#define NULL ((void *)0)
#endif

/*Definition ehsm error code*/
#define EHSM_ERR_MB_SUCCESS (0xA55AU)

#define EHSM_ERR_SW_SUCCESS                  (0xA55A5AA5U)
#define EHSM_ERR_PARAM_ERROR                 (1U)
#define EHSM_ERR_QUEUE_FULL                  (2U)
#define EHSM_ERR_NOT_INIT                    (3U)
#define EHSM_ERR_REPEATED_INIT               (4U)
#define EHSM_ERR_FREE_REPEATED               (5U)
#define EHSM_ERR_QUEUE_EMPTY                 (6U)
#define EHSM_ERR_CMDPOOL_EMPTY               (7U)
#define EHSM_ERR_INVALID_ADDRESS             (8U)
#define EHSM_ERR_WONG_IRQ_NUM                (9U)
#define EHSM_ERR_NOT_SUPPORT                 (10U)
#define EHSM_ERR_INVALID_HANDLE              (11U)
#define EHSM_ERR_MISMATCH_KEY_USAGE          (12U)
#define EHSM_ERR_NOT_ALIGNED                 (13U)
#define EHSM_ERR_DATA_NOT_EMPTY              (14U)
#define EHSM_ERR_INIT_DEV_FAIL               (15U)
#define EHSM_ERR_WRITE_DEV_FAIL              (16U)
#define EHSM_ERR_READ_DEV_FAIL               (17U)
#define EHSM_ERR_ERASE_DEV_FAIL              (18U)
#define EHSM_ERR_CTX_OVERFLOW                (19U)
#define EHSM_ERR_OUTPUT_OVERFLOW             (20U)
#define EHSM_ERR_HMAC_K_BUF_OCCUPIED         (21U)
#define EHSM_ERR_HMAC_INVALID_K_IDX          (22U)
#define EHSM_ERR_REMAP_FAILED                (23U)
#define EHSM_ERR_REMAP_ADDRESS_OVERFLOW      (24U)
#define EHSM_ERR_MAC_LEN_WRONG_FORMAT        (25U)
#define EHSM_ERR_MAC_VRY_FAILED              (26U)
#define EHSM_ERR_INVALID_DIR                 (27U)
#define EHSM_ERR_OUT_OF_MEM                  (28U)
#define EHSM_ERR_DBG_BUF_OVERFLOW            (29U)
#define EHSM_ERR_DBG_CMD_INCOMPLETE          (30U)
#define EHSM_ERR_DBG_PROTO_ERROR             (31U)
#define EHSM_ERR_INVALID_CHANNEL             (32U)
#define EHSM_ERR_NO_CMD_EXEC_PERM            (33U)
#define EHSM_ERR_INVALID_CMD                 (34U)
#define EHSM_ERR_SM2_DATA_BUF_OVERFLOW       (35U)
#define EHSM_ERR_SM2_SIGNATURE_GEN_FAILED    (36U)
#define EHSM_ERR_SM2_SIGNATURE_VRY_FAILED    (37U)
#define EHSM_ERR_RSA_CALCULATE_FAILED        (38U)
#define EHSM_ERR_WRONG_SZ_OF_SIGNATURE       (39U)
#define EHSM_ERR_LIMIT_OF_AUTHORITY          (40U)
#define EHSM_ERR_VERIFY_KEY_INTEGRITY        (43U)
#define EHSM_ERR_EXEC_CRYPTO_LIB             (44U)
#define EHSM_ERR_MISMATCH_KEY_PERMISSION     (45U)
#define EHSM_ERR_NOT_EXIST_KEY_PART          (46U)
#define EHSM_ERR_NOT_EXIST_KEY               (47U)
#define EHSM_ERR_INVALID_KEY_ID              (48U)
#define EHSM_ERR_INVALID_ALGORITHM           (49U)
#define EHSM_ERR_DH_KEY_TOO_LEN              (50U)
#define EHSM_ERR_NOT_ALLOWED_CREATION_KEY    (51U)
#define EHSM_ERR_NOT_EMPTY_KEY_HANDLE        (52U)
#define EHSM_ERR_IV_OVERFLOW                 (53U)
#define EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT  (54U)
#define EHSM_ERR_XTS_WRONG_DATA_LENGTH       (55U)
#define EHSM_ERR_INVALID_CIPHER_MODE         (57U)
#define EHSM_ERR_INVALID_PADDING             (58U)
#define EHSM_ERR_NOT_FREE_SPACE              (59U)
#define EHSM_ERR_NVM_DATA_CHECKSUM           (60U)
#define EHSM_ERR_NVM_WRITE_DATA              (61U)
#define EHSM_ERR_NVM_SYNC_DATA               (62U)
#define EHSM_ERR_ECDSA_SIGNATURE_GEN_FAILED  (63U)
#define EHSM_ERR_ECDSA_SIGNATURE_VRY_FAILED  (64U)
#define EHSM_ERR_SM2_CIPHER_ENC_FAILED       (65U)
#define EHSM_ERR_SM2_CIPHER_DEC_FAILED       (66U)
#define EHSM_ERR_INPUT_OVERFLOW              (67U)
#define EHSM_ERR_RSA_SIGNATURE_GEN_FAILED    (68U)
#define EHSM_ERR_RSA_SIGNATURE_VRY_FAILED    (69U)
#define EHSM_ERR_KEY_NOT_FOUND               (70U)
#define EHSM_ERR_WRONG_IV_SIZE               (71U)
#define EHSM_ERR_NVM_ENCRYPTION_KEY_DATA     (72U)
#define EHSM_ERR_NVM_SIGNATURE_KEY_DATA      (73U)
#define EHSM_ERR_GCM_TAG_VRY_FAILED          (74U)
#define EHSM_ERR_CCM_TAG_VRY_FAILED          (75U)
#define EHSM_ERR_KEY_UPDATE_ERROR            (76U)
#define EHSM_ERR_KEY_INVALID                 (77U)
#define EHSM_ERR_KEY_NOT_AVAILABLE           (78U)
#define EHSM_ERR_KEY_EMPTY_ERROR             (79U)
#define EHSM_ERR_KEY_WRITE_PROTECTED         (80U)
#define EHSM_ERR_EHSM_LIFECYCLE_LIMIT        (81U)
#define EHSM_ERR_WRONG_CHALLENGE_TYPE        (82U)
#define EHSM_ERR_WRONG_DATA_LENGTH           (83U)
#define EHSM_ERR_WRONG_ALGORITHM             (84U)
#define EHSM_ERR_WRONG_KEY_TYPE              (85U)
#define EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH (86U)
#define EHSM_ERR_DEBUG_AUTH_FAILED           (87U)
#define EHSM_ERR_SEQUENCE                    (88U)
#define EHSM_ERR_NO_DATA_IN_MAC_FINAL        (89U)

#define EHSM_ERR_SKE_WORK_ERROR        (256U)
#define EHSM_ERR_PKE_WORK_ERROR        (257U)
#define EHSM_ERR_HASH_WORK_ERROR       (258U)
#define EHSM_ERR_TRNG_WORK_ERROR       (259U)
#define EHSM_ERR_WRONG_K_LEVEL         (260U)
#define EHSM_ERR_WRONG_AUTH_TYPE       (261U)
#define EHSM_ERR_WRONG_PUBKEY          (262U)
#define EHSM_ERR_WRONG_VERSION_COUNTER (263U)
#define EHSM_ERR_FW_VERIFY_FAILED      (264U)
#define EHSM_ERR_WRONG_PROC_MODE       (265U)
#define EHSM_ERR_DATA_CHECK_ERROR      (266U)
#define EHSM_ERR_WRONG_FW_TYPE         (267U)
#define EHSM_ERR_WRONG_CONTEXT         (268U)
#define EHSM_ERR_INVALID_CODE_FLAG     (269U)
#define EHSM_ERR_FW_AUTH_FAILED        (270U)
#define EHSM_ERR_SELFTEST_FAILED       (280U)
#define EHSM_ERR_WRONG_KEY_SIZE        (281U)
#define EHSM_ERR_CALC_HASH             (282U)
#define EHSM_ERR_INVALID_IMAGE_SIZE    (283U)

// definition of mailbox cmd size
// The length of AEAD cmd is at least 25 word
#define MAX_CMD_WORD_LEN (25U)
#define MAX_RSP_WORD_LEN (5U)

#define SKE_ALG_AES SKE_ALG_AES_128

// ---------------------------------------------- //
//
#ifdef __GNUC__
#define ATTR_INTERRUPT __attribute__((interrupt, aligned(64)))
#elif defined(__ICCRISCV__)
// for IAR, alignment is set by compiler flags
#define ATTR_INTERRUPT __interrupt
#else
#define ATTR_INTERRUPT
#endif

// usage macro static assert
#define _EXPAND_NAME(x)        _EXPAND_NAME2(x, __LINE__)
#define _EXPAND_NAME2(x, line) _EXPAND_NAME3(x, line)
#define _EXPAND_NAME3(x, line) x##line

#define STATIC_ASSERT(expr)                        \
    enum _EXPAND_NAME(_static_assert_enum) {       \
        _EXPAND_NAME(_VAL) = 1 / (uint32_t)(expr), \
    }

// unused variable
#define UNUSED(x) (void)(x)

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef uint8_t bool_t;
typedef uint64_t raddr_t;

#define TRUE  (1U)
#define FALSE (0U)

typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint32_t data[MAX_CMD_WORD_LEN - 1];
} cmd_req_data_st;

typedef struct {
    uint32_t ret_code;
    uint32_t rsp_data_len;
    uint32_t data[MAX_RSP_WORD_LEN - 1];
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
