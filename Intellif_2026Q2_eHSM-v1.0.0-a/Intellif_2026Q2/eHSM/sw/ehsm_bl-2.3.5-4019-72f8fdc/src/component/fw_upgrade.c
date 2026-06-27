/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "fw_upgrade.h"
#include "config.h"
#include "crypto_common/eccp_curve.h"
#include "crypto_lib_api.h"
#include "types.h"
#include "fw_verify.h"
#include "mb.h"
#include "secure_boot.h"
#include "mbcmd_parser.h"
#include "otp_key.h"
#include "sysreg.h"
#include "util.h"
#include "mmap.h"
#include "otp_data.h"
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

/***********************************************************************************************************************
 *  LOCAL CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL VARIABLES
 **********************************************************************************************************************/
static image_alg_upgrade_ctx_st g_image_alg_upgrade[1];
static image_alg_code_ctx_st g_image_alg_code[1];
static image_ctx_st g_ctx[1];
static ehsm_upgrade_data_st g_upgrade_data[1];
static ehsm_code_data_st g_code_data[1];

static uint8_t g_upgrade_signature[MAX_SIGNATURE_SIZE] = { 0 };
static uint8_t g_upgrade_pubkey[MAX_PUBKEY_SIZE] = { 0 };

static uint8_t g_code_signature[MAX_SIGNATURE_SIZE] = { 0 };
static uint8_t g_code_pubkey[MAX_PUBKEY_SIZE] = { 0 };
static uint8_t g_code_version[EHSM_VERSION_COUNTER_LEN] = { 0 };


static fid_u32_t g_fw_upgrade_cfi = FID_U32_VAL(0);
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static void image_ctx_init(void)
{
    (void)util_memset(g_image_alg_upgrade, 0, sizeof(image_alg_upgrade_ctx_st));
    (void)util_memset(g_image_alg_code, 0, sizeof(image_alg_code_ctx_st));

    g_upgrade_data->pubkey = g_upgrade_pubkey;
    g_upgrade_data->signature = g_upgrade_signature;
    g_ctx->upgrade_alg_ctx = g_image_alg_upgrade;
    g_ctx->upgrade_data = g_upgrade_data;

    g_code_data->pubkey = g_code_pubkey;
    g_code_data->signature = g_code_signature;
    g_code_data->version = g_code_version;
    g_ctx->code_alg_ctx = g_image_alg_code;
    g_ctx->code_data = g_code_data;
}

static void image_ctx_finish(void)
{

    (void)util_memset(g_upgrade_signature, 0, sizeof(g_upgrade_signature));
    (void)util_memset(g_upgrade_pubkey, 0, sizeof(g_upgrade_pubkey));
    (void)util_memset(g_image_alg_upgrade, 0, sizeof(image_alg_upgrade_ctx_st));

    (void)util_memset(g_code_signature, 0, sizeof(g_code_signature));
    (void)util_memset(g_code_pubkey, 0, sizeof(g_code_pubkey));
    (void)util_memset(g_code_version, 0, sizeof(g_code_version));
    (void)util_memset(g_image_alg_code, 0, sizeof(image_alg_code_ctx_st));
}

static uint8_t get_upgrade_key_usage(uint8_t up_alg)
{
    uint8_t verify_key_usage;

    switch (up_alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
    case CODE_UPGRADE_ALG_ECC_P256R1:
    case CODE_UPGRADE_ALG_SM2:
        verify_key_usage = K_USAGE_HASH;
        break;
    case CODE_UPGRADE_ALG_AES_CMAC:
    case CODE_UPGRADE_ALG_SM4_CMAC:
        verify_key_usage = K_USAGE_SKE;
        break;
    default:
        verify_key_usage = K_USAGE_INVALID;
        break;
    }

    return verify_key_usage;
}

uint8_t get_code_key_usage(uint8_t verify_alg)
{
    uint8_t verify_key_usage;
    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
    case CODE_VERIFY_ALG_ECC_P256R1:
    case CODE_VERIFY_ALG_SM2:
        verify_key_usage = K_USAGE_HASH;
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        verify_key_usage = K_USAGE_SKE;
        break;
    default:
        verify_key_usage = K_USAGE_INVALID;
        break;
    }

    return verify_key_usage;
}

static uint8_t get_code_upgrade_alg(uint8_t fw_type)
{
    uint8_t upgrade_alg;

    if ((fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH)) {
        upgrade_alg = sysreg_get_fw_upd_alg();
    } else if (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY) {
        upgrade_alg = sysreg_get_fw_upd_alg();
    } else if (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
        upgrade_alg = sysreg_get_soc_upd_alg();
    } else {
        upgrade_alg = CODE_UPGRADE_ALG_INVALID;
    }

    return upgrade_alg;
}

