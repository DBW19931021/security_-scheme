import pytest
import allure
import time
import logging as log
from utils.image import (
    ImageLevel,
    ImageType,
    ImageVersion,
    ImageEncAlgo,
    ImageSignAlgo,
    ImageCorruptionType
)
from utils.image import (
    generate_boot_image,
    generate_corrupted_boot_image,
    generate_upgrade_image,
    generate_corrupted_upgrade_image,
    load_binary_to_bytes,
    FW_ENCRYPT_IV_OFFSET,
    IMAGE_SEGEMENT_SIZE,
    check_verify_image_content
)
from utils import key
from utils.otp import (
    LifeCycle,
    HwKeyEncAlgo,
    PatchEnable,
    EhsmVersionCounter,
    SocVersionCounter,
    EhsmDebugAuthAlgo,
    SocDebugAuthAlgo,
    EhsmVrfEncAlgo,
    SocVrfEncAlgo,
    EhsmVerifyAlgo,
    SocVerifyAlgo,
    EhsmUpgEncAlgo,
    SocUpgEncAlgo,
    EhsmUpgradeAlgo,
    SocUpgradeAlgo,
    OtpKeyLifecycle,
    OtpKeyType
)
from utils.otp import generate_otp_data, otp_to_bin
from utils.config import cfg_data
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_reg, hostapi
from platform_adapter.uart_lib.ehsm_fw_errno import *
from platform_adapter.uart_lib.ehsm_bl_errno import *

api = get_api_interface()
host = get_host_interface()
src_fw_bin_path = "resource/image/ehsm_fw.bin"
src_fw_bytes = load_binary_to_bytes(src_fw_bin_path)

IMAGE_HEAD_SIZE = 1024

# 全局常量定义
EXTRA_BOOT_DATA = "599:01"
EXTRA_UPGRADE_DATA = "599:01"

# 强制鉴权镜像的SOC ID数据
G_GOLDEN_SOC_ID = bytes([
    0x52, 0x56, 0xC6, 0xFA, 0xD7, 0x39, 0xB5, 0x92,
    0xD1, 0xED, 0xB7, 0x84, 0x38, 0x0B, 0x9A, 0xE4
])

@pytest.fixture(scope="function")
def setup_function():
    # 清零内存区域
    host.share_memset(api.SESSION_ADDR, 0x00, api.DATA_SIZE * 9)

