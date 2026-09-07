import logging
import time
import allure
import pytest
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

def _get_otp_bin( lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc})

def read_mem_ecc_iram_in()->int:
    t, reg = host.read_memory(hostapi.HSM_ERR_HW0,4)
    iram_a = int.from_bytes(reg,"little")
    if((0x01 & iram_a>>13) == 1):
        ret = 1
    else:
        ret = 0

    return ret

OTP_SENSOR_RSP_CTRL = hostapi.OTP_BASE + 0x30

def common_delay_ms(ms_value):
    cur_time = time.time()
    new_time = cur_time
    while (new_time - cur_time < ms_value):
        new_time = time.time()

def check_soc_reset()-> int:
    t, ret = host.read_memory(0x40010060,4)
    reg = int.from_bytes(ret,"little")
    if reg&(1<<18) != (1<<18):
        return 1
    return 0


@pytest.fixture(scope="module")
def setup_module():
    logging.debug("Setting up module for tests")

@pytest.fixture(scope="function")
def setup_function():
    host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')
    yield host
    host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

def read_memory_in_loop()->int:
    t, status = host.read_memory(hostapi.HSM_STATUS_IN, 4)
    return int.from_bytes(status,"little")

def read_otp_hsm_status_in(over_time_ms: int)->int:
    start_time = time.time()
    while((0x01 & (read_memory_in_loop() >> 18)) == 0):
        if (over_time_ms != 0):
            now_time = time.time()
            if ((now_time - start_time) > over_time_ms) :
                ret = 2
                return ret
    ret = 1
    return ret

