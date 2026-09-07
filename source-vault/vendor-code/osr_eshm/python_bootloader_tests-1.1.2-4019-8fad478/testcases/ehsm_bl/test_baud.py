import logging as log
import time
import pytest
import allure
from platform_adapter.api.constants import EhsmDrvMode
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_reg, hostapi
from utils import otp
from utils.config import cfg_data

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

EHSM_CLK_FREQ_60M = 60000000

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

@pytest.fixture(scope="function")
def setup_function():
    api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_INTERRUPT)
    api.ehsm_ctx_init(0, True)
    yield api
    api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_WAIT_AND_POLL)
    api.ehsm_ctx_init(0, False)

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc})

def _get_otp_bin_wdt(level: str, lc: str) -> bytes:
    return otp.otp_to_bin({"watchdog_level":level, "lifecycle": lc})

def _get_otp_bin_log(log_en: str, lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc, "uart_log_en": log_en})

def watch_dog_error_check():
    start_time = time.time()
    while(True):
        status = host.check_watchdog_error_status()
        if status == True:
            true_time = time.time()
            if true_time - start_time > 10:
                log.debug("10秒内watchdog error 被持续置为1")
                return True
        else:
            false_time = time.time()
            if false_time- start_time >10:
                log.debug("10秒内watchdog error 被持续置为0")
                return False


