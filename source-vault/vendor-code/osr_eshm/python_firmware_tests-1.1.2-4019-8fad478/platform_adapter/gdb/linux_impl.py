from .base import GdbInterface

class LinuxGdb(GdbInterface):
    def start_session(self):
        print("[LinuxGdb] 启动")

    def send_command(self, cmd: str) -> str:
        print(f"[LinuxGdb] 执行命令: {cmd}")
        return "linux response"

    def stop_session(self):
        print("[LinuxGdb] 停止")
