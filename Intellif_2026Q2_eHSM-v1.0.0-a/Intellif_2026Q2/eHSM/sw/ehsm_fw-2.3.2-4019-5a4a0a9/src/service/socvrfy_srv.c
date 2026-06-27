/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "socvrfy_srv.h"
#include "../crypto_include/crypto_common/utility_sec.h"
#include "component/crypto_api.h"
#include "component/otp_key.h"
#include "component/otp_map.h"
#include "component/util.h"
#include "driver/kmu_driver.h"
#include "driver/otp/otp_driver.h"
#include "driver/sysreg.h"
#include "fid.h"
#include "mb.h"
#include "mmap.h"
#include "service/hash/hash_srv.h"
#include "types.h"
#include "misc_srv.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define FW_CFI_INIT_VAL  1000
#define FW_CFI_FINAL_VAL 1010

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
static fid_u32_t g_fw_verify_cfi = FID_U32_VAL(0);

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static uint32_t socvrfy_check_code_sign(
    raddr_t image_addr, uint32_t verify_alg, const uint8_t *header, uint32_t key_id);
static uint32_t socvrfy_check_param(const mb_cmd_soc_verify_st *cmd);
static uint32_t socvrfy_data_sec_double_check(
    const uint8_t *data, const uint8_t *expect_data, uint32_t byte_len, uint32_t error_sw, uint32_t expect_sw);
static uint32_t socvrfy_update_otp_ver_cnt(const uint8_t *header);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static bool_t socvrfy_header_is_valid(const uint8_t *header)
{
    bool_t ret;
    uint32_t valid;

    if (NULL == header) {
        ret = false;
    } else {
        util_memcpy(&valid, &header[VALID_FLAG_OFFSET], 4);
        if (valid != CODE_VALID_FLAG) {
            ret = false;
        } else {
            ret = true;
        }
    }
    return ret;
}

static cpt_ske_alg_e socvrfy_get_dec_alg(uint32_t verify_alg)
{
    cpt_ske_alg_e dec_alg = SKE_ALG_AES;

    if ((CODE_VERIFY_ALG_RSA2048 == verify_alg) || (CODE_VERIFY_ALG_RSA3072 == verify_alg)) {
        dec_alg = SKE_ALG_AES;
    } else if (CODE_VERIFY_ALG_SM2 == verify_alg) {
        dec_alg = SKE_ALG_SM4;
    } else if (CODE_VERIFY_ALG_AES_CMAC == verify_alg) {
        dec_alg = SKE_ALG_AES;
    } else if (CODE_VERIFY_ALG_SM4_CMAC == verify_alg) {
        dec_alg = SKE_ALG_SM4;
    } else {
        // do nothing
    }

    return dec_alg;
}

static uint32_t socvrfy_verify_pubkey(
    cpt_hash_alg_e hash_alg, const uint8_t *pubkey, uint32_t pubkey_len, uint32_t key_id)
{
    uint32_t calc_value[OTP_KEY_DATA_SIZE];
    uint32_t pubkey_hash[OTP_KEY_DATA_SIZE];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t tmp[16];

    ret = cpt_hash(hash_alg, pubkey, pubkey_len, (uint8_t *)calc_value);
    if (EHSM_ERR_SW_SUCCESS != ret) {
        (void)util_memset(calc_value, 0, OTP_KEY_DATA_SIZE);
    } else {
        ret = otpkey_read_data(key_id, (uint8_t *)pubkey_hash);
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        (void)cpt_get_rand_fast(tmp, 3U << 2);
        // sleep random number & securely cmp
        uint32_sleep((uint32_t)tmp[0] & 0x1FU, (tmp[0] >> 4));
        if (0u == uint32_cmp_sec(calc_value, pubkey_hash, 8, tmp[0] >> 7)) {
            uint32_sleep((uint32_t)tmp[1] & 0x1FU, (tmp[1] >> 4));
            if (0u == uint32_cmp_sec(calc_value, pubkey_hash, 8, tmp[1] >> 7)) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_WRONG_PUBKEY;
            }
        } else {
            ret = EHSM_ERR_WRONG_PUBKEY;
        }
    }

    return ret;
}

