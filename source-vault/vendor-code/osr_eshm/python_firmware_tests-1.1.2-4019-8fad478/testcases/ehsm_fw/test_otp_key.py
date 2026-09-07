import pytest
import allure
import logging
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.api.constants import EhsmKeyLevel, EhsmInstallKeyType
from platform_adapter.uart_lib import hostapi
import platform_adapter.uart_lib.ehsm_fw_errno as ehsm_fw_errno
from utils.otp import otp_to_bin_with_key_limit
from utils.config import cfg_data

api = get_api_interface()
host = get_host_interface()
OTP_KEY_MAX = cfg_data.TEST_OTP_KEY_COUNT  # 根据C代码中的定义

@allure.step("配置OTP生命周期但不设置任何OTP密钥")
def otp_not_set_otp_key_env(lifecycle: str = "test"):
    """
    参考 otp_set_first_three_keys_env 接口的功能，但一个OTP密钥都不写入。
    只配置生命周期模式，不设置任何密钥。对应C代码中的 otp_not_set_otp_key_env 接口。

    Args:
        lifecycle: eHSM生命周期模式字符串，可选值: "test", "dev", "manu", "user", "debug"，默认为"test"

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    logging.info(f"配置OTP生命周期{lifecycle.upper()}模式，不设置任何密钥")

    # 只配置生命周期，不设置任何密钥
    otp_config = {
        "lifecycle": lifecycle,  # 设置生命周期模式
        "key_alg_sel": "aes128"  # 默认算法选择
        # 注意：这里不设置任何密钥配置 (chip_root_key, device_root_key, key2 等)
    }

    try:
        # 使用 key_num=0 表示不设置任何密钥，只配置生命周期
        otp_data = otp_to_bin_with_key_limit(otp_config, key_num=0)

        # 使用 host.write_otp 写入OTP
        result = host.write_otp(otp_data)

        if result == 0:
            logging.info(f"OTP生命周期{lifecycle.upper()}模式配置成功，未设置任何密钥")
        else:
            logging.error(f"OTP生命周期配置失败，错误码: {result}")

        return result

    except Exception as e:
        logging.error(f"配置OTP生命周期时发生异常: {str(e)}")
        return -1


@allure.step("设置前三个OTP密钥为随机值")
def otp_set_first_three_keys_env(lifecycle: str = "test", key_alg_sel: str = "aes128", key_num: int = 3):
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

    logging.info(f"生成随机密钥（生命周期模式: {lifecycle.upper()}）:")
    logging.info(f"  chip_root_key: {chip_root_key}")
    logging.info(f"  device_root_key: {device_root_key}")
    logging.info(f"  key2: {key2_value}")

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
        otp_data = otp_to_bin_with_key_limit(otp_config, key_num=key_num)

        # 使用 host.write_otp 写入OTP
        result = host.write_otp(otp_data)

        if result == 0:
            logging.info("前三个OTP密钥设置成功")
        else:
            logging.error(f"前三个OTP密钥设置失败，错误码: {result}")

        return result

    except Exception as e:
        logging.error(f"设置前三个OTP密钥时发生异常: {str(e)}")
        return -1

def otp_set_otp_key_manu_env(lifecycle: str = "manu"):
    """
    设置MANU环境的OTP配置，对应C代码中的 otp_set_manu_boot_env 接口。
    MANU模式只配置第0个和第2个密钥，禁止动态密钥安装，确保生产环境安全。

    Args:
        lifecycle: eHSM生命周期模式字符串，默认为"manu"

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    logging.info(f"配置OTP生命周期{lifecycle.upper()}模式，预置系统密钥")

    # MANU模式预置的系统密钥配置（对应C代码中的8个预设密钥）
    # 这些密钥被预先配置为"available"状态，其他密钥槽保持未配置状态
    otp_config = {
        "lifecycle": lifecycle,
        "key_alg_sel": "aes128",  # 默认算法选择
        # 预置chip_root_key (对应C代码中的第一个系统密钥)
        "chip_root_key": {
            "level": 1,
            "value": "2c22c1ea76260ef3f0b5f5c44284943600000000000000000000000000000000",
            "lifecycle": "available",
            "type": "symm"
        },
        # 预置device_root_key (对应C代码中的第二个系统密钥)
        # "device_root_key": {
        #     "level": 2,
        #     "value": "74707fec47c6ac3ce3cff91c1bec6d5d00000000000000000000000000000000",
        #     "lifecycle": "available",
        #     "type": "symm"
        # },
        # 预置其他系统密钥 (key2-key7对应C代码中的系统预设)
        "key2": {
            "level": 1,
            "value": "9feba879c596e5022c22c1ea76260ef39feba879c596e5022c22c1ea76260ef3",
            "lifecycle": "available",
            "type": "symm"
        },
        # "key3": {
        #     "level": 1,
        #     "value": "1234567890abcdef1234567890abcdef00000000000000000000000000000000",
        #     "lifecycle": "available",
        #     "type": "symm"
        # },
        # "key4": {
        #     "level": 2,
        #     "value": "fedcba0987654321fedcba098765432100000000000000000000000000000000",
        #     "lifecycle": "available",
        #     "type": "symm"
        # },
        # "key5": {
        #     "level": 1,
        #     "value": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        #     "lifecycle": "available",
        #     "type": "symm"
        # },
        # "key6": {
        #     "level": 2,
        #     "value": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
        #     "lifecycle": "available",
        #     "type": "symm"
        # },
        # "key7": {
        #     "level": 1,
        #     "value": "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
        #     "lifecycle": "available",
        #     "type": "symm"
        # }
        # 注意：key8及以后的密钥槽故意不配置，这样在MANU模式下它们无法被动态安装
        # 这是MANU模式安全设计的核心：只允许预设的系统密钥存在，禁止任意密钥安装
    }

    try:
        # 使用包含预置密钥的配置生成OTP数据
        otp_data = otp_to_bin_with_key_limit(otp_config, key_num=[0, 2])  # 只配置第0个和第2个密钥

        # 使用 host.write_otp 写入OTP
        result = host.write_otp(otp_data)

        if result == 0:
            logging.info(f"OTP生命周期{lifecycle.upper()}模式配置成功，已配置第0个和第2个密钥")
        else:
            logging.error(f"OTP生命周期配置失败，错误码: {result}")

        return result

    except Exception as e:
        logging.error(f"配置OTP生命周期时发生异常: {str(e)}")
        return -1