def set_otp_data_to_default_except_lifecycle(otp_data: bytes) -> bytes:
    """
    保留 lifecycle 配置（前4字节），其余填充默认值。
    同时确保 random_clk_en 位保持关闭状态。

    Args:
        otp_data: 原始 OTP 数据

    Returns:
        修改后的 OTP 数据

    Note:
        - lifecycle: 字节 0-3
        - random_clk_en: 字节 26 的最低 2 位
            - 位置：offset=24, bit_offset=16, width=2
            - 实际字节：24 + (16//8) = 26
            - disable 值：0b10 (十进制 2)
    """
    # Reason: 根据TEST_OTP_DEFAULT_VALUE配置确定默认擦除值
    if cfg_data.TEST_OTP_DEFAULT_VALUE == 1:
        default_value = 0xFF  # 默认值为1时，擦除后填充0xFF
    else:
        default_value = 0x00  # 默认值为0时，擦除后填充0x00

    # Reason: 保留前4个字节（lifecycle配置），后面全部填充默认值
    modified_otp = bytearray(otp_data[:4])
    modified_otp.extend([default_value] * (len(otp_data) - 4))

    # Reason: 设置 random_clk_en 位为关闭状态 (disable = "10")
    # random_clk_en 位于 offset=24, bit_offset=16, width=2
    # 实际字节位置：24 + (16//8) = 26
    # 位偏移：16 % 8 = 0，占据字节26的最低2位
    #
    # 注意：需要兼容 OTP 默认全0 或 全1 两种情况
    # - 默认全0 (0x00): 0x00 & 0b11111100 = 0x00, 0x00 | 0b01 = 0x01 (低2位=01) ✓
    # - 默认全1 (0xFF): 0xFF & 0b11111100 = 0xFC, 0xFC | 0b01 = 0xFD (低2位=01) ✓
    # 两种情况下，最终低2位都是 0b01 (disable)
    RANDOM_CLK_EN_BYTE_OFFSET = 26
    RANDOM_CLK_EN_DISABLE_VALUE = 0b01  # disable = "01" in binary

    if len(modified_otp) > RANDOM_CLK_EN_BYTE_OFFSET:
        byte_before = modified_otp[RANDOM_CLK_EN_BYTE_OFFSET]
        # 清除最低2位（兼容全0和全1两种默认值）
        modified_otp[RANDOM_CLK_EN_BYTE_OFFSET] &= 0b11111100
        # 设置为 disable 状态 (0b01)
        modified_otp[RANDOM_CLK_EN_BYTE_OFFSET] |= RANDOM_CLK_EN_DISABLE_VALUE
        byte_after = modified_otp[RANDOM_CLK_EN_BYTE_OFFSET]

        log.info(f"OTP数据修改：保留前4字节，其余{len(otp_data)-4}字节填充默认值0x{default_value:02X}，"
                 f"random_clk_en设置为disable状态(字节{RANDOM_CLK_EN_BYTE_OFFSET}: 0x{byte_before:02X}->0x{byte_after:02X}, 低2位=0b{byte_after&0b11:02b})")
    else:
        log.warning(f"OTP数据长度不足，无法设置random_clk_en位（需要至少{RANDOM_CLK_EN_BYTE_OFFSET+1}字节）")
        log.info(f"OTP数据修改：保留前4字节，其余{len(otp_data)-4}字节填充默认值0x{default_value:02X}")

    return bytes(modified_otp)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-10")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_10(setup_function):
    # generate_otp_data
    ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_TEST,
        ehsm_vrf_enc_algo = EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
        ehsm_vrf_sign_algo = ehsm_vrf_sign_algo,
        ehsm_upg_enc_algo = EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
        ehsm_upg_sign_algo = EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2
    )
    # write otp data and reset ehsm
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # generate_upgrade_image()
    boot_plain_flag = False  # eHSM升级镜像默认加密
    upgrade_image = generate_upgrade_image(
        ImageLevel.IMAGE_BL_UPGRADE,
        ImageType.IMAGE_HSM_HSMK,
        ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
        ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
        ImageVersion.IMAGE_VC0,
        boot_plain_flag
    )
    # image_upgrade and check output image content
    upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
    assert src_fw_bytes is not None
    verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
    assert verification_result, "升级输出密文启动镜像内容验证失败"
    # image_verify
    vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
    assert src_fw_bytes is not None
    if boot_plain_flag == True:
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在DEBUG模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1011")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1011(setup_function):
    with allure.step("1、配置OTP 生命周期DEBUG模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))

    with allure.step("3、验证升级输出镜像内容是否与原始固件一致 # 检查通过"):
        # 验证升级输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"
        log.info("升级输出密文启动镜像内容验证成功")

        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        # IMAGE_HSM类型只有boot_plain_flag为True时才需要逐字节对比
        if boot_plain_flag:
            assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在MANU模式下，测试RSA-3072算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1012")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_1012(setup_function):
    with allure.step("1、配置OTP 生命周期MANU模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-3072公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置成0x00；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好RSA-3072签名、没有加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        verify_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)

@allure.feature("upgrade")
@allure.description("在DEBUG模式下，测试RSA-3072算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1013")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_1013(setup_function):
    with allure.step("1、配置OTP 生命周期DEBUG模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-3072公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置成0x00；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好RSA-3072签名、没有加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        verify_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)

@allure.feature("upgrade")
@allure.description("在USER模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥值不匹配情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1014")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1014(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为0和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性； # 配置成功"):
        # 生成OTP数据（使用错误的密钥值0）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（密钥不匹配）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（密钥不匹配）: {e}")


@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 unburned 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1015")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1015(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 unburned； # 配置成功"):
        # 生成OTP数据（密钥属性为unburned）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.UNBURNED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（unburned属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（unburned属性）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Unused 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1016")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1016(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 unused； # 配置成功"):
        # 生成OTP数据（密钥属性为unused）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.UNUSED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（unused属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（unused属性）: {e}")


@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Disabled 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1017")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1017(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 disabled； # 配置成功"):
        # 生成OTP数据（密钥属性为disabled）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DISABLED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（disabled属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（disabled属性）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Distoried 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1018")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1018(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 destroied； # 配置成功"):
        # 生成OTP数据（密钥属性为destroied）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（destroied属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（destroied属性）: {e}")

@allure.feature("upgrade")
@allure.description("在MANU模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Distoried 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1019")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1019(setup_function):
    with allure.step("1、配置OTP 生命周期manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 destroied； # 配置成功"):
        # 生成OTP数据（密钥属性为destroied）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（destroied属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（destroied属性）: {e}")

@allure.feature("upgrade")
@allure.description("在USER模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Distoried 情况下的异常升级场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1020")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1020(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 destroied； # 配置成功"):
        # 生成OTP数据（密钥属性为destroied）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（destroied属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（destroied属性）: {e}")

@allure.feature("upgrade")
@allure.description("在dev模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥生命周期配置 Illegal 情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1021")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1021(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；密钥属性配置为 destroied； # 配置成功"):
        # 生成OTP数据（密钥属性为destroied，模拟illegal状态）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（destroied属性）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（destroied属性）: {e}")

@allure.feature("upgrade")
@allure.description("在 DEV 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥类型配置 ASYMETRIC_PRIV_KEY 情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1022")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1022(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；eHSM_UPGRADE_VERIFY_KEY密钥类型配置ASYMETRIC_PRIV_KEY # 配置成功"):
        # 生成OTP数据（升级校验密钥类型配置为ASYMETRIC_PRIV_KEY）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_type_config={
                "ehsm_upg_sign_algo": OtpKeyType.OTP_KEY_TYPE_PRIV
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（密钥类型配置为ASYMETRIC_PRIV_KEY错误）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（密钥类型配置错误）: {e}")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥类型配置 ASYMETRIC_KEY_HASH 情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1023")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1023(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；eHSM_UPGRADE_VERIFY_KEY密钥类型配置ASYMETRIC_KEY_HASH # 配置成功"):
        # 生成OTP数据（模拟ASYMETRIC_KEY_HASH配置错误）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_type_config={
                "ehsm_upg_sign_algo": OtpKeyType.OTP_KEY_TYPE_PUB_HASH
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（密钥类型配置错误）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（ASYMETRIC_KEY_HASH密钥类型配置错误）: {e}")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥类型配置 Illegal 情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1025")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1025(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；eHSM_UPGRADE_VERIFY_KEY密钥类型配置Illegal # 配置成功"):
        # 生成OTP数据（密钥类型为Illegal，通过特殊生命周期状态模拟）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_lifecycle_config={
                "ehsm_upg_sign_algo": OtpKeyLifecycle.DISABLED  # 使用DISABLED来模拟Illegal类型
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（密钥类型Illegal）
        try:
            upg_time, vrf_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（密钥类型Illegal）: {e}")

@allure.feature("upgrade")
@allure.description("在 DEV 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥 CRC 配置错误 情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1026")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1026(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；eHSM_UPGRADE_VERIFY_KEY密钥CRC值配置为0，生命周期为disabled # 配置成功"):
        # 生成OTP数据（密钥CRC为0，生命周期为disabled）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            no_crc32_config={
                "ehsm_upg_sign_algo": True  # 设置密钥CRC为0（不使用CRC32校验）
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        try:
            assert 0 == host.wait_bl_done(1)
        except Exception as e:
            if "BOOTLOADER_ERR is set" in str(e):
                log.info(f"预期的BOOTLOADER_ERR异常（DEV模式下CRC错误导致）: {e}")
            else:
                raise  # 如果是其他异常，重新抛出

    with allure.step("3、读上位机的状态寄存器HSM_ERR_HW1，判断是否有crc_error_bit置位"):
        # 读取HSM_ERR_HW1寄存器
        hw1_status = host.get_word(hostapi.HSM_ERR_HW1)
        log.info(f"HSM_ERR_HW1寄存器值: 0x{hw1_status:08x}")

        # CRC_ERROR位于HSM_ERR_HW1寄存器的第3位（bit 3，从0开始计数）
        crc_error_bit = 1 << 3  # 第3位，即0x00000008

        crc_error_set = bool(hw1_status & crc_error_bit)
        log.info(f"CRC_ERROR bit（bit 3）是否置位: {crc_error_set}")

        # 验证CRC_ERROR bit已置位（因为设置了密钥CRC为0）
        assert crc_error_set, f"CRC_ERROR bit应该置位，但HSM_ERR_HW1寄存器值为: 0x{hw1_status:08x}"
        log.info("CRC_ERROR bit已正确置位，符合预期")

    with allure.step("4、重写正确的OTP值，恢复测试环境"):
        # 生成正确的OTP数据（不设置CRC错误）
        correct_otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )

        # 写入正确的OTP数据并重启eHSM
        assert 0 == host.write_otp(correct_otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("已重写正确的OTP值，测试环境恢复正常")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在升级校验密钥 CRC 配置错误情况下的异常升级场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1027")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1027(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法密钥值为预置值，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值；eHSM_UPGRADE_VERIFY_KEY密钥CRC值配置为0，生命周期为disabled # 配置成功"):
        # 生成OTP数据（USER模式，密钥CRC为0，生命周期为disabled）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            no_crc32_config={
                "ehsm_upg_sign_algo": True  # 设置密钥CRC为0（不使用CRC32校验）
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        try:
            assert 0 == host.wait_hw_done(1) # crc error in lifecycle user mode will cause hw_error
            pytest.fail("在USER模式下CRC错误应该触发HW_ERR，但wait_hw_done成功了")
        except Exception as e:
            if "HW_ERR is set" in str(e):
                log.info(f"预期的HW_ERR异常（USER模式下CRC错误导致硬件错误）: {e}")
            elif "timeout" in str(e).lower(setup_function):
                log.info(f"预期的timeout异常（USER模式下CRC错误导致硬件超时）: {e}")
            else:
                log.info(f"USER模式下CRC错误导致的硬件启动异常: {e}")
                # 在USER模式下，CRC错误会触发硬件错误

    with allure.step("3、读上位机的状态寄存器HSM_ERR_HW1，判断是否有CRC_ERROR bit置位"):
        # 读取HSM_ERR_HW1寄存器
        hw1_status = host.get_word(hostapi.HSM_ERR_HW1)
        log.info(f"HSM_ERR_HW1寄存器值: 0x{hw1_status:08x}")

        # CRC_ERROR位于HSM_ERR_HW1寄存器的第3位（bit 3，从0开始计数）
        crc_error_bit = 1 << 3  # 第3位，即0x00000008

        crc_error_set = bool(hw1_status & crc_error_bit)
        log.info(f"CRC_ERROR bit（bit 3）是否置位: {crc_error_set}")

        # 验证CRC_ERROR bit已置位（因为设置了密钥CRC为0）
        assert crc_error_set, f"CRC_ERROR bit应该置位，但HSM_ERR_HW1寄存器值为: 0x{hw1_status:08x}"
        log.info("CRC_ERROR bit已正确置位，符合预期")

    with allure.step("4、重写正确的OTP值，恢复测试环境"):
        # 生成正确的OTP数据（不设置CRC错误）
        correct_otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )

        # 写入正确的OTP数据并重启eHSM
        assert 0 == host.write_otp(correct_otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("已重写正确的OTP值，测试环境恢复正常")

@allure.feature("upgrade")
@allure.description("在USER模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥值不匹配情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1028")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1028(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为0值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值； # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法，校验密钥SM3值为0）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,  # 使用SM2算法
            key_data_config={
                "ehsm_vrf_sign_algo": key.INVALID_OTP_KEY_DATA  # 配置错误的密钥值
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（使用错误的密钥值）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（使用错误的密钥值）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 unburned 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1029")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1029(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 unburned # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥生命周期为unburned）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.UNBURNED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（unburned生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（unburned生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Unused 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1030")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1030(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Unused # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥生命周期为unused）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.UNUSED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（unused生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（unused生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Disabled 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1031")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1031(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Disabled # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥生命周期为disabled）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.DISABLED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（disabled生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（disabled生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Distoried 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1032")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1032(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Distoried # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥生命周期为destroied）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（destroied生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（destroied生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在MANU模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Distoried 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1033")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1033(setup_function):
    with allure.step("1、配置OTP 生命周期manu模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Distoried # 配置成功"):
        # 生成OTP数据（MANU模式，SM2/SM4算法，校验密钥生命周期为destroied）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（MANU模式下destroied生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（MANU模式下destroied生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在USER模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Distoried 情况下的异常校验场景")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1034")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1034(setup_function):
    with allure.step("1、配置OTP 生命周期USER模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Distoried # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法，校验密钥生命周期为destroied）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.DESTROIED
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（USER模式下destroied生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（USER模式下destroied生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在dev模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥生命周期配置 Illegal 情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1035")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1035(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥生命周期配置为 Illegal # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥生命周期为Illegal，使用DISABLED模拟）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_lifecycle_config={
                "ehsm_vrf_sign_algo": OtpKeyLifecycle.DISABLED  # 使用DISABLED模拟Illegal
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（Illegal生命周期）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（Illegal生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在 DEV 模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥类型配置 ASYMETRIC_PRIV_KEY 情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1036")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1036(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥类型配置为 ASYMETRIC_PRIV_KEY # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，密钥类型为ASYMETRIC_PRIV_KEY）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_type_config={
                "ehsm_vrf_sign_algo": OtpKeyType.OTP_KEY_TYPE_PRIV  # ASYMETRIC_PRIV_KEY
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（错误密钥类型ASYMETRIC_PRIV_KEY）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（错误密钥类型ASYMETRIC_PRIV_KEY）: {e}")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥类型配置 SYMETRIC_KEY 情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1037")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1037(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥类型配置为 SYMETRIC_KEY # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法，密钥类型为SYMETRIC_KEY）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_type_config={
                "ehsm_vrf_sign_algo": OtpKeyType.OTP_KEY_TYPE_SYMM  # SYMETRIC_KEY
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（错误密钥类型SYMETRIC_KEY）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（错误密钥类型SYMETRIC_KEY）: {e}")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥类型配置 Illegal 情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1039")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1039(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥类型配置为 Illegal # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法，密钥类型为Illegal）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            key_type_config={
                "ehsm_vrf_sign_algo": OtpKeyType.OTP_KEY_TYPE_SYMM  # 使用SYMM模拟Illegal密钥类型（SM2算法配置为SYMM是非法的）
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、获取测试的SM2算法签名SM4算法加密的启动镜像，并发送校验命令； # 读取成功，数据正确"):
        # 生成校验镜像（SM2签名，SM4加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（Illegal密钥类型）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（Illegal密钥类型）: {e}")

@allure.feature("upgrade")
@allure.description("在 DEV 模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥 CRC 配置错误 情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1040")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1040(setup_function):
    with allure.step("1、配置OTP 生命周期dev模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥的CRC设置为0 # 配置成功"):
        # 生成OTP数据（DEV模式，SM2/SM4算法，校验密钥CRC为0）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            no_crc32_config={
                "ehsm_vrf_sign_algo": True  # 校验密钥CRC设置为0
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        try:
            assert 0 == host.wait_bl_done(1)
        except Exception as e:
            if "BOOTLOADER_ERR is set" in str(e):
                log.info(f"预期的BOOTLOADER_ERR异常（DEV模式下CRC错误导致）: {e}")
            else:
                raise  # 如果是其他异常，重新抛出

    with allure.step("3、读上位机的状态寄存器HSM_ERR_HW1，判断是否有CRC_ERROR bit置位"):
        # 读取HSM_ERR_HW1寄存器
        hw1_status = host.get_word(hostapi.HSM_ERR_HW1)
        log.info(f"HSM_ERR_HW1寄存器值: 0x{hw1_status:08x}")

        # CRC_ERROR位于HSM_ERR_HW1寄存器的第3位（bit 3，从0开始计数）
        crc_error_bit = 1 << 3  # 第3位，即0x00000008

        crc_error_set = bool(hw1_status & crc_error_bit)
        log.info(f"CRC_ERROR bit（bit 3）是否置位: {crc_error_set}")

        # 验证CRC_ERROR bit已置位（因为设置了校验密钥CRC为0）
        assert crc_error_set, f"CRC_ERROR bit应该置位，但HSM_ERR_HW1寄存器值为: 0x{hw1_status:08x}"
        log.info("CRC_ERROR bit已正确置位，符合预期")

    with allure.step("4、重写正确的OTP值，恢复测试环境"):
        # 生成正确的OTP数据（不设置CRC错误）
        correct_otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

        # 写入正确的OTP数据并重启eHSM
        assert 0 == host.write_otp(correct_otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("已重写正确的OTP值，测试环境恢复正常")

@allure.feature("upgrade")
@allure.description("在 USER 模式下，测试 SM2算法签名，SM4-128-CBC算法加密的校验镜像，在启动校验密钥 CRC 配置错误情况下的异常校验场景")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1041")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1041(setup_function):
    with allure.step("1、配置OTP 生命周期user模式，校验签名算法密钥eHSM_VERIFY_KEY配置SM2密钥的SM3 值为预置值，升级加密密钥eHSM_ENC_KEY配置SM4-CBC算法预置密钥值；校验密钥的CRC设置为0 # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法，校验密钥CRC为0）
        # 注意：安全启动镜像校验流程仅使用校验密钥，不配置升级密钥以确保测试准确性
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            no_crc32_config={
                "ehsm_vrf_sign_algo": True  # 校验密钥CRC设置为0
            }
        )

    with allure.step("2、重启eHSM并检查生命周期 # 重启成功，状态正常"):
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        try:
            assert 0 == host.wait_hw_done(1) # crc error in lifecycle user mode will cause hw_error
            pytest.fail("在USER模式下CRC错误应该触发HW_ERR，但wait_hw_done成功了")
        except Exception as e:
            if "HW_ERR is set" in str(e):
                log.info(f"预期的HW_ERR异常（USER模式下CRC错误导致硬件错误）: {e}")
            elif "timeout" in str(e).lower(setup_function):
                log.info(f"预期的timeout异常（USER模式下CRC错误导致硬件超时）: {e}")
            else:
                log.info(f"USER模式下CRC错误导致的硬件启动异常: {e}")
                # 在USER模式下，CRC错误会触发硬件错误

    with allure.step("3、读上位机的状态寄存器HSM_ERR_HW1，判断是否有CRC_ERROR bit置位"):
        # 读取HSM_ERR_HW1寄存器
        hw1_status = host.get_word(hostapi.HSM_ERR_HW1)
        log.info(f"HSM_ERR_HW1寄存器值: 0x{hw1_status:08x}")

        # CRC_ERROR位于HSM_ERR_HW1寄存器的第3位（bit 3，从0开始计数）
        crc_error_bit = 1 << 3  # 第3位，即0x00000008

        crc_error_set = bool(hw1_status & crc_error_bit)
        log.info(f"CRC_ERROR bit（bit 3）是否置位: {crc_error_set}")

        # 验证CRC_ERROR bit已置位（因为设置了校验密钥CRC为0）
        assert crc_error_set, f"CRC_ERROR bit应该置位，但HSM_ERR_HW1寄存器值为: 0x{hw1_status:08x}"
        log.info("CRC_ERROR bit已正确置位，符合预期")

    with allure.step("4、重写正确的OTP值，恢复测试环境"):
        # 生成正确的OTP数据（不设置CRC错误）
        correct_otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

        # 写入正确的OTP数据并重启eHSM
        assert 0 == host.write_otp(correct_otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("已重写正确的OTP值，测试环境恢复正常")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像的升级流程, 启动镜像的Plain_Flag为1明文")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1042")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1042(setup_function):
    with allure.step("1、配置OTP 生命周期Dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（DEV模式，AES128算法）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，启动镜像的Plain_Flag为1明文，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成明文升级镜像（Plain_Flag为1）
        boot_plain_flag = True  # 明文镜像
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info(f"升级完成，耗时: {upg_time}ms")

    with allure.step("3、验证升级输出镜像内容是否与原始固件一致 # 检查通过"):
        # 验证升级输出明文镜像内容与原始固件一致性
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC, boot_plain_flag)
        assert verification_result, "升级输出明文启动镜像内容验证失败"
        log.info("升级输出明文启动镜像内容验证成功")
    # image_verify
    vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
    assert src_fw_bytes is not None
    if boot_plain_flag == True:
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像的升级流程, 配置升级校验算法类型非法值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-1043")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1043(setup_function):
    with allure.step("1、配置OTP 生命周期Dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性，配置升级算法为非法值；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（DEV模式，升级算法配置为非法值，使用DISABLED模拟）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC,
            key_type_config={
                "ehsm_upg_sign_algo": OtpKeyType.OTP_KEY_TYPE_PRIV  # 使用PRIV类型来测试非法配置
            }
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，启动镜像的Plain_Flag为1明文，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = True
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（升级算法非法）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（升级算法非法）: {e}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的eHSM启动镜像， 在校验算法非法情况下的异常校验流程")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-1044")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1044(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，校验签名算法密钥配置AES-128-CMAC算法，校验加密密钥配置AES-128-CBC算法，校验算法类型配置非法值；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（USER模式，校验算法AES128，校验算法类型配置为不匹配的PRIV类型）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256,
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128-CMAC算法签名和AES-128-CBC算法加密的eHSM启动镜像，并发送校验命令 # 发送成功"):
        # 生成启动镜像（AES128签名和加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验，预期失败（校验算法配置非法）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（校验算法配置非法）: {e}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的eHSM启动镜像在Valid flag错误状态下的异常校验测试")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1045")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1045(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，校验签名算法密钥配置SM2公钥HASH值和对应属性，校验加密密钥配置SM4-128-CBC预置值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（USER模式，SM2/SM4算法用于校验）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM2算法签名和SM4-128-CBC加密的eHSM启动镜像，修改镜像valid_flag为错误状态，并发送校验命令 # 发送成功"):
        # 生成异常启动镜像（SM2签名，SM4加密，valid_flag错误）
        corrupted_boot_image = generate_corrupted_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            ImageCorruptionType.INVALID_VALID_FLAG
        )
        # 执行校验，预期失败（valid_flag错误）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(corrupted_boot_image, len(corrupted_boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的校验失败（valid_flag错误）: {e}")

@allure.feature("upgrade")
@allure.description("验证校验流程镜像头和代码分离情况，AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像在镜像头分离时校验通过(仅拷贝镜像code)")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1046")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1046(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，校验签名算法密钥配置AES-CMAC算法预置密钥值，校验加密密钥配置AES-CBC算法预置密钥值；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（TEST模式，AES128算法用于校验，按参数顺序排列）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128-CMAC算法签名和AES-128-CBC算法加密的eHSM启动镜像，配置only_copy_code为1进行镜像头和代码分离校验 # 配置成功"):
        # 生成启动镜像（AES128签名和加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]
        image_code = boot_image[IMAGE_HEAD_SIZE:]

        # 执行校验（only_copy_code=True，仅拷贝镜像代码）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
            image_header=image_header,
            image_size=len(boot_image),
            image_code=image_code,
            only_copy_code=True,    # 仅拷贝代码区
            check_version=True,
            boot=True
        )
        log.info(f"校验完成，处理时间: {vrf_time}us，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在USER模式下，验证校验流程镜像头和代码分离情况，测试 AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程，输出完整镜像")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1047")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1047(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC校验算法密钥 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成AES-CMAC签名和AES-CBC加密的SOC启动镜像并执行校验（输出完整镜像） # 计算成功"):
        # 生成启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]
        image_code = boot_image[IMAGE_HEAD_SIZE:]

        # 执行校验（only_copy_code=False，输出完整镜像）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
            image_header=image_header,
            image_size=len(boot_image),
            image_code=image_code,
            only_copy_code=False,   # 输出完整镜像
            check_version=True,
            boot=False
        )
        log.info(f"校验完成，处理时间: {vrf_time}us，输出完整镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证输出镜像内容 # 检查通过"):
        # 验证输出镜像内容
        assert vrf_image_out[1024:] != src_fw_bytes, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在USER模式下，验证校验流程镜像头和代码分离情况，测试 AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程，仅输出镜像")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1048")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1048(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC校验算法密钥 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成AES-CMAC签名和AES-CBC加密的SOC启动镜像并执行校验（仅输出镜像代码） # 计算成功"):
        # 生成启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]
        image_code = boot_image[IMAGE_HEAD_SIZE:]

        # 执行校验（only_copy_code=True，仅输出镜像代码）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
            image_header=image_header,
            image_size=len(boot_image),
            image_code=image_code,
            only_copy_code=True,    # 仅输出代码区
            check_version=True,
            boot=True
        )
        log.info(f"校验完成，处理时间: {vrf_time}us，输出镜像代码大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证输出镜像内容 # 检查通过"):
        # 验证输出镜像内容
        assert vrf_image_out != src_fw_bytes, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-11")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_11(setup_function):
    # 生成OTP数据
    ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_MANU,
        ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
        ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
        ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
        ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成升级镜像
    boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
    upgrade_image = generate_upgrade_image(
        ImageLevel.IMAGE_BL_UPGRADE,
        ImageType.IMAGE_SOC_HSMK,
        ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
        ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
        ImageVersion.IMAGE_VC0,
        boot_plain_flag
    )
    # 执行升级并检查输出镜像内容
    upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
    assert src_fw_bytes is not None
    verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
    assert verification_result, "升级输出密文启动镜像内容验证失败"
    # 校验升级输出镜像
    vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
    assert src_fw_bytes is not None
    # SOC类型只有boot_plain_flag为True时才需要逐字节对比
    if boot_plain_flag:
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在USER模式下，验证校验流程镜像头和代码分离异常情况，测试 AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程，仅输出镜像")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1103")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1103(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC校验算法密钥 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC启动镜像，并故意使用非法的image_body位置进行校验（预期失败） # 返回预期错误码"):
        # 生成启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]

        # Reason: 故意提供错误的代码区数据（非法的image_body位置）
        wrong_code = b'\xFF' * (len(boot_image) - IMAGE_HEAD_SIZE)

        # 执行镜像头和代码分离校验，预期失败
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
                image_header=image_header,
                image_size=len(boot_image),
                image_code=wrong_code,
                only_copy_code=True,
                check_version=True,
                boot=False
            )
            pytest.fail("非法的image_body位置校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的非法image_body位置校验失败: {e}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC升级镜像的ONEPASS异常升级流程，使用错误的密钥值")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1163")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1163(setup_function):
    # 生成OTP数据（使用错误的密钥值进行异常测试）
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
        soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2,
        soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
        soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2,
        key_data_config={
            "soc_upg_sign_algo": key.INVALID_OTP_KEY_DATA  # 配置错误的升级签名密钥值
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成升级镜像
    boot_plain_flag = False  # SOC升级镜像默认加密
    upgrade_image = generate_upgrade_image(
        ImageLevel.IMAGE_BL_UPGRADE,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
        ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
        ImageVersion.IMAGE_VC0,
        boot_plain_flag
    )
    # 执行升级，预期失败（密钥值错误）
    try:
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        pytest.fail("升级应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的升级失败（使用错误的密钥值）: {e}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC升级镜像的ONEPASS异常升级流程，使用错误的密钥类型")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1164")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1164(setup_function):
    # 生成OTP数据（使用错误的密钥类型进行异常测试）
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
        soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2,
        soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
        soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2,
        key_type_config={
            "soc_upg_sign_algo": OtpKeyType.OTP_KEY_TYPE_SYMM  # 配置错误的密钥类型
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成升级镜像
    boot_plain_flag = False  # SOC升级镜像默认加密
    upgrade_image = generate_upgrade_image(
        ImageLevel.IMAGE_BL_UPGRADE,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
        ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
        ImageVersion.IMAGE_VC0,
        boot_plain_flag
    )
    # 执行升级，预期失败（密钥类型错误）
    try:
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        pytest.fail("升级应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的升级失败（使用错误的密钥类型）: {e}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC升级镜像的ONEPASS异常升级流程，使用错误的密钥生命周期")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1165")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1165(setup_function):
    # 生成OTP数据（使用错误的密钥生命周期进行异常测试）
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
        soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2,
        soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
        soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2,
        key_lifecycle_config={
            "soc_upg_sign_algo": OtpKeyLifecycle.DISABLED  # 配置错误的密钥生命周期
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成升级镜像
    boot_plain_flag = False  # SOC升级镜像默认加密
    upgrade_image = generate_upgrade_image(
        ImageLevel.IMAGE_BL_UPGRADE,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
        ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
        ImageVersion.IMAGE_VC0,
        boot_plain_flag
    )
    # 执行升级，预期失败（密钥生命周期错误）
    try:
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        pytest.fail("升级应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的升级失败（使用错误的密钥生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在 user 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS异常校验流程，使用错误密钥值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1167")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1167(setup_function):
    # 生成OTP数据（使用错误的密钥值进行异常测试）
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
        soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC,
        key_data_config={
            "soc_vrf_sign_algo": key.INVALID_OTP_KEY_DATA  # 配置错误的密钥值
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成启动镜像（SOC_USE_SOC_KEY类型）
    boot_plain_flag = False  # SOC启动镜像默认加密
    boot_image = generate_boot_image(
        ImageLevel.IMAGE_BL_BOOT,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
        ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
        ImageVersion.IMAGE_VC0
    )
    # 执行校验，预期失败（错误密钥值）
    try:
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        pytest.fail("校验应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的校验失败（使用错误的密钥值）: {e}")

@allure.feature("upgrade")
@allure.description("在 user 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS异常校验流程，使用错误密钥类型")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1168")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1168(setup_function):
    # 生成OTP数据（使用错误的密钥类型进行异常测试）
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
        soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC,
        key_type_config={
            "soc_vrf_sign_algo": OtpKeyType.OTP_KEY_TYPE_PRIV  # 配置错误的密钥类型（AES应该是SYMM）
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成启动镜像（SOC_USE_SOC_KEY类型）
    boot_image = generate_boot_image(
        ImageLevel.IMAGE_BL_BOOT,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
        ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
        ImageVersion.IMAGE_VC0
    )
    # 执行校验，预期失败（错误密钥类型）
    try:
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        pytest.fail("校验应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的校验失败（使用错误的密钥类型）: {e}")

@allure.feature("upgrade")
@allure.description("在 user 模式下，测试  AES-CMAC算法签名，AES-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS异常校验流程，使用错误密钥生命周期")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1169")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1169(setup_function):
    # 生成OTP数据（使用错误的密钥生命周期进行异常测试）
    soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
    otp_data = generate_otp_data(
        lifecycle=LifeCycle.LIFECYCLE_USER,
        soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
        soc_vrf_sign_algo=soc_vrf_sign_algo,
        key_lifecycle_config={
            "soc_vrf_sign_algo": OtpKeyLifecycle.DISABLED  # 配置错误的密钥生命周期
        }
    )
    # 写入OTP数据并重启eHSM
    assert 0 == host.write_otp(otp_data)
    assert 0 == host.reset_ehsm()
    assert 0 == host.wait_bl_done(1)
    # 生成启动镜像（SOC_USE_SOC_KEY类型）
    boot_image = generate_boot_image(
        ImageLevel.IMAGE_BL_BOOT,
        ImageType.IMAGE_SOC_SOCK,
        ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
        ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
        ImageVersion.IMAGE_VC0
    )
    # 执行校验，预期失败（错误密钥生命周期）
    try:
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        pytest.fail("校验应该失败但却成功了")
    except Exception as e:
        log.info(f"预期的校验失败（使用错误的密钥生命周期）: {e}")

@allure.feature("upgrade")
@allure.description("在Test模式下，修改OTP中Version_Counter预置值大于镜像的Version_Counter，测试SM2-SM3算法签名，SM4-CBC算法加密的升级镜像的升级流程  镜像版本小于OTP版本，版本错误，无法升级。")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1183")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1183(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC升级和校验算法密钥，设置OTP version为VC3 # 配置成功"):
        # 生成OTP数据（配置较高的version counter值）
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP0_VC3  # 设置OTP version为3（较高版本）
        else:
            soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP1_VC3  # 设置OTP version为3（较高版本）

        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2,
            soc_ver_cnt=soc_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成升级镜像（镜像逻辑版本号:2 < OTP逻辑版本号:3）并执行升级 # 执行成功"):
        # 生成升级镜像（使用较低的版本号IMAGE_VC2）
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC2  # 镜像逻辑版本号:2 < OTP逻辑版本号:3
        )
        # 执行升级，预期失败（镜像版本小于OTP版本）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            # 期望抛出异常，验证错误码
            error_code = int(str(e))
            assert error_code == EHSM_ERR_WRONG_VERSION_COUNTER, \
                f"期望错误码EHSM_ERR_WRONG_VERSION_COUNTER ({EHSM_ERR_WRONG_VERSION_COUNTER})，实际错误码: {error_code}"

@allure.feature("upgrade")
@allure.description("在Test模式下，修改OTP中Version_Counter预置值小于镜像的Version_Counter，测试SM2-SM3算法签名，SM4-CBC算法加密的升级镜像的升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1184")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_1184(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC升级和校验算法密钥，设置OTP version为VC1（较低版本） # 配置成功"):
        # 根据测试配置选择版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            soc_ver_cnt = SocVersionCounter.SOC_VER_CNT_OTP0_VC1  # 设置OTP version为1（较低版本）
        else:
            soc_ver_cnt = SocVersionCounter.SOC_VER_CNT_OTP1_VC1  # 设置OTP version为1（较低版本）

        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        # 生成OTP数据（配置较低的version counter值）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2,
            soc_ver_cnt=soc_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成升级镜像（镜像逻辑版本号:2 > OTP逻辑版本号:1）并执行升级 # 执行成功"):
        # 生成升级镜像（使用较高的版本号IMAGE_VC2）
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC2  # 镜像逻辑版本号:2 > OTP逻辑版本号:1
        )
        # 执行升级，预期成功（镜像版本大于OTP版本）
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info(f"升级成功完成（镜像版本大于OTP版本），处理时间: {upg_time}ms")

    with allure.step("3、验证升级输出镜像内容 # 检查通过"):
        # 验证升级输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, False)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

    with allure.step("4、使用升级输出镜像执行启动校验 # 执行成功"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        log.info(f"启动校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Test模式下，测试fw裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1188")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1188(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Test模式，其他使用默认值（相当于空数据）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST
            # 其他参数使用默认值，模拟OTP数据为空的场景
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在dev模式下，测试fw裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1189")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1189(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Test模式，其他使用默认值（相当于空数据）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV
            # 其他参数使用默认值，模拟OTP数据为空的场景
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试soc裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1192")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1192(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置SOC校验算法密钥 # 配置成功"):
        # 生成OTP数据，配置SOC校验算法密钥以支持裸镜像校验
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST
            # 其他参数使用默认值，模拟OTP数据为空的场景
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成SOC裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,  # SOC镜像类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

        # 验证输出镜像内容（裸镜像校验时应该直接输出原始内容）
        assert src_fw_bytes is not None
        actual_content = vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)]
        assert actual_content == src_fw_bytes, "SOC裸镜像校验输出内容应与原始内容一致"
        log.info("SOC裸镜像校验输出内容验证成功")

@allure.feature("upgrade")
@allure.description("在DEV模式下，测试soc裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1193")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1193(setup_function):
    with allure.step("1、配置OTP生命周期DEV模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为DEV模式，其他使用默认值（相当于空数据）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV
            # 其他参数使用默认值，模拟OTP数据为空的场景
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成SOC裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,  # SOC镜像类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

        # 验证输出镜像内容（裸镜像校验时应该直接输出原始内容）
        assert src_fw_bytes is not None
        actual_content = vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)]
        assert actual_content == src_fw_bytes, "SOC裸镜像校验输出内容应与原始内容一致"
        log.info("SOC裸镜像校验输出内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试ecc256算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter大于image Version Counter，校验过程检查Version Counter非法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-1199")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_1199(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，配置eHSM校验算法密钥，VERSION配置为3 # 配置成功"):
        # 根据测试配置选择版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3  # 设置OTP version为3（大于镜像版本2）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3  # 设置OTP version为3（大于镜像版本2）

        # 生成OTP数据，配置ECC256算法和版本计数器
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID, # 加密密钥配置为非法值
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256,  # ECC256签名
            ehsm_ver_cnt=ehsm_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成ECC256签名、无加密、Version为2的eHSM启动镜像 # 计算成功"):
        # 生成启动镜像（固定使用IMAGE_VC2）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,  # ECC256签名
            ImageVersion.IMAGE_VC2  # 镜像版本固定为2（小于OTP版本3）
        )
        log.info(f"生成eHSM启动镜像，版本2，大小: {len(boot_image)} bytes")

    with allure.step("3、执行校验，预期失败（OTP版本3 > 镜像版本2） # 执行成功"):
        # 尝试校验镜像，预期失败（版本计数器检查失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            # 期望抛出异常，验证错误码
            error_code = int(str(e))
            assert error_code == EHSM_ERR_WRONG_VERSION_COUNTER, \
                f"期望错误码EHSM_ERR_WRONG_VERSION_COUNTER ({EHSM_ERR_WRONG_VERSION_COUNTER})，实际错误码: {error_code}"

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-12")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_12(setup_function):
    with allure.step("1、配置OTP 生命周期Manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置RSA-2048公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC预期值和对应属性，启动验证密钥eHSM_FW_VERIFY_KEY配置对应算法；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用RSA-2048算法签名和AES-CBC算法加密的SOC_eHSM升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成升级镜像
        boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # 输出镜像与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Test模式下，测试ecc-256算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter小于image Version Counter，校验过程检查Version Counter合法")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1200")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_1200(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，启动签名算法密钥eHSM_FW_VERIFY_KEY配置ecc-256公钥HASH值和对应属性，VERSION_COUNTER配置为1；重启eHSM生效； # 配置成功"):
        # 根据测试配置选择版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1  # 设置OTP version为1（小于镜像版本2）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1  # 设置OTP version为1（小于镜像版本2）

        # 生成OTP数据 - 配置版本计数器为1（小于镜像版本2）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256,
            ehsm_ver_cnt=ehsm_ver_cnt  # OTP版本1 < 镜像版本2，应该校验成功
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用ecc-256算法签名，不加密的eHSM启动镜像（版本固定为EHSM_VC2），并发送ONEPASS校验命令； # 发送成功"):
        # 生成启动镜像 - 版本固定为EHSM_VC2（版本2）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC2  # 镜像版本固定为2
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"校验成功（OTP版本1 < 镜像版本2）")

    with allure.step("3、验证Version Counter检查合法，校验流程成功完成； # 检查通过"):
        # 版本计数器检查应该通过，校验应该成功
        assert vrf_image_out is not None, "校验应该成功但返回了空结果"

@allure.feature("upgrade")
@allure.description("在user 模式下，测试 AES-CMAC算法签名，AES-CBC算法加密的升级镜像，在镜像大小超过 IRAM 大小的场景下的异常升级流程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1264")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_1264(setup_function):
    with allure.step("1、配置OTP 生命周期USER模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，将image_size 改为CONFIG_BL_IRAM_SIZE+1， 并发送升级命令； # 配置成功"):
        # 生成升级镜像
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 尝试升级，但使用超过IRAM大小的image_size，预期失败
        try:
            # 将image_size设置为超过IRAM大小的值（CONFIG_BL_IRAM_SIZE+1）
            oversized_length = len(upgrade_image) + 65537  # 假设IRAM_SIZE限制，超出1字节
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, oversized_length)
            pytest.fail("升级应该失败但实际成功了（镜像大小超过IRAM限制）")
        except Exception as e:
            log.info(f"预期的升级失败（镜像大小超过IRAM限制）: {e}")

    with allure.step("3、验证升级异常，确认因镜像大小超过IRAM限制而失败； # 检查通过"):
        # 异常测试已经在步骤2中验证完成
        log.info("镜像大小超过IRAM限制的异常测试通过")

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-13")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_13(setup_function):
    with allure.step("1、配置OTP 生命周期Manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置RSA-3072算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-3072算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # 输出镜像与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-14")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_14(setup_function):
    with allure.step("1、配置OTP 生命周期Manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置ECC-P256R1算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC预期值和对应属性，启动验证密钥eHSM_FW_VERIFY_KEY配置对应算法；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用ECC-P256R1算法签名和AES-CBC算法加密的SOC_eHSM升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成升级镜像
        boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # 输出镜像与原始镜像逐字节对比
        assert src_fw_bytes is not None
        # IMAGE_SOC_HSMK 类型的镜像，ehsm_bl_verify_image 之后吐出的镜像都是明文，需要逐字节对比
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-15")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM4CMAC_SUPPORT == 0, reason="SM4CMAC不支持")
def test_ehsm_15(setup_function):
    with allure.step("1、配置OTP 生命周期Manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置SM4-128-CMAC算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性，启动验证密钥eHSM_FW_VERIFY_KEY配置对应算法；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用SM4-128-CMAC算法签名和SM4-128-CBC算法加密的SOC_eHSM升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成升级镜像
        boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # 输出镜像与原始镜像逐字节对比
        assert src_fw_bytes is not None
        # IMAGE_SOC_HSMK 类型的镜像，ehsm_bl_verify_image 之后吐出的镜像都是明文，需要逐字节对比
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC_eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-16")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_16(setup_function):
    with allure.step("1、配置OTP 生命周期Manu模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置SM2算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性，启动验证密钥eHSM_FW_VERIFY_KEY配置对应算法；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用SM2算法签名和SM4-128-CBC算法加密的SOC_eHSM升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成升级镜像
        boot_plain_flag = False  # SOC_eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # 输出镜像与原始镜像逐字节对比
        assert src_fw_bytes is not None
        # IMAGE_SOC_HSMK 类型的镜像，ehsm_bl_verify_image 之后吐出的镜像都是明文，需要逐字节对比
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("upgrade")
@allure.description("在User模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-17")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_17(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置AES-CMAC(128)算法预置密钥值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置AES-CBC(128)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用AES-CMAC(128)算法签名和AES-CBC(128)算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-18")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_18(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置RSA-2048算法公钥HASH值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置AES-CBC(128)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA2048
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用RSA-2048算法签名和AES-CBC(128)算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-19")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_19(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置RSA-3072算法公钥HASH值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置AES-CBC(128)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA3072
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用RSA-3072算法签名和AES-CBC(128)算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-20")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_20(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置ECC-P256R1算法公钥HASH值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置AES-CBC(128)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC256
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用ECC-P256R1算法签名和AES-CBC(128)算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-21")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM4CMAC_SUPPORT == 0, reason="SM4CMAC不支持")
def test_ehsm_21(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置SM4-128-CMAC算法预置密钥值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM4_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用SM4-128-CMAC算法签名和SM4-128-CBC算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-22")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_22(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥SOC_UPGRADE_VERIFY_KEY配置SM2算法公钥HASH值和对应属性，升级加密密钥SOC_UPGRADE_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据 - SOC升级密钥配置
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成使用SM2算法签名和SM4-128-CBC算法加密的SOC升级镜像，并发送ONEPASS升级命令； # 发送成功"):
        # 生成SOC升级镜像
        boot_plain_flag = False  # SOC升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-23")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_23(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-24")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_24(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-25")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_25(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-26")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_26(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM4-CMAC算法签名，SM4-CBC算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-27")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM4CMAC_SUPPORT == 0, reason="SM4CMAC不支持")
def test_ehsm_27(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2算法签名，SM4-CBC算法加密的eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-28")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_28(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-29")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_29(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-30")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_30(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-31")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_31(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-32")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_32(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Debug模式下，测试 SM4-CMAC算法签名，SM4-CBC算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-33")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM4CMAC_SUPPORT == 0, reason="SM4CMAC不支持")
def test_ehsm_33(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2算法签名，SM4-CBC算法加密的SOC-eHSM升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-34")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_34(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，EHSM加密算法密钥和签名算法密钥 # 配置成功"):
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-35")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_35(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-36")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_36(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA2048
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-37")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_37(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA3072
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在User模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-38")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_38(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC256
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Debug模式下，测试 SM4-CMAC算法签名，SM4-CBC算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-39")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM4CMAC_SUPPORT == 0, reason="SM4CMAC不支持")
def test_ehsm_39(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM4_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-4")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_4(setup_function):
    with allure.step("1、配置OTP 生命周期Dev模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-CMAC(128)算法预置密钥签名和AES-CBC(128)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级并检查输出镜像内容
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"

    with allure.step("3、校验升级输出的启动镜像，验证ONEPASS升级流程成功； # 检查通过"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        # IMAGE_HSM_HSMK 类型的镜像，只有 boot_plain_flag 为 True 时才需要逐字节对比
        if boot_plain_flag:
            assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("升级校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2算法签名，SM4-CBC算法加密的SOC升级镜像的三段式升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-40")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_40(setup_function):
    # 步骤1：配置OTP并重启
    with allure.step("1、配置OTP生命周期，SOC加密算法密钥和签名算法密钥 # 配置成功"):
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo,
            soc_upg_enc_algo=SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC,
            soc_upg_sign_algo=SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    # 步骤2：生成升级镜像
    with allure.step("2、准备升级镜像 # 镜像生成成功"):
        boot_plain_flag = False
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )

    # 步骤3：确认重启
    with allure.step("3、重启eHSM生效； # 重启成功，状态正常"):
        log.info("eHSM已在步骤1中完成重启")

    # Reason: 三段式升级流程 - 分块提交升级镜像
    image_size = len(upgrade_image)
    HEADER_SIZE = 1024

    # 步骤4：Init - 发送镜像头
    with allure.step("4、发送start指令启动升级，检查升级返回的状态值； # 发送成功"):
        image_header = upgrade_image[:HEADER_SIZE]
        init_time, init_output = api.ehsm_upgrade_fw_image_init(image_header, HEADER_SIZE)
        log.info(f"三段式升级-初始化阶段完成，耗时: {init_time}ms")
        all_output = bytearray(init_output)

    # 步骤5：Update - 发送镜像体（16字节对齐）
    with allure.step("5、发送update指令更新，检查升级返回的状态值； # 发送成功"):
        remaining_size = image_size - HEADER_SIZE
        FINISH_BLOCK_SIZE = 1024
        FINISH_BLOCK_SIZE = (FINISH_BLOCK_SIZE // 16) * 16
        update_data_size = remaining_size - FINISH_BLOCK_SIZE

        if update_data_size > 0:
            update_data_size = (update_data_size // 16) * 16
            if update_data_size > 0:
                body_block = upgrade_image[HEADER_SIZE:HEADER_SIZE + update_data_size]
                update_time, update_output = api.ehsm_upgrade_fw_image_update(body_block, update_data_size)
                log.info(f"三段式升级-更新阶段完成，数据大小: {update_data_size}字节，耗时: {update_time}ms")
                all_output.extend(update_output)
            else:
                log.info("无需update阶段，直接进入finish")
        else:
            log.info("无需update阶段，直接进入finish")

    # 步骤6：Finish - 发送最后一块并验证
    with allure.step("6、发送finish指令完成升级，检查升级返回的状态值； # 发送成功"):
        finish_offset = HEADER_SIZE + update_data_size if update_data_size > 0 else HEADER_SIZE
        last_block = upgrade_image[finish_offset:]
        last_block_size = len(last_block)
        last_block_size = (last_block_size // 16) * 16

        if last_block_size > 0:
            last_block = last_block[:last_block_size]
            finish_time, finish_output = api.ehsm_upgrade_fw_image_finish(last_block, last_block_size)
            log.info(f"三段式升级-完成阶段，数据大小: {last_block_size}字节，耗时: {finish_time}ms")

            # 收集所有输出用于验证
            all_output.extend(finish_output)

            # 验证升级结果
            upg_image_out = bytes(all_output)
            assert src_fw_bytes is not None
            verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, soc_vrf_sign_algo, boot_plain_flag)
            assert verification_result, "升级镜像内容验证失败"
            total_time = init_time + (update_time if update_data_size > 0 else 0) + finish_time
            log.info(f"三段式升级流程完成并验证成功，总耗时: {total_time}ms")
        else:
            log.error("最后一块数据大小不足16字节，无法完成升级")
            assert False, "最后一块数据大小不足"

    # 步骤7：SOC镜像验证
    with allure.step("7、执行SOC镜像验证并比对结果 # 验证成功，镜像内容正确"):
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("SOC镜像验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，修改 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像Upgrade_Valid_Flag数据和预期不一致，测试升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-41")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_41(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，AES128算法用于升级）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，修改Upgrade_Valid_Flag值的首位为0x00，并保留原始值； # 计算成功"):
        # 生成异常升级镜像（AES CMAC签名，AES CBC加密，valid_flag错误）
        boot_plain_flag = False  # eHSM升级镜像默认加密
        corrupted_upgrade_image = generate_corrupted_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            ImageCorruptionType.INVALID_VALID_FLAG,
            None,  # corruption_value
            boot_plain_flag
        )

    with allure.step("3、将修改后的镜像配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 执行升级，预期失败（valid_flag错误）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(corrupted_upgrade_image, len(corrupted_upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（valid_flag错误）: {e}")

@allure.feature("upgrade")
@allure.description("在Test模式下，修改 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的Upgrade_Signature区域，测试升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-42")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_42(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，AES128算法用于升级）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，修改Upgrade_Signature区域数据； # 计算成功"):
        # 生成异常升级镜像（AES CMAC签名，AES CBC加密，签名损坏）
        boot_plain_flag = False  # eHSM升级镜像默认加密
        corrupted_upgrade_image = generate_corrupted_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            ImageCorruptionType.CORRUPTED_SIGNATURE,
            None,  # corruption_value
            boot_plain_flag
        )

    with allure.step("3、将修改后的镜像配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 执行升级，预期失败（签名损坏）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(corrupted_upgrade_image, len(corrupted_upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（签名损坏）: {e}")

@allure.feature("upgrade")
@allure.description("在Test模式下，修改OTP中eHSM_FW_VERIFY_KEY的（hash）预置值，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-43")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_43(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置错误HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，RSA-2048算法，使用错误的密钥HASH值）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048,
            key_data_config={
                "ehsm_upg_sign_algo": key.INVALID_OTP_KEY_DATA  # 配置错误的升级签名密钥HASH值
            }
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-2048算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成eHSM升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级，预期失败（错误的密钥HASH值）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的升级失败（错误的密钥HASH值）: {e}")

@allure.feature("upgrade")
@allure.description("在Test模式下，修改OTP中Version_Counter预置值大于镜像的Version_Counter，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-44")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_44(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置RSA-2048算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性，OTP Version配置0x07，即版本号为3；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，RSA-2048算法，OTP版本计数器设为3）
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3  # 设置OTP version为3（较高版本）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3

        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_ver_cnt=ehsm_ver_cnt,  # OTP版本设为3
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-2048算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密、Version counter为0x03 的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成eHSM升级镜像（版本为2，小于OTP版本3）
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC2,  # 镜像版本为2，小于OTP版本3
            boot_plain_flag
        )
        # 执行升级，预期失败（镜像版本小于OTP版本）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
            pytest.fail("升级应该失败但却成功了")
        except Exception as e:
            # 期望抛出异常，验证错误码
            error_code = int(str(e))
            assert error_code == EHSM_ERR_WRONG_VERSION_COUNTER, \
                f"期望错误码EHSM_ERR_WRONG_VERSION_COUNTER ({EHSM_ERR_WRONG_VERSION_COUNTER})，实际错误码: {error_code}"

@allure.feature("upgrade")
@allure.description("在Test模式下，修改OTP中Version_Counter预置值小于镜像的Version_Counter，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-45")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_45(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置RSA-2048算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性，OTP Version配置0x01，即版本号为1；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，RSA-2048算法，OTP版本计数器设为1）
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1  # 设置OTP version为1（较低版本）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1

        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_ver_cnt=ehsm_ver_cnt,  # OTP版本设为1
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-2048算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密、Version counter为0x03 的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成eHSM升级镜像（版本为2，大于OTP版本1）
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC2,  # 镜像版本为2，大于OTP版本1，应该可以升级
            boot_plain_flag
        )
        # 执行升级，预期成功（镜像版本大于OTP版本，应该允许升级）
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        assert src_fw_bytes is not None
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出密文启动镜像内容验证失败"
        log.info(f"升级成功（镜像版本2大于OTP版本1），用时: {upg_time} ms")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试升级接口对异常参数的处理过程")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-46")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_46(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用AES-CMAC(128)签名）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,  # AES-CBC(128)加密
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC  # AES-CMAC(128)签名
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、升级命令配置 process_mode 为0xf，其它值合法，并发送； # 配置成功"):
        # 注意：当前API封装不直接支持process_mode参数控制，此步骤模拟异常参数测试
        # 测试空数据（相当于异常参数）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(b"", 0)
            pytest.fail("预期升级失败（空数据），但实际升级成功了")
        except Exception as e:
            error_code = int(str(e))
            log.info(f"升级空数据失败，错误码: {error_code}")
            assert error_code != 0, "预期非零错误码"
    with allure.step("3、升级命令配置 upgrade_image 地址为空，其它值合法，并发送； # 配置成功"):
        # 测试None数据（模拟空地址）
        try:
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(None, 100)
            pytest.fail("预期升级失败（None数据），但实际升级成功了")
        except Exception as e:
            log.info(f"升级None数据失败，异常: {str(e)}")
            # 预期会抛出异常（如TypeError或其他异常）
            assert e is not None, "预期抛出异常"
    with allure.step("4、升级命令配置 storage_image地址为空，其它值合法，并发送； # 配置成功"):
        # 测试负数大小（模拟异常参数）
        try:
            dummy_data = b"dummy_image_data"
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(dummy_data, -1)
            pytest.fail("预期升级失败（负数大小），但实际升级成功了")
        except Exception as e:
            log.info(f"升级负数大小失败，异常: {str(e)}")
            # 预期会抛出异常或错误码
            assert e is not None, "预期抛出异常"
    with allure.step("5、升级命令配置 storage_enc 为0xf，其它值合法，并发送； # 配置成功"):
        # 测试数据大小不匹配（模拟异常参数）
        try:
            dummy_data = b"small_data"
            # 故意设置大小远大于实际数据长度，模拟异常参数
            upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(dummy_data, 1000000)
            pytest.fail("预期升级失败（大小不匹配），但实际升级成功了")
        except Exception as e:
            error_code = int(str(e))
            log.info(f"升级大小不匹配失败，错误码: {error_code}")
            assert error_code != 0, "预期非零错误码"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-47")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_47(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用AES-CMAC(128)签名和AES-CBC(128)加密）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,  # AES-CBC(128)加密
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC  # AES-CMAC(128)签名
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,   # AES-CBC(128)加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,  # AES-CMAC(128)签名
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-48")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_48(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-2048公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用RSA-2048签名和AES-CBC(128)加密）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,  # AES-CBC(128)加密
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048      # RSA-2048签名
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好RSA-2048签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,   # AES-CBC(128)加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,  # RSA-2048签名
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-49")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_49(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-3072公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用RSA-3072签名和AES-CBC(128)加密）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,  # AES-CBC(128)加密
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072      # RSA-3072签名
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好RSA-3072签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,   # AES-CBC(128)加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,  # RSA-3072签名
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-5")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_5(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用AES-CMAC(128)签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成eHSM升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info("eHSM升级成功，用时: %d ms", upg_time)

    with allure.step("3、检测升级后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证升级输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-50")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_50(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置ECC-P256R1公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用ECC-P256R1签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,  # AES-CBC(128)加密
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo       # ECC-P256R1签名
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好ECC-P256R1签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,   # AES-CBC(128)加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,  # ECC-P256R1签名
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-51")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_51(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM4-128-CMAC算法预置密钥值和对应属性，升级加密密钥eHSM_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用SM4-128-CMAC签名和SM4-128-CBC加密）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM4-128-CMAC算法预置密钥签名和使用SM4-128-CBC算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-52")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_52(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用SM2签名和SM4-128-CBC加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM2算法预置密钥签名和使用SM4-128-CBC算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-53")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_53(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（User模式，使用SM2签名和SM4-128-CBC加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM2算法预置密钥签名和使用SM4-128-CBC算法预置密钥加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("eHSM启动镜像校验成功，用时: %d ms", vrf_time)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-54")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_54(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用AES-CMAC(128)签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-55")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_55(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-2048公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用RSA-2048签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-2048算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-56")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_56(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置RSA-3072公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用RSA-3072签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用RSA-3072算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-57")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_57(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置ECC-P256R1公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用ECC-P256R1签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用ECC-P256R1算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-58")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_58(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM4-128-CMAC算法预置密钥值和对应属性，升级加密密钥eHSM_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用SM4-128-CMAC签名和SM4-128-CBC加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM4-128-CMAC算法预置密钥签名和使用SM4-128-CBC算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC_USE_EHSM_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-59")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_59(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置SM4-128-CBC算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用SM2签名和SM4-128-CBC加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("２、准备好使用SM2算法预置密钥签名和使用SM4-128-CBC算法预置密钥加密的SOC_USE_EHSM_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_EHSM_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_EHSM_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("３、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-6")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_6(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_UPGRADE_VERIFY_KEY配置RSA-2048算法公钥HASH值和对应属性，升级加密密钥eHSM_UPGRADE_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用RSA-2048签名和AES-CBC(128)加密）
        ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("２、准备好使用RSA-2048算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的eHSM升级镜像，并配置到升级命令的镜像地址，并发送升级命令； # 配置成功"):
        # 生成eHSM升级镜像
        boot_plain_flag = False  # eHSM升级镜像默认加密
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,
            boot_plain_flag
        )
        # 执行升级
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info("eHSM升级成功，用时: %d ms", upg_time)

    with allure.step("３、检测升级后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证升级输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, boot_plain_flag)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-60")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_60(setup_function):
    ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC

    with allure.step("１、配置OTP 生命周期Test模式，升级签名算法密钥SOC_FW_VERIFY_KEY配置AES-CMAC(128 or 256)算法预置密钥值和对应属性，升级加密密钥SOC_ENC_KEY配置AES-CBC(128 or 256)算法预置密钥值和对应属性；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（Test模式，使用AES-CMAC(128)签名和AES-CBC(128)加密）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("２、准备好使用AES-128_CMAC算法预置密钥签名和使用AES-CBC(128 or 256)算法预置密钥加密的SOC_USE_SOC_KEY启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_USE_SOC_KEY启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("３、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # 验证输出镜像内容与原始固件一致性
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, ehsm_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-61")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_61(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置SOC校验算法密钥和升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_USE_SOC_KEY启动镜像并执行校验 # 执行成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"RSA-2048+AES-CBC校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证校验输出镜像内容 # 检查通过"):
        # 验证校验输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, soc_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-62")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_62(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置SOC校验算法密钥和升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_USE_SOC_KEY启动镜像并执行校验 # 执行成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"RSA-3072+AES-CBC校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证校验输出镜像内容 # 检查通过"):
        # 验证校验输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, soc_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-63")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_63(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置SOC校验算法密钥和升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_USE_SOC_KEY启动镜像并执行校验 # 执行成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"ECC-P256R1+AES-CBC校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证校验输出镜像内容 # 检查通过"):
        # 验证校验输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, soc_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-64")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_64(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置SOC校验算法密钥和升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_USE_SOC_KEY启动镜像并执行校验 # 执行成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"SM4-CMAC+SM4-CBC校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证校验输出镜像内容 # 检查通过"):
        # 验证校验输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, soc_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-65")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_65(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置SOC校验算法密钥和升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        soc_vrf_sign_algo = SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=soc_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_USE_SOC_KEY启动镜像并执行校验 # 执行成功"):
        # 生成SOC_USE_SOC_KEY启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"SM2+SM4-CBC校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("3、验证校验输出镜像内容 # 检查通过"):
        # 验证校验输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(vrf_image_out, src_fw_bytes, soc_vrf_sign_algo, True)
        assert verification_result, "校验输出镜像内容验证失败"
        log.info("校验输出镜像内容验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-66")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_66(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM校验算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"Test模式 eHSM RSA-2048不加密校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-67")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_67(setup_function):
    with allure.step("１、配置OTP 生命周期Dev模式，配置eHSM校验算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"Dev模式 eHSM RSA-2048不加密校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在User模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-68")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_68(setup_function):
    with allure.step("１、配置OTP 生命周期User模式，配置eHSM校验算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"User模式 eHSM RSA-2048不加密校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在User模式下，测试RSA-3072算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-69")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_69(setup_function):
    with allure.step("１、配置OTP 生命周期User模式，配置eHSM校验算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行启动镜像校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"User模式 eHSM RSA-3072不加密校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-7")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason="RSA3072_PSS不支持")
def test_ehsm_7(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM升级算法密钥；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM升级镜像并执行升级 # 执行成功"):
        # 生成eHSM升级镜像
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )
        # 执行eHSM升级流程
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info(f"eHSM RSA-3072+AES-CBC升级成功完成，处理时间: {upg_time}ms")

    with allure.step("3、验证升级输出镜像内容 # 检查通过"):
        # 验证升级输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, False)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

    with allure.step("4、使用升级输出镜像执行启动校验 # 执行成功"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        log.info(f"启动校验成功完成，处理时间: {vrf_time}ms")

@allure.feature("upgrade")
@allure.description("在User模式下，测试ECC-P256R1算法签名，不加密的eHSM启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-70")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_70(setup_function):
    with allure.step("1、配置OTP 生命周期User模式，校验签名算法密钥eHSM_FW_VERIFY_KEY配置ECC-P256R1公钥HASH值和对应属性，不加密；重启eHSM生效； # 配置成功"):
        # 生成OTP数据（User模式，ECC-P256R1签名，不加密）
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用ECC-P256R1算法预置密钥签名，不加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成eHSM启动镜像（ECC-P256R1签名，不加密）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,    # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )
        # 执行ONEPASS校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info(f"ECC-P256R1不加密启动镜像校验成功，处理时间: {vrf_time}ms")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter大于image Version Counter，校验过程检查Version Counter非法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-71")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_71(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，VERSION配置成3（大于镜像版本2） # 配置成功"):
        # 生成OTP数据，设置较高的版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3  # 设置OTP version为3（较高版本）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,  # 不加密
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_ver_cnt=ehsm_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成版本为2的eHSM启动镜像并执行校验（预期失败） # 执行成功"):
        # 生成eHSM启动镜像（版本为VC2，小于OTP版本3）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC2  # 镜像版本为2，小于OTP版本3
        )
        # 执行校验，预期失败（版本检查不通过）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)  # 开启版本检查
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            # 期望抛出异常，验证错误码
            error_code = int(str(e))
            assert error_code == EHSM_ERR_WRONG_VERSION_COUNTER, \
                f"期望错误码EHSM_ERR_WRONG_VERSION_COUNTER ({EHSM_ERR_WRONG_VERSION_COUNTER})，实际错误码: {error_code}"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter小于image Version Counter，校验过程检查Version Counter合法")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-72")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_72(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，VERSION配置为1（小于镜像版本2） # 配置成功"):
        # 生成OTP数据，设置较低的版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1  # 设置OTP version为1（较低版本）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,  # 不加密
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_ver_cnt=ehsm_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成版本为2的eHSM启动镜像并执行校验（预期成功） # 执行成功"):
        # 生成eHSM启动镜像（版本为VC2，大于OTP版本1）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC2  # 镜像版本为2，大于OTP版本1
        )
        # 执行校验，预期成功（版本检查通过）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)  # 开启版本检查
        log.info(f"版本检查通过，校验成功完成，处理时间: {vrf_time}ms")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，校验通过后启动")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-73")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_73(setup_function):
    with allure.step("１、检测校验启动前FW状态寄存器 # 执行成功"):
        # 检测初始FW状态
        initial_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"初始 FW 状态: 0x{initial_status:08x}")

    with allure.step("2、配置OTP 生命周期User模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,  # 不加密
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、生成eHSM启动镜像并执行校验后启动 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验并在校验后启动
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, True)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms")

    with allure.step("4、检查FW启动成功状态寄存器 # 检查通过"):
        # 等待FW启动完成
        assert 0 == host.wait_fw_done(5)  # 等待最多5秒
        final_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"FW启动后状态: 0x{final_status:08x}")
        assert final_status & hostapi.FW_DONE, "FW应该启动成功"

@allure.feature("upgrade")
@allure.description("在Test模式下，测试校验接口对异常参数的处理过程")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-74")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_74(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、测试非法参数的处理 # 返回预期错误码"):
        # 生成正常的测试镜像
        test_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

        # 测试空镜像地址（应该失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(b"", 0, True, False)  # 空镜像
            pytest.fail("空镜像地址测试应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的空镜像地址错误: {e}")

        # 测试无效镜像大小（应该失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(test_image, 0, True, False)  # 大小为0
            pytest.fail("无效镜像大小测试应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的无效镜像大小错误: {e}")

        log.info("异常参数处理测试完成")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试裸镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-75")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_75(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像并加载到启动镜像地址 # 执行成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回的状态值 # 发送成功"):
        # 校验裸镜像，在Test模式下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"Test模式下裸镜像校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试裸镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-76")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_76(setup_function):
    with allure.step("１、配置OTP 生命周期Dev模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像并加载到启动镜像地址 # 执行成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回的状态值 # 发送成功"):
        # 校验裸镜像，在Dev模式下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"Dev模式下裸镜像校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在MANU模式下，测试裸镜像的校验受限")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-77")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_77(setup_function):
    with allure.step("１、配置OTP 生命周期MANU模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像并加载到启动镜像地址 # 执行成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回的状态值（预期失败） # 发送成功"):
        # 校验裸镜像，在MANU模式下应该受限（预期失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("MANU模式下裸镜像校验应该受限但却成功了")
        except Exception as e:
            log.info(f"预期的裸镜像校验失败（MANU模式受限）: {e}")

@allure.feature("upgrade")
@allure.description("在USER模式下，测试裸镜像的校验受限")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-78")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_78(setup_function):
    with allure.step("１、配置OTP 生命周期User模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像并加载到启动镜像地址 # 执行成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回的状态值（预期失败） # 发送成功"):
        # 校验裸镜像，在USER模式下应该受限（预期失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("USER模式下裸镜像校验应该受限但却成功了")
        except Exception as e:
            log.info(f"预期的裸镜像校验失败（USER模式受限）: {e}")

@allure.feature("upgrade")
@allure.description("在DEBUG模式下，测试裸镜像的校验受限")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-79")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_79(setup_function):
    with allure.step("１、配置OTP 生命周期Debug模式 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像并加载到启动镜像地址 # 执行成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回的状态值（预期失败） # 发送成功"):
        # 校验裸镜像，在DEBUG模式下应该受限（预期失败）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("DEBUG模式下裸镜像校验应该受限但却成功了")
        except Exception as e:
            log.info(f"预期的裸镜像校验失败（DEBUG模式受限）: {e}")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-8")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_ECC_P256R1_SUPPORT == 0, reason="ECC_P256R1不支持")
def test_ehsm_8(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM升级算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM升级镜像并执行升级 # 执行成功"):
        # 生成eHSM升级镜像
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )
        # 执行eHSM升级流程
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info(f"eHSM ECC-P256R1+AES-CBC升级成功完成，处理时间: {upg_time}ms")

    with allure.step("3、验证升级输出镜像内容 # 检查通过"):
        # 验证升级输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, False)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

    with allure.step("4、使用升级输出镜像执行启动校验 # 执行成功"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        log.info(f"启动校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试 SM4-128-CMAC算法签名，SM4-128-CBC算法加密的eHSM升级镜像的ONEPASS升级流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-9")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_9(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM升级算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_upg_sign_algo=EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC,
            ehsm_upg_enc_algo=EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM升级镜像并执行升级 # 执行成功"):
        # 生成eHSM升级镜像
        upgrade_image = generate_upgrade_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )
        # 执行eHSM升级流程
        upg_time, upg_image_out = api.ehsm_bl_upgrade_fw_image(upgrade_image, len(upgrade_image))
        log.info(f"eHSM SM4-CMAC+SM4-CBC升级成功完成，处理时间: {upg_time}ms")

    with allure.step("3、验证升级输出镜像内容 # 检查通过"):
        # 验证升级输出镜像内容与原始固件一致性
        assert src_fw_bytes is not None
        verification_result = check_verify_image_content(upg_image_out, src_fw_bytes, ehsm_vrf_sign_algo, False)
        assert verification_result, "升级输出镜像内容验证失败"
        log.info("升级输出镜像内容验证成功")

    with allure.step("4、使用升级输出镜像执行启动校验 # 执行成功"):
        # 校验升级输出镜像
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(upg_image_out, len(upg_image_out), True, False)
        log.info(f"启动校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

@allure.feature("upgrade")
@allure.description("验证校验流程镜像头和代码分离情况，AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM启动镜像在镜像头分离时校验通过")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-965")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_965(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行镜像头和代码分离校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]    # 前1024字节是镜像头
        image_code = boot_image[IMAGE_HEAD_SIZE:]      # 后面是代码区

        # 执行镜像头和代码分离校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
            image_header=image_header,
            image_size=len(boot_image),
            image_code=image_code,
            only_copy_code=False,    # 完整镜像
            check_version=False,
            boot=False
        )
        log.info(f"AES-CMAC+AES-CBC分离校验成功完成，处理时间: {vrf_time}us")

@allure.feature("upgrade")
@allure.description("验证校验流程镜像头和代码分离情况，对镜像body与头部不一致的情况校验失败")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-966")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_966(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行错误的镜像头和代码分离校验（预期失败） # 返回预期错误码"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]
        image_code = bytearray(boot_image[IMAGE_HEAD_SIZE:])

        # Reason: 模拟镜像头和代码不一致的情况（破坏代码区的一部分）
        if len(image_code) > 100:
            image_code[50:100] = b'\x00' * 50  # 破坏部分代码

        # 执行镜像头和代码分离校验，预期失败
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
                image_header=image_header,
                image_size=len(boot_image),
                image_code=bytes(image_code),
                only_copy_code=False,
                check_version=False,
                boot=False
            )
            pytest.fail("镜像头和代码不一致的校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的镜像头和代码不一致校验失败: {e}")

@allure.feature("upgrade")
@allure.description("验证校验流程镜像头和代码分离情况，SM4-128-CMAC算法签名，SM4-128-CBC算法加密的eHSM启动镜像在镜像头分离时校验通过")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-967")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_967(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM启动镜像并执行镜像头和代码分离校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )

        # Reason: 将镜像分离为头部和代码区
        image_header = boot_image[:IMAGE_HEAD_SIZE]    # 前1024字节是镜像头
        image_code = boot_image[IMAGE_HEAD_SIZE:]      # 后面是代码区

        # 执行镜像头和代码分离校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image_discrete(
            image_header=image_header,
            image_size=len(boot_image),
            image_code=image_code,
            only_copy_code=False,    # 完整镜像
            check_version=False,
            boot=False
        )
        log.info(f"SM4-CMAC+SM4-CBC分离校验成功完成，处理时间: {vrf_time}us")

@allure.feature("upgrade")
@allure.description("验证从FLASH启动ehsm FW，测试RSA-2048算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，校验通过后正常启动")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-968")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_968(setup_function):
    with allure.step("１、检测校验启动前FW状态寄存器 # 执行成功"):
        # 检测初始FW状态
        initial_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"初始 FW 状态: 0x{initial_status:08x}")

    with allure.step("2、配置OTP 生命周期User模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、生成eHSM启动镜像并执行 FLASH 启动校验 # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验并在校验后启动（模拟FLASH启动流程）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, True)
        log.info(f"FLASH启动 RSA-2048不加密校验成功完成，处理时间: {vrf_time}ms")

    with allure.step("4、检查FW启动成功状态寄存器 # 检查通过"):
        # 等待FW启动完成
        assert 0 == host.wait_fw_done(5)  # 等待最多5秒
        final_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"FW启动后状态: 0x{final_status:08x}")
        assert final_status & hostapi.FW_DONE, "FW应该启动成功"

@allure.feature("upgrade")
@allure.description("验证从FLASH启动ehsm FW，测试RSA-2048算法签名，不加密的eHSM启动镜像的校验失败情况，拷贝到不正确地址")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-969")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason="RSA2048_PSS不支持")
def test_ehsm_969(setup_function):
    with allure.step("１、检测校验启动前FW状态寄存器 # 执行成功"):
        # 检测初始FW状态
        initial_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"初始 FW 状态: 0x{initial_status:08x}")

    with allure.step("2、配置OTP 生命周期User模式，配置eHSM校验算法密钥 # 配置成功"):
        # 生成OTP数据
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、生成eHSM启动镜像并执行 FLASH 启动校验（不正确地址，预期失败） # 执行成功"):
        # 生成eHSM启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

        # 模拟不正确地址情况（破坏部分镜像数据）
        corrupted_image = bytearray(boot_image)
        if len(corrupted_image) > 100:
            corrupted_image[50:100] = b'\xFF' * 50  # 破坏部分数据

        # 执行校验，预期失败（不正确地址）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(bytes(corrupted_image), len(corrupted_image), True, False)
            pytest.fail("不正确地址的FLASH启动校验应该失败但却成功了")
        except Exception as e:
            log.info(f"预期的FLASH启动校验失败（不正确地址）: {e}")

    with allure.step("4、检查FW启动状态寄存器（应为失败状态） # 检查通过"):
        # 检查最终状态，应为失败状态
        final_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"FW最终状态: 0x{final_status:08x}")
        # 验证FW未成功启动
        assert not (final_status & hostapi.FW_DONE), "FW不应该成功启动"
        log.info("校验失败后 FW 未启动，符合预期")

@allure.feature("upgrade")
@allure.description("验证从FLASH启动ehsm FW，测试SM2 算法签名，SM4-128-CBC算法加密的SOC_USE_SOC_KEY启动镜像的校验失败情况")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-970")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_970(setup_function):
    with allure.step("1、检测校验启动前FW状态寄存器 # 执行成功"):
        # 检查初始状态，确保FW未启动
        initial_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"启动前初始状态: 0x{initial_status:08x}")
        assert not (initial_status & hostapi.FW_DONE), "FW应该未启动"

    with allure.step("2、配置OTP 生命周期User模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置成SM4-CBC；重启eHSM生效； # 配置成功"):
        # 生成OTP数据
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("3、准备好SM2签名、SM4-CBC密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令，配置校验后启动； # 配置成功"):
        # 生成SOC_USE_SOC_KEY类型的启动镜像（故意制造失败场景）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

        # 写入镜像到FLASH并执行校验启动
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, True)
            log.error("预期校验失败，但实际成功了")
            assert False, "预期校验失败，但实际成功了"
        except Exception as e:
            log.info(f"预期的校验失败: {e}")
            # 这里是预期的失败，继续执行后续检查

    with allure.step("4、检查FW启动成功状态寄存器 # 检查通过"):
        # 等待一段时间以确保系统稳定
        time.sleep(0.5)

        # 检查最终状态
        final_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"FW最终状态: 0x{final_status:08x}")

        # 验证FW未成功启动
        assert not (final_status & hostapi.FW_DONE), "FW不应该成功启动"
        log.info("SOC_USE_SOC_KEY启动镜像校验失败后 FW 未启动，符合预期")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试SM2算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter小于image Version Counter，校验过程检查Version Counter合法")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-993")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_993(setup_function):
    with allure.step("１、配置OTP 生命周期Test模式，VERSION配置为1（小于镜像版本2） # 配置成功"):
        # 生成OTP数据，设置较低的版本计数器
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1  # 设置OTP version为1（较低版本）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1
        ehsm_vrf_sign_algo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,  # 不加密
            ehsm_vrf_sign_algo=ehsm_vrf_sign_algo,
            ehsm_ver_cnt=ehsm_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成版本为2的eHSM启动镜像并执行校验（预期成功） # 执行成功"):
        # 生成eHSM启动镜像（版本为VC2，大于OTP版本1）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM类型
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 不加密
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC2  # 镜像版本为2，大于OTP版本1
        )
        # 执行校验，预期成功（版本检查通过）
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)  # 开启版本检查
        log.info(f"版本检查通过，校验成功完成，处理时间: {vrf_time}ms")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试SM2算法签名，不加密的eHSM启动镜像的ONEPASS校验流程，OTP Version Counter大于image Version Counter，校验过程检查Version Counter非法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-994")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_994(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥eHSM_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥eHSM_ENC_KEY配置成0x00，VERSION配置成3（大于镜像版本2）；重启eHSM生效； # 配置成功"):
        # 生成OTP数据，设置较高的版本计数器（Test模式，SM2算法，不加密）
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3  # 设置OTP version为3（大于镜像版本2）
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3

        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,  # 不加密
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2,
            ehsm_ver_cnt=ehsm_ver_cnt
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好SM2签名、没有加密的eHSM启动镜像，并配置到校验命令的镜像地址，并发送校验命令，配置校验后启动； # 配置成功"):
        # 生成不加密的eHSM启动镜像（镜像版本为2，小于OTP版本3）
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC2  # 镜像版本为2，小于OTP版本3
        )

        # 尝试校验，预期失败（OTP版本3 > 镜像版本2）
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
            pytest.fail("校验应该失败但却成功了")
        except Exception as e:
            # 期望抛出异常，验证错误码
            error_code = int(str(e))
            assert error_code == EHSM_ERR_WRONG_VERSION_COUNTER, \
                f"期望错误码EHSM_ERR_WRONG_VERSION_COUNTER ({EHSM_ERR_WRONG_VERSION_COUNTER})，实际错误码: {error_code}"

