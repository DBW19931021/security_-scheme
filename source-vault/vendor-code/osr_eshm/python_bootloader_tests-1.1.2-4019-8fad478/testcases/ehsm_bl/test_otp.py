import logging as log
import struct
import time
import pytest
import allure
from platform_adapter.api.constants import EhsmLifecycle
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_reg, hostapi
from utils import otp
from utils.config import cfg_data
import platform_adapter.uart_lib.ehsm_bl_errno as bl_errno


gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

OTP_MIN_ADDRESS = cfg_data.TEST_BL_OTP_BASE_ADDR-1
OTP_MAX_ADDRESS = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE + 1

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc})

def _get_otp_bin_rclk(lc: str, rclk_en: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc, "random_clock_level": rclk_en})

def _get_otp_bin_rcl(lc: str, rclk: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc, "random_clock_level": rclk})

def check_lifecycle()->EhsmLifecycle:
    addr = hostapi.HSM_STATUS_IN
    t , ret = host.read_memory(addr, 4)
    reg_lifecycle = int.from_bytes(ret,"little")
    reg = reg_lifecycle & 0x00007F00
    match reg:
        case 256 :
            cur_lc = EhsmLifecycle.EHSM_LC_TEST
        case 512:
            cur_lc = EhsmLifecycle.EHSM_LC_DEVELOP
        case 1024:
            cur_lc = EhsmLifecycle.EHSM_LC_MANUFACTURE
        case 2048:
            cur_lc = EhsmLifecycle.EHSM_LC_USER
        case 4096:
            cur_lc = EhsmLifecycle.EHSM_LC_DEBUG
        case _:
            cur_lc = EhsmLifecycle.EHSM_LC_DESTORY
    return cur_lc

