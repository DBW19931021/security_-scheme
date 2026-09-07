#!/usr/bin/env python3
# -*- coding:utf-8-*-
"""
command send and recv
"""

import sys
import struct
import logging as log

from utils.util import api

from utils.config import cfg_data

sys.path.append(str(cfg_data.CONFIG_TEST_ROOT_DIR) + "/stp_pro/py")
from platform_adapter.uart_lib.cmddef import CMD_MAX_COUNT

from stp_uart import STPUart

_stp = STPUart(True, cfg_data.CONFIG_TEST_UART_PORT, cfg_data.CONFIG_TEST_UART_BAUDRATE)

class CmdRspError(Exception):pass

RSP_OK              = 0
RSP_ERR_UNKNOWN_CMD = 1
RSP_ERR_DATA_LENGTH = 2
RSP_ERR_DATA_VALUE  = 3

@api
def send_cmd_recv_rsp(cmd_id: int, data: bytes = b"") -> tuple[int, bytes]:
    assert cmd_id < CMD_MAX_COUNT
    assert len(data) < 65536

    cmd = struct.pack("<HH", cmd_id, len(data)) + data
    _stp.send(cmd)

    rsp_head = _stp.recv(8)
    time, rsp_id, size = struct.unpack("<LHH", rsp_head)
    if size > 0:
        data = _stp.recv(size)
    else:
        data = b""

    if rsp_id != RSP_OK:
        raise CmdRspError(str(rsp_id))

    return (time, data)

@api
def uart_close():
    _stp.close()