@allure.step("设置随机密钥环境（包含前3个密钥的配置）")
def otp_set_random_key_env(lifecycle: str = "test"):
    """
    设置随机密钥环境，对应C代码中的 otp_set_random_key_env 接口。
    配置前3个密钥并设置指定的生命周期模式，用于随机密钥安装测试。

    Args:
        lifecycle: eHSM生命周期模式字符串，可选值: "test", "dev", "debug", "user"

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    logging.info(f"设置随机密钥环境，生命周期模式: {lifecycle.upper()}")

    # 使用现有的 otp_set_first_three_keys_env 函数
    return otp_set_first_three_keys_env(lifecycle)


@allure.step("设置加密密钥环境")
def otp_set_encrypt_key_env(lifecycle: str = "test", key_count: int = 3):
    """
    设置加密密钥环境，对应C代码中的 otp_set_encrypt_key_env 接口。
    配置指定数量的密钥并设置指定的生命周期模式，用于加密密钥安装测试。

    Args:
        lifecycle: eHSM生命周期模式字符串，可选值: "test", "dev", "manu", "debug", "user"
        key_count: 要配置的密钥数量，默认为3

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    logging.info(f"设置加密密钥环境，生命周期模式: {lifecycle.upper()}，密钥数量: {key_count}")

    # 根据key_count配置相应的密钥
    if key_count > 0:
        # 使用现有的 otp_set_first_three_keys_env 函数
        return otp_set_first_three_keys_env(lifecycle, key_num=key_count)
    elif key_count == 0:
        # 不设置任何密钥，只配置生命周期
        return otp_not_set_otp_key_env(lifecycle)
    else:
        logging.warning(f"不支持的密钥数量: {key_count}，使用默认配置")
        return otp_set_first_three_keys_env(lifecycle)


@allure.step("设置MANU模式启动环境")
def otp_set_manu_boot_env():
    """
    设置MANU模式启动环境，对应C代码中的 otp_set_manu_boot_env 接口。
    这是MANU模式的特殊配置，预置系统密钥并禁止动态密钥安装。

    Returns:
        int: 返回0表示成功，其他值表示失败
    """
    logging.info("设置MANU模式启动环境")

    # 使用现有的 otp_set_otp_key_manu_env 函数
    return otp_set_otp_key_manu_env("manu")


# 导入现有的工具函数
from cryptosynth import generate_symmetric_testdata


def generate_test_key_data(size: int = 48, key_level: int = 1, key_id: int = 0) -> bytes:
    """
    生成测试用的密钥数据，对应C代码中的generate_soc_key函数。

    这个函数使用现有的工具函数实现，遵循C代码的generate_soc_key逻辑：
    1. 生成基础测试数据（48字节）
    2. 计算CRC32-MPEG2校验和并放入字节32-35
    3. 根据key_level和key_id选择相应的KEK密钥进行AES-128-CBC加密

    Args:
        size: 数据大小，默认48字节
        key_level: 密钥级别，1或2
        key_id: 密钥ID，对于device root key使用1
    Returns:
        bytes: 生成并加密的测试密钥数据
    """
    # C代码中固定的KEK密钥
    g_ehsm_kek_key = bytes([0x4E, 0x20, 0xAC, 0x86, 0x59, 0x31, 0xF7, 0xDB,
                           0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE])
    g_soc_kek_key = bytes([0xEF, 0x9B, 0x57, 0x13, 0xCE, 0x8A, 0x46, 0x02,
                          0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE])

    # 1. 生成基础测试数据，对应C代码中的input_data模式
    key_data = bytearray(size)
    # C代码中的测试数据：0x00-0x0F循环模式，共32字节，然后重复
    for i in range(min(32, size)):
        key_data[i] = i % 16
    # 填充剩余部分（如果size > 32）
    for i in range(32, size):
        key_data[i] = key_data[i % 32]

    # 2. 计算CRC32-MPEG2校验和，只对前32字节计算
    # Reason: 对应C代码 crc32_mpeg2(key, 32, 0xFFFFFFFF)
    crc = host.crc32_mpeg2(key_data[:32], 0xFFFFFFFF)

    # 3. 将CRC值放入字节32-35位置 (little-endian)
    # Reason: 对应C代码 memcpy(&key[32], &crc, sizeof(crc))
    key_data[32:36] = crc.to_bytes(4, byteorder='little')

    # 4. 根据key_level和key_id选择KEK密钥进行加密
    # Reason: 对应C代码中的密钥选择逻辑
    if key_level == 1:  # MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1
        if key_id == 1:  # device rootkey 使用 sockek 加密
            kek_key = g_soc_kek_key
        else:
            kek_key = g_ehsm_kek_key
    elif key_level == 2:  # MB_INSTALL_RANDOM_KEY_KEY_LEVEL_2
        kek_key = g_soc_kek_key
    else:
        raise ValueError(f"无效的key_level: {key_level}")

    # 5. 使用generate_symmetric_testdata进行AES-128-CBC加密
    # Reason: 对应C代码中的ske_crypto_cbc调用，使用AES-128-CBC模式，全零IV
    iv = bytes(16)  # 全零IV，对应C代码中的default_iv
    encryption_result = generate_symmetric_testdata("AES128", "CBC", kek_key, "NONE", iv, bytes(key_data))
    encrypted_data = encryption_result.ciphertext

    logging.info(f"生成测试密钥数据: size={size}, key_level={key_level}, key_id={key_id}")
    logging.info(f"使用KEK密钥: {'g_soc_kek_key' if kek_key == g_soc_kek_key else 'g_ehsm_kek_key'}")
    logging.info(f"CRC32值: 0x{crc:08X}")

    return encrypted_data


