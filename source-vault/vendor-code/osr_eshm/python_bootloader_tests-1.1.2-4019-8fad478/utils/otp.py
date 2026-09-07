#!/usr/bin/env python3
# -*- coding:utf-8  -*-
"""
提供OTP数据生成功能
"""

import os
import toml
import allure
import pytest
import platform
import subprocess
import logging as log
from enum import IntEnum
from pathlib import Path
from utils import key
from utils.util import api
from utils.config import cfg_data

custom = f"{cfg_data.TEST_CUSTOM_ID:04x}"
otptool_dir = str(Path(__file__).parent.parent / "tools" / "third_part" / "otp_tool")
toml_dir = str(Path(__file__).parent.parent / "tools" / "otp_data" / custom )


def call_otptool(*args):
    """
    跨平台调用 otptool 的函数。

    :param args: otptool 的命令行参数
    :return: 命令的完整路径和参数列表
    """
    system = platform.system()
    if system == "Windows":
        otptool_path = os.path.join(otptool_dir, "x86_64-windows", "bin", "otptool.exe")
    elif system == "Linux":
        otptool_path = os.path.join(otptool_dir, "x86_64-linux", "bin", "otptool")
    else:
        raise NotImplementedError(f"Unsupported operating system: {system}")

    command = [otptool_path] + list(args)
    return command


def gen_otp_data(layout_path, values_path, output_bin_path, output_txt_path):
    """
    生成 OTP 数据

    :param layout_path: Layout 文件的路径
    :param values_path: Values 文件的路径
    :param output_bin_path: 生成的二进制 OTP 数据文件的输出路径
    :param output_txt_path: 生成的可读文本文件的输出路径
    :return: OTP 文件的二进制字节信息
    """
    command = call_otptool('gen', '--layout', layout_path, '--values', values_path, '--output', output_bin_path, '--parse', output_txt_path)
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"生成 OTP 数据失败: {result.stderr}")
    try:
        with open(output_bin_path, 'rb') as f:
            return f.read()
    except FileNotFoundError:
        raise RuntimeError(f"无法读取生成的二进制文件: {output_bin_path}")


def parse_otp_data(layout_path, input_bin_path, output_txt_path):
    """
    解析 OTP 数据

    :param layout_path: 用于解析的 Layout 文件路径
    :param input_bin_path: 要解析的二进制 OTP 数据文件路径
    :param output_txt_path: 生成的可读文本文件的输出路径
    :return: 子进程调用结果
    """
    command = call_otptool('parse', '--layout', layout_path, '--input', input_bin_path, '--parse', output_txt_path)
    return subprocess.run(command, capture_output=True, text=True)

@api
@allure.step("将配置数据生成 OTP 数据")
def otp_to_bin(values_config=None, layout_config=None, output_bin_path=None, output_txt_path=None) -> bytes:
    """
    基于配置数据生成 OTP 二进制文件和文本描述文件。
    使用临时文件进行配置修改，不会改变原始的 values.toml 和 layout.toml 文件。

    :param values_config: 用于临时修改 values.toml 的配置字典，默认为空
    :param layout_config: 用于临时修改 layout.toml 的配置字典，默认为空。
                         字典结构应该匹配 TOML 格式，例如：
                         {"uid": {"add_crc32": True}}
                         {"bits": [{"name": "key_alg_sel", "choices": {"aes128": "10", "sm4": "00"}}]}
    :param output_bin_path: 生成的二进制 OTP 数据文件的输出路径，默认为项目内的 .otp.bin
    :param output_txt_path: 生成的可读文本文件的输出路径，默认为项目内的 .otp.txt
    :return: OTP 文件的二进制字节信息
    """
    import tempfile
    import shutil

    # 设置默认路径
    default_layout_path = os.path.join(toml_dir, 'layout.toml')
    default_values_path = os.path.join(toml_dir, 'values.toml')
    if output_bin_path is None:
        output_bin_path = os.path.join(os.path.dirname(__file__), '.otp.bin')
    if output_txt_path is None:
        output_txt_path = os.path.join(os.path.dirname(__file__), '.otp.txt')

    # 处理 layout 配置
    layout_path = default_layout_path
    temp_layout_file = None

    if layout_config:
        try:
            # 读取默认 layout.toml
            with open(default_layout_path, 'r', encoding='utf-8') as f:
                layout_data = toml.load(f)

            # 合并配置
            layout_data = _merge_layout_config(layout_data, layout_config)

            # 创建临时 layout 文件
            temp_layout_file = tempfile.NamedTemporaryFile(mode='w', suffix='.toml', delete=False, encoding='utf-8')
            toml.dump(layout_data, temp_layout_file)
            temp_layout_file.close()
            layout_path = temp_layout_file.name

        except Exception as e:
            if temp_layout_file and os.path.exists(temp_layout_file.name):
                os.unlink(temp_layout_file.name)
            log.warning(f"处理 layout_config 时出错: {str(e)}，使用默认 layout.toml")
            layout_path = default_layout_path

    # 处理 values 配置
    values_path = default_values_path
    temp_values_file = None

    try:
        # 读取默认 values.toml
        try:
            with open(default_values_path, 'r', encoding='utf-8') as f:
                values_data = toml.load(f)
        except Exception as e:
            raise RuntimeError(f"读取默认 values.toml 文件失败: {str(e)}")

        if values_config:
            try:
                # 检查配置键是否存在
                non_existent_keys = [key for key in values_config if key not in values_data]
                if non_existent_keys:
                    raise RuntimeError(f"以下配置键在 values.toml 中不存在: {', '.join(non_existent_keys)}")

                # 更新配置数据
                values_data.update(values_config)

                # 创建临时 values 文件
                temp_values_file = tempfile.NamedTemporaryFile(mode='w', suffix='.toml', delete=False, encoding='utf-8')
                toml.dump(values_data, temp_values_file)
                temp_values_file.close()
                values_path = temp_values_file.name
                log.debug(f"otp_values_temp_file_path : {values_path}")

            except Exception as e:
                if temp_values_file and os.path.exists(temp_values_file.name):
                    os.unlink(temp_values_file.name)
                raise RuntimeError(f"处理 values_config 时出错: {str(e)}")

        # 生成 OTP 数据
        return gen_otp_data(layout_path, values_path, output_bin_path, output_txt_path)

    finally:
        # 清理临时文件
        if temp_layout_file and os.path.exists(temp_layout_file.name):
            os.unlink(temp_layout_file.name)
        if temp_values_file and os.path.exists(temp_values_file.name):
            os.unlink(temp_values_file.name)


