/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "config.h"
#include "fwup_srv.h"
#include "types.h"
#include "mb.h"
#include "mmap.h"
#include "driver/sysreg.h"
#include "driver/otp/otp_driver.h"
#include "component/util.h"
#include "component/otp_key.h"
#include "component/otp_map.h"
#include "service/socvrfy_srv.h"
#include "component/crypto_api.h"
#include "fid.h"

/***********************************************************************************************************************
 *  VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define FW_UPD_CFI_INIT_VAL  1000
#define FW_UPD_CFI_FINAL_VAL 1005
/***********************************************************************************************************************
 *  EXTERN
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL TYPEDEFS
 **********************************************************************************************************************/
typedef struct {
    uint8_t process_mode;
    uint8_t *image;
    uint32_t image_size;
    uint8_t *storage;
    uint32_t storage_size;
    raddr_t host_stor_addr;
    uint8_t *ctx;
    uint32_t ctx_size;
} ehsm_image_upgrade_st;

typedef struct {
    uint8_t *signature;
    uint8_t *pubkey;
} ehsm_upgrade_data_st;

typedef struct {
    uint8_t upgrade_alg;
    uint32_t is_decrypt_code;
    cpt_ske_ctx_st ske_ctx[1];
    uint32_t upgrade_already_size;
    uint32_t upgrade_total_size;
    uint32_t enc_key_id;
    uint32_t verify_key_id;
    uint8_t image_type;
    struct {
        cpt_hash_ctx_st hash_ctx[1];
        cpt_ske_cmac_ctx_st cmac_ctx[1];
    } alg_ctx;
} image_alg_upgrade_ctx_st;

typedef struct {
    uint8_t *signature;
    uint8_t *version;
} ehsm_code_data_st;

typedef struct {
    uint8_t is_analysis_code;
    uint8_t code_init_ctx_flag;
    uint32_t code_already_size;
    uint32_t code_total_size;
    uint32_t code_type;
    uint8_t code_enc;
    raddr_t code_addr;
    uint32_t enc_key_id;
    uint8_t verify_alg;
    cpt_ske_ctx_st ske_ctx[1];
} image_alg_code_ctx_st;

typedef struct {
    image_alg_upgrade_ctx_st *upgrade_alg_ctx;
    image_alg_code_ctx_st *code_alg_ctx;
    ehsm_upgrade_data_st *upgrade_data;
    ehsm_code_data_st *code_data;
    bool_t is_active;
} image_ctx_st;

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static image_alg_upgrade_ctx_st g_image_alg_upgrade[1];
static image_alg_code_ctx_st g_image_alg_code[1];
static image_ctx_st g_ctx[1];

static uint8_t g_upgrade_signature[RSA3072_PUBLIC_K_N_LEN] = { 0 };
static uint8_t g_upgrade_pubkey[RSA3072_PUBLIC_K_LEN] = { 0 };

static uint8_t g_code_signature[RSA3072_PUBLIC_K_N_LEN] = { 0 };
static uint8_t g_code_version[OTP_VERSION_SIZE] = { 0 };


static fid_u32_t g_fw_upgrade_cfi = FID_U32_VAL(0);
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t fwup_get_upd_verify_key(uint8_t fw_type);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void image_ctx_init(void)
{
    static ehsm_upgrade_data_st g_upgrade_data[1];
    static ehsm_code_data_st g_code_data[1];

    (void)util_memset(g_image_alg_upgrade, 0, sizeof(image_alg_upgrade_ctx_st));
    (void)util_memset(g_image_alg_code, 0, sizeof(image_alg_code_ctx_st));

    g_upgrade_data->pubkey = g_upgrade_pubkey;
    g_upgrade_data->signature = g_upgrade_signature;
    g_code_data->signature = g_code_signature;
    g_code_data->version = g_code_version;

    g_ctx->upgrade_alg_ctx = g_image_alg_upgrade;
    g_ctx->upgrade_data = g_upgrade_data;
    g_ctx->code_alg_ctx = g_image_alg_code;
    g_ctx->code_data = g_code_data;
    g_ctx->is_active = true;
}

static void image_ctx_finish(void)
{

    (void)util_memset(g_upgrade_signature, 0, sizeof(g_upgrade_signature));
    (void)util_memset(g_upgrade_pubkey, 0, sizeof(g_upgrade_pubkey));
    (void)util_memset(g_image_alg_upgrade, 0, sizeof(image_alg_upgrade_ctx_st));

    (void)util_memset(g_code_signature, 0, sizeof(g_code_signature));
    (void)util_memset(g_code_version, 0, sizeof(g_code_version));
    (void)util_memset(g_image_alg_code, 0, sizeof(image_alg_code_ctx_st));
    g_ctx->is_active = false;
}

