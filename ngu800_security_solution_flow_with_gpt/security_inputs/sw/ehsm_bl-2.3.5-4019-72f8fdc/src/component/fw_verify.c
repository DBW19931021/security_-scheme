/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "fw_verify.h"
#include "config.h"
#include "mmap.h"
#include "types.h"
#include "mb.h"
#include "crypto_lib_api.h"
#include "mbcmd_parser.h"
#include "secure_boot.h"
#include "otp_key.h"
#include "otp_data.h"
#include "util.h"
#include "kmu_driver.h"
#include "sysreg.h"
#include "flash.h"
#include "../crypto_include/crypto_common/utility_sec.h"
#include "debug.h"
#include "fid.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/* 固件校验成功Version counter 存放在dram中，由ehsm固件来更新到OTP中*/
#define DRAM_BASE_ADDR 0x20000000U
#define SOC_VERSION_COUNTER_ADDR \
    (DRAM_BASE_ADDR + OTP_DATA_COPY_SIZE) /* SOC version counter 存放地址，前OTP_DATA_COPY_SIZE字节存放OTP数据(verison counter以及之前的数据)*/
#define EHSM_VERSION_COUNTER_ADDR (SOC_VERSION_COUNTER_ADDR + OTP_VERSION_LENGTH + 4U) /* ehsm version counter 存放地址*/
#define VERSION_COUNTER_VALID     0xA55A5AA5

#define FW_CFI_INIT_VAL  1000
#define FW_CFI_FINAL_VAL 1009
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

#define GET_PATCH_SIZE() 0

static fid_u32_t g_fw_verify_cfi = FID_U32_VAL(0);
/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t fwverify_check_code_sign(raddr_t code_addr, bool_t code_in_soc_ram, uint32_t image_size,
    uint32_t verify_alg, uint8_t *header, uint32_t key_id);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/

bool_t fwverify_header_is_valid(uint8_t *head)
{
    bool_t ret;
    uint32_t valid;

    util_memcpy(&valid, &head[VALID_FLAG_OFFSET], 4);
    if (valid != CODE_VALID_FLAG) {
        ret = false;
    } else {
        ret = true;
    }

    return ret;
}

#define KEY_ID_TO_LEVEL(key_id) ((key_id) < EHSM_OTP_SOC_DEBUG_KEY_ID ? K_LEVEL_1 : K_LEVEL_2)

static uint32_t fwverify_get_otpkey_phyid(uint32_t key_id, uint16_t *phy_id)
{
    // key type for getting phyid must be ske
    uint32_t ret = otpkey_check_usage(key_id, K_USAGE_SKE, KEY_ID_TO_LEVEL(key_id), KEY_USAGE_NONE);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_get_phyid(key_id, phy_id);
    }
    return ret;
}

static uint32_t fwverify_read_otpkey_data(uint32_t key_id, uint8_t *buf)
{
    // key type for reading data must be public key hash
    uint32_t ret = otpkey_check_usage(key_id, K_USAGE_HASH, KEY_ID_TO_LEVEL(key_id), KEY_USAGE_NONE);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_read_data(key_id, buf);
    }
    return ret;
}

uint32_t fwverify_check_fw_header(uint8_t *header, uint8_t check_version, uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (true == fwverify_header_is_valid(header)) {
        if (check_version == MB_BL_VERIFY_IMAGE_CHECK_VERSION_ON) {
            if (false == fwverify_check_ver_cnt(version, (uint8_t *)(&header[VERSION_COUNTER_OFFSET]))) {
                ret = EHSM_ERR_WRONG_VERSION_COUNTER;
            }
        }
    } else {
        ret = EHSM_ERR_INVALID_CODE_FLAG;
    }

    return ret;
}

static cpt_ske_alg_e fwverify_get_dec_alg(uint32_t verify_alg)
{
    cpt_ske_alg_e dec_alg;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_SM2:
    case CODE_VERIFY_ALG_SM4_CMAC:
        dec_alg = SKE_ALG_SM4;
        break;
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
    case CODE_VERIFY_ALG_ECC_P256R1:
    case CODE_VERIFY_ALG_AES_CMAC:
        dec_alg = SKE_ALG_AES;
        break;
    default:
        dec_alg = SKE_ALG_SM4;
        break;
    }

    return dec_alg;
}

static uint32_t fwverify_verify_pubkey(cpt_hash_alg_e hash_alg, uint8_t *pubkey, uint32_t pubkey_len, uint32_t key_id)
{
    uint32_t calc_value[OTP_KEY_MAX_SIZE];
    uint32_t pubkey_hash[OTP_KEY_MAX_SIZE];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (EHSM_ERR_SW_SUCCESS != cpt_hash(hash_alg, pubkey, pubkey_len, (uint8_t *)calc_value)) {
        (void)util_memset(calc_value, 0, sizeof(calc_value));
        ret = EHSM_ERR_HASH_WORK_ERROR;
    } else {
        ret = fwverify_read_otpkey_data(key_id, (uint8_t *)pubkey_hash);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = util_data_sec_double_check((uint8_t *)calc_value, (uint8_t *)pubkey_hash, OTP_KEY_MAX_SIZE,
            EHSM_ERR_WRONG_PUBKEY, EHSM_ERR_SW_SUCCESS);
    }

    return ret;
}

