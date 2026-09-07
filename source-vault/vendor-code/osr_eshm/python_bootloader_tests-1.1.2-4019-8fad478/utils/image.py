import allure
import sys, os
import tempfile
import subprocess
import logging as log
from enum import IntEnum
from typing import Tuple, Union
from Cryptodome.Cipher import AES
from Cryptodome.Util.Padding import unpad
from gmssl import sm4
from utils import image, key, logger
from utils.otp import EhsmVerifyAlgo, SocVerifyAlgo
from utils.util import api
from utils.config import cfg_data

# Reason: 新工具 imgtool 是预编译的 Rust 可执行文件，根据平台选择对应二进制
_IMGTOOL_BASE = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "tools", "third_part", "ehsm_image_tool"))
if sys.platform == "win32":
    tool_path = os.path.join(_IMGTOOL_BASE, "x86_64-windows", "bin", "imgtool.exe")
else:
    tool_path = os.path.join(_IMGTOOL_BASE, "x86_64-linux", "bin", "imgtool")

def _write_key_toml(key_str: str, path: str) -> None:
    """
    将旧格式密钥字符串（ALGO:HEX）转换为新工具所需的 TOML 密钥文件。

    旧格式布局（HEX 拼接顺序）：
      - 对称 sm4/aes128/aes192/aes256：直接是密钥值
      - sm2：04<pub_64byte><priv_32byte>  公钥含 0x04 前缀，共 65 字节=130hex
      - ecc256：<pub_64byte><priv_32byte> 公钥不含 0x04，共 64 字节=128hex
      - rsa2048：<e_64byte><n_256byte><d_256byte>
      - rsa3072：<e_64byte><n_384byte><d_384byte>
    """
    algo, _, hex_val = key_str.partition(":")
    algo = algo.lower()

    if algo in ("sm4", "aes128", "aes192", "aes256"):
        # Reason: 对称密钥直接写 value 字段
        content = f'type = "{algo}"\nvalue = "{hex_val.upper()}"\n'

    elif algo == "sm2":
        # Reason: SM2 公钥含 0x04 前缀 65字节=130hex，后跟私钥 32字节=64hex
        pub_hex = hex_val[:130].upper()
        priv_hex = hex_val[130:].upper()
        content = (
            f'type = "sm2"\n\n'
            f'[value]\n'
            f'public = "{pub_hex}"\n'
            f'private = "{priv_hex}"\n'
        )

    elif algo == "ecc256":
        # Reason: ECC256 公钥不含 0x04 前缀 64字节=128hex，后跟私钥 32字节=64hex
        pub_hex = hex_val[:128].upper()
        priv_hex = hex_val[128:].upper()
        content = (
            f'type = "ecc256"\n\n'
            f'[value]\n'
            f'public = "{pub_hex}"\n'
            f'private = "{priv_hex}"\n'
        )

    elif algo == "rsa2048":
        # Reason: RSA2048 = e(64字节=128hex) + n(256字节=512hex) + d(256字节=512hex)
        e_hex = hex_val[:128].upper()
        n_hex = hex_val[128:128 + 512].upper()
        d_hex = hex_val[128 + 512:].upper()
        content = (
            f'type = "rsa2048"\n\n'
            f'[value]\n'
            f'e = "{e_hex}"\n'
            f'n = "{n_hex}"\n'
            f'd = "{d_hex}"\n'
        )

    elif algo == "rsa3072":
        # Reason: RSA3072 = e(64字节=128hex) + n(384字节=768hex) + d(384字节=768hex)
        e_hex = hex_val[:128].upper()
        n_hex = hex_val[128:128 + 768].upper()
        d_hex = hex_val[128 + 768:].upper()
        content = (
            f'type = "rsa3072"\n\n'
            f'[value]\n'
            f'e = "{e_hex}"\n'
            f'n = "{n_hex}"\n'
            f'd = "{d_hex}"\n'
        )

    else:
        raise ValueError(f"Unsupported key algorithm: {algo}")

    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w") as f:
        f.write(content)


def _parse_extra_data(extra_str: str) -> list:
    """
    解析 extra_data 字符串为列表。
    格式："offset:hex_value"，多条以逗号分隔，例如 "599:01,600:FF"
    """
    result = []
    for item in extra_str.split(","):
        item = item.strip()
        if not item:
            continue
        offset_str, _, value_str = item.partition(":")
        result.append({"offset": int(offset_str), "value": value_str.upper()})
    return result


def _build_extra_data_toml(entries: list, section: str) -> str:
    """将 extra_data 列表转换为 TOML 格式字符串，section 如 'boot' 或 'upgrade.upgrade_layer'"""
    lines = []
    for entry in entries:
        lines.append(f'\n[[{section}.extra_data]]')
        lines.append(f'offset = {entry["offset"]}')
        lines.append(f'value = "{entry["value"]}"')
    return "\n".join(lines)


def _toml_path(path: str) -> str:
    """将路径中的反斜杠替换为正斜杠，避免 TOML 字符串中的转义序列错误。"""
    return path.replace("\\", "/")


def _run_imgtool(config_path: str, output_path: str) -> Tuple[int, bytes]:
    """调用新工具 imgtool，返回 (status_code, image_bytes_or_error_bytes)"""
    command = [tool_path, config_path, "-o", output_path, "-f"]
    log.info(f"imgtool command: {' '.join(command)}")
    result = None
    try:
        result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        log.info(f"STDOUT: {result.stdout.decode(errors='replace')}")
        with open(output_path, "rb") as f:
            return 0, f.read()
    except subprocess.CalledProcessError as e:
        stdout = e.stdout.decode(errors="replace") if e.stdout else ""
        stderr = e.stderr.decode(errors="replace") if e.stderr else ""
        log.error(f"STDOUT: {stdout}")
        log.error(f"STDERR: {stderr}")
        return 1, (e.stderr or b"imgtool subprocess error")
    except FileNotFoundError as e:
        log.error(f"imgtool not found: {e}")
        return 2, b"imgtool not found"
    except Exception as e:
        log.error(f"Exception: {e}")
        return 3, str(e).encode()


