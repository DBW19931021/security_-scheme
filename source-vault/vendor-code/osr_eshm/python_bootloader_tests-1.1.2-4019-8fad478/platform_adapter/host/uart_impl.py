import logging as log
import struct
from platform_adapter.api.constants import EhsmAuthAlgo, EhsmChallengeType
from utils.util import api
from .base import HostInterface
import allure
from typing import Tuple
from platform_adapter.uart_lib import soccmd, cmddef, hostapi
import time


class UartHost(HostInterface):
    @api
    @allure.step("使用字节 {byte_val} 填充 Memory {soc_addr} 地址 {size} 大小区域")
    def share_memset(self, soc_addr: int, byte_val: int, size: int) -> int:
        assert soc_addr >= 0
        assert 0 <= byte_val <= 255
        assert size > 0
        soccmd.send_cmd_recv_rsp(cmddef.CMD_SHARE_MEMSET,
                                struct.pack("<LLL", soc_addr, byte_val, size))
        return 0

    @api
    @allure.step("复制 Memory 从 {src_addr} 到 {dst_addr} 地址 {size} 大小数据")
    def share_memcpy(self, src_addr: int, dst_addr: int, size: int) -> int:
        assert src_addr >= 0
        assert dst_addr >= 0
        assert size > 0
        soccmd.send_cmd_recv_rsp(cmddef.CMD_SHARE_MEMCPY,
                                struct.pack("<LLL", src_addr, dst_addr, size))
        return 0

    @api
    @allure.step("读取 Memory {soc_addr} 地址 {size} 大小数据")
    def read_memory(self, soc_addr: int, size: int) -> Tuple[int, bytes]:
        assert soc_addr >= 0
        assert 0 < size < 65536

        _, data = soccmd.send_cmd_recv_rsp(cmddef.CMD_READ_MEMORY,
                                        struct.pack("<LH", soc_addr, size))
        # log.debug("read data: %s", data.hex())
        assert len(data) == size
        return 0, data


    @api
    @allure.step("写入 Memory {soc_addr} 地址  数据： {data}")
    def write_memory(self, soc_addr: int, data: bytes) -> int:
        assert soc_addr >= 0
        soccmd.send_cmd_recv_rsp(cmddef.CMD_WRITE_MEMORY,
                                struct.pack("<L", soc_addr) + data)
        return 0

    @allure.step("获取 Memory {soc_addr} 地址一个 word 数据")
    def get_word(self, soc_addr: int) -> int:
        assert soc_addr >= 0
        _, data = soccmd.send_cmd_recv_rsp(cmddef.CMD_GET_WORD,
                                        struct.pack("<L", soc_addr))
        assert len(data) == 4
        return int.from_bytes(data, "little")


    @allure.step("设置 Memory {soc_addr} 地址一个 word 数据: {val}")
    def set_word(self, soc_addr: int, val: int) -> int:
        assert soc_addr >= 0
        assert 0 <= val <= 0xFFFFFFFF
        soccmd.send_cmd_recv_rsp(cmddef.CMD_SET_WORD,
                                struct.pack("<LL", soc_addr, val))

    @api
    @allure.step("设置 Memory {write_addr} 值为 {write_val} 并等待 {wait_addr} 满足掩码 {wait_mask}")
    def set_word_and_wait(self,
                         write_addr: int,
                         write_val: int,
                         wait_addr: int,
                         wait_mask: int,
                         timeout_count: int = 0xFFFFFFFF) -> int:
        """
        设置指定地址的值，然后等待另一个地址的值满足掩码条件

        Args:
            write_addr: 要写入的地址
            write_val: 要写入的值
            wait_addr: 要等待的地址
            wait_mask: 等待掩码
            timeout_count: 超时计数，默认为 0xFFFF

        Returns:
            int: 等待时间

        Raises:
            Exception: 如果等待超时
        """
        assert write_addr >= 0
        assert 0 <= write_val <= 0xFFFFFFFF
        assert wait_addr >= 0
        assert 0 <= wait_mask <= 0xFFFFFFFF
        assert timeout_count >= 0

        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_SET_WORD_AND_WAIT,
            struct.pack("<LLLLL", write_addr, write_val, wait_addr, wait_mask,
                       timeout_count))
        if data[0] == 0:
            raise Exception("CMD_SET_WORD_AND_WAIT timeout")
        return t


    @api
    @allure.step("写 OTP 数据: {otp_bin}")
    def write_otp(self, otp_bin: bytes) -> int:
        self.write_memory(hostapi.OTP_BASE, otp_bin)
        return 0

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
            word = self.get_word(hostapi.HSM_STATUS_IN)
            if word & hostapi.HW_ERR:
                raise Exception("HW_ERR is set")
            if word & hostapi.HW_DONE:
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
            word = self.get_word(hostapi.HSM_STATUS_IN)
            if word & hostapi.BL_ERR:
                self.get_ehsm_status_information()
                raise Exception("BOOTLOADER_ERR is set")
            if word & hostapi.BL_DONE:
                time.sleep(0.1)
                return 0
            time.sleep(0.01)
            if time.time() - start >= timeout:
                self.get_ehsm_status_information()
                raise Exception("wait_bl_done timeout")
        return 0


    @api
    @allure.step("等待 {timeout} ms, 检查 Firmware 是否启动成功")
    def wait_fw_done(self, timeout: float = 1) -> int:
        start = time.time()
        while 1:
            word = self.get_word(hostapi.HSM_STATUS_IN)
            if word & hostapi.FW_ERR:
                self.get_ehsm_status_information()
                raise Exception("FIRMWARE_ERR is set")
            if word & hostapi.FW_DONE:
                return 0
            time.sleep(0.01)
            if time.time() - start >= timeout:
                self.get_ehsm_status_information()
                raise Exception("wait_fw_done timeout")
        return 0

    @api
    @allure.step("等待 {timeout} ms, 检查 Patch 是否加载成功")
    def bl_patch_success(self, timeout: float = 1) -> int:
        """
        等待并检查 Patch 加载是否成功

        Args:
            timeout: 超时时间(秒)

        Returns:
            0: Patch加载成功

        Raises:
            Exception: Patch加载失败或超时
        """
        # 延时让bootloader有时间加载patch
        time.sleep(0.05)

        # 读取 HSM_ERR_FW1 寄存器，检查 patch_load_failed (bit 10)
        word = self.get_word(hostapi.HSM_ERR_FW1)
        if word & hostapi.PATCH_LOAD_FAILED:
            self.get_ehsm_status_information()
            raise Exception("PATCH_LOAD_FAILED is set")

        # Patch加载成功（patch_load_failed标志未置位）
        return 0

    @api
    def sign(self, challenge:bytes, challenge_type: EhsmChallengeType, alg: EhsmAuthAlgo)->Tuple[int, bytes]:
        return super().sign(challenge, challenge_type, alg)


    @api
    @allure.step("计算CRC32")
    def crc32_mpeg2(self, data: bytes, crc: int = 0xFFFFFFFF) -> int:
        return super().crc32_mpeg2(data, crc)

    @allure.step("获取调试鉴权状态 {challenge_type} : 0-未认证，1-已认证")
    def check_debug_auth_status(self, challenge_type: EhsmChallengeType) -> bool:
        time.sleep(0.5)
        word = self.get_word(hostapi.HSM_STATUS_IN)
        if challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_EHSM_DEBUG:
            if word & hostapi.HSM_DBG_EN:
                return True
            else:
                return False
        elif challenge_type == EhsmChallengeType.EHSM_CHALLENGE_TYPE_SOC_DEBUG:
            if word & hostapi.SOC_DBG_EN:
                return True
            else:
                return False
        else:
            return False

    @allure.step("获取看门狗超时错误状态码：False-未超时，True-超时错误码")
    def check_watchdog_error_status(self) -> bool:
        word = self.get_word(hostapi.HSM_ERR_FW1)
        if word & hostapi.WDG_TIMEOUT:
            return True
        else:
            return False
    @allure.step("获取CPU WFI状态：False-未触发，True-触发")
    def check_cpu_wfi_status(self) -> bool:
        word = self.get_word(hostapi.HSM_STATUS_IN)
        log.debug(f"cpu wfi: %x", word)
        if word & hostapi.CPU_WFI:
            return True
        else:
            return False

    @api
    @allure.step("获取 eHSM 状态信息")
    def get_ehsm_status_information(self) -> None:
        # 读取所有状态寄存器
        hsm_status_in = self.get_word(hostapi.HSM_STATUS_IN)
        hsm_status_in1 = self.get_word(hostapi.HSM_STATUS_IN1)
        hsm_err_sensor_in = self.get_word(hostapi.HSM_ERR_SENSOR_IN)
        hsm_err_hw0 = self.get_word(hostapi.HSM_ERR_HW0)
        hsm_err_hw1 = self.get_word(hostapi.HSM_ERR_HW1)
        hsm_err_fw0 = self.get_word(hostapi.HSM_ERR_FW0)
        hsm_err_fw1 = self.get_word(hostapi.HSM_ERR_FW1)
        hsm_alarm_trig0 = self.get_word(hostapi.HSM_ALARM_TRIG0)
        hsm_alarm_trig1 = self.get_word(hostapi.HSM_ALARM_TRIG1)

        # 打印原始寄存器值 - 只显示非0的寄存器
        if hsm_status_in:
            log.info(f"HSM_STATUS_IN: 0x{hsm_status_in:08X} (0b{hsm_status_in:032b})")
        if hsm_status_in1:
            log.info(f"HSM_STATUS_IN1: 0x{hsm_status_in1:08X} (0b{hsm_status_in1:032b})")
        if hsm_err_sensor_in:
            log.info(f"HSM_ERR_SENSOR_IN: 0x{hsm_err_sensor_in:08X} (0b{hsm_err_sensor_in:032b})")
        if hsm_err_hw0:
            log.info(f"HSM_ERR_HW0: 0x{hsm_err_hw0:08X} (0b{hsm_err_hw0:032b})")
        if hsm_err_hw1:
            log.info(f"HSM_ERR_HW1: 0x{hsm_err_hw1:08X} (0b{hsm_err_hw1:032b})")
        if hsm_err_fw0:
            log.info(f"HSM_ERR_FW0: 0x{hsm_err_fw0:08X} (0b{hsm_err_fw0:032b})")
        if hsm_err_fw1:
            log.info(f"HSM_ERR_FW1: 0x{hsm_err_fw1:08X} (0b{hsm_err_fw1:032b})")
        if hsm_alarm_trig0:
            log.info(f"HSM_ALARM_TRIG0: 0x{hsm_alarm_trig0:08X} (0b{hsm_alarm_trig0:032b})")
        if hsm_alarm_trig1:
            log.info(f"HSM_ALARM_TRIG1: 0x{hsm_alarm_trig1:08X} (0b{hsm_alarm_trig1:032b})")

        # 解析 HSM_STATUS_IN 和 HSM_STATUS_IN1 (64位合并)
        combined_status = (hsm_status_in1 << 32) | hsm_status_in

        log.info("=== HSM 状态信息解析 ===")

        # 解析位0-31 (HSM_STATUS_IN) - 只显示非0的位
        if hsm_status_in & (1 << 0):
            log.info("hw_boot_done (bit 0): True")
        if hsm_status_in & (1 << 1):
            log.info("hw_boot_err (bit 1): True")
        if hsm_status_in & (1 << 2):
            log.info("bootloader_done (bit 2): True")
        if hsm_status_in & (1 << 3):
            log.info("bootloader_err (bit 3): True")
        if hsm_status_in & (1 << 4):
            log.info("firmware_done (bit 4): True")
        if hsm_status_in & (1 << 5):
            log.info("firmware_err (bit 5): True")
        if hsm_status_in & (1 << 6):
            log.info("soc_verify_done (bit 6): True")
        if hsm_status_in & (1 << 7):
            log.info("soc_verify_err (bit 7): True")
        if hsm_status_in & (1 << 8):
            log.info("hsm_lifecycle_test (bit 8): True")
        if hsm_status_in & (1 << 9):
            log.info("hsm_lifecycle_dev (bit 9): True")
        if hsm_status_in & (1 << 10):
            log.info("hsm_lifecycle_manu (bit 10): True")
        if hsm_status_in & (1 << 11):
            log.info("hsm_lifecycle_user (bit 11): True")
        if hsm_status_in & (1 << 12):
            log.info("hsm_lifecycle_debug (bit 12): True")
        if hsm_status_in & (1 << 13):
            log.info("hsm_lifecycle_destroy (bit 13): True")
        if hsm_status_in & (1 << 14):
            log.info("hsm_lifecycle_undef (bit 14): True")
        if hsm_status_in & (1 << 15):
            log.info("hsm_fw_sta0 (bit 15): True")
        if hsm_status_in & (1 << 16):
            log.info("hsm_dbg_en (bit 16): True")
        if hsm_status_in & (1 << 17):
            log.info("soc_dbg_en (bit 17): True")
        if hsm_status_in & (1 << 18):
            log.info("soc_reset (bit 18): True")
        if hsm_status_in & (1 << 19):
            log.info("soc_cpu_reset (bit 19): True")
        if hsm_status_in & (1 << 20):
            log.info("soc_cpu_release (bit 20): True")

        # 提取多位字段 - 只在非0时显示
        hsm_fw_sta1 = (hsm_status_in >> 21) & 0xF  # bits 24:21
        if hsm_fw_sta1:
            log.info(f"hsm_fw_sta1 (bits 24:21): 0x{hsm_fw_sta1:X}")

        if hsm_status_in & (1 << 25):
            log.info("cpu_wfi (bit 25): True")
        if hsm_status_in & (1 << 27):
            log.info("cpu_hart_halted (bit 27): True")

        # 解析位32-63 (HSM_STATUS_IN1) - 只在非0时显示
        hsm_fw_sta2 = hsm_status_in1 & 0xFFFF  # bits 47:32
        if hsm_fw_sta2:
            log.info(f"hsm_fw_sta2 (bits 47:32): 0x{hsm_fw_sta2:X}")

        hsm_fw_sta3 = (hsm_status_in1 >> 16) & 0x7FF  # bits 58:48
        if hsm_fw_sta3:
            log.info(f"hsm_fw_sta3 (bits 58:48): 0x{hsm_fw_sta3:X}")

        if hsm_status_in1 & (1 << (59-32)):
            log.info("dma_bus_busy (bit 59): True")
        if hsm_status_in1 & (1 << (60-32)):
            log.info("otp_bus_busy (bit 60): True")
        if hsm_status_in1 & (1 << (61-32)):
            log.info("cfg_bus_busy (bit 61): True")
        if hsm_status_in1 & (1 << (62-32)):
            log.info("nvm_bus_busy (bit 62): True")
        if hsm_status_in1 & (1 << (63-32)):
            log.info("soc_ram_bus_busy (bit 63): True")

        log.info("=== 其他寄存器信息 ===")
        if hsm_err_sensor_in:
            log.info(f"HSM_ERR_SENSOR_IN: 0x{hsm_err_sensor_in:08X}")

        # 解析 HSM_ERR_HW0 (位0-31)
        if hsm_err_hw0:
            log.info(f"HSM_ERR_HW0: 0x{hsm_err_hw0:08X}")
            if hsm_err_hw0 & (1 << 0):
                log.info("  mem_ecc_1b_irom (bit 0): True")
            if hsm_err_hw0 & (1 << 1):
                log.info("  mem_ecc_1b_iram (bit 1): True")
            if hsm_err_hw0 & (1 << 2):
                log.info("  mem_ecc_1b_dram (bit 2): True")
            if hsm_err_hw0 & (1 << 3):
                log.info("  mem_ecc_1b_kmu (bit 3): True")

            # 解析多位字段 mem_ecc_1b_pke (bits 7:4)
            mem_ecc_1b_pke = (hsm_err_hw0 >> 4) & 0xF
            if mem_ecc_1b_pke:
                log.info(f"  mem_ecc_1b_pke3/2/1/0 (bits 7:4): 0x{mem_ecc_1b_pke:X}")

            if hsm_err_hw0 & (1 << 11):
                log.info("  mem_ecc_1b_err_sum (bit 11): True")
            if hsm_err_hw0 & (1 << 12):
                log.info("  mem_ecc_mb_irom (bit 12): True")
            if hsm_err_hw0 & (1 << 13):
                log.info("  mem_ecc_mb_iram (bit 13): True")
            if hsm_err_hw0 & (1 << 14):
                log.info("  mem_ecc_mb_dram (bit 14): True")
            if hsm_err_hw0 & (1 << 15):
                log.info("  mem_ecc_mb_kmu (bit 15): True")

            # 解析多位字段 mem_ecc_mb_pke (bits 19:16)
            mem_ecc_mb_pke = (hsm_err_hw0 >> 16) & 0xF
            if mem_ecc_mb_pke:
                log.info(f"  mem_ecc_mb_pke3/2/1/0 (bits 19:16): 0x{mem_ecc_mb_pke:X}")

            if hsm_err_hw0 & (1 << 24):
                log.info("  soc_err_axi_dma_wr (bit 24): True")
            if hsm_err_hw0 & (1 << 25):
                log.info("  soc_err_axi_dma_rd (bit 25): True")
            if hsm_err_hw0 & (1 << 26):
                log.info("  soc_err_ahb_mem (bit 26): True")
            if hsm_err_hw0 & (1 << 27):
                log.info("  soc_err_ahb_otp (bit 27): True")
            if hsm_err_hw0 & (1 << 28):
                log.info("  soc_err_ahb_nvm (bit 28): True")
            if hsm_err_hw0 & (1 << 29):
                log.info("  soc_err_ahb_cfg (bit 29): True")

        # 解析 HSM_ERR_HW1 (位32-40，但存储在32位寄存器中，所以实际是位0-8)
        if hsm_err_hw1:
            log.info(f"HSM_ERR_HW1: 0x{hsm_err_hw1:08X}")
            if hsm_err_hw1 & (1 << 0):  # bit 32 in overall scheme
                log.info("  hw_trng_ht_fail (bit 32): True")
            if hsm_err_hw1 & (1 << 1):  # bit 33 in overall scheme
                log.info("  hw_trng_retry_fail (bit 33): True")
            if hsm_err_hw1 & (1 << 2):  # bit 34 in overall scheme
                log.info("  hw_trng_retry_warning (bit 34): True")
            if hsm_err_hw1 & (1 << 3):  # bit 35 in overall scheme
                log.info("  otp_key_crc_err (bit 35): True")
            if hsm_err_hw1 & (1 << 8):  # bit 40 in overall scheme
                log.info("  wdt_timeout (bit 40): True")

        # 解析 HSM_ERR_FW0 (位0-31)
        if hsm_err_fw0:
            log.info(f"HSM_ERR_FW0: 0x{hsm_err_fw0:08X}")
            if hsm_err_fw0 & (1 << 0):
                log.info("  secboot_selftest_fail (bit 0): 自检时有算法失败")
            if hsm_err_fw0 & (1 << 16):
                log.info("  selftest_hash_fail (bit 16): HASH 自检失败")
            if hsm_err_fw0 & (1 << 17):
                log.info("  selftest_ske_fail (bit 17): SKE 自检失败")
            if hsm_err_fw0 & (1 << 18):
                log.info("  selftest_pke_fail (bit 18): PKE 自检失败")
            if hsm_err_fw0 & (1 << 19):
                log.info("  selftest_trng_fail (bit 19): 随机数自检失败")
            if hsm_err_fw0 & (1 << 20):
                log.info("  selftest_hw_fail (bit 20): 自检出现硬件问题")

        # 解析 HSM_ERR_FW1 (位32-63，但存储在32位寄存器中，所以实际是位0-31)
        if hsm_err_fw1:
            log.info(f"HSM_ERR_FW1: 0x{hsm_err_fw1:08X}")
            if hsm_err_fw1 & (1 << 5):  # bit 37 in overall scheme
                log.info("  wdg_timeout (bit 37): Watchdog 超时，此位由软件设置")
            if hsm_err_fw1 & (1 << 8):  # bit 40 in overall scheme
                log.info("  bl_verify_failed (bit 40): BootLoader ROM code 校验失败，ROM 可能已损坏")
            if hsm_err_fw1 & (1 << 9):  # bit 41 in overall scheme
                log.info("  cpu_exception (bit 41): CPU 异常")
            if hsm_err_fw1 & (1 << 10):  # bit 42 in overall scheme
                log.info("  patch_load_failed (bit 42): 加载补丁失败")
            if hsm_err_fw1 & (1 << 31):  # bit 63 in overall scheme
                log.info("  hw_version_mismatch (bit 63): 软件与硬件的版本或客户编号不匹配")
        if hsm_alarm_trig0:
            log.info(f"HSM_ALARM_TRIG0: 0x{hsm_alarm_trig0:08X}")
        if hsm_alarm_trig1:
            log.info(f"HSM_ALARM_TRIG1: 0x{hsm_alarm_trig1:08X}")

    @allure.step("Echo 回环测试")
    def echo(self, data: bytes) -> bytes:
        """向 Server 发送 echo 命令并返回回环数据"""
        _, ret = soccmd.send_cmd_recv_rsp(cmddef.CMD_ECHO, data)
        return ret

    @allure.step("获取 Server 版本信息")
    def get_server_version(self) -> tuple:
        """获取 Server 版本号，返回 (major, minor, patch)"""
        _, data = soccmd.send_cmd_recv_rsp(cmddef.CMD_GET_SERVER_VERSION)
        assert len(data) >= 3
        return data[0], data[1], data[2]