static uint32_t fwverify_check_rsa_signature(uint8_t *hash_value, uint8_t *signature, uint8_t *pubkey, uint32_t n_bits)
{
    volatile uint32_t ret = EHSM_ERR_SW_SUCCESS; // add volatile avoid optimization double check

    fid_inc_u32(&g_fw_verify_cfi, 1);
    ret = cpt_rsa_ssa_pss_verify_by_msg_digest(RSA_SIGNATURE_HASH, RSA_SIGNATURE_HASH, -1, (uint8_t *)hash_value,
        pubkey, RSA_PUBLIC_K_E_LEN * 8, (uint8_t *)&pubkey[RSA_PUBLIC_K_E_LEN], n_bits, signature);
    fid_inc_u32(&g_fw_verify_cfi, 1);
    if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        ret = EHSM_ERR_FW_VERIFY_FAILED;
    }
    fid_inc_u32(&g_fw_verify_cfi, 5);

    return ret;
}

static uint32_t fwverify_check_ecc_p256r1_signature(uint8_t *hash_value, uint8_t *signature, uint8_t *pubkey)
{
    log_debug_hex("hash_value", hash_value, 32);
    log_debug_hex("signature", signature, 64);
    log_debug_hex("pubkey", pubkey, 64);

    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    fid_inc_u32(&g_fw_verify_cfi, 1);
    ret = cpt_ecdsa_verify((const eccp_curve_t *)secp256r1, hash_value, /* sha256 size */ 32, pubkey, signature);
    fid_inc_u32(&g_fw_verify_cfi, 1);
    if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        ret = EHSM_ERR_FW_VERIFY_FAILED;
    }
    fid_inc_u32(&g_fw_verify_cfi, 5);

    return ret;
}

static uint32_t fwverify_check_sm2_signature(uint8_t *hash_value, uint8_t *signature, uint8_t *pubkey)
{
    volatile uint32_t ret = EHSM_ERR_SW_SUCCESS; // add volatile avoid optimization double check

    fid_inc_u32(&g_fw_verify_cfi, 1);
    ret = cpt_sm2_verify((uint8_t *)hash_value, pubkey, signature);
    fid_inc_u32(&g_fw_verify_cfi, 1);
    if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        ret = EHSM_ERR_FW_VERIFY_FAILED;
    }
    fid_inc_u32(&g_fw_verify_cfi, 5);

    return ret;
}

static uint32_t fwverify_check_signature(uint32_t verify_alg, uint8_t *hash_value, uint8_t *signature, uint8_t *pubkey)
{
    uint32_t ret;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
        ret = fwverify_check_rsa_signature(hash_value, signature, pubkey, 2048);
        break;
    case CODE_VERIFY_ALG_RSA3072:
        ret = fwverify_check_rsa_signature(hash_value, signature, pubkey, 3072);
        break;

    case CODE_VERIFY_ALG_ECC_P256R1:
        ret = fwverify_check_ecc_p256r1_signature(hash_value, signature, pubkey);
        break;

    case CODE_VERIFY_ALG_SM2:
        ret = fwverify_check_sm2_signature(hash_value, signature, pubkey);
        break;
    default:
        ret = EHSM_ERR_WRONG_ALGORITHM;
        break;
    }
    return ret;
}

static void fwverify_save_soc_version_counter(uint8_t *version_in_img)
{
    *(volatile uint32_t *)(SOC_VERSION_COUNTER_ADDR) = VERSION_COUNTER_VALID;
    util_memcpy((uint8_t *)(SOC_VERSION_COUNTER_ADDR + 4), version_in_img, OTP_VERSION_LENGTH);
}

