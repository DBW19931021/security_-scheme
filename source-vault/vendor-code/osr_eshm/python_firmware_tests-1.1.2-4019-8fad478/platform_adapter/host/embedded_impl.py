from .base import HostInterface
from abc import ABC, abstractmethod
import allure
from utils.util import api
import logging as log
from typing import Tuple
import time
OTP_BASE = 0x6004_0000

HW_DONE = 1 << 0
HW_ERR = 1 << 1
BL_DONE = 1 << 2
BL_ERR = 1 << 3
FW_DONE = 1 << 4
FW_ERR = 1 << 5
PATCH_LOAD_FAILED = 1 << 10

class EmbeddedHost(HostInterface):
    @api
    @allure.step("使用字节 {byte_val} 填充 Memory {soc_addr} 地址 {size} 大小区域")
    def share_memset(self, soc_addr: int, byte_val: int, size: int) -> int:
        assert soc_addr >= 0
        assert 0 <= byte_val <= 255
        assert size > 0
        log.debug("share_memset: addr=0x%x, byte=0x%x, size=%d", soc_addr, byte_val, size)
        return 0

    @api
    @allure.step("复制 Memory 从 {src_addr} 到 {dst_addr} 地址 {size} 大小数据")
    def share_memcpy(self, src_addr: int, dst_addr: int, size: int) -> int:
        assert src_addr >= 0
        assert dst_addr >= 0
        assert size > 0
        log.debug("share_memcpy: src=0x%x, dst=0x%x, size=%d", src_addr, dst_addr, size)
        return 0

    @api
    @allure.step("读取 Memory {soc_addr} 地址 {size} 大小数据")
    def read_memory(self, soc_addr: int, size: int) -> Tuple[int, bytes]:
        assert soc_addr >= 0
        assert size > 0

        CHUNK_SIZE = 65535  # 最大单次读取大小（保持与uart_impl一致）

        # Reason: 分块读取以支持大数据量，保持与uart_impl实现一致
        if size <= CHUNK_SIZE:
            ret, memory = 0, b"0b1234"  # 模拟实现
            log.debug("read data: %s", memory.hex())
            return ret, memory
        else:
            # 分块模拟读取
            result = bytearray()
            offset = 0
            while offset < size:
                chunk_size = min(CHUNK_SIZE, size - offset)
                ret, memory = 0, b"0b1234"  # 模拟实现
                result.extend(memory)
                offset += chunk_size
            return 0, bytes(result)


    @api
    @allure.step("写入 Memory {soc_addr} 地址  数据： {data}")
    def write_memory(self, soc_addr: int, data: bytes) -> int:
        ret, memory = 0, "0b1234"
        log.debug("read data: %s", data.hex())
        return ret

    @api
    @allure.step("获取 Memory {soc_addr} 地址一个 word 数据")
    def get_word(self, soc_addr: int) -> int:
        assert soc_addr >= 0
        ret = 0
        return ret


    @api
    @allure.step("设置 Memory {soc_addr} 地址一个 word 数据: {val}")
    def set_word(self, soc_addr: int, val: int) -> int:
        assert soc_addr >= 0
        ret = 0
        return ret


    @api
    @allure.step("写 OTP 数据: {otp_bin}")
    def write_otp(self, otp_bin: bytes) -> None:
        self.write_memory(OTP_BASE, otp_bin)


    @api
    @allure.step("复位 eHSM")
    def reset_ehsm(self) -> int:
        self.set_word(0x40010008, 0xFFFFFFFE)
        self.set_word(0x40010008, 0xFFFFFFFF)
        return 0


    @api
    @allure.step("等待 {timeout} ms, 检查 Hardware 是否启动成功")
    def wait_hw_done(self, timeout: float = 1) -> int:
        start = time.time()
        while 1:
            word = self.get_word(HSM_STATUS_IN)
            if word & HW_ERR:
                raise Exception("HW_ERR is set")
            if word & HW_DONE:
                time.sleep(0.1)
                return 0
            time.sleep(0.01)
            if time.time() - start >= timeout:
                raise Exception("wait_hw_done timeout")
        return 0


    @api
    @allure.step("等待 {timeout} ms, 检查 Bootloader 是否启动成功")
    def wait_bl_done(self, timeout: float = 1) -> int:
        start = time.time()
        while 1:
            word = self.get_word(HSM_STATUS_IN)
            if word & BL_ERR:
                raise Exception("BOOTLOADER_ERR is set")
            if word & BL_DONE:
                time.sleep(0.1)
                return
            time.sleep(0.01)
            if time.time() - start >= timeout:
                raise Exception("wait_bl_done timeout")
        return 0


    @api
    @allure.step("等待 {timeout} ms, 检查 Firmware 是否启动成功")
    def wait_fw_done(self, timeout: float = 1) -> None:
        start = time.time()
        while 1:
            word = self.get_word(HSM_STATUS_IN)
            if word & FW_ERR:
                raise Exception("FIRMWARE_ERR is set")
            if word & FW_DONE:
                return
            time.sleep(0.01)
            if time.time() - start >= timeout:
                raise Exception("wait_fw_done timeout")

    @api
    @allure.step("等待 {timeout} ms, 检查 Patch 是否加载成功")
    def bl_patch_success(self, timeout: float = 1) -> int:
        """
        等待并检查 Patch 加载是否成功（embedded 模拟实现）

        Args:
            timeout: 超时时间(秒)

        Returns:
            0: Patch加载成功

        Raises:
            Exception: Patch加载失败或超时
        """
        # 延时让bootloader有时间加载patch
        time.sleep(0.05)

        # 模拟实现：读取 HSM_ERR_FW1 寄存器，检查 patch_load_failed (bit 10)
        word = self.get_word(HSM_ERR_FW1)
        if word & PATCH_LOAD_FAILED:
            raise Exception("PATCH_LOAD_FAILED is set")

        # Patch加载成功（patch_load_failed标志未置位）
        return 0

    @api
    @allure.step("签名")
    def sign(self, challenge:bytes, challenge_type: int)->Tuple[int, bytes]:
        return 1, "1234"

    @api
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
