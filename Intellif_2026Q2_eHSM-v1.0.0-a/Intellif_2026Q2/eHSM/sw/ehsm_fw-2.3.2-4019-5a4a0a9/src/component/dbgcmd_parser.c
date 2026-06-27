/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "dbgcmd_parser.h"
#include "dbgauth_srv.h"
#include "mb.h"
#include "util.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
#define DBG_CMD_END 0x40U

#define DBG_TYPE_OFFSET    (3U)
#define DBG_ALG_OFFSET     (4U)
#define DBG_REQ_END_OFFSET (5U)
#define DBG_DIS_END_OFFSET (4U)

#define PUBLIC_KEY_SIZE         (128U)
#define RSA2048_SIGN_SIZE       (512U)
#define RSA3072_SIGN_SIZE       (768U)
#define RSA2048_PUBLIC_KEY_SIZE (128U + RSA2048_SIGN_SIZE)
#define RSA3072_PUBLIC_KEY_SIZE (128U + RSA3072_SIGN_SIZE)

#define DBG_VERIFY_ALG_SM2         0x31U
#define DBG_VERIFY_ALG_ECC256      0x32U
#define DBG_VERIFY_ALG_SM4_CMAC    0x33U
#define DBG_VERIFY_ALG_AES128_CMAC 0x34U
#define DBG_VERIFY_ALG_RSA         0x35U

#define DBG_CMAC_SIGNATURE_SIZE 16U

#define DBG_PARAMETER_ERROR 0x50
#define DBG_DATA_ERROR      0x51
#define DBG_HASH_ERROR      0x52
#define DBG_VERIFY_ERROR    0x53

#define DBG_TYPE_EHSM 0x31U
#define DBG_TYPE_SOC  0x33U

/* 32 bytes of random number, 16 bytes of UID */
#define EHSM_DEBUG_CHALLENGE_SIZE (32U + 16U)

// End character of command
#define DBG_END 0x40U
// ASCCII of character ':'
#define DBG_COLON 0x3AU
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
static const uint8_t ERROR_VERIFY_ERROR[] = { 0x46, 0x41, 0x49, 0x4C }; //"FAIL"
static const uint8_t ERROR_VERIFY_PARAMETER[]
    = { 0x5F, 0x50, 0x41, 0x52, 0x41, 0x4D, 0x45, 0x54, 0x45, 0x52, 0x5F }; //"_PARAMETER_"
static const uint8_t VERIFY_SUCCESS[] = { 0x50, 0x41, 0x53, 0x53 };         //"PASS"
static const uint8_t DBG_END_HEAD[] = { 0x45, 0x4E, 0x44 };                 //"END"

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t dbg_check_challenge_param(const dbg_cmd_param_st *cmd_param);
static uint32_t dbg_check_close_debug_param(const dbg_cmd_param_st *cmd_param);
static uint32_t dbg_build_verify_resp(uint8_t *ret_buf, uint32_t ret_code);
static uint32_t dbg_build_close_debug_resp(uint8_t *ret_buf, uint32_t ret_code);
static uint32_t dbg_build_random_resp(
    uint8_t *challenge_buf, uint32_t challenge_len, uint8_t *ret_buf, uint32_t ret_code, uint8_t type);
static void dbg_chartohex(const uint8_t *in, uint16_t len, uint8_t *out);
static void dbg_hextochar(const uint8_t *in, uint16_t len, uint8_t *out);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t dbg_get_challenge(dbg_cmd_param_st *cmd_param)
{
    uint32_t rsp_len = 0;
    uint32_t ret = 0;
    uint32_t param_ret = 0;
    uint8_t challenge_buf[EHSM_DEBUG_CHALLENGE_SIZE];
    uint8_t challenge_type;

    param_ret = dbg_check_challenge_param(cmd_param);
    if (EHSM_ERR_SW_SUCCESS != param_ret) {
        rsp_len = dbg_build_random_resp(NULL, 0, cmd_param->reps, param_ret, cmd_param->data[DBG_TYPE_OFFSET]);
        cmd_param->reps_size = rsp_len;
        ret = EHSM_ERR_DBG_PROTO_ERROR;
    } else {
        challenge_type = (uint8_t)(cmd_param->data[DBG_TYPE_OFFSET] - 0x30U);
        ret = dbgauth_get_challenge(challenge_type, challenge_buf);
        cmd_param->reps_size = dbg_build_random_resp(
            challenge_buf, EHSM_DEBUG_CHALLENGE_SIZE, cmd_param->reps, ret, cmd_param->data[DBG_TYPE_OFFSET]);
    }

    return ret;
}

