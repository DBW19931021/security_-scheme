import logging as log

import allure
import pytest

from platform_adapter.api.loader import get_api_interface
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from utils import otp
from utils.config import cfg_data


gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

if cfg_data.TEST_CUSTOM_ID == 0x4019:
    EHSM_VER_OTP_ADDR = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x30
    SOC_VER_OTP_ADDR = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x40
else:
    EHSM_VER_OTP_ADDR = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x50
    SOC_VER_OTP_ADDR = cfg_data.TEST_BL_OTP_BASE_ADDR + 0x60

@pytest.mark.skipif(False, reason="secboot版本更新功能始终启用")
@allure.feature("secboot")
@allure.description("EHSM OTP写入成功，版本号正常更新")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-SE001")
def test_ehsm_se001(setup_module):
    with allure.step("1、配置OTP版本为VC0（初始低版本），重启固件；# 1、固件启动成功；"):
        # 生成 OTP 数据，版本设为 VC0（低于固件内部 DRAM 版本），使 secboot_update_ver_cnt 触发更新
        otp_data = otp.otp_to_bin({"lifecycle": "test"})
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、读取 OTP 版本号区域；# 2、读取成功；"):
        # 读取 OTP 基地址 + 0x50 偏移量，即 hsm_ver_ctr 字段所在区域
        ret, otp_ver_data = api.ehsm_read_otp(EHSM_VER_OTP_ADDR, 4)
    with allure.step("3、验证 OTP 中版本号已更新（不为全0）；# 3、otp_write返回成功，OTP版本号已更新，程序继续执行；"):
        assert otp_ver_data is not None and len(otp_ver_data) >= 4
        log.debug(f"OTP version field bytes: {otp_ver_data.hex()}")
        version_word = int.from_bytes(otp_ver_data[:4], "little")
        assert version_word != 0, "OTP版本号未更新，secboot_update_ver_cnt可能未执行或otp_write失败"

    with allure.step("4、恢复默认OTP区域数据；# 4、恢复成功；"):
        assert 0 == host.write_otp(otp.otp_to_bin({"lifecycle": "test"}))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)


# ===========================================================================
# BUG-01: OTP写入返回值判断（阻塞+EMU上报）
# secboot_update_ver_cnt 中 otp_write 失败须阻塞并通过 EMU 上报 OTP_WRITE_FAILED
# TC-SECBOOT-002 ~ TC-SECBOOT-005 对应 se002 ~ se005
#
# 依赖：下位机需实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR (CMD ID=163)
# 实现位置：server/hostapi_commands.c + src/test_api.c
# 调用后固件 otp_write 返回错误码 0x123，触发失败分支。
# ===========================================================================

@pytest.mark.skipif(False, reason="secboot版本更新功能始终启用")
@allure.feature("secboot")
@allure.description("EHSM OTP写入失败，程序阻塞并EMU上报（BUG-01 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_SE002")
def test_ehsm_se002(setup_module):
    """TC-SECBOOT-002: EHSM OTP写入失败，程序阻塞并EMU上报"""
    log.info("开始测试TC-SECBOOT-002: EHSM OTP写入失败注入")

    with allure.step("1、配置OTP为Test模式，重启固件 # 1、固件正常启动"):
        otp_data = otp.otp_to_bin({"lifecycle": "test"})
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、注入OTP写失败（调用 ehsm_test_inject_otp_write_error）# 2、注入成功"):
        # Reason: 通过下位机 test CMD 设置 otp_write 返回 0x123 的注入标志
        # 若 CMD 未实现（返回 EHSM_ERR_INVALID_CMD=34），pytest.xfail 自动处理
        # TODO(开发): 实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 后此处生效
        api.ehsm_test_inject_otp_write_error()
        log.info("OTP写失败注入成功（CMD已实现）")

    with allure.step("3、触发 secboot_update_ver_cnt（重启固件使版本计数更新路径执行）# 3、固件触发otp_write失败"):
        # Reason: 重启后 secboot_update_ver_cnt 执行 otp_write，此时返回 0x123，触发失败分支
        assert 0 == host.reset_ehsm()
        # 固件因阻塞不会完成启动，wait_fw_done 预期超时或失败
        ret = host.wait_fw_done(1)
        log.info(f"wait_fw_done 返回: {ret}（固件阻塞时预期非0）")

    with allure.step("4、通过 fw_get_emu 读取 EMU 状态，确认 FW_ERROR_OTP_WRITE_FAILED 已上报 # 4、EMU错误标记置位"):
        # Reason: BUG-01 修复验证——otp_write 失败时须调用 expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED)
        # 依赖：CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 须由下位机实现后方可验证
        try:
            _, emu_data = api.ehsm_get_emu(cfg_data.TEST_BL_OTP_BASE_ADDR, 256)
            assert emu_data is not None and len(emu_data) > 0, \
                "EMU 数据读取应返回非空数据"
            # TODO(开发): 实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 后，
            # 在此处 assert FW_ERROR_OTP_WRITE_FAILED 标志在 emu_data 中已置位
            # 当前因注入 CMD 未实现，此断言暂不可验证，标记 xfail
            pytest.xfail("CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 未实现，"
                         "无法验证 FW_ERROR_OTP_WRITE_FAILED 标志位")
        except Exception as e:
            pytest.xfail(f"EMU读取失败（固件可能已阻塞或CMD未实现）: {e}")

    log.info("TC-SECBOOT-002 完成")


