import struct
import pytest
import allure
import logging as log
from platform_adapter.api.constants import EhsmDrvMode
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_fw_errno, hostapi
from utils import otp
from utils.otp import otp_to_bin_with_key_limit
from utils.config import cfg_data

api = get_api_interface()
host = get_host_interface()

# HSM_ERR_HW1 错误位定义 (bit 32-40 对应 HSM_ERR_HW1 的 bit 0-8)
# 写入地址: 0x30100144, 读取地址: 0x3010000C
HW_TRNG_HT_FAIL = 0x01 << 0       # bit 32: TRNG 健康测试失败
HW_TRNG_RETRY_FAIL = 0x01 << 1    # bit 33: TRNG 重试失败
HW_TRNG_RETRY_WARNING = 0x01 << 2 # bit 34: TRNG 重试警告
WDT_TIMEOUT = 0x01 << 8           # bit 40: 看门狗超时

# HSM_ERR_HW0 错误位定义 (MEM ECC 多bit错误)
# 写入地址: 0x30100140, 读取地址: 0x30100008
MEM_ECC_MB_IROM = 0x01 << 12      # bit 12: IROM ECC 错误
MEM_ECC_MB_IRAM = 0x01 << 13      # bit 13: IRAM ECC 错误
MEM_ECC_MB_DRAM = 0x01 << 14      # bit 14: DRAM ECC 错误
MEM_ECC_MB_KMU = 0x01 << 15       # bit 15: KMU ECC 错误
MEM_ECC_MB_PKE0 = 0x01 << 16      # bit 16: PKE0 ECC 错误
MEM_ECC_MB_PKE1 = 0x01 << 17      # bit 17: PKE1 ECC 错误
MEM_ECC_MB_PKE2 = 0x01 << 18      # bit 18: PKE2 ECC 错误
MEM_ECC_MB_PKE3 = 0x01 << 19      # bit 19: PKE3 ECC 错误

# HSM_ERR_HW0 错误位定义 (MEM ECC 1bit 错误)
# 写入地址: 0x30100140, 读取地址: 0x30100008
MEM_ECC_1B_IROM = 0x01 << 0       # bit 0: IROM ECC 1bit 错误
MEM_ECC_1B_IRAM = 0x01 << 1       # bit 1: IRAM ECC 1bit 错误
MEM_ECC_1B_DRAM = 0x01 << 2       # bit 2: DRAM ECC 1bit 错误
MEM_ECC_1B_KMU = 0x01 << 3        # bit 3: KMU ECC 1bit 错误
MEM_ECC_1B_PKE3 = 0x01 << 4       # bit 4: PKE3 ECC 1bit 错误
MEM_ECC_1B_PKE2 = 0x01 << 5       # bit 5: PKE2 ECC 1bit 错误
MEM_ECC_1B_PKE1 = 0x01 << 6       # bit 6: PKE1 ECC 1bit 错误
MEM_ECC_1B_PKE0 = 0x01 << 7       # bit 7: PKE0 ECC 1bit 错误

# CPU WFI 状态位
SYS_HSM_STA1_CPU_WIT_BIT = 0x2000000

OTP_BASE_ADDR = hostapi.OTP_BASE              # OTP基地址
OTP_FIELD_BASE = 0x070                  # OTP字段基础偏移
OTP_ROW = 0x0A                          # OTP行号
OTP_COL = 0x09                          # OTP列号
OTP_OFFSET = OTP_FIELD_BASE + 0x4 * (OTP_ROW * 2 + OTP_COL)  # OTP偏移
OTP_WRITE_VALUE = 0xEF                  # 写入OTP的值

HW_STATUS_REG_ADDR = 0x40010070         # 硬件状态寄存器地址
HW_CHECK_BIT = 3                        # 检查的bit位

MEM_ECC_REG_ADDR = 0x3010000C           # ECC寄存器地址
MEM_ECC_CHECK_BIT = 3                   # ECC检查的bit位

