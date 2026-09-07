import logging as log
import struct
import time
import pytest
import allure
from platform_adapter.api.constants import EhsmDrvMode
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from utils.config import cfg_data
from utils import otp
from utils.watchdog_helper import calculate_watchdog_timeout, verify_watchdog_timeout

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()


@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")


@allure.feature("watchdog")
@allure.description(
    "发送进入循环指令的命令，观察看门狗是否正常超时，并响应超时中断，触发异常错误，计算看门狗超时时间"
)
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-W001")
def test_watchdog_w001(setup_module):
    with allure.step("进入loop指令会导致下位机不响应指令，需要server配置异步和中断模式 # 配置成功"):
        api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_INTERRUPT)
        api.ehsm_ctx_init(0, True)
    with allure.step("先检查看门狗错误码是否为0 # 检查通过"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert host.check_watchdog_error_status() == False

    with allure.step("发送固件进入循环指令的命令 # 发送成功"):
        try:
            t = api.ehsm_test_jump_to_loop()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("等待下位机超时中，请打开串口观察")
            else:
                log.debug(f"错误码{e.ret_code}")
    with allure.step("200ms 看门狗超时时间，检查看门狗是否触发超时，并反馈错误码 # 检查通过"):
        start_time = time.time()
        while time.time() - start_time < 30:
            if time.time() - start_time >= 0.2:
                status = host.check_watchdog_error_status()
                if status:
                    log.info(f"看门狗超时触发，超时时间为{(time.time()-start_time)}")
                    break
            time.sleep(0.01)  # 每 10ms 检查一次，可根据需求调整


@allure.feature("watchdog")
@allure.description("检查WFI指令是否生效")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-W002")
def test_watchdog_w002(setup_module):
    with allure.step("先检查CPU WFI 状态是否为0 # 检查通过"):
        api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_INTERRUPT)
        api.ehsm_ctx_init(0, True)
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        assert host.check_cpu_wfi_status() == False
    with allure.step("发送进入循环指令的命令，观察看门狗是否正常超时，并响应超时中断，触发异常错误，计算看门狗超时时间 # 发送成功"):
        try:
            t = api.ehsm_enter_wfi()
        except hostapi.HostApiError as e:
            if  1437187020 == e.ret_code:
                log.debug("进入中断")
            else:
                log.debug(f"错误码{e.ret_code}")
        time.sleep(1.1)
    with allure.step("检查CPU WFI 状态 # 检查通过"):
        start_time = time.time()
        while time.time() - start_time < 30:
            if time.time() - start_time >= 0.2:
                status = host.check_cpu_wfi_status()
                if status:
                    log.info(f"看门狗超时触发，超时时间为{(time.time()-start_time)}")
                    break
            time.sleep(0.01)  # 每 10ms 检查一次，可根据需求调整
        assert host.check_cpu_wfi_status() == True


# =====================================================================
# 2.4版本看门狗测试 - OTP配置辅助函数
# =====================================================================

def _get_otp_bin_wdt_v24(n: int, lc: str = "test") -> bytes:
    """
    生成2.4版本看门狗配置的OTP二进制数据

    Args:
        n: 看门狗级别 (0-15，其中0表示disable)
        lc: 生命周期，默认"test"

    Returns:
        bytes: OTP二进制数据

    Raises:
        ValueError: 如果N值超出范围
    """
    # Reason: N值用4bit表示，范围必须是0-15
    if not 0 <= n <= 15:
        raise ValueError(f"N值必须在0-15范围内，当前值: {n}")

    # Reason: N=0对应disable（关闭看门狗），N>=1对应level1-level15
    if n == 0:
        return otp.otp_to_bin({
            "watchdog_level": "disable",
            "lifecycle": lc
        })
    else:
        return otp.otp_to_bin({
            "watchdog_level": f"level{n}",
            "lifecycle": lc
        })


def _get_otp_bin_wdt_disable(lc: str = "test") -> bytes:
    """
    生成关闭看门狗的OTP配置

    Args:
        lc: 生命周期，默认"test"

    Returns:
        bytes: OTP二进制数据
    """
    return otp.otp_to_bin({
        "watchdog_level": "disable",
        "lifecycle": lc
    })


# =====================================================================
# 2.4版本看门狗测试 - P0核心用例（FW层）
# =====================================================================

# =====================================================================
# 2.4版本看门狗测试 - P1边界值用例（FW层）
# =====================================================================
