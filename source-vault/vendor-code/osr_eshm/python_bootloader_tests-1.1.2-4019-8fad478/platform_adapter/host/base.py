from abc import ABC, abstractmethod
import allure
from platform_adapter.api.constants import EhsmAuthAlgo, EhsmChallengeType
from utils.config import cfg_data
from utils.util import api
from utils import key
import logging as log
from typing import Tuple
import time
from cryptosynth import (
    generate_ecc_sign_testdata,
    generate_symmetric_testdata,
    generate_sm2_sign_testdata,
    generate_rsa_sign_testdata,
)

# Global variable to track RSA key size
_current_rsa_key_size = 2048


class HostInterface(ABC):
    def set_rsa_key_size(self, key_size: int):
        """Set the RSA key size to use for signing operations"""

        global _current_rsa_key_size
        _current_rsa_key_size = key_size

    @abstractmethod
    @allure.step("使用字节 {byte_val} 填充 Memory {soc_addr} 地址 {size} 大小区域")
    def share_memset(self, soc_addr: int, byte_val: int, size: int) -> int:
        log.debug("share_memset")
        return 0

    @abstractmethod
    @allure.step("复制 Memory 从 {src_addr} 到 {dst_addr} 地址 {size} 大小数据")
    def share_memcpy(self, src_addr: int, dst_addr: int, size: int) -> int:
        log.debug("share_memcpy")
        return 0

    @abstractmethod
    def read_memory(self, soc_addr: int, size: int) -> Tuple[int, bytes]:
        log.debug("read_memory")
        return 0, "0011"

    @abstractmethod
    @allure.step("写入 Memory {soc_addr} 地址  数据： {data}")
    def write_memory(self, soc_addr: int, data: bytes) -> int:
        log.debug("write_memory")
        return 0

    @abstractmethod
    @allure.step("获取 Memory {soc_addr} 地址一个 word 数据")
    def get_word(self, soc_addr: int) -> int:
        return 0


    @abstractmethod
    @allure.step("设置 Memory {soc_addr} 地址一个 word 数据: {val}")
    def set_word(self, soc_addr: int, val: int) -> int:
        return 0

    @abstractmethod
    @allure.step("设置 Memory {write_addr} 值为 {write_val} 并等待 {wait_addr} 满足掩码 {wait_mask}")
    def set_word_and_wait(self,
                         write_addr: int,
                         write_val: int,
                         wait_addr: int,
                         wait_mask: int,
                         timeout_count: int = 0xFFFF) -> int:
        return 0

    @abstractmethod
    @allure.step("写 OTP 数据: {otp_bin}")
    def write_otp(self, otp_bin: bytes) -> int:
        return 0

    @abstractmethod
    @allure.step("复位 eHSM")
    def reset_ehsm(self) -> int:
        return 0

    @abstractmethod
    @allure.step("等待 {timeout} ms, 检查 hw 是否启动成功")
    def wait_hw_done(self, timeout: float = 1) -> int:
        start = time.time()
        return 0

    @abstractmethod
    @allure.step("等待 {timeout} ms, 检查 Bootloader 是否启动成功")
    def wait_bl_done(self, timeout: float = 1) -> int:
        start = time.time()
        return 0

    @abstractmethod
    @allure.step("等待 {timeout} ms, 检查 Firmware 是否启动成功")
    def wait_fw_done(self, timeout: float = 1) -> int:
        start = time.time()
        return 0

    @abstractmethod
    @allure.step("等待 {timeout} ms, 检查 Patch 是否加载成功")
    def bl_patch_success(self, timeout: float = 1) -> int:
        start = time.time()
        return 0

    @abstractmethod
    @allure.step("签名: 签名值：{challenge}, 挑战类型：{challenge_type}, 算法：{alg}")
    def sign(self, challenge:bytes, challenge_type: EhsmChallengeType, alg: EhsmAuthAlgo)->Tuple[int, bytes]:
        # aes - cmac / sm4 -cmac / rsa2048 / rsa 3072, ecc256,
        # ECC256
        if alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp256r1", key.EHSM_DEBUG_SIGN_KEY_ECC256_PRIKEY, key.EHSM_DEBUG_SIGN_KEY_ECC256_PUBKEY, "SHA256")
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp256r1", key.SOC_DEBUG_SIGN_KEY_ECC256_PRIKEY, key.SOC_DEBUG_SIGN_KEY_ECC256_PUBKEY, "SHA256")
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_ECDSA_P256R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp256r1", key.USER_AUTH_SIGN_KEY_ECC256_PRIKEY, key.USER_AUTH_SIGN_KEY_ECC256_PUBKEY, "SHA256")
            sign_data = data.signature

        # SM4
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            data = generate_symmetric_testdata("SM4", "CMAC", key.EHSM_DEBUG_SIGN_KEY_SM4_BYTES[0:16], "NONE", None, challenge)
            sign_data = data.ciphertext
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            data = generate_symmetric_testdata("SM4", "CMAC", key.SOC_DEBUG_SIGN_KEY_SM4_BYTES[0:16], "NONE", None, challenge)
            sign_data = data.ciphertext
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            data = generate_symmetric_testdata("SM4", "CMAC", key.USER_AUTH_SIGN_KEY_SM4_BYTES[0:16], "NONE", None, challenge)
            sign_data = data.ciphertext
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM4_CBC and (challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_FW_AUTH):
            iv = bytes([0] * 16)
            data = generate_symmetric_testdata("SM4", "CBC", key.EHSM_DEBUG_SIGN_KEY_SM4_BYTES[0:16], "NONE", iv, challenge[0:32])
            sign_data = data.ciphertext

        # AES

        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            if cfg_data.TEST_BL_DBG_AES256CMAC_SUPPORT:
                data = generate_symmetric_testdata("AES256", "CMAC", key.EHSM_DEBUG_SIGN_KEY_AES128, "NONE", None, challenge)
                sign_data = data.ciphertext
            else:
                data = generate_symmetric_testdata("AES128", "CMAC", key.EHSM_DEBUG_SIGN_KEY_AES128_BYTES[0:16], "NONE", None, challenge)
                sign_data = data.ciphertext
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            if cfg_data.TEST_BL_DBG_AES256CMAC_SUPPORT:
                data = generate_symmetric_testdata("AES256", "CMAC", key.SOC_DEBUG_SIGN_KEY_AES128, "NONE", None, challenge)
                sign_data = data.ciphertext
            else:
                data = generate_symmetric_testdata("AES128", "CMAC", key.SOC_DEBUG_SIGN_KEY_AES128_BYTES[0:16], "NONE", None, challenge)
                sign_data = data.ciphertext
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_AES128_CMAC and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            if cfg_data.TEST_BL_DBG_AES256CMAC_SUPPORT:
                data = generate_symmetric_testdata("AES256", "CMAC", key.USER_AUTH_SIGN_KEY_AES128, "NONE", None, challenge)
                sign_data = data.ciphertext
            else:
                data = generate_symmetric_testdata("AES128", "CMAC", key.USER_AUTH_SIGN_KEY_AES128_BYTES[0:16], "NONE", None, challenge)
                sign_data = data.ciphertext

        # SM2-SM3
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            data = generate_sm2_sign_testdata(challenge, key.EHSM_DEBUG_SIGN_KEY_SM2_PRIKEY, key.EHSM_DEBUG_SIGN_KEY_SM2_PUBKEY)
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            data = generate_sm2_sign_testdata(challenge, key.SOC_DEBUG_SIGN_KEY_SM2_PRIKEY, key.SOC_DEBUG_SIGN_KEY_SM2_PUBKEY)
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SM3_SM2 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            data = generate_sm2_sign_testdata(challenge, key.USER_AUTH_SIGN_KEY_SM2_PRIKEY, key.USER_AUTH_SIGN_KEY_SM2_PUBKEY)
            sign_data = data.signature

        # ECC384
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA384_ECDSA_P384R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp384r1", key.EHSM_DEBUG_SIGN_KEY_ECC384_PRIKEY, key.EHSM_DEBUG_SIGN_KEY_ECC384_PUBKEY, "SHA384")
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA384_ECDSA_P384R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp384r1", key.SOC_DEBUG_SIGN_KEY_ECC384_PRIKEY, key.SOC_DEBUG_SIGN_KEY_ECC384_PUBKEY, "SHA384")
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA384_ECDSA_P384R1 and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            data = generate_ecc_sign_testdata(challenge, "ECDSA", "secp384r1", key.USER_AUTH_SIGN_KEY_ECC384_PRIKEY, key.USER_AUTH_SIGN_KEY_ECC384_PUBKEY, "SHA384")
            sign_data = data.signature

        # RSA-SHA256 (支持 2048 和 3072)
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            if _current_rsa_key_size == 3072:
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.EHSM_VERIFY_SIGN_KEY_RSA3072_E,
                                                  n=key.EHSM_VERIFY_SIGN_KEY_RSA3072_N,
                                                  d=key.EHSM_VERIFY_SIGN_KEY_RSA3072_D,
                                                  key_size=3072)
            else:  # default to 2048
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.EHSM_VERIFY_SIGN_KEY_RSA2048_E,
                                                  n=key.EHSM_VERIFY_SIGN_KEY_RSA2048_N,
                                                  d=key.EHSM_VERIFY_SIGN_KEY_RSA2048_D,
                                                  key_size=2048)
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            if _current_rsa_key_size == 3072:
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.SOC_VERIFY_SIGN_KEY_RSA3072_E,
                                                  n=key.SOC_VERIFY_SIGN_KEY_RSA3072_N,
                                                  d=key.SOC_VERIFY_SIGN_KEY_RSA3072_D,
                                                  key_size=3072)
            else:  # default to 2048
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.SOC_VERIFY_SIGN_KEY_RSA2048_E,
                                                  n=key.SOC_VERIFY_SIGN_KEY_RSA2048_N,
                                                  d=key.SOC_VERIFY_SIGN_KEY_RSA2048_D,
                                                  key_size=2048)
            sign_data = data.signature
        elif alg == EhsmAuthAlgo.EHSM_AUTH_ALGO_SHA256_RSA and challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_USER_AUTH:
            if _current_rsa_key_size == 3072:
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.USER_AUTH_SIGN_KEY_RSA3072_E,
                                                  n=key.USER_AUTH_SIGN_KEY_RSA3072_N,
                                                  d=key.USER_AUTH_SIGN_KEY_RSA3072_D,
                                                  key_size=3072)
            else:  # default to 2048
                data = generate_rsa_sign_testdata(data=challenge, hash_alg="SHA256", mode="PSS",
                                                  e=key.USER_AUTH_SIGN_KEY_RSA2048_E,
                                                  n=key.USER_AUTH_SIGN_KEY_RSA2048_N,
                                                  d=key.USER_AUTH_SIGN_KEY_RSA2048_D,
                                                  key_size=2048)
            sign_data = data.signature

        else:
            log.debug("输入错误！")
            return 1, b" "
        return 0, sign_data

    @abstractmethod
    @allure.step("计算CRC32")
    def crc32_mpeg2(self, data: bytes, crc: int = 0xFFFFFFFF) -> int:
        """
        CRC32/MPEG-2 算法实现（多项式 0x04C11DB7，初始值可设为 0xFFFFFFFF）。

        :param data: 要计算 CRC 的数据（bytes 类型）
        :param crc: 初始 CRC 值（默认 0xFFFFFFFF）
        :return: 计算得到的 CRC32 值（int 类型）
        """
        for byte in data:
            crc ^= byte << 24
            for _ in range(8):
                if crc & 0x80000000:
                    crc = (crc << 1) ^ 0x04C11DB7
                else:
                    crc <<= 1
                crc &= 0xFFFFFFFF  # 保持 32 位
        return crc

    @abstractmethod
    @allure.step("获取调试鉴权状态 {challenge_type} : 0-未认证，1-已认证")
    def check_debug_auth_status(self, challenge_type: EhsmChallengeType) -> bool:
        return False

    @abstractmethod
    def check_watchdog_error_status(self) -> bool:
        return False

    @abstractmethod
    def check_cpu_wfi_status(self) -> bool:
        return False

    @abstractmethod
    def get_ehsm_status_information(self) -> None:
        return False

    @abstractmethod
    @allure.step("Echo 回环测试，发送数据")
    def echo(self, data: bytes) -> bytes:
        """向 Server 发送 echo 命令并返回回环数据"""
        return data

    @abstractmethod
    @allure.step("获取 Server 版本信息")
    def get_server_version(self) -> Tuple[int, int, int]:
        """获取 Server 版本号，返回 (major, minor, patch)"""
        return (0, 0, 0)