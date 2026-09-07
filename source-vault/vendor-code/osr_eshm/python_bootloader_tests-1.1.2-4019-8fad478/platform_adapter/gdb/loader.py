from ..platform_config import PLATFORM
from .linux_impl import LinuxGdb
from .embedded_impl import EmbeddedGdb
from .uart_impl import UartGdb
from .base import GdbInterface

def get_gdb_interface() -> GdbInterface:
    if PLATFORM == "linux":
        return LinuxGdb()
    elif PLATFORM == "embedded":
        return EmbeddedGdb()
    elif PLATFORM == "uart":
        return UartGdb()
    else:
        raise NotImplementedError(f"Unsupported platform: {PLATFORM}")
