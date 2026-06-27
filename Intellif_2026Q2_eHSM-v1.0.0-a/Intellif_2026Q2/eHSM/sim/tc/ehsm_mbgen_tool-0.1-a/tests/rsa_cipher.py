# coding:utf-8
# 指针类型的数据需使用 HEX 格式的 str 表示，会被转换为 bytes，如果为None, 则表示NULL
{
    "cmd": {
        "cmd_id": 0x0501,  # RSA-Cipher
        "direction" : 0, # Geneation
        "key_handle" : 0x100001, # 固定1号 ram key
        "input_addr": "b88ec9ff1b00317a64967dc6a93cafec411212f4dbebd4ada94f59e5a52910f31049cd2b6de6fef195f2a68a6b87c361875cc1231f46036d0a1c834ee6850fdfb33e5d9af5ca07dc2d0eaa9b6462edf543583916df16351f8210bee8c1960342e27931e3f4ee73d06212c08bf451e26f47e8df7bf292d23c7a6a7b673d7889cb",  # 输入消息为 0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
        "input_size": 128,  # 数据长度
        "output_addr": "00" * 128,  # digest为输出数据，这里仅填充128字节的0
        "output_size": 128,
    },
    "check": {
        "output_addr": "1A664A99F9344BDD535B1748EDC1EAF5C21E6F44356E23F89C1104A16A7BE5C08E3CB6AD4FFEC9BEAE6489FB78B356AA2AD06C829D9114CDDC88D2503F18A8B97A70A982C16E9A286F925A1FED2B47D91E3CF00621EA2390A2ADC2CB1815A1AF198CFFF125E7F223371791B0761EF0311EE83F9ECF08F2A1A318E701B0F037E5",
    },
}  # type: ignore