@api
@allure.step("制作安全启动镜像  {params}")
def create_boot_image(params: dict) -> Tuple[int, bytes]:
    """
    创建启动镜像的接口（使用新工具 imgtool，TOML 配置文件驱动）。
    :param params: 启动镜像生成所需的参数字典，包含以下键：
                   - 'fw_image': 启动镜像输入文件路径（相对 resource/image/）。
                   - 'image_type': 镜像类型字符串，如 "ehsm-ehsmkey"。
                   - 'version': 版本计数器（32位hex字符串，裸镜像可省略）。
                   - 'enc_key': 加密密钥（ALGO:HEX格式，可选）。
                   - 'sign_key': 签名密钥（ALGO:HEX格式，无则生成裸镜像）。
                   - 'output_image': 输出镜像文件名（相对 resource/image/）。
                   - 'iv': 初始化向量（32位hex字符串，可选）。
                   - 'extra_boot_data': 额外数据（"offset:hex_value"格式，可选）。
                   - 'naked_flag': 裸镜像标志（True时忽略密钥参数）。
    :rtype: Tuple[int, bytes]
    """
    output_image = params.get("output_image", "")
    output_path = os.path.abspath(f"resource/image/{output_image}")
    input_path = os.path.abspath(f"resource/image/{params.get('fw_image', '')}")
    naked_flag = params.get("naked_flag", False)

    # Reason: 使用临时目录存放密钥 TOML 和配置文件，避免多次调用间互相覆盖
    with tempfile.TemporaryDirectory() as tmp_dir:
        version_counter = params.get("version", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
        toml_lines = [
            '[general]',
            'mode = "boot"',
            f'output = "{_toml_path(output_path)}"',
            f'input_bin = "{_toml_path(input_path)}"',
            f'image_type = "{params.get("image_type", "ehsm-ehsmkey")}"',
            f'version_counter = "{version_counter}"',
            'export_c_array = false',
            '',
            '[boot]',
        ]

        if not naked_flag:
            sign_key_str = params.get("sign_key")
            enc_key_str = params.get("enc_key")
            iv_str = params.get("iv")

            if sign_key_str:
                sign_key_path = os.path.join(tmp_dir, "sign_key.toml")
                _write_key_toml(sign_key_str, sign_key_path)
                toml_lines.append(f'sign_key = "{_toml_path(sign_key_path)}"')

            if enc_key_str:
                enc_key_path = os.path.join(tmp_dir, "enc_key.toml")
                _write_key_toml(enc_key_str, enc_key_path)
                toml_lines.append(f'enc_key = "{_toml_path(enc_key_path)}"')

            if iv_str:
                toml_lines.append(f'iv = "{iv_str}"')

            extra_boot_data = params.get("extra_boot_data")
            if extra_boot_data:
                entries = _parse_extra_data(extra_boot_data)
                toml_lines.append(_build_extra_data_toml(entries, "boot"))

        config_content = "\n".join(toml_lines) + "\n"
        config_path = os.path.join(tmp_dir, "boot_config.toml")
        with open(config_path, "w") as f:
            f.write(config_content)
        log.info(f"boot config:\n{config_content}")

        return _run_imgtool(config_path, output_path)


@api
@allure.step("制作安全升级镜像  {params} ")
def create_upgrade_image(params: dict) -> Tuple[int, bytes]:
    """
    创建升级镜像的接口（使用新工具 imgtool，TOML 配置文件驱动）。
    :param params: 升级镜像生成所需的参数字典，包含以下键：
                   - 'fw_image': 固件镜像输入文件路径（相对 resource/image/）。
                   - 'image_type': 镜像类型字符串。
                   - 'version': 版本计数器（32位hex字符串）。
                   - 'enc_key': 升级层加密密钥（ALGO:HEX格式，可选）。
                   - 'vrf_sign_key': 启动层签名密钥（ALGO:HEX格式）。
                   - 'upg_sign_key': 升级层签名密钥（ALGO:HEX格式）。
                   - 'output_image': 输出镜像文件名（相对 resource/image/）。
                   - 'iv': 初始化向量（32位hex字符串，可选）。
                   - 'boot_plain_flag': 值为 "plain" 时设置启动层 plain=true（可选）。
                   - 'extra_upgrade_data': 升级层额外数据（"offset:hex_value"格式，可选）。
                   - 'naked_flag': 裸镜像标志（可选）。
    :rtype: Tuple[int, bytes]
    """
    output_image = params.get("output_image", "")
    output_path = os.path.abspath(f"resource/image/{output_image}")
    input_path = os.path.abspath(f"resource/image/{params.get('fw_image', '')}")
    naked_flag = params.get("naked_flag", False)

    with tempfile.TemporaryDirectory() as tmp_dir:
        version_counter = params.get("version", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
        toml_lines = [
            '[general]',
            'mode = "upgrade"',
            f'output = "{_toml_path(output_path)}"',
            f'input_bin = "{_toml_path(input_path)}"',
            f'image_type = "{params.get("image_type", "ehsm-ehsmkey")}"',
            f'version_counter = "{version_counter}"',
            'export_c_array = false',
            '',
        ]

        # [upgrade.boot_layer] — 启动层：签名 + plain 标志
        vrf_sign_key_str = params.get("vrf_sign_key")
        boot_plain = params.get("boot_plain_flag") == "plain"
        toml_lines.append('[upgrade.boot_layer]')
        if vrf_sign_key_str and not naked_flag:
            vrf_key_path = os.path.join(tmp_dir, "vrf_sign_key.toml")
            _write_key_toml(vrf_sign_key_str, vrf_key_path)
            toml_lines.append(f'sign_key = "{_toml_path(vrf_key_path)}"')
        if boot_plain:
            toml_lines.append('plain = true')
        toml_lines.append('')

        # [upgrade.upgrade_layer] — 升级层：签名 + 加密 + IV + extra_data
        toml_lines.append('[upgrade.upgrade_layer]')
        if not naked_flag:
            upg_sign_key_str = params.get("upg_sign_key")
            enc_key_str = params.get("enc_key")
            iv_str = params.get("iv")

            if upg_sign_key_str:
                upg_key_path = os.path.join(tmp_dir, "upg_sign_key.toml")
                _write_key_toml(upg_sign_key_str, upg_key_path)
                toml_lines.append(f'sign_key = "{_toml_path(upg_key_path)}"')

            if enc_key_str:
                enc_key_path = os.path.join(tmp_dir, "enc_key.toml")
                _write_key_toml(enc_key_str, enc_key_path)
                toml_lines.append(f'enc_key = "{_toml_path(enc_key_path)}"')

            if iv_str:
                toml_lines.append(f'iv = "{iv_str}"')

            extra_upgrade_data = params.get("extra_upgrade_data")
            if extra_upgrade_data:
                entries = _parse_extra_data(extra_upgrade_data)
                toml_lines.append(_build_extra_data_toml(entries, "upgrade.upgrade_layer"))

        config_content = "\n".join(toml_lines) + "\n"
        config_path = os.path.join(tmp_dir, "upgrade_config.toml")
        with open(config_path, "w") as f:
            f.write(config_content)
        log.info(f"upgrade config:\n{config_content}")

        return _run_imgtool(config_path, output_path)


@api
@allure.step("制作 {corrupt_type} 非法安全启动镜像")
def make_invalid_boot_image(image: bytes, *, corrupt_type: str) -> bytes:
    """
    根据指定破坏类型，破坏启动镜像的某个字段并返回破坏后的镜像数据。
    支持的破坏类型包括：
        - "Signature"
        - "Public_Key"
        - "Encrypt_IV"
        - "Valid_Flag"
        - "Image_Type"
        - "Plain_Flag"
        - "Naked_Flag"
        - "User_Version"
        - "Code_Size"
        - "Version_Counter"

    :param image: 原始启动镜像的二进制数据。
    :type image: bytes
    :param corrupt_type: 要破坏的字段类型。
    :type corrupt_type: str
    :return: 被破坏后的镜像二进制数据。
    :rtype: bytes

    :raises ValueError: 如果给定的 corrupt_type 不在支持范围内。

    :example:
    >>> corrupted = make_invalid_boot_image(image_data, corrupt_type="Signature")
    >>> with open("corrupt_boot.img", "wb") as f:
    ...     f.write(corrupted)
    """
    corrupt_map = {
        "Signature": (0, 256),         # 假设签名在偏移
        "Public_Key": (256, 320),      # 公钥
        "Encrypt_IV": (576, 16),       # IV 偏移，长度 16
        "Valid_Flag": (592, 4),        # 有效标志位
        "Image_Type": (596, 1),        # 镜像类型
        "Plain_Flag": (597, 1),        # 明文标志
        "Naked_Flag": (598, 1),        # 裸露标志
        "User_Version": (599, 1),      # 研发镜像或商用镜像：• 0: 商用镜像 • 1: 研发镜像
        "Code_Size": (604, 4),         # 代码区大小，32位整数
        "Version_Counter": (608, 16),  # 版本计数器，16 byte
    }
    if corrupt_type not in corrupt_map:
        raise ValueError(f"Unsupported corrupt_type: {corrupt_type}")
    offset, length = corrupt_map[corrupt_type]
    corrupted = bytearray(image)
    # 用随机或固定错误值覆盖目标区域
    for i in range(length):
        corrupted[offset + i] ^= 0xFF  # 简单的按位取反破坏
    _save_bin_file(corrupted, "resource/image/"+ corrupt_type.lower().replace(" ", "_").replace("-", "_") + ".bin")
    return bytes(corrupted)


@api
@allure.step("制作 {corrupt_type} 非法安全升级镜像")
def make_invalid_upgrade_image(image: bytes, *, corrupt_type: str) -> bytes:
    """
    根据指定的错误类型破坏升级镜像中的字段并返回破坏后的镜像数据。
    支持的破坏类型包括：
        - "Upgrade_Signature"
        - "Upgrade_Public_Key"
        - "Upgrade_Encrypt_IV"
        - "Upgrade_Valid_Flag"
        - "Upgrade_Image_Type"
        - "Upgrade_Plain_Flag"
        - "Upgrade_Naked_Flag"
        - "Upgrade_User_Version"
        - "Upgrade_Image_Size"
        - "Upgrade_Version_Counter"

    :param image: 原始升级镜像的二进制内容。
    :type image: bytes
    :param corrupt_type: 需要破坏的字段名称。
    :type corrupt_type: str
    :return: 被破坏后的镜像数据。
    :rtype: bytes

    :raises ValueError: 如果 corrupt_type 不在支持列表中。
    """
    corrupt_map = {
        "Upgrade_Signature": (0, 256),           # 升级签名AES256/SM4 CMAC: 长度为 16 字节,RSA2048: 长度为 256 字节,RSA3072: 签名值的前 256 字节,SM2: 长度为 64 字节
        "Upgrade_Public_Key": (256, 320),         # 公钥
        "Upgrade_Encrypt_IV": (576, 16),        # 加密 IV（16字节）
        "Upgrade_Valid_Flag": (592, 4),         # 有效标志
        "Upgrade_Image_Type": (596, 1),                 # 镜像类型
        "Upgrade_Plain_Flag": (597, 1),         # 明文标志
        "Upgrade_Naked_Flag": (598, 1),         # 裸露标志
        "Upgrade_User_Version": (599, 1),              # 研发镜像或商用镜像
        "Upgrade_Image_Size": (604, 4),               # 升级镜像大小（4字节）
        "Upgrade_Version_Counter": (608, 16),            # 版本计数器（16字节）
    }

    if corrupt_type not in corrupt_map:
        raise ValueError(f"Unsupported corrupt_type: {corrupt_type}")

    offset, length = corrupt_map[corrupt_type]
    corrupted = bytearray(image)

    for i in range(length):
        corrupted[offset + i] ^= 0xFF  # 按位取反破坏字段数据
    _save_bin_file(corrupted, "resource/image/"+ corrupt_type.lower().replace(" ", "_").replace("-", "_") + ".bin")
    return bytes(corrupted)

def _save_bin_file(data: bytes, filename: str) -> None:
    """
    将二进制数据保存为 .bin 文件。

    :param data: 要写入的二进制数据
    :param filename: 文件名（包括路径）
    """
    with open(filename, "wb") as f:
        f.write(data)

# secure boot & secure patch & secure upgrade image generate
class ImageLevel(IntEnum):
    IMAGE_BL_BOOT    = 0
    IMAGE_BL_UPGRADE = 1
    IMAGE_BL_PATCH   = 2
    IMAGE_FW_BOOT    = 3
    IMAGE_FW_UPGRADE = 4

class ImageType(IntEnum):
    IMAGE_HSM_HSMK   = 0
    IMAGE_SOC_SOCK   = 1
    IMAGE_SOC_HSMK   = 2
    IMAGE_PATCH_HSMK = 3
    IMAGE_FORCE_CERT = 4
    IMAGE_RD_CERT    = 5

class ImageEncAlgo(IntEnum):
    IMAGE_ENC_ALGO_SM4    = 0
    IMAGE_ENC_ALGO_AES128 = 1
    IMAGE_ENC_ALGO_AES192 = 2
    IMAGE_ENC_ALGO_AES256 = 3
    IMAGE_ENC_ALGO_NONE   = 4

class ImageSignAlgo(IntEnum):
    IMAGE_SIGN_ALGO_SM4_CMAC    = 0
    IMAGE_SIGN_ALGO_SM2         = 1
    IMAGE_SIGN_ALGO_AES128_CMAC = 2
    IMAGE_SIGN_ALGO_AES256_CMAC = 3
    IMAGE_SIGN_ALGO_RSA2048     = 4
    IMAGE_SIGN_ALGO_RSA3072     = 5
    IMAGE_SIGN_ALGO_ECC256      = 6
    IMAGE_SIGN_ALGO_ECC384      = 7
    IMAGE_SIGN_ALGO_NONE        = 8

class ImageVersion(IntEnum):
    IMAGE_VC0 = 0
    IMAGE_VC1 = 1
    IMAGE_VC2 = 2
    IMAGE_VC3 = 3

class ImageCorruptionType(IntEnum):
    INVALID_VALID_FLAG = 1
    INVALID_IMAGE_TYPE = 2
    INVALID_PLAIN_FLAG = 3
    INVALID_NAKED_FLAG = 4
    INVALID_IMAGE_OWNER = 5
    INVALID_IMAGE_SIZE = 6
    INVALID_VERSION_CNT = 7
    CORRUPTED_SIGNATURE = 8
    CORRUPTED_PUBLIC_KEY = 9
    CORRUPTED_ENCRYPT_IV = 10
    CORRUPTED_PUB_KEY_EXT = 11
    CORRUPTED_ENCRYPT_IMG = 12

# image_tool_cmd_section
ImageTypeToCmdStr = {
    ImageType.IMAGE_HSM_HSMK:"ehsm-ehsmkey",
    ImageType.IMAGE_SOC_SOCK:"soc-sockey",
    ImageType.IMAGE_SOC_HSMK:"soc-ehsmkey",
    ImageType.IMAGE_PATCH_HSMK:"patch-ehsmkey",
    ImageType.IMAGE_FORCE_CERT:"force-load-cert",
    ImageType.IMAGE_RD_CERT:"rd-cert",
}

# image_name_section
ImageLevelToNameStr = {
    ImageLevel.IMAGE_BL_BOOT:"bl_boot_image",
    ImageLevel.IMAGE_BL_UPGRADE:"bl_upgrade_image",
    ImageLevel.IMAGE_BL_PATCH:"bl_patch_image",
    ImageLevel.IMAGE_FW_BOOT:"fw_boot_image",
    ImageLevel.IMAGE_FW_UPGRADE:"fw_upgrade_image",
}

ImageTypeToNameStr = {
    ImageType.IMAGE_HSM_HSMK:"hsm_hsmk",
    ImageType.IMAGE_SOC_SOCK:"soc_sock",
    ImageType.IMAGE_SOC_HSMK:"soc_hsmk",
    ImageType.IMAGE_PATCH_HSMK:"patch",
    ImageType.IMAGE_FORCE_CERT:"force_cert",
    ImageType.IMAGE_RD_CERT:"rd_cert",
}

ImageEncAlgoToNameStr = {
    ImageEncAlgo.IMAGE_ENC_ALGO_SM4:"sm4",
    ImageEncAlgo.IMAGE_ENC_ALGO_AES128:"aes128",
    ImageEncAlgo.IMAGE_ENC_ALGO_AES192:"aes192",
    ImageEncAlgo.IMAGE_ENC_ALGO_AES256:"aes256",
    ImageEncAlgo.IMAGE_ENC_ALGO_NONE:"plain",
}

ImageSignAlgoToNameStr = {
    ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC:"sm4",
    ImageSignAlgo.IMAGE_SIGN_ALGO_SM2:"sm2",
    ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC:"aes128",
    ImageSignAlgo.IMAGE_SIGN_ALGO_AES256_CMAC:"aes256",
    ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048:"rsa2048",
    ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072:"rsa3072",
    ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256:"ecc256",
    ImageSignAlgo.IMAGE_SIGN_ALGO_ECC384:"ecc384",
    ImageSignAlgo.IMAGE_SIGN_ALGO_NONE:"sign_none",
}

ImageVersionToNameStr = {
    ImageVersion.IMAGE_VC0:"vc0",
    ImageVersion.IMAGE_VC1:"vc1",
    ImageVersion.IMAGE_VC2:"vc2",
    ImageVersion.IMAGE_VC3:"vc3",
}

IMAGE_NAME_SPLIT = "_"
IMAGE_NAKED_SUFFIX = "NAKED"

@api
@allure.step("加载原始镜像并转换为字节序列输出")
def load_binary_to_bytes(file_path) -> bytes:
    """
    加载bin文件并转换为bytes对象
    参数:
        file_path (str): bin文件的路径
    返回:
        bytes: 文件内容的bytes对象，如果出错则返回None
    """
    try:
        # 以二进制读取模式打开文件
        with open(file_path, 'rb') as file:
            # 读取文件内容，返回bytes对象
            file_bytes = file.read()
            return file_bytes
    except FileNotFoundError:
        print(f"错误: 找不到文件 {file_path}")
    except IOError as e:
        print(f"错误: 读取文件时发生IO错误 - {e}")
    except Exception as e:
        print(f"错误: 处理文件时发生意外错误 - {e}")
    return None

# boot image
@api
@allure.step("根据配置生成安全启动镜像")
def generate_boot_image(
    image_level: ImageLevel,
    image_type: ImageType,
    enc_algo: ImageEncAlgo,
    sign_algo: ImageSignAlgo,
    version_cnt: ImageVersion,
    extra_boot_data: str = None,
    naked_flag: bool = False,
    fw_image: str = "ehsm_fw.bin"
) -> bytes:
    """
    根据配置参数生成安全启动镜像

    Args:
        image_level: 镜像级别
        image_type: 镜像类型
        enc_algo: 加密算法
        sign_algo: 签名算法
        version_cnt: 版本计数器
        extra_boot_data: 额外启动数据文件路径
        naked_flag: 是否为裸镜像
        fw_image: 源固件镜像文件名，默认为 ehsm_fw.bin

    Returns:
        启动镜像的二进制数据
    """
    base_section = [
        ImageLevelToNameStr[image_level],
        ImageSignAlgoToNameStr[sign_algo],
        ImageTypeToNameStr[image_type],
        ImageVersionToNameStr[version_cnt]
    ]

    # naked_flag 判断优先于 enc_algo 判断
    if naked_flag:
        # 裸镜像命名：使用 base_section 拼接并以 NAKED 结尾
        image_name_section = [base_section[0], base_section[2], IMAGE_NAKED_SUFFIX]
    else:
        enc_element = ImageEncAlgoToNameStr[enc_algo]
        if enc_algo is None:
            image_name_section = base_section + [enc_element]
        else:
            image_name_section = [base_section[0]] + [enc_element] + base_section[1:]

    image_name = IMAGE_NAME_SPLIT.join(image_name_section) + ".bin"

    if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
        if version_cnt == ImageVersion.IMAGE_VC0:
            vercnt = f"{key.VER_COUNTER_OTP0_VC0_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC1:
            vercnt = f"{key.VER_COUNTER_OTP0_VC1_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC2:
            vercnt = f"{key.VER_COUNTER_OTP0_VC2_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC3:
            vercnt = f"{key.VER_COUNTER_OTP0_VC3_STR}"
        else:
            vercnt = f"{key.VER_COUNTER_OTP0_VC0_STR}"
    else:
        if version_cnt == ImageVersion.IMAGE_VC0:
            vercnt = f"{key.VER_COUNTER_OTP1_VC0_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC1:
            vercnt = f"{key.VER_COUNTER_OTP1_VC1_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC2:
            vercnt = f"{key.VER_COUNTER_OTP1_VC2_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC3:
            vercnt = f"{key.VER_COUNTER_OTP1_VC3_STR}"
        else:
            vercnt = f"{key.VER_COUNTER_OTP1_VC0_STR}"

    if enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_SM4:
        enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_SM4_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES128:
        enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES128_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES192:
        enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES192_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES256:
        enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES256_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_NONE:
        enc_key = None
    else:
        enc_key = None

    if sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_SM4_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_SM2:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_SM2_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_AES128_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_AES256_CMAC:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_AES256_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_RSA2048_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_RSA3072_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_ECC256_STR_WITHOUT_0X04}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_ECC384:
        sign_key = f"{key.EHSM_VERIFY_SIGN_KEY_ECC384_STR_WITHOUT_0X04}"
    else:
        sign_key = None

    # 构建公共参数
    boot_params = {
        "fw_image": fw_image,
        "image_type": ImageTypeToCmdStr[image_type],
        "output_image": image_name
    }

    # naked_flag 判断优先，裸镜像使用简化参数
    if naked_flag:
        boot_params["naked_flag"] = True  # 标记为裸镜像模式
    else:
        # 正常模式：添加完整参数
        boot_params.update({
            "version": vercnt,
            "enc_key": enc_key,
            "sign_key": sign_key,
            "iv": f"{key.IV_STR}"
        })

        # 添加启动扩展字段
        if extra_boot_data is not None:
            boot_params["extra_boot_data"] = extra_boot_data

    # 生成安全启动镜像
    status, boot_image = image.create_boot_image(boot_params)
    assert status == 0
    return boot_image

# 镜像格式常量定义
FW_SIGNATURE_OFFSET = 0
FW_PUBLIC_KEY_OFFSET = 256
FW_ENCRYPT_IV_OFFSET = 256 + 320
FW_VALID_FLAG_OFFSET = 256 + 320 + 16
FW_IMAGE_TYPE_OFFSET = 256 + 320 + 16 + 4
FW_PLAIN_FLAG_OFFSET = 256 + 320 + 16 + 4 + 1
FW_NAKED_FLAG_OFFSET = 256 + 320 + 16 + 4 + 1 + 1
FW_IMAGE_OWNR_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1
FW_RESERVED_D_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1 + 1
FW_IMAGE_SIZE_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1 + 1 + 4
FW_VERSION_CNT_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1 + 1 + 4 + 4
FW_PUB_KEY_EXT_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1 + 1 + 4 + 4 + 16
FW_ENCRYPT_IMG_OFFSET = 256 + 320 + 16 + 4 + 1 + 1 + 1 + 1 + 4 + 4 + 16 + 400
IMAGE_SEGEMENT_SIZE = FW_ENCRYPT_IMG_OFFSET

IMAGE_CODE_VALID_FLAG = 0x8E97645D

# 辅助函数
def _modify_image_field(image: bytearray, offset: int, size: int, value: bytes):
    """
    修改镜像中指定偏移位置的字段值

    Args:
        image: 镜像数据
        offset: 偏移位置
        size: 字段大小
        value: 要设置的值
    """
    if len(image) <= offset:
        return

    # 确保不会越界
    actual_size = min(size, len(value), len(image) - offset)
    image[offset:offset + actual_size] = value[:actual_size]

def _set_first_byte_to_ff(data: bytes) -> bytes:
    """将字节序列的第一个字节设为0xFF"""
    if len(data) == 0:
        return b'\xFF'

    result = bytearray(data)
    result[0] = 0xFF
    return bytes(result)

@api
@allure.step("生成异常镜像")
def generate_corrupted_boot_image(
    image_level: ImageLevel,
    image_type: ImageType,
    enc_algo: ImageEncAlgo,
    sign_algo: ImageSignAlgo,
    version_cnt: ImageVersion,
    corruption_type: ImageCorruptionType,
    corruption_value: bytes = None
) -> bytes:
    """
    基于正常镜像生成异常镜像

    Args:
        image_level: 镜像级别
        image_type: 镜像类型
        enc_algo: 加密算法
        sign_algo: 签名算法
        version_cnt: 版本计数器
        corruption_type: 异常类型
        corruption_value: 自定义异常值，如果为None则使用默认异常值

    Returns:
        异常镜像的二进制数据
    """
    # 首先生成正常镜像
    normal_image = generate_boot_image(image_level, image_type, enc_algo, sign_algo, version_cnt)

    # 创建可修改的镜像副本
    corrupted_image = bytearray(normal_image)

    # 根据异常类型修改对应字段
    if corruption_type == ImageCorruptionType.INVALID_VALID_FLAG:
        # 修改有效标志为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_flag = corruption_value
        else:
            original_flag = corrupted_image[FW_VALID_FLAG_OFFSET:FW_VALID_FLAG_OFFSET + 4]
            invalid_flag = _set_first_byte_to_ff(original_flag)
        _modify_image_field(corrupted_image, FW_VALID_FLAG_OFFSET, 4, invalid_flag)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_TYPE:
        # 修改镜像类型为无效值：0xFF
        invalid_type = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_IMAGE_TYPE_OFFSET, 1, invalid_type)

    elif corruption_type == ImageCorruptionType.INVALID_PLAIN_FLAG:
        # 修改明文标志为无效值：0xFF
        invalid_plain = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_PLAIN_FLAG_OFFSET, 1, invalid_plain)

    elif corruption_type == ImageCorruptionType.INVALID_NAKED_FLAG:
        # 修改裸机标志为无效值：0xFF
        invalid_naked = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_NAKED_FLAG_OFFSET, 1, invalid_naked)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_OWNER:
        # 修改镜像所有者为无效值：0xFF
        invalid_owner = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_IMAGE_OWNR_OFFSET, 1, invalid_owner)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_SIZE:
        # 修改镜像大小为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_size = corruption_value
        else:
            original_size = corrupted_image[FW_IMAGE_SIZE_OFFSET:FW_IMAGE_SIZE_OFFSET + 4]
            invalid_size = _set_first_byte_to_ff(original_size)
        _modify_image_field(corrupted_image, FW_IMAGE_SIZE_OFFSET, 4, invalid_size)

    elif corruption_type == ImageCorruptionType.INVALID_VERSION_CNT:
        # 修改版本计数器为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_version = corruption_value
        else:
            original_version = corrupted_image[FW_VERSION_CNT_OFFSET:FW_VERSION_CNT_OFFSET + 16]
            invalid_version = _set_first_byte_to_ff(original_version)
        _modify_image_field(corrupted_image, FW_VERSION_CNT_OFFSET, 16, invalid_version)

    elif corruption_type == ImageCorruptionType.CORRUPTED_SIGNATURE:
        # 破坏签名数据：第一个字节设为0xFF
        if corruption_value:
            corrupted_sig = corruption_value
        else:
            original_sig = corrupted_image[FW_SIGNATURE_OFFSET:FW_SIGNATURE_OFFSET + 256]
            corrupted_sig = _set_first_byte_to_ff(original_sig)
        _modify_image_field(corrupted_image, FW_SIGNATURE_OFFSET, 256, corrupted_sig)

    elif corruption_type == ImageCorruptionType.CORRUPTED_PUBLIC_KEY:
        # 破坏公钥数据：第一个字节设为0xFF
        if corruption_value:
            corrupted_pubkey = corruption_value
        else:
            original_pubkey = corrupted_image[FW_PUBLIC_KEY_OFFSET:FW_PUBLIC_KEY_OFFSET + 320]
            corrupted_pubkey = _set_first_byte_to_ff(original_pubkey)
        _modify_image_field(corrupted_image, FW_PUBLIC_KEY_OFFSET, 320, corrupted_pubkey)

    elif corruption_type == ImageCorruptionType.CORRUPTED_ENCRYPT_IV:
        # 破坏加密IV：第一个字节设为0xFF
        if corruption_value:
            corrupted_iv = corruption_value
        else:
            original_iv = corrupted_image[FW_ENCRYPT_IV_OFFSET:FW_ENCRYPT_IV_OFFSET + 16]
            corrupted_iv = _set_first_byte_to_ff(original_iv)
        _modify_image_field(corrupted_image, FW_ENCRYPT_IV_OFFSET, 16, corrupted_iv)

    elif corruption_type == ImageCorruptionType.CORRUPTED_PUB_KEY_EXT:
        # 破坏扩展公钥：第一个字节设为0xFF
        if corruption_value:
            corrupted_ext = corruption_value
        else:
            original_ext = corrupted_image[FW_PUB_KEY_EXT_OFFSET:FW_PUB_KEY_EXT_OFFSET + 400]
            corrupted_ext = _set_first_byte_to_ff(original_ext)
        _modify_image_field(corrupted_image, FW_PUB_KEY_EXT_OFFSET, 400, corrupted_ext)

    elif corruption_type == ImageCorruptionType.CORRUPTED_ENCRYPT_IMG:
        # 破坏加密镜像数据：第一个字节设为0xFF
        if len(corrupted_image) > FW_ENCRYPT_IMG_OFFSET:
            if corruption_value:
                corrupted_data = corruption_value
            else:
                data_size = min(64, len(corrupted_image) - FW_ENCRYPT_IMG_OFFSET)
                original_data = corrupted_image[FW_ENCRYPT_IMG_OFFSET:FW_ENCRYPT_IMG_OFFSET + data_size]
                corrupted_data = _set_first_byte_to_ff(original_data)
            _modify_image_field(corrupted_image, FW_ENCRYPT_IMG_OFFSET, len(corrupted_data), corrupted_data)

    return bytes(corrupted_image)

