#ifndef CRYPTO_UTIL_H
#define CRYPTO_UTIL_H

/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include "mb.h"
#include <ske/ske.h>
#include <pke/sm2.h>
#include <trng/trng.h>
#include <hash_hmac/hash.h>
#include "kms.h"
#include "driver/reg_lock.h"
#include "schedule/expt_det.h"
#include "component/crypto_api.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
//TODO: Add the MACRO to a united header
#define SYS_GEN_REG                     (0x300F0800U)

#define CRYPTO_HASH_USING_AHB_DMA_REG_VAL       (0x1U)

#define MB_DIR_VALID_VALUE_MAX                  (1U)

#define SYM_SP_KEY_MAZ_SZ                       (32U)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
#ifdef CONFIG_UNIT_TEST
static uint32_t tb_reg[1];
static inline volatile uint32_t* crypto_get_ahb_dma_cfg_reg(void)
{
    return (volatile uint32_t *)tb_reg;
}
#else
static inline volatile uint32_t* crypto_get_ahb_dma_cfg_reg(void)
{
    //TODO: Add the MACRO of this address
    return (volatile uint32_t *)(0x30006000U);
}
#endif

static inline uint32_t crypto_using_ahb_dma(uint32_t cfg_val)
{
    //TODO: Add the lock mechanism
    volatile uint32_t *dma_cfg_reg = crypto_get_ahb_dma_cfg_reg();
    uint32_t origin_val = *dma_cfg_reg;
    sysreg_unlock_reg(SYS_REG_BASE);
    *dma_cfg_reg = cfg_val;
    sysreg_lock_reg(SYS_REG_BASE);
    if (*dma_cfg_reg != cfg_val) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
    return origin_val; 
}

static inline void crypto_release_ahb_dma(uint32_t origin_val)
{
    //TODO: Add the lock mechanism
    volatile uint32_t *dma_cfg_reg = crypto_get_ahb_dma_cfg_reg();
    sysreg_unlock_reg(SYS_REG_BASE);
    *dma_cfg_reg = origin_val;
    sysreg_lock_reg(SYS_REG_BASE);
    if (*dma_cfg_reg != origin_val) {
        (void)expt_det_add_error(FW_ERROR_REG_CFG_FAILED);
    }
}

