# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0401,  # SM2-cipher
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction": 1, # Decryption
        "key_handle" : 0x100001, # 固定1号 ram key
        "input_addr" : "047492835DA00E8D53D3EF739D021F9F87B289DE754FD0DA98EA3EB9007249A3F9412E020154329B3AD3EEE4615FA32A83F917A9C5EDF566650ABF408A835C17B485361060466BEBA8DF39AEE72E1AAE31B48F30D3632E0AC245741EA015A6964BB771130AAB3CDA8535F715753928B4F0BC59FA29D99D3EBFECC08669C67DB7",
        "input_size": 128,  # 数据长度
        "output_addr": "00" * 128,  # output_addr为输出数据buffer，这里仅填充31字节的0
        "output_size": 128,
    },
    "check": {
        "output_addr": "399f9bf0004fdced749f1b6545ec025a801fcf0632709cac73a760ed2ae46a",
    },
}  # type: ignore
