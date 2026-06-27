# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0502,  # RSA-sign
        "process_mode": 7,  # ONEPASS, TB测试不能测三段式
        "direction" : 1, # Generation
        "rsa_padding_type" : 0, #No padding
        "algorithm": 5,  # SHA1
        "key_handle" : 0x100001, # 固定1号 ram key
        "msg_addr": "0123456789",
        "msg_size" : 5, 
        "sign_addr": "132E66C8445B209BB91644B96E8937292D606BFCE4EAEF60062936F7F0FEAD315048679E2C4591C8E8E16BE7979C0F6BE644D9EF923B444225C48BAE6E209F7FDBF82C76067B55480A5385FB01195275B69F9F16146A2459BF08B7345BF25B7C60E80D138FDA520D0AC07F46858C935E0557AE4F86028099775DA0DAFF9210E3",
        "sign_size": 128,
        "salt_size": 0,
        "sign_ctx": None,  # ONEPASS 无需 ctx buffer，设置为NULL值
        "sign_ctx_size": 0,
    },
    "check": {
    },
}  # type: ignore
