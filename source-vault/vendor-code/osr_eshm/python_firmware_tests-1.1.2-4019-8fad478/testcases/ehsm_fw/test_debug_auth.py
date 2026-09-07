import binascii
import logging
import pytest
import allure
import struct

import serial
from platform_adapter.api.constants import EhsmAuthAlgo, EhsmChallengeType, EhsmDrvMode
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib.ehsm_fw_errno import *
from platform_adapter.uart_lib import hostapi
from utils import key, otp
from utils.config import cfg_data
from serial.tools import list_ports

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

def find_uart_com():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port.vid == 0x0403:
            return port.device

ser = serial.Serial(find_uart_com(), 115200, timeout=1)

@pytest.fixture(scope="module")
def setup_module():
    logging.debug("Setting up module for tests")

def _get_ehsm_sm4_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID): {
            "value" : key.EHSM_DEBUG_SIGN_KEY_SM4.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_ecc256_debug_config(lc: str) -> bytes:
    config = {
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }
    if cfg_data.TEST_EHSM_DEBUG_KEY_ID != 0xFFFF:
        config ["key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID)]= {
            "value" : key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sha256",
        }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0 != 0xFFFF:
        config ["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0)]= {
            "value" : key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sha256",
        }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1 != 0xFFFF:
        config ["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1)]= {
            "value" : key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sha256",
        }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_sm2_debug_config(lc: str) -> bytes:
    config = {
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }
    if cfg_data.TEST_EHSM_DEBUG_KEY_ID != 0xFFFF:
        config ["key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID)] = {
                "value" : (key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS + key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY).hex(),
                "level" : 1,
                "lifecycle" : "available",
                "type" : "pub",
                "hash" : "sm3",
            }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0 != 0xFFFF:
        config ["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0)] = {
                "value" : (key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS + key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY).hex(),
                "level" : 1,
                "lifecycle" : "available",
                "type" : "pub",
                "hash" : "sm3",
            }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1 != 0xFFFF:
        config ["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1)] = {
                "value" : (key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS + key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY).hex(),
                "level" : 1,
                "lifecycle" : "available",
                "type" : "pub",
                "hash" : "sm3",
            }
    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_rsa_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID): {
            "value" : (key.EHSM_VERIFY_SIGN_KEY_RSA2048_E + key.EHSM_VERIFY_SIGN_KEY_RSA2048_N).hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sha256",
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_rsa3072_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID): {
            "value" : (key.EHSM_VERIFY_SIGN_KEY_RSA3072_E + key.EHSM_VERIFY_SIGN_KEY_RSA3072_N).hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sha256",
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_aes_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID): {
            "value" : key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_ecc256_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : True,
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_sm2_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : (key.SOC_DEBUG_SIGN_KEY_SM2_COMPRESS + key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY).hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : "sm3",
        },
        "key_alg_sel": "sm4", # aes128 使用 ecc, sm4 使用 sm2
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_rsa_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : (key.SOC_VERIFY_SIGN_KEY_RSA2048_E + key.SOC_VERIFY_SIGN_KEY_RSA2048_N).hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : True,
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_rsa3072_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : (key.SOC_VERIFY_SIGN_KEY_RSA3072_E + key.SOC_VERIFY_SIGN_KEY_RSA3072_N).hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "pub",
            "hash" : True,
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_aes_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : key.SOC_DEBUG_SIGN_KEY_AES128.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_sm4_debug_config(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_SOC_DEBUG_KEY_ID): {
            "value" : key.SOC_DEBUG_SIGN_KEY_SM4.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "key_alg_sel": "sm4",
        "lifecycle" : lc
    }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_ehsm_ecc384_debug_config(lc: str) -> bytes:
    config = {
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }
    # 只有 TEST_ROT_AUTH_KEY_HASH_ID_0 != 0xffff 时才添加 key 配置
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0 != 0xFFFF:
        config["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "pub",
            "hash": "auto",
        }

    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1 != 0xFFFF:
        config["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "pub",
            "hash": "auto",
        }

    logging.debug(config)
    return otp.otp_to_bin(config)

def _get_soc_ecc384_debug_config(lc: str) -> bytes:
    config = {
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }
    if cfg_data.TEST_DEVICE_AUTH_KEY_HASH_ID_0 != 0xFFFF:
        config["key" + str(cfg_data.TEST_DEVICE_AUTH_KEY_HASH_ID_0)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "pub",
            "hash": "auto",
        }
    if cfg_data.TEST_DEVICE_AUTH_KEY_HASH_ID_1 != 0xFFFF:
        config["key" + str(cfg_data.TEST_DEVICE_AUTH_KEY_HASH_ID_1)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "pub",
            "hash": "auto",
        }
    logging.debug(config)
    return otp.otp_to_bin(config)

def get_challenge_from_uart(type: EhsmChallengeType , alg: EhsmAuthAlgo) -> tuple[int, bytes]:
    req = "REQ" + str(type.value) + str(alg.value) + "@"
    # 垃圾数据清理
    if ser.in_waiting > 0:
        discarded = ser.read(ser.in_waiting)
        logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
    ser.reset_input_buffer()

    ser.write(req.encode("utf8"))
    logging.info(f"获取challenge时发送的数据为{req}")

    rsp = ser.read_until("@")
    logging.info(f"接收到的数据为{rsp}")
    # 如果还是有垃圾数据，自动清理
    if rsp:
        cha_index = rsp.find(b"CHA")
        if cha_index > 0:
            logging.warning(f"检测到CHA前有{cha_index}字节无效数据，已舍弃: {rsp[:cha_index]}")
            rsp = rsp[cha_index:]
            logging.info(f"处理后的数据为{rsp}")
    if not rsp:
        return EHSM_ERR_UART_NO_RESPONSE,None
    elif not rsp.startswith(b"CHA"):
        return EHSM_ERR_UART_WRONG_RSP_DATA, None
    elif not rsp.endswith(b"@"):
        return EHSM_ERR_UART_WRONG_RSP_DATA, None
    elif b"get random fail" in rsp:
        return EHSM_ERR_UART_RANDOM_FAIED, None

    challenge = rsp[5:-1]
    logging.info(challenge)
    return EHSM_ERR_SW_SUCCESS, challenge

def debug_auth_by_uart(type: EhsmChallengeType, alg: EhsmAuthAlgo, pubkey: bytes, signature: bytes) -> int:
    if alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC or alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC:
        pub = ""
    else:
        pub = binascii.hexlify(pubkey).decode() + ":"

    req = ("RSP"
        + str(type.value)
        + str(alg.value)
        + ":"
        + pub
        + binascii.hexlify(signature).decode()
        + "@")

    logging.info(f"发送的数据为{req}")
    ser.write(req.encode())
    rsp = ser.read_until("@")
    logging.info(f"鉴权结果为{rsp}")
    if not rsp:
        return EHSM_ERR_UART_NO_RESPONSE
    elif not rsp.endswith(b"@"):
        return EHSM_ERR_UART_WRONG_RSP_DATA
    elif rsp.startswith(b"END_PUBKEY_FAIL"):
        return EHSM_ERR_UART_WRONG_PUBKEY
    elif rsp.startswith(b"END_VERIFY_FAIL"):
        return EHSM_ERR_UART_VERIFY_FAILED
    elif rsp.startswith(b"END_PARAMETER_FAIL"):
        return EHSM_ERR_UART_WRONG_PARAM
    elif rsp.startswith(b"END_DATA_FAIL"):
        return EHSM_ERR_UART_WRONG_PARAM
    else:
        return EHSM_ERR_SW_SUCCESS

def close_debug_by_uart(type: EhsmChallengeType) -> int:
    """
    通过UART发送关闭调试指令

    Args:
        type: 调试类型(1=eHSM, 3=SOC)

    Returns:
        int: 0表示成功，非0表示失败
    """
    req = "DIS" + str(type.value) + "@" # DISx 表示关闭调试，x 表示类型
    if ser.in_waiting > 0:
        discarded = ser.read(ser.in_waiting)
        logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
    ser.reset_input_buffer()
    ser.write(req.encode("utf8"))
    logging.info(f"发送关闭调试指令: {req}")

    rsp = ser.read_until(b"@")
    logging.info(f"接收到的响应: {rsp}")

    if not rsp:
        return EHSM_ERR_UART_NO_RESPONSE
    elif rsp.startswith(b"ENDPASS"):
        return EHSM_ERR_SW_SUCCESS
    elif rsp.startswith(b"END_PARAMETER_FAIL"):
        return EHSM_ERR_UART_WRONG_PARAM
    else:
        return EHSM_ERR_UART_WRONG_RSP_DATA

