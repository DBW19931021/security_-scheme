# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0a01,  # RNG
        "algorithm": 0,  # SM4 CTRDRBG
        "random_data_addr": "00" * 32,  # random_data_addr为输出数据，这里仅填充32字节的0
        "require_size": 32,
    },
    "check": {
        # 如TB需要固定RNG的输出，请修改下述random_data_addr的数据为需要binding的数据
        "random_data_addr" : "00" * 32,
    },
}  # type: ignore
