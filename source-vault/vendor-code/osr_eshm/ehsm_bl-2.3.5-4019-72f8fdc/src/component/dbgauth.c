/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "fw_verify.h"
#include "trng/trng.h"
#include "mb.h"
#include "mmap.h"
#include "dbgauth.h"
#include "types.h"
#include "util.h"
#include "sysreg.h"
#include "otp_key.h"
#include "otp_data.h"
#include "debug.h"
#include "fid.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define DEBUG_AUTH_CFI_INIT_VAL  1000
#define DEBUG_AUTH_CFI_FINAL_VAL 1009
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*Definition the debug mode*/
typedef enum {
    DEBUG_CLOSE,
    DEBUG_AUTH,
} debug_mode_e;

/*Definition the challenge struction*/
typedef struct {
    uint8_t type;
    uint32_t size;
    uint8_t *buf;
} ehsm_get_challenge_st;

/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
// The global variable of ehsm challenge
static ehsm_get_challenge_st g_ehsm_challenge[1];
// The global variable of soc challenge
static ehsm_get_challenge_st g_soc_challenge[1];
// The global variable of user auth challenge
static ehsm_get_challenge_st g_user_auth_challenge[1];
// The global buffer of ehsm challenge
static uint8_t g_ehsm_challenge_buf[EHSM_DEBUG_CHALLENGE_SIZE];
// The global buffer of soc challenge
static uint8_t g_soc_challenge_buf[EHSM_DEBUG_CHALLENGE_SIZE];
// The global buffer of user auth challenge
static uint8_t g_user_auth_challenge_buf[EHSM_DEBUG_CHALLENGE_SIZE];
// User authentication result use to advanced user authentiction operations
static volatile bool_t g_user_auth_result = false;

static fid_u32_t g_debug_auth_cfi = FID_U32_VAL(0);
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      Check life cycle and get the challenge buffer
 *
 *   @param [in] e_type The type of challenge
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t check_lifecycle_type(uint8_t e_type, uint8_t **challenge_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if (challenge_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((e_type < MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG) || (e_type > MB_EHSM_CHALLENGE_TYPE_USER_AUTH)) {
        ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
    } else if (FID_EQ(SYS_STA0_LIFECYCLE_DESTROY, life_cycle)) {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    } else {
        // nothing to do
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == e_type) {
            *challenge_data = g_ehsm_challenge_buf;
        } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == e_type) {
            *challenge_data = g_soc_challenge_buf;
        } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == e_type) {
            *challenge_data = g_user_auth_challenge_buf;
        } else {
            ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
        }
    }

    return ret;
}
/**
 *   @brief      Clear global challenge data size/type/buffer according to challenge type
 *
 *   @param [in] challenge_type The type of challenge
 *
 *   @return     uint32_t
 *
 *   @note
 */
