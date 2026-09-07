/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "rsa_srv.h"
#include "mb.h"
#include "crypto_util.h"
#include "pke_srv_util.h"
#include "mmap.h"
#include "component/util.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define RSA_SRV_DATA_BUF_WORD_SIZE       (RSA_MAX_WORD_LEN)
#define RSA_SRV_MSG_DIGEST_MAX_BYTE_SIZE (HASH_DIGEST_MAX_WORD_LEN << 2)
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    // All the pointer should be u32, since the RSA key param is u32 pointer in CryLib APIs
    uint32_t n_byte_sz;
    uint32_t e_byte_sz;
    uint32_t *n;
    uint32_t *e;
    uint32_t *d;
    uint32_t *p;
    uint32_t *q;
    uint32_t *dp;
    uint32_t *dq;
    uint32_t *u;
} rsa_key_st;

typedef enum { RSA_CRT_MODE = 0U, RSA_NORMAL_MODE = 1U } rsa_crt_mode_e;
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
 *   @brief      RSA single-call encryption/decryption service
 *
 *   @param [in] rsa_cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_cipher_onepass(const mb_cmd_rsa_cipher_st *rsa_cipher_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      RSA stepwise signification/verification service
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_sign_stepwise_handler(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      RSA single-call signification/verification service
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_sign_onepass(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      RSA signification/verification init service
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_sign_init(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      RSA signification/verification update service
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_sign_update(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      RSA signification/verification final service
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_srv_sign_final(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      Get the key and crt mode for cipher or sign servcie
 *
 *   @param [in] key_handle
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] rsa_key
 *   @param [in] check_usage_bits
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_get_key_and_crt_mode(uint32_t key_handle, uint32_t *key_buf, uint32_t key_buf_sz,
    rsa_key_st *rsa_key, uint32_t check_usage_bits, rsa_crt_mode_e *crt_mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_keydata_st keydata;
    uint8_t req_kpart;
    util_memset(rsa_key, 0, sizeof(rsa_key_st));
    if ((KEY_USAGE_ENCRYPT == check_usage_bits) || (KEY_USAGE_VERIFY == check_usage_bits)) {
        req_kpart = KMS_KEY_PART_PUBKEY;
    } else if ((KEY_USAGE_DECRYPT == check_usage_bits) || (KEY_USAGE_SIGN == check_usage_bits)) {
#ifdef RSA_SEC
        // RSA secure APIs need the pubkey
        req_kpart = KMS_KEY_PART_PAIRKEY;
#else
        req_kpart = KMS_KEY_PART_PRIVKEY;
#endif
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = kms_read_key(key_handle, (uint8_t *)key_buf, key_buf_sz, &keydata, req_kpart, check_usage_bits);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((keydata.algo_id >= KMS_KEY_ALG_RSA_1024) && (keydata.algo_id <= KMS_KEY_ALG_RSA_4096_CRT)) {
            rsa_key->n_byte_sz = (uint32_t)keydata.keypair.rsa.n_size;
            rsa_key->e_byte_sz = (uint32_t)keydata.keypair.rsa.e_size;
            rsa_key->n = keydata.keypair.rsa.n;
            rsa_key->e = keydata.keypair.rsa.e;
            if (keydata.algo_id < KMS_KEY_ALG_RSA_1024_CRT) {
                rsa_key->d = keydata.keypair.rsa.p_d;
                *crt_mode = RSA_NORMAL_MODE;
            } else {
                rsa_key->q = keydata.keypair.rsa.q;
                rsa_key->p = keydata.keypair.rsa.p_d;
                rsa_key->dp = keydata.keypair.rsa.dp;
                rsa_key->dq = keydata.keypair.rsa.dq;
                rsa_key->u = keydata.keypair.rsa.u;
                *crt_mode = RSA_CRT_MODE;
            }
        } else {
            ret = EHSM_ERR_INVALID_HANDLE;
        }
    }
    return ret;
}

/**
 *   @brief      Read RSA pubkey of the plain key from host
 *
 *   @param [in] key_buf
 *   @param [in] rsa_key
 *   @param [in] rsa_plain_key
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_read_pubkey_key(uint32_t *key_buf, rsa_key_st *rsa_key, mb_rsa_key_st *rsa_plain_key)
{
    uint32_t ret;

    ret = mmap_read_remote_data(key_buf, rsa_plain_key->n, rsa_plain_key->n_byte_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsa_key->n = (uint32_t *)(void *)key_buf;
        ret = mmap_read_remote_data(&key_buf[rsa_plain_key->n_byte_sz / 4], rsa_plain_key->e, rsa_plain_key->e_byte_sz);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsa_key->e = &key_buf[rsa_plain_key->n_byte_sz / 4];
    }

    return ret;
}

/**
 *   @brief      Read RSA private key of the plain key from host
 *
 *   @param [out] key_buf
 *   @param [in] rsa_key
 *   @param [in] rsa_plain_key
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_read_privkey_key(
    uint32_t *key_buf, rsa_key_st *rsa_key, mb_rsa_key_st *rsa_plain_key, rsa_crt_mode_e crt_mode)
{
    uint32_t ret;
    uint32_t crt_key_size;

    if (RSA_CRT_MODE == crt_mode) {
        crt_key_size = rsa_plain_key->n_byte_sz >> 1;
        ret = mmap_read_remote_data(key_buf, rsa_plain_key->p, crt_key_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->p = key_buf;
            ret = mmap_read_remote_data(&key_buf[crt_key_size / 4], rsa_plain_key->q, crt_key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->q = &key_buf[crt_key_size / 4];
            ret = mmap_read_remote_data(&key_buf[crt_key_size * 2 / 4], rsa_plain_key->dp, crt_key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->dp = &key_buf[crt_key_size * 2 / 4];
            ret = mmap_read_remote_data(&key_buf[crt_key_size * 3 / 4], rsa_plain_key->dq, crt_key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->dq = &key_buf[crt_key_size * 3 / 4];
            ret = mmap_read_remote_data(&key_buf[crt_key_size * 4 / 4], rsa_plain_key->u, crt_key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->u = &key_buf[crt_key_size * 4 / 4];
        }
    } else {
        ret = mmap_read_remote_data(key_buf, rsa_plain_key->d, rsa_plain_key->n_byte_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsa_key->d = (uint32_t *)(void *)key_buf;
        }
    }

    return ret;
}

/**
 *   @brief      Read RSA plain key from host
 *
 *   @param [in] key_handle
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] rsa_key
 *   @param [in] check_usage_bits
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_read_plain_key(raddr_t plain_key_addr, uint32_t *key_buf, uint32_t key_buf_sz, rsa_key_st *rsa_key,
    uint32_t check_usage_bits, rsa_crt_mode_e *crt_mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    mb_rsa_key_st rsa_plain_key[1];

    ret = mmap_read_remote_data(rsa_plain_key, plain_key_addr, sizeof(mb_rsa_key_st));
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((rsa_plain_key->n_byte_sz == 0) || (rsa_plain_key->n_byte_sz > RSA_MAX_BYTE_LEN)
            || ((rsa_plain_key->n_byte_sz & 3U) != 0U)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            // The buffer size must be greater than the sum of the sizes of p, q, dp, dq, u, n, and e
            uint32_t need_buffer_size = (rsa_plain_key->n_byte_sz >> 1) * 5 + rsa_plain_key->n_byte_sz * 2;
            *crt_mode = rsa_plain_key->crt_mode ? RSA_CRT_MODE : RSA_NORMAL_MODE;
            if ((rsa_plain_key->e_byte_sz > RSA_MAX_BYTE_LEN) || (key_buf_sz < need_buffer_size)
                || (rsa_plain_key->e_byte_sz == 0) || ((rsa_plain_key->e_byte_sz & 3U) != 0U)) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsa_key->n_byte_sz = rsa_plain_key->n_byte_sz;
        rsa_key->e_byte_sz = rsa_plain_key->e_byte_sz;
        if ((KEY_USAGE_ENCRYPT == check_usage_bits) || (KEY_USAGE_VERIFY == check_usage_bits)) {
            ret = rsa_read_pubkey_key(key_buf, rsa_key, rsa_plain_key);
        } else if ((KEY_USAGE_DECRYPT == check_usage_bits) || (KEY_USAGE_SIGN == check_usage_bits)) {
            ret = rsa_read_pubkey_key(key_buf, rsa_key, rsa_plain_key);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = rsa_read_privkey_key(&key_buf[(rsa_plain_key->n_byte_sz + rsa_plain_key->e_byte_sz) / 4], rsa_key,
                    rsa_plain_key, *crt_mode);
            }
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

/**
 *   @brief      Get the key and crt mode for cipher servcie
 *
 *   @param [in] key_handle
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] rsa_key
 *   @param [in] check_usage_bits
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_cipher_get_key_and_crt_mode(const mb_cmd_rsa_cipher_st *rsa_cipher_cmd_data, uint32_t *key_buf,
    uint32_t key_buf_sz, rsa_key_st *rsa_key, uint32_t check_usage_bits, rsa_crt_mode_e *crt_mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (rsa_cipher_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        ret = rsa_read_plain_key(
            rsa_cipher_cmd_data->plain_key_addr, key_buf, key_buf_sz, rsa_key, check_usage_bits, crt_mode);
        break;
    case MB_KEY_HANDLE:
        ret = rsa_get_key_and_crt_mode(
            rsa_cipher_cmd_data->key_handle, key_buf, key_buf_sz, rsa_key, check_usage_bits, crt_mode);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Get the key and crt mode for sign servcie
 *
 *   @param [in] rsa_sign_cmd_data
 *   @param [in] key_buf
 *   @param [in] key_buf_sz
 *   @param [in] rsa_key
 *   @param [in] check_usage_bits
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_sign_get_key_and_crt_mode(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, uint32_t *key_buf,
    uint32_t key_buf_sz, rsa_key_st *rsa_key, uint32_t check_usage_bits, rsa_crt_mode_e *crt_mode)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (rsa_sign_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        ret = rsa_read_plain_key(
            rsa_sign_cmd_data->plain_key_addr, key_buf, key_buf_sz, rsa_key, check_usage_bits, crt_mode);
        break;
    case MB_KEY_HANDLE:
        ret = rsa_get_key_and_crt_mode(
            rsa_sign_cmd_data->key_handle, key_buf, key_buf_sz, rsa_key, check_usage_bits, crt_mode);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      RSA calcaulation service without padding
 *
 *   @param [in] rsa_key
 *   @param [in] input
 *   @param [in] input_sz
 *   @param [in] output
 *   @param [in] output_sz
 *   @param [in] dir
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_calculate(rsa_key_st *rsa_key, uint32_t *input, uint32_t input_sz, uint32_t *output,
    uint32_t *output_sz, uint8_t dir, rsa_crt_mode_e crt_mode)
{
    uint32_t ret;
    uint32_t crt_sz;
    // All the input and output should be in big-endian format in the CryLib RSA APIs
    reverse_byte_array((uint8_t *)input, (uint8_t *)input, input_sz);
    reverse_byte_array((uint8_t *)rsa_key->n, (uint8_t *)rsa_key->n, rsa_key->n_byte_sz);
    // The secure verison API do need the pubkey in Signification or decryption
    reverse_byte_array((uint8_t *)rsa_key->e, (uint8_t *)rsa_key->e, rsa_key->e_byte_sz);
    // Verification or encryption
    if (0U == dir) {
        ret = cpt_rsa_modexp(
            input, rsa_key->e, rsa_key->n, output, (rsa_key->e_byte_sz << 3), (rsa_key->n_byte_sz << 3));
    } else // Signification or decryption
    {
        if (RSA_CRT_MODE == crt_mode) {
            crt_sz = rsa_key->n_byte_sz >> 1;
            reverse_byte_array((uint8_t *)rsa_key->p, (uint8_t *)rsa_key->p, crt_sz);
            reverse_byte_array((uint8_t *)rsa_key->q, (uint8_t *)rsa_key->q, crt_sz);
            reverse_byte_array((uint8_t *)rsa_key->dp, (uint8_t *)rsa_key->dp, crt_sz);
            reverse_byte_array((uint8_t *)rsa_key->dq, (uint8_t *)rsa_key->dq, crt_sz);
            reverse_byte_array((uint8_t *)rsa_key->u, (uint8_t *)rsa_key->u, crt_sz);
            ret = cpt_rsa_crt_modexp_with_pub(input, rsa_key->p, rsa_key->q, rsa_key->dp, rsa_key->dq, rsa_key->u,
                rsa_key->e, output, (rsa_key->e_byte_sz << 3), (rsa_key->n_byte_sz << 3));
        } else {
            reverse_byte_array((uint8_t *)rsa_key->d, (uint8_t *)rsa_key->d, rsa_key->n_byte_sz);
            ret = cpt_rsa_modexp_with_pub(input, rsa_key->e, rsa_key->d, rsa_key->n, output, (rsa_key->e_byte_sz << 3),
                (rsa_key->n_byte_sz << 3));
        }
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_RSA_CALCULATE_FAILED;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        reverse_byte_array((uint8_t *)output, (uint8_t *)output, rsa_key->n_byte_sz);
        *output_sz = rsa_key->n_byte_sz;
    }
    return ret;
}

/**
 *   @brief      RSA calculation service with SSA-PSS padding
 *
 *   @param [in] msg_hash_alg
 *   @param [in] mgf_hash_alg
 *   @param [in] salt
 *   @param [in] salt_sz
 *   @param [in] digest
 *   @param [in] output
 *   @param [in] output_sz
 *   @param [in] rsa_key
 *   @param [in] dir
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_calculate_with_ssa_pss(cpt_hash_alg_e msg_hash_alg, cpt_hash_alg_e mgf_hash_alg,
    const uint8_t *salt, uint32_t salt_sz, const uint32_t *digest, uint32_t *output, uint32_t *output_sz,
    rsa_key_st *rsa_key, uint8_t dir, rsa_crt_mode_e crt_mode)
{
    uint32_t ret;
    RSA_CRT_PRIVATE_KEY d;
    d.p = (uint8_t *)rsa_key->p;
    d.q = (uint8_t *)rsa_key->q;
    d.dp = (uint8_t *)rsa_key->dp;
    d.dq = (uint8_t *)rsa_key->dq;
    d.u = (uint8_t *)rsa_key->u;
    if (MB_SIG_VRY == dir) {
        /*
         * The param salt_sz is converted into int32_t, which does not need to check if the value is valid,
         * because the CryLib will check it.
         */
        ret = cpt_rsa_ssa_pss_verify_by_msg_digest(msg_hash_alg, mgf_hash_alg, (int32_t)salt_sz,
            (const uint8_t *)digest, (uint8_t *)rsa_key->e, (rsa_key->e_byte_sz << 3), (uint8_t *)rsa_key->n,
            (rsa_key->n_byte_sz << 3), (uint8_t *)output);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_RSA_SIGNATURE_VRY_FAILED;
        }
    } else {
        if (RSA_CRT_MODE == crt_mode) {
            ret = cpt_rsa_ssa_pss_crt_sign_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_sz,
                (const uint8_t *)digest, &d, (rsa_key->n_byte_sz << 3), (uint8_t *)output);
        } else {
            ret = cpt_rsa_ssa_pss_sign_by_msg_digest(msg_hash_alg, mgf_hash_alg, salt, salt_sz, (const uint8_t *)digest,
                (uint8_t *)rsa_key->d, (uint8_t *)rsa_key->n, (rsa_key->n_byte_sz << 3), (uint8_t *)output);
        }
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_RSA_SIGNATURE_GEN_FAILED;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        *output_sz = rsa_key->n_byte_sz;
    }

    return ret;
}

/**
 *   @brief      RSA signture generation or verfication without padding
 *
 *   @param [in] rsa_key
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] signature
 *   @param [in] signature_sz
 *   @param [in] dir
 *   @param [in] crt_mode
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_sign_vry_without_padding(rsa_key_st *rsa_key, uint32_t *digest, uint32_t digest_sz,
    uint32_t *signature, uint32_t *signature_sz, uint8_t dir, rsa_crt_mode_e crt_mode)
{
    uint32_t ret;
    uint32_t local_digest[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t local_digest_sz;
    if (MB_SIG_VRY == dir) {
        // The verification will input the signture and get the local_digest as the result
        ret = rsa_calculate(rsa_key, signature, *signature_sz, local_digest, &local_digest_sz, MB_SIG_VRY, crt_mode);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            // To enhance the efficience, it will not check that if the other data in local_digest is 0
            ret = util_memcmp(digest, &local_digest[(rsa_key->n_byte_sz - digest_sz) / 4U], digest_sz);
            if (0U == ret) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_RSA_SIGNATURE_VRY_FAILED;
            }
        }
    } else {
        ret = rsa_calculate(rsa_key, digest, digest_sz, signature, signature_sz, MB_SIG_GEN, crt_mode);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_RSA_SIGNATURE_GEN_FAILED;
        }
    }
    return ret;
}

/**
 *   @brief      RSA signture generation or verfication
 *
 *   @param [in] rsa_key
 *   @param [in] digest
 *   @param [in] digest_sz
 *   @param [in] signature
 *   @param [in] signature_sz
 *   @param [in] dir
 *   @param [in] crt_mode
 *   @param [in] alg
 *   @param [in] padding
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t rsa_sign_vry(rsa_key_st *rsa_key, uint32_t *digest, uint32_t digest_sz, uint32_t *signature,
    uint32_t *signature_sz, uint8_t dir, rsa_crt_mode_e crt_mode, cpt_hash_alg_e alg, uint8_t padding, uint32_t salt_sz)
{
    uint32_t ret;
    if (MB_RSA_SIGN_RSA_PADDING_TYPE_NONE == padding) {
        ret = rsa_sign_vry_without_padding(rsa_key, digest, digest_sz, signature, signature_sz, dir, crt_mode);
    } else if (MB_RSA_SIGN_RSA_PADDING_TYPE_RSASSA_PSS == padding) {
        // The salt value will be set in CryLib API
        ret = rsa_calculate_with_ssa_pss(
            alg, alg, NULL, salt_sz, digest, signature, signature_sz, rsa_key, dir, crt_mode);
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static uint32_t rsa_srv_cipher_onepass(const mb_cmd_rsa_cipher_st *rsa_cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint32_t local_input[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t local_output[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t input_sz = rsa_cipher_cmd_data->input_size;
    uint32_t output_sz;
    uint32_t key_buf[KMS_KEY_DATA_MAX_SIZE / 4];
    rsa_key_st rsa_key;
    rsa_crt_mode_e crt_mode;
    uint32_t check_usage_bits;

    // Only response the size of output when the service successfully execute
    rsp_data->rsp_data_len = 0U;
    check_usage_bits = (MB_CIPHER_ENC == rsa_cipher_cmd_data->direction) ? KEY_USAGE_ENCRYPT : KEY_USAGE_DECRYPT;
    ret = rsa_cipher_get_key_and_crt_mode(
        rsa_cipher_cmd_data, key_buf, sizeof(key_buf), &rsa_key, check_usage_bits, &crt_mode);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // The size of input should not be greater than the length of n
        if (input_sz <= rsa_key.n_byte_sz) {
            // Clear the data buffer since the data will be change the order of endian
            util_memset(local_input, 0U, rsa_key.n_byte_sz);
            util_memset(local_output, 0U, rsa_key.n_byte_sz);
            ret = mmap_read_remote_data(local_input, rsa_cipher_cmd_data->input_addr, input_sz);
        } else {
            ret = EHSM_ERR_INPUT_OVERFLOW;
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = rsa_calculate(
                &rsa_key, local_input, input_sz, local_output, &output_sz, rsa_cipher_cmd_data->direction, crt_mode);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (output_sz <= rsa_cipher_cmd_data->output_size) {
                ret = mmap_write_remote_data(rsa_cipher_cmd_data->output_addr, local_output, output_sz);
            } else {
                ret = EHSM_ERR_OUTPUT_OVERFLOW;
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = output_sz;
                rsp_data->rsp_data_len = sizeof(output_sz);
            }
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t rsa_get_msg_digest(
    const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, uint32_t digest[RSA_SRV_DATA_BUF_WORD_SIZE], uint32_t *digest_sz)
{
    uint32_t ret;
    cpt_hash_alg_e alg = get_lib_hash_alg(rsa_sign_cmd_data->algorithm);

    if (rsa_sign_cmd_data->msg_type == MB_MSG_TYPE_MESSAGE) {
        ret = pke_srv_hash_onepass(
            alg, rsa_sign_cmd_data->msg_addr, rsa_sign_cmd_data->msg_size, (uint8_t *)digest, digest_sz);
    } else {
        if ((rsa_sign_cmd_data->msg_size <= RSA_SRV_MSG_DIGEST_MAX_BYTE_SIZE)
            && (rsa_sign_cmd_data->msg_size <= (RSA_SRV_DATA_BUF_WORD_SIZE << 2))) {
            *digest_sz = rsa_sign_cmd_data->msg_size;
            ret = mmap_read_remote_data(digest, rsa_sign_cmd_data->msg_addr, rsa_sign_cmd_data->msg_size);
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }
    return ret;
}

static uint32_t rsa_srv_sign_onepass(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint32_t key_buf[KMS_KEY_DATA_MAX_SIZE / 4];
    rsa_key_st rsa_key;
    // The length of digest should be RSA_SRV_DATA_BUF_WORD_SIZE for secure API
    uint32_t digest[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t digest_sz;
    uint32_t local_signature[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t signature_sz;
    rsa_crt_mode_e crt_mode;
    uint8_t dir = rsa_sign_cmd_data->direction;
    cpt_hash_alg_e alg = get_lib_hash_alg(rsa_sign_cmd_data->algorithm);
    uint32_t check_usage_bits;

    // Only signature generation service will response the size of output to HOST
    rsp_data->rsp_data_len = 0U;
    check_usage_bits = (MB_SIG_GEN == rsa_sign_cmd_data->direction) ? KEY_USAGE_SIGN : KEY_USAGE_VERIFY;
    ret = rsa_sign_get_key_and_crt_mode(
        rsa_sign_cmd_data, key_buf, sizeof(key_buf), &rsa_key, check_usage_bits, &crt_mode);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Clear the data buffer since the data will be change the order of endian
        util_memset(digest, 0U, sizeof(digest));
        util_memset(local_signature, 0U, rsa_key.n_byte_sz);
        if (MB_SIG_VRY == dir) {
            if (rsa_sign_cmd_data->sign_size <= sizeof(local_signature)) {
                // Get the signture form HOST
                signature_sz = rsa_sign_cmd_data->sign_size;
                ret = mmap_read_remote_data(local_signature, rsa_sign_cmd_data->sign_addr, signature_sz);
            } else {
                ret = EHSM_ERR_WRONG_SZ_OF_SIGNATURE;
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = rsa_get_msg_digest(rsa_sign_cmd_data, digest, &digest_sz);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = rsa_sign_vry(&rsa_key, digest, digest_sz, local_signature, &signature_sz, dir, crt_mode, alg,
            rsa_sign_cmd_data->rsa_padding_type, rsa_sign_cmd_data->salt_size);
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) || (EHSM_ERR_RSA_SIGNATURE_VRY_FAILED == ret)) {
        if (MB_SIG_GEN == dir) {
            if (signature_sz <= rsa_sign_cmd_data->sign_size) {
                ret = mmap_write_remote_data(rsa_sign_cmd_data->sign_addr, local_signature, signature_sz);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    rsp_data->data[0] = signature_sz;
                    rsp_data->rsp_data_len = sizeof(signature_sz);
                }
            } else {
                ret = EHSM_ERR_OUTPUT_OVERFLOW;
            }
        } else {
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0U : ret;
            rsp_data->rsp_data_len += sizeof(ret);
            /*
             * The result of verification will stored in response data, and the return code will only indicate
             * that if the process of calculation successfully work
             */
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t rsa_srv_sign_stepwise_handler(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (rsa_sign_cmd_data->process_mode) {
    case MB_START: {
        ret = rsa_srv_sign_init(rsa_sign_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = rsa_srv_sign_update(rsa_sign_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = rsa_srv_sign_init(rsa_sign_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = rsa_srv_sign_update(rsa_sign_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = rsa_srv_sign_final(rsa_sign_cmd_data, rsp_data);
        break;
    }
    default: {
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }
    }
    return ret;
}

static uint32_t rsa_srv_sign_init(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    // No need to response data in init service
    rsp_data->rsp_data_len = 0U;
    cpt_hash_alg_e alg = get_lib_hash_alg(rsa_sign_cmd_data->algorithm);
    return pke_srv_init(rsa_sign_cmd_data->sign_ctx, alg);
}

static uint32_t rsa_srv_sign_update(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    // No need to response data in update service
    rsp_data->rsp_data_len = 0U;
    return pke_srv_update(rsa_sign_cmd_data->sign_ctx, rsa_sign_cmd_data->msg_addr, rsa_sign_cmd_data->msg_size);
}

static uint32_t rsa_srv_sign_final(const mb_cmd_rsa_sign_st *rsa_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint32_t key_buf[KMS_KEY_DATA_MAX_SIZE / 4];
    rsa_key_st rsa_key;
    // The length of digest should be RSA_SRV_DATA_BUF_WORD_SIZE for secure API
    uint32_t digest[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t digest_sz;
    uint32_t local_signature[RSA_SRV_DATA_BUF_WORD_SIZE];
    uint32_t signature_sz;
    rsa_crt_mode_e crt_mode;
    uint8_t dir = rsa_sign_cmd_data->direction;
    uint32_t check_usage_bits;
    cpt_hash_alg_e alg = get_lib_hash_alg(rsa_sign_cmd_data->algorithm);

    // Only signature generation service will response the size of output to HOST
    rsp_data->rsp_data_len = 0U;
    check_usage_bits = (MB_SIG_GEN == rsa_sign_cmd_data->direction) ? KEY_USAGE_SIGN : KEY_USAGE_VERIFY;
    ret = rsa_sign_get_key_and_crt_mode(
        rsa_sign_cmd_data, key_buf, sizeof(key_buf), &rsa_key, check_usage_bits, &crt_mode);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        // Clear the data buffer since the data will be change the order of endian
        util_memset(digest, 0U, sizeof(digest));
        util_memset(local_signature, 0U, rsa_key.n_byte_sz);
        if (MB_SIG_VRY == dir) {
            if (rsa_sign_cmd_data->sign_size <= sizeof(local_signature)) {
                // Get the signture form HOST
                signature_sz = rsa_sign_cmd_data->sign_size;
                ret = mmap_read_remote_data(local_signature, rsa_sign_cmd_data->sign_addr, signature_sz);
            } else {
                ret = EHSM_ERR_WRONG_SZ_OF_SIGNATURE;
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = pke_srv_hash_final(rsa_sign_cmd_data->sign_ctx, rsa_sign_cmd_data->msg_addr, rsa_sign_cmd_data->msg_size,
            (uint8_t *)digest, &digest_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = rsa_sign_vry(&rsa_key, digest, digest_sz, local_signature, &signature_sz, dir, crt_mode, alg,
                rsa_sign_cmd_data->rsa_padding_type, rsa_sign_cmd_data->salt_size);
        }
    }
    if ((EHSM_ERR_SW_SUCCESS == ret) || (EHSM_ERR_RSA_SIGNATURE_VRY_FAILED == ret)) {
        if (MB_SIG_GEN == dir) {
            if (signature_sz <= rsa_sign_cmd_data->sign_size) {
                ret = mmap_write_remote_data(rsa_sign_cmd_data->sign_addr, local_signature, signature_sz);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    rsp_data->data[0] = signature_sz;
                    rsp_data->rsp_data_len = sizeof(signature_sz);
                }
            } else {
                ret = EHSM_ERR_OUTPUT_OVERFLOW;
            }
        } else {
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0 : ret;
            rsp_data->rsp_data_len += sizeof(ret);
            /*
             * The result of verification will stored in response data, and the return code will only indicate
             * that if the process of calculation successfully work
             */
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    util_memset(key_buf, 0U, sizeof(key_buf));
    return ret;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t pke_srv_rsa_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t pke_srv_rsa_cipher_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_rsa_cipher_st *rsa_cipher_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        rsa_cipher_cmd_data = (const mb_cmd_rsa_cipher_st *)(req_data);
        ret = cmd_check_direction(rsa_cipher_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = rsa_srv_cipher_onepass(rsa_cipher_cmd_data, rsp_data);
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

uint32_t pke_srv_rsa_sign_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_rsa_sign_st *rsa_sign_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        rsa_sign_cmd_data = (const mb_cmd_rsa_sign_st *)(req_data);
        ret = cmd_check_direction(rsa_sign_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode_and_msg_type(rsa_sign_cmd_data->process_mode, rsa_sign_cmd_data->msg_type);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (MB_ONE_PASS == rsa_sign_cmd_data->process_mode) {
                    ret = rsa_srv_sign_onepass(rsa_sign_cmd_data, rsp_data);
                } else {
                    ret = rsa_srv_sign_stepwise_handler(rsa_sign_cmd_data, rsp_data);
                }
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