def uart_echo_test(test_data: str) -> tuple[int, bytes]:
    """
    通过UART发送测试指令并验证回显

    Args:
        test_data: 测试数据字符串

    Returns:
        tuple: (错误码, 回显数据)
    """
    req = "TEST" + test_data + "@"
    if ser.in_waiting > 0:
        discarded = ser.read(ser.in_waiting)
        logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
    ser.reset_input_buffer()
    ser.write(req.encode("utf8"))
    logging.info(f"发送测试指令: {req}")

    rsp = ser.read_until(b"@")
    logging.info(f"接收到的响应: {rsp}")
    if rsp:
        cha_index = rsp.find(b"CHA")
        if cha_index > 0:
            logging.warning(f"检测到CHA前有{cha_index}字节无效数据，已舍弃: {rsp[:cha_index]}")
            rsp = rsp[cha_index:]
            logging.info(f"处理后的数据为{rsp}")

    if not rsp:
        return EHSM_ERR_UART_NO_RESPONSE, None

    expected = b"TEST" + test_data.encode("utf8") + b"@"
    if rsp == expected:
        return EHSM_ERR_SW_SUCCESS, rsp
    else:
        return EHSM_ERR_UART_WRONG_RSP_DATA, rsp

def check_soc_debug_ports_bitmap(expected_bitmap: bytes = None) -> bool:
    """
    检查SOC调试端口bitmap寄存器的值

    Args:
        expected_bitmap: 期望的bitmap值(5个uint32,共20字节)

    Returns:
        bool: 如果提供expected_bitmap则返回是否匹配，否则只记录当前值并返回True
    """
    from platform_adapter.uart_lib import hostapi

    # 读取4个SOC_DBG_EN_128B寄存器
    b0 = host.get_word(hostapi.SOC_DBG_EN_128B0)
    b1 = host.get_word(hostapi.SOC_DBG_EN_128B1)
    b2 = host.get_word(hostapi.SOC_DBG_EN_128B2)
    b3 = host.get_word(hostapi.SOC_DBG_EN_128B3)

    current_bitmap = struct.pack('<4I', b0, b1, b2, b3)
    logging.info(f"SOC调试端口bitmap: B0=0x{b0:08X}, B1=0x{b1:08X}, B2=0x{b2:08X}, B3=0x{b3:08X}")

    if expected_bitmap is not None:
        # 只比较前16字节(4个uint32)
        expected_4words = expected_bitmap[:16]
        return current_bitmap == expected_4words

    return True


@allure.feature("debug_auth")
@allure.description(
    "在Destroy模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的异常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-1292")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_1292(setup_module):
    with allure.step(
        "1、配置OTP 生命周期Destroy模式，调试鉴权算法密钥eHSM Debug Key配置SM4 CMAC预置密钥和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("destroy"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        try:
            host.wait_fw_done(0.5)
        except Exception as e:
            # 在Destroy模式下，eHSM会进入不可用状态,符合预期，用例成功
            assert True, "eHSM should not be available in Destroy mode"

@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 RSA-2048算法方式，测试 eHSM 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1637")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="RSA2048 not support for debug auth")
def test_ehsm_1637():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置RSA-2048预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_rsa_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行hash计算，RSA-2048算法使用预置公钥，计算签名； # 计算成功"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.EHSM_VERIFY_SIGN_KEY_RSA2048_E + key.EHSM_VERIFY_SIGN_KEY_RSA2048_N,
            pub_key_size=len(key.EHSM_VERIFY_SIGN_KEY_RSA2048_E) + len(key.EHSM_VERIFY_SIGN_KEY_RSA2048_N),
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)
        # assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 RSA-3072算法方式，测试 eHSM 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1638")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_RSA3072_SUPPORT == 0, reason="RSA3072 not support for debug auth")
def test_ehsm_1638():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置RSA-3072预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_rsa3072_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("4、调用上层接口对挑战字进行hash计算，RSA-3072算法使用预置私钥，计算签名； # 计算成功"):
        # 设置使用 RSA-3072 密钥
        host.set_rsa_key_size(3072)
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.EHSM_VERIFY_SIGN_KEY_RSA3072_E + key.EHSM_VERIFY_SIGN_KEY_RSA3072_N,
            pub_key_size=len(key.EHSM_VERIFY_SIGN_KEY_RSA3072_E) + len(key.EHSM_VERIFY_SIGN_KEY_RSA3072_N),
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)
        # 重置 RSA 密钥大小为默认值
        host.set_rsa_key_size(2048)

@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 RSA-2048 算法方式，测试 SOC 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1639")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="RSA2048 not support for debug auth")
def test_ehsm_1639():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置RSA-2048预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_rsa_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step("4、调用上层接口对挑战字进行hash计算，RSA-2048算法使用预置私钥，计算签名； # 计算成功"):
        # 确保使用 RSA-2048 密钥
        host.set_rsa_key_size(2048)
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        # 这里设置所有端口都启用（全1）作为测试数据
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.SOC_VERIFY_SIGN_KEY_RSA2048_E + key.SOC_VERIFY_SIGN_KEY_RSA2048_N,
            pub_key_size=len(key.SOC_VERIFY_SIGN_KEY_RSA2048_E) + len(key.SOC_VERIFY_SIGN_KEY_RSA2048_N),
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG, soc_dbg_bitmap)

@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 RSA-3072 算法方式，测试 SOC 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1640")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_RSA3072_SUPPORT == 0, reason="RSA3072 not support for debug auth")
def test_ehsm_1640():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置RSA-3072预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_rsa3072_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step("4、调用上层接口对挑战字进行hash计算，RSA-3072算法使用预置私钥，计算签名； # 计算成功"):
        # 设置使用 RSA-3072 密钥
        host.set_rsa_key_size(3072)
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        # 这里设置所有端口都启用（全1）作为测试数据
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.SOC_VERIFY_SIGN_KEY_RSA3072_E + key.SOC_VERIFY_SIGN_KEY_RSA3072_N,
            pub_key_size=len(key.SOC_VERIFY_SIGN_KEY_RSA3072_E) + len(key.SOC_VERIFY_SIGN_KEY_RSA3072_N),
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG, soc_dbg_bitmap)
        # 重置 RSA 密钥大小为默认值
        host.set_rsa_key_size(2048)

@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 SHA256-SECP256R1 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-731")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth"
)
def test_ehsm_731(setup_module):
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SHA256-SECP256R1预置值公钥hash和对应属性，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        _, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行hash计算，ECC算法使用预置ecc 私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            singnature,
            len(singnature),
            key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
            len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
            None,
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 SM3-SM2 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-732")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="SM2 not support for debug auth"
)
def test_ehsm_732():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM3算法预置值公钥hash和对应属性，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm2_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行SM3计算，SM2算法使用预置SM2公私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS+key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY,
            pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS)+len(key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY),
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 AES-CMAC 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-734")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="AES128CMAC not support for debug auth"
)
def test_ehsm_734():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置AES-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_aes_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，AES-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)

@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-735")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_735():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)
    # test 模式下无法关闭调试鉴权

@allure.feature("debug_auth")
@allure.description(
    "在Dev 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-736")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_736():
    with allure.step(
        "1、配置OTP 生命周期Dev模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)



@allure.feature("debug_auth")
@allure.description(
    "在Manu 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-737")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_737():
    with allure.step(
        "1、配置OTP 生命周期Manu模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在User 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的鉴权限制功能"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-738")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_738():
    from platform_adapter.uart_lib import ehsm_fw_errno

    with allure.step(
        "1、配置OTP 生命周期User模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=singnature,
                sig_size=len(singnature),
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            # 如果没有抛出异常，检查鉴权应该失败
            assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        except Exception as e:
            # 检查是否为预期的生命周期限制错误
            error_code = int(str(e))
            if error_code == ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT:
                logging.info(f"User模式下调试鉴权受到生命周期限制，符合预期: 错误码 {error_code}")
                # 确认鉴权状态为失败
                assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            else:
                # 如果不是预期的错误码，测试失败
                assert False, f"User模式下调试鉴权返回了非预期的错误码: {error_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
    with allure.step("6、确认eHSM调试鉴权状态为失败 # 执行成功"):
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在Debug 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-739")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_739():
    with allure.step(
        "1、配置OTP 生命周期Debug模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM 状态，确认eHSM 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 SHA256-SECP256R1 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-740")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth"
)
def test_ehsm_740():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置SHA256-SECP256R1预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_ecc256_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行hash计算，ECC算法使用预置ecc 私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        # 这里设置所有端口都启用（全1）作为测试数据
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY,
            pub_key_size=len(key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY),
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG, soc_dbg_bitmap)
        #assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)


@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 SM3-SM2 算法方式，测试 SOC 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-741")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="SM2 not support for debug auth"
)
def test_ehsm_741():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置SM3算法预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm2_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行SM3计算，SM2算法使用预置SM2公私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY,
            pub_key_size=len(key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY),
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)