static uint8_t get_key_level(uint8_t fw_type)
{
    uint8_t key_level;

    if ((fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH)) {
        key_level = K_LEVEL_1;
    } else if (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY) {
        key_level = K_LEVEL_1;
    } else if (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
        key_level = K_LEVEL_2;
    } else {
        key_level = K_LEVEL_NO_CHECK;
    }

    return key_level;
}

static uint32_t get_upgrade_verify_key(uint8_t fw_type)
{
    uint32_t key_id;

    if ((fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY)
        || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH)) {
        key_id = EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID;
    } else {
        key_id = EHSM_OTP_SOC_UPG_VERIFY_KEY_ID;
    }

    return key_id;
}

static uint32_t get_upgrade_encrypt_key(uint8_t fw_type)
{
    uint32_t key_id;

    if ((fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY)
        || (fw_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH)) {
        key_id = EHSM_OTP_EHSM_UPG_ENCRYPT_KEY_ID;
    } else {
        key_id = EHSM_OTP_SOC_UPG_ENCRYPT_KEY_ID;
    }

    return key_id;
}

static void get_upgrade_alg(image_alg_upgrade_ctx_st *upgrade_ctx, uint8_t fw_type)
{
    uint8_t verify_key_usage;
    uint32_t verify_key_id;
    uint32_t encrypt_key_id;
    uint8_t key_level;

    upgrade_ctx->upgrade_alg = get_code_upgrade_alg(fw_type);
    verify_key_usage = get_upgrade_key_usage(upgrade_ctx->upgrade_alg);
    verify_key_id = get_upgrade_verify_key(fw_type);
    encrypt_key_id = get_upgrade_encrypt_key(fw_type);
    key_level = get_key_level(fw_type);

    if (K_USAGE_INVALID == verify_key_usage) {
        upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
    } else {
        // check verify key
        if (EHSM_ERR_SW_SUCCESS != otpkey_check_usage(verify_key_id, verify_key_usage, key_level, KEY_USAGE_NONE)) {
            upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
        } else {
            upgrade_ctx->verify_key_id = verify_key_id;
        }

        // check enc key
        if (CODE_UPGRADE_ALG_INVALID != upgrade_ctx->upgrade_alg) {
            if (EHSM_ERR_SW_SUCCESS != otpkey_check_usage(encrypt_key_id, K_USAGE_SKE, key_level, KEY_USAGE_NONE)) {
                upgrade_ctx->upgrade_alg = CODE_UPGRADE_ALG_INVALID;
            } else {
                upgrade_ctx->enc_key_id = encrypt_key_id;
            }
        }
    }
}

static void get_code_alg(image_alg_code_ctx_st *code_ctx)
{
    uint8_t verify_key_usage;
    uint8_t key_lvl;

    if ((MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == code_ctx->code_type)
        || (MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == code_ctx->code_type)
        || (MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH == code_ctx->code_type)) {
        code_ctx->verify_alg = sysreg_get_fw_verify_alg();
        code_ctx->verify_key_id = EHSM_OTP_EHSM_FW_VERIFY_KEY_ID;
        code_ctx->enc_key_id = EHSM_OTP_EHSM_ENCRYPT_KEY_ID;
        key_lvl = K_LEVEL_1;
    } else if (MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == code_ctx->code_type) {
        code_ctx->verify_alg = sysreg_get_soc_verify_alg();
        code_ctx->verify_key_id = EHSM_OTP_SOC_FW_VERIFY_KEY_ID;
        code_ctx->enc_key_id = EHSM_OTP_SOC_ENCRYPT_KEY_ID;
        key_lvl = K_LEVEL_2;
    } else {
        code_ctx->verify_alg = CODE_VERIFY_ALG_INVALID;
        key_lvl = K_LEVEL_2;
    }

    verify_key_usage = get_code_key_usage(code_ctx->verify_alg);
    if (K_USAGE_INVALID == verify_key_usage) {
        code_ctx->verify_alg = CODE_VERIFY_ALG_INVALID;
    } else {
        if (EHSM_ERR_SW_SUCCESS
            != otpkey_check_usage(code_ctx->verify_key_id, verify_key_usage, key_lvl, KEY_USAGE_NONE)) {
            code_ctx->verify_alg = CODE_VERIFY_ALG_INVALID;
        }

        if (MB_BL_VERIFY_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) {
            if (EHSM_ERR_SW_SUCCESS != otpkey_check_usage(code_ctx->enc_key_id, K_USAGE_SKE, key_lvl, KEY_USAGE_NONE)) {
                code_ctx->verify_alg = CODE_UPGRADE_ALG_INVALID;
            }
        }
    }
}

