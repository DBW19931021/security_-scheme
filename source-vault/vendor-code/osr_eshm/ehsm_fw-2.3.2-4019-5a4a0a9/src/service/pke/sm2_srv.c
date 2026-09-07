/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "sm2_srv.h"
#include "pke_srv_util.h"
#include "mb.h"
#include "crypto_util.h"
#include "component/util.h"
#include "cpu_porting.h"
#include "mmap.h"
#include "kms.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
// The SM2 encryption/decryption could only deal with the data which's size not greater than SM2_SRV_DATA_BUF_BYTE_SIZE
#define SM2_SRV_DATA_BUF_BYTE_SIZE (1024U)
#define SM2_CURVE_K_BUF_SIZE       (100U)
#define SM3_BLOCK_BYTE_SIZE        (64U)
#define SM2_SIGNATURE_SIZE         (64U)
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
 *   @brief      SM2 encryption/decryption single-call service in DMA mode
 *
 *   @param [in] sm2_cipher_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_cipher_onepass(const mb_cmd_sm2_cipher_st *sm2_cipher_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      SM2 stepwise service
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_stepwise_handler(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      SM2 signature generation/verification single-call service in DMA mode
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_onepass(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      Calculating the E in init service
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_init(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      Calculating the E in update service
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_update(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data);

/**
 *   @brief      Calculating the E and generate or verify the signature in SM2 algorithm
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_final(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      SM2 signture generation service
 *
 *   @param [in] E
 *   @param [in] rand_k
 *   @param [in] priKey
 *   @param [in] signature
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign_internal(
    const uint8_t *E, const uint8_t *rand_k, const uint8_t *priKey, uint8_t *signature)
{
    uint32_t ret = cpt_sm2_sign(E, rand_k, priKey, signature);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_SM2_SIGNATURE_GEN_FAILED;
    }
    return ret;
}

/**
 *   @brief      SM2 verification service
 *
 *   @param [in] E
 *   @param [in] pubKey
 *   @param [in] signature
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_verify_internal(const uint8_t *E, const uint8_t *pubKey, const uint8_t *signature)
{
    uint32_t ret = cpt_sm2_verify(E, pubKey, signature);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_SM2_SIGNATURE_VRY_FAILED;
    }
    return ret;
}

/**
 *   @brief      SM2 encryption service
 *
 *   @param [in] M
 *   @param [in] MByteLen
 *   @param [in] rand_k
 *   @param [in] pubKey
 *   @param [in] order
 *   @param [in] C
 *   @param [in] CByteLen
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_encrypt_internal(const uint8_t *M, uint32_t MByteLen, const uint8_t *rand_k,
    const uint8_t *pubKey, cpt_sm2_cipher_order_e order, uint8_t *C, uint32_t *CByteLen)
{
    uint32_t ret = cpt_sm2_encrypt(M, MByteLen, rand_k, pubKey, order, C, CByteLen);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_SM2_CIPHER_ENC_FAILED;
    }
    return ret;
}

/**
 *   @brief      SM2 decryption service
 *
 *   @param [in] C
 *   @param [in] CByteLen
 *   @param [in] priKey
 *   @param [in] order
 *   @param [in] M
 *   @param [in] MByteLen
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_decrypt_internal(const uint8_t *C, uint32_t CByteLen, const uint8_t *priKey,
    cpt_sm2_cipher_order_e order, uint8_t *M, uint32_t *MByteLen)
{
    uint32_t ret = cpt_sm2_decrypt(C, CByteLen, priKey, order, M, MByteLen);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_SM2_CIPHER_DEC_FAILED;
    }
    return ret;
}

/**
 *   @brief      Check if this service require could be executed in DMA mode
 *
 *   @param [in] ctx_buf_addr
 *   @param [in] mode
 *   @param [in] input_msg_sz
 *   @param [in] fill_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_check_dma_mode(raddr_t ctx_buf_addr, hash_mode_e *mode, uint32_t input_msg_sz, uint32_t *fill_sz)
{
    uint32_t ret;
    uint32_t level;
    cpt_hash_rctx_st rctx[1];
    const uint8_t *map_addr;

    // The memory remap API may be used in mailbox ISR
    level = cpu_enter_critical();
    map_addr = mmap_remap_addr_u64(ctx_buf_addr);
    if (NULL != map_addr) {
        util_memcpy(rctx, map_addr, sizeof(cpt_hash_rctx_st));
        // No data cache in rctx->hash_buffer and the request data size is aligend to block size
        if ((0U == (rctx->total[0] % rctx->block_byte_len)) && (0U == (input_msg_sz % rctx->block_byte_len))) {
            *fill_sz = 0U;
            *mode = HASH_DMA_MODE;
        } else {
            // Get the size which could fill the buffer up to block size
            *fill_sz = rctx->block_byte_len - (rctx->total[0] % rctx->block_byte_len);
            *mode = HASH_CPU_MODE;
        }
        ret = EHSM_ERR_SW_SUCCESS;
    } else {
        ret = EHSM_ERR_INVALID_ADDRESS;
    }
    cpu_exit_critical(level);
    return ret;
}

/**
 *   @brief      Get the key in SM2 encryption/decryption service
 *
 *   @param [in] sm2_cipher_cmd_data
 *   @param [in] key_buf
 *   @param [in] buff_size
 *   @param [in] pubkey
 *   @param [in] prikey
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_cipher_get_key_from_key_handle(const mb_cmd_sm2_cipher_st *sm2_cipher_cmd_data, uint8_t *key_buf,
    uint32_t buff_size, uint8_t **pubkey, uint8_t **prikey)
{
    uint32_t ret;
    kms_keydata_st keydata;
    uint8_t req_kpart;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    if (MB_CIPHER_ENC == sm2_cipher_cmd_data->direction) {
        req_kpart = KMS_KEY_PART_PUBKEY;
        check_usage_bits |= KEY_USAGE_ENCRYPT;
    } else {
        req_kpart = KMS_KEY_PART_PRIVKEY;
        check_usage_bits |= KEY_USAGE_DECRYPT;
    }

    ret = kms_read_key(sm2_cipher_cmd_data->key_handle, key_buf, buff_size, &keydata, req_kpart, check_usage_bits);

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (keydata.algo_id == KMS_KEY_ALG_SM2) {
            *pubkey = keydata.keypair.ecc.pub_key;
            *prikey = keydata.keypair.ecc.priv_key;
        } else {
            ret = EHSM_ERR_INVALID_HANDLE;
        }
    }

    return ret;
}

/**
 *   @brief      Read SM2 plain key from host
 *
 *   @param [in] plain_key_addr
 *   @param [in] key_buf
 *   @param [in] buff_size
 *   @param [in] pubkey
 *   @param [in] prikey
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_read_plain_key(
    raddr_t plain_key_addr, uint8_t *key_buf, uint32_t buff_size, uint8_t **pubkey, uint8_t **prikey)
{
    uint32_t ret;
    uint32_t sm2_key_size = sizeof(mb_sm2_key_st);

    if (buff_size < sm2_key_size) {
        ret = EHSM_ERR_OUTPUT_OVERFLOW;
    } else {
        ret = mmap_read_remote_data(key_buf, plain_key_addr, sm2_key_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            *pubkey = ((mb_sm2_key_st *)key_buf)->pubkey;
            *prikey = ((mb_sm2_key_st *)key_buf)->privkey;
        }
    }

    return ret;
}

/**
 *   @brief      Get the key in SM2 encryption/decryption service
 *
 *   @param [in] sm2_cipher_cmd_data
 *   @param [in] key_buf
 *   @param [in] buff_size
 *   @param [in] pubkey
 *   @param [in] prikey
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_cipher_get_key(const mb_cmd_sm2_cipher_st *sm2_cipher_cmd_data, uint8_t *key_buf,
    uint32_t buff_size, uint8_t **pubkey, uint8_t **prikey)
{
    uint32_t ret;

    switch (sm2_cipher_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        ret = sm2_read_plain_key(sm2_cipher_cmd_data->plain_key_addr, key_buf, buff_size, pubkey, prikey);
        break;
    case MB_KEY_HANDLE:
        ret = sm2_cipher_get_key_from_key_handle(sm2_cipher_cmd_data, key_buf, buff_size, pubkey, prikey);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      Get the key in SM2 signature generation/verification service from KMS
 *
 *   @param [in] sm2_sign_cmd_data
 *   @param [in] key_buf
 *   @param [in] buff_size
 *   @param [in] pubkey
 *   @param [in] prikey
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_sign_get_key_from_key_handle(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, uint8_t *key_buf,
    uint32_t buff_size, uint8_t **pubkey, uint8_t **prikey)
{
    uint32_t ret;
    kms_keydata_st keydata;
    uint8_t req_kpart;
    uint32_t check_usage_bits = KEY_USAGE_NONE;
    uint8_t process_mode = sm2_sign_cmd_data->process_mode;

    // The pubkey will be used in init and streamstart mode for calculting the value Z
    if ((MB_START == process_mode) || (MB_STREAMSTART == process_mode)) {
        req_kpart = KMS_KEY_PART_PUBKEY;
    } else {
        if (MB_SIG_VRY == sm2_sign_cmd_data->direction) {
            req_kpart = KMS_KEY_PART_PUBKEY;
            check_usage_bits |= KEY_USAGE_VERIFY;
        } else {
            // Since the process of calculting value Z need the pubkey, so it will need the key pair in single-call
            // signification service
            req_kpart = (MB_ONE_PASS == process_mode) ? KMS_KEY_PART_PAIRKEY : KMS_KEY_PART_PRIVKEY;
            check_usage_bits |= KEY_USAGE_SIGN;
        }
    }

    ret = kms_read_key(sm2_sign_cmd_data->key_handle, key_buf, buff_size, &keydata, req_kpart, check_usage_bits);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (keydata.algo_id == KMS_KEY_ALG_SM2) {
            *pubkey = keydata.keypair.ecc.pub_key;
            *prikey = keydata.keypair.ecc.priv_key;
        } else {
            ret = EHSM_ERR_INVALID_HANDLE;
        }
    }
    return ret;
}

/**
 *   @brief      Get the key in SM2 signature generation service
 *
 *   @param [in] sm2_cipher_cmd_data
 *   @param [in] key_buf
 *   @param [in] buff_size
 *   @param [in] pubkey
 *   @param [in] prikey
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_sign_get_key(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, uint8_t *key_buf, uint32_t buff_size,
    uint8_t **pubkey, uint8_t **prikey)
{
    uint32_t ret;

    switch (sm2_sign_cmd_data->key_type) {
    case MB_PLAIN_KEY:
        ret = sm2_read_plain_key(sm2_sign_cmd_data->plain_key_addr, key_buf, buff_size, pubkey, prikey);
        break;
    case MB_KEY_HANDLE:
        ret = sm2_sign_get_key_from_key_handle(sm2_sign_cmd_data, key_buf, buff_size, pubkey, prikey);
        break;
    default:
        ret = EHSM_ERR_WRONG_KEY_TYPE;
        break;
    }

    return ret;
}

/**
 *   @brief      SM2 signature gerneration service
 *
 *   @param [in] E
 *   @param [in] prikey
 *   @param [in] rs_addr
 *   @param [in] rbuf_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_sign(
    const uint8_t *E, const uint8_t *prikey, raddr_t rs_addr, uint32_t rbuf_sz, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t signature[SM2_SIGNATURE_SIZE];

    // Only success will the size of signature response to HOST
    rsp_data->rsp_data_len = 0U;
    ret = sm2_srv_sign_internal(E, NULL, prikey, signature);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (rbuf_sz >= SM2_SIGNATURE_SIZE) {
            ret = mmap_write_remote_data(rs_addr, signature, SM2_SIGNATURE_SIZE);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                rsp_data->data[0] = SM2_SIGNATURE_SIZE;
                rsp_data->rsp_data_len = sizeof(uint32_t);
            }
        } else {
            ret = EHSM_ERR_OUTPUT_OVERFLOW;
        }
    }
    return ret;
}

/**
 *   @brief      SM2 signature vrification service
 *
 *   @param [in] E
 *   @param [in] pubkey
 *   @param [in] rs_addr
 *   @param [in] rbuf_sz
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_vry(
    const uint8_t *E, const uint8_t *pubkey, raddr_t rs_addr, uint32_t rbuf_sz, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t signature[SM2_SIGNATURE_SIZE];
    rsp_data->rsp_data_len = 0U;
    if (rbuf_sz == SM2_SIGNATURE_SIZE) {
        ret = mmap_read_remote_data(signature, rs_addr, SM2_SIGNATURE_SIZE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            // No output data
            rsp_data->data[0] = 0U;
            rsp_data->rsp_data_len += sizeof(uint32_t);
            ret = sm2_srv_verify_internal(E, pubkey, signature);
            rsp_data->data[1] = (EHSM_ERR_SW_SUCCESS == ret) ? 0U : ret;
            ;
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

static uint32_t sm2_srv_cipher_onepass(const mb_cmd_sm2_cipher_st *sm2_cipher_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t local_input[SM2_SRV_DATA_BUF_BYTE_SIZE];
    uint32_t input_sz = sm2_cipher_cmd_data->input_size;
    uint8_t local_output[SM2_SRV_DATA_BUF_BYTE_SIZE];
    uint32_t output_sz;
    uint8_t key_buf[SM2_CURVE_K_BUF_SIZE];
    uint8_t *pubkey;
    uint8_t *prikey;
    /* Max input size: plaintext limited to sizeof(local_output) - SM2_CIPHER_ENC_OVERHEAD;
     * ciphertext limited to sizeof(local_output) */
    uint32_t max_input_size = (MB_CIPHER_ENC == sm2_cipher_cmd_data->direction)
        ? (sizeof(local_output) - SM2_CIPHER_ENC_OVERHEAD)
        : sizeof(local_output);
    /* Min input size: plaintext at least 1 byte;
     * ciphertext at least SM2_CIPHER_ENC_OVERHEAD + 1 (overhead + 1 byte data) */
    uint32_t min_input_size = (MB_CIPHER_ENC == sm2_cipher_cmd_data->direction) ? 1U : (SM2_CIPHER_ENC_OVERHEAD + 1U);

    // Only when the return code is EHSM_ERR_SW_SUCCESS, the rsponse data will be given the size of output data
    rsp_data->rsp_data_len = 0U;
    ret = sm2_cipher_get_key(sm2_cipher_cmd_data, key_buf, sizeof(key_buf), &pubkey, &prikey);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((input_sz >= min_input_size) && (input_sz <= max_input_size)) {
            ret = mmap_read_remote_data(local_input, sm2_cipher_cmd_data->input_addr, input_sz);
        } else {
            ret = EHSM_ERR_SM2_DATA_BUF_OVERFLOW;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (MB_CIPHER_ENC == sm2_cipher_cmd_data->direction) {
                ret = sm2_srv_encrypt_internal(
                    local_input, input_sz, NULL, pubkey, SM2_C1C3C2, local_output, &output_sz);
            } else {
                ret = sm2_srv_decrypt_internal(local_input, input_sz, prikey, SM2_C1C3C2, local_output, &output_sz);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (output_sz <= sm2_cipher_cmd_data->output_size) {
                    ret = mmap_write_remote_data(sm2_cipher_cmd_data->output_addr, local_output, output_sz);
                } else {
                    ret = EHSM_ERR_OUTPUT_OVERFLOW;
                }
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        rsp_data->data[0] = output_sz;
        rsp_data->rsp_data_len = sizeof(output_sz);
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

/**
 *   @brief      Calculating the E in single-call service
 *
 *   @param [in] pubkey
 *   @param [in] E
 *   @param [in] input_addr
 *   @param [in] input_sz
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t sm2_srv_local_get_E(const uint8_t *pubkey, uint8_t *E, raddr_t input_addr, uint32_t input_sz)
{
    uint32_t ret;
    uint8_t local_msg[SM2_SRV_DATA_BUF_BYTE_SIZE];
    uint8_t Z[SM3_DIGEST_BYTE_LEN];
    cpt_hash_ctx_st cpu_ctx;
    cpt_hash_dma_ctx_st dma_ctx;
    uint32_t msg_h;
    uint32_t msg_l;
    uint32_t digest_h;
    uint32_t digest_l;
    raddr_t msg_addr = input_addr;
    uint32_t msg_sz = input_sz;

    ret = cpt_sm2_getZ(NULL, 0, pubkey, Z);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_init(&cpu_ctx, HASH_SM3);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_update(&cpu_ctx, Z, SM3_DIGEST_BYTE_LEN);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (msg_sz > 32U) {
            /*
             * Since the value Z has been put into the buffer of ctx, it need to add more 32 byte to calculate
             * as a block, after that the DMA mode could be used
             */
            ret = mmap_read_remote_data(local_msg, msg_addr, 32U);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_update(&cpu_ctx, local_msg, 32U);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                msg_addr += 32U;
                msg_sz -= 32U;
                cpt_hash_ctx_switch(&cpu_ctx, &dma_ctx, HASH_CPU_2_DMA);
                msg_l = (uint32_t)msg_addr;
                msg_h = (uint32_t)(msg_addr >> 32);
                digest_h = 0U;
                // Set the system general register, whihc could be accessed by AHB-DMA, as the result address
                digest_l = SYS_GEN_REG;
                // Set the DMA mode to AXI-read and AHB-write
                ret = cpt_hash_dma_final(&dma_ctx, msg_h, msg_l, msg_sz, digest_h, digest_l, EHSM_WRITE_SOC_READ);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                util_memcpy(E, (uint8_t *)digest_l, SM3_DIGEST_BYTE_LEN);
            }
        } else {
            // The message size not enough for a block
            ret = mmap_read_remote_data(local_msg, msg_addr, msg_sz);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_update(&cpu_ctx, local_msg, msg_sz);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_final(&cpu_ctx, E);
            }
        }
    }
    return ret;
}

static uint32_t sm2_srv_sign_onepass(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t key_buf[SM2_CURVE_K_BUF_SIZE];
    uint8_t *pubkey;
    uint8_t *prikey;
    uint8_t E[SM3_DIGEST_BYTE_LEN];

    ret = sm2_sign_get_key(sm2_sign_cmd_data, key_buf, sizeof(key_buf), &pubkey, &prikey);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (sm2_sign_cmd_data->msg_type == MB_MSG_TYPE_MESSAGE) {
            ret = sm2_srv_local_get_E(pubkey, E, sm2_sign_cmd_data->msg_addr, sm2_sign_cmd_data->msg_size);
        } else {
            if (sm2_sign_cmd_data->msg_size == sizeof(E)) {
                ret = mmap_read_remote_data(E, sm2_sign_cmd_data->msg_addr, sm2_sign_cmd_data->msg_size);
            } else {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_SIG_GEN == sm2_sign_cmd_data->direction) {
            ret = sm2_srv_sign(E, prikey, sm2_sign_cmd_data->sign_addr, sm2_sign_cmd_data->sign_size, rsp_data);
        } else {
            ret = sm2_srv_vry(E, pubkey, sm2_sign_cmd_data->sign_addr, sm2_sign_cmd_data->sign_size, rsp_data);
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t sm2_srv_sign_init(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t *pubkey;
    uint8_t *prikey;
    uint8_t key_buf[SM2_CURVE_K_BUF_SIZE];
    uint8_t Z[SM3_DIGEST_BYTE_LEN];
    cpt_hash_ctx_st hash_ctx;

    ret = sm2_sign_get_key(sm2_sign_cmd_data, key_buf, sizeof(key_buf), &pubkey, &prikey);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_sm2_getZ(NULL, 0, pubkey, Z);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_init(&hash_ctx, HASH_SM3);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_update(&hash_ctx, Z, SM3_DIGEST_BYTE_LEN);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (sm2_sign_cmd_data->sign_ctx_size >= sizeof(cpt_hash_rctx_st)) {
            ret = cpt_hash_rctx_switch_cpu_ctx(sm2_sign_cmd_data->sign_ctx, &hash_ctx, HASH_LCTX_2_RCTX);
        } else {
            ret = EHSM_ERR_CTX_OVERFLOW;
        }
    }
    // No need to response data in init stage
    rsp_data->rsp_data_len = 0U;
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t sm2_srv_sign_update(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    hash_mode_e mode;
    raddr_t rctx_addr = sm2_sign_cmd_data->sign_ctx;
    raddr_t remain_msg_addr = sm2_sign_cmd_data->msg_addr;
    uint32_t remain_sz = sm2_sign_cmd_data->msg_size;
    uint32_t once_sz;
    uint32_t fill_sz;
    ret = sm2_check_dma_mode(rctx_addr, &mode, remain_sz, &fill_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        do {
            if (HASH_CPU_MODE == mode) {
                if (remain_sz < fill_sz) {
                    once_sz = remain_sz;
                } else {
                    once_sz = fill_sz;
                }
                ret = pke_srv_cpu_update(rctx_addr, remain_msg_addr, once_sz);
                /*
                 * Once pke_cpu_update() has been executed, it will calculate the remian data in ctx buffer.
                 * So that the fill_sz will be used as the size as local_msg[] in pke_cpu_update();
                 */
                fill_sz = HASH_SRV_DATA_BUF_BYTE_SIZE;
                /*
                 * If remain_sz < fill_sz, the loop will be ended, on the other hand the hash could be used in
                 * DMA mode since all the stored data in the buffer has been used. So that the value mode may
                 * may only valid when remain_sz >= fill_sz
                 */
                mode = HASH_DMA_MODE;
            } else {
                once_sz = (remain_sz / SM3_BLOCK_BYTE_SIZE) * SM3_BLOCK_BYTE_SIZE;
                if (0U != once_sz) {
                    ret = pke_srv_dma_update(rctx_addr, remain_msg_addr, once_sz);
                } else {
                    // Switch to CPU mode for next loop
                    mode = HASH_CPU_MODE;
                }
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                remain_sz -= once_sz;
                remain_msg_addr += (uint64_t)once_sz;
            } else {
                remain_sz = 0U;
            }
        } while (remain_sz > 0U);
    }
    // No need to response data in update stage
    rsp_data->rsp_data_len = 0U;
    return ret;
}

static uint32_t sm2_srv_sign_final(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    uint8_t digest[SM3_DIGEST_BYTE_LEN];
    uint32_t digest_sz;
    uint8_t key_buf[SM2_CURVE_K_BUF_SIZE];
    uint8_t *pubkey;
    uint8_t *prikey;

    // The ctx may consider to keep in EHSM, so that hash final API should call first to release the buffer even if the
    // key relative API could not susccessfully execute
    ret = pke_srv_hash_final(
        sm2_sign_cmd_data->sign_ctx, sm2_sign_cmd_data->msg_addr, sm2_sign_cmd_data->msg_size, digest, &digest_sz);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = sm2_sign_get_key(sm2_sign_cmd_data, key_buf, sizeof(key_buf), &pubkey, &prikey);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_SIG_GEN == sm2_sign_cmd_data->direction) {
            ret = sm2_srv_sign(digest, prikey, sm2_sign_cmd_data->sign_addr, sm2_sign_cmd_data->sign_size, rsp_data);
        } else {
            ret = sm2_srv_vry(digest, pubkey, sm2_sign_cmd_data->sign_addr, sm2_sign_cmd_data->sign_size, rsp_data);
        }
    }
    util_memset(key_buf, 0x00, sizeof(key_buf));
    return ret;
}

static uint32_t sm2_srv_sign_stepwise_handler(const mb_cmd_sm2_sign_st *sm2_sign_cmd_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    switch (sm2_sign_cmd_data->process_mode) {
    case MB_START: {
        ret = sm2_srv_sign_init(sm2_sign_cmd_data, rsp_data);
        break;
    }
    case MB_UPDATE: {
        ret = sm2_srv_sign_update(sm2_sign_cmd_data, rsp_data);
        break;
    }
    case MB_STREAMSTART: {
        ret = sm2_srv_sign_init(sm2_sign_cmd_data, rsp_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = sm2_srv_sign_update(sm2_sign_cmd_data, rsp_data);
        }
        break;
    }
    case MB_FINISH: {
        ret = sm2_srv_sign_final(sm2_sign_cmd_data, rsp_data);
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
uint32_t pke_srv_sm2_init(void)
{
    return EHSM_ERR_SW_SUCCESS;
}

uint32_t pke_srv_sm2_cipher_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_sm2_cipher_st *sm2_cipher_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        sm2_cipher_cmd_data = (const mb_cmd_sm2_cipher_st *)(req_data);
        ret = cmd_check_direction(sm2_cipher_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = sm2_srv_cipher_onepass(sm2_cipher_cmd_data, rsp_data);
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

uint32_t pke_srv_sm2_sign_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;
    const mb_cmd_sm2_sign_st *sm2_sign_cmd_data;
    if ((NULL != req_data) && (NULL != rsp_data)) {
        sm2_sign_cmd_data = (const mb_cmd_sm2_sign_st *)(req_data);
        ret = cmd_check_direction(sm2_sign_cmd_data->direction);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cmd_check_process_mode_and_msg_type(sm2_sign_cmd_data->process_mode, sm2_sign_cmd_data->msg_type);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (MB_ONE_PASS == sm2_sign_cmd_data->process_mode) {
                    ret = sm2_srv_sign_onepass(sm2_sign_cmd_data, rsp_data);
                } else {
                    ret = sm2_srv_sign_stepwise_handler(sm2_sign_cmd_data, rsp_data);
                }
            }
        }
    } else {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}