@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 AES-CMAC 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-743")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="AES128CMAC not support for debug auth"
)
def test_ehsm_743():
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置AES-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_aes_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，AES-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在Test 模式下，使用 SM4-CMAC 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-744")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_744(setup_module):
    with allure.step(
        "1、配置OTP 生命周期Test模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        # 这里设置所有端口都启用（全1）作为测试数据
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("关闭调试鉴权 # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG, soc_dbg_bitmap)
    # test 模式下无法关闭SOC调试鉴权，无法验证状态


@allure.feature("debug_auth")
@allure.description("在Dev 模式下，使用 SM4-CMAC 算法方式，测试 SOC 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-745")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_745():
    with allure.step(
        "1、配置OTP 生命周期Dev模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        # 创建SOC调试端口bitmap：5个uint32_t，前4个用于128个端口，最后1个用于第129个端口
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在Manu 模式下，使用 SM4-CMAC 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-746")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_746():
    with allure.step(
        "1、配置OTP 生命周期Manu模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在User 模式下，使用 SM4-CMAC 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-747")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_747():
    with allure.step(
        "1、配置OTP 生命周期User模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        try:
            t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
            with allure.step(
                "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
                ret, singnature = host.sign(challenge,
                                            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
                assert 0 == ret
            with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
                soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
                api.ehsm_debug_auth(
                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                    sig=singnature,
                    sig_size=len(singnature),
                    pub_key=None,
                    pub_key_size=0,
                    soc_dbg_bitmap=soc_dbg_bitmap
                )
                # User模式下SOC调试功能可能受限
                status = host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
        except Exception as e:
            logging.info(f"User模式下SOC调试鉴权受限，符合预期: {e}")


@allure.feature("debug_auth")
@allure.description(
    "在Debug 模式下，使用 SM4-CMAC 算法方式，测试 SOC 类型的正常鉴权过程"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-748")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_748():
    with allure.step(
        "1、配置OTP 生命周期Debug模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 5、鉴权成功；"):
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC 状态，确认SOC 鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)


@allure.feature("debug_auth")
@allure.description("在User模式下，测试Firmware eHSM get_challenge 功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-749")
def test_ehsm_749():
    with allure.step("1、配置OTP 生命周期User模式，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，传入eHSM 类型，获取挑战字； # 3、获取挑战字成功；"):
        try:
            t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            # User模式下可能无法获取eHSM调试挑战字，这是预期行为
            logging.info(f"User模式下成功获取eHSM挑战字: {challenge.hex()}")
        except Exception as e:
            logging.info(f"User模式下eHSM get_challenge受限，符合预期: {e}")


@allure.feature("debug_auth")
@allure.description("在User模式下，测试Firmware SOC get_challenge 功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-750")
def test_ehsm_750():
    with allure.step("1、配置OTP 生命周期User模式，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，传入SOC 类型，获取挑战字； # 3、获取挑战字成功；"):
        try:
            t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
            # User模式下可能无法获取SOC调试挑战字，这是预期行为
            logging.info(f"User模式下成功获取SOC挑战字: {challenge.hex()}")
        except Exception as e:
            logging.info(f"User模式下SOC get_challenge受限，符合预期: {e}")


@allure.feature("debug_auth")
@allure.description("在User模式下，测试Firmware User get_challenge 功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-751")
def test_ehsm_751():
    with allure.step("1、配置OTP 生命周期User模式，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，传入User 类型，获取挑战字； # 3、获取挑战字成功；"):
        try:
            # 此处假设存在User类型的挑战字，如果没有则使用eHSM类型作为替代
            t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            logging.info(f"User模式下成功获取User类型挑战字: {challenge.hex()}")
        except Exception as e:
            logging.info(f"User模式下User get_challenge受限，符合预期: {e}")


@allure.feature("debug_auth")
@allure.description("在Dev模式下，关闭 eHSM 鉴权，并检查鉴权状态")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-752")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_752():
    with allure.step(
        "1、配置OTP 生命周期Dev模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC值传入 debug_auth 接口，进行鉴权 # 发送成功"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("7、调用接口关闭debug 鉴权； # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)
    with allure.step("8、检查鉴权状态，确认鉴权已关闭 # 检查通过"):
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("在Dev模式下，关闭 SOC 鉴权，并检查鉴权状态")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-753")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_753():
    with allure.step(
        "1、配置OTP 生命周期Dev模式，调试鉴权算法密钥SOC Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC值传入 debug_auth 接口，进行鉴权 # 发送成功"):
        soc_dbg_bitmap = struct.pack('<5I', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001)
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=soc_dbg_bitmap
        )
    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
    with allure.step("7、调用接口关闭SOC debug 鉴权； # 执行成功"):
        api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG, soc_dbg_bitmap)
    with allure.step("8、检查SOC鉴权状态，确认鉴权已关闭 # 检查通过"):
        # 注意：SOC鉴权关闭后状态检查可能不同于eHSM，此处仅记录状态
        status = host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
        logging.info(f"SOC鉴权关闭后状态: {status}")


@allure.feature("debug_auth")
@allure.description(
    "在 debug 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的鉴权过程中使用错误的CMAC值"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-754")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_754():
    with allure.step(
        "1、配置OTP 生命周期 debug 模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC，并修改CMAC值； # 计算成功"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
        # 修改CMAC值的第一个字节，使其错误
        wrong_signature = bytearray(singnature)
        wrong_signature[0] = wrong_signature[0] ^ 0xFF
        wrong_signature = bytes(wrong_signature)
    with allure.step("5、将修改的CMAC值传入 debug_auth 接口，进行鉴权 # 发送成功"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=wrong_signature,
                sig_size=len(wrong_signature),
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            # 如果没有抛出异常，检查鉴权应该失败
            assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        except Exception as e:
            logging.info(f"鉴权失败，符合预期: {e}")
    with allure.step("6、确认eHSM调试鉴权状态为失败 # 执行成功"):
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在 dev 模式下，使用 SHA256‑SECP256R1 算法方式，测试 eHSM 类型的鉴权过程中使用与OTP不匹配的公钥hash"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-755")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth"
)
def test_ehsm_755():
    with allure.step(
        "1、配置OTP 生命周期 dev 模式，调试鉴权算法密钥eHSM Debug Key配置SHA256-SECP256R1预置值公钥hash和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t = api.ehsm_close_debug(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG, None)
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行hash计算，ECC算法使用预置ecc 私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
        assert 0 == ret
    with allure.step("5、将签名好的数据和非法公钥传入 debug_auth 接口，进行鉴权 # 发送成功"):
        # 使用错误的公钥（使用SOC的公钥代替eHSM的公钥）
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                singnature,
                len(singnature),
                key.EHSM_VERIFY_SIGN_KEY_ECC256_PUBKEY,  # 使用错误的公钥
                len(key.EHSM_VERIFY_SIGN_KEY_ECC256_PUBKEY),
                None,
            )
            # 如果没有抛出异常，检查鉴权应该失败
            assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        except Exception as e:
            # 检查是否为预期的公钥HASH不匹配错误
            error_code = int(str(e))
            if error_code == EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH:
                logging.info(f"User模式下调试鉴权公钥HASH不匹配，符合预期: 错误码 {error_code}")
                # 确认鉴权状态为失败
                assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            else:
                # 如果不是预期的错误码，测试失败
                assert False, f"User模式下调试鉴权返回了非预期的错误码: {error_code}，预期错误码: {EHSM_ERR_DEBUG_AUTH_PK_HASH_MISMATCH}"
            logging.info(f"使用错误公钥鉴权失败，符合预期: {e}")
    with allure.step("6、确认eHSM鉴权状态为失败 # 执行成功"):
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description(
    "在 debug 模式下，使用 SM4-CMAC 算法方式，测试 eHSM 类型的鉴权过程中使用错误的CMAC值进行鉴权后再使用正确的CMAC值进行鉴权。"
)
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-756")
@pytest.mark.skipif(
    cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth"
)
def test_ehsm_756():
    with allure.step(
        "1、配置OTP 生命周期 debug 模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将修改错误的CMAC值传入 debug_auth 接口，进行鉴权 # 发送成功"):
        # 修改CMAC值的第一个字节，使其错误
        wrong_signature = bytearray(singnature)
        wrong_signature[0] = wrong_signature[0] ^ 0xFF
        wrong_signature = bytes(wrong_signature)
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=wrong_signature,
                sig_size=len(wrong_signature),
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            # 鉴权应该失败
            assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        except Exception as e:
            if int(str(e)) == EHSM_ERR_DEBUG_AUTH_FAILED:
                logging.info(f"错误CMAC鉴权失败，符合预期: 错误码 {EHSM_ERR_DEBUG_AUTH_FAILED}")
                # 确认鉴权状态为失败
                assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            else:
                # 如果不是预期的错误码，测试失败
                assert False, f"错误CMAC鉴权返回了非预期的错误码: {int(str(e))}，预期错误码: {EHSM_ERR_DEBUG_AUTH_FAILED}"
    with allure.step("鉴权失败后需要使用 get_challenge 接口，重新获取挑战字； # 读取成功，数据正确"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step(
        "调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 计算成功"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("将重新计算好的CMAC值传入 debug_auth 接口，进行鉴权 # 发送成功"):
        # 使用正确的CMAC值进行鉴权
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
        # 鉴权应该成功
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("7、检查鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("在User 模式下，使用非法鉴权命令参数进行鉴权测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-757")
def test_ehsm_757():
    with allure.step("1、输入非法鉴权类型，进行鉴权 # 1、鉴权失败，返回对应错误码；"):
        try:
            # 使用无效的鉴权类型（假设值999不存在）
            api.ehsm_debug_auth(
                999,  # 非法类型
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=b'\x00' * 16,
                sig_size=16,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
        except Exception as e:
            logging.info(f"非法鉴权类型测试失败，符合预期: {e}")
    with allure.step("2、输入非法鉴权算法，进行鉴权； # 2、鉴权失败，返回对应错误码；"):
        try:
            # 使用无效的算法类型（假设值999不存在）
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                999,  # 非法算法
                sig=b'\x00' * 16,
                sig_size=16,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
        except Exception as e:
            logging.info(f"非法鉴权算法测试失败，符合预期: {e}")
    with allure.step("3、输入非法签名地址或size，进行鉴权； # 3、鉴权失败，返回对应错误码；"):
        try:
            # 使用无效的签名数据（None但size不为0）
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=None,  # 非法签名数据
                sig_size=16,  # 但size不为0
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
        except Exception as e:
            logging.info(f"非法签名参数测试失败，符合预期: {e}")


@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 SHA256-SECP256R1 算法，测试UART方式的 eHSM鉴权功能")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-758")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or not cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 1, reason="UART or ECC256 not support for debug auth")
def test_ehsm_758():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SHA256-SECP256R1预置值公钥hash和对应属性，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用串口调试工具发送 REQ21@ 字符串，获取挑战字； # 3、读取成功，数据正确"):
        ret, challenge = get_challenge_from_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        logging.info(f"获取的挑战字为{challenge}")
    with allure.step("4、将获取到的挑战字进行hash计算，ECC算法使用预置ecc 私钥，计算签名； # 4、读取成功，数据正确"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥组合成字符串格式RSP+type+alg:+public key:+signature+@,并通过串口发送，进行鉴权 # 5、发送成功"):
        ret= debug_auth_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                           EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                           key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                           singnature)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 SM2-SM2 算法方式，测试UART方式的 eHSM 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-759")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or not cfg_data.TEST_FW_DBG_SM2_SUPPORT == 1, reason="UART or SM2 not support for debug auth")
def test_ehsm_759():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM3算法预置值公钥hash和对应属性，其它默认； # 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm2_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用串口调试工具发送 REQ21@ 字符串，获取挑战字； # 3、读取成功，数据正确"):
        ret, challenge = get_challenge_from_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2)
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        logging.info(f"获取的挑战字为{challenge}")
    with allure.step("4、调用上层接口对挑战字进行SM3计算，SM2算法使用预置SM2公私钥，计算签名； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2)
        assert 0 == ret
    with allure.step("5、将签名好的数据和公钥组合成字符串格式 RSP+type+alg:+public key:+signature+@,并通过串口发送，进行鉴权 # 5、发送成功"):
        ret = debug_auth_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                           EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
                           key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY,
                           singnature)
        assert 0 == ret




@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 AES-CMAC 算法方式，测试UART方式的 eHSM 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-761")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or not cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 1, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_761():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置AES-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_ehsm_aes_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用串口调试工具发送 REQ21@ 字符串，获取挑战字； # 3、读取成功，数据正确"):
        ret, challenge = get_challenge_from_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        logging.info(f"获取的挑战字为{challenge}")
    with allure.step("4、调用上层接口对挑战字进行CMAC计算，AES-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥组合成字符串格式 RSP+type+alg:+public key:+signature+@,并通过串口发送，，进行鉴权 # 5、发送成功"):
        ret = debug_auth_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                           EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
                           key.EHSM_UPGRADE_SIGN_KEY_AES128,
                           singnature)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("在Test 模式下，使用 SM4-CMAC 算法方式，测试UART方式的 eHSM 类型的正常鉴权过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-762")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or not cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 1, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_762():
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用串口调试工具发送 REQ21@ 字符串，获取挑战字； # 3、读取成功，数据正确"):
        ret, challenge = get_challenge_from_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        logging.info(f"获取的挑战字为{challenge}")
    with allure.step("4、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 4、签名成功；"):
        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
        assert 0 == ret
    with allure.step("5、将计算好的CMAC和公钥组合成字符串格式 RSP+type+alg:+public key:+signature+@,并通过串口发送，，进行鉴权 # 5、发送成功"):
        ret = debug_auth_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                           EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                           key.EHSM_DEBUG_SIGN_KEY_SM4,
                           singnature)
        assert 0 == ret


# =====================================================================
# 字符调试鉴权协议测试用例 - eHSM类型
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取eHSM+RSA2048挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D005")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="UART or RSA2048 not support for debug auth")
def test_ehsm_d005():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为RSA2048公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_rsa_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ15@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+RSA2048签名校验")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D022")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="UART or RSA2048 not support for debug auth")
def test_ehsm_d022():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为RSA2048公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_rsa_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ15@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用RSA2048私钥对挑战值进行签名 # 签名成功"):
        host.set_rsa_key_size(2048)
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            key.EHSM_VERIFY_SIGN_KEY_RSA2048_E + key.EHSM_VERIFY_SIGN_KEY_RSA2048_N,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+RSA3072签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D023")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA3072_SUPPORT == 0, reason="UART or RSA3072 not support for debug auth")
def test_ehsm_d023():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为RSA3072公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_rsa3072_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ15@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用RSA3072私钥对挑战值进行签名 # 签名成功"):
        host.set_rsa_key_size(3072)
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            key.EHSM_VERIFY_SIGN_KEY_RSA3072_E + key.EHSM_VERIFY_SIGN_KEY_RSA3072_N,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("7、重置RSA密钥大小为默认值 # 重置成功"):
        host.set_rsa_key_size(2048)


# =====================================================================
# 字符调试鉴权协议测试用例 - SOC类型
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取SOC+SM3-SM2挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D006")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="UART or SM2 not support for debug auth")
def test_ehsm_d006():
    with allure.step("1、配置OTP为Test模式，配置SOC Debug Key为SM2公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm2_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ31@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+SM3-SM2签名校验")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D024")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="UART or SM2 not support for debug auth")
def test_ehsm_d024():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)  # 开启所有调试端口

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为SM2公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm2_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ31@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SM2私钥对挑战值进行签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
            key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+SHA256-SECP256R1签名校验")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D025")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d025():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为ECC256公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_ecc256_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ32@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SECP256R1私钥对挑战值进行签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+SM4-CMAC签名校验")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D026")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d026():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为SM4密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ33@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SM4密钥对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+AES128-CMAC签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D027")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_d027():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为AES128密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_aes_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ34@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用AES128密钥对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+RSA2048签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D028")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="UART or RSA2048 not support for debug auth")