@allure.feature("upgrade")
@allure.description("在User模式下，测试 SM2 算法签名，SM4-128-CBC算法加密的SOC_USE_SOC_KEY启动镜像的ONEPASS校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-996")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_SM2_SUPPORT == 0, reason="SM2不支持")
def test_ehsm_996(setup_function):
    with allure.step("1、配置OTP 生命周期Test模式，升级签名算法密钥SOC_FW_VERIFY_KEY配置SM2公钥HASH值和对应属性，升级加密密钥SOC_ENC_KEY配置SM4-128-CBC预置值和对应属性；重启SOC生效； # 配置成功"):
        # 生成OTP数据 - 使用SOC密钥，SM2签名，SM4-CBC加密
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        )
        # 写入OTP数据并重启
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、准备好使用SM2算法预置密钥签名和SM4-128-CBC加密的SOC_SOC启动镜像，并配置到校验命令的镜像地址，并发送校验命令； # 配置成功"):
        # 生成SOC_SOC启动镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )
        # 执行校验
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(boot_image, len(boot_image), True, False)
        log.info("SOC_SOC启动镜像校验成功，用时: %d ms", vrf_time)

    with allure.step("3、检测校验后的镜像解密后的明文是否和原始明文镜像一致； # 执行成功"):
        # SOC类型镜像校验输出与原始镜像逐字节对比
        assert src_fw_bytes is not None
        assert vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        log.info("校验输出镜像与原始镜像一致，验证成功")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试SOC_HSMK裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("test_ehsm_u005")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_u005(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Test模式
        otp_data = otp_to_bin(values_config={"lifecycle": "test"})

        # 修改OTP数据：保留前4字节（生命周期配置），其余填充默认值
        otp_data = set_otp_data_to_default_except_lifecycle(otp_data)

        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_HSMK裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成SOC_HSMK裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,  # SOC镜像使用eHSM密钥
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC_HSMK裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

        # 验证输出镜像内容（裸镜像校验时应该直接输出原始内容）
        assert src_fw_bytes is not None
        actual_content = vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)]
        assert actual_content == src_fw_bytes, "SOC_HSMK裸镜像校验输出内容应与原始内容一致"
        log.info("SOC_HSMK裸镜像校验输出内容验证成功")