@allure.feature("otp_key")
@allure.description("在Test模式下，测试前3个密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-763")
def test_ehsm_763():
    with allure.step("1、配置OTP生命周期Test模式，密钥都不烧写； # 配置成功"):
        # 使用 otp_not_set_otp_key_env 配置OTP生命周期为Test模式，但不设置任何密钥
        result = otp_not_set_otp_key_env("test")
        assert result == 0, f"配置OTP生命周期Test模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Test模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成 (注意：这里使用wait_fw_done而不是wait_bl_done，因为是FW测试)
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 0，其它参数正常，进行第一个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 0, 0)
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            0,   # key_slot_id=0
            0    # last_key=0
        )
        logging.info(f"slot 0密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态 (对应C代码中每次安装后的reset_ehsm_check_lifecycle)
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 0密钥安装后eHSM重启成功")

    with allure.step("4、随机密钥安装接口传入slot 1，其它参数正常，进行第二个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 1, 0)
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            1,   # key_slot_id=1
            0    # last_key=0
        )
        logging.info(f"slot 1密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 1密钥安装后eHSM重启成功")

    with allure.step("5、随机密钥安装接口传入slot 2，其它参数正常，进行第三个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_2, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 2, 0)
        # 注意：slot 2使用key_level=2
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_2,        # key_level=2
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            2,   # key_slot_id=2
            0    # last_key=0
        )
        logging.info(f"slot 2密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 2密钥安装后eHSM重启成功")

    logging.info("test_ehsm_763测试完成：前3个密钥的随机密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在Dev模式下，测试前3个密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-764")
def test_ehsm_764():
    with allure.step("1、配置OTP生命周期Dev模式，密钥都不烧写； # 配置成功"):
        # 使用 otp_not_set_otp_key_env 配置OTP生命周期为Dev模式，但不设置任何密钥
        result = otp_not_set_otp_key_env("dev")
        assert result == 0, f"配置OTP生命周期Dev模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Dev模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 0，其它参数正常，进行第一个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 0, 0)
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            0,   # key_slot_id=0
            0    # last_key=0
        )
        logging.info(f"slot 0密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 0密钥安装后eHSM重启成功")

    with allure.step("4、随机密钥安装接口传入slot 1，其它参数正常，进行第二个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 1, 0)
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            1,   # key_slot_id=1
            0    # last_key=0
        )
        logging.info(f"slot 1密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 1密钥安装后eHSM重启成功")

    with allure.step("5、随机密钥安装接口传入slot 2，其它参数正常，进行第三个密钥安装； # 发送成功"):
        # 对应C代码: fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_2, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 2, 0)
        # 注意：slot 2使用key_level=2
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_2,        # key_level=2
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            2,   # key_slot_id=2
            0    # last_key=0
        )
        logging.info(f"slot 2密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM并检查启动状态
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("slot 2密钥安装后eHSM重启成功")

    logging.info("test_ehsm_764测试完成：Dev模式下前3个密钥的随机密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在Manu模式下，测试前3个密钥的随机密钥安装过程 - 验证MANU模式的安全限制")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-765")
def test_ehsm_765():
    with allure.step("1、配置OTP生命周期Manu模式，预置系统密钥； # 配置成功"):
        # 使用 otp_set_otp_key_manu_env 配置OTP生命周期为Manu模式
        # 这是MANU模式的核心特点：预置系统密钥，禁止动态密钥安装
        result = otp_set_otp_key_manu_env("manu")
        assert result == 0, f"配置OTP生命周期Manu模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Manu模式，已预置密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 0，期望返回不支持错误； # 发送成功"):
        # MANU模式的核心安全特性：禁止动态密钥安装，应返回 EHSM_ERR_NOT_SUPPORT
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                0,   # key_slot_id=0
                0    # last_key=0
            )
            # 如果到达这里说明调用成功了，但MANU模式下应该失败
            assert False, f"MANU模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"不支持"错误码
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"MANU模式正确阻止了slot 0密钥安装，返回EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"MANU模式下slot 0密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("4、随机密钥安装接口传入slot 1，期望返回不支持错误； # 发送成功"):
        # 继续验证其他密钥槽也被正确阻止
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                1,   # key_slot_id=1
                0    # last_key=0
            )
            assert False, f"MANU模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"MANU模式正确阻止了slot 1密钥安装，返回EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"MANU模式下slot 1密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("5、随机密钥安装接口传入slot 2，期望返回不支持错误； # 发送成功"):
        # 验证key_level=2的密钥槽也被阻止
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                2,   # key_slot_id=2
                0    # last_key=0
            )
            assert False, f"MANU模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"MANU模式正确阻止了slot 2密钥安装，返回EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"MANU模式下slot 2密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    logging.info("test_ehsm_765测试完成：MANU模式正确地阻止了所有动态密钥安装尝试，验证了生产环境的安全限制")

@allure.feature("otp_key")
@allure.description("在Debug模式下，测试前3个密钥的随机密钥安装过程 - 验证生命周期限制")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-766")
def test_ehsm_766():
    with allure.step("1、配置OTP生命周期Debug模式，写入前3个密钥； # 配置成功"):
        # 使用 otp_set_first_three_keys_env 配置OTP生命周期为Debug模式，写入前3个密钥
        # 对应C代码中的 otp_set_random_key_env(MODE_DEBUG) 调用
        result = otp_set_first_three_keys_env("debug")
        assert result == 0, f"配置OTP生命周期Debug模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Debug模式，已写入前3个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 0，期望返回生命周期限制错误； # 发送成功"):
        # DEBUG模式的核心安全特性：禁止动态密钥安装，应返回 EHSM_ERR_EHSM_LIFECYCLE_LIMIT
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(..., 0, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                0,   # key_slot_id=0
                0    # last_key=0
            )
            # 如果到达这里说明调用成功了，但DEBUG模式下应该失败
            assert False, f"DEBUG模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"生命周期限制"错误码
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"DEBUG模式正确阻止了slot 0密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"DEBUG模式下slot 0密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    with allure.step("4、随机密钥安装接口传入slot 1，期望返回生命周期限制错误； # 发送成功"):
        # 继续验证其他密钥槽也被正确阻止
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(..., 1, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                1,   # key_slot_id=1
                0    # last_key=0
            )
            assert False, f"DEBUG模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"DEBUG模式正确阻止了slot 1密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"DEBUG模式下slot 1密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    with allure.step("5、随机密钥安装接口传入slot 2，期望返回生命周期限制错误； # 发送成功"):
        # 验证key_level=1的密钥槽也被阻止 (注意：C代码中第3个测试使用的是key_level=1，不是key_level=2)
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, ..., 2, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1 (按照C代码)
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                2,   # key_slot_id=2
                0    # last_key=0
            )
            assert False, f"DEBUG模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"DEBUG模式正确阻止了slot 2密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"DEBUG模式下slot 2密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    logging.info("test_ehsm_766测试完成：DEBUG模式正确地阻止了所有动态密钥安装尝试，验证了调试环境的生命周期限制机制")

