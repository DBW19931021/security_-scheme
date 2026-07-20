/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mb.h"
#include "mmap.h"
#include "misc_srv.h"
#include "otpkinstl_srv.h"
#include "component/util.h"
#include "../driver/sysreg.h"
#include "component/otp_key.h"
#include "component/otp_map.h"
#include "../driver/kmu_driver.h"
#include "../driver/otp/otp_driver.h"
#include "component/crypto_api.h"
#include "fid.h"
/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
/*Definition the otp key slot id */
#define KEYID_USAGE_CHIP_ROOT_K1Y   (0x00U) //
#define KEYID_USAGE_DEVICE_ROOT_K1Y (0x01U) //
/*Definition the otp last key value */
#define KEY_ATTR_LAST_KEY (0x0CU)
/*Definition the otp non-last key value */
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0
#define KEY_ATTR_NO_LAST_KEY (0x0FU)
#else
#define KEY_ATTR_NO_LAST_KEY (0x08U)
#endif
/***********************************************************************************************************************
 *  TYPEDEFS
 **********************************************************************************************************************/
/*Definition the otp key attribute data */
typedef struct {
    // The install otp key id
    uint32_t key_id;
    // The install otp key level
    uint32_t key_level;
    // The install otp key usage
    uint32_t key_usage;
    // The install otp key lifecycle
    uint32_t key_cycle;
    // The install otp key last flag
    uint32_t key_last_flag;
} ehsm_otp_key_attr_st;
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
static uint32_t crc32_mpeg2(const uint8_t *in, uint32_t num, uint32_t crc);
/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      write key data to the address corresponding to ID
 *
 *   @param [in] key_id The key slot id
 *   @param [in] attr The key attribute data
 *   @param [in] key The key raw data
 *   @param [in] crc The key crc32 data
 *
 *   @return     uint32_t
 *
 *   @note
 *         1.The length of the written data must be a multiple of the minimum OTP write unit
 *         2.It is necessary to ensure that the area being checked for writing is empty
 */
