/***********************************************************************************************************************
 *  INCLUDES
 **********************************************************************************************************************/
#include "mbcmd_parser.h"
#include "mb.h"
#include "sysreg.h"
#include "config.h"
#include "version.h"
#include <ske/ske.h>
#include "crypto_lib_api.h"
#include "util.h"
#include "otp_key.h"
#include "mmap.h"
#include "otp.h"
#include "flash.h"
#include "uart.h"
#include "fw_verify.h"
#include "fw_upgrade.h"
#include "selftest.h"
#include "otp_data.h"
#include "dbgauth.h"
#include "emu.h"
#include "debug.h"
#include "kmu_driver.h"
#include "fid.h"
#include "cpu_porting.h"

/***********************************************************************************************************************
 *   VERSION CHECK
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *  DEFINES & MACROS
 **********************************************************************************************************************/
#define BOOT_LOADER_TYPE 0U

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
static uint32_t mbcmdpars_get_enc_keyid(uint8_t key_level, bool_t check_usage, uint16_t *keyid);

/***********************************************************************************************************************
 *  LOCAL FUNCTION
 **********************************************************************************************************************/
static uint32_t mbcmdpars_dec_key_by_rtlkey(uint8_t *indata, uint8_t key_lvl, uint8_t *out_key)
{
    uint16_t encrypt_keyid = KID_USAGE_UNUSED;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    cpt_ske_alg_e encrypt_alg = sysreg_get_otpkey_dec_alg();
    uint8_t default_iv[16];
    uint8_t key[64];
    uint32_t out_len;
    uint32_t crc, tmp;

    util_memset(key, 0, sizeof(key));
    util_memset(default_iv, 0, sizeof(default_iv));
    if (GET_RANDOM_K_LEVEL_1 == key_lvl) {
        encrypt_keyid = KID_INSTALL_KEK_EHSM;
    } else if ((GET_RANDOM_K_LEVEL_2 == key_lvl) || (GET_RANDOM_K_LEVEL_DEVICE_ROOT_KEY == key_lvl)) {
        // Device Root Key使用CHIP RTL KEK SOC加密，因为Device Root Key可以再MANU模式写入，但是MANU模式CHIP RTL KEK
        // eHSM被禁用了
        encrypt_keyid = KID_INSTALL_KEK_SOC;
    } else {
        ret = EHSM_ERR_WRONG_K_LEVEL;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_crypto(encrypt_alg, SKE_MODE_CBC, SKE_CRYPTO_DECRYPT, NULL, encrypt_keyid, default_iv,
            SKE_NO_PADDING, indata, key, OTP_INSTALL_K_SIZE, &out_len);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            util_memcpy(&crc, &key[OTP_K_VALUE_SIZE], sizeof(uint32_t));
            tmp = util_crc32(key, OTP_K_VALUE_SIZE, 0xFFFFFFFFu);
            if (tmp == crc) {
                (void)util_memcpy(out_key, key, OTP_K_VALUE_SIZE);
                ret = EHSM_ERR_SW_SUCCESS;
            } else {
                ret = EHSM_ERR_DATA_CHECK_ERROR;
            }
        } else {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        }
    }

    return ret;
}

