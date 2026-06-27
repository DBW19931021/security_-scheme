#include <stdio.h>
#include "ehsmdrv/basic/api.h"
#include "ehsmdrv/basic/types.h"

#include "fw_demo.h"
#include "common/utils.h"
#include "ehsmdrv/basic/port/ehsm_host_port.h"
#include "common/get_version.h"
#include "common/otp_data/ehsm_demo_otp_data.h"

#include "fw_demo/msg_digest/ehsm_demo_hash.h"

#include "fw_demo/msg_digest/ehsm_demo_hmac.h"

#include "fw_demo/symm/ehsm_demo_symm_cipher.h"

#include "fw_demo/symm/ehsm_demo_aead.h"


#include "fw_demo/symm/ehsm_demo_mac.h"

#include "fw_demo/asymm/ehsm_demo_sm2.h"

#include "fw_demo/asymm/ehsm_demo_ecdsa.h"

#include "fw_demo/asymm/ehsm_demo_rsa.h"


#include "fw_demo/rng/ehsm_demo_rng.h"

#include "fw_demo/otp/ehsm_demo_rd_wr_otp.h"

#include "fw_demo/otp/ehsm_demo_chg_lifecycle.h"

#include "fw_demo/otp/ehsm_demo_chg_ctrl_field.h"

#include "fw_demo/otp/ehsm_demo_install_otp_key.h"

#include "fw_demo/debug_auth/ehsm_demo_debug_auth.h"

#include "fw_demo/misc/ehsm_demo_rd_wr_reg.h"

#include "fw_demo/misc/ehsm_demo_set_uart_buad_div.h"

#include "fw_demo/key_mgr/ehsm_demo_gen_key.h"

#include "fw_demo/key_mgr/ehsm_demo_derive_key.h"

#include "fw_demo/key_mgr/ehsm_demo_exchg_key.h"

#include "fw_demo/key_mgr/ehsm_demo_import_key.h"

#include "fw_demo/key_mgr/ehsm_demo_export_key.h"

#include "fw_demo/key_mgr/ehsm_demo_remove_key.h"

// #% #if CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN
#include "fw_demo/key_mgr/ehsm_demo_get_pub_from_priv.h"
// #% #endif /* CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN */


#include "fw_demo/image_upgrade_verify/ehsm_demo_image_verify.h"

// #% #if CONFIG_HOST_IMAGE_UPGREADE_EN
#include "fw_demo/image_upgrade_verify/ehsm_demo_image_upgrade.h"
// #% #endif /* CONFIG_HOST_IMAGE_UPGREADE_EN */



// #% #if CONFIG_HOST_PKE_SIGN_VER_BY_DIGEST_EN
#include "fw_demo/asymm/msg_digest/test_pke_sign_ver_by_msg_digest.h"
// #% #endif /* CONFIG_HOST_PKE_SIGN_VER_BY_DIGEST_EN */

void ehsm_fw_demo_entry(void)
{
    ehsm_port_printf("eHSM firmware demo starts. \r\n\r\n");

    /* 通过 SoC 写映射后的 OTP 地址及偏移写 OTP，g_otp_data 由 fw_demo/tools/otp_data_tool/gen_otp_data.sh 生成 */
    ehsm_port_write_otp(g_otp_data, 0x0U, sizeof(g_otp_data));

    /* 重启 eHSM 使 OTP 数据同步到 eHSM 寄存器，KMU 等 */
    demo_reset_ehsm_wait_ready();

    /* 等待 eHSM HW_BOOT_DONE 和 HSM_READY */
    while ((ehsm_port_read_reg(REG_HSM_STATUS_0) & (DEMO_SYSSTA0_HW_BOOT_DONE | DEMO_SYSSTA0_HSM_READY)) == 0) { }

    // test_parallel();


    /* Hash 算法示例，包含不同的驱动模式。 */
    ehsm_demo_hash_entry();

    /* Hmac 生成和校验示例。 */
    ehsm_demo_hmac_entry();

    /* 对称算法加解密示例。 */
    ehsm_demo_symm_cipher_entry();

    /* AEAD 加解密示例。 */
    ehsm_demo_aead_entry();


    /* Mac 生成和校验示例。 */
    ehsm_demo_mac_entry();

    /* SM2 加解密、签名验签示例。 */
    ehsm_demo_sm2_entry();

    /* ECDSA 示例。 */
    ehsm_demo_ecdsa_entry();

    /* RSA 加解密、签名验签示例。 */
    ehsm_demo_rsa_entry();


    /* 随机数生成示例。 */
    ehsm_demo_rng_entry();

    /* 读写 OTP 的示例 */
    ehsm_demo_rd_wr_otp_entry();

    /* 更改生命周期示例。 */
    // ehsm_demo_chg_lifecycle_entry();

    /* 更改 OTP 控制字段示例。 */
    ehsm_demo_chg_ctrl_field_entry();

    /* 安装 OTP 密钥示例。 */
    demo_install_otp_key_entry();

    /* 鉴权调试示例。 */
    ehsm_demo_debug_auth_entry();

    /* 读写 REG 示例。 */
    ehsm_demo_rd_wr_reg_entry();

    ehsm_demo_set_uart_buad_div_entry();

    /* 生成密钥示例。 */
    ehsm_demo_gen_key_entry();

    /* 派生密钥示例。 */
    ehsm_demo_derive_key_entry();

    ehsm_demo_exchg_key_entry();

    ehsm_demo_import_key_entry();

    ehsm_demo_export_key_entry();

    ehsm_demo_remove_key_entry();

    // #% #if CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN
    ehsm_demo_get_pub_from_priv_key_entry();
    // #% #endif /* CONFIG_HOST_GET_PUB_FROM_PRIV_KEY_EN */


    ehsm_demo_image_verify_entry();

    ehsm_demo_image_upgrade_entry();



    // #% #if CONFIG_HOST_PKE_SIGN_VER_BY_DIGEST_EN
    ehsm_demo_pke_sign_verify_by_msg_digest_test();
    // #% #endif /* CONFIG_HOST_PKE_SIGN_VER_BY_DIGEST_EN */

    ehsm_port_printf("eHSM firmware demo ends with success !!! \r\n\r\n");
}