# upgrade image
# 辅助函数
def _get_decrypt_key(vrf_sign_algo) -> bytes:
    """根据验证签名算法获取解密密钥"""
    try:
        # EHSM SM4相关算法
        ehsm_sm4_algos = {
            EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC,
            EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        }

        # SOC SM4相关算法
        soc_sm4_algos = {
            SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC,
            SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        }

        # EHSM算法使用EHSM密钥
        if vrf_sign_algo in ehsm_sm4_algos:
            # EHSM SM4算法使用EHSM SM4密钥
            key_bytes = key.EHSM_VERIFY_ENCRYPT_KEY_SM4
            trimmed_key = key_bytes.rstrip(b'\x00')  # 去掉末尾的零
            log.info(f"Using EHSM SM4 key: original_len={len(key_bytes)}, trimmed_len={len(trimmed_key)}")
            return trimmed_key
        elif isinstance(vrf_sign_algo, EhsmVerifyAlgo):
            # EHSM AES算法使用EHSM AES128密钥
            key_bytes = key.EHSM_VERIFY_ENCRYPT_KEY_AES128
            trimmed_key = key_bytes.rstrip(b'\x00')  # 去掉末尾的零
            log.info(f"Using EHSM AES128 key: original_len={len(key_bytes)}, trimmed_len={len(trimmed_key)}")
            return trimmed_key
        # SOC算法使用SOC密钥
        elif vrf_sign_algo in soc_sm4_algos:
            # SOC SM4算法使用SOC SM4密钥
            key_bytes = key.SOC_VERIFY_ENCRYPT_KEY_SM4
            trimmed_key = key_bytes.rstrip(b'\x00')  # 去掉末尾的零
            log.info(f"Using SOC SM4 key: original_len={len(key_bytes)}, trimmed_len={len(trimmed_key)}")
            return trimmed_key
        elif isinstance(vrf_sign_algo, SocVerifyAlgo):
            # SOC AES算法使用SOC AES128密钥
            key_bytes = key.SOC_VERIFY_ENCRYPT_KEY_AES128
            trimmed_key = key_bytes.rstrip(b'\x00')  # 去掉末尾的零
            log.info(f"Using SOC AES128 key: original_len={len(key_bytes)}, trimmed_len={len(trimmed_key)}")
            return trimmed_key
        else:
            log.error(f"Unknown vrf_sign_algo type: {type(vrf_sign_algo)}, value: {vrf_sign_algo}")
            return None

    except Exception as e:
        log.error(f"Error getting decrypt key: {e}")
        return None