static uint32_t mbcmdpars_enc_key(cpt_ske_alg_e encrypt_alg, uint16_t encrypt_keyid, uint8_t *key_buf)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t crc;
    uint32_t key_length = OTP_K_VALUE_SIZE;

    ret = cpt_ske_crypto(encrypt_alg, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, NULL, encrypt_keyid, NULL, SKE_NO_PADDING,
        key_buf, key_buf, key_length, (uint32_t *)(&key_length));
    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (OTP_K_VALUE_SIZE == key_length) {
            crc = util_crc32(key_buf, OTP_K_VALUE_SIZE, 0xFFFFFFFFu);
            util_memcpy(&key_buf[OTP_K_VALUE_SIZE], &crc, sizeof(uint32_t));
        } else {
            ret = EHSM_ERR_WRONG_KEY_SIZE;
        }
    } else {
        ret = EHSM_ERR_SKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t mbcmdpars_get_symkey(uint8_t *out_key, uint32_t key_length)
{
    return cpt_get_rand(out_key, key_length);
}

static uint32_t mbcmdpars_get_sm2key(
    uint8_t *out_key, uint32_t key_length, uint16_t encrypt_keyid, cpt_ske_alg_e encrypt_alg)
{
    uint32_t ret;

    ret = cpt_sm2_getkey(out_key, &out_key[32]);
    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = cpt_ske_crypto(encrypt_alg, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, NULL, encrypt_keyid, NULL, SKE_NO_PADDING,
            out_key, out_key, key_length, (uint32_t *)(&key_length));
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t mbcmdpars_get_secckey(
    uint8_t *out_key, uint32_t key_length, uint16_t encrypt_keyid, cpt_ske_alg_e encrypt_alg)
{
    uint32_t ret;

    ret = eccp_getkey(secp256r1, out_key, &out_key[32]);
    if ((uint32_t)PKE_SUCCESS == ret) {
        ret = cpt_ske_crypto(encrypt_alg, SKE_MODE_ECB, SKE_CRYPTO_ENCRYPT, NULL, encrypt_keyid, NULL, SKE_NO_PADDING,
            out_key, out_key, key_length, (uint32_t *)(&key_length));
        if (EHSM_ERR_SW_SUCCESS != ret) {
            ret = EHSM_ERR_SKE_WORK_ERROR;
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    } else {
        ret = EHSM_ERR_PKE_WORK_ERROR;
    }

    return ret;
}

static uint32_t mbcmdpars_get_rankey(uint8_t key_type, uint8_t *out_key, uint8_t key_level)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t key_length = OTP_K_VALUE_SIZE;
    cpt_ske_alg_e encrypt_alg = sysreg_get_otpkey_dec_alg();
    uint32_t crc;
    uint16_t encrypt_keyid;

    if (NULL == out_key) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        switch (key_type) {
        case GET_RANDOM_K_TYPE_SYMMETRIC:
            ret = mbcmdpars_get_symkey(out_key, key_length);
            break;
        case GET_RANDOM_K_TYPE_SM2PRIVATE:
        case GET_RANDOM_K_TYPE_ECC256PRIVATE:
            ret = mbcmdpars_get_enc_keyid(key_level, true, &encrypt_keyid);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                if (GET_RANDOM_K_TYPE_SM2PRIVATE == key_type) {
                    ret = mbcmdpars_get_sm2key(out_key, key_length, encrypt_keyid, encrypt_alg);
                } else {
                    ret = mbcmdpars_get_secckey(out_key, key_length, encrypt_keyid, encrypt_alg);
                }
            }
            break;
        default:
            ret = EHSM_ERR_WRONG_KEY_TYPE;
            break;
        }
    }
    if (EHSM_ERR_SW_SUCCESS == ret) {
        crc = util_crc32(out_key, OTP_K_VALUE_SIZE, 0xFFFFFFFFu);
        util_memcpy(&out_key[OTP_K_VALUE_SIZE], &crc, sizeof(uint32_t));
    }

    return ret;
}

static uint32_t mbcmdpars_check_lifecycle(uint8_t key_level)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    if (GET_RANDOM_K_LEVEL_1 == key_level) {
        if (life_cycle > SYS_STA0_LIFECYCLE_DEVELOP) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    } else if ((GET_RANDOM_K_LEVEL_2 == key_level) || (GET_RANDOM_K_LEVEL_DEVICE_ROOT_KEY == key_level)) {
        if (life_cycle > SYS_STA0_LIFECYCLE_MANUFACTURE) {
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        }
    } else {
        ret = EHSM_ERR_WRONG_K_LEVEL;
    }

    return ret;
}

static uint32_t mbcmdpars_get_enc_keyid(uint8_t key_level, bool_t check_usage, uint16_t *keyid)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t enc_keyid;
    uint16_t phy_key_id;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }
    switch (key_level) {
    case GET_RANDOM_K_LEVEL_1:
        enc_keyid = EHSM_OTP_CHIP_ROOT_KEY_ID;
        break;
    case GET_RANDOM_K_LEVEL_2:
        enc_keyid = EHSM_OTP_DEVICE_ROOT_KEY_ID;
        break;
    case GET_RANDOM_K_LEVEL_DEVICE_ROOT_KEY:
        if (life_cycle >= SYS_STA0_LIFECYCLE_MANUFACTURE) {
            // MANU模式之后chip root key不会解密到KMU，无法使用
            ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
        } else {
            enc_keyid = EHSM_OTP_CHIP_ROOT_KEY_ID;
        }
        break;
    default:
        ret = EHSM_ERR_WRONG_K_LEVEL;
        break;
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        if (check_usage == true) {
            ret = otpkey_check_usage(enc_keyid, OTP_KEY_ALGO_SKE_TYPE, K_LEVEL_1, KEY_USAGE_NONE);
        } else {
            ret = EHSM_ERR_SW_SUCCESS;
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = otpkey_get_phyid(enc_keyid, &phy_key_id);
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        *keyid = phy_key_id;
    }

    return ret;
}

