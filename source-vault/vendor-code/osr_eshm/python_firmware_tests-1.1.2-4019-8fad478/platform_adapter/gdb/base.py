from abc import ABC, abstractmethod
from typing import Tuple

class GdbInterface(ABC):
    @abstractmethod
    def load_elf(self, path: str)->Tuple[int, str]:
        pass

    @abstractmethod
    def send_command(self, cmd: str) -> str:
        pass

    @abstractmethod
    def stop_session(self) -> None:
        pass