static uint8_t get_upgrade_otp_key_type(uint8_t up_alg)
{
    uint8_t key_type;

    switch (up_alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
    case CODE_UPGRADE_ALG_ECC_P256R1:
    case CODE_UPGRADE_ALG_SM2:
        key_type = OTP_KEY_ALGO_HASH_TYPE;
        break;
    case CODE_UPGRADE_ALG_AES128_CMAC:
    case CODE_UPGRADE_ALG_SM4_CMAC:
        key_type = OTP_KEY_ALGO_SKE_TYPE;
        break;
    default:
        key_type = OTP_KEY_INVALID_TYPE;
        break;
    }

    return key_type;
}

static uint8_t get_upgrade_alg(uint8_t fw_type)
{
    uint8_t upgrade_alg;

    if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
        upgrade_alg = sysreg_get_fw_upd_alg();
    } else if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY) {
        upgrade_alg = sysreg_get_fw_upd_alg();
    } else if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
        upgrade_alg = sysreg_get_soc_upd_alg();
    } else {
        upgrade_alg = CODE_UPGRADE_ALG_INVALID;
    }

    return upgrade_alg;
}

static uint8_t get_otp_key_level(uint8_t fw_type)
{
    uint8_t key_level;

    if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
        key_level = OTP_KEY_LEVEL_1;
    } else if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY) {
        key_level = OTP_KEY_LEVEL_1;
    } else if (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
        key_level = OTP_KEY_LEVEL_2;
    } else {
        key_level = OTP_KEY_LEVEL_NO_CHECK;
    }

    return key_level;
}

static uint32_t get_upgrade_encrypt_key(uint8_t fw_type)
{
    uint32_t key_id;

    if ((fw_type == MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) || (fw_type == MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY)) {
        key_id = EHSM_OTP_EHSM_UPG_ENCRYPT_KEY_ID;
    } else {
        key_id = EHSM_OTP_SOC_UPG_ENCRYPT_KEY_ID;
    }

    return key_id;
}

static void get_upgrade_key_and_alg(image_alg_upgrade_ctx_st *upgrade_ctx, uint8_t fw_type)
{
    uint8_t verify_key_type;
    uint32_t verify_key_id;
    uint32_t encrypt_key_id;
    uint8_t key_level;

    if (NULL != upgrade_ctx) {

        upgrade_ctx->upgrade_alg = get_upgrade_alg(fw_type);
        verify_key_type = get_upgrade_otp_key_type(upgrade_ctx->upgrade_alg);
        verify_key_id = fwup_get_upd_verify_key(fw_type);
        encrypt_key_id = get_upgrade_encrypt_key(fw_type);
        key_level = get_otp_key_level(fw_type);

        if (OTP_KEY_INVALID_TYPE == verify_key_type) {
            upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
        } else {
            // check verify key
            if (EHSM_ERR_SW_SUCCESS != otpkey_check_usage(verify_key_id, verify_key_type, key_level, KEY_USAGE_NONE)) {
                upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
            } else {
                upgrade_ctx->verify_key_id = verify_key_id;
            }

            // check enc key
            if (EHSM_ERR_SW_SUCCESS
                != otpkey_check_usage(encrypt_key_id, OTP_KEY_ALGO_SKE_TYPE, key_level, KEY_USAGE_NONE)) {
                upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
            } else {
                upgrade_ctx->enc_key_id = encrypt_key_id;
            }
        }
    }
}

static void get_code_key_and_alg(image_alg_code_ctx_st *code_ctx)
{
    uint8_t key_lvl;

    if (NULL != code_ctx) {
        if ((MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == code_ctx->code_type)
            || (MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == code_ctx->code_type)) {
            code_ctx->verify_alg = sysreg_get_fw_verify_alg();
            code_ctx->enc_key_id = EHSM_OTP_EHSM_ENCRYPT_KEY_ID;
            key_lvl = OTP_KEY_LEVEL_1;
        } else if (MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == code_ctx->code_type) {
            code_ctx->verify_alg = sysreg_get_soc_verify_alg();
            code_ctx->enc_key_id = EHSM_OTP_SOC_ENCRYPT_KEY_ID;
            key_lvl = OTP_KEY_LEVEL_2;
        } else {
            code_ctx->verify_alg = CODE_VERIFY_ALG_INVALID;
            key_lvl = OTP_KEY_LEVEL_2;
        }

        if ((EHSM_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) && (CODE_VERIFY_ALG_INVALID != code_ctx->verify_alg)) {
            if (EHSM_ERR_SW_SUCCESS
                != otpkey_check_usage(code_ctx->enc_key_id, OTP_KEY_ALGO_SKE_TYPE, key_lvl, KEY_USAGE_NONE)) {
                code_ctx->verify_alg = CODE_UPGRADE_ALG_INVALID;
            }
        }
    }
}

static uint32_t fwup_get_upd_verify_key(uint8_t fw_type)
{
    uint32_t key_id;

    if ((MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == fw_type) || (MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == fw_type)) {
        key_id = EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID;
    } else if (MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == fw_type) {
        key_id = EHSM_OTP_SOC_UPG_VERIFY_KEY_ID;
    } else {
        key_id = EHSM_OTP_INVALID_KEY_ID;
    }

    return key_id;
}