static uint32_t dbg_close_debug(dbg_cmd_param_st *cmd_param)
{
    uint32_t ret = 0;
    uint32_t param_ret = 0;
    uint8_t type;

    param_ret = dbg_check_close_debug_param(cmd_param);
    if (param_ret != EHSM_ERR_SW_SUCCESS) {
        cmd_param->reps_size = dbg_build_close_debug_resp(cmd_param->reps, param_ret);
        ret = EHSM_ERR_DBG_PROTO_ERROR;
    } else {
        type = (uint8_t)(cmd_param->data[DBG_TYPE_OFFSET] - 0x30U);
        ret = dbgauth_close_debug(type);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            cmd_param->reps_size = dbg_build_close_debug_resp(cmd_param->reps, ret);
        } else {
            cmd_param->reps_size = dbg_build_close_debug_resp(cmd_param->reps, DBG_PARAMETER_ERROR);
        }
    }

    return ret;
}

static uint32_t dbg_check_debug_verify_param(const dbg_cmd_param_st *cmd_param)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == cmd_param) {
        ret = DBG_PARAMETER_ERROR;
    } else if (DBG_COLON != cmd_param->data[DBG_ALG_OFFSET + 1U]) {
        ret = DBG_DATA_ERROR;
    } else if ((DBG_VERIFY_ALG_SM2 != cmd_param->data[DBG_ALG_OFFSET])
        && (DBG_VERIFY_ALG_ECC256 != cmd_param->data[DBG_ALG_OFFSET])
        && (DBG_VERIFY_ALG_SM4_CMAC != cmd_param->data[DBG_ALG_OFFSET])
        && (DBG_VERIFY_ALG_AES128_CMAC != cmd_param->data[DBG_ALG_OFFSET])
        && (DBG_VERIFY_ALG_RSA != cmd_param->data[DBG_ALG_OFFSET])) {
        ret = DBG_PARAMETER_ERROR;
    } else {
        // do nothing
        ;
    }
    return ret;
}

static uint32_t dbg_check_challenge_param(const dbg_cmd_param_st *cmd_param)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == cmd_param) {
        ret = DBG_PARAMETER_ERROR;
    } else if ((cmd_param->data[DBG_ALG_OFFSET] != DBG_VERIFY_ALG_SM2)
        && (cmd_param->data[DBG_ALG_OFFSET] != DBG_VERIFY_ALG_ECC256)
        && (cmd_param->data[DBG_ALG_OFFSET] != DBG_VERIFY_ALG_SM4_CMAC)
        && (cmd_param->data[DBG_ALG_OFFSET] != DBG_VERIFY_ALG_AES128_CMAC)
        && (cmd_param->data[DBG_ALG_OFFSET] != DBG_VERIFY_ALG_RSA)) {
        ret = DBG_PARAMETER_ERROR;
    } else if (cmd_param->data[DBG_REQ_END_OFFSET] != DBG_END) {
        ret = DBG_DATA_ERROR;
    } else if ((cmd_param->data[DBG_TYPE_OFFSET] != DBG_TYPE_EHSM)
        && (cmd_param->data[DBG_TYPE_OFFSET] != DBG_TYPE_SOC)) {
        ret = DBG_PARAMETER_ERROR;
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

static uint32_t dbg_check_close_debug_param(const dbg_cmd_param_st *cmd_param)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == cmd_param) {
        ret = DBG_PARAMETER_ERROR;
    } else if (cmd_param->data[DBG_DIS_END_OFFSET] != DBG_END) {
        ret = DBG_DATA_ERROR;
    } else {
        // do nothing
        ;
    }

    return ret;
}

