# -*- coding:utf-8-*-
"""

"""

import functools
import inspect
import logging as log
from typing import Any, Callable


def _reprval(v: Any):
    if v is None:
        ret = "None"
    elif isinstance(v, (bytes, bytearray)):
        ret = f"{type(v).__name__}({len(v):d}:{v.hex()})"
    elif isinstance(v, bool):
        ret = f"{str(v)}"
    elif isinstance(v, int):
        ret = f"{type(v).__name__}(0x{v:x}/{v:d})"
    elif isinstance(v, (list, tuple)):
        ret = f"{type(v).__name__}[" + ", ".join(_reprval(i) for i in v) + "]"
    elif isinstance(v, dict):
        ret = (
            f"{type(v).__name__}"
            + "{"
            + ", ".join(f"{_reprval(k)} : {_reprval(dv)}" for k, dv in v.items())
            + "}"
        )
    elif isinstance(v, str):
        ret = f'{type(v).__name__}("{v}")'
    else:
        ret = f"{type(v).__name__}({str(v)})"
    return ret


def api(f):
    """wrapper for API functions"""

    @functools.wraps(f)
    def wrapper(*args, **kwargs):
        argdict = inspect.getcallargs(f, *args, **kwargs)
        arglist = inspect.getfullargspec(f)[0]

        l: list[str] = []

        for arg in arglist:
            argvalue = _reprval(argdict[arg])
            l.append(f"{arg}={argvalue}")

        fname = f.__module__.split(".")[-1] + "." + f.__name__
        log.debug(f"API call: {fname}({', '.join(l)}) ...")

        ret = f(*args, **kwargs)

        retmsg = _reprval(ret)
        log.debug(f"API return: {fname}(...) -> {retmsg}")

        return ret

    return wrapper
