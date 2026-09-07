#!/usr/bin/env python3
# -*- coding:utf-8  -*-
"""
2.4版本看门狗测试辅助函数
提供超时时间计算和验证功能
"""

import time
import logging as log
from utils.config import cfg_data
from platform_adapter.host.loader import get_host_interface

host = get_host_interface()


def calculate_watchdog_timeout(n: int, freq_hz: int = None) -> float:
    """
    计算看门狗理论超时时间

    Args:
        n: OTP配置的N值 (0-15)
        freq_hz: 系统频率(Hz)，默认使用cfg_data.TEST_CPU_FREQ_HZ

    Returns:
        float: 超时时间(秒)

    Raises:
        ValueError: 如果N值超出范围
    """
    # Reason: N值用4bit表示，范围必须是0-15
    if not 0 <= n <= 15:
        raise ValueError(f"N值必须在0-15范围内，当前值: {n}")

    if freq_hz is None:
        freq_hz = cfg_data.TEST_CPU_FREQ_HZ

    # Reason: 根据2.4版本公式计算超时cycle数
    timeout_cycles = 2 ** (18 + n)
    timeout_seconds = timeout_cycles / freq_hz

    log.debug(f"看门狗配置: N={n}, 频率={freq_hz}Hz")
    log.debug(f"超时cycles={timeout_cycles}, 超时时间={timeout_seconds:.3f}s")

    return timeout_seconds


def verify_watchdog_timeout(
    n: int,
    tolerance: float = 0.12,
    freq_hz: int = None
) -> tuple[bool, float]:
    """
    验证看门狗超时时间

    Args:
        n: N值配置
        tolerance: 允许误差范围(0.12表示±12%)
        freq_hz: 系统频率(Hz)

    Returns:
        tuple: (是否在预期范围内, 实际超时时间)
    """
    expected_timeout = calculate_watchdog_timeout(n, freq_hz)

    # 计算容差范围
    min_timeout = expected_timeout * (1 - tolerance)
    max_timeout = expected_timeout * (1 + tolerance)
    max_wait = expected_timeout * 2  # 最多等待2倍理论时间

    log.info(f"预期超时时间: {expected_timeout:.3f}s")
    log.info(f"容差范围: [{min_timeout:.3f}s, {max_timeout:.3f}s]")

    start_time = time.time()
    timeout_triggered = False
    actual_timeout = 0.0

    # Reason: 循环检测看门狗超时状态
    while time.time() - start_time < max_wait:
        current_time = time.time() - start_time

        # 只在预期时间附近开始检测
        if current_time >= min_timeout * 0.9:
            status = host.check_watchdog_error_status()
            if status:
                actual_timeout = current_time
                timeout_triggered = True
                log.info(f"看门狗超时触发，实际时间: {actual_timeout:.3f}s")
                break

        time.sleep(0.01)  # 10ms检测间隔

    if not timeout_triggered:
        log.error(f"看门狗未在{max_wait:.3f}s内超时")
        return False, 0.0

    # 验证超时时间在容差范围内
    in_range = min_timeout <= actual_timeout <= max_timeout
    if in_range:
        deviation = abs(actual_timeout - expected_timeout) / expected_timeout * 100
        log.info(f"超时时间验证通过，偏差: {deviation:.1f}%")
    else:
        log.error(f"超时时间超出容差范围！")

    return in_range, actual_timeout