static uint32_t fwupd_get_verify_key_id(uint8_t fw_type)
{
    uint32_t key_id;

    if ((MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == fw_type) || (MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == fw_type)
        || (MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH == fw_type)) {
        key_id = EHSM_OTP_EHSM_UPG_VERIFY_KEY_ID;
    } else if (MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == fw_type) {
        key_id = EHSM_OTP_SOC_UPG_VERIFY_KEY_ID;
    } else {
        key_id = EHSM_OTP_INVALID_KEY_ID;
    }

    return key_id;
}

static uint32_t check_signature(
    uint32_t verify_alg, uint8_t *hash_value, uint8_t *signature, uint8_t *pubkey, uint8_t fw_type)
{
    volatile uint32_t ret; // add volatile avoid optimization double check
    uint32_t key_id;

    key_id = fwupd_get_verify_key_id(fw_type);
    if (EHSM_OTP_INVALID_KEY_ID != key_id) {
        if (CODE_UPGRADE_ALG_RSA2048 == verify_alg || CODE_UPGRADE_ALG_RSA3072 == verify_alg) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_rsa_ssa_pss_verify_by_msg_digest(RSA_SIGNATURE_HASH, RSA_SIGNATURE_HASH, -1,
                (uint8_t *)hash_value, pubkey, RSA_PUBLIC_K_E_LEN * 8, (uint8_t *)&pubkey[RSA_PUBLIC_K_E_LEN],
                CODE_UPGRADE_ALG_RSA2048 == verify_alg ? 2048u : 3072u, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        } else
            if (CODE_UPGRADE_ALG_ECC_P256R1 == verify_alg) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_ecdsa_verify(secp256r1, hash_value, 32, pubkey, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        } else
            if ((CODE_UPGRADE_ALG_SM2 == verify_alg) || (CODE_VERIFY_ALG_SM2 == verify_alg)) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            ret = cpt_sm2_verify((uint8_t *)hash_value, pubkey, signature);
            fid_inc_u32(&g_fw_upgrade_cfi, 4);
        } else
        {
            ret = EHSM_ERR_WRONG_ALGORITHM;
        }
    } else {
        ret = EHSM_ERR_WRONG_FW_TYPE;
    }
    return ret;
}

static uint32_t get_upgrade_signdata_by_alg(
    uint8_t alg, uint8_t *head, ehsm_upgrade_data_st *upgrade_data, uint32_t key_id)
{
    uint32_t ret;

    switch (alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
        (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET],
            alg == CODE_UPGRADE_ALG_RSA2048 ? RSA2048_PUBLIC_K_N_LEN : RSA3072_PUBLIC_K_N_LEN);
        ret = fwverify_get_and_verify_pubkey(&head[IMAGE_PUBLIC_K_OFFSET], upgrade_data->pubkey, alg, key_id);
        break;
    case CODE_UPGRADE_ALG_ECC_P256R1:
    case CODE_UPGRADE_ALG_SM2:
        (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET], 64);
        ret = fwverify_get_and_verify_pubkey(&head[IMAGE_PUBLIC_K_OFFSET], upgrade_data->pubkey, alg, key_id);
        break;

    case CODE_UPGRADE_ALG_AES_CMAC:
    case CODE_UPGRADE_ALG_SM4_CMAC:
        (void)util_memcpy(upgrade_data->signature, &head[SIGNATURE_OFFSET], 16);
        ret = EHSM_ERR_SW_SUCCESS;
        break;

    default:
        ret = EHSM_ERR_INVALID_ALGORITHM;
        break;
    }

    return ret;
}

static uint32_t image_upgrade_asym_ctx_init(uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;

    // sign hash init
    ret = cpt_hash_init(upgrade_ctx->alg_ctx.hash_ctx, HASH_SHA256);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_update(
            upgrade_ctx->alg_ctx.hash_ctx, (uint8_t *)(&head[IMAGE_CODE_SIZE_RAM_OFFSET]), IMAGE_INFO_HASH_SIZE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
            ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // ske dec init
                ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, SKE_ALG_AES, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                    (uint8_t *)(&head[IV_OFFSET]), SKE_NO_PADDING);
                if (EHSM_ERR_SW_SUCCESS != ret) {
                    ret = EHSM_ERR_SKE_WORK_ERROR;
                }
            }
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_HASH_WORK_ERROR;
    }

    return ret;
}

