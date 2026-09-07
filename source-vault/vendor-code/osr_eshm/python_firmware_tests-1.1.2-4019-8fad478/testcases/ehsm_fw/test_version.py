import logging
import struct
import allure
import pytest
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from platform_adapter.uart_lib import ehsm_fw_errno

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

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

@pytest.fixture(scope="module")
def setup_module():
    logging.debug("Setting up module for tests")

@allure.feature("version")
@allure.description("读取 firmware version 信息，并打印出来")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-847")
def test_ehsm_847(setup_module):
    with allure.step("1、发送 read_ehsm_version mailbox 指令；并打印状态值； # 1、发送成功"):
        ver = b'\x00'
        t , version = api.ehsm_get_version(ver)
        version_print(version)

@allure.feature("version")
@allure.description("对 read_version 命令进行异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-848")
def test_ehsm_848(setup_module):
    with allure.step("1、发送 read_ehsm_version mailbox 指令，ver_addr 传入 NULL； # 1、接口返回对应错误码；"):
        try:
            t , version = api.ehsm_get_version(None)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                logging.info(f"read version命令传入异常参数时地址非法，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"read version命令传入异常参数时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"read version命令传入异常参数时未抛出异常，不符合预期"
    with allure.step("2、发送 read_ehsm_version mailbox 指令，ver_size 传入 0； # 接口返回对应错误码；"):
        pass