def _merge_layout_config(layout_data, layout_config):
    """
    合并 layout 配置到默认 layout 数据中

    :param layout_data: 默认的 layout 数据（字典）
    :param layout_config: 要合并的配置（字典）
    :return: 合并后的 layout 数据
    """
    result = layout_data.copy()

    for key, value in layout_config.items():
        if key in ['config']:
            # 合并 config 段
            if key in result:
                result[key].update(value)
            else:
                result[key] = value
        elif key in ['bytes', 'bits', 'key']:
            # 处理数组类型的段（bytes, bits, key）
            if not isinstance(value, list):
                raise ValueError(f"配置段 '{key}' 应该是列表格式")

            if key in result:
                # 按名称匹配并更新现有项
                for new_item in value:
                    if 'name' not in new_item:
                        raise ValueError(f"配置段 '{key}' 中的项必须包含 'name' 字段")

                    # 在现有数据中查找匹配的项
                    found = False
                    for i, existing_item in enumerate(result[key]):
                        if existing_item.get('name') == new_item['name']:
                            result[key][i].update(new_item)
                            found = True
                            break

                    if not found:
                        raise ValueError(f"在默认 layout 中未找到名为 '{new_item['name']}' 的 {key} 项")
            else:
                result[key] = value
        else:
            # 直接处理单个项（如 uid 等）
            # 在 bytes 段中查找匹配的名称
            found = False
            if 'bytes' in result:
                for i, item in enumerate(result['bytes']):
                    if item.get('name') == key:
                        result['bytes'][i].update(value)
                        found = True
                        break

            if not found:
                raise ValueError(f"在默认 layout 中未找到名为 '{key}' 的配置项")

    return result

# otp contral field config
class LifeCycle(IntEnum):
    LIFECYCLE_TEST    = 0
    LIFECYCLE_DEV     = 1
    LIFECYCLE_MANU    = 2
    LIFECYCLE_USER    = 3
    LIFECYCLE_DEBUG   = 4
    LIFECYCLE_DESTROY = 5

class PatchEnable(IntEnum):
    PATCH_ENABLE   = 0
    PATCH_DISABLE  = 1
    PATCH_ENABLE1  = 2
    PATCH_DISABLE1 = 3

class HwKeyEncAlgo(IntEnum):
    HW_KEY_ENC_ALGO_SM4_ECB    = 0
    HW_KEY_ENC_ALGO_AES128_ECB = 1

class EhsmVersionCounter(IntEnum):
    EHSM_VER_CNT_OTP0_VC0 = 0
    EHSM_VER_CNT_OTP0_VC1 = 1
    EHSM_VER_CNT_OTP0_VC2 = 2
    EHSM_VER_CNT_OTP0_VC3 = 3
    EHSM_VER_CNT_OTP1_VC0 = 4
    EHSM_VER_CNT_OTP1_VC1 = 5
    EHSM_VER_CNT_OTP1_VC2 = 6
    EHSM_VER_CNT_OTP1_VC3 = 7

class SocVersionCounter(IntEnum):
    SOC_VER_CNT_OTP0_VC0 = 0
    SOC_VER_CNT_OTP0_VC1 = 1
    SOC_VER_CNT_OTP0_VC2 = 2
    SOC_VER_CNT_OTP0_VC3 = 3
    SOC_VER_CNT_OTP1_VC0 = 4
    SOC_VER_CNT_OTP1_VC1 = 5
    SOC_VER_CNT_OTP1_VC2 = 6
    SOC_VER_CNT_OTP1_VC3 = 7

# otp key config
class OtpKeyType(IntEnum):
    OTP_KEY_TYPE_SYMM     = 0
    OTP_KEY_TYPE_PRIV     = 1
    OTP_KEY_TYPE_PUB_HASH = 2
    OTP_KEY_TYPE_RESERVED = 3

class OtpKeyHashType(IntEnum):
    OTP_KEY_HASH_TYPE_TRUE   = 0
    OTP_KEY_HASH_TYPE_FALSE  = 1
    OTP_KEY_HASH_TYPE_AUTO   = 2
    OTP_KEY_HASH_TYPE_NONE   = 3
    OTP_KEY_HASH_TYPE_SM3    = 4
    OTP_KEY_HASH_TYPE_SHA256 = 5

class OtpKeyLifecycle(IntEnum):
    """OTP密钥生命周期枚举"""
    UNBURNED  = 0  # 未烧录
    UNUSED    = 1  # 未使用
    AVAILABLE = 2  # 可用
    DISABLED  = 3  # 禁用
    DESTROIED = 4  # 销毁

class EhsmDebugAuthAlgo(IntEnum):
    EHSM_DEBUG_AUTH_ALGO_SM4_CMAC    = 0
    EHSM_DEBUG_AUTH_ALGO_SM2         = 1
    EHSM_DEBUG_AUTH_ALGO_AES128_CMAC = 2
    EHSM_DEBUG_AUTH_ALGO_AES256_CMAC = 3
    EHSM_DEBUG_AUTH_ALGO_ECC256      = 4
    EHSM_DEBUG_AUTH_ALGO_ECC384      = 5
    EHSM_DEBUG_AUTH_ALGO_INVALID     = 6

class SocDebugAuthAlgo(IntEnum):
    SOC_DEBUG_AUTH_ALGO_SM4_CMAC    = 0
    SOC_DEBUG_AUTH_ALGO_SM2         = 1
    SOC_DEBUG_AUTH_ALGO_AES128_CMAC = 2
    SOC_DEBUG_AUTH_ALGO_AES256_CMAC = 3
    SOC_DEBUG_AUTH_ALGO_ECC256      = 4
    SOC_DEBUG_AUTH_ALGO_ECC384      = 5
    SOC_DEBUG_AUTH_ALGO_INVALID     = 6

class EhsmVrfEncAlgo(IntEnum):
    EHSM_VRF_ENC_ALGO_SM4_CBC    = 0
    EHSM_VRF_ENC_ALGO_AES128_CBC = 1
    EHSM_VRF_ENC_ALGO_AES192_CBC = 2
    EHSM_VRF_ENC_ALGO_AES256_CBC = 3
    EHSM_VRF_ENC_ALGO_INVALID    = 4

class SocVrfEncAlgo(IntEnum):
    SOC_VRF_ENC_ALGO_SM4_CBC    = 0
    SOC_VRF_ENC_ALGO_AES128_CBC = 1
    SOC_VRF_ENC_ALGO_AES192_CBC = 2
    SOC_VRF_ENC_ALGO_AES256_CBC = 3
    SOC_VRF_ENC_ALGO_INVALID    = 4

class EhsmVerifyAlgo(IntEnum):
    EHSM_VERIFY_ALGO_SM4_CMAC    = 0
    EHSM_VERIFY_ALGO_SM2         = 1
    EHSM_VERIFY_ALGO_AES128_CMAC = 2
    EHSM_VERIFY_ALGO_AES256_CMAC = 3
    EHSM_VERIFY_ALGO_RSA2048     = 4
    EHSM_VERIFY_ALGO_RSA3072     = 5
    EHSM_VERIFY_ALGO_ECC256      = 6
    EHSM_VERIFY_ALGO_ECC384      = 7
    EHSM_VERIFY_ALGO_INVALID     = 8