static uint32_t image_upgrade_sm2_ctx_init(uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint8_t hash_value[32];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;

    // sign hash init
    ret = cpt_sm2_getZ(NULL, 0, (uint8_t *)(&head[PUBLIC_K_OFFSET]), hash_value);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_init(upgrade_ctx->alg_ctx.hash_ctx, HASH_SM3);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(upgrade_ctx->alg_ctx.hash_ctx, (uint8_t *)hash_value, 32);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(
                upgrade_ctx->alg_ctx.hash_ctx, (uint8_t *)(&head[IMAGE_CODE_SIZE_RAM_OFFSET]), IMAGE_INFO_HASH_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    // ske dec init
                    ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, SKE_ALG_SM4, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                        (uint8_t *)(&head[IV_OFFSET]), SKE_NO_PADDING);
                    if (EHSM_ERR_SW_SUCCESS != ret) {
                        ret = EHSM_ERR_SKE_WORK_ERROR;
                    }
                }
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
    } else {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t image_upgrade_ske_ctx_init(
    uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx, cpt_ske_alg_e ske_algorithm)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_enc_key;
    uint16_t phy_vrf_key;

    ret = otpkey_get_phyid(upgrade_ctx->verify_key_id, &phy_vrf_key);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_get_phyid(upgrade_ctx->enc_key_id, &phy_enc_key);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cmac_init(upgrade_ctx->alg_ctx.cmac_ctx, ske_algorithm, SKE_VERIFY_MAC, NULL, phy_vrf_key, 16);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cmac_update(
                upgrade_ctx->alg_ctx.cmac_ctx, (uint8_t *)(&head[IMAGE_CODE_SIZE_RAM_OFFSET]), IMAGE_INFO_HASH_SIZE);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                    // ske dec init
                    ret = cpt_ske_cbc_init(upgrade_ctx->ske_ctx, ske_algorithm, SKE_CRYPTO_DECRYPT, NULL, phy_enc_key,
                        (uint8_t *)(&head[IV_OFFSET]), SKE_NO_PADDING);
                    if (EHSM_ERR_SW_SUCCESS != ret) {
                        ret = EHSM_ERR_SKE_WORK_ERROR;
                    }
                } else {
                    ret = EHSM_ERR_SW_SUCCESS;
                }
            } else {
                ret = EHSM_ERR_SKE_WORK_ERROR;
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    }

    return ret;
}

static uint32_t image_upgrade_ctx_init(uint8_t *head, image_alg_upgrade_ctx_st *upgrade_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e ske_algorithm;

    switch (upgrade_ctx->upgrade_alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
    case CODE_UPGRADE_ALG_ECC_P256R1:
        ret = image_upgrade_asym_ctx_init(head, upgrade_ctx);
        break;
    case CODE_UPGRADE_ALG_SM2:
        ret = image_upgrade_sm2_ctx_init(head, upgrade_ctx);
        break;
    case CODE_UPGRADE_ALG_AES_CMAC:
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
        ret = EHSM_ERR_INVALID_ALGORITHM;
        break;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        (void)util_memcpy((uint8_t *)&(upgrade_ctx->upgrade_total_size), (uint8_t *)&head[IMAGE_CODE_SIZE_OFFSET], 4u);
    }

    return ret;
}

static uint32_t image_code_ske_ctx_init(
    image_alg_code_ctx_st *code_ctx, ehsm_image_upgrade_st *image_upgrade, cpt_ske_alg_e ske_algorithm)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t phy_vrf_key;

    ret = otpkey_get_phyid(code_ctx->verify_key_id, &phy_vrf_key);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cmac_init(code_ctx->alg_ctx.cmac_ctx, ske_algorithm, SKE_GENERATE_MAC, NULL, phy_vrf_key, 16);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (image_upgrade->image_size >= VALID_FLAG_OFFSET) {
                ret = cpt_ske_cmac_update(code_ctx->alg_ctx.cmac_ctx, &(image_upgrade->storage[VALID_FLAG_OFFSET]),
                    image_upgrade->image_size - VALID_FLAG_OFFSET);
                if (EHSM_ERR_SW_SUCCESS != ret) {
                    ret = EHSM_ERR_SKE_WORK_ERROR;
                }
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    }
    return ret;
}

static uint32_t image_code_enc_ctx_init(
    image_alg_code_ctx_st *code_ctx, ehsm_image_upgrade_st *image_upgrade, cpt_ske_alg_e ske_algorithm)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t iv[16];
    uint16_t phy_enc_key;

    // get iv
    ret = cpt_get_rand((uint8_t *)iv, 16);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        ret = EHSM_ERR_TRNG_WORK_ERROR;
    }

    // cbc enc init
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_get_phyid(code_ctx->enc_key_id, &phy_enc_key);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_cbc_init(
            code_ctx->ske_ctx, ske_algorithm, SKE_CRYPTO_ENCRYPT, NULL, phy_enc_key, iv, SKE_NO_PADDING);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            (void)util_memcpy(&image_upgrade->storage[IV_OFFSET], iv, 16);
            if (image_upgrade->image_size >= EHSM_CODE_INFO_SIZE) {
                ret = cpt_ske_cbc_update_blocks(code_ctx->ske_ctx, &image_upgrade->storage[EHSM_CODE_INFO_SIZE],
                    &image_upgrade->storage[EHSM_CODE_INFO_SIZE], image_upgrade->image_size - EHSM_CODE_INFO_SIZE);
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    }

    return ret;
}