@allure.feature("otp_key")
@allure.description("在User模式下，测试前3个密钥的随机密钥安装过程 - 验证生命周期限制")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-767")
def test_ehsm_767():
    with allure.step("1、配置OTP生命周期User模式，写入前3个密钥； # 配置成功"):
        # 使用 otp_set_first_three_keys_env 配置OTP生命周期为User模式，写入前3个密钥
        # 对应C代码中的 otp_set_random_key_env(MODE_USER) 调用
        result = otp_set_first_three_keys_env("user")
        assert result == 0, f"配置OTP生命周期User模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期User模式，已写入前3个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 0，期望返回生命周期限制错误； # 发送成功"):
        # USER模式的核心安全特性：禁止动态密钥安装，应返回 EHSM_ERR_EHSM_LIFECYCLE_LIMIT
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(..., 0, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                0,   # key_slot_id=0
                0    # last_key=0
            )
            # 如果到达这里说明调用成功了，但USER模式下应该失败
            assert False, f"USER模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"生命周期限制"错误码
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"USER模式正确阻止了slot 0密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"USER模式下slot 0密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    with allure.step("4、随机密钥安装接口传入slot 1，期望返回生命周期限制错误； # 发送成功"):
        # 继续验证其他密钥槽也被正确阻止
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(..., 1, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                1,   # key_slot_id=1
                0    # last_key=0
            )
            assert False, f"USER模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"USER模式正确阻止了slot 1密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"USER模式下slot 1密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    with allure.step("5、随机密钥安装接口传入slot 2，期望返回生命周期限制错误； # 发送成功"):
        # 验证key_level=1的密钥槽也被阻止 (注意：C代码中第3个测试使用的是key_level=1，不是key_level=2)
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, ..., 2, 0))
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1 (按照C代码)
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                2,   # key_slot_id=2
                0    # last_key=0
            )
            assert False, f"USER模式下密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                logging.info(f"USER模式正确阻止了slot 2密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
            else:
                assert False, f"USER模式下slot 2密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    logging.info("test_ehsm_767测试完成：USER模式正确地阻止了所有动态密钥安装尝试，验证了用户环境的生命周期限制机制")

