import os
from pathlib import Path
import re
import shutil
import threading
import time
from typing import Tuple
from .base import GdbInterface
import logging as log
import subprocess
import select
from typing import Optional, Union, List

# ---------------------- 自定义异常 ----------------------
class GdbError(Exception):
    """GDB 基础异常"""
    pass

class GdbTimeoutError(GdbError):
    """GDB 操作超时"""
    pass

class GdbCommandError(GdbError):
    """GDB 命令执行失败"""
    def __init__(self, cmd: str, message: str):
        self.cmd = cmd
        self.message = message
        super().__init__(f"Command '{cmd}' failed: {message}")
    def __str__(self):
        return f"[GDB Command Error] {self.message} (Command: '{self.cmd}')"
    
class FileLoadError(GdbError):
    """文件加载失败"""
    pass

class ConnectionError(GdbError):
    """远程连接失败"""
    pass

class MemoryAccessError(GdbError):
    def __init__(self, address: int, message: str):
        self.address = address
        self.message = message
        super().__init__(f"Memory access error at 0x{address:x}: {message}")

    def __str__(self):
        return f"[Memory Error] 0x{self.address:x}: {self.message}"
    
class OpenOCDError(GdbError):
    """OpenOCD 相关错误"""
    def __init__(self, message: str, log: Optional[str] = None):
        self.log = log
        super().__init__(f"OpenOCD Error: {message}\nLog: {log if log else 'N/A'}")

# ---------------------- 新增 OpenOCD 输出读取线程 ----------------------
class OpenOCDOutputReader:
    def __init__(self, process: subprocess.Popen):
        self.process = process
        self.output_buffer = []
        self._stop_event = threading.Event()
        self._thread = threading.Thread(target=self._read_output, daemon=True)
        self._thread.start()

    def _read_output(self):
        """后台线程持续读取 OpenOCD 输出"""
        while not self._stop_event.is_set() and self.process.poll() is None:
            line = self.process.stdout.readline()
            if line:
                self.output_buffer.append(line.strip())
            else:
                time.sleep(0.1)

    def get_output(self) -> List[str]:
        """获取当前累积的输出并清空缓冲区"""
        output = self.output_buffer.copy()
        self.output_buffer.clear()
        return output

    def stop(self):
        """停止读取线程"""
        self._stop_event.set()
        self._thread.join(timeout=1)