static uint32_t image_code_ctx_init(image_alg_code_ctx_st *code_ctx, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_INVALID_ALGORITHM;
    cpt_ske_alg_e ske_algorithm;

    switch (code_ctx->verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
    case CODE_VERIFY_ALG_ECC_P256R1:
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
        return ret; // ret is EHSM_ERR_INVALID_ALGORITHM, return immediately
    }

    // cmac init
    switch (code_ctx->verify_alg) {
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        ret = image_code_ske_ctx_init(code_ctx, image_upgrade, ske_algorithm);
        break;
    default:
        ret = EHSM_ERR_SW_SUCCESS;
        break;
    }

    if ((MB_BL_VERIFY_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = image_code_enc_ctx_init(code_ctx, image_upgrade, ske_algorithm);
    }

    // set head_flag
    if (EHSM_ERR_SW_SUCCESS == ret) {
        code_ctx->code_init_ctx_flag = 1;
        (void)util_memcpy(
            (uint8_t *)&(code_ctx->code_total_size), (uint8_t *)&image_upgrade->storage[CODE_SIZE_OFFSET], 4u);
        code_ctx->code_already_size = image_upgrade->image_size - EHSM_CODE_INFO_SIZE;
        code_ctx->code_addr = image_upgrade->host_stor_addr;
    }

    return ret;
}

static uint32_t image_upgrade_ctx_update(image_alg_upgrade_ctx_st *upgrade_ctx, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_INVALID_ALGORITHM;

    switch (upgrade_ctx->upgrade_alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
    case CODE_UPGRADE_ALG_ECC_P256R1:
    case CODE_UPGRADE_ALG_SM2:
        ret = cpt_hash_update(upgrade_ctx->alg_ctx.hash_ctx, image_upgrade->image, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_update_blocks(
                    upgrade_ctx->ske_ctx, image_upgrade->image, image_upgrade->storage, image_upgrade->image_size);
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
        break;
    case CODE_UPGRADE_ALG_AES_CMAC:
    case CODE_UPGRADE_ALG_SM4_CMAC:
        ret = cpt_ske_cmac_update(upgrade_ctx->alg_ctx.cmac_ctx, image_upgrade->image, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_update_blocks(
                    upgrade_ctx->ske_ctx, image_upgrade->image, image_upgrade->storage, image_upgrade->image_size);
            } else {
                ret = EHSM_ERR_SW_SUCCESS;
            }
        }
        break;
    default:
        ret = EHSM_ERR_INVALID_ALGORITHM;
        break;
    }

    return ret;
}

static uint32_t image_code_ctx_update(image_alg_code_ctx_st *code_ctx, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (code_ctx->verify_alg) {
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        ret = cpt_ske_cmac_update(code_ctx->alg_ctx.cmac_ctx, image_upgrade->storage, image_upgrade->image_size);
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
        break;
    default:
        break;
    }

    if ((MB_BL_VERIFY_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) && (EHSM_ERR_SW_SUCCESS == ret)) {
        ret = cpt_ske_cbc_update_blocks(
            code_ctx->ske_ctx, image_upgrade->storage, image_upgrade->storage, image_upgrade->image_size);
    }

    return ret;
}

static uint32_t image_upgrade_ctx_finish(image_alg_upgrade_ctx_st *upgrade_ctx, ehsm_upgrade_data_st *upgrade_data)
{
    uint32_t ret = EHSM_ERR_INVALID_ALGORITHM;
    uint8_t hash_value[32];

    switch (upgrade_ctx->upgrade_alg) {
    case CODE_UPGRADE_ALG_RSA2048:
    case CODE_UPGRADE_ALG_RSA3072:
    case CODE_UPGRADE_ALG_ECC_P256R1:
    case CODE_UPGRADE_ALG_SM2:
        ret = cpt_hash_final(upgrade_ctx->alg_ctx.hash_ctx, hash_value);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                ret = cpt_ske_cbc_final(upgrade_ctx->ske_ctx);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = check_signature(upgrade_ctx->upgrade_alg, hash_value, upgrade_data->signature,
                        upgrade_data->pubkey, upgrade_ctx->image_type);
                }
            } else {
                ret = check_signature(upgrade_ctx->upgrade_alg, hash_value, upgrade_data->signature,
                    upgrade_data->pubkey, upgrade_ctx->image_type);
            }
        }
        break;
    case CODE_UPGRADE_ALG_AES_CMAC:
    case CODE_UPGRADE_ALG_SM4_CMAC:
        fid_inc_u32(&g_fw_upgrade_cfi, 1);
        ret = cpt_ske_cmac_final(upgrade_ctx->alg_ctx.cmac_ctx, upgrade_data->signature);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
            if (IMAGE_DECRYPT_CODE == upgrade_ctx->is_decrypt_code) {
                fid_inc_u32(&g_fw_upgrade_cfi, 1);
                ret = cpt_ske_cbc_final(upgrade_ctx->ske_ctx);
            } else {
                fid_inc_u32(&g_fw_upgrade_cfi, 1);
                ret = EHSM_ERR_SW_SUCCESS;
            }
            fid_inc_u32(&g_fw_upgrade_cfi, 1);
        }
        fid_inc_u32(&g_fw_upgrade_cfi, 1);
        break;
    default:
        ret = EHSM_ERR_INVALID_ALGORITHM;
        break;
    }

    return ret;
}