static uint32_t fwverify_copy_data_dma(raddr_t src_addr, raddr_t dst_addr, uint32_t size, uint32_t dma_cfg)
{
    uint32_t in_h = (uint32_t)(src_addr >> 32);
    uint32_t in_l = (uint32_t)(src_addr);
    uint32_t out_h = (uint32_t)(dst_addr >> 32);
    uint32_t out_l = (uint32_t)(dst_addr);
    uint32_t out_size = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if (src_addr != dst_addr) {
        ret = cpt_ske_dma_crypto(SKE_ALG_AES, SKE_MODE_BYPASS, SKE_CRYPTO_DECRYPT, NULL, 0, NULL, SKE_NO_PADDING, in_h,
                  in_l, out_h, out_l, size, &out_size, NULL, dma_cfg)
                == EHSM_ERR_SW_SUCCESS
            ? EHSM_ERR_SW_SUCCESS
            : EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t fwverify_ske_decrypt_dma(raddr_t input_addr, raddr_t output_addr, uint32_t size, uint16_t phy_key_id,
    uint32_t algo, uint8_t *iv, uint32_t dma_cfg)
{
    uint32_t in_h = (uint32_t)(input_addr >> 32);
    uint32_t in_l = (uint32_t)(input_addr);
    uint32_t out_h = (uint32_t)(output_addr >> 32);
    uint32_t out_l = (uint32_t)(output_addr);
    uint32_t out_size = 0;

    uint32_t ret = cpt_ske_dma_crypto(algo, SKE_MODE_CBC, SKE_CRYPTO_DECRYPT, NULL, phy_key_id, iv, SKE_NO_PADDING,
                       in_h, in_l, out_h, out_l, size, &out_size, NULL, dma_cfg)
            == EHSM_ERR_SW_SUCCESS
        ? EHSM_ERR_SW_SUCCESS
        : EHSM_ERR_SKE_WORK_ERROR;

    return ret;
}

static uint32_t fwverify_vry_soc_fw_in_soc_ram(mb_cmd_bl_verify_image_st *cmd)
{
    raddr_t in_addr = cmd->code_addr;
    raddr_t out_addr;
    uint32_t verify_alg;
    uint32_t verify_key;
    cpt_ske_alg_e dec_alg;
    uint32_t ret;
    uint8_t header[EHSM_CODE_INFO_SIZE];
    uint8_t version[OTP_VERSION_LENGTH];
    uint8_t image_type;
    uint8_t code_plain;
    uint8_t naked = 0;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    if (0 == cmd->image_out_addr) {
        return EHSM_ERR_INVALID_ADDRESS;
    }

    if (0 == cmd->code_addr) {
        return EHSM_ERR_INVALID_ADDRESS;
    }

    if (MB_BL_VERIFY_IMAGE_ONLY_COPY_CODE_YES == cmd->only_copy_code) {
        out_addr = cmd->image_out_addr;
    } else {
        out_addr = cmd->image_out_addr + EHSM_CODE_INFO_SIZE;
    }

    ret = mmap_read_remote_data(header, cmd->image_addr, EHSM_CODE_INFO_SIZE);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        naked = header[NAKED_FLAG_OFFSET];
        fid_inc_u32(&g_fw_verify_cfi, 1);
        if (MB_BL_IMAGE_NAKED != naked) {
            ret = otpdata_get_soc_ver_cnt(version);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = fwverify_check_fw_header(header, cmd->check_version, version);
            }
        } else if (!(FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST)
                       || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP))) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else if (!fwverify_header_is_valid(header)) {
            ret = EHSM_ERR_INVALID_CODE_FLAG;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }

    uint32_t enc_key_id;
    if (EHSM_ERR_SW_SUCCESS == ret) {
        code_plain = header[PLAIN_FLAG_OFFSET];
        image_type = header[IMAGE_TYPE_OFFSET];
        if (image_type == MB_BL_VERIFY_IMAGE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
            enc_key_id = EHSM_OTP_SOC_ENCRYPT_KEY_ID;
            verify_key = EHSM_OTP_SOC_FW_VERIFY_KEY_ID;
            verify_alg = sysreg_get_soc_verify_alg();
            dec_alg = fwverify_get_dec_alg(verify_alg);
        } else {
            enc_key_id = EHSM_OTP_EHSM_ENCRYPT_KEY_ID;
            verify_key = EHSM_OTP_EHSM_FW_VERIFY_KEY_ID;
            verify_alg = sysreg_get_fw_verify_alg();
            dec_alg = fwverify_get_dec_alg(verify_alg);
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_BL_IMAGE_NAKED == naked || MB_BL_VERIFY_IMAGE_CODE_PLAIN_YES == code_plain) {
            // just copy data
            ret = fwverify_copy_data_dma(in_addr, out_addr, cmd->image_size - EHSM_CODE_INFO_SIZE, SOC_READ_SOC_WRITE);
        } else {
            uint16_t phy_key_id;
            ret = fwverify_get_otpkey_phyid(enc_key_id, &phy_key_id);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                // decrypt data
                ret = fwverify_ske_decrypt_dma(in_addr, out_addr, cmd->image_size - EHSM_CODE_INFO_SIZE, phy_key_id,
                    dec_alg, &header[IV_OFFSET], SOC_READ_SOC_WRITE);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret && MB_BL_IMAGE_NAKED != naked) {
            ret = fwverify_check_code_sign(out_addr, true, cmd->image_size, verify_alg, header, verify_key);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                fwverify_save_soc_version_counter((uint8_t *)(&header[VERSION_COUNTER_OFFSET]));
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret && MB_BL_VERIFY_IMAGE_ONLY_COPY_CODE_YES != cmd->only_copy_code) {
            // copy header
            ret = fwverify_copy_data_dma(cmd->image_addr, cmd->image_out_addr, EHSM_CODE_INFO_SIZE, SOC_READ_SOC_WRITE);
        }
    }
    if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        // 重新计算naked标志，避免前面的参数判断被攻击绕过
        uint8_t naked2 = header[NAKED_FLAG_OFFSET];
        if (FID_EQ(naked2, MB_BL_IMAGE_NAKED) && (naked == naked2) && (life_cycle <= SYS_STA0_LIFECYCLE_DEVELOP)) {
            // 确实是naked镜像，则配置流程保护变量，避免最后检查流程保护变量失败
            fid_inc_u32(&g_fw_verify_cfi, 7);
        } else if (((naked2 != naked) || (naked == MB_BL_IMAGE_NAKED)) && (life_cycle > SYS_STA0_LIFECYCLE_DEVELOP)) {
            // 不被攻击这些条件不可能满足，直接panic
            fid_panic();
        } else {
            //
        }
    }

    return ret;
}


