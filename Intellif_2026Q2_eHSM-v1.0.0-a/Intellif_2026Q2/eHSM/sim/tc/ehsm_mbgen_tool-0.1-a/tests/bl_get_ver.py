# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0xFF10,  # bl_read_ver
        "ver_addr": "00" * 128, # buffer
        "ver_size": 128,  #  size
    },
    "check": {
        "ver_addr": "000200", # type: 0(bootloader), major_ver: 2, minor_ver: 0
    },
}  # type: ignore