def _decrypt_code(encrypted_data: bytes, iv: bytes, dec_key: bytes, vrf_sign_algo) -> bytes:
    """根据签名算法选择对应的解密方法"""
    try:
        log.info(f"Decrypt key length: {len(dec_key)}, key: {dec_key.hex()[:32]}...")
        log.info(f"IV: {iv.hex()}")
        log.info(f"Encrypted data size: {len(encrypted_data)}")

        # EHSM SM4相关算法
        ehsm_sm4_algos = {
            EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM4_CMAC,
            EhsmVerifyAlgo.EHSM_VERIFY_ALGO_SM2
        }

        # SOC SM4相关算法
        soc_sm4_algos = {
            SocVerifyAlgo.SOC_VERIFY_ALGO_SM4_CMAC,
            SocVerifyAlgo.SOC_VERIFY_ALGO_SM2
        }

        if vrf_sign_algo in ehsm_sm4_algos or vrf_sign_algo in soc_sm4_algos:
            # 使用SM4解密
            log.info("Using SM4 CBC decryption")
            sm4_cipher = sm4.CryptSM4()
            sm4_cipher.set_key(dec_key, sm4.SM4_DECRYPT)
            return sm4_cipher.crypt_cbc(encrypted_data, iv)
        else:
            # 使用AES解密 (NOPADDING模式，与SM4保持一致)
            log.info("Using AES CBC decryption (no padding)")
            cipher = AES.new(dec_key, AES.MODE_CBC, iv)
            return cipher.decrypt(encrypted_data)
    except Exception as e:
        log.error(f"Decryption error: {e}")
        return None

