#!/usr/bin/env python3
# -*- coding:utf-8  -*-
""" """

import logging as log
from typing import Optional
from platform_adapter.api.constants import EhsmAeadMode, EhsmAuthAlgo, EhsmBlGenKeyType, EhsmChallengeType, EhsmCtrlField, EhsmKeyLevel, EhsmLifecycle, EhsmSymmAlgo
from platform_adapter.uart_lib import soccmd, cmddef
import struct
from utils.util import api
from utils.config import cfg_data
# Reason: host.loader 和 uart_impl 的导入已移至 get_version() 内部懒加载，
# 避免 uart_impl → hostapi → host.loader 的循环导入

IRAM_BASE = 0x0000_0000
IRAM_SIZE = 0x10_0000

DRAM_BASE = 0x2000_0000
DRAM_SIZE = 0x10_0000

SYS_REG_BASE = 0x4001_0000
SYS_REG_SIZE = 0x2_0000
if cfg_data.TEST_CUSTOM_ID == 0x4019:
    SHARE_RAM_BASE = 0x6001_9000
else:
    SHARE_RAM_BASE = 0x6000_0000
SHARE_RAM_SIZE = 0x4_0000
if cfg_data.TEST_HW_PLATFORM_NEW_SUPPORT == 1:
    OTP_BASE = 0x6007_C000
else:
    OTP_BASE = 0x6004_0000
HSM_STATUS_IN = 0x4001_0060
HSM_STATUS_IN1 = 0x4001_0064
HSM_ERR_SENSOR_IN = 0x4001_0068
HSM_ERR_HW0 = 0x4001_006C
HSM_ERR_HW1 = 0x4001_0070
HSM_ERR_FW0 = 0x4001_0074
HSM_ERR_FW1 = 0x4001_0078
HSM_ALARM_TRIG0 = 0x4001_0080
HSM_ALARM_TRIG1 = 0x4001_0084
SOC_DBG_EN_128B0 = 0x4001_0088
SOC_DBG_EN_128B1 =  0x4001_008C
SOC_DBG_EN_128B2 = 0x4001_0090
SOC_DBG_EN_128B3 = 0x4001_0094

HW_DONE = 1 << 0
HW_ERR = 1 << 1
BL_DONE = 1 << 2
BL_ERR = 1 << 3
FW_DONE = 1 << 4
FW_ERR = 1 << 5
HSM_DBG_EN = 1 << 16
SOC_DBG_EN = 1 << 17
CPU_WFI = 1 << 25
WDG_TIMEOUT = 1 << 5 # 37bit
PATCH_LOAD_FAILED = 1 << 10 # bit 10 in HSM_ERR_FW1, patch load failed

CTX_ADDR = SHARE_RAM_BASE
CTX_SIZE = 1024

DATA1_ADDR = SHARE_RAM_BASE + 1024 * 1
DATA2_ADDR = SHARE_RAM_BASE + 1024 * 2
DATA3_ADDR = SHARE_RAM_BASE + 1024 * 3
DATA4_ADDR = SHARE_RAM_BASE + 1024 * 4
DATA5_ADDR = SHARE_RAM_BASE + 1024 * 5
DATA6_ADDR = SHARE_RAM_BASE + 1024 * 6
DATA7_ADDR = SHARE_RAM_BASE + 1024 * 7
DATA8_ADDR = SHARE_RAM_BASE + 1024 * 8
DATA9_ADDR = SHARE_RAM_BASE + 1024 * 9

SW_TYPE_BL = 0
SW_TYPE_FW = 1

EHSM_OK = 0

EHSM_DRV_MODE_INTERRUPT = 0  # 底层Mailbox使用中断收取数据，能够支持真正的同步和异步 */
EHSM_DRV_MODE_WAIT_AND_POLL = (
    1  # 底层Mailbox使用轮询收取数据，异步接口可以正常使用，但本质上是同步模式 */
)

HASH_ALGOS = {
    "SM3": 0,
    "MD5": 1,
    "SHA256": 2,
    "SHA384": 3,
    "SHA512": 4,
    "SHA1": 5,
    "SHA224": 6,
    "SHA512_224": 7,
    "SHA512_256": 8,
    "SHA3_224": 9,
    "SHA3_256": 10,
    "SHA3_384": 11,
    "SHA3_512": 12,
}

SKE_ALGOS = {
    "DES": 0,
    "TDES_128": 1,
    "TDES_192": 2,
    "AES_128": 5,
    "AES_192": 6,
    "AES_256": 7,
    "SM4": 8,
}

SKE_MODES = {
    "ECB": 1,
    "XTS": 2,
    "CBC": 3,
    "CFB": 4,
    "OFB": 5,
    "CTR": 6,
}

MAC_MODES = {
    "CMAC": 7,
    "CBC_MAC": 8,
    "GMAC": 9,
}

KEY_TYPES = {
    "DES": 0x01,
    "TDES_128": 0x02,
    "TDES_192": 0x03,
    "AES_128": 0x04,
    "AES_192": 0x05,
    "AES_256": 0x06,
    "SM4": 0x07,
    "SM2": 0x08,
    "RSA_1024": 0x09,
    "RSA_2048": 0x0A,
    "RSA_3072": 0x0B,
    "RSA_4096": 0x0C,
    "RSA_1024_CRT": 0x0D,
    "RSA_2048_CRT": 0x0E,
    "RSA_3072_CRT": 0x0F,
    "RSA_4096_CRT": 0x10,
    "DH": 0x11,
    "ECC_BRAINPOOLP_160R1": 0x12,
    "ECC_BRAINPOOLP_192R1": 0x13,
    "ECC_BRAINPOOLP_224R1": 0x14,
    "ECC_BRAINPOOLP_256R1": 0x15,
    "ECC_BRAINPOOLP_320R1": 0x16,
    "ECC_BRAINPOOLP_384R1": 0x17,
    "ECC_BRAINPOOLP_512R1": 0x18,
    "ECC_SECP_192R1": 0x19,
    "ECC_SECP_224R1": 0x1A,
    "ECC_SECP_256R1": 0x1B,
    "ECC_SECP_384R1": 0x1C,
    "ECC_SECP_521R1": 0x1D,
    "ED25519": 0x1E,
    "X25519": 0x1F,
    "SM4_XTS": 0x20,
    "AES_128_XTS": 0x21,
    "AES_192_XTS": 0x22,
    "AES_256_XTS": 0x23,
    "CHACHA": 0x24,
    "HMAC": 0x25,
    "ECC_SECP_160K1": 0x26,
    "ECC_SECP_192K1": 0x27,
    "ECC_SECP_224K1": 0x28,
    "ECC_SECP_256K1": 0x29,
    "SM9_ENC_USERPRIV": 0x2A,
    "SM9_SIGN_USERPRIV": 0x2B,
    "SM9_EXCHG_USERPRIV": 0x2C,
    "SM9_EXCHG_TEMP": 0x2D,
    "ECC_SECP_160R1": 0x2E,
    "ECC_SECP_160R2": 0x2F,
}

class HostApiError(Exception):
    def __init__(self, ret_code: int):
        self.ret_code = ret_code
        super().__init__(str(self.ret_code))

def _check_ret(ret_data: bytes):
    assert len(ret_data) >= 4
    ret_code = int.from_bytes(ret_data[:4], "little")
    if ret_code != 0:
        log.debug(f"error code: %d", ret_code - 65536)
        ret_code = ret_code - 65536
        raise HostApiError(ret_code)

@api
def init_library(drv_mode: int) -> None:
    _, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_INIT_LIB, drv_mode.to_bytes(1, "little")
    )
    assert len(data) == 4
    _check_ret(data)

@api
def init_ctx(ctx_addr: int, mb_chl: int, is_async: bool):
    soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_INIT_CTX,
        struct.pack("<LBB", ctx_addr, mb_chl, int(is_async)),
    )

@api
def deinit_ctx():
     soccmd.uart_close()