static uint32_t socvrfy_verify_rsa_signature(
    const uint8_t *hash_value, const uint8_t *signature, const uint8_t *pubkey, uint32_t n_bits)
{
    volatile uint32_t ret = EHSM_ERR_SW_SUCCESS; // add volatile avoid optimization double check

    fid_inc_u32(&g_fw_verify_cfi, 1);
    if ((NULL == hash_value) || (NULL == signature) || (NULL == pubkey)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_fw_verify_cfi, 1);
        ret = cpt_rsa_ssa_pss_verify_by_msg_digest(
            RSA_SIGNATURE_HASH, RSA_SIGNATURE_HASH, -1, hash_value, pubkey, 512u, &pubkey[64], n_bits, signature);
        if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
            ret = EHSM_ERR_FW_VERIFY_FAILED;
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }
    fid_inc_u32(&g_fw_verify_cfi, 4);

    return ret;
}

static uint32_t socvrfy_verify_sm2_signature(const uint8_t *hash_value, const uint8_t *signature, const uint8_t *pubkey)
{
    volatile uint32_t ret = EHSM_ERR_SW_SUCCESS; // add volatile avoid optimization double check

    fid_inc_u32(&g_fw_verify_cfi, 1);
    if ((NULL == hash_value) || (NULL == signature) || (NULL == pubkey)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_fw_verify_cfi, 1);
        ret = cpt_sm2_verify(hash_value, pubkey, signature);
        if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
            ret = EHSM_ERR_FW_VERIFY_FAILED;
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }
    fid_inc_u32(&g_fw_verify_cfi, 4);

    return ret;
}

static uint32_t socvrfy_ecc_p256r1_verify_code_sign(
    const uint8_t *hash_value, const uint8_t *signature, const uint8_t *pubkey)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    fid_inc_u32(&g_fw_verify_cfi, 1);
    ret = cpt_ecdsa_verify((const cpt_eccp_curve_st *)secp256r1, hash_value, /* sha256 size */ 32, pubkey, signature);
    fid_inc_u32(&g_fw_verify_cfi, 1);
    if (FID_NOT_EQ(EHSM_ERR_SW_SUCCESS, ret)) {
        ret = EHSM_ERR_FW_VERIFY_FAILED;
    }
    fid_inc_u32(&g_fw_verify_cfi, 5);

    return ret;
}

static uint32_t socvrfy_verify_signature(
    uint32_t verify_alg, const uint8_t *hash_value, const uint8_t *signature, const uint8_t *pubkey)
{
    uint32_t ret;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
        ret = socvrfy_verify_rsa_signature(hash_value, signature, pubkey, 2048);
        break;
    case CODE_VERIFY_ALG_RSA3072:
        ret = socvrfy_verify_rsa_signature(hash_value, signature, pubkey, 3072);
        break;
    case CODE_VERIFY_ALG_SM2:
        ret = socvrfy_verify_sm2_signature(hash_value, signature, pubkey);
        break;
    case CODE_VERIFY_ALG_ECC_P256R1:
        ret = socvrfy_ecc_p256r1_verify_code_sign(hash_value, signature, pubkey);
        break;
    default:
        ret = EHSM_ERR_WRONG_ALGORITHM;
        break;
    }

    return ret;
}

static uint32_t socvrfy_read_and_check_header(uint8_t *header_buff, raddr_t header_addr, uint8_t check_version)
{
    uint32_t life_cycle = sysreg_get_life_cycle();
    uint8_t naked;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t version[32];
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    if ((NULL == header_buff) || (0U == header_addr)) {
        ret = EHSM_ERR_PARAM_ERROR;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_read_remote_data(header_buff, header_addr, EHSM_IMAGE_HEADER_SIZE);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        naked = header_buff[NAKED_FLAG_OFFSET];
        fid_inc_u32(&g_fw_verify_cfi, 1);
        if (MB_FW_IMAGE_NAKED != naked) {
            ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)version, OTP_VERSION_SIZE);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = socvrfy_check_fw_header(header_buff, check_version, version);
            }
        } else if (!(FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST)
                       || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP))) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }

    return ret;
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
            in_l, out_h, out_l, size, &out_size, NULL, dma_cfg);
    }

    return ret;
}

