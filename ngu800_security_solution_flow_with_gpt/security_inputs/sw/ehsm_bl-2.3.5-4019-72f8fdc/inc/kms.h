#ifndef KMS_H
#define KMS_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*KMS key import/export config*/

#define KMS_INVALID_KEY_ID    (0xFFFFFFFFU)
#define KMS_INVALID_DATA_ADDR (0U)

#define KMS_KEY_SIZE_DATA_MAX_BYTE (12U)
#define KMS_SYM_KEY_DATA_MAX_SIZE  (512U)
#define KMS_ECC_KEYPAIR_MAX_SIZE   (198U)
/*key exchange DH public key max size is 512 bytes*/
#define KMS_DH_PUBKEY_MAX_SIZE (512U)
/*key exchange DH private key max size is 512 bytes*/
#define KMS_DH_PRIVKEY_MAX_SIZE     (512U)
#define KMS_RSA_DH_KEYPAIR_MAX_SIZE (1024U)
#define KMS_RSA_DH_P_PARAM_MAX_SIZE (512U)
#define KMS_RSA_DH_Q_PARAM_MAX_SIZE (512U)
#define KMS_RSA_DH_G_PARAM_MAX_SIZE (512U)
/*RSA CRT 4096bit is max key size*/
#define KMS_KEY_DATA_MAX_SIZE (2304U)
#define KMS_KEY_HEAD_SIZE     (sizeof(kms_key_format_st))

/*Definition the key usage and permission type */
#define KEY_USAGE_NONE                (0x0U)
#define KEY_USAGE_SIGN                (0x1U)
#define KEY_USAGE_VERIFY              (0x2U)
#define KEY_USAGE_ENCRYPT             (0x4U)
#define KEY_USAGE_DECRYPT             (0x8U)
#define KEY_USAGE_TIMESTAMP           (0x10U)
#define KEY_USAGE_SECUREBOOT          (0x20U)
#define KEY_USAGE_SECURESTORAGE       (0x40U)
#define KEY_USAGE_KEYCREATION         (0x80U)
#define KEY_USAGE_CREATION_TRANSP_KEY (0x100U)
#define KEY_USAGE_UTCSYNC             (0x200U)
#define KEY_USAGE_TRANSPORT           (0x400U)
#define KEY_PERMIT_REMOVE             (0x800U)
#define KEY_PERMIT_IMPORT_PLAINTEXT   (0x1000U)
#define KEY_PERMIT_IMPORT_CIPHERTEXT  (0x2000U)
#define KEY_PERMIT_EXPORT_PLAINTEXT   (0x4000U)
#define KEY_PERMIT_EXPORT_CIPHERTEXT  (0x8000U)
#define KEY_PERMIT_BOOT_FAIL_USAGE    (0x10000U)
#define KEY_PERMIT_DEBUG_USAGE        (0x20000U)
#define KEY_PERMIT_WILDCARD           (0x40000U)
#define KEY_PERMIT_WR_PRT             (0x80000U)
#define KEY_USAGE_PERMIT_ALL_BITS     (0x3FFFFU)

#define KEY_USAGE_FLAGS                                                                              \
    (KEY_USAGE_SIGN | KEY_USAGE_VERIFY | KEY_USAGE_ENCRYPT | KEY_USAGE_DECRYPT | KEY_USAGE_TIMESTAMP \
        | KEY_USAGE_SECUREBOOT | KEY_USAGE_KEYCREATION | KEY_USAGE_SECURESTORAGE | KEY_USAGE_UTCSYNC)
#define KEY_EXPORT_FLAGS (KEY_PERMIT_EXPORT_PLAINTEXT | KEY_PERMIT_EXPORT_CIPHERTEXT)

/*Definition key derive slat and password max size*/
#define KMS_DERIVE_SALT_MAX_SIZE     (128U)
#define KMS_DERIVE_PASSWORD_MAX_SIZE (KMS_SYM_KEY_DATA_MAX_SIZE)

/*Definition key import export signature size */
#define KMS_KEY_DATA_SIGNATURE_SIZE (32U)

/*Definition key id mask */
#define KMS_KEY_ID_MASK (0xFFFFFU)

/*Definition key id type and mask number*/
#define KMS_KEY_TYPE_MASK (0x300000U)
#define KMS_KEY_TYPE_SHE  (0x000000U)
#define KMS_KEY_TYPE_EHSM (0x100000U)
#define KMS_KEY_TYPE_OTP  (0x200000U)
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*Definition the key algorithm ID */
#define KMS_KEY_ALG_RANDOM               (0U)
#define KMS_KEY_ALG_DES                  (1U)
#define KMS_KEY_ALG_TDES_128             (2U)
#define KMS_KEY_ALG_TDES_192             (3U)
#define KMS_KEY_ALG_AES_128              (4U)
#define KMS_KEY_ALG_AES_192              (5U)
#define KMS_KEY_ALG_AES_256              (6U)
#define KMS_KEY_ALG_SM4                  (7U)
#define KMS_KEY_ALG_AES_128_XTS          (8U)
#define KMS_KEY_ALG_AES_192_XTS          (9U)
#define KMS_KEY_ALG_AES_256_XTS          (10U)
#define KMS_KEY_ALG_CHACHA               (11U)
#define KMS_KEY_ALG_HMAC                 (12U)
#define KMS_KEY_ALG_RSA_1024             (13U)
#define KMS_KEY_ALG_RSA_2048             (14U)
#define KMS_KEY_ALG_RSA_3072             (15U)
#define KMS_KEY_ALG_RSA_4096             (16U)
#define KMS_KEY_ALG_RSA_1024_CRT         (17U)
#define KMS_KEY_ALG_RSA_2048_CRT         (18U)
#define KMS_KEY_ALG_RSA_3072_CRT         (19U)
#define KMS_KEY_ALG_RSA_4096_CRT         (20U)
#define KMS_KEY_ALG_RSA_DH               (21U)
#define KMS_KEY_ALG_SM2                  (22U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_160R1 (23U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_192R1 (24U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_224R1 (25U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_256R1 (26U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_320R1 (27U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_384R1 (28U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_512R1 (29U)
#define KMS_KEY_ALG_ECC_SECP_192R1       (30U)
#define KMS_KEY_ALG_ECC_SECP_224R1       (31U)
#define KMS_KEY_ALG_ECC_SECP_256R1       (32U)
#define KMS_KEY_ALG_ECC_SECP_384R1       (33U)
#define KMS_KEY_ALG_ECC_SECP_521R1       (34U)
#define KMS_KEY_ALG_X25519               (35U)
#define KMS_KEY_ALG_ED25519              (36U)
#define KMS_KEY_ALG_END                  (37U)

/*Definition the key part type */
#define KMS_KEY_PART_PUBKEY  (0x1U)
#define KMS_KEY_PART_PRIVKEY (0x2U)
#define KMS_KEY_PART_PAIRKEY (0x3U)

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/
#endif /* KMS */
