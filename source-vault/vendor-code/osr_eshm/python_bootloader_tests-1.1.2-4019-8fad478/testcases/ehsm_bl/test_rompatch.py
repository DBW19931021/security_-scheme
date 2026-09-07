import logging as log
import time
import os
import re
import allure
import pytest
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib.hostapi import SHARE_RAM_BASE
from utils import otp
from utils.config import cfg_data

host = get_host_interface()
api = get_api_interface()

@pytest.fixture(scope="function")
def setup_function():
    """测试用例级别的setup"""
    # 清零共享内存区域，使用 size=8 或合适的大小
    host.write_memory(SHARE_RAM_BASE, b'\x00' * 8)
    time.sleep(0.1)
    yield api
    host.write_memory(SHARE_RAM_BASE, b'\x00' * 8)
    time.sleep(0.1)

def calculate_patch_start_addr():
    """从项目的 ehsm_bl.dis 反汇编文件中提取并计算 sch_start 的入口地址偏移"""
    # 客户 ID 格式转换：0x4019 -> "4019"
    custom_id_str = f"{cfg_data.TEST_CUSTOM_ID:04x}" if hasattr(cfg_data, 'TEST_CUSTOM_ID') else "0000"
    dis_file_path = os.path.join("resource", "dis", custom_id_str, "ehsm_bl.dis")

    if not os.path.exists(dis_file_path):
        raise FileNotFoundError(
            f"找不到对应项目的反汇编文件: {dis_file_path};\n"
            f"请手动将对应项目的 ehsm_bl.dis 文件放入该目录下以进行测试;"
        )

    # 匹配 sch_start 的入口地址线，形如 "10003944 <sch_start>:"
    pattern = re.compile(r"^\s*([0-9a-fA-F]+)\s+<sch_start>:")
    with open(dis_file_path, "r", encoding="utf-8") as f:
        for line in f:
            match = pattern.match(line)
            if match:
                sch_start_addr = int(match.group(1), 16)
                # 计算公式：(sch_start_addr - ROM起始地址0x10000000) >> 2
                offset = (sch_start_addr - 0x10000000) >> 2
                return f"{offset:04X}"

    raise ValueError(f"在文件 {dis_file_path} 中未找到 <sch_start> 的入口地址定义")

def get_probe0_offset():
    """从项目的 ehsm_bl.dis 反汇编文件中提取并计算 bl_patch_test_probe0 的入口地址偏移"""
    # Reason: 动态解析 bl_patch_test_probe0 地址偏移，防止硬编码
    custom_id_str = f"{cfg_data.TEST_CUSTOM_ID:04x}" if hasattr(cfg_data, 'TEST_CUSTOM_ID') else "0000"
    dis_file_path = os.path.join("resource", "dis", custom_id_str, "ehsm_bl.dis")

    if not os.path.exists(dis_file_path):
        raise FileNotFoundError(
            f"找不到对应项目的反汇编文件: {dis_file_path};\n"
            f"请手动将对应项目的 ehsm_bl.dis 文件放入该目录下以进行测试;"
        )

    pattern = re.compile(r"^\s*([0-9a-fA-F]+)\s+<bl_patch_test_probe0>:")
    with open(dis_file_path, "r", encoding="utf-8") as f:
        for line in f:
            match = pattern.match(line)
            if match:
                addr = int(match.group(1), 16)
                # 计算公式：(addr - ROM起始地址0x10000000) >> 2
                offset = (addr - 0x10000000) >> 2
                return f"{offset:04X}"

    raise ValueError(f"在文件 {dis_file_path} 中未找到 <bl_patch_test_probe0> 的入口地址定义")




CFG0_MAPPING = [
    (4, 3), (2, 1), (8, 7), (6, 5),
    (12, 11), (10, 9), (16, 15), (14, 13)
]