static uint32_t socvrfy_dec_image(
    const mb_cmd_soc_verify_st *cmd, const uint8_t *header, uint16_t dec_phy_key, cpt_ske_alg_e dec_alg)
{
    uint32_t out_h;
    uint32_t out_l;
    uint32_t in_h;
    uint32_t in_l;
    raddr_t in_addr;
    raddr_t out_addr;
    uint32_t out_size;
    uint32_t ret;

    if ((NULL == cmd) || (NULL == header)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        in_addr = cmd->image_addr;
        out_addr = cmd->image_out_addr;
        out_h = (uint32_t)(out_addr >> 32);
        out_l = (uint32_t)(out_addr);

        ret = fwverify_copy_data_dma(in_addr, out_addr, EHSM_IMAGE_HEADER_SIZE, SOC_READ_SOC_WRITE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            in_addr += EHSM_IMAGE_HEADER_SIZE;
            in_h = (uint32_t)(in_addr >> 32);
            in_l = (uint32_t)in_addr;
            out_addr += EHSM_IMAGE_HEADER_SIZE;
            out_h = (uint32_t)(out_addr >> 32);
            out_l = (uint32_t)(out_addr);
            ret = cpt_ske_dma_crypto(dec_alg, SKE_MODE_CBC, SKE_CRYPTO_DECRYPT, NULL, dec_phy_key,
                (const uint8_t *)(&header[IV_OFFSET]), SKE_NO_PADDING, in_h, in_l, out_h, out_l,
                cmd->image_size - EHSM_IMAGE_HEADER_SIZE, (uint32_t *)(&out_size), NULL, SOC_READ_SOC_WRITE);
        }
    }
    return ret;
}

static uint32_t socvrfy_check_verify_key(uint32_t verify_key, uint32_t verify_alg)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t key_level;
    uint8_t alg_type;

    if (EHSM_OTP_EHSM_FW_VERIFY_KEY_ID == verify_key) {
        key_level = OTP_KEY_LEVEL_1;
    } else if (EHSM_OTP_SOC_FW_VERIFY_KEY_ID == verify_key) {
        key_level = OTP_KEY_LEVEL_2;
    } else {
        ret = EHSM_ERR_INVALID_KEY_ID;
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        switch (verify_alg) {
        case CODE_VERIFY_ALG_SM2:
        case CODE_VERIFY_ALG_RSA2048:
        case CODE_VERIFY_ALG_RSA3072:
        case CODE_VERIFY_ALG_ECC_P256R1:
            alg_type = OTP_KEY_ALGO_HASH_TYPE;
            break;
        case CODE_VERIFY_ALG_AES_CMAC:
        case CODE_VERIFY_ALG_SM4_CMAC:
            alg_type = OTP_KEY_ALGO_SKE_TYPE;
            break;
        default:
            ret = EHSM_ERR_WRONG_ALGORITHM;
            break;
        }
    }

    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = otpkey_check_usage(verify_key, alg_type, key_level, KEY_USAGE_NONE);
    }

    return ret;
}

static uint32_t socvrfy_get_alg_and_key(
    uint8_t image_type, uint32_t *verify_key, uint32_t *verify_alg, uint16_t *dec_key, cpt_ske_alg_e *dec_alg)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == verify_key) || (NULL == verify_alg) || (NULL == dec_key) || (NULL == dec_alg)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (image_type == EHSM_IMAGE_TYPE_SOC_FW_USE_SOC_KEY) {
        ret = otpkey_check_usage(EHSM_OTP_SOC_ENCRYPT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_2, KEY_USAGE_NONE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otpkey_get_phyid(EHSM_OTP_SOC_ENCRYPT_KEY_ID, dec_key);
            *verify_key = EHSM_OTP_SOC_FW_VERIFY_KEY_ID;
            *verify_alg = sysreg_get_soc_verify_alg();
            *dec_alg = socvrfy_get_dec_alg(*verify_alg);
        }

    } else if (image_type == EHSM_IMAGE_TYPE_SOC_FW_USE_EHSM_KEY) {
        ret = otpkey_check_usage(EHSM_OTP_EHSM_ENCRYPT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_1, KEY_USAGE_NONE);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otpkey_get_phyid(EHSM_OTP_EHSM_ENCRYPT_KEY_ID, dec_key);
            *verify_key = EHSM_OTP_EHSM_FW_VERIFY_KEY_ID;
            *verify_alg = sysreg_get_fw_verify_alg();
            *dec_alg = socvrfy_get_dec_alg(*verify_alg);
        }
    } else {
        ret = EHSM_ERR_WRONG_FW_TYPE;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = socvrfy_check_verify_key(*verify_key, *verify_alg);
    }

    return ret;
}