static void clear_challenge_st(uint8_t challenge_type)
{
    if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == challenge_type) {
        util_memset(g_ehsm_challenge_buf, 0, sizeof(g_ehsm_challenge_buf));
        g_ehsm_challenge->type = EHSM_CHALLENGE_TYPE_INVALID;
        g_ehsm_challenge->size = 0;
        g_ehsm_challenge->buf = NULL;
    } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == challenge_type) {
        util_memset(g_soc_challenge_buf, 0, sizeof(g_soc_challenge_buf));
        g_soc_challenge->type = EHSM_CHALLENGE_TYPE_INVALID;
        g_soc_challenge->size = 0;
        g_soc_challenge->buf = NULL;
    } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == challenge_type) {
        util_memset(g_user_auth_challenge_buf, 0, sizeof(g_user_auth_challenge_buf));
        g_user_auth_challenge->type = EHSM_CHALLENGE_TYPE_INVALID;
        g_user_auth_challenge->size = 0;
        g_user_auth_challenge->buf = NULL;
    } else {
        // nothing to do
    }
}
/**
 *   @brief      Set global challenge data size/type/buffer according to challenge type
 *
 *   @param [in] challenge_type The type of challenge
 *   @param [in] challenge_buf The buffer address of challenge
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t set_challenge_st(uint8_t challenge_type, uint8_t *challenge_buf)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == challenge_buf) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == challenge_type) {
            g_ehsm_challenge->type = MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG;
            g_ehsm_challenge->size = EHSM_DEBUG_CHALLENGE_SIZE;
            g_ehsm_challenge->buf = challenge_buf;
        } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == challenge_type) {
            g_soc_challenge->type = MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG;
            g_soc_challenge->size = EHSM_DEBUG_CHALLENGE_SIZE;
            g_soc_challenge->buf = challenge_buf;
        } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == challenge_type) {
            g_user_auth_challenge->type = MB_EHSM_CHALLENGE_TYPE_USER_AUTH;
            g_user_auth_challenge->size = EHSM_DEBUG_CHALLENGE_SIZE;
            g_user_auth_challenge->buf = challenge_buf;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}
/**
 *   @brief      Acquire challenge buffer
 *
 *   @param [in] challenge_type The type of challenge
 *   @param [out] challenge Used to get the challenge data buffer
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t acquire_challenge_buffer(uint8_t challenge_type, ehsm_get_challenge_st **challenge)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (challenge == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == challenge_type) {
            *challenge = g_ehsm_challenge;
        } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == challenge_type) {
            *challenge = g_soc_challenge;
        } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == challenge_type) {
            *challenge = g_user_auth_challenge;
        } else {
            ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
        }
    }

    return ret;
}
/**
 *   @brief      Check debug authentication data whether is valid
 *
 *   @param [in] challenge_type The type of challenge
 *   @param [in] debug_auth_st A point pointer to struction of ehsm_debug_auth_st
 *   @param [out] challenge Used to get the challenge data buffer
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t check_auth_data(
    uint8_t challenge_type, ehsm_debug_auth_st *debug_auth, ehsm_get_challenge_st **challenge)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (debug_auth->alg) {
        case MB_BL_DEBUG_AUTH_ALG_SM2_WITH_SM3:
            if (debug_auth->signature_size != 64) {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            } else if (debug_auth->public_key_size == 65 && debug_auth->public_key[4] == 0x04) {
                debug_auth->public_key = &debug_auth->public_key[4];
            } else if (debug_auth->public_key_size == 64) {
                // add prefix 0x04
                debug_auth->public_key = &debug_auth->public_key[3];
                debug_auth->public_key[0] = 0x04;
                debug_auth->public_key_size = 65;
            } else {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            }
            break;

        case MB_BL_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256:
            if (debug_auth->signature_size != 64) {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            } else if (debug_auth->public_key_size == 64) {
                debug_auth->public_key = &debug_auth->public_key[4];
            } else {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            }
            break;

        case MB_BL_DEBUG_AUTH_ALG_SHA256_RSA:
            // support RSA2048 and RSA3072
            if (debug_auth->signature_size != debug_auth->public_key_size - 64) {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            } else if (debug_auth->public_key_size == 64 + 256 || debug_auth->public_key_size == 64 + 384) {
                debug_auth->public_key = &debug_auth->public_key[4];
            } else {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            }
            break;

        case MB_BL_DEBUG_AUTH_ALG_SM4_CMAC:
        case MB_BL_DEBUG_AUTH_ALG_AES128_CMAC:
            if (debug_auth->signature_size != 16) {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            }
            break;

        default:
            ret = EHSM_ERR_WRONG_ALGORITHM;
            break;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = acquire_challenge_buffer(challenge_type, challenge);
    }

    return ret;
}
/**
 *   @brief      Check the challenge data whehter is valid and return the debug authentication logic key id
 *
 *   @param [in] type The type of debug authentication
 *   @param [in] challenge A point pointer to struction of ehsm_get_challenge_st
 *   @param [out] otp_key_logic_id The otp logic key id is used to debug authentication
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t check_challenge_data(uint8_t type, const ehsm_get_challenge_st *challenge, uint32_t *otp_key_logic_id)
{
    uint32_t check_rsp = EHSM_ERR_SW_SUCCESS;

    if ((challenge == NULL) || (otp_key_logic_id == NULL)) {
        check_rsp = EHSM_ERR_PARAM_ERROR;
    } else {
        if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == type) {
            if ((challenge->size != EHSM_DEBUG_CHALLENGE_SIZE)
                || (challenge->type != MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG)) {
                check_rsp = EHSM_ERR_WRONG_DATA_LENGTH;
            } else {
                *otp_key_logic_id = EHSM_OTP_EHSM_DEBUG_KEY_ID;
            }
        } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == type) {
            if ((challenge->size != EHSM_DEBUG_CHALLENGE_SIZE)
                || (challenge->type != MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG)) {
                check_rsp = EHSM_ERR_WRONG_DATA_LENGTH;
            } else {
                *otp_key_logic_id = EHSM_OTP_SOC_DEBUG_KEY_ID;
            }
        } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == type) {
            if ((challenge->size != EHSM_DEBUG_CHALLENGE_SIZE)
                || (challenge->type != MB_EHSM_CHALLENGE_TYPE_USER_AUTH)) {
                check_rsp = EHSM_ERR_WRONG_DATA_LENGTH;
            } else {
                *otp_key_logic_id = EHSM_OTP_USER_AUTH_KEY_ID;
            }
        } else {
            // other type should not get here
            check_rsp = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    }

    return check_rsp;
}
/**
 *   @brief      Check the challenge data whether is valid and get the internal public key hash value
 *
 *   @param [in] type The type of debug authentication
 *   @param [in] challenge A point pointer to struction of ehsm_get_challenge_st
 *   @param [out] hash_value The hash of internel public key data only valid of PKE authentication algorithm
 *   @param [in] alg The algorithm of debug authentication
 *   @param [out] sp_key_id The index of KMU key only valid of SKE authentication algorithm
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t check_verify_data(
    uint8_t type, const ehsm_get_challenge_st *challenge, uint8_t *hash_value, uint8_t alg, uint16_t *sp_key_id)
{
    uint8_t key_type;
    uint32_t otp_key_logic_id = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    if ((challenge == NULL) || (hash_value == NULL) || (sp_key_id == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((type < MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG) || (type > MB_EHSM_CHALLENGE_TYPE_USER_AUTH)) {
        ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
    } else if (SYS_STA0_LIFECYCLE_DESTROY == life_cycle) {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    } else {
        ret = check_challenge_data(type, challenge, &otp_key_logic_id);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((alg == MB_BL_DEBUG_AUTH_ALG_SM4_CMAC) || (alg == MB_BL_DEBUG_AUTH_ALG_AES128_CMAC)) {
            key_type = OTP_KEY_ALGO_SKE_TYPE;
        } else {
            key_type = OTP_KEY_ALGO_HASH_TYPE;
        }

        ret = otpkey_check_usage(otp_key_logic_id, key_type, OTP_KEY_LEVEL_NO_CHECK, KEY_USAGE_NONE);
        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_WRONG_KEY_TYPE;
        } else {
            if ((alg == MB_BL_DEBUG_AUTH_ALG_SM4_CMAC) || (alg == MB_BL_DEBUG_AUTH_ALG_AES128_CMAC)) {
                ret = otpkey_get_phyid(otp_key_logic_id, sp_key_id);
            } else {
                ret = otpkey_read_data(otp_key_logic_id, hash_value);
            }
        }

        if (ret != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_WRONG_KEY_TYPE;
        }
    }

    return ret;
}

/**
 *   @brief      Debug authentication for sm2 algorithm
 *
 *   @param [in] debug_auth A point pointer to struction of ehsm_debug_auth_st
 *   @param [in] debug_auth A point pointer to struction of ehsm_get_challenge_st
 *   @param [in] inner_hash The hash of internel public key data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t debug_auth_sm2(
    const ehsm_debug_auth_st *debug_auth, const ehsm_get_challenge_st *challenge, const uint8_t *inner_hash)
{
    uint8_t tmpHash[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL) || (inner_hash == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_hash(HASH_SM3, (uint8_t *)&debug_auth->public_key[0], 65, tmpHash);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = util_data_sec_double_check(
                inner_hash, tmpHash, 32, EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH, EHSM_ERR_SW_SUCCESS);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_sm2_getZ(NULL, 0, debug_auth->public_key, tmpHash);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_sm2_getE(challenge->buf, challenge->size, tmpHash, tmpHash);
        }
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_debug_auth_cfi, 1);
            ret = cpt_sm2_verify(tmpHash, debug_auth->public_key, debug_auth->signature);
            fid_inc_u32(&g_debug_auth_cfi, 1);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                ret = EHSM_ERR_DEBUG_AUTH_FAILED;
            }
        }
    }

    return ret;
}

/**
 *   @brief      Debug authentication for ecdsa algorithm
 *
 *   @param [in] debug_auth A point pointer to struction of ehsm_debug_auth_st
 *   @param [in] debug_auth A point pointer to struction of ehsm_get_challenge_st
 *   @param [in] inner_hash The hash of internel public key data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t debug_auth_ecdsa(
    const ehsm_debug_auth_st *debug_auth, const ehsm_get_challenge_st *challenge, const uint8_t *inner_hash)
{
    uint8_t tmpHash[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL) || (inner_hash == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_hash(HASH_SHA256, debug_auth->public_key, 64, tmpHash);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = util_data_sec_double_check(
                inner_hash, tmpHash, 32, EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH, EHSM_ERR_SW_SUCCESS);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_hash(HASH_SHA256, challenge->buf, challenge->size, tmpHash);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_debug_auth_cfi, 1);
            ret = cpt_ecdsa_verify(
                (const eccp_curve_t *)secp256r1, tmpHash, 32, debug_auth->public_key, debug_auth->signature);
            fid_inc_u32(&g_debug_auth_cfi, 1);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                ret = EHSM_ERR_DEBUG_AUTH_FAILED;
            }
        }
    }

    util_memset(tmpHash, 0, sizeof(tmpHash));
    return ret;
}

static uint32_t debug_auth_rsa(
    const ehsm_debug_auth_st *debug_auth, const ehsm_get_challenge_st *challenge, const uint8_t *inner_hash)
{
    uint8_t tmpHash[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL) || (inner_hash == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_hash(HASH_SHA256, debug_auth->public_key, debug_auth->public_key_size, tmpHash);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = util_data_sec_double_check(
                inner_hash, tmpHash, 32, EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH, EHSM_ERR_SW_SUCCESS);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_hash(HASH_SHA256, challenge->buf, challenge->size, tmpHash);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_debug_auth_cfi, 1);
            ret = cpt_rsa_ssa_pss_verify_by_msg_digest(HASH_SHA256, HASH_SHA256, -1, tmpHash, debug_auth->public_key,
                RSA_PUBLIC_K_E_LEN * 8, debug_auth->public_key + RSA_PUBLIC_K_E_LEN,
                (debug_auth->public_key_size - RSA_PUBLIC_K_E_LEN) * 8, debug_auth->signature);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                ret = EHSM_ERR_DEBUG_AUTH_FAILED;
            }
            fid_inc_u32(&g_debug_auth_cfi, 1);
        }
    }

    util_memset(tmpHash, 0, sizeof(tmpHash));
    return ret;
}

/**
 *   @brief      Debug authentication for cmac algorithm
 *
 *   @param [in] debug_auth A point pointer to struction of ehsm_debug_auth_st
 *   @param [in] challenge A point pointer to struction of ehsm_get_challenge_st
 *   @param [in] sp_key_id The key index of kmu
 *
 *   @return     uint32_t
 *
 *   @note Because some life cycle of firmware the ske key can not be read by CPU so SKE using key through secure
 * port index
 */