static inline uint32_t cmd_check_process_mode(uint8_t process_mode)
{
    uint32_t ret;
    if ((MB_START == process_mode) || (MB_UPDATE == process_mode) || (MB_STREAMSTART == process_mode) ||
        (MB_FINISH == process_mode) || (MB_ONE_PASS == process_mode))
    {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static inline uint32_t cmd_check_process_mode_and_msg_type(uint8_t process_mode, uint8_t msg_type)
{
    uint32_t ret;
    if ((MB_START == process_mode) || (MB_UPDATE == process_mode) || (MB_STREAMSTART == process_mode) ||
        (MB_FINISH == process_mode) || (MB_ONE_PASS == process_mode))
    {
        /*Only one-pass mode support message digest*/
        if ((MB_MSG_TYPE_DIGEST == msg_type) && (MB_ONE_PASS != process_mode))
        {
            ret = EHSM_ERR_PARAM_ERROR;
        }
        else
        {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }
    else
    {
        ret = EHSM_ERR_PARAM_ERROR;
    }
    return ret;
}

static inline uint32_t cmd_check_direction(uint8_t dir)
{
    uint32_t ret;
    if (dir > MB_DIR_VALID_VALUE_MAX)
    {
        ret = EHSM_ERR_INVALID_DIR;
    }
    else
    {
        ret = EHSM_ERR_SW_SUCCESS;
    }
    return ret;
}

static inline cpt_hash_alg_e get_lib_hash_alg(uint8_t alg)
{
    cpt_hash_alg_e lib_alg;
    switch(alg)
    {
        case MB_HASH_ALGORITHM_SM3:
        {
            lib_alg = HASH_SM3;
            break;
        }
        case MB_HASH_ALGORITHM_MD5:
        {
            lib_alg = HASH_MD5;
            break;
        }
        case MB_HASH_ALGORITHM_SHA256:
        {
            lib_alg = HASH_SHA256;
            break;
        }
        case MB_HASH_ALGORITHM_SHA384:
        {
            lib_alg = HASH_SHA384;
            break;
        }
        case MB_HASH_ALGORITHM_SHA512:
        {
            lib_alg = HASH_SHA512;
            break;
        }
        case MB_HASH_ALGORITHM_SHA1:
        {
            lib_alg = HASH_SHA1;
            break;
        }
        case MB_HASH_ALGORITHM_SHA224:
        {
            lib_alg = HASH_SHA224;
            break;
        }
        case MB_HASH_ALGORITHM_SHA512_224:
        {
            lib_alg = HASH_SHA512_224;
            break;
        }
        case MB_HASH_ALGORITHM_SHA512_256:
        {
            lib_alg = HASH_SHA512_256;
            break;
        }
        case MB_HASH_ALGORITHM_SHA3_224:
        {
            lib_alg = HASH_SHA3_224;
            break;
        }
        case MB_HASH_ALGORITHM_SHA3_256:
        {
            lib_alg = HASH_SHA3_256;
            break;
        }
        case MB_HASH_ALGORITHM_SHA3_384:
        {
            lib_alg = HASH_SHA3_384;
            break;
        }
        case MB_HASH_ALGORITHM_SHA3_512:
        {
            lib_alg = HASH_SHA3_512;
            break;
        }
        default:
        {
            lib_alg = HASH_INVALID_ALG;
            break;
        }
    }
    return lib_alg;
}

static inline const cpt_eccp_curve_st* get_curve_param_type(uint32_t curve_id)
{
    const cpt_eccp_curve_st *p;
    switch (curve_id)
    {
        case KMS_KEY_ALG_ECC_BRAINPOOLP_160R1:
        {
            p = brainpoolp160r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_192R1:
        {
            p = brainpoolp192r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_224R1:
        {
            p = brainpoolp224r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_256R1:
        {
            p = brainpoolp256r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_320R1:
        {
            p = brainpoolp320r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_384R1:
        {
            p = brainpoolp384r1;
            break;
        }
        case KMS_KEY_ALG_ECC_BRAINPOOLP_512R1:
        {
            p = brainpoolp512r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_160R1:
        {
            p = secp160r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_160R2:
        {
            p = secp160r2;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_192R1:
        {
            p = secp192r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_224R1:
        {
            p = secp224r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_256R1:
        {
            p = secp256r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_384R1:
        {
            p = secp384r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_521R1:
        {
            p = secp521r1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_160K1:
        {
            p = secp160k1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_192K1:
        {
            p = secp192k1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_224K1:
        {
            p = secp224k1;
            break;
        }
        case KMS_KEY_ALG_ECC_SECP_256K1:
        {
            p = secp256k1;
            break;
        }
        case KMS_KEY_ALG_SM2:
        {
            p = sm2_curve;
            break;
        }
        default:
        {
            p = (const cpt_eccp_curve_st* )NULL;
            break;
        }
    }
    return p;
}

static inline cpt_ske_alg_e get_ske_alg(uint8_t algo_id)
{
    cpt_ske_alg_e ske_alg;

    switch (algo_id)
    {
        case MB_SYMM_CIPHER_ALGORITHM_DES:
        {
            ske_alg = SKE_ALG_DES;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_TDES_128:
        {
            ske_alg = SKE_ALG_TDES_128;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_TDES_192:
        {
            ske_alg = SKE_ALG_TDES_192;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_AES_128:
        {
            ske_alg = SKE_ALG_AES_128;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_AES_192:
        {
            ske_alg = SKE_ALG_AES_192;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_AES_256:
        {
            ske_alg = SKE_ALG_AES_256;
            break;
        }

        case MB_SYMM_CIPHER_ALGORITHM_SM4:
        {
            ske_alg = SKE_ALG_SM4;
            break;
        }
        default:
        {
            ske_alg = SKE_ALG_INVALID;
            break;
        }
    }

    return ske_alg;
}

static inline cpt_ske_mode_e get_ske_mode(uint8_t cipher_mode)
{
    cpt_ske_mode_e mode;
    switch (cipher_mode)
    {
        case MB_SYMM_CIPHER_CIPHER_MODE_ECB:
        {
            mode = SKE_MODE_ECB;
            break;
        }
        case MB_SYMM_CIPHER_CIPHER_MODE_XTS:
        {
            mode = SKE_MODE_XTS;
            break;
        }
        case MB_SYMM_CIPHER_CIPHER_MODE_CBC:
        {
            mode = SKE_MODE_CBC;
            break;
        }
        case MB_SYMM_CIPHER_CIPHER_MODE_CFB:
        {
            mode = SKE_MODE_CFB;
            break;
        }
        case MB_SYMM_CIPHER_CIPHER_MODE_OFB:
        {
            mode = SKE_MODE_OFB;
            break;
        }
        case MB_SYMM_CIPHER_CIPHER_MODE_CTR:
        {
            mode = SKE_MODE_CTR;
            break;
        }
        default:
        {
            mode = SKE_MODE_INVALID;
            break;
        }
    }
    return mode;
}

static inline cpt_ske_padding_e get_ske_padding(uint8_t padding)
{
    cpt_ske_padding_e ske_padding;
    switch (padding)
    {
        case MB_SYMM_CIPHER_PADDING_NO_PADDING:
        {
            ske_padding = SKE_NO_PADDING;
            break;
        }
        case MB_SYMM_CIPHER_PADDING_PKCS7:
        {
            ske_padding = SKE_PKCS_5_7_PADDING;
            break;
        }
        case MB_SYMM_CIPHER_PADDING_ONEWITHZEROS:
        {
            ske_padding = SKE_ISO_7816_4_PADDING;
            break;
        }
        default:
        {
            ske_padding = SKE_INVALID_PADDING;
            break;
        }
    }
    return ske_padding;
}

static inline uint32_t get_kms_ske_alg(uint8_t alg, uint8_t cipher_mode)
{
    uint32_t kms_alg;
    switch (alg) 
    {
        case MB_SYMM_CIPHER_ALGORITHM_DES:
        {
            kms_alg = KMS_KEY_ALG_DES;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_TDES_128:
        {
            kms_alg = KMS_KEY_ALG_TDES_128;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_TDES_192:
        {
            kms_alg = KMS_KEY_ALG_TDES_192;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_AES_128:
        {
            kms_alg = (MB_SYMM_CIPHER_CIPHER_MODE_XTS == cipher_mode) ? KMS_KEY_ALG_AES_128_XTS : KMS_KEY_ALG_AES_128;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_AES_192:
        {
            kms_alg = (MB_SYMM_CIPHER_CIPHER_MODE_XTS == cipher_mode) ? KMS_KEY_ALG_AES_192_XTS : KMS_KEY_ALG_AES_192;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_AES_256:
        {
            kms_alg = (MB_SYMM_CIPHER_CIPHER_MODE_XTS == cipher_mode) ? KMS_KEY_ALG_AES_256_XTS : KMS_KEY_ALG_AES_256;
            break;
        }
        case MB_SYMM_CIPHER_ALGORITHM_SM4:
        {
            kms_alg = (MB_SYMM_CIPHER_CIPHER_MODE_XTS == cipher_mode) ? KMS_KEY_ALG_SM4_XTS : KMS_KEY_ALG_SM4;
            break;
        }
        default:
        {
            kms_alg = KMS_KEY_ALG_END;
            break;
        }
    }
    return kms_alg;
}

void crypto_srv_init(void);
#endif /* CRYPTO_UTIL_H */