static uint32_t socvrfy_update_otp_ver_cnt(const uint8_t *header)
{
    uint32_t ver_in_otp[OTP_VERSION_SIZE / 4U];
    uint32_t ver_in_image[OTP_VERSION_SIZE / 4U];
    uint32_t ret;

    ret = otp_read(OTP_SOC_VERSION_ADDRESS, (uint8_t *)ver_in_otp, OTP_VERSION_SIZE);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        util_memcpy(ver_in_image, &header[VERSION_COUNTER_OFFSET], OTP_VERSION_SIZE);
        /* Randomize timing to prevent fault injection using otp_read bus timing as trigger */
        fid_delay();
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0 == 0
        if (uint32_BigNumCmp(ver_in_image, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) < 0)
#else
        if (uint32_BigNumCmp(ver_in_image, OTP_VERSION_SIZE / 4U, ver_in_otp, OTP_VERSION_SIZE / 4U) > 0)
#endif
        {
            ret = misc_write_otp_data(OTP_SOC_VERSION_ADDRESS, (const uint8_t *)ver_in_image, OTP_VERSION_SIZE);
        }

        fid_inc_u32(&g_fw_verify_cfi, 1U);
    }
    return ret;
}

static uint32_t socvrfy_vry_soc_fw(const mb_cmd_soc_verify_st *cmd)
{
    uint32_t verify_alg = 0U;
    uint32_t verify_key = 0U;
    cpt_ske_alg_e dec_alg;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t header[EHSM_IMAGE_HEADER_SIZE + EHSM_CODE_SIZE_RAM_OFFSET];
    uint16_t phy_dec_key;

    fid_set_u32(&g_fw_verify_cfi, FW_CFI_INIT_VAL);
    ret = socvrfy_check_param(cmd);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = socvrfy_read_and_check_header(header, cmd->image_addr, cmd->check_version);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (MB_FW_IMAGE_NAKED == header[NAKED_FLAG_OFFSET]) {
            ret = fwverify_copy_data_dma(cmd->image_addr, cmd->image_out_addr, cmd->image_size, SOC_READ_SOC_WRITE);
        } else {
            ret = socvrfy_get_alg_and_key(header[IMAGE_TYPE_OFFSET], &verify_key, &verify_alg, &phy_dec_key, &dec_alg);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (EHSM_IMAGE_CODE_PLAIN_YES == header[PLAIN_FLAG_OFFSET]) {
                    ret = fwverify_copy_data_dma(
                        cmd->image_addr, cmd->image_out_addr, cmd->image_size, SOC_READ_SOC_WRITE);
                } else {
                    ret = socvrfy_dec_image(cmd, header, phy_dec_key, dec_alg);
                }
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (FID_EQ(MB_FW_IMAGE_NAKED, header[NAKED_FLAG_OFFSET])) {
                /* Naked image has no signature to be verified */
                fid_inc_u32(&g_fw_verify_cfi, 7U);
            } else {
                ret = socvrfy_check_code_sign(cmd->image_out_addr, verify_alg, header, verify_key);
            }
        }
    }

    /* Update OTP version counter after successful verification */
    if (FID_EQ(ret, EHSM_ERR_SW_SUCCESS)) {
        if (FID_NOT_EQ(MB_FW_IMAGE_NAKED, header[NAKED_FLAG_OFFSET])
            && FID_EQ(MB_SOC_VERIFY_CHECK_VERSION_ON, cmd->check_version)) {
            ret = socvrfy_update_otp_ver_cnt(header);
        } else {
            fid_inc_u32(&g_fw_verify_cfi, 1U);
        }
    }

    if (FID_EQ(ret, EHSM_ERR_SW_SUCCESS)) {
        if (fid_get_u32(&g_fw_verify_cfi) != FW_CFI_FINAL_VAL) {
            fid_panic();
        }
    }

    return ret;
}