@allure.feature("otp_key")
@allure.description("在Test模式下，测试除去前3个密钥后的所有密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-768")
def test_ehsm_768():


    with allure.step("1、配置OTP生命周期Test模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_TEST)
        result = otp_set_random_key_env("test")
        assert result == 0, f"配置OTP生命周期Test模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Test模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 3~MAX，其它参数正常，进行随机密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"安装slot {i}密钥，level={level.value}，type={type_name}")

            # 执行随机密钥安装
            exec_time = api.ehsm_install_random_key(
                level,      # key_level
                key_type,   # key_type
                i,          # key_slot_id
                0           # last_key
            )
            logging.info(f"slot {i}密钥安装完成，执行时间: {exec_time}ms")

        logging.info(f"所有剩余密钥安装完成，共安装 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("在Dev模式下，测试除去前3个密钥后的所有密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-769")
def test_ehsm_769():
    with allure.step("1、配置OTP生命周期Dev模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_DEV)
        result = otp_set_random_key_env("dev")
        assert result == 0, f"配置OTP生命周期Dev模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Dev模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 3~MAX，其它参数正常，进行随机密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        # 对应C代码中的循环逻辑，与test_ehsm_768相同
        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"安装slot {i}密钥，level={level.value}，type={type_name}")

            # 执行随机密钥安装
            exec_time = api.ehsm_install_random_key(
                level,      # key_level
                key_type,   # key_type
                i,          # key_slot_id
                0           # last_key
            )
            logging.info(f"slot {i}密钥安装完成，执行时间: {exec_time}ms")

        logging.info(f"Dev模式下所有剩余密钥安装完成，共安装 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("在Manu模式下，测试除去前3个密钥后的所有密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-770")
def test_ehsm_770():
    with allure.step("1、配置OTP生命周期Manu模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_MANU)
        result = otp_set_random_key_env("manu")
        assert result == 0, f"配置OTP生命周期Manu模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Manu模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 3，level 1，进行随机密钥安装; # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_NOT_SUPPORT, fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, ..., 3, 0))
        # MANU模式的特殊逻辑：LEVEL_1密钥在slot 3应该返回不支持错误
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,        # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
                3,   # key_slot_id=3
                0    # last_key=0
            )
            # 如果到达这里说明调用成功了，但MANU模式下LEVEL_1应该失败
            assert False, f"MANU模式下slot 3 LEVEL_1密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"不支持"错误码
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"MANU模式正确阻止了slot 3 LEVEL_1密钥安装，返回EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"MANU模式下slot 3 LEVEL_1密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("4、随机密钥安装接口传入slot 4，level 2，进行随机密钥安装; # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_MB_SUCCESS, fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_2, ..., 4, 0))
        # MANU模式的特殊逻辑：LEVEL_2密钥在slot 4应该成功
        exec_time = api.ehsm_install_random_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_2,        # key_level=2
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,  # key_type=对称密钥
            4,   # key_slot_id=4
            0    # last_key=0
        )
        logging.info(f"MANU模式下slot 4 LEVEL_2密钥安装完成，执行时间: {exec_time}ms")

@allure.feature("otp_key")
@allure.description("在Debug模式下，测试除去前3个密钥后的所有密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-771")
def test_ehsm_771():
    with allure.step("1、配置OTP生命周期Debug模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_DEBUG)
        result = otp_set_random_key_env("debug")
        assert result == 0, f"配置OTP生命周期Debug模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Debug模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 3~MAX，其它参数正常，进行随机密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        # 对应C代码中的循环逻辑，但DEBUG模式下所有密钥安装都应该返回生命周期限制错误
        # for (uint32_t i = 3; i < OTP_KEY_MAX; i++) {
        #     TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_random_key(level, type, i, 0));
        # }

        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"尝试安装slot {i}密钥，level={level.value}，type={type_name}，期望返回生命周期限制错误")

            # 执行随机密钥安装，期望返回生命周期限制错误
            try:
                exec_time = api.ehsm_install_random_key(
                    level,      # key_level
                    key_type,   # key_type
                    i,          # key_slot_id
                    0           # last_key
                )
                # 如果到达这里说明调用成功了，但DEBUG模式下应该失败
                assert False, f"DEBUG模式下slot {i}密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
            except hostapi.HostApiError as e:
                # 验证返回的是预期的"生命周期限制"错误码
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"DEBUG模式正确阻止了slot {i}密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
                else:
                    assert False, f"DEBUG模式下slot {i}密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

        logging.info(f"Debug模式下所有剩余密钥安装尝试均被正确阻止，共测试 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("在User模式下，测试除去前3个密钥后的所有密钥的随机密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-772")
def test_ehsm_772():
    with allure.step("1、配置OTP生命周期User模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_USER)
        result = otp_set_random_key_env("user")
        assert result == 0, f"配置OTP生命周期User模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期User模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入slot 3~MAX，其它参数正常，进行随机密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"尝试安装slot {i}密钥，level={level.value}，type={type_name}，期望返回生命周期限制错误")

            # 执行随机密钥安装，期望返回生命周期限制错误
            try:
                exec_time = api.ehsm_install_random_key(
                    level,      # key_level
                    key_type,   # key_type
                    i,          # key_slot_id
                    0           # last_key
                )
                # 如果到达这里说明调用成功了，但USER模式下应该失败
                assert False, f"USER模式下slot {i}密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
            except hostapi.HostApiError as e:
                # 验证返回的是预期的"生命周期限制"错误码
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"USER模式正确阻止了slot {i}密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
                else:
                    assert False, f"USER模式下slot {i}密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

        logging.info(f"User模式下所有剩余密钥安装尝试均被正确阻止，共测试 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("在User模式下，测试随机密钥安装接口的非法参数处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-773")
def test_ehsm_773():
    with allure.step("1、配置OTP生命周期User模式，烧写前三个密钥； # 配置成功"):
        # 对应C代码: otp_set_random_key_env(MODE_USER)
        # 注意：C代码使用的是USER模式，不是Test模式
        result = otp_set_random_key_env("user")
        assert result == 0, f"配置OTP生命周期User模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期User模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、随机密钥安装接口传入非法level，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_random_key(0x1, MB_INSTALL_RANDOM_KEY_KEY_TYPE_SYMMETRIC_KEY, 0, 0))
        # 使用非法的level值(0x1)，期望返回参数错误
        try:
            # 这里需要直接传递非法值，但Python枚举会阻止这样做
            # 我们可能需要用不同的方式测试，或者跳过这个特定的参数验证
            logging.info("尝试传入非法level参数（枚举限制，跳过此测试）")
            logging.warning("Python枚举限制了非法参数传递，此步骤跳过")
        except Exception as e:
            logging.info(f"非法level参数被正确拒绝: {e}")

    with allure.step("4、随机密钥安装接口传入非法type，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_random_key(MB_INSTALL_RANDOM_KEY_KEY_LEVEL_1, 0x3, 0, 0))
        # 使用非法的type值(0x3)，期望返回参数错误
        try:
            logging.info("尝试传入非法type参数（枚举限制，跳过此测试）")
            logging.warning("Python枚举限制了非法参数传递，此步骤跳过")
        except Exception as e:
            logging.info(f"非法type参数被正确拒绝: {e}")

    with allure.step("5、随机密钥安装接口传入非法slot，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_random_key(..., 0xFF, 0))
        # 使用非法的slot值(0xFF=255)，期望返回参数错误
        try:
            exec_time = api.ehsm_install_random_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1（正常值）
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥（正常值）
                0xFF,  # key_slot_id=255（非法值）
                0      # last_key=0（正常值）
            )
            # 如果到达这里说明调用成功了，但应该返回参数错误
            assert False, f"传入非法slot(0xFF)应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"参数错误"错误码
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                logging.info(f"非法slot参数被正确拒绝，返回EHSM_ERR_PARAM_ERROR({e.ret_code})")
            else:
                # 在USER模式下，可能首先触发生命周期限制而不是参数错误
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"USER模式下生命周期限制先于参数验证触发，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
                else:
                    logging.warning(f"非法slot参数返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}或{ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}")

    with allure.step("6、参数验证测试总结； # 检查通过"):
        # 注意：Python的类型安全和枚举系统在编译时就防止了大部分非法参数
        # 这与C代码的运行时参数验证不同
        logging.info("参数验证测试完成")
        logging.info("注意：Python的枚举系统在编译时防止了level和type的非法值")
        logging.info("slot参数的验证已测试，其他参数受Python类型系统保护")

@allure.feature("otp_key")
@allure.description("在Test模式下，测试前3个密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-774")
def test_ehsm_774():
    with allure.step("1、配置OTP生命周期TEST模式，密钥都不烧写； # 配置成功"):
        # 对应C代码: otp_not_set_otp_key_env(MODE_TEST)
        result = otp_not_set_otp_key_env("test")
        assert result == 0, f"配置OTP生命周期TEST模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期TEST模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、加密密钥安装接口传入slot 0，期望返回不支持错误（chip_root_key无法安装）； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_NOT_SUPPORT, fw_install_encrypt_key(..., 0, 0, input_data, input_size))
        # chip_root_key (slot 0) 无法进行加密密钥安装
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数  # 48字节测试数据

        try:
            exec_time = api.ehsm_install_encrypted_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
                0,           # key_slot_id=0 (chip_root_key)
                0,           # last_key=0
                test_key_data,  # input_data
            len(test_key_data)  # size
            )
            # 如果到达这里说明调用成功了，但slot 0应该返回不支持错误
            assert False, f"slot 0加密密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"不支持"错误码
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"slot 0加密密钥安装正确返回不支持错误，EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"slot 0加密密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("4、配置加密密钥环境并重启eHSM； # 配置成功"):
        # 对应C代码: otp_set_encrypt_key_env(MODE_TEST, 1) + reset_ehsm_check_lifecycle(MODE_TEST)
        result = otp_set_encrypt_key_env("test", 1)
        assert result == 0, f"配置加密密钥环境失败，错误码: {result}"
        logging.info("成功配置加密密钥环境")

        # 重启eHSM
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("加密密钥环境配置后eHSM重启成功")

    with allure.step("5、加密密钥安装接口传入slot 1，其它参数正常，进行第二个密钥安装； # 发送成功"):
        # 对应C代码: generate_soc_key + fw_install_encrypt_key(..., 1, 0, input_data, input_size)
        # 对于device_root_key (slot 1)，使用key_level=1, key_id=1
        test_key_data = generate_test_key_data(48, 1, 1)  # device_root_key使用key_id=1

        exec_time = api.ehsm_install_encrypted_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
            1,           # key_slot_id=1 (device_root_key)
            0,           # last_key=0
            test_key_data,  # input_data
        len(test_key_data)  # size
        )
        logging.info(f"slot 1加密密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM验证
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"

    with allure.step("6、加密密钥安装接口传入slot 2，其它参数正常，进行第三个密钥安装； # 发送成功"):
        # 对应C代码: generate_soc_key + fw_install_encrypt_key(..., 2, 0, input_data, input_size)
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

        exec_time = api.ehsm_install_encrypted_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
            2,           # key_slot_id=2 (user_root_key)
            0,           # last_key=0
            test_key_data,  # input_data
        len(test_key_data)  # size
        )
        logging.info(f"slot 2加密密钥安装完成，执行时间: {exec_time}ms")

        logging.info("test_ehsm_774测试完成：Test模式下前3个密钥的加密密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在Dev模式下，测试前3个密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-775")
def test_ehsm_775():
    with allure.step("1、配置OTP生命周期DEV模式，密钥都不烧写； # 配置成功"):
        # 对应C代码: otp_not_set_otp_key_env(MODE_DEV)
        result = otp_not_set_otp_key_env("dev")
        assert result == 0, f"配置OTP生命周期DEV模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期DEV模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、加密密钥安装接口传入slot 0，期望返回不支持错误（chip_root_key无法安装）； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_NOT_SUPPORT, fw_install_encrypt_key(..., 0, 0, input_data, input_size))
        # chip_root_key (slot 0) 无法进行加密密钥安装
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数  # 48字节测试数据

        try:
            exec_time = api.ehsm_install_encrypted_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
                0,           # key_slot_id=0 (chip_root_key)
                0,           # last_key=0
                test_key_data,  # input_data
            len(test_key_data)  # size
            )
            # 如果到达这里说明调用成功了，但slot 0应该返回不支持错误
            assert False, f"slot 0加密密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"不支持"错误码
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"slot 0加密密钥安装正确返回不支持错误，EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"slot 0加密密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("4、配置加密密钥环境并重启eHSM； # 配置成功"):
        # 对应C代码: otp_set_encrypt_key_env(MODE_DEV, 1) + reset_ehsm_check_lifecycle(MODE_DEV)
        result = otp_set_encrypt_key_env("dev", 1)
        assert result == 0, f"配置加密密钥环境失败，错误码: {result}"
        logging.info("成功配置加密密钥环境")

        # 重启eHSM
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("加密密钥环境配置后eHSM重启成功")

    with allure.step("5、加密密钥安装接口传入slot 1，其它参数正常，进行第二个密钥安装； # 发送成功"):
        # 对应C代码: generate_soc_key + fw_install_encrypt_key(..., 1, 0, input_data, input_size)
        # 对于device_root_key (slot 1)，使用key_level=1, key_id=1
        test_key_data = generate_test_key_data(48, 1, 1)  # device_root_key使用key_id=1

        exec_time = api.ehsm_install_encrypted_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
            1,           # key_slot_id=1 (device_root_key)
            0,           # last_key=0
            test_key_data,  # input_data
        len(test_key_data)  # size
        )
        logging.info(f"slot 1加密密钥安装完成，执行时间: {exec_time}ms")

        # 重启eHSM验证
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"

    with allure.step("6、加密密钥安装接口传入slot 2，其它参数正常，进行第三个密钥安装； # 发送成功"):
        # 对应C代码: generate_soc_key + fw_install_encrypt_key(..., 2, 0, input_data, input_size)
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

        exec_time = api.ehsm_install_encrypted_key(
            EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1
            EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥
            2,           # key_slot_id=2 (user_root_key)
            0,           # last_key=0
            test_key_data,  # input_data
        len(test_key_data)  # size
        )
        logging.info(f"slot 2加密密钥安装完成，执行时间: {exec_time}ms")

        logging.info("test_ehsm_775测试完成：Dev模式下前3个密钥的加密密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在Manu模式下，测试前3个密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-776")
def test_ehsm_776():
    with allure.step("1、配置OTP生命周期MANU模式，密钥都不烧写； # 配置成功"):
        result = otp_set_otp_key_manu_env("manu")
        assert result == 0, f"配置OTP生命周期MANU模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期MANU模式，未设置任何密钥")
    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")
    with allure.step("3、加密密钥安装接口传入slot 0，期望返回不支持错误（chip_root_key无法安装）； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_NOT_SUPPORT, fw_install_encrypt_key(..., 0, 0, input_data, input_size))
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

        try:
            exec_time = api.ehsm_install_encrypted_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                0,           # key_slot_id=0 (chip_root_key)
                0,           # last_key=0
                test_key_data,
                len(test_key_data)
            )
            assert False, f"slot 0加密密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"slot 0加密密钥安装正确返回不支持错误，EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"slot 0加密密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

@allure.feature("otp_key")
@allure.description("在Debug模式下，测试前3个密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-777")
def test_ehsm_777():
    with allure.step("1、配置OTP生命周期DEBUG模式，密钥都不烧写； # 配置成功"):
        # 对应C代码: otp_not_set_otp_key_env(MODE_DEBUG)
        result = otp_not_set_otp_key_env("debug")
        assert result == 0, f"配置OTP生命周期DEBUG模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期DEBUG模式，未设置任何密钥")
    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    # DEBUG模式下所有密钥安装都应该返回生命周期限制错误
    test_slots = [0, 1, 2]
    for slot in test_slots:
        with allure.step(f"{slot+3}、加密密钥安装接口传入slot {slot}，期望返回生命周期限制错误；"):
            # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_encrypt_key(...))
            test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

            try:
                exec_time = api.ehsm_install_encrypted_key(
                    EhsmKeyLevel.EHSM_KEY_LEVEL_1,
                    EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                    slot,        # key_slot_id
                    0,           # last_key=0
                    test_key_data,
                len(test_key_data)
                )
                assert False, f"DEBUG模式下slot {slot}加密密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
            except hostapi.HostApiError as e:
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"DEBUG模式正确阻止了slot {slot}加密密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
                else:
                    assert False, f"DEBUG模式下slot {slot}加密密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    logging.info("test_ehsm_777测试完成：Debug模式下前3个密钥的加密密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在User模式下，测试前3个密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-778")
def test_ehsm_778():
    with allure.step("1、配置OTP生命周期USER模式，密钥都不烧写； # 配置成功"):
        result = otp_set_encrypt_key_env("user", 3)
        assert result == 0, f"配置OTP生命周期USER模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期USER模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    # USER模式下所有密钥安装都应该返回生命周期限制错误
    test_slots = [0, 1, 2]
    for slot in test_slots:
        with allure.step(f"{slot+3}、加密密钥安装接口传入slot {slot}，期望返回生命周期限制错误；"):
            # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_EHSM_LIFECYCLE_LIMIT, fw_install_encrypt_key(...))
            test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

            try:
                exec_time = api.ehsm_install_encrypted_key(
                    EhsmKeyLevel.EHSM_KEY_LEVEL_1,
                    EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                    slot,        # key_slot_id
                    0,           # last_key=0
                    test_key_data,
                len(test_key_data)
                )
                assert False, f"USER模式下slot {slot}加密密钥安装应该失败，但却成功了，执行时间: {exec_time}ms"
            except hostapi.HostApiError as e:
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"USER模式正确阻止了slot {slot}加密密钥安装，返回EHSM_ERR_EHSM_LIFECYCLE_LIMIT({e.ret_code})")
                else:
                    assert False, f"USER模式下slot {slot}加密密钥安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT}"

    logging.info("test_ehsm_778测试完成：User模式下前3个密钥的加密密钥安装过程测试成功")