static uint32_t check_pke_signature(
    uint32_t verify_alg, const uint8_t *hash_value, const uint8_t *signature, const uint8_t *pubkey, uint8_t fw_type)
{
    volatile uint32_t ret = EHSM_ERR_SW_SUCCESS; // add volatile avoid optimization double check
    uint32_t key_id;
    uint32_t pubkey_bits = 0;

    key_id = fwup_get_upd_verify_key(fw_type);
    if (EHSM_OTP_INVALID_KEY_ID != key_id) {
        if ((CODE_UPGRADE_ALG_RSA2048 == verify_alg) || (CODE_UPGRADE_ALG_RSA3072 == verify_alg)) {
            pubkey_bits = (CODE_UPGRADE_ALG_RSA2048 == verify_alg) ? 2048U : 3072U;
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_rsa_ssa_pss_verify_by_msg_digest(RSA_SIGNATURE_HASH, RSA_SIGNATURE_HASH, -1, hash_value, pubkey,
                512u, &pubkey[64], pubkey_bits, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        }
        if (CODE_UPGRADE_ALG_SM2 == verify_alg) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_sm2_verify(hash_value, pubkey, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        }
        if (CODE_UPGRADE_ALG_ECC_P256R1 == verify_alg) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_ecdsa_verify(secp256r1, hash_value, 32, pubkey, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        }

    } else {
        ret = EHSM_ERR_WRONG_FW_TYPE;
    }
    return ret;
}

static uint32_t get_upgrade_signdata_by_alg(
    uint8_t alg, const uint8_t *head, const ehsm_upgrade_data_st *upgrade_data, uint32_t key_id)
{
    uint32_t ret;
    uint32_t sign_bytes;

    if ((NULL == head) || (NULL == upgrade_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (alg) {
        case CODE_UPGRADE_ALG_RSA2048:
        case CODE_UPGRADE_ALG_RSA3072:
            sign_bytes = (CODE_UPGRADE_ALG_RSA2048 == alg) ? 256U : 384U;
            (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET], sign_bytes);
            ret = socvrfy_get_and_verify_pubkey(&head[PUBLIC_K_OFFSET], upgrade_data->pubkey, alg, key_id);
            break;
        case CODE_UPGRADE_ALG_SM2:
        case CODE_UPGRADE_ALG_ECC_P256R1:
            (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET], 64);
            ret = socvrfy_get_and_verify_pubkey(&head[PUBLIC_K_OFFSET], upgrade_data->pubkey, alg, key_id);
            break;



        case CODE_UPGRADE_ALG_AES128_CMAC:
        case CODE_UPGRADE_ALG_SM4_CMAC:
            (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET], 16);
            ret = EHSM_ERR_SW_SUCCESS;
            break;

        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }
    }

    return ret;
}

static uint32_t image_upgrade_rsa_ctx_init(const uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;

    if ((NULL == head) || (NULL == upgrade_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // sign hash init
        ret = cpt_hash_init(upgrade_ctx->alg_ctx.hash_ctx, HASH_SHA256);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(upgrade_ctx->alg_ctx.hash_ctx, &head[VALID_FLAG_OFFSET], EHSM_IMAGE_HEADER_HASH_SIZE);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
            ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // ske dec init
                ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, SKE_ALG_AES, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                    &head[IV_OFFSET], SKE_NO_PADDING);
            }
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }

    return ret;
}

static uint32_t image_upgrade_sm2_ctx_init(const uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint8_t hash_value[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;

    if ((NULL == head) || (NULL == upgrade_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // sign hash init
        ret = cpt_sm2_getZ(NULL, 0, &head[PUBLIC_K_OFFSET], hash_value);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_init(upgrade_ctx->alg_ctx.hash_ctx, HASH_SM3);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_update(upgrade_ctx->alg_ctx.hash_ctx, (uint8_t *)hash_value, 32);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_update(
                    upgrade_ctx->alg_ctx.hash_ctx, &head[VALID_FLAG_OFFSET], EHSM_IMAGE_HEADER_HASH_SIZE);
            }
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
            ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // ske dec init
                ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, SKE_ALG_SM4, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                    &head[IV_OFFSET], SKE_NO_PADDING);
            }
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }

    return ret;
}


static uint32_t image_upgrade_ske_ctx_init(
    const uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx, cpt_ske_alg_e ske_algorithm)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;
    uint16_t phy_vrf_key;

    if ((NULL == head) || (NULL == upgrade_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = otpkey_get_phyid(upgrade_ctx->verify_key_id, &phy_vrf_key);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cmac_init(upgrade_ctx->alg_ctx.cmac_ctx, ske_algorithm, SKE_VERIFY_MAC, NULL, phy_vrf_key, 16);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cmac_update(
                upgrade_ctx->alg_ctx.cmac_ctx, &head[VALID_FLAG_OFFSET], EHSM_IMAGE_HEADER_HASH_SIZE);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                // ske dec init
                ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, ske_algorithm, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                    &head[IV_OFFSET], SKE_NO_PADDING);
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
    }

    return ret;
}