static uint32_t socvrfy_get_sign_by_alg(raddr_t image_addr, uint8_t *signature, uint32_t verify_alg)
{
    uint32_t size;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_RSA2048:
        size = 256;
        break;
    case CODE_VERIFY_ALG_RSA3072:
        size = 384;
        break;
    case CODE_VERIFY_ALG_SM2:
        size = 64;
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        size = 16;
        break;
    case CODE_VERIFY_ALG_ECC_P256R1:
        size = 64;
        break;
    default:
        size = 0;
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_read_remote_data(signature, image_addr, size);
    }

    return ret;
}

static uint32_t socvrfy_calc_sm3_header_hash(
    cpt_hash_ctx_st *hash_ctx, const uint8_t *pukkey, raddr_t verify_addr, uint32_t verify_size)
{
    uint8_t z[SM2_Z_SIZE];
    uint8_t padding[SM3_BLOCK_BYTE_SIZE];
    uint32_t ret;

    if ((NULL == hash_ctx) || (NULL == pukkey) || (0U == verify_addr)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = cpt_sm2_getZ(NULL, 0, pukkey, z);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_init(hash_ctx, HASH_SM3);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(hash_ctx, (uint8_t *)z, SM2_Z_SIZE);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mmap_read_remote_data(padding, verify_addr, verify_size);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_update(hash_ctx, padding, verify_size);
        }
    }

    return ret;
}

static uint32_t socvrfy_calc_sm3_code_hash(
    cpt_hash_ctx_st *hash_ctx, raddr_t verify_addr, uint32_t verify_size, uint8_t *signature)
{
    cpt_hash_dma_ctx_st dma_hash_ctx[1];
    uint32_t dma_size;
    uint32_t remain_size;
    uint32_t in_h;
    uint32_t in_l;
    uint32_t digest_h;
    uint32_t digest_l;
    raddr_t cur_addr = verify_addr;
    uint8_t remain[SM3_BLOCK_BYTE_SIZE];
    uint32_t ret;

    if ((NULL == hash_ctx) || (NULL == signature)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cpt_hash_ctx_switch(hash_ctx, dma_hash_ctx, HASH_CPU_2_DMA);
        dma_size = (verify_size / SM3_BLOCK_BYTE_SIZE) * SM3_BLOCK_BYTE_SIZE;
        remain_size = verify_size % SM3_BLOCK_BYTE_SIZE;

        in_h = (uint32_t)(verify_addr >> 32);
        in_l = (uint32_t)(verify_addr);
        digest_h = 0;
        digest_l = (uint32_t)(SYS_GEN_REG);
        if (remain_size == 0U) {
            ret = cpt_hash_dma_final(dma_hash_ctx, in_h, in_l, dma_size, digest_h, digest_l, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                util_memcpy((void *)signature, (void *)SYS_GEN_REG, SM2_Z_SIZE);
            }
        } else {
            ret = cpt_hash_dma_update_blocks(dma_hash_ctx, in_h, in_l, dma_size, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                cur_addr += dma_size;
                ret = mmap_read_remote_data(remain, cur_addr, remain_size);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                cpt_hash_ctx_switch(hash_ctx, dma_hash_ctx, HASH_DMA_2_CPU);
                ret = cpt_hash_update(hash_ctx, (uint8_t *)remain, remain_size);
            }
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = cpt_hash_final(hash_ctx, signature);
            }
        }
    }

    return ret;
}

static uint32_t socvrfy_calc_sm2_e_value(
    raddr_t verify_addr, uint32_t data_size, const uint8_t *pukkey, uint8_t *signature)
{
    uint32_t ret;
    cpt_hash_ctx_st hash_ctx[1];
    raddr_t cur_addr = verify_addr;
    uint32_t hash_size = data_size;
    uint8_t padding_size;

    if ((NULL == pukkey) || (NULL == signature)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        padding_size = SM3_BLOCK_BYTE_SIZE - SM2_Z_SIZE;
        ret = socvrfy_calc_sm3_header_hash(hash_ctx, pukkey, verify_addr, padding_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            cur_addr += padding_size;
            hash_size -= padding_size;
            ret = socvrfy_calc_sm3_code_hash(hash_ctx, cur_addr, hash_size, signature);
        }
    }
    return ret;
}

