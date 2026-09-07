import time
import pytest
import allure

import logging as log
from platform_adapter.api.constants import EhsmCtrlField
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from utils import otp
from platform_adapter.uart_lib import ehsm_fw_errno

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin( {"lifecycle": lc})

def _get_otp_bin_with_trng_rst(lc: str, trng_rst_times: int) -> bytes:
    """
    生成包含trng_rst_times配置的OTP数据
    用于测试修改OTP基地址+0x018的第36-39位
    """
    config = {
        "lifecycle": lc,
        "trng_rst_times": trng_rst_times
    }
    return otp.otp_to_bin(config)

KEY_ALG_SEL_AES_128 = (0b01 << 12)

EHSM_CODE_UPGRADE_ALG_SM4_CMAC = (0b101 << 36)
EHSM_CODE_VERIFY_ALG_SM4_CMAC = (0b11 << 32)
EHSM_BOOT_TYPE_SEQUENTIAL = (0b01 << 10)

SOC_BOOT_TYPE_SEQUENTIAL = (0b1 << 10)
SOC_UPGRADE_ALG_AES_128_CMAC = (0b100 << 4)
SOC_BOOT_ALG_AES_128_CMAC = (0b10 << 0)

@allure.feature("ctrl_field")
@allure.description("在Test模式下，修改HW control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-831")
def test_ehsm_831(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改HW control field； # 3、发送成功"):
        value = KEY_ALG_SEL_AES_128
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Test模式下，修改EHSM control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-832")
def test_ehsm_832(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改EHSM control field； # 3、发送成功"):
        value = EHSM_BOOT_TYPE_SEQUENTIAL | EHSM_CODE_UPGRADE_ALG_SM4_CMAC | EHSM_CODE_VERIFY_ALG_SM4_CMAC
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_EHSM,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)


@allure.feature("ctrl_field")
@allure.description("在Test模式下，修改SOC control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-833")
def test_ehsm_833(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改SOC control field； # 3、发送成功"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Dev模式下，修改HW control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-834")
def test_ehsm_834(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        # 只配置生命周期，不修改其他字段
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改HW control field； # 3、发送成功"):
        ret_code, otp_bytes = host.read_memory(hostapi.OTP_BASE,1028)
        otp_data = bytearray(otp_bytes)

        # 读取当前OTP配置并修改第36-39位（trng_rst_times字段）
        # 获取OTP基地址+0x018偏移处的8字节数据
        otp_offset_0x18 = int.from_bytes(otp_data[0x18:0x20], "little")
        # 清除第36-39位，然后设置新值（0b1111 = 15）
        otp_offset_0x18 &= ~(0xF << 36)  # 清除第36-39位
        otp_offset_0x18 |= (0b1111 << 36)  # 设置第36-39位为0b1111
        otp_data[0x18:0x20] = otp_offset_0x18.to_bytes(8, "little")
        # 将修改后的完整8字节数据传递给函数
        value = bytes(otp_data)
        log.info(value)
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Dev模式下，修改EHSM control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-835")
def test_ehsm_835(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改EHSM control field； # 3、发送成功"):
        value = EHSM_BOOT_TYPE_SEQUENTIAL | EHSM_CODE_UPGRADE_ALG_SM4_CMAC | EHSM_CODE_VERIFY_ALG_SM4_CMAC
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_EHSM,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Dev模式下，修改SOC control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-836")
def test_ehsm_836(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改SOC control field； # 3、发送成功"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Manu模式下，修改HW control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-837")
def test_ehsm_837(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改HW control field； # 3、manu模式下向OTP区域写入数据受到生命周期限制"):
        value = KEY_ALG_SEL_AES_128
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Manu模式下，修改EHSM control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-838")
def test_ehsm_838(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改EHSM control field； # 3、manu模式下向OTP区域写入数据受到生命周期限制"):
        value = EHSM_BOOT_TYPE_SEQUENTIAL | EHSM_CODE_UPGRADE_ALG_SM4_CMAC | EHSM_CODE_VERIFY_ALG_SM4_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_EHSM,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Manu模式下，修改SOC control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-839")
def test_ehsm_839(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改SOC control field； # 3、发送成功"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC,value)
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在User模式下，修改HW control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-840")
def test_ehsm_840(setup_module):
    with allure.step("1、配置OTP 生命周期User模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改HW control field； # 3、user模式下向OTP区域写入数据受到生命周期限制"):
        value = KEY_ALG_SEL_AES_128
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在User模式下，修改EHSM control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-841")
def test_ehsm_841(setup_module):
    with allure.step("1、配置OTP 生命周期User模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改EHSM control field； # 3、user模式下向OTP区域写入数据受到生命周期限制"):
        value = EHSM_BOOT_TYPE_SEQUENTIAL | EHSM_CODE_UPGRADE_ALG_SM4_CMAC | EHSM_CODE_VERIFY_ALG_SM4_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_EHSM,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在User模式下，修改SOC control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-842")
def test_ehsm_842(setup_module):
    with allure.step("1、配置OTP 生命周期User模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改SOC control field； # 3、user模式下向OTP区域写入数据受到生命周期限制"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Debug模式下，修改HW control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-843")
def test_ehsm_843(setup_module):
    with allure.step("1、配置OTP 生命周期Debug模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改HW control field； # 3、debug模式下向OTP区域写入数据受到生命周期限制"):
        value = KEY_ALG_SEL_AES_128
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Debug模式下，修改EHSM control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-844")
def test_ehsm_844(setup_module):
    with allure.step("1、配置OTP 生命周期Debug模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改EHSM control field； # 3、debug模式下向OTP区域写入数据受到生命周期限制"):
        value = EHSM_BOOT_TYPE_SEQUENTIAL | EHSM_CODE_UPGRADE_ALG_SM4_CMAC | EHSM_CODE_VERIFY_ALG_SM4_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_EHSM,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Debug模式下，修改SOC control field")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-845")
def test_ehsm_845(setup_module):
    with allure.step("1、配置OTP 生命周期Debug模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改SOC control field； # 3、debug模式下向OTP区域写入数据受到生命周期限制"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("ctrl_field")
@allure.description("在Test模式下，对修改control field 进行异常参数处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-846")
def test_ehsm_846(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改非法 control field 类型； # 3、TEST模式下传入异常参数返回预期的错误码"):
        value = SOC_BOOT_TYPE_SEQUENTIAL | SOC_UPGRADE_ALG_AES_128_CMAC | SOC_BOOT_ALG_AES_128_CMAC
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(0x0F,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下传入异常参数修改control field 时参数异常，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下传入异常参数修改control field 时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下传入异常参数修改control field 时未抛出异常，不符合预期"

    with allure.step("4、发送 change_control命令，修改value 为 NULL； # 5、Test模式下传入空地址时返回预期的错误码"):
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_SOC, None)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"Test模式下传入空地址修改control field 时地址非法，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下传入空地址修改control field 时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"Test模式下传入空地址修改control field 时未抛出异常，不符合预期"



@allure.feature("ctrl_field")
@allure.description("在Test模式下，测试control_field异常参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-C001")
def test_ehsm_c001(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_control命令，修改value地址为none； # 3、发送失败"):
        value = None
        try:
            t = api.ehsm_change_control_field(EhsmCtrlField.EHSM_CTRL_FIELD_HW,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入空地址时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入空地址时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("5、发送 change_control命令，修改type为保留值； # 3、发送失败"):
        value = KEY_ALG_SEL_AES_128
        value = value.to_bytes(8,"little")
        try:
            t = api.ehsm_change_control_field(3,value)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"传入保留控制字段时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入保留控制字段时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"传入保留控制字段时未抛出异常，不符合预期"
    with allure.step("6、重启eHSM，并检查启动状态和control field； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)