# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0801,  # import key
        "key_handle": 0x100001,  # 指定 key handle
        "transport_key_handle": 0xFFFFFFFF,  # 明文导入
        "authenticity_key_handle": 0xFFFFFFFF,  # 不带MAC
        "key_data_addr": "03300000 19 03 0000 3000 1800 28E3C76B633DDD3CEC9D30485F9BA51FCCD901C4CC56C8E836BA00E7DEA0910A9BF47042F7CD84E873ABE68B356EC6897A46F11B33B0C7E6CF32AC081170E168C0A125ED282D7FCB",  # 权限, algo_id, part_info, reserved, pub_len, pri_len, key_value, 注意：公钥不传压缩标记位
        "key_data_size": 12 + 97,  # 数据长度
        "key_signature_addr": None,  # 无MAC值
        "key_signature_size": 0,
    },
    "check": {
    },
}  # type: ignore
