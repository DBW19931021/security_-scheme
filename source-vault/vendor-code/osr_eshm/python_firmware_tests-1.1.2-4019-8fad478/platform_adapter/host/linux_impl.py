from .base import HostInterface

class LinuxHost(HostInterface):
    def start_session(self):
        print("[LinuxHost] 启动")

    def send_command(self, cmd: str) -> str:
        print(f"[LinuxHost] 执行命令: {cmd}")
        return "linux response"

    def stop_session(self):
        print("[LinuxHost] 停止")
