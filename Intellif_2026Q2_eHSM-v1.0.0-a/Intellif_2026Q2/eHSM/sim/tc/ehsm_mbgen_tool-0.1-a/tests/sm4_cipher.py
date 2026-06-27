# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0101,  # SKE
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction": 0, # Encryption
        "padding": 0, # No padding
        "algorithm": 8,  # SM4
        "cipher_mode" : 1, # ECB mode
        "key_handle" : 0x100001, # 固定1号 ram key
        "input_addr": "FFEEDDCCBBAA99887766554433221100",  # 输入消息为 0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
        "input_size": 16,  # 数据长度
        "output_addr": "00" * 16,  # output_addr为输出数据，这里仅填充16字节的0
        "output_size": 16,
        "iv_addr" : None,
        "iv_size" : 0,
        "context": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "context_size": 0,
    },
    "check": {
        "output_addr": "CE558C7F9E4BF964A3099FCDE3A12FC4",
    },
}  # type: ignore