class SocVerifyAlgo(IntEnum):
    SOC_VERIFY_ALGO_SM4_CMAC    = 0
    SOC_VERIFY_ALGO_SM2         = 1
    SOC_VERIFY_ALGO_AES128_CMAC = 2
    SOC_VERIFY_ALGO_AES256_CMAC = 3
    SOC_VERIFY_ALGO_RSA2048     = 4
    SOC_VERIFY_ALGO_RSA3072     = 5
    SOC_VERIFY_ALGO_ECC256      = 6
    SOC_VERIFY_ALGO_ECC384      = 7
    SOC_VERIFY_ALGO_INVALID     = 8

class EhsmUpgEncAlgo(IntEnum):
    EHSM_UPG_ENC_ALGO_SM4_CBC    = 0
    EHSM_UPG_ENC_ALGO_AES128_CBC = 1
    EHSM_UPG_ENC_ALGO_AES192_CBC = 2
    EHSM_UPG_ENC_ALGO_AES256_CBC = 3
    EHSM_UPG_ENC_ALGO_INVALID    = 4

class SocUpgEncAlgo(IntEnum):
    SOC_UPG_ENC_ALGO_SM4_CBC    = 0
    SOC_UPG_ENC_ALGO_AES128_CBC = 1
    SOC_UPG_ENC_ALGO_AES192_CBC = 2
    SOC_UPG_ENC_ALGO_AES256_CBC = 3
    SOC_UPG_ENC_ALGO_ECC256     = 4
    SOC_UPG_ENC_ALGO_RSA2048    = 5
    SOC_UPG_ENC_ALGO_INVALID    = 6

class EhsmUpgradeAlgo(IntEnum):
    EHSM_UPGRADE_ALGO_SM4_CMAC    = 0
    EHSM_UPGRADE_ALGO_SM2         = 1
    EHSM_UPGRADE_ALGO_AES128_CMAC = 2
    EHSM_UPGRADE_ALGO_AES256_CMAC = 3
    EHSM_UPGRADE_ALGO_RSA2048     = 4
    EHSM_UPGRADE_ALGO_RSA3072     = 5
    EHSM_UPGRADE_ALGO_ECC256      = 6
    EHSM_UPGRADE_ALGO_ECC384      = 7
    EHSM_UPGRADE_ALGO_INVALID     = 8

class SocUpgradeAlgo(IntEnum):
    SOC_UPGRADE_ALGO_SM4_CMAC    = 0
    SOC_UPGRADE_ALGO_SM2         = 1
    SOC_UPGRADE_ALGO_AES128_CMAC = 2
    SOC_UPGRADE_ALGO_AES256_CMAC = 3
    SOC_UPGRADE_ALGO_RSA2048     = 4
    SOC_UPGRADE_ALGO_RSA3072     = 5
    SOC_UPGRADE_ALGO_ECC256      = 6
    SOC_UPGRADE_ALGO_ECC384      = 7
    SOC_UPGRADE_ALGO_INVALID     = 8

# otp_tool_cmd_section
LifeCycleToCmdStr = {
    LifeCycle.LIFECYCLE_TEST:"test",
    LifeCycle.LIFECYCLE_DEV:"dev",
    LifeCycle.LIFECYCLE_MANU:"manu",
    LifeCycle.LIFECYCLE_USER:"user",
    LifeCycle.LIFECYCLE_DEBUG:"debug",
    LifeCycle.LIFECYCLE_DESTROY:"destroy",
}

PatchEnableToCmdStr = {
    PatchEnable.PATCH_ENABLE:"enable",
    PatchEnable.PATCH_DISABLE:"disable",
    PatchEnable.PATCH_ENABLE1:"enable1",
    PatchEnable.PATCH_DISABLE1:"disable1",
}

HwKeyEncAlgoToCmdStr = {
    HwKeyEncAlgo.HW_KEY_ENC_ALGO_SM4_ECB:"sm4",
    HwKeyEncAlgo.HW_KEY_ENC_ALGO_AES128_ECB:"aes128",
}

EhsmVersionCounterToCmdStr = {
    EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC0:key.VER_COUNTER_OTP0_VC0_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC1:key.VER_COUNTER_OTP0_VC1_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC2:key.VER_COUNTER_OTP0_VC2_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC3:key.VER_COUNTER_OTP0_VC3_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC0:key.VER_COUNTER_OTP1_VC0_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC1:key.VER_COUNTER_OTP1_VC1_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC2:key.VER_COUNTER_OTP1_VC2_STR,
    EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC3:key.VER_COUNTER_OTP1_VC3_STR,
}

SocVersionCounterToCmdStr = {
    SocVersionCounter.SOC_VER_CNT_OTP0_VC0:key.VER_COUNTER_OTP0_VC0_STR,
    SocVersionCounter.SOC_VER_CNT_OTP0_VC1:key.VER_COUNTER_OTP0_VC1_STR,
    SocVersionCounter.SOC_VER_CNT_OTP0_VC2:key.VER_COUNTER_OTP0_VC2_STR,
    SocVersionCounter.SOC_VER_CNT_OTP0_VC3:key.VER_COUNTER_OTP0_VC3_STR,
    SocVersionCounter.SOC_VER_CNT_OTP1_VC0:key.VER_COUNTER_OTP1_VC0_STR,
    SocVersionCounter.SOC_VER_CNT_OTP1_VC1:key.VER_COUNTER_OTP1_VC1_STR,
    SocVersionCounter.SOC_VER_CNT_OTP1_VC2:key.VER_COUNTER_OTP1_VC2_STR,
    SocVersionCounter.SOC_VER_CNT_OTP1_VC3:key.VER_COUNTER_OTP1_VC3_STR,
}

OtpKeyTypeToCmdStr = {
    OtpKeyType.OTP_KEY_TYPE_SYMM:"symm",
    OtpKeyType.OTP_KEY_TYPE_PRIV:"priv",
    OtpKeyType.OTP_KEY_TYPE_PUB_HASH:"pub",
    OtpKeyType.OTP_KEY_TYPE_RESERVED:"reserved", # TODO: don't support now
}

OtpKeyHashTypeToCmdValue = {
    OtpKeyHashType.OTP_KEY_HASH_TYPE_TRUE:True,
    OtpKeyHashType.OTP_KEY_HASH_TYPE_FALSE:False,
    OtpKeyHashType.OTP_KEY_HASH_TYPE_AUTO:"auto",
    OtpKeyHashType.OTP_KEY_HASH_TYPE_NONE:"none",
    OtpKeyHashType.OTP_KEY_HASH_TYPE_SM3:"sm3",
    OtpKeyHashType.OTP_KEY_HASH_TYPE_SHA256:"sha256",
}

OtpKeyLifecycleToCmdStr = {
    OtpKeyLifecycle.UNBURNED:"unburned",
    OtpKeyLifecycle.UNUSED:"unused",
    OtpKeyLifecycle.AVAILABLE:"available",
    OtpKeyLifecycle.DISABLED:"disabled",
    OtpKeyLifecycle.DESTROIED:"destroied",
}