static uint32_t image_upgrade_ctx_init(const uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e ske_algorithm;

    if ((NULL == head) || (NULL == upgrade_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (upgrade_ctx->upgrade_alg) {
        case CODE_UPGRADE_ALG_RSA2048:
        case CODE_UPGRADE_ALG_RSA3072:
        case CODE_UPGRADE_ALG_ECC_P256R1:
            ret = image_upgrade_rsa_ctx_init(head, upgrade_ctx);
            break;
        case CODE_UPGRADE_ALG_SM2:
            ret = image_upgrade_sm2_ctx_init(head, upgrade_ctx);
            break;
        case CODE_UPGRADE_ALG_AES128_CMAC:
        case CODE_UPGRADE_ALG_SM4_CMAC:
            if (CODE_UPGRADE_ALG_SM4_CMAC == upgrade_ctx->upgrade_alg) {
                ske_algorithm = SKE_ALG_SM4;
            } else
            {
                ske_algorithm = SKE_ALG_AES;
            }
            ret = image_upgrade_ske_ctx_init(head, upgrade_ctx, ske_algorithm);
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            (void)util_memcpy((uint8_t *)&(upgrade_ctx->upgrade_total_size), &head[CODE_SIZE_OFFSET], 4u);
        }
    }

    return ret;
}

static uint32_t image_code_enc_ctx_init(
    image_alg_code_ctx_st *code_ctx, const ehsm_image_upgrade_st *image_upgrade, cpt_ske_alg_e ske_algorithm)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t iv[16];
    uint16_t phy_enc_key;

    if ((NULL == code_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // get iv
        ret = cpt_get_rand((uint8_t *)iv, 16);
        // cbc enc init
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otpkey_get_phyid(code_ctx->enc_key_id, &phy_enc_key);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cbc_init(
                code_ctx->ske_ctx, ske_algorithm, SKE_CRYPTO_ENCRYPT, NULL, phy_enc_key, iv, SKE_NO_PADDING);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            (void)util_memcpy(&image_upgrade->storage[IV_OFFSET], iv, 16);
            if (image_upgrade->image_size >= EHSM_IMAGE_HEADER_SIZE) {
                ret = cpt_ske_cbc_update_blocks(code_ctx->ske_ctx, &image_upgrade->storage[EHSM_IMAGE_HEADER_SIZE],
                    &image_upgrade->storage[EHSM_IMAGE_HEADER_SIZE],
                    image_upgrade->image_size - EHSM_IMAGE_HEADER_SIZE);
            }
        }
    }

    return ret;
}

static uint32_t image_code_ctx_init(image_alg_code_ctx_st *code_ctx, const ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e ske_algorithm;

    if ((NULL == code_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (code_ctx->verify_alg) {
        case CODE_VERIFY_ALG_RSA2048:
        case CODE_VERIFY_ALG_RSA3072:
        case CODE_UPGRADE_ALG_ECC_P256R1:
            ske_algorithm = SKE_ALG_AES;
            break;

        case CODE_VERIFY_ALG_SM2:
            ske_algorithm = SKE_ALG_SM4;
            break;
        case CODE_VERIFY_ALG_AES_CMAC:
            ske_algorithm = SKE_ALG_AES;
            break;
        case CODE_VERIFY_ALG_SM4_CMAC:
            ske_algorithm = SKE_ALG_SM4;
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }

        if ((EHSM_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) && (EHSM_ERR_SW_SUCCESS == ret)) {
            ret = image_code_enc_ctx_init(code_ctx, image_upgrade, ske_algorithm);
        }

        // set head_flag
        if (EHSM_ERR_SW_SUCCESS == ret) {
            code_ctx->code_init_ctx_flag = 1;
            (void)util_memcpy(
                (uint8_t *)&(code_ctx->code_total_size), (uint8_t *)&image_upgrade->storage[CODE_SIZE_OFFSET], 4u);
            code_ctx->code_already_size = image_upgrade->image_size - EHSM_IMAGE_HEADER_SIZE;
            code_ctx->code_addr = image_upgrade->host_stor_addr;
        }
    }

    return ret;
}

static uint32_t fwup_pke_upgrade_ctx_update(
    image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == upgrade_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpt_hash_update(upgrade_ctx->alg_ctx.hash_ctx, image_upgrade->image, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_update_blocks(
                    upgrade_ctx->ske_ctx, image_upgrade->image, image_upgrade->storage, image_upgrade->image_size);
            }
        }
    }

    return ret;
}


static uint32_t fwup_cmac_upgrade_ctx_update(
    image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == upgrade_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpt_ske_cmac_update(upgrade_ctx->alg_ctx.cmac_ctx, image_upgrade->image, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_update_blocks(
                    upgrade_ctx->ske_ctx, image_upgrade->image, image_upgrade->storage, image_upgrade->image_size);
            }
        }
    }

    return ret;
}