@allure.feature("baud")
@allure.description("在Test模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-105")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_105(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin( "test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具：设置端口号为COM12 USB Serial Port 波特率为38400 #2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 38400 #3、设置成功"):
       t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 38400"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 38400"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        else:
            pass


@allure.feature("baud")
@allure.description("在Dev模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-106")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_106(setup_module):
    with allure.step("1、配置生命周期为Dev模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin( "dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具：设置端口号为COM12 USB Serial Port 波特率为56000 # 设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 56000 # 3、设置成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/56000))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 56000"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 56000"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/56000)
        else:
            pass

@allure.feature("baud")
@allure.description("在Manu模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-107")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_107(setup_module):
    with allure.step("1、配置生命周期为Manu模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具：设置端口号为COM12 USB Serial Port 波特率为230400 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 230400 # 3、设置成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/230400))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 230400"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 230400"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/230400)
        else:
            pass

@allure.feature("baud")
@allure.description("在User模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-108")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_108(setup_module):
    with allure.step("1、配置生命周期为User模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具： 设置端口号为COM12 USB Serial Port 波特率为115200 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 115200 # 设置成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/115200))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 115200"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 115200"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/115200)
        else:
            pass

@allure.feature("baud")
@allure.description("在Debug模式下，测试baudrate设置功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-109")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_109(setup_module):
    with allure.step("1、配置生命周期为Debug模式；并重启生效； # 1、生命周期配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具：设置端口号为COM12 USB Serial Port 波特率为128000 # 2、设置成功"):
        pass
    with allure.step("3、设置波特率为cfg_data.TEST_CPU_FREQ_HZ / 128000 # 3、设置成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/128000))
    with allure.step("4、检查下位机串口工具打印的日志 # 4、下位机串口工具日志打印：set baudrate 128000"):
        pass
    with allure.step("5、检查配置baudrate后寄存器写入的值 # 5、UART_BRR中的值为：cfg_data.TEST_CPU_FREQ_HZ / 128000"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/128000)
        else:
            pass

@allure.feature("baud")
@allure.description("验证Bootloader的watchdog配置level=5后是否能在配置值生效")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1181")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WATCHDOG_CTRL == 0,reason="OTP看门狗控制功能不支持")
def test_ehsm_1181(setup_function):
    with allure.step("1、设置看门狗level等于5，复位 # 1、设置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("level5","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、上位机发送死循环命令 # 2、发送成功"):
        assert host.check_watchdog_error_status() == False
        try:
            t = api.ehsm_test_jump_to_loop()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("等待下位机超时中，请打开串口观察")
            else:
                log.debug(f"错误码{e.ret_code}")
        time.sleep(30)
    with allure.step("3、上位机端持续读取watch dog error 位，检测到返回1 # 3、读取成功，数据正确"):
        watch_dog_error_check()


@allure.feature("baud")
@allure.description("验证Bootloader的watchdog配置level=3后是否能在配置值生效")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1261")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WATCHDOG_CTRL == 0,reason="OTP看门狗控制功能不支持")
def test_ehsm_1261(setup_function):
    with allure.step("1、设置看门狗level等于3，复位 # 1、设置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("level3","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、上位机发送死循环命令 # 2、发送成功"):
        assert host.check_watchdog_error_status() == False
        try:
            t = api.ehsm_test_jump_to_loop()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("等待下位机超时中，请打开串口观察")
            else:
                log.debug(f"错误码{e.ret_code}")
        time.sleep(30)
    with allure.step("3、上位机端持续读取watch dog error 位，检测到返回1 # 3、读取成功，数据正确"):
        assert 1 == watch_dog_error_check()

@allure.feature("baud")
@allure.description("验证Bootloader的watchdog开关3")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1262")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WATCHDOG_CTRL == 0,reason="OTP看门狗控制功能不支持")
def test_ehsm_1262(setup_function):
    with allure.step("1、设置看门狗关闭状态，关闭看门狗 # 1、设置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("disable","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、上位机发送死循环命令 # 2、发送成功"):
        assert host.check_watchdog_error_status() == False
        try:
            t = api.ehsm_test_jump_to_loop()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("等待下位机超时中，请打开串口观察")
            else:
                log.debug(f"错误码{e.ret_code}")
        time.sleep(30)
    with allure.step("3、上位机端读取watch dog error 位 # 3、读取成功，数据正确"):
        assert 0 == watch_dog_error_check()

@allure.feature("baud")
@allure.description("验证Bootloader的watchdog配置level=4后是否能在配置值生效")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1263")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WATCHDOG_CTRL == 0,reason="OTP看门狗控制功能不支持")
def test_ehsm_1263(setup_function):
    with allure.step("1、设置看门狗level等于4，复位 # 1、设置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("level4","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、上位机发送死循环命令 # 2、发送成功"):
        assert host.check_watchdog_error_status() == False
        try:
            t = api.ehsm_test_jump_to_loop()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("等待下位机超时中，请打开串口观察")
            else:
                log.debug(f"错误码{e.ret_code}")
        time.sleep(30)
    with allure.step("3、上位机端持续读取watch dog error 位，检测到返回1 # 3、读取成功，数据正确"):
        assert 1 == watch_dog_error_check()

@allure.feature("baud")
@allure.description("验证Bootloader的log开关功能，正常打开关闭")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-922")
@pytest.mark.skipif( cfg_data.TEST_BL_SET_BAUD_SUPPORT == 0,reason="BAUD 功能不支持")
def test_ehsm_922(setup_module):
    with allure.step("1、配置生命周期为TEST模式，开启OTP LOG功能；并重启生效；#1、 设置成功，重启成功，配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_log("enable", "test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、打开串口工具：设置端口号为COMx USB Serial Port 波特率为115200 # 2、设置成功"):
        pass
    with allure.step("3、检查对应寄存器配置值是否和预期一致；# 3、配置一致"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/115200)
    with allure.step("4、观察串口日志 # 4、执行成功"):
        pass
    with allure.step("5、配置生命周期为TEST模式，关闭OTP LOG功能；并重启生效； # 5、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_log("disable","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("6、检查对应寄存器配置值是否和预期不一致 # 6、配置不一致"):
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret2 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret2,"little") != int(cfg_data.TEST_CPU_FREQ_HZ/115200)

@allure.feature("baud")
@allure.description("验证Bootloader的watchdog开关和等级配置功能，正常打开关闭、并配置级别")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-923")
@pytest.mark.skipif( cfg_data.TEST_BL_OTP_WATCHDOG_CTRL == 0,reason="OTP看门狗控制功能不支持")
def test_ehsm_923(setup_module):
    with allure.step("1、配置OTP生命周期为TEST模式，关闭Watchdog功能；并重启生效；上位机置断点 # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("disable","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、读取对应寄存器值与预期进行比较 # 2、读取成功，数据正确"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)

    with allure.step("3、配置OTP生命周期为TEST模式，设置Watchdog level 为1；并重启生效；上位机置断点 #3、 配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("level1","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("4、读取对应寄存器值与预期进行比较 # 4、读取成功，数据正确"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)

    with allure.step("5、配置OTP生命周期为TEST模式，设置Watchdog level 为15；并重启生效；上位机置断点 # 5、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_wdt("level15","test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)

    with allure.step("6、读取对应寄存器值与预期进行比较 # 6、读取成功，数据正确"):
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
