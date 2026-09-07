/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "ecdsa_srv.h"
#include "mb.h"
#include "crypto_util.h"
#include "pke_srv_util.h"
#include "crypto_common/eccp_curve.h"
#include "mmap.h"
#include "kms.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define MAX_ECCP_BYTE_LEN       ((ECCP_MAX_BIT_LEN + 7U) / 8U)
#define MAX_ECCP_SIGNATURE_SIZE (MAX_ECCP_BYTE_LEN * 2U)
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

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/**
 *   @brief      ECDSA single-call service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_onepass(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      ECDSA stepwise service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_stepwise_handler(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      ECDSA init service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_init(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      ECDSA update service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_update(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      ECDSA final service, this function will calculate the digest and then generate
 *               or verify the signature
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_final(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

/**
 *   @brief      ECDSA signture generation service
 *
 *   @param [in] curve
 *   @param [in] E
 *   @param [in] EByteLen
 *   @param [in] rand_k
 *   @param [in] priKey
 *   @param [in] signature
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_sign_internal(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen,
    const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature)
{
    uint32_t ret = cpt_ecdsa_sign(curve, E, EByteLen, rand_k, priKey, signature);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_ECDSA_SIGNATURE_GEN_FAILED;
    }
    return ret;
}

/**
 *   @brief      ECDSA verification service
 *
 *   @param [in] curve
 *   @param [in] E
 *   @param [in] EByteLen
 *   @param [in] pubKey
 *   @param [in] signature
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_verify_internal(const cpt_eccp_curve_st *curve, const uint8_t *E, uint32_t EByteLen,
    const uint8_t *pubKey, const uint8_t *signature)
{
    uint32_t ret = cpt_ecdsa_verify(curve, E, EByteLen, pubKey, signature);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_ECDSA_SIGNATURE_VRY_FAILED;
    }
    return ret;
}

/**
 *   @brief      ED25519 signture generation service
 *
 *   @param [in] mode
 *   @param [in] prikey
 *   @param [in] pubkey
 *   @param [in] ctx
 *   @param [in] ctxByteLen
 *   @param [in] M
 *   @param [in] MByteLen
 *   @param [in] RS
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ed25519_srv_sign_internal(cpt_ed25519_mode_e mode, const uint8_t *prikey, const uint8_t *pubkey,
    const uint8_t *ctx, uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, uint8_t *RS)
{
    return cpt_ed25519_sign(mode, prikey, pubkey, ctx, ctxByteLen, M, MByteLen, RS);
}

/**
 *   @brief      ED25519 verification service
 *
 *   @param [in] mode
 *   @param [in] pubkey
 *   @param [in] ctx
 *   @param [in] ctxByteLen
 *   @param [in] M
 *   @param [in] MByteLen
 *   @param [in] RS
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ed25519_srv_verify_internal(cpt_ed25519_mode_e mode, const uint8_t *pubkey, const uint8_t *ctx,
    uint8_t ctxByteLen, const uint8_t *M, uint32_t MByteLen, const uint8_t *RS)
{
    return cpt_ed25519_verify(mode, pubkey, ctx, ctxByteLen, M, MByteLen, RS);
}

/**
 *   @brief      Get the key for ECDSA and ed25519 service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] curve
 *   @param [in] pubkey
 *   @param [in] prikey
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] ed25519_sel
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecc_get_key_from_handle(const mb_cmd_ecdsa_st *ecdsa_cmd_data, const cpt_eccp_curve_st **curve,
    uint8_t **pubkey, uint8_t **prikey, uint8_t *key_buf, uint32_t key_buf_sz, bool_t *ed25519_sel)
{
    uint32_t ret;
    kms_keydata_st keydata;
    uint8_t req_kpart;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    if (MB_SIG_GEN == ecdsa_cmd_data->direction) {
        req_kpart = KMS_KEY_PART_PRIVKEY;
        check_usage_bits |= KEY_USAGE_SIGN;
    } else {
        req_kpart = KMS_KEY_PART_PUBKEY;
        check_usage_bits |= KEY_USAGE_VERIFY;
    }

    ret = kms_read_key(ecdsa_cmd_data->key_handle, key_buf, key_buf_sz, &keydata, req_kpart, check_usage_bits);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (KMS_KEY_ALG_ED25519 == keydata.algo_id) {
            if (MB_ECDSA_ALGORITHM_SHA512 == ecdsa_cmd_data->algorithm) {
                *ed25519_sel = true;
            } else {
                ret = EHSM_ERR_NOT_SUPPORT;
            }
        } else {
            *ed25519_sel = false;
            *curve = get_curve_param_type(keydata.algo_id);
            if (NULL == *curve) {
                ret = EHSM_ERR_INVALID_HANDLE;
            }
        }
        *pubkey = keydata.keypair.ecc.pub_key;
        *prikey = keydata.keypair.ecc.priv_key;
    }
    return ret;
}

/**
 *   @brief      Read the plain key for ECDSA and ed25519 service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] curve
 *   @param [in] pubkey
 *   @param [in] prikey
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] ed25519_sel
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecc_read_plain_key(const mb_cmd_ecdsa_st *ecdsa_cmd_data, const cpt_eccp_curve_st **curve,
    uint8_t **pubkey, uint8_t **prikey, uint8_t *key_buf, uint32_t key_buf_sz, bool_t *ed25519_sel)
{
    uint32_t ret;
    mb_ecc_key_st ecc_key;
    uint32_t privkey_size = 0;

    ret = mmap_read_remote_data(&ecc_key, ecdsa_cmd_data->plain_key_addr, sizeof(mb_ecc_key_st));
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (KMS_KEY_ALG_ED25519 == ecc_key.curve_id) {
            if (MB_ECDSA_ALGORITHM_SHA512 == ecdsa_cmd_data->algorithm) {
                *ed25519_sel = true;
            } else {
                ret = EHSM_ERR_NOT_SUPPORT;
            }
        } else {
            *ed25519_sel = false;
            *curve = get_curve_param_type(ecc_key.curve_id);
            if (NULL == *curve) {
                ret = EHSM_ERR_INVALID_HANDLE;
            } else if (key_buf_sz < (GET_BYTE_LEN((*curve)->eccp_p_bitLen) * 3)) {
                ret = EHSM_ERR_OUTPUT_OVERFLOW;
            } else {
                // nothing todo
            }
        }
        if ((EHSM_ERR_SW_SUCCESS == ret) && (ecc_key.privkey != 0U)) {
            if (*ed25519_sel) {
                privkey_size = 32;
            } else {
                privkey_size = GET_BYTE_LEN((*curve)->eccp_p_bitLen);
            }
            ret = mmap_read_remote_data(key_buf, ecc_key.privkey, privkey_size);
            *prikey = key_buf;
        }
        if ((EHSM_ERR_SW_SUCCESS == ret) && (ecc_key.pubkey != 0U)) {
            uint32_t pubkey_size;
            if (*ed25519_sel) {
                pubkey_size = 32; // ED25519 public key is 32 bytes
            } else {
                pubkey_size = GET_BYTE_LEN((*curve)->eccp_p_bitLen) * 2;
            }
            ret = mmap_read_remote_data(&key_buf[privkey_size], ecc_key.pubkey, pubkey_size);
            *pubkey = &key_buf[privkey_size];
        }
    }

    return ret;
}
/**
 *   @brief      Get the key for ECDSA and ed25519 service
 *
 *   @param [in] ecdsa_cmd_data
 *   @param [in] curve
 *   @param [in] pubkey
 *   @param [in] prikey
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] ed25519_sel
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecc_get_key(const mb_cmd_ecdsa_st *ecdsa_cmd_data, const cpt_eccp_curve_st **curve, uint8_t **pubkey,
    uint8_t **prikey, uint8_t *key_buf, uint32_t key_buf_sz, bool_t *ed25519_sel)
{
    uint32_t ret;

    switch (ecdsa_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        ret = ecc_read_plain_key(ecdsa_cmd_data, curve, pubkey, prikey, key_buf, key_buf_sz, ed25519_sel);
        break;
    case MB_KEY_HANDLE:
        ret = ecc_get_key_from_handle(ecdsa_cmd_data, curve, pubkey, prikey, key_buf, key_buf_sz, ed25519_sel);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Signature-generating function with some ecc algorithm(NOT include ED25519)
 *
 *   @param [in] curve
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] prikey
 *   @param [in] rs_addr
 *   @param [in] rs_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_sign(const cpt_eccp_curve_st *curve, const uint8_t *digest, uint32_t digest_sz,
    const uint8_t *prikey, raddr_t rs_addr, uint32_t rs_sz, cmd_rsp_data_st *rsp_data)
{
    uint8_t local_signature[MAX_ECCP_SIGNATURE_SIZE];
    uint32_t signature_sz = ((curve->eccp_n_bitLen + 7U) / 8U) * 2U;
    uint32_t ret = ecdsa_srv_sign_internal(curve, digest, digest_sz, NULL, prikey, local_signature);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (rs_sz >= signature_sz) {
            ret = mmap_write_remote_data(rs_addr, local_signature, signature_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = signature_sz;
                rsp_data->rsp_data_len = sizeof(signature_sz);
            }
        } else {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    }
    return ret;
}

/**
 *   @brief      Signature-verifing function with some ecc algorithm(NOT include ED25519)
 *
 *   @param [in] curve
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] pubkey
 *   @param [in] rs_addr
 *   @param [in] rs_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_vry(const cpt_eccp_curve_st *curve, const uint8_t *digest, uint32_t digest_sz,
    const uint8_t *pubkey, raddr_t rs_addr, uint32_t rs_sz, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t local_signature[MAX_ECCP_SIGNATURE_SIZE];
    uint32_t signature_sz = ((curve->eccp_n_bitLen + 7U) / 8U) * 2U;
    rsp_data->rsp_data_len = 0U;
    if (signature_sz == rs_sz) {
        ret = mmap_read_remote_data(local_signature, rs_addr, signature_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            ret = ecdsa_srv_verify_internal(curve, digest, digest_sz, pubkey, local_signature);
            rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0U : ret;
            rsp_data->rsp_data_len += sizeof(ret);
            /*
             * The result of verification will stored in response data, and the return code will only indicate
             * that if the process of calculation successfully work
             */
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_WRONG_SZ_OF_SIGNATURE;
    }

    return ret;
}