@api
@allure.step("验证升级镜像输出内容")
def check_verify_image_content(
    image_out: bytes,
    raw_image: bytes,
    vrf_sign_algo: 'Union[EhsmVerifyAlgo, SocVerifyAlgo]',
    boot_plain_flag: bool = False
) -> bool:
    """
    验证升级镜像输出内容是否与原始固件一致

    Args:
        image_out: 升级命令输出的镜像数据
        raw_image: 原始固件数据
        vrf_sign_algo: OTP中配置的验证签名算法
        boot_plain_flag: 是否为明文镜像，True表示明文，False表示加密

    Returns:
        bool: 验证成功返回True，失败返回False
    """
    try:
        if len(image_out) < IMAGE_SEGEMENT_SIZE:
            log.error(f"Image output size {len(image_out)} is smaller than segment size {IMAGE_SEGEMENT_SIZE}")
            return False

        # 提取IV和加密代码区
        v_iv = image_out[FW_ENCRYPT_IV_OFFSET:FW_ENCRYPT_IV_OFFSET + 16]
        v_code_base = image_out[IMAGE_SEGEMENT_SIZE:]
        code_size = len(image_out) - IMAGE_SEGEMENT_SIZE

        log.info(f"IV: {v_iv.hex()}")
        log.info(f"Code size: {code_size}")
        log.info(f"Raw image size: {len(raw_image)}")
        log.info(f"boot_plain_flag: {boot_plain_flag}")
        log.info(f"vrf_sign_algo: {vrf_sign_algo}")

        # 根据明文标志选择处理方法
        if boot_plain_flag:
            # 明文镜像，直接复制代码区
            plain_code = v_code_base
            log.info("Using plain text image (no decryption)")
        else:
            # 需要解密，根据签名算法选择密钥
            dec_key = _get_decrypt_key(vrf_sign_algo)
            if dec_key is None:
                log.error(f"Failed to get decrypt key for sign algorithm {vrf_sign_algo}")
                return False

            plain_code = _decrypt_code(v_code_base, v_iv, dec_key, vrf_sign_algo)
            if plain_code is None:
                log.error("Decryption failed")
                return False

        # 逐字节比较
        compare_size = min(len(raw_image), len(plain_code))
        for i in range(compare_size):
            if raw_image[i] != plain_code[i]:
                log.error(f"Mismatch at byte {i}: expected 0x{raw_image[i]:02x}, got 0x{plain_code[i]:02x}")
                return False

        if len(raw_image) != compare_size:
            log.warning(f"Size mismatch: raw_image={len(raw_image)}, plain_code={len(plain_code)}")

        log.info("Image content verification passed!")
        return True

    except Exception as e:
        log.error(f"Error during image verification: {e}")
        return False