static uint32_t image_upgrade_ctx_update(
    image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == upgrade_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (upgrade_ctx->upgrade_alg) {
        case CODE_UPGRADE_ALG_RSA2048:
        case CODE_UPGRADE_ALG_RSA3072:
        case CODE_UPGRADE_ALG_ECC_P256R1:
        case CODE_UPGRADE_ALG_SM2:
            ret = fwup_pke_upgrade_ctx_update(upgrade_ctx, image_upgrade);
            break;

        case CODE_UPGRADE_ALG_AES128_CMAC:
        case CODE_UPGRADE_ALG_SM4_CMAC:
            ret = fwup_cmac_upgrade_ctx_update(upgrade_ctx, image_upgrade);
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }
    }

    return ret;
}

static uint32_t image_code_ctx_update(image_alg_code_ctx_st *code_ctx, const ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == code_ctx) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (EHSM_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) {
            ret = cpt_ske_cbc_update_blocks(
                code_ctx->ske_ctx, image_upgrade->storage, image_upgrade->storage, image_upgrade->image_size);
        }
    }

    return ret;
}

static uint32_t fwup_pke_upgrade_ctx_finsh(
    image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_upgrade_data_st *upgrade_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t hash_value[32];

    if ((NULL == upgrade_ctx) || (NULL == upgrade_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpt_hash_final(upgrade_ctx->alg_ctx.hash_ctx, hash_value);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_final(upgrade_ctx->ske_ctx);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = check_pke_signature(upgrade_ctx->upgrade_alg, hash_value, upgrade_data->signature,
                        upgrade_data->pubkey, upgrade_ctx->image_type);
                }
            } else {
                ret = check_pke_signature(upgrade_ctx->upgrade_alg, hash_value, upgrade_data->signature,
                    upgrade_data->pubkey, upgrade_ctx->image_type);
            }
        }
    }

    return ret;
}

static uint32_t fwup_cmac_upgrade_ctx_finsh(
    image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_upgrade_data_st *upgrade_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == upgrade_ctx) || (NULL == upgrade_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_fw_upgrade_cfi, 1);
        ret = cpt_ske_cmac_final(upgrade_ctx->alg_ctx.cmac_ctx, upgrade_data->signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                fid_inc_u32(&g_fw_upgrade_cfi, 1);
                ret = cpt_ske_cbc_final(upgrade_ctx->ske_ctx);
            }
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
        } else {
            ret = EHSM_ERR_FW_VERIFY_FAILED;
        }
    }
    fid_inc_u32(&g_fw_upgrade_cfi, 1);

    return ret;
}

static uint32_t image_upgrade_ctx_finsh(image_alg_upgrade_ctx_st *upgrade_ctx, const ehsm_upgrade_data_st *upgrade_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == upgrade_ctx) || (NULL == upgrade_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (upgrade_ctx->upgrade_alg) {
        case CODE_UPGRADE_ALG_RSA2048:
        case CODE_UPGRADE_ALG_RSA3072:
        case CODE_UPGRADE_ALG_ECC_P256R1:
        case CODE_UPGRADE_ALG_SM2:
            ret = fwup_pke_upgrade_ctx_finsh(upgrade_ctx, upgrade_data);
            break;
        case CODE_UPGRADE_ALG_AES128_CMAC:
        case CODE_UPGRADE_ALG_SM4_CMAC:
            ret = fwup_cmac_upgrade_ctx_finsh(upgrade_ctx, upgrade_data);
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }
    }

    return ret;
}

static uint32_t image_code_ctx_finsh(image_alg_code_ctx_st *code_ctx, uint8_t *out)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == code_ctx) || (NULL == out)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (EHSM_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) {
            ret = cpt_ske_cbc_final(code_ctx->ske_ctx);
        }
    }

    return ret;
}

static uint32_t image_upgrade_alg_and_ctx_init(const uint8_t *head, const image_ctx_st *image_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_id;

    if ((NULL == head) || (NULL == image_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // upgrade alg init
        get_upgrade_key_and_alg(image_ctx->upgrade_alg_ctx, image_ctx->upgrade_alg_ctx->image_type);
        if (CODE_UPGRADE_ALG_INVALID == image_ctx->upgrade_alg_ctx->upgrade_alg) {
            ret = EHSM_ERR_WRONG_ALGORITHM;
        } else {
            key_id = fwup_get_upd_verify_key(image_ctx->upgrade_alg_ctx->image_type);
            // upgrade alg ctx_init
            ret = image_upgrade_ctx_init(head, image_ctx->upgrade_alg_ctx);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = get_upgrade_signdata_by_alg(
                    image_ctx->upgrade_alg_ctx->upgrade_alg, head, image_ctx->upgrade_data, key_id);
            }
        }
    }

    return ret;
}

