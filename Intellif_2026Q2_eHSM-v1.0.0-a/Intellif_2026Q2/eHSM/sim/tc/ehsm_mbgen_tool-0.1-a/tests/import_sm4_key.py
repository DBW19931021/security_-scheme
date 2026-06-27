# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0801,  # import key
        "key_handle": 0x100001,  # 指定 key handle
        "transport_key_handle": 0xFFFFFFFF,  # 明文导入
        "authenticity_key_handle": 0xFFFFFFFF,  # 不带MAC
        "key_data_addr": "0F300000 07 02 0000 0000 1000 00112233445566778899aabbccddeeff",  # 权限, algo_id, part_info, reserved, pub_len, pri_len, key_value
        "key_data_size": 12 + 16,  # 数据长度
        "key_signature_addr": None,  # 无MAC值
        "key_signature_size": 0,
    },
    "check": {
    },
}  # type: ignore