static void get_signature_by_alg(uint8_t *image_header, uint8_t *signature, uint32_t verify_alg)
{
    uint32_t size;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
        size = 256;
        break;
    case CODE_VERIFY_ALG_RSA3072:
        size = 384;
        break;
    case CODE_VERIFY_ALG_ECC_P256R1:
        size = 64;
        break;

    case CODE_VERIFY_ALG_SM2:
        size = 64;
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
        size = 16;
        break;
    case CODE_VERIFY_ALG_SM4_CMAC:
        size = 16;
        break;
    default:
        size = 0;
        break;
    }

    if (size > 0) {
        util_memcpy(signature, image_header, size);
    }
}

static void fwverify_copy_data(raddr_t addr, uint8_t *data, uint32_t size, bool_t in_soc_ram)
{
    if ((addr != 0) && (data != NULL) && (size != 0)) {
        if (true == in_soc_ram) {
            (void)mmap_read_remote_data(data, addr, size);
        } else {
            util_memcpy(data, (void *)((uint32_t)addr), size);
        }
    }
}

// SM2 path CFI: fwverify_check_sm2_signature adds +7 (1+1+5) to g_fw_verify_cfi
static uint32_t fwverify_sm2_check_code_sign(
    raddr_t code_addr, bool_t code_in_soc_ram, uint32_t image_size, uint8_t *header, uint32_t key_id)
{
    raddr_t verify_addr = code_addr;
    uint32_t in_h = (uint32_t)(verify_addr >> 32);
    uint32_t in_l = (uint32_t)(verify_addr);
    uint32_t ret;
    uint32_t digest_h;
    uint32_t digest_l;
    uint8_t pubkey[65];
    uint32_t code_size;
    uint32_t code_dma_cfg;
    uint8_t signature[64];
    uint8_t signature_orig[64];
    cpt_hash_ctx_st hash_ctx[1];
    uint8_t z[SM2_Z_SIZE];
    uint8_t padding[SM3_BLOCK_BYTE_SIZE];
    cpt_hash_dma_ctx_st dma_hash_ctx[1];
    uint32_t padding_size = 48;

    digest_h = 0;
    digest_l = (uint32_t)(SYS_GEN_REG);
    code_size = image_size - EHSM_CODE_INFO_SIZE;

    get_signature_by_alg(header, signature_orig, CODE_VERIFY_ALG_SM2);
    code_dma_cfg = (true == code_in_soc_ram) ? EHSM_WRITE_SOC_READ : EHSM_READ_EHSM_WRITE;

    ret = fwverify_get_and_verify_pubkey(&header[PUBLIC_K_OFFSET], pubkey, CODE_VERIFY_ALG_SM2, key_id);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_sm2_getZ(NULL, 0, pubkey, z);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_init(hash_ctx, HASH_SM3);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(hash_ctx, (uint8_t *)z, SM2_Z_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(hash_ctx, (uint8_t *)&header[EHSM_CODE_SIZE_RAM_OFFSET], EHSM_CODE_INFO_HASH_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (code_size <= padding_size) {
                fwverify_copy_data(verify_addr, padding, code_size, code_in_soc_ram);
                ret = cpt_hash_update(hash_ctx, (uint8_t *)padding, code_size);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = cpt_hash_final(hash_ctx, signature);
                }
            } else {
                fwverify_copy_data(verify_addr, padding, padding_size, code_in_soc_ram);
                ret = cpt_hash_update(hash_ctx, (uint8_t *)padding, padding_size);
                verify_addr += padding_size;
                code_size -= padding_size;
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    in_h = (uint32_t)(verify_addr >> 32);
                    in_l = (uint32_t)(verify_addr);
                    cpt_hash_ctx_switch(hash_ctx, dma_hash_ctx, HASH_CPU_2_DMA);
                    ret = cpt_hash_dma_final(dma_hash_ctx, in_h, in_l, code_size, digest_h, digest_l, code_dma_cfg);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    util_memcpy(signature, (void *)SYS_GEN_REG, SM2_Z_SIZE);
                }
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = fwverify_check_signature(CODE_VERIFY_ALG_SM2, signature, signature_orig, pubkey);
        } else {
            ret = EHSM_ERR_CALC_HASH;
        }
    }

    return ret;
}