@allure.step("设置前三个OTP密钥为随机值")
def otp_set_first_three_keys_env(lifecycle: str = "test", key_alg_sel: str = "aes128"):
    """
    类似于 otp_set_random_key_env 接口的功能，用于设置前三个OTP密钥为随机值。
    使用 otp_to_bin_first_three_keys 接口生成OTP数据，然后使用 host.write_otp 接口写入。

    前三个密钥对应：
    - chip_root_key: 32字节随机值 + 32字节零填充
    - device_root_key: 32字节随机值 + 32字节零填充
    - key2: 32字节随机值 + 32字节零填充

    Args:
        lifecycle: eHSM生命周期模式字符串，可选值: "test", "dev", "manu", "user", "debug"，默认为"test"
        key_alg_sel: 密钥算法选择，可选值: "aes128", "sm4"，默认为"aes128"

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    # 生成三个32字节的随机密钥，每个密钥32字节+32字节零填充=64字节
    # 固定值替代 os.urandom(32).hex()
    chip_root_key = "2c22c1ea76260ef3f0b5f5c44284943600000000000000000000000000000000"  # 对应第一个数组
    device_root_key = "74707fec47c6ac3ce3cff91c1bec6d5d00000000000000000000000000000000"  # 对应第二个数组
    key2_value = "9feba879c596e5022c22c1ea76260ef39feba879c596e5022c22c1ea76260ef3"  # 对应第三个数组（重复模式）

    log.info(f"生成随机密钥（生命周期模式: {lifecycle.upper()}）:")
    log.info(f"  chip_root_key: {chip_root_key}")
    log.info(f"  device_root_key: {device_root_key}")
    log.info(f"  key2: {key2_value}")

    # 配置前三个密钥，并设置生命周期
    otp_config = {
        "lifecycle": lifecycle,  # 设置生命周期模式
        "chip_root_key": {
            "level": 1,
            "value": chip_root_key,
            "lifecycle": "available",
            "type": "symm"
        },
        "device_root_key": {
            "level": 1,
            "value": device_root_key,
            "lifecycle": "available",
            "type": "symm"
        },
        "key2": {
            "level": 1,
            "value": key2_value,
            "lifecycle": "available",
            "type": "symm"
        },
        "key_alg_sel": key_alg_sel  # 使用参数值而不是硬编码
    }

    try:
        # 使用专门的函数生成只包含前三个密钥的二进制数据
        otp_data = otp_to_bin_with_key_limit(otp_config, key_num=3)

        # 使用 host.write_otp 写入OTP
        result = host.write_otp(otp_data)

        if result == 0:
            log.info("前三个OTP密钥设置成功")
        else:
            log.error(f"前三个OTP密钥设置失败，错误码: {result}")

        return result

    except Exception as e:
        log.error(f"设置前三个OTP密钥时发生异常: {str(e)}")
        return -1


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

def status_print(status: bytes) -> None:
    if len(status) != 28:
        raise ValueError(f"expect 28 bytes, got {len(status)}")

    (
        o_hsm_status0,
        o_hsm_status1,
        o_hsm_err_sensor,
        o_hsm_err_hw0,
        o_hsm_err_hw1,
        o_hsm_err_fw0,
        o_hsm_err_fw1
    ) = struct.unpack('< I I I I I I I', status)
    log.debug(f"o_hsm_status0               : 0x{o_hsm_status0:08x}")
    log.debug(f"o_hsm_status1               : 0x{o_hsm_status1:08x}")
    log.debug(f"o_hsm_err_sensor            : 0x{o_hsm_err_sensor:08x}")
    log.debug(f"o_hsm_err_hw0               : 0x{o_hsm_err_hw0:08x}")
    log.debug(f"o_hsm_err_hw1               : 0x{o_hsm_err_hw1:08x}")
    log.debug(f"o_hsm_err_fw0               : 0x{o_hsm_err_fw0:08x}")
    log.debug(f"o_hsm_err_fw1               : 0x{o_hsm_err_fw1:08x}")

def emu_status_check_equal(status: bytes) -> None:
    if len(status) != 28:
        raise ValueError(f"expect 28 bytes, got {len(status)}")

    (
        o_hsm_status0,
        o_hsm_status1,
        o_hsm_err_sensor,
        o_hsm_err_hw0,
        o_hsm_err_hw1,
        o_hsm_err_fw0,
        o_hsm_err_fw1
    ) = struct.unpack('< I I I I I I I', status)
    _, host_status0 = host.read_memory(hostapi.HSM_STATUS_IN, 4)
    _, host_status1 = host.read_memory(hostapi.HSM_STATUS_IN1, 4)
    _, host_err_sensor = host.read_memory(hostapi.HSM_ERR_SENSOR_IN, 4)
    _, host_err_hw0 = host.read_memory(hostapi.HSM_ERR_HW0, 4)
    _, host_err_hw1 = host.read_memory(hostapi.HSM_ERR_HW1, 4)
    _, host_err_fw0 = host.read_memory(hostapi.HSM_ERR_FW0, 4)
    _, host_err_fw1 = host.read_memory(hostapi.HSM_ERR_FW1, 4)
    host_status0 = int.from_bytes(host_status0, 'little')
    host_status1 = int.from_bytes(host_status1, 'little')
    host_err_sensor = int.from_bytes(host_err_sensor, 'little')
    host_err_hw0 = int.from_bytes(host_err_hw0, 'little')
    host_err_hw1 = int.from_bytes(host_err_hw1, 'little')
    host_err_fw0 = int.from_bytes(host_err_fw0, 'little')
    host_err_fw1 = int.from_bytes(host_err_fw1, 'little')
    assert o_hsm_status0 == host_status0
    assert o_hsm_status1 == host_status1
    assert o_hsm_err_sensor == host_err_sensor
    assert o_hsm_err_hw0 == host_err_hw0
    assert o_hsm_err_hw1 == host_err_hw1
    assert o_hsm_err_fw0 == host_err_fw0
    assert o_hsm_err_fw1 == host_err_fw1

def get_cpu_wfi_status():
    """检查CPU WFI状态

    Returns:
        int: 0表示成功(检测到WFI状态), 1表示超时失败
    """
    for i in range(10):
        _, data = host.read_memory(hostapi.HSM_STATUS_IN, 4)
        status = int.from_bytes(data, 'little') & SYS_HSM_STA1_CPU_WIT_BIT
        if status:
            return 0  # EHSM_ERR_MB_SUCCESS

    return 1  # 超时错误

@allure.feature("emu")
@allure.description("检查 WFI 指令的响应，进入 WFI 后会出现 SOC_WFI 信号，再次发送命令后，WFI 信号清除")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-1294")
def test_ehsm_1294():
    with allure.step("1、判断 WFI 信号清除状态； # WFI 状态清除"):
        assert get_cpu_wfi_status() == 1
    with allure.step("2、使用命令进入 WFI 状态； # WFI 状态进入"):
        t=api.ehsm_enter_wfi()
        assert get_cpu_wfi_status() == 0
    with allure.step("3、发起mailbox 指令 get_emu； # 执行成功"):
        t, status=api.ehsm_get_emu_status()
        status_print(status)
    with allure.step("4、检查 WFI 状态是否退出； # WFI 状态清除"):
        assert get_cpu_wfi_status() == 1


@allure.feature("emu")
@allure.description("获取 EMU 状态")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-814")
def test_ehsm_814():
    with allure.step("1、发送 get_emu_status mailbox 指令；并打印状态值； # 发送成功"):
        t, status=api.ehsm_get_emu_status()
        status_print(status)
    with allure.step("2、检查获取的 EMU 状态值与 HOST 侧寄存器值是否一致； # 检查通过"):
        emu_status_check_equal(status)

@allure.feature("emu")
@allure.description("测试emu_addr为0时，get_emu指令能否正常执行")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-e026")
def test_ehsm_e026():
    with allure.step("1、判断 WFI 信号清除状态； # WFI 状态清除"):
        assert get_cpu_wfi_status() == 1
    with allure.step("2、使用命令进入 WFI 状态； # WFI 状态进入"):
        t=api.ehsm_enter_wfi()
        assert get_cpu_wfi_status() == 0
    with allure.step("3、发起mailbox 指令 get_emu； # 执行成功"):
        try:
            t, status=api.ehsm_get_emu_status(0)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"传入emu_addr为0进行验证验证失败，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"传入emu_addr为0进行验证返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS}"
        else:
            assert False, f"传入emu_addr为0进行验证未抛出异常，不符合预期"
    with allure.step("4、检查 WFI 状态是否退出； # WFI 状态清除"):
        assert get_cpu_wfi_status() == 1
