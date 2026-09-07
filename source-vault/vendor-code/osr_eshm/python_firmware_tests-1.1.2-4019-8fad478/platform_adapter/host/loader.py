from ..platform_config import PLATFORM
from .linux_impl import LinuxHost
from .embedded_impl import EmbeddedHost
from .uart_impl import UartHost
from .base import HostInterface

def get_host_interface() -> HostInterface:
    if PLATFORM == "linux":
        return LinuxHost()
    elif PLATFORM == "embedded":
        return EmbeddedHost()
    elif PLATFORM == "uart":
        return UartHost()
    else:
        raise NotImplementedError(f"Unsupported platform: {PLATFORM}")
