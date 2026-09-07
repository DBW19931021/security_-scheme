import logging as log
import allure
import pytest
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_fw_errno, hostapi
from platform_adapter.uart_lib.soccmd import CmdRspError
from utils import otp
from utils.config import cfg_data
from platform_adapter.uart_lib import ehsm_fw_errno

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
@allure.description("在Test模式下，测试Firmware REG读写功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-792")
def test_ehsm_792(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入REG区域 0x33400000, 长度16字节，0x00~0xFF 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("3、读取REG区域 0x33400000, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes


@allure.feature("reg")
@allure.description("在Test模式下，测试Firmware REG读写边界")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-793")
def test_ehsm_793(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入REG区域 0x33500001/0x333fffff, 长度16字节，0x00~0xFF 数据； # 2、超出reg边界的区域写入数据返回预期的错误码"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # 写入REG区域最小边界-1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MIN_ADDRESS, len(data_src_bytes))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"在小于REG边界的地址写入时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在小于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"在小于REG边界的地址写入时未抛出异常，不符合预期"

        # 写入REG区域最大边界+1
        try:
            t = api.ehsm_write_reg(data_src_bytes, REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"在大于REG边界的地址写入时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"在大于REG边界的地址写入时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"在大于REG边界的地址写入时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域 0x33500001/0x333fffff, 长度16字节，读取到tmp空间； # 3、超出reg边界的区域读取数据返回预期的错误码"):
        # 读REG区域最小边界-1
        try:
            t , data_temp1 = api.ehsm_read_reg(REG_MIN_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"对小于REG边界的地址读值时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对小于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"对小于REG边界的地址读值时未抛出异常，不符合预期"

        # 读REG区域最大边界+1
        try:
            t , data_temp2 = api.ehsm_read_reg(REG_MAX_ADDRESS, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"对大于REG边界的地址读值时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"对大于REG边界的地址读值时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"对大于REG边界的地址读值时未抛出异常，不符合预期"

@allure.feature("reg")
@allure.description("在Dev模式下，测试Firmware REG读写功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-794")
def test_ehsm_794(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入REG区域 0x33400000, 长度16字节，0x00~0xFF 数据； # 2、写入成功"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)
        t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("3、读取REG区域 0x33400000, 长度16字节，读取到tmp空间； # 3、读取成功，数据正确"):
        t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
    with allure.step("4、对比tmp数据； # 4、对比tmp数据与写入数据一致；"):
        assert data_temp == data_src_bytes

@allure.feature("reg")
@allure.description("在Manu模式下，测试Firmware REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-795")
def test_ehsm_795(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("1、写入REG区域 0x33400000, 长度16字节，0x00~0xFF 数据； # 2、manu模式下向reg写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Manu模式下写入
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("3、读取REG区域 0x33400000, 长度16字节，读取到tmp空间； # 3、manu模式下读取reg数据受到生命周期限制"):
        # Manu模式下读取
        try:
            t , data_temp1 = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Manu模式下对REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式下对REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Manu模式下对REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在User模式下，测试Firmware REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-796")
def test_ehsm_796(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、写入REG区域 0x33400000, 长度16字节，0x00~0xFF 数据； # 2、写入受到生命周期限制；"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # User模式下写入
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("1、读取REG区域 0x33400000, 长度16字节，读取到tmp空间； # 3、读取数据受到生命周期限制；"):
        # User模式下读取
        try:
            t , data_temp1 = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"User模式下对REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式下对REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"User模式下对REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Debug模式下，测试Firmware REG读写功能的限制能力")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-797")
def test_ehsm_797(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("1、写入REG区域 0x33400000, 长度16字节，0x00~0xFF 数据； # 2、写入数据受到生命周期限制"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Debug模式下写入
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下向REG区域写入数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下向REG区域写入数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下向REG区域写入数据时未抛出异常，不符合预期"
    with allure.step("1、读取REG区域 0x33400000, 长度16字节，读取到tmp空间； # 3、读取数据受到生命周期限制"):
        # Debug模式下读取
        try:
            t , data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"Debug模式下对REG区域读取数据受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式下对REG区域读取数据返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"Debug模式下对REG区域读取数据时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Test模式下，测试Firmware REG读写接口异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-798")
def test_ehsm_798(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("1、REG读写接口传入空的src_addr；其它参数合法； # 2、报非法参数；"):
        data_src = [0] * 16
        data_temp = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # Test模式下写入异常参数：空地址
        try:
            t = api.ehsm_write_reg(data_src_bytes, 0, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下对写入接口传入空地址时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下对写入接口传入空地址时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下对写入接口传入空地址时未抛出异常，不符合预期"
    with allure.step("1、REG读写接口传入空的dst_addr；其它参数合法； # 3、报非法参数；"):
        # Test模式下读取异常参数：空地址
        try:
            t , data_temp = api.ehsm_read_reg(0, len(data_src))
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下读取接口读取空地址时地址越界，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下读取接口读取空地址时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下读取接口读取空地址时未抛出异常，不符合预期"


@allure.feature("reg")
@allure.description("在Test模式下，测试REG写入接口size异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-r001")
@pytest.mark.skipif(False, reason="REG功能始终启用")
def test_ehsm_r001(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、REG写入接口传入过大长度（超过REG区域大小）；其它参数合法； # 2、报内存不足错误；"):
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 16
        data_src_bytes = bytes(data_src)

        # test模式下接口写入异常参数：长度过大
        oversized_length = cfg_data.TEST_BL_REG_SIZE + 100
        try:
            t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, oversized_length)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG写入接口传入过大长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG写入接口传入过大长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG写入接口传入过大长度时未抛出异常，不符合预期"

    with allure.step("3、REG写入接口传入跨越边界的长度（地址合法但地址+长度超出REG区域）；其它参数合法； # 3、报内存不足错误；"):
        # test模式下接口写入异常参数：跨越边界的长度
        # 从REG区域末尾前16字节开始，写入32字节，会超出边界
        boundary_addr = cfg_data.TEST_BL_REG_BASE_ADDR + cfg_data.TEST_BL_REG_SIZE - 16
        boundary_size = 32  # 超出边界
        try:
            t = api.ehsm_write_reg(data_src_bytes, boundary_addr, boundary_size)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG写入接口传入跨越边界的长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG写入接口传入跨越边界的长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG写入接口传入跨越边界的长度时未抛出异常，不符合预期"

    with allure.step("4、恢复默认OTP区域数据； # 4、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("reg")
@allure.description("在Test模式下，测试REG写入接口src_data异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-r002")
@pytest.mark.skipif(False, reason="REG功能始终启用")
def test_ehsm_r002(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、REG写入接口传入NULL数据（None）；其它参数合法； # 2、报无效地址错误；"):
        # test模式下接口写入异常参数：None数据
        # Reason: 传入None时，API层转换为src_addr=0，固件可能返回EHSM_ERR_INVALID_ADDRESS(8)或EHSM_ERR_PARAM_ERROR(1)
        try:
            t = api.ehsm_write_reg(None, cfg_data.TEST_BL_REG_BASE_ADDR, 16)
            assert False, f"Test模式下REG写入接口传入None数据时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            if e.ret_code == ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS:
                log.info(f"Test模式下REG写入接口传入None数据时返回错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG写入接口传入None数据返回了非预期的错误码: {e.ret_code}，预期错误码:{ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"

    with allure.step("3、恢复默认OTP区域数据； # 3、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("reg")
@allure.description("在Test模式下，测试REG读取接口ehsm_src_addr异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-r003")
@pytest.mark.skipif(False, reason="REG功能始终启用")
def test_ehsm_r003(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、REG读取接口传入越界地址（小于REG基地址）；其它参数合法； # 2、报内存不足错误；"):
        # test模式下接口读取异常参数：越界地址（小于基地址）
        try:
            t, data_temp = api.ehsm_read_reg(REG_MIN_ADDRESS, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG读取接口传入越界地址（小于基地址）时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG读取接口传入越界地址（小于基地址）返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG读取接口传入越界地址（小于基地址）时未抛出异常，不符合预期"

    with allure.step("3、REG读取接口传入越界地址（大于REG最大地址）其它参数合法； # 3、报内存不足错误；"):
        # test模式下接口读取异常参数：越界地址（大于最大地址）
        try:
            t, data_temp = api.ehsm_read_reg(REG_MAX_ADDRESS, 16)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG读取接口传入越界地址（大于最大地址）时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG读取接口传入越界地址（大于最大地址）返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG读取接口传入越界地址（大于最大地址）时未抛出异常，不符合预期"

    with allure.step("4、恢复默认OTP区域数据； # 4、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("reg")
@allure.description("在Test模式下，测试REG读取接口size异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-r004")
@pytest.mark.skipif(False, reason="REG功能始终启用")
def test_ehsm_r004(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、REG读取接口传入过大长度（超过REG区域大小）；其它参数合法； # 2、报内存不足错误；"):
        # test模式下接口读取异常参数：长度过大
        oversized_length = cfg_data.TEST_BL_REG_SIZE + 100
        try:
            t, data_temp = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, oversized_length)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG读取接口传入过大长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG读取接口传入过大长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG读取接口传入过大长度时未抛出异常，不符合预期"

    with allure.step("3、REG读取接口传入跨越边界的长度（地址合法但地址+长度超出REG区域）；其它参数合法； # 3、报内存不足错误；"):
        # test模式下接口读取异常参数：跨越边界的长度
        # 从REG区域末尾前16字节开始，读取32字节，会超出边界
        boundary_addr = cfg_data.TEST_BL_REG_BASE_ADDR + cfg_data.TEST_BL_REG_SIZE - 16
        boundary_size = 32  # 超出边界
        try:
            t, data_temp = api.ehsm_read_reg(boundary_addr, boundary_size)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM == e.ret_code:
                log.info(f"Test模式下REG读取接口传入跨越边界的长度时内存不足，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下REG读取接口传入跨越边界的长度返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_OUT_OF_MEM}"
        else:
            assert False, f"Test模式下REG读取接口传入跨越边界的长度时未抛出异常，不符合预期"

    with allure.step("4、恢复默认OTP区域数据； # 4、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

@allure.feature("reg")
@allure.description("在Test模式下，测试REG读取接口host_dst_addr数据正确性验证")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-r005")
@pytest.mark.skipif(False, reason="REG功能始终启用")
def test_ehsm_r005(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、向REG区域写入测试数据； # 2、写入成功；"):
        # 准备测试数据
        data_src = [0] * 16
        for i in range(len(data_src)):
            data_src[i] = i % 256
        data_src_bytes = bytes(data_src)
        # 写入测试数据
        t = api.ehsm_write_reg(data_src_bytes, cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))

    with allure.step("3、REG读取接口读取数据到host侧，验证数据长度正确； # 3、返回数据长度与请求长度一致；"):
        # test模式下读取数据并验证
        t, data_read = api.ehsm_read_reg(cfg_data.TEST_BL_REG_BASE_ADDR, len(data_src))

        # 验证返回数据长度
        if len(data_read) != len(data_src_bytes):
            assert False, f"Test模式下REG读取接口返回数据长度不正确: 实际长度 {len(data_read)}，预期长度 {len(data_src_bytes)}"
        else:
            log.info(f"Test模式下REG读取接口返回数据长度正确: {len(data_read)} 字节")

    with allure.step("4、验证读取到host侧的数据内容正确； # 4、数据内容与写入数据一致；"):
        # 验证返回数据内容
        if data_read != data_src_bytes:
            assert False, f"Test模式下REG读取接口返回数据内容不正确: 实际数据 {data_read.hex()}，预期数据 {data_src_bytes.hex()}"
        else:
            log.info(f"Test模式下REG读取接口返回数据内容正确，与写入数据一致")

    with allure.step("5、恢复默认OTP区域数据； # 5、执行成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
