# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0103,  # CCM
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction": 0, # Encryption
        "algorithm": 8,  # SM4
        "key_handle" : 0x100001, # 固定1号 ram key
        "input_addr": "FFEEDDCCBBAA99887766554433221100",  # 输入消息为 0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
        "input_size": 16,  # 数据长度
        "output_addr": "00" * 16,  # output_addr为输出数据，这里仅填充16字节的0
        "output_size": 16,
        "aad_addr" : "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
        "aad_size" : 16,
        "tag_addr" : "00" * 16, # tag_addr为MAC输出数据，这里仅填充16字节的0
        "tag_size" : 16,
        "nonce_addr" : "FFFFFFFFFFFFFFFFFFFFFFFFFF",
        "nonce_size" : 13,
        "context": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "context_size": 0,
    },
    "check": {
        "output_addr": "33E4696C184B2EC1E70AF7BB09488C6E",
        "tag_addr": "12B4C62ECD213C3F6E9E06F9801A0880",
    },
}  # type: ignore