static uint32_t write_otp_key(uint32_t key_id, uint32_t attr, const uint8_t *key, uint32_t crc)
{
    uint32_t key_attr = attr;
    uint32_t key_crc32 = crc;
    uint32_t key_address = 0;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t otp_key_buff[OTP_KEY_SIZE];

    if (key == NULL) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memset(otp_key_buff, 0x00, sizeof(otp_key_buff));
        util_memcpy(otp_key_buff, &key_attr, OTP_KEY_ATTR_SIZE);
        util_memcpy(&otp_key_buff[OTP_KEY_ATTR_SIZE], key, OTP_KEY_DATA_SIZE);
        util_memcpy(&otp_key_buff[OTP_KEY_ATTR_SIZE + OTP_KEY_DATA_SIZE], &key_crc32, OTP_KEY_CRC_SIZE);
        // get the otp key write address according to otp key id
        key_address = otpmap_convert_keyid_to_addr(key_id);
        // ensure that the area is empty
        ret = otp_verify(key_address, OTP_KEY_SIZE);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = misc_write_otp_data(key_address, otp_key_buff, OTP_KEY_SIZE);
        } else {
            if (ret == EHSM_ERR_DATA_NOT_EMPTY) {
                ret = EHSM_ERR_OTP_KEY_INSTALL_TWICE;
            }
        }
    }

    return ret;
}
/**
 *   @brief      Check otp key install operation whether is meet FW life cycle requirement
 *
 *   @param [in] key_id The key slot id
 *   @param [in] key_id The level of key
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t otpkinstl_check_life_cycle(uint32_t key_id, uint32_t key_level)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    (void)key_id;

    // USER and DEBUG mode not support to installation
    if ((SYS_STA0_LIFECYCLE_USER == life_cycle) || (SYS_STA0_LIFECYCLE_DEBUG == life_cycle)) {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    } else {
        // Manufacture mode only support installation device root key and key levle 2 key
        if (SYS_STA0_LIFECYCLE_MANUFACTURE == life_cycle) {
            if (OTP_KEY_LEVEL_2 == key_level) {
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_NOT_SUPPORT;
            }
        }
    }

    return ret;
}
/**
 *   @brief     Encryption otp key and then output the ciphertext key data
 *
 *   @param [in] alg The algorithm of encryption
 *   @param [in] crypto The crypto mode of encryption
 *   @param [in] encrypt_logic_keyid The otp ecnryption logice key id
 *   @param [in] in The input plaintext key data
 *   @param [out] out The output ciphertext key data
 *   @param [in] in_bytes The size of input plaintext key data
 *   @param [out] out_bytes The size of output ciphertext key data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t encrypt_otp_key(cpt_ske_alg_e alg, cpt_ske_crypto_e crypto, uint32_t encrypt_logic_keyid,
    const uint8_t *in, uint8_t *out, uint32_t in_bytes, uint32_t *out_bytes)
{
    uint16_t encrypt_keyid;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    // Get otp physical id
    ret = otpkey_get_phyid(encrypt_logic_keyid, &encrypt_keyid);
    if (ret == EHSM_ERR_SW_SUCCESS) {
        ret = cpt_ske_crypto(
            alg, SKE_MODE_ECB, crypto, NULL, encrypt_keyid, NULL, SKE_NO_PADDING, in, out, in_bytes, out_bytes);
    }

    return ret;
}
/**
 *   @brief      Generate random key and encrypted to ciphertext format
 *
 *   @param [in] random_key_type The generate random key algorithm type
 *   @param [out] cipher_key The ciphertext of otp key data
 *   @param [in] key_len The length of plaintext key
 *   @param [in] encrypt_logic_keyid The otp ecnryption logice key id
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gen_random_otp_key(
    uint8_t random_key_type, uint8_t *cipher_key, uint32_t key_len, uint32_t encrypt_logic_keyid)
{
    uint8_t key_buf[128];
    uint32_t key_length = key_len;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t otpkey_encrypt_alg_value;
    cpt_ske_alg_e encrypt_alg = SKE_ALG_AES_128;

    otpkey_encrypt_alg_value = sysreg_get_otpkey_dec_alg();
    if (OTP_CTRL0_K_ALG_SEL_AES128 != otpkey_encrypt_alg_value) {
        encrypt_alg = SKE_ALG_SM4;
    }

    switch (random_key_type) {
    case EHSM_FW_RND_KEY_TYPE_SYMMETRIC_KEY:
        // Symmetric key data does not need to be encrypted because the key is decrypted during hardware startup, making
        // the key data unique
        ret = cpt_get_rand(cipher_key, key_len);
        break;
    case EHSM_FW_RND_KEY_TYPE_SM2_PRIVATE_KEY:
        ret = cpt_sm2_getkey(key_buf, &key_buf[32]);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = encrypt_otp_key(
                encrypt_alg, SKE_CRYPTO_ENCRYPT, encrypt_logic_keyid, key_buf, cipher_key, key_length, &key_length);
        }
        break;
    case EHSM_FW_RND_KEY_TYPE_SECP256R1_PRIVATE_KEY:
        ret = cpt_eccp_getkey((const cpt_eccp_curve_st *)secp256r1, key_buf, &key_buf[32]);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            ret = encrypt_otp_key(
                encrypt_alg, SKE_CRYPTO_ENCRYPT, encrypt_logic_keyid, key_buf, cipher_key, key_length, &key_length);
        }
        break;
    default:
        ret = EHSM_ERR_PARAM_ERROR;
        break;
    }

    util_memset(key_buf, 0, sizeof(key_buf));
    return ret;
}
/**
 *   @brief      Construct anotp key attribute for a word it format definition HW TRM
 *
 *   @param [in] key_attr A struction of the key attribute
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t gen_otp_key_attr(const ehsm_otp_key_attr_st key_attr)
{
    uint32_t otp_key_attr = ((key_attr.key_usage << 8U) | (key_attr.key_level << 4U) | key_attr.key_cycle);
    otp_key_attr |= (key_attr.key_last_flag << 28);
#if CONFIG_EHSM_OTP_DEFAULT_BIT_0
    otp_key_attr = ~otp_key_attr;
#endif
    return otp_key_attr;
}
/**
 *   @brief      Decrypt ciphertext otp key data and verify key data
 *
 *   @param [in] key_id Indicate which otp key id to be installed
 *   @param [in] indata The ciphertext key data
 *   @param [in] in_len The size of ciphertext key data
 *   @param [in] key_lvl The level of key. the level_1 will be decrypted using the KEK_EHSM key. the level_2 will be
 * decrypted using the KEK_SOC key
 *   @param [out] out_key The buffer to store plaintext key data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t decrypt_cipher_key(
    uint32_t key_id, const uint8_t *indata, uint32_t in_len, uint32_t key_lvl, uint8_t *out_key)
{
    uint32_t crc;
    uint32_t tmp;
    uint32_t out_len = 0;
    uint8_t key[64] = { 0 };
    uint8_t default_iv[16];
    uint16_t encrypt_keyid;
    uint32_t otpkey_encrypt_alg_value;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e encrypt_alg = SKE_ALG_AES_128;

    if ((indata == NULL) || (out_key == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        util_memset(default_iv, 0x00, sizeof(default_iv));
        otpkey_encrypt_alg_value = sysreg_get_otpkey_dec_alg();
        if (OTP_CTRL0_K_ALG_SEL_AES128 != otpkey_encrypt_alg_value) {
            encrypt_alg = SKE_ALG_SM4;
        }

        if (OTP_KEY_LEVEL_1 == key_lvl) {
            /*Due to the EHSM_KEK can not be used in manufacture mode,
              so device root key use SOC_KEK to decrypt key data*/
            if (key_id == KEYID_USAGE_DEVICE_ROOT_K1Y) {
                encrypt_keyid = KMU_KID_INSTALL_KEK_SOC;
            } else {
                encrypt_keyid = KMU_KID_INSTALL_KEK_EHSM;
            }
        } else if (OTP_KEY_LEVEL_2 == key_lvl) {
            encrypt_keyid = KMU_KID_INSTALL_KEK_SOC;
        } else {
            ret = EHSM_ERR_WRONG_KEY_LEVEL;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_crypto(encrypt_alg, SKE_MODE_CBC, SKE_CRYPTO_DECRYPT, NULL, encrypt_keyid, default_iv,
            SKE_NO_PADDING, indata, key, in_len, &out_len);
        if (ret == EHSM_ERR_SW_SUCCESS) {
            util_memcpy(&crc, &key[OTP_KEY_DATA_SIZE], OTP_KEY_CRC_SIZE);
            tmp = crc32_mpeg2(key, OTP_KEY_DATA_SIZE, 0xFFFFFFFFU);
            if (tmp == crc) {
                util_memcpy(out_key, key, OTP_KEY_DATA_SIZE);
            } else {
                ret = EHSM_ERR_DATA_CHECKSUM;
            }
        }
    }
    util_memset(key, 0x00, sizeof(key));

    return ret;
}
/**
 *   @brief      Install random key to firmware. the asymmetric private key data will be encrypted into ciphertext befer
 * writing to OTP device. the symmetric key not need to be encrypted.
 *
 *   @param [in] key_attr The otp key attribute data contains the key level/usage/lifecycle/last_flag information
 *   @param [in] key The otp raw key data
 *   @param [in] key_size The size of otp raw key
 *
 *   @return     uint32_t
 *
 *   @note Symmetric key data does not need to be encrypted because the key is decrypted during hardware startup, making
 * the key data unique
 */
