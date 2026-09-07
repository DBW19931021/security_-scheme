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
#define NULL 0
#endif

/* Only for debug, must set to 0 for release */
#define DEBUG_ERR_CODE 0

#if DEBUG_ERR_CODE
uint32_t util_register_err(uint32_t code, const char *file, uint32_t line);
#define REG_ERR(x) util_register_err(x, __FILE__, __LINE__)
#else
#define REG_ERR(x) x
#endif

/*Definition ehsm error code*/
#define EHSM_ERR_MB_SUCCESS (0xA55AU)

#define EHSM_ERR_SW_SUCCESS                  (0xA55A5AA5U)
#define EHSM_ERR_PARAM_ERROR                 REG_ERR(1U)
#define EHSM_ERR_QUEUE_FULL                  REG_ERR(2U)
#define EHSM_ERR_NOT_INIT                    REG_ERR(3U)
#define EHSM_ERR_REPEATED_INIT               REG_ERR(4U)
#define EHSM_ERR_FREE_REPEATED               REG_ERR(5U)
#define EHSM_ERR_QUEUE_EMPTY                 (6U)
#define EHSM_ERR_CMDPOOL_EMPTY               REG_ERR(7U)
#define EHSM_ERR_INVALID_ADDRESS             REG_ERR(8U)
#define EHSM_ERR_WONG_IRQ_NUM                REG_ERR(9U)
#define EHSM_ERR_NOT_SUPPORT                 REG_ERR(10U)
#define EHSM_ERR_INVALID_HANDLE              REG_ERR(11U)
#define EHSM_ERR_MISMATCH_KEY_USAGE          REG_ERR(12U)
#define EHSM_ERR_NOT_ALIGNED                 REG_ERR(13U)
#define EHSM_ERR_DATA_NOT_EMPTY              REG_ERR(14U)
#define EHSM_ERR_INIT_DEV_FAIL               REG_ERR(15U)
#define EHSM_ERR_WRITE_DEV_FAIL              REG_ERR(16U)
#define EHSM_ERR_READ_DEV_FAIL               REG_ERR(17U)
#define EHSM_ERR_ERASE_DEV_FAIL              REG_ERR(18U)
#define EHSM_ERR_CTX_OVERFLOW                REG_ERR(19U)
#define EHSM_ERR_OUTPUT_OVERFLOW             REG_ERR(20U)
#define EHSM_ERR_HMAC_K_BUF_OCCUPIED         REG_ERR(21U)
#define EHSM_ERR_HMAC_INVALID_K_IDX          REG_ERR(22U)
#define EHSM_ERR_REMAP_FAILED                REG_ERR(23U)
#define EHSM_ERR_REMAP_ADDRESS_OVERFLOW      REG_ERR(24U)
#define EHSM_ERR_MAC_LEN_WRONG_FORMAT        REG_ERR(25U)
#define EHSM_ERR_MAC_VRY_FAILED              REG_ERR(26U)
#define EHSM_ERR_INVALID_DIR                 REG_ERR(27U)
#define EHSM_ERR_OUT_OF_MEM                  REG_ERR(28U)
#define EHSM_ERR_DBG_BUF_OVERFLOW            REG_ERR(29U)
#define EHSM_ERR_DBG_CMD_INCOMPLETE          REG_ERR(30U)
#define EHSM_ERR_DBG_PROTO_ERROR             REG_ERR(31U)
#define EHSM_ERR_INVALID_CHANNEL             REG_ERR(32U)
#define EHSM_ERR_NO_CMD_EXEC_PERM            REG_ERR(33U)
#define EHSM_ERR_INVALID_CMD                 REG_ERR(34U)
#define EHSM_ERR_SM2_DATA_BUF_OVERFLOW       REG_ERR(35U)
#define EHSM_ERR_SM2_SIGNATURE_GEN_FAILED    REG_ERR(36U)
#define EHSM_ERR_SM2_SIGNATURE_VRY_FAILED    REG_ERR(37U)
#define EHSM_ERR_RSA_CALCULATE_FAILED        REG_ERR(38U)
#define EHSM_ERR_WRONG_SZ_OF_SIGNATURE       REG_ERR(39U)
#define EHSM_ERR_LIMIT_OF_AUTHORITY          REG_ERR(40U)
#define EHSM_ERR_VERIFY_KEY_INTEGRITY        REG_ERR(43U)
#define EHSM_ERR_EXEC_CRYPTO_LIB             REG_ERR(44U)
#define EHSM_ERR_MISMATCH_KEY_PERMISSION     REG_ERR(45U)
#define EHSM_ERR_NOT_EXIST_KEY_PART          REG_ERR(46U)
#define EHSM_ERR_NOT_EXIST_KEY               REG_ERR(47U)
#define EHSM_ERR_INVALID_KEY_ID              REG_ERR(48U)
#define EHSM_ERR_INVALID_ALGORITHM           REG_ERR(49U)
#define EHSM_ERR_DH_KEY_TOO_LEN              REG_ERR(50U)
#define EHSM_ERR_NOT_ALLOWED_CREATION_KEY    REG_ERR(51U)
#define EHSM_ERR_NOT_EMPTY_KEY_HANDLE        REG_ERR(52U)
#define EHSM_ERR_IV_OVERFLOW                 REG_ERR(53U)
#define EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT  REG_ERR(54U)
#define EHSM_ERR_XTS_WRONG_DATA_LENGTH       REG_ERR(55U)
#define EHSM_ERR_INVALID_CIPHER_MODE         REG_ERR(57U)
#define EHSM_ERR_INVALID_PADDING             REG_ERR(58U)
#define EHSM_ERR_NOT_FREE_SPACE              REG_ERR(59U)
#define EHSM_ERR_DATA_CHECKSUM               REG_ERR(60U)
#define EHSM_ERR_NVM_WRITE_DATA              REG_ERR(61U)
#define EHSM_ERR_NVM_SYNC_DATA               REG_ERR(62U)
#define EHSM_ERR_ECDSA_SIGNATURE_GEN_FAILED  REG_ERR(63U)
#define EHSM_ERR_ECDSA_SIGNATURE_VRY_FAILED  REG_ERR(64U)
#define EHSM_ERR_SM2_CIPHER_ENC_FAILED       REG_ERR(65U)
#define EHSM_ERR_SM2_CIPHER_DEC_FAILED       REG_ERR(66U)
#define EHSM_ERR_INPUT_OVERFLOW              REG_ERR(67U)
#define EHSM_ERR_RSA_SIGNATURE_GEN_FAILED    REG_ERR(68U)
#define EHSM_ERR_RSA_SIGNATURE_VRY_FAILED    REG_ERR(69U)
#define EHSM_ERR_KEY_NOT_FOUND               REG_ERR(70U)
#define EHSM_ERR_WRONG_IV_SIZE               REG_ERR(71U)
#define EHSM_ERR_NVM_ENCRYPTION_KEY_DATA     REG_ERR(72U)
#define EHSM_ERR_NVM_SIGNATURE_KEY_DATA      REG_ERR(73U)
#define EHSM_ERR_GCM_TAG_VRY_FAILED          REG_ERR(74U)
#define EHSM_ERR_CCM_TAG_VRY_FAILED          REG_ERR(75U)
#define EHSM_ERR_KEY_UPDATE_ERROR            REG_ERR(76U)
#define EHSM_ERR_KEY_INVALID                 REG_ERR(77U)
#define EHSM_ERR_KEY_NOT_AVAILABLE           REG_ERR(78U)
#define EHSM_ERR_KEY_EMPTY_ERROR             REG_ERR(79U)
#define EHSM_ERR_KEY_WRITE_PROTECTED         REG_ERR(80U)
#define EHSM_ERR_EHSM_LIFECYCLE_LIMIT        REG_ERR(81U)
#define EHSM_ERR_WRONG_CHALLENGE_TYPE        REG_ERR(82U)
#define EHSM_ERR_WRONG_DATA_LENGTH           REG_ERR(83U)
#define EHSM_ERR_WRONG_ALGORITHM             REG_ERR(84U)
#define EHSM_ERR_WRONG_KEY_TYPE              REG_ERR(85U)
#define EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH REG_ERR(86U)
#define EHSM_ERR_DEBUG_AUTH_FAILED           REG_ERR(87U)
#define EHSM_ERR_SEQUENCE                    REG_ERR(88U)
#define EHSM_ERR_NO_DATA_IN_MAC_FINAL        REG_ERR(89U)
#define EHSM_ERR_WRONG_KEY_LEVEL             REG_ERR(90U)
#define EHSM_ERR_OTP_KEY_INSTALL_TWICE       REG_ERR(91U)
#define EHSM_ERR_OTP_WRITE_CMP_ERROR         REG_ERR(92U)
#define EHSM_ERR_REMOVE_USER_KEY_FAILED      REG_ERR(93U)
#define EHSM_ERR_UTC_REG_VALUE_MISMATCH      REG_ERR(94U)
#define EHSM_ERR_UTC_RESYNC_FAILED           REG_ERR(95U)
#define EHSM_ERR_WONG_UTC_TO_IDX             REG_ERR(96U)
#define EHSM_ERR_COUNTER_REG_VALUE_MISMATCH  REG_ERR(97U)
#define EHSM_ERR_UTC_TIMER_NOT_SYNC          REG_ERR(98U)
#define EHSM_ERR_UTC_TIMER_DISABLE           REG_ERR(99U)
#define EHSM_ERR_ALL_COUNTER_OCCURIED        REG_ERR(100U)
#define EHSM_ERR_INVALID_COUNTER_ID          REG_ERR(101U)
#define EHSM_ERR_COUNTER_DISABLE             REG_ERR(102U)
#define EHSM_ERR_COUNTER_OVERFLOW            REG_ERR(103U)
#define EHSM_ERR_UTC_TIMER_OVERFLOW          REG_ERR(104U)
#define EHSM_ERR_INVALID_DH_PQGH_SIZE        REG_ERR(105U)
#define EHSM_ERR_DBG_PROCESSING              REG_ERR(106U)
#define EHSM_ERR_DH_PARAM_OF_PQ_IS_EVEN      REG_ERR(107U)
#define EHSM_ERR_WRONG_VERSION_COUNTER       REG_ERR(108U)
#define EHSM_ERR_INVALID_CODE_FLAG           REG_ERR(109U)
#define EHSM_ERR_WRONG_PUBKEY                REG_ERR(110U)
#define EHSM_ERR_FW_VERIFY_FAILED            REG_ERR(111U)
#define EHSM_ERR_WRONG_FW_TYPE               REG_ERR(112U)
#define EHSM_ERR_STL_FAILED                  REG_ERR(113U)
#define EHSM_ERR_WRONG_PROC_MODE             REG_ERR(114U)
#define EHSM_ERR_DATA_CHECK_ERROR            REG_ERR(115U)
#define EHSM_ERR_WRONG_TAG_SIZE              REG_ERR(116U)
#define EHSM_ERR_WRONG_NONCE_SIZE            REG_ERR(117U)
#define EHSM_ERR_WRONG_AAD_SIZE              REG_ERR(118U)