static void dbg_parse_asym_verify_data(const dbg_cmd_param_st *cmd_param, ehsm_debug_auth_st *ehsm_debug_auth)
{
    uint32_t pub_key_index = DBG_ALG_OFFSET + 2U;
    uint8_t hex_buffer[192U];
    uint32_t signature_index = DBG_ALG_OFFSET + PUBLIC_KEY_SIZE + 2U + 1U;

    if ((NULL == cmd_param) || (NULL == cmd_param->data) || (NULL == ehsm_debug_auth)) {
        // do nothing
    } else {
        util_memset(hex_buffer, 0, sizeof(hex_buffer));
        dbg_chartohex(&cmd_param->data[pub_key_index], 128U, hex_buffer);
        util_memcpy(&cmd_param->data[pub_key_index + 4U], hex_buffer, 64U);
        dbg_chartohex(&cmd_param->data[signature_index], 128U, hex_buffer);
        util_memcpy(&cmd_param->data[signature_index], hex_buffer, 64U);

        ehsm_debug_auth->signature_size = PUBLIC_KEY_SIZE / 2U;
        ehsm_debug_auth->public_key_size = PUBLIC_KEY_SIZE / 2U;
        ehsm_debug_auth->public_key = &cmd_param->data[pub_key_index];
        ehsm_debug_auth->signature = &cmd_param->data[signature_index];
    }
}

static void dbg_parse_rsa_data(const dbg_cmd_param_st *cmd_param, ehsm_debug_auth_st *ehsm_debug_auth)
{
    uint32_t pub_key_index = DBG_ALG_OFFSET + 2U;
    uint8_t hex_buffer[2048];
    uint32_t signature_index;
    uint32_t colon_index = DBG_ALG_OFFSET + RSA2048_PUBLIC_KEY_SIZE + 2U;
    uint32_t pubkey_size;
    uint32_t sign_size;

    if ((NULL == cmd_param) || (NULL == cmd_param->data) || (NULL == ehsm_debug_auth)) {
        // do nothing
    } else {
        if (cmd_param->data_size <= colon_index) {
            // do nothing
        } else {
            if (cmd_param->data[colon_index] == DBG_COLON) {
                pubkey_size = RSA2048_PUBLIC_KEY_SIZE;
                signature_index = pub_key_index + pubkey_size + 1U;
                sign_size = RSA2048_SIGN_SIZE;
            } else {
                pubkey_size = RSA3072_PUBLIC_KEY_SIZE;
                signature_index = pub_key_index + pubkey_size + 1U;
                sign_size = RSA3072_SIGN_SIZE;
            }

            util_memset(hex_buffer, 0, sizeof(hex_buffer));
            dbg_chartohex(&cmd_param->data[pub_key_index], (uint16_t)pubkey_size, hex_buffer);
            util_memcpy(&cmd_param->data[pub_key_index + 4U], hex_buffer, pubkey_size >> 1U);
            dbg_chartohex(&cmd_param->data[signature_index], (uint16_t)sign_size, hex_buffer);
            util_memcpy(&cmd_param->data[signature_index], hex_buffer, sign_size >> 1U);

            ehsm_debug_auth->signature_size = sign_size >> 1;
            ehsm_debug_auth->public_key_size = pubkey_size >> 1;
            ehsm_debug_auth->public_key = &cmd_param->data[pub_key_index];
            ehsm_debug_auth->signature = &cmd_param->data[signature_index];
        }
    }
}

static void dbg_parse_sym_verify_data(const dbg_cmd_param_st *cmd_param, ehsm_debug_auth_st *ehsm_debug_auth)
{
    uint8_t hex_buffer[192];
    uint32_t signature_index;

    if ((NULL == cmd_param) || (NULL == cmd_param->data) || (NULL == ehsm_debug_auth)) {
        // do nothing
    } else {
        util_memset(hex_buffer, 0, sizeof(hex_buffer));
        signature_index = DBG_ALG_OFFSET + 2U;
        dbg_chartohex(&cmd_param->data[signature_index], DBG_CMAC_SIGNATURE_SIZE * 2U, hex_buffer);
        util_memcpy(&cmd_param->data[signature_index], hex_buffer, DBG_CMAC_SIGNATURE_SIZE);
        ehsm_debug_auth->signature_size = DBG_CMAC_SIGNATURE_SIZE;
        ehsm_debug_auth->signature = &cmd_param->data[signature_index];
    }
}