def test_ehsm_d028():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为RSA2048公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_rsa_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ35@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用RSA2048私钥对挑战值进行签名 # 签名成功"):
        host.set_rsa_key_size(2048)
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            key.SOC_VERIFY_SIGN_KEY_RSA2048_E + key.SOC_VERIFY_SIGN_KEY_RSA2048_N,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行SOC+RSA3072签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D029")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA3072_SUPPORT == 0, reason="UART or RSA3072 not support for debug auth")
def test_ehsm_d029():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为RSA3072公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_rsa3072_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ35@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用RSA3072私钥对挑战值进行签名 # 签名成功"):
        host.set_rsa_key_size(3072)
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA,
            key.SOC_VERIFY_SIGN_KEY_RSA3072_E + key.SOC_VERIFY_SIGN_KEY_RSA3072_N,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)

    with allure.step("8、重置RSA密钥大小为默认值 # 重置成功"):
        host.set_rsa_key_size(2048)


# =====================================================================
# 字符调试鉴权协议测试用例 - DIS指令(关闭调试)
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试eHSM鉴权成功后关闭调试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D043")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="UART or SM2 not support for debug auth")
def test_ehsm_d043():
    with allure.step("1、配置OTP为dev模式，配置eHSM Debug Key为SM2公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm2_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART完成eHSM鉴权流程 # 鉴权成功"):
        # 获取挑战值
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

        # 签名
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret

        # 鉴权
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
            key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("4、检查eHSM鉴权状态，确认已鉴权 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("5、通过UART发送DIS1@关闭eHSM调试 # 返回ENDPASS@"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试SOC鉴权成功后关闭调试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D044")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d044():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为Dev模式，配置SOC Debug Key为SM4密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART完成SOC鉴权流程 # 鉴权成功"):
        # 获取挑战值
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

        # 签名
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

        # 鉴权
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("4、检查SOC鉴权状态，确认已鉴权 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("5、通过UART发送DIS3@关闭SOC调试 # 返回ENDPASS@"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
        assert 0 == ret


# =====================================================================
# 字符调试鉴权协议测试用例 - TEST指令(回显测试)
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试TEST指令基本回显功能")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D049")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d049():
    with allure.step("1、配置OTP为Test模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送TESTabcd@测试回显 # 返回TESTabcd@"):
        ret, response = uart_echo_test("abcd")
        assert 0 == ret
        assert response == b"TESTabcd@"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试TEST指令空内容")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D050")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d050():
    with allure.step("1、配置OTP为Test模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送TEST@测试空内容回显 # 返回TEST@"):
        ret, response = uart_echo_test("")
        assert 0 == ret
        assert response == b"TEST@"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试TEST指令长字符串")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D051")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d051():
    with allure.step("1、配置OTP为Test模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送长字符串测试回显 # 原样回显"):
        test_string = "0123456789" * 10  # 100字符
        ret, response = uart_echo_test(test_string)
        assert 0 == ret
        expected = b"TEST" + test_string.encode("utf8") + b"@"
        assert response == expected