static uint32_t mbcmdpars_handle_otp_write(cmd_packet_st *packet)
{
    mb_cmd_bl_otp_write_st otp_write_cmd[1];
    uint8_t *src_addr;
    uint32_t ret;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    util_memcpy(otp_write_cmd, &packet->cmd_data, sizeof(mb_cmd_bl_otp_write_st));
    if (FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST) || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP)) {
        src_addr = mmap_remap_addr_u64(otp_write_cmd->host_src_addr);
        ret = otp_write((uint32_t)otp_write_cmd->ehsm_dst_addr, (uint8_t *)src_addr, otp_write_cmd->size);
    } else {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    }

    return ret;
}

static uint32_t mbcmdpars_handle_otp_read(cmd_packet_st *packet)
{
    mb_cmd_bl_otp_read_st otp_read_cmd[1];
    uint8_t *dst_addr;
    uint32_t ret;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    util_memcpy(otp_read_cmd, &packet->cmd_data, sizeof(mb_cmd_bl_otp_read_st));
    if (FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST) || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP)) {
        dst_addr = mmap_remap_addr_u64(otp_read_cmd->host_dst_addr);
        ret = otp_read((uint32_t)otp_read_cmd->ehsm_src_addr, (uint8_t *)dst_addr, otp_read_cmd->size);
    } else {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    }

    return ret;
}

static uint32_t mbcmdpars_handle_reg_write(cmd_packet_st *packet)
{
    mb_cmd_bl_reg_wr_st reg_write_cmd[1];
    uint8_t *src_addr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    util_memcpy(reg_write_cmd, &packet->cmd_data, sizeof(mb_cmd_bl_reg_wr_st));
    if ((0 == reg_write_cmd->ehsm_dst_addr) || (0 == reg_write_cmd->host_src_addr) || (0 == reg_write_cmd->size)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST) || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP)) {
        src_addr = mmap_remap_addr_u64(reg_write_cmd->host_src_addr);
        if ((reg_write_cmd->ehsm_dst_addr >= AHB_CFG_BASE_ADDR)
            && ((reg_write_cmd->ehsm_dst_addr + reg_write_cmd->size) < AHB_CFG_END_ADDR)) {
            util_memcpy((void *)(uint32_t)reg_write_cmd->ehsm_dst_addr, (uint8_t *)src_addr, reg_write_cmd->size);
        } else {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    } else {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    }

    return ret;
}

static uint32_t mbcmdpars_handle_reg_read(cmd_packet_st *packet)
{
    mb_cmd_bl_reg_rd_st reg_read_cmd[1];
    uint8_t *dst_addr;
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint32_t life_cycle = sysreg_get_life_cycle();
    fid_delay();
    uint32_t life_cycle2 = sysreg_get_life_cycle();

    if (!FID_EQ(life_cycle, life_cycle2)) {
        fid_panic();
    }

    util_memcpy(reg_read_cmd, &packet->cmd_data, sizeof(mb_cmd_bl_reg_rd_st));
    if ((0 == reg_read_cmd->ehsm_src_addr) || (0 == reg_read_cmd->host_dst_addr) || (0 == reg_read_cmd->size)) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else if (FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_MCUTEST) || FID_EQ(life_cycle, SYS_STA0_LIFECYCLE_DEVELOP)) {
        dst_addr = mmap_remap_addr_u64(reg_read_cmd->host_dst_addr);
        if ((reg_read_cmd->ehsm_src_addr >= AHB_CFG_BASE_ADDR)
            && ((reg_read_cmd->ehsm_src_addr + reg_read_cmd->size) < AHB_CFG_END_ADDR)) {
            util_memcpy((uint8_t *)dst_addr, (uint8_t *)(uint32_t)reg_read_cmd->ehsm_src_addr, reg_read_cmd->size);
        } else {
            ret = EHSM_ERR_WRONG_DATA_LENGTH;
        }
    } else {
        ret = EHSM_ERR_EHSM_LIFECYCLE_LIMIT;
    }

    return ret;
}

