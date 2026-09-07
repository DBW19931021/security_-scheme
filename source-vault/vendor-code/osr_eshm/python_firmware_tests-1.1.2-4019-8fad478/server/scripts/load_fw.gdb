target extended-remote 127.0.0.1:3333


# set memmap backup register
set {int[2]}0x30003800 = {0, 0}

# store Params + FW data to 0x60041000
restore params_fw_in_soc_0x60041000.bin binary 0xe0041000

restore C:\Users\osr\AppData\Local\Programs\Python\Python312\Lib\site-packages\pyfastbl\fast_bl.bin binary 0x10000000