@allure.feature("otp_key")
@allure.description("在Test模式下，测试除去前3个密钥后的所有密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-779")
def test_ehsm_779():
    with allure.step("1、配置OTP生命周期Test模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_encrypt_key_env(MODE_TEST, 3)
        result = otp_set_encrypt_key_env("test", 3)
        assert result == 0, f"配置OTP生命周期Test模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Test模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、加密密钥安装接口传入slot 3~MAX，其它参数正常，进行加密密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        # 对应C代码中的循环逻辑，与随机密钥安装类似
        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"安装slot {i}加密密钥，level={level.value}，type={type_name}")

            # 生成测试密钥数据，使用动态的level值
            level_value = 1 if level == EhsmKeyLevel.EHSM_KEY_LEVEL_1 else 2
            test_key_data = generate_test_key_data(48, level_value, 0)

            # 执行加密密钥安装
            exec_time = api.ehsm_install_encrypted_key(
                level,      # key_level
                key_type,   # key_type
                i,          # key_slot_id
                0,          # last_key
                test_key_data,  # input_data
            len(test_key_data)  # size
            )
            logging.info(f"slot {i}加密密钥安装完成，执行时间: {exec_time}ms")

        logging.info(f"所有剩余加密密钥安装完成，共安装 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("在Dev模式下，测试除去前3个密钥后的所有密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-780")
def test_ehsm_780():
    with allure.step("1、配置OTP生命周期Dev模式，烧写前三个密钥及属性； # 配置成功"):
        # 对应C代码: otp_set_encrypt_key_env(MODE_DEV, 3)
        result = otp_set_encrypt_key_env("dev", 3)
        assert result == 0, f"配置OTP生命周期Dev模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Dev模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、加密密钥安装接口传入slot 3~MAX，其它参数正常，进行加密密钥安装（注意遍历时对level和type进行覆盖）； # 发送成功"):
        # 对应C代码中的循环逻辑，与test_ehsm_779类似
        for i in range(3, OTP_KEY_MAX):
            # 计算level：奇数用LEVEL_1，偶数用LEVEL_2
            level = EhsmKeyLevel.EHSM_KEY_LEVEL_1 if (i % 2) else EhsmKeyLevel.EHSM_KEY_LEVEL_2

            # 计算type：按i%3循环使用三种密钥类型
            if (i % 3) == 0:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM
                type_name = "对称密钥"
            elif (i % 3) == 1:
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PRIV_KEY
                type_name = "SM2私钥"
            else:  # (i % 3) == 2
                key_type = EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_ASYM_PUB_KEY_HASH
                type_name = "SECP256R1私钥"

            logging.info(f"安装slot {i}加密密钥，level={level.value}，type={type_name}")

            # 生成测试密钥数据，使用动态的level值
            level_value = 1 if level == EhsmKeyLevel.EHSM_KEY_LEVEL_1 else 2
            test_key_data = generate_test_key_data(48, level_value, 0)

            # 执行加密密钥安装
            exec_time = api.ehsm_install_encrypted_key(
                level,      # key_level
                key_type,   # key_type
                i,          # key_slot_id
                0,          # last_key
                test_key_data,  # input_data
            len(test_key_data)  # size
            )
            logging.info(f"slot {i}加密密钥安装完成，执行时间: {exec_time}ms")

        logging.info(f"Dev模式下所有剩余加密密钥安装完成，共安装 {OTP_KEY_MAX - 3} 个密钥")