static uint32_t mbcmdpars_handle_get_randkey(cmd_packet_st *packet)
{
    mb_cmd_bl_get_random_key_st cmd[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint8_t key_buf[128];

    (void)util_memcpy((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_get_random_key_st));
    if (0 == cmd->key_addr) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        ret = mbcmdpars_check_lifecycle(cmd->key_level);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            // device root key只支持对称密钥
            if ((cmd->key_type != GET_RANDOM_K_TYPE_SYMMETRIC)
                && (GET_RANDOM_K_LEVEL_DEVICE_ROOT_KEY == cmd->key_level)) {
                ret = EHSM_ERR_WRONG_KEY_TYPE;
            }
        }

        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mbcmdpars_get_rankey(cmd->key_type, key_buf, cmd->key_level);
            if (EHSM_ERR_SW_SUCCESS == ret) {
                ret = mmap_write_remote_data(cmd->key_addr, key_buf, OTP_K_SIZE);
            }
        }
    }

    return ret;
}

static uint32_t mbcmdpars_handle_encrypt_key(cmd_packet_st *packet)
{
    mb_cmd_bl_encrypt_key_st cmd[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;
    uint16_t encrypt_keyid;
    uint8_t key_buf[128];
    cpt_ske_alg_e encrypt_alg = sysreg_get_otpkey_dec_alg();

    util_memset(key_buf, 0, sizeof(key_buf));
    (void)util_memcpy((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_encrypt_key_st));
    if (cmd->input_size != OTP_INSTALL_K_SIZE) {
        ret = EHSM_ERR_WRONG_KEY_SIZE;
    } else if ((0 == cmd->input_addr) || (0 == cmd->output_addr)) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        ret = mbcmdpars_check_lifecycle(cmd->key_level);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mbcmdpars_get_enc_keyid(cmd->key_level, true, &encrypt_keyid);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mmap_read_remote_data(key_buf, cmd->input_addr, cmd->input_size);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mbcmdpars_dec_key_by_rtlkey(key_buf, cmd->key_level, key_buf);
        }
    }

    if (EHSM_ERR_SW_SUCCESS == ret) {
        ret = mbcmdpars_enc_key(encrypt_alg, encrypt_keyid, key_buf);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mmap_write_remote_data(cmd->output_addr, key_buf, OTP_K_SIZE);
        }
    }

    return ret;
}

static uint32_t mbcmdpars_handle_set_baud(cmd_packet_st *packet)
{
    mb_cmd_bl_set_uart_baudrate_st cmd[1];
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    (void)util_memcpy((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_set_uart_baudrate_st));
    if (cmd->baud_div == 0U) {
        ret = EHSM_ERR_PARAM_ERROR;
    } else {
        uint32_t level = cpu_enter_critical();
        uart_set_baudrate(cmd->baud_div);
        (void)cpu_exit_critical(level);
        log_info("bdrate: %d\n", CONFIG_BL_CPU_FREQ_HZ / cmd->baud_div);
    }
    return ret;
}

static uint32_t mbcmdpars_handle_read_version(cmd_packet_st *packet)
{
    mb_cmd_bl_read_ver_st cmd[1];
    mb_ehsm_version_st version[1];
    uint32_t ret;

    util_memset(version, 0, sizeof(mb_ehsm_version_st));

    (void)util_memcpy((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_read_ver_st));
    if (cmd->ver_size < sizeof(mb_ehsm_version_st)) {
        ret = EHSM_ERR_WRONG_DATA_LENGTH;
    } else if (cmd->ver_addr == 0U) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        version->type = BOOT_LOADER_TYPE;
        version->ver_major = EHSM_BL_VER_MAJOR;
        version->ver_minor = EHSM_BL_VER_MINOR;
        version->ver_patch = EHSM_BL_VER_PATCH;

        uint32_t i = 0;
        while (EHSM_BL_VER_PRE_RELEASE[i] != '\0' && i < sizeof(version->ver_pre_release)) {
            version->ver_pre_release[i] = EHSM_BL_VER_PRE_RELEASE[i];
            i++;
        }

        version->pke_engine_ver = cpt_pke_get_version();
        version->pke_lib_ver = cpt_pke_get_driver_version();
        version->ske_engine_ver = cpt_ske_get_version();
        version->ske_lib_ver = cpt_ske_get_driver_version();
        version->hash_engine_ver = cpt_hash_get_version();
        version->hash_lib_ver = cpt_hash_get_driver_version();
        version->trng_engine_ver = cpt_trng_get_version();
        version->trng_lib_ver = cpt_trng_get_driver_version();
        version->hw_ver = SYS_VER1_REG;
        STATIC_ASSERT(OTP_UID_LEN <= sizeof(version->uid));
        ret = otp_read(OTP_BASE_ADDR + OTP_UID_OFFSET, version->uid, OTP_UID_LEN);
        if (EHSM_ERR_SW_SUCCESS == ret) {
            ret = mmap_write_remote_data(cmd->ver_addr, version, sizeof(mb_ehsm_version_st));
        }
    }

    return ret;
}