// RSA/ECC path CFI: fwverify_check_rsa_signature/fwverify_check_ecc_p256r1_signature adds +7 (1+1+5) to g_fw_verify_cfi
static uint32_t fwverify_asym_check_code_sign(raddr_t code_addr, bool_t code_in_soc_ram, uint32_t image_size,
    uint8_t *header, uint32_t key_id, uint32_t verify_alg)
{
    raddr_t verify_addr = code_addr;
    uint32_t in_h = (uint32_t)(verify_addr >> 32);
    uint32_t in_l = (uint32_t)(verify_addr);
    uint32_t ret;
    uint32_t digest_h;
    uint32_t digest_l;
    uint8_t pubkey[MAX_PUBKEY_SIZE];
    uint32_t code_size;
    uint32_t code_dma_cfg;
    uint8_t signature[MAX_SIGNATURE_SIZE];
    uint8_t signature_orig[MAX_SIGNATURE_SIZE];
    cpt_hash_ctx_st hash_ctx[1];
    cpt_hash_dma_ctx_st dma_hash_ctx[1];
    uint8_t padding[64];
    uint32_t padding_size = 16;

    digest_h = 0;
    digest_l = (uint32_t)(SYS_GEN_REG);
    code_size = image_size - EHSM_CODE_INFO_SIZE;

    get_signature_by_alg(header, signature_orig, verify_alg);
    code_dma_cfg = (true == code_in_soc_ram) ? EHSM_WRITE_SOC_READ : EHSM_READ_EHSM_WRITE;
    log_debug_hex("header pubkey", &header[PUBLIC_K_OFFSET], 64);

    ret = fwverify_get_and_verify_pubkey(&header[PUBLIC_K_OFFSET], pubkey, verify_alg, key_id);
    log_debug_hex("pubkey1", pubkey, 64);

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_hash_init(hash_ctx, HASH_SHA256);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(hash_ctx, (uint8_t *)&header[EHSM_CODE_SIZE_RAM_OFFSET], EHSM_CODE_INFO_HASH_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (code_size <= padding_size) {
                fwverify_copy_data(verify_addr, padding, code_size, code_in_soc_ram);
                ret = cpt_hash_update(hash_ctx, (uint8_t *)padding, code_size);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    ret = cpt_hash_final(hash_ctx, signature);
                }
            } else {
                fwverify_copy_data(verify_addr, padding, padding_size, code_in_soc_ram);
                ret = cpt_hash_update(hash_ctx, (uint8_t *)padding, padding_size);
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    verify_addr += padding_size;
                    in_h = (uint32_t)(verify_addr >> 32);
                    in_l = (uint32_t)(verify_addr);
                    code_size -= padding_size;
                    cpt_hash_ctx_switch(hash_ctx, dma_hash_ctx, HASH_CPU_2_DMA);
                    ret = cpt_hash_dma_final(dma_hash_ctx, in_h, in_l, code_size, digest_h, digest_l, code_dma_cfg);
                    log_debug_hex("pubkey2", pubkey, 64);
                }
                if (EHSM_ERR_SW_SUCCESS == ret) {
                    util_memcpy(signature, (void *)SYS_GEN_REG, (uint32_t)hash_get_digest_word_len(HASH_SHA256) << 2);
                }
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = fwverify_check_signature(verify_alg, signature, signature_orig, pubkey);
        } else {
            ret = EHSM_ERR_CALC_HASH;
        }
    }

    return ret;
}

