# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0801,  # import key
        "key_handle": 0x100001,  # 指定 key handle
        "transport_key_handle": 0xFFFFFFFF,  # 明文导入
        "authenticity_key_handle": 0xFFFFFFFF,  # 不带MAC
        "key_data_addr": "0f300000 08 03 0000 4100 2000 0464f59b1f005c0040308522e9b498c515e691fa54c1989dfe859efcab6a38151b743dd70bedc3d8438fb1ab2a72844f912e888ecb10449d6749b9ab7102ad15ca2ec176a8f19b6cdfaee8f7c77eeacc1f136bb744b4a531d67efe52f0bb528deb",  # 权限, algo_id, part_info, reserved, pub_len, pri_len, key_value
        "key_data_size": 12 + 97,  # 数据长度
        "key_signature_addr": None,  # 无MAC值
        "key_signature_size": 0,
    },
    "check": {
    },
}  # type: ignore