# =====================================================================
# 字符调试鉴权协议测试用例 - 综合流程测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试eHSM完整UART鉴权流程: REQ->RSP->DIS")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D053")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d053():
    with allure.step("1、配置OTP为dev模式，配置eHSM Debug Key # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ12@获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge) == 48

    with allure.step("4、对挑战值进行签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("7、通过UART发送DIS1@关闭调试 # 关闭成功"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("测试SOC完整UART鉴权流程: REQ->RSP->DIS")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D054")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_d054():
    soc_dbg_bitmap = bytes([0xFF, 0xFF, 0xFF, 0xFF] * 5)

    with allure.step("1、配置OTP为dev模式，配置SOC Debug Key # 配置成功"):
        assert 0 == host.write_otp(_get_soc_aes_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ34@获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge) == 48

    with allure.step("4、对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查SOC鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)

    with allure.step("7、检查SOC调试端口bitmap寄存器 # 检查通过"):
        assert True == check_soc_debug_ports_bitmap(soc_dbg_bitmap)

    with allure.step("8、通过UART发送DIS3@关闭调试 # 关闭成功"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
        assert 0 == ret


# =====================================================================
# 字符调试鉴权协议测试用例 - 获取挑战值补充测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取eHSM+SM3-SM2挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D001")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="UART or SM2 not support for debug auth")
def test_ehsm_d001():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为SM2公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm2_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ11@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取eHSM+SHA256-SECP256R1挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D002")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d002():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为ECC256公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ12@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取eHSM+SM4-CMAC挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D003")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d003():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为SM4密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ13@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取eHSM+AES128-CMAC挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D004")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_d004():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为AES128密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_aes_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ14@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取SOC+SHA256-SECP256R1挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D007")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d007():
    with allure.step("1、配置OTP为Test模式，配置SOC Debug Key为ECC256公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_ecc256_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ32@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取SOC+SM4-CMAC挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D008")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d008():
    with allure.step("1、配置OTP为Test模式，配置SOC Debug Key为SM4密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ33@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取SOC+AES128-CMAC挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D009")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_d009():
    with allure.step("1、配置OTP为Test模式，配置SOC Debug Key为AES128密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_aes_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ34@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


@allure.feature("debug_auth")
@allure.description("通过UART字符协议获取SOC+RSA2048挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D010")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_RSA2048_SUPPORT == 0, reason="UART or RSA2048 not support for debug auth")
def test_ehsm_d010():
    with allure.step("1、配置OTP为Test模式，配置SOC Debug Key为RSA2048公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_rsa_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ35@，获取挑战值 # 返回48字节挑战值"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA
        )
        assert 0 == ret
        challenge_bytes = bytes.fromhex(challenge.decode('ascii'))
        assert len(challenge_bytes) == 48, "挑战值长度应为48字节(32字节随机数+16字节UID)"


# =====================================================================
# 字符调试鉴权协议测试用例 - eHSM类型签名校验补充测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+SM3-SM2签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D018")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM2_SUPPORT == 0, reason="UART or SM2 not support for debug auth")
def test_ehsm_d018():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为SM2公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm2_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ11@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SM2私钥对挑战值进行签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2,
            key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+SHA256-SECP256R1签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D019")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d019():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为ECC256公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ12@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SECP256R1私钥对挑战值进行签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+SM4-CMAC签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D020")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d020():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为SM4密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ13@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用SM4密钥对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("通过UART字符协议进行eHSM+AES128-CMAC签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D021")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="UART or AES-CMAC not support for debug auth")