EhsmVerifyAlgoToCmdStr = {
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC:"sm4",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2:"sm2",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC:"aes128",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES256_CMAC:"aes256", # TODO:not support now
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048:"rsa2048",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072:"rsa3072",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256:"eccp256r1",
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC384:"eccp384r1", # TODO:not support now
    EhsmVerifyAlgo.EHSM_VERIFY_ALGO_INVALID:"invalid",
}

SocVerifyAlgoToCmdStr = {
    SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC:"sm4",
    SocVerifyAlgo.SOC_VERIFY_ALGO_SM2:"sm2",
    SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC:"aes128",
    SocVerifyAlgo.SOC_VERIFY_ALGO_AES256_CMAC:"aes256", # TODO:not support now
    SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048:"rsa2048",
    SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072:"rsa3072",
    SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256:"eccp256r1",
    SocVerifyAlgo.SOC_VERIFY_ALGO_ECC384:"eccp384r1", # TODO:not support now
    SocVerifyAlgo.SOC_VERIFY_ALGO_INVALID:"invalid",
}

EhsmUpgradeAlgoToCmdStr = {
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC:"sm4",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2:"sm2",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC:"aes128",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES256_CMAC:"aes256", # TODO:not support now
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048:"rsa2048",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072:"rsa3072",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256:"eccp256r1",
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC384:"eccp384r1", # TODO:not support now
    EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_INVALID:"invalid",
}

SocUpgradeAlgoToCmdStr = {
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM4_CMAC:"sm4",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2:"sm2",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES128_CMAC:"aes128",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES256_CMAC:"aes256", # TODO:not support now
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA2048:"rsa2048",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA3072:"rsa3072",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC256:"eccp256r1",
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC384:"eccp384r1", # TODO:not support now
    SocUpgradeAlgo.SOC_UPGRADE_ALGO_INVALID:"invalid",
}

