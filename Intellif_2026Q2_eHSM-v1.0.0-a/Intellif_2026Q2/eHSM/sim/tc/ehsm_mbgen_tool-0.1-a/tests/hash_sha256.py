# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0301,  # HASH
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "algorithm": 2,  # SHA256
        "data": "1122334455",  # 输入消息为 0x11, 0x22, 0x33, 0x44, 0x55
        "data_size": 5,  # 数据长度
        "digest": "00" * 32,  # digest为输出数据，这里仅填充32字节的0
        "digest_size": 32,
        "hash_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "hash_ctx_size": 0,
    },
    "check": {
        "digest": "b9ea0a42b00fed95e53c20d121a9d3769cb993beccb2eb2184f97ff9e0f818d8",
    },
}  # type: ignore