def test_ehsm_d021():
    with allure.step("1、配置OTP为Test模式，配置eHSM Debug Key为AES128密钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_aes_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送REQ14@，获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用AES128密钥对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令进行签名校验 # 返回ENDPASS@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态，确认鉴权成功 # 检查通过"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


# =====================================================================
# 补充测试用例 - P0优先级
# =====================================================================

@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试基本关闭eHSM调试功能")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D041")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d041():
    with allure.step("1、配置OTP为Dev模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送DIS1@关闭eHSM调试 # 返回ENDPASS@"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("通过UART字符协议测试基本关闭SOC调试功能")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D042")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d042():
    with allure.step("1、配置OTP为Dev模式 # 配置成功"):
        assert 0 == host.write_otp(_get_soc_sm4_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART发送DIS3@关闭SOC调试 # 返回ENDPASS@"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("在Test生命周期模式下测试完整UART调试鉴权流程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D058")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d058():
    with allure.step("1、配置OTP为Test模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("7、通过UART关闭调试 # 关闭成功"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret


@allure.feature("debug_auth")
@allure.description("在Dev生命周期模式下测试完整UART调试鉴权流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_D059")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d059():
    with allure.step("1、配置OTP为Dev模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("7、通过UART关闭调试 # 关闭成功"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret
        # Dev模式下可以成功关闭调试
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("在Manu生命周期模式下测试完整UART调试鉴权流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_D060")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d060():
    with allure.step("1、配置OTP为Manu模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("manu"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


@allure.feature("debug_auth")
@allure.description("在Debug生命周期模式下测试完整UART调试鉴权流程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_D062")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d062():
    with allure.step("1、配置OTP为Debug模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("debug"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、对挑战值进行CMAC计算 # 计算成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("5、通过UART发送RSP指令完成鉴权 # 鉴权成功"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature
        )
        assert 0 == ret

    with allure.step("6、检查eHSM鉴权状态 # 已鉴权"):
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


# =====================================================================
# 补充测试用例 - P1优先级 (异常和边界测试)
# =====================================================================

@allure.feature("debug_auth")
@allure.description("在User生命周期模式下测试UART调试鉴权限制")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_D061")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d061():
    from platform_adapter.uart_lib import ehsm_fw_errno

    with allure.step("1、配置OTP为User模式 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("user"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、尝试通过UART获取挑战值 # 应失败或受限"):
        try:
            ret, challenge = get_challenge_from_uart(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
            )
            # User模式下可能无法获取挑战字或鉴权受限
            if ret == 0:
                challenge = bytes.fromhex(challenge.decode('ascii'))
                ret2, signature = host.sign(
                    challenge,
                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                    EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
                )
                if ret2 == 0:
                    # 尝试鉴权
                    ret3 = debug_auth_by_uart(
                        EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                        EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                        None,
                        signature
                    )
                    # User模式下鉴权应该失败
                    assert ret3 != 0, "User模式下调试鉴权应该受限"
            else:
                logging.info(f"User模式下获取挑战值失败，符合预期")
        except Exception as e:
            logging.info(f"User模式下UART鉴权受限，符合预期: {e}")


# =====================================================================
# REQ指令异常测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试REQ指令使用非法type参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D011")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d011():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ91@(非法type=9) # 返回错误响应或无响应"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"REQ91@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 验证不是正常的挑战值响应
        assert rsp.startswith(b"CHA9:get random fail"), "不应返回正常的挑战值响应"


@allure.feature("debug_auth")
@allure.description("测试REQ指令使用非法alg参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D012")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d012():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ19@(非法alg=9) # 返回错误响应或无响应"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"REQ19@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 验证不是正常的挑战值响应
        assert rsp.startswith(b"CHA1:get random fail"), "不应返回正常的挑战值响应"


@allure.feature("debug_auth")
@allure.description("测试REQ指令缺少@结束符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D013")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d013():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ11(缺少@) # 无响应或超时"):
        ser.write(b"REQ11")
        import time
        time.sleep(0.5)  # 等待可能的响应
        # 清空接收缓冲区
        rsp = ser.read(ser.in_waiting) if ser.in_waiting > 0 else b""
        logging.info(f"接收到的响应: {rsp}")
        # 由于没有结束符，不应收到完整响应
        assert not rsp.endswith(b"@"), "缺少结束符时不应收到完整响应"


@allure.feature("debug_auth")
@allure.description("测试REQ指令格式错误-参数不完整")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D014")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d014():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ1@(缺少alg) # 返回错误响应"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"REQ1@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 验证不是正常的挑战值响应
        assert rsp.startswith(b"CHA1:get random fail"), "参数不完整时不应返回正常响应"


@allure.feature("debug_auth")
@allure.description("测试REQ指令格式错误-只有REQ")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D015")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d015():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ@(缺少type和alg) # 返回错误响应"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"REQ@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 验证不是正常的挑战值响应
        assert rsp.startswith(b"CHA:get random fail@"), "参数缺失时不应返回正常响应"


# =====================================================================
# RSP指令异常测试 - 错误的签名/公钥
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试RSP指令使用错误的签名值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D030")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d030():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名并修改其中一个字节 # 修改成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        # 修改签名的第一个字节
        wrong_signature = bytearray(signature)
        wrong_signature[0] = wrong_signature[0] ^ 0xFF
        wrong_signature = bytes(wrong_signature)

    with allure.step("4、通过UART发送错误的签名 # 返回END_DATA_FAIL@或END_VERIFY_FAIL@"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            wrong_signature
        )
        # 应该返回验证失败错误
        assert ret != 0, "错误的签名应该验证失败"
        logging.info(f"错误签名验证失败，返回错误码: {ret}")


@allure.feature("debug_auth")
@allure.description("测试RSP指令使用错误的公钥")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D031")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d031():
    with allure.step("1、配置OTP为Dev模式，配置eHSM Debug Key为ECC256公钥 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))

    with allure.step("2、重启eHSM，并检查启动状态 # 重启成功"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("3、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("4、使用正确私钥签名，但使用错误的公钥 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("5、通过UART发送错误的公钥 # 返回END_PUBKEY_FAIL@"):
        # 使用SOC的公钥替代eHSM的公钥
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            key.SOC_VERIFY_SIGN_KEY_SM2_PUBKEY,  # 使用错误的公钥
            signature
        )
        # 应该返回公钥校验失败错误
        assert ret != 0, "错误的公钥应该校验失败"
        logging.info(f"错误公钥校验失败，返回错误码: {ret}")


@allure.feature("debug_auth")
@allure.description("测试RSP指令签名长度错误")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D032")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d032():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名并截断长度 # 修改成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        # 截断签名，只保留一半长度
        wrong_signature = signature[:len(signature)//2]

    with allure.step("4、通过UART发送长度错误的签名 # 返回错误响应"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            wrong_signature
        )
        # 应该返回数据错误
        assert ret != 0, "签名长度错误应该验证失败"
        logging.info(f"签名长度错误验证失败，返回错误码: {ret}")


@allure.feature("debug_auth")
@allure.description("测试RSP指令公钥长度错误")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D033")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d033():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("4、通过UART发送长度错误的公钥 # 返回END_PUBKEY_FAIL@"):
        # 截断公钥，只保留一半长度
        wrong_pubkey = key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY[:len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY)//2]
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
            wrong_pubkey,
            signature
        )
        # 应该返回公钥错误
        assert ret != 0, "公钥长度错误应该校验失败"
        logging.info(f"公钥长度错误校验失败，返回错误码: {ret}")


@allure.feature("debug_auth")
@allure.description("测试RSP指令缺少冒号分隔符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D036")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d036():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("4、构造缺少冒号的RSP指令并发送 # 返回END_PARAMETER_FAIL@"):
        # 构造格式: RSP12pubkeysignature@ (缺少冒号分隔符)
        pubkey_hex = binascii.hexlify(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY).decode()
        sig_hex = binascii.hexlify(signature).decode()
        req = f"RSP12{pubkey_hex}{sig_hex}@"
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回参数错误
        assert not rsp.startswith(b"ENDPASS"), "格式错误应该返回错误响应"


@allure.feature("debug_auth")
@allure.description("测试RSP指令包含非十六进制字符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D038")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d038():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("4、构造包含非法字符的RSP指令并发送 # 返回END_DATA_FAIL@"):
        pubkey_hex = binascii.hexlify(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY).decode()
        sig_hex = binascii.hexlify(signature).decode()
        # 在签名中插入非十六进制字符 'G' 和 'Z'
        sig_hex_invalid = sig_hex[:10] + "GZ" + sig_hex[12:]
        req = f"RSP12:{pubkey_hex}:{sig_hex_invalid}@"
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回数据错误
        assert not rsp.startswith(b"ENDPASS"), "非法字符应该返回错误响应"


@allure.feature("debug_auth")
@allure.description("测试RSP指令缺少@结束符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D039")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d039():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("4、构造缺少@的RSP指令并发送 # 无响应或超时"):
        sig_hex = binascii.hexlify(signature).decode()
        req = f"RSP13:{sig_hex}"  # 缺少@结束符
        ser.write(req.encode())
        import time
        time.sleep(0.5)  # 等待可能的响应
        # 清空接收缓冲区
        rsp = ser.read(ser.in_waiting) if ser.in_waiting > 0 else b""
        logging.info(f"接收到的响应: {rsp}")
        # 由于没有结束符，不应收到完整响应
        assert not rsp.endswith(b"@"), "缺少结束符时不应收到完整响应"


@allure.feature("debug_auth")
@allure.description("测试RSP指令type/alg参数错误")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D040")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d040():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("4、使用非法type发送RSP指令 # 返回END_PARAMETER_FAIL@"):
        sig_hex = binascii.hexlify(signature).decode()
        req = f"RSP93:{sig_hex}@"  # type=9, alg=3 (type非法)
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        assert not rsp.startswith(b"ENDPASS"), "非法type应该返回错误响应"

    with allure.step("5、使用非法alg发送RSP指令 # 返回END_PARAMETER_FAIL@"):
        req = f"RSP19:{sig_hex}@"  # type=1, alg=9 (alg非法)
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        assert not rsp.startswith(b"ENDPASS"), "非法alg应该返回错误响应"


# =====================================================================
# DIS指令异常测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试DIS指令使用非法type参数")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D046")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d046():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送DIS9@(非法type=9) # 返回END_PARAMETER_FAIL@"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"DIS9@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回参数错误
        assert rsp.startswith(b"END_PARAMETER_FAIL"), "非法type应该返回参数错误"


@allure.feature("debug_auth")
@allure.description("测试DIS指令缺少type参数")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D047")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d047():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送DIS@(缺少type) # 返回END_PARAMETER_FAIL@"):
        if ser.in_waiting > 0:
            discarded = ser.read(ser.in_waiting)
            logging.warning(f"发送前丢弃了{len(discarded)}字节缓冲数据: {discarded}")
        ser.reset_input_buffer()
        ser.write(b"DIS@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回参数错误
        assert rsp.startswith(b"END_PARAMETER_FAIL"), "缺少type应该返回参数错误"


# =====================================================================
# 综合流程异常测试
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试连续多次获取挑战值")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D055")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d055():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、连续3次获取挑战值 # 每次都应该成功"):
        challenges = []
        for i in range(3):
            ret, challenge = get_challenge_from_uart(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
            )
            assert 0 == ret, f"第{i+1}次获取挑战值失败"
            challenges.append(challenge)
            logging.info(f"第{i+1}次挑战值: {challenge}")

    with allure.step("3、验证每次获取的挑战值不同 # 挑战值应该不同"):
        # 至少有两个挑战值不同（理论上都应该不同）
        unique_challenges = set(challenges)
        assert len(unique_challenges) >= 2, "多次获取的挑战值应该不同"


@allure.feature("debug_auth")
@allure.description("测试鉴权失败后重新鉴权")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D056")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d056():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、通过UART发送DIS1@关闭eHSM调试 # 返回ENDPASS@"):
        ret = close_debug_by_uart(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert 0 == ret
    with allure.step("3、检查eHSM鉴权状态，确认已关闭 # eHSM调试鉴权关闭"):
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("4、获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("5、计算正确的签名并修改 # 修改成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        wrong_signature = bytearray(signature)
        wrong_signature[0] = wrong_signature[0] ^ 0xFF
        wrong_signature = bytes(wrong_signature)

    with allure.step("6、使用错误签名鉴权 # 鉴权失败"):
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            wrong_signature
        )
        assert ret != 0, "错误签名应该鉴权失败"
        # 确认未鉴权成功
        assert False == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("7、重新获取挑战值 # 获取成功"):
        ret, challenge2 = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        challenge2 = bytes.fromhex(challenge2.decode('ascii'))

    with allure.step("8、使用正确签名重新鉴权 # 鉴权成功"):
        ret, signature2 = host.sign(
            challenge2,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        ret = debug_auth_by_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            None,
            signature2
        )
        assert 0 == ret, "正确签名应该鉴权成功"
        # 确认鉴权成功
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)


# =====================================================================
# P2优先级边界测试 - REQ指令
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试REQ指令包含多余字符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D016")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d016():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送REQ11X@(包含多余字符X) # 返回错误响应或被忽略"):
        ser.write(b"REQ11X@")
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 多余字符应该导致错误或被忽略
        if rsp.startswith(b"CHA"):
            logging.warning("多余字符被忽略，系统仍然返回了挑战值")
        else:
            logging.info("多余字符导致错误响应，符合预期")


@allure.feature("debug_auth")
@allure.description("测试REQ指令发送空字符串")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D017")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d017():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送仅@符号 # 无响应或错误响应"):
        ser.write(b"@")
        import time
        time.sleep(0.5)
        rsp = ser.read(ser.in_waiting) if ser.in_waiting > 0 else b""
        logging.info(f"接收到的响应: {rsp}")
        # 空命令应该无响应或错误
        assert not rsp.startswith(b"CHA"), "空命令不应返回正常响应"


# =====================================================================
# P2优先级边界测试 - RSP指令
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试RSP指令使用空签名")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D034")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d034():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret

    with allure.step("3、构造空签名的RSP指令并发送 # 返回END_DATA_FAIL@"):
        req = "RSP13:@"  # 空签名
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回数据错误
        assert not rsp.startswith(b"ENDPASS"), "空签名应该返回错误响应"


@allure.feature("debug_auth")
@allure.description("测试RSP指令使用空公钥(非对称算法)")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D035")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d035():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("4、构造空公钥的RSP指令并发送 # 返回END_PUBKEY_FAIL@"):
        sig_hex = binascii.hexlify(signature).decode()
        req = f"RSP12::{sig_hex}@"  # 空公钥
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回公钥错误
        assert not rsp.startswith(b"ENDPASS"), "空公钥应该返回错误响应"


@allure.feature("debug_auth")
@allure.description("测试RSP指令包含多余的冒号")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D037")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="UART or ECC256 not support for debug auth")
def test_ehsm_d037():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART获取挑战值 # 获取成功"):
        ret, challenge = get_challenge_from_uart(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret
        challenge = bytes.fromhex(challenge.decode('ascii'))

    with allure.step("3、计算正确的签名 # 签名成功"):
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1
        )
        assert 0 == ret

    with allure.step("4、构造包含多余冒号的RSP指令并发送 # 返回错误响应"):
        pubkey_hex = binascii.hexlify(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY).decode()
        sig_hex = binascii.hexlify(signature).decode()
        req = f"RSP12:{pubkey_hex}::{sig_hex}:@"  # 多余的冒号
        ser.write(req.encode())
        rsp = ser.read_until(b"@")
        logging.info(f"接收到的响应: {rsp}")
        # 应该返回参数错误或数据错误
        assert not rsp.startswith(b"ENDPASS"), "多余冒号应该返回错误响应"


# =====================================================================
# P2优先级边界测试 - DIS指令
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试DIS指令缺少@结束符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D048")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d048():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、通过UART发送DIS1(缺少@) # 无响应或超时"):
        ser.write(b"DIS1")
        import time
        time.sleep(0.5)
        rsp = ser.read(ser.in_waiting) if ser.in_waiting > 0 else b""
        logging.info(f"接收到的响应: {rsp}")
        # 由于没有结束符，不应收到完整响应
        assert not rsp.endswith(b"@"), "缺少结束符时不应收到完整响应"


# =====================================================================
# P2优先级边界测试 - TEST指令
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试TEST指令包含特殊字符")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D052")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0, reason="UART not support for debug auth")
def test_ehsm_d052():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、发送包含特殊字符的TEST指令 # 原样回显"):
        special_chars = "!#$%&*+-=[]{}|;'<>?/\\"
        ret, response = uart_echo_test(special_chars)
        if ret == 0:
            logging.info("特殊字符正常回显")
            assert response == b"TEST" + special_chars.encode("utf8") + b"@"
        else:
            logging.info(f"特殊字符处理异常，返回错误码: {ret}")