@api
@allure.step("根据配置生成 OTP 数据")
def generate_otp_data(
    lifecycle: LifeCycle = LifeCycle.LIFECYCLE_TEST,
    hw_key_enc_algo: HwKeyEncAlgo = HwKeyEncAlgo.HW_KEY_ENC_ALGO_AES128_ECB,
    patch_en: PatchEnable = PatchEnable.PATCH_DISABLE,
    ehsm_ver_cnt: EhsmVersionCounter = None,
    soc_ver_cnt: SocVersionCounter = None,
    ehsm_debug_auth_algo: EhsmDebugAuthAlgo = EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_INVALID,
    soc_debug_auth_algo: SocDebugAuthAlgo = SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_INVALID,
    ehsm_vrf_enc_algo: EhsmVrfEncAlgo = EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID,
    soc_vrf_enc_algo: SocVrfEncAlgo = SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID,
    ehsm_vrf_sign_algo: EhsmVerifyAlgo = EhsmVerifyAlgo.EHSM_VERIFY_ALGO_INVALID,
    soc_vrf_sign_algo: SocVerifyAlgo = SocVerifyAlgo.SOC_VERIFY_ALGO_INVALID,
    ehsm_upg_enc_algo: EhsmUpgEncAlgo = EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_INVALID,
    soc_upg_enc_algo: SocUpgEncAlgo = SocUpgEncAlgo.SOC_UPG_ENC_ALGO_INVALID,
    ehsm_upg_sign_algo: EhsmUpgradeAlgo = EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_INVALID,
    soc_upg_sign_algo: SocUpgradeAlgo = SocUpgradeAlgo.SOC_UPGRADE_ALGO_INVALID,
    key_data_config: dict = None,
    key_lifecycle_config: dict = None,
    key_type_config: dict = None,
    no_crc32_config: dict = None
) -> bytes:
    # 辅助函数：根据算法获取对应的 lifecycle 配置
    def get_key_lifecycle(algo_name: str, default: OtpKeyLifecycle = OtpKeyLifecycle.AVAILABLE) -> str:
        if key_lifecycle_config and algo_name in key_lifecycle_config:
            return OtpKeyLifecycleToCmdStr[key_lifecycle_config[algo_name]]
        return OtpKeyLifecycleToCmdStr[default]

    # 辅助函数：根据算法获取对应的 key_type 配置
    def get_key_type(algo_name: str, default: OtpKeyType = OtpKeyType.OTP_KEY_TYPE_SYMM) -> str:
        if key_type_config and algo_name in key_type_config:
            return OtpKeyTypeToCmdStr[key_type_config[algo_name]]
        return OtpKeyTypeToCmdStr[default]

    # 辅助函数：根据算法获取对应的 key_data 配置并转换为hex字符串
    def get_key_data_hex(algo_name: str, default_key_data: bytes) -> str:
        if key_data_config and algo_name in key_data_config:
            return key_data_config[algo_name].hex()
        return default_key_data.hex()

    # 辅助函数：根据算法获取对应的 no_crc32 配置
    def get_no_crc32(algo_name: str, default: bool = False) -> bool:
        if no_crc32_config and algo_name in no_crc32_config:
            return no_crc32_config[algo_name]
        return default

    # 辅助函数：根据算法和最终的 key_type 获取对应的 hash 配置
    def get_key_hash(algo_name: str, default_type: OtpKeyType, algo_enum) -> any:
        # 获取最终的密钥类型（枚举）
        final_type = key_type_config.get(algo_name, default_type) if key_type_config else default_type

        if final_type == OtpKeyType.OTP_KEY_TYPE_SYMM or final_type == OtpKeyType.OTP_KEY_TYPE_PRIV:
            return OtpKeyHashTypeToCmdValue[OtpKeyHashType.OTP_KEY_HASH_TYPE_FALSE]
        elif final_type == OtpKeyType.OTP_KEY_TYPE_PUB_HASH:
            # 精确匹配SM2算法，并显式指定 hash 算法为 sm3
            if (algo_enum == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_SM2 or
                algo_enum == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_SM2 or
                algo_enum == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2 or
                algo_enum == SocVerifyAlgo.SOC_VERIFY_ALGO_SM2 or
                algo_enum == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2 or
                algo_enum == SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2):
                return OtpKeyHashTypeToCmdValue[OtpKeyHashType.OTP_KEY_HASH_TYPE_SM3]
            else:
                return OtpKeyHashTypeToCmdValue[OtpKeyHashType.OTP_KEY_HASH_TYPE_TRUE]
        else:
            return None

    # 根据配置设置默认的版本计数器
    if ehsm_ver_cnt is None:
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP0_VC0
        else:
            ehsm_ver_cnt = EhsmVersionCounter.EHSM_VER_CNT_OTP1_VC0

    if soc_ver_cnt is None:
        if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
            soc_ver_cnt = SocVersionCounter.SOC_VER_CNT_OTP0_VC0
        else:
            soc_ver_cnt = SocVersionCounter.SOC_VER_CNT_OTP1_VC0

    # 根据 ehsm & soc 的 debug_auth_algo 选择正确的 debug_auth 密钥
    if cfg_data.TEST_EHSM_DEBUG_KEY_ID == cfg_data.TEST_SOC_DEBUG_KEY_ID:
        if ehsm_debug_auth_algo != soc_debug_auth_algo:
            soc_debug_auth_algo = ehsm_debug_auth_algo

    if ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_SM4_CMAC:
        ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_SM4
        ehsm_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_SM2_HASH
        else:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_SM2_COMPRESS+key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY
        ehsm_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_AES128_CMAC:
        ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_AES128
        ehsm_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_AES256_CMAC:
        ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_AES256
        ehsm_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_ECC256_HASH
        else:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY
        ehsm_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_ECC384_HASH
        else:
            ehsm_debug_auth_key = key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY
        ehsm_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_debug_auth_algo == EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_INVALID:
        ehsm_debug_auth_key = key.INVALID_OTP_KEY_DATA
        ehsm_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        ehsm_debug_auth_key = None
        ehsm_debug_auth_key_type = None

    if soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_SM4_CMAC:
        soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_SM4
        soc_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_SM2_HASH
        else:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_SM2_COMPRESS+key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY
        soc_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_AES128_CMAC:
        soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_AES128
        soc_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_AES256_CMAC:
        soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_AES256
        soc_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_ECC256_HASH
        else:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY
        soc_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_debug_auth_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_ECC384_HASH
        else:
            soc_debug_auth_key = key.SOC_DEBUG_SIGN_KEY_ECC384_PUBKEY
        soc_debug_auth_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_debug_auth_algo == SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_INVALID:
        soc_debug_auth_key = key.INVALID_OTP_KEY_DATA
        soc_debug_auth_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        soc_debug_auth_key = None
        soc_debug_auth_key_type = None

    # 根据 ehsm & soc 的 verify_encrypt_algo 选择正确的 verify_enc 密钥
    if cfg_data.TEST_EHSM_ENCRYPT_KEY_ID == cfg_data.TEST_SOC_ENCRYPT_KEY_ID:
        if ehsm_vrf_enc_algo != soc_vrf_enc_algo:
            soc_vrf_enc_algo = ehsm_vrf_enc_algo

    if ehsm_vrf_enc_algo == EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_SM4_CBC:
        ehsm_vrf_enc_key = key.EHSM_VERIFY_ENCRYPT_KEY_SM4
    elif ehsm_vrf_enc_algo == EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES128_CBC:
        ehsm_vrf_enc_key = key.EHSM_VERIFY_ENCRYPT_KEY_AES128
    elif ehsm_vrf_enc_algo == EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES192_CBC:
        ehsm_vrf_enc_key = key.EHSM_VERIFY_ENCRYPT_KEY_AES192
    elif ehsm_vrf_enc_algo == EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_AES256_CBC:
        ehsm_vrf_enc_key = key.EHSM_VERIFY_ENCRYPT_KEY_AES256
    elif ehsm_vrf_enc_algo == EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID:
        ehsm_vrf_enc_key = key.INVALID_OTP_KEY_DATA
    else:
        ehsm_vrf_enc_key = None

    if soc_vrf_enc_algo == SocVrfEncAlgo.SOC_VRF_ENC_ALGO_SM4_CBC:
        soc_vrf_enc_key = key.SOC_VERIFY_ENCRYPT_KEY_SM4
    elif soc_vrf_enc_algo == SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES128_CBC:
        soc_vrf_enc_key = key.SOC_VERIFY_ENCRYPT_KEY_AES128
    elif soc_vrf_enc_algo == SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES192_CBC:
        soc_vrf_enc_key = key.SOC_VERIFY_ENCRYPT_KEY_AES192
    elif soc_vrf_enc_algo == SocVrfEncAlgo.SOC_VRF_ENC_ALGO_AES256_CBC:
        soc_vrf_enc_key = key.SOC_VERIFY_ENCRYPT_KEY_AES256
    elif soc_vrf_enc_algo == SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID:
        soc_vrf_enc_key = key.INVALID_OTP_KEY_DATA
    else:
        soc_vrf_enc_key = None

    # 根据 ehsm & soc 的 verify_algo 选择正确的 verify_sign 密钥
    if cfg_data.TEST_EHSM_VERIFY_KEY_ID == cfg_data.TEST_SOC_VERIFY_KEY_ID:
        if ehsm_vrf_sign_algo != soc_vrf_sign_algo:
            soc_vrf_sign_algo = ehsm_vrf_sign_algo

    if ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC:
        ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_SM4
        ehsm_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_SM2_HASH
        else:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_SM2_COMPRESS+key.EHSM_VERIFY_SIGN_KEY_SM2_PUBKEY
        ehsm_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES128_CMAC:
        ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_AES128
        ehsm_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_AES256_CMAC:
        ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_AES256
        ehsm_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA2048:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_RSA2048_HASH
        else:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_RSA2048_E+key.EHSM_VERIFY_SIGN_KEY_RSA2048_N
        ehsm_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_RSA3072:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_RSA3072_HASH
        else:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_RSA3072_E+key.EHSM_VERIFY_SIGN_KEY_RSA3072_N
        ehsm_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_ECC256_HASH
        else:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_ECC256_PUBKEY
        ehsm_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_ECC384_HASH
        else:
            ehsm_vrf_sign_key = key.EHSM_VERIFY_SIGN_KEY_ECC384_PUBKEY
        ehsm_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_vrf_sign_algo == EhsmVerifyAlgo.EHSM_VERIFY_ALGO_INVALID:
        ehsm_vrf_sign_key = key.INVALID_OTP_KEY_DATA
        ehsm_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        ehsm_vrf_sign_key = None
        ehsm_vrf_sign_key_type = None

    if soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC:
        soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_SM4
        soc_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_SM2_HASH
        else:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_SM2_COMPRESS+key.SOC_VERIFY_SIGN_KEY_SM2_PUBKEY
        soc_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_AES128_CMAC:
        soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_AES128
        soc_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_AES256_CMAC:
        soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_AES256
        soc_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_RSA2048:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_RSA2048_HASH
        else:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_RSA2048_E+key.SOC_VERIFY_SIGN_KEY_RSA2048_N
        soc_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_RSA3072:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_RSA3072_HASH
        else:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_RSA3072_E+key.SOC_VERIFY_SIGN_KEY_RSA3072_N
        soc_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_ECC256_HASH
        else:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_ECC256_PUBKEY
        soc_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_vrf_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_ECC384_HASH
        else:
            soc_vrf_sign_key = key.SOC_VERIFY_SIGN_KEY_ECC384_PUBKEY
        soc_vrf_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_vrf_sign_algo == SocVerifyAlgo.SOC_VERIFY_ALGO_INVALID:
        soc_vrf_sign_key = key.INVALID_OTP_KEY_DATA
        soc_vrf_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        soc_vrf_sign_key = None
        soc_vrf_sign_key_type = None

    # 根据 ehsm & soc 的 upgrade_encrypt_algo 选择正确的 upgrade_enc 密钥
    if cfg_data.TEST_EHSM_UPGRADE_ENCRYPT_KEY_ID == cfg_data.TEST_SOC_UPGRADE_ENCRYPT_KEY_ID:
        if ehsm_upg_enc_algo != soc_upg_enc_algo:
            soc_upg_enc_algo = ehsm_upg_enc_algo

    # Reason: 当 EHSM_ENCRYPT_KEY_ID == EHSM_UPGRADE_ENCRYPT_KEY_ID 时，两者共用同一个 OTP key slot，
    # 必须使用相同的密钥，否则后写入的密钥会覆盖前面的，导致解密失败
    if cfg_data.TEST_EHSM_ENCRYPT_KEY_ID == cfg_data.TEST_EHSM_UPGRADE_ENCRYPT_KEY_ID:
        ehsm_upg_enc_key = ehsm_vrf_enc_key
    elif ehsm_upg_enc_algo == EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_SM4_CBC:
        ehsm_upg_enc_key = key.EHSM_UPGRADE_ENCRYPT_KEY_SM4
    elif ehsm_upg_enc_algo == EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES128_CBC:
        ehsm_upg_enc_key = key.EHSM_UPGRADE_ENCRYPT_KEY_AES128
    elif ehsm_upg_enc_algo == EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES192_CBC:
        ehsm_upg_enc_key = key.EHSM_UPGRADE_ENCRYPT_KEY_AES192
    elif ehsm_upg_enc_algo == EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_AES256_CBC:
        ehsm_upg_enc_key = key.EHSM_UPGRADE_ENCRYPT_KEY_AES256
    elif ehsm_upg_enc_algo == EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_INVALID:
        ehsm_upg_enc_key = key.INVALID_OTP_KEY_DATA
    else:
        ehsm_upg_enc_key = None

    # Reason: 当 SOC_ENCRYPT_KEY_ID == SOC_UPGRADE_ENCRYPT_KEY_ID 时，两者共用同一个 OTP key slot，
    # 必须使用相同的密钥，否则后写入的密钥会覆盖前面的，导致解密失败
    if cfg_data.TEST_SOC_ENCRYPT_KEY_ID == cfg_data.TEST_SOC_UPGRADE_ENCRYPT_KEY_ID:
        soc_upg_enc_key = soc_vrf_enc_key
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_SM4_CBC:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_SM4
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES128_CBC:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_AES128
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES192_CBC:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_AES192
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_AES256_CBC:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_AES256
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_ECC256:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_ECC256
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_RSA2048:
        soc_upg_enc_key = key.SOC_UPGRADE_ENCRYPT_KEY_RSA2048
    elif soc_upg_enc_algo == SocUpgEncAlgo.SOC_UPG_ENC_ALGO_INVALID:
        soc_upg_enc_key = key.INVALID_OTP_KEY_DATA
    else:
        soc_upg_enc_key = None

    # 根据 ehsm & soc 的 upgrade_algo 选择正确的 upgrade_enc & upgrade_sign 密钥
    if cfg_data.TEST_EHSM_UPGRADE_VERIFY_KEY_ID == cfg_data.TEST_SOC_UPGRADE_VERIFY_KEY_ID:
        if ehsm_upg_sign_algo != soc_upg_sign_algo:
            soc_upg_sign_algo = ehsm_upg_sign_algo

    if ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM4_CMAC:
        ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_SM4
        ehsm_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_SM2_HASH
        else:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_SM2_COMPRESS+key.EHSM_UPGRADE_SIGN_KEY_SM2_PUBKEY
        ehsm_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES128_CMAC:
        ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_AES128
        ehsm_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_AES256_CMAC:
        ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_AES256
        ehsm_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA2048:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_RSA2048_HASH
        else:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_RSA2048_E+key.EHSM_UPGRADE_SIGN_KEY_RSA2048_N
        ehsm_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_RSA3072:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_RSA3072_HASH
        else:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_RSA3072_E+key.EHSM_UPGRADE_SIGN_KEY_RSA3072_N
        ehsm_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_ECC256_HASH
        else:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_ECC256_PUBKEY
        ehsm_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("ehsm_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_ECC384_HASH
        else:
            ehsm_upg_sign_key = key.EHSM_UPGRADE_SIGN_KEY_ECC384_PUBKEY
        ehsm_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif ehsm_upg_sign_algo == EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_INVALID:
        ehsm_upg_sign_key = key.INVALID_OTP_KEY_DATA
        ehsm_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        ehsm_upg_sign_key = None
        ehsm_upg_sign_key_type = None

    if soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM4_CMAC:
        soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_SM4
        soc_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_SM2:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_SM2_HASH
        else:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_SM2_COMPRESS+key.SOC_UPGRADE_SIGN_KEY_SM2_PUBKEY
        soc_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES128_CMAC:
        soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_AES128
        soc_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_AES256_CMAC:
        soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_AES256
        soc_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA2048:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_RSA2048_HASH
        else:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_RSA2048_E+key.SOC_UPGRADE_SIGN_KEY_RSA2048_N
        soc_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_RSA3072:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_RSA3072_HASH
        else:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_RSA3072_E+key.SOC_UPGRADE_SIGN_KEY_RSA3072_N
        soc_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC256:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_ECC256_HASH
        else:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_ECC256_PUBKEY
        soc_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_ECC384:
        # negative_test: 检查key_type_config，非对称算法的key_type配置为PRIV或SYMM时，otp_tools要求输入长度为32字节，使用hash值
        configured_type = key_type_config.get("soc_upg_sign_algo") if key_type_config else None
        if configured_type in [OtpKeyType.OTP_KEY_TYPE_PRIV, OtpKeyType.OTP_KEY_TYPE_SYMM]:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_ECC384_HASH
        else:
            soc_upg_sign_key = key.SOC_UPGRADE_SIGN_KEY_ECC384_PUBKEY
        soc_upg_sign_key_type = configured_type or OtpKeyType.OTP_KEY_TYPE_PUB_HASH
    elif soc_upg_sign_algo == SocUpgradeAlgo.SOC_UPGRADE_ALGO_INVALID:
        soc_upg_sign_key = key.INVALID_OTP_KEY_DATA
        soc_upg_sign_key_type = OtpKeyType.OTP_KEY_TYPE_SYMM
    else:
        soc_upg_sign_key = None
        soc_upg_sign_key_type = None

    otp_config = {
        "lifecycle": LifeCycleToCmdStr[lifecycle],
        "patch_en": PatchEnableToCmdStr[patch_en],
        "key_alg_sel": HwKeyEncAlgoToCmdStr[hw_key_enc_algo],
        "hsm_ver_ctr": EhsmVersionCounterToCmdStr[ehsm_ver_cnt],
        "soc_ver_ctr": SocVersionCounterToCmdStr[soc_ver_cnt],
        "hsm_verify_alg": EhsmVerifyAlgoToCmdStr[ehsm_vrf_sign_algo],
        "soc_verify_alg": SocVerifyAlgoToCmdStr[soc_vrf_sign_algo],
        "hsm_upgrade_alg": EhsmUpgradeAlgoToCmdStr[ehsm_upg_sign_algo],
        "soc_upgrade_alg": SocUpgradeAlgoToCmdStr[soc_upg_sign_algo],
    }

    # 根据各个算法是否为INVALID来判断对应Key分组是否存在
    # EHSM Debug Key
    if ehsm_debug_auth_algo != EhsmDebugAuthAlgo.EHSM_DEBUG_AUTH_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_EHSM_DEBUG_KEY_ID}"] = {
            "value" : get_key_data_hex("ehsm_debug_auth_algo", ehsm_debug_auth_key),
            "level" : 1,
            "lifecycle" : get_key_lifecycle("ehsm_debug_auth_algo"),
            "type" : get_key_type("ehsm_debug_auth_algo", ehsm_debug_auth_key_type),
            "hash" : get_key_hash("ehsm_debug_auth_algo", ehsm_debug_auth_key_type, ehsm_debug_auth_algo),
            "no_crc32" : get_no_crc32("ehsm_debug_auth_algo"),
        }

    # SOC Debug Key
    if soc_debug_auth_algo != SocDebugAuthAlgo.SOC_DEBUG_AUTH_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_SOC_DEBUG_KEY_ID}"] = {
            "value" : get_key_data_hex("soc_debug_auth_algo", soc_debug_auth_key),
            "level" : 2,
            "lifecycle" : get_key_lifecycle("soc_debug_auth_algo"),
            "type" : get_key_type("soc_debug_auth_algo", soc_debug_auth_key_type),
            "hash" : get_key_hash("soc_debug_auth_algo", soc_debug_auth_key_type, soc_debug_auth_algo),
            "no_crc32" : get_no_crc32("soc_debug_auth_algo"),
        }

    # EHSM Encrypt Key
    if ehsm_vrf_enc_algo != EhsmVrfEncAlgo.EHSM_VRF_ENC_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_EHSM_ENCRYPT_KEY_ID}"] = {
            "value" : get_key_data_hex("ehsm_vrf_enc_algo", ehsm_vrf_enc_key),
            "level" : 1,
            "lifecycle" : get_key_lifecycle("ehsm_vrf_enc_algo"),
            "type" : get_key_type("ehsm_vrf_enc_algo"),
            "hash" : False,
            "no_crc32" : get_no_crc32("ehsm_vrf_enc_algo"),
        }

    # SOC Encrypt Key
    if soc_vrf_enc_algo != SocVrfEncAlgo.SOC_VRF_ENC_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_SOC_ENCRYPT_KEY_ID}"] = {
            "value" : get_key_data_hex("soc_vrf_enc_algo", soc_vrf_enc_key),
            "level" : 2,
            "lifecycle" : get_key_lifecycle("soc_vrf_enc_algo"),
            "type" : get_key_type("soc_vrf_enc_algo"),
            "hash" : False,
            "no_crc32" : get_no_crc32("soc_vrf_enc_algo"),
        }

    # EHSM Verify Key
    if ehsm_vrf_sign_algo != EhsmVerifyAlgo.EHSM_VERIFY_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_EHSM_VERIFY_KEY_ID}"] = {
            "value" : get_key_data_hex("ehsm_vrf_sign_algo", ehsm_vrf_sign_key),
            "level" : 1,
            "lifecycle" : get_key_lifecycle("ehsm_vrf_sign_algo"),
            "type" : get_key_type("ehsm_vrf_sign_algo", ehsm_vrf_sign_key_type),
            "hash" : get_key_hash("ehsm_vrf_sign_algo", ehsm_vrf_sign_key_type, ehsm_vrf_sign_algo),
            "no_crc32" : get_no_crc32("ehsm_vrf_sign_algo"),
        }

    # SOC Verify Key
    if soc_vrf_sign_algo != SocVerifyAlgo.SOC_VERIFY_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_SOC_VERIFY_KEY_ID}"] = {
            "value" : get_key_data_hex("soc_vrf_sign_algo", soc_vrf_sign_key),
            "level" : 2,
            "lifecycle" : get_key_lifecycle("soc_vrf_sign_algo"),
            "type" : get_key_type("soc_vrf_sign_algo", soc_vrf_sign_key_type),
            "hash" : get_key_hash("soc_vrf_sign_algo", soc_vrf_sign_key_type, soc_vrf_sign_algo),
            "no_crc32" : get_no_crc32("soc_vrf_sign_algo"),
        }

    # EHSM Upgrade Encrypt Key
    if ehsm_upg_enc_algo != EhsmUpgEncAlgo.EHSM_UPG_ENC_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_EHSM_UPGRADE_ENCRYPT_KEY_ID}"] = {
            "value" : get_key_data_hex("ehsm_upg_enc_algo", ehsm_upg_enc_key),
            "level" : 1,
            "lifecycle" : get_key_lifecycle("ehsm_upg_enc_algo"),
            "type" : get_key_type("ehsm_upg_enc_algo"),
            "hash" : False,
            "no_crc32" : get_no_crc32("ehsm_upg_enc_algo"),
        }

    # SOC Upgrade Encrypt Key
    if soc_upg_enc_algo != SocUpgEncAlgo.SOC_UPG_ENC_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_SOC_UPGRADE_ENCRYPT_KEY_ID}"] = {
            "value" : get_key_data_hex("soc_upg_enc_algo", soc_upg_enc_key),
            "level" : 2,
            "lifecycle" : get_key_lifecycle("soc_upg_enc_algo"),
            "type" : get_key_type("soc_upg_enc_algo"),
            "hash" : False,
            "no_crc32" : get_no_crc32("soc_upg_enc_algo"),
        }

    # EHSM Upgrade Verify Key
    if ehsm_upg_sign_algo != EhsmUpgradeAlgo.EHSM_UPGRADE_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_EHSM_UPGRADE_VERIFY_KEY_ID}"] = {
            "value" : get_key_data_hex("ehsm_upg_sign_algo", ehsm_upg_sign_key),
            "level" : 1,
            "lifecycle" : get_key_lifecycle("ehsm_upg_sign_algo"),
            "type" : get_key_type("ehsm_upg_sign_algo", ehsm_upg_sign_key_type),
            "hash" : get_key_hash("ehsm_upg_sign_algo", ehsm_upg_sign_key_type, ehsm_upg_sign_algo),
            "no_crc32" : get_no_crc32("ehsm_upg_sign_algo"),
        }

    # SOC Upgrade Verify Key
    if soc_upg_sign_algo != SocUpgradeAlgo.SOC_UPGRADE_ALGO_INVALID:
        otp_config[f"key{cfg_data.TEST_SOC_UPGRADE_VERIFY_KEY_ID}"] = {
            "value" : get_key_data_hex("soc_upg_sign_algo", soc_upg_sign_key),
            "level" : 2,
            "lifecycle" : get_key_lifecycle("soc_upg_sign_algo"),
            "type" : get_key_type("soc_upg_sign_algo", soc_upg_sign_key_type),
            "hash" : get_key_hash("soc_upg_sign_algo", soc_upg_sign_key_type, soc_upg_sign_algo),
            "no_crc32" : get_no_crc32("soc_upg_sign_algo"),
        }
    try:
        otp_data = otp_to_bin(otp_config)
        log.debug("Gen command output (bytes): %s", ' '.join([f'{b:02x}' for b in otp_data]))
    except Exception as e:
        otp_data = None
        log.debug(f"生成 OTP 数据时出错: {str(e)}")
        pytest.fail("生成 OTP 数据失败")
    return otp_data