static uint32_t image_code_alg_and_ctx_init(const ehsm_image_upgrade_st *image_upgrade, const image_ctx_st *image_ctx)
{
    uint32_t ret;

    if ((NULL == image_upgrade) || (NULL == image_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        // verify alg init
        if (image_upgrade->image_size < EHSM_IMAGE_HEADER_SIZE) {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        } else {
            // 从启动固件读取plain flag，如果是1，不对code区加密，否则需要对code区加密
            image_ctx->code_alg_ctx->code_enc = image_upgrade->storage[PLAIN_FLAG_OFFSET];
            image_ctx->code_alg_ctx->code_type = image_upgrade->storage[IMAGE_TYPE_OFFSET];
            // code head
            get_code_key_and_alg(image_ctx->code_alg_ctx);
            if (CODE_VERIFY_ALG_INVALID == image_ctx->code_alg_ctx->verify_alg) {
                ret = EHSM_ERR_WRONG_ALGORITHM;
            } else {
                if (MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == image_ctx->code_alg_ctx->code_type) {
                    ret = otp_read(
                        OTP_EHSM_VERSION_ADDRESS, (uint8_t *)image_ctx->code_data->version, OTP_VERSION_SIZE);
                } else {
                    ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)image_ctx->code_data->version, OTP_VERSION_SIZE);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = socvrfy_check_fw_header(
                        image_upgrade->storage, MB_SOC_VERIFY_CHECK_VERSION_ON, image_ctx->code_data->version);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = image_code_ctx_init(image_ctx->code_alg_ctx, image_upgrade);
                }
            }
        }
    }

    return ret;
}

static bool_t fwup_header_is_valid(const uint8_t *head)
{
    bool_t ret;
    uint32_t valid;

    if (NULL == head) {
        ret = false;
    } else {
        util_memcpy(&valid, &head[VALID_FLAG_OFFSET], 4);
        if (valid != UPGRADE_VALID_FLAG) {
            ret = false;
        } else {
            ret = true;
        }
    }

    return ret;
}

static uint32_t fwup_check_fw_header(const uint8_t *header, const uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == header) || (NULL == version)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (true == fwup_header_is_valid(header)) {
            if (false == socvrfy_check_ver_cnt(version, &header[VERSION_COUNTER_OFFSET])) {
                ret = EHSM_ERR_WRONG_VERSION_COUNTER;
            }
        } else {
            ret = EHSM_ERR_INVALID_CODE_FLAG;
        }
    }

    return ret;
}

static uint32_t image_upgrage_init(const uint8_t *head, uint32_t head_size, const image_ctx_st *image_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t version[32];

    if ((NULL == head) || (NULL == image_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (EHSM_IMAGE_HEADER_SIZE != head_size) {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        } else {
            image_ctx->code_alg_ctx->code_type = head[IMAGE_TYPE_OFFSET];
            image_ctx->upgrade_alg_ctx->image_type = head[IMAGE_TYPE_OFFSET];

            if (MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == image_ctx->code_alg_ctx->code_type) {
                ret = otp_read(OTP_EHSM_VERSION_ADDRESS, (uint8_t *)version, OTP_VERSION_SIZE);
            } else if ((MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == image_ctx->code_alg_ctx->code_type)
                || (MB_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == image_ctx->code_alg_ctx->code_type)) {
                ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)version, OTP_VERSION_SIZE);
            } else {
                ret = EHSM_ERR_WRONG_FW_TYPE;
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = fwup_check_fw_header(head, version);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = image_upgrade_alg_and_ctx_init(head, image_ctx);
        }
    }

    return ret;
}

static uint32_t image_upgrade_update(const ehsm_image_upgrade_st *image_upgrade, const image_ctx_st *image_ctx)
{
    uint32_t ret;

    if ((NULL == image_upgrade) || (NULL == image_ctx)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = image_upgrade_ctx_update(image_ctx->upgrade_alg_ctx, image_upgrade);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_ANALYSIS_CODE == image_ctx->code_alg_ctx->is_analysis_code) {
                if (1u != image_ctx->code_alg_ctx->code_init_ctx_flag) {
                    ret = image_code_alg_and_ctx_init(image_upgrade, image_ctx);
                } else {
                    image_ctx->code_alg_ctx->code_already_size += image_upgrade->image_size;
                    ret = image_code_ctx_update(image_ctx->code_alg_ctx, image_upgrade);
                }
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
    }

    return ret;
}

static uint32_t image_upgrade_finish(const image_ctx_st *image_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (NULL == image_ctx) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = image_upgrade_ctx_finsh(image_ctx->upgrade_alg_ctx, image_ctx->upgrade_data);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_ANALYSIS_CODE == image_ctx->code_alg_ctx->is_analysis_code) {
                ret = image_code_ctx_finsh(image_ctx->code_alg_ctx, image_ctx->code_data->signature);
            }
        }
    }

    return ret;
}

static uint32_t write_code_to_storage(raddr_t addr, const uint8_t *in, uint32_t bytes, uint8_t image_type)
{
    uint32_t ret;

    if (image_type == MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
        ret = mmap_write_remote_data(addr, in, bytes);
    } else {
        ret = mmap_write_remote_data(addr, in, bytes);
    }

    return ret;
}