@allure.feature("otp")
@allure.description("在Dev模式下，测试bootloader OTP读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1050")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_1050(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，0x01递增到0x0F 数据； # 2、超出OTP边界的地址写入数据时返回了预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # dev模式下写入OTP区域边界-1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在小于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在小于OTP边界的地址写入时未抛出异常，不符合预期"

        # dev模式下写入OTP区域边界+1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在大于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在大于OTP边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，读取到tmp空间； # 3、超出OTP边界的地址读取数据时返回了预期的错误码"):
        # dev模式下读取OTP区域边界值-1
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在小于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在小于OTP边界的地址读取数据时未抛出异常，不符合预期"

        # dev模式下读取OTP区域边界值+1
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在大于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在大于OTP边界的地址读取数据时未抛出异常，不符合预期"


@allure.feature("otp")
@allure.description("在Manu模式下，测试bootloader OTP读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1052")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_1052(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，0x01递增到0x0F 数据； # 2、manu模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # manu模式下写入OTP区域边界-1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域最小边界-1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域最小边界-1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域最小边界-1写入数据时未抛出异常，不符合预期"

        # manu模式下写入OTP区域边界+1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域最大边界+1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域最大边界+1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域最大边界+1写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，读取到tmp空间； # 3、manu模式下OTP读取数据受到生命周期限制"):
        # manu模式下读取OTP区域边界值-1
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域最小边界-1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域最小边界-1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域最小边界-1读取数据时未抛出异常，不符合预期"

        # manu模式下读取OTP区域边界值+1
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域最大边界+1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域最大边界+1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域最大边界+1读取数据时未抛出异常，不符合预期"


@allure.feature("otp")
@allure.description("在User模式下，测试bootloader OTP读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1055")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_1055(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，0x01递增到0x0F 数据； # 2、user模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # user模式下写入OTP区域边界-1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"user模式下向OTP区域最小边界-1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"user模式下向OTP区域最小边界-1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"user模式下向OTP区域最小边界-1写入数据时未抛出异常，不符合预期"

        # user模式下写入OTP区域边界+1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"user模式下向OTP区域最大边界+1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"user模式下向OTP区域最大边界+1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"user模式下向OTP区域最大边界+1写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，读取到tmp空间； # 3、user模式下OTP读取数据受到生命周期限制"):
        # user模式下读取OTP区域边界值-1
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"user模式下向OTP区域最小边界-1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"user模式下向OTP区域最小边界-1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"user模式下向OTP区域最小边界-1读取数据时未抛出异常，不符合预期"

        # user模式下读取OTP区域边界值+1
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"user模式下向OTP区域最大边界+1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"user模式下向OTP区域最大边界+1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"user模式下向OTP区域最大边界+1读取数据时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("在Debug模式下，测试bootloader OTP读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1056")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_1056(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，0x01递增到0x0F 数据； # 2、debug模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # debug模式下写入OTP区域边界-1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"debug模式下向OTP区域最小边界-1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"debug模式下向OTP区域最小边界-1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"debug模式下向OTP区域最小边界-1写入数据时未抛出异常，不符合预期"

        # debug模式下写入OTP区域边界+1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"debug模式下向OTP区域最大边界+1写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"debug模式下向OTP区域最大边界+1写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"debug模式下向OTP区域最大边界+1写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，读取到tmp空间； # 3、debug模式下OTP读取数据受到生命周期限制"):
        # debug模式下读取OTP区域边界值-1
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"debug模式下向OTP区域最小边界-1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"debug模式下向OTP区域最小边界-1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"debug模式下向OTP区域最小边界-1读取数据时未抛出异常，不符合预期"

        # debug模式下读取OTP区域边界值+1
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"debug模式下向OTP区域最大边界+1读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"debug模式下向OTP区域最大边界+1读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"debug模式下向OTP区域最大边界+1读取数据时未抛出异常，不符合预期"


@allure.feature("otp")
@allure.description("在Test模式下，测试bootloader OTP读写功能")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-110")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_110(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes

@allure.feature("otp")
@allure.description("在Test模式下，测试bootloader OTP读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-111")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_111(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，0x01递增到0x0F 数据； # 2、超出OTP边界的地址写入数据时返回了预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # test模式下写入OTP区域边界值-1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"test模式下在小于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"test模式下在小于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"test模式下在小于OTP边界的地址写入时未抛出异常，不符合预期"

        # test模式下写入OTP区域边界值+1
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"test模式下在大于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"test模式下在大于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"test模式下在大于OTP边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS - 1/OTP_BASE_ADDRESS + CONFIG_BL_OTP_SIZE + 1, 长度16字节，读取到tmp空间； # 3、超出OTP边界的地址读取数据时返回了预期的错误码"):
        # test模式下读取OTP区域边界值-1
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"test模式下在小于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"test模式下在小于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"test模式下在小于OTP边界的地址读取数据时未抛出异常，不符合预期"

        # test模式下读取OTP区域边界值+1
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"test模式下在大于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"test模式下在大于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"test模式下在大于OTP边界的地址读取数据时未抛出异常，不符合预期"


@allure.feature("otp")
@allure.description("在Dev模式下，测试bootloader OTP读写功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-112")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_112(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t, data_temp = api.ehsm_read_otp( cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert int.from_bytes(data_temp,"little") == int.from_bytes(data_src_bytes,"little")

@allure.feature("otp")
@allure.description("在Manu模式下，测试bootloader OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-113")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_113(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； #2、manu模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Manu模式下写入数据
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、manu模式下OTP读取数据受到生命周期限制"):
        # Manu模式下读取数据
        try:
            t ,data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下读取数据时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))

@allure.feature("otp")
@allure.description("在User模式下，测试bootloader OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-114")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_114(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； #2、user模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # user模式下写入数据
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、user模式下OTP读取数据受到生命周期限制"):
        # user模式下读取数据
        try:
            t ,data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下读取数据时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

@allure.feature("otp")
@allure.description("在Debug模式下，测试bootloader OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-115")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_115(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入OTP区域OTP_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、debug模式下OTP写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Debug模式下写入数据
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域OTP_BASE_ADDRESS, 长度16字节，读取到tmp空间； #3、debug模式下OTP读取数据受到生命周期限制"):
        # Debug模式下读取数据
        try:
            t ,data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下读取数据时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)


@allure.feature("otp")
@allure.description("在Test模式下，测试bootloader OTP接口异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-116")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_READ_SUPPORT == 0,reason="OTP READ功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0,reason="OTP WRITE功能不支持")
def test_ehsm_116(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、OTP读写接口传入空的src_addr；其它参数合法； # 2、报非法参数，符合预期；"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 写入异常参数
        try:
            t = api.ehsm_write_otp(data_src_bytes, 0, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下写入异常数据参数异常，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下写入异常数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下写入异常数据时未抛出异常，不符合预期"
    with allure.step("3、OTP读写接口传入空的dst_addr；其它参数合法； # 3、报非法参数，符合预期；"):
        try:
            t ,data_temp = api.ehsm_read_otp(0, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下读取异常数据参数异常，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下读取异常数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下读取异常数据时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP读取接口size=0参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O001")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_READ_SUPPORT == 0, reason="OTP READ功能不支持")
def test_ehsm_o001(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP读取接口，地址合法但size=0； # 2、报非法参数，符合预期；"):
        try:
            t, data = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, 0)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP读取size=0时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP读取size=0时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP读取size=0时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP读取接口size过大导致越界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O002")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_READ_SUPPORT == 0, reason="OTP READ功能不支持")
def test_ehsm_o002(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP读取接口，从合法地址读取但size超过OTP区域大小； # 2、报非法参数，符合预期；"):
        oversized = cfg_data.TEST_BL_OTP_SIZE * 2
        try:
            t, data = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, oversized)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP读取size过大时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP读取size过大时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP读取size过大时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP读取接口地址+size越界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O003")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_READ_SUPPORT == 0, reason="OTP READ功能不支持")
def test_ehsm_o003(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP读取接口，地址合法但地址+size超出OTP区域； # 2、报非法参数，符合预期；"):
        # Reason: 从OTP区域的末尾前8字节开始读取16字节，会越界8字节
        near_end_addr = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE - 8
        try:
            t, data = api.ehsm_read_otp(near_end_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP读取地址+size越界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP读取地址+size越界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP读取地址+size越界时未抛出异常，不符合预期"


@allure.feature("otp")
@allure.description("测试OTP写入接口size=0参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O005")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0, reason="OTP WRITE功能不支持")
def test_ehsm_o005(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP写入接口，数据和地址合法但size=0； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        try:
            t = api.ehsm_write_otp(data_src, cfg_data.TEST_BL_OTP_BASE_ADDR, 0)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP写入size=0时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP写入size=0时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP写入size=0时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP写入接口size超大导致越界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O006")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0, reason="OTP WRITE功能不支持")
def test_ehsm_o006(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP写入接口，数据和地址合法但size超过OTP区域大小； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        oversized = cfg_data.TEST_BL_OTP_SIZE * 2
        try:
            t = api.ehsm_write_otp(data_src, cfg_data.TEST_BL_OTP_BASE_ADDR, oversized)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP写入size超大时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP写入size超大时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP写入size超大时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP写入接口地址+size越界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O007")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0, reason="OTP WRITE功能不支持")
def test_ehsm_o007(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP写入接口，地址合法但地址+size超出OTP区域； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        # Reason: 从OTP区域的末尾前8字节开始写入16字节，会越界8字节
        near_end_addr = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE - 8
        try:
            t = api.ehsm_write_otp(data_src, near_end_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP写入地址+size越界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP写入地址+size越界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP写入地址+size越界时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP写入接口地址小于最小边界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O008")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0, reason="OTP WRITE功能不支持")
def test_ehsm_o008(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP写入接口，地址小于OTP最小边界； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        invalid_addr = cfg_data.TEST_BL_OTP_BASE_ADDR - 4
        try:
            t = api.ehsm_write_otp(data_src, invalid_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP写入地址小于最小边界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP写入地址小于最小边界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP写入地址小于最小边界时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("测试OTP写入接口地址大于最大边界")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O009")
@pytest.mark.skipif(cfg_data.TEST_BL_OTP_WRITE_SUPPORT == 0, reason="OTP WRITE功能不支持")
def test_ehsm_o009(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用OTP写入接口，地址大于OTP最大边界； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        invalid_addr = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE + 4
        try:
            t = api.ehsm_write_otp(data_src, invalid_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"OTP写入地址大于最大边界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"OTP写入地址大于最大边界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"OTP写入地址大于最大边界时未抛出异常，不符合预期"