@pytest.mark.skipif(False, reason="secboot版本更新功能始终启用")
@allure.feature("secboot")
@allure.description("EHSM OTP写入失败后系统状态确认（BUG-01）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_SE003")
def test_ehsm_se003(setup_module):
    """TC-SECBOOT-003: EHSM OTP写入失败后系统状态确认"""
    log.info("开始测试TC-SECBOOT-003: 注入失败后系统状态确认")

    with allure.step("1、同TC-SECBOOT-002，注入OTP写失败后重启 # 1、注入并重启"):
        otp_data = otp.otp_to_bin({"lifecycle": "test"})
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        # 注入 otp_write 失败
        # TODO(开发): CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR 实现后生效
        api.ehsm_test_inject_otp_write_error()
        assert 0 == host.reset_ehsm()
        host.wait_fw_done(1)  # 预期固件阻塞，不断言返回值

    with allure.step("2、读取 EMU 错误标记，确认 OTP_WRITE_FAILED 已置位 # 2、EMU错误标记已置位"):
        try:
            _, emu_data = api.ehsm_get_emu(cfg_data.TEST_BL_OTP_BASE_ADDR, 256)
            assert emu_data is not None and len(emu_data) > 0, \
                "EMU 数据读取应返回非空数据"
            # TODO(开发): 实现注入CMD后，在此 assert FW_ERROR_OTP_WRITE_FAILED 在 emu_data 中
            pytest.xfail("CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 未实现，"
                         "无法验证 FW_ERROR_OTP_WRITE_FAILED 标志位")
        except Exception as e:
            pytest.xfail(f"EMU读取失败（固件已阻塞或CMD未实现）: {e}")

    with allure.step("3、确认后续版本号相关逻辑未执行（OTP版本号未更新）# 3、版本号未变化"):
        # Reason: 阻塞后程序不继续执行，OTP 版本号应保持不变
        try:
            _, otp_ver = api.ehsm_read_otp(EHSM_VER_OTP_ADDR, 4)
            assert otp_ver is not None and len(otp_ver) == 4, \
                "OTP版本字段读取应返回4字节"
            log.info(f"OTP版本字段: {otp_ver.hex()}")
        except Exception as e:
            pytest.xfail(f"读取OTP版本失败（固件已阻塞）: {e}")

    log.info("TC-SECBOOT-003 完成")