static uint32_t mbcmdpars_handle_get_test_result(cmd_packet_st *packet)
{
    uint32_t result[2];
    mb_cmd_bl_read_self_test_result_st cmd[1];
    uint32_t ret;

    (void)util_memcpy((uint8_t *)cmd, &packet->cmd_data, sizeof(mb_cmd_bl_read_self_test_result_st));
    if (0 == cmd->result_addr) {
        ret = EHSM_ERR_INVALID_ADDRESS;
    } else {
        selftest_get_test_result(result);
        ret = mmap_write_remote_data(cmd->result_addr, result, sizeof(result));
    }

    return ret;
}




/***********************************************************************************************************************
 *  GLOBAL FUNCTION
 **********************************************************************************************************************/
uint32_t mbcmdpars_parse_cmd(cmd_packet_st *packet)
{
    uint32_t ret = EHSM_ERR_SW_SUCCESS;

    log_debug("cmd: 0x%04x\n", packet->cmd_data.cmd_id);
    switch (packet->cmd_data.cmd_id) {
    case MB_CMD_ID_BL_OTP_READ:
        ret = mbcmdpars_handle_otp_read(packet);
        break;
    case MB_CMD_ID_BL_OTP_WRITE:
        ret = mbcmdpars_handle_otp_write(packet);
        break;
    case MB_CMD_ID_BL_REG_WR:
        ret = mbcmdpars_handle_reg_write(packet);
        break;
    case MB_CMD_ID_BL_REG_RD:
        ret = mbcmdpars_handle_reg_read(packet);
        break;
    case MB_CMD_ID_BL_FW_UPGRADE:
        ret = fwupd_image_upgrade(packet, 0);
        break;
    case MB_CMD_ID_BL_VERIFY_IMAGE:
        ret = fwverify_verify_image(packet);
        break;
    case MB_CMD_ID_BL_GET_RANDOM_KEY:
        ret = mbcmdpars_handle_get_randkey(packet);
        break;
    case MB_CMD_ID_BL_ENCRYPT_KEY:
        ret = mbcmdpars_handle_encrypt_key(packet);
        break;
    case MB_CMD_ID_BL_GET_CHALLENGE: {
        uint32_t level = cpu_enter_critical();
        ret = dbgauth_srv_handler(&packet->cmd_data, &packet->rsp_data);
        (void)cpu_exit_critical(level);
        break;
    }
    case MB_CMD_ID_BL_DEBUG_AUTH:
    case MB_CMD_ID_BL_CLOSE_DEBUG: {
        uint32_t level = cpu_enter_critical();
        ret = dbgauth_srv_handler(&packet->cmd_data, &packet->rsp_data);
        (void)cpu_exit_critical(level);
        break;
    }
    case MB_CMD_ID_BL_SET_UART_BAUDRATE:
        ret = mbcmdpars_handle_set_baud(packet);
        break;
    case MB_CMD_ID_BL_READ_VER:
        ret = mbcmdpars_handle_read_version(packet);
        break;
    case MB_CMD_ID_BL_SELF_TEST:
        ret = selftest_test_alg(EHSM_SELF_TEST_ALL);
        break;
    case MB_CMD_ID_BL_READ_SELF_TEST_RESULT:
        ret = mbcmdpars_handle_get_test_result(packet);
        break;


    default:
        ret = EHSM_ERR_INVALID_CMD;
        break;
    }

    log_debug("ret for 0x%04x: 0x%08x\n", packet->cmd_data.cmd_id, ret);
    return ret;
}
/**
 *
 */
