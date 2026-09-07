import logging as log
import pytest
import allure
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from utils import otp
from utils.config import cfg_data
import platform_adapter.uart_lib.ehsm_bl_errno as bl_errno

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc})

REG_MAX_ADDRESS = 0x33500001
REG_MIN_ADDRESS = 0x333fffff


@allure.feature("reg")
@allure.description("在Dev模式下，测试bootloader REG读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1110")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_1110(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、超出REG区域边界写入数据，返回预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # dev模式下写入REG区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在小于REG边界的地址写入时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在小于REG边界的地址写入时未抛出异常，不符合预期"

        # dev模式下写入REG区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在大于REG边界的地址写入时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在大于REG边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，读取到tmp空间； # 3、超出REG区域边界读取数据，返回预期的错误码"):
        # dev模式下读REG区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"对小于REG边界的地址读值时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对小于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"对小于REG边界的地址读值时未抛出异常，不符合预期"

        # dev模式下读REG区域最大边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"对大于REG边界的地址读值时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对大于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"对大于REG边界的地址读值时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Manu模式下，测试bootloader REG读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1111")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_1111(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、超出REG区域边界写入数据，返回预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Manu模式下写入REG区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"对小于REG边界的地址读值时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对小于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"对小于REG边界的地址读值时未抛出异常，不符合预期"

        # Manu模式下写入REG区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在大于REG边界的地址写入时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在大于REG边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，读取到tmp空间； # 3、超出REG区域边界读取数据，返回预期的错误码"):
        # manu模式下读REG区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"对小于REG边界的地址读值时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对小于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"对小于REG边界的地址读值时未抛出异常，不符合预期"

        # manu模式下读REG区域最小边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"对大于REG边界的地址读值时数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对大于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"对大于REG边界的地址读值时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在User模式下，测试bootloader REG读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1112")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_1112(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、user模式下在超出REG边界区域写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 写入REG区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在小于REG边界的地址写入受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"在小于REG边界的地址写入时未抛出异常，不符合预期"

        # 写入REG区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在大于REG边界的地址写入受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"大于REG边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，读取到tmp空间； # 3、user模式下在超出REG边界区域读取数据受到生命周期限制"):
        # 读REG区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在小于REG边界的地址读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"在小于REG边界的地址读取数据时未抛出异常，不符合预期"

        # 读REG区域最大边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在大于REG边界的地址读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"大于REG边界的地址读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Debug模式下，测试bootloader REG读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1113")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_1113(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、debug模式下在超出REG区域写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Debug模式下写入REG区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在小于REG边界的地址写入受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"在小于REG边界的地址写入时未抛出异常，不符合预期"

         # Debug模式下写入REG区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在大于REG边界的地址写入受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"大于REG边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS 长度16字节，读取到tmp空间； # 3、debug模式下在超出REG区域读取数据受到生命周期限制"):
        # Debug模式下读REG区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在小于REG边界的地址读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"在小于REG边界的地址读取数据时未抛出异常，不符合预期"

        # Debug模式下读REG区域最大边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"在大于REG边界的地址读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"大于REG边界的地址读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Test模式下，测试bootloader REG读写功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-117")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_117(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("3、读取REG区域REG_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes

@allure.feature("reg")
@allure.description("在Test模式下，测试bootloader REG读写边界")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-118")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_118(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、超出REG边界区域写入数据返回预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Test模式下写入OTP区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在小于REG边界的地址写入数据，数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在小于REG边界的地址写入数据时未抛出异常，不符合预期"

        # Test模式下写入OTP区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在大于REG边界的地址写入数据，数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在大于REG边界的地址写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_MIN_ADDRESS/REG_MAX_ADDRESS, 长度16字节，读取到tmp空间； # 3、超出REG边界区域读取数据返回预期的错误码"):
        # Test模式下读取OTP区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在小于REG边界的地址读取数据，数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在小于REG边界的地址读取数据时未抛出异常，不符合预期"

        # Test模式下读取OTP区域最小边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"在大于REG边界的地址读取数据，数据长度错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址读取数据时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"在大于REG边界的地址读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Dev模式下，测试bootloader REG读写功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-119")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_119(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("3、读取REG区域REG_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes

@allure.feature("reg")
@allure.description("在Manu模式下，测试bootloader REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-120")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_120(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、manu模式下REG写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        # manu模式下写入数据
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_BASE_ADDRESS, 长度16字节，读取到tmp空间； # 3、manu模式下REG读取数据受到生命周期限制"):
        # manu模式下读取数据
        try:
            t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下对REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下对REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下对REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在User模式下，测试bootloader REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-121")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_121(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、user模式下REG写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # User模式下写入数据
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_BASE_ADDRESS, 长度16字节，读取到tmp空间； #3、user模式下REG读取数据受到生命周期限制"):
        # User模式下读取数据
        try:
            t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下对REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下对REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下对REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Debug模式下，测试bootloader REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-122")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_122(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、写入REG区域REG_BASE_ADDRESS, 长度16字节，0x01递增到0x0F 数据； # 2、debug模式下REG写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        # Debug模式下写入数据
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域REG_BASE_ADDRESS, 长度16字节，读取到tmp空间； #3、debug模式下REG读取数据受到生命周期限制"):
        # Debug模式下读取数据
        try:
            t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Test模式下，测试bootloader REG读写接口异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-123")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_READ_SUPPORT == 0,reason="REG READ 功能不支持")
@pytest.mark.skipif( cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0,reason="REG WRITE 功能不支持")
def test_ehsm_123(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、REG读写接口传入空的src_addr；其它参数合法； # 2、写入非法参数时报非法参数，符合预期；"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        # Test模式下写入异常数据
        try:
            t = api.ehsm_write_reg(data_src_bytes, 0, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下对写入接口传入空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下对写入接口传入空地址时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下对写入接口传入空地址时未抛出异常，不符合预期"
    with allure.step("3、REG读写接口传入空的dst_addr；其它参数合法； # 3、读取非法参数时报非法参数，符合预期；"):
        # Test模式下读取异常数据
        try:
            t , data_temp = api.ehsm_read_reg(0, len(data_src))
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下读取接口读取空地址时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下读取接口读取空地址时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下读取接口读取空地址时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG读取接口size=0参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_R001")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_READ_SUPPORT == 0, reason="REG READ功能不支持")
def test_ehsm_r001(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG读取接口，地址合法但size=0； # 2、报非法参数，符合预期；"):
        try:
            t, data = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, 0)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"REG读取size=0时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG读取size=0时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"REG读取size=0时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG读取接口size过大导致越界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R002")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_READ_SUPPORT == 0, reason="REG READ功能不支持")
def test_ehsm_r002(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG读取接口，从合法地址读取但size超过REG区域大小； # 2、报非法参数，符合预期；"):
        oversized = cfg_data.TEST_BL_REG_SIZE * 2
        try:
            t, data = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, oversized)
        except hostapi.HostApiError as e:
            if  bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG读取size过大时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG读取size过大时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"REG读取size过大时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG读取接口地址+size越界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R003")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_READ_SUPPORT == 0, reason="REG READ功能不支持")
def test_ehsm_r003(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG读取接口，地址合法但地址+size超出REG区域； # 2、报非法参数，符合预期；"):
        # Reason: 从REG区域的末尾前8字节开始读取16字节，会越界8字节
        near_end_addr = cfg_data.TEST_BL_REG_BASE_ADDR + cfg_data.TEST_BL_REG_SIZE - 8
        try:
            t, data = api.ehsm_read_reg(near_end_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code or bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG读取地址+size越界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG读取地址+size越界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"REG读取地址+size越界时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG写入接口size=0参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_R004")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0, reason="REG WRITE功能不支持")
def test_ehsm_r004(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG写入接口，数据和地址合法但size=0； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        try:
            t = api.ehsm_write_reg(data_src, cfg_data.TEST_BL_REG_BASE_ADDR, 0)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"REG写入size=0时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG写入size=0时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"REG写入size=0时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG写入接口size超大导致越界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R005")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0, reason="REG WRITE功能不支持")
def test_ehsm_r005(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG写入接口，数据和地址合法但size超过REG区域大小； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        oversized = cfg_data.TEST_BL_REG_SIZE * 2
        try:
            t = api.ehsm_write_reg(data_src, cfg_data.TEST_BL_REG_BASE_ADDR, oversized)
        except hostapi.HostApiError as e:
            if  bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG写入size超大时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG写入size超大时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"REG写入size超大时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG写入接口地址+size越界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R006")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0, reason="REG WRITE功能不支持")
def test_ehsm_r006(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG写入接口，地址合法但地址+size超出REG区域； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        # Reason: 从REG区域的末尾前8字节开始写入16字节，会越界8字节
        near_end_addr = cfg_data.TEST_BL_REG_BASE_ADDR + cfg_data.TEST_BL_REG_SIZE - 8
        try:
            t = api.ehsm_write_reg(data_src, near_end_addr, 16)
        except hostapi.HostApiError as e:
            if  bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG写入地址+size越界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG写入地址+size越界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"REG写入地址+size越界时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG写入接口地址小于最小边界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R007")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0, reason="REG WRITE功能不支持")
def test_ehsm_r007(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG写入接口，地址小于REG最小边界； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        invalid_addr = cfg_data.TEST_BL_REG_BASE_ADDR - 4
        try:
            t = api.ehsm_write_reg(data_src, invalid_addr, 16)
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG写入地址小于最小边界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG写入地址小于最小边界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"REG写入地址小于最小边界时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("测试REG写入接口地址大于最大边界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_R008")
@pytest.mark.skipif(cfg_data.TEST_BL_REG_WRITE_SUPPORT == 0, reason="REG WRITE功能不支持")
def test_ehsm_r008(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、调用REG写入接口，地址大于REG最大边界； # 2、报非法参数，符合预期；"):
        data_src = bytes([i % 256 for i in range(16)])
        invalid_addr = cfg_data.TEST_BL_REG_BASE_ADDR + cfg_data.TEST_BL_REG_SIZE + 4
        try:
            t = api.ehsm_write_reg(data_src, invalid_addr, 16)
        except hostapi.HostApiError as e:
            if  bl_errno.EHSM_ERR_WRONG_DATA_LENGTH == e.ret_code:
                log.info(f"REG写入地址大于最大边界时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"REG写入地址大于最大边界时返回了非预期的错误码: {e.ret_code}，预期错误码: {bl_errno.EHSM_ERR_WRONG_DATA_LENGTH}"
        else:
            assert False, f"REG写入地址大于最大边界时未抛出异常，不符合预期"