// CMAC path CFI: adds +7 (1+1+1+1+1+1+1) to g_fw_verify_cfi
static uint32_t fwverify_sym_check_code_sign(raddr_t code_addr, bool_t code_in_soc_ram, uint32_t image_size,
    uint32_t verify_alg, uint8_t *header, uint32_t key_id)
{
    uint32_t in_h;
    uint32_t in_l;
    uint32_t ret;
    uint32_t digest_h;
    uint32_t digest_l;
    uint32_t code_size;
    uint32_t code_dma_cfg;
    uint8_t signature[256];
    uint8_t signature_orig[256];
    uint16_t phy_key_id;
    cpt_ske_alg_e alg;
    cpt_ske_cmac_dma_ctx_st dma_ctx[1];
    cpt_ske_cmac_ctx_st cpu_ctx[1];
    uint8_t block[16] = { 0x0 };

    digest_h = 0U;
    digest_l = (uint32_t)(SYS_GEN_REG);
    code_size = image_size - EHSM_CODE_INFO_SIZE;

    get_signature_by_alg(header, signature_orig, verify_alg);
    code_dma_cfg = (true == code_in_soc_ram) ? EHSM_WRITE_SOC_READ : EHSM_READ_EHSM_WRITE;

    fid_inc_u32(&g_fw_verify_cfi, 1);
    ret = fwverify_get_otpkey_phyid(key_id, &phy_key_id);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        alg = (verify_alg == CODE_VERIFY_ALG_AES_CMAC) ? SKE_ALG_AES : SKE_ALG_SM4;
        fid_inc_u32(&g_fw_verify_cfi, 1);
        ret = cpt_ske_cmac_init(cpu_ctx, alg, SKE_VERIFY_MAC, NULL, phy_key_id, 16);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_fw_verify_cfi, 1);
            ret = cpt_ske_cmac_update(cpu_ctx, (uint8_t *)&header[EHSM_CODE_SIZE_RAM_OFFSET], EHSM_CODE_INFO_HASH_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_fw_verify_cfi, 1);
            // CPU会缓存最后一个block不计算，需要update一个block，保证header里的数据完整计算
            ret = cpt_ske_cmac_update(cpu_ctx, (uint8_t *)block, 16);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            cpt_cmac_ctx_cpu_2_dma(cpu_ctx, dma_ctx);
            in_h = (uint32_t)(code_addr >> 32);
            in_l = (uint32_t)(code_addr);
            fid_inc_u32(&g_fw_verify_cfi, 1);
            ret = cpt_ske_dma_cmac_update_including_last_block(
                dma_ctx, in_h, in_l, code_size, digest_h, digest_l, NULL, code_dma_cfg);
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy(signature, (void *)SYS_GEN_REG, 16);
            fid_inc_u32(&g_fw_verify_cfi, 1);
            ret = util_data_sec_double_check(
                signature, signature_orig, 16, EHSM_ERR_DATA_CHECK_ERROR, EHSM_ERR_SW_SUCCESS);
            fid_inc_u32(&g_fw_verify_cfi, 1);
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    }
    return ret;
}

static uint32_t fwverify_check_code_sign(raddr_t code_addr, bool_t code_in_soc_ram, uint32_t image_size,
    uint32_t verify_alg, uint8_t *header, uint32_t key_id)
{
    uint32_t ret;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_SM2:
        ret = fwverify_sm2_check_code_sign(code_addr, code_in_soc_ram, image_size, header, key_id);
        break;
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
        ret = fwverify_asym_check_code_sign(code_addr, code_in_soc_ram, image_size, header, key_id, verify_alg);
        break;
    case CODE_VERIFY_ALG_ECC_P256R1:
        ret = fwverify_asym_check_code_sign(code_addr, code_in_soc_ram, image_size, header, key_id, verify_alg);
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        ret = fwverify_sym_check_code_sign(code_addr, code_in_soc_ram, image_size, verify_alg, header, key_id);
        break;
    default:
        ret = EHSM_ERR_WRONG_ALGORITHM;
        break;
    }

    return ret;
}

static void fwverify_save_hsm_version_counter(uint8_t *version_in_img)
{
    *(volatile uint32_t *)(EHSM_VERSION_COUNTER_ADDR) = VERSION_COUNTER_VALID;
    util_memcpy((uint8_t *)(EHSM_VERSION_COUNTER_ADDR + 4), version_in_img, OTP_VERSION_LENGTH);
}