static uint32_t debug_auth_cmac(
    const ehsm_debug_auth_st *debug_auth, const ehsm_get_challenge_st *challenge, uint16_t sp_key_id)
{
    cpt_ske_alg_e alg = SKE_ALG_INVALID;
    uint8_t cmac[EHSM_DBG_AUTH_CMAC_SIGNATURE_SIZE];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (debug_auth->alg == MB_BL_DEBUG_AUTH_ALG_SM4_CMAC) {
            alg = SKE_ALG_SM4;
        } else if (debug_auth->alg == MB_BL_DEBUG_AUTH_ALG_AES128_CMAC) {
            alg = SKE_ALG_AES;
        }
        if (alg == SKE_ALG_INVALID) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        fid_inc_u32(&g_debug_auth_cfi, 1);
        ret = cpt_ske_cmac(alg, SKE_GENERATE_MAC, NULL, sp_key_id, challenge->buf, challenge->size, cmac,
            EHSM_DBG_AUTH_CMAC_SIGNATURE_SIZE);
        fid_inc_u32(&g_debug_auth_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = util_data_sec_double_check(debug_auth->signature, cmac, EHSM_DBG_AUTH_CMAC_SIGNATURE_SIZE,
                EHSM_ERR_DEBUG_AUTH_FAILED, EHSM_ERR_SW_SUCCESS);
        }
        fid_inc_u32(&g_debug_auth_cfi, 4);
    }

    return ret;
}