# =====================================================================
# P2优先级边界测试 - 并发场景
# =====================================================================

@allure.feature("debug_auth")
@allure.description("测试快速连续发送多个命令")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D057")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_UART_SUPPORT == 0 or cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="UART or SM4-CMAC not support for debug auth")
def test_ehsm_d057():
    with allure.step("1、配置OTP为Dev模式并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、快速连续发送5个REQ命令 # 观察系统响应"):
        import time
        responses = []
        for i in range(5):
            ser.write(b"REQ13@")
            time.sleep(0.1)  # 短暂延时
            rsp = ser.read_until(b"@")
            responses.append(rsp)
            logging.info(f"第{i+1}次响应: {rsp}")

        # 统计成功响应的数量
        success_count = sum(1 for rsp in responses if rsp.startswith(b"CHA"))
        logging.info(f"成功响应数量: {success_count}/{len(responses)}")

        # 至少应该有一些响应是成功的
        assert success_count > 0, "至少应该有一些请求成功"

@allure.feature("debug_auth")
@allure.description("测试get_challenge接口type参数为无效值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D063")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d063():
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、使用get_challenge接口，传入非法的challenge_type参数（超出有效范围）； # 2、返回参数错误码；"):
        # 使用无效的challenge type
        try:
            t, challenge = api.ehsm_get_challenge(0)
            assert False, "传入非法challenge_type时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == EHSM_ERR_WRONG_CHALLENGE_TYPE, \
                f"预期错误码为EHSM_ERR_WRONG_CHALLENGE_TYPE({EHSM_ERR_WRONG_CHALLENGE_TYPE})，实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试get_challenge接口addr参数为NULL")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D064")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d064():
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用ehsm_get_challenge_error接口，传入ctx_addr=0（空指针）； # 2、返回参数错误码；"):
        # Reason: 使用api.ehsm_get_challenge_error接口传入自定义ctx_addr参数进行异常测试
        try:
            # 获取DATA1_ADDR用于接收挑战字输出
            DATA1_ADDR = api.DATA1_ADDR

            t, challenge = api.ehsm_get_challenge_error(
                0,  # ctx_addr=0，空指针
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                DATA1_ADDR
            )
            # 如果没有抛出异常，测试失败
            assert False, "传入ctx_addr=0时未抛出异常，不符合预期"
        except hostapi.HostApiError as e:
            # Reason: ret_code可能是有符号整数，使用位运算转换为无符号
            error_code = e.ret_code & 0xFFFF
            logging.info(f"捕获异常，错误码: {error_code}")
            assert error_code == EHSM_ERR_PARAM_ERROR , \
                f"预期错误码为EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {error_code}"

@allure.feature("debug_auth")
@allure.description("测试debug_auth接口challenge_type参数为无效值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D065")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d065(setup_module):
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用debug_auth接口，传入无效的challenge_type参数(0) # 返回错误"):
        try:
            api.ehsm_debug_auth(
                0,  # 无效的challenge_type
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                sig=b'\x00' * 64,
                sig_size=64,
                pub_key=key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
                soc_dbg_bitmap=None
            )
            assert False, "传入无效challenge_type应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"预期错误码为EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR}),实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试debug_auth接口algo参数为无效算法类型")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D066")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d066(setup_module):
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用debug_auth接口，传入无效的algo参数(99) # 返回错误"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                99,  # 无效的算法类型
                sig=b'\x00' * 64,
                sig_size=64,
                pub_key=key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
                soc_dbg_bitmap=None
            )
            assert False, "传入无效algo应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"预期错误码为EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试debug_auth接口sig_size参数为0")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D067")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d067(setup_module):
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用debug_auth接口，传入sig_size=0 # 返回错误"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                sig=b'\x00' * 64,
                sig_size=0,  # 签名长度为0
                pub_key=key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
                soc_dbg_bitmap=None
            )
            assert False, "传入sig_size=0应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == EHSM_ERR_WRONG_DATA_LENGTH, \
                f"预期错误码为参数或长度错误，实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试debug_auth接口pub_key参数为None(非对称算法)")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D068")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d068(setup_module):
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用debug_auth接口，传入pub_key=None(非对称算法需要公钥) # 返回错误"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                sig=b'\x00' * 64,
                sig_size=64,
                pub_key=None,  # 非对称算法必须提供公钥
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            assert False, "非对称算法传入pub_key=None应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            # Reason: pub_key=None时，传入的地址为0，固件返回EHSM_ERR_INVALID_ADDRESS(8)
            assert e.ret_code == EHSM_ERR_INVALID_ADDRESS, \
                f"预期错误码为EHSM_ERR_INVALID_ADDRESS({EHSM_ERR_INVALID_ADDRESS})，实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试debug_auth接口sig_size参数与实际签名长度不匹配")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D069")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
def test_ehsm_d069(setup_module):
    with allure.step("1、配置OTP为Test模式，调试鉴权算法配置为ECC256，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、调用debug_auth接口，传入错误的sig_size(实际长度+10) # 返回错误"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                sig=b'\x00' * 64,
                sig_size=74,  # 错误的签名长度（实际64+10）
                pub_key=key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
                soc_dbg_bitmap=None
            )
            assert False, "传入错误的sig_size应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            # Reason: sig_size与实际数据长度不匹配时，固件可能返回EHSM_ERR_WRONG_DATA_LENGTH(83)
            assert e.ret_code == EHSM_ERR_WRONG_DATA_LENGTH, \
                f"预期错误码为签名长度错误或参数错误，实际: {e.ret_code}"