static uint32_t fwverify_vry_hsm_fw_in_soc_ram(mb_cmd_bl_verify_image_st *cmd)
{
    uint32_t dma_mode = 0;
    raddr_t in_addr = cmd->code_addr;
    uint32_t out_addr = 0;
    uint32_t verify_alg;
    uint32_t input_size = cmd->image_size - EHSM_CODE_INFO_SIZE;
    uint8_t *fw_header;
    uint8_t header_buffer[EHSM_CODE_INFO_SIZE];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e dec_alg;
    uint8_t version[OTP_VERSION_LENGTH];
    uint8_t code_plain;
    uint8_t naked = 0;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if (cmd->image_size <= EHSM_CODE_INFO_SIZE || cmd->image_size > CONFIG_BL_IRAM_SIZE - GET_PATCH_SIZE()) {
        ret = EHSM_ERR_INVALID_IMAGE_SIZE;
    }

    if (MB_BL_VERIFY_IMAGE_ONLY_COPY_CODE_YES == cmd->only_copy_code) {
        fw_header = (uint8_t *)header_buffer;
        out_addr = EHSM_CODE_IRAM_BASE;
    } else {
        fw_header = (uint8_t *)EHSM_CODE_IRAM_BASE;
        out_addr = EHSM_CODE_IRAM_BASE + EHSM_CODE_INFO_SIZE;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_read_remote_data(fw_header, cmd->image_addr, EHSM_CODE_INFO_SIZE);
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        naked = fw_header[NAKED_FLAG_OFFSET];
        fid_inc_u32(&g_fw_verify_cfi, 1);
        if (MB_BL_IMAGE_NAKED != naked) {
            ret = otpdata_get_hsm_ver_cnt(version);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = fwverify_check_fw_header(fw_header, cmd->check_version, version);
            }
        } else if (!(FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST)
                       || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP))) {
            // naked image can only run in test mode and develop mode
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else if (!fwverify_header_is_valid(fw_header)) {
            ret = EHSM_ERR_INVALID_CODE_FLAG;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        dma_mode = AHB_DMA_SKE | AHB_WRITE_AXI_READ;

        verify_alg = sysreg_get_fw_verify_alg();
        dec_alg = fwverify_get_dec_alg(verify_alg);
        code_plain = fw_header[PLAIN_FLAG_OFFSET];
        if ((MB_BL_VERIFY_IMAGE_CODE_PLAIN_YES == code_plain) || (MB_BL_IMAGE_NAKED == naked)) {
            // just copy data
            ret = fwverify_copy_data_dma(in_addr, (raddr_t)out_addr, input_size, dma_mode);
        } else {
            // decrypt data
            uint16_t phy_key_id;
            ret = fwverify_get_otpkey_phyid(EHSM_OTP_EHSM_ENCRYPT_KEY_ID, &phy_key_id);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = fwverify_ske_decrypt_dma(
                    in_addr, (raddr_t)out_addr, input_size, phy_key_id, dec_alg, &fw_header[IV_OFFSET], dma_mode);
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret && MB_BL_IMAGE_NAKED != naked) {
            ret = fwverify_check_code_sign(
                (raddr_t)out_addr, false, cmd->image_size, verify_alg, fw_header, EHSM_OTP_EHSM_FW_VERIFY_KEY_ID);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                fwverify_save_hsm_version_counter((uint8_t *)(&fw_header[VERSION_COUNTER_OFFSET]));
            }
        }
    }

    if (FID_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        // 重新计算naked标志，避免前面的参数判断被攻击绕过
        uint8_t naked2 = fw_header[NAKED_FLAG_OFFSET];
        if (FID_EQ(naked2, MB_BL_IMAGE_NAKED) && (naked == naked2) && (life_cycle <= SYS_STA0_LIFECYCLE_DEVELOP)) {
            // 确实是naked镜像，则配置流程保护变量，避免最后检查流程保护变量失败
            fid_inc_u32(&g_fw_verify_cfi, 7);
        } else if (((naked2 != naked) || (naked == MB_BL_IMAGE_NAKED)) && (life_cycle > SYS_STA0_LIFECYCLE_DEVELOP)) {
            // 不被攻击这些条件不可能满足，直接panic
            fid_panic();
        } else {
            //
        }

        if (MB_BL_VERIFY_IMAGE_BOOT_AFTER_VERIFY_ON == cmd->boot_after_verify) {
            secboot_enable_boot_fw(out_addr);
        }
    }

    return ret;
}