static uint32_t dbg_parse_verify_data(const dbg_cmd_param_st *cmd_param, ehsm_debug_auth_st *ehsm_debug_auth)
{
    uint8_t alg;
    uint32_t pub_key_index = DBG_ALG_OFFSET + 2U;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == cmd_param) || (NULL == cmd_param->data) || (NULL == ehsm_debug_auth)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        alg = cmd_param->data[DBG_ALG_OFFSET];
        switch (alg) {
        case DBG_VERIFY_ALG_SM2:
            dbg_parse_asym_verify_data(cmd_param, ehsm_debug_auth);
            ehsm_debug_auth->alg = MB_AUTH_ALG_SM2_WITH_SM3;
            cmd_param->data[pub_key_index + 3U] = 0x04U;
            break;
        case DBG_VERIFY_ALG_ECC256:
            dbg_parse_asym_verify_data(cmd_param, ehsm_debug_auth);
            ehsm_debug_auth->alg = MB_AUTH_ALG_ECCSECP256R1_WITH_SHA256;
            break;
        case DBG_VERIFY_ALG_SM4_CMAC:
            dbg_parse_sym_verify_data(cmd_param, ehsm_debug_auth);
            ehsm_debug_auth->alg = MB_AUTH_ALG_SM4_CMAC;
            break;
        case DBG_VERIFY_ALG_AES128_CMAC:
            dbg_parse_sym_verify_data(cmd_param, ehsm_debug_auth);
            ehsm_debug_auth->alg = MB_AUTH_ALG_AES128_CMAC;
            break;
        case DBG_VERIFY_ALG_RSA:
            dbg_parse_rsa_data(cmd_param, ehsm_debug_auth);
            ehsm_debug_auth->alg = MB_AUTH_ALG_SHA256_RSA;
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }
    }

    return ret;
}

static uint32_t dbg_debug_verify(dbg_cmd_param_st *cmd_param)
{
    uint8_t type;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_debug_auth_st ehsm_debug_auth[1];

    util_memset(ehsm_debug_auth, 0, sizeof(ehsm_debug_auth_st));
    ret = dbg_check_debug_verify_param(cmd_param);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        type = (uint8_t)(cmd_param->data[DBG_TYPE_OFFSET] - 0x30U);
        if (MB_EHSM_CHALLENGE_TYPE_SOC_DEBUG == type) {
            util_memset(ehsm_debug_auth->soc_dbg_bitmap, 0xFF, sizeof(ehsm_debug_auth->soc_dbg_bitmap));
        }
        ret = dbg_parse_verify_data(cmd_param, ehsm_debug_auth);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = dbgauth_ehsm_debug_auth(type, ehsm_debug_auth);
            if (ret == EHSM_ERR_SW_SUCCESS) {
                cmd_param->reps_size = dbg_build_verify_resp(cmd_param->reps, ret);
            } else if (ret == EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH) {
                cmd_param->reps_size = dbg_build_verify_resp(cmd_param->reps, DBG_HASH_ERROR);
            } else {
                cmd_param->reps_size = dbg_build_verify_resp(cmd_param->reps, DBG_VERIFY_ERROR);
            }
        } else {
            cmd_param->reps_size = dbg_build_verify_resp(cmd_param->reps, DBG_PARAMETER_ERROR);
        }
    } else {
        cmd_param->reps_size = dbg_build_verify_resp(cmd_param->reps, DBG_PARAMETER_ERROR);
    }

    return ret;
}

static uint32_t dbg_build_random_resp(
    uint8_t *challenge_buf, uint32_t challenge_len, uint8_t *ret_buf, uint32_t ret_code, uint8_t type)
{
    uint32_t rsp_len = 0;
    static const uint8_t ERROR_RANDOM[] = { 0x67, 0x65, 0x74, 0x20, 0x72, 0x61, 0x6E, 0x64, 0x6F, 0x6D, 0x20, 0x66,
        0x61, 0x69, 0x6C };                                     //"get random fail"
    static const uint8_t DBG_CHA_HEAD[] = { 0x43, 0x48, 0x41 }; //"CHA"

    if (NULL == ret_buf) {
        rsp_len = 0;
    } else {
        util_memcpy(ret_buf, DBG_CHA_HEAD, sizeof(DBG_CHA_HEAD));
        rsp_len = (uint32_t)sizeof(DBG_CHA_HEAD);
        // If type is '@', do not set type
        if (type != '@') {
            ret_buf[rsp_len] = type;
            rsp_len++;
        }
        ret_buf[rsp_len] = DBG_COLON;
        rsp_len++;

        if ((ret_code == EHSM_ERR_SW_SUCCESS) && (NULL != challenge_buf)) {
            dbg_hextochar(challenge_buf, (uint16_t)challenge_len, &ret_buf[rsp_len]);
            rsp_len += challenge_len * 2U;
        } else {
            util_memcpy(&ret_buf[rsp_len], ERROR_RANDOM, sizeof(ERROR_RANDOM));
            rsp_len += (uint32_t)sizeof(ERROR_RANDOM);
        }

        if (challenge_buf != NULL) {
            util_memset(challenge_buf, 0, challenge_len);
        } else {
            ;
        }

        ret_buf[rsp_len] = DBG_END;
        rsp_len++;
    }

    return rsp_len;
}