static uint32_t fw_gen_random_otp_key(uint8_t random_key_type, const ehsm_otp_key_attr_st *key_attr, uint32_t key_len)
{
    uint32_t crc;
    uint32_t word_attr;
    uint32_t rsp = EHSM_ERR_SW_SUCCESS;
    uint8_t key_data[OTP_KEY_DATA_SIZE];
    uint32_t encrypt_logic_keyid = 0;

    if (key_attr == NULL) {
        rsp = EHSM_ERR_PARAM_ERROR;
    } else {
        rsp = otpkinstl_check_life_cycle(key_attr->key_id, key_attr->key_level);
    }

    if (EHSM_ERR_SW_SUCCESS == rsp) {
        if (key_attr->key_id != KEYID_USAGE_CHIP_ROOT_K1Y) {
            if (OTP_KEY_LEVEL_1 == key_attr->key_level) {
                encrypt_logic_keyid = EHSM_OTP_CHIP_ROOT_KEY_ID;
                rsp = otpkey_check_usage(
                    EHSM_OTP_CHIP_ROOT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_NO_CHECK, KEY_USAGE_NONE);
            } else if (OTP_KEY_LEVEL_2 == key_attr->key_level) {
                encrypt_logic_keyid = EHSM_OTP_DEVICE_ROOT_KEY_ID;
                rsp = otpkey_check_usage(
                    EHSM_OTP_DEVICE_ROOT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_NO_CHECK, KEY_USAGE_NONE);
            } else {
                rsp = EHSM_ERR_PARAM_ERROR;
            }
        }
    }

    if (EHSM_ERR_SW_SUCCESS == rsp) {
        util_memset(key_data, 0, sizeof(key_data));
        rsp = gen_random_otp_key(random_key_type, key_data, key_len, encrypt_logic_keyid);
        if (EHSM_ERR_SW_SUCCESS == rsp) {
            word_attr = gen_otp_key_attr(*key_attr);
            crc = crc32_mpeg2(key_data, OTP_KEY_DATA_SIZE, 0xFFFFFFFFU);
            rsp = write_otp_key(key_attr->key_id, word_attr, key_data, crc);
        }
    }

    util_memset(key_data, 0, sizeof(key_data));
    return rsp;
}
/**
 *   @brief      Install encrypted key to firmware. the key data will be encrypted into ciphertext befer writing to OTP
 * device.
 *
 *   @param [in] key_attr The otp key attribute data contains the key level/usage/lifecycle/last_flag information
 *   @param [in] key The otp raw key data
 *   @param [in] in_key_size The size of otp raw key
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t fw_encrypt_key(const ehsm_otp_key_attr_st *key_attr, uint8_t *key, uint32_t in_key_size)
{
    uint32_t crc;
    uint32_t word_attr;
    uint32_t key_length = 0;
    uint8_t key_data[64] = { 0 };
    uint32_t encrypt_logic_keyid = 0;
    uint32_t key_size = in_key_size;
    uint32_t rsp = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e encrypt_alg = SKE_ALG_AES_128;

    if ((key_attr == NULL) || (key == NULL)) {
        rsp = EHSM_ERR_PARAM_ERROR;
    } else if (48U != key_size) {
        rsp = EHSM_ERR_WRONG_DATA_LENGTH;
    } else {
        rsp = otpkinstl_check_life_cycle(key_attr->key_id, key_attr->key_level);
    }

    // check encrypt key attr
    if (EHSM_ERR_SW_SUCCESS == rsp) {
        if (key_attr->key_id != KEYID_USAGE_CHIP_ROOT_K1Y) {
            if (OTP_KEY_LEVEL_1 == key_attr->key_level) {
                encrypt_logic_keyid = EHSM_OTP_CHIP_ROOT_KEY_ID;
                rsp = otpkey_check_usage(
                    EHSM_OTP_CHIP_ROOT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_NO_CHECK, KEY_USAGE_NONE);
            } else if (OTP_KEY_LEVEL_2 == key_attr->key_level) {
                encrypt_logic_keyid = EHSM_OTP_DEVICE_ROOT_KEY_ID;
                rsp = otpkey_check_usage(
                    EHSM_OTP_DEVICE_ROOT_KEY_ID, OTP_KEY_ALGO_SKE_TYPE, OTP_KEY_LEVEL_NO_CHECK, KEY_USAGE_NONE);
            } else {
                rsp = EHSM_ERR_PARAM_ERROR;
            }
        } else {
            rsp = EHSM_ERR_NOT_SUPPORT;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == rsp) {
        rsp = decrypt_cipher_key(key_attr->key_id, key, key_size, key_attr->key_level, key_data);
        key_size = 32U;
        util_memcpy(key, key_data, key_size);
    }

    if (EHSM_ERR_SW_SUCCESS == rsp) {
        encrypt_alg = (OTP_CTRL0_K_ALG_SEL_AES128 == sysreg_get_otpkey_dec_alg()) ? SKE_ALG_AES_128 : SKE_ALG_SM4;
        rsp = encrypt_otp_key(
            encrypt_alg, SKE_CRYPTO_ENCRYPT, encrypt_logic_keyid, key, key_data, key_size, (uint32_t *)(&key_length));
        if (EHSM_ERR_SW_SUCCESS == rsp) {
            word_attr = gen_otp_key_attr(*key_attr);
            crc = crc32_mpeg2(key_data, OTP_KEY_DATA_SIZE, 0xFFFFFFFFU);
            rsp = write_otp_key(key_attr->key_id, word_attr, key_data, crc);
        }
    }

    util_memset(key_data, 0, sizeof(key_data));

    return rsp;
}
/**
 *   @brief      Install firmware random key handle.
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t otpkinstl_fw_install_random_key(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    ehsm_otp_key_attr_st key_attr[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    const mb_cmd_install_random_key_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_install_random_key_st *)req_data;
        if (cmd_data->key_type == EHSM_FW_RND_KEY_TYPE_SYMMETRIC_KEY) {
            key_attr->key_usage = OTP_KEY_ALGO_SKE_TYPE;
        } else if ((cmd_data->key_type == EHSM_FW_RND_KEY_TYPE_SM2_PRIVATE_KEY)
            || (cmd_data->key_type == EHSM_FW_RND_KEY_TYPE_SECP256R1_PRIVATE_KEY)) {
            key_attr->key_usage = OTP_KEY_ALGO_PKE_TYPE;
        } else {
            key_attr->key_usage = OTP_KEY_ALGO_NO_CHECK;
        }

        if (cmd_data->key_level == EHSM_FW_KEY_LEVEL_1) {
            key_attr->key_level = OTP_KEY_LEVEL_1;
        } else if (cmd_data->key_level == EHSM_FW_KEY_LEVEL_2) {
            key_attr->key_level = OTP_KEY_LEVEL_2;
        } else {
            key_attr->key_level = OTP_KEY_LEVEL_NO_CHECK;
        }

        key_attr->key_id = cmd_data->key_slot_id;
        key_attr->key_cycle = OTP_KEY_LIFECYCLE_ENABLE;
        key_attr->key_last_flag = (cmd_data->key_last_flag == 1U) ? KEY_ATTR_LAST_KEY : KEY_ATTR_NO_LAST_KEY;

        if ((key_attr->key_usage != OTP_KEY_ALGO_NO_CHECK) && (key_attr->key_level != OTP_KEY_LEVEL_NO_CHECK)
            && (key_attr->key_id < CONFIG_EHSM_OTP_KEY_COUNT)) {
            ret = fw_gen_random_otp_key(cmd_data->key_type, key_attr, OTP_KEY_DATA_SIZE);
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    return ret;
}
/**
 *   @brief      Install firmware encryption key handle. during the key transmission process
 *               it will be ecnrypted by the K_KEK and will be redecrypted into plaintext after reaching the firmware
 * end
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
static uint32_t otpkinstl_fw_install_encrypt_key(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint8_t key_buf[64];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    ehsm_otp_key_attr_st key_attr[1];
    const mb_cmd_install_encrypt_key_st *cmd_data = NULL;

    if ((req_data == NULL) || (rsp_data == NULL)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        cmd_data = (const mb_cmd_install_encrypt_key_st *)req_data;
        if (cmd_data->key_type == EHSM_FW_ENC_KEY_TYPE_SYMMETRIC_KEY) {
            key_attr->key_usage = OTP_KEY_ALGO_SKE_TYPE;
        } else if (cmd_data->key_type == EHSM_FW_ENC_KEY_TYPE_ASYMMETRIC_PRIVATE_KEY) {
            key_attr->key_usage = OTP_KEY_ALGO_PKE_TYPE;
        } else if (cmd_data->key_type == EHSM_FW_ENC_KEY_TYPE_ASYMMETRIC_PUBLIC_KEY_HASH) {
            key_attr->key_usage = OTP_KEY_ALGO_HASH_TYPE;
        } else {
            key_attr->key_usage = OTP_KEY_ALGO_NO_CHECK;
        }

        if (cmd_data->key_level == EHSM_FW_KEY_LEVEL_1) {
            key_attr->key_level = OTP_KEY_LEVEL_1;
        } else if (cmd_data->key_level == EHSM_FW_KEY_LEVEL_2) {
            key_attr->key_level = OTP_KEY_LEVEL_2;
        } else {
            key_attr->key_level = OTP_KEY_LEVEL_NO_CHECK;
        }

        if (cmd_data->input_size < sizeof(key_buf)) {
            key_attr->key_id = cmd_data->key_slot_id;
            key_attr->key_cycle = OTP_KEY_LIFECYCLE_ENABLE;
            key_attr->key_last_flag = (cmd_data->key_last_flag == 1U) ? KEY_ATTR_LAST_KEY : KEY_ATTR_NO_LAST_KEY;
            ret = mmap_read_remote_data(key_buf, cmd_data->input_addr, cmd_data->input_size);
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if ((key_attr->key_usage != OTP_KEY_ALGO_NO_CHECK) && (key_attr->key_level != OTP_KEY_LEVEL_NO_CHECK)
            && (key_attr->key_id < CONFIG_EHSM_OTP_KEY_COUNT)) {
            ret = fw_encrypt_key(key_attr, key_buf, cmd_data->input_size);
        } else {
            ret = EHSM_ERR_PARAM_ERROR;
        }
    }

    util_memset(key_buf, 0, sizeof(key_buf));

    return ret;
}

/**
 * @brief Calculate the CRC32 checksum using the MPEG-2 polynomial.
 *
 * This function calculates the CRC32 checksum of a given input buffer using the MPEG-2 polynomial.
 * The CRC32 checksum is a widely used error-detecting code that can detect accidental changes to raw data.
 *
 * @param in A pointer to the input buffer for which the CRC32 checksum is to be calculated.
 * @param num The number of bytes in the input buffer.
 * @param crc The initial CRC32 value. This is typically set to 0xFFFFFFFF for the first call.
 * @return The calculated CRC32 checksum.
 */