CFG1_MAPPING = [
    (20, 19), (18, 17), (24, 23), (22, 21),
    (28, 27), (26, 25), (32, 31), (30, 29)
]
FIXED_DATA_LIST = [
    "011106CE", "22CC0010", "B79701A0", "2326F4FE",
    "B7470030", "93870780", "2324F4FE", "832784FE",
    "37970160", "98C38327", "84FE9107", "23A00700",
    "8327C4FE", "4557988B", "8327C4FE", "85074957",
    "988B8327", "C4FE8907", "4D57988B", "8327C4FE",
    "8D075157", "988B8327", "C4FE9107", "5557988B",
    "8327C4FE", "95075957", "988B8327", "C4FE9907",
    "5D57988B", "0100F240", "62440561", "01008280"
]

def parse_config(config, mapping):
    """解析单个配置参数的有效位"""
    enabled_bits = []

    for i, hex_char in enumerate(config):
        # 将十六进制字符转换为4位二进制
        binary = bin(int(hex_char, 16))[2:].zfill(4)

        # 获取对应的位映射
        bit1, bit2 = mapping[i]

        # 将4位二进制分成两个2位组
        high_bits = binary[:2]  # 高2位
        low_bits = binary[2:]   # 低2位

        # 检查高2位是否有效 (01或10)
        if high_bits in ["01", "10"]:
            enabled_bits.append(bit1)

        # 检查低2位是否有效 (01或10)
        if low_bits in ["01", "10"]:
            enabled_bits.append(bit2)

    return sorted(enabled_bits)

def convert_to_little_endian(addr):
    """
    将地址转换为小端对齐格式

    Args:
        addr (str): 十六进制地址字符串，如"0E28"

    Returns:
        str: 小端对齐的地址字符串，如"280E"
    """
    # 确保地址长度为4个字符（16位）或5个字符（支持越界测试如FFFFF）
    if len(addr) not in [4, 5]:
        raise ValueError("地址必须是4位或5位十六进制字符串")

    # 验证是否为有效的十六进制字符串
    if not all(c in '0123456789ABCDEFabcdef' for c in addr):
        raise ValueError("地址必须是有效的十六进制字符串")

    # 统一补齐为5位或截断/填充处理;此处为保持16位(4字符)或20位(5字符)的小端转换
    if len(addr) == 4:
        little_endian = addr[2:4] + addr[0:2]
    else:
        # FFFFF 小端排列，可根据需要定义;这里由于原本是 4 字符，若为 5 字符将其格式化为对应的字节序;
        # 比如：FFFFF -> 0FFFFF -> 对应的字节流为 \xff\xff\x0f，小端十六进制字符串表示为 "FFFF0F"
        # 针对越界测试 FFFFF，我们需要转为 64 字节对应的 Patch_addr 字段的单项格式限制;
        # 按照 OTP layout.toml 定义，Patch_addr 长度为 64 字节，共 32 个 patch，每行 2 字节（即 4 位十六进制，16位）;
        # 如果硬件逻辑中 Patch_addr 的每一项只占 16 位，任何 5 位的地址都会在硬件写入/解析时被截断;
        # 但既然要直接向 write_otp 传入 FFFFF，此处转换为 4 字符的截断或者特殊处理：
        # 为了能让 generate_address_sequence 继续以整数计算并转回十六进制，我们支持 5 位长度的地址格式;
        little_endian = addr[3:5] + addr[1:3] + "0" + addr[0]

    return little_endian  # 返回大写格式