static uint32_t dbg_build_verify_resp(uint8_t *ret_buf, uint32_t ret_code)
{
    uint32_t index = (uint32_t)sizeof(DBG_END_HEAD);
    static const uint8_t ERROR_VERIFY_VERIFY[] = { 0x5F, 0x56, 0x45, 0x52, 0x49, 0x46, 0x59, 0x5F }; //"_VERIFY_"
    static const uint8_t ERROR_VERIFY_HASH[] = { 0x5F, 0x50, 0x55, 0x42, 0x4B, 0x45, 0x59, 0x5F };   //"_PUBKEY_"
    static const uint8_t ERROR_VERIFY_DATA[] = { 0x5F, 0x44, 0x41, 0x54, 0x41, 0x5F };               //"_DATA_"

    if (NULL == ret_buf) {
        index = 0;
    } else {
        util_memcpy(ret_buf, DBG_END_HEAD, sizeof(DBG_END_HEAD));
        if (EHSM_ERR_SW_SUCCESS == ret_code) {
            util_memcpy(&ret_buf[index], VERIFY_SUCCESS, sizeof(VERIFY_SUCCESS));
            index += (uint32_t)sizeof(VERIFY_SUCCESS);
        } else {
            switch (ret_code) {
            case DBG_VERIFY_ERROR:
                util_memcpy(&ret_buf[index], ERROR_VERIFY_VERIFY, sizeof(ERROR_VERIFY_VERIFY));
                index += (uint32_t)sizeof(ERROR_VERIFY_VERIFY);
                break;
            case DBG_PARAMETER_ERROR:
                util_memcpy(&ret_buf[index], ERROR_VERIFY_PARAMETER, sizeof(ERROR_VERIFY_PARAMETER));
                index += (uint32_t)sizeof(ERROR_VERIFY_PARAMETER);
                break;
            case DBG_HASH_ERROR:
                util_memcpy(&ret_buf[index], ERROR_VERIFY_HASH, sizeof(ERROR_VERIFY_HASH));
                index += (uint32_t)sizeof(ERROR_VERIFY_HASH);
                break;
            case DBG_DATA_ERROR:
                util_memcpy(&ret_buf[index], ERROR_VERIFY_DATA, sizeof(ERROR_VERIFY_DATA));
                index += (uint32_t)sizeof(ERROR_VERIFY_DATA);
                break;
            default:
                util_memcpy(&ret_buf[index], ERROR_VERIFY_VERIFY, sizeof(ERROR_VERIFY_VERIFY));
                index += (uint32_t)sizeof(ERROR_VERIFY_VERIFY);
                break;
            }
            util_memcpy(&ret_buf[index], ERROR_VERIFY_ERROR, sizeof(ERROR_VERIFY_ERROR));
            index += (uint32_t)sizeof(ERROR_VERIFY_ERROR);
        }
        ret_buf[index] = DBG_END;
        index++;
    }

    return index;
}

static uint32_t dbg_build_close_debug_resp(uint8_t *ret_buf, uint32_t ret_code)
{
    uint32_t index = (uint32_t)sizeof(DBG_END_HEAD);

    if (NULL == ret_buf) {
        index = 0;
    } else {
        util_memcpy(ret_buf, DBG_END_HEAD, sizeof(DBG_END_HEAD));
        if (EHSM_ERR_SW_SUCCESS == ret_code) {
            util_memcpy(&ret_buf[index], VERIFY_SUCCESS, sizeof(VERIFY_SUCCESS));
            index += (uint32_t)sizeof(VERIFY_SUCCESS);
        } else {
            util_memcpy(&ret_buf[index], ERROR_VERIFY_PARAMETER, sizeof(ERROR_VERIFY_PARAMETER));
            index += (uint32_t)sizeof(ERROR_VERIFY_PARAMETER);
            util_memcpy(&ret_buf[index], ERROR_VERIFY_ERROR, sizeof(ERROR_VERIFY_ERROR));
            index += (uint32_t)sizeof(ERROR_VERIFY_ERROR);
        }
        ret_buf[index] = DBG_END;
        index++;
    }

    return index;
}

