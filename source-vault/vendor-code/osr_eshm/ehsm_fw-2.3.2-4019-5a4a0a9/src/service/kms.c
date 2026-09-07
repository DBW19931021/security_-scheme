/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "kms.h"
#include "types.h"
#include "mb.h"
#include "mmap.h"
#include "cpu_porting.h"
#include "she/she_key.h"
#include "component/kds.h"
#include "component/util.h"
#include "component/otp_key.h"
#include "crypto_util.h"
#include "config.h"
#include "../driver/sysreg.h"
#include "../driver/kmu_driver.h"
#include <crypto_common/utility.h>
#include "service/pke/sm9_srv.h"
#include "driver/watchdog_driver.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*Definition the encryption/signature block max/min size*/
#define KMS_KEY_ENCRYPTION_BLOCK_MAX_SIZE (0x20U)
#define KMS_KEY_SIGNATURE_BLOCK_MIN_SIZE  (0x08U)
/* key derive function type */
#define KMS_KEY_DERIVE_FUN_KDFX963 (0x01U)
#define KMS_KEY_DERIVE_FUN_PBKDF2  (0x02U)
/*Definition the SM2 key exchange role type*/
#define KMS_SM2_ROLE_SPONSOR   (0x0U)
#define KMS_SM2_ROLE_RESPONSOR (0x1U)
/*Definition the key derived root password come from type*/
#define KMS_KEY_DERIVE_USER_PASSWD    (0x1U)
#define KMS_KEY_DERIVE_USER_KEYHANDLE (0x2U)
/*Definition the derived key output dest*/
#define KMS_KEY_DERIVE_OUTPUT_TO_KMS (0x1U)
/*Definition the DH command data max size.
its data format is {p_len + p_value + q_len + q_value + g_len + g_value + h_len + h_value}.
the value of a x_len represents the length of the value of x_value. h_value is optional,
setting the value of h_len to zero indicates that h_value does not exist. and the length of x_len is 4 bytes.*/
#define KMS_DH_COMMON_DATA_MAX_SIZE ((512U * 4U) + 16U)

#define KMS_SM9_EXCHG_SA_SB_SIZE (32U)
#define KMS_SM9_EXCHG_S1_S2_SIZE (32U)

#define KMS_SM9_EXCHG_KEY_MAX_SIZE (512U)

#define RSA_KEY_E_SIZE_LIMITE (8U)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
/*Definition the key algorithm type*/
typedef enum {
    KMS_ALG_TYPE_NONE = 0,
    KMS_ALG_TYPE_SYM,
    KMS_ALG_TYPE_ECC,
    KMS_ALG_TYPE_DH,
    KMS_ALG_TYPE_RSA_CRT,
    KMS_ALG_TYPE_RSA_COMM,
    KMS_ALG_TYPE_SM9_USER_PRIV,
    KMS_ALG_TYPE_END,
} kms_alg_type_e;
/*Definition the key element size*/
typedef struct {
    uint16_t pub_key_mem_size;
    uint16_t priv_key_mem_size;
    uint16_t ext_key_mem_size;
} kms_key_mem_size_st;
/*Definition the key algorithm id, private/public size and type sturction*/
typedef struct {
    uint8_t algo_id;
    uint16_t privkey_size;
    uint16_t pubkey_size;
    kms_alg_type_e type;
} algo_keypair_size;
/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static const algo_keypair_size g_key_size_arry[] = {
    { KMS_KEY_ALG_RANDOM, 512, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_DES, 8, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_TDES_128, 16, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_TDES_192, 24, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_128, 16, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_192, 24, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_256, 32, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_SM4, 16, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_SM2, 32, 65, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_RSA_1024, 128, 128, KMS_ALG_TYPE_RSA_COMM },
    { KMS_KEY_ALG_RSA_2048, 256, 256, KMS_ALG_TYPE_RSA_COMM },
    { KMS_KEY_ALG_RSA_3072, 384, 384, KMS_ALG_TYPE_RSA_COMM },
    { KMS_KEY_ALG_RSA_4096, 512, 512, KMS_ALG_TYPE_RSA_COMM },
    { KMS_KEY_ALG_RSA_1024_CRT, 320, 128, KMS_ALG_TYPE_RSA_CRT },
    { KMS_KEY_ALG_RSA_2048_CRT, 640, 256, KMS_ALG_TYPE_RSA_CRT },
    { KMS_KEY_ALG_RSA_3072_CRT, 960, 384, KMS_ALG_TYPE_RSA_CRT },
    { KMS_KEY_ALG_RSA_4096_CRT, 1280, 512, KMS_ALG_TYPE_RSA_CRT },
    { KMS_KEY_ALG_RSA_DH, 512, 512, KMS_ALG_TYPE_DH },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_160R1, 20, 40, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_192R1, 24, 48, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_224R1, 28, 56, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_256R1, 32, 64, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_320R1, 40, 80, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_384R1, 48, 96, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_BRAINPOOLP_512R1, 64, 128, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_192R1, 24, 48, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_224R1, 28, 56, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_256R1, 32, 64, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_384R1, 48, 96, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_521R1, 66, 132, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_X25519, 32, 32, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ED25519, 32, 32, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_SM4_XTS, 32, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_128_XTS, 32, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_192_XTS, 48, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_AES_256_XTS, 64, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_CHACHA, 32, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_HMAC, 512, 0, KMS_ALG_TYPE_SYM },
    { KMS_KEY_ALG_ECC_SECP_160K1, 21, 40, KMS_ALG_TYPE_ECC }, // private key length is 21
    { KMS_KEY_ALG_ECC_SECP_192K1, 24, 48, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_224K1, 29, 56, KMS_ALG_TYPE_ECC }, // private key length is 29
    { KMS_KEY_ALG_ECC_SECP_256K1, 32, 64, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_SM9_ENC_USERPRIV, 128, 0, KMS_ALG_TYPE_SM9_USER_PRIV },
    { KMS_KEY_ALG_SM9_SIGN_USERPRIV, 64, 0, KMS_ALG_TYPE_SM9_USER_PRIV },
    { KMS_KEY_ALG_SM9_EXCH_USERPRIV, 128, 0, KMS_ALG_TYPE_SM9_USER_PRIV },
    { KMS_KEY_ALG_SM9_EXCH_TMPKEY, 32, 64, KMS_ALG_TYPE_ECC },
    { KMS_KEY_ALG_ECC_SECP_160R1, 21, 40, KMS_ALG_TYPE_ECC }, // private key length is 21
    { KMS_KEY_ALG_ECC_SECP_160R2, 21, 40, KMS_ALG_TYPE_ECC }, // private key length is 21
    { KMS_KEY_ALG_END, 0, 0, KMS_ALG_TYPE_END },
};
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t kms_get_ske_alg(uint8_t algo_id, cpt_ske_alg_e *ske_alg);
static uint32_t kms_calcu_key_mem_size(const kms_key_format_st *khead, kms_key_mem_size_st *kmem_size);
static uint32_t kms_dh_generate_key(
    const cpt_dh_para_st *dh_para, uint8_t *prikey, uint8_t *pubkey, uint32_t prikey_size);
static uint32_t kms_alloc_key_mem(
    uint8_t *data, const kms_key_format_st *khead, uint8_t req_kpart, kms_keypair_st *keypair);
static uint32_t kms_parse_dh_common_data(raddr_t dh_common_addr, uint32_t dh_common_size, uint32_t *dh_common_buff,
    uint32_t dh_common_buff_size, cpt_dh_para_st *dh_para);
static uint32_t kms_expend_dh_keypair(uint8_t *priv_key_data, uint32_t priv_key_size, uint8_t *pub_key_data,
    uint32_t pub_key_size, uint32_t dh_p_len, uint32_t dh_q_len);
static uint32_t kms_gen_exchange_dh_key(const mb_cmd_ehsm_key_exchange_st *cmd_data, const kms_keydata_st *local_kdata,
    uint8_t *remote_pubkey, uint32_t remote_pubkey_size, uint8_t *create_key, uint32_t create_key_size);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t kms_check_alg_and_permit(uint8_t algo_id, uint32_t permit)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (permit > KEY_USAGE_PERMIT_ALL_BITS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // nothing to do
    }

    return ret;
}

