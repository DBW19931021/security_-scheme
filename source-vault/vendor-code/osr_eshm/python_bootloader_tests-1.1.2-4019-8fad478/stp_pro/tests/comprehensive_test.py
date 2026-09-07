#!/usr/bin/env python3
# -*- coding:utf-8 -*-
"""
STP协议综合测试程序
合并了底层协议测试和高层应用测试的完整测试套件

需要配合独立运行的 server.py 服务端进行测试

测试类别：
- 底层协议测试：RESYNC、错误处理、帧格式验证
- 高层应用测试：边界条件、压力测试、稳定性测试

使用方法:
1. 先启动服务端: python3 tests/server.py --port /dev/ttyACM0
2. 运行测试: python3 tests/comprehensive_test.py --port /dev/ttyUSB0
"""

import sys
import time
import argparse
import struct
import random
from pathlib import Path
from typing import List, Tuple, Optional

# sys.path.append(str(Path(__file__).parent.parent / "py"))

import logging as log
from stp_uart import (
    STPUart,
    STPTimeoutError,
    STPCrcError,
    STPResync,
    STPError,
    TYPE_DATA,
    TYPE_ACK,
    TYPE_SWITCH,
    TYPE_RESYNC_REQ,
    TYPE_RESYNC_ACK,
    TYPE_ERR,
    TYPE_MASK,
    ERR_CODE_CRC,
    ERR_CODE_CHAR_TIMEOUT,
    ERR_CODE_TYPE,
    ERR_CODE_MASK,
    FRAME_LEAD,
    calc_crc,
    OFF_TYPE,
    OFF_LEN,
    OFF_PAYLOAD,
)