static uint32_t dbg_handle_cmd(dbg_cmd_param_st *cmd_param)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    bool_t handled = false;
    static const uint8_t DBG_HEAD_REQ[3] = { 0x52, 0x45, 0x51 };        //"REQ"
    static const uint8_t DBG_HEAD_RSP[3] = { 0x52, 0x53, 0x50 };        //"RSP"
    static const uint8_t DBG_HEAD_DIS[3] = { 0x44, 0x49, 0x53 };        //"DIS"
    static const uint8_t DBG_HEAD_TEST[4] = { 0x54, 0x45, 0x53, 0x54 }; //"TEST"

    if ((NULL == cmd_param) || (NULL == cmd_param->data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (util_memcmp(DBG_HEAD_REQ, cmd_param->data, 3) == 0U) {
            ret = dbg_get_challenge(cmd_param);
            handled = true;
        }
        if (util_memcmp(DBG_HEAD_RSP, cmd_param->data, 3) == 0U) {
            ret = dbg_debug_verify(cmd_param);
            handled = true;
        }
        if (util_memcmp(DBG_HEAD_DIS, cmd_param->data, 3) == 0U) {
            ret = dbg_close_debug(cmd_param);
            handled = true;
        }
        if (util_memcmp(DBG_HEAD_TEST, cmd_param->data, 4) == 0U) {
            cmd_param->reps_size
                = (cmd_param->data_size > MAX_DBG_PROTO_SIZE) ? MAX_DBG_PROTO_SIZE : cmd_param->data_size;
            util_memcpy(cmd_param->reps, cmd_param->data, cmd_param->reps_size);
            ret = EHSM_ERR_SW_SUCCESS;
            handled = true;
        }

        if (handled == false) {
            ret = EHSM_ERR_DBG_PROTO_ERROR;
        }
    }

    return ret;
}

static uint32_t dbg_analysis_parser(dbgcmdparser_st *parser)
{
    uint32_t ret = 0;
    dbg_cmd_param_st cmd_param[1];
    uint8_t reps_buffer[MAX_DBG_PROTO_SIZE];

    if (NULL == parser) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memset(reps_buffer, 0, MAX_DBG_PROTO_SIZE);
        parser->state = DBG_STATE_PROCESS;
        cmd_param->data = parser->cmd_buffer;
        cmd_param->data_size = parser->data_size;
        cmd_param->reps = reps_buffer;
        cmd_param->reps_size = 0;

        if (cmd_param->data[cmd_param->data_size - 1U] == DBG_CMD_END) {
            ret = dbg_handle_cmd(cmd_param);
            parser->rsp_size = cmd_param->reps_size;
            util_memcpy(parser->cmd_buffer, reps_buffer, parser->rsp_size);
        } else {
            ret = EHSM_ERR_DBG_PROTO_ERROR;
        }
    }

    return ret;
}

static uint8_t dbg_hextochar_interenl(uint8_t tmp)
{
    uint8_t convert = tmp;

    convert &= 0x0fU;
    if (convert >= 10U) {
        convert = convert - 0x0aU + 0x41U;
    } else {
        convert = convert + 0x30U;
    }

    return convert;
}

static uint8_t dbg_chartohex_interenl(uint8_t tmp)
{
    uint8_t convert = tmp;

    if ((convert >= 0x30U) && (convert <= 0x39U)) {
        convert -= (uint8_t)0x30U;
    } else if ((convert >= 0x41U) && (convert <= 0x46U)) {
        convert -= (uint8_t)0x37U;
    } else if ((convert >= 0x61U) && (convert <= 0x66U)) {
        convert -= (uint8_t)0x57U;
    } else {
        convert = 0xFFU;
    }

    return convert;
}
/**
 * @brief Convert a binary buffer to a hexadecimal ASCII string.
 *
 * This function takes a binary buffer and converts each byte to two hexadecimal
 * ASCII characters. The resulting string is stored in the output buffer.
 *
 * @param in A pointer to the input binary buffer.
 * @param len The length of the input binary buffer in bytes.
 * @param out A pointer to the output buffer where the hexadecimal string will be stored.
 */