@allure.feature("sensor")
@allure.description("验证TEST模式下，当err rsp ctrl配置了SOC reset时，不会触发sensor错误，上位机的 o_hsm_status的bit18（soc_reset) 不会被置1，BL可以继续处理命令")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1158")
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1158(setup_module, setup_function):
    with allure.step("1、配置OTP模式为TEST模式，并重启下位机ehsm #1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin( "test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、ehsm_write_otp_data_ex配置了SOC reset # 2、配置成功"):
        sensor_o = [ 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ]

        sensor_in = 0x00000001
        sensor_inject = sensor_in.to_bytes(4,"little")
        sensor_otp = bytes(sensor_o)
        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、触发映射的SOC_SENSOR_OUT 寄存器，配置0x00008000 # 3、配置成功"):
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
    with allure.step("4、检查上位机o_hsm_statusbit18  SYS_HSM_STA1[18] 5秒之内是否为1，5秒之内都为0时跳出循环； # 4、检查通过"):
        assert 2 == read_otp_hsm_status_in(5)
    with allure.step("5、发送命令（设置baud），查看baud是否设置成功 # 5、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)

        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证USER/DEBUG/DEV/MANU模式下，当err rsp ctrl配置了0时， 触发sensor错误，上位机的 o_hsm_status的bit18（soc_reset)不会被置1，BL正常处理命令")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1166")
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1166(setup_module, setup_function):
    with allure.step("1、配置OTP模式为DEV模式， sensor rsp ctrl 配置为0，并重启下位机ehsm # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、ehsm_write_otp_data_ex配置了SOC reset=0 # 2、配置成功"):
        sensor_o = [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ]

        sensor_in = 0x00008000
        sensor_inject = sensor_in.to_bytes(4,"little")
        sensor_otp = bytes(sensor_o)

        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、触发映射的SOC_SENSOR_OUT 寄存器，配置0x00008000； # 3、配置成功"):
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
    with allure.step("4、检查上位机o_hsm_statusbit18  SYS_HSM_STA1[18] 5秒之内都为0； # 4、检查通过"):
       assert 2 == read_otp_hsm_status_in(5)
    with allure.step("5、发送命令（设置baud），查看baud是否设置成功 # 5、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')
    with allure.step("6、配置OTP模式为MANU模式，重复步骤2-5 # 6、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin( "manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
        assert 2 == read_otp_hsm_status_in(5)
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')
    with allure.step("7、配置OTP模式为USER模式，重复步骤2-5 # 7、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
        assert 2 == read_otp_hsm_status_in(5)
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')
    with allure.step("8、配置OTP模式为DEBUG模式，重复步骤2-5 # 8、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin( "debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
        assert 2 == read_otp_hsm_status_in(5)
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        if cfg_data.TEST_CUSTOM_ID != 0xA002:
            t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
            assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)

        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')


@allure.feature("sensor")
@allure.description("验证USER模式下，当去读取一个未初始化的IRAM时，上位机会读到 iram_ecc错误，但BL会被Hold住")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1174")
@pytest.mark.manual
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1174(setup_module, setup_function):
    with allure.step("1、配置OTP模式为USER模式,并重启下位机 # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、读取IRAM寄存器，IRAM未初始化的位置：0x10880000 # 2、读取成功，数据正确"):
        t, iram_in = api.ehsm_test_read_memory(0x10880000, 4)
        iram_ecc = int.from_bytes(iram_in,"little")
    with allure.step("3、上位机检测是否能读取到报错：MEM_ECC_MB_IRAM，寄存器位置：HSM_ERR_HW_IN0 >> 13 # 3、读取成功，数据正确"):
        assert 1 == read_mem_ecc_iram_in()
    with allure.step("4、发送BL设置波特率命令验证是否程序被卡住 # 4、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)

        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证DEBUG模式下，当去读取一个未初始化的IRAM时，上位机会读到 iram_ecc错误，但BL会被Hold住")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1175")
@pytest.mark.manual
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1175(setup_module, setup_function):
    with allure.step("1、配置OTP模式为DEBUG模式,并重启下位机 # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、读取IRAM寄存器，IRAM未初始化的位置：0x10880000； # 2、读取成功，数据正确"):
        t, iram_in = api.ehsm_test_read_memory(0x10880000, 4)
        iram_ecc = int.from_bytes(iram_in,"little")
    with allure.step("3、上位机检测是否能读取到报错：MEM_ECC_MB_IRAM，寄存器位置：HSM_ERR_HW_IN0 >> 13 # 3、读取成功，数据正确"):
        assert 1 == read_mem_ecc_iram_in()
    with allure.step("4、发送BL设置波特率命令验证是否程序被卡住 # 4、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证DEV模式下，当去读取一个未初始化的IRAM时，上位机会读到 iram_ecc错误，但BL会被Hold住")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1176")
@pytest.mark.manual
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1176(setup_module, setup_function):
    with allure.step("1、/配置OTP模式为DEV模式,并重启下位机 # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、读取IRAM寄存器，IRAM未初始化的位置：0x10880000 # 2、读取成功，数据正确"):
        t, iram_in = api.ehsm_test_read_memory(0x10880000, 4)
        iram_ecc = int.from_bytes(iram_in,"little")
    with allure.step("3、上位机检测是否能读取到报错：MEM_ECC_MB_IRAM，寄存器位置：HSM_ERR_HW_IN0 >> 13 # 3、读取成功，数据正确"):
        assert 1 == read_mem_ecc_iram_in()
    with allure.step("4、发送BL设置波特率命令验证是否程序被卡住 # 4、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.TEST_CPU_FREQ_HZ/38400))
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.TEST_CPU_FREQ_HZ/38400)
        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证MANU模式下，当去读取一个未初始化的IRAM时，上位机会读到 iram_ecc错误，但BL会被Hold住")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1177")
@pytest.mark.manual
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1177(setup_module, setup_function):
    with allure.step("1、配置OTP模式为MANU模式,并重启下位机 # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、读取IRAM寄存器，IRAM未初始化的位置：0x10880000 # 2、读取成功，数据正确"):
        t, iram_in = api.ehsm_test_read_memory(0x10880000, 4)
    with allure.step("3、上位机检测是否能读取到报错：MEM_ECC_MB_IRAM，寄存器位置：HSM_ERR_HW_IN0 >> 13 # 3、读取成功，数据正确"):
        t, iram_a = api.ehsm_test_read_memory(ehsm_reg.HSM_ERR_HW_IN0, 4)
        a = int.from_bytes(iram_a,"little")
        assert 1 == read_mem_ecc_iram_in()
    with allure.step("4、发送BL设置波特率命令验证是否程序被卡住 # 4、发送成功"):
        t = api.ehsm_bl_set_uart_baudrate(int(cfg_data.CONFIG_TEST_CLK_FREQ/38400))
        t , ret1 = api.ehsm_test_read_memory(ehsm_reg.UART_BAUD_DIV, 4)
        assert int.from_bytes(ret1,"little") == int(cfg_data.CONFIG_TEST_CLK_FREQ/38400)
        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证 TEST 模式下 sensor 状态寄存器信号控制复位 ehsm功能，下位机手动检查ehsm复位 ehsm操作")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1703")
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_1703(setup_module, setup_function):
    with allure.step("1、配置OTP sensor rsp ctrl 位置的一个控制域，并重启下位机ehsm # 1、配置成功"):
        sensor_o = [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ]

        sensor_in = 0x00008000
        sensor_inject = sensor_in.to_bytes(4,"little")
        sensor_otp = bytes(sensor_o)

        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、触发映射的SOC_SENSOR_OUT 寄存器，配置0x00008000 # 2、配置成功"):
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
    with allure.step("3、手动断点进入下位机，检查下位机是否调用到secboot_emu_err_handler 函数中的sysreg_hsm_reset接口 # 3、检查通过"):
        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')


@allure.feature("sensor")
@allure.description("验证sensor 状态寄存器信号控制复位 ehsm功能，下位机手动检查ehsm复位 ehsm操作")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-958")
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_958(setup_module, setup_function):
    with allure.step("1、配置OTP sensor rsp ctrl 位置的一个控制域，并重启下位机ehsm # 1、配置成功"):
        sensor_o = [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ]

        sensor_in = 0x00008000
        sensor_inject = sensor_in.to_bytes(4,"little")
        sensor_otp = bytes(sensor_o)

        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、触发映射的SOC_SENSOR_OUT 寄存器，配置0x00008000 # 2、配置成功"):
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, sensor_inject)
    with allure.step("3、手动断点进入下位机，检查下位机是否调用到secboot_emu_err_handler 函数中的sysreg_hsm_reset接口 # 3、检查通过"):
        host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')