static void fwup_get_store_addr(raddr_t *addr, const mb_cmd_fw_upgrade_st *cmd_upgrade, uint8_t image_type)
{
    if ((NULL != addr) && (NULL != cmd_upgrade)) {
        if (image_type == MB_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
            *addr = cmd_upgrade->storage_addr;
        } else {
            *addr = cmd_upgrade->storage_addr;
        }
    }
}


static uint32_t fwup_image_upgrade(const ehsm_image_upgrade_st *image_upgrade, const image_ctx_st *image_ctx)
{
    uint32_t ret;

    if (NULL == image_upgrade) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (image_upgrade->process_mode) {
        case MB_START:
            ret = image_upgrage_init(image_upgrade->image, image_upgrade->image_size, image_ctx);
            break;
        case MB_UPDATE:
            ret = image_upgrade_update(image_upgrade, image_ctx);
            break;
        case MB_FINISH:
            ret = image_upgrade_finish(image_ctx);
            break;
        default:
            ret = EHSM_ERR_WRONG_PROC_MODE;
            break;
        }
    }

    return ret;
}

static uint32_t image_upgrade_flow_init(
    const image_ctx_st *ctx, const mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;

    if ((NULL == ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        image_upgrade->process_mode = MB_START;
        image_upgrade->image_size = cmd_upgrade->image_size;
        image_upgrade->host_stor_addr = cmd_upgrade->storage_addr;

        ret = mmap_read_remote_data((uint8_t *)image_upgrade->image, cmd_upgrade->image_addr, EHSM_IMAGE_HEADER_SIZE);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ctx->code_alg_ctx->code_enc = image_upgrade->image[PLAIN_FLAG_OFFSET];
            ret = fwup_image_upgrade(image_upgrade, ctx);
        }

    }

    return ret;
}

