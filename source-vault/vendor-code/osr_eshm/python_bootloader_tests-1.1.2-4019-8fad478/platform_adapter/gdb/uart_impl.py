from .base import GdbInterface
from typing import Tuple, Optional
import subprocess
import platform
import re
from pathlib import Path


class UartGdb(GdbInterface):
    """UART GDB 接口实现，支持通过 OpenOCD 读取寄存器"""

    # 默认 OpenOCD 配置文件（相对于 server/scripts 目录）
    DEFAULT_CONFIG = "openocd_ftdi.cfg"
    DEFAULT_TIMEOUT = 10

    def __init__(self):
        self._openocd_path: Optional[str] = None

    def _get_openocd_path(self) -> str:
        """获取 OpenOCD 可执行文件路径"""
        if self._openocd_path:
            return self._openocd_path

        if platform.system() == 'Windows':
            # Reason: Windows 下检查常见的 OpenOCD 安装路径
            openocd_paths = [
                r'D:\A04_Works\07_ai_event\04_openocd\openocd-0.12.0\bin\openocd.exe',
                r'C:\openocd\bin\openocd.exe',
                r'C:\Program Files\OpenOCD\bin\openocd.exe',
                'openocd.exe'
            ]
            for path in openocd_paths:
                if Path(path).exists():
                    self._openocd_path = path
                    return path

        self._openocd_path = 'openocd'
        return 'openocd'

    def _get_default_config_path(self) -> str:
        """获取默认配置文件路径"""
        # Reason: 配置文件位于 server/scripts 目录下
        script_dir = Path(__file__).parent.parent.parent / "server" / "scripts"
        return str(script_dir / self.DEFAULT_CONFIG)

    def read_register(self, address: int, config_file: Optional[str] = None) -> Tuple[bool, int]:
        """
        使用 OpenOCD 读取指定地址的寄存器值

        Args:
            address: 寄存器地址
            config_file: OpenOCD 配置文件路径，默认使用 openocd_ftdi.cfg

        Returns:
            Tuple[bool, int]: (成功标志, 寄存器值)
        """
        if config_file is None:
            config_file = self._get_default_config_path()

        try:
            openocd_path = self._get_openocd_path()
            # Reason: 使用 -c 参数直接执行命令，比脚本文件方式更可靠
            cmd = [
                openocd_path,
                '-f', config_file,
                '-c', f'mdw 0x{address:08x}',
                '-c', 'shutdown'
            ]

            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace',
                timeout=self.DEFAULT_TIMEOUT
            )

            # Reason: 解析 mdw 输出格式: "0x30006000: 00000000"
            all_output = (result.stdout or "") + (result.stderr or "")
            # 匹配格式: "0x30006000: 00000000" 或 "0x30006000: 0x00000000"
            pattern = rf'0x{address:08x}:\s*(?:0x)?([0-9a-fA-F]+)'
            match = re.search(pattern, all_output, re.IGNORECASE)

            if match:
                value = int(match.group(1), 16)
                return True, value
            else:
                print(f"[UartGdb] 未能解析寄存器值，输出: {all_output}")
                return False, 0

        except subprocess.TimeoutExpired:
            print(f"[UartGdb] OpenOCD 执行超时")
            return False, 0
        except Exception as e:
            print(f"[UartGdb] OpenOCD 执行错误: {e}")
            return False, 0

    def load_elf(self, path: str) -> Tuple[int, str]:
        pass

    def send_command(self, cmd: str) -> str:
        print(f"[UartGdb] 执行命令: {cmd}")
        return "uart response"

    def stop_session(self):
        print("[UartGdb] 停止")