static uint32_t image_code_ctx_finish(image_alg_code_ctx_st *code_ctx, uint8_t *out)
{
    uint32_t ret;

    switch (code_ctx->verify_alg) {
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        ret = cpt_ske_cmac_final(code_ctx->alg_ctx.cmac_ctx, out);
        break;
    default:
        ret = EHSM_ERR_SW_SUCCESS;
        break;
    }

    if (MB_BL_VERIFY_IMAGE_CODE_PLAIN_NO == code_ctx->code_enc) {
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_ske_cbc_final(code_ctx->ske_ctx);
        }
    }

    return ret;
}

static uint32_t image_upgrade_alg_and_ctx_init(uint8_t *head, image_ctx_st *image_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_id;

    // upgrade alg init
    get_upgrade_alg(image_ctx->upgrade_alg_ctx, image_ctx->upgrade_alg_ctx->image_type);
    if (CODE_UPGRADE_ALG_INVALID == image_ctx->upgrade_alg_ctx->upgrade_alg) {
        ret = EHSM_ERR_WRONG_ALGORITHM;
    } else {
        key_id = fwupd_get_verify_key_id(image_ctx->upgrade_alg_ctx->image_type);
        // upgrade alg ctx_init
        if (EHSM_ERR_SW_SUCCESS == image_upgrade_ctx_init(head, image_ctx->upgrade_alg_ctx)) {
            ret = get_upgrade_signdata_by_alg(
                image_ctx->upgrade_alg_ctx->upgrade_alg, head, image_ctx->upgrade_data, key_id);
        } else {
            ret = EHSM_ERR_WRONG_CONTEXT;
        }
    }

    return ret;
}

static uint32_t image_code_alg_and_ctx_init(ehsm_image_upgrade_st *image_upgrade, image_ctx_st *image_ctx)
{
    uint32_t ret;
    bool_t check_version;

    // verify alg init
    if (image_upgrade->image_size < EHSM_CODE_INFO_SIZE) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        // 从启动固件读取plain flag，如果是1，不对code区加密，否则需要对code区加密
        image_ctx->code_alg_ctx->code_enc = image_upgrade->storage[PLAIN_FLAG_OFFSET];
        image_ctx->code_alg_ctx->code_type = image_upgrade->storage[IMAGE_TYPE_OFFSET];
        // code head
        get_code_alg(image_ctx->code_alg_ctx);
        if (CODE_VERIFY_ALG_INVALID == image_ctx->code_alg_ctx->verify_alg) {
            ret = EHSM_ERR_WRONG_ALGORITHM;
        } else {
            if (MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == image_ctx->code_alg_ctx->code_type) {
                otpdata_get_hsm_ver_cnt(image_ctx->code_data->version);
                check_version = true;
            } else if (MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH == image_ctx->code_alg_ctx->code_type) {
                // patch does not have version counter
                check_version = false;
            } else {
                otpdata_get_soc_ver_cnt(image_ctx->code_data->version);
                check_version = true;
            }
            ret = fwverify_check_fw_header(image_upgrade->storage, check_version, image_ctx->code_data->version);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = image_code_ctx_init(image_ctx->code_alg_ctx, image_upgrade);
            }
        }
    }

    return ret;
}

bool_t fwupgrade_header_is_valid(uint8_t *head)
{
    bool_t ret;
    uint32_t valid;

    util_memcpy(&valid, &head[VALID_FLAG_OFFSET], 4);
    if (valid != UPGRADE_VALID_FLAG) {
        ret = false;
    } else {
        ret = true;
    }

    return ret;
}

uint32_t fwupgrade_check_fw_header(uint8_t *header, bool_t check_version, uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (true == fwupgrade_header_is_valid(header)) {
        if (check_version && (false == fwverify_check_ver_cnt(version, (uint8_t *)(&header[VERSION_COUNTER_OFFSET])))) {
            ret = EHSM_ERR_WRONG_VERSION_COUNTER;
        }
    } else {
        ret = EHSM_ERR_INVALID_CODE_FLAG;
    }

    return ret;
}

