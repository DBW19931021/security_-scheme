#!/usr/bin/env python3
# -*- coding:utf-8  -*-

import logging as log
import os
from pathlib import Path

_cfg_dict: dict[str, str|int|bool] = {}

_cfg_dict["CONFIG_TEST_ROOT_DIR"] = str(Path(__file__).parent.parent)

for fname in os.environ.get("EHSM_CONFIG_FILES", "").split(":"):
    if fname:
        content = (Path(_cfg_dict["CONFIG_TEST_ROOT_DIR"]) / "config" / fname).read_text('utf-8')
        exec(content, _cfg_dict)

if '__builtins__' in _cfg_dict:
    del _cfg_dict['__builtins__']

for k, v in _cfg_dict.items():
    log.debug("%s = %s", k, str(v))


class _ConfigData:
    def __init__(self, cfg_info: dict[str, str|int|bool]):
        self.__cfg_info = cfg_info

    def __getattr__(self, name: str) -> str|int|bool|None:
        if name not in self.__cfg_info:
            raise KeyError(f"Cannot find config {name}")
        return self.__cfg_info[name]


cfg_data = _ConfigData(_cfg_dict)

del _cfg_dict


__all__ = [
    "cfg_data"
]