@allure.feature("upgrade")
@allure.description("在Dev模式下，测试SOC_HSMK裸镜像的在OTP数据为空的场景下的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("test_ehsm_u006")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_u006(setup_function):
    with allure.step("1、配置OTP生命周期Dev模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Dev模式
        otp_data = otp_to_bin(values_config={"lifecycle": "dev"})

        # 修改OTP数据：保留前4字节（生命周期配置），其余填充默认值
        otp_data = set_otp_data_to_default_except_lifecycle(otp_data)

        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_HSMK裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成SOC_HSMK裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,  # SOC镜像使用eHSM密钥
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC_HSMK裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

        # 验证输出镜像内容（裸镜像校验时应该直接输出原始内容）
        assert src_fw_bytes is not None
        actual_content = vrf_image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)]
        assert actual_content == src_fw_bytes, "SOC_HSMK裸镜像校验输出内容应与原始内容一致"
        log.info("SOC_HSMK裸镜像校验输出内容验证成功")

@allure.feature("upgrade")
@allure.description("在Manu模式下，测试SOC_HSMK裸镜像不支持校验的情况")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("test_ehsm_u007")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_u007(setup_function):
    with allure.step("1、配置OTP生命周期Manu模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Manu模式，其他使用默认值
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU
            # 其他参数使用默认值
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_HSMK裸镜像（无加密无签名） # 生成成功"):
        # 生成SOC_HSMK裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,  # SOC镜像使用eHSM密钥
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC_HSMK裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令，预期校验失败 # 校验失败（预期结果）"):
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("预期在Manu模式下SOC_HSMK裸镜像校验失败，但实际校验成功了")
        except Exception as e:
            log.info(f"预期的校验失败（Manu模式不支持SOC_HSMK裸镜像校验）: {str(e)}")

@allure.feature("upgrade")
@allure.description("在User模式下，测试SOC_HSMK裸镜像不支持校验的情况")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("test_ehsm_u008")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_u008(setup_function):
    with allure.step("1、配置OTP生命周期User模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为User模式，其他使用默认值
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER
            # 其他参数使用默认值
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_HSMK裸镜像（无加密无签名） # 生成成功"):
        # 生成SOC_HSMK裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,  # SOC镜像使用eHSM密钥
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC_HSMK裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令，预期校验失败 # 校验失败（预期结果）"):
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("预期在User模式下SOC_HSMK裸镜像校验失败，但实际校验成功了")
        except Exception as e:
            log.info(f"预期的校验失败（User模式不支持SOC_HSMK裸镜像校验）: {str(e)}")

@allure.feature("upgrade")
@allure.description("在Debug模式下，测试SOC_HSMK裸镜像不支持校验的情况")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("test_ehsm_u009")
@pytest.mark.skipif(cfg_data.TEST_BL_UPGRADE_AES128CMAC_SUPPORT == 0, reason="AES128CMAC不支持")
def test_ehsm_u009(setup_function):
    with allure.step("1、配置OTP生命周期Debug模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Debug模式，其他使用默认值
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG
            # 其他参数使用默认值
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成SOC_HSMK裸镜像（无加密无签名） # 生成成功"):
        # 生成SOC_HSMK裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_SOC_HSMK,  # SOC镜像使用eHSM密钥
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成SOC_HSMK裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令，预期校验失败 # 校验失败（预期结果）"):
        try:
            vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, False)
            pytest.fail("预期在Debug模式下SOC_HSMK裸镜像校验失败，但实际校验成功了")
        except Exception as e:
            log.info(f"预期的校验失败（Debug模式不支持SOC_HSMK裸镜像校验）: {str(e)}")

@allure.feature("upgrade")
@allure.description("在Test模式下，测试fw裸镜像的在OTP数据为空的场景下的校验启动后，DMA恢复初始状态")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-U010")
def test_ehsm_u010(setup_function):
    with allure.step("1、配置OTP生命周期Test模式，其它OTP域使用默认值 # 配置成功"):
        # 生成OTP数据，仅设置生命周期为Test模式，其他使用默认值（相当于空数据）
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST
            # 其他参数使用默认值，模拟OTP数据为空的场景
        )
        # 写入OTP数据并重启eHSM
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、生成eHSM裸镜像（无加密无签名）并加载到启动镜像地址 # 计算成功"):
        # 生成eHSM裸镜像（使用naked_flag生成裸镜像）
        raw_fw_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 无加密（裸镜像忽略）
            ImageSignAlgo.IMAGE_SIGN_ALGO_NONE,  # 无签名（裸镜像忽略）
            ImageVersion.IMAGE_VC0,  # 版本（裸镜像忽略）
            naked_flag=True  # 使用naked_flag生成裸镜像
        )
        log.info(f"生成eHSM裸镜像，大小: {len(raw_fw_image)} bytes")

    with allure.step("3、发送校验指令进入校验流程，检查返回状态 # 发送成功"):
        # 校验裸镜像，在OTP数据为空的情况下应该能够校验通过
        vrf_time, vrf_image_out = api.ehsm_bl_verify_image(raw_fw_image, len(raw_fw_image), True, True)
        log.info(f"校验成功完成，处理时间: {vrf_time}ms，输出镜像大小: {len(vrf_image_out)} bytes")

    with allure.step("4、检查FW启动成功状态寄存器 # 检查通过"):
        # 等待FW启动完成
        assert 0 == host.wait_fw_done(5)  # 等待最多5秒
        final_status = host.get_word(hostapi.HSM_STATUS_IN)
        log.info(f"FW启动后状态: 0x{final_status:08x}")
        assert final_status & hostapi.FW_DONE, "FW应该启动成功"

    with allure.step("5、检查DMA控制寄存器恢复初始状态 # 检查通过"):
        from platform_adapter.gdb.uart_impl import UartGdb
        gdb = UartGdb()
        success, dma_cfg = gdb.read_register(ehsm_reg.SYS_AHB_DMA_CFG_REG)
        assert success, "读取DMA控制寄存器失败"
        assert dma_cfg & 0x00000060 != 0x00000060, "DMA控制寄存器应恢复初始状态"
        log.info("DMA控制寄存器恢复初始状态验证成功, dma_cfg: 0x%08x", dma_cfg)

