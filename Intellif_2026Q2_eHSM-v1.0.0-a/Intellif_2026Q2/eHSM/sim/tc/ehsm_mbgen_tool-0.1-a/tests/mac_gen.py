# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0201,  # MAC
        "direction" : 0, # Geneation
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "algorithm": 5,  # AES 128
        "cipher_mode" : 8, # CBC MAC
        "key_handle" : 0x100001, # 固定1号 ram key
        "data": "FFEEDDCCBBAA99887766554433221100",  # 输入消息为 0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
        "data_size": 16,  # 数据长度
        "mac": "00" * 16,  # mac为输出数据，这里仅填充16字节的0
        "mac_size": 16,
        "iv_addr" : None, # IV 用于GMAC
        "mac_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "mac_ctx_size": 0,
    },
    "check": {
        "digest": "7221393918A24D7D0EE3F2223BFA5B35",
    },
}  # type: ignore