def generate_address_sequence(start_addr, enabled_bits):
    """
    生成根据使能位筛选的地址序列的小端对齐字符串

    Args:
        start_addr (str): 起始地址，如"0E28"
        enabled_bits (list): 使能的位列表，如[1, 2, 5]

    Returns:
        str: 根据使能位筛选的地址序列的小端对齐字符串
    """
    # 将起始地址转换为整数
    start_int = int(start_addr, 16)

    # 生成完整的地址序列
    address_sequence = []
    for i in range(32):
        # 计算当前地址
        current_addr_int = start_int + i

        # 如果起始地址为 5 位（如 FFFFF），则不限制在 0xFFFF 内
        max_val = 0xFFFFF if len(start_addr) == 5 else 0xFFFF
        if current_addr_int > max_val:
            current_addr_int = current_addr_int & max_val

        # 转换为对应位数的十六进制字符串
        current_addr_hex = format(current_addr_int, f'0{len(start_addr)}X')

        # 转换为小端格式
        little_endian_addr = convert_to_little_endian(current_addr_hex)

        # 添加到序列
        address_sequence.append(little_endian_addr)

    # 根据使能位筛选地址
    filtered_sequence = []
    for i in range(32):
        # 位号从1开始，索引从0开始，所以需要转换
        bit_number = i + 1

        if bit_number in enabled_bits:
            # 如果该位使能，添加对应地址
            filtered_sequence.append(address_sequence[i])
        else:
            # 如果该位不使能，添加与地址长度匹配的 "0000" 或 "000000"
            pad_len = 6 if len(start_addr) == 5 else 4
            filtered_sequence.append("0" * pad_len)

    # 将所有地址连接成一个字符串
    return ''.join(filtered_sequence)

def generate_fixed_data_sequence(enabled_bits):
    """
    生成根据使能位筛选的固定数据序列

    Args:
        enabled_bits (list): 使能的位列表，如[1, 2, 5]

    Returns:
        str: 根据使能位筛选的固定数据序列字符串
    """
    # 根据使能位筛选固定数据
    filtered_sequence = []
    for i in range(32):
        # 位号从1开始，索引从0开始，所以需要转换
        bit_number = i + 1

        if bit_number in enabled_bits:
            # 如果该位使能，添加对应的固定数据
            filtered_sequence.append(FIXED_DATA_LIST[i])
        else:
            # 如果该位不使能，添加"00000000"
            filtered_sequence.append("00000000")

    # 将所有数据连接成一个字符串
    return ''.join(filtered_sequence)


def get_enabled_bits(cfg0, cfg1, addr=None):
    """
    根据 cfg0 和 cfg1 参数解析使能位，并生成完整的 OTP 配置
    规则: 00和11表示无效, 01和10表示有效

    Args:
        cfg0 (str): 8位十六进制字符串，如"55555500"，控制 patch 0-15
        cfg1 (str): 8位十六进制字符串，如"00000000"，控制 patch 16-31
        addr (str): 起始地址的十六进制字符串，默认使用全局变量 PATCH_START_ADDR

    Returns:
        dict: 包含cfg0、cfg1、地址序列和固定数据序列的字典

    Examples:
        # 使能所有 32 个 patch (0b01 模式)
        >>> get_enabled_bits("55555555", "55555555")

        # 使能第 0-11 行 patch
        >>> get_enabled_bits("55555500", "00000000")
    """
    # Reason: 使用全局起始地址，如果未指定则使用 PATCH_START_ADDR
    PATCH_START_ADDR = calculate_patch_start_addr()
    if addr is None:
        addr = PATCH_START_ADDR

    # Reason: 验证参数格式

    if len(cfg0) != 8 or not all(c in '0123456789ABCDEFabcdef' for c in cfg0):
        raise ValueError("cfg0必须是8位十六进制字符串")

    if len(cfg1) != 8 or not all(c in '0123456789ABCDEFabcdef' for c in cfg1):
        raise ValueError("cfg1必须是8位十六进制字符串")

    # 验证地址格式
    if len(addr) not in [4, 5] or not all(c in '0123456789ABCDEFabcdef' for c in addr):
        raise ValueError("地址必须是4位或5位十六进制字符串")

    # Reason: 转换为大写以保持一致性
    cfg0 = cfg0.upper()
    cfg1 = cfg1.upper()
    addr = addr.upper()

    # Reason: 解析 cfg0 和 cfg1 中的有效使能位
    cfg0_enabled = parse_config(cfg0, CFG0_MAPPING)
    cfg1_enabled = parse_config(cfg1, CFG1_MAPPING)

    # Reason: 合并所有使能位
    all_enabled_bits = cfg0_enabled + cfg1_enabled

    # Reason: 生成根据使能位筛选的地址序列
    address_sequence = generate_address_sequence(addr, all_enabled_bits)

    # Reason: 生成根据使能位筛选的固定数据序列
    fixed_data_sequence = generate_fixed_data_sequence(all_enabled_bits)

    # 返回指定格式的字典
    return {
        "patch_en_cfg0_full": cfg0,
        "patch_en_cfg1_full": cfg1,
        "patch_addr_full": address_sequence,
        "patch_data_full": fixed_data_sequence
    }

