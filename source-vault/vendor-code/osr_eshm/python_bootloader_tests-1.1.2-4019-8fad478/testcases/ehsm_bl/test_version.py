import logging
import struct
import pytest
import allure
from platform_adapter.api.constants import EhsmDrvMode
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_reg
from platform_adapter.uart_lib.hostapi import HSM_ERR_FW1, OTP_BASE
from utils import key, otp
from utils.config import cfg_data

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    logging.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc})

def _get_otp_bin_uid(lc: str,id:str) -> bytes:
    return otp.otp_to_bin({"lifecycle": lc , "uid":id})

def version_print(version: bytes) -> None:
    if len(version) != 128:
        raise ValueError(f"expect 128 bytes, got {len(version)}")

    (
        t, vmaj, vmin, vpat,
        pre, r0,
        pke_e, pke_l,
        ske_e, ske_l,
        hash_e, hash_l,
        trng_e, trng_l,
        hw,
        uid_raw,
        r1,
    ) = struct.unpack('< B B B B 8s 8s I I I I I I I I I 16s 56s', version)

    pre_str = pre.split(b'\x00', 1)[0].decode(errors='replace')

    logging.debug(f"type               : {t}")
    logging.debug(f"major              : {vmaj}")
    logging.debug(f"minor              : {vmin}")
    logging.debug(f"patch              : {vpat}")
    logging.debug(f"pre_release        : {pre_str}")
    logging.debug(f"reserved0          : {r0.hex()}")
    logging.debug(f"pke_engine_ver     : 0x{pke_e:08x}")
    logging.debug(f"pke_lib_ver        : 0x{pke_l:08x}")
    logging.debug(f"ske_engine_ver     : 0x{ske_e:08x}")
    logging.debug(f"ske_lib_ver        : 0x{ske_l:08x}")
    logging.debug(f"hash_engine_ver    : 0x{hash_e:08x}")
    logging.debug(f"hash_lib_ver       : 0x{hash_l:08x}")
    logging.debug(f"trng_engine_ver    : 0x{trng_e:08x}")
    logging.debug(f"trng_lib_ver       : 0x{trng_l:08x}")
    logging.debug(f"hw_ver             : 0x{hw:08x}")
    logging.debug(f"uid                : {uid_raw.hex()}")
    logging.debug(f"reserved1          : {r1.hex()}")

@allure.feature("version")
@allure.description("在DEV模式下，获取版本信息")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1005")
def test_ehsm_1005(setup_module):
    with allure.step("1、配置OTP 生命周期DEV模式，默认OTP密钥配置； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功，版本信息如下："):
        version_print(version)

@allure.feature("version")
@allure.description("在MANU模式下，获取版本信息")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1006")
def test_ehsm_1006(setup_module):
    with allure.step("1、配置OTP 生命周期MANU模式，默认OTP密钥配置； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 打印成功，版本信息如下："):
        version_print(version)

@allure.feature("version")
@allure.description("在DEBUG模式下，获取版本信息")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1007")
def test_ehsm_1007(setup_module):
    with allure.step("1、配置OTP 生命周期DEBUG模式，默认OTP密钥配置； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功，版本信息如下："):
        version_print(version)