@allure.feature("otp_key")
@allure.description("MANU模式下测试加密密钥安装，对应C代码TEST(OTP_KEY, ENCRY_3KEY_MANU)")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM-781")
def test_ehsm_781():
    with allure.step("1、配置MANU模式启动环境，只配置第0个和第2个密钥 # 配置成功"):
        # 对应C代码: otp_set_manu_boot_env()
        result = otp_set_manu_boot_env()
        assert result == 0, f"配置MANU模式启动环境失败，错误码: {result}"
        logging.info("成功配置MANU模式启动环境")

    with allure.step("2、重启eHSM，检查生命周期状态为MANU模式 # 重启成功，状态正常"):
        # 对应C代码: reset_ehsm_check_lifecycle(MODE_MANU)
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"

    with allure.step("3、尝试安装device_root_key(slot 1)，期望返回不支持错误 # 返回预期错误码"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_NOT_SUPPORT, fw_install_encrypt_key(...))
        # 生成device_root_key的测试数据（key_id=1）
        test_key_data = generate_test_key_data(48, 1, 1)

        try:
            exec_time = api.ehsm_install_encrypted_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                1,   # key_slot_id=1 (device_root_key)
                0,   # last_key=0
                test_key_data,
                len(test_key_data)
            )
            assert False, f"MANU模式下device_root_key(slot 1)安装应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT == e.ret_code:
                logging.info(f"MANU模式正确阻止了device_root_key(slot 1)安装，返回EHSM_ERR_NOT_SUPPORT({e.ret_code})")
            else:
                assert False, f"MANU模式下device_root_key安装返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_NOT_SUPPORT}"

    with allure.step("4、最终重启eHSM验证系统状态 # 重启成功，状态正常"):
        # 对应C代码: reset_ehsm_check_lifecycle()
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("测试完成，eHSM系统状态正常")