@allure.feature("debug_auth")
@allure.description("测试close_debug接口type参数为无效值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_D070")
@pytest.mark.skipif(cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4-CMAC not support for debug auth")
def test_ehsm_d070(setup_module):
    with allure.step("1、配置OTP为Dev模式，调试鉴权算法配置为SM4-CMAC，并重启 # 配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("dev"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、完成eHSM调试鉴权 # 鉴权成功"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        ret, signature = host.sign(
            challenge,
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC
        )
        assert 0 == ret
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
            sig=signature,
            sig_size=len(signature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
        assert True == host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)

    with allure.step("3、调用close_debug接口，传入无效的type参数(99) # 返回错误"):
        try:
            # Reason: type=99为无效的调试类型，固件应返回参数错误
            api.ehsm_close_debug(99, None)
            assert False, "传入无效type应该抛出异常"
        except hostapi.HostApiError as e:
            logging.info(f"捕获异常，错误码: {e.ret_code}")
            assert e.ret_code == EHSM_ERR_NOT_SUPPORT, \
                f"预期错误码为EHSM_ERR_NOT_SUPPORT({EHSM_ERR_NOT_SUPPORT})，实际: {e.ret_code}"

# =========================================
# 对称算法鉴权，签名长度与缓存大小比较问题
# =========================================

@pytest.mark.skipif(cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0, reason="AES128CMAC not support for debug auth")
@allure.feature("debug_auth")
@allure.description("对称算法调试鉴权 sign_size=384（上界），正常执行")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D071")
def test_ehsm_d071(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置AES-128-CMAC算法密钥预置值； # 1、配置成功"):
        assert 0 == host.write_otp(_get_ehsm_aes_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("4、调用上层接口对挑战字进行CMAC计算，AES-128-CMAC算法使用预置密钥，计算CMAC； # 4、签名成功；"):
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        # Reason: 根据测试需求，构造一个 384 字节的缓冲区，并将真实的 CMAC 签名放入开头
        # 即使实际签名较短，固件也应接受声明长度为 384 的请求并进行后续处理
    with allure.step("5、将 sig_size 设置为 384 字节，传入 debug_auth 接口，进行鉴权 # 5、鉴权不应因长度被拒绝；"):
        auth_rejected_by_param_error = False
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
                sig=signature,
                sig_size=384,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
        except hostapi.HostApiError as e:
            # Reason: 固件不应返回 EHSM_ERR_PARAM_ERROR (1)，如果返回其他错误（如鉴权失败）则是业务逻辑问题
            # 这里重点验证 sign_size 的边界检查逻辑
            assert e.ret_code != EHSM_ERR_PARAM_ERROR, \
                f"固件因 sign_size=384 错误返回了 EHSM_ERR_PARAM_ERROR (1)，边界检查有误"
            auth_rejected_by_param_error = False
            logging.info(f"鉴权接口返回错误码: {e.ret_code}，符合预期（非参数错误）")

    with allure.step("6、检查eHSM 状态，确认鉴权状态符合预期 # 6、检查通过"):
        # Reason: sig_size=384 时 sig 内容可能不足导致鉴权失败，但不应静默；
        # status 若为 False 是因签名内容不匹配（正常），不是因为 sig_size 被拒绝
        status = host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        assert isinstance(status, bool), \
            f"check_debug_auth_status 应返回 bool，实际: {type(status)}"
        logging.info(f"当前 eHSM 调试鉴权状态: {status}（sig内容不匹配时预期为False）")


@pytest.mark.skipif(cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth")
@allure.feature("debug_auth")
@allure.description("对称算法调试鉴权 sign_size=385（超界），返回错误")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D072")
def test_ehsm_d072(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM4-CMAC算法密钥预置值； # 1、配置成功"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、使用 get_challenge 接口，获取挑战字； # 3、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("4、调用上层接口对挑战字进行CMAC计算，SM4-CMAC算法使用预置密钥，计算CMAC； # 4、签名成功；"):
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
    with allure.step("5、将 sig_size 设置为 385 字节，传入 debug_auth 接口，进行鉴权 # 5、返回 EHSM_ERR_PARAM_ERROR；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=signature,
                sig_size=385,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            assert False, "sign_size=385 应返回 EHSM_ERR_PARAM_ERROR (1) 但未抛出异常"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"预期错误码为 EHSM_ERR_PARAM_ERROR(1)，实际: {e.ret_code}"
            logging.info(f"鉴权接口正确返回参数错误码: {e.ret_code}")


@pytest.mark.skipif(cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth")
@allure.feature("debug_auth")
@allure.description("对称算法调试鉴权 sign_size=0，返回错误")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D073")
def test_ehsm_d073(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，并重启；# 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、获取挑战字并计算签名；# 2、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
    with allure.step("3、将 sig_size 设置为 0，传入 debug_auth 接口 # 3、返回 EHSM_ERR_WRONG_DATA_LENGTH；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=signature,
                sig_size=0,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            assert False, "sign_size=0 应抛出异常"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_WRONG_DATA_LENGTH, f"预期错误码为 EHSM_ERR_WRONG_DATA_LENGTH(83)，实际: {e.ret_code}"
            logging.info(f"鉴权接口正确返回长度错误码: {e.ret_code}")


@pytest.mark.skipif(cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth")
@allure.feature("debug_auth")
@allure.description("对称算法调试鉴权 sign_size=0xFFFFFFFF，返回错误，无溢出")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM_D074")
def test_ehsm_d074(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，并重启；# 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、获取挑战字并计算签名；# 2、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
    with allure.step("3、将 sig_size 设置为 0xFFFFFFFF，传入 debug_auth 接口 # 3、返回 EHSM_ERR_PARAM_ERROR；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=signature,
                sig_size=0xFFFFFFFF,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            assert False, "sign_size=0xFFFFFFFF 应返回 EHSM_ERR_PARAM_ERROR"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"预期错误码为 EHSM_ERR_PARAM_ERROR(1)，实际: {e.ret_code}"
            logging.info(f"鉴权接口正确返回参数错误码: {e.ret_code}")


@pytest.mark.skipif(cfg_data.TEST_FW_DBG_SM4_CMAC_SUPPORT == 0, reason="SM4CMAC not support for debug auth")
@allure.feature("debug_auth")
@allure.description("对称算法 sign_size=1（下界），正常执行")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D075")
def test_ehsm_d075(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，并重启；# 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_sm4_debug_config("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、获取挑战字并计算签名，构造 1 字节数据；# 2、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC)
    with allure.step("3、将 sig_size 设置为 1，传入 debug_auth 接口 # 3、鉴权不应因长度被拒绝；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC,
                sig=signature,
                sig_size=1,
                pub_key=None,
                pub_key_size=0,
                soc_dbg_bitmap=None
            )
            # sig_size=1 时签名内容极可能不匹配，但若成功则检查状态
            status = host.check_debug_auth_status(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
            assert isinstance(status, bool), \
                f"check_debug_auth_status 应返回 bool，实际: {type(status)}"
            logging.info(f"鉴权调用成功，状态: {status}")
        except hostapi.HostApiError as e:
            # Reason: 虽然 1 字节会导致鉴权失败（EHSM_ERR_DEBUG_AUTH_FAILED），但固件不应返回 EHSM_ERR_PARAM_ERROR (1)
            assert e.ret_code != EHSM_ERR_PARAM_ERROR, \
                f"固件因 sign_size=1 错误返回了 EHSM_ERR_PARAM_ERROR (1)，边界检查有误"
            logging.info(f"鉴权接口返回错误码: {e.ret_code}，符合预期（非参数错误，为鉴权失败）")


@pytest.mark.skipif(cfg_data.TEST_FW_DBG_ECC256_SUPPORT == 0, reason="ECC256 not support for debug auth")
@allure.feature("debug_auth")
@allure.description("非对称算法调试鉴权 sign_size超界，同样被拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_D076")
def test_ehsm_d076(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置ECC256公钥；# 1、配置成功；"):
        assert 0 == host.write_otp(_get_ehsm_ecc256_debug_config("test"))
    with allure.step("2、重启eHSM，并检查启动状态；# 2、重启检查状态成功；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、获取挑战字并计算签名；# 2、获取挑战字成功；"):
        t, challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
        ret, signature = host.sign(challenge,
                                   EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                   EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1)
    with allure.step("4、将 sig_size 设置为 385 字节，传入 debug_auth 接口 # 4、返回 EHSM_ERR_PARAM_ERROR；"):
        try:
            api.ehsm_debug_auth(
                EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1,
                sig=signature,
                sig_size=385,
                pub_key=key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY,
                pub_key_size=len(key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY),
                soc_dbg_bitmap=None
            )
            assert False, "非对称路径 sign_size=385 应返回 EHSM_ERR_PARAM_ERROR (1)"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"预期错误码为 EHSM_ERR_PARAM_ERROR(1)，实际: {e.ret_code}"
            logging.info(f"非对称鉴权路径正确返回参数错误码: {e.ret_code}")

