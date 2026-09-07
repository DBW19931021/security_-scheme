from ..platform_config import PLATFORM
from .linux_impl import LinuxApi
from .embedded_impl import EmbeddedApi
from .uart_impl import UartApi
from .base import ApiInterface

def get_api_interface() -> ApiInterface:
    if PLATFORM == "linux":
        return LinuxApi()
    elif PLATFORM == "embedded":
        return EmbeddedApi()
    elif PLATFORM == "uart":
        return UartApi()
    else:
        raise NotImplementedError(f"Unsupported platform: {PLATFORM}")