static uint32_t image_upgrade_init(uint8_t *head, uint32_t head_size, image_ctx_st *image_ctx)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t version[32] = { 0 };

    if (EHSM_CODE_INFO_SIZE != head_size) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        image_ctx->upgrade_alg_ctx->image_type = head[IMAGE_TYPE_OFFSET];

        if (MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW == image_ctx->upgrade_alg_ctx->image_type) {
            otpdata_get_hsm_ver_cnt(version);
        } else if ((MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY == image_ctx->upgrade_alg_ctx->image_type)
            || (MB_BL_FW_UPGRADE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY == image_ctx->upgrade_alg_ctx->image_type)) {
            otpdata_get_soc_ver_cnt(version);
        } else if (MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH == image_ctx->upgrade_alg_ctx->image_type) {
            // patch no need to check version
        } else {
            ret = EHSM_ERR_WRONG_FW_TYPE;
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = fwupgrade_check_fw_header(
                head, MB_BL_FW_UPGRADE_IMAGE_TYPE_PATCH != image_ctx->upgrade_alg_ctx->image_type, version);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = image_upgrade_alg_and_ctx_init(head, image_ctx);
    }

    return ret;
}

static uint32_t image_upgrade_update(ehsm_image_upgrade_st *image_upgrade, image_ctx_st *image_ctx)
{
    uint32_t ret;

    if (EHSM_ERR_SW_SUCCESS == image_upgrade_ctx_update(image_ctx->upgrade_alg_ctx, image_upgrade)) {
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
    } else {
        ret = EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t image_upgrade_finish(image_ctx_st *image_ctx)
{
    uint32_t ret;

    if (EHSM_ERR_SW_SUCCESS == image_upgrade_ctx_finish(image_ctx->upgrade_alg_ctx, image_ctx->upgrade_data)) {
        if (IMAGE_ANALYSIS_CODE == image_ctx->code_alg_ctx->is_analysis_code) {
            if (EHSM_ERR_SW_SUCCESS
                == image_code_ctx_finish(image_ctx->code_alg_ctx, image_ctx->code_data->signature)) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_SKE_WORK_ERROR;
            }
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_DATA_CHECK_ERROR;
    }

    return ret;
}

static uint32_t write_code_to_storage(raddr_t addr, uint8_t *in, uint32_t bytes, uint8_t image_type)
{
    uint32_t ret;

    if (image_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
        ret = mmap_write_remote_data(addr, in, bytes);
    } else {
        ret = mmap_write_remote_data(addr, in, bytes);
    }

    return ret;
}

static void fwupd_init_stor_addr(void)
{
}

static void fwupd_update_stor_addr(raddr_t addr)
{
    UNUSED(addr);
}

static void fwupd_get_stor_addr(raddr_t *addr, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, uint8_t image_type)
{
    if (image_type == MB_BL_FW_UPGRADE_IMAGE_TYPE_EHSM_FW) {
        *addr = cmd_upgrade->storage_addr;
    } else {
        *addr = cmd_upgrade->storage_addr;
    }
}

static void fwupd_reset_stor_addr(void)
{
}

static uint32_t bpl_image_upgrade(ehsm_image_upgrade_st *image_upgrade, image_ctx_st *image_ctx)
{
    uint32_t ret;

    switch (image_upgrade->process_mode) {
    case MB_START:
        ret = image_upgrade_init(image_upgrade->image, image_upgrade->image_size, image_ctx);
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

    return ret;
}

static uint32_t image_upgrade_flow_init(
    image_ctx_st *ctx, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;

    image_upgrade->process_mode = MB_START;
    image_upgrade->image_size = cmd_upgrade->image_size;
    image_upgrade->host_stor_addr = cmd_upgrade->storage_addr;

    ret = mmap_read_remote_data((uint8_t *)image_upgrade->image, cmd_upgrade->image_addr, IMAGE_INFO_SIZE);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        ctx->code_alg_ctx->code_enc = image_upgrade->image[PLAIN_FLAG_OFFSET];
        ret = bpl_image_upgrade(image_upgrade, ctx);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        fwupd_init_stor_addr();
    }

    return ret;
}

static uint32_t upgrade_update_flow_process_data(
    image_ctx_st *ctx, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;

    ret = mmap_read_remote_data((uint8_t *)image_upgrade->image, cmd_upgrade->image_addr, image_upgrade->image_size);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        cmd_upgrade->image_addr += image_upgrade->image_size;
        image_upgrade->host_stor_addr = cmd_upgrade->storage_addr;
        ret = bpl_image_upgrade(image_upgrade, ctx);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (IMAGE_ANALYSIS_CODE == ctx->code_alg_ctx->is_analysis_code) {
            if (EHSM_ERR_SW_SUCCESS
                == (write_code_to_storage(cmd_upgrade->storage_addr, (uint8_t *)image_upgrade->storage,
                    image_upgrade->image_size, ctx->upgrade_alg_ctx->image_type))) {
                cmd_upgrade->storage_addr += image_upgrade->image_size;
            } else {
                ret = EHSM_ERR_INVALID_ADDRESS;
            }
        }
    }

    return ret;
}

static uint32_t image_upgrade_flow_update(
    image_ctx_st *ctx, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t block_cnt;
    uint32_t left_block_size;
    uint32_t read_size;
    uint32_t input_image_size = cmd_upgrade->image_size;

    fwupd_get_stor_addr(&cmd_upgrade->storage_addr, cmd_upgrade, ctx->upgrade_alg_ctx->image_type);
    image_upgrade->process_mode = MB_UPDATE;
    block_cnt = input_image_size / IMAGE_UPGRADE_BLOCK_SIZE;
    left_block_size = input_image_size % IMAGE_UPGRADE_BLOCK_SIZE;
    read_size = IMAGE_UPGRADE_BLOCK_SIZE;

    if ((ctx->upgrade_alg_ctx->upgrade_already_size + input_image_size) > ctx->upgrade_alg_ctx->upgrade_already_size) {
        ctx->upgrade_alg_ctx->upgrade_already_size += input_image_size;
        if (0U != (left_block_size & 0x0FU)) {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    } else {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
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

        if (ret == EHSM_ERR_SW_SUCCESS) {
            fwupd_update_stor_addr(cmd_upgrade->storage_addr);
        }
    }

    return ret;
}

static uint32_t image_upgrade_flow_finish(
    image_ctx_st *ctx, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

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
            ret = bpl_image_upgrade(image_upgrade, ctx);
        }
    } else {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    }

    return ret;
}