static uint32_t crc32_mpeg2(const uint8_t *in, uint32_t num, uint32_t crc)
{
    uint32_t i;
    const uint8_t *in_p = in;
    uint32_t crc_v = crc;
    uint32_t loop = num;
    // Iterate over each byte in the input buffer
    for (; loop > 0U; loop--) {
        // XOR the current byte with the CRC value, shifted to the left by 24 bits
        crc_v = crc_v ^ (((uint32_t)(*in_p)) << 24);
        in_p++;
        // Iterate over each bit in the current byte
        for (i = 0; i < 8U; i++) {
            // If the most significant bit of the CRC value is set
            if ((crc_v & 0x80000000U) != 0U) {
                // Shift the CRC value to the left by 1 bit and XOR with the MPEG-2 polynomial
                crc_v = (crc_v << 1U) ^ (0x04C11DB7U);
            } else {
                // Shift the CRC value to the left by 1 bit
                crc_v <<= 1U;
            }
        }
    }
    // Return the calculated CRC32 checksum
    return (crc_v);
}
/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
/**
 *   @brief      otp key install service processing function entry
 *
 *   @param [in] req_data
 *   @param [in] rsp_data
 *
 *   @return     uint32_t
 *
 *   @note
 */
uint32_t otpkinstl_srv_handler(const cmd_req_data_st *req_data, const cmd_rsp_data_st *rsp_data)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    if ((NULL == req_data) || (NULL == rsp_data)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (req_data->cmd_id) {
        case MB_CMD_ID_INSTALL_RANDOM_KEY:
            ret = otpkinstl_fw_install_random_key(req_data, rsp_data);
            break;
        case MB_CMD_ID_INSTALL_ENCRYPT_KEY:
            ret = otpkinstl_fw_install_encrypt_key(req_data, rsp_data);
            break;
        default:
            ret = EHSM_ERR_INVALID_CMD;
            break;
        }
    }

    return ret;
}