@api
@allure.step("根据配置生成安全升级镜像")
def generate_upgrade_image(
    image_level: ImageLevel,
    image_type: ImageType,
    enc_algo: ImageEncAlgo,
    sign_algo: ImageSignAlgo,
    version_cnt: ImageVersion,
    boot_plain_flag: bool = False,
    extra_upgrade_data: str = None,
    naked_flag: bool = False,
    fw_image: str = "ehsm_fw.bin"
) -> bytes:
    """
    根据配置参数生成安全升级镜像

    Args:
        image_level: 镜像级别
        image_type: 镜像类型
        enc_algo: 加密算法
        sign_algo: 签名算法
        version_cnt: 版本计数器
        boot_plain_flag: 启动镜像明文标志，True表示明文启动
        extra_upgrade_data: 额外升级数据文件路径
        naked_flag: 是否为裸镜像
        fw_image: 源固件镜像文件名，默认为 ehsm_fw.bin

    Returns:
        升级镜像的二进制数据
    """
    base_section = [
        ImageLevelToNameStr[image_level],
        ImageSignAlgoToNameStr[sign_algo],
        ImageTypeToNameStr[image_type],
        ImageVersionToNameStr[version_cnt]
    ]

    # naked_flag 判断优先于 enc_algo 判断
    if naked_flag:
        # 裸镜像命名：使用 base_section 拼接并以 NAKED 结尾
        image_name_section = [base_section[0], base_section[2], IMAGE_NAKED_SUFFIX]
    else:
        enc_element = ImageEncAlgoToNameStr[enc_algo]
        if enc_algo is None:
            image_name_section = base_section + [enc_element]
        else:
            image_name_section = [base_section[0]] + [enc_element] + base_section[1:]

    image_name = IMAGE_NAME_SPLIT.join(image_name_section) + "_upgrade.bin"

    # 版本计数器配置（与启动镜像相同）
    if cfg_data.TEST_OTP_DEFAULT_VALUE == 0:
        if version_cnt == ImageVersion.IMAGE_VC0:
            vercnt = f"{key.VER_COUNTER_OTP0_VC0_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC1:
            vercnt = f"{key.VER_COUNTER_OTP0_VC1_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC2:
            vercnt = f"{key.VER_COUNTER_OTP0_VC2_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC3:
            vercnt = f"{key.VER_COUNTER_OTP0_VC3_STR}"
        else:
            vercnt = f"{key.VER_COUNTER_OTP0_VC0_STR}"
    else:
        if version_cnt == ImageVersion.IMAGE_VC0:
            vercnt = f"{key.VER_COUNTER_OTP1_VC0_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC1:
            vercnt = f"{key.VER_COUNTER_OTP1_VC1_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC2:
            vercnt = f"{key.VER_COUNTER_OTP1_VC2_STR}"
        elif version_cnt == ImageVersion.IMAGE_VC3:
            vercnt = f"{key.VER_COUNTER_OTP1_VC3_STR}"
        else:
            vercnt = f"{key.VER_COUNTER_OTP1_VC0_STR}"

    # 加密密钥配置
    # Reason: 当 EHSM_ENCRYPT_KEY_ID == EHSM_UPGRADE_ENCRYPT_KEY_ID 时，两者共用同一个 OTP key slot，
    # 升级镜像必须使用与 VERIFY 相同的密钥，否则设备无法正确解密升级镜像
    if cfg_data.TEST_EHSM_ENCRYPT_KEY_ID == cfg_data.TEST_EHSM_UPGRADE_ENCRYPT_KEY_ID:
        if enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_SM4:
            enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_SM4_STR}"
        elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES128:
            enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES128_STR}"
        elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES192:
            enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES192_STR}"
        elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES256:
            enc_key = f"{key.EHSM_VERIFY_ENCRYPT_KEY_AES256_STR}"
        elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_NONE:
            enc_key = None
        else:
            enc_key = None
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_SM4:
        enc_key = f"{key.EHSM_UPGRADE_ENCRYPT_KEY_SM4_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES128:
        enc_key = f"{key.EHSM_UPGRADE_ENCRYPT_KEY_AES128_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES192:
        enc_key = f"{key.EHSM_UPGRADE_ENCRYPT_KEY_AES192_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_AES256:
        enc_key = f"{key.EHSM_UPGRADE_ENCRYPT_KEY_AES256_STR}"
    elif enc_algo == ImageEncAlgo.IMAGE_ENC_ALGO_NONE:
        enc_key = None
    else:
        enc_key = None

    # 升级签名密钥配置（与启动镜像签名密钥相同）
    if sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_SM4_CMAC:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_SM4_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_SM2:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_SM2_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_AES128_CMAC:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_AES128_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_AES256_CMAC:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_AES256_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_RSA2048:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_RSA2048_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_RSA3072:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_RSA3072_STR}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_ECC256:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_ECC256_STR_WITHOUT_0X04}"
    elif sign_algo == ImageSignAlgo.IMAGE_SIGN_ALGO_ECC384:
        upg_sign_key = f"{key.EHSM_UPGRADE_SIGN_KEY_ECC384_STR_WITHOUT_0X04}"
    else:
        upg_sign_key = None

    # 验证签名密钥配置（与升级签名密钥相同）
    vrf_sign_key = upg_sign_key

    # 构建公共参数
    upgrade_params = {
        "fw_image": fw_image,
        "image_type": ImageTypeToCmdStr[image_type],
        "output_image": image_name,
        "version": vercnt,
        "enc_key": enc_key,
        "vrf_sign_key": vrf_sign_key,
        "iv": f"{key.IV_STR}"
    }

    # naked_flag 判断优先，裸镜像使用简化参数
    if naked_flag:
        upgrade_params["naked_flag"] = True  # 标记为裸镜像模式
    else:
        # 正常模式：添加完整参数
        upgrade_params["upg_sign_key"] = upg_sign_key

        # 添加明文启动标志
        if boot_plain_flag:
            upgrade_params["boot_plain_flag"] = "plain"

        # 添加升级扩展字段
        if extra_upgrade_data is not None:
            upgrade_params["extra_upgrade_data"] = extra_upgrade_data

    # 生成安全升级镜像
    status, upgrade_image = image.create_upgrade_image(upgrade_params)
    assert status == 0
    return upgrade_image