/**
 *   @brief      Signature genration/verification function with some ecc algorithm(NOT include ED25519)
 *
 *   @param [in] curve
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] pubkey
 *   @param [in] prikey
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ecdsa_srv_sign_vry(const cpt_eccp_curve_st *curve, const uint8_t *digest, uint32_t digest_sz,
    const uint8_t *pubkey, const uint8_t *prikey, const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    if (MB_SIG_GEN == ecdsa_cmd_data->direction) {
        ret = ecdsa_srv_sign(
            curve, digest, digest_sz, prikey, ecdsa_cmd_data->sign_addr, ecdsa_cmd_data->sign_size, rsp_data);
    } else {
        ret = ecdsa_srv_vry(
            curve, digest, digest_sz, pubkey, ecdsa_cmd_data->sign_addr, ecdsa_cmd_data->sign_size, rsp_data);
    }
    return ret;
}

/**
 *   @brief      Signature-generating function with ED25519 algorithm
 *
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] prikey
 *   @param [in] pubkey
 *   @param [in] rs_addr
 *   @param [in] rs_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ed25519_srv_sign(const uint8_t *digest, uint32_t digest_sz, const uint8_t *prikey,
    const uint8_t *pubkey, raddr_t rs_addr, uint32_t rs_sz, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t local_signature[64];
    uint32_t signature_sz = 64U;
    if (rs_sz >= signature_sz) {
        // TODO: Check if the pubkey is necessary
        ret = ed25519_srv_sign_internal(
            Ed25519_PH_WITH_PH_M, prikey, pubkey, NULL, 0, digest, digest_sz, local_signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mmap_write_remote_data(rs_addr, local_signature, signature_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = signature_sz;
                rsp_data->rsp_data_len = sizeof(signature_sz);
            }
        }
    } else {
        ret = EHSM_ERR_OUTPUT_OVERFLOW;
    }
    return ret;
}

/**
 *   @brief      Signature-verifing function with ED25519 algorithm
 *
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] pubkey
 *   @param [in] rs_addr
 *   @param [in] rs_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ed25519_srv_vry(const uint8_t *digest, uint32_t digest_sz, const uint8_t *pubkey, raddr_t rs_addr,
    uint32_t rs_sz, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t local_signature[64];
    uint32_t signature_sz = 64U;
    rsp_data->rsp_data_len = 0U;
    if (signature_sz == rs_sz) {
        ret = mmap_read_remote_data(local_signature, rs_addr, signature_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            // No output data
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            ret = ed25519_srv_verify_internal(
                Ed25519_PH_WITH_PH_M, pubkey, NULL, 0, digest, digest_sz, local_signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[1] = 0U;
            } else {
                rsp_data->data[1] = EHSM_ERR_ECDSA_SIGNATURE_VRY_FAILED;
            }
            rsp_data->rsp_data_len += sizeof(uint32_t);
            /*
             * The result of verification will stored in response data, and the return code will only indicate
             * that if the process of calculation successfully work
             */
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_WRONG_SZ_OF_SIGNATURE;
    }
    return ret;
}