static uint32_t socvrfy_sm2_verify_code_sign(raddr_t image_addr, const uint8_t *header, uint32_t key_id)
{
    uint8_t signature[64];
    uint8_t signature_orig[64];
    uint32_t code_size;
    uint32_t hash_size;
    uint8_t pukkey[MAX_PUBKEY_SIZE];
    raddr_t verify_addr = image_addr + EHSM_CODE_SIZE_RAM_OFFSET;
    uint32_t ret;

    if ((0U == image_addr) || (NULL == header)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memcpy(&code_size, &header[CODE_SIZE_OFFSET], 4U);
        hash_size = code_size + EHSM_IMAGE_HEADER_HASH_SIZE;

        ret = socvrfy_get_sign_by_alg(image_addr, signature_orig, CODE_VERIFY_ALG_SM2);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = socvrfy_get_and_verify_pubkey(&header[PUBLIC_K_OFFSET], pukkey, CODE_VERIFY_ALG_SM2, key_id);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = socvrfy_calc_sm2_e_value(verify_addr, hash_size, pukkey, signature);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = socvrfy_verify_signature(CODE_VERIFY_ALG_SM2, signature, signature_orig, pukkey);
            }
        }
    }
    return ret;
}

static uint32_t socvrfy_rsa_verify_code_sign(
    raddr_t image_addr, const uint8_t *header, uint32_t key_id, uint32_t verify_alg)
{
    raddr_t verify_addr = image_addr + EHSM_CODE_SIZE_RAM_OFFSET;
    uint32_t in_h = (uint32_t)(verify_addr >> 32);
    uint32_t in_l = (uint32_t)(verify_addr);
    uint32_t ret;
    uint32_t digest_h;
    uint32_t digest_l;
    uint8_t pukkey[512];
    uint32_t code_size;
    uint32_t hash_size;
    uint8_t signature[384];
    uint8_t signature_orig[384];

    if ((NULL == header) || (0U == image_addr)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        digest_h = 0;
        digest_l = (uint32_t)(SYS_GEN_REG);
        util_memcpy(&code_size, &header[CODE_SIZE_OFFSET], 4U);
        hash_size = code_size + EHSM_IMAGE_HEADER_HASH_SIZE;

        ret = socvrfy_get_sign_by_alg(image_addr, signature_orig, verify_alg);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = socvrfy_get_and_verify_pubkey(&header[PUBLIC_K_OFFSET], pukkey, verify_alg, key_id);
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = cpt_hash_dma(HASH_SHA256, in_h, in_l, hash_size, digest_h, digest_l, NULL, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                util_memcpy(
                    (void *)signature, (void *)SYS_GEN_REG, (uint32_t)hash_get_digest_word_len(HASH_SHA256) << 2);
                ret = socvrfy_verify_signature(verify_alg, signature, signature_orig, pukkey);
            }
        }
    }
    return ret;
}