@api
@allure.step("生成异常升级镜像")
def generate_corrupted_upgrade_image(
    image_level: ImageLevel,
    image_type: ImageType,
    enc_algo: ImageEncAlgo,
    sign_algo: ImageSignAlgo,
    version_cnt: ImageVersion,
    corruption_type: ImageCorruptionType,
    corruption_value: bytes = None,
    boot_plain_flag: bool = False
) -> bytes:
    """
    基于正常升级镜像生成异常升级镜像
    升级镜像使用与启动镜像相同的格式和偏移量

    Args:
        image_level: 镜像级别
        image_type: 镜像类型
        enc_algo: 加密算法
        sign_algo: 签名算法
        version_cnt: 版本计数器
        corruption_type: 异常类型
        corruption_value: 自定义异常值，如果为None则使用默认异常值
        boot_plain_flag: 启动镜像明文标志，True表示明文启动

    Returns:
        异常升级镜像的二进制数据
    """
    # 首先生成正常升级镜像
    normal_upgrade_image = generate_upgrade_image(image_level, image_type, enc_algo, sign_algo, version_cnt, boot_plain_flag)

    # 创建可修改的镜像副本
    corrupted_image = bytearray(normal_upgrade_image)

    # 根据异常类型修改对应字段（使用与启动镜像相同的偏移量）
    if corruption_type == ImageCorruptionType.INVALID_VALID_FLAG:
        # 修改有效标志为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_flag = corruption_value
        else:
            original_flag = corrupted_image[FW_VALID_FLAG_OFFSET:FW_VALID_FLAG_OFFSET + 4]
            invalid_flag = _set_first_byte_to_ff(original_flag)
        _modify_image_field(corrupted_image, FW_VALID_FLAG_OFFSET, 4, invalid_flag)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_TYPE:
        # 修改镜像类型为无效值：0xFF
        invalid_type = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_IMAGE_TYPE_OFFSET, 1, invalid_type)

    elif corruption_type == ImageCorruptionType.INVALID_PLAIN_FLAG:
        # 修改明文标志为无效值：0xFF
        invalid_plain = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_PLAIN_FLAG_OFFSET, 1, invalid_plain)

    elif corruption_type == ImageCorruptionType.INVALID_NAKED_FLAG:
        # 修改裸机标志为无效值：0xFF
        invalid_naked = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_NAKED_FLAG_OFFSET, 1, invalid_naked)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_OWNER:
        # 修改镜像所有者为无效值：0xFF
        invalid_owner = corruption_value if corruption_value else b'\xFF'
        _modify_image_field(corrupted_image, FW_IMAGE_OWNR_OFFSET, 1, invalid_owner)

    elif corruption_type == ImageCorruptionType.INVALID_IMAGE_SIZE:
        # 修改镜像大小为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_size = corruption_value
        else:
            original_size = corrupted_image[FW_IMAGE_SIZE_OFFSET:FW_IMAGE_SIZE_OFFSET + 4]
            invalid_size = _set_first_byte_to_ff(original_size)
        _modify_image_field(corrupted_image, FW_IMAGE_SIZE_OFFSET, 4, invalid_size)

    elif corruption_type == ImageCorruptionType.INVALID_VERSION_CNT:
        # 修改版本计数器为无效值：第一个字节设为0xFF
        if corruption_value:
            invalid_version = corruption_value
        else:
            original_version = corrupted_image[FW_VERSION_CNT_OFFSET:FW_VERSION_CNT_OFFSET + 16]
            invalid_version = _set_first_byte_to_ff(original_version)
        _modify_image_field(corrupted_image, FW_VERSION_CNT_OFFSET, 16, invalid_version)

    elif corruption_type == ImageCorruptionType.CORRUPTED_SIGNATURE:
        # 破坏签名数据：第一个字节设为0xFF
        if corruption_value:
            corrupted_sig = corruption_value
        else:
            original_sig = corrupted_image[FW_SIGNATURE_OFFSET:FW_SIGNATURE_OFFSET + 256]
            corrupted_sig = _set_first_byte_to_ff(original_sig)
        _modify_image_field(corrupted_image, FW_SIGNATURE_OFFSET, 256, corrupted_sig)

    elif corruption_type == ImageCorruptionType.CORRUPTED_PUBLIC_KEY:
        # 破坏公钥数据：第一个字节设为0xFF
        if corruption_value:
            corrupted_pubkey = corruption_value
        else:
            original_pubkey = corrupted_image[FW_PUBLIC_KEY_OFFSET:FW_PUBLIC_KEY_OFFSET + 320]
            corrupted_pubkey = _set_first_byte_to_ff(original_pubkey)
        _modify_image_field(corrupted_image, FW_PUBLIC_KEY_OFFSET, 320, corrupted_pubkey)

    elif corruption_type == ImageCorruptionType.CORRUPTED_ENCRYPT_IV:
        # 破坏加密IV：第一个字节设为0xFF
        if corruption_value:
            corrupted_iv = corruption_value
        else:
            original_iv = corrupted_image[FW_ENCRYPT_IV_OFFSET:FW_ENCRYPT_IV_OFFSET + 16]
            corrupted_iv = _set_first_byte_to_ff(original_iv)
        _modify_image_field(corrupted_image, FW_ENCRYPT_IV_OFFSET, 16, corrupted_iv)

    elif corruption_type == ImageCorruptionType.CORRUPTED_PUB_KEY_EXT:
        # 破坏扩展公钥：第一个字节设为0xFF
        if corruption_value:
            corrupted_ext = corruption_value
        else:
            original_ext = corrupted_image[FW_PUB_KEY_EXT_OFFSET:FW_PUB_KEY_EXT_OFFSET + 400]
            corrupted_ext = _set_first_byte_to_ff(original_ext)
        _modify_image_field(corrupted_image, FW_PUB_KEY_EXT_OFFSET, 400, corrupted_ext)

    elif corruption_type == ImageCorruptionType.CORRUPTED_ENCRYPT_IMG:
        # 破坏加密镜像数据：第一个字节设为0xFF
        if len(corrupted_image) > FW_ENCRYPT_IMG_OFFSET:
            if corruption_value:
                corrupted_data = corruption_value
            else:
                data_size = min(64, len(corrupted_image) - FW_ENCRYPT_IMG_OFFSET)
                original_data = corrupted_image[FW_ENCRYPT_IMG_OFFSET:FW_ENCRYPT_IMG_OFFSET + data_size]
                corrupted_data = _set_first_byte_to_ff(original_data)
            _modify_image_field(corrupted_image, FW_ENCRYPT_IMG_OFFSET, len(corrupted_data), corrupted_data)

    return bytes(corrupted_image)
