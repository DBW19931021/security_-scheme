#ifndef KMS_H
#define KMS_H
/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "types.h"
#include <ske/ske.h>

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*KMS key import/export config*/

/*KMS key operate permission management config*/

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
#define KMS_RSA_DH_H_PARAM_MAX_SIZE (KMS_RSA_DH_P_PARAM_MAX_SIZE)
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
#define KEY_USAGE_PERMIT_ALL_BITS     (0xFFFFFU)

#define KEY_USAGE_FLAGS                                                                              \
    (KEY_USAGE_SIGN | KEY_USAGE_VERIFY | KEY_USAGE_ENCRYPT | KEY_USAGE_DECRYPT | KEY_USAGE_TIMESTAMP \
        | KEY_USAGE_SECUREBOOT | KEY_USAGE_KEYCREATION | KEY_USAGE_SECURESTORAGE | KEY_USAGE_UTCSYNC)
#define KEY_EXPORT_FLAGS (KEY_PERMIT_EXPORT_PLAINTEXT | KEY_PERMIT_EXPORT_CIPHERTEXT)

/*Definition key derive salt and password max size*/
#define KMS_DERIVE_SALT_MAX_SIZE     (128U)
#define KMS_DERIVE_PASSWORD_MAX_SIZE (KMS_SYM_KEY_DATA_MAX_SIZE)

/*Definition key import export signature size */
#define KMS_KEY_DATA_SIGNATURE_SIZE (32U)

/*Definition key id mask */
#define KMS_KEY_ID_MASK (0xFFFFFU)

/*Definition key id type and mask number*/
#define KMS_KEY_TYPE_MASK (0xFFF00000U)
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
#define KMS_KEY_ALG_SM2                  (8U)
#define KMS_KEY_ALG_RSA_1024             (9U)
#define KMS_KEY_ALG_RSA_2048             (10U)
#define KMS_KEY_ALG_RSA_3072             (11U)
#define KMS_KEY_ALG_RSA_4096             (12U)
#define KMS_KEY_ALG_RSA_1024_CRT         (13U)
#define KMS_KEY_ALG_RSA_2048_CRT         (14U)
#define KMS_KEY_ALG_RSA_3072_CRT         (15U)
#define KMS_KEY_ALG_RSA_4096_CRT         (16U)
#define KMS_KEY_ALG_RSA_DH               (17U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_160R1 (18U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_192R1 (19U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_224R1 (20U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_256R1 (21U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_320R1 (22U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_384R1 (23U)
#define KMS_KEY_ALG_ECC_BRAINPOOLP_512R1 (24U)
#define KMS_KEY_ALG_ECC_SECP_192R1       (25U)
#define KMS_KEY_ALG_ECC_SECP_224R1       (26U)
#define KMS_KEY_ALG_ECC_SECP_256R1       (27U)
#define KMS_KEY_ALG_ECC_SECP_384R1       (28U)
#define KMS_KEY_ALG_ECC_SECP_521R1       (29U)
#define KMS_KEY_ALG_ED25519              (30U)
#define KMS_KEY_ALG_X25519               (31U)
#define KMS_KEY_ALG_SM4_XTS              (32U)
#define KMS_KEY_ALG_AES_128_XTS          (33U)
#define KMS_KEY_ALG_AES_192_XTS          (34U)
#define KMS_KEY_ALG_AES_256_XTS          (35U)
#define KMS_KEY_ALG_CHACHA               (36U)
#define KMS_KEY_ALG_HMAC                 (37U)
#define KMS_KEY_ALG_ECC_SECP_160K1       (38U)
#define KMS_KEY_ALG_ECC_SECP_192K1       (39U)
#define KMS_KEY_ALG_ECC_SECP_224K1       (40U)
#define KMS_KEY_ALG_ECC_SECP_256K1       (41U)
#define KMS_KEY_ALG_SM9_ENC_USERPRIV     (42U)
#define KMS_KEY_ALG_SM9_SIGN_USERPRIV    (43U)
#define KMS_KEY_ALG_SM9_EXCH_USERPRIV    (44U)
#define KMS_KEY_ALG_SM9_EXCH_TMPKEY      (45U)
#define KMS_KEY_ALG_ECC_SECP_160R1       (46U)
#define KMS_KEY_ALG_ECC_SECP_160R2       (47U)
#define KMS_KEY_ALG_END                  (48U)

/*Definition the key part type */
#define KMS_KEY_PART_PUBKEY  (0x1U)
#define KMS_KEY_PART_PRIVKEY (0x2U)
#define KMS_KEY_PART_PAIRKEY (0x3U)

/*Definition the key data source */
typedef enum {
    /*key data source from ram buffer */
    KMS_KEY_SRC_TYPE_RAM = 1,
    /*key data source from hardware secure port */
    KMS_KEY_SRC_TYPE_SECRUE_PORT = 2,
} kms_key_src_type_e;

/*Definition the key storage format */
typedef struct {
    uint32_t permit;
    uint8_t algo_id;
    uint8_t part_info;
    uint8_t reserved[2];
    /*pub_key_size symmetric key not need to set, asymmetric key
    1. RSA DH and ECC key set the pub key size
    2. RSA key set rsa_e size the rsa n size can get from algo_id*/
    uint16_t pub_key_size;
    /*priv_key_size symmetric key size the private key size, asymmetric key
    1. RSA normal key set RSA_d size
    2. RSA crt key set RSA_q/p/dq/dp/u parameters total size
    3. RSA DH and ECC key set the private key size */
    uint16_t priv_key_size;
    uint8_t raw_data[0];
} kms_key_format_st;

/*Definition the key usage format*/
typedef struct {
    uint8_t *key;
    uint16_t size;
} kms_symkey_st;

typedef struct {
    uint8_t *pub_key;
    uint16_t pub_size;
    uint8_t *priv_key;
    uint16_t priv_size;
} kms_ecckey_st;

typedef struct {
    uint32_t *n;
    uint32_t *e;
    uint32_t *p_d;
    uint32_t *q;
    uint32_t *dp;
    uint32_t *dq;
    uint32_t *u;
    uint16_t n_size;
    uint16_t e_size;
} kms_rsakey_st;

typedef struct {
    uint8_t *pub_key;
    uint8_t *priv_key;
    uint16_t pub_size;
    uint16_t priv_size;
} kms_dhkey_st;

typedef struct {
    kms_symkey_st symm;
    kms_ecckey_st ecc;
    kms_rsakey_st rsa;
    kms_dhkey_st dh;
} kms_keypair_st;

typedef struct {
    uint8_t algo_id;
    uint8_t keypart;
    uint16_t sp_key_id;
    kms_keypair_st keypair;
    kms_key_src_type_e src_type;
} kms_keydata_st;

/***********************************************************************************************************************
 *  CONSTANTS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL VARIABLES
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t kms_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data);
uint32_t kms_read_key(uint32_t key_handle, uint8_t *key_buff, uint32_t buff_size, kms_keydata_st *keydata,
    uint8_t req_kpart, uint32_t check_usage_bits);
#endif /* KMS */