static uint32_t socvrfy_sym_verify_code_sign(
    raddr_t image_addr, uint32_t verify_alg, const uint8_t *header, uint32_t key_id)
{
    raddr_t verify_addr = image_addr + EHSM_CODE_SIZE_RAM_OFFSET;
    uint32_t in_h = (uint32_t)(verify_addr >> 32);
    uint32_t in_l = (uint32_t)(verify_addr);
    uint32_t ret;
    uint32_t digest_h;
    uint32_t digest_l;
    uint32_t code_size;
    uint32_t hash_size;
    uint8_t signature[256];
    uint8_t signature_orig[256];
    uint16_t phy_key_id;
    cpt_ske_alg_e alg;

    fid_inc_u32(&g_fw_verify_cfi, 1);
    if ((NULL == header) || (0U == image_addr)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        fid_inc_u32(&g_fw_verify_cfi, 1);
        digest_h = 0U;
        digest_l = (uint32_t)(SYS_GEN_REG);
        util_memcpy(&code_size, &header[CODE_SIZE_OFFSET], 4U);
        hash_size = code_size + EHSM_IMAGE_HEADER_HASH_SIZE;

        ret = socvrfy_get_sign_by_alg(image_addr, signature_orig, verify_alg);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = otpkey_get_phyid(key_id, &phy_key_id);
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            fid_inc_u32(&g_fw_verify_cfi, 1);
            alg = (verify_alg == CODE_VERIFY_ALG_AES_CMAC) ? SKE_ALG_AES : SKE_ALG_SM4;
            ret = cpt_ske_dma_cmac(
                alg, NULL, phy_key_id, in_h, in_l, hash_size, digest_h, digest_l, 16, NULL, EHSM_WRITE_SOC_READ);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                fid_inc_u32(&g_fw_verify_cfi, 1);
                util_memcpy(signature, (void *)SYS_GEN_REG, 16);
                ret = socvrfy_data_sec_double_check(
                    signature, signature_orig, 16, EHSM_ERR_FW_VERIFY_FAILED, EHSM_ERR_SW_SUCCESS);
            }
        }
        fid_inc_u32(&g_fw_verify_cfi, 1);
    }
    fid_inc_u32(&g_fw_verify_cfi, 1);

    return ret;
}

static uint32_t socvrfy_check_code_sign(raddr_t image_addr, uint32_t verify_alg, const uint8_t *header, uint32_t key_id)
{
    uint32_t ret;

    switch (verify_alg) {
    case CODE_VERIFY_ALG_SM2:
        ret = socvrfy_sm2_verify_code_sign(image_addr, header, key_id);
        break;
    case CODE_VERIFY_ALG_RSA2048:
    case CODE_VERIFY_ALG_RSA3072:
    case CODE_VERIFY_ALG_ECC_P256R1:
        ret = socvrfy_rsa_verify_code_sign(image_addr, header, key_id, verify_alg);
        break;
    case CODE_VERIFY_ALG_AES_CMAC:
    case CODE_VERIFY_ALG_SM4_CMAC:
        ret = socvrfy_sym_verify_code_sign(image_addr, verify_alg, header, key_id);
        break;
    default:
        ret = EHSM_ERR_WRONG_ALGORITHM;
        break;
    }

    return ret;
}

