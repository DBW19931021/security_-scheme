# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0402,  # SM2-sign
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction" : 0, # Verify
        "algorithm": 5,  # SHA1
        "key_handle" : 0x100001, # 固定1号 ram key
        "msg_addr": "0123456789",
        "msg_size" : 5, 
        "sign_addr": "FA56E684F2219F4C97B826CDAEA12DDF10481D4CC76C18591E554EDDD764CE9ECABD0F44B79BC7E74AF5C894A5A175AE142FA3C87FDD35562312293FDB8C5CA6",
        "sign_size": 64,
        "sign_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "sign_ctx_size": 0,
    },
    "check": {
    },
}  # type: ignore
