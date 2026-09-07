#!/usr/bin/env python3
# -*- coding:utf-8-*-
"""
STP协议服务端测试程序（回显服务器）
"""

import sys
import argparse
import time
from pathlib import Path

# sys.path.append(str(Path(__file__).parent.parent / "py"))

import logging as log
import random

from stp_uart import STPUart, STPResync, STPTimeoutError, STPError


def string_output_handler(data: bytes):
    """字符串输出回调函数"""
    try:
        text = data.decode('utf-8')
        print(f"[STRING] {text}")
    except UnicodeDecodeError:
        print(f"[STRING] {data.hex()}")


def rand_recv(port: STPUart, size: int) -> bytes:
    print(f"total recv {size} data");
    ret = bytearray(size)
    recv_size = 0
    while recv_size < size:
        rand_size = random.randint(1, size - recv_size)
        print(f">>> prepare to recv {rand_size} bytes")
        data = port.recv(rand_size)
        ret[recv_size : recv_size + len(data)] = data
        recv_size += len(data)
        print(">>>recv: ", len(data), "left: ", size - recv_size)

    return bytes(ret)


def rand_send(port: STPUart, data: bytes) -> None:
    send_size = 0
    size = len(data)
    print(f"total send {len(data)} data")
    while send_size != size:
        send_len = random.randint(1, size - send_size)
        port.send(data[send_size : send_size + send_len])
        send_size += send_len
        print(f">>>send {send_len}, left: {size - send_size}")

    print(">>> send over...")


def main(args):
    """主服务器函数"""
    log.info("STP服务端启动")
    
    try:
        # 使用新的API：is_client=False 表示服务端，增加字符串输出回调
        stp = STPUart(is_client=False, port=args.port, baudrate=args.baudrate, string_callback=string_output_handler)
        log.info(f"服务端监听中: {args.port} @ {args.baudrate}")
        
        # 发送服务端启动消息
        stp.send_string(b"Server ready")
        
        idx = 0
        while True:
            idx += 1
            try:
                # 接收数据长度（2字节）
                ret = rand_recv(stp, 2)
                data_len = int.from_bytes(ret, "big")
                print(f"data_len: {data_len}")
                
                # 接收实际数据
                data = rand_recv(stp, data_len)
                if args.verbose:
                    print(f"data: {data.hex()}")
                else:
                    print(f"received {data_len} bytes")
                print("-- recv data size %d --" % len(data))

                # 回显数据
                rand_send(stp, data)
                print("echo completed")
                stp.send_string(b"%d done" % idx)
                time.sleep(0.01) # 等待对端打印
                
            except STPResync:
                log.debug("收到客户端RESYNC请求")
                continue
            except STPTimeoutError:
                log.debug("通信超时，继续等待")
                continue
            except STPError as e:
                log.debug(f"STPError，{str(e)}")
                continue
                
    except KeyboardInterrupt:
        log.info("服务端被用户中断")
        if 'stp' in locals():
            stp.close()
        sys.exit(0)
    except Exception as e:
        import traceback
        traceback.print_exc()
        log.error(f"服务端异常: {e}")
        if 'stp' in locals():
            stp.close()
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="STP协议服务端测试程序（回显服务器）")
    parser.add_argument("--port", "-p", default="/dev/ttyACM0", help="串口号 (默认: /dev/ttyACM0)")
    parser.add_argument("--baudrate", "-b", type=int, default=115200, help="波特率 (默认: 115200)")
    parser.add_argument("--verbose", "-v", action="store_true", help="详细日志输出")
    
    args = parser.parse_args()
    
    # 设置日志级别
    if args.verbose:
        log.basicConfig(
            format="[%(levelname)s:%(funcName)s:%(lineno)d] %(message)s", 
            level=log.DEBUG
        )
    else:
        log.basicConfig(
            format="[%(levelname)s:%(funcName)s:%(lineno)d] %(message)s", 
            level=log.INFO
        )
    
    log.info(f"服务端配置: 串口={args.port}, 波特率={args.baudrate}")
    main(args)