/**
 *   @brief      Signature genration/verification function with ED25519 algorithm
 *
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] pubkey
 *   @param [in] prikey
 *   @param [in] ecdsa_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t ed25519_srv_sign_vry(const uint8_t *digest, uint32_t digest_sz, const uint8_t *pubkey,
    const uint8_t *prikey, const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    if (MB_SIG_GEN == ecdsa_cmd_data->direction) {
        ret = ed25519_srv_sign(
            digest, digest_sz, prikey, pubkey, ecdsa_cmd_data->sign_addr, ecdsa_cmd_data->sign_size, rsp_data);
    } else {
        ret = ed25519_srv_vry(
            digest, digest_sz, pubkey, ecdsa_cmd_data->sign_addr, ecdsa_cmd_data->sign_size, rsp_data);
    }
    return ret;
}

static uint32_t ecdsa_srv_onepass(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[KMS_ECC_KEYPAIR_MAX_SIZE];
    uint8_t *prikey = NULL;
    uint8_t *pubkey = NULL;
    const cpt_eccp_curve_st *curve = NULL;
    bool_t ed25519_sel;
    uint8_t digest[64];
    uint32_t digest_sz;
    cpt_hash_alg_e alg = get_lib_hash_alg(ecdsa_cmd_data->algorithm);

    ret = ecc_get_key(ecdsa_cmd_data, &curve, &pubkey, &prikey, key_buf, sizeof(key_buf), &ed25519_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (ecdsa_cmd_data->msg_type == MB_MSG_TYPE_MESSAGE) {
            ret = pke_srv_hash_onepass(alg, ecdsa_cmd_data->msg_addr, ecdsa_cmd_data->msg_size, digest, &digest_sz);
        } else {
            if (ecdsa_cmd_data->msg_size <= sizeof(digest)) {
                digest_sz = ecdsa_cmd_data->msg_size;
                ret = mmap_read_remote_data(digest, ecdsa_cmd_data->msg_addr, ecdsa_cmd_data->msg_size);
            } else {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (true == ed25519_sel) {
            ret = ed25519_srv_sign_vry(digest, digest_sz, pubkey, prikey, ecdsa_cmd_data, rsp_data);
        } else {
            ret = ecdsa_srv_sign_vry(curve, digest, digest_sz, pubkey, prikey, ecdsa_cmd_data, rsp_data);
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t ecdsa_srv_init(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    rsp_data->rsp_data_len = 0U;
    cpt_hash_alg_e alg = get_lib_hash_alg(ecdsa_cmd_data->algorithm);
    return pke_srv_init(ecdsa_cmd_data->sign_ctx, alg);
}

static uint32_t ecdsa_srv_update(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    rsp_data->rsp_data_len = 0U;
    return pke_srv_update(ecdsa_cmd_data->sign_ctx, ecdsa_cmd_data->msg_addr, ecdsa_cmd_data->msg_size);
}

static uint32_t ecdsa_srv_final(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[KMS_ECC_KEYPAIR_MAX_SIZE];
    uint8_t *prikey = NULL;
    uint8_t *pubkey = NULL;
    const cpt_eccp_curve_st *curve = NULL;
    uint8_t digest[64];
    uint32_t digest_sz;
    bool_t ed25519_sel;

    ret = ecc_get_key(ecdsa_cmd_data, &curve, &pubkey, &prikey, key_buf, sizeof(key_buf), &ed25519_sel);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = pke_srv_hash_final(
            ecdsa_cmd_data->sign_ctx, ecdsa_cmd_data->msg_addr, ecdsa_cmd_data->msg_size, digest, &digest_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (true == ed25519_sel) {
                ret = ed25519_srv_sign_vry(digest, digest_sz, pubkey, prikey, ecdsa_cmd_data, rsp_data);
            } else {
                ret = ecdsa_srv_sign_vry(curve, digest, digest_sz, pubkey, prikey, ecdsa_cmd_data, rsp_data);
            }
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t ecdsa_srv_stepwise_handler(const mb_cmd_ecdsa_st *ecdsa_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (ecdsa_cmd_data->process_mode) {
    case MB_START: {
        ret = ecdsa_srv_init(ecdsa_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = ecdsa_srv_update(ecdsa_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = ecdsa_srv_init(ecdsa_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = ecdsa_srv_update(ecdsa_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = ecdsa_srv_final(ecdsa_cmd_data, rsp_data);
        break;
    }
    default: {
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t pke_srv_ecdsa_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t pke_srv_ecdsa_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_ecdsa_st *ecdsa_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        ecdsa_cmd_data = (const mb_cmd_ecdsa_st *)(req_data);
        ret = cmd_check_direction(ecdsa_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode_and_msg_type(ecdsa_cmd_data->process_mode, ecdsa_cmd_data->msg_type);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (MB_ONE_PASS == ecdsa_cmd_data->process_mode) {
                    ret = ecdsa_srv_onepass(ecdsa_cmd_data, rsp_data);
                } else {
                    ret = ecdsa_srv_stepwise_handler(ecdsa_cmd_data, rsp_data);
                }
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
