#!/usr/bin/env python3
# -*- coding:utf-8  -*-
"""
STP协议客户端测试程序
"""

import sys
import argparse
import time
from pathlib import Path

# sys.path.append(str(Path(__file__).parent.parent / "py"))

import random
import logging as log
from stp_uart import STPUart


def string_output_handler(data: bytes):
    """字符串输出回调函数"""
    text = data.decode('utf-8', 'replace')
    print(f"[STRING] {text}")


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
    """主测试函数"""
    i = 0
    log.info("STP客户端测试开始")
    
    try:
        # 使用新的API：is_client=True 表示客户端，增加字符串输出回调
        stp = STPUart(is_client=True, port=args.port, baudrate=args.baudrate, string_callback=string_output_handler)
        
        # 发送一些测试字符串帧
        stp.send_string(b"Client started")
        stp.send_string(f"Testing {args.count} rounds".encode('utf-8'))
        
        for _ in range(args.count):
            data_len = random.randint(1, 1024)
            data = random.randbytes(data_len)
            # data = bytes([i for i in range(256)])

            print(f"data len: {data_len}")
            if args.verbose:
                print(f"data: {data.hex()}")
            
            stp.send_string(b"%d start" % (i + 1))
            time.sleep(0.01)  # 等待对方打印完成
            rand_send(stp, data_len.to_bytes(2, "big"))
            rand_send(stp, data)
            ret = rand_recv(stp, data_len)
            
            if ret != data:
                print(
                    "---recv data error!!!!!!!!--- \nsend: %s\nrecv: %s"
                    % (data.hex(), ret.hex())
                )
                sys.exit(1)
            else:
                print("data ok %d" % (i + 1))
            i += 1
        
        stp.close()
        log.info("STP客户端测试完成")
        
    except KeyboardInterrupt:
        log.info("测试被用户中断")
        if 'stp' in locals():
            stp.close()
        sys.exit(130)
    except Exception as e:
        log.error(f"测试异常: {e}")
        if 'stp' in locals():
            stp.close()
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="STP协议客户端测试程序")
    parser.add_argument("--port", "-p", default="/dev/ttyUSB0", help="串口号 (默认: /dev/ttyUSB0)")
    parser.add_argument("--baudrate", "-b", type=int, default=115200, help="波特率 (默认: 115200)")
    parser.add_argument("--verbose", "-v", action="store_true", help="详细日志输出")
    parser.add_argument("--rounds", "-r", type=int, default=3, help="测试轮数 (默认: 3)")
    parser.add_argument("--count", "-c", type=int, default=100, help="每轮测试次数 (默认: 100)")
    
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
    
    log.info(f"客户端配置: 串口={args.port}, 波特率={args.baudrate}, 轮数={args.rounds}, 每轮次数={args.count}")
    
    for i in range(args.rounds):
        print(f"----------- round {i+1}/{args.rounds} ----------------")
        main(args)