@api
def get_version(ctx_addr: int) -> tuple[int, int, int, int, str]:
    _, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_GET_VERSION, struct.pack("<LL", ctx_addr, DATA1_ADDR)
    )
    _check_ret(data)
    # Reason: 懒加载避免模块级循环导入（uart_impl → hostapi → host.loader）
    from platform_adapter.host.loader import get_host_interface
    host = get_host_interface()
    data = host.read_memory(DATA1_ADDR, 12)
    assert len(data) == 12

    fw_type, major, minor, patch = struct.unpack("<BBBB", data[:4])
    pre_rel = data[4:].decode("utf-8").rstrip("\x00")

    return (fw_type, major, minor, patch, pre_rel)

@api
def ehsm_bl_debug_auth(
    ctx_addr: int,
    type: int,
    algo: int,
    sig: int,
    sig_size: int,
    pubkey: int,
    pubkey_szie: int,
    bitmaps: int,
) -> None:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_DEBUG_AUTH,
        struct.pack(
            "<LLLLLLLL",
            ctx_addr,
            type,
            algo,
            sig,
            sig_size,
            pubkey,
            pubkey_szie,
            bitmaps
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_sign_onepass_gen(
    ctx: int,
    hash_algo: int,
    key_handle: int,
    padding: int,
    msg: int,
    msg_size: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_ONEPASS_GEN,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            hash_algo,
            key_handle,
            padding,
            msg,
            msg_size,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_rsa_sign_onepass_verify(
    ctx: int,
    hash_algo: int,
    key_handle: int,
    padding: int,
    msg: int,
    msg_size: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_ONEPASS_VERIFY,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            hash_algo,
            key_handle,
            padding,
            msg,
            msg_size,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_rsa_sign_onepass_gen_with_digest(
    ctx: int,
    algo: int,
    key_handle: int,
    padding: int,
    digest: int,
    digest_size: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_ONEPASS_GEN_WITH_DIGEST,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            algo,
            key_handle,
            padding,
            digest,
            digest_size,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_rsa_sign_onepass_verify_with_digest(
    ctx: int,
    algo: int,
    key_handle: int,
    padding: int,
    digest: int,
    digest_size: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_ONEPASS_VERIFY_WITH_DIGEST,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            algo,
            key_handle,
            padding,
            digest,
            digest_size,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_rsa_sign_onepass_ex(
    ctx: int,
    algo: int,
    use_plain_key: bool,
    key_handle: int,
    key: int,
    padding: int,
    gen_sig: bool,
    is_digest: bool,
    input_addr: int,
    input_size: int,
    sig_addr: int,
    sig_size: int,
    salt_size: int,
    verify_result_addr: int
) -> tuple[int, int, int]:
    """
    RSA签名/验签统一接口 (ex版本)

    Reason: 统一接口支持密钥句柄/明文密钥、签名/验签、原始消息/摘要的所有组合
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_ONEPASS_EX,
        struct.pack(
            "<LLLLLLLLLLLLLL",
            ctx,
            algo,
            int(use_plain_key),
            key_handle,
            key,
            padding,
            int(gen_sig),
            int(is_digest),
            input_addr,
            input_size,
            sig_addr,
            sig_size,
            salt_size,
            verify_result_addr
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little"), int.from_bytes(data[8:12], "little")

@api
def ehsm_bl_verify_image(
    ctx: int,
    image_addr: int,
    image_size: int,
    check_version: bool,
    boot_after_verify: bool,
    image_out: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_VERIFY_IMAGE,
        struct.pack(
            "<LLLLLL",
            ctx,
            image_addr,
            image_size,
            int(check_version),
            int(boot_after_verify),
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_upgrade_fw_image(
    ctx: int,
    image_addr: int,
    image_size: int,
    image_out: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_UPGRADE_FW_IMAGE,
        struct.pack(
            "<LLLL",
            ctx,
            image_addr,
            image_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_cipher(
    ctx: int,
    key_handle: int,
    enc: bool,
    input_addr: int,
    input_size: int,
    output_addr: int,
    output_size: int,
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_CIPHER,
        struct.pack(
            "<LLLLLLL",
            ctx,
            key_handle,
            int(enc),
            input_addr,
            input_size,
            output_addr,
            output_size,
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_bl_close_debug(
     ctx:int,
     type: int,
     soc_dbg_bitmap: int
     ) -> int:

    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_CLOSE_DEBUG,
        struct.pack(
            "<LLL",
            ctx,
            type,
            soc_dbg_bitmap
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_get_challenge(ctx: int, challenge_type: int , out_addr: int) -> tuple[int,int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_GET_CHALLENGE,
        struct.pack(
            "<LLL",
            ctx,
            challenge_type,
            out_addr
        )
    )
    _check_ret(data)
    return t ,48

@api
def ehsm_bl_get_version(ctx: int, out_addr: int) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_GET_VERSION,
        struct.pack(
            "<LL",
            ctx,
            out_addr
        )
    )
    _check_ret(data)
    return t, 128

@api
def ehsm_bl_inject_error(ctx: int, values: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_INJECT_ERROR,
        struct.pack(
            "<LL",
            ctx,
            values
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_read_otp(ctx: int, buf_addr: int, ehsm_src_addr: int, size: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_READ_OTP,
        struct.pack(
            "<LLLL",
            ctx,
            buf_addr,
            ehsm_src_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_read_reg(ctx: int, buf_addr: int, ehsm_src_addr: int, size: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_READ_REG,
        struct.pack(
            "<LLLL",
            ctx,
            buf_addr,
            ehsm_src_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_set_uart_baudrate(ctx: int, baud_div: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_SET_UART_BAUDRATE,
        struct.pack(
            "<LL",
            ctx,
            baud_div
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_set_hsm_freq(ctx: int, hsm_freq: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_SET_HSM_FREQ,
        struct.pack(
            "<LL",
            ctx,
            hsm_freq
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_upgrade_fw_image(ctx: int, image: int, image_size: int, image_out: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_UPGRADE_FW_IMAGE,
        struct.pack(
            "<LLLL",
            ctx,
            image,
            image_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_verify_image(
        ctx: int,
        image: int,
        image_size: int,
        check_version: bool,
        boot: bool,
        image_out: int
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_VERIFY_IMAGE,
        struct.pack(
            "<LLLLLL",
            ctx,
            image,
            image_size,
            int(check_version),
            int(boot),
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_write_otp(ctx: int,  data_buf: int, ehsm_dest_addr: int, size: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_WRITE_OTP,
        struct.pack(
            "<LLLL",
            ctx,
            data_buf,
            ehsm_dest_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_write_reg(ctx, data_buf: int, ehsm_dest_addr: int, size: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_WRITE_REG,
        struct.pack(
            "<LLLL",
            ctx,
            data_buf,
            ehsm_dest_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_encrypt_key(
    ctx: int,
    key_level: int,
    input_data: int,
    size: int,
    key_out: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_ENCRYPT_KEY,
        struct.pack(
            "<LLLLL",
            ctx,
            key_level,
            input_data,
            size,
            key_out
        ),
    )
    _check_ret(data)
    return t, 36

@api
def ehsm_bl_fw_auth(
     ctx: int,
     type: int,
     arg: int,
     auth_data: int,
     out_addr: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_FW_AUTH,
        struct.pack(
            "<LLLLL",
            ctx,
            type,
            arg,
            auth_data,
            out_addr
        )
    )
    _check_ret(data)
    return t, 720

@api
def ehsm_bl_get_random_key(
     ctx: int,
     key_level: int,
     key_type: int,
     key_out: int
    ) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_GET_RANDOM_KEY,
        struct.pack(
            "<LLLL",
            ctx,
            key_level,
            key_type,
            key_out
        ),
    )
    _check_ret(data)
    return t, 36

@api
def ehsm_bl_get_self_test_result(ctx: int, result: int) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_GET_SELF_TEST_RESULT,
        struct.pack(
            "<LL",
            ctx,
            result
        ),
    )
    _check_ret(data)
    return t, 8

@api
def ehsm_bl_get_socid(ctx: int, socid: int) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_GET_SOCID,
        struct.pack(
            "<LL",
            ctx,
            socid
        ),
    )
    _check_ret(data)
    return t, 16

@api
def ehsm_bl_self_test(ctx: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_SELF_TEST,
        struct.pack(
            "<L",
            ctx
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_onepass_enc(
        ctx: int,
        algo: int,
        mode: int,
        key_handle: int,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        input: int,
        input_size: int,
        output: int,
        tag: int,
        tag_size: int
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_ONEPASS_ENC,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_handle,
            nonce,
            nonce_size,
            aad,
            aad_size,
            input,
            input_size,
            output,
            tag,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_onepass_dec(
        ctx: int,
        algo: int,
        mode: int,
        key_handle: int,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        input: int,
        input_size: int,
        output: int,
        tag: int,
        tag_size: int
    ) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_ONEPASS_DEC,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_handle,
            nonce,
            nonce_size,
            aad,
            aad_size,
            input,
            input_size,
            output,
            tag,
            tag_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_aead_init(
        ctx: int,
        algo: int,
        mode: int,
        key_handle: int,
        enc: bool,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        data_size: int,
        tag_size: int,
        session: int
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_INIT,
        struct.pack(
            "<LLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_handle,
            int(enc),
            nonce,
            nonce_size,
            aad,
            aad_size,
            data_size,
            tag_size,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_update(
     ctx: int,
     input: int,
     input_size: int,
     output: int
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_UPDATE,
        struct.pack(
            "<LLLL",
            ctx,
            input,
            input_size,
            output
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_finish_enc(
     ctx: int,
    input: int,
    input_size: int,
    output: int,
    tag: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_FINISH_ENC,
        struct.pack(
            "<LLLLL",
            ctx,
            input,
            input_size,
            output,
            tag
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_finish_dec(
     ctx: int,
     input: int,
     input_size: int,
     output: int,
     tag: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_FINISH_DEC,
        struct.pack(
            "<LLLLL",
            ctx,
            input,
            input_size,
            output,
            tag
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_aead_onepass_enc_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    input_addr: int,
    input_size: int,
    output_addr: int,
    tag_addr: int,
    tag_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_ONEPASS_ENC_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_addr,
            key_size,
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            output_addr,
            tag_addr,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_aead_onepass_dec_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    input_addr: int,
    input_size: int,
    output_addr: int,
    tag_addr: int,
    tag_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_ONEPASS_DEC_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_addr,
            key_size,
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            output_addr,
            tag_addr,
            tag_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_aead_init_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    enc: bool,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    data_size: int,
    tag_size: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_AEAD_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            algo,
            mode,
            key_addr,
            key_size,
            int(enc),
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            data_size,
            tag_size,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_onepass_enc_with_plain_key(
    ctx: int,
    key_addr: int,
    key_size: int,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    constant: int,
    input_addr: int,
    input_size: int,
    output_addr: int,
    tag_addr: int,
    tag_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_ONEPASS_ENC_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            key_addr,
            key_size,
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            input_addr,
            input_size,
            output_addr,
            tag_addr,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_onepass_dec_with_plain_key(
    ctx: int,
    key_addr: int,
    key_size: int,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    constant: int,
    input_addr: int,
    input_size: int,
    output_addr: int,
    tag_addr: int,
    tag_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_ONEPASS_DEC_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            key_addr,
            key_size,
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            input_addr,
            input_size,
            output_addr,
            tag_addr,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_init_with_plain_key(
    ctx: int,
    key_addr: int,
    key_size: int,
    enc: bool,
    nonce_addr: int,
    nonce_size: int,
    aad_addr: int,
    aad_size: int,
    constant: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLL",
            ctx,
            key_addr,
            key_size,
            int(enc),
            nonce_addr,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_onepass_enc(
        ctx: int,
        key_handle: int,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        constant: int,
        input: int,
        input_size: int,
        output: int,
        tag: int,
        tag_size: int
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_ONEPASS_ENC,
        struct.pack(
            "<LLLLLLLLLLLL",
            ctx,
            key_handle,
            nonce,
            nonce_size,
            aad,
            aad_size,
            constant,
            input,
            input_size,
            output,
            tag,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_onepass_dec(
        ctx: int,
        key_handle: int,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        constant: int,
        input: int,
        input_size: int,
        tag: int,
        tag_size: int,
        output: int
    ) -> tuple[int, int]:
    # Reason: C struct chacha_onepass_dec_cmd_st field order is:
    # ctx, key_handle, nonce, nonce_size, aad, aad_size, constant,
    # input, input_size, output, tag, tag_size, verify_result (13 fields = 52 bytes)
    # verify_result is written by server, send 0 as placeholder
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_ONEPASS_DEC,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            key_handle,
            nonce,
            nonce_size,
            aad,
            aad_size,
            constant,
            input,
            input_size,
            output,
            tag,
            tag_size,
            0  # verify_result placeholder, filled by server
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_chacha_init(
        ctx: int,
        key_handle: int,
        enc: bool,
        nonce: int,
        nonce_size: int,
        aad: int,
        aad_size: int,
        constant: int,
        session: int
    ) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_INIT,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            key_handle,
            int(enc),
            nonce,
            nonce_size,
            aad,
            aad_size,
            constant,
            session
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_chacha_update(
     ctx: int,
     input: int,
     input_size: int,
     output: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_UPDATE,
        struct.pack(
            "<LLLL",
            ctx,
            input,
            input_size,
            output
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_finish_enc(
     ctx: int,
     input: int,
     input_size: int,
     output: int,
     tag: int,
     tag_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_FINISH_ENC,
        struct.pack(
            "<LLLLLL",
            ctx,
            input,
            input_size,
            output,
            tag,
            tag_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_chacha_finish_dec(
     ctx: int,
     input: int,
     input_size: int,
     tag: int,
     tag_size: int,
     output: int
    ) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHACHA_FINISH_DEC,
        struct.pack(
            "<LLLLLLL",
            ctx,
            input,
            input_size,
            output,
            tag,
            tag_size,
            0
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_change_control_field(
     ctx: int,
     field: int,
     value: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHANGE_CONTROL_FIELD,
        struct.pack(
            "<LLL",
            ctx,
            field,
            value
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_change_lifecycle(
     ctx: int,
     lifecycle: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CHANGE_LIFECYCLE,
        struct.pack(
            "<LL",
            ctx,
            lifecycle
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_close_debug(
     ctx: int,
     type: int,
     soc_dbg_bitmap: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CLOSE_DEBUG,
        struct.pack(
            "<LLL",
            ctx,
            type,
            soc_dbg_bitmap
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_create_counter(
     ctx: int,
     counter_id: int,
     counter_value: int
) -> tuple[int, int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CREATE_COUNTER,
        struct.pack(
            "<LLL",
            ctx,
            counter_id,
            counter_value
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little"), int.from_bytes(data[8:12], "little")

@api
def ehsm_ctx_init(ctx: int, mb_ch: int, is_async: bool) -> None:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_INIT_CTX,
        struct.pack(
            "<LLL",
            ctx,
            mb_ch,
            int(is_async)
        ),
    )
    _check_ret(data)

@api
def ehsm_ctx_poll(ctx: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_CTX_POLL,
        struct.pack(
            "<L",
            ctx
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_debug_auth(
        ctx: int,
        challenge_type: int,
        algo: int,
        sig: int,
        sig_size: int,
        pub_key: int,
        pub_key_size: int,
        soc_dbg_bitmap: int,
    ) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_DEBUG_AUTH,
        struct.pack(
            "<LLLLLLLL",
            ctx,
            challenge_type,
            algo,
            sig,
            sig_size,
            pub_key,
            pub_key_size,
            soc_dbg_bitmap
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_delete_counter(
    ctx_addr:int,
    counter_id:int
) -> int :
    t, data = soccmd.send_cmd_recv_rsp(
    cmddef.CMD_HOSTAPI_DELETE_COUNTER,
    struct.pack(
        "<LL",
        ctx_addr,
        counter_id
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_driver_get_version(void) -> None:
    t, data = soccmd.send_cmd_recv_rsp(
    cmddef.CMD_HOSTAPI_DRIVER_GET_VERSION
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_driver_init_library(
    drv_mode:int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
    cmddef.CMD_HOSTAPI_DRIVER_INIT_LIBRANY,
    struct.pack(
        "<L",
        drv_mode
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_ecdsa_finish_gen(
    ctx_addr: int,
    sig_addr: int,
    sig_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_ECDSA_FINISH_GEN,
        struct.pack(
            "<LLL",
            ctx_addr,
            sig_addr,
            sig_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_ecdsa_finish_verify(
    ctx_addr: int,
    sig_addr: int,
    sig_size: int
) -> tuple[int, bool]:
    # Reason: C结构需要4个字段,verify_result作为输出参数占位
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_ECDSA_FINISH_VERIFY,
        struct.pack(
            "<LLLL",
            ctx_addr,
            sig_addr,
            sig_size,
            0  # verify_result占位符,会被C函数填充
        )
    )
    _check_ret(data)
    # Reason: C代码返回 ecdsa_finish_verify_rsp_st {ret, verify_result}
    verify_result = bool(int.from_bytes(data[4:8], "little"))
    return t, verify_result

@api
def ehsm_ecdsa_init(
    ctx_addr: int,
    algo: int,
    key_handle: int,
    gen_sig: bool,
    session: int,
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_ECDSA_INIT,
        struct.pack(
            "<LLLLL",
            ctx_addr,
            algo,
            key_handle,
            gen_sig,
            session,
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_ecdsa_onepass_gen(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        msg: int,
        msg_size: int,
        sig: int,
        sig_size: int,
    ) -> tuple[int, bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ECDSA_ONEPASS_GEN,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                algo,
                key_handle,
                msg,
                msg_size,
                sig,
                sig_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_ecdsa_onepass_verify(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ECDSA_ONEPASS_VERIFY,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                algo,
                key_handle,
                msg,
                msg_size,
                sig,
                sig_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[8:12], "little")

@api
def ehsm_ecdsa_onepass_gen_with_digest(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        digest: int,
        digest_size: int,
        sig: int,
        sig_size: int,
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ECDSA_ONEPASS_GEN_WITH_DIGEST,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                algo,
                key_handle,
                digest,
                digest_size,
                sig,
                sig_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_ecdsa_onepass_verify_with_digest(
        ctx_addr: int,
        hash_algo: int,
        key_handle: int,
        digest: int,
        digest_size: int,
        sig: int,
        sig_size: int
    ) -> tuple[int, bool]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ECDSA_ONEPASS_VERIFY_WITH_DIGEST,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                hash_algo,
                key_handle,
                digest,
                digest_size,
                sig,
                sig_size
            ),
        )
        _check_ret(data)
        return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_ecdsa_onepass_ex(
    ctx: int,
    algo: int,
    use_plain_key: bool,
    key_handle: int,
    key: int,
    gen_sig: bool,
    is_digest: bool,
    input_addr: int,
    input_size: int,
    sig_addr: int,
    sig_size: int,
    verify_result_addr: int
) -> tuple[int, int]:
    """
    ECDSA签名/验签统一接口 (ex版本)

    Reason: 统一接口支持密钥句柄/明文密钥、签名/验签、原始消息/摘要的所有组合
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_ECDSA_ONEPASS_EX,
        struct.pack(
            "<LLLLLLLLLLLL",
            ctx,
            algo,
            int(use_plain_key),
            key_handle,
            key,
            int(gen_sig),
            int(is_digest),
            input_addr,
            input_size,
            sig_addr,
            sig_size,
            verify_result_addr
        )
    )
    _check_ret(data)
    # Reason: 返回结构是 ecdsa_rsp_st {ret, sig_size, verify_result}, 总是12字节
    sig_size = struct.unpack("<L", data[4:8])[0]
    verify_result = struct.unpack("<L", data[8:12])[0]

    if gen_sig:
        # 签名模式: 返回签名长度
        return t, sig_size
    else:
        # 验签模式: 返回验签结果
        return t, verify_result

@api
def ehsm_ecdsa_update(
        ctx_addr: int,
        msg: int,
        msg_size: int,
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ECDSA_UPDATE,
            struct.pack(
                "<LLL",
                ctx_addr,
                msg,
                msg_size,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_enter_wfi(
        ctx_addr: bytes
    ) -> tuple[int,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_ENTER_WFI,
            struct.pack(
                "<L",
                ctx_addr
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_gen_random(
        ctx_addr: int,
        algo: int,
        rand_buf: bytes,
        rand_size: int,
    ) -> tuple[int ,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_GEN_RANDOM,
            struct.pack(
                "<LLLL",
                ctx_addr,
                algo,
                rand_buf,
                rand_size,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_get_challenge(
        ctx_addr: int,
        challenge_type: int,
        output: int
    ) -> tuple[int ,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_GET_CHALLENGE,
            struct.pack(
                "<LLL",
                ctx_addr,
                challenge_type,
                output,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_get_emu_status(
        ctx_addr: int,
        status_buf: int
    ) -> tuple[int,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_GET_EMU_STATUS,
            struct.pack(
                "<LL",
                ctx_addr,
                status_buf,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_get_utc_time(
        ctx_addr: int,
        utc_time: bytes,
    ) -> tuple[int,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_GET_UTC_TIME,
            struct.pack(
                "<LL",
                ctx_addr,
                utc_time,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_get_version(
        ctx_addr: int,
        version: int,
    ) -> tuple[int,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_GET_VERSION,
            struct.pack(
                "<LL",
                ctx_addr,
                version,
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_hash_finish(
        ctx_addr: int,
        digest: int,
        digest_size: int
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HASH_FINISH,
            struct.pack(
                "<LLL",
                ctx_addr,
                digest,
                digest_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_hash_init(
        ctx_addr: int,
        algo: int,
        session: int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HASH_INIT,
            struct.pack(
                "<LLL",
                ctx_addr,
                algo,
                session
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hash_onepass(
        ctx_addr: int,
        algo: int,
        msg: int,
        msg_size: int,
        digest_addr: int,
        buf_size: int
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HASH_ONEPASS,
            struct.pack(
                "<LLLLLL",
                ctx_addr,
                algo,
                msg,
                msg_size,
                digest_addr,
                buf_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_hash_update(
        ctx_addr: int,
        msg: int,
        msg_size: int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HASH_UPDATE,
            struct.pack(
                "<LLL",
                ctx_addr,
                msg,
                msg_size
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hmac_finish_gen(
        ctx_addr: int,
        hmac: bytes,
        hmac_size: int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_FINISH_GEN,
            struct.pack(
                "<LLL",
                ctx_addr,
                hmac,
                hmac_size
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hmac_finish_verify(
        ctx_addr: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bool]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_FINISH_VERIFY,
            struct.pack(
                "<LLL",
                ctx_addr,
                hmac,
                hmac_size
            ),
        )
        _check_ret(data)
        return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_hmac_init(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        gen_hmac: bool,
        session: int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_INIT,
            struct.pack(
                "<LLLLL",
                ctx_addr,
                algo,
                key_handle,
                gen_hmac,
                session
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hmac_onepass_gen(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        msg: int,
        msg_size:int,
        hmac:bytes,
        hmac_size:int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_ONEPASS_GEN,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                algo,
                key_handle,
                msg,
                msg_size,
                hmac,
                hmac_size
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hmac_onepass_verify(
        ctx_addr: int,
        algo: int,
        key_handle: int,
        msg: int,
        msg_size:int,
        hmac:int,
        hmac_size:int
    ) -> tuple[int, bool]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_ONEPASS_VERIFY,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                algo,
                key_handle,
                msg,
                msg_size,
                hmac,
                hmac_size
            ),
        )
        _check_ret(data)
        return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_hmac_update(
        ctx_addr: int,
        msg: bytes,
        msg_size:int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_HMAC_UPDATE,
            struct.pack(
                "<LLL",
                ctx_addr,
                msg,
                msg_size
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_hmac_onepass_gen_with_plain_key(
    ctx: int,
    algo: int,
    key_addr: int,
    key_size: int,
    msg_addr: int,
    msg_size: int,
    hmac_addr: int,
    hmac_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_HMAC_ONEPASS_GEN_WITH_KEY,
        struct.pack(
            "<LLLLLLLL",
            ctx,
            algo,
            key_addr,
            key_size,
            msg_addr,
            msg_size,
            hmac_addr,
            hmac_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_hmac_onepass_verify_with_plain_key(
    ctx: int,
    algo: int,
    key_addr: int,
    key_size: int,
    msg_addr: int,
    msg_size: int,
    hmac_addr: int,
    hmac_size: int,
    verify_result_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_HMAC_ONEPASS_VERIFY_WITH_KEY,
        struct.pack(
            "<LLLLLLLLL",
            ctx,
            algo,
            key_addr,
            key_size,
            msg_addr,
            msg_size,
            hmac_addr,
            hmac_size,
            verify_result_addr
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_hmac_init_with_plain_key(
    ctx: int,
    algo: int,
    key_addr: int,
    key_size: int,
    gen_hmac: bool,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_HMAC_INIT_WITH_KEY,
        struct.pack(
            "<LLLLLL",
            ctx,
            algo,
            key_addr,
            key_size,
            int(gen_hmac),
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_increase_counter(
        ctx_addr: int,
        counter_id: bytes,
        increase_value:int,
        current_value:bytes
    ) -> tuple[int ,bytes]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_INCREASE_COUNTER,
            struct.pack(
                "<LLLL",
                ctx_addr,
                counter_id,
                increase_value,
                current_value
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_inject_error(
        ctx_addr: int,
        values:bytes
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_INJECT_ERROR,
            struct.pack(
                "<LL",
                ctx_addr,
                values
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_install_encrypted_key(
        ctx_addr: int,
        key_level:int,
        key_type: int,
        key_slot_id:int,
        last_key:int,
        input_data:bytes,
        size:int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_INSTALL_ENCRYPTED_KEY,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                key_level,
                key_type,
                key_slot_id,
                last_key,
                input_data,
                size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_install_random_key(
        ctx_addr: int,
        key_level:int,
        key_type:int,
        key_slot_id:int,
        last_key :int,
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_INSTALL_RANDOM_KEY,
            struct.pack(
                "<LLLLL",
                ctx_addr,
                key_level,
                key_type,
                key_slot_id,
                last_key
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_km_derive_key(
        ctx_addr: int,
        hash_algo:int,
        derive_algo:int,
        derive_type:int,
        privilege:int,
        key_type:int,
        key_size:int,
        parent_key_handle:int,
        salt:int,
        salt_size:int,
        password:int,
        password_size:int,
        iter_times:int,
        key_handle:int,
    ) -> tuple[int ,int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_DERIVE_KEY,
            struct.pack(
                "<LLLLLLLLLLLLLL",
                ctx_addr,
                hash_algo,
                derive_algo,
                derive_type,
                privilege,
                key_type,
                key_size,
                parent_key_handle,
                salt,
                salt_size,
                password,
                password_size,
                iter_times,
                key_handle
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_km_derive_key_to_soc(
        ctx_addr: int,
        hash_algo:int,
        derive_algo:int,
        derive_type:int,
        parent_key_handle:int,
        salt:bytes,
        salt_size:int,
        iter_times:int,
        soc_channel_id:int
    ) -> tuple[int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_DERIVE_KEY_TO_SOC,
            struct.pack(
                "<LLLLLLLLL",
                ctx_addr,
                hash_algo,
                derive_algo,
                derive_type,
                parent_key_handle,
                salt,
                salt_size,
                iter_times,
                soc_channel_id
            ),
        )
        _check_ret(data)
        return t

@api
def ehsm_km_exchange_key(
        ctx_addr: int,
        rmt_pub_key:int,
        rmt_pub_key_size:int,
        privilege:int,
        key_type:int,
        hmac_key_size:int,
        local_key_handle:int,
        dh_params:int,
        dh_params_size:int,
        sm2_params:int,
        key_handle:int
    ) -> tuple[int ,int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_EXCHANGE_KEY,
            struct.pack(
                "<LLLLLLLLLLL",
                ctx_addr,
                rmt_pub_key,
                rmt_pub_key_size,
                privilege,
                key_type,
                hmac_key_size,
                local_key_handle,
                dh_params,
                dh_params_size,
                sm2_params,
                key_handle
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_km_export_key(
        ctx_addr: int,
        target_key_handle:int,
        transport_key_handle:int,
        auth_key_handle:int,
        key_part :int,
        key_data:int,
        key_data_size :int,
        mac:int,
        mac_size :int
    ) -> tuple[int, int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_EXPORT_KEY,
            struct.pack(
                "<LLLLLLLLL",
                ctx_addr,
                target_key_handle,
                transport_key_handle,
                auth_key_handle,
                key_part,
                key_data,
                key_data_size,
                mac,
                mac_size
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little"), int.from_bytes(data[8:12], "little")

@api
def ehsm_km_gen_key(
        ctx_addr: int,
        key_type: int,
        privilege: int,
        rsa_e_bit_size: int,
        hmac_key_size: int,
        dh_params_addr: int,
        dh_params_size: int,
        key_handle: int
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_GEN_KEY,
            struct.pack(
                "<LLLLLLLL",
                ctx_addr,
                key_type,
                privilege,
                rsa_e_bit_size,
                hmac_key_size,
                dh_params_addr,
                dh_params_size,
                key_handle
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_km_get_pub_from_priv(
        ctx_addr: int,
        key_handle:int,
        dh_params:int,
        dh_params_size :int,
        pub_key  :int,
        pub_key_size:int,
        key_type :int,
    ) -> tuple[int,int,int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_GET_PUB_FROM_PRIV,
            struct.pack(
                "<LLLLLLL",
                ctx_addr,
                key_handle,
                dh_params,
                dh_params_size,
                pub_key,
                pub_key_size,
                key_type
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little"),int.from_bytes(data[8:12], "little")

@api
def ehsm_km_import_key(
        ctx_addr: int,
        trans_key_handle: int,
        auth_key_handle: int,
        key_data_addr: int,
        key_data_size: int,
        mac_addr: int,
        mac_size: int,
        key_handle: int
    ) -> tuple[int, int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_IMPORT_KEY,
            struct.pack(
                "<LLLLLLLL",
                ctx_addr,
                trans_key_handle,
                auth_key_handle,
                key_data_addr,
                key_data_size,
                mac_addr,
                mac_size,
                key_handle
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_km_remove_key(
        ctx_addr: int,
        key_handle: int
    ) -> int:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_REMOVE_KEY,
            struct.pack(
                "<LL",
                ctx_addr,
                key_handle
            )
        )
        _check_ret(data)
        return t

@api
def ehsm_km_sm9_exchange_key(
            ctx_addr: int,
            privilege: int,
            key_type: int,
            role:int,
            user_priv_key_handle:int,
            user_tmp_key_handle:int,
            hmac_key_size:int,
            extra_params:bytes,
    ) -> tuple[int ,int]:
        t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_KM_SM9_EXCHANGE_KEY,
            struct.pack(
                "<LLLLLLLL",
                ctx_addr,
                privilege,
                key_type,
                role,
                user_priv_key_handle,
                user_tmp_key_handle,
                hmac_key_size,
                extra_params
            ),
        )
        _check_ret(data)
        return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_mac_onepass_gen(
    ctx_addr: int,
    algo: int,
    mode: int,
    key_handle: int,
    iv_addr: int,
    iv_size: int,
    msg: int,
    msg_size: int,
    mac: int,
    mac_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_ONEPASS_GEN,
        struct.pack(
            "<LLLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            key_handle,
            iv_addr,
            iv_size,
            msg,
            msg_size,
            mac,
            mac_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_onepass_verify(
    ctx_addr: int,
    algo: int,
    mode: int,
    key_handle: int,
    iv_addr: int,
    iv_size: int,
    msg: int,
    msg_size: int,
    mac: int,
    mac_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_ONEPASS_VERIFY,
        struct.pack(
            "<LLLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            key_handle,
            iv_addr,
            iv_size,
            msg,
            msg_size,
            mac,
            mac_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_mac_init(
    ctx_addr: int,
    algo: int,
    mode: int,
    key_handle: int,
    gen_mac: bool,
    iv: int,
    iv_size: int,
    mac_size: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_INIT,
        struct.pack(
            "<LLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            key_handle,
            int(gen_mac),
            iv,
            iv_size,
            mac_size,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_update(
    ctx_addr: int,
    msg: int,
    msg_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_UPDATE,
        struct.pack(
            "<LLL",
            ctx_addr,
            msg,
            msg_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_finish_gen(
    ctx_addr: int,
    msg: int,
    msg_size: int,
    mac: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_FINISH_GEN,
        struct.pack(
            "<LLLL",
            ctx_addr,
            msg,
            msg_size,
            mac
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_finish_verify(
    ctx_addr: int,
    msg: int,
    msg_size: int,
    mac: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_FINISH_VERIFY,
        struct.pack(
            "<LLLL",
            ctx_addr,
            msg,
            msg_size,
            mac
        )
    )
    _check_ret(data)
    verify_result = bool.from_bytes(data[4:8], "little")
    return t, verify_result

@api
def ehsm_mac_onepass_gen_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    iv_addr: int,
    iv_size: int,
    msg_addr: int,
    msg_size: int,
    mac_addr: int,
    mac_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_ONEPASS_GEN_WITH_KEY,
        struct.pack(
            "<LLLLLLLLLLL",
            ctx, algo, mode, key_addr, key_size,
            iv_addr, iv_size, msg_addr, msg_size,
            mac_addr, mac_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_onepass_verify_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    iv_addr: int,
    iv_size: int,
    msg_addr: int,
    msg_size: int,
    mac_addr: int,
    mac_size: int,
    verify_result_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_ONEPASS_VERIFY_WITH_KEY,
        struct.pack(
            "<LLLLLLLLLLLL",
            ctx, algo, mode, key_addr, key_size,
            iv_addr, iv_size, msg_addr, msg_size,
            mac_addr, mac_size, verify_result_addr
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_mac_init_with_plain_key(
    ctx: int,
    algo: int,
    mode: int,
    key_addr: int,
    key_size: int,
    gen_mac: bool,
    iv_addr: int,
    iv_size: int,
    mac_size: int,
    session_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_MAC_INIT_WITH_KEY,
        struct.pack(
            "<LLLLLLLLLL",
            ctx, algo, mode, key_addr, key_size,
            1 if gen_mac else 0, iv_addr, iv_size,
            mac_size, session_addr
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_read_counter(
    ctx_addr: int,
    counter_id: int,
    counter_value: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_READ_COUNTER,
            struct.pack(
                "<LLL",
                ctx_addr,
                counter_id,
                counter_value
            ),
        )
    _check_ret(data)
    return t

@api
def ehsm_read_otp(
    ctx_addr: int,
    buf: int,
    ehsm_src_addr: int,
    size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_READ_OTP,
            struct.pack(
                "<LLLL",
                ctx_addr,
                buf,
                ehsm_src_addr,
                size
            ),
        )
    _check_ret(data)
    return t

@api
def ehsm_read_reg(
    ctx_addr: int,
    buf: int,
    ehsm_src_addr: int,
    size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_READ_REG,
            struct.pack(
                "<LLLL",
                ctx_addr,
                buf,
                ehsm_src_addr,
                size
            ),
        )
    _check_ret(data)
    return t

@api
def ehsm_rsa_cipher(
    ctx_addr: int,
    key_handle: int,
    enc: bool,
    input: int,
    input_size: int,
    output: int,
    output_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_CIPHER,
        struct.pack(
            "<LLLLLLL",
            ctx_addr,
            key_handle,
            int(enc),
            input,
            input_size,
            output,
            output_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_rsa_sign_init(
    ctx_addr: int,
    algo: int,
    key_handle: int,
    gen_sig: int,
    padding: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_INIT,
        struct.pack(
            "<LLLLLL",
            ctx_addr,
            algo,
            key_handle,
            gen_sig,
            padding,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_sign_update(
    ctx_addr: int,
    msg: int,
    msg_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_UPDATE,
        struct.pack(
            "<LLL",
            ctx_addr,
            msg,
            msg_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_sign_finish_gen(
    ctx_addr: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_FINISH_GEN,
        struct.pack(
            "<LLLL",
            ctx_addr,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_sign_finish_verify(
    ctx_addr: int,
    sig: int,
    sig_size: int,
    salt_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_FINISH_VERIFY,
        struct.pack(
            "<LLLL",
            ctx_addr,
            sig,
            sig_size,
            salt_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_set_uart_baudrate(
    ctx_addr: int,
    baud_div: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_SET_UART_BAUDRATE,
            struct.pack(
                "<LL",
                ctx_addr,
                baud_div
            ),
        )
    _check_ret(data)
    return t

@api
def ehsm_set_utc_time(
    ctx_addr: int,
    utc_time: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
            cmddef.CMD_HOSTAPI_SET_UTC_TIME,
            struct.pack(
                "<LL",
                ctx_addr,
                utc_time
            ),
        )
    _check_ret(data)
    return t

@api
def ehsm_sm2_cipher(
    ctx_addr: int,
    key_handle: int,
    enc: bool,
    input: int,
    input_size: int,
    output: int,
    output_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_CIPHER,
        struct.pack(
            "<LLLLLLL",
            ctx_addr,
            key_handle,
            int(enc),
            input,
            input_size,
            output,
            output_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm2_sign_onepass_gen(
    ctx_addr: int,
    key_handle: int,
    msg: int,
    msg_size: int,
    sig: int,
    sig_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_ONEPASS_GEN,
        struct.pack(
            "<LLLLLL",
            ctx_addr,
            key_handle,
            msg,
            msg_size,
            sig,
            sig_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")


@api
def ehsm_sm2_sign_onepass_gen_with_digest(
    ctx_addr: int,
    key_handle: int,
    digest: int,
    digest_size: int,
    sig: int,
    sig_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_ONEPASS_GEN_WITH_DIGEST,
        struct.pack(
            "<LLLLLL",
            ctx_addr,
            key_handle,
            digest,
            digest_size,
            sig,
            sig_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")
@api
def ehsm_sm2_sign_onepass_verify(
    ctx: int,
    key_handle: int,
    msg: int,
    msg_size: int,
    sig: int,
    sig_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_ONEPASS_VERIFY,
        struct.pack(
            "<LLLLLL",
            ctx,
            key_handle,
            msg,
            msg_size,
            sig,
            sig_size,
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_sm2_sign_onepass_verify_with_digest(
    ctx_addr: int,
    key_handle: int,
    digest: int,
    digest_size: int,
    sig: int,
    sig_size: int
) -> tuple[int, bool]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_ONEPASS_VERIFY_WITH_DIGEST,
        struct.pack(
            "<LLLLLL",
            ctx_addr,
            key_handle,
            digest,
            digest_size,
            sig,
            sig_size,
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_sm2_sign_onepass_ex(
    ctx: int,
    use_plain_key: bool,
    key_handle: int,
    key: int,
    gen_sig: bool,
    is_digest: bool,
    input_addr: int,
    input_size: int,
    sig_addr: int,
    sig_size: int,
    verify_result_addr: int
) -> tuple[int, int, bool]:
    """
    SM2签名/验签统一接口 (ex版本)

    Reason: 统一接口支持密钥句柄/明文密钥、签名/验签、原始消息/摘要(E值)的所有组合
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_ONEPASS_EX,
        struct.pack(
            "<LLLLLLLLLLL",
            ctx,
            int(use_plain_key),
            key_handle,
            key,
            int(gen_sig),
            int(is_digest),
            input_addr,
            input_size,
            sig_addr,
            sig_size,
            verify_result_addr
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little"),bool.from_bytes(data[8:12], "little")

@api
def ehsm_sm2_sign_init(
    ctx_addr: int,
    key_handle: int,
    gen_sig: bool,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_INIT,
        struct.pack(
            "<LLLL",
            ctx_addr,
            key_handle,
            int(gen_sig),
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_sm2_sign_update(
    ctx_addr: int,
    msg: int,
    msg_size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_UPDATE,
        struct.pack(
            "<LLL",
            ctx_addr,
            msg,
            msg_size,
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_sm2_sign_finish_gen(
    ctx_addr: int,
    sig: int,
    sig_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_FINISH_GEN,
        struct.pack(
            "<LLL",
            ctx_addr,
            sig,
            sig_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm2_sign_finish_verify(
    ctx_addr: int,
    sig: int,
    sig_size: int
) -> bool:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_FINISH_VERIFY,
        struct.pack(
            "<LLL",
            ctx_addr,
            sig,
            sig_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[8:12], "little")

@api
def ehsm_sm9_cipher(
    ctx_addr: int,
    key_handle: int,
    enc: bool,
    enc_type: int,
    padding: int,
    key2_size: int,
    hid: int,
    kgc_pub_key: int,
    input: int,
    input_size: int,
    output: int,
    output_size: int,
    id: int,
    id_size: int,
    fp12g: int,
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM9_CIPHER,
        struct.pack(
            "<LLLLLLLLLLLLLLL",
            ctx_addr,
            key_handle,
            int(enc),
            enc_type,
            padding,
            key2_size,
            hid,
            kgc_pub_key,
            input,
            input_size,
            output,
            output_size,
            id,
            id_size,
            fp12g
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm9_sign_onepass_gen(
    ctx_addr: int,
    key_handle: int,
    msg: int,
    msg_size: int,
    sig: int,
    sig_size: int,
    kgc_pub_key: int,
    fp12g: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM9_SIGN_ONEPASS_GEN,
        struct.pack(
            "<LLLLLLLL",
            ctx_addr,
            key_handle,
            msg,
            msg_size,
            sig,
            sig_size,
            kgc_pub_key,
            fp12g
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_sm9_sign_onepass_verify(
    ctx_addr: int,
    msg: int,
    msg_size: int,
    id: int,
    id_size: int,
    hid: int,
    kgc_pub_key: int,
    fp12g: int,
    sig: int,
    sig_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM9_SIGN_ONEPASS_VERIFY,
        struct.pack(
            "<LLLLLLLLLL",
            ctx_addr,
            msg,
            msg_size,
            id,
            id_size,
            hid,
            kgc_pub_key,
            fp12g,
            sig,
            sig_size
        )
    )
    _check_ret(data)
    return t, bool.from_bytes(data[4:8], "little")

@api
def ehsm_symm_cipher_onepass(
    ctx_addr: int,
    algo: int,
    mode: int,
    padding: int,
    key_handle: int,
    enc: bool,
    iv: int,
    iv_size: int,
    input: int,
    input_size: int,
    output: int,
    outbuf_size: int,
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_ONEPASS,
        struct.pack(
            "<LLLLLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            padding,
            key_handle,
            int(enc),
            iv,
            iv_size,
            input,
            input_size,
            output,
            outbuf_size,
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_symm_cipher_onepass_plain_key(
    ctx_addr: int,
    algo: int,
    mode: int,
    padding: int,
    enc: bool,
    key: int,
    key_size,
    iv: int,
    iv_size: int,
    input: int,
    input_size: int,
    output: int,
    outbuf_size: int,
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_ONEPASS_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            padding,
            int(enc),
            key,
            key_size,
            iv,
            iv_size,
            input,
            input_size,
            output,
            outbuf_size,
        ),
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_symm_cipher_init(
    ctx_addr: int,
    algo: int,
    mode: int,
    padding: int,
    key_handle: int,
    enc: bool,
    iv: int,
    iv_size: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_INIT,
        struct.pack(
            "<LLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            padding,
            key_handle,
            int(enc),
            iv,
            iv_size,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_symm_cipher_init_with_plain_key(
    ctx_addr: int,
    algo: int,
    mode: int,
    padding: int,
    key: int,
    key_size: int,
    enc: bool,
    iv: int,
    iv_size: int,
    session: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLL",
            ctx_addr,
            algo,
            mode,
            padding,
            key,
            key_size,
            int(enc),
            iv,
            iv_size,
            session
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_symm_cipher_update(
    ctx_addr: int,
    input: int,
    input_size: int,
    output: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_UPDATE,
        struct.pack(
            "<LLLL",
            ctx_addr,
            input,
            input_size,
            output
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_symm_cipher_finish(
    ctx_addr: int,
    input: int,
    input_size: int,
    output: int,
    output_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SYMM_CIPHER_FINISH,
        struct.pack(
            "<LLLLL",
            ctx_addr,
            input,
            input_size,
            output,
            output_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_upgrade_fw_image(
    ctx_addr: int,
    image: int,
    image_size: int,
    image_out: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_UPGRADE_FW_IMAGE,
        struct.pack(
            "<LLLL",
            ctx_addr,
            image,
            image_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_upgrade_fw_image_init(
    ctx_addr: int,
    image_header: int,
    header_size: int,
    image_out: int
) -> int:
    """Three-stage upgrade firmware image - init stage"""
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_UPGRADE_FW_IMAGE_INIT,
        struct.pack(
            "<LLLL",
            ctx_addr,
            image_header,
            header_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_upgrade_fw_image_update(
    ctx_addr: int,
    body_block: int,
    block_size: int,
    image_out: int
) -> int:
    """Three-stage upgrade firmware image - update stage"""
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_UPGRADE_FW_IMAGE_UPDATE,
        struct.pack(
            "<LLLL",
            ctx_addr,
            body_block,
            block_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_upgrade_fw_image_finish(
    ctx_addr: int,
    body_block: int,
    block_size: int,
    image_out: int
) -> int:
    """Three-stage upgrade firmware image - finish stage"""
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_UPGRADE_FW_IMAGE_FINISH,
        struct.pack(
            "<LLLL",
            ctx_addr,
            body_block,
            block_size,
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_verify_image(
    ctx_addr: int,
    image: int,
    image_size: int,
    check_version: bool,
    boot: bool,
    image_out: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_VERIFY_IMAGE,
        struct.pack(
            "<LLLLLL",
            ctx_addr,
            image,
            image_size,
            int(check_version),
            int(boot),
            image_out
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_verify_tbbr_img(
    ctx_addr: int,
    img_size: int,
    img_data: int,
    trusted_fw_nv_ctr_in_otp: int,
    non_trusted_fw_nv_ctr_in_otp: int,
    trusted_fw_nv_ctr_in_cert: int,
    non_trusted_fw_nv_ctr_in_otp_cert: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_VERIFY_TBBR_IMG,
        struct.pack(
            "<LLLLLLL",
            ctx_addr,
            img_size,
            img_data,
            trusted_fw_nv_ctr_in_otp,
            non_trusted_fw_nv_ctr_in_otp,
            trusted_fw_nv_ctr_in_cert,
            non_trusted_fw_nv_ctr_in_otp_cert
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_write_otp(
    ctx_addr: int,
    src_data: int,
    ehsm_dest_addr: int,
    size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_WRITE_OTP,
        struct.pack(
            "<LLLL",
            ctx_addr,
            src_data,
            ehsm_dest_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_write_reg(
    ctx_addr: int,
    src_data: int,
    ehsm_dest_addr: int,
    size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_WRITE_REG,
        struct.pack(
            "<LLLL",
            ctx_addr,
            src_data,
            ehsm_dest_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_test_read_memory(ctx: int, buf_addr: int, ehsm_src_addr: int, size: int) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_TEST_READ_MEMORY,
        struct.pack(
            "<LLLL",
            ctx,
            buf_addr,
            ehsm_src_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_test_write_memory(
    ctx_addr: int,
    src_data: int,
    ehsm_dest_addr: int,
    size: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_TEST_WRITE_MEMORY,
        struct.pack(
            "<LLLL",
            ctx_addr,
            src_data,
            ehsm_dest_addr,
            size
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_test_jump_to_addr(
    ctx_addr: int,
    ehsm_dest_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_TEST_JUMP_TO_ADDR,
        struct.pack(
            "<LL",
            ctx_addr,
            ehsm_dest_addr,
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_test_jump_to_loop(
    ctx_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_TEST_JUMP_TO_LOOP,
        struct.pack(
            "<L",
            ctx_addr
        ),
    )
    _check_ret(data)
    return t

@api
def ehsm_rsa_cipher_with_plain_key(
    ctx_addr: int,
    key_addr: int,
    key_size: int,
    enc: bool,
    input_addr: int,
    input_size: int,
    output_addr: int,
    output_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_CIPHER_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLL",
            ctx_addr,
            key_addr,
            key_size,
            int(enc),
            input_addr,
            input_size,
            output_addr,
            output_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm2_cipher_with_plain_key(
    ctx_addr: int,
    key_addr: int,
    key_size: int,
    enc: bool,
    input_addr: int,
    input_size: int,
    output_addr: int,
    output_buff_size: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_CIPHER_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLL",
            ctx_addr,
            key_addr,
            key_size,
            int(enc),
            input_addr,
            input_size,
            output_addr,
            output_buff_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_rsa_sign_init_with_plain_key(
    ctx_addr: int,
    algo: int,
    key_addr: int,
    key_size: int,
    gen_sig: bool,
    padding: int,
    session_addr: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_RSA_SIGN_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLL",
            ctx_addr,
            algo,
            key_addr,
            key_size,
            int(gen_sig),
            padding,
            session_addr
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm2_sign_init_with_plain_key(
    ctx_addr: int,
    key_addr: int,
    key_size: int,
    gen_sig: bool,
    session_addr: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM2_SIGN_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLL",
            ctx_addr,
            key_addr,
            key_size,
            int(gen_sig),
            session_addr
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_ecdsa_init_with_plain_key(
    ctx_addr: int,
    algo: int,
    key_addr: int,
    gen_sig: bool,
    session_addr: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_ECDSA_INIT_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLL",
            ctx_addr,
            algo,
            key_addr,
            int(gen_sig),
            session_addr
        )
    )
    _check_ret(data)
    # Reason: C代码只返回ret,session在设备端已初始化,返回session地址
    return t, session_addr

@api
def ehsm_sm9_cipher_with_plain_key(
    ctx_addr: int,
    key_addr: int,
    key_size: int,
    enc: bool,
    enc_type: int,
    padding: int,
    key2_size: int,
    hid: int,
    kgc_pub_key_addr: int,
    input_addr: int,
    input_size: int,
    output_addr: int,
    output_buff_size: int,
    id_addr: int,
    id_size: int,
    fp12g_addr: int
) -> tuple[int, int]:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM9_CIPHER_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLLLLLLLLLL",
            ctx_addr,
            key_addr,
            key_size,
            int(enc),
            enc_type,
            padding,
            key2_size,
            hid,
            kgc_pub_key_addr,
            input_addr,
            input_size,
            output_addr,
            output_buff_size,
            id_addr,
            id_size,
            fp12g_addr
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little")

@api
def ehsm_sm9_sign_onepass_gen_with_plain_key(
    ctx_addr: int,
    key_addr: int,
    msg_addr: int,
    msg_size: int,
    sig_addr: int,
    sig_size: int,
    kgc_pub_key_addr: int,
    fp12g_addr: int
) -> int:
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_SM9_SIGN_ONEPASS_GEN_WITH_PLAIN_KEY,
        struct.pack(
            "<LLLLLLLL",
            ctx_addr,
            key_addr,
            msg_addr,
            msg_size,
            sig_addr,
            sig_size,
            kgc_pub_key_addr,
            fp12g_addr
        )
    )
    _check_ret(data)
    return t

@api
def ehsm_bl_verify_image_discrete(
    ctx: int,
    image: int,
    image_size: int,
    code: int,
    only_copy_code: bool,
    check_version: bool,
    boot: bool,
    image_out: int
) -> int:
    """
    BL下校验安全启动镜像（镜像头和代码分离）

    Args:
        ctx: ehsm context 地址
        image: 镜像头地址
        image_size: 镜像总大小（头部+代码）
        code: 代码区地址（0表示紧跟镜像头后面）
        only_copy_code: 是否只复制代码区
        check_version: 是否检查版本
        boot: 校验成功后是否启动（仅对eHSM镜像有效）
        image_out: 镜像解密输出地址（仅对SOC镜像有效）

    Returns:
        校验耗时(us)
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_BL_VERIFY_IMAGE_DISCRETE,
        struct.pack(
            "<LLLLLLLL",
            ctx,
            image,
            image_size,
            code,
            int(only_copy_code),
            int(check_version),
            int(boot),
            image_out
        )
    )
    _check_ret(data)
    return t

def ehsm_pqc_dsa_onepass_ex(
    ctx: int,
    sign_algo: int,
    sign_mode: int,
    hash_algo: int,
    is_det: int,
    use_plain_key: bool,
    key_handle: int,
    key_addr: int,
    gen_sig: bool,
    input_addr: int,
    input_size: int,
    pqc_ctx_str_addr: int,
    pqc_ctx_str_size: int,
    sig_addr: int,
    sig_size: int,
    verify_result_addr: int
) -> tuple[int, int, int]:
    """
    PQC DSA 签名/验签统一接口

    Reason: 统一接口支持密钥句柄/明文密钥、签名/验签、原始消息/摘要的所有组合
    Reason: 修正字段顺序和数量，与C端结构体 pqc_dsa_onepass_ex_cmd_st 保持一致（16个字段，64字节）
    """
    # Reason: 构造命令数据包
    cmd_data = struct.pack(
        "<LLLLLLLLLLLLLLLL",
        ctx,
        sign_algo,
        sign_mode,
        hash_algo,
        is_det,
        int(use_plain_key),
        key_handle,
        key_addr,
        int(gen_sig),
        input_addr,
        input_size,
        pqc_ctx_str_addr,
        pqc_ctx_str_size,
        sig_addr,
        sig_size,
        verify_result_addr
    )
    import logging as log
    log.debug(f"[PQC DSA] Sending command: len={len(cmd_data)} bytes (expected 64)")
    log.debug(f"[PQC DSA] Parameters: ctx=0x{ctx:x}, sign_algo={sign_algo}, sign_mode={sign_mode}, hash_algo={hash_algo}, is_det={is_det}")
    log.debug(f"[PQC DSA] use_plain_key={use_plain_key}, key_handle=0x{key_handle:x}, key_addr=0x{key_addr:x}")
    log.debug(f"[PQC DSA] gen_sig={gen_sig}, input_addr=0x{input_addr:x}, input_size={input_size}")
    log.debug(f"[PQC DSA] pqc_ctx_str_addr=0x{pqc_ctx_str_addr:x}, pqc_ctx_str_size={pqc_ctx_str_size}")
    log.debug(f"[PQC DSA] sig_addr=0x{sig_addr:x}, sig_size={sig_size}, verify_result_addr=0x{verify_result_addr:x}")

    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_PQC_DSA_ONEPASS_EX,
        cmd_data
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little"), int.from_bytes(data[8:12], "little")

@api
def ehsm_pqc_ml_kem_ex(
    ctx: int,
    is_encaps: bool,
    use_plain_parent_key: bool,
    parent_key_handle: int,
    parent_key_addr: int,
    cipher_key_data_addr: int,
    cipher_key_data_size: int,
    ss_out_type: int,
    ss_key_type: int,
    ss_privilege: int,
    ss_key_handle_addr: int,
    ss_key_addr: int,
    ss_key_size: int
) -> tuple[int, int, int]:
    """
    ML-KEM 密钥封装/解封统一接口

    Reason: 支持封装/解封、密钥句柄/明文密钥、输出密钥句柄/明文密钥的所有组合
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_PQC_ML_KEM_EX,
        struct.pack(
            "<LLLLLLLLLLLLL",
            ctx,
            int(is_encaps),
            int(use_plain_parent_key),
            parent_key_handle,
            parent_key_addr,
            cipher_key_data_addr,
            cipher_key_data_size,
            ss_out_type,
            ss_key_type,
            ss_privilege,
            ss_key_handle_addr,
            ss_key_addr,
            ss_key_size
        )
    )
    _check_ret(data)
    return t, int.from_bytes(data[4:8], "little"), int.from_bytes(data[8:12], "little")


@api
def ehsm_test_inject_otp_write_error(
    ctx_addr: int
) -> int:
    """
    注入 OTP 写失败：调用后，固件执行 secboot_update_ver_cnt 中的 otp_write 会返回错误码 0x123，
    触发 expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED) 并使程序阻塞。

    TODO(开发): 下位机需在 server/hostapi_commands.c 中实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR
    (CMD ID = 163) 的 handler，参考 test_cmd_hostapi_test_read_memory 的结构：
        - 参数: ctx_addr (uint32_t)
        - 调用: ehsm_test_inject_otp_write_error(ctx_addr)  // 下位机设置注入标志
        - 响应: ret (uint32_t)
    下位机 src/test_api.c 中需新增：
        uint32_t ehsm_test_inject_otp_write_error(ehsm_ctx_st *ctx);
    该函数设置一个 hook/flag，使下一次 otp_write 调用返回 0x123。

    Reason: secboot_update_ver_cnt 在 otp_write 失败时调用 expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED)
    并阻塞，Python 测试侧需要通过此 CMD 触发注入才能覆盖该分支。
    """
    t, data = soccmd.send_cmd_recv_rsp(
        cmddef.CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR,
        struct.pack(
            "<L",
            ctx_addr
        ),
    )
    _check_ret(data)
    return t