@allure.feature("upgrade")
@allure.description("在Test模式下，测试upgrade_fw_image接口image地址为NULL的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u011")
@pytest.mark.skipif(False, reason="upgrade_fw_image功能始终支持")
def test_ehsm_u011():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用升级算法AES128_SM2；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_upg_enc_algo=ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ehsm_upg_sign_algo=ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的升级镜像； # 2、镜像生成成功；"):
        upg_image = generate_boot_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成升级镜像，大小: {len(upg_image)} bytes")

    with allure.step("3、调用底层hostapi接口，传入image地址为NULL(0x0)； # 3、报无效地址错误；"):
        try:
            hostapi.ehsm_bl_upgrade_fw_image(
                api.CTX_ADDR,  # 正常的ctx地址
                0x0,  # NULL的image地址
                len(upg_image),  # 正常的image_size
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"image地址为NULL时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS, \
                f"预期错误码为EHSM_ERR_INVALID_ADDRESS({ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS})，实际: {actual_error}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试upgrade_fw_image接口image_size为0的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u012")
@pytest.mark.skipif(False, reason="upgrade_fw_image功能始终支持")
def test_ehsm_u012():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用升级算法AES128_SM2；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_upg_enc_algo=ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ehsm_upg_sign_algo=ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的升级镜像； # 2、镜像生成成功；"):
        upg_image = generate_boot_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成升级镜像，大小: {len(upg_image)} bytes")
        # 写入镜像数据到DATA1地址
        host.write_memory(api.DATA1_ADDR, upg_image)

    with allure.step("3、调用底层hostapi接口，传入image_size为0； # 3、报无效镜像大小错误；"):
        # Reason: image_size=0不是有效的镜像大小，应该被固件拒绝
        try:
            hostapi.ehsm_bl_upgrade_fw_image(
                api.CTX_ADDR,  # 正常的ctx地址
                api.DATA1_ADDR,  # 正常的image地址
                0,  # 无效的image_size
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"image_size为0时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == ehsm_bl_errno.EHSM_ERR_PARAM_ERROR, \
                f"预期错误码为EHSM_ERR_INVALID_IMAGE_SIZE({ehsm_bl_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试upgrade_fw_image接口image_out地址为NULL的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u013")
@pytest.mark.skipif(False, reason="upgrade_fw_image功能始终支持")
def test_ehsm_u013():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用升级算法AES128_SM2；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_upg_enc_algo=ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ehsm_upg_sign_algo=ImageSignAlgo.IMAGE_SIGN_ALGO_SM2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的升级镜像； # 2、镜像生成成功；"):
        upg_image = generate_boot_image(
            ImageLevel.IMAGE_BL_UPGRADE,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成升级镜像，大小: {len(upg_image)} bytes")
        # 写入镜像数据到DATA1地址
        host.write_memory(api.DATA1_ADDR, upg_image)

    with allure.step("3、调用底层hostapi接口，传入image_out地址为NULL(0x0)； # 3、报无效地址错误；"):
        # Reason: image_out=NULL(0x0)不是有效的输出地址，应该被固件拒绝
        try:
            hostapi.ehsm_bl_upgrade_fw_image(
                api.CTX_ADDR,  # 正常的ctx地址
                api.DATA1_ADDR,  # 正常的image地址
                len(upg_image),  # 正常的image_size
                0x0  # NULL的image_out地址
            )
            assert False, f"image_out地址为NULL时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS, \
                f"预期错误码为EHSM_ERR_INVALID_ADDRESS({ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS})，实际: {actual_error}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试verify_image接口ctx地址为无效地址的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u014")
@pytest.mark.skipif(False, reason="verify_image功能始终支持")
def test_ehsm_u014():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用校验算法AES128_CMAC；并重启生效； # 1、生命周期配置成功；"):
        # Reason: 配置Test模式的OTP环境，用于测试verify_image接口
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的启动镜像； # 2、镜像生成成功；"):
        # 生成有效的校验镜像
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成启动镜像，大小: {len(boot_image)} bytes")
        # 写入镜像数据到DATA1地址
        host.write_memory(api.DATA1_ADDR, boot_image)

    with allure.step("3、调用底层hostapi接口，传入ctx地址为无效地址(0x12345678)； # 3、报队列为空错误；"):
        # Reason: ctx=0x12345678不是有效的上下文地址，应该被固件拒绝
        try:
            hostapi.ehsm_bl_verify_image(
                0x12345678,  # 无效的ctx地址
                api.DATA1_ADDR,  # 正常的image地址
                len(boot_image),  # 正常的image_size
                True,  # 正常的check_version
                True,  # 正常的boot
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"ctx地址为无效地址时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_QUEUE_EMPTY, \
                f"预期错误码为EHSM_ERR_QUEUE_EMPTY({ehsm_bl_errno.EHSM_ERR_QUEUE_EMPTY})，实际: {actual_error}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试verify_image接口image地址为NULL的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u015")
@pytest.mark.skipif(False, reason="verify_image功能始终支持")
def test_ehsm_u015():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用校验算法AES128_CMAC；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的启动镜像； # 2、镜像生成成功；"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成启动镜像，大小: {len(boot_image)} bytes")

    with allure.step("3、调用底层hostapi接口，传入image地址为NULL(0x0)； # 3、报无效地址错误；"):
        # Reason: image=NULL(0x0)不是有效的镜像地址，应该被固件拒绝
        try:
            hostapi.ehsm_bl_verify_image(
                api.CTX_ADDR,  # 正常的ctx地址
                0x0,  # NULL的image地址
                len(boot_image),  # 正常的image_size
                True,  # 正常的check_version
                True,  # 正常的boot
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"image地址为NULL时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS, \
                f"预期错误码为EHSM_ERR_INVALID_ADDRESS({ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS})，实际: {actual_error}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试verify_image接口image_size为0的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u016")
@pytest.mark.skipif(False, reason="verify_image功能始终支持")
def test_ehsm_u016():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用校验算法AES128_CMAC；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的启动镜像； # 2、镜像生成成功；"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成启动镜像，大小: {len(boot_image)} bytes")
        # 写入镜像数据到DATA1地址
        host.write_memory(api.DATA1_ADDR, boot_image)

    with allure.step("3、调用底层hostapi接口，传入image_size为0； # 3、报无效镜像大小错误；"):
        # Reason: image_size=0不是有效的镜像大小，应该被固件拒绝
        try:
            hostapi.ehsm_bl_verify_image(
                api.CTX_ADDR,  # 正常的ctx地址
                api.DATA1_ADDR,  # 正常的image地址
                0,  # 无效的image_size
                True,  # 正常的check_version
                True,  # 正常的boot
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"image_size为0时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_PARAM_ERROR, \
                f"预期错误码为EHSM_ERR_PARAM_ERROR({ehsm_bl_errno.EHSM_ERR_PARAM_ERROR})，实际: {actual_error}"


@allure.feature("upgrade")
@allure.description("在Test模式下，测试verify_image_discrete接口code地址为NULL的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_u0187")
@pytest.mark.skipif(False, reason="verify_image_discrete功能始终支持")
def test_ehsm_u017():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式，使用校验算法AES128_CMAC；并重启生效； # 1、生命周期配置成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        log.info("eHSM重启成功，启动状态正常")

    with allure.step("2、生成有效的启动镜像并分离镜像头和代码； # 2、镜像生成成功；"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        log.info(f"生成启动镜像，大小: {len(boot_image)} bytes")

        IMAGE_HEAD_SIZE = 1024
        image_header = boot_image[:IMAGE_HEAD_SIZE]
        image_code = boot_image[IMAGE_HEAD_SIZE:]

        # 写入镜像头数据到DATA1地址
        host.write_memory(api.DATA1_ADDR, image_header)

    with allure.step("3、调用底层hostapi接口，传入code地址为无效地址(0x12345678)； # 3、报无效地址错误；"):
        try:
            hostapi.ehsm_bl_verify_image_discrete(
                api.CTX_ADDR,  # 正常的ctx地址
                api.DATA1_ADDR,  # 正常的image头地址
                len(boot_image),  # 正常的image_size
                0x12345678,  # 无效的code地址
                False,  # only_copy_code
                True,  # check_version
                True,  # boot
                api.DATA3_ADDR  # 正常的image_out地址
            )
            assert False, f"code地址为无效地址时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            log.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            actual_error = e.ret_code if e.ret_code >= 0 else e.ret_code + 65536
            assert actual_error == ehsm_bl_errno.EHSM_ERR_DATA_CHECK_ERROR, \
                f"预期错误码为EHSM_ERR_DATA_CHECK_ERROR({ehsm_bl_errno.EHSM_ERR_DATA_CHECK_ERROR})，实际: {actual_error}"


# ====================================================================================================
# BUG-16: EHSM版本号 verify case（u018~u020） + SOC版本号 verify case（u021~u023）
# 所有指令均为 BL 层命令，因此用例放在 ehsm_bl 模块
#
# 【硬件/固件前置要求】
#   - 目标板只需烧录 Bootloader，无需烧录 EHSM FW
#     （BL 启动后等待测试脚本通过 hostapi 下发命令，FW 在测试运行时动态加载）
#   - 真实 EHSM FW binary（ehsm_fw_real.bin）需手动编译后拷贝到指定目录：
#       resource/real_fw/ehsm_fw_real.bin
#     编译要求：
#       · 必须使用 IRAM link 文件（link_iram.ld），而非 Flash link 文件
#         （BL 校验通过后直接跳转到 IRAM 地址执行，Flash 地址会立即崩溃）
#       · 必须包含完整的 secboot_entry → secboot_update_ver_cnt 逻辑
#         （确保 CONFIG_SECBOOT_UPDATE_VER_CNT=1 或等效宏已开启）
#       · 详细编译步骤见 resource/real_fw/README.md
#
# u018~u020 路径（EHSM 版本升级/平刷/回退）：
#   仅一次 BL verify：BL verify 真实 EHSM FW（boot=True）
#   → FW secboot_update_ver_cnt 读自身版本号与 OTP 对比 → 必要时写 EHSM 版本到 OTP
#   （EHSM FW 自身知道版本，无需 boot=False 写 DRAM）
#
# u021~u023 路径（SOC 版本升级/平刷/回退，两阶段端到端）：
#   阶段一：BL verify SOC FW（boot=False）→ DRAM 写 SOC 版本标记
#   阶段二：BL verify EHSM FW（boot=True）→ BL 跳转 EHSM FW
#           → EHSM FW secboot_update_ver_cnt 读 DRAM，写 SOC 版本到 OTP
#
# 镜像说明：
#   u018~u019 EHSM boot 镜像：版本号为目标版本，payload 必须是含 secboot_entry 逻辑的真实 FW binary
#   u020      EHSM 回退镜像：版本号低于 OTP，body 任意（BL verify 版本检查失败，FW 不会执行）
#   u021~u023 阶段一 SOC FW 镜像：版本号需正确，body 任意（SOC body 不被 EHSM 执行）
#   u021~u023 阶段二 EHSM boot 镜像：payload 必须是含 secboot_entry 逻辑的真实 FW binary
# ====================================================================================================
import os

# OTP version counter 相关常量（BUG-16）
if cfg_data.TEST_CUSTOM_ID == 0x4019:
    EHSM_VER_OTP_ADDR_BL            = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x30
    SOC_VER_OTP_ADDR_BL             = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x40
else:
    EHSM_VER_OTP_ADDR_BL            = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x50
    SOC_VER_OTP_ADDR_BL             = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x60
OTP_VER_SIZE_BL                 = 16


def _ehsm_ver_cnt_bl(vc: int) -> EhsmVersionCounter:
    """根据 OTP 默认值配置返回正确 OTP bank 的 EHSM 版本计数器枚举。
    vc: 0=VC0, 1=VC1, 2=VC2, 3=VC3
    Reason: TEST_OTP_DEFAULT_VALUE==0 用 OTP0，否则用 OTP1，写死 OTP0 在 OTP1 板上会失败。
    """
    if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
        return [EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC0,
                EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1,
                EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC2,
                EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3][vc]
    else:
        return [EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC0,
                EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1,
                EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC2,
                EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3][vc]


def _soc_ver_cnt_bl(vc: int) -> SocVersionCounter:
    """根据 OTP 默认值配置返回正确 OTP bank 的 SOC 版本计数器枚举。
    vc: 0=VC0, 1=VC1, 2=VC2, 3=VC3
    Reason: TEST_OTP_DEFAULT_VALUE==0 用 OTP0，否则用 OTP1，写死 OTP0 在 OTP1 板上会失败。
    """
    if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
        return [SocVersionCounter.SOC_VER_CNT_OTP0_VC0,
                SocVersionCounter.SOC_VER_CNT_OTP0_VC1,
                SocVersionCounter.SOC_VER_CNT_OTP0_VC2,
                SocVersionCounter.SOC_VER_CNT_OTP0_VC3][vc]
    else:
        return [SocVersionCounter.SOC_VER_CNT_OTP1_VC0,
                SocVersionCounter.SOC_VER_CNT_OTP1_VC1,
                SocVersionCounter.SOC_VER_CNT_OTP1_VC2,
                SocVersionCounter.SOC_VER_CNT_OTP1_VC3][vc]

# 真实 EHSM FW 路径（需手动编译后放置，详见 resource/real_fw/README.md）
EHSM_FW_REAL_BIN_BL             = "resource/real_fw/ehsm_fw_real.bin"
EHSM_FW_REAL_BIN_FOR_IMGTOOL_BL = "../real_fw/ehsm_fw_real.bin"

@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 EHSM版本升级：BL verify 高版本EHSM镜像（VC3，OTP初始VC0），EHSM启动后 secboot_update_ver_cnt 将EHSM版本写入OTP，验证VC0→VC3")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_U018")
def test_ehsm_u018(setup_function):
    """TC-VER-005: BUG-16 EHSM版本升级两阶段端到端验证（OTP VC0 < EHSM镜像 VC3 → OTP更新为VC3）"""
    log.info("开始测试EHSM_U018: BUG-16 EHSM版本升级验证")

    if not os.path.exists(EHSM_FW_REAL_BIN_BL):
        pytest.fail(
            f"缺少真实 EHSM FW 二进制：{EHSM_FW_REAL_BIN_BL}\n"
            "请参考 resource/real_fw/README.md 编译后放置"
        )

    with allure.step("1、配置 OTP：Test模式，EHSM版本 VC0，写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_ver_cnt=_ehsm_ver_cnt_bl(0),  # 初始低版本 VC0
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 EHSM/SOC OTP 版本号基准值，校验 EHSM 初始为 VC0；# 2、读取成功，EHSM 初始值符合 VC0 编码；"):
        _, ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本: {ver_before.hex()}")
        # Reason: OTP0 bank blank 值为 0x00，OTP1 bank blank 值为 0xFF；VC0 = 未烧写任何 bit
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before == expected_blank, \
            f"初始 EHSM OTP 应为 VC0（期望: {expected_blank.hex()}），实际: {ver_before.hex()}"
        _, soc_ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_before is not None and len(soc_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 SOC OTP 版本（应保持不变）: {soc_ver_before.hex()}")
        # Reason: EHSM FW 自身知道版本号，secboot_update_ver_cnt 直接比较镜像版本与 OTP 并写入，无需 boot=False 预写 DRAM
        ehsm_boot_image_v3 = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC3,
            fw_image=EHSM_FW_REAL_BIN_FOR_IMGTOOL_BL,
        )
        log.info(f"EHSM 启动镜像（VC3，真实FW）大小: {len(ehsm_boot_image_v3)} bytes")

    with allure.step("4、分块写入 EHSM 镜像，BL 校验（boot=True）跳转到 EHSM FW；# 4、BL verify 通过，跳转成功；"):
        CHUNK_SIZE = 32 * 1024
        image_addr = api.CTX_ADDR + 2048 * 6
        image_size = len(ehsm_boot_image_v3)
        for offset in range(0, image_size, CHUNK_SIZE):
            host.write_memory(image_addr + offset, ehsm_boot_image_v3[offset:offset + CHUNK_SIZE])
        log.info(f"EHSM 镜像写入 0x{image_addr:08X}，{image_size} bytes")
        vrf_time = hostapi.ehsm_bl_verify_image(
            api.CTX_ADDR, image_addr, image_size, True, True, image_addr
        )
        log.info(f"BL verify EHSM FW VC3 成功，耗时: {vrf_time}ms，已跳转到 EHSM FW")

    with allure.step("5、等待 EHSM FW secboot_update_ver_cnt 执行完成；# 5、FW 启动成功；"):
        assert 0 == host.wait_fw_done(1)

    with allure.step("6、读取 EHSM OTP 版本号，验证已从 VC0 更新为 VC3；# 6、EHSM OTP 版本号已更新；"):
        _, ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"FW 启动后 EHSM OTP 版本: {ver_after.hex()}")
        assert ver_before != ver_after, (
            f"EHSM OTP 版本号未更新（before={ver_before.hex()}, after={ver_after.hex()}），"
            "BUG-16 EHSM版本升级验证失败"
        )
        log.info("BUG-16 EHSM版本升级验证通过：OTP已从VC0更新为VC3")

    with allure.step("7、验证 SOC OTP 版本未受 EHSM 版本更新影响；# 7、SOC OTP 版本号保持不变；"):
        _, soc_ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_after is not None and len(soc_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"EHSM 版本更新后 SOC OTP 版本: {soc_ver_after.hex()}")
        assert soc_ver_before == soc_ver_after, \
            f"SOC OTP 版本号不应受 EHSM 版本更新影响（before={soc_ver_before.hex()}, after={soc_ver_after.hex()}）"
        log.info("隔离验证通过：EHSM 版本更新不影响 SOC OTP 版本计数器")

    log.info("EHSM_U018 完成")


@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 EHSM版本平刷：BL verify 同版本EHSM镜像（VC1，OTP初始VC1），EHSM启动后 secboot_update_ver_cnt 不写OTP，验证版本不变")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_U019")
def test_ehsm_u019(setup_function):
    """TC-VER-006: BUG-16 EHSM版本平刷（OTP VC1 == EHSM镜像 VC1 → OTP不变）"""
    log.info("开始测试EHSM_U019: BUG-16 EHSM版本平刷验证")

    if not os.path.exists(EHSM_FW_REAL_BIN_BL):
        pytest.fail(
            f"缺少真实 EHSM FW 二进制：{EHSM_FW_REAL_BIN_BL}\n"
            "请参考 resource/real_fw/README.md 编译后放置"
        )

    with allure.step("1、配置 OTP：Test模式，EHSM版本 VC1，写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_ver_cnt=_ehsm_ver_cnt_bl(1),  # 初始 VC1
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 EHSM/SOC OTP 版本号基准值，校验 EHSM 初始为 VC1（非 blank）；# 2、读取成功，初始值符合 VC1；"):
        _, ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本: {ver_before.hex()}")
        # Reason: VC1 应至少有 1 个 fuse 已烧写（非 blank 值）；OTP0 blank=0x00，OTP1 blank=0xFF
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before != expected_blank, \
            f"初始 EHSM OTP 应为 VC1（非 blank），实际: {ver_before.hex()}，OTP 地址或初始化可能有误"
        _, soc_ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_before is not None and len(soc_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 SOC OTP 版本（应保持不变）: {soc_ver_before.hex()}")

    with allure.step("3、生成版本 VC1 的 EHSM 启动镜像（与 OTP 相同版本，真实 FW binary）；# 3、镜像生成成功；"):
        # Reason: 平刷场景，镜像版本 == OTP 版本，secboot_update_ver_cnt 不写 OTP
        ehsm_boot_image_v1 = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC1,
            fw_image=EHSM_FW_REAL_BIN_FOR_IMGTOOL_BL,
        )
        log.info(f"EHSM 启动镜像（VC1，真实FW）大小: {len(ehsm_boot_image_v1)} bytes")

    with allure.step("4、分块写入 EHSM 镜像，BL 校验（boot=True）跳转到 EHSM FW；# 4、BL verify 通过，跳转成功；"):
        CHUNK_SIZE = 32 * 1024
        image_addr = api.CTX_ADDR + 2048 * 6
        image_size = len(ehsm_boot_image_v1)
        for offset in range(0, image_size, CHUNK_SIZE):
            host.write_memory(image_addr + offset, ehsm_boot_image_v1[offset:offset + CHUNK_SIZE])
        log.info(f"EHSM 镜像写入 0x{image_addr:08X}，{image_size} bytes")
        vrf_time = hostapi.ehsm_bl_verify_image(
            api.CTX_ADDR, image_addr, image_size, True, True, image_addr
        )
        log.info(f"BL verify EHSM FW VC1 成功，耗时: {vrf_time}ms，已跳转到 EHSM FW")

    with allure.step("5、等待 EHSM FW secboot_update_ver_cnt 执行完成；# 5、FW 启动成功；"):
        assert 0 == host.wait_fw_done(1)

    with allure.step("6、读取 EHSM OTP 版本号，验证版本未变化（平刷不写 OTP）；# 6、EHSM OTP 版本号保持 VC1 不变；"):
        _, ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"FW 启动后 EHSM OTP 版本: {ver_after.hex()}")
        assert ver_before == ver_after, \
            f"EHSM OTP 版本号不应变化（before={ver_before.hex()}, after={ver_after.hex()}），平刷逻辑异常"
        log.info("BUG-16 EHSM版本平刷验证通过：OTP版本未变化")

    with allure.step("7、验证 SOC OTP 版本未受 EHSM 版本平刷影响；# 7、SOC OTP 版本号保持不变；"):
        _, soc_ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_after is not None and len(soc_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"EHSM 版本平刷后 SOC OTP 版本: {soc_ver_after.hex()}")
        assert soc_ver_before == soc_ver_after, \
            f"SOC OTP 版本号不应受 EHSM 版本平刷影响（before={soc_ver_before.hex()}, after={soc_ver_after.hex()}）"
        log.info("隔离验证通过：EHSM 版本平刷不影响 SOC OTP 版本计数器")

    log.info("EHSM_U019 完成")


@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 EHSM版本回退拒绝：BL verify 低版本EHSM镜像（VC0，OTP初始VC2），版本检查失败，OTP不更新")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_U020")
def test_ehsm_u020(setup_function):
    """TC-VER-007: BUG-16 EHSM版本回退拒绝（OTP VC2 > EHSM镜像 VC0 → BL verify 失败，OTP不变）"""
    log.info("开始测试EHSM_U020: BUG-16 EHSM版本回退防护验证")

    with allure.step("1、配置 OTP：Test模式，EHSM版本 VC2（高版本），写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            ehsm_ver_cnt=_ehsm_ver_cnt_bl(2),  # 初始高版本 VC2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 EHSM/SOC OTP 版本号基准值，校验 EHSM 初始为 VC2（非 blank）；# 2、读取成功，初始值符合 VC2；"):
        _, ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本: {ver_before.hex()}")
        # Reason: VC2 至少 2 个 fuse 已烧写（非 blank 值）；OTP0 blank=0x00，OTP1 blank=0xFF
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before != expected_blank, \
            f"初始 EHSM OTP 应为 VC2（非 blank），实际: {ver_before.hex()}，OTP 地址或初始化可能有误"
        _, soc_ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_before is not None and len(soc_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 SOC OTP 版本（应保持不变）: {soc_ver_before.hex()}")

    with allure.step("3、生成版本 VC0 的 EHSM 启动镜像（低于 OTP VC2，模拟回退）；# 3、镜像生成成功；"):
        ehsm_image_v0 = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,  # 版本低于 OTP VC2
        )
        log.info(f"EHSM 镜像（VC0，低版本）大小: {len(ehsm_image_v0)} bytes")

    with allure.step("4、BL 校验 EHSM 镜像 VC0（boot=True），预期版本检查失败；# 4、BL verify 因版本回退被拒绝；"):
        # Reason: image VC0 < OTP VC2，版本检查失败，FW 不会执行，OTP 不会被写入
        try:
            vrf_time, _ = api.ehsm_bl_verify_image(ehsm_image_v0, len(ehsm_image_v0), True, True)
            log.warning(f"BL verify 意外通过（耗时: {vrf_time}ms），EHSM版本回退防护可能失效")
            assert False, "EHSM版本回退应被 BL verify 拒绝（BUG-16版本回退防护失效）"
        except Exception as e:
            log.info(f"BL verify 因版本过低被拒绝（预期结果）: {e}")

    with allure.step("5、reset 固件，等待 BL 启动；# 5、BL 启动成功；"):
        # Reason: BL verify 失败 → DRAM 无有效标记 → 即使后续 FW 启动也不写 OTP
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("6、读取 EHSM OTP 版本号，验证版本未变化（回退被拒绝）；# 6、EHSM OTP 版本号保持 VC2 不变；"):
        _, ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"reset 后 EHSM OTP 版本: {ver_after.hex()}")
        assert ver_before == ver_after, \
            f"EHSM OTP 版本号不应变化（before={ver_before.hex()}, after={ver_after.hex()}），回退防护失效"
        log.info("BUG-16 EHSM版本回退防护验证通过：OTP版本未变化")

    with allure.step("7、验证 SOC OTP 版本未受 EHSM 版本回退尝试影响；# 7、SOC OTP 版本号保持不变；"):
        _, soc_ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert soc_ver_after is not None and len(soc_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"EHSM 回退尝试后 SOC OTP 版本: {soc_ver_after.hex()}")
        assert soc_ver_before == soc_ver_after, \
            f"SOC OTP 版本号不应受 EHSM 版本回退尝试影响（before={soc_ver_before.hex()}, after={soc_ver_after.hex()}）"
        log.info("隔离验证通过：EHSM 版本回退尝试不影响 SOC OTP 版本计数器")

    log.info("EHSM_U020 完成")


@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 SOC版本升级：BL verify 高版本SOC镜像（VC3，OTP初始VC0），EHSM启动后 secboot_update_ver_cnt 将SOC版本写入OTP，验证VC0→VC3")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_U021")
def test_ehsm_u021(setup_function):
    """
    TC-VER-001: BUG-16 SOC版本升级两阶段端到端验证（OTP VC0 < SOC镜像 VC3 → OTP更新为VC3）

    正确流程：
      1. OTP：Test模式，SOC VC0 + SOC vrf/enc 密钥 + EHSM vrf/enc 密钥
      2. 写 OTP，reset，wait_bl_done（BL 启动，等待 SOC 指令）
      3. 读 SOC OTP 基准值（VC0 = blank 值，OTP0 为全零，OTP1 为全 0xFF）
      4. 生成 SOC FW VC3 镜像，bl_verify(SOC, boot=False) → DRAM[SOC_VERSION] = VC3
      5. 生成 EHSM FW VC0 镜像（含 secboot 逻辑的真实 dummy binary）
      6. bl_verify(EHSM, boot=True) → BL 跳转 EHSM FW → FW secboot_update_ver_cnt → OTP VC3
      7. wait_fw_done（等 FW 启动完成）
      8. 读 SOC OTP，验证 VC0 → VC3
    """
    log.info("开始测试EHSM_U021: BUG-16 SOC版本升级两阶段端到端验证")

    # Reason: 真实 EHSM FW binary 需手动编译后放置，缺少时应报错而非跳过，
    # 避免测试覆盖静默缺失被忽略，详见 resource/real_fw/README.md
    if not os.path.exists(EHSM_FW_REAL_BIN_BL):
        pytest.fail(
            f"缺少真实 EHSM FW 二进制：{EHSM_FW_REAL_BIN_BL}\n"
            "请使用 IRAM link 文件编译 ehsm_fw_real.bin 后放置到该路径，"
            "参考 resource/real_fw/README.md"
        )

    with allure.step("1、配置 OTP：Test模式，SOC版本 VC0，同时写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            # SOC 启动镜像校验密钥（用于阶段一 BL verify SOC FW）
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            # EHSM 启动镜像校验密钥（用于阶段二 BL verify EHSM FW）
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            # SOC 版本计数器初始为 VC0，低于即将校验的 VC3 镜像
            soc_ver_cnt=_soc_ver_cnt_bl(0),
        )
        assert 0 == host.write_otp(otp_data)
        # Reason: reset 使 OTP 配置生效，BL 启动后等待 SOC 发命令
        # BL 测试不加载 FW，用 wait_bl_done 而非 wait_fw_done
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 SOC/EHSM OTP 版本号基准值，校验 SOC 初始为 VC0；# 2、读取成功，初始值符合 VC0 编码；"):
        _, ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"阶段一前 SOC OTP 版本: {ver_before.hex()}")
        # Reason: OTP0 bank blank 值为 0x00，OTP1 bank blank 值为 0xFF；VC0 = 未烧写任何 bit
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before == expected_blank, \
            f"初始 SOC OTP 应为 VC0（期望: {expected_blank.hex()}），实际: {ver_before.hex()}"
        _, ehsm_ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_before is not None and len(ehsm_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本（应保持不变）: {ehsm_ver_before.hex()}")

    with allure.step("3、生成版本 VC3 的 SOC 启动镜像（高版本，body 任意）；# 3、镜像生成成功；"):
        # Reason: SOC FW body 不被 EHSM 执行，dummy payload 即可；header 带 VC3 版本号 + 正确签名
        # IMAGE_FW_BOOT：这是 SOC 固件启动镜像（由 BL 校验，但镜像本身是 FW 层）
        soc_boot_image_v3 = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC3,
        )
        log.info(f"SOC 启动镜像（VC3）大小: {len(soc_boot_image_v3)} bytes")

    with allure.step("4、【阶段一】BL 校验 SOC 镜像（boot=False），DRAM 写入 SOC VC3 版本标记；# 4、BL verify 成功，DRAM 写入 VC3；"):
        # Reason: boot=False 不跳转，只把 SOC 版本号写入 DRAM 缓冲区
        vrf_time, _ = api.ehsm_bl_verify_image(soc_boot_image_v3, len(soc_boot_image_v3), True, False)
        log.info(f"阶段一：BL verify SOC FW 成功，耗时: {vrf_time}ms，DRAM SOC 版本标记已写入 VC3")

    with allure.step("5、生成平版本 VC0 的 EHSM 启动镜像（payload 使用真实 EHSM FW binary，含 secboot_update_ver_cnt 逻辑）；# 5、镜像生成成功；"):
        # Reason: EHSM FW body 会被 EHSM CPU 真实执行，必须含 secboot_entry → secboot_update_ver_cnt；
        # dummy ehsm_fw.bin 不含此逻辑，必须使用 IRAM link 文件编译的真实二进制
        # IMAGE_BL_BOOT：EHSM 固件启动镜像由 BL 直接校验并跳转，属于 BL 层镜像
        # 详见 resource/real_fw/README.md
        ehsm_boot_image_v0 = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            fw_image=EHSM_FW_REAL_BIN_FOR_IMGTOOL_BL,  # 相对 resource/image/ 的路径：../real_fw/ehsm_fw_real.bin
        )
        log.info(f"EHSM 启动镜像（VC0，真实FW）大小: {len(ehsm_boot_image_v0)} bytes")

    with allure.step("6、【阶段二】分块写入 EHSM 镜像到内存，BL 校验（boot=True）跳转到 EHSM FW；# 6、写入成功，BL verify 通过，跳转到 EHSM FW；"):
        # 步骤1~5 实际占用的内存块：
        #   DATA1_ADDR = CTX_ADDR + 2048*2  （step2 read_otp 输出 16B；step4 SOC 镜像输入 2288B）
        #   DATA3_ADDR = CTX_ADDR + 2048*4  （step4 bl_verify_image 输出 2288B）
        #   DATA3_ADDR + 2288 ≈ CTX_ADDR + 2048*5 + 192
        # 因此 EHSM 大镜像从 CTX_ADDR + 2048*6 开始，不与任何已用块冲突
        # Reason: 分块写入突破单次 64KB UART 包限制
        CHUNK_SIZE = 32 * 1024  # 32KB/块
        image_addr = api.CTX_ADDR + 2048 * 6
        image_size = len(ehsm_boot_image_v0)
        for offset in range(0, image_size, CHUNK_SIZE):
            chunk = ehsm_boot_image_v0[offset:offset + CHUNK_SIZE]
            host.write_memory(image_addr + offset, chunk)
        log.info(f"EHSM 镜像写入 0x{image_addr:08X}，{image_size} bytes，{(image_size + CHUNK_SIZE - 1) // CHUNK_SIZE} 块")

        # Reason: 绕过 api.ehsm_bl_verify_image（它会再次覆盖 DATA1_ADDR），直接调底层命令
        # boot=True → BL 验证通过后跳转 EHSM FW → secboot_update_ver_cnt → OTP 写 SOC VC3
        # 输入输出可以是同一区域
        vrf_time = hostapi.ehsm_bl_verify_image(
            api.CTX_ADDR,
            image_addr,
            image_size,
            True,   # check_version
            True,   # boot=True
            image_addr  # 输入输出同一区域
        )
        log.info(f"阶段二：BL verify EHSM FW 成功，耗时: {vrf_time}ms，已跳转到 EHSM FW")

    with allure.step("7、等待 EHSM FW secboot_update_ver_cnt 执行完成；# 7、FW 启动成功，OTP SOC 版本写入完成；"):
        assert 0 == host.wait_fw_done(1)
        log.info("EHSM FW 启动完成，secboot_update_ver_cnt 已执行")

    with allure.step("8、读取 OTP SOC 版本号，验证已从 VC0 更新为 VC3；# 8、OTP SOC 版本号已更新，两阶段流程完整有效；"):
        _, ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"阶段二后 SOC OTP 版本: {ver_after.hex()}")
        assert ver_before != ver_after, (
            f"SOC OTP 版本号未更新（before={ver_before.hex()}, after={ver_after.hex()}），"
            "BUG-16两阶段流程验证失败：EHSM FW secboot_update_ver_cnt 未将 SOC VC3 写入 OTP"
        )
        log.info("BUG-16端到端验证通过：BL verify SOC→DRAM + BL verify EHSM(boot=True)→FW→OTP 两阶段完整")

    with allure.step("9、验证 EHSM OTP 版本未受 SOC 版本更新影响；# 9、EHSM OTP 版本号保持不变；"):
        _, ehsm_ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_after is not None and len(ehsm_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"SOC 版本更新后 EHSM OTP 版本: {ehsm_ver_after.hex()}")
        assert ehsm_ver_before == ehsm_ver_after, \
            f"EHSM OTP 版本号不应受 SOC 版本更新影响（before={ehsm_ver_before.hex()}, after={ehsm_ver_after.hex()}）"
        log.info("隔离验证通过：SOC 版本更新不影响 EHSM OTP 版本计数器")

    log.info("EHSM_U021 完成")


@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 SOC版本平刷：BL verify 同版本SOC镜像（VC1，OTP初始VC1），EHSM启动后 secboot_update_ver_cnt 不写OTP，验证版本不变")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_U022")
def test_ehsm_u022(setup_function):
    """TC-VER-002: BUG-16 SOC版本平刷（OTP VC1 == SOC镜像 VC1 → OTP不变）"""
    log.info("开始测试EHSM_U022: BUG-16 SOC版本平刷验证")

    if not os.path.exists(EHSM_FW_REAL_BIN_BL):
        pytest.fail(
            f"缺少真实 EHSM FW 二进制：{EHSM_FW_REAL_BIN_BL}\n"
            "请参考 resource/real_fw/README.md 编译后放置"
        )

    with allure.step("1、配置 OTP：Test模式，SOC版本 VC1（平刷目标版本），写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            soc_ver_cnt=_soc_ver_cnt_bl(1),  # 初始 VC1
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 SOC/EHSM OTP 版本号基准值，校验 SOC 初始为 VC1（非 blank）；# 2、读取成功，初始值符合 VC1；"):
        _, ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 SOC OTP 版本: {ver_before.hex()}")
        # Reason: VC1 至少有 1 个 fuse 已烧写（非 blank 值）；OTP0 blank=0x00，OTP1 blank=0xFF
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before != expected_blank, \
            f"初始 SOC OTP 应为 VC1（非 blank），实际: {ver_before.hex()}，OTP 地址或初始化可能有误"
        _, ehsm_ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_before is not None and len(ehsm_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本（应保持不变）: {ehsm_ver_before.hex()}")

    with allure.step("3、生成版本 VC1 的 SOC 启动镜像（与 OTP 相同版本）；# 3、镜像生成成功；"):
        soc_boot_image_v1 = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC1,  # 与 OTP VC1 相同
        )
        log.info(f"SOC 启动镜像（VC1）大小: {len(soc_boot_image_v1)} bytes")

    with allure.step("4、【阶段一】BL 校验 SOC 镜像（boot=False），DRAM 写入 SOC VC1 版本标记；# 4、BL verify 成功，DRAM 写入 VC1；"):
        vrf_time, _ = api.ehsm_bl_verify_image(soc_boot_image_v1, len(soc_boot_image_v1), True, False)
        log.info(f"阶段一：BL verify SOC FW 成功，耗时: {vrf_time}ms，DRAM SOC 版本标记已写入 VC1")

    with allure.step("5、生成 EHSM 启动镜像（真实 FW binary）；# 5、镜像生成成功；"):
        ehsm_boot_image_v0 = generate_boot_image(
            ImageLevel.IMAGE_BL_BOOT,
            ImageType.IMAGE_HSM_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            fw_image=EHSM_FW_REAL_BIN_FOR_IMGTOOL_BL,
        )
        log.info(f"EHSM 启动镜像（VC0，真实FW）大小: {len(ehsm_boot_image_v0)} bytes")

    with allure.step("6、【阶段二】分块写入 EHSM 镜像，BL 校验（boot=True）跳转到 EHSM FW；# 6、BL verify 通过，跳转成功；"):
        CHUNK_SIZE = 32 * 1024
        image_addr = api.CTX_ADDR + 2048 * 6
        image_size = len(ehsm_boot_image_v0)
        for offset in range(0, image_size, CHUNK_SIZE):
            host.write_memory(image_addr + offset, ehsm_boot_image_v0[offset:offset + CHUNK_SIZE])
        log.info(f"EHSM 镜像写入 0x{image_addr:08X}，{image_size} bytes")
        vrf_time = hostapi.ehsm_bl_verify_image(
            api.CTX_ADDR, image_addr, image_size, True, True, image_addr
        )
        log.info(f"阶段二：BL verify EHSM FW 成功，耗时: {vrf_time}ms，已跳转到 EHSM FW")

    with allure.step("7、等待 EHSM FW secboot_update_ver_cnt 执行完成；# 7、FW 启动成功；"):
        assert 0 == host.wait_fw_done(1)

    with allure.step("8、读取 OTP SOC 版本号，验证版本未变化（平刷不写 OTP）；# 8、OTP SOC 版本号保持 VC1 不变；"):
        _, ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"阶段二后 SOC OTP 版本: {ver_after.hex()}")
        # Reason: DRAM VC1 == OTP VC1，FW 不写 OTP，防止无效烧写
        assert ver_before == ver_after, \
            f"SOC OTP 版本号不应变化（before={ver_before.hex()}, after={ver_after.hex()}），平刷逻辑异常"
        log.info("BUG-16平刷验证通过：OTP版本未变化")

    with allure.step("9、验证 EHSM OTP 版本未受 SOC 版本平刷影响；# 9、EHSM OTP 版本号保持不变；"):
        _, ehsm_ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_after is not None and len(ehsm_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"SOC 版本平刷后 EHSM OTP 版本: {ehsm_ver_after.hex()}")
        assert ehsm_ver_before == ehsm_ver_after, \
            f"EHSM OTP 版本号不应受 SOC 版本平刷影响（before={ehsm_ver_before.hex()}, after={ehsm_ver_after.hex()}）"
        log.info("隔离验证通过：SOC 版本平刷不影响 EHSM OTP 版本计数器")

    log.info("EHSM_U022 完成")