def _gen_otp_config(patch_config, patch_en="enable1"):
    """
    生成 OTP 配置并转换为二进制

    Args:
        patch_config: patch 配置字典（来自 get_enabled_bits 或硬编码配置）
        patch_en: patch 使能模式，可选 "disable"/"enable"/"enable1"/"disable1"

    Returns:
        OTP 二进制数据
    """
    if "patch_en_full" in patch_config:
        patch_en_value = patch_config["patch_en_full"]
    else:
        patch_en_value = patch_en

    otp_config = {
            "lifecycle": "test",
            "patch_en": patch_en_value,
            "Patch_en_cfg0": patch_config["patch_en_cfg0_full"],
            "Patch_en_cfg1": patch_config["patch_en_cfg1_full"],
            "Patch_addr": patch_config["patch_addr_full"],
            "Patch_data": patch_config["patch_data_full"]
        }
    otp_bin = otp.otp_to_bin(otp_config)
    return otp_bin

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("配置patch总使能开关关闭，patch行不生效时，无法进行patch")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP001")
def test_ehsm_rp001(setup_function):
    with allure.step("1、清除共享地址内存0x60019000内容 # 1、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("2、配置 IPatch 全局总使能为 `disable`（0b00），patch_eh_cfg配置为全0，patch地址配置为无效地址;#2、配置成功"):
        disable_0b00 = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_0b00, patch_en="disable"))
    with allure.step("3、复位启动并等待运行结束，读取共享内存值;#3、配置成功，共享内存值符合预期"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"共享地址内存内容: {int.from_bytes(ret1,'little')}")
        assert ret1 == b'\x11"3\xa5\x00\x00\x00\x00'

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("配置patch总使能开关关闭，patch行生效时，无法进行patch")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP002")
def test_ehsm_rp002(setup_function):
    with allure.step("1、清除共享地址内存0x60019000内容 # 1、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("2、配置 IPatch 全局总使能为 `disable1`（0b01），patch_eh_cfg配置为全5，patch地址配置为有效地址;#2、配置成功"):
        enable_0b01 = get_enabled_bits("55555555", "55555555")
        assert 0 == host.write_otp(_gen_otp_config(enable_0b01, patch_en="disable1"))
    with allure.step("3、复位启动并等待运行结束，读取共享内存值;#3、配置成功，共享内存值符合预期"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"共享地址内存内容: {int.from_bytes(ret1,'little')}")
        assert ret1 == b'\x11"3\xa5\x00\x00\x00\x00'

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("配置patch总使能开关开启，patch行不生效时，无法进行patch")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP003")
def test_ehsm_rp003(setup_function):
    with allure.step("1、清除共享地址内存0x60019000内容 # 1、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("2、配置 IPatch 全局总使能为 `enable`（0b01），patch_eh_cfg配置为全F，patch地址配置为无效地址;#2、配置成功"):
        disable_0b11 = get_enabled_bits("FFFFFFFF", "FFFFFFFF")
        assert 0 == host.write_otp(_gen_otp_config(disable_0b11, patch_en="enable"))
    with allure.step("3、复位启动并等待运行结束，读取共享内存值;#3、配置成功，共享内存值符合预期"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"共享地址内存内容: {int.from_bytes(ret1,'little')}")
        assert ret1 == b'\x11"3\xa5\x00\x00\x00\x00'


@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("配置patch总使能开关开启，patch行有效时，可以正常进行patch")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP004")
def test_ehsm_rp004(setup_function):
    with allure.step("1、清除共享地址内存0x60019000内容 # 1、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("2、配置 IPatch 全局总使能为 `enable1`（0b10），patch_eh_cfg配置为全5，patch地址配置为有效地址;#2、配置成功"):
        enable_0b10 = get_enabled_bits("AAAAAAAA", "AAAAAAAA")
        assert 0 == host.write_otp(_gen_otp_config(enable_0b10, patch_en="enable1"))
    with allure.step("3、复位启动并等待运行结束，读取共享内存值;#3、配置成功，共享内存值符合预期"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"共享地址内存内容: {int.from_bytes(ret1,'little')}")
        assert ret1 == b'\xf1\xf2\xf3\xf4\xf5\xf6\xf7\x00'

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("32行Patch全替换的情况下，验证Patch可以正常进行")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP005")
def test_ehsm_rp005(setup_function):
    with allure.step("1、清除共享地址内存0x60019000内容 # 1、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("2、配置OTP32行patch使能，配置值0b00，其他默认，重启bootloader，等待patch替换; #2、配置成功"):
        enable_0b01 = get_enabled_bits("55555555", "55555555")
        assert 0 == host.write_otp(_gen_otp_config(enable_0b01, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
    with allure.step("3、上位机读取共享地址内存0x60019000的值;#3、配置成功，共享内存值符合预期"):
        t, ret1 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"共享地址内存内容: {int.from_bytes(ret1,'little')}")
        assert ret1 == b'\x11"3\xa5\x00\x00\x00\x00'
    with allure.step("4、清除共享地址内存0x60019000内容;#4、清除成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        time.sleep(5)
        t, ret = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"第二次清除后共享地址内存内容: {int.from_bytes(ret,'little')}")
    with allure.step("5、配置OTP32行patch使能关闭，配置值0b01，其他默认，重启bootloader，等待patch替换;#5、清除成功"):
        enable_0b10 = get_enabled_bits("55555555", "55555555")
        assert 0 == host.write_otp(_gen_otp_config(enable_0b10, patch_en="enable1"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
    with allure.step("6、上位机重新读取共享地址内存0x60019000的值，与3中读取的值作比较;#6、两次读的值不相同"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE,8)
        log.info(f"重新读取的共享地址内存内容: {int.from_bytes(ret2,'little')}")
        assert ret2 == b'\xf1\xf2\xf3\xf4\xf5\xf6\xf7\x00'
        assert ret2 != ret1

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("验证rom patch单条指令替换功能")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP006")
def test_ehsm_rp006(setup_function):
    with allure.step("1、读取共享内存0x60019000的内容;#1、结果为0xA5332211"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        disable_config = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_config, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"未Patch前共享内存内容: {hex(int.from_bytes(ret1, 'little'))}")
        assert int.from_bytes(ret1, 'little') == 0xA5332211

    with allure.step("2、配置OTP第一行patch命中，配置使使能开关0b01，重启bootloader，等待patch替换;#2、重启成功"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        PROBE0_OFFSET = get_probe0_offset()
        patch_config = get_enabled_bits("01000000", "00000000", addr=PROBE0_OFFSET)
        patched_data = "1305A00A" + patch_config["patch_data_full"][8:]
        patch_config["patch_data_full"] = patched_data
        assert 0 == host.write_otp(_gen_otp_config(patch_config, patch_en="enable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)

    with allure.step("3、读取共享内存0x60019000的内容;#3、结果为0xA53322AA"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"Patch后共享内存内容: {hex(int.from_bytes(ret2, 'little'))}")
        assert int.from_bytes(ret2, 'little') == 0xA53322AA


@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("验证多行连续指令替换验证;")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP007")
def test_ehsm_rp007(setup_function):
    with allure.step("1、读取共享内存0x60019000的内容;#1、结果为0xA5332211;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        disable_config = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_config, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"未Patch前共享内存内容: {hex(int.from_bytes(ret1, 'little'))}")
        assert int.from_bytes(ret1, 'little') == 0xA5332211

    with allure.step("2、配置OTP前三行patch命中，配置使能开关0b01，重启bootloader，等待patch替换;#2、重启成功;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        PROBE0_OFFSET = get_probe0_offset()
        patch_config = get_enabled_bits("15000000", "00000000", addr=PROBE0_OFFSET)
        patched_data = "1305A00A" + "1305B00B" + "1305C00C" + patch_config["patch_data_full"][24:]
        patch_config["patch_data_full"] = patched_data
        assert 0 == host.write_otp(_gen_otp_config(patch_config, patch_en="enable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)

    with allure.step("3、读取共享内存0x60019000的内容;#2、结果为0xA533CCCC;"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"Patch后共享内存内容: {hex(int.from_bytes(ret2, 'little'))}")
        assert int.from_bytes(ret2, 'little') == 0xA533CCCC

@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("多行不连续指令替换验证;")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP009")
def test_ehsm_rp009(setup_function):
    with allure.step("1、读取共享内存0x60019000的内容;#1、结果为0xA5332211;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        disable_config = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_config, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"未Patch前共享内存内容: {hex(int.from_bytes(ret1, 'little'))}")
        assert int.from_bytes(ret1, 'little') == 0xA5332211
    with allure.step("2、配置OTP第1-5行patch命中，8-12行patch命中，16-18行命中，其他不命中，配置使能开关0b01，重启bootloader，等待patch替换;#2、重启成功;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        PROBE0_OFFSET = get_probe0_offset()
        patch_config = get_enabled_bits("55415540", "05000000", addr=PROBE0_OFFSET)

        # 重新规划地址：除了 1 (probe0), 3 (probe1), 5 (probe2) 外，其余使能行映射到安全地址 (0x9FFF) 避免覆盖系统指令
        addr_list = [patch_config["patch_addr_full"][i:i+4] for i in range(0, len(patch_config["patch_addr_full"]), 4)]
        safe_addr_le = convert_to_little_endian("9FFF")
        for idx in [1, 3, 7, 8, 9, 10, 11, 15, 16, 17]:
            addr_list[idx] = safe_addr_le
        patch_config["patch_addr_full"] = "".join(addr_list)

        # 将 patch_data_full 分解并修改
        data_list = [patch_config["patch_data_full"][i:i+8] for i in range(0, len(patch_config["patch_data_full"]), 8)]
        data_list[0] = "1305A00A"  # 第 1 行 (probe0) 替换为返回 0xAA
        data_list[2] = "13052002"  # 第 3 行 (probe1) 替换为返回 0x22 (即默认值)
        data_list[4] = "1305C00C"  # 第 5 行 (probe2) 替换为返回 0xCC
        patch_config["patch_data_full"] = "".join(data_list)

        assert 0 == host.write_otp(_gen_otp_config(patch_config, patch_en="enable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
    with allure.step("3、读取共享内存0x60019000的内容;#3、结果为0xA5CC22AA;"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"Patch后共享内存内容: {hex(int.from_bytes(ret2, 'little'))}")
        assert int.from_bytes(ret2, 'little') == 0xA5CC22AA


@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("多个单行patch指令替换验证，替换后返回值满足0xA5CCBBAA;")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP010")
def test_ehsm_rp010(setup_function):
    with allure.step("1、读取共享内存0x60019000的内容;#1、结果为0xA5332211;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        disable_config = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_config, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"未Patch前共享内存内容: {hex(int.from_bytes(ret1, 'little'))}")
        assert int.from_bytes(ret1, 'little') == 0xA5332211
    with allure.step("2、配置OTP第1、3、5行patch命中，配置使能开关0b01，重启bootloader，等待patch替换;#2、重启成功;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        PROBE0_OFFSET = get_probe0_offset()
        patch_config = get_enabled_bits("11010000", "00000000", addr=PROBE0_OFFSET)

        # 修改第 1、3、5 行的 patch_data
        data_list = [patch_config["patch_data_full"][i:i+8] for i in range(0, len(patch_config["patch_data_full"]), 8)]
        data_list[0] = "1305A00A"  # 第 1 行 (probe0) 替换为返回 0xAA
        data_list[2] = "1305B00B"  # 第 3 行 (probe1) 替换为返回 0xBB
        data_list[4] = "1305C00C"  # 第 5 行 (probe2) 替换为返回 0xCC
        patch_config["patch_data_full"] = "".join(data_list)

        assert 0 == host.write_otp(_gen_otp_config(patch_config, patch_en="enable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
    with allure.step("3、读取共享内存0x60019000的内容;#3、结果为0xA5CCBBAA;"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"Patch后共享内存内容: {hex(int.from_bytes(ret2, 'little'))}")
        assert int.from_bytes(ret2, 'little') == 0xA5CCBBAA


@pytest.mark.skipif(cfg_data.TEST_ROM_PATCH_SUPPORT != 1,reason="ROM_PATCH功能不支持")
@allure.feature("rompatch")
@allure.description("配置patch有效时，其他功能可以正常进行;")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-RP008")
def test_ehsm_rp008(setup_function):
    with allure.step("1、读取共享内存0x60019000的内容;# 1、结果为0xA5332211;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        disable_config = get_enabled_bits("00000000", "00000000")
        assert 0 == host.write_otp(_gen_otp_config(disable_config, patch_en="disable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
        t, ret1 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"未Patch前共享内存内容: {hex(int.from_bytes(ret1, 'little'))}")
        assert int.from_bytes(ret1, 'little') == 0xA5332211
    with allure.step("2、配置OTP第1、5行patch命中，配置使能开关0b01，重启bootloader，等待patch替换;#2、重启成功;"):
        assert 0 == host.write_memory(SHARE_RAM_BASE, b'\x00\x00\x00\x00\x00\x00\x00\x00')
        PROBE0_OFFSET = get_probe0_offset()
        patch_config = get_enabled_bits("01010000", "00000000", addr=PROBE0_OFFSET)

        data_list = [patch_config["patch_data_full"][i:i+8] for i in range(0, len(patch_config["patch_data_full"]), 8)]
        data_list[0] = "1305A00A"  # 第 1 行 (probe0) 替换为返回 0xAA
        data_list[4] = "1305C00C"  # 第 5 行 (probe2) 替换为返回 0xCC
        patch_config["patch_data_full"] = "".join(data_list)

        assert 0 == host.write_otp(_gen_otp_config(patch_config, patch_en="enable"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_bl_done(1)
        time.sleep(5)
    with allure.step("3、读取共享内存0x60019000的内容，确认patch替换完成;#3、结果为0xA5CC22AA;"):
        t, ret2 = host.read_memory(SHARE_RAM_BASE, 4)
        log.info(f"Patch后共享内存内容: {hex(int.from_bytes(ret2, 'little'))}")
        assert int.from_bytes(ret2, 'little') == 0xA5CC22AA
    with allure.step("4、执行 api.ehsm_bl_get_version() 读取版本号，确认业务未受阻;#4、版本号读取成功;"):
        ver = api.ehsm_bl_get_version()
        log.info(f"读取到 BL 版本号: {ver}")
        assert ver is not None