static uint32_t kms_check_key_size_param(
    uint8_t algo_id, uint32_t key_size, uint32_t dh_common_size, uint32_t rsa_e_bit_size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = g_key_size_arry[algo_id].type;

    if (key_type == KMS_ALG_TYPE_SYM) {
        uint16_t key_max_size = g_key_size_arry[algo_id].privkey_size;
        ret = (key_size > key_max_size) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
    } else if (key_type == KMS_ALG_TYPE_DH) {
        if ((dh_common_size > KMS_DH_COMMON_DATA_MAX_SIZE) || (key_size > KMS_DH_PRIVKEY_MAX_SIZE)) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    } else if ((key_type == KMS_ALG_TYPE_RSA_CRT) || (key_type == KMS_ALG_TYPE_RSA_COMM)) {
        uint32_t rsa_e_byte_size = (rsa_e_bit_size + 7U) / 8U;
        ret = (rsa_e_byte_size > RSA_KEY_E_SIZE_LIMITE) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
    } else {
        // nothing to do
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (((algo_id == KMS_KEY_ALG_RANDOM) || (algo_id == KMS_KEY_ALG_HMAC)) && (key_size == 0U)) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

static kms_key_format_st *kms_get_key_format_pointer(void *key_data)
{
    kms_key_format_st *khead = NULL;

    if (key_data != NULL) {
        khead = (kms_key_format_st *)key_data;
    }

    return khead;
}

static uint32_t kms_check_genkey_param(const mb_cmd_ehsm_gen_key_st *cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (cmd_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (kms_check_alg_and_permit(cmd_data->algo_id, cmd_data->permit) != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((cmd_data->algo_id >= KMS_KEY_ALG_SM9_ENC_USERPRIV)
        && (cmd_data->algo_id <= KMS_KEY_ALG_SM9_EXCH_TMPKEY)) {
        ret = EHSM_ERR_NOT_SUPPORT;
    } else if (kms_check_key_size_param(
                   cmd_data->algo_id, cmd_data->key_size, cmd_data->dh_common_size, cmd_data->rsa_e_bit_size)
        != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // nothing to do
    }

    return ret;
}

static uint32_t kms_fill_key_head(const mb_cmd_ehsm_gen_key_st *cmd_data, kms_key_format_st *khead, uint32_t *key_size)
{
    uint32_t dh_p_size = 0;
    uint32_t dh_q_size = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((cmd_data == NULL) || (khead == NULL) || (key_size == NULL) || (cmd_data->algo_id >= KMS_KEY_ALG_END)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        key_type = g_key_size_arry[cmd_data->algo_id].type;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        khead->algo_id = cmd_data->algo_id;
        khead->permit = cmd_data->permit;
        khead->part_info = KMS_KEY_PART_PAIRKEY;

        if (key_type == KMS_ALG_TYPE_SYM) {
            khead->part_info = KMS_KEY_PART_PRIVKEY;
            khead->priv_key_size
                = ((cmd_data->algo_id == KMS_KEY_ALG_RANDOM) || (cmd_data->algo_id == KMS_KEY_ALG_HMAC))
                ? (uint16_t)cmd_data->key_size
                : g_key_size_arry[cmd_data->algo_id].privkey_size;
            *key_size = khead->priv_key_size;
        }

        if (key_type == KMS_ALG_TYPE_ECC) {
            khead->pub_key_size = g_key_size_arry[cmd_data->algo_id].pubkey_size;
            khead->priv_key_size = g_key_size_arry[cmd_data->algo_id].privkey_size;
            *key_size = (uint32_t)khead->pub_key_size + khead->priv_key_size;
        }

        if ((key_type == KMS_ALG_TYPE_RSA_CRT) || (key_type == KMS_ALG_TYPE_RSA_COMM)) {
            uint32_t rsa_e_bytes = ((uint32_t)cmd_data->rsa_e_bit_size + 7U) >> 3;
            uint32_t rsa_n_bytes = g_key_size_arry[cmd_data->algo_id].pubkey_size;
            uint32_t rsa_d_bytes = (key_type == KMS_ALG_TYPE_RSA_CRT) ? (rsa_n_bytes / 2U * 5U) : rsa_n_bytes;
            khead->pub_key_size = (uint16_t)((rsa_e_bytes + 0x3U) & (~0x3U));
            khead->priv_key_size = (uint16_t)rsa_d_bytes;
            *key_size = khead->pub_key_size + rsa_n_bytes + rsa_d_bytes;
        }

        if (key_type == KMS_ALG_TYPE_DH) {
            ret = mmap_read_remote_data(&dh_p_size, cmd_data->dh_common_addr, 4);
            if ((EHSM_ERR_SW_SUCCESS == ret) && (dh_p_size <= KMS_RSA_DH_P_PARAM_MAX_SIZE)) {
                ret = mmap_read_remote_data(&dh_q_size, cmd_data->dh_common_addr + 4U + dh_p_size, 4);
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                if ((dh_p_size > KMS_RSA_DH_P_PARAM_MAX_SIZE) || (dh_q_size > KMS_RSA_DH_Q_PARAM_MAX_SIZE)) {
                    ret = EHSM_ERR_INVALID_DH_PQGH_SIZE;
                } else {
                    khead->pub_key_size = (uint16_t)dh_p_size;
                    khead->priv_key_size = ((cmd_data->key_size != 0U) ? cmd_data->key_size : (uint16_t)dh_q_size);
                    *key_size = (uint32_t)khead->pub_key_size + khead->priv_key_size;
                }
            }
        }
    }

    return ret;
}

static uint32_t kms_gen_rsa_dh_key(const mb_cmd_ehsm_gen_key_st *cmd_data, const kms_keypair_st *keypair)
{
    cpt_dh_para_st dh_para[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t dh_common_data[KMS_DH_COMMON_DATA_MAX_SIZE >> 2];

    if ((cmd_data == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (cmd_data->key_size > KMS_DH_PRIVKEY_MAX_SIZE) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = kms_parse_dh_common_data(
            cmd_data->dh_common_addr, cmd_data->dh_common_size, dh_common_data, KMS_DH_COMMON_DATA_MAX_SIZE, dh_para);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_dh_generate_key(dh_para, keypair->dh.priv_key, keypair->dh.pub_key, cmd_data->key_size);
        }
    }

    return ret;
}

static uint32_t kms_crypto_gen_ecc_key(uint8_t algo_id, const kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (keypair == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (algo_id == KMS_KEY_ALG_SM2) {
            ret = cpt_sm2_getkey(keypair->ecc.priv_key, keypair->ecc.pub_key);
        } else if (algo_id == KMS_KEY_ALG_ED25519) {
            ret = cpt_ed25519_getkey(keypair->ecc.priv_key, keypair->ecc.pub_key);
        } else if (algo_id == KMS_KEY_ALG_X25519) {
            ret = cpt_x25519_getkey(keypair->ecc.priv_key, keypair->ecc.pub_key);
        } else {
            cpt_eccp_curve_st const *ec_curve = NULL;
            ec_curve = get_curve_param_type(algo_id);
            if (ec_curve != NULL) {
                ret = cpt_eccp_getkey(ec_curve, keypair->ecc.priv_key, keypair->ecc.pub_key);
            } else {
                ret = EHSM_ERR_INVALID_ALGORITHM;
            }
        }
    }

    return ret;
}

static uint32_t kms_gen_key_data(const mb_cmd_ehsm_gen_key_st *cmd_data, const kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((cmd_data == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (cmd_data->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        key_type = g_key_size_arry[cmd_data->algo_id].type;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (key_type == KMS_ALG_TYPE_DH) {
            ret = kms_gen_rsa_dh_key(cmd_data, keypair);
        } else {
            if (key_type == KMS_ALG_TYPE_SYM) {
                ret = cpt_get_rand(keypair->symm.key, keypair->symm.size);
            } else if (key_type == KMS_ALG_TYPE_ECC) {
                ret = kms_crypto_gen_ecc_key(cmd_data->algo_id, keypair);
            } else if (key_type == KMS_ALG_TYPE_RSA_COMM) {
                watchdog_stop(); // stop watchdog due to long processing time
                ret = cpt_rsa_get_key(keypair->rsa.e, keypair->rsa.p_d, keypair->rsa.n, cmd_data->rsa_e_bit_size,
                    (uint32_t)keypair->rsa.n_size << 3U);
                watchdog_start(DEFAULT_WDT_TIMEOUT); // restart watchdog
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    reverse_byte_array((uint8_t *)keypair->rsa.e, (uint8_t *)keypair->rsa.e, keypair->rsa.e_size);
                    reverse_byte_array((uint8_t *)keypair->rsa.n, (uint8_t *)keypair->rsa.n, keypair->rsa.n_size);
                    reverse_byte_array((uint8_t *)keypair->rsa.p_d, (uint8_t *)keypair->rsa.p_d, keypair->rsa.n_size);
                }
            } else if (key_type == KMS_ALG_TYPE_RSA_CRT) {
                watchdog_stop(); // stop watchdog due to long processing time
                ret = cpt_rsa_get_crtkey(keypair->rsa.e, keypair->rsa.p_d, keypair->rsa.q, keypair->rsa.dp,
                    keypair->rsa.dq, keypair->rsa.u, keypair->rsa.n, cmd_data->rsa_e_bit_size,
                    (uint32_t)keypair->rsa.n_size << 3U);
                watchdog_start(DEFAULT_WDT_TIMEOUT); // restart watchdog
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    reverse_byte_array((uint8_t *)keypair->rsa.e, (uint8_t *)keypair->rsa.e, keypair->rsa.e_size);
                    reverse_byte_array((uint8_t *)keypair->rsa.n, (uint8_t *)keypair->rsa.n, keypair->rsa.n_size);
                    reverse_byte_array((uint8_t *)keypair->rsa.p_d, (uint8_t *)keypair->rsa.p_d,
                        ((uint32_t)keypair->rsa.n_size >> 1U));
                    reverse_byte_array(
                        (uint8_t *)keypair->rsa.q, (uint8_t *)keypair->rsa.q, ((uint32_t)keypair->rsa.n_size >> 1U));
                    reverse_byte_array(
                        (uint8_t *)keypair->rsa.dq, (uint8_t *)keypair->rsa.dq, ((uint32_t)keypair->rsa.n_size >> 1U));
                    reverse_byte_array(
                        (uint8_t *)keypair->rsa.dp, (uint8_t *)keypair->rsa.dp, ((uint32_t)keypair->rsa.n_size >> 1U));
                    reverse_byte_array(
                        (uint8_t *)keypair->rsa.u, (uint8_t *)keypair->rsa.u, ((uint32_t)keypair->rsa.n_size >> 1U));
                }
            } else {
                ret = EHSM_ERR_NOT_SUPPORT;
            }
        }
    }

    return ret;
}

static uint32_t kms_sign_verify_key_data(uint32_t key_handle, const uint8_t *in, uint32_t in_size, uint8_t *out,
    uint32_t *out_size, cpt_ske_mac_e mac_action)
{
    cpt_ske_alg_e ske_alg;
    uint8_t iv[32];
    uint32_t mac_size;
    kms_keydata_st key_data;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    /*The encryption and decryption key only support symmetric key*/
    uint8_t key_buff[KMS_KEY_HEAD_SIZE + KMS_SYM_KEY_DATA_MAX_SIZE];

    util_memset(iv, 0x00, sizeof(iv));

    if ((in == NULL) || (out == NULL) || (out_size == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (*out_size < KMS_KEY_SIGNATURE_BLOCK_MIN_SIZE) {
        ret = EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT;
    } else {
        ret = kms_read_key(
            key_handle, key_buff, sizeof(key_buff), &key_data, KMS_KEY_PART_PRIVKEY, KEY_USAGE_TRANSPORT);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if ((key_data.algo_id < KMS_KEY_ALG_END) && (g_key_size_arry[key_data.algo_id].type == KMS_ALG_TYPE_SYM)) {
            if ((key_data.algo_id == KMS_KEY_ALG_RANDOM) || (key_data.algo_id == KMS_KEY_ALG_HMAC)
                || (key_data.algo_id == KMS_KEY_ALG_CHACHA)) {
                ret = EHSM_ERR_NOT_SUPPORT;
            } else {
                ret = kms_get_ske_alg(key_data.algo_id, &ske_alg);
            }
        } else {
            ret = EHSM_ERR_NOT_SUPPORT;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        mac_size = cpt_ske_get_block_byte_len(ske_alg);
        mac_size = (*out_size > mac_size) ? mac_size : *out_size;

        if (key_data.src_type == KMS_KEY_SRC_TYPE_SECRUE_PORT) {
            ret = cpt_ske_cmac(ske_alg, mac_action, NULL, key_data.sp_key_id, in, in_size, out, (uint8_t)mac_size);
        } else {
            ret = cpt_ske_cmac(ske_alg, mac_action, key_data.keypair.symm.key, 0, in, in_size, out, (uint8_t)mac_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            *out_size = (mac_action == SKE_GENERATE_MAC) ? mac_size : *out_size;
        }
    }
    util_memset(key_buff, 0x00, sizeof(key_buff));

    return ret;
}

static uint32_t kms_en_dec_key_data(uint32_t key_handle, const uint8_t *in, uint32_t in_size, uint8_t *out,
    uint32_t *out_size, cpt_ske_crypto_e crypto_dir)
{
    cpt_ske_alg_e ske_alg;
    uint8_t iv[32];
    kms_keydata_st key_data;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    /*The encryption and decryption key only support symmetric key*/
    uint8_t key_buff[KMS_KEY_HEAD_SIZE + KMS_SYM_KEY_DATA_MAX_SIZE];

    util_memset(iv, 0x00, sizeof(iv));
    if ((in == NULL) || (out == NULL) || (out_size == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = kms_read_key(
            key_handle, key_buff, sizeof(key_buff), &key_data, KMS_KEY_PART_PRIVKEY, KEY_USAGE_TRANSPORT);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if ((key_data.algo_id < KMS_KEY_ALG_END) && (g_key_size_arry[key_data.algo_id].type == KMS_ALG_TYPE_SYM)) {
            if ((key_data.algo_id == KMS_KEY_ALG_RANDOM) || (key_data.algo_id == KMS_KEY_ALG_HMAC)
                || (key_data.algo_id == KMS_KEY_ALG_CHACHA)) {
                ret = EHSM_ERR_NOT_SUPPORT;
            } else {
                ret = kms_get_ske_alg(key_data.algo_id, &ske_alg);
            }
        } else {
            ret = EHSM_ERR_NOT_SUPPORT;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (key_data.src_type == KMS_KEY_SRC_TYPE_SECRUE_PORT) {
            ret = cpt_ske_crypto(ske_alg, SKE_MODE_CBC, crypto_dir, NULL, key_data.sp_key_id, iv, SKE_PKCS_5_7_PADDING,
                in, out, in_size, (uint32_t *)out_size);
        } else {
            ret = cpt_ske_crypto(ske_alg, SKE_MODE_CBC, crypto_dir, key_data.keypair.symm.key, 0, iv,
                SKE_PKCS_5_7_PADDING, in, out, in_size, (uint32_t *)out_size);
        }
    }
    util_memset(key_buff, 0x00, sizeof(key_buff));

    return ret;
}

static uint32_t kms_check_operation_permission(void)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;


    return ret;
}

static uint32_t kms_get_key_handle_permission(const uint32_t key_handle, uint32_t *key_permit)
{
    kms_key_format_st key;
    otp_key_attributes_st otp_k_attr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_type = key_handle & KMS_KEY_TYPE_MASK;

    if (key_permit == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((key_type != KMS_KEY_TYPE_OTP) && (key_type != KMS_KEY_TYPE_SHE) && (key_type != KMS_KEY_TYPE_EHSM)) {
        ret = EHSM_ERR_INVALID_HANDLE;
    } else {
        // nothing to do
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        /*check target key_handle whether exist, and check it permit flag whether match with request*/
        if (key_type == KMS_KEY_TYPE_OTP) {
            ret = otpkey_aquire_key_attr(key_handle, &otp_k_attr);
            if (ret == EHSM_ERR_SW_SUCCESS) {
                *key_permit = otp_k_attr.key_usage;
            }
        } else if (key_type == KMS_KEY_TYPE_EHSM) {
            ret = kds_read_key(key_handle, 0, &key, sizeof(key));
            if (ret == EHSM_ERR_SW_SUCCESS) {
                *key_permit = key.permit;
            }
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

static uint32_t kms_check_derive_key_params_ext(const mb_cmd_ehsm_key_derive_st *cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (cmd_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (cmd_data->key_deriv_func == KMS_KEY_DERIVE_FUN_PBKDF2) {
            if (cmd_data->salt_data_size > KMS_DERIVE_SALT_MAX_SIZE) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (cmd_data->derive_type == KMS_KEY_DERIVE_USER_PASSWD) {
            if ((cmd_data->pw_data_addr == KMS_INVALID_DATA_ADDR)
                || (cmd_data->pw_data_size > KMS_DERIVE_PASSWORD_MAX_SIZE)) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        } else if (cmd_data->derive_type == KMS_KEY_DERIVE_USER_KEYHANDLE) {
            if (cmd_data->parent_key_handle == KMS_INVALID_KEY_ID) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

static uint32_t kms_check_derive_key_params(const mb_cmd_ehsm_key_derive_st *cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (cmd_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (kms_check_alg_and_permit(cmd_data->algo_id, cmd_data->permit) != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    /*only support symmetric key*/
    else if ((g_key_size_arry[cmd_data->algo_id].type != KMS_ALG_TYPE_SYM)
        || (cmd_data->hash_alg > MB_EHSM_KEY_DERIVE_HASH_ALG_SHA3_512)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // nothing to do
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        uint16_t key_max_size = g_key_size_arry[cmd_data->algo_id].privkey_size;

        if ((cmd_data->algo_id == KMS_KEY_ALG_RANDOM) || (cmd_data->algo_id == KMS_KEY_ALG_HMAC)) {
            ret = ((cmd_data->key_size == 0U) || (cmd_data->key_size > key_max_size)) ? EHSM_ERR_PARAM_ERROR
                                                                                      : EHSM_ERR_SW_SUCCESS;
        } else {
            ret = (cmd_data->key_size != key_max_size) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if ((cmd_data->key_deriv_func > KMS_KEY_DERIVE_FUN_PBKDF2)
            || (cmd_data->key_deriv_func < KMS_KEY_DERIVE_FUN_KDFX963)
            || (cmd_data->key_out_dir != KMS_KEY_DERIVE_OUTPUT_TO_KMS)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = kms_check_derive_key_params_ext(cmd_data);
        }
    }

    return ret;
}

static uint32_t kms_check_exchange_sm2_key_params(const mb_sm2_param_struct_st *sm2_param)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (sm2_param == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (sm2_param->local_tmp_key_handle == KMS_INVALID_KEY_ID) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((sm2_param->peer_temp_pubkey_size) > (KMS_KEY_HEAD_SIZE + KMS_ECC_KEYPAIR_MAX_SIZE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // nothing to do;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (sm2_param->sm2_role > KMS_SM2_ROLE_RESPONSOR) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else if ((sm2_param->s1_s2_value_addr == KMS_INVALID_DATA_ADDR)
            || (sm2_param->sa_sb_value_addr == KMS_INVALID_DATA_ADDR)
            || (sm2_param->peer_temp_pubkey_addr == KMS_INVALID_DATA_ADDR)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            // nothing to do;
        }
    }

    return ret;
}

static uint32_t kms_check_exchange_key_params(const mb_cmd_ehsm_key_exchange_st *cmd_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (cmd_data == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (kms_check_alg_and_permit(cmd_data->algo_id, cmd_data->permit) != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // algo_id here refers to the output session key algorithm (must be symmetric),
        // not the local key pair algorithm (which is ECC/DH).
        ret = (g_key_size_arry[cmd_data->algo_id].type != KMS_ALG_TYPE_SYM) ? EHSM_ERR_PARAM_ERROR
                                                                            : EHSM_ERR_SW_SUCCESS;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if ((cmd_data->remote_pubkey_data == KMS_INVALID_DATA_ADDR)
            || (cmd_data->remote_pubkey_size > KMS_DH_PUBKEY_MAX_SIZE)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else if (cmd_data->local_key_handle == KMS_INVALID_KEY_ID) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else if (cmd_data->dh_common_size > KMS_DH_COMMON_DATA_MAX_SIZE) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            // nothing to do
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        uint16_t key_max_size = g_key_size_arry[cmd_data->algo_id].privkey_size;

        if ((cmd_data->algo_id == KMS_KEY_ALG_RANDOM) || (cmd_data->algo_id == KMS_KEY_ALG_HMAC)) {
            ret = ((cmd_data->key_size == 0U) || (cmd_data->key_size > key_max_size)) ? EHSM_ERR_PARAM_ERROR
                                                                                      : EHSM_ERR_SW_SUCCESS;
        } else {
            ret = (cmd_data->key_size != key_max_size) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
        }
    }

    return ret;
}

static uint32_t kms_gen_derived_key(const mb_cmd_ehsm_key_derive_st *cmd_data, const kms_keydata_st *keydata,
    uint8_t *create_key, uint16_t create_key_size)
{
    const uint8_t *salt_ptr = NULL;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t salt_buff[KMS_DERIVE_SALT_MAX_SIZE];

    if ((cmd_data == NULL) || (keydata == NULL) || (create_key == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (((keydata->src_type == KMS_KEY_SRC_TYPE_SECRUE_PORT)
                   && (KMS_KEY_DERIVE_FUN_KDFX963 == cmd_data->key_deriv_func))
        || (keydata->algo_id >= KMS_KEY_ALG_END) || (g_key_size_arry[keydata->algo_id].type != KMS_ALG_TYPE_SYM)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (cmd_data->salt_data_addr != KMS_INVALID_DATA_ADDR) {
            salt_ptr = salt_buff;
            ret = mmap_read_remote_data(salt_buff, cmd_data->salt_data_addr, cmd_data->salt_data_size);
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (KMS_KEY_DERIVE_FUN_PBKDF2 == cmd_data->key_deriv_func) {
            const uint8_t *root_password = NULL;
            root_password = (keydata->src_type != KMS_KEY_SRC_TYPE_SECRUE_PORT) ? keydata->keypair.symm.key : NULL;
            ret = cpt_pbkdf2_hmac(get_lib_hash_alg(cmd_data->hash_alg), root_password, keydata->sp_key_id,
                keydata->keypair.symm.size, salt_ptr, cmd_data->salt_data_size, cmd_data->itera_times, create_key,
                create_key_size);
        } else
        {
            ret = cpt_ansi_x9_63_kdf(get_lib_hash_alg(cmd_data->hash_alg), keydata->keypair.symm.key,
                keydata->keypair.symm.size, NULL, 0, create_key, create_key_size, NULL, 0);
        }
    }

    return ret;
}

static uint32_t kms_gen_exchange_sm2_key(const mb_cmd_ehsm_key_exchange_st *cmd_data, const kms_keydata_st *local_kdata,
    const uint8_t *remote_pubkey, uint8_t *create_key, uint32_t create_key_size)
{
    kms_keydata_st local_tmp_kdata;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t local_tmp_key[KMS_KEY_HEAD_SIZE + KMS_ECC_KEYPAIR_MAX_SIZE];
    uint8_t peer_tmp_pubkey[KMS_KEY_HEAD_SIZE + KMS_ECC_KEYPAIR_MAX_SIZE];

    if ((cmd_data == NULL) || (local_kdata == NULL) || (remote_pubkey == NULL) || (create_key == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (local_kdata->algo_id != KMS_KEY_ALG_SM2) {
        ret = EHSM_ERR_INVALID_ALGORITHM;
    } else {
        mb_sm2_param_struct_st sm2_param;
        ret = mmap_read_remote_data(&sm2_param, cmd_data->sm2_param_addr, sizeof(mb_sm2_param_struct_st));
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_check_exchange_sm2_key_params(&sm2_param);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = mmap_read_remote_data(
                peer_tmp_pubkey, sm2_param.peer_temp_pubkey_addr, sm2_param.peer_temp_pubkey_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_read_key(sm2_param.local_tmp_key_handle, local_tmp_key, sizeof(local_tmp_key), &local_tmp_kdata,
                KMS_KEY_PART_PAIRKEY, KEY_USAGE_KEYCREATION);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            uint8_t za[32];
            uint8_t zb[32];
            uint8_t sa[32];
            uint8_t sb[32];
            cpt_sm2_exchange_role_e role
                = (sm2_param.sm2_role == KMS_SM2_ROLE_SPONSOR) ? SM2_Role_Sponsor : SM2_Role_Responsor;

            ret = cpt_sm2_getZ(NULL, 0, (uint8_t *)local_kdata->keypair.ecc.pub_key, za);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_sm2_getZ(NULL, 0, (const uint8_t *)remote_pubkey, zb);
            }
            if (ret == EHSM_ERR_SW_SUCCESS) {
                ret = cpt_sm2_exchangekey(role, local_kdata->keypair.ecc.priv_key, remote_pubkey,
                    local_tmp_kdata.keypair.ecc.priv_key, local_tmp_kdata.keypair.ecc.pub_key, peer_tmp_pubkey, za, zb,
                    create_key_size, create_key, sa, sb);
            }

            if (ret == EHSM_ERR_SW_SUCCESS) {
                ret = mmap_write_remote_data(sm2_param.s1_s2_value_addr, sa, sizeof(sa));
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = mmap_write_remote_data(sm2_param.sa_sb_value_addr, sb, sizeof(sb));
                }
            }
        }
    }
    util_memset(local_tmp_key, 0x00, sizeof(local_tmp_key));
    return ret;
}

static uint32_t kms_gen_exchange_key(const mb_cmd_ehsm_key_exchange_st *cmd_data, const kms_keydata_st *local_kdata,
    uint8_t *remote_pubkey, uint32_t remote_pubkey_size, uint8_t *create_key, uint32_t create_key_size)
{
    uint32_t dh_key_len = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((cmd_data == NULL) || (local_kdata == NULL) || (remote_pubkey == NULL) || (create_key == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (local_kdata->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_INVALID_ALGORITHM;
    } else if ((g_key_size_arry[local_kdata->algo_id].type != KMS_ALG_TYPE_DH)
        && (g_key_size_arry[local_kdata->algo_id].type != KMS_ALG_TYPE_ECC)) {
        ret = EHSM_ERR_INVALID_ALGORITHM;
    } else {
        // nothing to do
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (local_kdata->algo_id == KMS_KEY_ALG_SM2) {
            ret = kms_gen_exchange_sm2_key(cmd_data, local_kdata, remote_pubkey, create_key, create_key_size);
        } else if (local_kdata->algo_id == KMS_KEY_ALG_RSA_DH) {
            ret = kms_gen_exchange_dh_key(
                cmd_data, local_kdata, remote_pubkey, remote_pubkey_size, create_key, create_key_size);
        } else {
            if (local_kdata->algo_id == KMS_KEY_ALG_X25519) {
                dh_key_len = C25519_BYTE_LEN;
                ret = cpt_x25519_compute_key(
                    local_kdata->keypair.ecc.priv_key, remote_pubkey, create_key, dh_key_len, NULL);
            } else {
                const cpt_eccp_curve_st *ec_curve = get_curve_param_type(local_kdata->algo_id);
                if (ec_curve != NULL) {
                    dh_key_len = GET_BYTE_LEN(ec_curve->eccp_p_bitLen);
                    ret = cpt_ecdh_compute_key(
                        ec_curve, local_kdata->keypair.ecc.priv_key, remote_pubkey, create_key, dh_key_len, NULL);
                } else {
                    ret = EHSM_ERR_PARAM_ERROR;
                }
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = (create_key_size > dh_key_len) ? EHSM_ERR_DH_KEY_TOO_LEN : EHSM_ERR_SW_SUCCESS;
            }
        }
    }

    return ret;
}

static uint32_t kms_calcu_key_mem_offset(
    const kms_key_format_st *khead, const uint8_t *req_kpart, uint32_t *koffset, uint32_t *ksize)
{
    uint16_t k_storage_size = 0;
    kms_key_mem_size_st kmm_size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((khead == NULL) || (koffset == NULL) || (ksize == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((khead->part_info & *req_kpart) != *req_kpart) {
        ret = EHSM_ERR_NOT_EXIST_KEY_PART;
    } else {
        ret = kms_calcu_key_mem_size(khead, &kmm_size);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        *koffset = 0;

        if (*req_kpart == KMS_KEY_PART_PAIRKEY) {
            k_storage_size
                = (uint16_t)(kmm_size.priv_key_mem_size + kmm_size.ext_key_mem_size + kmm_size.pub_key_mem_size);
        }

        if (*req_kpart == KMS_KEY_PART_PUBKEY) {
            k_storage_size = kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size;
        }

        if (*req_kpart == KMS_KEY_PART_PRIVKEY) {
            // Note that if the key type is ED25519 or SM2, will attempt to read the entire key pair data, because
            // considering the following resons:
            // 1. According crypto library requset ED25519 sign need key pair data, requesting a private key is actually
            // a key pair.
            // 2. sm2 exchange need the public key to calculate the z value.
            if ((khead->algo_id == KMS_KEY_ALG_ED25519) || (khead->algo_id == KMS_KEY_ALG_SM2)) {
                k_storage_size
                    = (uint16_t)(kmm_size.priv_key_mem_size + kmm_size.ext_key_mem_size + kmm_size.pub_key_mem_size);
            } else if (g_key_size_arry[khead->algo_id].type == KMS_ALG_TYPE_RSA_CRT) {
                // Rsa crt private key not include rsa_n data
                *koffset = (uint32_t)kmm_size.pub_key_mem_size + (uint32_t)kmm_size.ext_key_mem_size;
                k_storage_size = kmm_size.priv_key_mem_size;
            } else {
                *koffset = kmm_size.pub_key_mem_size;
                k_storage_size = kmm_size.priv_key_mem_size + kmm_size.ext_key_mem_size;
            }
        }

        if (k_storage_size > *ksize) {
            ret = EHSM_ERR_OUT_OF_MEM;
        } else {
            *ksize = k_storage_size;
        }
    }

    return ret;
}

static uint32_t kms_calcu_key_mem_size(const kms_key_format_st *khead, kms_key_mem_size_st *kmem_size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e type = KMS_ALG_TYPE_NONE;

    if ((khead == NULL) || (kmem_size == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (khead->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        type = g_key_size_arry[khead->algo_id].type;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        util_memset(kmem_size, 0x00, sizeof(kms_key_mem_size_st));

        if (type == KMS_ALG_TYPE_SYM) {
            kmem_size->priv_key_mem_size = khead->priv_key_size;
        } else if ((type == KMS_ALG_TYPE_DH) || (type == KMS_ALG_TYPE_ECC)) {
            kmem_size->pub_key_mem_size = khead->pub_key_size;
            kmem_size->priv_key_mem_size = khead->priv_key_size;
        } else {
            kmem_size->pub_key_mem_size = khead->pub_key_size;
            kmem_size->priv_key_mem_size = khead->priv_key_size;
            kmem_size->ext_key_mem_size = g_key_size_arry[khead->algo_id].pubkey_size;
        }
    }

    return ret;
}

static uint32_t kms_alloc_pairkey_mem(uint8_t *data, const kms_key_format_st *khead, kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((data == NULL) || (khead == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (khead->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        key_type = g_key_size_arry[khead->algo_id].type;

        if (key_type == KMS_ALG_TYPE_SYM) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        util_memset(keypair, 0x00, sizeof(kms_keypair_st));

        if (key_type == KMS_ALG_TYPE_ECC) {
            keypair->ecc.pub_key = data;
            keypair->ecc.pub_size = khead->pub_key_size;
            keypair->ecc.priv_key = &keypair->ecc.pub_key[khead->pub_key_size];
            keypair->ecc.priv_size = khead->priv_key_size;
        }

        if (key_type == KMS_ALG_TYPE_DH) {
            keypair->dh.pub_key = data;
            keypair->dh.pub_size = khead->pub_key_size;
            keypair->dh.priv_key = &keypair->dh.pub_key[khead->pub_key_size];
            keypair->dh.priv_size = khead->priv_key_size;
        }

        if ((key_type == KMS_ALG_TYPE_RSA_CRT) || (key_type == KMS_ALG_TYPE_RSA_COMM)) {
            uint16_t rsa_n_byte_size = g_key_size_arry[khead->algo_id].pubkey_size;

            keypair->rsa.e = (uint32_t *)(void *)data;
            /*the rsa member pointer is *uint32*/
            keypair->rsa.e_size = khead->pub_key_size;
            keypair->rsa.n = &keypair->rsa.e[khead->pub_key_size >> 2];
            keypair->rsa.n_size = rsa_n_byte_size;
            keypair->rsa.p_d = &keypair->rsa.n[rsa_n_byte_size >> 2];
            if (key_type == KMS_ALG_TYPE_RSA_CRT) {
                keypair->rsa.q = &keypair->rsa.p_d[rsa_n_byte_size >> 3U];
                keypair->rsa.dp = &keypair->rsa.q[rsa_n_byte_size >> 3U];
                keypair->rsa.dq = &keypair->rsa.dp[rsa_n_byte_size >> 3U];
                keypair->rsa.u = &keypair->rsa.dq[rsa_n_byte_size >> 3U];
            }
        }
    }

    return ret;
}

static uint32_t kms_alloc_privkey_mem(uint8_t *data, const kms_key_format_st *khead, kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((data == NULL) || (khead == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (khead->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        key_type = g_key_size_arry[khead->algo_id].type;
        util_memset(keypair, 0x00, sizeof(kms_keypair_st));

        if (key_type == KMS_ALG_TYPE_SYM) {
            keypair->symm.key = data;
            keypair->symm.size = khead->priv_key_size;
        }

        if (key_type == KMS_ALG_TYPE_ECC) {
            // Note that if the key type is ED25519 or SM2, will attempt to read the entire key pair data, because
            // considering the following resons:
            // 1. According crypto library requset ED25519 sign need key pair data, requesting a private key is actually
            // a key pair.
            // 2. Sm2 exchange need the public key to calculate the z value.
            if ((khead->part_info == KMS_KEY_PART_PAIRKEY)
                && ((khead->algo_id == KMS_KEY_ALG_ED25519) || (khead->algo_id == KMS_KEY_ALG_SM2))) {
                keypair->ecc.pub_key = data;
                keypair->ecc.pub_size = khead->pub_key_size;
                keypair->ecc.priv_key = &keypair->ecc.pub_key[khead->pub_key_size];
                keypair->ecc.priv_size = khead->priv_key_size;
            } else {
                keypair->ecc.priv_key = data;
                keypair->ecc.priv_size = khead->priv_key_size;
            }
        }

        if ((key_type == KMS_ALG_TYPE_RSA_CRT) || (key_type == KMS_ALG_TYPE_RSA_COMM)) {
            uint16_t rsa_n_byte_size = g_key_size_arry[khead->algo_id].pubkey_size;
            keypair->rsa.n_size = rsa_n_byte_size;
            if (key_type == KMS_ALG_TYPE_RSA_CRT) {
                keypair->rsa.p_d = (uint32_t *)(void *)data;
                keypair->rsa.q = &keypair->rsa.p_d[rsa_n_byte_size >> 3U];
                keypair->rsa.dp = &keypair->rsa.q[rsa_n_byte_size >> 3U];
                keypair->rsa.dq = &keypair->rsa.dp[rsa_n_byte_size >> 3U];
                keypair->rsa.u = &keypair->rsa.dq[rsa_n_byte_size >> 3U];
            } else {
                keypair->rsa.n = (uint32_t *)(void *)data;
                keypair->rsa.p_d = &keypair->rsa.n[rsa_n_byte_size >> 2];
            }
        }

        if (key_type == KMS_ALG_TYPE_DH) {
            keypair->dh.priv_key = data;
            keypair->dh.priv_size = khead->priv_key_size;
        }

    }

    return ret;
}
/**
 *   @brief     According to the key header information assign the data address to the corresponding member in the
 * kms_keydata_st structure.
 *
 *   @param [in] data        The data address
 *   @param [in] khead       A pointer point to the the key header structure
 *   @param [in] req_kpart   The key part information
 *   @param [in] keypair     A pointer point to structure of kms_keydata_st
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_alloc_pubkey_mem(uint8_t *data, const kms_key_format_st *khead, kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((data == NULL) || (khead == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (khead->algo_id >= KMS_KEY_ALG_END) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        key_type = g_key_size_arry[khead->algo_id].type;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        util_memset(keypair, 0x00, sizeof(kms_keypair_st));
        if (key_type == KMS_ALG_TYPE_ECC) {
            keypair->ecc.pub_key = data;
            keypair->ecc.pub_size = khead->pub_key_size;
        } else if ((key_type == KMS_ALG_TYPE_RSA_CRT) || (key_type == KMS_ALG_TYPE_RSA_COMM)) {
            uint16_t rsa_n_byte_size = g_key_size_arry[khead->algo_id].pubkey_size;
            keypair->rsa.e = (uint32_t *)(void *)data;
            keypair->rsa.e_size = khead->pub_key_size;
            keypair->rsa.n = &keypair->rsa.e[khead->pub_key_size >> 2];
            keypair->rsa.n_size = rsa_n_byte_size;
        } else if (key_type == KMS_ALG_TYPE_DH) {
            keypair->dh.pub_key = data;
            keypair->dh.pub_size = khead->pub_key_size;
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}
/**
 *   @brief     According to the key header information and key partion assign the data address to the corresponding
 * member in the kms_keydata_st structure.
 *
 *   @param [in] data        The data address
 *   @param [in] khead       A pointer point to the the key header structure
 *   @param [in] req_kpart   The key part information
 *   @param [in] keypair     A pointer point to structure of kms_keydata_st
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_alloc_key_mem(
    uint8_t *data, const kms_key_format_st *khead, uint8_t req_kpart, kms_keypair_st *keypair)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((data == NULL) || (khead == NULL) || (keypair == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((khead->part_info & req_kpart) != req_kpart) {
        ret = EHSM_ERR_NOT_EXIST_KEY_PART;
    } else {
        if (req_kpart == KMS_KEY_PART_PAIRKEY) {
            ret = kms_alloc_pairkey_mem(data, khead, keypair);
        } else if (req_kpart == KMS_KEY_PART_PUBKEY) {
            ret = kms_alloc_pubkey_mem(data, khead, keypair);
        } else {
            ret = kms_alloc_privkey_mem(data, khead, keypair);
        }
    }

    return ret;
}
/**
 *   @brief     Read the otp key data to user buffer, and the key data in the buffer will be parsed and the address of
 * each element in the key in the buffer will be assigned to the corresponding member in the kms_keydata_st structure.
 *
 *   @param [in] key_handle  The key handle to indicate which key to be read
 *   @param [in] key_buff    The buffer used to store the key data
 *   @param [in] buff_size   The buffer size
 *   @param [in] keydata     A pointer point to structure of kms_keydata_st
 *   @param [in] req_kpart   The part requesting to read the key
 *
 *   @return     uint32_t
 *
 *   @note Otp key only exist private key so the parameter of req_kpart only support input KMS_KEY_PART_PRIVKEY
 */
static uint32_t kms_read_otp_key(
    uint32_t key_handle, uint8_t *key_buff, uint32_t buff_size, kms_keydata_st *keydata, uint8_t req_kpart)
{
    otp_key_attributes_st otp_k_attr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    util_memset(keydata, 0x00, sizeof(kms_keydata_st));

    if ((key_buff == NULL) || (keydata == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (req_kpart != KMS_KEY_PART_PRIVKEY) {
        ret = EHSM_ERR_NOT_EXIST_KEY_PART;
    } else if (buff_size < OTP_KEY_DATA_SIZE) {
        ret = EHSM_ERR_OUT_OF_MEM;
    } else {
        ret = otpkey_aquire_key_attr(key_handle, &otp_k_attr);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (otp_k_attr.key_algo_id >= KMS_KEY_ALG_END) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            keydata->keypart = req_kpart;
            keydata->algo_id = otp_k_attr.key_algo_id;
            if (g_key_size_arry[otp_k_attr.key_algo_id].type == KMS_ALG_TYPE_SYM) {
                keydata->src_type = KMS_KEY_SRC_TYPE_SECRUE_PORT;
                keydata->keypair.symm.size = OTP_KEY_DATA_SIZE;
                ret = otpkey_get_phyid(key_handle, &keydata->sp_key_id);
            } else {
                keydata->src_type = KMS_KEY_SRC_TYPE_RAM;
                keydata->keypair.ecc.priv_key = key_buff;
                keydata->keypair.ecc.priv_size = OTP_KEY_DATA_SIZE;
                ret = otpkey_read_data(key_handle, keydata->keypair.ecc.priv_key);
            }
        }
    }

    return ret;
}
/**
 *   @brief     Read the ehsm key data to user buffer, and the key data in the buffer will be parsed and the address of
 * each element in the key in the buffer will be assigned to the corresponding member in the kms_keydata_st structure.
 *
 *   @param [in] key_handle  The key handle to indicate which key to be read
 *   @param [in] key_buff    The buffer used to store the key data
 *   @param [in] buff_size   The buffer size
 *   @param [in] keydata     A pointer point to structure of kms_keydata_st, the key data will be parsed
 *   @param [in] req_kpart   The part requesting to read the key
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_read_memory_key(
    uint32_t key_handle, uint8_t *key_buff, uint32_t buff_size, kms_keydata_st *keydata, uint8_t req_kpart)
{
    uint32_t ret;
    uint32_t kmem_off;
    uint32_t kmem_size;
    kms_key_format_st key_hd;
    uint8_t act_req_kpart = req_kpart;

    if ((key_buff == NULL) || (keydata == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memset(&key_hd, 0x00, sizeof(kms_key_format_st));

        ret = kds_read_key(key_handle, 0, &key_hd, KMS_KEY_HEAD_SIZE);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            kmem_size = buff_size;
            ret = kms_calcu_key_mem_offset(&key_hd, &act_req_kpart, &kmem_off, &kmem_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kds_read_key(key_handle, (uint16_t)(KMS_KEY_HEAD_SIZE + kmem_off), key_buff, (uint16_t)kmem_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_alloc_key_mem(key_buff, &key_hd, act_req_kpart, &keydata->keypair);
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        keydata->keypart = act_req_kpart;
        keydata->algo_id = key_hd.algo_id;
        keydata->src_type = KMS_KEY_SRC_TYPE_RAM;
    }

    return ret;
}
/**
 *   @brief     Calculate the public key data according to the related private key
 *
 *   @param [in] cmd_data   The mailbox command data
 *   @param [out] keypair   input the private key output the public key
 *   @param [in] key_alg_id   The private key algorithm id used to selection correct calculation function
 *   @param [in] pkey_max_size   The max size of public buffer
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_calcu_public_key_from_priv(const mb_cmd_ehsm_get_pub_from_priv_st *cmd_data,
    kms_keypair_st *keypair, uint8_t key_alg_id, uint32_t pkey_max_size)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((cmd_data == NULL) || (keypair == NULL) || (key_alg_id >= KMS_KEY_ALG_END)
        || (g_key_size_arry[key_alg_id].pubkey_size > pkey_max_size)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (key_alg_id == KMS_KEY_ALG_RSA_DH) {
            cpt_dh_para_st dh_para;
            uint32_t dh_common_data[KMS_DH_COMMON_DATA_MAX_SIZE >> 2];

            ret = kms_parse_dh_common_data(cmd_data->dh_common_addr, cmd_data->dh_common_size, dh_common_data,
                KMS_DH_COMMON_DATA_MAX_SIZE, &dh_para);
            if (ret == EHSM_ERR_SW_SUCCESS) {
                keypair->dh.pub_size = (uint16_t)(dh_para.p_bits >> 3);
                keypair->dh.pub_key = &keypair->dh.priv_key[KMS_DH_PRIVKEY_MAX_SIZE];
                ret = kms_expend_dh_keypair(keypair->dh.priv_key, keypair->dh.priv_size, keypair->dh.pub_key,
                    keypair->dh.pub_size, dh_para.p_bits >> 3, dh_para.q_bits >> 3);
                if (ret == EHSM_ERR_SW_SUCCESS) {
                    ret = cpt_dh_generate_pubkey_from_prikey(&dh_para, keypair->dh.priv_key, keypair->dh.pub_key);
                }
            }
        } else {
            keypair->ecc.pub_size = g_key_size_arry[key_alg_id].pubkey_size;
            keypair->ecc.pub_key = &keypair->ecc.priv_key[keypair->ecc.priv_size];

            if (key_alg_id == KMS_KEY_ALG_SM2) {
                ret = cpt_sm2_get_pubkey_from_prikey(keypair->ecc.priv_key, keypair->ecc.pub_key);
            } else if (key_alg_id == KMS_KEY_ALG_X25519) {
                ret = cpt_x25519_get_pubkey_from_prikey(keypair->ecc.priv_key, keypair->ecc.pub_key);
            } else if (key_alg_id == KMS_KEY_ALG_ED25519) {
                ret = cpt_ed25519_get_pubkey_from_prikey(keypair->ecc.priv_key, keypair->ecc.pub_key);
            } else {
                const cpt_eccp_curve_st *ec_curve = NULL;
                ec_curve = get_curve_param_type(key_alg_id);
                if (ec_curve != NULL) {
                    ret = cpt_eccp_get_pubkey_from_prikey(ec_curve, keypair->ecc.priv_key, keypair->ecc.pub_key);
                } else {
                    ret = EHSM_ERR_INVALID_ALGORITHM;
                }
            }
        }
    }

    return ret;
}
/**
 *   @brief     Check the imported key header data whether is valid,calculate the key data length based on the key
 * header data, and check the key length whether is valid
 *
 *   @param [in] khead              A pointer point to struction of kms_key_format_st
 *   @param [in] import_data_size   The size of import key data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_check_key_head_validity(const kms_key_format_st *khead, uint32_t import_data_size)
{
    uint32_t ext_key_size = 0;
    uint32_t plain_key_total_size = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_alg_type_e key_type = KMS_ALG_TYPE_NONE;

    if ((khead == NULL) || ((khead->part_info < KMS_KEY_PART_PUBKEY) || (khead->part_info > KMS_KEY_PART_PAIRKEY))) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (kms_check_alg_and_permit(khead->algo_id, khead->permit) != EHSM_ERR_SW_SUCCESS) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        key_type = g_key_size_arry[khead->algo_id].type;
        uint32_t pub_key_size = g_key_size_arry[khead->algo_id].pubkey_size;
        uint32_t priv_key_size = g_key_size_arry[khead->algo_id].privkey_size;

        if (KMS_ALG_TYPE_SYM == key_type) {
            uint32_t priv_key_size_equ_ret = (khead->priv_key_size != g_key_size_arry[khead->algo_id].privkey_size)
                ? EHSM_ERR_PARAM_ERROR
                : EHSM_ERR_SW_SUCCESS;
            uint32_t priv_key_size_than_ret
                = ((khead->priv_key_size == 0U)
                      || (khead->priv_key_size > g_key_size_arry[khead->algo_id].privkey_size))
                ? EHSM_ERR_PARAM_ERROR
                : EHSM_ERR_SW_SUCCESS;

            ret = (khead->part_info != KMS_KEY_PART_PRIVKEY) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = ((khead->algo_id == KMS_KEY_ALG_HMAC) || (khead->algo_id == KMS_KEY_ALG_RANDOM))
                    ? priv_key_size_than_ret
                    : priv_key_size_equ_ret;
            }
        }
        if (KMS_ALG_TYPE_SM9_USER_PRIV == key_type) {
            ret = EHSM_ERR_NOT_SUPPORT;
        }

        if ((KMS_ALG_TYPE_RSA_CRT == key_type) || (KMS_ALG_TYPE_RSA_COMM == key_type) || (KMS_ALG_TYPE_DH == key_type)
            || (KMS_ALG_TYPE_ECC == key_type)) {
            uint32_t pub_key_size_ret = 0;
            uint32_t priv_key_size_ret = 0;

            if (KMS_ALG_TYPE_DH == key_type) {
                priv_key_size_ret = ((khead->priv_key_size == 0U) || (khead->priv_key_size > priv_key_size))
                    ? EHSM_ERR_PARAM_ERROR
                    : EHSM_ERR_SW_SUCCESS;
                pub_key_size_ret = ((khead->pub_key_size == 0U) || (khead->pub_key_size > pub_key_size))
                    ? EHSM_ERR_PARAM_ERROR
                    : EHSM_ERR_SW_SUCCESS;
            } else {
                if ((KMS_ALG_TYPE_RSA_CRT == key_type) || (KMS_ALG_TYPE_RSA_COMM == key_type)) {
                    // rsa crt private key not contains rsa_n data
                    ext_key_size = ((KMS_ALG_TYPE_RSA_CRT == key_type) && (khead->part_info == KMS_KEY_PART_PRIVKEY))
                        ? 0U
                        : pub_key_size;
                    // check the size of rsa_e whether is multiple of 4
                    pub_key_size_ret = ((khead->pub_key_size % 4U) != 0U) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
                    if (EHSM_ERR_SW_SUCCESS == pub_key_size_ret) {
                        pub_key_size_ret
                            = ((khead->pub_key_size == 0U) || (khead->pub_key_size > RSA_KEY_E_SIZE_LIMITE))
                            ? EHSM_ERR_PARAM_ERROR
                            : EHSM_ERR_SW_SUCCESS;
                    }
                } else {
                    pub_key_size_ret = ((khead->pub_key_size == 0U) || (khead->pub_key_size != pub_key_size))
                        ? EHSM_ERR_PARAM_ERROR
                        : EHSM_ERR_SW_SUCCESS;
                }

                priv_key_size_ret = ((khead->priv_key_size == 0U) || (khead->priv_key_size != priv_key_size))
                    ? EHSM_ERR_PARAM_ERROR
                    : EHSM_ERR_SW_SUCCESS;
            }

            ret = ((khead->part_info & KMS_KEY_PART_PUBKEY) == KMS_KEY_PART_PUBKEY) ? pub_key_size_ret
                                                                                    : EHSM_ERR_SW_SUCCESS;
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = ((khead->part_info & KMS_KEY_PART_PRIVKEY) == KMS_KEY_PART_PRIVKEY) ? priv_key_size_ret
                                                                                          : EHSM_ERR_SW_SUCCESS;
            }
        }

        // ensure that the imported data contains complete key content
        uint32_t import_size_ret = 0;
        plain_key_total_size = KMS_KEY_HEAD_SIZE + khead->pub_key_size + ext_key_size + khead->priv_key_size;
        import_size_ret = (import_data_size < plain_key_total_size) ? EHSM_ERR_PARAM_ERROR : EHSM_ERR_SW_SUCCESS;
        ret = (ret == EHSM_ERR_SW_SUCCESS) ? import_size_ret : ret;
    }

    return ret;
}
/**
 *   @brief     Decrypt and verify the key data
 *
 *   @param [in] transport_key_handle    The key handle be used to decryption
 *   @param [in] authenticity_key_handle    The key handle be used to verification
 *   @param [in] key_data    The input cipher key data
 *   @param [in/out] key_size    Input cipher key data size, output the plain key data size
 *   @param [in] key_signature_data    Input the signature data
 *   @param [in] key_signature_size    Input the signature data size
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_decrypt_verify_key_data(uint32_t transport_key_handle, uint32_t authenticity_key_handle,
    uint8_t *key_data, uint32_t *key_size, uint8_t *key_signature_data, uint32_t *key_signature_size)
{
    uint32_t priv_koff = 0;
    uint32_t priv_enc_size = 0;
    kms_key_mem_size_st kmm_size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_key_format_st *khead = kms_get_key_format_pointer(key_data);

    if ((key_data == NULL) || (key_size == NULL) || (key_signature_data == NULL) || (key_signature_size == NULL)
        || (khead == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        /*Not need to decryption if decryption key invalid*/
        if (transport_key_handle == KMS_INVALID_KEY_ID) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            /*Parse the import key data, then decrypt the private part*/
            ret = kms_calcu_key_mem_size(khead, &kmm_size);
            if (ret == EHSM_ERR_SW_SUCCESS) {
                /*
                    Decryption the key private partion. For RSA key only encrypt rsa_d data

                    RSA private format  symm/ecc private format
                    ------------------- -----------------------
                        RSA_n                   private
                    ------------------- -----------------------
                        RSA_d
                    -------------------
                */
                if (khead->part_info == KMS_KEY_PART_PRIVKEY) {
                    priv_koff = kmm_size.ext_key_mem_size;
                    if (*key_size < (priv_koff + KMS_KEY_HEAD_SIZE)) {
                        ret = EHSM_ERR_PARAM_ERROR;
                    } else {
                        priv_enc_size = *key_size - priv_koff - KMS_KEY_HEAD_SIZE;
                        ret = kms_en_dec_key_data(transport_key_handle, &khead->raw_data[priv_koff], priv_enc_size,
                            &khead->raw_data[priv_koff], &priv_enc_size, SKE_CRYPTO_DECRYPT);
                    }
                }

                /*
                    Decryption the key private partion. For RSA key only encrypt rsa_d data

                    RSA key pair format  DH/ECC key pair format  symm key pair format
                    ------------------- ----------------------- -----------------------
                        RSA_e                   public               private
                    ------------------- ----------------------- -----------------------
                        RSA_n                   private
                    ------------------- -----------------------
                        RSA_d
                    -------------------
                */
                if (khead->part_info == KMS_KEY_PART_PAIRKEY) {
                    priv_koff = (uint32_t)kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size;
                    if (*key_size < (priv_koff + KMS_KEY_HEAD_SIZE)) {
                        ret = EHSM_ERR_PARAM_ERROR;
                    } else {
                        priv_enc_size = *key_size - priv_koff - KMS_KEY_HEAD_SIZE;
                        ret = kms_en_dec_key_data(transport_key_handle, &khead->raw_data[priv_koff], priv_enc_size,
                            &khead->raw_data[priv_koff], &priv_enc_size, SKE_CRYPTO_DECRYPT);
                    }
                }

                if (khead->part_info == KMS_KEY_PART_PUBKEY) {
                    /*Not need to decryption if imported key are public key*/
                    priv_koff = (uint32_t)kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size;
                    priv_enc_size = 0;
                }
            }

            if (ret == EHSM_ERR_SW_SUCCESS) {
                *key_size = KMS_KEY_HEAD_SIZE + priv_koff + priv_enc_size;
            }
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            /*Not need to verify the key signature if authenticity_key_handle key invalid*/
            ret = (authenticity_key_handle == KMS_INVALID_KEY_ID)
                ? EHSM_ERR_SW_SUCCESS
                : kms_sign_verify_key_data(authenticity_key_handle, key_data, *key_size, key_signature_data,
                      key_signature_size, SKE_VERIFY_MAC);
        }
    }

    return ret;
}
/**
 *   @brief     Sign and ecnrypt the key data
 *
 *   @param [in] transport_key_handle    The key handle be used to encryption
 *   @param [in] authenticity_key_handle    The key handle be used to signature
 *   @param [in] key_data    The input plain key data
 *   @param [in/out] key_size    Input plain key data size, output the encryption key data size
 *   @param [out] key_signature_data    Output the signature data
 *   @param [out] key_signature_size    Output the signature data size
 *   @param [in] req_key_part    Indicate which key partion will be encryption and signature
 *
 *   @return     uint32_t
 *
 *   @note Only encryption the key private partion
 */
static uint32_t kms_sign_encrypt_key_data(uint32_t transport_key_handle, uint32_t authenticity_key_handle,
    uint8_t *key_data, uint32_t *key_size, uint8_t *key_signature_data, uint32_t *key_signature_size,
    uint8_t req_key_part)
{
    uint16_t priv_koff = 0;
    uint16_t priv_ksize = 0;
    uint32_t priv_enc_size = 0;
    kms_key_mem_size_st kmm_size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    kms_key_format_st *khead = kms_get_key_format_pointer(key_data);

    if ((key_data == NULL) || (key_size == NULL) || (key_signature_data == NULL) || (key_signature_size == NULL)
        || (khead == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((khead->part_info & req_key_part) != req_key_part) {
        ret = EHSM_ERR_NOT_EXIST_KEY_PART;
    } else {
        ret = kms_calcu_key_mem_size(khead, &kmm_size);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        /*Build the export key data, then encrypt the private part*/
        /*
            Requst to export private key,
            1. Be exported key is complete key pair. need to move the key private part to public part, then
           encryption(for RSA key only encrypt rsa_d).
            2. Be exported key only exist private key pair, so encryption it dierctly

            RSA key pair format          RSA export private key           RSA export public key
            -------------------          -----------------------         -----------------------
                RSA_e                          RSA_n                            RSA_e
            -------------------  ------> ----------------------- ------> -----------------------
                RSA_n                          RSA_d(encrypted)                 RSA_n
            -------------------          -----------------------         -----------------------
                RSA_d
            -------------------
        */
        if (req_key_part == KMS_KEY_PART_PRIVKEY) {
            if (khead->part_info == KMS_KEY_PART_PAIRKEY) {
                uint32_t move_size = (uint32_t)kmm_size.ext_key_mem_size + kmm_size.priv_key_mem_size;
                util_memcpy(&khead->raw_data[0], &khead->raw_data[kmm_size.pub_key_mem_size], move_size);
            }

            khead->pub_key_size = 0;
            khead->part_info = req_key_part;
            kmm_size.pub_key_mem_size = 0;
            priv_koff = kmm_size.ext_key_mem_size;
            priv_ksize = kmm_size.priv_key_mem_size;
            priv_enc_size = priv_ksize;
        } else if (req_key_part == KMS_KEY_PART_PAIRKEY) {
            priv_koff = kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size;
            priv_ksize = kmm_size.priv_key_mem_size;
            priv_enc_size = priv_ksize;
        } else {
            khead->priv_key_size = 0;
            khead->part_info = req_key_part;
            kmm_size.priv_key_mem_size = 0;
        }

        uint32_t export_plain_key_size
            = KMS_KEY_HEAD_SIZE + kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size + kmm_size.priv_key_mem_size;
        /*Not need to generate the key signature if authenticity_key_handle key invalid*/
        ret = (authenticity_key_handle == KMS_INVALID_KEY_ID)
            ? EHSM_ERR_SW_SUCCESS
            : kms_sign_verify_key_data(authenticity_key_handle, key_data, export_plain_key_size, key_signature_data,
                  key_signature_size, SKE_GENERATE_MAC);

        /*Only encryption pirvate key*/
        if ((ret == EHSM_ERR_SW_SUCCESS) && (req_key_part != KMS_KEY_PART_PUBKEY)) {
            /*Need to decryption if decryption key invalid*/
            ret = (transport_key_handle == KMS_INVALID_KEY_ID)
                ? EHSM_ERR_SW_SUCCESS
                : kms_en_dec_key_data(transport_key_handle, &khead->raw_data[priv_koff], priv_ksize,
                      &khead->raw_data[priv_koff], &priv_enc_size, SKE_CRYPTO_ENCRYPT);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            uint32_t key_buff_size = *key_size;
            uint32_t export_k_total_size
                = KMS_KEY_HEAD_SIZE + kmm_size.pub_key_mem_size + kmm_size.ext_key_mem_size + priv_enc_size;
            *key_size = export_k_total_size;
            ret = (export_k_total_size > key_buff_size) ? EHSM_ERR_OUT_OF_MEM : EHSM_ERR_SW_SUCCESS;
        }
    }

    return ret;
}

/**
 *   @brief     Check the target key handle whether is write protection
 *
 *   @param [in] target_key_handle    The key handle be checked
 *
 *   @return     uint32_t
 *
 *   @note If key not exist in ehsm, will be passed the checking logic
 */
static uint32_t kms_check_key_write_protection_disable(uint32_t target_key_handle)
{
    uint32_t key_permit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (target_key_handle != KMS_INVALID_KEY_ID) {
        /*read target key permission data*/
        ret = kms_get_key_handle_permission(target_key_handle, &key_permit);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            /*Ensuer That the generate key does not have KEY_PERMIT_WR_PRT*/
            ret = ((key_permit & KEY_PERMIT_WR_PRT) == KEY_PERMIT_WR_PRT) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                          : EHSM_ERR_SW_SUCCESS;
        }
        /*key not exist so not need to check it permissions*/
        else if (ret == EHSM_ERR_NOT_EXIST_KEY) {
            ret = EHSM_ERR_SW_SUCCESS;
        } else {
            // nothing to do
        }
    } else {
        // nothing to do
    }

    return ret;
}

/*key import operation permissions check
1.Ensuer That the imported key does not have transfer protection permissions
2.Ensuer That the imported key does not have write protection permissions
3.Ensuer That the imported key have KEY_PERMIT_IMPORT_PLAINTEXT or KEY_PERMIT_IMPORT_CIPHERTEXT according to
corresponding decryption key handle 4.Ensuer That the decryption key have KEY_USAGE_TRANSPORT permissions (it permission
check in calling kms_en_dec_key_data)*/
static uint32_t kms_check_import_key_permission(
    uint32_t import_key_handle, uint32_t import_key_permit, uint32_t transport_key_handle)
{
    uint32_t key_permit;
    uint32_t chk_permit_bit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    /*1. Ensuer That the imported key does not have transfer protection permissions*/
    ret = ((import_key_permit & KEY_USAGE_TRANSPORT) != 0U) ? EHSM_ERR_NOT_ALLOWED_CREATION_KEY : EHSM_ERR_SW_SUCCESS;
    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (import_key_handle != KMS_INVALID_KEY_ID) {
            /*read target key permission data*/
            ret = kms_get_key_handle_permission(import_key_handle, &key_permit);
            if (ret == EHSM_ERR_SW_SUCCESS) {
                /*2.Ensuer That the imported key does not have write protection permissions*/
                ret = ((key_permit & KEY_PERMIT_WR_PRT) == KEY_PERMIT_WR_PRT) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                              : EHSM_ERR_SW_SUCCESS;
                if (ret == EHSM_ERR_SW_SUCCESS) {
                    /*2. Ensuer That the imported key have KEY_PERMIT_IMPORT_PLAINTEXT or KEY_PERMIT_IMPORT_CIPHERTEXT
                     * according to corresponding decryption key handle*/
                    chk_permit_bit = (transport_key_handle == KMS_INVALID_KEY_ID) ? KEY_PERMIT_IMPORT_PLAINTEXT
                                                                                  : KEY_PERMIT_IMPORT_CIPHERTEXT;
                    ret = ((key_permit & chk_permit_bit) != chk_permit_bit) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                            : EHSM_ERR_SW_SUCCESS;
                }
            }
            /*key not exist so not need to check it permissions*/
            else if (ret == EHSM_ERR_NOT_EXIST_KEY) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                // read key data error
            }
        }
    }

    return ret;
}
// key export operation permissions check
// 1. Ensure that the exported key is EHSM key type
// 2. Ensuer That the exported key does not have transfer protection permissions
// 3. Ensuer That the exported key have KEY_PERMIT_EXPORT_PLAINTEXT or KEY_PERMIT_EXPORT_CIPHERTEXT according to
// corresponding encryption key handle
static uint32_t kms_check_export_key_permission(uint32_t export_key_handle, uint32_t transport_key_handle)
{
    uint32_t key_permit;
    uint32_t chk_permit_bit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_type = export_key_handle & KMS_KEY_TYPE_MASK;

    /*read target key permission data*/
    ret = kms_get_key_handle_permission(export_key_handle, &key_permit);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        /*Ensure that the exported key type is EHSM key type */
        if (key_type == KMS_KEY_TYPE_EHSM) {
            /*1. Ensuer That the exported key does not have transfer protection permissions*/
            ret = ((key_permit & KEY_USAGE_TRANSPORT) != 0U) ? EHSM_ERR_MISMATCH_KEY_PERMISSION : EHSM_ERR_SW_SUCCESS;
            /*2. Ensuer That the exported key have KEY_PERMIT_EXPORT_PLAINTEXT or KEY_PERMIT_EXPORT_CIPHERTEXT according
             * to corresponding decryption key handle*/
            chk_permit_bit = (transport_key_handle == KMS_INVALID_KEY_ID) ? KEY_PERMIT_EXPORT_PLAINTEXT
                                                                          : KEY_PERMIT_EXPORT_CIPHERTEXT;
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = ((key_permit & chk_permit_bit) != chk_permit_bit) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                        : EHSM_ERR_SW_SUCCESS;
            }
        } else {
            ret = EHSM_ERR_NOT_SUPPORT;
        }
    }

    return ret;
}
// key derive operation permissions check
// 1.check key write protection is disable
// 2.IF derive root input for KEY_HANDLE
//     3.Ensuer That the root key has KEY_USAGE_KEYCREATION permissions
//     4.IF the root key have KEY_USAGE_CREATION_TRANSP_KEY permissions
//         5.1 Ensuer that the derived new key has KEY_USAGE_TRANSPORT permissions
//         5.2 Ensuer that the derived new key does not have non KEY_USAGE_TRANSPORT permissions
//         5.2 Ensuer that the root key is an OTP key
//     5.IF the root key does not have KEY_USAGE_CREATION_TRANSP_KEY
//         5.1 Ensuer that the derived new key does not have KEY_USAGE_TRANSPORT permissions
// 3.IF derive root input for USER
//     3.1 Ensuer that the derived new key does not have KEY_USAGE_TRANSPORT permissions
static uint32_t kms_check_derive_key_permission(
    uint32_t target_key_handle, uint32_t derive_key_permit, uint32_t parent_key_handle, uint32_t derive_type)
{
    uint32_t root_key_permit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    ret = kms_check_key_write_protection_disable(target_key_handle);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (derive_type == KMS_KEY_DERIVE_USER_KEYHANDLE) {
            if (parent_key_handle == KMS_INVALID_KEY_ID) {
                ret = EHSM_ERR_PARAM_ERROR;
            } else {
                ret = kms_get_key_handle_permission(parent_key_handle, &root_key_permit);
            }

            if (ret == EHSM_ERR_SW_SUCCESS) {
                ret = ((root_key_permit & KEY_USAGE_KEYCREATION) == KEY_USAGE_KEYCREATION)
                    ? EHSM_ERR_SW_SUCCESS
                    : EHSM_ERR_MISMATCH_KEY_PERMISSION;
            }

            if (ret == EHSM_ERR_SW_SUCCESS) {
                if ((root_key_permit & KEY_USAGE_CREATION_TRANSP_KEY) == KEY_USAGE_CREATION_TRANSP_KEY) {
                    ret = ((derive_key_permit & KEY_USAGE_TRANSPORT) == KEY_USAGE_TRANSPORT)
                        ? EHSM_ERR_SW_SUCCESS
                        : EHSM_ERR_MISMATCH_KEY_PERMISSION;
                    if (EHSM_ERR_SW_SUCCESS == ret) {
                        ret = ((derive_key_permit & (~KEY_USAGE_TRANSPORT)) == 0U) ? EHSM_ERR_SW_SUCCESS
                                                                                   : EHSM_ERR_MISMATCH_KEY_PERMISSION;
                    }
                } else {
                    ret = ((derive_key_permit & KEY_USAGE_TRANSPORT) == 0U) ? EHSM_ERR_SW_SUCCESS
                                                                            : EHSM_ERR_MISMATCH_KEY_PERMISSION;
                }
            }

            if ((ret == EHSM_ERR_SW_SUCCESS)
                && ((root_key_permit & KEY_USAGE_CREATION_TRANSP_KEY) == KEY_USAGE_CREATION_TRANSP_KEY)) {
                ret = ((parent_key_handle & KMS_KEY_TYPE_MASK) == KMS_KEY_TYPE_OTP) ? EHSM_ERR_SW_SUCCESS
                                                                                    : EHSM_ERR_NOT_ALLOWED_CREATION_KEY;
            }
        } else {
            // Not allow create transport key if root password input for user
            ret = ((derive_key_permit & KEY_USAGE_TRANSPORT) == 0U) ? EHSM_ERR_SW_SUCCESS
                                                                    : EHSM_ERR_NOT_ALLOWED_CREATION_KEY;
        }
    }

    return ret;
}
/**
 *   @brief     Check key exchange operation target key handle whether is exist, write protection, and permission are
 * allowed
 *
 *   @param [in] target_key_handle    The key handle of exchanged production new key
 *   @param [in] input_permit         The permission value of exchanged production new key
 *   @param [in] local_key_handle     The local prvaite key handle
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_check_exchange_key_permission(
    uint32_t target_key_handle, uint32_t input_permit, uint32_t local_key_handle)
{
    uint32_t local_key_permit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    /*check target key not write protect permission*/
    ret = kms_check_key_write_protection_disable(target_key_handle);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        /*1.Ensure that the exchagne new key does not have KEY_USAGE_TRANSPORT permission */
        ret = ((input_permit & KEY_USAGE_TRANSPORT) != 0U) ? EHSM_ERR_MISMATCH_KEY_PERMISSION : EHSM_ERR_SW_SUCCESS;
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_get_key_handle_permission(local_key_handle, &local_key_permit);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            /*Check key exchange operation permission
            2.Ensure that the local key has KEY_USAGE_KEYCREATION permission
            3.Ensure that the local key does not have KEY_USAGE_CREATION_TRANSP_KEY permission
            */
            ret = ((local_key_permit & KEY_USAGE_KEYCREATION) == 0U) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                     : EHSM_ERR_SW_SUCCESS;
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = ((local_key_permit & KEY_USAGE_CREATION_TRANSP_KEY) == KEY_USAGE_CREATION_TRANSP_KEY)
                    ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                    : EHSM_ERR_SW_SUCCESS;
            }
        }
    }

    return ret;
}
/**
 *   @brief     generation key function use to generate symmetric or asymmetric key
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_generate_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t key_size = 0;
    uint32_t key_handle;
    kms_keypair_st keypair;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_gen_key_st *cmd_data = NULL;
    uint8_t key_buff[KMS_KEY_HEAD_SIZE + KMS_KEY_DATA_MAX_SIZE];
    kms_key_format_st *khead = kms_get_key_format_pointer(key_buff);

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data) || (NULL == khead)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_gen_key_st *)(req_data);
        util_memset(key_buff, 0x00, sizeof(key_buff));
        ret = kms_check_genkey_param(cmd_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            /*check target key not write protect permission*/
            ret = kms_check_key_write_protection_disable(cmd_data->key_handle);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            /*Ensure that the new key created does not have transfer protection and creation transport key permissions
             */
            ret = ((cmd_data->permit & KEY_USAGE_TRANSPORT) != 0U) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                   : EHSM_ERR_SW_SUCCESS;
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = ((cmd_data->permit & KEY_USAGE_CREATION_TRANSP_KEY) != 0U) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                                 : EHSM_ERR_SW_SUCCESS;
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = kms_fill_key_head(cmd_data, khead, &key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = kms_alloc_key_mem(khead->raw_data, khead, khead->part_info, &keypair);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = kms_gen_key_data(cmd_data, &keypair);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            key_handle = cmd_data->key_handle;
            ret = kds_write_key(key_buff, (uint16_t)(KMS_KEY_HEAD_SIZE + key_size), &key_handle);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 4;
            rsp_data->data[0] = key_handle;
        }
        util_memset(key_buff, 0x00, sizeof(key_buff));
    }

    return ret;
}

/**
 *   @brief     Import key data from host to ehsm
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_import_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t key_handle;
    uint32_t plain_key_size;
    uint32_t key_signature_size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_import_key_st *cmd_data = NULL;
    uint8_t host_key_signature[KMS_KEY_DATA_SIGNATURE_SIZE];
    uint8_t host_key_data[KMS_KEY_HEAD_SIZE + KMS_KEY_DATA_MAX_SIZE + KMS_KEY_ENCRYPTION_BLOCK_MAX_SIZE];
    const kms_key_format_st *khead = kms_get_key_format_pointer(host_key_data);

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data) || (NULL == khead)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_import_key_st *)(req_data);

        ret = (cmd_data->key_data_size > sizeof(host_key_data))
            ? EHSM_ERR_PARAM_ERROR
            : mmap_read_remote_data(host_key_data, cmd_data->key_data_addr, cmd_data->key_data_size);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (cmd_data->authenticity_key_handle != KMS_INVALID_KEY_ID) {
                ret = (cmd_data->key_signature_size > sizeof(host_key_signature))
                    ? EHSM_ERR_PARAM_ERROR
                    : mmap_read_remote_data(
                          host_key_signature, cmd_data->key_signature_addr, cmd_data->key_signature_size);
            }
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_check_key_head_validity(khead, cmd_data->key_data_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_check_import_key_permission(cmd_data->key_handle, khead->permit, cmd_data->transport_key_handle);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            plain_key_size = cmd_data->key_data_size;
            key_signature_size = cmd_data->key_signature_size;
            ret = kms_decrypt_verify_key_data(cmd_data->transport_key_handle, cmd_data->authenticity_key_handle,
                host_key_data, &plain_key_size, host_key_signature, &key_signature_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            key_handle = cmd_data->key_handle;
            ret = kds_write_key(host_key_data, (uint16_t)plain_key_size, &key_handle);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 4;
            rsp_data->data[0] = key_handle;
        }

        util_memset(host_key_data, 0x00, sizeof(host_key_data));
    }

    return ret;
}

/**
 *   @brief     Export key data from ehsm to host, allowing partial data of a key to be exported, including public key,
 * private key, and key pairs.
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_export_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t key_size = 0;
    uint32_t signature_size = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_export_key_st *cmd_data = NULL;
    uint8_t local_key_signature[KMS_KEY_DATA_SIGNATURE_SIZE];
    uint8_t local_key_data[KMS_KEY_HEAD_SIZE + KMS_KEY_DATA_MAX_SIZE + KMS_KEY_ENCRYPTION_BLOCK_MAX_SIZE];
    const kms_key_format_st *khead = kms_get_key_format_pointer(local_key_data);

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data) || (NULL == khead)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_export_key_st *)(req_data);
        util_memset(local_key_data, 0x00, sizeof(local_key_data));

        ret = kms_check_export_key_permission(cmd_data->key_handle, cmd_data->transport_key_handle);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kds_read_entire_key(
                cmd_data->key_handle, local_key_data, sizeof(local_key_data) - KMS_KEY_ENCRYPTION_BLOCK_MAX_SIZE);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            key_size = sizeof(local_key_data);
            signature_size = cmd_data->key_signature_size;
            /*3.Ensuer That the encryption and verification key have KEY_USAGE_TRANSPORT permissions (it permission
             * check in calling kms_en_dec_key_data)*/
            ret = kms_sign_encrypt_key_data(cmd_data->transport_key_handle, cmd_data->authenticity_key_handle,
                local_key_data, &key_size, local_key_signature, &signature_size, cmd_data->req_key_part);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = (key_size > cmd_data->key_data_size)
                ? EHSM_ERR_OUT_OF_MEM
                : mmap_write_remote_data(cmd_data->key_data_addr, local_key_data, key_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (cmd_data->authenticity_key_handle != KMS_INVALID_KEY_ID) {
                ret = (signature_size > cmd_data->key_signature_size)
                    ? EHSM_ERR_OUT_OF_MEM
                    : mmap_write_remote_data(cmd_data->key_signature_addr, local_key_signature, signature_size);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 8;
            rsp_data->data[0] = key_size;
            rsp_data->data[1] = signature_size;
        }
        util_memset(local_key_data, 0x00, sizeof(local_key_data));
    }

    return ret;
}

/**
 *   @brief     remove key
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note The OTP key can not be removed.
 */
static uint32_t kms_remove_key(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t key_permit;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_remove_key_st *cmd_data;

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_remove_key_st *)(req_data);

        /*read target key permission data*/
        ret = kms_get_key_handle_permission(cmd_data->key_handle, &key_permit);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            /*check target key have remove permission*/
            ret = ((key_permit & KEY_PERMIT_REMOVE) != KEY_PERMIT_REMOVE) ? EHSM_ERR_MISMATCH_KEY_PERMISSION
                                                                          : EHSM_ERR_SW_SUCCESS;
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kds_remove_key(cmd_data->key_handle);
        }
    }

    return ret;
}

/**
 *   @brief    key derivation function
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note Only key_out_dir=0x01 (output to KMS) is supported.
 */
static uint32_t kms_derive_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t key_handle;
    kms_keydata_st parent_keydata;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_key_derive_st *cmd_data = NULL;
    uint8_t gen_key_data[KMS_KEY_HEAD_SIZE + KMS_SYM_KEY_DATA_MAX_SIZE];
    uint8_t parent_key_data[KMS_KEY_HEAD_SIZE + KMS_SYM_KEY_DATA_MAX_SIZE];
    kms_key_format_st *gen_key = kms_get_key_format_pointer(gen_key_data);
    kms_key_format_st *parent_key = kms_get_key_format_pointer(parent_key_data);

    util_memset(gen_key_data, 0x00, sizeof(gen_key_data));

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data) || (NULL == gen_key) || (NULL == parent_key)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_key_derive_st *)(req_data);

        ret = kms_check_derive_key_params(cmd_data);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            // Ensure that key derivation permission allow
            ret = kms_check_derive_key_permission(
                cmd_data->key_handle, cmd_data->permit, cmd_data->parent_key_handle, cmd_data->derive_type);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (cmd_data->derive_type == KMS_KEY_DERIVE_USER_KEYHANDLE) {
                // Note that the parameter of check_usage_bits has been checked by kms_check_derive_key_permission so no
                // need check again
                ret = kms_read_key(cmd_data->parent_key_handle, parent_key_data, sizeof(parent_key_data),
                    &parent_keydata, KMS_KEY_PART_PRIVKEY, KEY_USAGE_NONE);
            } else {
                parent_keydata.algo_id = KMS_KEY_ALG_RANDOM;
                parent_keydata.keypart = KMS_KEY_PART_PRIVKEY;
                parent_keydata.src_type = KMS_KEY_SRC_TYPE_RAM;
                parent_keydata.keypair.symm.key = parent_key->raw_data;
                parent_keydata.keypair.symm.size = (uint16_t)cmd_data->pw_data_size;
                // The key derivation root password input for user
                ret = mmap_read_remote_data(parent_key->raw_data, cmd_data->pw_data_addr, cmd_data->pw_data_size);
            }
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_gen_derived_key(cmd_data, &parent_keydata, gen_key->raw_data, (uint16_t)cmd_data->key_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            gen_key->permit = cmd_data->permit;
            gen_key->algo_id = cmd_data->algo_id;
            gen_key->part_info = KMS_KEY_PART_PRIVKEY;
            gen_key->priv_key_size = (uint16_t)cmd_data->key_size;

            key_handle = cmd_data->key_handle;
            ret = kds_write_key(gen_key_data, (uint16_t)(KMS_KEY_HEAD_SIZE + cmd_data->key_size), &key_handle);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 4;
            rsp_data->data[0] = key_handle;
        }
    }

    util_memset(gen_key_data, 0x00, sizeof(gen_key_data));
    util_memset(parent_key_data, 0x00, sizeof(parent_key_data));

    return ret;
}

/**
 *   @brief     Key exchange (Diffie-Hellman or ECDH protocol) to obtain a common secret key between local and remote
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_exchange_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t key_handle;
    kms_keydata_st local_keydata;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_key_exchange_st *cmd_data = NULL;
    uint8_t remote_pubkey_data[KMS_DH_PUBKEY_MAX_SIZE];
    uint8_t gen_key_data[KMS_KEY_HEAD_SIZE + KMS_SYM_KEY_DATA_MAX_SIZE];
    uint8_t local_key_data[KMS_KEY_HEAD_SIZE + KMS_DH_PRIVKEY_MAX_SIZE];
    kms_key_format_st *gen_key = kms_get_key_format_pointer(gen_key_data);

    util_memset(gen_key_data, 0x00, sizeof(gen_key_data));

    if (kms_check_operation_permission() != EHSM_ERR_SW_SUCCESS) {
        ret = EHSM_ERR_LIMIT_OF_AUTHORITY;
    } else if ((NULL == req_data) || (NULL == rsp_data) || (NULL == gen_key)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_key_exchange_st *)(req_data);
        ret = kms_check_exchange_key_params(cmd_data);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_check_exchange_key_permission(cmd_data->key_handle, cmd_data->permit, cmd_data->local_key_handle);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = mmap_read_remote_data(remote_pubkey_data, cmd_data->remote_pubkey_data, cmd_data->remote_pubkey_size);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            // Note that the parameter of check_usage_bits has been check by ksm_check_derive_key_permission so no need
            // check again
            ret = kms_read_key(cmd_data->local_key_handle, local_key_data, sizeof(local_key_data), &local_keydata,
                KMS_KEY_PART_PRIVKEY, KEY_USAGE_NONE);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = kms_gen_exchange_key(cmd_data, &local_keydata, remote_pubkey_data, cmd_data->remote_pubkey_size,
                gen_key->raw_data, cmd_data->key_size);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            gen_key->permit = cmd_data->permit;
            gen_key->algo_id = cmd_data->algo_id;
            gen_key->part_info = KMS_KEY_PART_PRIVKEY;
            gen_key->priv_key_size = (uint16_t)cmd_data->key_size;

            key_handle = cmd_data->key_handle;
            ret = kds_write_key(gen_key_data, (uint16_t)(KMS_KEY_HEAD_SIZE + cmd_data->key_size), &key_handle);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 4;
            rsp_data->data[0] = key_handle;
        }
    }

    util_memset(gen_key_data, 0x00, sizeof(gen_key_data));
    util_memset(local_key_data, 0x00, sizeof(local_key_data));
    util_memset(remote_pubkey_data, 0x00, sizeof(remote_pubkey_data));

    return ret;
}

/**
 *   @brief      Calculate the public key data through private key
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t kms_calculate_key(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    kms_keydata_st kdata;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_ehsm_get_pub_from_priv_st *cmd_data = NULL;
    uint8_t key_data[KMS_KEY_HEAD_SIZE + KMS_RSA_DH_KEYPAIR_MAX_SIZE];
    uint32_t pub_size;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_ehsm_get_pub_from_priv_st *)(req_data);
        if ((cmd_data->public_key_addr == KMS_INVALID_DATA_ADDR) || (cmd_data->key_handle == KMS_INVALID_KEY_ID)) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = kms_read_key(
                cmd_data->key_handle, key_data, sizeof(key_data), &kdata, KMS_KEY_PART_PRIVKEY, KEY_USAGE_NONE);
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (kdata.algo_id >= KMS_KEY_ALG_END) {
                ret = EHSM_ERR_INVALID_ALGORITHM;
            } else {
                if (g_key_size_arry[kdata.algo_id].type == KMS_ALG_TYPE_SYM) {
                    ret = EHSM_ERR_NOT_SUPPORT;
                } else {
                    ret = kms_calcu_public_key_from_priv(
                        cmd_data, &kdata.keypair, kdata.algo_id, cmd_data->public_key_buffer_size);
                }
            }
        }

        if (ret == EHSM_ERR_SW_SUCCESS) {
            if (kdata.algo_id == KMS_KEY_ALG_RSA_DH) {
                pub_size = (uint32_t)kdata.keypair.dh.pub_size;
                ret = mmap_write_remote_data(cmd_data->public_key_addr, kdata.keypair.dh.pub_key, pub_size);
            } else {
                pub_size = (uint32_t)kdata.keypair.ecc.pub_size;
                ret = mmap_write_remote_data(cmd_data->public_key_addr, kdata.keypair.ecc.pub_key, pub_size);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            rsp_data->rsp_data_len = 8;
            rsp_data->data[0] = kdata.algo_id;
            rsp_data->data[1] = pub_size;
        }
    }

    util_memset(key_data, 0x00, sizeof(key_data));

    return ret;
}


/**
 *   @brief      Check key exist usage flags whether match the input paramters chk_usage_bits.
 *               for both specific flags KEY_PERMIT_DEBUG_USAGE and KEY_PERMIT_BOOT_FAIL_USAGE will check with current
 * system status.
 *
 *   @param [in]  key_handle The handle of key
 *   @param [out] key_buff   the buffer use to store key data
 *   @param [in]  buff_size  the size of buffer
 *   @param [out] keydata    a pointer point to struct of kms_keydata_st use to store key information
 *   @param [out] req_kpart  which key part to be read
 *
 *   @return     uint32_t
 *
 *   @note Please ensure that the buffer size is greater than or equal to the key data
 */
static uint32_t kms_check_key_usage(uint32_t key_usage_bits, uint32_t chk_usage_bits)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if ((key_usage_bits & chk_usage_bits) != chk_usage_bits) {
            ret = EHSM_ERR_MISMATCH_KEY_PERMISSION;
        }
    }

    return ret;
}

/**
 *   @brief      Convert the key management service algorithm id to ske algorithm
 *
 *   @param [algo_id] key management service algorithm id
 *   @param [ske_alg] The ske algorithm
 *
 *   @return     uint32_t
 *
 *   @note
 */

static uint32_t kms_get_ske_alg(uint8_t algo_id, cpt_ske_alg_e *ske_alg)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (ske_alg == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (algo_id) {
        case KMS_KEY_ALG_DES: {
            *ske_alg = SKE_ALG_DES;
            break;
        }

        case KMS_KEY_ALG_TDES_128: {
            *ske_alg = SKE_ALG_TDES_128;
            break;
        }

        case KMS_KEY_ALG_TDES_192: {
            *ske_alg = SKE_ALG_TDES_192;
            break;
        }

        case KMS_KEY_ALG_AES_128:
        case KMS_KEY_ALG_AES_128_XTS: {
            *ske_alg = SKE_ALG_AES_128;
            break;
        }

        case KMS_KEY_ALG_AES_192:
        case KMS_KEY_ALG_AES_192_XTS: {
            *ske_alg = SKE_ALG_AES_192;
            break;
        }

        case KMS_KEY_ALG_AES_256:
        case KMS_KEY_ALG_AES_256_XTS: {
            *ske_alg = SKE_ALG_AES_256;
            break;
        }

        case KMS_KEY_ALG_SM4:
        case KMS_KEY_ALG_SM4_XTS: {
            *ske_alg = SKE_ALG_SM4;
            break;
        }

        default: {
            ret = EHSM_ERR_INVALID_ALGORITHM;
            break;
        }
        }
    }

    return ret;
}

static uint32_t kms_read_dh_field_sz(
    const uint8_t *addr, uint32_t offset, uint32_t buf_size, uint32_t max_field_sz, uint32_t *field_sz)
{
    uint32_t ret;

    if (offset + 4U > buf_size) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memcpy(field_sz, &addr[offset], 4);
        if (*field_sz > max_field_sz) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }

    return ret;
}

static uint32_t kms_read_and_check_dh_sz_param(const uint8_t *remap_addr, uint32_t dh_common_size, uint32_t *dh_p_sz,
    uint32_t *dh_q_sz, uint32_t *dh_g_sz, uint32_t *dh_h_sz)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((remap_addr == NULL) || (dh_p_sz == NULL) || (dh_q_sz == NULL) || (dh_g_sz == NULL) || (dh_h_sz == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kms_read_dh_field_sz(remap_addr, 0, dh_common_size, KMS_RSA_DH_P_PARAM_MAX_SIZE, dh_p_sz);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kms_read_dh_field_sz(remap_addr, 4U + *dh_p_sz, dh_common_size, KMS_RSA_DH_Q_PARAM_MAX_SIZE, dh_q_sz);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kms_read_dh_field_sz(
            remap_addr, 8U + *dh_p_sz + *dh_q_sz, dh_common_size, KMS_RSA_DH_G_PARAM_MAX_SIZE, dh_g_sz);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kms_read_dh_field_sz(
            remap_addr, 12U + *dh_p_sz + *dh_q_sz + *dh_g_sz, dh_common_size, KMS_RSA_DH_H_PARAM_MAX_SIZE, dh_h_sz);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (16U + *dh_p_sz + *dh_q_sz + *dh_g_sz + *dh_h_sz > dh_common_size) {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}
/**
 *   @brief      Read the dh command data and parse out dh p/q/g/h data
 *
 *   @param [in] dh_common_addr The host address of dh commom data
 *   @param [in] dh_common_size The size of dh common data
 *   @param [out] dh_common_buff The buffer of storage the dh common data
 *   @param [in] dh_common_buff_size  The size of storage buffer
 *   @param [out] dh_para A point pointer to structure of DH_PARA
 *
 *   @return     uint32_t
 *
 *   @note the size of dh h params can be set to zero it means that the data of dh h param not exist
 */
static uint32_t kms_parse_dh_common_data(raddr_t dh_common_addr, uint32_t dh_common_size, uint32_t *dh_common_buff,
    uint32_t dh_common_buff_size, cpt_dh_para_st *dh_para)
{
    uint32_t level;
    uint32_t dh_p_sz = 0;
    uint32_t dh_q_sz = 0;
    uint32_t dh_g_sz = 0;
    uint32_t dh_h_sz = 0;
    const uint8_t *remap_addr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    level = cpu_enter_critical();

    if ((dh_common_buff == NULL) || (dh_para == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (dh_common_size > dh_common_buff_size) {
        ret = EHSM_ERR_OUT_OF_MEM;
    } else {
        remap_addr = mmap_remap_addr_u64(dh_common_addr);
        ret = (NULL == remap_addr) ? EHSM_ERR_REMAP_FAILED : EHSM_ERR_SW_SUCCESS;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = kms_read_and_check_dh_sz_param(remap_addr, dh_common_size, &dh_p_sz, &dh_q_sz, &dh_g_sz, &dh_h_sz);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            dh_para->p_bits = dh_p_sz << 3;
            dh_para->q_bits = dh_q_sz << 3;
            dh_para->g_bits = dh_g_sz << 3;

            dh_para->p = &dh_common_buff[0];
            dh_para->q = &dh_common_buff[(KMS_RSA_DH_P_PARAM_MAX_SIZE / sizeof(uint32_t))];
            dh_para->g = &dh_common_buff[(KMS_RSA_DH_P_PARAM_MAX_SIZE / sizeof(uint32_t)) * 2U];
            dh_para->p_h = NULL;

            reverse_byte_array((const uint8_t *)&remap_addr[4U], (uint8_t *)dh_para->p, dh_p_sz);
            reverse_byte_array((const uint8_t *)&remap_addr[8U + dh_p_sz], (uint8_t *)dh_para->q, dh_q_sz);
            reverse_byte_array((const uint8_t *)&remap_addr[12U + dh_p_sz + dh_q_sz], (uint8_t *)dh_para->g, dh_g_sz);
            if (dh_h_sz != 0U) {
                dh_para->p_h = &dh_common_buff[(KMS_RSA_DH_P_PARAM_MAX_SIZE / sizeof(uint32_t)) * 3U];
                reverse_byte_array(
                    (const uint8_t *)&remap_addr[16U + dh_p_sz + dh_q_sz + dh_g_sz], (uint8_t *)dh_para->p_h, dh_h_sz);
            }

            // p can not be even.
            if (0u == (dh_para->p[0] & 1u)) {
                ret = EHSM_ERR_DH_PARAM_OF_PQ_IS_EVEN;
            } else {
                if (0u != (dh_p_sz & 3u)) {
                    util_memset(&((uint8_t *)dh_para->p)[dh_p_sz], 0, 4u - (dh_p_sz & 3u));
                }

                if (0u != (dh_q_sz & 3u)) {
                    util_memset(&((uint8_t *)dh_para->q)[dh_q_sz], 0, 4u - (dh_q_sz & 3u));
                }

                if (0u != (dh_g_sz & 3u)) {
                    util_memset(&((uint8_t *)dh_para->g)[dh_g_sz], 0, 4u - (dh_g_sz & 3u));
                }
            }
        }
    }

    cpu_exit_critical(level);

    return ret;
}

/**
 *   @brief      Expend the public and private key data of DH to meet the requirements of the crypto libray.
 *
 *   @param [in/out] priv_key_data The dh private key data
 *   @param [in] priv_key_size The size of dh private key data
 *   @param [in/out] pub_key_data The dh public key data
 *   @param [in] pub_key_size  The size of dh public key data
 *   @param [in] dh_p_len The size of dh p data
 *   @param [in] dh_q_len The size of dh q data
 *
 *   @return     uint32_t
 *
 *   @note The actual length of DH private and public keys is 512 bytes, and space has been pre allocated.
 */
static uint32_t kms_expend_dh_keypair(uint8_t *priv_key_data, uint32_t priv_key_size, uint8_t *pub_key_data,
    uint32_t pub_key_size, uint32_t dh_p_len, uint32_t dh_q_len)
{
    uint8_t *src = NULL;
    uint8_t *dst = NULL;
    uint32_t diff_size = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((priv_key_data == NULL) || (pub_key_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((dh_p_len > KMS_DH_PRIVKEY_MAX_SIZE) || (dh_q_len > KMS_DH_PUBKEY_MAX_SIZE)
        || (priv_key_size > KMS_DH_PRIVKEY_MAX_SIZE) || (pub_key_size > KMS_DH_PUBKEY_MAX_SIZE)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (dh_q_len > priv_key_size) {
            diff_size = dh_q_len - priv_key_size;
            src = priv_key_data;
            dst = &priv_key_data[diff_size];
            (void)kds_util_memmove(dst, src, priv_key_size);
            util_memset(src, 0x00, diff_size);
        }

        if (dh_p_len > pub_key_size) {
            diff_size = dh_p_len - pub_key_size;
            src = pub_key_data;
            dst = &pub_key_data[diff_size];
            (void)kds_util_memmove(dst, src, pub_key_size);
            util_memset(src, 0x00, diff_size);
        }
    }

    return ret;
}

static uint32_t kms_gen_exchange_dh_key(const mb_cmd_ehsm_key_exchange_st *cmd_data, const kms_keydata_st *local_kdata,
    uint8_t *remote_pubkey, uint32_t remote_pubkey_size, uint8_t *create_key, uint32_t create_key_size)
{
    DH_PARA dh_para;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t dh_common_data[KMS_DH_COMMON_DATA_MAX_SIZE >> 2];

    if ((cmd_data == NULL) || (local_kdata == NULL) || (remote_pubkey == NULL) || (create_key == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (local_kdata->algo_id != KMS_KEY_ALG_RSA_DH) {
        ret = EHSM_ERR_INVALID_ALGORITHM;
    } else {
        ret = kms_parse_dh_common_data(
            cmd_data->dh_common_addr, cmd_data->dh_common_size, dh_common_data, KMS_DH_COMMON_DATA_MAX_SIZE, &dh_para);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (create_key_size > (dh_para.p_bits >> 3)) {
            ret = EHSM_ERR_DH_KEY_TOO_LEN;
        } else {
            ret = kms_expend_dh_keypair(local_kdata->keypair.dh.priv_key, local_kdata->keypair.dh.priv_size,
                remote_pubkey, remote_pubkey_size, dh_para.p_bits >> 3, dh_para.q_bits >> 3);
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = cpt_dh_compute_key(&dh_para, local_kdata->keypair.dh.priv_key, remote_pubkey, create_key);
    }

    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      read key data to user buffer
 *
 *   @param [in] key_handle The handle of key
 *   @param [out] key_buff  the buffer use to store key data
 *   @param [in] buff_size  the size of buffer
 *   @param [out] keydata   a pointer point to struct of kms_keydata_st use to store key information
 *   @param [out] req_kpart which key part to be read
 *   @param [in] check_usage_bits key permissions that need to be chekced
 *
 *   @return     uint32_t
 *
 *   @note Please ensure that the buffer size is greater than or equal to the key data
 */
uint32_t kms_read_key(uint32_t key_handle, uint8_t *key_buff, uint32_t buff_size, kms_keydata_st *keydata,
    uint8_t req_kpart, uint32_t check_usage_bits)
{
    uint32_t key_exist_usage;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_type = key_handle & KMS_KEY_TYPE_MASK;

    if ((key_buff == NULL) || (keydata == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((check_usage_bits & KEY_USAGE_PERMIT_ALL_BITS) != check_usage_bits) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((key_type != KMS_KEY_TYPE_OTP) && (key_type != KMS_KEY_TYPE_SHE) && (key_type != KMS_KEY_TYPE_EHSM)) {
        ret = EHSM_ERR_INVALID_HANDLE;
    } else {
        ret = kms_get_key_handle_permission(key_handle, &key_exist_usage);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = kms_check_key_usage(key_exist_usage, check_usage_bits);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        if (key_type == KMS_KEY_TYPE_OTP) {
            ret = kms_read_otp_key(key_handle, key_buff, buff_size, keydata, req_kpart);
        } else if (key_type == KMS_KEY_TYPE_EHSM) {
            ret = kms_read_memory_key(key_handle, key_buff, buff_size, keydata, req_kpart);
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}

/**
 *   @brief      Generater dh key pair with private key size
 *
 *   @param [in] dh_para The buffer pointer point to DH parmater.
 *   @param [out] prikey  The buffer of private key.
 *   @param [out] pubkey  The buffer of public key.
 *   @param [in] prikey_size  The size of private key.
 *
 *   @return     uint32_t
 *
 *   @note If the prikey_size is zero, the generate private key length is same with dh q byte length.
 */
static uint32_t kms_dh_generate_key(
    const cpt_dh_para_st *dh_para, uint8_t *prikey, uint8_t *pubkey, uint32_t prikey_size)
{
    uint32_t qByteLen;
    uint32_t tmpBitLen;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == dh_para) || (NULL == prikey) || (NULL == pubkey)) {
        ret = DH_POINTER_NULL;
    } else {
        if (prikey_size == 0U) {
            qByteLen = GET_BYTE_LEN(dh_para->q_bits);
        } else {
            qByteLen = prikey_size;
        }

        do {
            ret = cpt_get_rand((uint8_t *)prikey, qByteLen);
            if (ret != EHSM_ERR_SW_SUCCESS) {
                break;
            }

            // make sure prikey <= q
            tmpBitLen = (dh_para->q_bits) & 7u;
            if (0u != tmpBitLen) {
                prikey[0u] &= (uint8_t)((1u << (tmpBitLen)) - 1u);
            }

            ret = cpt_dh_generate_pubkey_from_prikey(dh_para, prikey, pubkey);

            // prikey should be in [2, q-2]
        } while (
            (EHSM_ERR_DH_ZERO_ALL == ret) || (EHSM_ERR_DH_VALUE_ONE == ret) || (EHSM_ERR_DH_INTEGER_TOO_BIG == ret));
    }

    return ret;
}

/**
 *   @brief      key management service processing function entry
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t kms_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    uint32_t ret;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (req_data->cmd_id) {
        case MB_CMD_ID_EHSM_GEN_KEY:
            ret = kms_generate_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_IMPORT_KEY:
            ret = kms_import_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_EXPORT_KEY:
            ret = kms_export_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_KEY_DERIVE:
            ret = kms_derive_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_KEY_EXCHANGE:
            ret = kms_exchange_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_GET_PUB_FROM_PRIV:
            ret = kms_calculate_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_EHSM_REMOVE_KEY:
            ret = kms_remove_key(req_data, rsp_data);
            break;
        default:
            ret = EHSM_ERR_INVALID_CMD;
            break;
        }
    }

    return ret;
}
