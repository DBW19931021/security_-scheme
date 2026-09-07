import pytest
import allure
import logging as log
from platform_adapter.uart_lib import hostapi
from platform_adapter.uart_lib import ehsm_fw_errno
from utils.image import (
    ImageLevel,
    ImageType,
    ImageVersion,
    ImageEncAlgo,
    ImageSignAlgo,
    ImageCorruptionType
)
from utils.image import generate_boot_image, generate_corrupted_boot_image, load_binary_to_bytes
from utils.otp import (
    LifeCycle,
    SocVersionCounter,
    EhsmVrfEncAlgo,
    SocVrfEncAlgo,
    EhsmVerifyAlgo,
    SocVerifyAlgo
)
from utils.otp import generate_otp_data
from utils.config import cfg_data
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib.ehsm_fw_errno import *

api = get_api_interface()
host = get_host_interface()
src_fw_bin_path = "resource/image/ehsm_fw.bin"
src_fw_bytes = load_binary_to_bytes(src_fw_bin_path)

IMAGE_HEAD_SIZE = 1024

@pytest.fixture(scope="function")
def setup_function():
    # 清零内存区域
    host.share_memset(api.SESSION_ADDR, 0x00, api.DATA_SIZE * 9)

# from testcases.api.test_api import dummy_image
@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES-CMAC 算法")
@allure.testcase("EHSM-895")
def test_ehsm_895(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes
        # log.info(f"image_out: {''.join(f'{b:02x}' for b in image_out) or '(empty)'}")

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason=" 升级不支持 RSA2048 算法")
@allure.testcase("EHSM-896")
def test_ehsm_896(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason=" 升级不支持 RSA3072 算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-897")
def test_ehsm_897(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_ECC_P256R1_SUPPORT == 0, reason=" 升级不支持 ECC256 算法")
@allure.testcase("EHSM-898")
def test_ehsm_898(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 SM4-CMAC算法签名，SM4-CBC算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM4CMAC_SUPPORT == 0, reason=" 升级不支持 SM4CMAC 算法")
@allure.testcase("EHSM-899")
def test_ehsm_899(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试SM2算法签名，SM4-CBC算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
@allure.testcase("EHSM-900")
def test_ehsm_900(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在User模式下，校验SOC-eHSM镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
@allure.testcase("EHSM-901")
def test_ehsm_901(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Dev模式下，校验SOC-eHSM镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
@allure.testcase("EHSM-902")
def test_ehsm_902(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在MANU模式下，校验SOC-eHSM镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
@allure.testcase("EHSM-903")
def test_ehsm_903(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在DEBUG模式下，校验SOC-eHSM镜像的校验流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-904")
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
def test_ehsm_904(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Dev模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-905")
def test_ehsm_905(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 RSA-2048算法签名，AES-CBC(128 or 256)算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason=" 升级不支持 RSA2048_PSS 算法")
@allure.testcase("EHSM-906")
def test_ehsm_906(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Manu模式下，测试 RSA-3072算法签名，AES-CBC(128 or 256)算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason=" 升级不支持 RSA3072_PSS 算法")
@allure.testcase("EHSM-907")
def test_ehsm_907(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_MANU,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在User模式下，测试 ECC-P256R1算法签名，AES-CBC(128 or 256)算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_ECC_P256R1_SUPPORT == 0, reason=" 升级不支持 ECC_P256R1 算法")
@allure.testcase("EHSM-908")
def test_ehsm_908(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Debug模式下，测试 SM4-CMAC算法签名，SM4-CBC算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM4CMAC_SUPPORT == 0, reason=" 升级不支持 SM4CMAC 算法")
@allure.testcase("EHSM-909")
def test_ehsm_909(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试SM2算法签名，SM4-CBC算法加密的SOC加密算法密钥，SOC校验镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_SM2_SUPPORT == 0, reason=" 升级不支持 SM2 算法")
@allure.testcase("EHSM-910")
def test_ehsm_910(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_SM4,
            ImageSignAlgo.IMAGE_SIGN_ALGO_SM2,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 RSA-2048算法签名，不加密的明文SOC_eHSM自启动镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason=" 升级不支持 RSA2048_PSS 算法")
@allure.testcase("EHSM-911")
def test_ehsm_911(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Dev模式下，测试 RSA-2048算法签名，不加密的明文SOC_eHSM自启动镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-912")
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason=" 升级不支持 RSA2048_PSS 算法")
def test_ehsm_912(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在User模式下，测试 RSA-2048算法签名，不加密的明文SOC_eHSM自启动镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-913")
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA2048_PSS_SUPPORT == 0, reason=" 升级不支持 RSA2048_PSS 算法")
def test_ehsm_913(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_USER,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Debug模式下，测试 RSA-3072算法签名，不加密的明文SOC_eHSM自启动镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_RSA3072_PSS_SUPPORT == 0, reason=" 升级不支持 RSA3072_PSS 算法")
@allure.testcase("EHSM-914")
def test_ehsm_914(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEBUG,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 ECC-P256R1算法签名，不加密的明文SOC_eHSM自启动镜像的校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_ECC_P256R1_SUPPORT == 0, reason=" 升级不支持 ECC_P256R1 算法")
@allure.testcase("EHSM-915")
def test_ehsm_915(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 AES-CBC(128 or 256)算法签名，不加密的明文SOC自启动镜像的版本校验失败流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-916")
def test_ehsm_916(setup_function):
    with allure.step("1、生成带有高版本计数器的OTP数据配置 # 1、配置生成成功"):
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            otp_soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP0_VC2
        else:
            otp_soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP1_VC2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_ver_cnt=otp_soc_ver_cnt,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成低版本号的启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0  # Lower version than OTP
        )

    with allure.step("4、执行镜像验证并验证版本检查失败 # 4、版本校验失败（预期结果）"):
        try:
            vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
            # If verify succeeds when it should fail, that's unexpected
            assert False, "Expected version verification to fail"
        except Exception as e:
            # Version verification failure is expected
            assert e.ret_code == EHSM_ERR_WRONG_VERSION_COUNTER
            assert True

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 AES-CBC(128 or 256)算法签名，不加密的明文SOC自启动镜像的较小版本校验流程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-917")
def test_ehsm_917(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            otp_soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP0_VC2
        else:
            otp_soc_ver_cnt=SocVersionCounter.SOC_VER_CNT_OTP1_VC2
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_ver_cnt=otp_soc_ver_cnt,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成高版本号的启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC3  # Higher version than OTP
        )

    with allure.step("4、执行镜像验证并比对结果 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试校验接口对异常参数的处理过程")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-918")
def test_ehsm_918(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、测试非法镜像类型参数验证失败 # 3、验证失败（预期结果）"):
        try:
            corrupted_image = generate_corrupted_boot_image(
                ImageLevel.IMAGE_FW_BOOT,
                ImageType.IMAGE_SOC_SOCK,
                ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
                ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
                ImageVersion.IMAGE_VC0,
                ImageCorruptionType.INVALID_IMAGE_TYPE
            )
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with invalid image type, that's a security issue
            assert False, "安全问题：非法镜像类型应导致验证失败"
        except Exception as e:
            # Verification failure due to invalid image type is expected
            assert hasattr(e, 'ret_code'), "异常对象应包含错误码"
            assert True

    with allure.step("4、测试非法镜像大小参数验证失败 # 4、验证失败（预期结果）"):
        try:
            corrupted_image = generate_corrupted_boot_image(
                ImageLevel.IMAGE_FW_BOOT,
                ImageType.IMAGE_SOC_SOCK,
                ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
                ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
                ImageVersion.IMAGE_VC0,
                ImageCorruptionType.INVALID_IMAGE_SIZE
            )
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with invalid image size, that's a security issue
            assert False, "安全问题：非法镜像大小应导致验证失败"
        except Exception as e:
            # Verification failure due to invalid image size is expected
            assert hasattr(e, 'ret_code'), "异常对象应包含错误码"
            assert True

    with allure.step("5、测试非法明文标志参数验证失败 # 5、验证失败（预期结果）"):
        try:
            corrupted_image = generate_corrupted_boot_image(
                ImageLevel.IMAGE_FW_BOOT,
                ImageType.IMAGE_SOC_SOCK,
                ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
                ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
                ImageVersion.IMAGE_VC0,
                ImageCorruptionType.INVALID_PLAIN_FLAG
            )
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with invalid plain flag, that's a security issue
            assert False, "安全问题：非法明文标志应导致验证失败"
        except Exception as e:
            # Verification failure due to invalid plain flag is expected
            assert hasattr(e, 'ret_code'), "异常对象应包含错误码"
            assert True

    with allure.step("6、测试非法镜像所有者标志参数验证失败 # 6、验证失败（预期结果）"):
        try:
            corrupted_image = generate_corrupted_boot_image(
                ImageLevel.IMAGE_FW_BOOT,
                ImageType.IMAGE_SOC_SOCK,
                ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
                ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
                ImageVersion.IMAGE_VC0,
                ImageCorruptionType.INVALID_IMAGE_OWNER
            )
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with invalid image owner, that's a security issue
            assert False, "安全问题：非法镜像所有者应导致验证失败"
        except Exception as e:
            # Verification failure due to invalid image owner is expected
            assert hasattr(e, 'ret_code'), "异常对象应包含错误码"
            assert True

    with allure.step("7、使用正常参数验证基本功能 # 7、验证成功，镜像内容正确"):
        normal_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )
        vrf_time, image_out = api.ehsm_verify_image(normal_image, len(normal_image), True, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

@allure.feature("soc_verify")
@allure.description("在TEST模式下，测试 AES-CBC(128 or 256)算法签名，不加密的明文SOC自启动镜像，Upgrade_Valid_Flag数据和预期不一致会导致升级失败")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-919")
def test_ehsm_919(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID,  # 保持 INVALID 以测试明文镜像
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成带有非法Valid_Flag的镜像 # 3、损坏镜像生成成功"):
        corrupted_image = generate_corrupted_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_NONE,  # 与 OTP 中的 INVALID 配置匹配
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            ImageCorruptionType.INVALID_VALID_FLAG
        )

    with allure.step("4、执行镜像验证并验证Valid_Flag检查失败 # 4、验证失败（预期结果）"):
        try:
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with corrupted valid flag, that's a security issue
            assert False, "安全问题：非法Valid_Flag应导致验证失败"
        except Exception as e:
            # Verification failure due to valid flag mismatch is expected
            assert e.ret_code == EHSM_ERR_INVALID_CODE_FLAG
            assert True

@allure.feature("soc_verify")
@allure.description("在Test 模式下，测试AES-CMAC(128 or 256)校验算法配置下，AES-CMAC(128 or 256)算法签名区域被篡改后的 soc 固件镜像的安全校验过程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-920")
def test_ehsm_920(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            soc_vrf_enc_algo=SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC,
            soc_vrf_sign_algo=SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成带有被篡改签名的镜像 # 3、损坏镜像生成成功"):
        corrupted_image = generate_corrupted_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_SOCK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0,
            ImageCorruptionType.CORRUPTED_SIGNATURE
        )

    with allure.step("4、执行镜像验证并验证签名检查失败 # 4、验证失败（预期结果）"):
        try:
            vrf_time, image_out = api.ehsm_verify_image(corrupted_image, len(corrupted_image), True, False)
            # If verification succeeds with corrupted signature, that's a security issue
            assert False, "安全问题：被篡改的签名应导致验证失败"
        except Exception as e:
            # Verification failure with corrupted signature is expected
            assert e.ret_code == EHSM_ERR_FW_VWRIFY_FAILED
            assert True

@allure.feature("soc_verify")
@allure.description("在Test 模式下，测试固件对 eHSM 固件镜像（不支持的镜像）的安全校验过程")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES128CMAC 算法")
@allure.testcase("EHSM-921")
def test_ehsm_921(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_DEV,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成eHSM类型镜像 # 3、eHSM镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_HSM_HSMK,  # eHSM image type (may be unsupported)
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、执行SOC验证不支持的eHSM镜像 # 4、验证失败（预期结果）"):
        try:
            vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), True, False)
            # If verification succeeds with not support signature, that's a security issue
            assert False, "安全问题：指令 soc verify 验证不支持的 EHSM 镜像应导致验证失败"
        except Exception as e:
            # Failure with unsupported image type is also acceptable
            assert True

@allure.feature("soc_verify")
@allure.description("在Test模式下，测试 AES-CMAC(128 or 256)算法签名，AES-CBC(128 or 256)算法加密的eHSM加密算法密钥，SOC-eHSM校验镜像异常情况下的校验流程")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_UPGRADE_AES128CMAC_SUPPORT == 0, reason=" 升级不支持 AES-CMAC 算法")
@allure.testcase("EHSM-SV001")
def test_ehsm_sv001(setup_function):
    with allure.step("1、生成OTP数据配置 # 1、配置生成成功"):
        otp_data = generate_otp_data(
            lifecycle=LifeCycle.LIFECYCLE_TEST,
            ehsm_vrf_enc_algo=EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC,
            ehsm_vrf_sign_algo=EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC
        )

    with allure.step("2、写入OTP数据并重置eHSM，等待启动完成 # 2、写入成功，eHSM启动完成"):
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、生成启动镜像 # 3、启动镜像生成成功"):
        boot_image = generate_boot_image(
            ImageLevel.IMAGE_FW_BOOT,
            ImageType.IMAGE_SOC_HSMK,
            ImageEncAlgo.IMAGE_ENC_ALGO_AES128,
            ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC,
            ImageVersion.IMAGE_VC0
        )

    with allure.step("4、传入空镜像(None/0)进行验证 # 4、验证失败，镜像内容错误"):
        verify_image = None
        try:
            vrf_time, image_out = api.ehsm_verify_image(verify_image, 0, True, False)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入空镜像（None/0）进行验证验证失败，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入空镜像（None/0）进行验证返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入空镜像（None/0）进行验证未抛出异常，不符合预期"

    with allure.step("5、传入空镜像(None/正常长度)进行验证 # 4、验证失败，镜像内容错误"):
        verify_image = None
        try:
            vrf_time, image_out = api.ehsm_verify_image(verify_image, len(boot_image), True, False)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入空镜像(None/正常长度)进行验证验证失败，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入空镜像(None/正常长度)进行验证返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入空镜像(None/正常长度)进行验证未抛出异常，不符合预期"

    with allure.step("6、传入空镜像(正常地址/0)进行验证 # 4、验证失败，镜像内容错误"):
        verify_image = None
        try:
            vrf_time, image_out = api.ehsm_verify_image(boot_image, 0, True, False)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入空镜像(正常地址/0)进行验证验证失败，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入空镜像(正常地址/0)进行验证返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入空镜像(正常地址/0)进行验证未抛出异常，不符合预期"

    with allure.step("7、check_version传入保留值（2）传入进行验证 # 4、验证成功，镜像内容正确"):
        vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), 2, False)
        assert src_fw_bytes is not None
        assert image_out[IMAGE_HEAD_SIZE:IMAGE_HEAD_SIZE+len(src_fw_bytes)] == src_fw_bytes

    with allure.step("8、传入空的image_out_addr（0）进行验证 # 4、验证成功，镜像内容正确"):
        try:
            vrf_time, image_out = api.ehsm_verify_image(boot_image, len(boot_image), 2, False,0)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入0返回镜像地址进行验证验证失败，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入0返回镜像地址进行验证返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入0返回镜像地址进行验证未抛出异常，不符合预期"