/**
 *   @brief      Check the life cycle whether is allowed to do debug authentication or close debug authentication
 * result
 *
 *   @param [in] type Indicate which type of challenge
 *   @param [in] mode Operation mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t debug_auth_check_lifecycle_by_type(uint8_t type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    fid_inc_u32(&g_debug_auth_cfi, 1);
    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    fid_inc_u32(&g_debug_auth_cfi, 1);
    if (type == MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG) {
        if (life_cycle == SYS_STA0_LIFECYCLE_USER) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    } else if (type == MB_EHSM_CHALLENGE_TYPE_USER_AUTH) {
        if (life_cycle == SYS_STA0_LIFECYCLE_MANUFACTURE) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    } else {
        // nothing to do
    }
    fid_inc_u32(&g_debug_auth_cfi, 1);
    return ret;
}
/**
 *   @brief      Check the debug authentication data and call different algorithms to authentication
 *
 *   @param [in] type Indicate which type of challenge
 *   @param [in] debug_auth A point pointer to struction of ehsm_debug_auth_st
 *   @param [in] challenge A point pointer to struction of ehsm_get_challenge_st
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t check_and_verify(
    uint8_t type, const ehsm_debug_auth_st *debug_auth, const ehsm_get_challenge_st *challenge)
{
    uint16_t sp_key_id;
    uint8_t innerHash[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((debug_auth == NULL) || (challenge == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = check_verify_data(type, challenge, innerHash, debug_auth->alg, &sp_key_id);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        switch (debug_auth->alg) {
        case MB_BL_DEBUG_AUTH_ALG_SM2_WITH_SM3:
            ret = debug_auth_sm2(debug_auth, challenge, innerHash);
            break;
        case MB_BL_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256:
            ret = debug_auth_ecdsa(debug_auth, challenge, innerHash);
            break;
        case MB_BL_DEBUG_AUTH_ALG_SM4_CMAC:
        case MB_BL_DEBUG_AUTH_ALG_AES128_CMAC:
            ret = debug_auth_cmac(debug_auth, challenge, sp_key_id);
            break;
        case MB_BL_DEBUG_AUTH_ALG_SHA256_RSA:
            ret = debug_auth_rsa(debug_auth, challenge, innerHash);
            break;
        default:
            ret = EHSM_ERR_WRONG_ALGORITHM;
            break;
        }
    }

    return ret;
}

/**
 *   @brief      Get the challenge handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t dbgauth_get_challenge_handle(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint8_t *out_data;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_bl_get_challenge_st *cmd_data = NULL;
    uint8_t challenge_buf[EHSM_DEBUG_CHALLENGE_SIZE];
    uint32_t rand_size = 0;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_bl_get_challenge_st *)(req_data);
        ret = check_lifecycle_type(cmd_data->type, &out_data);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        rand_size = EHSM_DEBUG_CHALLENGE_SIZE;
        ret = cpt_get_rand(out_data, rand_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otp_read(OTP_UID_ADDRESS, &out_data[32], 16);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                util_memcpy(challenge_buf, out_data, rand_size);
                ret = set_challenge_st(cmd_data->type, out_data);
            } else {
                util_memset(out_data, 0, rand_size);
            }
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_write_remote_data(cmd_data->addr, challenge_buf, rand_size);
    } else {
        if (NULL != cmd_data) {
            clear_challenge_st(cmd_data->type);
        }
        g_user_auth_result = false;
    }

    util_memset(challenge_buf, 0, EHSM_DEBUG_CHALLENGE_SIZE);

    return ret;
}

/**
 *   @brief      Debug authentication handler
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t dbgauth_debug_authentication_handle(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint8_t pubkey[MAX_PUBKEY_SIZE + 4];
    uint8_t signatures[MAX_SIGNATURE_SIZE];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_debug_auth_st ehsm_debug_auth[1];
    const mb_cmd_bl_debug_auth_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_bl_debug_auth_st *)(req_data);
        if ((MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == cmd_data->type)
            || (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == cmd_data->type)
            || (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == cmd_data->type)) {
            if (((cmd_data->alg == MB_BL_DEBUG_AUTH_ALG_SM2_WITH_SM3)
                    || (cmd_data->alg == MB_BL_DEBUG_AUTH_ALG_ECCSECP256R1_WITH_SHA256)
                    || (cmd_data->alg == MB_BL_DEBUG_AUTH_ALG_SHA256_RSA))
                && (cmd_data->pub_size <= MAX_PUBKEY_SIZE) && (cmd_data->sign_size <= MAX_SIGNATURE_SIZE)) {
                ehsm_debug_auth->public_key = pubkey;
                ehsm_debug_auth->public_key_size = cmd_data->pub_size;

                ret = mmap_read_remote_data(&pubkey[4], cmd_data->pub_addr, cmd_data->pub_size);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = mmap_read_remote_data(signatures, cmd_data->sign_addr, cmd_data->sign_size);
                }
            } else if (((cmd_data->alg == MB_BL_DEBUG_AUTH_ALG_SM4_CMAC)
                    || (cmd_data->alg == MB_BL_DEBUG_AUTH_ALG_AES128_CMAC))
                && (cmd_data->sign_size <= MAX_SIGNATURE_SIZE)) {
                ret = mmap_read_remote_data(signatures, cmd_data->sign_addr, cmd_data->sign_size);

            } else {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    // if challenge type is soc need to read the debug bit map
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == cmd_data->type)) {
        if (cmd_data->soc_dbg_bitmap_size != 5U) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = mmap_read_remote_data(
                ehsm_debug_auth->soc_dbg_bitmap, cmd_data->soc_dbg_bitmap_addr, 5 * sizeof(uint32_t));
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ehsm_debug_auth->signature = signatures;
        ehsm_debug_auth->alg = cmd_data->alg;
        ehsm_debug_auth->signature_size = cmd_data->sign_size;
        ret = dbgauth_ehsm_debug_auth((uint8_t)cmd_data->type, ehsm_debug_auth);
    } else {
        // Ensure challenge is cleared on early error paths
        if (NULL != cmd_data) {
            clear_challenge_st((uint8_t)cmd_data->type);
        }
    }

    return ret;
}
/**
 *   @brief      Close the hardware secure debug port or set the authentication result to false
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note The device life cycle must be allowed.
 *
 *   Life_cycle     Ehsm_challenge_type  User_auth_challenge_type
 *    MCUTEST          enable                 enable
 *    DEVELOP          enable                 enable
 *    MANUFACTURET     enable                 disable
 *    USER             disable                enable
 *    DEBUG            enable                 enable
 *    DESTROY          enable                 enable
 *
 */