static uint32_t socvrfy_check_param(const mb_cmd_soc_verify_st *cmd)
{
    uint32_t ret;

    if (NULL == cmd) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((cmd->check_version != MB_SOC_VERIFY_CHECK_VERSION_OFF)
        && (cmd->check_version != MB_SOC_VERIFY_CHECK_VERSION_ON)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if ((0U == cmd->image_addr) || (0U == cmd->image_out_addr)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else if (0U == cmd->image_size) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        ret = EHSM_ERR_SW_SUCCESS;
    }

    return ret;
}

static uint32_t socvrfy_data_sec_double_check(
    const uint8_t *data, const uint8_t *expect_data, uint32_t byte_len, uint32_t error_sw, uint32_t expect_sw)
{
    uint32_t ret;
    uint32_t cmp_data[64];
    uint32_t exp_data[64];
    uint8_t rand_data[16];

    (void)util_memcpy((uint8_t *)cmp_data, data, byte_len);
    (void)util_memcpy((uint8_t *)exp_data, expect_data, byte_len);

    (void)cpt_get_rand_fast(rand_data, 3u << 2);

    // sleep rand number & securely cmp
    uint32_sleep((uint32_t)rand_data[0] & 0x1Fu, (rand_data[0] >> 4));
    if (0u != uint32_cmp_sec(cmp_data, exp_data, byte_len >> 2, rand_data[0] >> 7)) {
        ret = error_sw;
    } else {
        uint32_sleep((uint32_t)rand_data[1] & 0x1Fu, (rand_data[1] >> 4));
        if (0u != uint32_cmp_sec(cmp_data, exp_data, byte_len >> 2, rand_data[1] >> 7)) {
            ret = error_sw;
        } else {
            ret = expect_sw;
        }
    }

    return ret;
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t socvrfy_srv_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    mb_cmd_soc_verify_st cmd[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memcpy(cmd, req_data, sizeof(mb_cmd_soc_verify_st));
        ret = socvrfy_vry_soc_fw(cmd);
    }

    return ret;
}

uint32_t socvrfy_get_and_verify_pubkey(const uint8_t *key_head, uint8_t *pukkey, uint32_t verify_alg, uint32_t key_id)
{
    uint32_t pubkey_len;
    cpt_hash_alg_e key_hash;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == key_head) || (NULL == pukkey)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (verify_alg) {
        case CODE_VERIFY_ALG_RSA2048:
            pubkey_len = RSA2048_PUBLIC_K_LEN;
            key_hash = RSA_PUBLIC_K_HASH;
            break;
        case CODE_VERIFY_ALG_RSA3072:
            pubkey_len = RSA3072_PUBLIC_K_LEN;
            key_hash = RSA_PUBLIC_K_HASH;
            break;
        case CODE_VERIFY_ALG_SM2:
            pubkey_len = SM2_PUBLIC_K_LEN;
            key_hash = SM2_PUBLIC_K_HASH;
            break;
        case CODE_VERIFY_ALG_ECC_P256R1:
            pubkey_len = ECCP256R1_PUBLIC_K_LEN;
            key_hash = ECC_PUBLIC_K_HASH;
            break;
        default:
            ret = EHSM_ERR_PARAM_ERROR;
            break;
        }
        if (EHSM_ERR_SW_SUCCESS == ret) {
            if (CODE_VERIFY_ALG_RSA3072 == verify_alg) {
                // RSA3072 pubkey is stored in Public_Key + 128 and Public_Key_Ext field, see the document
                util_memcpy(pukkey, &key_head[128], RSA_PUBLIC_K_E_LEN);
                util_memcpy(&pukkey[RSA_PUBLIC_K_E_LEN], &key_head[IMAGE_PUBLIC_K_EXT_OFFSET - IMAGE_PUBLIC_K_OFFSET],
                    RSA3072_PUBLIC_K_N_LEN);
            } else {
                util_memcpy(pukkey, key_head, pubkey_len);
            }
            ret = socvrfy_verify_pubkey(key_hash, pukkey, pubkey_len, key_id);
        }
    }

    return ret;
}

uint32_t socvrfy_check_fw_header(const uint8_t *header, uint8_t check_version, const uint8_t *version)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == header) || (NULL == version)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        if (true == socvrfy_header_is_valid(header)) {
            if (check_version == MB_SOC_VERIFY_CHECK_VERSION_ON) {
                if (false == socvrfy_check_ver_cnt(version, (const uint8_t *)(&header[VERSION_COUNTER_OFFSET]))) {
                    ret = EHSM_ERR_WRONG_VERSION_COUNTER;
                }
            }
        } else {
            ret = EHSM_ERR_INVALID_CODE_FLAG;
        }
    }
    return ret;
}

bool_t socvrfy_check_ver_cnt(const uint8_t *otp_version_counter, const uint8_t *code_version_counter)
{
    uint32_t otp_counter[OTP_VERSION_SIZE / 4u];
    uint32_t image_counter[OTP_VERSION_SIZE / 4u];
    bool_t ret;
    int32_t cmp;
    uint32_t word_size = OTP_VERSION_SIZE / 4u;

    if ((NULL == otp_version_counter) || (NULL == code_version_counter)) {
        ret = false;
    } else {
        util_memcpy(otp_counter, otp_version_counter, OTP_VERSION_SIZE);
        util_memcpy(image_counter, code_version_counter, OTP_VERSION_SIZE);

#if !CONFIG_EHSM_OTP_DEFAULT_BIT_0
        for (uint32_t i = 0; i < word_size; i++) {
            otp_counter[i] = ~otp_counter[i];
            image_counter[i] = ~image_counter[i];
        }
#endif
        cmp = uint32_BigNumCmp(image_counter, word_size, otp_counter, word_size);
        if (cmp >= 0) {
            cmp = uint32_BigNumCmp(image_counter, word_size, otp_counter, word_size);
            if (cmp >= 0) {
                ret = true;
            } else {
                ret = false;
            }
        } else {
            ret = false;
        }
    }

    return ret;
}
/**
 *
 */