@allure.feature("version")
@allure.description("验证在 TEST 模式，UID CRC 错误的情况下，检查bl能够正常启动，没有报错信息，且响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1305")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1305(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC错误； # 1、配置成功"):
        assert 0 == host.write_otp(otp.otp_to_bin(values_config={"lifecycle": "test"}, layout_config={"uid": {"add_crc32": False}}))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("验证在 DEV 模式，UID CRC 错误的情况下，检查bl能够正常启动，发出告警信息，正常响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1306")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1306(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC错误； # 1、配置成功"):
        assert 0 == host.write_otp(otp.otp_to_bin(values_config={"lifecycle": "dev"}, layout_config={"uid": {"add_crc32": False}}))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0x40000000
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("验证在 MANU 模式，UID CRC 错误的情况下，检查bl能够正常启动，发出告警信息，正常响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1307")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1307(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC错误； # 1、配置成功"):
        assert 0 == host.write_otp(otp.otp_to_bin(values_config={"lifecycle": "manu"}, layout_config={"uid": {"add_crc32": False}}))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0x40000000
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("验证在 USER 模式，UID CRC 错误的情况下，检查bl能够正常启动，发出告警信息，正常响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1308")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1308(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC错误； # 1、配置成功"):
        assert 0 == host.write_otp(otp.otp_to_bin(values_config={"lifecycle": "user"}, layout_config={"uid": {"add_crc32": False}}))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0x40000000
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("验证在 DEBUG 模式，UID CRC 错误的情况下，检查bl能够正常启动，发出告警信息，正常响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1309")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1309(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC错误； # 1、配置成功"):
        assert 0 == host.write_otp(otp.otp_to_bin(values_config={"lifecycle": "debug"}, layout_config={"uid": {"add_crc32": False}}))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0x40000000
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("验证在 USER 模式，UID CRC 正确的情况下，检查bl能够正常启动，没有告警信息，正常响应指令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1310")
@pytest.mark.skipif( cfg_data.TEST_BL_UID_CRC == 0,reason="UID CRC 功能不支持")
def test_ehsm_1310(setup_module):
    with allure.step("1、配置OTP生命周期，和UID及CRC正确； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
    with allure.step("2、重启eHSM，并检查生命周期 # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、检查报错信息 # 3、检查通过"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") & 0x40000000 == 0x00000000
    with allure.step("4、获取version信息，并打印 # 4、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
        version_print(version)

@allure.feature("version")
@allure.description("在Test模式下，获取版本信息")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-147")
def test_ehsm_147(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，默认OTP密钥配置； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功"):
        version_print(version)

@allure.feature("version")
@allure.description("在User模式下，获取版本信息")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-148")
def test_ehsm_148(setup_module):
    with allure.step("1、配置OTP 生命周期User模式，默认OTP密钥配置； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功"):
        version_print(version)

@allure.feature("version")
@allure.description("验证在空片情况上电运行能力，可以获取版本信息（仅配置生命周期）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-962")
def test_ehsm_962(setup_module):
    with allure.step("1、配置OTP生命周期为TEST，其它位为默认状态，根据默认配置可为0或1； # 1、配置成功"):
        otp_data = bytearray(1024)
        erase_otp_value = 0x00
        write_size = 1024
        otp_data[:write_size] = bytes([erase_otp_value]) * write_size
        assert 0 == host.write_memory(OTP_BASE,otp_data)
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功"):
        version_print(version)

@allure.feature("version")
@allure.description("验证Bootloader对硬件版本号的检查，Bootloader软件与硬件版本不一致时会报错误")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-963")
def test_ehsm_963(setup_module):
    with allure.step("1、配置OTP为默认状态，根据默认配置可为0或1； # 1、配置成功"):
        otp_data = bytearray(1024)
        erase_otp_value = 0xFF
        write_size = 1024
        otp_data[:write_size] = bytes([erase_otp_value]) * write_size
        assert 0 == host.write_memory(OTP_BASE,otp_data)
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
    with allure.step("3、将version数据传入获取版本信息接口，并发送 # 3、读取成功，数据正确"):
        t, version  = api.ehsm_bl_get_version()
    with allure.step("4、打印version信息 # 4、打印成功"):
        version_print(version)
    with allure.step("5、读取soc报警信号 # 5、读取成功，数据正确"):
        t, ret = host.read_memory(HSM_ERR_FW1,4)
        assert int.from_bytes(ret,"little") == 0x00000000


@allure.feature("version")
@allure.description("在Test模式下，测试get_version接口ver_addr参数为NULL地址的异常处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_v001")
@pytest.mark.skipif(False, reason="get_version功能始终支持")
def test_ehsm_v001():
    from platform_adapter.uart_lib import ehsm_bl_errno, hostapi

    with allure.step("1、配置OTP生命周期Test模式；并重启生效； # 1、生命周期配置成功；"):
        # Reason: 配置Test模式的OTP环境，用于测试get_version接口
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        logging.info("eHSM重启成功，启动状态正常")

    with allure.step("2、调用get_version接口，传入ver_addr=NULL(0x0)； # 2、报无效地址错误；"):
        # Reason: ver_addr=NULL(0x0)不是有效的输出地址，应该被固件拒绝
        try:
            # 直接调用底层hostapi接口，传入NULL地址
            from platform_adapter.api.uart_impl import UartApi
            uart_api = UartApi()
            t, ret = hostapi.ehsm_bl_get_version(uart_api.CTX_ADDR, 0x0)
            assert False, f"ver_addr=NULL时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            logging.info(f"✓ 捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS, \
                f"预期错误码为EHSM_ERR_INVALID_ADDRESS({ehsm_bl_errno.EHSM_ERR_INVALID_ADDRESS})，实际: {e.ret_code}"

