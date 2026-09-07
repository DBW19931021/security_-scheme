import allure
import pytest
from platform_adapter.api.constants import EhsmDrvMode
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_reg, hostapi
from utils import otp
from utils.config import cfg_data
from platform_adapter.uart_lib import ehsm_fw_errno
import logging as log

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin( {"lifecycle": lc})

@allure.feature("baud")
@allure.description("在Test模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1641")
def test_ehsm_1641(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、打开串口工具：设置对应设备端口号波特率为38400 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 38400 # 3、波特率设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 38400"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 38400"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)

@allure.feature("baud")
@allure.description("在Dev模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1642")
def test_ehsm_1642(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、打开串口工具：设置对应设备端口号波特率为56000 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 56000 # 3、波特率设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/56000))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 56000"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 56000"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/56000)

@allure.feature("baud")
@allure.description("在Manu模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1643")
def test_ehsm_1643(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、打开串口工具：设置对应设备端口号波特率为230400 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为EHSM_CLK_FREQ / 230400 # 3、波特率设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/230400))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 230400"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：EHSM_CLK_FREQ / 230400"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/230400)

@allure.feature("baud")
@allure.description("在User模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1644")
def test_ehsm_1644(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、打开串口工具：设置对应设备端口号波特率为115200 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 115200 # 3、波特率设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/115200))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 115200"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 115200"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/115200)

@allure.feature("baud")
@allure.description("在Debug模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1645")
def test_ehsm_1645(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、打开串口工具：设置对应设备端口号波特率为128000 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 128000 # 3、波特率设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/128000))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 128000"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 128000"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/128000)

@allure.feature("baud")
@allure.description("在dev模式下，测试设置波特率命令对异常参数的处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-1675")
def test_ehsm_1675(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、设置波特率为16 # 2、设置成功"):
        t = api.ehsm_set_uart_baudrate(16)
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == 16
    with allure.step("3、设置波特率为15 # 3、设置波特率异常参数时返回预期错误码"):
        try:
            t = api.ehsm_set_uart_baudrate(15)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"设置波特率为15时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"设置波特率为15时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"设置波特率为15时未抛出异常，不符合预期"
    with allure.step("4、设置波特率为EHSM_CLK_FREQ / 56000 # 4、设置成功"):
        t = api.ehsm_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/56000))
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/56000)