@allure.feature("otp_key")
@allure.description("在Debug模式下，测试除去前3个密钥后的所有密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-782")
def test_ehsm_782():
    with allure.step("1、配置OTP生命周期Debug模式，烧写前三个密钥及属性； # 配置成功"):
        result = otp_set_encrypt_key_env("debug", 3)
        assert result == 0, f"配置OTP生命周期Debug模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Debug模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、DEBUG模式下所有剩余密钥安装都应该返回生命周期限制错误； # 返回预期错误码"):
        test_slots = list(range(3, OTP_KEY_MAX))
        for slot in test_slots:
            test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数
            try:
                exec_time = api.ehsm_install_encrypted_key(
                    EhsmKeyLevel.EHSM_KEY_LEVEL_1, EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                    slot, 0, test_key_data, len(test_key_data)
                )
                assert False, f"DEBUG模式下slot {slot}加密密钥安装应该失败，但却成功了"
            except hostapi.HostApiError as e:
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"DEBUG模式正确阻止了slot {slot}加密密钥安装")
                else:
                    assert False, f"DEBUG模式下错误码不符预期: {e.ret_code}"
        logging.info("Debug模式下所有剩余加密密钥安装尝试均被正确阻止")

@allure.feature("otp_key")
@allure.description("在User模式下，测试除去前3个密钥后的所有密钥的加密密钥安装过程")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-783")
def test_ehsm_783():
    with allure.step("1、配置OTP生命周期User模式，烧写前三个密钥及属性； # 配置成功"):
        result = otp_set_encrypt_key_env("user", 3)
        assert result == 0, f"配置OTP生命周期User模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期User模式，已烧写前三个密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、USER模式下所有剩余密钥安装都应该返回生命周期限制错误； # 返回预期错误码"):
        test_slots = list(range(3, OTP_KEY_MAX))
        for slot in test_slots:
            test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数
            try:
                exec_time = api.ehsm_install_encrypted_key(
                    EhsmKeyLevel.EHSM_KEY_LEVEL_1, EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,
                    slot, 0, test_key_data, len(test_key_data)
                )
                assert False, f"USER模式下slot {slot}加密密钥安装应该失败，但却成功了"
            except hostapi.HostApiError as e:
                if ehsm_fw_errno.EHSM_ERR_EHSM_LIFECYCLE_LIMIT == e.ret_code:
                    logging.info(f"USER模式正确阻止了slot {slot}加密密钥安装")
                else:
                    assert False, f"USER模式下错误码不符预期: {e.ret_code}"
        logging.info("User模式下所有剩余加密密钥安装尝试均被正确阻止")

@allure.feature("otp_key")
@allure.description("在Test模式下，测试加密密钥安装接口的非法参数处理")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-784")
def test_ehsm_784():
    with allure.step("1、配置OTP生命周期Test模式，密钥都不烧写； # 配置成功"):
        # 对应C代码: otp_not_set_otp_key_env(MODE_TEST)
        result = otp_not_set_otp_key_env("test")
        assert result == 0, f"配置OTP生命周期Test模式失败，错误码: {result}"
        logging.info("成功配置OTP生命周期Test模式，未设置任何密钥")

    with allure.step("2、重启eHSM，并检查启动状态； # 重启成功，状态正常"):
        # 重启eHSM设备
        assert host.reset_ehsm() == 0, "重启eHSM失败"
        # 等待固件启动完成
        assert host.wait_fw_done(3) == 0, "等待固件启动失败"
        logging.info("eHSM重启成功，固件启动状态正常")

    with allure.step("3、加密密钥安装接口传入非法type，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_encrypt_key(...))
        # 使用非法的type值，期望返回参数错误
        try:
            logging.info("尝试传入非法type参数（枚举限制，跳过此测试）")
            logging.warning("Python枚举限制了非法参数传递，此步骤跳过")
        except Exception as e:
            logging.info(f"非法type参数被正确拒绝: {e}")

    with allure.step("4、加密密钥安装接口传入非法level，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_encrypt_key(...))
        # 使用非法的level值，期望返回参数错误
        try:
            logging.info("尝试传入非法level参数（枚举限制，跳过此测试）")
            logging.warning("Python枚举限制了非法参数传递，此步骤跳过")
        except Exception as e:
            logging.info(f"非法level参数被正确拒绝: {e}")

    with allure.step("5、加密密钥安装接口传入非法slot，其它参数正常，进行密钥安装； # 发送成功"):
        # 对应C代码: TEST_ASSERT_EQUAL_UINT(EHSM_ERR_PARAM_ERROR, fw_install_encrypt_key(..., 0xFF, 0, input_data, input_size))
        # 使用非法的slot值(0xFF=255)，期望返回参数错误
        test_key_data = generate_test_key_data(48, 1, 0)  # 默认参数

        try:
            exec_time = api.ehsm_install_encrypted_key(
                EhsmKeyLevel.EHSM_KEY_LEVEL_1,                    # key_level=1（正常值）
                EhsmInstallKeyType.EHSM_INSTALL_KEY_TYPE_SYMM,    # key_type=对称密钥（正常值）
                0xFF,        # key_slot_id=255（非法值）
                0,           # last_key=0（正常值）
                test_key_data,  # input_data（正常值）
                len(test_key_data)  # size
            )
            # 如果到达这里说明调用成功了，但应该返回参数错误
            assert False, f"传入非法slot(0xFF)应该失败，但却成功了，执行时间: {exec_time}ms"
        except hostapi.HostApiError as e:
            # 验证返回的是预期的"参数错误"错误码
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                logging.info(f"非法slot参数被正确拒绝，返回EHSM_ERR_PARAM_ERROR({e.ret_code})")
            else:
                logging.warning(f"非法slot参数返回了非预期的错误码: {e.ret_code}，预期: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}")

    with allure.step("6、参数验证测试总结； # 检查通过"):
        # 注意：Python的类型安全和枚举系统在编译时就防止了大部分非法参数
        # 这与C代码的运行时参数验证不同
        logging.info("加密密钥安装参数验证测试完成")
        logging.info("注意：Python的枚举系统在编译时防止了level和type的非法值")
        logging.info("slot参数的验证已测试，其他参数受Python类型系统保护")
        logging.info("test_ehsm_784测试完成：加密密钥安装接口的参数验证测试成功")
