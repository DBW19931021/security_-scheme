# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0601,  # ECDSA
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction" : 0, # Verify
        "algorithm": 2,  # SHA256
        "key_handle" : 0x100001, # 固定1号 ram key
        "msg_addr": "0123456789",
        "msg_size" : 5, 
        "sign_addr": "15F66F50A24FD6A7385057C0969EAA89181F9CBBBCA1CC243EF70B3CA54A160E10F31694AC4B8FB9657833AF7C025A5E",
        "sign_size": 48,
        "sign_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "sign_ctx_size": 0,
    },
    "check": {
    },
}  # type: ignore