class ComprehensiveTester:
    """综合测试器，包含底层协议测试和高层应用测试"""

    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.stp: Optional[STPUart] = None

    def setup_connection(self) -> bool:
        """建立客户端连接"""
        try:
            if self.stp:
                self.stp.close()
            self.stp = STPUart(is_client=True, port=self.port, baudrate=self.baudrate)
            log.info(f"客户端连接建立成功: {self.port} @ {self.baudrate}")
            return True
        except Exception as e:
            log.error(f"客户端连接失败：{e}")
            return False

    def cleanup_connection(self):
        """清理连接"""
        if self.stp:
            try:
                self.stp.close()
                log.debug("客户端连接已关闭")
            except:
                pass
            self.stp = None

    # ==================== 底层协议测试方法 ====================

    def send_corrupted_frame(
        self, frame_type: int, payload: bytes = None, corrupt_type: str = "crc"
    ):
        """发送损坏的帧"""
        if corrupt_type == "crc":
            # 发送CRC错误的帧
            head = bytearray(
                [FRAME_LEAD[0], frame_type, 0 if not payload else len(payload)]
            )
            self.stp.get_com().write(head)
            if payload:
                self.stp.get_com().write(payload)
            # 发送错误的CRC
            wrong_crc = b"\xff\xff"
            self.stp.get_com().write(wrong_crc)
            log.debug(
                f"发送CRC错误帧: {head.hex()}{payload.hex() if payload else ''}{wrong_crc.hex()}"
            )

        elif corrupt_type == "incomplete":
            # 发送不完整的帧
            head = bytearray(
                [FRAME_LEAD[0], frame_type, 10 if not payload else len(payload)]
            )
            self.stp.get_com().write(head)
            if payload:
                # 只发送部分payload
                partial_payload = (
                    payload[: len(payload) // 2] if len(payload) > 1 else b""
                )
                self.stp.get_com().write(partial_payload)
                log.debug(f"发送不完整帧: {head.hex()}{partial_payload.hex()}")
            else:
                log.debug(f"发送不完整帧头: {head.hex()}")

        elif corrupt_type == "invalid_type":
            # 发送无效类型的帧
            invalid_type = 0xFF
            head = bytearray(
                [FRAME_LEAD[0], invalid_type, 0 if not payload else len(payload)]
            )
            self.stp.get_com().write(head)
            if payload:
                self.stp.get_com().write(payload)
                crc = calc_crc(head + payload)
            else:
                crc = calc_crc(head)
            self.stp.get_com().write(crc)
            log.debug(
                f"发送无效类型帧: {head.hex()}{payload.hex() if payload else ''}{crc.hex()}"
            )

    def wait_and_recv_frame(self, timeout: float = 5.0) -> bytes:
        """等待并接收帧，带超时"""
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                frame = self.stp.recv_frame()
                return frame
            except (STPTimeoutError, STPError):
                continue
            except Exception as e:
                log.debug(f"接收帧异常: {e}")
                continue
        log.warning(f"接收帧超时({timeout}秒)")
        return b""

    def verify_error_frame(self, frame: bytes, expected_error_code: int) -> bool:
        """验证错误帧"""
        if len(frame) < 3:
            return False
        frame_type = frame[OFF_TYPE] & TYPE_MASK
        error_code = frame[OFF_TYPE] & ERR_CODE_MASK
        if frame_type == TYPE_ERR and error_code == expected_error_code:
            log.info(
                f"收到期望的错误帧: 类型={frame_type:02x}, 错误码={error_code:02x}"
            )
            return True
        else:
            log.error(
                f"错误帧不匹配: 期望错误码={expected_error_code:02x}, 实际类型={frame_type:02x}, 实际错误码={error_code:02x}"
            )
            return False

    def test_resync_basic(self) -> bool:
        """底层测试1: 基本RESYNC功能测试"""
        try:
            print("测试基本RESYNC功能...")
            log.info("发送RESYNC_REQ帧")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            # 等待RESYNC_ACK响应
            frame = self.wait_and_recv_frame(timeout=3.0)
            if not frame:
                print("✗ 未收到RESYNC_ACK响应")
                return False

            frame_type = frame[OFF_TYPE] & TYPE_MASK
            if frame_type == TYPE_RESYNC_ACK:
                print("  收到RESYNC_ACK响应")
                # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
                self.stp.reset_comm()
                # 测试重新发送数据
                test_data = b"Hello after resync"
                self.stp.send_frame(TYPE_DATA, test_data)
                ack_frame = self.wait_and_recv_frame(timeout=3.0)
                if ack_frame and (ack_frame[OFF_TYPE] & TYPE_MASK) == TYPE_ACK:
                    print("✓ 基本RESYNC功能测试通过")
                    return True
                else:
                    print("✗ RESYNC后数据发送失败")
                    return False
            else:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False
        except Exception as e:
            print(f"✗ 基本RESYNC测试异常：{e}")
            return False

    def test_crc_error(self) -> bool:
        """底层测试2: CRC错误测试"""
        try:
            print("测试CRC错误处理...")
            test_data = b"CRC error test data"
            self.send_corrupted_frame(TYPE_DATA, test_data, "crc")

            # 等待错误帧响应
            error_frame = self.wait_and_recv_frame(timeout=3.0)
            if not error_frame:
                print("✗ CRC错误未收到错误帧响应")
                return False

            if self.verify_error_frame(error_frame, ERR_CODE_CRC):
                print("  CRC错误正确处理")
                # 发送正确的帧验证恢复
                self.stp.send_frame(TYPE_DATA, b"Recovery test")
                ack_frame = self.wait_and_recv_frame(timeout=3.0)
                if ack_frame and (ack_frame[OFF_TYPE] & TYPE_MASK) == TYPE_ACK:
                    print("✓ CRC错误处理测试通过")
                    return True
                else:
                    print("✗ CRC错误后通信未恢复")
                    return False
            else:
                print("✗ CRC错误帧验证失败")
                return False
        except Exception as e:
            print(f"✗ CRC错误测试异常：{e}")
            return False

    def test_incomplete_frame(self) -> bool:
        """底层测试3: 不完整帧测试"""
        try:
            print("测试不完整帧处理...")
            test_data = b"Incomplete frame test"
            self.send_corrupted_frame(TYPE_DATA, test_data, "incomplete")

            # 等待字符超时错误帧
            error_frame = self.wait_and_recv_frame(timeout=5.0)
            if not error_frame:
                print("✗ 不完整帧未收到错误响应")
                return False

            if self.verify_error_frame(error_frame, ERR_CODE_CHAR_TIMEOUT):
                print("  不完整帧错误正确处理")
                # 发送正确帧验证恢复
                self.stp.send_frame(TYPE_DATA, b"Recovery after incomplete")
                ack_frame = self.wait_and_recv_frame(timeout=3.0)
                if ack_frame and (ack_frame[OFF_TYPE] & TYPE_MASK) == TYPE_ACK:
                    print("✓ 不完整帧处理测试通过")
                    return True
                else:
                    print("✗ 不完整帧错误后通信未恢复")
                    return False
            else:
                print("✗ 不完整帧错误验证失败")
                return False
        except Exception as e:
            print(f"✗ 不完整帧测试异常：{e}")
            return False

    def test_resync_with_data(self) -> bool:
        """底层测试4: RESYNC中断数据传输后恢复测试"""
        try:
            import struct

            print("测试数据传输中断后RESYNC恢复...")

            # 步骤1: 发送2字节大端长度（20）
            print("  发送数据长度20...")
            length_data = struct.pack(">H", 20)
            for byte in length_data:
                self.stp.get_com().write(bytes([byte]))

            # 步骤2: 发送10字节数据（一半数据）
            print("  发送10字节数据（中断传输）...")
            partial_data = b"1234567890"  # 10字节
            for byte in partial_data:
                self.stp.get_com().write(bytes([byte]))

            # 步骤3: 发送resync_req帧，模拟恢复通信
            print("  发送RESYNC_REQ恢复...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            # 步骤4: 接收resync_ack帧
            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ 未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，开始重新传输...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 步骤5: 重新发送2字节大端长度（20）
            self.stp.send(struct.pack(">H", 20))

            # 步骤6: 发送完整20字节数据
            test_data = b"12345678901234567890"  # 20字节
            self.stp.send(test_data)

            # 步骤7: 接收20字节回显数据
            echo_data = self.stp.recv(20)

            # 步骤8: 判断收发数据相同
            if echo_data == test_data:
                print("✓ RESYNC数据传输恢复测试通过")
                return True
            else:
                print(f"✗ 数据不匹配: 发送={test_data.hex()}, 接收={echo_data.hex()}")
                return False

        except Exception as e:
            print(f"✗ RESYNC数据传输恢复测试异常：{e}")
            return False

    def test_resync_during_data_transfer(self) -> bool:
        """底层测试5: 数据传输中断后RESYNC恢复测试"""
        try:
            import struct

            print("测试数据传输中客户端主动RESYNC...")

            # 开始正常数据传输
            print("  开始数据传输...")
            self.stp.send(struct.pack(">H", 30))
            partial_data = b"12345678901234567890123456789"  # 29字节，少发1字节
            self.stp.send(partial_data[:-5])  # 只发送前24字节

            print("  数据传输中断，发送RESYNC_REQ...")
            # 客户端主动发起resync
            self.stp.send_frame(TYPE_RESYNC_REQ)

            # 等待RESYNC_ACK
            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ 传输中断后未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，重新开始数据传输...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 重新发送新的数据
            new_test_data = b"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234"  # 30字节
            self.stp.send(struct.pack(">H", len(new_test_data)))
            self.stp.send(new_test_data)

            # 接收回显
            echo_data = self.stp.recv(len(new_test_data))

            if echo_data == new_test_data:
                print("✓ 数据传输中断RESYNC恢复测试通过")
                return True
            else:
                print(
                    f"✗ 数据不匹配: 发送={new_test_data.hex()}, 接收={echo_data.hex()}"
                )
                return False

        except Exception as e:
            print(f"✗ 数据传输中断RESYNC测试异常：{e}")
            return False

    def test_resync_after_crc_error(self) -> bool:
        """底层测试6: CRC错误后使用RESYNC恢复测试"""
        try:
            import struct

            print("测试CRC错误后RESYNC恢复...")

            # 发送CRC错误的帧
            print("  发送CRC错误帧...")
            test_data = b"Test CRC error recovery"
            self.send_corrupted_frame(TYPE_DATA, test_data, "crc")

            # 等待错误帧响应
            error_frame = self.wait_and_recv_frame(timeout=3.0)
            if error_frame:
                if self.verify_error_frame(error_frame, ERR_CODE_CRC):
                    print("  收到CRC错误响应")
                else:
                    print("  收到非CRC错误响应")
            else:
                print("  未收到错误响应（可能被服务端忽略）")

            # 使用RESYNC恢复通信
            print("  发送RESYNC_REQ恢复通信...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ CRC错误后未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，验证通信恢复...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 验证通信恢复
            test_data = b"Recovery test after CRC error"
            self.stp.send(struct.pack(">H", len(test_data)))
            self.stp.send(test_data)
            echo_data = self.stp.recv(len(test_data))

            if echo_data == test_data:
                print("✓ CRC错误后RESYNC恢复测试通过")
                return True
            else:
                print(
                    f"✗ CRC错误后通信未恢复: 发送={test_data.hex()}, 接收={echo_data.hex()}"
                )
                return False

        except Exception as e:
            print(f"✗ CRC错误后RESYNC测试异常：{e}")
            return False

    def test_resync_after_timeout(self) -> bool:
        """底层测试7: 超时错误后使用RESYNC恢复测试"""
        try:
            import struct

            print("测试超时错误后RESYNC恢复...")

            # 发送不完整帧造成超时
            print("  发送不完整帧造成超时...")
            test_data = b"Test timeout error recovery"
            self.send_corrupted_frame(TYPE_DATA, test_data, "incomplete")

            # 等待超时错误帧响应
            error_frame = self.wait_and_recv_frame(timeout=5.0)
            if error_frame:
                if self.verify_error_frame(error_frame, ERR_CODE_CHAR_TIMEOUT):
                    print("  收到超时错误响应")
                else:
                    print("  收到非超时错误响应")
            else:
                print("  未收到错误响应（可能超时处理中）")

            # 使用RESYNC恢复通信
            print("  发送RESYNC_REQ恢复通信...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ 超时错误后未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，验证通信恢复...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 验证通信恢复
            test_data = b"Recovery test after timeout"
            self.stp.send(struct.pack(">H", len(test_data)))
            self.stp.send(test_data)
            echo_data = self.stp.recv(len(test_data))

            if echo_data == test_data:
                print("✓ 超时错误后RESYNC恢复测试通过")
                return True
            else:
                print(
                    f"✗ 超时错误后通信未恢复: 发送={test_data.hex()}, 接收={echo_data.hex()}"
                )
                return False

        except Exception as e:
            print(f"✗ 超时错误后RESYNC测试异常：{e}")
            return False

    def test_resync_role_based_reset(self) -> bool:
        """底层测试8: RESYNC基于角色的固定模式重置测试"""
        try:
            import struct

            print("测试RESYNC基于角色的固定模式重置...")

            # 客户端发送RESYNC_REQ，验证重置为发送模式
            print("  客户端发送RESYNC_REQ...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ 未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，验证客户端重置为发送模式...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 验证客户端重置后仍在发送模式
            test_data1 = b"Test client send mode after resync"
            self.stp.send(struct.pack(">H", len(test_data1)))
            self.stp.send(test_data1)
            echo_data1 = self.stp.recv(len(test_data1))

            if echo_data1 != test_data1:
                print(
                    f"✗ 客户端发送模式验证失败: 发送={test_data1.hex()}, 接收={echo_data1.hex()}"
                )
                return False

            print("  客户端发送模式验证通过")

            # 再次发送RESYNC验证稳定性
            print("  再次RESYNC验证稳定性...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=3.0)
            if not resync_frame:
                print("✗ 第二次RESYNC未收到ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 第二次RESYNC收到非期望响应，类型: {frame_type:02x}")
                return False

            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()
            # 验证第二次RESYNC后通信正常
            test_data2 = b"Test second resync stability"
            self.stp.send(struct.pack(">H", len(test_data2)))
            self.stp.send(test_data2)
            echo_data2 = self.stp.recv(len(test_data2))

            if echo_data2 == test_data2:
                print("✓ RESYNC基于角色的固定模式重置测试通过")
                return True
            else:
                print(
                    f"✗ 第二次RESYNC后验证失败: 发送={test_data2.hex()}, 接收={echo_data2.hex()}"
                )
                return False

        except Exception as e:
            print(f"✗ RESYNC基于角色的固定模式重置测试异常：{e}")
            return False

    def test_multiple_resync(self) -> bool:
        """底层测试9: 多次连续RESYNC测试"""
        try:
            import struct

            print("测试多次连续RESYNC...")

            resync_count = 3
            for i in range(resync_count):
                print(f"  第{i+1}次RESYNC...")

                # 发送RESYNC_REQ
                self.stp.send_frame(TYPE_RESYNC_REQ)

                # 等待RESYNC_ACK
                resync_frame = self.wait_and_recv_frame(timeout=3.0)
                if not resync_frame:
                    print(f"✗ 第{i+1}次RESYNC未收到ACK响应")
                    return False

                frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
                if frame_type != TYPE_RESYNC_ACK:
                    print(f"✗ 第{i+1}次RESYNC收到非期望响应，类型: {frame_type:02x}")
                    return False

                # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
                self.stp.reset_comm()
                # 验证通信正常
                test_data = f"Test data after resync {i+1}".encode()
                self.stp.send(struct.pack(">H", len(test_data)))
                self.stp.send(test_data)
                echo_data = self.stp.recv(len(test_data))

                if echo_data != test_data:
                    print(f"✗ 第{i+1}次RESYNC后通信验证失败")
                    return False

                print(f"  第{i+1}次RESYNC成功")

            print("✓ 多次连续RESYNC测试通过")
            return True

        except Exception as e:
            print(f"✗ 多次连续RESYNC测试异常：{e}")
            return False

    def test_resync_after_multiple_errors(self) -> bool:
        """底层测试10: 多种错误后RESYNC恢复测试"""
        try:
            import struct

            print("测试多种错误后RESYNC恢复...")

            # 定义多种错误类型
            error_scenarios = [
                ("crc", "CRC错误"),
                ("incomplete", "不完整帧"),
                ("invalid_type", "无效类型"),
            ]

            # 连续发送多种错误
            for corrupt_type, description in error_scenarios:
                print(f"  制造{description}...")
                test_data = f"Error test: {description}".encode()
                self.send_corrupted_frame(TYPE_DATA, test_data, corrupt_type)

                # 短暂等待错误响应（可选）
                error_frame = self.wait_and_recv_frame(timeout=1.0)
                if error_frame:
                    print(f"    收到{description}响应")

                # 立即发送下一个错误，不等待

            print("  多种错误发送完成，使用RESYNC恢复...")

            # 使用RESYNC恢复
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=5.0)
            if not resync_frame:
                print("✗ 多种错误后未收到RESYNC_ACK响应")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type != TYPE_RESYNC_ACK:
                print(f"✗ 多种错误后收到非期望响应，类型: {frame_type:02x}")
                return False

            print("  收到RESYNC_ACK，验证通信恢复...")
            # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
            self.stp.reset_comm()

            # 验证通信完全恢复
            test_data = b"Recovery after multiple errors"
            self.stp.send(struct.pack(">H", len(test_data)))
            self.stp.send(test_data)
            echo_data = self.stp.recv(len(test_data))

            if echo_data == test_data:
                print("✓ 多种错误后RESYNC恢复测试通过")
                return True
            else:
                print(
                    f"✗ 多种错误后通信未恢复: 发送={test_data.hex()}, 接收={echo_data.hex()}"
                )
                return False

        except Exception as e:
            print(f"✗ 多种错误后RESYNC测试异常：{e}")
            return False

    def test_resync_during_receive(self) -> bool:
        """底层测试12: RESYNC中断接收测试 - 服务端发送状态重置到接收状态"""
        try:
            import struct

            print("测试RESYNC中断接收 - 服务端发送状态重置...")

            # 步骤1: 发送2字节数据长度
            data_size = 40
            print(f"  发送数据长度{data_size}...")
            self.stp.send(struct.pack(">H", data_size))

            # 步骤2: 发送对应长度的数据
            full_data = b"1234567890" * 4  # 40字节
            print("  发送40字节数据...")
            self.stp.send(full_data)

            # 步骤3: 接收一半对应长度的数据（20字节）
            print("  接收20字节数据（一半）...")
            half_data = self.stp.recv(20)

            # 步骤4: 发送重新同步帧
            print("  发送RESYNC_REQ中断接收...")
            self.stp.resync()

            # 步骤5: 重新发送2字节数据长度
            print(f"  重新发送数据长度{data_size}...")
            self.stp.send(struct.pack(">H", data_size))

            # 步骤6: 发送对应长度的数据
            print("  重新发送40字节数据...")
            self.stp.send(full_data)

            # 步骤7: 接收对应长度的数据
            print("  接收完整40字节数据...")
            received_data = self.stp.recv(data_size)

            # 步骤8: 检查发送和接收数据相同
            if received_data == full_data:
                print("✓ RESYNC中断接收测试通过 - 服务端正确重置到接收状态")
                return True
            else:
                print(
                    f"✗ 数据不匹配: 发送={full_data[:20].hex()}..., 接收={received_data[:20].hex()}..."
                )
                return False

        except Exception as e:
            print(f"✗ RESYNC中断接收测试异常：{e}")
            return False

    def test_string_frame_no_impact(self) -> bool:
        """底层测试13: 字符串帧不影响数据传输测试"""
        try:
            import struct

            print("测试字符串帧对数据传输的影响...")

            # 步骤1: 发送2字节数据长度
            data_size = 50
            full_data = b"ABCDEFGHIJ" * 5  # 50字节
            half_size = data_size // 2  # 25字节

            print(f"  发送数据长度{data_size}...")
            self.stp.send(struct.pack(">H", data_size))

            # 步骤2: 发送一半对应长度的数据
            print(f"  发送前{half_size}字节数据...")
            self.stp.send(full_data[:half_size])

            # 步骤3: 发送字符串数据
            string_data = b"Debug: Half data sent"
            print("  发送字符串帧...")
            self.stp.send_string(string_data)

            # 步骤4: 发送剩下一半对应长度的数据
            print(f"  发送后{data_size - half_size}字节数据...")
            self.stp.send(full_data[half_size:])

            # 步骤5: 接收一半对应长度的数据
            print(f"  接收前{half_size}字节数据...")
            received_half1 = self.stp.recv(half_size)

            # 步骤6: 发送字符串数据
            string_data2 = b"Debug: Half data received"
            print("  发送字符串帧...")
            self.stp.send_string(string_data2)

            # 步骤7: 接收剩下一半对应长度的数据
            print(f"  接收后{data_size - half_size}字节数据...")
            received_half2 = self.stp.recv(data_size - half_size)

            # 步骤8: 检查发送和接收数据相同
            received_data = received_half1 + received_half2
            if received_data == full_data:
                print("✓ 字符串帧不影响数据传输测试通过")
                return True
            else:
                print(
                    f"✗ 数据不匹配: 发送={full_data[:20].hex()}..., 接收={received_data[:20].hex()}..."
                )
                print(f"    发送长度={len(full_data)}, 接收长度={len(received_data)}")
                return False

        except Exception as e:
            print(f"✗ 字符串帧影响测试异常：{e}")
            return False

    def test_multiple_errors_recovery(self) -> bool:
        """底层测试11: 多次错误后恢复测试（原有测试）"""
        try:
            print("测试多次错误后RESYNC恢复...")
            error_types = [
                ("crc", "CRC错误"),
                ("invalid_type", "无效类型"),
                ("incomplete", "不完整帧"),
            ]

            # 连续发送多种错误帧
            for corrupt_type, description in error_types:
                test_data = f"Error test: {description}".encode()
                self.send_corrupted_frame(TYPE_DATA, test_data, corrupt_type)
                # 尝试接收错误响应
                error_frame = self.wait_and_recv_frame(timeout=2.0)
                if error_frame:
                    print(f"  {description}收到错误响应")
                time.sleep(0.3)

            # 发送RESYNC_REQ尝试恢复
            print("  发送RESYNC_REQ恢复...")
            self.stp.send_frame(TYPE_RESYNC_REQ)

            resync_frame = self.wait_and_recv_frame(timeout=5.0)
            if not resync_frame:
                print("✗ 多次错误后RESYNC未收到ACK")
                return False

            frame_type = resync_frame[OFF_TYPE] & TYPE_MASK
            if frame_type == TYPE_RESYNC_ACK:
                # 手动重置客户端状态（因为是手工发送的RESYNC_REQ）
                self.stp.reset_comm()
                # 验证通信恢复
                recovery_data = b"Recovery after multiple errors"
                self.stp.send_frame(TYPE_DATA, recovery_data)
                ack_frame = self.wait_and_recv_frame(timeout=3.0)
                if ack_frame and (ack_frame[OFF_TYPE] & TYPE_MASK) == TYPE_ACK:
                    print("✓ 多次错误恢复测试通过")
                    return True
                else:
                    print("✗ 多次错误后通信未恢复")
                    return False
            else:
                print(f"✗ 多次错误后RESYNC响应异常: {frame_type:02x}")
                return False
        except Exception as e:
            print(f"✗ 多次错误恢复测试异常：{e}")
            return False

    # ==================== 高层应用测试方法 ====================

    def test_max_payload(self) -> bool:
        """应用测试1: 最大负载数据传输（255字节）"""
        try:
            print("测试最大负载（255字节）...")
            max_data = random.randbytes(255)

            # 发送长度和数据
            self.stp.send(struct.pack(">H", 255))
            self.stp.send(max_data)

            # 接收回显数据
            echo_data = self.stp.recv(255)

            if echo_data == max_data:
                print("✓ 最大负载测试通过")
                return True
            else:
                print("✗ 最大负载测试失败：数据不匹配")
                return False
        except Exception as e:
            print(f"✗ 最大负载测试异常：{e}")
            return False

    def test_min_payload(self) -> bool:
        """应用测试2: 最小负载数据传输（1字节）"""
        try:
            print("测试最小负载（1字节）...")
            min_data = random.randbytes(1)

            # 发送长度和数据
            self.stp.send(struct.pack(">H", 1))
            self.stp.send(min_data)

            # 接收回显数据
            echo_data = self.stp.recv(1)

            if echo_data == min_data:
                print("✓ 最小负载测试通过")
                return True
            else:
                print("✗ 最小负载测试失败：数据不匹配")
                return False
        except Exception as e:
            print(f"✗ 最小负载测试异常：{e}")
            return False

    def test_zero_payload(self) -> bool:
        """应用测试3: 零负载数据传输"""
        try:
            print("测试零负载（0字节）...")

            # 发送长度0
            self.stp.send(struct.pack(">H", 0))
            time.sleep(0.5)

            # 发送测试数据验证连接正常
            test_data = b"test"
            self.stp.send(struct.pack(">H", len(test_data)))
            self.stp.send(test_data)
            echo_data = self.stp.recv(len(test_data))

            if echo_data == test_data:
                print("✓ 零负载测试通过")
                return True
            else:
                print("✗ 零负载测试失败：后续通信异常")
                return False
        except Exception as e:
            print(f"✗ 零负载测试异常：{e}")
            return False

    def test_fragmentation(self) -> bool:
        """应用测试4: 大数据分片传输"""
        try:
            print("测试大数据分片传输（1KB）...")
            large_data = random.randbytes(1024)

            # 告诉服务端总数据长度
            self.stp.send(struct.pack(">H", 1024))

            # 分片发送
            sent = 0
            while sent < 1024:
                chunk_size = min(random.randint(50, 200), 1024 - sent)
                self.stp.send(large_data[sent : sent + chunk_size])
                sent += chunk_size
                if sent % 256 == 0:
                    print(f"  已发送 {sent}/1024 字节")

            # 分片接收回显
            received_data = b""
            while len(received_data) < 1024:
                chunk_size = min(random.randint(50, 200), 1024 - len(received_data))
                chunk = self.stp.recv(chunk_size)
                received_data += chunk
                if len(received_data) % 256 == 0:
                    print(f"  已接收 {len(received_data)}/1024 字节")

            if received_data == large_data:
                print("✓ 分片传输测试通过")
                return True
            else:
                print("✗ 分片传输测试失败：数据不匹配")
                return False
        except Exception as e:
            print(f"✗ 分片传输测试异常：{e}")
            return False

    def test_high_frequency(self) -> bool:
        """应用测试5: 高频数据传输"""
        try:
            print("测试高频数据传输（50个小包）...")
            success_count = 0
            packet_count = 50

            start_time = time.time()

            for i in range(packet_count):
                try:
                    size = random.randint(1, 20)
                    data = random.randbytes(size)

                    self.stp.send(struct.pack(">H", size))
                    self.stp.send(data)
                    echo = self.stp.recv(size)

                    if echo == data:
                        success_count += 1

                    if (i + 1) % 10 == 0:
                        print(f"  进度: {i + 1}/{packet_count}")

                except Exception as e:
                    log.debug(f"高频测试第{i+1}包异常：{e}")

            elapsed = time.time() - start_time
            frequency = packet_count / elapsed
            success_rate = success_count / packet_count

            print(f"  完成：{packet_count}包，{elapsed:.2f}秒，{frequency:.1f}包/秒")

            if success_rate >= 0.9:
                print(f"✓ 高频传输测试通过，成功率：{success_rate:.1%}")
                return True
            else:
                print(f"✗ 高频传输测试失败，成功率：{success_rate:.1%}")
                return False
        except Exception as e:
            print(f"✗ 高频传输测试异常：{e}")
            return False

    def test_random_patterns(self) -> bool:
        """应用测试6: 随机数据模式测试"""
        try:
            print("测试随机数据模式...")
            patterns = [
                (b"\x00" * 50, "全零"),
                (b"\xff" * 50, "全一"),
                (b"\x55" * 50, "0x55模式"),
                (b"\xaa" * 50, "0xAA模式"),
                (bytes(range(256)), "递增序列"),
                (b"\x5a" * 30, "帧头字节"),
                (random.randbytes(100), "随机数据"),
            ]

            success_count = 0

            for data, desc in patterns:
                try:
                    print(f"  测试 {desc} ({len(data)}字节)")

                    self.stp.send(struct.pack(">H", len(data)))
                    self.stp.send(data)
                    echo = self.stp.recv(len(data))

                    if echo == data:
                        success_count += 1
                        print(f"    ✓ {desc} 通过")
                    else:
                        print(f"    ✗ {desc} 失败")

                except Exception as e:
                    print(f"    ✗ {desc} 异常：{e}")

            if success_count >= len(patterns) * 0.8:
                print(f"✓ 随机模式测试通过 ({success_count}/{len(patterns)})")
                return True
            else:
                print(f"✗ 随机模式测试失败 ({success_count}/{len(patterns)})")
                return False
        except Exception as e:
            print(f"✗ 随机模式测试异常：{e}")
            return False

    def test_long_duration(self, duration: int = 20) -> bool:
        """应用测试7: 长时间运行稳定性"""
        try:
            print(f"测试长时间运行稳定性（{duration}秒）...")
            start_time = time.time()
            packet_count = 0
            error_count = 0

            while time.time() - start_time < duration:
                try:
                    size = random.randint(1, 50)
                    data = random.randbytes(size)

                    self.stp.send(struct.pack(">H", size))
                    self.stp.send(data)
                    echo = self.stp.recv(size)

                    if echo == data:
                        packet_count += 1
                    else:
                        error_count += 1

                    if packet_count % 20 == 0 and packet_count > 0:
                        elapsed = time.time() - start_time
                        print(
                            f"  {elapsed:.0f}秒: {packet_count}包成功，{error_count}包错误"
                        )

                    time.sleep(random.uniform(0.01, 0.05))

                except Exception as e:
                    error_count += 1
                    log.debug(f"长时间测试异常：{e}")

            total = packet_count + error_count
            success_rate = packet_count / total if total > 0 else 0

            print(f"  完成：{packet_count}包成功，{error_count}包错误")

            if success_rate >= 0.95:
                print(f"✓ 长时间稳定性测试通过，成功率：{success_rate:.1%}")
                return True
            else:
                print(f"✗ 长时间稳定性测试失败，成功率：{success_rate:.1%}")
                return False
        except Exception as e:
            print(f"✗ 长时间稳定性测试异常：{e}")
            return False

    def test_mode_switching(self) -> bool:
        """应用测试8: 模式切换测试"""
        try:
            print("测试发送/接收模式切换...")

            for i in range(3):
                print(f"  第{i+1}轮测试...")

                size = random.randint(10, 50)
                data = random.randbytes(size)
                self.stp.send(struct.pack(">H", size))
                self.stp.send(data)

                echo = self.stp.recv(size)

                if echo != data:
                    print(f"✗ 模式切换测试失败：第{i+1}轮数据不匹配")
                    return False

                time.sleep(0.1)

            print("✓ 模式切换测试通过")
            return True
        except Exception as e:
            print(f"✗ 模式切换测试异常：{e}")
            return False

    def test_resync_api(self) -> bool:
        """应用测试9: RESYNC API测试"""
        try:
            print("测试RESYNC API...")

            # 先发送正常数据
            test_data = random.randbytes(50)
            self.stp.send(struct.pack(">H", 50))
            self.stp.send(test_data)
            echo = self.stp.recv(50)

            if echo != test_data:
                print("✗ RESYNC API测试失败：初始通信异常")
                return False

            print("  初始通信正常，执行RESYNC...")
            self.stp.resync()

            # 测试RESYNC后的通信
            test_data2 = random.randbytes(30)
            self.stp.send(struct.pack(">H", 30))
            self.stp.send(test_data2)
            echo2 = self.stp.recv(30)

            if echo2 == test_data2:
                print("✓ RESYNC API测试通过")
                return True
            else:
                print("✗ RESYNC API测试失败：RESYNC后通信异常")
                return False
        except Exception as e:
            print(f"✗ RESYNC API测试异常：{e}")
            return False

    def test_abnormal_disconnect(self) -> bool:
        """应用测试10: 异常断开重连测试"""
        try:
            print("测试异常断开重连...")

            # 先进行正常通信
            test_data = random.randbytes(50)
            self.stp.send(struct.pack(">H", 50))
            self.stp.send(test_data)
            echo = self.stp.recv(50)

            if echo != test_data:
                print("✗ 断开重连测试失败：初始通信异常")
                return False

            print("  初始通信正常，模拟异常断开...")

            # 获取底层串口并直接关闭
            com = self.stp.get_com()
            if com.is_open:
                com.close()

            print("  连接已断开，等待1秒...")
            time.sleep(1.0)

            # 尝试重新连接
            print("  尝试重新连接...")
            self.stp = None
            if not self.setup_connection():
                print("✗ 断开重连测试失败：无法重新连接")
                return False

            # 测试新连接
            test_data2 = random.randbytes(30)
            self.stp.send(struct.pack(">H", 30))
            self.stp.send(test_data2)
            echo2 = self.stp.recv(30)

            if echo2 == test_data2:
                print("✓ 异常断开重连测试通过")
                return True
            else:
                print("✗ 断开重连测试失败：重连后通信异常")
                return False
        except Exception as e:
            print(f"✗ 异常断开重连测试异常：{e}")
            return False


def run_protocol_tests(tester: ComprehensiveTester) -> Tuple[int, int]:
    """运行底层协议测试"""
    print("\n" + "=" * 60)
    print("底层协议测试")
    print("=" * 60)

    tests = [
        ("基本RESYNC功能", tester.test_resync_basic),
        ("CRC错误处理", tester.test_crc_error),
        ("不完整帧处理", tester.test_incomplete_frame),
        ("RESYNC数据传输恢复", tester.test_resync_with_data),
        ("数据传输中断RESYNC", tester.test_resync_during_data_transfer),
        ("CRC错误后RESYNC恢复", tester.test_resync_after_crc_error),
        ("超时错误后RESYNC恢复", tester.test_resync_after_timeout),
        ("RESYNC角色固定重置", tester.test_resync_role_based_reset),
        ("多次连续RESYNC", tester.test_multiple_resync),
        ("多种错误后RESYNC恢复", tester.test_resync_after_multiple_errors),
        ("RESYNC中断接收测试", tester.test_resync_during_receive),
        ("字符串帧不影响数据传输", tester.test_string_frame_no_impact),
        ("多次错误恢复（原有）", tester.test_multiple_errors_recovery),
    ]

    passed = 0
    total = len(tests)

    for name, test_func in tests:
        print(f"\n--- {name} ---")

        # 每个测试前重新建立连接，确保干净的初始状态
        tester.cleanup_connection()
        time.sleep(0.2)  # 短暂延时确保清理完成

        if tester.setup_connection():
            if test_func():
                passed += 1
            else:
                print(f"  {name} 测试失败")
        else:
            print(f"  {name} 连接建立失败")

        time.sleep(0.5)

    print(f"\n底层协议测试结果：{passed}/{total} 通过")
    return passed, total


def run_boundary_tests(tester: ComprehensiveTester) -> Tuple[int, int]:
    """运行边界条件测试"""
    print("\n" + "=" * 60)
    print("边界条件测试")
    print("=" * 60)

    tests = [
        ("最大负载", tester.test_max_payload),
        ("最小负载", tester.test_min_payload),
        ("零负载", tester.test_zero_payload),
        ("分片传输", tester.test_fragmentation),
    ]

    passed = 0
    total = len(tests)

    for name, test_func in tests:
        print(f"\n--- {name} ---")

        # 每个测试前重新建立连接，确保干净的初始状态
        tester.cleanup_connection()
        time.sleep(0.2)

        if tester.setup_connection():
            if test_func():
                passed += 1
            else:
                print(f"  {name} 测试失败")
        else:
            print(f"  {name} 连接建立失败")

        time.sleep(0.5)

    print(f"\n边界条件测试结果：{passed}/{total} 通过")
    return passed, total


def run_stress_tests(tester: ComprehensiveTester) -> Tuple[int, int]:
    """运行压力测试"""
    print("\n" + "=" * 60)
    print("压力测试")
    print("=" * 60)

    tests = [
        ("高频传输", tester.test_high_frequency),
        ("随机模式", tester.test_random_patterns),
    ]

    passed = 0
    total = len(tests)

    for name, test_func in tests:
        print(f"\n--- {name} ---")

        # 每个测试前重新建立连接，确保干净的初始状态
        tester.cleanup_connection()
        time.sleep(0.2)

        if tester.setup_connection():
            if test_func():
                passed += 1
            else:
                print(f"  {name} 测试失败")
        else:
            print(f"  {name} 连接建立失败")

        time.sleep(0.5)

    print(f"\n压力测试结果：{passed}/{total} 通过")
    return passed, total


def run_stability_tests(
    tester: ComprehensiveTester, quick: bool = False
) -> Tuple[int, int]:
    """运行稳定性测试"""
    print("\n" + "=" * 60)
    print("稳定性测试")
    print("=" * 60)

    # 快速模式只运行10秒，否则20秒
    duration = 10 if quick else 20

    passed = 0
    total = 1

    print(f"\n--- 长时间运行稳定性 ---")

    # 稳定性测试前重新建立连接
    tester.cleanup_connection()
    time.sleep(0.2)

    if tester.setup_connection():
        if tester.test_long_duration(duration):
            passed = 1
        else:
            print("  长时间运行稳定性测试失败")
    else:
        print("  长时间运行稳定性连接建立失败")

    print(f"\n稳定性测试结果：{passed}/{total} 通过")
    return passed, total


def run_application_tests(tester: ComprehensiveTester) -> Tuple[int, int]:
    """运行高层应用测试"""
    print("\n" + "=" * 60)
    print("高层应用测试")
    print("=" * 60)

    tests = [
        ("模式切换", tester.test_mode_switching),
        ("RESYNC API", tester.test_resync_api),
        ("异常断开重连", tester.test_abnormal_disconnect),
    ]

    passed = 0
    total = len(tests)

    for name, test_func in tests:
        print(f"\n--- {name} ---")

        # 每个测试前重新建立连接，确保干净的初始状态
        tester.cleanup_connection()
        time.sleep(0.2)

        if tester.setup_connection():
            if test_func():
                passed += 1
            else:
                print(f"  {name} 测试失败")
        else:
            print(f"  {name} 连接建立失败")

        time.sleep(0.5)

    print(f"\n高层应用测试结果：{passed}/{total} 通过")
    return passed, total


def main(args):
    """主测试函数"""
    print("\n" + "=" * 80)
    print("STP协议综合测试 - 客户端")
    print("=" * 80)
    print(f"配置: 端口={args.port}, 波特率={args.baudrate}")
    print("\n注意：请确保 server.py 正在运行！")
    print("=" * 80)

    tester = ComprehensiveTester(args.port, args.baudrate)

    # 测试初始连接是否正常
    if not tester.setup_connection():
        print("\n✗ 无法建立连接，请检查：")
        print("  1. server.py 是否正在运行")
        print("  2. 串口连接是否正确")
        print("  3. 波特率是否匹配")
        return 1

    # 初始连接测试完成，清理连接，后续每个测试会重新建立连接
    tester.cleanup_connection()

    try:
        total_passed = 0
        total_tests = 0

        # 根据参数选择测试
        if args.test_type == "all" or args.test_type == "protocol":
            passed, total = run_protocol_tests(tester)
            total_passed += passed
            total_tests += total
            time.sleep(1)  # 测试类别间间隔

        if args.test_type == "all" or args.test_type == "boundary":
            passed, total = run_boundary_tests(tester)
            total_passed += passed
            total_tests += total
            time.sleep(1)  # 测试类别间间隔

        if args.test_type == "all" or args.test_type == "stress":
            passed, total = run_stress_tests(tester)
            total_passed += passed
            total_tests += total
            time.sleep(1)  # 测试类别间间隔

        if args.test_type == "all" or args.test_type == "stability":
            passed, total = run_stability_tests(tester, args.quick)
            total_passed += passed
            total_tests += total
            time.sleep(1)  # 测试类别间间隔

        if args.test_type == "all" or args.test_type == "application":
            passed, total = run_application_tests(tester)
            total_passed += passed
            total_tests += total

        # 显示总结
        print("\n" + "=" * 80)
        print("综合测试结果总结")
        print("=" * 80)
        print(f"通过: {total_passed}/{total_tests}")
        success_rate = (total_passed / total_tests * 100) if total_tests > 0 else 0
        print(f"成功率: {success_rate:.1f}%")

        if total_passed == total_tests:
            print("\n🎉 所有测试通过！")
            return 0
        else:
            print(f"\n⚠️  {total_tests - total_passed} 个测试失败")
            return 1

    except KeyboardInterrupt:
        print("\n\n测试被用户中断")
        return 130
    except Exception as e:
        print(f"\n\n测试异常: {e}")
        return 1
    finally:
        tester.cleanup_connection()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="STP协议综合测试程序（客户端）",
        epilog="需要先运行 server.py 作为服务端",
    )
    parser.add_argument(
        "--port", "-p", default="/dev/ttyUSB0", help="客户端串口 (默认: /dev/ttyUSB0)"
    )
    parser.add_argument(
        "--baudrate", "-b", type=int, default=115200, help="波特率 (默认: 115200)"
    )
    parser.add_argument("--verbose", "-v", action="store_true", help="详细日志输出")
    parser.add_argument(
        "--quick", "-q", action="store_true", help="快速测试模式（缩短稳定性测试时间）"
    )
    parser.add_argument(
        "--test-type",
        "-t",
        choices=["all", "protocol", "boundary", "stress", "stability", "application"],
        default="all",
        help="选择测试类型 (默认: all)",
    )

    args = parser.parse_args()

    # 设置日志级别
    if args.verbose:
        log.basicConfig(
            format="[%(levelname)s:%(funcName)s:%(lineno)d] %(message)s",
            level=log.DEBUG,
        )
    else:
        log.basicConfig(format="[%(levelname)s] %(message)s", level=log.INFO)

    sys.exit(main(args))
