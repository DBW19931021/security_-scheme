# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0302,  # HMAC
        "direction" : 0, # Geneation
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "algorithm": 0,  # SM3
        "key_handle" : 0x100001, # 固定1号 ram key
        "data": "FFEEDDCCBBAA99887766554433221100",  # 输入消息为 0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
        "data_size": 16,  # 数据长度
        "digest": "00" * 32,  # digest为输出数据，这里仅填充32字节的0
        "digest_size": 32,
        "hmac_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "hmac_ctx_size": 0,
    },
    "check": {
        "digest": "DF1062B4A11AA93C777493B95147C128E6853F99C152F23BD2AFA775672C2840",
    },
}  # type: ignore
