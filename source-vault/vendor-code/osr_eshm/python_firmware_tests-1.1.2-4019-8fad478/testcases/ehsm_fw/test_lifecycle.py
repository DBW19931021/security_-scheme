import allure
import pytest
from platform_adapter.api.constants import EhsmAuthAlgo, EhsmChallengeType, EhsmDrvMode, EhsmLifecycle
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_fw_errno
from platform_adapter.uart_lib import hostapi
from platform_adapter.uart_lib.hostapi import HSM_STATUS_IN
from utils import key, otp
import logging as log
from utils.config import cfg_data

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin( {"lifecycle": lc})

def _get_otp_bin_user_aes_128_cmac(lc: str) -> bytes:
    config = {
        "key" + str(cfg_data.TEST_USER_AUTH_KEY_ID): {
            "value" : key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "lifecycle" : lc
    }
    log.debug(config)
    return otp.otp_to_bin(config)

def _get_otp_bin_sm4_128_cmac(lc: str) -> bytes:
    config = {
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }
    if cfg_data.TEST_EHSM_DEBUG_KEY_ID != 0xFFFF:
        config["key" + str(cfg_data.TEST_EHSM_DEBUG_KEY_ID)] = {
                "value" : key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
                "level" : 1,
                "lifecycle" : "available",
                "type" : "symm",
            }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0 != 0xFFFF:
         config["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_0)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "symm",
        }
    if cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1 != 0xFFFF:
         config["key" + str(cfg_data.TEST_ROT_AUTH_KEY_HASH_ID_1)] = {
            "value": key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level": 1,
            "lifecycle": "available",
            "type": "symm",
        }
    log.debug(config)
    return otp.otp_to_bin(config)


def check_lifecycle()->EhsmLifecycle:
    addr = HSM_STATUS_IN
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

@allure.feature("lifecycle")
@allure.description("在Test模式下，修改lifecycle 为 Dev模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-816")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_816(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEV； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEVELOP)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_DEVELOP == check_lifecycle()


@allure.feature("lifecycle")
@allure.description("在Test模式下，修改lifecycle 为 Manu模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-817")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_817(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 MANU； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_MANUFACTURE)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_MANUFACTURE == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Test模式下，修改lifecycle 为 User模式")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif( cfg_data.TEST_EHSM_DEBUG_KEY_ID == 0xFFFF,reason="OTP功能密钥映射：eHSM Debug Key 功能无效")
@allure.testcase("EHSM-818")
def test_ehsm_818(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_sm4_128_cmac("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 USER； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("4、使用 get_challenge 接口，获取挑战字； # 4、读取成功，数据正确"):
        t,ret = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("5、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 5、计算成功"):
        ret, singnature = host.sign(ret,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("6、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 6、发送成功"):
        api.ehsm_bl_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("7、发送 change_lifecycle命令，修改为 USER； # 7、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("8、重启eHSM，并检查启动状态和生命周期； # 8、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_USER == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Test模式下，修改lifecycle 为 Debug模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-819")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_819(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEBUG； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_DEBUG == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Dev模式下，修改lifecycle 为 Manu模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-820")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_820(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 MANU； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_MANUFACTURE)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_MANUFACTURE == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Dev模式下，修改lifecycle 为 User模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-821")
@pytest.mark.skipif( cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0,reason="AES128CMAC 签名算法 功能不支持")
def test_ehsm_821(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_sm4_128_cmac("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 USER； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("4、使用 get_challenge 接口，获取挑战字； # 4、读取成功，数据正确"):
        t,ret = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("5、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 5、计算成功"):
        ret, singnature = host.sign(ret,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("6、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 6、发送成功"):
        api.ehsm_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("7、发送 change_lifecycle命令，修改为 USER； # 7、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("8、重启eHSM，并检查启动状态和生命周期； # 8、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_USER == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Dev模式下，修改lifecycle 为 Debug模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-822")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_822(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEBUG； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_DEBUG == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Manu模式下，修改lifecycle 为 User模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-823")
@pytest.mark.skipif( cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0,reason="AES128CMAC 签名算法 功能不支持")
def test_ehsm_823(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin_sm4_128_cmac("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 USER； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("4、使用 get_challenge 接口，获取挑战字； # 4、读取成功，数据正确"):
        t,ret = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG)
    with allure.step("5、调用上层接口对挑战字进行CMAC计算，SM4-128-CMAC算法使用预置密钥钥，计算CMAC； # 5、计算成功"):
        ret, singnature = host.sign(ret,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert 0 == ret
    with allure.step("6、将计算好的CMAC和公钥传入 debug_auth 接口，进行鉴权 # 6、发送成功"):
        api.ehsm_bl_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("7、发送 change_lifecycle命令，修改为 USER； # 7、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
    with allure.step("8、重启eHSM，并检查启动状态和生命周期； # 8、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_USER == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Manu模式下，修改lifecycle 为 Debug模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-824")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_824(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEBUG； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_DEBUG == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在User模式下，修改lifecycle 为 Debug模式")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-825")
@pytest.mark.skipif( cfg_data.TEST_FW_DBG_AES128CMAC_SUPPORT == 0,reason="AES128CMAC 签名算法 功能不支持")
def test_ehsm_825(setup_module):
    with allure.step("1、配置OTP 生命周期User模式； # 1、配置成功"):
        api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_WAIT_AND_POLL)
        api.ehsm_ctx_init(0, False)
        assert 0 == host.write_otp(_get_otp_bin_user_aes_128_cmac("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEBUG； # 3、user模式下修改生命周期为debug受到生命周期限制"):
        try:
            t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                log.info(f"修改生命周期为Debug时受到生命周期限制，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"修改生命周期为Debug时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"
        else:
            assert False, f"修改生命周期为Debug时未抛出异常，不符合预期"
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_USER == check_lifecycle()
    with allure.step("5、使用USER 鉴权，获取挑战字，并发送鉴权命令 # 5、读取成功，数据正确"):
        t,challenge = api.ehsm_get_challenge(EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH)

        ret, singnature = host.sign(challenge,
                                    EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH,
                                    EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC)
        assert ret == 0
        log.info(singnature)
        ret = api.ehsm_bl_debug_auth(
            EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH,
            EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC,
            sig=singnature,
            sig_size=len(singnature),
            pub_key=None,
            pub_key_size=0,
            soc_dbg_bitmap=None
        )
    with allure.step("6、发送 change_lifecycle命令，修改为 DEBUG； # 6、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)
    with allure.step("7、重启eHSM，并检查启动状态和生命周期； # 7、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_DEBUG == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Debug模式下，反向修改lifecycle 为 User模式")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-826")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_826(setup_module):
    with allure.step("1、配置OTP 生命周期Debug模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("debug"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 USER； # 3、debug模式修改为user模式返回预期的错误码"):
        try:
            t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_USER)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                log.info(f"Debug模式修改为User模式是不支持的，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Debug模式修改为User模式时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"
        else:
            assert False, f"Debug模式修改为User模式时未抛出异常，不符合预期"

@allure.feature("lifecycle")
@allure.description("在User模式下，反向修改lifecycle 为Manu模式")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-827")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_827(setup_module):
    with allure.step("1、配置OTP 生命周期User模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("user"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 Manu # 3、user模式修改为manu模式返回预期的错误码"):
        try:
            t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_MANUFACTURE)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                log.info(f"User模式修改为Manu模式是不支持的，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"User模式修改为Manu模式时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"
        else:
            assert False, f"User模式修改为Manu模式时未抛出异常，不符合预期"

@allure.feature("lifecycle")
@allure.description("在Manu模式下，反向修改lifecycle 为 Dev模式")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-828")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_828(setup_module):
    with allure.step("1、配置OTP 生命周期Manu模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("manu"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 DEV； # 3、manu模式修改为dev模式时返回预期的错误码"):
        try:
            t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEVELOP)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                log.info(f"Manu模式修改为Dev模式是不支持的，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Manu模式修改为Dev模式时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"
        else:
            assert False, f"Manu模式修改为Dev模式时未抛出异常，不符合预期"

@allure.feature("lifecycle")
@allure.description("在Dev模式下，反向修改lifecycle 为 Test模式")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-829")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_829(setup_module):
    with allure.step("1、配置OTP 生命周期Dev模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("dev"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 TEST； # 3、dev模式修改为test模式返回预期的错误码"):
        try:
            t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_TEST)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                log.info(f"Dev模式修改为Test模式是不支持的，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Dev模式修改为Test模式时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"
        else:
            assert False, f"Dev模式修改为Test模式时未抛出异常，不符合预期"

@allure.feature("lifecycle")
@allure.description("在Test模式下，对修改lifecycle 命令进行非法参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-830")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_830(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为错误的lifecycle； # 3、test模式下修改为错误生命周期时返回预期的错误码"):
        try:
            t = api.ehsm_change_lifecycle(0)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"Test模式下将生命周期修改为错误生命周期参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"Test模式下将生命周期修改为错误生命周期时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"Test模式下将生命周期修改为错误生命周期时未抛出异常，不符合预期"

@allure.feature("lifecycle")
@allure.description("在Test模式下，修改lifecycle 为 Destroy 模式（destroy 模式后无法正常启动）")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-1293")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_1293(setup_module):
    with allure.step("1、配置OTP 生命周期Test模式，调试鉴权算法密钥eHSM Debug Key配置SM4-128-CMAC算法密钥预置值和对应属性，其它默认； # 1、配置成功"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
    with allure.step("2、重启eHSM，并检查启动状态； # 2、重启成功，状态正常"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("3、发送 change_lifecycle命令，修改为 Destroy ； # 3、发送成功"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DESTORY)
    with allure.step("4、重启eHSM，并检查启动状态和生命周期； # 4、destory模式下重启下位机超时，符合预期"):
        assert 0 == host.reset_ehsm()
        try:
            assert 0 == host.wait_fw_done(1)
        except Exception as e:
            if "wait_fw_done timeout" in str(e):
                log.info(f"Destory模式下重启下位机超时，符合预期: 错误信息 {str(e)}")
            else:
                assert False, f"Destory模式下重启下位机超时返回了非预期的错误信息: {str(e)}，预期错误码: wait_fw_done timeout"
        else:
            assert False, f"Destory模式下重启下位机未超时，不符合预期"

        assert EhsmLifecycle.EHSM_LC_DESTORY == check_lifecycle()
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert EhsmLifecycle.EHSM_LC_TEST == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Test模式下，测试change_lifecycle接口传入非法lifecycle值（超出有效范围）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-l001")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_l001(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、change_lifecycle接口传入非法的lifecycle值（999）；# 2、报非法参数；"):
        # 使用无效的lifecycle值（假设值999不存在，有效值为1-6）
        try:
            t = api.ehsm_change_lifecycle(999)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"change_lifecycle接口传入非法lifecycle值时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"change_lifecycle接口传入非法lifecycle值返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, "change_lifecycle接口传入非法lifecycle值时未抛出异常，不符合预期"

    with allure.step("3、确认生命周期未改变； # 3、生命周期保持Test模式；"):
        assert EhsmLifecycle.EHSM_LC_TEST == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Test模式下，测试change_lifecycle接口传入超大lifecycle值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-l002")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_l002(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、change_lifecycle接口传入超大lifecycle值（0xFFFFFFFF）；# 2、报非法参数；"):
        # 使用超大值测试
        try:
            t = api.ehsm_change_lifecycle(0xFFFFFFFF)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"change_lifecycle接口传入超大lifecycle值时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"change_lifecycle接口传入超大lifecycle值返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, "change_lifecycle接口传入超大lifecycle值时未抛出异常，不符合预期"

    with allure.step("3、确认生命周期未改变； # 3、生命周期保持Test模式；"):
        assert EhsmLifecycle.EHSM_LC_TEST == check_lifecycle()

@allure.feature("lifecycle")
@allure.description("在Test模式下，测试change_lifecycle接口传入边界外lifecycle值")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-l003")
@pytest.mark.skipif(False, reason="生命周期管理功能始终启用")
def test_ehsm_l003(setup_module):
    with allure.step("1、配置生命周期为Test模式；并重启生效； # 1、生命周期配置成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、change_lifecycle接口传入边界外lifecycle值（7）；# 2、报非法参数；"):
        # 使用刚好超出有效范围的值（有效值为1-6，测试7）
        try:
            t = api.ehsm_change_lifecycle(7)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"change_lifecycle接口传入边界外lifecycle值时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"change_lifecycle接口传入边界外lifecycle值返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, "change_lifecycle接口传入边界外lifecycle值时未抛出异常，不符合预期"

    with allure.step("3、确认生命周期未改变； # 3、生命周期保持Test模式；"):
        assert EhsmLifecycle.EHSM_LC_TEST == check_lifecycle()