static uint32_t upgrade_update_flow_process_data(
    const image_ctx_st *ctx, mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;

    if ((NULL == ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = mmap_read_remote_data(
            (uint8_t *)image_upgrade->image, cmd_upgrade->image_addr, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            cmd_upgrade->image_addr += image_upgrade->image_size;
            image_upgrade->host_stor_addr = cmd_upgrade->storage_addr;
            ret = fwup_image_upgrade(image_upgrade, ctx);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_ANALYSIS_CODE == ctx->code_alg_ctx->is_analysis_code) {
                ret = write_code_to_storage(cmd_upgrade->storage_addr, (uint8_t *)image_upgrade->storage,
                    image_upgrade->image_size, ctx->upgrade_alg_ctx->image_type);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            cmd_upgrade->storage_addr += image_upgrade->image_size;
        }
    }

    return ret;
}

static uint32_t image_upgrade_flow_update(
    const image_ctx_st *ctx, mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t block_cnt;
    uint32_t left_block_size = 0;
    uint32_t read_size;
    uint32_t input_image_size;

    if ((NULL == ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        input_image_size = cmd_upgrade->image_size;
        raddr_t addr;
        fwup_get_store_addr(&addr, cmd_upgrade, ctx->upgrade_alg_ctx->image_type);
        cmd_upgrade->storage_addr = addr;
        image_upgrade->process_mode = MB_UPDATE;
        block_cnt = input_image_size / IMAGE_UPGRADE_BLOCK_SIZE;
        left_block_size = input_image_size % IMAGE_UPGRADE_BLOCK_SIZE;
        read_size = IMAGE_UPGRADE_BLOCK_SIZE;

        if ((ctx->upgrade_alg_ctx->upgrade_already_size + input_image_size)
            > ctx->upgrade_alg_ctx->upgrade_already_size) {
            ctx->upgrade_alg_ctx->upgrade_already_size += input_image_size;
            if ((0U != (left_block_size & 0x0FU))
                || (ctx->upgrade_alg_ctx->upgrade_already_size > CONFIG_EHSM_MAX_UPGRADE_FW_CODE_SIZE)) {
                ret = EHSM_ERR_WRONG_DATA_LENGTH;
            }
        } else {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        for (uint32_t i = 0; i < block_cnt; i++) {
            image_upgrade->image_size = read_size;
            ret = upgrade_update_flow_process_data(ctx, cmd_upgrade, image_upgrade);
            if (EHSM_ERR_SW_SUCCESS != ret) {
                break;
            }
        }

        if ((EHSM_ERR_SW_SUCCESS == ret) && (0u != left_block_size)) {
            image_upgrade->image_size = left_block_size;
            ret = upgrade_update_flow_process_data(ctx, cmd_upgrade, image_upgrade);
        }

    }

    return ret;
}

static uint32_t image_upgrade_flow_finish(
    const image_ctx_st *ctx, const mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (ctx->upgrade_alg_ctx->upgrade_total_size == ctx->upgrade_alg_ctx->upgrade_already_size) {
            if (IMAGE_ANALYSIS_CODE == ctx->code_alg_ctx->is_analysis_code) {
                if (ctx->code_alg_ctx->code_already_size != ctx->code_alg_ctx->code_total_size) {
                    ret = EHSM_ERR_WRONG_DATA_LENGTH;
                }
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                image_upgrade->process_mode = MB_FINISH;
                image_upgrade->image_size = 0;
                image_upgrade->host_stor_addr = cmd_upgrade->storage_addr;
                ret = fwup_image_upgrade(image_upgrade, ctx);
            }
        } else {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    }

    return ret;
}

static uint32_t image_upgrade_flow_one_pass(
    const image_ctx_st *ctx, mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;
    uint32_t left_block_size = 0;

    fid_set_u32(&g_fw_upgrade_cfi, FW_UPD_CFI_INIT_VAL);
    if ((NULL == ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (cmd_upgrade->image_size >= EHSM_IMAGE_HEADER_SIZE) {
            left_block_size = cmd_upgrade->image_size - EHSM_IMAGE_HEADER_SIZE;
        }
        cmd_upgrade->image_size = EHSM_IMAGE_HEADER_SIZE;

        // init
        ret = image_upgrade_flow_init(ctx, cmd_upgrade, image_upgrade);
        if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
            // update
            cmd_upgrade->image_addr += EHSM_IMAGE_HEADER_SIZE;
            cmd_upgrade->image_size = left_block_size;
            ret = image_upgrade_flow_update(ctx, cmd_upgrade, image_upgrade);
            if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
                ret = image_upgrade_flow_finish(ctx, cmd_upgrade, image_upgrade);
            }
        }
    }
    if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        if (fid_get_u32(&g_fw_upgrade_cfi) != FW_UPD_CFI_FINAL_VAL) {
            fid_panic();
        }
    }

    return ret;
}

static uint32_t fwup_do_upgrade(
    const image_ctx_st *image_ctx, mb_cmd_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;

    if ((NULL == image_ctx) || (NULL == cmd_upgrade) || (NULL == image_upgrade)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {

        switch (cmd_upgrade->process_mode) {
        case MB_START:
            ret = image_upgrade_flow_init(image_ctx, cmd_upgrade, image_upgrade);
            break;
        case MB_UPDATE:
        case MB_FINISH:
            ret = image_upgrade_flow_update(image_ctx, cmd_upgrade, image_upgrade);
            if ((EHSM_ERR_SW_SUCCESS == ret) && FID_EQ(MB_FINISH, cmd_upgrade->process_mode)) {
                fid_set_u32(&g_fw_upgrade_cfi, FW_UPD_CFI_INIT_VAL);
                ret = image_upgrade_flow_finish(image_ctx, cmd_upgrade, image_upgrade);
                if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
                    if (fid_get_u32(&g_fw_upgrade_cfi) != FW_UPD_CFI_FINAL_VAL) {
                        fid_panic();
                    }
                }
            }
            break;
        case MB_ONE_PASS:
            ret = image_upgrade_flow_one_pass(image_ctx, cmd_upgrade, image_upgrade);
            break;
        default:
            ret = EHSM_ERR_WRONG_PROC_MODE;
            break;
        }
    }

    return ret;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t fwup_srv_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t block_buf[IMAGE_UPGRADE_BLOCK_SIZE];
    mb_cmd_fw_upgrade_st cmd_upgrade[1];
    ehsm_image_upgrade_st image_upgrade[1];
    const image_ctx_st *image_ctx = g_ctx;

    if ((NULL == req_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        (void)rsp_data;
        util_memset(cmd_upgrade, 0, sizeof(mb_cmd_fw_upgrade_st));
        (void)util_memcpy((uint8_t *)cmd_upgrade, req_data, sizeof(mb_cmd_fw_upgrade_st));
        if ((0U == cmd_upgrade->image_addr) || (0U == cmd_upgrade->storage_addr)) {
            ret = EHSM_ERR_INVALID_ADDRESS;
        } else if (0U == cmd_upgrade->image_size) {
            ret = EHSM_ERR_PARAM_ERROR;
        } else {
            (void)util_memset(block_buf, 0, sizeof(block_buf));
            image_upgrade->image = block_buf;
            image_upgrade->storage = block_buf;
            if ((MB_START == cmd_upgrade->process_mode) || (MB_ONE_PASS == cmd_upgrade->process_mode)) {
                image_ctx_init();
                image_ctx->upgrade_alg_ctx->is_decrypt_code = IMAGE_DECRYPT_CODE;
                image_ctx->code_alg_ctx->is_analysis_code = IMAGE_ANALYSIS_CODE;
            } else if (false == image_ctx->is_active) {
                ret = EHSM_ERR_WRONG_PROC_MODE;
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = fwup_do_upgrade(image_ctx, cmd_upgrade, image_upgrade);
            }
            if ((EHSM_ERR_SW_SUCCESS != ret) || (MB_FINISH == cmd_upgrade->process_mode)
                || (MB_ONE_PASS == cmd_upgrade->process_mode)) {
                image_ctx_finish();
            }
        }
    }

    return ret;
}