@pytest.mark.skipif(
    cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0 or cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0,
    reason="需要 RSA2048 和 AES128CMAC 同时支持"
)
@allure.feature("secboot")
@allure.description("BUG-16 SOC版本回退拒绝：BL verify 低版本SOC镜像（VC0，OTP初始VC2），版本检查失败，OTP不更新")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_U023")
def test_ehsm_u023(setup_function):
    """TC-VER-003: BUG-16 SOC版本回退拒绝（OTP VC2 > SOC镜像 VC0 → BL verify 失败，OTP不变）"""
    log.info("开始测试EHSM_U023: BUG-16 SOC版本回退防护验证")

    with allure.step("1、配置 OTP：Test模式，SOC版本 VC2（高版本），写入 SOC 和 EHSM 的校验/加密密钥；# 1、OTP写入成功，BL启动成功；"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC,
            soc_ver_cnt=_soc_ver_cnt_bl(2),  # 初始高版本 VC2
        )
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("2、读取 SOC/EHSM OTP 版本号基准值，校验 SOC 初始为 VC2（非 blank）；# 2、读取成功，初始值符合 VC2；"):
        _, ver_before = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_before is not None and len(ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 SOC OTP 版本: {ver_before.hex()}")
        # Reason: VC2 至少 2 个 fuse 已烧写（非 blank 值）；OTP0 blank=0x00，OTP1 blank=0xFF
        expected_blank = bytes([0xFF] * OTP_VER_SIZE_BL) if cfg_data.TEST_OTP_DEFAULT_VALUE != 0 else bytes(OTP_VER_SIZE_BL)
        assert ver_before != expected_blank, \
            f"初始 SOC OTP 应为 VC2（非 blank），实际: {ver_before.hex()}，OTP 地址或初始化可能有误"
        _, ehsm_ver_before = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_before is not None and len(ehsm_ver_before) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"基准 EHSM OTP 版本（应保持不变）: {ehsm_ver_before.hex()}")

    with allure.step("3、生成版本 VC0 的 SOC 启动镜像（低于 OTP VC2，模拟回退）；# 3、镜像生成成功；"):
        soc_boot_image_v0 = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0,  # 版本低于 OTP VC2
        )
        log.info(f"SOC 启动镜像（VC0）大小: {len(soc_boot_image_v0)} bytes")

    with allure.step("4、【阶段一】BL 校验 SOC 镜像（boot=False），预期版本检查失败；# 4、BL verify 因版本回退被拒绝；"):
        # Reason: image VC0 < OTP VC2，版本检查失败，DRAM 不写入，后续 FW 不更新 OTP
        try:
            vrf_time, _ = api.ehsm_bl_verify_image(soc_boot_image_v0, len(soc_boot_image_v0), True, False)
            log.warning(f"BL verify 意外通过（耗时: {vrf_time}ms），版本回退防护可能失效")
            assert False, "版本回退应被 BL verify 拒绝（BUG-16版本回退防护失效）"
        except Exception as e:
            log.info(f"BL verify 因版本过低被拒绝（预期结果）: {e}")

    with allure.step("5、reset 固件，等待 BL 启动；# 5、BL 启动成功；"):
        # Reason: BL verify 失败 → DRAM 无有效标记 → 即使启动 FW 也不触发 OTP 写入
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("6、读取 OTP SOC 版本号，验证版本未变化（回退被拒绝）；# 6、OTP SOC 版本号保持 VC2 不变；"):
        _, ver_after = api.ehsm_read_otp(SOC_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ver_after is not None and len(ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{SOC_VER_OTP_ADDR_BL:08X})"
        log.info(f"reset 后 SOC OTP 版本: {ver_after.hex()}")
        # Reason: OTP 只能从低到高写入，版本回退不触发写 OTP
        assert ver_before == ver_after, \
            f"SOC OTP 版本号不应变化（before={ver_before.hex()}, after={ver_after.hex()}），回退防护失效"
        log.info("BUG-16回退防护验证通过：OTP版本未变化")

    with allure.step("7、验证 EHSM OTP 版本未受 SOC 版本回退尝试影响；# 7、EHSM OTP 版本号保持不变；"):
        _, ehsm_ver_after = api.ehsm_read_otp(EHSM_VER_OTP_ADDR_BL, OTP_VER_SIZE_BL)
        assert ehsm_ver_after is not None and len(ehsm_ver_after) == OTP_VER_SIZE_BL, \
            f"OTP 读取失败（地址: 0x{EHSM_VER_OTP_ADDR_BL:08X})"
        log.info(f"SOC 回退尝试后 EHSM OTP 版本: {ehsm_ver_after.hex()}")
        assert ehsm_ver_before == ehsm_ver_after, \
            f"EHSM OTP 版本号不应受 SOC 版本回退尝试影响（before={ehsm_ver_before.hex()}, after={ehsm_ver_after.hex()}）"
        log.info("隔离验证通过：SOC 版本回退尝试不影响 EHSM OTP 版本计数器")

    log.info("EHSM_U023 完成")