static uint32_t image_upgrade_flow_one_pass(
    image_ctx_st *ctx, mb_cmd_bl_fw_upgrade_st *cmd_upgrade, ehsm_image_upgrade_st *image_upgrade)
{
    uint32_t ret;
    uint32_t left_block_size = 0;

    fid_set_u32(&g_fw_upgrade_cfi, FW_UPD_CFI_INIT_VAL);
    if (cmd_upgrade->image_size >= IMAGE_INFO_SIZE) {
        left_block_size = cmd_upgrade->image_size - IMAGE_INFO_SIZE;
    }
    cmd_upgrade->image_size = IMAGE_INFO_SIZE;

    // init
    ret = image_upgrade_flow_init(ctx, cmd_upgrade, image_upgrade);
    if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        // update
        cmd_upgrade->image_addr += IMAGE_INFO_SIZE;
        cmd_upgrade->image_size = left_block_size;
        ret = image_upgrade_flow_update(ctx, cmd_upgrade, image_upgrade);
        if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
            ret = image_upgrade_flow_finish(ctx, cmd_upgrade, image_upgrade);
        }
    }
    if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        if (fid_get_u32(&g_fw_upgrade_cfi) != FW_UPD_CFI_FINAL_VAL) {
            fid_panic();
        }
    }
    return ret;
}

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t fwupd_image_upgrade(cmd_packet_st *packet, uint8_t is_only_verify)
{
    uint32_t ret;
    uint8_t block_buf[IMAGE_UPGRADE_BLOCK_SIZE];
    mb_cmd_bl_fw_upgrade_st cmd_upgrade[1];
    ehsm_image_upgrade_st image_upgrade[1];
    image_ctx_st *image_ctx = g_ctx;

    if (NULL == packet) {
        return EHSM_ERR_PARAM_ERROR;
    }

    util_memset(cmd_upgrade, 0, sizeof(mb_cmd_bl_fw_upgrade_st));
    (void)util_memcpy((uint8_t *)cmd_upgrade, &packet->cmd_data, sizeof(mb_cmd_bl_fw_upgrade_st));
    if ((0 == cmd_upgrade->image_addr) || (0 == cmd_upgrade->storage_addr)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else if (0 == cmd_upgrade->image_size) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        (void)util_memset(block_buf, 0, sizeof(block_buf));
        image_upgrade->image = block_buf;
        image_upgrade->storage = block_buf; // in-place operation: underlying cpt_ske_cbc supports input == output
        if ((MB_START == cmd_upgrade->process_mode) || (MB_ONE_PASS == cmd_upgrade->process_mode)) {
            image_ctx_init();
            if (1u == is_only_verify) {
                image_ctx->code_alg_ctx->is_analysis_code = 0;
                image_ctx->upgrade_alg_ctx->is_decrypt_code = 0;
            } else {
                image_ctx->code_alg_ctx->is_analysis_code = IMAGE_ANALYSIS_CODE;
                image_ctx->upgrade_alg_ctx->is_decrypt_code = IMAGE_DECRYPT_CODE;
            }
        }

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

        if ((EHSM_ERR_SW_SUCCESS != ret) || (MB_FINISH == cmd_upgrade->process_mode)
            || (MB_ONE_PASS == cmd_upgrade->process_mode)) {
            image_ctx_finish();
            fwupd_reset_stor_addr();
        }
    }

    return ret;
}