@allure.feature("sensor")
@allure.description("验证sensor 状态寄存器信号控制清除所有密钥功能，密钥无法使用报错误")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-959")
@pytest.mark.skipif( cfg_data.TEST_BL_SENSOR_SUPPORT == 0,reason="SENSOR 功能不支持")
def test_ehsm_959(setup_module, setup_function):
    with allure.step("1、配置OTP sensor rsp ctrl 位置的一个控制域，并重启下位机ehsm # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("2、触发映射的SOC_SENSOR_OUT 寄存器，配置0x80000000 # 2、配置成功"):
        sensor_o = [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 ]
        sensor_i = 0x80008000
        sensor_inject = sensor_i.to_bytes(4,"little")
        sensor_otp = bytes(sensor_o)
        t = host.write_memory(OTP_SENSOR_RSP_CTRL, sensor_otp)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、配置校验镜像命令，并配置SOC_SENSOR_OUT 寄存器为0x00；并重启ehsm # 3、配置成功"):
        t = host.write_memory(0x40010054, sensor_inject)
        t = host.write_memory(ehsm_reg.SOC_SENSOR_OUT, b'\x00\x00\x00\x00')
    with allure.step("4、发送进行校验命令，该命令会使用OTP密钥进行镜像校验； # 4、发送成功"):
        # TODO
        pass