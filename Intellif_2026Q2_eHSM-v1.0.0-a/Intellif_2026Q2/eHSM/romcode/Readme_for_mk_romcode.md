mk_romcode_hex:
    Usage: ./mk_romcode_hex [xx.vhex] [-l (the storage space of instruction memory (unit: kByte))] [-k xx_-key.v] [-o output_name] [-64]

Example:
    ./mk_romcode_hex ehsm_fw.vhex -l 512 -k ../rtl/osr_define_key.v -o firmware

Note:
    1. The [-k xx_key.v] is optional, only used for ROM cipher.
    2. The [-o xxx] is indispensable, the output file name.
    3. The[-64] is optional, used to generate 64 bits romcode.
    4. The example of the hex length:
        (a) -l 512: 512kByte
        (b) -l 256: 256kByte
        (c) -l 128: 128kByte
        (d) -l 64 : 64kByte



mk_romcode_bin:
    Usage: ./mk_romcode_bin [xx.bin] [-l (the length of the "xx.bin" file)] [-k xx_key.v] [-o output_name] [-64]

Example:
    ./mk_romcode_bin ehsm_fw.bin -l 80000 -k ../rtl/osr_define_key.v -o firmware

Note:
    1. The [-k xx_key.v] is optional, only used for ROM cipher.
    2. The [-o xxx] is indispensable, the output file name.
    3. The[-64] is optional, used to generate 64 bits romcode.
    4. The example of the hex length:
        (a) -l 80000: 512kByte
        (b) -l 40000: 256kByte
        (c) -l 20000: 128kByte
        (d) -l 10000: 64kByte