@api
@allure.step("生成指定密钥的OTP二进制数据")
def otp_to_bin_with_key_limit(values_config=None, key_num=3, output_bin_path=None, output_txt_path=None) -> bytes:
    """
    类似 otp_to_bin 接口，但只处理指定的OTP密钥。
    通过读取默认layout.toml文件，然后根据key_num参数保留指定的[[key]]定义。
    使用临时文件进行配置修改，不会改变原始的 values.toml 和 layout.toml 文件。

    :param values_config: 用于临时修改 values.toml 的配置字典，默认为空
    :param key_num: 要保留的密钥，支持两种格式：
                   - int: 保留前N个密钥，默认为3（前三个密钥）
                   - list: 保留指定索引的密钥，例如[0,2]表示保留第1个和第3个密钥
    :param output_bin_path: 生成的二进制 OTP 数据文件的输出路径，默认为项目内的 .otp.bin
    :param output_txt_path: 生成的可读文本文件的输出路径，默认为项目内的 .otp.txt
    :return: OTP 文件的二进制字节信息
    """
    import tempfile
    import shutil

    # 设置默认路径
    default_layout_path = os.path.join(toml_dir, 'layout.toml')
    default_values_path = os.path.join(toml_dir, 'values.toml')
    if output_bin_path is None:
        output_bin_path = os.path.join(os.path.dirname(__file__), '.otp.bin')
    if output_txt_path is None:
        output_txt_path = os.path.join(os.path.dirname(__file__), '.otp.txt')

    # 验证key_num参数
    if isinstance(key_num, int):
        if key_num < 0:
            raise ValueError("key_num不能为负数")
    elif isinstance(key_num, list):
        if not all(isinstance(idx, int) and idx >= 0 for idx in key_num):
            raise ValueError("key_num列表中的所有元素必须为非负整数")
        if len(key_num) != len(set(key_num)):
            raise ValueError("key_num列表中不能包含重复的索引")
    else:
        raise ValueError("key_num必须为int类型或list类型")

    # 处理 layout 配置 - 读取并限制密钥数量
    layout_path = default_layout_path
    temp_layout_file = None

    try:
        # 读取默认 layout.toml
        with open(default_layout_path, 'r', encoding='utf-8') as f:
            layout_data = toml.load(f)

        # 检查是否有key段
        if 'key' not in layout_data:
            raise ValueError("layout.toml文件中没有找到[[key]]段")

        # 根据key_num参数选择密钥
        original_keys = layout_data['key']

        if isinstance(key_num, int):
            # 传统的int模式：保留前N个密钥
            if key_num > len(original_keys):
                log.warning(f"指定的key_num({key_num})大于实际密钥数量({len(original_keys)})，将使用所有可用密钥")
                key_num = len(original_keys)

            if key_num == 0:
                # 当 key_num=0 时，创建空的密钥数组
                limited_keys = []
                log.info(f"原始layout包含{len(original_keys)}个密钥，现在设置为不包含任何密钥")
            else:
                limited_keys = original_keys[:key_num]
                log.info(f"原始layout包含{len(original_keys)}个密钥，现在限制为前{key_num}个密钥")
                for i, key_def in enumerate(limited_keys):
                    log.info(f"  保留密钥{i+1}: {key_def.get('name', 'unnamed')} (偏移: {key_def.get('offset', 'unknown')})")

        elif isinstance(key_num, list):
            # 新的list模式：保留指定索引的密钥
            if not key_num:
                # 空列表，不保留任何密钥
                limited_keys = []
                log.info(f"原始layout包含{len(original_keys)}个密钥，现在设置为不包含任何密钥")
            else:
                # 验证索引是否有效
                invalid_indices = [idx for idx in key_num if idx >= len(original_keys)]
                if invalid_indices:
                    raise ValueError(f"以下密钥索引超出范围: {invalid_indices}，实际密钥数量: {len(original_keys)}")

                # 按索引选择密钥
                limited_keys = [original_keys[idx] for idx in key_num]
                log.info(f"原始layout包含{len(original_keys)}个密钥，现在选择索引{key_num}的密钥")
                for idx, key_def in zip(key_num, limited_keys):
                    log.info(f"  保留密钥索引{idx}: {key_def.get('name', 'unnamed')} (偏移: {key_def.get('offset', 'unknown')})")

        layout_data['key'] = limited_keys

        # 创建临时 layout 文件
        temp_layout_file = tempfile.NamedTemporaryFile(mode='w', suffix='.toml', delete=False, encoding='utf-8')
        toml.dump(layout_data, temp_layout_file)
        temp_layout_file.close()
        layout_path = temp_layout_file.name

    except Exception as e:
        if temp_layout_file and os.path.exists(temp_layout_file.name):
            os.unlink(temp_layout_file.name)
        log.warning(f"处理 layout 配置时出错: {str(e)}，使用默认 layout.toml")
        layout_path = default_layout_path

    # 处理 values 配置 - 与 otp_to_bin 保持完全一致的逻辑
    values_path = default_values_path
    temp_values_file = None

    try:
        # 读取默认 values.toml
        try:
            with open(default_values_path, 'r', encoding='utf-8') as f:
                values_data = toml.load(f)
        except Exception as e:
            raise RuntimeError(f"读取默认 values.toml 文件失败: {str(e)}")

        if values_config:
            try:
                # 检查配置键是否存在
                non_existent_keys = [key for key in values_config if key not in values_data]
                if non_existent_keys:
                    raise RuntimeError(f"以下配置键在 values.toml 中不存在: {', '.join(non_existent_keys)}")

                # 更新配置数据
                values_data.update(values_config)

                # 创建临时 values 文件
                temp_values_file = tempfile.NamedTemporaryFile(mode='w', suffix='.toml', delete=False, encoding='utf-8')
                toml.dump(values_data, temp_values_file)
                temp_values_file.close()
                values_path = temp_values_file.name

            except Exception as e:
                if temp_values_file and os.path.exists(temp_values_file.name):
                    os.unlink(temp_values_file.name)
                raise RuntimeError(f"处理 values_config 时出错: {str(e)}")

        # 生成 OTP 数据
        return gen_otp_data(layout_path, values_path, output_bin_path, output_txt_path)

    finally:
        # 清理临时文件
        if temp_layout_file and os.path.exists(temp_layout_file.name):
            os.unlink(temp_layout_file.name)
        if temp_values_file and os.path.exists(temp_values_file.name):
            os.unlink(temp_values_file.name)