static void dbg_hextochar(const uint8_t *in, uint16_t len, uint8_t *out)
{
    uint8_t tmp[256 * 2];

    if (len > 256U) {
        len = 256U;
    }

    for (uint16_t i = 0; i < len; i++) {
        tmp[i * 2U] = dbg_hextochar_interenl(in[i] >> 4U);
        tmp[(i * 2U) + 1U] = dbg_hextochar_interenl(in[i]);
    }

    util_memcpy(out, tmp, ((uint32_t)len * 2U));
}

/**
 * @brief Convert a hexadecimal ASCII string to a binary buffer.
 *
 * This function takes a hexadecimal ASCII string and converts each pair of
 * characters to one binary byte. The resulting buffer is stored in the output buffer.
 *
 * @param in A pointer to the input hexadecimal ASCII string.
 * @param len The length of the input hexadecimal string in characters.
 * @param out A pointer to the output buffer where the binary data will be stored.
 */
static void dbg_chartohex(const uint8_t *in, uint16_t len, uint8_t *out)
{
    uint8_t tmp[(1024 + 1) / 2];

    for (uint16_t i = 0; i < len; i = i + 2U) {
        tmp[i / 2U] = 0;
        tmp[i / 2U] = dbg_chartohex_interenl(in[i]);

        if ((i + 1U) < len) {
            tmp[i / 2U] = (uint8_t)((tmp[i / 2U] << 4U) + dbg_chartohex_interenl(in[i + 1U]));
        }
    }

    util_memcpy(out, tmp, (((uint32_t)len + 1U) / 2U));
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t dbgcmdpars_init_parser(dbgcmdparser_st *parser)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == parser) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        parser->rsp_size = 0;
        parser->data_size = 0;
        parser->rsp_index = 0;
        (void)util_memset(parser->cmd_buffer, 0, MAX_DBG_PROTO_SIZE);
        parser->state = DBG_STATE_INIT;
    }

    return ret;
}

uint32_t dbgcmdpars_append_data(dbgcmdparser_st *parser, const uint8_t *data, uint32_t size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t recv_index;
    uint32_t size_temp = 0;

    if ((NULL == parser) || (NULL == data) || (size == 0U)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if ((DBG_STATE_INIT == parser->state) || (DBG_STATE_RECEIVING == parser->state)) {
            parser->state = DBG_STATE_RECEIVING;
            recv_index = parser->data_size;
            while ((size_temp < size) && (recv_index < MAX_DBG_PROTO_SIZE)) {
                parser->cmd_buffer[recv_index] = data[size_temp];
                recv_index++;
                if (data[size_temp] == DBG_CMD_END) {
                    parser->state = DBG_STATE_RECV_COMPLETE;
                    break;
                }
                size_temp++;
            }
            parser->data_size = recv_index;
        } else {
            ret = EHSM_ERR_DBG_PROCESSING;
        }

        if (parser->data_size >= MAX_DBG_PROTO_SIZE) {
            (void)dbgcmdpars_init_parser(parser);
            ret = EHSM_ERR_DBG_BUF_OVERFLOW;
        }
    }

    return ret;
}

uint32_t dbgcmdpars_recv_complete(const dbgcmdparser_st *parser)
{
    uint32_t ret;

    if (NULL == parser) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (parser->state == DBG_STATE_RECV_COMPLETE) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            ret = EHSM_ERR_DBG_CMD_INCOMPLETE;
        }
    }

    return ret;
}

uint32_t dbgcmdpars_srv_handler(const cmd_packet_st *packet)
{
    uint32_t ret;
    const dbg_cmd_st *dbg_cmd = NULL;
    dbgcmdparser_st *parser = NULL;

    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        dbg_cmd = (const dbg_cmd_st *)&packet->cmd_data;
        parser = dbg_cmd->parser;
        if (NULL == parser) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = dbg_analysis_parser(parser);
        }
    }

    return ret;
}

uint32_t dbgcmdpars_trans_to_packet(dbgcmdparser_st *parser, cmd_packet_st *packet)
{
    uint32_t ret;

    if ((NULL == parser) || (NULL == packet)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        dbg_cmd_st *dbg_cmd = (dbg_cmd_st *)&packet->cmd_data;
        dbg_cmd->cmd_id = DBG_CMD_ID;
        dbg_cmd->cmd_id_inv = (uint16_t)(~DBG_CMD_ID);
        dbg_cmd->parser = parser;
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}
/**
 *
 */