#define EHSM_ERR_SM9_ENC_FAILED      REG_ERR(119U)
#define EHSM_ERR_SM9_DEC_FAILED      REG_ERR(120U)
#define EHSM_ERR_SM9_SIGN_GEN_FAILED REG_ERR(121U)
#define EHSM_ERR_SM9_SIGN_VRY_FAILED REG_ERR(122U)
#define EHSM_ERR_SM9_EXCHANGE_FAILED REG_ERR(123U)



// definition of mailbox cmd size
// The length of AEAD cmd is at least 32 word
#define MAX_CMD_RSP_WORD_LEN (32U)
#define MAX_CMD_RSP_BYTE_LEN (20U)

#define PACKET_TYPE_MB       (1U)
#define PACKET_TYPE_DBG_AUTH (2U)

#ifdef __GNUC__
#define ATTR_INTERRUPT __attribute__((interrupt, aligned(64)))
#elif defined(__ICCRISCV__)
// for IAR, alignment is set by compiler flags
#define ATTR_INTERRUPT __interrupt
#else
#define ATTR_INTERRUPT
#endif

#define SKE_ALG_AES SKE_ALG_AES_128

#define UNUSED(x) (void)(x)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
typedef _Bool bool_t;
typedef uint64_t raddr_t;

#define TRUE  (true)
#define FALSE (false)

// TODO: change me
typedef struct {
    uint16_t cmd_id;
    uint16_t cmd_id_inv;
    uint32_t data[MAX_CMD_RSP_WORD_LEN - 1U];
} cmd_req_data_st;

typedef struct {
    uint32_t ret_code;
    uint32_t rsp_data_len;
    uint32_t data[(MAX_CMD_RSP_BYTE_LEN / sizeof(uint32_t)) - 1U];
} cmd_rsp_data_st;

typedef struct {
    uint32_t channel;
    raddr_t tag;
    raddr_t rsp_addr;
    uint8_t type;
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