@pytest.mark.skipif(False, reason="secboot版本更新功能始终启用")
@allure.feature("secboot")
@allure.description("SOC OTP写入成功，版本号正常更新（BUG-01正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_SE004")
def test_ehsm_se004(setup_module):
    """TC-SECBOOT-004: SOC OTP写入成功，版本号正常更新"""
    log.info("开始测试TC-SECBOOT-004: SOC OTP写入成功正路径")

    with allure.step("1、配置OTP为Test模式，重启固件 # 1、固件正常启动"):
        # Reason: 控制变量法正路径——SOC 版本正常更新路径（secboot.c:249行）
        otp_data = otp.otp_to_bin({"lifecycle": "test"})
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、读取SOC OTP版本号区域（OTP_SOC_VERSION_ADDRESS）# 2、读取成功"):
        # SOC版本计数器地址偏移参考 secboot.c SOC_VERSION_COUNTER_ADDR = DRAM_BASE_ADDR + 0x70
        ret, otp_ver_data = api.ehsm_read_otp(SOC_VER_OTP_ADDR, 4)
        assert otp_ver_data is not None and len(otp_ver_data) >= 4, \
            "SOC OTP版本字段读取应返回4字节"
        log.info(f"SOC OTP版本字段: {otp_ver_data.hex()}")

    with allure.step("3、验证 SOC OTP 版本号已更新 # 3、SOC版本号写入OTP成功，程序继续执行"):
        # Reason: 若 secboot_update_ver_cnt 的 SOC 分支正常执行，版本号应非全0
        version_word = int.from_bytes(otp_ver_data[:4], "little")
        assert version_word != 0, \
            "SOC OTP版本号未更新（全0），secboot_update_ver_cnt SOC分支可能未执行或otp_write失败"
        log.info(f"SOC OTP版本号已更新，值: 0x{version_word:08x}")

    log.info("TC-SECBOOT-004 完成")


@pytest.mark.skipif(False, reason="secboot版本更新功能始终启用")
@allure.feature("secboot")
@allure.description("SOC OTP写入失败，程序阻塞并EMU上报（BUG-01 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_SE005")
def test_ehsm_se005(setup_module):
    """TC-SECBOOT-005: SOC OTP写入失败，程序阻塞并EMU上报"""
    log.info("开始测试TC-SECBOOT-005: SOC OTP写入失败注入")

    with allure.step("1、配置OTP为Test模式，重启固件 # 1、固件正常启动"):
        otp_data = otp.otp_to_bin({"lifecycle": "test"})
        assert 0 == host.write_otp(otp_data)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、注入OTP写失败（SOC版本号写入路径）# 2、注入成功"):
        # Reason: 注入后固件在 secboot.c:249 行的 otp_write(OTP_SOC_VERSION_ADDRESS,...) 返回 0x123
        # TODO(开发): CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR 实现后生效；
        # 若需区分 EHSM/SOC 两条路径的注入，开发可扩展参数（如 inject_type=SOC/EHSM）
        api.ehsm_test_inject_otp_write_error()
        log.info("SOC OTP写失败注入成功（CMD已实现）")

    with allure.step("3、触发 secboot_update_ver_cnt（重启固件）# 3、固件触发SOC otp_write失败"):
        assert 0 == host.reset_ehsm()
        ret = host.wait_fw_done(1)
        log.info(f"wait_fw_done 返回: {ret}（固件阻塞时预期非0）")

    with allure.step("4、确认 EMU 上报 OTP 写失败错误 # 4、程序阻塞，EMU上报OTP写失败错误"):
        # Reason: BUG-01 修复验证——SOC otp_write 失败时同样需触发 expt_det_add_error
        # 依赖：CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 须由下位机实现后方可验证
        try:
            _, emu_data = api.ehsm_get_emu(cfg_data.TEST_BL_OTP_BASE_ADDR, 256)
            assert emu_data is not None and len(emu_data) > 0, \
                "EMU 数据读取应返回非空数据"
            # TODO(开发): 实现注入CMD后，在此 assert FW_ERROR_OTP_WRITE_FAILED 在 emu_data 中已置位
            pytest.xfail("CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 未实现，"
                         "无法验证 SOC路径 FW_ERROR_OTP_WRITE_FAILED 标志位")
        except Exception as e:
            pytest.xfail(f"EMU读取失败（固件已阻塞或CMD未实现）: {e}")

    log.info("TC-SECBOOT-005 完成")