static uint32_t fwverify_check_param(mb_cmd_bl_verify_image_st *cmd)
{
    uint32_t ret;

    if (NULL == cmd) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((cmd->check_version != MB_BL_VERIFY_IMAGE_CHECK_VERSION_OFF)
        && (cmd->check_version != MB_BL_VERIFY_IMAGE_CHECK_VERSION_ON)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((cmd->boot_after_verify != MB_BL_VERIFY_IMAGE_BOOT_AFTER_VERIFY_ON)
        && (cmd->boot_after_verify != MB_BL_VERIFY_IMAGE_BOOT_AFTER_VERIFY_OFF)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((0 == cmd->image_addr)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else if (0 == cmd->image_size) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (cmd->image_size <= EHSM_CODE_INFO_SIZE) {
        ret = EHSM_ERR_INVALID_IMAGE_SIZE;
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t fwverify_vry_hsm_fw(mb_cmd_bl_verify_image_st *cmd)
{

    return fwverify_vry_hsm_fw_in_soc_ram(cmd);

}

uint32_t fwverify_vry_soc_fw(mb_cmd_bl_verify_image_st *cmd)
{
    return fwverify_vry_soc_fw_in_soc_ram(cmd);
}

uint32_t fwverify_verify_image(cmd_packet_st *packet)
{
    mb_cmd_bl_verify_image_st cmd[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t image_type = 0xFF;

    fid_set_u32(&g_fw_verify_cfi, FW_CFI_INIT_VAL);
    if (NULL == packet) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memcpy(cmd, &packet->cmd_data, sizeof(mb_cmd_bl_verify_image_st));
        ret = fwverify_check_param(cmd);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (cmd->image_addr == EHSM_INTERNAL_NVM_IMAGE_ADDR_MAGIC) {
                ret = EHSM_ERR_PARAM_ERROR;
            }
            else {
                // 如果cmd->code_addr为0，说明固件头和code不是是分离的, cmd->code_add设置为cmd->image_addr之后1K的位置
                if (0U == cmd->code_addr) {
                    cmd->code_addr = cmd->image_addr + EHSM_CODE_INFO_SIZE;
                }
                (void)mmap_read_remote_data(&image_type, cmd->image_addr + IMAGE_TYPE_OFFSET, 1);
            }

            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (image_type == MB_BL_VERIFY_IMAGE_IMAGE_TYPE_EHSM_FW) {
                    ret = fwverify_vry_hsm_fw(cmd);
                } else if ((image_type == MB_BL_VERIFY_IMAGE_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY)
                    || (image_type == MB_BL_VERIFY_IMAGE_IMAGE_TYPE_SOC_FW_USE_SOC_KEY)) {
                    ret = fwverify_vry_soc_fw(cmd);
                } else {
                    ret = EHSM_ERR_WRONG_FW_TYPE;
                }
            }
        }
    }

    if (FID_EQ(ret, EHSM_ERR_SW_SUCCESS)) {
        if (fid_get_u32(&g_fw_verify_cfi) != FW_CFI_FINAL_VAL) {
            fid_panic();
        }
    }

    return ret;
}

STATIC_ASSERT(CODE_VERIFY_ALG_RSA2048 == CODE_UPGRADE_ALG_RSA2048);
STATIC_ASSERT(CODE_VERIFY_ALG_RSA3072 == CODE_UPGRADE_ALG_RSA3072);
STATIC_ASSERT(CODE_VERIFY_ALG_SM2 == CODE_UPGRADE_ALG_SM2);
STATIC_ASSERT(CODE_VERIFY_ALG_ECC_P256R1 == CODE_UPGRADE_ALG_ECC_P256R1);

uint32_t fwverify_get_and_verify_pubkey(uint8_t *key_head, uint8_t *pubkey, uint32_t verify_alg, uint32_t key_id)
{
    uint32_t pubkey_len;
    cpt_hash_alg_e key_hash;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (verify_alg) {

    case CODE_VERIFY_ALG_RSA2048:
        pubkey_len = RSA2048_PUBLIC_K_LEN;
        key_hash = RSA_PUBLIC_K_HASH;
        break;
    case CODE_VERIFY_ALG_RSA3072:
        pubkey_len = RSA3072_PUBLIC_K_LEN;
        key_hash = RSA_PUBLIC_K_HASH;
        break;

    case CODE_VERIFY_ALG_ECC_P256R1:
        pubkey_len = ECCP256R1_PUBLIC_K_LEN;
        key_hash = ECC_PUBLIC_K_HASH;
        break;

    case CODE_VERIFY_ALG_SM2:
        pubkey_len = SM2_PUBLIC_K_LEN;
        key_hash = SM2_PUBLIC_K_HASH;
        break;
    default:
        ret = EHSM_ERR_WRONG_ALGORITHM;
        pubkey_len = 0;
        key_hash = 0;
        break;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {

        if (CODE_VERIFY_ALG_RSA3072 == verify_alg) {
            // RSA3072 pubkey is stored in Public_Key + 128 and Public_Key_Ext field, see the document
            util_memcpy(pubkey, key_head + 128, RSA_PUBLIC_K_E_LEN);
            util_memcpy(pubkey + RSA_PUBLIC_K_E_LEN, key_head + IMAGE_PUBLIC_K_EXT_OFFSET - IMAGE_PUBLIC_K_OFFSET,
                RSA3072_PUBLIC_K_N_LEN);
        } else
        {
            util_memcpy(pubkey, key_head, pubkey_len);
            log_debug_hex("copied pubkey", pubkey, 64);
        }
        ret = fwverify_verify_pubkey(key_hash, pubkey, pubkey_len, key_id);
    }

    return ret;
}

bool_t fwverify_check_ver_cnt(uint8_t *otp_version_counter, uint8_t *code_version_counter)
{
    uint32_t otp_counter[OTP_VERSION_LENGTH >> 2u];
    uint32_t image_counter[OTP_VERSION_LENGTH >> 2u];
    bool_t ret;
    int32_t cmp;

    util_memcpy(otp_counter, otp_version_counter, OTP_VERSION_LENGTH);
    util_memcpy(image_counter, code_version_counter, OTP_VERSION_LENGTH);

    cmp = uint32_BigNumCmp(image_counter, OTP_VERSION_LENGTH / 4u, otp_counter, OTP_VERSION_LENGTH / 4);
    if (cmp >= 0) {
        fid_delay();
        cmp = uint32_BigNumCmp(image_counter, OTP_VERSION_LENGTH / 4u, otp_counter, OTP_VERSION_LENGTH / 4);
        if (cmp >= 0) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }

    return ret;
}


/**
 *
 */
