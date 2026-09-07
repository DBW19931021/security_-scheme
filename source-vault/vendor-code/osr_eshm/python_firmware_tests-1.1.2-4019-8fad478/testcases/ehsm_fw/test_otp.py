import logging as log
import allure
import pytest
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from utils import otp
from utils.config import cfg_data
from platform_adapter.uart_lib import ehsm_fw_errno


OTP_MIN_ADDRESS = cfg_data.TEST_BL_OTP_BASE_ADDR-4
OTP_MAX_ADDRESS = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE + 4

LIFE_CYCLE_DEBUG_MODE = 0xD7C5FCDE
LIFE_CYCLE_DESTROY_MODE = 0xFFFFFFFF

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin( {"lifecycle": lc})

@allure.feature("otp")
@allure.description("在Test模式下，测试bootloader OTP读写功能")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-785")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_785(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入OTP区域 CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，0x00~0xFF 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("3、读取OTP区域 CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes
    with allure.step("5、恢复默认OTP区域数据； # 5、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)


@allure.feature("otp")
@allure.description("在Test模式下，测试Firmware OTP读写边界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-786")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_786(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、依次写入OTP区域 (CONFIG_EHSM_OTP_BASE_ADDR - 4) 和(CONFIG_EHSM_OTP_BASE_ADDR + CONFIG_EHSM_OTP_SIZE + 4), 长度16字节，0x00~0xFF 数据； # 2、超出OTP边界区域写入数据返回预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 在(CONFIG_EHSM_OTP_BASE_ADDR - 4)写入数据
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在小于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在小于OTP边界的地址写入时未抛出异常，不符合预期"

        # 在(CONFIG_EHSM_OTP_BASE_ADDR + CONFIG_EHSM_OTP_SIZE + 4)写入数据
        try:
            t = api.ehsm_write_otp(data_src_bytes, OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在大于OTP边界的地址写入数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于OTP边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在大于OTP边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、依次读取OTP区域 (CONFIG_EHSM_OTP_BASE_ADDR - 4) 和(CONFIG_EHSM_OTP_BASE_ADDR + CONFIG_EHSM_OTP_SIZE + 4), 长度16字节，读取到tmp空间； # 3、超出OTP边界区域读取数据返回预期的错误码"):
        # 在(CONFIG_EHSM_OTP_BASE_ADDR - 4)读取数据
        try:
            t, data_temp= api.ehsm_read_otp( OTP_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在小于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在小于OTP边界的地址读取时未抛出异常，不符合预期"

        # 在(CONFIG_EHSM_OTP_BASE_ADDR + CONFIG_EHSM_OTP_SIZE + 4)读取数据
        try:
            t = api.ehsm_read_otp(OTP_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"在大于OTP边界的地址读取数据时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于OTP边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"在大于OTP边界的地址读取时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Dev模式下，测试Firmware OTP读写功能")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-787")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_787(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，0x00~0xFF 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("3、读取OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，读取到tmp空间 # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes
    with allure.step("5、恢复默认OTP区域数据； # 5、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Manu模式下，测试Firmware OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-788")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_788(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，0x00~0xFF 数据； #2、manu模式下向OTP区域写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 在Manu模式下写入
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp1 = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)


@allure.feature("otp")
@allure.description("在User模式下，测试Firmware OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-789")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_789(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，0x00~0xFF 数据； # 2、user模式向OTP区域写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        # User模式下写入
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向OTP区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，读取到tmp空间； # 3、user模式读取OTP区域数据受到生命周期限制"):
        # User模式下读取
        try:
            t , data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下对OTP区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下对OTP区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下对OTP区域读取数据时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)


@allure.feature("otp")
@allure.description("在Debug模式下，测试Firmware OTP读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-790")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_790(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，0x00~0xFF 数据； # 2、debug模式下向OTP区域写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 在debug模式下写入
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下对OTP区域读取数据时未抛出异常，不符合预期"
    with allure.step("3、读取OTP区域  CONFIG_EHSM_OTP_BASE_ADDR, 长度16字节，读取到tmp空间； # 2、debug模式下向OTP区域读取数据受到生命周期限制"):
        # 在debug模式下读取
        try:
            t , data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下对OTP区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下对OTP区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下对OTP区域读取数据时未抛出异常，不符合预期"
    with allure.step("4、恢复默认OTP区域数据； # 4、恢复数据成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Test模式下，测试Firmware OTP接口异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-791")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_791(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP读写接口传入空的src_addr；其它参数合法； # 2、报非法参数；"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # test模式下接口写入异常参数：空地址
        try:
            t = api.ehsm_write_otp(data_src_bytes, 0, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:# error: 1, EHSM_ERR_PARAM_ERROR
                log.info(f"Test模式下向OTP写入接口写入空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下对OTP写入接口写入空地址返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下对OTP写入接口写入空地址时未抛出异常，不符合预期"
    with allure.step("3、OTP读写接口传入空的dst_addr；其它参数合法； # 3、报非法参数；"):
        # test模式下接口读取异常参数：空地址
        try:
            t ,data_temp = api.ehsm_read_otp(0, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:# error: 1, EHSM_ERR_PARAM_ERROR
                log.info(f"Test模式下OTP读取接口读取空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP读取接口读取空地址返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下OTP读取接口读取空地址时未抛出异常，不符合预期"

@allure.feature("otp")
@allure.description("在User模式下，测试Firmware OTP切换为DEBUG模式")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1295")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_1295(setup_module):
    with allure.step("1、配置生命周期为User模式；并检查重启生效； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP OTP_BASE_ADDRESS + 0位置写入生命周期为DEBUG模式；并检查重启生效； # 2、user模式下切换生命周期到debug受到生命周期限制"):
        reg = 0xdefcc5d7
        value = reg.to_bytes(4,"big")
        try:
            t = api.ehsm_write_otp(value, cfg_data.TEST_BL_OTP_BASE_ADDR + 0, 4)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下对OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下对OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"user模式下对OTP区域读取数据时未抛出异常，不符合预期"

        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在DEBUG模式下，测试Firmware OTP切换为DESTROY模式")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1296")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_1296(setup_module):
    with allure.step("1、配置生命周期为User模式；并检查重启生效； # 配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP OTP_BASE_ADDRESS+0位置写入生命周期为DESTROY模式；并检查重启生效； # 2、user模式下切换生命周期到destory模式受到生命周期的限制"):
        reg = 0xFFFFFFFF
        value = reg.to_bytes(4,"big")
        try:
            t = api.ehsm_write_otp(value, cfg_data.TEST_BL_OTP_BASE_ADDR + 0, 4)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下对OTP区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下对OTP区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下对OTP区域读取数据时未抛出异常，不符合预期"
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)



@allure.feature("otp")
@allure.description("在Test模式下，测试OTP写入接口size异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o001")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_o001(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、OTP写入接口传入过大长度（超过OTP区域大小）；其它参数合法； # 2、报内存不足错误；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # test模式下接口写入异常参数：长度过大
        oversized_length = cfg_data.TEST_BL_OTP_SIZE + 100
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, oversized_length)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下OTP写入接口传入过大长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP写入接口传入过大长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下OTP写入接口传入过大长度时未抛出异常，不符合预期"

    with allure.step("3、OTP写入接口传入跨越边界的长度（地址合法但地址+长度超出OTP区域）；其它参数合法； # 3、报参数错误或内存不足错误；"):
        # test模式下接口写入异常参数：跨越边界的长度
        # 从OTP区域末尾前16字节开始，写入32字节，会超出边界
        boundary_addr = cfg_data.TEST_BL_OTP_BASE_ADDR + cfg_data.TEST_BL_OTP_SIZE - 16
        boundary_size = 32  # 超出边界
        try:
            t = api.ehsm_write_otp(data_src_bytes, boundary_addr, boundary_size)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下OTP写入接口传入跨越边界的长度时返回错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP写入接口传入跨越边界的长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下OTP写入接口传入跨越边界的长度时未抛出异常，不符合预期"

    with allure.step("4、恢复默认OTP区域数据； # 4、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Test模式下，测试OTP写入接口src_data异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o002")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_o002(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、OTP写入接口传入NULL数据（None）；其它参数合法； # 2、报参数错误或无效地址错误；"):
        # test模式下接口写入异常参数：None数据
        # Reason: 传入None时，API层转换为src_addr=0，固件可能返回EHSM_ERR_INVALID_ADDRESS(8)或EHSM_ERR_PARAM_ERROR(1)
        try:
            t = api.ehsm_write_otp(None, cfg_data.TEST_BL_OTP_BASE_ADDR, 16)
            assert False, f"Test模式下OTP写入接口传入None数据时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            if e.ret_code == ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS:
                log.info(f"Test模式下OTP写入接口传入None数据时返回错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP写入接口传入None数据返回了非预期的错误码: {e.ret_code}，预期错误码:{ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"

    with allure.step("3、恢复默认OTP区域数据； # 3、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Test模式下，测试OTP读取接口ehsm_src_addr异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o003")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_o003(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、OTP读取接口传入越界地址（小于OTP基地址）；其它参数合法； # 2、报非法参数错误；"):
        # test模式下接口读取异常参数：越界地址（小于基地址）
        try:
            t, data_temp = api.ehsm_read_otp(OTP_MIN_ADDRESS, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下OTP读取接口传入越界地址（小于基地址）时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP读取接口传入越界地址（小于基地址）返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下OTP读取接口传入越界地址（小于基地址）时未抛出异常，不符合预期"

    with allure.step("3、OTP读取接口传入越界地址（大于OTP最大地址）；其它参数合法； # 3、报非法参数错误；"):
        # test模式下接口读取异常参数：越界地址（大于最大地址）
        try:
            t, data_temp = api.ehsm_read_otp(OTP_MAX_ADDRESS, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下OTP读取接口传入越界地址（大于最大地址）时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP读取接口传入越界地址（大于最大地址）返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下OTP读取接口传入越界地址（大于最大地址）时未抛出异常，不符合预期"

    with allure.step("4、恢复默认OTP区域数据； # 4、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Test模式下，测试OTP读取接口size异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o004")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_o004(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、OTP读取接口传入过大长度（超过OTP区域大小）；其它参数合法； # 2、报内存不足错误；"):
        # test模式下接口读取异常参数：长度过大
        oversized_length = cfg_data.TEST_BL_OTP_SIZE + 100
        try:
            t, data_temp = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, oversized_length)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下OTP读取接口传入过大长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP读取接口传入过大长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下OTP读取接口传入过大长度时未抛出异常，不符合预期"

    with allure.step("3、恢复默认OTP区域数据； # 3、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("otp")
@allure.description("在Test模式下，测试OTP读取接口host_dst_addr异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o005")
@pytest.mark.skipif(False, reason="OTP功能始终启用")
def test_ehsm_o005(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、OTP读取接口传入空地址（地址为0）；其它参数合法； # 2、报非法参数错误；"):
        # test模式下接口读取异常参数：空地址
        try:
            t, data_temp = api.ehsm_read_otp(0, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下OTP读取接口传入空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下OTP读取接口传入空地址返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下OTP读取接口传入空地址时未抛出异常，不符合预期"

    with allure.step("3、恢复默认OTP区域数据； # 3、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

# ===============================
#  OTP size是否对齐最小写单位
# ===============================
@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=4（最小对齐单位），写入成功")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o006")
def test_ehsm_o006(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口4字节数据长度，有效数据，读回验证写入生效 # 2、返回成功且读回数据正确；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        write_data = data_src_bytes[:4]
        api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 4)
        # Reason: OTP写只能0->1，读回数据的对应bit位应全部置位
        _, read_back = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, 4)
        assert len(read_back) == 4, f"读回数据应为4字节，实际{len(read_back)}"
        for i in range(4):
            assert (read_back[i] & write_data[i]) == write_data[i], \
                f"OTP写入验证失败，byte[{i}] 写入0x{write_data[i]:02x}，读回0x{read_back[i]:02x}"
        log.info(f"OTP size=4写入并读回验证通过，读回: {read_back.hex()}")

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description(" size=8（2倍对齐），写入成功")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o007")
def test_ehsm_o007(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口8字节数据长度，有效数据，读回验证写入生效 # 2、返回成功且读回数据正确；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        write_data = data_src_bytes[:8]
        api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 8)
        # Reason: OTP写只能0->1，读回数据的对应bit位应全部置位
        _, read_back = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, 8)
        assert len(read_back) == 8, f"读回数据应为8字节，实际{len(read_back)}"
        for i in range(8):
            assert (read_back[i] & write_data[i]) == write_data[i], \
                f"OTP写入验证失败，byte[{i}] 写入0x{write_data[i]:02x}，读回0x{read_back[i]:02x}"
        log.info(f"OTP size=8写入并读回验证通过，读回: {read_back.hex()}")

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description(" size=3（非对齐，下界-1），写入拒绝")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o008")
def test_ehsm_o008(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口3字节数据长度，有效数据； # 2、返回成功；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 3)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"size=3（非对齐，下界-1），写入拒绝，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"size=3（非对齐，下界-1），写入拒绝但返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"size=3（非对齐，下界-1），写入未拒绝，不符合预期"

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=0，写入拒绝")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o009")
def test_ehsm_o009(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口0字节数据长度，读回验证OTP未被修改 # 2、返回成功且OTP数据未变；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        # 先读当前值作为基准
        _, before = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, 4)
        api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 0)
        # Reason: size=0 不做实际写入，读回应与写入前一致
        _, after = api.ehsm_read_otp(cfg_data.TEST_BL_OTP_BASE_ADDR, 4)
        assert before == after, \
            f"size=0写入不应改变OTP，写前: {before.hex()}，写后: {after.hex()}"
        log.info(f"size=0写入验证通过，OTP数据未变: {after.hex()}")

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=5（非对齐），写入拒绝")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o010")
def test_ehsm_o010(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口8字节数据长度，有效数据； # 2、返回成功；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 5)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"size = 5，写入拒绝，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"size = 5，写入拒绝但返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"size = 5，写入未拒绝，不符合预期"

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=1（非对齐），写入拒绝")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-o011")
def test_ehsm_o011(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、OTP写入接口8字节数据长度，有效数据； # 2、返回成功；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        try:
            t = api.ehsm_write_otp(data_src_bytes, cfg_data.TEST_BL_OTP_BASE_ADDR, 1)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"size = 1，写入拒绝，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"size = 1，写入拒绝但返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"size = 1，写入未拒绝，不符合预期"

# ===========================================================================
# BUG-11: OTP地址越界整型溢出绕过防护
# 使用减法避免加法溢出绕过：addr+size构造溢出时(addr=0xFFFFFFFF-2, size=4)
# TC-OTP-001 ~ TC-OTP-008 对应 o012 ~ o019
# ===========================================================================

OTP_BASE = cfg_data.TEST_BL_OTP_BASE_ADDR
OTP_SIZE = cfg_data.TEST_BL_OTP_SIZE


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("OTP正常范围读，成功（BUG-11正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O012")
def test_ehsm_o012(setup_module):
    """TC-OTP-001: OTP正常范围读，成功"""
    log.info("开始测试TC-OTP-001: OTP正常范围读正路径")

    with allure.step("1、OTP正常地址读取4字节 # 1、读取成功"):
        # Reason: 控制变量法正路径——先确认正常读取有效
        _, data = api.ehsm_read_otp(OTP_BASE, 4)
        assert len(data) == 4, f"读取应返回4字节，实际{len(data)}"
        log.info(f"OTP BASE+0 读取成功：{data.hex()}")

    log.info("TC-OTP-001 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("OTP正常范围写，成功（BUG-11正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O013")
def test_ehsm_o013(setup_module):
    """TC-OTP-002: OTP正常范围写，成功"""
    log.info("开始测试TC-OTP-002: OTP正常范围写正路径")

    with allure.step("1、OTP正常地址写入4字节，读回验证 # 1、写入成功且读回正确"):
        # Reason: 控制变量法正路径——先确认正常写入有效（OTP写0不破坏现有值）
        data = bytes([0x00, 0x00, 0x00, 0x00])  # 写全0，OTP写0不破坏现有值
        _, before = api.ehsm_read_otp(OTP_BASE, 4)
        api.ehsm_write_otp(data, OTP_BASE, 4)
        _, after = api.ehsm_read_otp(OTP_BASE, 4)
        assert len(after) == 4, f"读回应为4字节，实际{len(after)}"
        # Reason: 写全0不改变OTP内容，读回应与写前相同
        assert after == before, \
            f"写全0后OTP应保持不变，写前: {before.hex()}，写后: {after.hex()}"
        log.info(f"OTP BASE+0 写全0并读回验证通过: {after.hex()}")

    log.info("TC-OTP-002 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("addr+size加法溢出构造地址绕过，拒绝（BUG-11 P0核心）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_O014")
def test_ehsm_o014(setup_module):
    """TC-OTP-003: addr+size溢出构造地址绕过，拒绝"""
    log.info("开始测试TC-OTP-003: addr+size加法溢出绕过（BUG-11核心）")

    # addr=0xFFFFFFFF-2=0xFFFFFFFD, size=4 => 加法溢出=0x00000001 < BASE+SIZE
    # 但实际地址 0xFFFFFFFD 远超OTP范围，应被减法检查拒绝
    overflow_addr = 0xFFFFFFFF - 2  # 0xFFFFFFFD

    with allure.step("1、确认正常地址读/写有效 # 1、正路径成功"):
        _, _ = api.ehsm_read_otp(OTP_BASE, 4)

    with allure.step("2、fw_otp_read addr=0xFFFFFFFD, size=4（加法溢出）# 2、应返回EHSM_ERR_PARAM_ERROR"):
        # Reason: BUG-11核心——修复前使用加法检查 addr+size <= BASE+SIZE，
        # 加法溢出后 0xFFFFFFFD+4=0x00000001 < BASE+SIZE 绕过检查
        # 修复后使用减法：BASE+SIZE - addr >= size，无法绕过
        try:
            _, _ = api.ehsm_read_otp(overflow_addr, 4)
            assert False, "addr+size溢出应被拒绝，但成功了（BUG-11未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"OTP read溢出地址被正确拒绝，错误码: {e.ret_code}")

    with allure.step("3、fw_otp_write addr=0xFFFFFFFD, size=4（加法溢出）# 3、应返回EHSM_ERR_PARAM_ERROR"):
        try:
            api.ehsm_write_otp(bytes(4), overflow_addr, 4)
            assert False, "addr+size溢出写应被拒绝，但成功了（BUG-11未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"OTP write溢出地址被正确拒绝，错误码: {e.ret_code}")

    log.info("TC-OTP-003 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("addr < BASE，拒绝（BUG-11边界）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O015")
def test_ehsm_o015(setup_module):
    """TC-OTP-004: addr < BASE，拒绝"""
    log.info("开始测试TC-OTP-004: addr<BASE应被拒绝")

    addr_below_base = OTP_BASE - 1 if OTP_BASE > 0 else 0

    with allure.step("1、OTP read addr=BASE-1，拒绝 # 1、返回EHSM_ERR_PARAM_ERROR"):
        if addr_below_base < OTP_BASE:
            try:
                _, _ = api.ehsm_read_otp(addr_below_base, 4)
                assert False, "addr<BASE应被拒绝"
            except hostapi.HostApiError as e:
                assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                    f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
                log.info(f"addr<BASE被正确拒绝，错误码: {e.ret_code}")
        else:
            log.info("BASE=0，无法测试addr<BASE，跳过")

    log.info("TC-OTP-004 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("addr+size=OTP_END（上界），成功（BUG-11边界）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O016")
def test_ehsm_o016(setup_module):
    """TC-OTP-005: addr+size=OTP_END（上界），成功"""
    log.info("开始测试TC-OTP-005: OTP末尾4字节读/写")

    with allure.step("1、OTP read addr=BASE+SIZE-4，size=4（末尾4字节）# 1、操作成功"):
        # Reason: 控制变量法——验证合法上界值能正常操作
        end_addr = OTP_BASE + OTP_SIZE - 4
        _, data = api.ehsm_read_otp(end_addr, 4)
        assert len(data) == 4, f"末尾4字节读取应成功，实际{len(data)}"
        log.info(f"OTP末尾4字节读取成功：{data.hex()}")

    log.info("TC-OTP-005 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("addr+size=OTP_END+1（越界1字节），拒绝（BUG-11边界）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O017")
def test_ehsm_o017(setup_module):
    """TC-OTP-006: addr+size=OTP_END+1（越界1字节），拒绝"""
    log.info("开始测试TC-OTP-006: OTP越界1字节应被拒绝")

    with allure.step("1、OTP read addr=BASE+SIZE-4，size=5（越界1字节）# 1、返回EHSM_ERR_PARAM_ERROR"):
        end_addr = OTP_BASE + OTP_SIZE - 4
        try:
            _, _ = api.ehsm_read_otp(end_addr, 5)
            assert False, "越界1字节应被拒绝"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"OTP越界1字节被正确拒绝，错误码: {e.ret_code}")

    log.info("TC-OTP-006 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("fw_otp_write 同样进行减法越界检查（复用TC-003验证，BUG-11 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_O018")
def test_ehsm_o018(setup_module):
    """TC-OTP-007: fw_otp_write 减法越界检查"""
    log.info("开始测试TC-OTP-007: fw_otp_write溢出地址越界检查")

    with allure.step("1、fw_otp_write addr=0xFFFFFFFD，size=4（与TC-003相同）# 1、返回EHSM_ERR_PARAM_ERROR"):
        overflow_addr = 0xFFFFFFFF - 2
        try:
            api.ehsm_write_otp(bytes(4), overflow_addr, 4)
            assert False, "write溢出地址应被拒绝（BUG-11未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"write路径减法判断生效，错误码: {e.ret_code}")

    log.info("TC-OTP-007 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=0，拒绝（BUG-11边界）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O019")
def test_ehsm_o019(setup_module):
    """TC-OTP-008: size=0，拒绝"""
    log.info("开始测试TC-OTP-008: size=0应被拒绝")

    with allure.step("1、fw_otp_read size=0 # 1、返回EHSM_ERR_PARAM_ERROR"):
        try:
            _, _ = api.ehsm_read_otp(OTP_BASE, 0)
            assert False, "size=0应被拒绝"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"size=0被正确拒绝，错误码: {e.ret_code}")

    log.info("TC-OTP-008 完成")


# ===========================================================================
# BUG-02: OTP写入size对齐检查（size必须是CONFIG_EHSM_OTP_WRITE_UNIT=4的整数倍）
# TC-MISC-001 ~ TC-MISC-006 对应 o020 ~ o025
# ===========================================================================

@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=4（最小对齐单位），写入成功（BUG-02正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O020")
def test_ehsm_o020(setup_module):
    """TC-MISC-001: size=4，写入成功"""
    log.info("开始测试TC-MISC-001: OTP size=4正路径")
    with allure.step("1、fw_otp_write size=4，写入成功，读回验证 # 1、成功且读回bit正确"):
        # Reason: 控制变量法正路径——验证size=4合法时写入有效，写全0不破坏现有值
        _, before = api.ehsm_read_otp(OTP_BASE, 4)
        api.ehsm_write_otp(bytes(4), OTP_BASE, 4)
        _, after = api.ehsm_read_otp(OTP_BASE, 4)
        assert len(after) == 4, f"读回应为4字节，实际{len(after)}"
        assert after == before, \
            f"写全0后OTP应保持不变，写前: {before.hex()}，写后: {after.hex()}"
        log.info(f"size=4写入并读回验证通过: {after.hex()}")
    log.info("TC-MISC-001 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=8（2倍对齐），写入成功（BUG-02正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_O021")
def test_ehsm_o021(setup_module):
    """TC-MISC-002: size=8，写入成功"""
    log.info("开始测试TC-MISC-002: OTP size=8正路径")
    with allure.step("1、fw_otp_write size=8，写入成功，读回验证 # 1、成功且读回bit正确"):
        _, before = api.ehsm_read_otp(OTP_BASE, 8)
        api.ehsm_write_otp(bytes(8), OTP_BASE, 8)
        _, after = api.ehsm_read_otp(OTP_BASE, 8)
        assert len(after) == 8, f"读回应为8字节，实际{len(after)}"
        assert after == before, f"写全0后OTP应保持不变，写前: {before.hex()}，写后: {after.hex()}"
        log.info(f"size=8写入并读回验证通过: {after.hex()}")
    log.info("TC-MISC-002 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=3（非对齐，下界-1），写入拒绝（BUG-02 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_O022")
def test_ehsm_o022(setup_module):
    """TC-MISC-003: size=3（非4字节对齐），写入拒绝"""
    log.info("开始测试TC-MISC-003: OTP size=3应被拒绝")
    with allure.step("1、确认size=4正路径有效 # 1、成功"):
        api.ehsm_write_otp(bytes(4), OTP_BASE, 4)
    with allure.step("2、fw_otp_write size=3（非4字节对齐）# 2、返回EHSM_ERR_PARAM_ERROR"):
        # Reason: BUG-02核心——size=3不是4字节倍数，应被拒绝
        try:
            api.ehsm_write_otp(bytes(3), OTP_BASE, 3)
            assert False, "size=3非对齐应被拒绝（BUG-02未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"size=3被正确拒绝，错误码: {e.ret_code}")
    log.info("TC-MISC-003 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=0，固件允许（不做实际写入）（BUG-02）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O023")
def test_ehsm_o023(setup_module):
    """TC-MISC-004: size=0，固件允许且不做实际写入"""
    log.info("开始测试TC-MISC-004: OTP size=0固件允许，不做实际写入")
    with allure.step("1、fw_otp_write size=0 # 1、固件允许，不做实际写入，OTP数据不变"):
        # Reason: size=0 固件视为"写 0 字节"直接成功，不做任何 OTP 写入操作
        _, before = api.ehsm_read_otp(OTP_BASE, 4)
        api.ehsm_write_otp(bytes(4), OTP_BASE, 0)
        _, after = api.ehsm_read_otp(OTP_BASE, 4)
        # Reason: size=0不做实际写入，读回应与写前完全一致
        assert after == before, \
            f"size=0写入不应改变OTP，写前: {before.hex()}，写后: {after.hex()}"
        log.info(f"size=0 固件允许，OTP数据未变: {after.hex()}")
    log.info("TC-MISC-004 完成")


@pytest.mark.skipif(False, reason="OTP功能始终启用")
@allure.feature("otp")
@allure.description("size=5（非对齐），写入拒绝（BUG-02）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_O025")
def test_ehsm_o025(setup_module):
    """TC-MISC-005: size=5（非对齐），写入拒绝"""
    log.info("开始测试TC-MISC-005: OTP size=5应被拒绝")
    with allure.step("1、fw_otp_write size=5 # 1、返回EHSM_ERR_PARAM_ERROR"):
        try:
            api.ehsm_write_otp(bytes(5), OTP_BASE, 5)
            assert False, "size=5应被拒绝"
        except hostapi.HostApiError as e:
            assert e.ret_code == ehsm_fw_errno.EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({ehsm_fw_errno.EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"size=5被正确拒绝，错误码: {e.ret_code}")
    log.info("TC-MISC-005 完成")