static uint32_t dbgauth_close_debug_handle(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t soc_dbg_bitmap[5];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_bl_close_debug_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_bl_close_debug_st *)(req_data);
    }
    // if challenge type is soc need to read the debug bit map
    if ((EHSM_ERR_SW_SUCCESS == ret) && (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == cmd_data->type)) {
        if (cmd_data->soc_dbg_bitmap_size != 5U) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = mmap_read_remote_data(soc_dbg_bitmap, cmd_data->soc_dbg_bitmap_addr, 5 * sizeof(uint32_t));
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Check the operation life cycle is allowed
        if (debug_auth_check_lifecycle_by_type(cmd_data->type) != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else {
            if (cmd_data->type == MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG) {
                sysreg_disable_hsm_dbg();
            } else if (cmd_data->type
                == MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG) { /*  Soc debug bit map size is 5 words.
                                                        the first 4 words indicate whether to close the corresponding
                                                        hardware debug port per bit. the last word only the lowest bit
                                                        is valid(indicate whether 129 bits of the hardware debugging
                                                        port are disabled).
                                                    */
                sysreg_disable_soc_dbg_ext(0, soc_dbg_bitmap[0]);
                sysreg_disable_soc_dbg_ext(1, soc_dbg_bitmap[1]);
                sysreg_disable_soc_dbg_ext(2, soc_dbg_bitmap[2]);
                sysreg_disable_soc_dbg_ext(3, soc_dbg_bitmap[3]);

                if ((soc_dbg_bitmap[4] & 0x1U) == 0x1U) {
                    sysreg_disable_soc_dbg();
                }
            } else if (cmd_data->type == MB_EHSM_CHALLENGE_TYPE_USER_AUTH) {
                g_user_auth_result = false;
            } else {
                ret = EHSM_ERR_NOT_SUPPORT;
            }
        }
    }

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      Get the result of user authentication
 *
 *   @param
 *
 *   @return     bool_t the result of authentication
 *
 *   @note
 */
bool_t dbgauth_srv_acquire_user_auth_result(void)
{
    return g_user_auth_result;
}

/**
 *   @brief      debug authentication service processing function entry
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t dbgauth_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (req_data->cmd_id) {
        case MB_CMD_ID_BL_GET_CHALLENGE:
            ret = dbgauth_get_challenge_handle(req_data, rsp_data);
            break;
        case MB_CMD_ID_BL_DEBUG_AUTH:
            ret = dbgauth_debug_authentication_handle(req_data, rsp_data);
            break;
        case MB_CMD_ID_BL_CLOSE_DEBUG:
            ret = dbgauth_close_debug_handle(req_data, rsp_data);
            break;
        default:
            ret = EHSM_ERR_INVALID_CMD;
            break;
        }
    }

    return ret;
}

/**
 *   @brief      generate challenge, called by dbgcmd_parser module
 *
 *   @param [in] type
 *   @param [in] challenge_buf
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t dbgauth_get_challenge(uint8_t type, uint8_t *challenge_buf)
{
    uint8_t *out_data;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == challenge_buf) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = check_lifecycle_type(type, &out_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_get_rand(out_data, 32);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = otp_read(OTP_UID_ADDRESS, &out_data[32], 16);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    util_memcpy(challenge_buf, out_data, EHSM_DEBUG_CHALLENGE_SIZE);
                    ret = set_challenge_st(type, out_data);
                } else {
                    util_memset(out_data, 0, EHSM_DEBUG_CHALLENGE_SIZE);
                }
            }
        }
    }

    return ret;
}

/**
 *   @brief      close debug, called by dbgcmd_parser module
 *
 *   @param [in] type
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t dbgauth_close_debug(uint8_t type)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // Check the operation life cycle is allowed
    if (debug_auth_check_lifecycle_by_type(type) != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    } else {
        if (type == MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG) {
            sysreg_disable_hsm_dbg();
        } else if (type == MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG) {
            sysreg_disable_soc_dbg();
        } else if (type == MB_EHSM_CHALLENGE_TYPE_USER_AUTH) {
            g_user_auth_result = false;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

/**
 *   @brief      Do debug authenticaion and set the hardware debug port to open
 *
 *   @param [in] challenge_type Indicate which type of challenge
 *   @param [in] debug_auth_st A point pointer to struction of ehsm_debug_auth_st
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t dbgauth_ehsm_debug_auth(uint8_t challenge_type, ehsm_debug_auth_st *debug_auth_st)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_get_challenge_st *challenge;

    fid_set_u32(&g_debug_auth_cfi, DEBUG_AUTH_CFI_INIT_VAL);
    if (debug_auth_st == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = debug_auth_check_lifecycle_by_type(challenge_type);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = check_auth_data(challenge_type, debug_auth_st, &challenge);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = check_and_verify(challenge_type, debug_auth_st, challenge);
        }
    }

    if (FID_EQ(ret, EHSM_ERR_SW_SUCCESS)) {
        if (fid_get_u32(&g_debug_auth_cfi) != DEBUG_AUTH_CFI_FINAL_VAL) {
            fid_panic();
        }
        fid_delay();
        if (FID_EQ(MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG, challenge_type)) {
            sysreg_enable_hsm_dbg();
        } else if (FID_EQ(MB_EHSM_CHALLENGE_TYPE_USER_AUTH, challenge_type)) {
            g_user_auth_result = true;
        } else {
            /*  Soc debug bit map size is 5 words.
                the first 4 words indicate whether to open the corresponding hardware debug port per bit.
                the last word only the lowest bit is valid(indicate whether 129 bits of the hardware debugging port
               are enabled).
            */
            sysreg_enable_soc_dbg_ext(0, debug_auth_st->soc_dbg_bitmap[0]);
            sysreg_enable_soc_dbg_ext(1, debug_auth_st->soc_dbg_bitmap[1]);
            sysreg_enable_soc_dbg_ext(2, debug_auth_st->soc_dbg_bitmap[2]);
            sysreg_enable_soc_dbg_ext(3, debug_auth_st->soc_dbg_bitmap[3]);

            if ((debug_auth_st->soc_dbg_bitmap[4] & 0x1U) == 0x1U) {
                sysreg_enable_soc_dbg();
            }
        }
    }

    clear_challenge_st(challenge_type);
    return ret;
}

/**
 *   @brief      generate challenge, called by mbcmd_parser module
 *
 *   @param [in] type
 *   @param [in] challenge_buf
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t dbgauth_read_challenge(uint8_t type, uint8_t *challenge_buf)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (MB_EHSM_CHALLENGE_TYPE_EHSM_DEBUG == type) {
        if (g_ehsm_challenge->buf == NULL) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            util_memcpy(challenge_buf, g_ehsm_challenge->buf, EHSM_DEBUG_CHALLENGE_SIZE);
        }
    } else if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == type) {
        if (g_soc_challenge->buf == NULL) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            util_memcpy(challenge_buf, g_soc_challenge->buf, EHSM_DEBUG_CHALLENGE_SIZE);
        }
    } else if (MB_EHSM_CHALLENGE_TYPE_USER_AUTH == type) {
        if (g_user_auth_challenge->buf == NULL) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            util_memcpy(challenge_buf, g_user_auth_challenge->buf, EHSM_DEBUG_CHALLENGE_SIZE);
        }
    } else {
        ret = EHSM_ERR_WRONG_CHALLENGE_TYPE;
    }

    return ret;
}