class EmbeddedGdb(GdbInterface):
    def __init__(self, gdb_path: str = "riscv32-wing-elf-gdb",): 
        super().__init__()
        self.gdb_process = None
        self.elf_path = None
        self.bin_info = {}
        self.is_remote = False
        # 启动 GDB 进程
        self._start_gdb(gdb_path)
    def _start_gdb(self, gdb_path: str):
        """启动 GDB 进程"""
        try:
            self.gdb_process = subprocess.Popen(
                [gdb_path, "--nx", "--quiet"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
        except FileNotFoundError as e:
            raise GdbError(f"GDB not found at path: {gdb_path}") from e
        except Exception as e:
            raise GdbError(f"Failed to start GDB: {str(e)}") from e
        
    def _send_command(self, cmd: str, timeout: float = 10.0) -> str:
        """发送命令并返回输出（含错误检测）"""
        if self.gdb_process.poll() is not None:
            raise GdbError("GDB process is not running")

        # 重置输出缓冲区
        if self.openocd_reader is None:
            self.openocd_reader = OpenOCDOutputReader(self.gdb_process)
        else:
            self.openocd_reader.get_output()  # 清空旧数据

        # 发送命令
        self.gdb_process.stdin.write(cmd + "\n")
        self.gdb_process.stdin.flush()

        # 读取输出直到遇到 (gdb) 提示符或超时
        output = []
        start_time = time.time()
        gdb_prompt_re = re.compile(r"\(gdb\)")
        while True:
            # 检查超时
            if time.time() - start_time > timeout:
                raise GdbTimeoutError(f"Timeout while executing command: {cmd}")

            # 非阻塞读取输出
            # 获取最新输出
            new_lines = self.openocd_reader.get_output()
            for line in new_lines:
                output.append(line)
                if gdb_prompt_re.search(line):  # 检测到 (gdb) 提示符
                    return "\n".join(output[:-1])  # 排除最后的 (gdb) 行

            # 检查进程是否意外退出
            if self.gdb_process.poll() is not None:
                raise GdbError("GDB process exited unexpectedly")

            time.sleep(0.01)  # 降低 CPU 占用
    
    def start_openocd(
        self,
        config_files: List[str],
        openocd_path: str = "openocd",
        work_dir: Optional[str] = None,
        search_paths: Optional[List[str]] = None,
        port: int = 3333,
        timeout: float = 10.0,
        debug: bool = False
    ) -> None:
        """
        启动 OpenOCD 进程并等待其就绪
        :param config_files: OpenOCD 配置文件列表（如 ["board/st_nucleo_f4.cfg"]）
        :param openocd_path: OpenOCD 可执行文件路径
        :param work_dir: 工作目录（默认为当前目录）
        :param search_paths: 配置文件搜索路径
        :param port: 监听的 GDB 端口（用于验证是否启动成功）
        :param timeout: 启动超时时间（秒）
        :param debug: 是否打印 OpenOCD 日志到控制台
        """
        subprocess.call(["taskkill", "/F", "/T", "/IM", "openocd"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
        # 检查 OpenOCD 是否存在
        if not Path(openocd_path).exists():
            raise OpenOCDError(f"OpenOCD not found at: {openocd_path}")
        openocd_path = shutil.which("openocd")

        # 构建命令参数
        cmd = [openocd_path]
        if search_paths:
            cmd.extend(["-s", os.pathsep.join(search_paths)])
        for cfg in config_files:
            cmd.extend(["-f", cfg])
        cmd.insert(0, "/MIN,")
        cmd.insert(0, "start,")
        print(cmd)
        # 启动进程
        try:
            self.openocd_process = subprocess.Popen(
                ["start", "/MIN", "openocd", "-f", r"C:\\Program\ Files\\wing-openocd-windows\\openocd\\openocd_ftdi.cfg"],
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True  # Python 3.6+，自动解码为 str
            )

        # try:
        #     self.openocd_process = subprocess.Popen(
        #         cmd,
        #         cwd=work_dir,
        #         stdout=subprocess.PIPE,
        #         stderr=subprocess.STDOUT,
        #         text=True,
        #         bufsize=1,
        #         universal_newlines=True
        #     )
        except Exception as e:
            raise OpenOCDError(f"Failed to start OpenOCD: {cmd} {str(e)}") from e

        # 启动输出读取线程
        self.openocd_reader = OpenOCDOutputReader(self.openocd_process)
        
        # 等待 OpenOCD 就绪
        start_time = time.time()
        success_pattern = re.compile(rf"Listening on port {port} for gdb connections")
        error_patterns = [re.compile(r"Error:", re.IGNORECASE), re.compile(r"failed", re.IGNORECASE)]

        while time.time() - start_time < timeout:
            # 检查进程状态
            if self.openocd_process.poll() is not None:
                log = "\n".join(self.openocd_reader.get_output())
                self.stop_openocd()
                raise OpenOCDError("OpenOCD exited prematurely", log=log)

            # 获取最新输出
            for line in self.openocd_reader.get_output():
                if debug:
                    print(f"[OpenOCD] {line}")

                # 检测成功条件
                if success_pattern.search(line):
                    return

                # 检测错误条件
                for pattern in error_patterns:
                    if pattern.search(line):
                        log = "\n".join(self.openocd_reader.get_output())
                        self.stop_openocd()
                        raise OpenOCDError("OpenOCD startup failed", log=log)

            time.sleep(0.1)  # 避免 CPU 占用过高

        # 超时处理
        log = "\n".join(self.openocd_reader.get_output())
        self.stop_openocd()
        raise OpenOCDError(f"OpenOCD startup timed out after {timeout}s", log=log)

    def stop_openocd(self) -> None:
        """停止 OpenOCD 进程"""
        if self.openocd_process:
            if self.openocd_reader:
                self.openocd_reader.stop()  # 先停止读取线程
            try:
                self.openocd_process.terminate()
                self.openocd_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.openocd_process.kill()
            self.openocd_process = None
            self.openocd_reader = None


    def load_elf(self, elf_path: str):
        """加载 ELF 文件"""
        # if not os.path.exists(elf_path):
        #     raise FileLoadError(f"ELF file not found: {elf_path}")
        # try:
        #     self.elf_path = elf_path
        #     self._send_command(f"file {elf_path}")
        # except GdbCommandError as e:
        #     raise FileLoadError(f"Failed to load ELF: {e.message}") from e

        print("[EmbeddedGdb] 启动")
        # subprocess.call(["taskkill", "/F", "/T", "/IM", "openocd"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
        
        # openocd = subprocess.Popen(
        #     ["start", "/MIN", "openocd", "-f", r"C:\\Program Files\\wing-openocd-windows\\openocd\\openocd_ftdi.cfg"],
        #     shell=True,
        #     stdout=subprocess.PIPE,
        #     stderr=subprocess.PIPE,
        #     text=True  # Python 3.6+，自动解码为 str
        # )
        # stdout, stderr = openocd.communicate(timeout=10)

        # if openocd.returncode != 0:
        #     log.warning("Command returned non-zero exit code %d", openocd.returncode)
        #     log.warning("stderr: %s %s", stderr.strip(), stdout.strip())


        self.gdb = subprocess.Popen(["riscv32-wing-elf-gdb"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.send_cmd("target remote localhost:3333")
        self.read_output()

        self.send_cmd("target extended-remote localhost:3333\n")
        self.read_output()

        self.send_cmd(f"file {elf_path}\n")
        self.read_output()


        self.send_cmd(f"load\n")
        self.read_output()

        self.send_cmd(f"quit\n")
        self.read_output()

        self.gdb.wait(20)

        self.gdb.terminate()


        # subprocess.call(["taskkill", "/F", "/T", "/IM", "openocd"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, shell=True)
        # proc = subprocess.Popen(
        #     ["taskkill", "/F", "/T", "/IM", "openocd.exe"],
        #     stdout=subprocess.DEVNULL,
        #     stderr=subprocess.DEVNULL,
        #     shell=True
        # )
        print("[EmbeddedGdb] 结束")

    def send_command(self, cmd: str) -> str:
        print(f"[EmbeddedGdb] 执行命令: {cmd}")
        return "embedded response"

    def stop_session(self):
        print("[EmbeddedGdb] 停止")

    # 向 GDB 输入命令
    def send_cmd(self, cmd: str):
        print(f"==> {cmd.strip()}")
        self.gdb.stdin.write(cmd + "\n")
        self.gdb.stdin.flush()  # 确保命令立即发送
        time.sleep(0.1)
        print(f"==> {self.gdb.stderr.readline()}")

    # 读取输出（可按行读取）
    def read_output(self):
        time.sleep(0.1)  # 给 GDB 一点时间响应
        while True:
            line = self.gdb.stdout.readline()
            if not line:
                break
            print(f"<== {line.strip()}")
            if "(gdb)" in line:
                break

    def start_openocd2(self, cmd, timeout=10):
        try:
            # 启动 OpenOCD
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,  # 将 stderr 合并到 stdout
                text=True,
                shell=True,
                bufsize=1
            )

            def read_output():
                for line in proc.stdout:
                    print("[OpenOCD]", line.strip())

            # 启动读取输出线程
            output_thread = threading.Thread(target=read_output, daemon=True)
            output_thread.start()

            # 等待一段时间检查是否超时
            start_time = time.time()
            while time.time() - start_time < timeout:
                if proc.poll() is not None:
                    print(f"OpenOCD exited early with code {proc.returncode}")
                    return proc.returncode
                time.sleep(0.1)

            print("OpenOCD started successfully.")

            return proc  # 返回进程对象供后续控制（如 kill）

        except Exception as e:
            print(f"[Error] Failed to start OpenOCD: {e}")
            return None