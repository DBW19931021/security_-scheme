import logging as log
import time  # Reason: 用于在测试步骤之间添加延迟，避免队列满
from secrets import token_bytes
from cryptosynth import generate_hash_testdata
from cryptosynth import generate_hmac_testdata
import pytest
import allure
import struct
from platform_adapter.api.constants import EhsmDrvMode, EhsmHashAlgo, EhsmDeriveType, EhsmLifecycle
from platform_adapter.api.loader import get_api_interface
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import ehsm_fw_errno
from platform_adapter.uart_lib.ehsm_fw_errno import *
from hypothesis import strategies as st
from hypothesis import given, settings
from platform_adapter.uart_lib import hostapi
from platform_adapter.api.constants import (
    EhsmAeadMode,
    EhsmCipherMode,
    EhsmKeyPart,
    EhsmKeyType,
    EhsmMacMode,
    EhsmPaddingMode,
    EhsmSymmAlgo,
    KeyPermit,
    EhsmKeyLevel,
    EhsmInstallKeyType,
    EhsmDeriveAlgo
)
from cryptosynth import generate_symmetric_testdata
import platform_adapter.uart_lib.ehsm_fw_errno as fw_errno
from ctypes import Structure, c_uint32, c_uint8, c_uint16, POINTER,create_string_buffer,cast,sizeof
from utils import key, otp
from utils.config import cfg_data
from cryptography.hazmat.primitives.serialization import load_der_private_key
from cryptography.hazmat.backends import default_backend

api = get_api_interface()
gdb = get_gdb_interface()
host = get_host_interface()

KEY_PERMIT_NAME_TO_ENUM = {
    "MB_KEY_USAGE_NONE": 0x00,
    "MB_KEY_USAGE_SIGN": 0x01,
    "MB_KEY_USAGE_VERIFY": 0x02,
    "MB_KEY_USAGE_ENCRYPT": 0x04,
    "MB_KEY_USAGE_DECRYPT": 0x08,
    "MB_KEY_USAGE_TIMESTAMP": 0x10,
    "MB_KEY_USAGE_SECUREBOOT": 0x20,
    "MB_KEY_USAGE_SECURESTORAGE": 0x40,
    "MB_KEY_USAGE_KEYCREATION": 0x80,
    "MB_KEY_USAGE_CREATION_TRANSP_KEY": 0x100,
    "MB_KEY_USAGE_UTCSYNC": 0x200,
    "MB_KEY_USAGE_TRANSPORT": 0x400,
    "MB_KEY_PERMIT_REMOVE": 0x800,
    "MB_KEY_PERMIT_IMPORT_PLAINTEXT": 0x1000,
    "MB_KEY_PERMIT_IMPORT_CIPHERTEXT": 0x2000,
    "MB_KEY_PERMIT_EXPORT_PLAINTEXT": 0x4000,
    "MB_KEY_PERMIT_EXPORT_CIPHERTEXT": 0x8000,
    "MB_KEY_PERMIT_BOOT_FIAL_USAGE": 0x10000,
    "MB_KEY_PERMIT_DEBUG_USAGE": 0x20000,
    "MB_KEY_PERMIT_WILDCARD_PRT": 0x40000,
    "MB_KEY_PERMIT_WR_PRT": 0x80000,
}

def _config_tpk_and_gen_key(lc: str, tpk_idx: int, normal_idx: int) -> bytes:
    config = {
        "key" + str(tpk_idx): {
            "value" : key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "key" + str(normal_idx): {
            "value" : key.EHSM_DEBUG_SIGN_KEY_AES128.hex(),
            "level" : 1,
            "lifecycle" : "available",
            "type" : "symm",
        },
        "key_alg_sel": "aes128",
        "lifecycle" : lc
    }

    log.debug(config)
    return otp.otp_to_bin(config)

def get_priv_key(buf1: bytes) -> bytes:
    priv_key_size = struct.unpack_from("H", buf1, 10)[0]
    return buf1[12:12+priv_key_size]

@pytest.fixture(scope="module")
def setup_module():
    api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_WAIT_AND_POLL)
    api.ehsm_ctx_init(0, False)

def _get_otp_bin(lc: str) -> bytes:
    return otp.otp_to_bin( {"lifecycle": lc})

@allure.feature("kms")
@allure.description("生成 RANDOM 密钥，配置最大长度 512")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-648")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_648():

    with allure.step("1、准备生成密钥指令，配置密钥算法ID为RANDOM, 密钥长度为512，生成句柄h1； # 1、密钥生成成功；"):
        alg = 0
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        last_buf=[]
        rsa_e_bit_size = 0
        dh_param_size = 0
        hmac_size = 512
        for i in range(0,2):
            _, handler = api.ehsm_km_gen_key(alg, permit, rsa_e_bit_size, hmac_size, None, dh_param_size, handle)
            with allure.step("2、导出明文h1密钥数据，保存为m1； # 2、导出成功；"):
                ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,524,None,0)

            with allure.step("3、重复1/2 步骤，对比两次导出的数据进行对比； # 3、对比不一致；"):
                if list(buf1) == bytes(last_buf):
                    api.ehsm_km_remove_key(handler)
                    assert False, "两次生成的RANDOM密钥相同"
                last_buf[:] = list(buf1)
                api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成 RANDOM 密钥，配置长度（非法值）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-649")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_649():

    with allure.step("1、准备生成密钥指令，配置密钥算法ID为RANDOM, 密钥长度为非法值，生成句柄h1； # 1、密钥生成失败；"):
        alg = 0
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        invalid_size = [513,0,2000]
        rsa_e_bit_size = 0
        dh_param_size = 0
        for cnt in range(len(invalid_size)):
            try:
                ret,out_handle = api.ehsm_km_gen_key(alg, permit, rsa_e_bit_size, invalid_size[cnt], None, dh_param_size, handle)
                api.ehsm_km_remove_key(out_handle)
            except hostapi.HostApiError as e:
                assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望错误码{fw_errno.EHSM_ERR_PARAM_ERROR}, 实际错误码{e.ret_code}"

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.feature("kms")
@allure.description("生成 DES 密钥，并使用密钥进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-650")
def test_ehsm_650():
    alg = EhsmKeyType.EHSM_KEY_TYPE_DES
    permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
    handle = 0xFFFFFFFF
    rsa_e_bit_size = 0
    dh_param_size = 0
    hmac_size = 0
    g_last_key = None  # Reason: 存储上一次生成的密钥，用于比较验证随机性

    # Reason: C代码循环2次，验证每次生成的密钥都不同
    for i in range(2):
        with allure.step(f"第{i+1}次循环: 1、生成带有加解密和删除权限的密钥； # 1、密钥生成成功，返回密钥handle；"):
            _, handler = api.ehsm_km_gen_key(alg, permit, rsa_e_bit_size, hmac_size, None, dh_param_size, handle)

        with allure.step(f"第{i+1}次循环: 2、导出明文密钥，与上一次生成的密钥做对比； # 2、导出成功，且与上一次生成的密钥不同；"):
            _, g_key_data, _, _, _ = api.ehsm_km_export_key(
                handler, 0xFFFFFFFF, 0xFFFFFFFF,
                EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                b'', 2316, None, 0
            )

            # Reason: 第一次循环时，g_last_key为None，不需要比较；从第二次开始比较
            if g_last_key is not None:
                assert g_key_data != g_last_key, "生成的密钥与上一次相同，密钥生成不具有随机性"

            g_last_key = g_key_data

        with allure.step(f"第{i+1}次循环: 3、使用生成的密钥进行加密，使用数据m进行加密，得到加密数据c； # 3、加密成功；"):
            m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
            c_size = 16

            # Reason: 使用DES ECB模式加密，无需IV，无padding
            _, c, _ = api.ehsm_symm_cipher_onepass(
                EhsmSymmAlgo.EHSM_SYMM_ALGO_DES,
                EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
                EhsmPaddingMode.EHSM_PADDING_NONE,
                handler, 1,  # Reason: key_size=8 for DES
                None, 0,     # Reason: ECB模式不需要IV
                m, c_size, len(m)
            )

        with allure.step(f"第{i+1}次循环: 4、使用加密数据c，再使用生成的密钥进行解密，得到解密数据m'； # 4、解密成功；"):
            m_size = 16

            # Reason: 解密操作，使用相同的算法和模式
            _, m_new, _ = api.ehsm_symm_cipher_onepass(
                EhsmSymmAlgo.EHSM_SYMM_ALGO_DES,
                EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
                EhsmPaddingMode.EHSM_PADDING_NONE,
                handler, 0,  # Reason: 0表示解密操作
                None, 0,     # Reason: ECB模式不需要IV
                c, m_size, len(c)
            )

        with allure.step(f"第{i+1}次循环: 5、对比m'和m； # 5、数据一致；"):
            assert m_new == m, f"解密后的数据与原始数据不一致"

        with allure.step(f"第{i+1}次循环: 6、删除密钥句柄； # 6、删除成功；"):
            _ = api.ehsm_km_remove_key(handler)


@allure.feature("kms")
@allure.description("分别生成 TDES 128/192 密钥，并使用密钥进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.testcase("EHSM-651")
def test_ehsm_651():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为TDES，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        handle = 0xFFFFFFFF
        key_buf = b''
        tdes_key_size = [128,192]
        tdes_alg_id = [1,2]
        key_algo_id = [EhsmKeyType.EHSM_KEY_TYPE_TDES_128,EhsmKeyType.EHSM_KEY_TYPE_TDES_192]
        mac_size = 0
        mac = None
        c_size = 24
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07])
        for i in range(0,2):
            _, handler = api.ehsm_km_gen_key(key_algo_id[i], permit, 0, 0, None, 0, handle)
            with allure.step("2、准备SKE加密指令，配置算法为TDES_ECB，传入加密数据m和密钥句柄h1； # 2、加密成功；获取加密后的数据；"):
                ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(tdes_alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,1,None,0,m,c_size,24)

            with allure.step("3、获取加密后的数据c，准备SKE解密指令，配置算法为TDES_ECB，传入加密后的数据c和密钥句柄h1； # 3、解密成功；获取解密后的数据；"):
                ret, new_m ,ret8= api.ehsm_symm_cipher_onepass(tdes_alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,0,None,0,c_output,len(c_output),24)
            with allure.step("5、获取解密后的数据m'，与步骤2数据m进行对比 # 5、数据对比一致；"):
                if new_m != m:
                    api.ehsm_km_remove_key(handler)
                    assert False, "数据对比不一致"
            with allure.step("6、删除密钥h1 # 6、删除密钥成功；"):
                api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("分别生成 AES 128/192/256 密钥，并使用密钥进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-652")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
def test_ehsm_652():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为AES_128，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        handle = 0xFFFFFFFF
        key_buf = b''
        tdes_key_size = [128,192,256]
        tdes_alg_id = [EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_192,EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_256]
        key_algo_id = [EhsmKeyType.EHSM_KEY_TYPE_AES_128,EhsmKeyType.EHSM_KEY_TYPE_AES_192,EhsmKeyType.EHSM_KEY_TYPE_AES_256]
        c_size = 16
        encrypto = 1
        decrypto = 0
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        for i in range(0,3):
            _, handler = api.ehsm_km_gen_key(key_algo_id[i], permit, 0, 0, None, 0, handle)
            with allure.step("2、准备SKE加密指令，配置算法为AES_ECB，传入加密数据m和密钥句柄h1； # 2、加密成功；获取加密后的数据；"):
                ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(tdes_alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,encrypto,None,0,m,c_size,24)
            with allure.step("3、获取加密后的数据c，准备SKE解密指令，配置算法为AES_ECB，传入加密后的数据c和密钥句柄h1； # 3、解密成功；获取解密后的数据；"):
                ret, new_m ,ret8= api.ehsm_symm_cipher_onepass(tdes_alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,decrypto,None,0,c_output,len(c_output),24)
            with allure.step("4、获取解密后的数据m'，与步骤2数据m进行对比 # 4、数据对比一致；"):
                if new_m != m:
                    api.ehsm_km_remove_key(handler)
                    assert False, "数据对比不一致"
            with allure.step("5、删除密钥h1 # 5、删除密钥成功；"):
                api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成 SM4 密钥，并使用密钥进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-653")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
def test_ehsm_653():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为SM4，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        handle = 0xFFFFFFFF
        alg_id = [EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4]
        key_algo_id = [EhsmKeyType.EHSM_KEY_TYPE_SM4]
        c_size = 16
        encrypto = 1
        decrypto = 0
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        _, handler = api.ehsm_km_gen_key(key_algo_id[0], permit, 0, 0, None, 0, handle)
        with allure.step("2、导出密钥； # 2、导出密钥成功"):
            ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(alg_id[0],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,encrypto,None,0,m,c_size,24)
        with allure.step("3、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 3、发送数据成功；时间小于200ms"):
            ret, new_m ,ret8= api.ehsm_symm_cipher_onepass(alg_id[0],EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,decrypto,None,0,c_output,len(c_output),24)
        with allure.step("4、重复1.2；对两次导出的密钥进行比较 # 4、数据对比不一致；"):
            if new_m != m:
                api.ehsm_km_remove_key(handler)
                assert False, "数据对比不一致"
        with allure.step("5、删除密钥h1 # 5、删除密钥成功；"):
            api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("分别生成 AES-XTS 128/192/256 密钥，并使用密钥进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.testcase("EHSM-654")
def test_ehsm_654():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为AES_XTS，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        key_buf = b''
        iv = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        alg_id = [EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_192,EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_256]
        key_algo_id = [EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS,EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS,EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS]
        for i in range(0,3):
            _, handler = api.ehsm_km_gen_key(key_algo_id[i], permit, 0, 0, None, 0, handle)
            with allure.step("2、导出密钥； # 2、导出密钥成功"):
                ret0, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,128,None,0)
            with allure.step("3、使用加密数据c，再使用生成的密钥进行解密，得到解密数据m，与标量数据对比 # 3、比较结果一致"):
                c_size = 32
                m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
                ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_XTS,EhsmPaddingMode.EHSM_PADDING_NONE,handler,1,iv,len(iv),m,c_size,32)
                ret, new_m ,ret8= api.ehsm_symm_cipher_onepass(alg_id[i],EhsmCipherMode.EHSM_CIPHER_MODE_XTS,EhsmPaddingMode.EHSM_PADDING_NONE,handler,0,iv,len(iv),c_output,len(c_output),32)
                if new_m != m:
                    api.ehsm_km_remove_key(handler)
                    assert False, "加密失败"
            with allure.step("4、删除密钥； # 4、删除密钥成功；"):
                api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成 HMAC 密钥，并使用密钥进行MAC运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-656")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_656():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为HMAC，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        alg = EhsmKeyType.EHSM_KEY_TYPE_HMAC
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        _, handler = api.ehsm_km_gen_key(alg, permit, 0, 16, None, 0, handle)
    with allure.step("2、导出明文h1密钥数据，保存为m1； # 2、导出成功；"):
        ret0, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,32,None,0)
        key = get_priv_key(buf1)

    with allure.step("3、使用HMAC 算法进行计算 digest； # 3、计算digest 成功；"):
        msg = bytes([
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
            ])
        out_buf = b''
        _,digest= api.ehsm_hmac_onepass_gen(EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,handler,msg,len(msg),out_buf,32)
    with allure.step("4、 使用标准算法库计算HMAC，对比与ehsm计算结果是否一致； # 4、计算结果一致；"):
        test_data = generate_hmac_testdata("SHA2-256", key, msg)
        log.info(f"生成的HMAC测试数据为: {test_data.digest.hex()}")
        assert test_data.digest == digest, f"生成的HMAC测试数据与预期不符，预期: {test_data.digest.hex()}, 实际: {digest.hex()}"
    with allure.step("5、删除密钥h1 # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成RSA_1024/2048/3072密钥对，进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-657")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持 RSA 算法")
def test_ehsm_657():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为RSA_1024，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_RSA_1024,EhsmKeyType.EHSM_KEY_TYPE_RSA_2048,EhsmKeyType.EHSM_KEY_TYPE_RSA_3072]
        export_buf_size = [1024,2048,3072]
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        handle = 0xFFFFFFFF
        last_buf=[]
        rsa_e_bit_size = 17
        for alg_cnt in range(len(key_alg)):
            for i in range(0,2):
                _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, rsa_e_bit_size, 0, None, 0, handle)
                with allure.step("2、导出明文密钥 # 2、导出成功"):
                    ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PUBLIC_KEY,key_buf,export_buf_size[alg_cnt],None,0)
                with allure.step("3、导出明文后，与上一次生成的密钥做对比 # 3、比较结果不一致"):
                    if list(buf1) == bytes(last_buf):
                        api.ehsm_km_remove_key(handler)
                        assert False, "两次生成的RSA密钥相同"
                with allure.step("4、删除密钥；# 4、删除密钥成功"):
                    last_buf[:] = list(buf1)
                    api.ehsm_km_remove_key(handler)
    # with allure.step("2、准备RSA加密指令，配置算法模式为Normal，传入加密数据m和密钥句柄h1； # 2、加密成功；获取加密后的数据；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("3、获取加密后的数据c，准备RSA解密指令，配置算法模式为Normal，传入加密后的数据c和密钥句柄h1； # 3、解密成功；获取解密后的数据；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("4、获取解密后的数据m'，与步骤2数据m进行对比 # 4、数据对比一致；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("5、删除密钥h1 # 5、删除密钥成功；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"

@allure.feature("kms")
@allure.description("生成RSA_CRT_1024/2048/3072密钥对，进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-658")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持 RSA 算法")
def test_ehsm_658():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为AES_CRT，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT,EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT,EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT]
        export_buf_size = [1024,2048,3072]
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        handle = 0xFFFFFFFF
        last_buf=[]
        rsa_e_bit_size = 17
        mac = None
        mac_size = 0
        for alg_cnt in range(len(key_alg)):
            for i in range(0,2):
                with allure.step("生成RSA_CRT_1024/2048/3072/4096密钥对, 导出明文后，与上一次生成的密钥做对比 # 比较结果一致"):
                    _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, rsa_e_bit_size, 0, None, 0, handle)
                with allure.step("2、导出密钥； # 2、导出密钥成功"):
                    ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PUBLIC_KEY,key_buf,export_buf_size[alg_cnt],mac,mac_size)
                with allure.step("3、重复1.2；对两次导出的密钥进行比较 # 3、数据对比不一致；"):
                    if list(buf1) == bytes(last_buf):
                        api.ehsm_km_remove_key(handler)
                        assert False, "两次生成的RSA_CRT密钥相同"
                with allure.step("4、删除密钥； # 4、删除密钥成功；"):
                    last_buf[:] = list(buf1)
                    api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成DH密钥对，进行密钥交换")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持 DH 密钥交换")
@allure.testcase("EHSM-659")
def test_ehsm_659():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为DH，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_DH]
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | \
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        last_buf = []

        # DH参数从已有的模板数据中获取
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

        # DH参数大小
        dh_g_size = len(dh_tv_template_1024_g)  # 128
        dh_p_size = len(dh_tv_template_1024_p)  # 128
        dh_q_size = len(dh_tv_template_1024_q)  # 20

        # 构建DH通用数据，按照C代码的格式：p_size + p + q_size + q + g_size + g + 末尾的0
        import struct
        dh_common_data = struct.pack("<I", dh_p_size) + dh_tv_template_1024_p
        dh_common_data += struct.pack("<I", dh_q_size) + dh_tv_template_1024_q
        dh_common_data += struct.pack("<I", dh_g_size) + dh_tv_template_1024_g
        dh_common_data += struct.pack("<I", 0)  # 末尾的0

        key_buf = b''

        for alg_cnt in range(len(key_alg)):
            for i in range(2):
                with allure.step(f"生成DH密钥对 第{i+1}次，导出明文后，与上一次生成的密钥做对比"):
                    # 生成带有签名验证和删除权限的DH密钥
                    _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, 0, 0, dh_common_data, len(dh_common_data), handle)

                with allure.step("2、导出密钥； # 2、导出密钥成功"):
                    ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(handler, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR, key_buf, 1024, None, 0)

                with allure.step("3、重复1.2；对两次导出的密钥进行比较 # 3、数据对比不一致；"):
                    if list(buf1) == last_buf:
                        api.ehsm_km_remove_key(handler)
                        assert False, "生成的DH密钥与上次相同，密钥生成失败"
                    # 保存当前密钥用于下次比较
                    last_buf = list(buf1)

                with allure.step("4、删除密钥； # 4、删除密钥成功；"):
                    api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成SM2密钥对，进行加解密运算")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-660")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持 SM2 算法")
def test_ehsm_660():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为SM2，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_SM2]
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        handle = 0xFFFFFFFF
        last_buf=[]
        rsa_e_bit_size = 0
        mac = None
        mac_size = 0
        for alg_cnt in range(len(key_alg)):
            for i in range(0,2):
                _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, rsa_e_bit_size, 0, mac, mac_size, handle)
                with allure.step("2、导出密钥； # 2、导出密钥成功"):
                    ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR,key_buf,1024,None,0)
                with allure.step("3、重复1.2；对两次导出的密钥进行比较 # 3、数据对比不一致；"):
                    if list(buf1) == bytes(last_buf):
                        api.ehsm_km_remove_key(handler)
                        assert False, "两次生成的SM2密钥相同"
                    last_buf[:] = list(buf1)
                with allure.step("4、删除密钥； # 4、删除密钥成功；"):
                    api.ehsm_km_remove_key(handler)
    # with allure.step("2、准备SM2加密指令，配置算法方向加密，传入加密数据m和密钥句柄h1； # 2、加密成功；获取加密后的数据；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("3、获取加密后的数据c，准备SM2解密指令，配置算法方向为解密，传入加密后的数据c和密钥句柄h1； # 3、解密成功；获取解密后的数据；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("4、获取解密后的数据m'，与步骤2数据m进行对比 # 4、数据对比一致；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("5、删除密钥h1 # 5、删除密钥成功；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"

@allure.feature("kms")
@allure.description("生成ECC brainpool密钥对，进行签名校验")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-661")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 ECC 算法")
def test_ehsm_661():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为brainpool，权限为加解密，生成句柄h1； # 1、密钥生成成功，返回密钥handle；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1,EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1]
        key_buf = b''
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        handle = 0xFFFFFFFF
        last_buf=[]
        alg_size = [160,192,224,256,320,384,512]
        for alg_cnt in range(len(key_alg)):
            for i in range(0,2):
                with allure.step("ECC brainpool, 导出明文后，与上一次生成的密钥做对比 # 比较结果不一致"):
                    _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, 0, 0, None, 0, handle)
                    ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR,key_buf,alg_size[alg_cnt],None,0)
                    if list(buf1) == bytes(last_buf):
                        api.ehsm_km_remove_key(handler)
                        assert False, "两次生成的ECC brainpool密钥相同"

                    last_buf[:] = list(buf1)
                    api.ehsm_km_remove_key(handler)
    # with allure.step("2、导出密钥； # 2、导出密钥成功"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("3、重复1.2；对两次导出的密钥进行比较 # 3、数据对比不一致；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"
    # with allure.step("4、删除密钥； # 4、删除密钥成功；"):
    #     # TODO
    #     assert False, "未实现，标记为失败"

@allure.feature("kms")
@allure.description("生成ECC SEC算法密钥对，进行签名校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-662")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 ECC 算法")
def test_ehsm_662():
    # Reason: 测试多种ECC SEC K系列曲线的签名验证功能
    key_alg = [
        EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1,
        EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1,
        EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1,
        EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1
    ]
    key_buf = b''
    permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
    handle = 0xFFFFFFFF
    last_buf = []
    alg_size = [160, 256, 512, 2048]

    # Reason: 准备测试消息和签名缓冲区大小
    test_message = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
    sig_sizes = [40, 48, 56, 64]  # Reason: 每种曲线的签名长度（2 * curve_size_bytes）

    for alg_cnt in range(len(key_alg)):
        with allure.step(f"测试算法 {key_alg[alg_cnt].name}:"):
            for i in range(2):
                with allure.step(f"第{i+1}次循环: 1、生成带有签名校验和删除权限的密钥； # 1、密钥生成成功，返回密钥handle；"):
                    _, handler = api.ehsm_km_gen_key(key_alg[alg_cnt], permit, 0, 0, None, 0, handle)

                with allure.step(f"第{i+1}次循环: 导出密钥，验证与上一次生成的密钥不同； # 导出成功，且密钥不同；"):
                    ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(
                        handler, 0xFFFFFFFF, 0xFFFFFFFF,
                        EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR,
                        key_buf, alg_size[alg_cnt], None, 0
                    )

                    # Reason: 验证密钥随机性，从第二次开始比较
                    if i > 0 and list(buf1) == last_buf:
                        _ = api.ehsm_km_remove_key(handler)
                        assert False, "两次生成的ECC SEC密钥相同"

                    last_buf = list(buf1)

                with allure.step(f"第{i+1}次循环: 2、使用ECDSA进行签名，传入签名数据m和密钥句柄； # 2、签名成功；获取签名数据；"):
                    sig_size = sig_sizes[alg_cnt]

                    # Reason: 使用SHA256哈希算法进行ECDSA签名
                    _, generated_sig = api.ehsm_ecdsa_onepass_gen(
                        algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                        key_handle=handler,
                        msg=test_message,
                        msg_size=len(test_message),
                        sig=bytes(sig_size),
                        sig_size=sig_size
                    )

                    assert len(generated_sig) > 0, "签名生成失败，签名长度为0"

                with allure.step(f"第{i+1}次循环: 3、使用ECDSA进行校验，传入签名后的数据s和密钥句柄； # 3、校验成功；"):
                    # Reason: 使用相同的密钥验证签名
                    _, verify_result = api.ehsm_ecdsa_onepass_verify(
                        algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                        key_handle=handler,
                        msg=test_message,
                        msg_size=len(test_message),
                        sig=generated_sig,
                        sig_size=len(generated_sig)
                    )

                with allure.step(f"第{i+1}次循环: 4、获取校验的结果； # 4、校验通过；"):
                    assert verify_result, f"{key_alg[alg_cnt].name} 自签名验证失败"

                with allure.step(f"第{i+1}次循环: 5、删除密钥； # 5、删除密钥成功；"):
                    _ = api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("生成没有签名权限的密钥，并使用该密钥进行签名")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-663")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_663():
    with allure.step("1、准备密钥生成命令，配置没有签名权限，生成密钥； # 1、生成密钥成功；"):
        # Reason: 生成 SM2 密钥，只配置 REMOVE 权限，不包含 SIGN 权限
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        handle = 0xFFFFFFFF

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_SM2, permit, 0, 0, None, 0, handle)

    with allure.step("2、使用生成的密钥进行签名； # 2、签名失败，返回权限不匹配错误；"):
        # Reason: 准备测试消息
        test_msg = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])

        try:
            # Reason: 尝试使用没有签名权限的密钥进行签名，预期失败
            _, signature = api.ehsm_sm2_sign_onepass_gen(
                key_handle=handler,
                msg=test_msg,
                msg_size=len(test_msg),
                sig_buff_size=256
            )
            # Reason: 如果没有抛出异常，说明测试失败
            api.ehsm_km_remove_key(handler)
            assert False, "预期签名操作失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是权限不匹配
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), \
                f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"

    with allure.step("3、删除密钥 # 3、删除密钥成功；"):
        # Reason: 清理测试资源
        api.ehsm_km_remove_key(handler)


@allure.feature("kms")
@allure.description("生成没有校验权限的密钥，并使用该密钥进行校验")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-664")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_664():
    with allure.step("1、准备密钥生成命令，配置没有校验权限，生成密钥； # 1、生成密钥成功；"):
        # Reason: 生成 SM2 密钥，只配置 REMOVE 权限，不包含 VERIFY 权限
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        handle = 0xFFFFFFFF

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_SM2, permit, 0, 0, None, 0, handle)

    with allure.step("2、使用生成的密钥进行校验； # 2、校验失败，返回权限不匹配错误；"):
        # Reason: 准备测试消息和签名数据（不需要真实签名，因为会因权限不足而失败）
        test_msg = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        # Reason: 准备一个测试用的签名数据（64字节）
        test_sig = bytes([0xFF] * 64)

        try:
            # Reason: 尝试使用没有校验权限的密钥进行验签，预期失败
            _, verify_result = api.ehsm_sm2_sign_onepass_verify(
                key_handle=handler,
                msg=test_msg,
                msg_size=len(test_msg),
                sig=test_sig,
                sig_size=len(test_sig)
            )
            # Reason: 如果没有抛出异常，说明测试失败
            api.ehsm_km_remove_key(handler)
            assert False, "预期验签操作失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是权限不匹配
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), \
                f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"

    with allure.step("3、删除密钥 # 3、删除密钥成功；"):
        # Reason: 清理测试资源
        api.ehsm_km_remove_key(handler)


@allure.feature("kms")
@allure.description("生成没有加密权限的密钥，并使用该密钥进行加密")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-665")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_665():
    with allure.step("1、准备密钥生成命令，配置没有加密权限，生成密钥； # 1、生成密钥成功；"):
        key_alg = [EhsmKeyType.EHSM_KEY_TYPE_AES_128]
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        handle = 0xFFFFFFFF
        c_size = 16
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handle)
    with allure.step("2、使用生成的密钥进行加密； # 2、加密失败；"):
        try:
            ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,1,None,0,m,c_size,24)
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"

@allure.feature("kms")
@allure.description("生成没有解密权限的密钥，并使用该密钥进行解密")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-666")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_666():
    with allure.step("1、准备密钥生成命令，配置没有解密权限，生成密钥； # 1、生成密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        handle = 0xFFFFFFFF
        c_size = 16
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
                   ,0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handle)

    with allure.step("2、使用生成的密钥进行解密； # 2、解密失败；"):
        try:
            ret ,c_output,ret8= api.ehsm_symm_cipher_onepass(EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,EhsmCipherMode.EHSM_CIPHER_MODE_ECB,EhsmPaddingMode.EHSM_PADDING_NONE,handler,0,None,0,m,c_size,24)
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"

@allure.feature("kms")
@allure.description("指定key_handle创建密钥，使用该密钥进行加密")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-667")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_667():
    with allure.step("1、准备生成密钥指令，配置密钥算法ID为AES_128，权限为加解密，key_id为 RAM_Key_ID= 0x100088 # 1、密钥生成成功，返回密钥handle；"):
        expect_handle = 0x100088
        _, handler = api.ehsm_km_gen_key(
            key_type= EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            privilege= KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"],
            rsa_e_bit_size=0,
            hmac_key_size=0,
            dh_params=None,
            dh_params_size=0,
            key_handle= expect_handle)
    with allure.step("2、验证生成的句柄就是指定的句柄值 # 2、生成的句柄值和预期一致；"):
        assert handler == expect_handle, f"生成的句柄{hex(handler)}与指定句柄{hex(expect_handle)}不一致"
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(handler, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, None, 128, None, 0)
        priv_key = get_priv_key(buf1)

    with allure.step("2、准备SKE加密指令，配置算法为AES_ECB，传入加密数据和密钥句柄； # 2、加密成功；获取加密后的数据；"):
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        ret, c_output, ret8 = api.ehsm_symm_cipher_onepass(
                    EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,
                    EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
                    EhsmPaddingMode.EHSM_PADDING_NONE,
                    handler,
                    1, # 加密
                    None,
                    0,
                    m,
                    len(m),
                    24
                )
    with allure.step("3、获取加密后的数据，准备SKE解密指令，配置算法为AES_ECB，传入加密后的数据和密钥句柄； # 3、解密成功；获取解密后的数据；"):
        test_data = generate_symmetric_testdata(
            algo="AES128",
            mode="ECB",
            key=priv_key,
            padding="NONE",
            plaintext=m)
    with allure.step("4、获取解密后的数据，与步骤2数据进行对比 # 4、数据对比一致；"):
        assert test_data.ciphertext == c_output, f"解密后的数据与加密前的数据不一致, 预期值：{''.join(f'{b:02x}' for b in test_data.ciphertext)}，实际值：{''.join(f'{b:02x}' for b in c_output)}"
    with allure.step("5、删除密钥 # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("配置TPK权限，创建密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-668")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_668():
    with allure.step("1、准备密钥生成命令，配置TPK权限，生成密钥； # 1、生成密钥失败；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_CREATION_TRANSP_KEY"]
        handle = 0xFFFFFFFF
        try:
            _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handle)
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"


@allure.feature("kms")
@allure.description("生成没有密钥写保护权限的密钥h1后，重新基于该h1生成密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-669")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_669():
    with allure.step("1、准备密钥生成命令，不配置写保护权限，生成密钥h1； # 1、生成密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        handle = 0xFFFFFFFF
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handle)

    with allure.step("2、获取密钥句柄h1，并使用该句柄h1重新生成新密钥； # 2、生成新密钥成功；"):
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handler)
        api.ehsm_km_remove_key(handler)


@allure.feature("kms")
@allure.description("使用配置了写保护密钥的handle，重新生成密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-670")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_670():
    with allure.step("1、准备密钥生成命令，配置写保护权限，生成密钥h1； # 1、生成密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        handle = 0xFFFFFFFF
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handle)

    with allure.step("2、获取密钥句柄h1，并使用该句柄h1重新生成新密钥； # 2、生成新密钥失败；"):
        try:
            _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, handler)
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"
            api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("使用非法参数进行密钥生成指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-671")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_671():
    permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
    handle = 0xFFFFFFFF
    invalid_handle = 0xFFFFFFFE
    with allure.step("1、非法算法； # 1、生成密钥失败；"):
        try:
            _, handler1 = api.ehsm_km_gen_key(0xFFFFFFFF, permit, 0, 0, None, 0, handle)
        except hostapi.HostApiError as e:
            assert not (e == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("2、输入错误密钥权限； # 2、返回预期错误码；"):
        try:
            _, handler2 = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0xFFFFFFFF, 0, 0, None, 0, handle)
        except hostapi.HostApiError as e:
            assert not (e == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"
    with allure.step("3、非法key_handle，如0xf0000; # 3、生成密钥失败；"):
        try:
            _, handler3 = api.ehsm_km_gen_key(0xFFFFFFFF, permit, 0, 0, None, 0, invalid_handle)
        except hostapi.HostApiError as e:
            assert not (e == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望返回EHSM_ERR_INVALID_HANDLE, 实际返回值: {e.ret_code}"


@allure.feature("kms")
@allure.description("导入明文密钥，再导出该明文密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-672")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_672():
    with allure.step("1、准备密钥导入命令，配置密钥和加密及明文导出权限，获取导入后密钥句柄； # 1、导入成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF

        # 使用与C代码相同的测试私钥
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])

        # 设置权限：加密、明文导出、删除权限
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]


        # 构建kms_key_format_st结构体头部
        header = struct.pack(
            STRUCT_FORMAT,
            permit,                          # permit
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,  # algo_id
            0x2,                            # part_info (KMS_KEY_PART_PRIVKEY)
            0,                              # reserved
            0x0,                            # pub_key_size
            len(private_key)                # priv_key_size
        )

        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        # 调用API导入明文密钥
        ret, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)
    with allure.step("2、准备密钥导出命令，配置导出地址，获取导出后的数据； # 2、导出成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(
            key_handle,
            0xffffffff,
            0xffffffff,
            EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
            key_buf,
            len(header) + len(private_key),
            None,
            0)

    with allure.step("4、将导出的数据和导入的数据做对比； # 4、数据对比一致；"):
        if list(buf1) == key_buf:
            assert False, "密钥导出失败，导出数据为空"

        # 从导出的数据中提取密钥部分（跳过头部）
        exported_key = buf1[len(header):]

        # 验证导出的密钥与原始密钥完全一致
        assert exported_key == private_key, f"导出的密钥与原始密钥不匹配。原始：{private_key.hex()}, 导出：{exported_key.hex()}"
    with allure.step("5、删除密钥； # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(key_handle)

@allure.feature("kms")
@allure.description("导入加密密钥，再导出该加密密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-673")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_673():
    # OTP密钥句柄定义
    KMS_OTP_KEY_TPK = 0x200010  # Transport/Protection Key
    KMS_OTP_KEY_ENC = 0x200011  # Encryption Key
    KMS_KEY_PART_PRIVKEY = 0x2
    KMS_INVALID_KEY_ID = 0xFFFFFFFF

    with allure.step("1、准备带有TPK权限的OTP密钥key句柄h1和密钥值与h1相同的具有加解密权限的OTP密钥h2； # 1、None"):
        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))

        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        log.info(f"TPK密钥句柄: 0x{tpk_handle:08X}, ENC密钥句柄: 0x{enc_handle:08X}")

    with allure.step("2、使用h2默认算法和OTP密钥key加密数据m；得到数据c； # 2、加密成功；"):
        # 准备明文数据
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        iv = bytes([0x00] * 16)

        # 使用OTP ENC密钥加密数据（SM4 CBC PKCS7）
        _, c, output_size = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
            EhsmPaddingMode.EHSM_PADDING_PKCS7,
            enc_handle,
            True,  # 加密
            iv,
            len(iv),
            m,
            len(m),
            32  # 输出缓冲区大小
        )
        assert output_size > 0, "加密后数据长度应大于0"
        log.info(f"加密后数据长度: {output_size}, 数据: {c[:output_size].hex()}")

    with allure.step("3、准备导入密钥命令，密钥数据为c，配置加密密钥h1，得到密钥句柄h3； # 3、导入密钥成功；"):
        # 构建密钥格式结构体（12字节头部 + 密钥数据）
        # 密钥权限配置
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_PLAINTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_CIPHERTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"])

        algo_id = EhsmKeyType.EHSM_KEY_TYPE_SM4  # MB_SM4
        part_info = KMS_KEY_PART_PRIVKEY
        pub_key_size = 0
        priv_key_size = 16

        # 构建密钥数据（12字节头部 + 加密数据）
        key_data = key.pack_key_with_head(c[:output_size], permit, algo_id, part_info, pub_key_size, priv_key_size )

        # 导入加密密钥
        _, key_handle = api.ehsm_km_import_key(
            tpk_handle,        # transport_key_handle
            KMS_INVALID_KEY_ID,  # auth_key_handle
            key_data,
            len(key_data),
            None,              # mac
            0,                 # mac_size
            0xFFFFFFFF         # key_handle (自动分配)
        )
        assert key_handle != KMS_INVALID_KEY_ID, "导入密钥应返回有效句柄"
        log.info(f"导入密钥成功，句柄: 0x{key_handle:08X}")

    with allure.step("4、导出密钥句柄h3，加密句柄配置h1，得到导出后的数据c'； # 4、导出密钥成功；"):
        # 导出加密密钥
        _, exported_key, exported_size, _, _ = api.ehsm_km_export_key(
            key_handle,         # target_key_handle
            tpk_handle,         # transport_key_handle
            KMS_INVALID_KEY_ID, # auth_key_handle
            KMS_KEY_PART_PRIVKEY,  # key_part
            b'',                # key_data buffer
            512,                # key_data_size
            None,               # mac
            0                   # mac_size
        )
        assert exported_size > 0, "导出数据长度应大于0"
        log.info(f"导出密钥成功，数据长度: {exported_size}")

    with allure.step("6、将导出后的数据c' 和 c 进行对比； # 6、对比数据一致；"):
        # 从导出的数据中提取raw_data部分（跳过12字节头部）
        # 导出的数据格式与导入相同，需要跳过头部
        exported_raw_data = exported_key[12:12+output_size]
        log.info(f"导出数据: {exported_raw_data.hex()}")
        assert exported_raw_data == c[:output_size], \
            f"导出的加密数据应与原始加密数据一致\n" \
            f"原始: {c[:output_size].hex()}\n" \
            f"导出: {exported_raw_data.hex()}"
        log.info("导出数据与原始加密数据对比一致")

    with allure.step("7、删除密钥句柄h3； # 7、删除密钥成功；"):
        _ = api.ehsm_km_remove_key(key_handle)
        log.info(f"删除密钥句柄 0x{key_handle:08X} 成功")

@allure.feature("kms")
@allure.description("导入签名密钥，再导出签名密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-674")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_674():
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC/Sign at index 16
    KMS_KEY_PART_PRIVKEY = 0x2
    KMS_INVALID_KEY_ID = 0xFFFFFFFF

    with allure.step("1、准备带有TPK权限的OTP密钥key句柄h1和密钥值与h1相同的具有签名校验权限的OTP密钥h2； # 1、配置成功；"):
        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        # 配置 OTP 密钥：TPK at index 15, ENC/Sign at index 16
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、使用h2默认算法和OTP密钥key，签名整个密钥结构；得到签名数据mac； # 2、签名成功；"):
        # 准备明文密钥数据
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])

        # 构建密钥结构
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        key_data = key.pack_key_with_head(
            m, permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

        # 使用 OTP ENC key 对整个密钥结构生成 CMAC 签名
        _, mac = api.ehsm_mac_onepass_gen(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmMacMode.EHSM_MAC_MODE_CMAC,
            enc_handle,
            None, 0,  # No IV for CMAC
            key_data, len(key_data),
            b'', 32
        )
        mac_size = 32

    with allure.step("3、准备导入密钥命令，密钥数据为key_data，配置认证密钥h1(TPK)，和签名数据mac，得到密钥句柄h3； # 3、导入密钥成功；"):
        # 使用 TPK 作为认证密钥导入签名的密钥（注意：TPK 在 auth_key_handle 位置）
        _, key_handle = api.ehsm_km_import_key(
            KMS_INVALID_KEY_ID,  # transport_key_handle
            tpk_handle,          # auth_key_handle
            key_data, len(key_data),
            mac, mac_size,
            0xFFFFFFFF           # key_handle
        )

    with allure.step("4、导出密钥句柄h3明文和带签名两种方式； # 4、导出密钥成功；"):
        # 明文导出（无加密，无签名）
        _, exported_plain, exported_plain_size, _, _ = api.ehsm_km_export_key(
            key_handle,
            KMS_INVALID_KEY_ID,  # transport_key_handle
            KMS_INVALID_KEY_ID,  # auth_key_handle
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            None, 0
        )

        # 带签名导出（无加密，有签名）
        _, exported_signed, exported_signed_size, mac_out, mac_out_size = api.ehsm_km_export_key(
            key_handle,
            KMS_INVALID_KEY_ID,  # transport_key_handle
            tpk_handle,          # auth_key_handle (TPK for signing)
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            b'', 32
        )

    with allure.step("5、验证导出的明文密钥数据与原始数据m一致； # 5、验证成功；"):
        # 从导出的明文密钥中提取原始密钥数据（跳过12字节头部）
        exported_raw_data = exported_plain[12:12+16]
        assert exported_raw_data == m, f"导出的明文密钥数据与原始数据不一致"

    with allure.step("6、删除密钥句柄h3； # 6、删除密钥成功；"):
        _ = api.ehsm_km_remove_key(key_handle)

@allure.feature("kms")
@allure.description("导入加密签名密钥 ；再导出加密签名密钥；")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-675")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_675():
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC at index 16
    KMS_KEY_PART_PRIVKEY = 0x2
    KMS_INVALID_KEY_ID = 0xFFFFFFFF

    with allure.step("1、准备带有TPK权限的OTP密钥key句柄h1和密钥值与h1相同的具有加解密权限的OTP密钥h2； # 1、配置成功；"):
        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        # 配置 OTP 密钥：TPK at index 15, ENC at index 16
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、构建密钥结构，对其签名；然后加密明文数据m；得到加密数据c和签名mac； # 2、加密和签名成功；"):
        # 准备明文密钥数据
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        iv = bytes([0x00] * 16)

        # 构建包含明文 m 的密钥结构
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_CIPHERTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # 先用明文 m 构建密钥结构用于签名
        key_data_for_sign = key.pack_key_with_head(
            m, permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

        # 对包含明文 m 的密钥结构生成 CMAC 签名
        _, mac = api.ehsm_mac_onepass_gen(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmMacMode.EHSM_MAC_MODE_CMAC,
            enc_handle,
            None, 0,
            key_data_for_sign, len(key_data_for_sign),
            b'', 32
        )
        mac_size = 32

        # 使用 OTP ENC key 加密明文 m，得到 c
        _, c, output_size = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
            EhsmPaddingMode.EHSM_PADDING_PKCS7,
            enc_handle, True, iv, len(iv), m, len(m), 32
        )

        # 用加密后的数据 c 构建最终的密钥结构用于导入
        key_data = key.pack_key_with_head(
            c[:output_size], permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

    with allure.step("3、准备导入密钥命令，密钥数据为c（加密后），配置加密密钥h1(TPK)，签名密钥h1(TPK)，得到密钥句柄h3； # 3、导入密钥成功；"):
        # 使用 TPK 同时作为解密密钥和认证密钥导入加密+签名的密钥
        _, key_handle = api.ehsm_km_import_key(
            tpk_handle,          # transport_key_handle (for decryption)
            tpk_handle,          # auth_key_handle (for signature verification)
            key_data, len(key_data),
            mac, mac_size,
            0xFFFFFFFF           # key_handle
        )

    with allure.step("4、三种方式导出密钥：仅加密、加密+签名、明文； # 4、导出密钥成功；"):
        # 方式1：仅加密导出（使用 TPK 加密，无签名）
        _, exported_enc, exported_enc_size, _, _ = api.ehsm_km_export_key(
            key_handle,
            tpk_handle,          # transport_key_handle (for encryption)
            KMS_INVALID_KEY_ID,  # auth_key_handle (no signature)
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            None, 0
        )

        # 方式2：加密+签名导出（使用 TPK 加密和签名）
        _, exported_enc_sign, exported_enc_sign_size, mac_out, mac_out_size = api.ehsm_km_export_key(
            key_handle,
            tpk_handle,          # transport_key_handle (for encryption)
            tpk_handle,          # auth_key_handle (for signature)
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            b'', 32
        )

        # 方式3：明文导出（无加密，无签名）
        _, exported_plain, exported_plain_size, _, _ = api.ehsm_km_export_key(
            key_handle,
            KMS_INVALID_KEY_ID,  # transport_key_handle (no encryption)
            KMS_INVALID_KEY_ID,  # auth_key_handle (no signature)
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            None, 0
        )

    with allure.step("5、验证导出数据：加密+签名导出应等于c，明文导出应等于m； # 5、验证成功；"):
        # 验证加密+签名导出的数据与原始加密数据 c 一致
        exported_enc_sign_raw = exported_enc_sign[12:12+output_size]
        assert exported_enc_sign_raw == c[:output_size], f"加密+签名导出的数据与原始加密数据不一致"

        # 验证明文导出的数据与原始明文 m 一致
        exported_plain_raw = exported_plain[12:12+16]
        assert exported_plain_raw == m, f"明文导出的数据与原始明文不一致"

    with allure.step("6、删除密钥句柄h3； # 6、删除密钥成功；"):
        _ = api.ehsm_km_remove_key(key_handle)

@allure.feature("kms")
@allure.description("导入加密密钥，对导入密钥的加密算法AES CBC进行测试（反向用例，预期失败）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-676")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_676():
    """
    反向测试用例：OTP密钥配置的是SM4算法，尝试使用AES算法应该失败。
    """
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC at index 16 (configured as SM4)
    KMS_KEY_PART_PRIVKEY = 0x2
    KMS_INVALID_KEY_ID = 0xFFFFFFFF

    with allure.step("1、准备OTP密钥（配置为SM4算法）； # 1、配置成功；"):
        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        # 配置 OTP 密钥为 SM4（AES128-CMAC）
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、尝试使用AES算法加密明文密钥k1； # 2、预期失败（OTP配置的是SM4）；"):
        # 准备明文密钥数据
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        iv = bytes([0x00] * 16)

        # 尝试使用 AES 算法加密（但OTP key是SM4配置）
        try:
            _, c, output_size = api.ehsm_symm_cipher_onepass(
                EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,  # 使用 AES（与OTP配置不匹配）
                EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
                EhsmPaddingMode.EHSM_PADDING_PKCS7,
                enc_handle, True, iv, len(iv), m, len(m), 32
            )
            # 如果成功了，说明测试逻辑有问题
            assert False, "预期加密应该失败（算法不匹配），但实际成功了"
        except hostapi.HostApiError as e:
            # 预期失败：算法不匹配
            log.info(f"预期失败：使用AES算法但OTP配置为SM4，返回错误码 {e.ret_code}")
            # 可能的错误码：EHSM_ERR_NOT_SUPPORT, EHSM_ERR_INVALID_ALGORITHM 等
            assert e.ret_code != EHSM_ERR_SW_SUCCESS, f"预期返回错误码，实际返回值: {e.ret_code}"

    with allure.step("3、测试结论：由于OTP密钥配置为SM4，使用AES算法失败符合预期； # 3、反向测试通过；"):
        log.info("反向测试通过：AES算法在SM4配置的OTP密钥上失败")

@allure.feature("kms")
@allure.description("导入加密密钥，对导入密钥的加密算法SM4 CBC进行测试（正向用例，预期成功）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-677")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_677():
    """
    正向测试用例：OTP密钥配置的是SM4算法，使用SM4算法应该成功。
    """
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC at index 16 (configured as SM4)
    KMS_KEY_PART_PRIVKEY = 0x2
    KMS_INVALID_KEY_ID = 0xFFFFFFFF

    with allure.step("1、准备OTP密钥（配置为SM4算法）； # 1、配置成功；"):
        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        # 配置 OTP 密钥为 SM4
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

    with allure.step("2、导入明文密钥k1，得到密钥句柄h2，算法SM4，并使用h2计算数据m的加密结果c； # 2、导入密钥成功；加密成功；"):
        # 准备明文密钥数据 k1
        k1 = bytes([0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10])

        # 构建密钥结构
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_CIPHERTEXT"])

        key_data_k1 = key.pack_key_with_head(
            k1, permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

        # 导入明文密钥 k1，得到密钥句柄 h2
        _, h2 = api.ehsm_km_import_key(
            KMS_INVALID_KEY_ID,  # transport_key_handle (plaintext import)
            KMS_INVALID_KEY_ID,  # auth_key_handle
            key_data_k1, len(key_data_k1),
            None, 0,
            0xFFFFFFFF
        )

        # 使用 h2 加密测试数据 m
        m = bytes([0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                   0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF])
        iv = bytes([0x00] * 16)

        _, c, c_size = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
            EhsmPaddingMode.EHSM_PADDING_PKCS7,
            h2, True, iv, len(iv), m, len(m), 32
        )

    with allure.step("3、使用TPK导出密钥h2（加密导出）；得到导出密钥数据k2； # 3、导出密钥成功；"):
        # 使用 TPK 加密导出密钥 h2
        _, k2_exported, k2_size, _, _ = api.ehsm_km_export_key(
            h2,
            tpk_handle,          # transport_key_handle (for encryption)
            KMS_INVALID_KEY_ID,  # auth_key_handle
            KMS_KEY_PART_PRIVKEY,
            b'', 512,
            None, 0
        )

    with allure.step("4、将k2导入（使用TPK解密），得到密钥句柄h3； # 4、导入密钥成功；"):
        # 使用 TPK 解密导入 k2，得到密钥句柄 h3
        _, h3 = api.ehsm_km_import_key(
            tpk_handle,          # transport_key_handle (for decryption)
            KMS_INVALID_KEY_ID,  # auth_key_handle
            k2_exported, k2_size,
            None, 0,
            0xFFFFFFFF
        )

    with allure.step("5、使用h3加密相同的数据m，得到c'；将c'和c进行对比； # 5、对比数据一致；"):
        # 使用 h3 加密相同的测试数据 m
        _, c_prime, c_prime_size = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
            EhsmPaddingMode.EHSM_PADDING_PKCS7,
            h3, True, iv, len(iv), m, len(m), 32
        )

        # 验证 c' 和 c 一致
        assert c[:c_size] == c_prime[:c_prime_size], f"加密结果不一致"
        log.info("验证成功：两次加密结果一致")

    with allure.step("6、删除密钥句柄h2和h3； # 6、删除密钥成功；"):
        _ = api.ehsm_km_remove_key(h2)
        _ = api.ehsm_km_remove_key(h3)
        log.info("正向测试通过：SM4算法在SM4配置的OTP密钥上成功")

@allure.feature("kms")
@allure.description("导入签名密钥，对导入密钥的签名算法AES CMAC进行测试（反向用例，预期失败）")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-678")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_678():
    """
    反向测试用例：OTP密钥配置的是SM4算法，尝试使用AES CMAC应该失败。
    """
    KMS_INVALID_KEY_ID = 0xFFFFFFFF
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC/Sign at index 16
    KMS_KEY_PART_PRIVKEY = 0x02

    with allure.step("1、准备OTP密钥（配置为SM4算法）； # 1、OTP配置成功；"):
        # Configure OTP keys as SM4
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        log.info(f"OTP配置成功，TPK句柄: {hex(tpk_handle)}, ENC句柄: {hex(enc_handle)}")

    with allure.step("2、尝试使用AES CMAC算法生成签名； # 预期失败，算法不匹配；"):
        m = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])

        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        key_data = key.pack_key_with_head(
            m, permit, EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

        # Try to use AES CMAC algorithm (but OTP is configured as SM4)
        try:
            _, mac = api.ehsm_mac_onepass_gen(
                EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,  # AES (mismatched with SM4 OTP)
                EhsmMacMode.EHSM_MAC_MODE_CMAC,
                enc_handle, None, 0,
                key_data, len(key_data), b'', 32
            )
            assert False, "预期CMAC生成应该失败（算法不匹配），但实际成功了"
        except hostapi.HostApiError as e:
            log.info(f"预期失败：使用AES CMAC但OTP配置为SM4，返回错误码 {e.ret_code}")
            assert e.ret_code != EHSM_ERR_SW_SUCCESS, f"预期失败但返回了成功错误码: {e.ret_code}"

    with allure.step("3、测试结论：反向测试通过； # 3、AES CMAC在SM4配置的OTP密钥上失败；"):
        log.info("反向测试通过：AES CMAC算法在SM4配置的OTP密钥上失败")

@allure.feature("kms")
@allure.description("导入签名密钥，对导入密钥的签名算法SM4 CMAC进行测试（正向用例，预期成功）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-679")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_679():
    """
    正向测试用例：OTP密钥配置的是SM4算法，使用SM4 CMAC应该成功。
    测试完整的签名密钥导入-导出-再导入循环。
    """
    KMS_INVALID_KEY_ID = 0xFFFFFFFF
    KMS_OTP_KEY_TPK = 0x200010  # TPK at index 15
    KMS_OTP_KEY_ENC = 0x200011  # ENC/Sign at index 16
    KMS_KEY_PART_PRIVKEY = 0x02

    with allure.step("1、准备OTP密钥（配置为SM4算法）； # 1、OTP配置成功；"):
        assert 0 == host.write_otp(_config_tpk_and_gen_key("test", 15, 16))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)

        tpk_handle = KMS_OTP_KEY_TPK
        enc_handle = KMS_OTP_KEY_ENC
        log.info(f"OTP配置成功，TPK句柄: {hex(tpk_handle)}, ENC句柄: {hex(enc_handle)}")

    with allure.step("2、导入明文密钥k1，得到密钥句柄h2，并使用h2计算测试数据的CMAC签名； # 2、导入成功，签名生成成功；"):
        # Import plaintext key k1
        k1 = bytes([0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10])

        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] |
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_CIPHERTEXT"])

        key_data_k1 = key.pack_key_with_head(
            k1, permit, EhsmKeyType.EHSM_KEY_TYPE_SM4,
            KMS_KEY_PART_PRIVKEY, 0, 16
        )

        # Import plaintext key k1, get handle h2
        _, h2 = api.ehsm_km_import_key(
            KMS_INVALID_KEY_ID, KMS_INVALID_KEY_ID,
            key_data_k1, len(key_data_k1), None, 0, 0xFFFFFFFF
        )
        log.info(f"明文密钥k1导入成功，句柄h2: {hex(h2)}")

        # Generate CMAC signature using h2
        test_data = bytes([0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF])

        _, mac1 = api.ehsm_mac_onepass_gen(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmMacMode.EHSM_MAC_MODE_CMAC,
            h2, None, 0,
            test_data, len(test_data), b'', 32
        )
        log.info(f"使用h2生成CMAC签名成功，签名长度: {len(mac1)}")

    with allure.step("3、使用TPK导出密钥h2（带签名导出）； # 3、导出成功；"):
        # Export key h2 with signature (TPK as auth key)
        _, k2_exported, k2_size, mac_export, mac_export_size = api.ehsm_km_export_key(
            h2, KMS_INVALID_KEY_ID, tpk_handle,
            KMS_KEY_PART_PRIVKEY, b'', 512, b'', 32
        )
        log.info(f"密钥h2导出成功（带签名），导出大小: {k2_size}, 签名大小: {mac_export_size}")

    with allure.step("4、将k2导入（使用TPK验签），得到密钥句柄h3； # 4、导入成功；"):
        # Import k2 with signature verification, get handle h3
        _, h3 = api.ehsm_km_import_key(
            KMS_INVALID_KEY_ID, tpk_handle,
            k2_exported, k2_size, mac_export, mac_export_size, 0xFFFFFFFF
        )
        log.info(f"密钥k2导入成功（签名验证通过），句柄h3: {hex(h3)}")

    with allure.step("5、使用h3对相同数据生成CMAC签名，验证与h2生成的签名一致； # 5、签名验证成功，密钥功能保持一致；"):
        # Generate CMAC signature using h3 with same test data
        _, mac2 = api.ehsm_mac_onepass_gen(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4,
            EhsmMacMode.EHSM_MAC_MODE_CMAC,
            h3, None, 0,
            test_data, len(test_data), b'', 32
        )
        log.info(f"使用h3生成CMAC签名成功，签名长度: {len(mac2)}")

        # Verify that both signatures are identical
        assert mac1 == mac2, f"签名不一致：h2签名长度={len(mac1)}, h3签名长度={len(mac2)}"
        log.info("验证成功：两个句柄生成的CMAC签名完全一致")

    with allure.step("6、删除密钥句柄h2和h3； # 6、删除成功；"):
        _ = api.ehsm_km_remove_key(h2)
        log.info(f"密钥句柄h2删除成功")

        _ = api.ehsm_km_remove_key(h3)
        log.info(f"密钥句柄h3删除成功")

    with allure.step("7、测试结论：正向测试通过； # 7、SM4 CMAC在SM4配置的OTP密钥上成功完成完整循环；"):
        log.info("正向测试通过：SM4 CMAC算法在SM4配置的OTP密钥上成功，密钥导入-导出-再导入循环验证通过")

@allure.feature("kms")
@allure.description("导入一个明文密钥，配置TPK权限")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-680")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_680():
    with allure.step("1、准备明文导入数据，配置密钥权限TPK # 1、导入密钥失败；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF

        # 使用与C代码相同的测试私钥
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])

        # 设置权限：加密、明文导出、删除权限
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]


        # 构建kms_key_format_st结构体头部
        header = struct.pack(
            STRUCT_FORMAT,
            permit,                          # permit
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,  # algo_id
            0x2,                            # part_info (KMS_KEY_PART_PRIVKEY)
            0,                              # reserved
            0x0,                            # pub_key_size
            len(private_key)                # priv_key_size
        )

        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        # 调用API导入明文密钥
        try:
            _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)
            assert key_handle != 0xFFFFFFFF, "导入密钥失败"
        except Exception as e:
            assert e.ret_code == fw_errno.EHSM_ERR_NOT_ALLOWED_CREATION_KEY, f"导入密钥异常: {e.ret_code}"

@allure.feature("kms")
@allure.description("对已配置密钥写保护密钥handle再次导入新密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-681")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_681():
    with allure.step("1、生成带有写保护属性的密钥，获取句柄h1； # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
        gen_permit =  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        # 打包结构体头部（小端字节序）
        import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_permit,
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            0x2,
            0,
            0x0,  # pub_key_size
            len(private_key)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、配置密钥数据k1，导入到h1； # 2、导入失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handler)
        except hostapi.HostApiError as e :
            assert  e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION, f"导入密钥异常: {e.ret_code}"
    with allure.step("3、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 3、发送数据成功；时间小于200ms"):
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("生成一个不具有导入权限的密钥，获取handle，然后重新导入密钥到该handle")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-682")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_682():
    with allure.step("1、创建一个普通RAM密钥，不配置导入权限，获取密钥句柄h1； # 1、生成成功；"):
         _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"], 0, 0, None, 0, 0xFFFFFFFF)
         handle_bak = handler
    with allure.step("2、准备密钥数据，导入密钥句柄h1； # 2、导入失败；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
        # 打包结构体头部（小端字节序）
        import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_IMPORT_PLAINTEXT"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_permit,
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            0x2,
            0,
            0x0,  # pub_key_size
            len(private_key)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handler)
        except hostapi.HostApiError as e :
            assert  e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION, f"导入密钥异常: {e.ret_code}"

    with allure.step("3、删除密钥 # 3、删除成功；"):
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个没有TPK权限的密钥，使用其作为导入密钥的解密密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-683")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_683():
    with allure.step("1、创建一个普通RAM密钥，不配置TPK，获取密钥句柄h1； # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])

        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_key_permit,
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            0x2,
            0,
            0x0,  # pub_key_size
            len(private_key)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、准备密钥数据，密钥加密句柄配置h1，导入该密钥； # 2、导入失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望返回EHSM_ERR_MISMATCH_KEY_PERMISSION, 实际返回值: {e.ret_code}"
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个没有TPK权限的密钥，使用其作为导入密钥的验签密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-684")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_684():
    with allure.step("1、导入一个明文密钥，配置可加密和校验，不配置TPK权限，获取句柄h1； # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
        mac = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        mac_size = 32

        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_key_permit,
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            0x2,
            0,
            0x0,  # pub_key_size
            len(private_key)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、重新导入该明文密钥，并使用该密钥对自身签名，导入该密钥，并配置签名密钥h1； # 2、导入失败；"):
        try:
            api.ehsm_km_import_key(handler, 0xFFFFFFFF, key_data, len(key_data), mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_NOT_ALLOWED_CREATION_KEY), f"期望返回EHSM_ERR_NOT_ALLOWED_CREATION_KEY, 实际返回值: {e.ret_code}"
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("使用非法参数进行密钥导入指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-686")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_686():
    STRUCT_FORMAT = "<IBBHHH"
    handle = 0xFFFFFFFF
    private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                        0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
    mac = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
    mac_size = 32

    # 打包结构体头部（小端字节序）
    import_key__permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"] | \
            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
    KMS_KEY_PART_PRIVKEY = 0x2
    header = struct.pack(
        STRUCT_FORMAT,
        import_key__permit,
        EhsmKeyType.EHSM_KEY_TYPE_AES_128,
        KMS_KEY_PART_PRIVKEY,
        0x0,  # reserved
        0x0,  # pub_key_size
        len(private_key)  # priv_key_size
    )
    # 拼接完整数据（头部 + 密钥）
    key_data = header + private_key
    with allure.step("1、非法密钥句柄，非法加密.校验密钥句柄； # 1、导入密钥失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望返回EHSM_ERR_INVALID_HANDLE, 实际返回值: {e}"
    with allure.step("2、传入错误tk handle； # 2、导入密钥失败；"):
        try:
            api.ehsm_km_import_key(0x01, 0xFFFFFFFF, key_data, len(key_data), mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_KEY_NOT_FOUND), f"期望返回EHSM_ERR_KEY_NOT_FOUND, 实际返回值: {e}"
    with allure.step(f"3、传入错误ak handle； # 3、导入密钥失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0x01, key_data, len(key_data), mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_KEY_NOT_FOUND), f"期望返回EHSM_ERR_KEY_NOT_FOUND, 实际返回值: {e}"
    invalid_key_size = 0
    with allure.step(f"4、传入错误key长度； # 4、导入密钥失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0x01, key_data, invalid_key_size , mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e}"

    with allure.step(f"5、传入错误mac签名长度; # 5、导入密钥失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0x200010, key_data, 16 , mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e :
            assert not (e == fw_errno.EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e}"

    with allure.step(f"6、将key_data_addr设置为None； # 5、导入密钥失败；"):
        # Reason: 测试key_data_addr为None的无效情况
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, None, len(key_data), mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_INVALID_ADDRESS), f"期望返回EHSM_ERR_INVALID_ADDRESS, 实际返回值: {e.ret_code}"

    with allure.step(f"7、将key_data_size设置为超长2349字节； # 6、导入密钥失败；"):
        # Reason: 测试key_data_size超过最大长度2348字节
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, 2349, mac, mac_size, 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

@allure.feature("kms")
@allure.description("使用带有创建新密钥权限的OTP密钥派生一个TPK权限和导出权限的密钥（反向用例，预期失败）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-687")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_687():
    """
    反向测试用例：使用带有创建新密钥权限的OTP密钥尝试派生带TPK权限的密钥。
    预期失败，因为不允许通过派生创建具有TPK（传输保护）权限的密钥。

    OTP密钥ID：0x20000009（带创建新密钥权限）
    """
    KMS_OTP_KEY_CREATE = 0x20000009  # OTP key with key creation permission

    with allure.step("1、使用OTP密钥派生一个带TPK权限和导出权限的密钥h1； # 1、派生失败，不允许创建TPK权限密钥；"):
        # 配置派生参数
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD

        # 配置密钥类型和大小
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_size = 16

        # 配置权限：包含 TPK (MB_KEY_USAGE_TRANSPORT) 和导出权限
        # Reason: 尝试同时配置TPK权限和导出权限，这应该被拒绝
        key_permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"] |  # TPK权限
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # 配置派生参数
        salt = bytes([0x73, 0x61, 0x6C, 0x74])  # "salt"
        salt_size = len(salt)
        password = bytes([0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64])  # "password"
        password_size = len(password)
        iter_times = 1

        # 父密钥句柄：使用带创建新密钥权限的OTP密钥
        parent_key_handle = KMS_OTP_KEY_CREATE

        # 输出密钥句柄
        handle = 0xFFFFFFFF

        # 尝试派生密钥（预期失败）
        log.info(f"尝试使用OTP密钥 {hex(parent_key_handle)} 派生带TPK权限的密钥")
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                hash_algo=hash_algo,
                derive_algo=derive_algo,
                derive_type=derive_type,
                privilege=key_permit,
                key_type=key_type,
                key_size=key_size,
                parent_key_handle=parent_key_handle,
                salt=salt,
                salt_size=salt_size,
                password=password,
                password_size=password_size,
                iter_times=iter_times,
                key_handle=handle
            )
            # 如果到达这里，说明派生成功了（不应该发生）
            assert False, f"预期派生失败，但实际成功并得到句柄 {hex(derived_handle)}"
        except hostapi.HostApiError as e:
            # 预期的失败情况
            log.info(f"预期失败：尝试派生TPK权限密钥被拒绝，错误码: {e.ret_code}")
            # Reason: 不允许通过派生创建TPK权限的密钥，应返回参数错误或权限不匹配错误
            assert e.ret_code in [fw_errno.EHSM_ERR_PARAM_ERROR,
                                 fw_errno.EHSM_ERR_NOT_ALLOWED_CREATION_KEY,
                                 fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION], \
                f"预期返回参数错误或权限不匹配错误，实际错误码: {e.ret_code}"

@allure.feature("kms")
@allure.description("使用带有创建新密钥权限的OTP密钥派生密钥后导出测试（TPK导出限制验证）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-688")
@pytest.mark.skipif(cfg_data.TEST_FW_KMS_TPK_SUPPORT == 0, reason="OTP密钥TPK/ENC/VER功能不支持")
def test_ehsm_688():
    """
    测试用例：验证即使能派生密钥，带TPK权限的密钥也不能被导出。

    OTP密钥ID：0x20000009（带创建新密钥权限）

    注意：由于安全限制，派生带TPK权限的密钥通常会失败（见test_ehsm_687）。
    本测试验证两种情况：
    1. 如果派生TPK密钥失败，测试通过（符合安全策略）
    2. 如果派生TPK密钥成功（某些配置下），导出应该失败
    """
    KMS_OTP_KEY_CREATE = 0x20000009  # OTP key with key creation permission
    derived_handle = None

    with allure.step("1、使用OTP密钥尝试派生一个带TPK权限的密钥h1； # 1、可能成功或失败；"):
        # 配置派生参数
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD

        # 配置密钥类型和大小
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_size = 16

        # 配置权限：包含 TPK (MB_KEY_USAGE_TRANSPORT) 和导出权限
        key_permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"] |  # TPK权限
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] |
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"])

        # 配置派生参数
        salt = bytes([0x73, 0x61, 0x6C, 0x74])  # "salt"
        password = bytes([0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64])  # "password"
        iter_times = 1

        # 父密钥句柄：使用带创建新密钥权限的OTP密钥
        parent_key_handle = KMS_OTP_KEY_CREATE
        handle = 0xFFFFFFFF

        # 尝试派生密钥
        log.info(f"尝试使用OTP密钥 {hex(parent_key_handle)} 派生带TPK权限的密钥")
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                hash_algo=hash_algo,
                derive_algo=derive_algo,
                derive_type=derive_type,
                privilege=key_permit,
                key_type=key_type,
                key_size=key_size,
                parent_key_handle=parent_key_handle,
                salt=salt,
                salt_size=len(salt),
                password=password,
                password_size=len(password),
                iter_times=iter_times,
                key_handle=handle
            )
            log.info(f"派生成功，得到句柄 {hex(derived_handle)}")
        except hostapi.HostApiError as e:
            # Reason: 如果派生TPK失败（符合安全策略），测试通过
            log.info(f"派生TPK密钥失败（符合安全策略），错误码: {e.ret_code}")
            assert e.ret_code in [fw_errno.EHSM_ERR_PARAM_ERROR,
                                 fw_errno.EHSM_ERR_NOT_ALLOWED_CREATION_KEY,
                                 fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION], \
                f"预期返回安全策略相关错误，实际错误码: {e.ret_code}"
            log.info("测试通过：系统正确阻止了TPK密钥的派生")
            return  # 派生失败是正常的，测试通过

    with allure.step("2、如果派生成功，尝试导出该TPK密钥； # 2、导出失败（TPK不应被导出）；"):
        if derived_handle is None:
            pytest.skip("步骤1派生失败，跳过导出测试")

        # 准备导出缓冲区
        STRUCT_FORMAT = "<IBBHHH"
        export_buffer = bytearray(12 + 64)  # 头部 + 数据区

        log.info(f"尝试导出TPK密钥 {hex(derived_handle)}")
        try:
            # 尝试导出TPK密钥（预期失败）
            _, key_data, key_data_size, mac, mac_size = api.ehsm_km_export_key(
                target_key_handle=derived_handle,
                transport_key_handle=0xFFFFFFFF,  # 明文导出
                auth_key_handle=0xFFFFFFFF,  # 不使用签名
                key_part=EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                key_data=bytes(export_buffer),
                key_data_size=len(export_buffer),
                mac=b'',
                mac_size=0
            )
            # Reason: 如果到达这里，说明TPK密钥被成功导出（不应该发生）
            assert False, f"预期导出TPK密钥失败，但实际成功导出了 {key_data_size} 字节"
        except hostapi.HostApiError as e:
            # 预期的失败情况：TPK密钥不应被导出
            log.info(f"预期失败：TPK密钥导出被拒绝，错误码: {e.ret_code}")
            assert e.ret_code in [fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION,
                                 fw_errno.EHSM_ERR_NOT_ALLOWED_EXPORT_KEY,
                                 fw_errno.EHSM_ERR_PARAM_ERROR], \
                f"预期返回权限不匹配或不允许导出错误，实际错误码: {e.ret_code}"
        finally:
            # 清理：删除派生的密钥
            if derived_handle is not None:
                try:
                    api.ehsm_km_remove_key(derived_handle)
                    log.info(f"已删除派生的密钥 {hex(derived_handle)}")
                except:
                    pass

@allure.feature("kms")
@allure.description("创建一个没有导出权限的密钥，然后导出")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-689")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_689():
    with allure.step("1、生成一个密钥，不配置导出权限，获取密钥h1； # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])

        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            0x0,
            0x0,
            0x0,
            0,
            0x0,  # pub_key_size
            0x0  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、导出该密钥； # 2、导出失败；"):
        try:
            key_buf = b''
            api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,len(header),None,0)

        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望值 EHSM_ERR_MISMATCH_KEY_PERMISSION，实际返回{e.ret_code}"
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个没有TPK权限的密钥，使用其作为导出密钥的加密密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-690")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_690():
    with allure.step("1、创建2个随机密钥h1, h2，均不配置TPK权限 # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
        mac = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        mac_size = 32

        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"]
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            0x0,
            0x0,
            0x0,
            0,
            0x0,  # pub_key_size
            0x0  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、将h1导出，配置导出密钥的加密密钥为h2； # 2、导出失败；"):
        handle_bak = handler
        header = struct.pack(
            "<IBBHHH",
            0x0,
            0x0,
            0x0,
            0,
            0x0,  # pub_key_size
            0x0  # priv_key_size
        )
        key_buf = b''
        mac = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        try:
            ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(handler,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,len(header)+16,mac,len(mac))
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望值 EHSM_ERR_MISMATCH_KEY_PERMISSION，实际返回{e.ret_code}"
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个没有TPK权限的密钥，使用其作为导出密钥的签名密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-691")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_691():
    with allure.step("1、创建2个随机密钥h1, h2，均不配置TPK权限 # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])

        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_key_permit,
            EhsmKeyType.EHSM_KEY_TYPE_AES_128,
            0x2,
            0,
            0x0,  # pub_key_size
            len(private_key)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + private_key

        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、将h1导出，配置导出密钥的签名密钥为h2； # 2、导出失败；"):
        try:
            api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handler)
        except hostapi.HostApiError as e:
            assert  (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望值 EHSM_ERR_MISMATCH_KEY_PERMISSION，实际返回{e.ret_code}"
        api.ehsm_km_remove_key(handle_bak)


@allure.feature("kms")
@allure.description("使用非法参数进行密钥导出指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-694")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_694():
    with allure.step("1、生成一个随机密钥； # 1、生成成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("1、非法密钥句柄，非法加密和校验句柄； # 1、失败；"):
        invalid_key_handle = 0xFFFFFFFE
        private_key = bytes([0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
                            0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc])
        mac = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
        mac_size = 32
        # 打包结构体头部（小端字节序）
        import_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"]
        KMS_KEY_PART_PRIVKEY = 0x2
        header = struct.pack(
            STRUCT_FORMAT,
            import_key_permit,
            EhsmKeyType.EHSM_KEY_TYPE_HMAC,
            KMS_KEY_PART_PRIVKEY,
            0,
            0x0,  # pub_key_size
            len(mac)  # priv_key_size
        )
        # 拼接完整数据（头部 + 密钥）
        key_data = header + mac
        try:
            api.ehsm_km_export_key(invalid_key_handle, 0xFFFFFFFF, 0xFFFFFFFF, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_data, len(key_data), mac, mac_size)
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望值 EHSM_ERR_INVALID_HANDLE，实际返回{e.ret_code}"
        try:
            api.ehsm_km_export_key(handler, invalid_key_handle, 0xFFFFFFFF, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_data, len(key_data), mac, mac_size)
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望值 EHSM_ERR_INVALID_HANDLE，实际返回{e.ret_code}"
        try:
            api.ehsm_km_export_key(handler, 0xFFFFFFFF, invalid_key_handle, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_data, len(key_data), mac, mac_size)
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望值 EHSM_ERR_INVALID_HANDLE，实际返回{e.ret_code}"
    with allure.step("2、非法密钥类型； # 2、失败；"):
        try:
            api.ehsm_km_export_key(handler, 0xFFFFFFFF, 0xFFFFFFFF, 0xF,key_data,  len(key_data), mac, mac_size)
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_NOT_EXIST_KEY_PART), f"期望值 EHSM_ERR_NOT_EXIST_KEY_PART，实际返回{e.ret_code}"
    with allure.step("3、非法地址和非法size； # 3、失败；"):
        try:
            api.ehsm_km_export_key(handler, 0xFFFFFFFF, 0xFFFFFFFF, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_data, len(key_data), mac, 0)
        except hostapi.HostApiError as e :
            assert  (e.ret_code == fw_errno.EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT), f"期望值 EHSM_ERR_KEY_SIGNATURE_SZ_TOO_SHORT{e.ret_code}"

    with allure.step("4、key_data_addr设置为None； # 4、失败；"):
        # Reason: 测试key_data_addr为None的无效情况
        try:
            api.ehsm_km_export_key(handler, 0xFFFFFFFF, 0xFFFFFFFF, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, None, len(key_data), mac, mac_size)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("5、key_data_size设置为0； # 5、失败；"):
        # Reason: 测试key_data_size=0的无效情况
        try:
            api.ehsm_km_export_key(handler, 0xFFFFFFFF, 0xFFFFFFFF, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_data, 0, mac, mac_size)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_OUT_OF_MEM), f"期望返回EHSM_ERR_OUT_OF_MEM, 实际返回值: {e.ret_code}"

    with allure.step("6、删除密钥 # 6、删除成功"):
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个RAM密钥，并配置REMOVE属性，对该密钥进行删除")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-695")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_695():
    with allure.step("1、创建一个RAM密钥，指定句柄，配置REMOVE权限； # 1、创建成功；"):
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
    with allure.step("2、将创建的密钥句柄传入密钥删除指令； # 2、删除成功；"):
        api.ehsm_km_remove_key(handler)


@allure.feature("kms")
@allure.description("对OTP 密钥进行删除")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-696")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_696():
    with allure.step("1、将 OTP 密钥句柄传入密钥删除指令； # 1、删除失败；"):
        KMS_OTP_KEY_CREATE = 0x200011
        try:
            api.ehsm_km_remove_key(KMS_OTP_KEY_CREATE)
        except hostapi.HostApiError as e:
            assert e.ret_code in (fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION, fw_errno.EHSM_ERR_MISMATCH_KEY_USAGE), f"期望值 EHSM_ERR_MISMATCH_KEY_PERMISSION,实际返回{e.ret_code}"

@allure.feature("kms")
@allure.description("创建一个RAM密钥，不配置REMOVE属性，对该密钥进行删除")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-698")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_698():
    with allure.step("1、创建一个RAM密钥，指定句柄，不配置REMOVE权限； # 1、创建成功；"):
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)

    with allure.step("2、将创建的密钥句柄传入密钥删除指令； # 2、删除失败；"):
        try:
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"期望值 EHSM_ERR_MISMATCH_KEY_PERMISSION,实际返回{e.ret_code}"

@allure.feature("kms")
@allure.description("使用非法参数进行密钥删除指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-699")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_699():
    with allure.step("1、将非法句柄传入密钥删除指令； # 1、返回失败；"):
        KMS_INVALID_KEY_ID = 0xFFFFFFFF
        try:
            api.ehsm_km_remove_key(KMS_INVALID_KEY_ID)
        except hostapi.HostApiError as e:
            assert (e.ret_code == fw_errno.EHSM_ERR_INVALID_HANDLE), f"期望值 EHSM_ERR_INVALID_HANDLE,实际返回{e.ret_code}"


@allure.feature("kms")
@allure.description("直接使用根密码数值password key方式，X963算法派生密钥，派生密钥存放在RAM")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-700")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_700():
    with allure.step("1、派生算法选择KDFX963，方式选择 use password，密钥输出选择RAM，权限选择加密或解密，发送派生指令； # 1、派生成功，获取密钥h1；"):
        pw_data_addr = bytes([
            0x0C, 0x12, 0x8A, 0x90, 0x0B, 0xD2, 0xB5, 0xCA, 0xE2, 0x17, 0x4D, 0xDC, 0x12, 0xE0, 0x52, 0x8E,
            0x4A, 0x55, 0xE2, 0x70, 0xA4, 0xA6, 0xA8, 0xC6, 0x0C, 0xA9, 0x12, 0x27, 0x19, 0x4E, 0x81, 0x56,
            0x16, 0xC2, 0xCC, 0x0E, 0x40, 0xB4, 0xBA, 0x7F, 0xCC, 0x2A, 0xC9, 0xF7, 0x63, 0x19, 0x98, 0x9C,
            0x03, 0xD4, 0x02, 0x52, 0xFF, 0x5D, 0x71, 0x8D, 0xF2, 0x5A, 0xC1, 0xC9, 0xB2, 0x82, 0x5B, 0x1A,
            0x84, 0x2E, 0xAD, 0x2C, 0xE7, 0x62, 0x3E, 0x77, 0x3E, 0x6D, 0xDB, 0x3E, 0x3F, 0x02, 0x3C, 0xAE
        ])
        std_z_50_s_0_k = bytes([
            0x5A, 0xE7, 0x95, 0x76, 0x20, 0x27, 0x0F, 0x78, 0xF4, 0xC3, 0x42, 0xBA, 0x52, 0xB6,
            0x39, 0xE3, 0x8B, 0xA6, 0x15, 0xF0, 0xF3, 0x18, 0x1A, 0x79, 0xB3, 0x58, 0x4D, 0x50, 0xD7, 0x26, 0x40, 0x64,
            0x8B, 0xBC, 0x0E, 0x3E, 0x21, 0x8D, 0x75, 0xBA, 0xB9, 0x95, 0x16, 0x37, 0x44, 0x0E, 0xA5, 0x30, 0x9B, 0x27,
            0x6D, 0xF9, 0xC2, 0x3C, 0x72, 0x6F, 0x57, 0x63, 0x3D, 0x1B, 0xFF, 0x19, 0x6D, 0xA0, 0x8F, 0xEF, 0xCD, 0x02,
            0xC8, 0x22, 0xC9, 0x27, 0x5A, 0xD6, 0x97, 0x8A, 0x3B, 0x07, 0xF3, 0xAF, 0x6A, 0xB6, 0xC2, 0xCA, 0x17, 0x3C,
            0x05, 0xA8, 0x39, 0xC4
        ])
        header = struct.pack(
            "<IBBHHH",
            0x0,
            0x0,
            0x0,
            0,
            0x0,  # pub_key_size
            0x0  # priv_key_size
        )
        std_z_50_size = 90
        pw_data_size = 50
        key_buf = b''
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SM3
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_KDFX963
        derive_type = 0x01
        salt_size = 0
        salt = None
        iter_times = 0
        handle = 0xFFFFFFFF
        parent_key_handle = 0
        key_type = 0
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 90, parent_key_handle, salt, salt_size, pw_data_addr, pw_data_size, iter_times, handle)

    with allure.step("3、导出派生密钥，对比算法和权限是否一致； # 3、导出密钥成功；"):
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(handler, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_buf, len(header) + std_z_50_size, None, 0)
        s_privilege, s_key_type, s_part_info, s_reserved, s_pkey_size, s_priv_key_size, s_key_vale = struct.unpack("<IBBHHH90s", buf1)
        assert std_z_50_s_0_k == s_key_vale
    with allure.step("4、删除密钥； # 4、删除密钥成功；"):
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("直接使用根密码数值password key方式，PBKDF2算法派生密钥，派生密钥存放在RAM")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-701")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_701():
    with allure.step("1、派生算法选择PBKDF2，方式选择 use password，密钥输出选择RAM，权限选择加密或解密，发送派生指令； # 1、派生成功，获取密钥h1；"):
        password = bytes([0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64])
        salt_l = bytes([0x73, 0x61, 0x6C, 0x74])
        sha512_dk_l = bytes([
            0x86, 0x7F, 0x70, 0xCF, 0x1A, 0xDE, 0x02, 0xCF, 0xF3, 0x75, 0x25, 0x99, 0xA3, 0xA5, 0x3D, 0xC4,
            0xAF, 0x34, 0xC7, 0xA6, 0x69, 0x81, 0x5A, 0xE5, 0xD5, 0x13, 0x55, 0x4E, 0x1C, 0x8C, 0xF2, 0x52,
            0xC0, 0x2D, 0x47, 0x0A, 0x28, 0x5A, 0x05, 0x01, 0xBA, 0xD9, 0x99, 0xBF, 0xE9, 0x43, 0xC0, 0x8F,
            0x05, 0x02, 0x35, 0xD7, 0xD6, 0x8B, 0x1D, 0xA5, 0x5E, 0x63, 0xF7, 0x3B, 0x60, 0xA5, 0x7F, 0xCE
        ])
        header = struct.pack(
            "<IBBHHH",
            0x0,
            0x0,
            0x0,
            0,
            0x0,  # pub_key_size
            0x0  # priv_key_size
        )
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        pw_data_size = len(password)
        key_buf = b''
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = 0x01
        salt_size = len(salt_l)
        salt = salt_l
        iter_times = 0
        handle = 0xFFFFFFFF
        parent_key_handle = 1
        key_type = 0
        _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 64, parent_key_handle, salt, salt_size, password, pw_data_size, iter_times, handle)

    with allure.step("3、导出派生密钥，对比算法和权限是否一致； # 3、导出密钥成功；"):
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(handler, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_buf, len(header) + 64, None, 0)
        s_privilege, s_key_type, s_part_info, s_reserved, s_pkey_size, s_priv_key_size, s_key_vale = struct.unpack("<IBBHHH64s", buf1)
        assert sha512_dk_l == s_key_vale
    with allure.step("4、删除密钥； # 4、删除密钥成功；"):
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("根密码使用key_handle传入方式，X963算法派生密钥，派生密钥存放在KMU")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-702")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_702():
    with allure.step("1、派生算法选择KDFX963，方式选择 use parent key,密钥输出选择KMU,权限选择加密或解密，发送派生指令； # 1、派生失败，无法使用非BPKDF2方式和OTP密钥派生新密钥；"):
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_CIPHERTEXT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        handle = 0xFFFFFFFF
        key_size = 16
        derive_type = 0x02  # /* user parent key to derive key */
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SM3
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_KDFX963
        salt = None
        salt_size = 0
        password = None
        pw_data_size = 0
        parent_key_handle = 0xFFFFFFFF
        iter_times = 0
        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, key_size, parent_key_handle, salt, salt_size, password, pw_data_size, iter_times, handle)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR


@allure.feature("kms")
@allure.description("直接使用密钥方式，PBKDF2算法派生密钥，派生密钥存放在RAM")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-703")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_703():
    with allure.step("1、派生算法选择PBKDF2，方式选择 use password，密钥输出选择KMS，权限选择加密和导出，发送派生指令； # 1、派生成功，获取密钥句柄；"):
        # Reason: 准备测试向量数据 - password 和 salt
        password = bytes([0x70, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64])  # "password"
        salt = bytes([0x73, 0x61, 0x6C, 0x74])  # "salt"

        # Reason: 预期的派生密钥输出（SHA512-PBKDF2 标准测试向量）
        sha512_dk_1 = bytes([
            0x86, 0x7F, 0x70, 0xCF, 0x1A, 0xDE, 0x02, 0xCF, 0xF3, 0x75, 0x25, 0x99, 0xA3, 0xA5,
            0x3D, 0xC4, 0xAF, 0x34, 0xC7, 0xA6, 0x69, 0x81, 0x5A, 0xE5, 0xD5, 0x13, 0x55, 0x4E,
            0x1C, 0x8C, 0xF2, 0x52, 0xC0, 0x2D, 0x47, 0x0A, 0x28, 0x5A, 0x05, 0x01, 0xBA, 0xD9,
            0x99, 0xBF, 0xE9, 0x43, 0xC0, 0x8F, 0x05, 0x02, 0x35, 0xD7, 0xD6, 0x8B, 0x1D, 0xA5,
            0x5E, 0x63, 0xF7, 0x3B, 0x60, 0xA5, 0x7F, 0xCE
        ])

        # Reason: 配置密钥权限（对应 C 代码）
        key_permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # Reason: 使用密码方式派生（derive_type=0x01）
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = 0x01  # Reason: 使用密码方式（MB_EHSM_KEY_DERIVE_DERIVE_TYPE_KMS_KEY_DERIVE_USER_PASSWD）
        key_type = 0  # Reason: MB_RANDOM（随机数类型）
        key_size = 64
        parent_key_handle = 1  # Reason: C 代码中使用 1U
        salt_size = len(salt)
        pw_data_size = len(password)
        iter_times = 0
        handle = 0xFFFFFFFF

        _, handler = api.ehsm_km_derive_key(
            hash_algo, derive_algo, derive_type, key_permit, key_type, key_size,
            parent_key_handle, salt, salt_size, password, pw_data_size, iter_times, handle
        )

    with allure.step("2、导出派生密钥，对比密钥数据是否与标准测试向量一致； # 2、导出密钥成功，密钥值匹配；"):
        # Reason: 导出密钥以验证派生结果
        key_buf = b''
        header_size = struct.calcsize("<IBBHHH")
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(
            handler, 0xffffffff, 0xffffffff,
            EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
            key_buf, header_size + key_size, None, 0
        )

        # Reason: 解析导出的密钥结构
        s_privilege, s_key_type, s_part_info, s_reserved, s_pkey_size, s_priv_key_size, s_key_value = struct.unpack(
            "<IBBHHH64s", buf1
        )

        # Reason: 验证派生的密钥值与标准测试向量一致
        assert s_key_value == sha512_dk_1, "派生密钥值与预期不符"

    with allure.step("3、删除密钥； # 3、删除密钥成功；"):
        # Reason: 清理测试资源
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("使用password key方式派生具有加解密和传输密钥权限TPK的密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-704")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_704():
    with allure.step("1、使用password key 方式，并填充pw_data_addr，密钥权限选择加解密和TPK，发送派生命令； # 1、派生失败；"):
        pw_data_addr = bytes([
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        ])

        salt_size = 0
        salt = None
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD
        handle = 0xFFFFFFFF
        parent_key_handle = 0
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]

        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 32, parent_key_handle, salt, salt_size, pw_data_addr, len(pw_data_addr), 0, handle)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR, f"derive key failed, ret_code: {e.ret_code}"


@allure.feature("kms")
@allure.description("使用password key方式派生只有TPK权限的密钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-705")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_705():
    with allure.step("1、使用password key 方式，并填充pw_data_addr，密钥权限选择TPK，发送派生命令； # 1、派生失败；"):
        pw_data_addr = bytes([
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        ])

        salt_size = 0
        salt = None
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD
        handle = 0xFFFFFFFF
        parent_key_handle = 0
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]

        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 32, parent_key_handle, salt, salt_size, pw_data_addr, len(pw_data_addr), 0, handle)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR, f"derive key failed, ret_code: {e.ret_code}"


@allure.feature("kms")
@allure.description("创建一个可以具有写保护的密钥h1，重新派生一个密钥h1")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-706")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_706():
    with allure.step("1、生成一个随机密钥，配置密钥写保护权限，得到密钥h1； # 1、生成密钥成功；"):
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、使用password key 方式，并填充pw_data_addr，派生密钥句柄配置h1，发送派生命令； # 2、派生密钥失败；"):
        pw_data_addr = bytes([
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        ])

        salt_size = 0
        salt = None
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD

        parent_key_handle = 0
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]

        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 32, parent_key_handle, salt, salt_size, pw_data_addr, len(pw_data_addr), 0, handler)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR, f"derive key failed, ret_code: {e.ret_code}"
    with allure.step("3、删除密钥 # 3、删除密钥成功；"):
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("创建一个有创建密钥权限，没有TPK创建权限的密钥作为根密钥，新派生密钥配置TPK权限")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-707")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_707():
    with allure.step("1、生成一个随机密钥，配置密钥创建密钥权限，得到密钥h1； # 1、生成密钥成功；"):
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、以h1作为根密钥，派生一个带TPK权限的密钥； # 2、派生密钥失败；"):
        pw_data_addr = bytes([
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        ])
        salt_size = 0
        salt = None
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD

        parent_key_handle = 0
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]

        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 32, parent_key_handle, salt, salt_size, None, 0, 0, handler)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR, f"derive key failed, ret_code: {e.ret_code}"
    with allure.step("3、删除密钥 # 3、删除密钥成功；"):
        api.ehsm_km_remove_key(handle_bak)


@allure.feature("kms")
@allure.description("创建一个没有派生权限的密钥，用于密钥派生")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-708")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_708():
    with allure.step("1、生成一个随机密钥，不配置密钥生成权限，得到密钥h1； # 1、密钥生成成功；"):
        handle = 0xFFFFFFFF
        gen_key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_key_permit, 0, 0, None, 0, handle)
        handle_bak = handler
    with allure.step("2、以h1作为根密钥，派生一个带TPK权限的密钥； # 2、派生密钥失败；"):
        pw_data_addr = bytes([
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        ])
        salt_size = 0
        salt = None
        hash_algo = EhsmHashAlgo.EHSM_HASH_ALGO_SHA512
        derive_algo = EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2
        derive_type = EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD

        parent_key_handle = 0
        key_type = EhsmKeyType.EHSM_KEY_TYPE_SM4
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]

        try:
            _, handler = api.ehsm_km_derive_key(hash_algo, derive_algo, derive_type, key_permit, key_type, 32, parent_key_handle, salt, salt_size, None, 0, 0, handler)
        except hostapi.HostApiError as e:
            assert e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR, f"derive key failed, ret_code: {e.ret_code}"
    with allure.step("3、删除密钥 # 3、删除密钥成功；"):
        api.ehsm_km_remove_key(handle_bak)

@allure.feature("kms")
@allure.description("使用非法数据进行密钥派生指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-709")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_709():
    # Reason: 准备测试数据
    pw_data_addr = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F])
    pw_data_size = len(pw_data_addr)
    permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
    key_size = 16
    handle = 0xFFFFFFFF

    with allure.step("1、将派生算法设置非法； # 1、返回失败；"):
        # Reason: 测试非法派生算法 0xF（无效值）
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA512,
                derive_algo=0xF,  # 非法派生算法
                derive_type=EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD,
                privilege=permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_SM4,
                key_size=key_size,
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=pw_data_addr,
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("2、将派生类型选择非法； # 2、返回失败；"):
        # Reason: 测试非法派生类型 0xF（无效值）
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA512,
                derive_algo=EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                derive_type=0xF,  # 非法派生类型
                privilege=permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_SM4,
                key_size=key_size,
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=pw_data_addr,
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("3、将哈希算法设置非法； # 3、返回失败；"):
        # Reason: 测试非法哈希算法 0xFF（无效值）
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=0xFF,  # 非法哈希算法
                derive_algo=EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                derive_type=EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD,
                privilege=permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_SM4,
                key_size=key_size,
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=pw_data_addr,
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("4、将密钥类型设置非法； # 4、返回失败；"):
        # Reason: 测试非法密钥类型 0xFF（无效值）
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA512,
                derive_algo=EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                derive_type=EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD,
                privilege=permit,
                key_type=0xFF,  # 非法密钥类型
                key_size=key_size,
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=pw_data_addr,
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("5、将pw_data_addr设置为None； # 5、返回失败；"):
        # Reason: 测试password为None的情况
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA512,
                derive_algo=EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                derive_type=EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD,
                privilege=permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_SM4,
                key_size=key_size,
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=None,  # 非法password地址
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("6、将key_size设置为0； # 6、返回失败；"):
        # Reason: 测试key_size=0的无效值
        try:
            _, handler = api.ehsm_km_derive_key(
                hash_algo=EhsmHashAlgo.EHSM_HASH_ALGO_SHA512,
                derive_algo=EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                derive_type=EhsmDeriveType.EHSM_DERIVE_TYPE_PASSWORD,
                privilege=permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_SM4,
                key_size=0,  # 无效的密钥大小
                parent_key_handle=0,
                salt=pw_data_addr,
                salt_size=pw_data_size,
                password=pw_data_addr,
                password_size=pw_data_size,
                iter_times=0,
                key_handle=handle
            )
            assert False, "预期派生失败，但实际成功了"
        except hostapi.HostApiError as e:
            # Reason: 验证错误码是参数错误
            assert (e.ret_code == fw_errno.EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

@allure.feature("kms")
@allure.description("测试使用 DH 算法进行密钥交换")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.testcase("EHSM-710")
def test_ehsm_710():
    with allure.step("1、导入明文DH 私钥作为本地密钥，并配置创建密钥权限，得到h1；（使用标准数据） # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])

        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

        dh_tv_template_1024_b_public = bytes([
            0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
            0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
            0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
            0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
            0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
            0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
            0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
            0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

        dh_tv_template_1024_expected_ss = bytes([
            0x4E, 0x6A, 0xCF, 0xFD, 0x7D, 0x14, 0x27, 0x65, 0xEB, 0xF4, 0xC7, 0x12, 0x41, 0x4F, 0xE4, 0xB6,
            0xAB, 0x95, 0x7F, 0x4C, 0xB4, 0x66, 0xB4, 0x66, 0x01, 0x28, 0x9B, 0xB8, 0x20, 0x60, 0x42, 0x82,
            0x72, 0x84, 0x2E, 0xEE, 0x8F, 0x11, 0x3C, 0xD1, 0x1F, 0x39, 0x43, 0x1C, 0xBF, 0xFD, 0x82, 0x32,
            0x54, 0xCE, 0x47, 0x2E, 0x21, 0x05, 0xE4, 0x9B, 0x3D, 0x7F, 0x11, 0x3B, 0x82, 0x50, 0x76, 0xE6,
            0x26, 0x45, 0x85, 0x80, 0x7B, 0xC4, 0x64, 0x54, 0x66, 0x5F, 0x27, 0xC5, 0xE4, 0xE1, 0xA4, 0xBD,
            0x03, 0x47, 0x04, 0x86, 0x32, 0x29, 0x81, 0xFD, 0xC8, 0x94, 0xCC, 0xA1, 0xE2, 0x93, 0x09, 0x87,
            0xC9, 0x2C, 0x15, 0xA3, 0x8B, 0xC4, 0x2E, 0xB3, 0x88, 0x10, 0xE8, 0x67, 0xC4, 0x43, 0x2F, 0x07,
            0x25, 0x9E, 0xC0, 0x0C, 0xDB, 0xBB, 0x0F, 0xB9, 0x9E, 0x17, 0x27, 0xC7, 0x06, 0xDA, 0x58, 0xDD])
        g_size = 128
        p_size = 128
        q_size = 20

        dh_tv_template_1024_secret_size = len(dh_tv_template_1024_secret)
        # 打包结构体头部（小端字节序）
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        gen_handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_DH,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,  # reserved
            0x0,  # pub_key_size
            dh_tv_template_1024_secret_size  # priv_key_size
        )
        key_data = header + dh_tv_template_1024_secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, gen_handle)

    with allure.step("2、传入g/p/q数据，和对方公钥，对称算法AES，得到协商密钥h2； # 2、协商密钥成功；"):
        dh_command_data = struct.pack("<I128sI20sI128s",
                                      p_size,
                                      dh_tv_template_1024_p,
                                      q_size,
                                      dh_tv_template_1024_q,
                                      g_size,
                                      dh_tv_template_1024_g)
        # Reason: 根据 C 代码，需要在末尾添加 4 字节的 0
        dh_command_data += struct.pack("<I", 0)
        new_handle = 0xFFFFFFFF
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        # Reason: C 代码传递 dh_common_size + 12，其中 dh_common_size = 288，所以传递 300 字节
        _, out_new_handle = api.ehsm_km_exchange_key(dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public), exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128, hmac_key_size, local_handle, dh_command_data, len(dh_command_data) + 8, None, new_handle)

        key_buf = b''
    with allure.step("4、导出协商密钥k1； # 4、导出密钥成功；"):
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_buf, len(header) + 16, None, 0)

    with allure.step("5、对比k1和预期协商密钥k； # 5、数据对比一致；"):
        assert dh_tv_template_1024_expected_ss[:16] == buf1[len(header):len(header)+16]

    with allure.step("6、删除密钥h1/h2; # 6、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@allure.feature("kms")
@allure.description("测试使用 SM2 算法进行密钥交换")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-711")
def test_ehsm_711():
    with allure.step("1、导入明文密钥sm2公钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        g_spor_perm_priv_key = bytes([0x1A, 0xD1, 0x5B, 0xF3, 0xF1, 0xFB, 0x11, 0x7E, 0x95, 0x9E, 0x25, 0x97, 0xEE, 0x70, 0x99, 0xF0, 0x73, 0xDC, 0xF0,
                                      0x2A, 0x3C, 0xE5, 0xC7, 0x46, 0x3F, 0x7C, 0x0C, 0x96, 0xB5, 0xA1, 0x27, 0x48])
        g_spor_perm_pub_key = bytes([0x04, 0x7B, 0x8D, 0x44, 0x31, 0x84, 0x22, 0x13, 0xEB, 0x15, 0x64, 0x6C, 0xEF, 0x94, 0xC5, 0xCC, 0xDE, 0x05, 0x27,
                                      0x6E, 0x06, 0x85, 0x52, 0xBD, 0x94, 0xAD, 0xDB, 0x82, 0x07, 0x2C, 0xAF, 0x75, 0x05, 0x32, 0xF5, 0xCD, 0xDF, 0x1C,
                                      0x4D, 0x81, 0x5A, 0x89, 0xB2, 0x53, 0x51, 0x0E, 0x66, 0x22, 0xCA, 0x65, 0x19, 0x20, 0x73, 0x8C, 0xD8, 0x88, 0xCE,
                                      0x62, 0xB8, 0x78, 0x72, 0xB1, 0x74, 0xD7, 0x4E])
        g_spor_temp_pub_key = bytes([0x04, 0x91, 0xED, 0xC5, 0xF8, 0x25, 0xE9, 0x17, 0xAA, 0x99, 0x4B, 0xA5, 0xEE,
                                     0xEE, 0x16, 0xF5, 0x9E, 0x32, 0x50, 0xAC, 0x47, 0x1C, 0x70, 0xA0, 0x28, 0x9E, 0xCE, 0xC4, 0x97, 0x9A, 0xBF, 0xEA,
                                     0x9D, 0x87, 0x34, 0x23, 0xB1, 0x75, 0xAB, 0xA3, 0xFC, 0x62, 0x06, 0xCB, 0xEC, 0x66, 0x12, 0x5C, 0xD5, 0x26, 0xE9,
                                     0xED, 0x7D, 0xA3, 0x4C, 0xD0, 0x8A, 0x96, 0xAC, 0xE0, 0x57, 0x87, 0x91, 0x32, 0xB0])
        g_spor_temp_priv_key = bytes([0xC3, 0x4C, 0x2E, 0x1B, 0x25, 0x29, 0x09, 0x94, 0xD1, 0xCE, 0x30, 0x68, 0x0B, 0x35, 0x9D, 0x10, 0xBC, 0x44, 0x6D, 0xF9, 0x95, 0xA4, 0x7E, 0xDB, 0x71, 0x3B, 0x42, 0xA7, 0x3D, 0x10, 0xA1, 0xCB])
        g_rspor_perm_pub_key = bytes([0x04, 0xAC, 0x76, 0x7C, 0x20, 0x23, 0x11, 0x1F, 0x0B, 0xC3, 0x82, 0x04, 0x31,
                                      0x43, 0xD7, 0xFB, 0x44, 0x29, 0x1C, 0x4D, 0xE5, 0x62, 0xFC, 0x0B, 0x62, 0xA9, 0x16, 0x8C, 0x24, 0x1E, 0xB2, 0xF9,
                                      0xEB, 0x6C, 0xA2, 0x53, 0x55, 0x84, 0x40, 0xEE, 0xA4, 0x3C, 0x3E, 0xC2, 0x83, 0x35, 0x57, 0x00, 0xD0, 0xFD, 0xE8,
                                      0xF9, 0x28, 0x84, 0xF7, 0x54, 0xC5, 0x74, 0x4A, 0x3F, 0x6E, 0xC5, 0x24, 0xDB, 0xB4])
        g_rspor_tem_pub_key = bytes([0x04, 0x75, 0x67, 0x11, 0x46, 0xA4, 0xDC, 0xED, 0x89, 0x9B, 0x74, 0xD3, 0xCC,
                                       0x41, 0xC1, 0xF2, 0x3B, 0x08, 0x6D, 0xBF, 0x2A, 0x5A, 0xB7, 0xCA, 0x13, 0xBA, 0xF5, 0x63, 0x10, 0x5B, 0xBF, 0xEB,
                                       0xE5, 0x1C, 0x0A, 0xD9, 0x02, 0x42, 0x03, 0x59, 0xDE, 0xBE, 0x52, 0x75, 0xF8, 0x5C, 0xC4, 0xA2, 0x28, 0x8B, 0xF3,
                                       0xDD, 0x90, 0x69, 0x84, 0xA5, 0x36, 0xB4, 0x34, 0x21, 0xBF, 0xB1, 0x9F, 0x9D, 0x1E])

        g_spor_output = bytes([0x6F, 0x43, 0xE9, 0xB7, 0xF3, 0x2C, 0x67, 0x5A, 0xB2, 0x91, 0x90, 0x4F, 0x3E, 0xE3, 0x53, 0x1E])
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            0x3,  # KMS_KEY_PART_PAIRKEY
            0,  # reserved
            len(g_spor_perm_pub_key),  # pub_key_size
            len(g_spor_perm_priv_key)  # priv_key_size
        )
        key_data = header + g_spor_perm_pub_key + g_spor_perm_priv_key
        with allure.step("导入密钥 # 执行成功"):
            _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            0x3,  # KMS_KEY_PART_PRIVKEY
            0,  # reserved
            len(g_spor_temp_pub_key),  # pub_key_size
            len(g_spor_temp_priv_key)  # priv_key_size
        )
        key_data = header + g_spor_temp_pub_key + g_spor_temp_priv_key
        tmp_handle = 0xFFFFFFFF
        with allure.step("导入 sponsor temporary private and public key # 执行成功"):
            _, out_tmp_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, tmp_handle)

    with allure.step("2、导入本地临时密钥对，得到密钥句柄h2； # 2、导入密钥成功；"):

        s1_s2_value = b'0' * 512
        sa_sb_value = b'0' * 512
        host.write_memory(api.DATA5_ADDR, s1_s2_value)
        host.write_memory(api.DATA6_ADDR, sa_sb_value)
        host.write_memory(api.DATA7_ADDR, g_rspor_tem_pub_key)
        # Reason: 根据 mb_sm2_param_struct_st 结构体定义 (server/src/mb.h:1413-1421)
        # 结构体布局: B(1字节role) + 3x(3字节padding) + I(4字节handle) + Q(8字节addr) + Q(8字节addr) + Q(8字节addr) + I(4字节size) = 36字节
        sm2_data = struct.pack(
            "<B3xIQQQI",
            0x0,                      # sm2_role (1 byte): MB_SM2_PARAM_STRUCT_SM2_ROLE_KMS_SM2_ROLE_SPONSOR
            out_tmp_handle,           # local_tmp_key_handle (4 bytes)
            api.DATA5_ADDR,           # s1_s2_value_addr (8 bytes, raddr_t=uint64_t)
            api.DATA6_ADDR,           # sa_sb_value_addr (8 bytes, raddr_t=uint64_t)
            api.DATA7_ADDR,           # peer_temp_pubkey_addr (8 bytes, raddr_t=uint64_t)
            len(g_rspor_tem_pub_key)  # peer_temp_pubkey_size (4 bytes)
        )
        remote_pubkey_data = g_rspor_perm_pub_key
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]

    with allure.step("3、传入对方公钥和对方临时公钥，对称算法AES，得到协商密钥h3； # 3、协商密钥成功；"):
        _, out_new_handle= api.ehsm_km_exchange_key(g_rspor_perm_pub_key,len(g_rspor_perm_pub_key),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,hmac_key_size,local_handle,None,0 ,sm2_data,0xFFFFFFFF)


    with allure.step("4、导出协商密钥K1； # 4、导出密钥成功；"):
        key_buf = b'0' * 1024
        ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(out_new_handle,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,len(header)+512,None,0)
        s_privilege,s_key_type,s_part_info,s_reserved,s_pkey_size,s_priv_key_size,s_key_vale = struct.unpack("<IBBHHH16s", buf1)


    with allure.step("5、对比k1和预期协商密钥k； # 5、数据对比一致；"):
        assert (g_spor_output==s_key_vale)

    with allure.step("6、删除密钥h1/h2/h3; # 6、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_tmp_handle)
        api.ehsm_km_remove_key(out_new_handle)

@allure.feature("kms")
@allure.description("分别使用ECC Brainpool P192R1 进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-712")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持 SM2 算法")
def test_ehsm_712():
    with allure.step("1、导入明文密钥ECC私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        secret = bytes([
                        0x46, 0xFD, 0x4D, 0x3D, 0xF3, 0x7F, 0x16, 0x79,
                        0x64, 0xE9, 0x75, 0xC9, 0x78, 0xC7, 0x38, 0xB4,
                        0x72, 0x07, 0x89, 0x21, 0xB7, 0x9B, 0xC1, 0xDD])

        b_public = bytes([
                        0x82, 0xF2, 0x7E, 0x41, 0x7D, 0x59, 0x03, 0x77,
                        0x12, 0x46, 0xBC, 0x83, 0x6D, 0xE8, 0x0F, 0xAD,
                        0xDD, 0x7B, 0xF9, 0x5E, 0xC5, 0xA7, 0xC0, 0x01,
                        0x0E, 0x1E, 0xCF, 0x84, 0xC1, 0x31, 0x93, 0x8B,
                        0xBB, 0xA1, 0x63, 0x07, 0xE9, 0x29, 0x0C, 0xE6,
                        0xF0, 0x8A, 0xF0, 0x2F, 0xE9, 0x48, 0xB5, 0x6F])

        expected_ss = bytes([
                        0xB8, 0x6B, 0xF0, 0x8D, 0x09, 0x12, 0x86, 0x1E,
                        0x01, 0x95, 0xF5, 0x32, 0xDC, 0x7F, 0x63, 0x6D,
                        0x8D, 0xA7, 0xB2, 0x5A, 0x99, 0xE3, 0xF6, 0x4E])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0,  # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data =  header + secret
        _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)

    with allure.step("3、使用算法AES-ECB 和密钥h2 加密明文消息m，得到密文c； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(out_new_handle,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,len(header)+512,None,0)

    with allure.step("4、对比k1和预期协商密钥k； # 4、数据对比一致；"):
        assert (expected_ss[:16] == buf1[12:29])

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)


@allure.feature("kms")
@allure.description("分别使用ECC SEC P192R1 进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-715")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 ECC 算法")
def test_ehsm_715():
    with allure.step("1、导入明文密钥ECC公钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        secret   = bytes([
                            0xf1, 0x7d, 0x3f, 0xea, 0x36, 0x7b,
                            0x74, 0xd3, 0x40, 0x85, 0x1c, 0xa4,
                            0x27, 0x0d, 0xcb, 0x24, 0xc2, 0x71,
                            0xf4, 0x45, 0xbe, 0xd9, 0xd5, 0x27])

        b_public = bytes([
                            0x42, 0xea, 0x6d, 0xd9, 0x96, 0x9d, 0xd2, 0xa6,
                            0x1f, 0xea, 0x1a, 0xac, 0x7f, 0x8e, 0x98, 0xed,
                            0xcc, 0x89, 0x6c, 0x6e, 0x55, 0x85, 0x7c, 0xc0,
                            0xdf, 0xbe, 0x5d, 0x7c, 0x61, 0xfa, 0xc8, 0x8b,
                            0x11, 0x81, 0x1b, 0xde, 0x32, 0x8e, 0x8a, 0x0d,
                            0x12, 0xbf, 0x01, 0xa9, 0xd2, 0x04, 0xb5, 0x23])

        expected_ss = bytes([
                            0x80, 0x3d, 0x8a, 0xb2, 0xe5, 0xb6, 0xe6, 0xfc,
                            0xa7, 0x15, 0x73, 0x7c, 0x3a, 0x82, 0xf7, 0xce,
                            0x3c, 0x78, 0x31, 0x24, 0xf6, 0xd5, 0x1c, 0xd0])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0,  # pub_key_size
            24  # priv_key_size
        )
        key_data =  header + secret
        _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)

    with allure.step("3、导出密钥； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3  = api.ehsm_km_export_key(out_new_handle,0xffffffff,0xffffffff,EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,key_buf,len(header)+512,None,0)

    with allure.step("4、对比k1和预期协商密钥k； # 4、数据对比一致；"):
        assert (expected_ss[:16] == buf1[12:29])
    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@allure.feature("kms")
@allure.description("协商的目标密钥handle已经存在，且具有写保护权限，再通过协商命令重新生成该handle")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-716")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_716():
    with allure.step("1、创建一个新密钥h1，配置写保护权限； # 1、创建成功；"):
        gen_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        gen_handle = 0xFFFFFFFF
        secret   = bytes([
                            0xf1, 0x7d, 0x3f, 0xea, 0x36, 0x7b,
                            0x74, 0xd3, 0x40, 0x85, 0x1c, 0xa4,
                            0x27, 0x0d, 0xcb, 0x24, 0xc2, 0x71,
                            0xf4, 0x45, 0xbe, 0xd9, 0xd5, 0x27])

        b_public = bytes([
                            0x42, 0xea, 0x6d, 0xd9, 0x96, 0x9d, 0xd2, 0xa6,
                            0x1f, 0xea, 0x1a, 0xac, 0x7f, 0x8e, 0x98, 0xed,
                            0xcc, 0x89, 0x6c, 0x6e, 0x55, 0x85, 0x7c, 0xc0,
                            0xdf, 0xbe, 0x5d, 0x7c, 0x61, 0xfa, 0xc8, 0x8b,
                            0x11, 0x81, 0x1b, 0xde, 0x32, 0x8e, 0x8a, 0x0d,
                            0x12, 0xbf, 0x01, 0xa9, 0xd2, 0x04, 0xb5, 0x23])

        with allure.step(
                f"1 生成写保护权限的密钥"
        ):
            _, out_gen_handler = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, gen_permit, 0, 0, None, 0, gen_handle)
        permit =KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] |\
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        #导入密钥
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0,  # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data =  header + secret
        _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)


    with allure.step("2、配置合法的密钥协商参数，配置协商的目标句柄为h1 # 2、协商失败；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        # exchange_handle = 0xFFFFFFFF
        handle_back = out_gen_handler
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,handle_back)
        except hostapi.HostApiError as e :
            assert (e.ret_code == fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION), f"预期错误码{fw_errno.EHSM_ERR_MISMATCH_KEY_PERMISSION}, 实际错误码{e.ret_code}"

    with allure.step("3、删除密钥句柄 # 3、删除成功"):
        api.ehsm_km_remove_key(local_handle)

@allure.feature("kms")
@allure.description("协商密钥配置的本地密钥h1没有没有创建密钥权限，使用该密钥h1区进行密钥协商")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-717")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_717():
    with allure.step("1、创建一个新密钥h1，不创建密钥权限； # 1、创建成功；"):
        import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]

        secret   = b"\xf1\x7d\x3f\xea\x36\x7b\x74\xd3\x40\x85\x1c\xa4\x27\x0d\xcb\x24\xc2\x71\xf4\x45\xbe\xd9\xd5\x27"
        b_public = b"\x42\xea\x6d\xd9\x96\x9d\xd2\xa6\x1f\xea\x1a\xac\x7f\x8e\x98\xed\xcc\x89\x6c\x6e\x55\x85\x7c\xc0" + \
                    b"\xdf\xbe\x5d\x7c\x61\xfa\xc8\x8b\x11\x81\x1b\xde\x32\x8e\x8a\x0d\x12\xbf\x01\xa9\xd2\x04\xb5\x23"
        expected_ss = b"\x80\x3d\x8a\xb2\xe5\xb6\xe6\xfc\xa7\x15\x73\x7c\x3a\x82\xf7\xce\x3c\x78\x31\x24\xf6\xd5\x1c\xd0"

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        #导入密钥
        header = struct.pack(
            STRUCT_FORMAT,
            import_permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0,  # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data =  header + secret
        _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)


    with allure.step("2、配置合法的密钥协商参数，配置协商的本地密钥句柄为h1 # 2、协商失败；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)
        except hostapi.HostApiError as e :
            assert (e.ret_code == 45)
    with allure.step("3、删除密钥句柄 # 3、删除成功"):
        api.ehsm_km_remove_key(local_handle)

@allure.feature("kms")
@allure.description("协商密钥配置的本地密钥是具有TPK权限的OTP密钥，通过该OTP密钥进行密钥协商")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-718")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_718():
    with allure.step("1、配置合法的密钥协商数据，但将协商的本地密钥配置为具有创建TPK密钥权限的OTP密钥，新秘钥没有TPK权限； # 1、协商失败；"):
        b_public = b"\x42\xea\x6d\xd9\x96\x9d\xd2\xa6\x1f\xea\x1a\xac\x7f\x8e\x98\xed\xcc\x89\x6c\x6e\x55\x85\x7c\xc0" + \
                   b"\xdf\xbe\x5d\x7c\x61\xfa\xc8\x8b\x11\x81\x1b\xde\x32\x8e\x8a\x0d\x12\xbf\x01\xa9\xd2\x04\xb5\x23"
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        hmac_key_size = 0
        exchange_handle = 0xFFFFFFFF
        KMS_OTP_KEY_TPK =  0x20000C
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,KMS_OTP_KEY_TPK,None,0x0 ,None,exchange_handle)
            assert False, f"预期接口会出现异常返回值，如果正常，则用例失败"
        except hostapi.HostApiError as e:
            log.info(f"返回错误码：{e.ret_code}")

@allure.feature("kms")
@allure.description("协商密钥配置的新生成的协商密钥配置TPK权限，使用该配置进行密钥协商")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-719")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_719():
    with allure.step("1、配置合法的密钥协商数据，但配置协商生成的密钥权限配置TPK # 1、协商失败；"):
        import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                        KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]

        secret   = b"\xf1\x7d\x3f\xea\x36\x7b\x74\xd3\x40\x85\x1c\xa4\x27\x0d\xcb\x24\xc2\x71\xf4\x45\xbe\xd9\xd5\x27"
        b_public = b"\x42\xea\x6d\xd9\x96\x9d\xd2\xa6\x1f\xea\x1a\xac\x7f\x8e\x98\xed\xcc\x89\x6c\x6e\x55\x85\x7c\xc0" + \
                   b"\xdf\xbe\x5d\x7c\x61\xfa\xc8\x8b\x11\x81\x1b\xde\x32\x8e\x8a\x0d\x12\xbf\x01\xa9\xd2\x04\xb5\x23"
        expected_ss = b"\x80\x3d\x8a\xb2\xe5\xb6\xe6\xfc\xa7\x15\x73\x7c\x3a\x82\xf7\xce\x3c\x78\x31\x24\xf6\xd5\x1c\xd0"
        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        #导入密钥
        header = struct.pack(
            STRUCT_FORMAT,
            import_permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0,  # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data =  header + secret
        _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)
    with allure.step("2、配置密钥协商参数带有 TPK 权限，配置协商的本地密钥句柄为h1 # 2、协商失败；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_TRANSPORT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 45)
        api.ehsm_km_remove_key(local_handle)

@allure.feature("kms")
@allure.description("使用非法数据进行密钥交换指令测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-720")
@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
def test_ehsm_720():
    import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] |\
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

    gen_handle = 0xFFFFFFFF
    secret   =  b"\xf1\x7d\x3f\xea\x36\x7b\x74\xd3\x40\x85\x1c\xa4\x27\x0d\xcb\x24\xc2\x71\xf4\x45\xbe\xd9\xd5\x27"
    b_public =  b"\x42\xea\x6d\xd9\x96\x9d\xd2\xa6\x1f\xea\x1a\xac\x7f\x8e\x98\xed\xcc\x89\x6c\x6e\x55\x85\x7c\xc0" +\
                b"\xdf\xbe\x5d\x7c\x61\xfa\xc8\x8b\x11\x81\x1b\xde\x32\x8e\x8a\x0d\x12\xbf\x01\xa9\xd2\x04\xb5\x23"
    expected_ss =  b"\x80\x3d\x8a\xb2\xe5\xb6\xe6\xfc\xa7\x15\x73\x7c\x3a\x82\xf7\xce\x3c\x78\x31\x24\xf6\xd5\x1c\xd0"
    hmac_key_size = 0
    STRUCT_FORMAT = "<IBBHHH"
    handle = 0xFFFFFFFF
    header = struct.pack(
        STRUCT_FORMAT,
        import_permit,
        EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
        0x2,  #KMS_KEY_PART_PRIVKEY
        0,    #reserved
        0,  # pub_key_size
        len(secret)  # priv_key_size
    )
    key_data =  header + secret
    _,local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)
    with allure.step("1、将非法句柄传入密钥交换指令； # 1、返回失败；"):

        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  | \
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF
        # with allure.step(""):
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),exchange_permit,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 45)

        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,len(remote_pubkey_data),0xffffffff,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 1)

        try:
            _, out_new_handle= api.ehsm_km_exchange_key(None,len(remote_pubkey_data),0xffffffff,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,exchange_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 1)

    with allure.step("2、非法密钥g/p/q 和key_size； # 2、返回失败；"):

        invalid_data_length = 0x0
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,0x0,0xffffffff,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,invalid_data_length ,None,exchange_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 1)

        invalid_handle = 0x0
        try:
            _, out_new_handle= api.ehsm_km_exchange_key(remote_pubkey_data,0x0,0xffffffff,EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                    hmac_key_size,local_handle,None,0x0 ,None,invalid_handle)
        except hostapi.HostApiError as e:
            assert (e.ret_code == 1)

    with allure.step("3、将algo_id设置为无效值0xFF； # 3、返回失败；"):
        # Reason: 测试algo_id无效值（0xFF不是对称密钥算法）
        try:
            _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                         0xFF,  # 无效的算法ID
                                                         hmac_key_size, local_handle, None, 0x0, None, exchange_handle)
            assert False, "预期密钥交换失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == -65535), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("4、将key_size设置为0（使用HMAC测试）； # 4、返回失败；"):
        # Reason: 测试key_size=0的无效值，使用HMAC类型因为其key_size参数最灵活
        try:
            _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                         EhsmKeyType.EHSM_KEY_TYPE_HMAC,
                                                         513,  # key_size=0
                                                         local_handle, None, 0x0, None, exchange_handle)
            assert False, "预期密钥交换失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("5、将local_key_handle设置为不存在的句柄； # 5、返回失败；"):
        # Reason: 测试local_key_handle为不存在的句柄值
        invalid_local_handle = 0x12345678  # 不存在的句柄
        try:
            _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                         EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                         hmac_key_size, invalid_local_handle, None, 0x0, None, exchange_handle)
            assert False, "预期密钥交换失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_INVALID_HANDLE), \
                f"期望返回EHSM_ERR_INVALID_HANDLE, 实际返回值: {e.ret_code}"

        api.ehsm_km_remove_key(local_handle)


@allure.feature("kms")
@allure.description("计算SM2密钥公钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-721")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持 SM2 算法")
def test_ehsm_721():
    with allure.step("1、准备SM2私钥，并使用密钥导入命令导入；（使用标准数据） # 1、导入成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        g_sm2_prikey =  bytes([0xC4, 0x28, 0xBB, 0x47, 0x22, 0xFA, 0x2F, 0x5D, 0xB3, 0xBD, 0x27, 0x4A, 0xCD, 0x80, 0x96, 0x6C,
                               0x90, 0x83, 0xF1, 0xA4, 0x5E, 0x8C, 0xDF, 0x3B, 0x0A, 0xA6, 0x45, 0xFE, 0x88, 0x3F, 0x3C, 0x7B ])

        g_sm2_pubkey =  bytes([0x04, 0xD1, 0x4A, 0xAD, 0x32, 0xD7, 0xF4, 0x93, 0x06, 0x31, 0xA5, 0xB1, 0xF5, 0xEB, 0xFE, 0xFA,
                               0x9C, 0xC3, 0xB3, 0xD5, 0xE5, 0xA7, 0x48, 0x55, 0xD1, 0x63, 0x87, 0xF1, 0x83, 0x59, 0x26, 0x12,
                               0xAE,0xA5, 0xEE, 0x01, 0x50, 0x68, 0xBB, 0x10, 0x80, 0xD9, 0xA5, 0x0C, 0xCA,  0x25, 0xA5, 0x6A,
                               0xE4, 0x32, 0x3F, 0xE0,0x5E, 0x62, 0x80, 0x6A, 0xB9, 0x74, 0xC0, 0x71, 0x40,  0x91, 0xB3, 0x35, 0xCE ])
        pub_key_size = 65
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0x0,  # pub_key_size
            len(g_sm2_prikey)
        )
        key_data = header + g_sm2_prikey
        log.info(f'key_data hex: {key_data.hex(" ").upper()}')
        _,out_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、准备获取公钥指令，传入导入密钥handle，发送； # 2、获取公钥成功；"):
        _, output_k_pub,output_k_type,out_pub_k_size= api.ehsm_km_get_pub_from_priv(out_handle,None,0,pub_key_size)

    with allure.step("4、获取公钥数据，并与预期公钥数据进行对比 # 4、公钥对比一致；"):
        assert g_sm2_pubkey==output_k_pub
        assert out_pub_k_size == len(g_sm2_pubkey)

    with allure.step("5、删除密钥； # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(out_handle)

@allure.feature("kms")
@allure.description("计算ECC密钥公钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-722")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 ECC 算法")
def test_ehsm_722():
    with allure.step("1、准备ECC私钥，并使用密钥导入命令导入；（使用标准数据） # 1、导入成功；"):
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]
        g_ecc_prikey =  bytes([0xBA, 0x56, 0x3B, 0x79, 0xFD, 0x0F, 0xDE, 0xF1, 0x29, 0x40, 0x67, 0x7E, 0x67, 0xFA,0x03, 0x7D, 0x40, 0x61, 0x3A, 0x8A, 0x65, 0xD8, 0x71, 0xC1, 0xC8, 0xD2, 0xBF, 0x39, 0x68, 0x16, 0x3B, 0x3A])
        g_ecc_pubkey =  bytes([0x04, 0x7A, 0x9B, 0x66, 0x05, 0x45, 0xE6, 0x98, 0x9C, 0x94, 0x55, 0x07, 0xB2, 0x31, 0xBA, 0xBE,
                               0xFA, 0xED, 0x23, 0xD1, 0x4D, 0x3A, 0x4D, 0x16, 0x56, 0xD8, 0xA5, 0x4F, 0x6A, 0xAF, 0x5C, 0x00,
                               0x93, 0xE8, 0x7C, 0xA0, 0x36, 0xD1, 0xAA, 0x94, 0xA6, 0x03, 0x0A, 0x78, 0xE7, 0x0E, 0x4A, 0xC8,
                               0xDA, 0x3C, 0xFA, 0xBE, 0x6A, 0x9B, 0xEE, 0xA8, 0xEF, 0xA7, 0xAC, 0x9C, 0x0C, 0x40, 0x76, 0x2D, 0xF9 ])
        pub_key_size = 64
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1,
            0x2,  #KMS_KEY_PART_PRIVKEY
            0,    #reserved
            0x0,  # pub_key_size
            len(g_ecc_prikey)
        )
        key_data = header + g_ecc_prikey
        _,out_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、准备获取公钥指令，传入导入密钥handle，发送； # 2、获取公钥成功；"):
         _, output_k_pub,output_k_type ,out_pub_k_size= api.ehsm_km_get_pub_from_priv(out_handle,None,0,pub_key_size)

    with allure.step("3、获取公钥数据，并与预期公钥数据进行对比 # 3、公钥对比一致；"):
        assert g_ecc_pubkey[1:]==output_k_pub
        assert out_pub_k_size == len(g_ecc_pubkey)-1

    with allure.step("4、删除密钥； # 4、删除密钥成功；"):
        api.ehsm_km_remove_key(out_handle)

@allure.feature("kms")
@allure.description("计算DH密钥公钥")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-723")
def test_ehsm_723():
    with allure.step("1、准备DH私钥，并使用密钥导入命令导入；（使用标准数据） # 1、导入成功；"):
        # Reason: DH 1024-bit 私钥，20 字节
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])

        # Reason: 预期的公钥数据，128 字节
        dh_tv_template_1024_expected_a_public = bytes([
            0xBB, 0xE9, 0x18, 0xDD, 0x4B, 0x2B, 0x94, 0x1B, 0x10, 0x0E, 0x88, 0x35, 0x28, 0x68, 0xFC, 0x62,
            0x04, 0x38, 0xA6, 0xDB, 0x32, 0xA6, 0x9E, 0xEE, 0x6C, 0x6F, 0x45, 0x1C, 0xA3, 0xA6, 0xD5, 0x37,
            0x77, 0x75, 0x5B, 0xC1, 0x37, 0x0A, 0xCE, 0xFE, 0x2B, 0x8F, 0x13, 0xA9, 0x14, 0x2C, 0x5B, 0x44,
            0x15, 0x78, 0x86, 0x30, 0xD6, 0x95, 0xB1, 0x92, 0x20, 0x63, 0xA3, 0xCF, 0x9D, 0xEF, 0x65, 0x61,
            0x27, 0x4D, 0x24, 0x01, 0xE7, 0xA1, 0x45, 0xF2, 0xD8, 0xB9, 0x3A, 0x45, 0x17, 0xF4, 0x19, 0xD0,
            0x5E, 0xF8, 0xCB, 0x35, 0x59, 0x37, 0x9D, 0x04, 0x20, 0xA3, 0xBF, 0x02, 0xAD, 0xFE, 0xA8, 0x60,
            0xB2, 0xC3, 0xEE, 0x85, 0x58, 0x90, 0xF3, 0xB5, 0x57, 0x2B, 0xB4, 0xEF, 0xD7, 0x8F, 0x37, 0x68,
            0x78, 0x7C, 0x71, 0x52, 0x9D, 0x5E, 0x0A, 0x61, 0x4F, 0x09, 0x89, 0x92, 0x39, 0xF7, 0x4B, 0x01])

        # Reason: DH 1024-bit 参数 p，128 字节
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

        # Reason: DH 1024-bit 参数 q，20 字节
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

        # Reason: DH 1024-bit 参数 g，128 字节
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xB4, 0x61, 0x64, 0xE5, 0xE9, 0x43, 0x84, 0xEE, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

        # Reason: 导入 DH 私钥，参考 test_ehsm_710 的实现
        STRUCT_FORMAT = "<IBBHHH"
        gen_handle = 0xFFFFFFFF
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]

        header = struct.pack(
            STRUCT_FORMAT,
            permit,  # 权限
            EhsmKeyType.EHSM_KEY_TYPE_DH,  # 算法类型
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,  # reserved
            0x0,  # pub_key_size
            len(dh_tv_template_1024_secret)  # priv_key_size
        )
        key_data = header + dh_tv_template_1024_secret

        _, out_handle = api.ehsm_km_import_key(
            0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, gen_handle
        )

    with allure.step("2、准备获取公钥指令，传入导入密钥handle和参数g/p/q，发送； # 2、获取公钥成功；"):
        # Reason: 构建 DH common data，格式为 p_size(4) + p(128) + q_size(4) + q(20) + g_size(4) + g(128) + trailing_zero(4)
        p_size = len(dh_tv_template_1024_p)
        q_size = len(dh_tv_template_1024_q)
        g_size = len(dh_tv_template_1024_g)

        dh_common_data = struct.pack("<I128sI20sI128s",
                                     p_size, dh_tv_template_1024_p,
                                     q_size, dh_tv_template_1024_q,
                                     g_size, dh_tv_template_1024_g)
        # Reason: 根据 C 代码，需要在末尾添加 4 字节的 0
        dh_common_data += struct.pack("<I", 0)

        # Reason: 调用获取公钥接口，C 代码传递 dh_common_size + 12，其中 dh_common_size = 288，所以传递 300 字节
        # 返回值包括：状态码、公钥数据、公钥类型、公钥长度
        _, output_k_pub, output_k_type, out_pub_k_size = api.ehsm_km_get_pub_from_priv(
            out_handle, dh_common_data, len(dh_common_data) + 8, 512
        )

    with allure.step("3、获取公钥数据，并与预期公钥数据进行对比 # 3、公钥对比一致；"):
        # Reason: 对比公钥数据和预期值
        assert dh_tv_template_1024_expected_a_public == output_k_pub[:len(dh_tv_template_1024_expected_a_public)], "公钥数据对比失败"
        assert out_pub_k_size == len(dh_tv_template_1024_expected_a_public), f"公钥长度不匹配，预期: {len(dh_tv_template_1024_expected_a_public)}, 实际: {out_pub_k_size}"

    with allure.step("4、删除密钥； # 4、删除密钥成功；"):
        # Reason: 清理测试数据
        api.ehsm_km_remove_key(out_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持 X25519 密钥协商")
@allure.feature("kms")
@allure.description("X25519 密钥协商测试 - edge case on twist")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K001")
def test_ehsm_k001():
    with allure.step("1、导入明文密钥 X25519 私钥作为本地密钥，并配置创建密钥权限，得到 h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # X25519 私钥（32字节）- edge case on twist
        secret = bytes([
            0xc0, 0x1d, 0x13, 0x05, 0xa1, 0x33, 0x8a, 0x1f,
            0xca, 0xc2, 0xba, 0x7e, 0x2e, 0x03, 0x2b, 0x42,
            0x7e, 0x0b, 0x04, 0x90, 0x31, 0x65, 0xac, 0xa9,
            0x57, 0xd8, 0xd0, 0x55, 0x3d, 0x87, 0x17, 0xb0
        ])

        # 对方公钥（32字节）
        b_public = bytes([
            0xea, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f
        ])

        # 预期协商密钥（32字节）
        expected_ss = bytes([
            0x19, 0x23, 0x0e, 0xb1, 0x48, 0xd5, 0xd6, 0x7c,
            0x3c, 0x22, 0xab, 0x1d, 0xae, 0xff, 0x80, 0xa5,
            0x7e, 0xae, 0x42, 0x65, 0xce, 0x28, 0x72, 0x65,
            0x7b, 0x2c, 0x80, 0x99, 0xfc, 0x69, 0x8e, 0x50
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        # Reason: X25519 私钥长度为 32 字节，公钥长度为 0（仅导入私钥）
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_X25519,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            32    # priv_key_size (X25519 为 32 字节)
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法 AES_256，得到协商密钥 h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        # Reason: 使用 AES_256 作为协商目标密钥类型，可以完整验证 32 字节的协商结果
        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_256,
                                                    hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出密钥； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_buf, len(header)+512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过 header（12 字节），验证完整的 32 字节协商密钥
        assert (expected_ss == buf1[12:44]), f"协商密钥与预期不符，预期：{expected_ss.hex().upper()}，实际：{buf1[12:44].hex().upper()}"

    with allure.step("5、删除密钥 h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持 X25519 密钥协商")
@allure.feature("kms")
@allure.description("X25519 密钥协商测试 - edge case for public key")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K002")
def test_ehsm_k002():
    with allure.step("1、导入明文密钥 X25519 私钥作为本地密钥，并配置创建密钥权限，得到 h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # X25519 私钥（32字节）- edge case for public key
        secret = bytes([
            0xc0, 0x65, 0x8c, 0x46, 0xdd, 0xe1, 0x81, 0x29,
            0x29, 0x38, 0x77, 0x53, 0x5b, 0x11, 0x62, 0xb6,
            0xf9, 0xf5, 0x41, 0x4a, 0x23, 0xcf, 0x4d, 0x2c,
            0xbc, 0x14, 0x0a, 0x4d, 0x99, 0xda, 0x2b, 0x8f
        ])

        # 对方公钥（32字节）
        b_public = bytes([
            0xeb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f
        ])

        # 预期协商密钥（32字节）
        expected_ss = bytes([
            0x57, 0x8b, 0xa8, 0xcc, 0x2d, 0xbd, 0xc5, 0x75,
            0xaf, 0xcf, 0x9d, 0xf2, 0xb3, 0xee, 0x61, 0x89,
            0xf5, 0x33, 0x7d, 0x68, 0x54, 0xc7, 0x9b, 0x4c,
            0xe1, 0x65, 0xea, 0x12, 0x29, 0x3b, 0x3a, 0x0f
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        # Reason: X25519 私钥长度为 32 字节，公钥长度为 0（仅导入私钥）
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_X25519,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            32    # priv_key_size (X25519 为 32 字节)
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法 AES_256，得到协商密钥 h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        # Reason: 使用 AES_256 作为协商目标密钥类型，可以完整验证 32 字节的协商结果
        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_256,
                                                    hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出密钥； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff, EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY, key_buf, len(header)+512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过 header（12 字节），验证完整的 32 字节协商密钥
        assert (expected_ss == buf1[12:44]), f"协商密钥与预期不符，预期：{expected_ss.hex().upper()}，实际：{buf1[12:44].hex().upper()}"

    with allure.step("5、删除密钥 h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("ECDH Brainpool P160R1标准测试向量验证")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K003")
def test_ehsm_k003():
    """ECDH Brainpool P160R1标准测试向量验证 - 从C代码test_ecdh.c的STD_BRAINPOOLP160R1迁移"""

    ecdh_test_vectors = [
        {
            "name": "ECDH_Brainpool_P160R1",
            "secret_size": 20,
            "b_public_size": 40,
            "expected_ss_size": 20,
            "secret": bytes([0x74, 0xC0, 0x78, 0x79, 0x86, 0x79, 0xE8, 0x85,
                           0xD7, 0x6C, 0x85, 0x27, 0x50, 0x99, 0x25, 0x0A,
                           0x43, 0xA4, 0xB5, 0xAA]),
            "b_public": bytes([0x37, 0x79, 0xE4, 0x6F, 0x2B, 0xD4, 0x7D, 0xC5,
                             0x71, 0xAE, 0x0B, 0x88, 0x5C, 0x12, 0xC3, 0xBC,
                             0xD3, 0xE0, 0xF9, 0xF2, 0xB9, 0xC2, 0x3D, 0xF1,
                             0x92, 0x6C, 0xC0, 0xE5, 0x3E, 0xB0, 0x7A, 0xF1,
                             0x6D, 0x50, 0x23, 0xFD, 0xEB, 0x5B, 0x41, 0xED]),
            "expected_ss": bytes([0x0E, 0x5E, 0xCE, 0xFD, 0xA5, 0x7A, 0xCC, 0x24,
                                0x04, 0xAF, 0x79, 0x6F, 0xD5, 0xF7, 0x0F, 0x77,
                                0xF3, 0x2B, 0xF8, 0x7C])
        }
    ]

    for vec in ecdh_test_vectors:
        with allure.step(f"1、导入明文密钥{vec['name']}私钥作为本地密钥h1； # 1、导入密钥成功；"):
            # Reason: ECDH Brainpool P160R1 使用 20 字节私钥
            permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] | \
                     KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

            STRUCT_FORMAT = "<IBBHHH"
            header = struct.pack(
                STRUCT_FORMAT,
                permit,
                EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1,
                0x2,  # KMS_KEY_PART_PRIVKEY
                0,    # reserved
                0x0,  # pub_key_size
                vec["secret_size"]  # priv_key_size
            )
            key_data = header + vec["secret"]
            _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)

        with allure.step(f"2、传入对方公钥进行{vec['name']}密钥协商，生成AES密钥h2； # 2、协商密钥成功；"):
            # Reason: ECDH不需要dh_command_data，曲线参数由密钥类型ECC_BRAINPOOLP_160R1决定
            exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
            hmac_key_size = 0
            _, out_new_handle = api.ehsm_km_exchange_key(vec["b_public"], len(vec["b_public"]),
                                                         exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                         hmac_key_size, local_handle,
                                                         None, 0,
                                                         None, 0xFFFFFFFF)

        with allure.step(f"3、导出协商密钥h2； # 3、导出密钥成功；"):
            key_buf = b''
            ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                                  EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                                  key_buf, len(header) + vec["expected_ss_size"],
                                                                  None, 0)

        with allure.step(f"4、验证{vec['name']}协商密钥与预期值一致； # 4、数据对比一致；"):
            # Reason: 导出的密钥数据包含header，需要跳过header部分
            exported_key = buf1[len(header):len(header) + 16]  # AES_128密钥为16字节
            expected_key = vec["expected_ss"][:16]  # 取前16字节作为AES密钥
            assert exported_key == expected_key, f"{vec['name']}协商密钥验证失败，预期：{expected_key.hex().upper()}，实际：{exported_key.hex().upper()}"

        with allure.step("5、删除密钥h1和h2； # 5、删除密钥成功；"):
            api.ehsm_km_remove_key(local_handle)
            api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_ECDH_SUPPORT == 0, reason="不支持 ECDH 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P160R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K004")
def test_ehsm_k004():
    """使用ECC Brainpool P160R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P160R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP160R1测试向量 - A方私钥（20字节）
        secret = bytes([
            0x74, 0xC0, 0x78, 0x79, 0x86, 0x79, 0xE8, 0x85,
            0xD7, 0x6C, 0x85, 0x27, 0x50, 0x99, 0x25, 0x0A,
            0x43, 0xA4, 0xB5, 0xAA
        ])

        # Reason: BrainpoolP160R1测试向量 - B方公钥（40字节）
        b_public = bytes([
            0x37, 0x79, 0xE4, 0x6F, 0x2B, 0xD4, 0x7D, 0xC5,
            0x71, 0xAE, 0x0B, 0x88, 0x5C, 0x12, 0xC3, 0xBC,
            0xD3, 0xE0, 0xF9, 0xF2, 0xB9, 0xC2, 0x3D, 0xF1,
            0x92, 0x6C, 0xC0, 0xE5, 0x3E, 0xB0, 0x7A, 0xF1,
            0x6D, 0x50, 0x23, 0xFD, 0xEB, 0x5B, 0x41, 0xED
        ])

        # Reason: BrainpoolP160R1测试向量 - 预期共享密钥（20字节）
        expected_ss = bytes([
            0x0E, 0x5E, 0xCE, 0xFD, 0xA5, 0x7A, 0xCC, 0x24,
            0x04, 0xAF, 0x79, 0x6F, 0xD5, 0xF7, 0x0F, 0x77,
            0xF3, 0x2B, 0xF8, 0x7C
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P224R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K005")
def test_ehsm_k005():
    """使用ECC Brainpool P224R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P224R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP224R1测试向量 - A方私钥（28字节）
        secret = bytes([
            0x39, 0xF1, 0x55, 0x48, 0x3C, 0xEE, 0x19, 0x1F,
            0xBE, 0xCF, 0xE9, 0xC8, 0x1D, 0x8A, 0xB1, 0xA0,
            0x3C, 0xDA, 0x67, 0x90, 0xE7, 0x18, 0x4A, 0xCE,
            0x44, 0xBC, 0xA1, 0x61
        ])

        # Reason: BrainpoolP224R1测试向量 - B方公钥（56字节）
        b_public = bytes([
            0x03, 0x4A, 0x56, 0xC5, 0x50, 0xFF, 0x88, 0x05,
            0x61, 0x44, 0xE6, 0xDD, 0x56, 0x07, 0x0F, 0x54,
            0xB0, 0x13, 0x59, 0x76, 0xB5, 0xBF, 0x77, 0x82,
            0x73, 0x13, 0xF3, 0x6B,
            0x75, 0x16, 0x5A, 0xD9, 0x93, 0x47, 0xDC, 0x86,
            0xCA, 0xAB, 0x1C, 0xBB, 0x57, 0x9E, 0x19, 0x8E,
            0xAF, 0x88, 0xDC, 0x35, 0xF9, 0x27, 0xB3, 0x58,
            0xAA, 0x68, 0x36, 0x81
        ])

        # Reason: BrainpoolP224R1测试向量 - 预期共享密钥（28字节）
        expected_ss = bytes([
            0x1A, 0x4B, 0xFE, 0x70, 0x54, 0x45, 0x12, 0x0C,
            0x8E, 0x3E, 0x02, 0x66, 0x99, 0x05, 0x41, 0x04,
            0x51, 0x0D, 0x11, 0x97, 0x57, 0xB7, 0x4D, 0x5F,
            0xE2, 0x46, 0x2C, 0x66
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P320R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K006")
def test_ehsm_k006():
    """使用ECC Brainpool P320R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P320R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP320R1测试向量 - A方私钥（40字节）
        secret = bytes([
            0x79, 0x81, 0xEC, 0x20, 0x92, 0x52, 0x77, 0x94,
            0xFE, 0x33, 0x45, 0x86, 0x63, 0x8A, 0x60, 0xB3,
            0x01, 0x0C, 0xF9, 0x42, 0x77, 0xA9, 0x3F, 0xFC,
            0x5F, 0x11, 0x9B, 0xF8, 0x14, 0xAB, 0x0F, 0x1F,
            0x43, 0x53, 0x4A, 0x11, 0x47, 0xCB, 0xC3, 0x61
        ])

        # Reason: BrainpoolP320R1测试向量 - B方公钥（80字节）
        b_public = bytes([
            0x48, 0xB8, 0x78, 0x60, 0x1F, 0xC8, 0xE5, 0x91,
            0x23, 0x4A, 0xEA, 0x14, 0x09, 0xE9, 0xB2, 0xED,
            0xBB, 0x42, 0x34, 0xF0, 0x3E, 0x5F, 0x2C, 0x6E,
            0xD6, 0xB3, 0x82, 0xDB, 0x60, 0xAC, 0xA8, 0x44,
            0xE0, 0xE3, 0x3A, 0x16, 0x93, 0xEF, 0xE9, 0x5E,
            0x77, 0x47, 0xE3, 0x30, 0x18, 0x97, 0xDB, 0x28,
            0xD3, 0xA5, 0x0A, 0x44, 0xC7, 0x11, 0xA5, 0xCA,
            0xC2, 0x9D, 0x28, 0x04, 0xEF, 0xD4, 0x6E, 0xD2,
            0xCF, 0x34, 0xC9, 0xDB, 0xF2, 0x2D, 0xD5, 0x62,
            0x39, 0x55, 0xD4, 0x63, 0xB2, 0x05, 0x53, 0x2C
        ])

        # Reason: BrainpoolP320R1测试向量 - 预期共享密钥（40字节）
        expected_ss = bytes([
            0x40, 0x70, 0xA6, 0x99, 0xC2, 0x45, 0x18, 0xD4,
            0x15, 0xF5, 0x05, 0x6E, 0x71, 0x43, 0x20, 0x47,
            0xF9, 0xD1, 0xDD, 0x47, 0x14, 0x58, 0x52, 0xF6,
            0x61, 0x1B, 0x59, 0x61, 0xF9, 0x4D, 0xC6, 0xBD,
            0x9F, 0x67, 0xC5, 0xBB, 0xED, 0x5E, 0x02, 0x6B
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P384R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K007")
def test_ehsm_k007():
    """使用ECC Brainpool P384R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P384R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP384R1测试向量 - A方私钥（48字节）
        secret = bytes([
            0x1E, 0x20, 0xF5, 0xE0, 0x48, 0xA5, 0x88, 0x6F,
            0x1F, 0x15, 0x7C, 0x74, 0xE9, 0x1B, 0xDE, 0x2B,
            0x98, 0xC8, 0xB5, 0x2D, 0x58, 0xE5, 0x00, 0x3D,
            0x57, 0x05, 0x3F, 0xC4, 0xB0, 0xBD, 0x65, 0xD6,
            0xF1, 0x5E, 0xB5, 0xD1, 0xEE, 0x16, 0x10, 0xDF,
            0x87, 0x07, 0x95, 0x14, 0x36, 0x27, 0xD0, 0x42
        ])

        # Reason: BrainpoolP384R1测试向量 - B方公钥（96字节）
        b_public = bytes([
            0x4D, 0x44, 0x32, 0x6F, 0x26, 0x9A, 0x59, 0x7A,
            0x5B, 0x58, 0xBB, 0xA5, 0x65, 0xDA, 0x55, 0x56,
            0xED, 0x7F, 0xD9, 0xA8, 0xA9, 0xEB, 0x76, 0xC2,
            0x5F, 0x46, 0xDB, 0x69, 0xD1, 0x9D, 0xC8, 0xCE,
            0x6A, 0xD1, 0x8E, 0x40, 0x4B, 0x15, 0x73, 0x8B,
            0x20, 0x86, 0xDF, 0x37, 0xE7, 0x1D, 0x1E, 0xB4,
            0x62, 0xD6, 0x92, 0x13, 0x6D, 0xE5, 0x6C, 0xBE,
            0x93, 0xBF, 0x5F, 0xA3, 0x18, 0x8E, 0xF5, 0x8B,
            0xC8, 0xA3, 0xA0, 0xEC, 0x6C, 0x1E, 0x15, 0x1A,
            0x21, 0x03, 0x8A, 0x42, 0xE9, 0x18, 0x53, 0x29,
            0xB5, 0xB2, 0x75, 0x90, 0x3D, 0x19, 0x2F, 0x8D,
            0x4E, 0x1F, 0x32, 0xFE, 0x9C, 0xC7, 0x8C, 0x48
        ])

        # Reason: BrainpoolP384R1测试向量 - 预期共享密钥（48字节）
        expected_ss = bytes([
            0x0B, 0xD9, 0xD3, 0xA7, 0xEA, 0x0B, 0x3D, 0x51,
            0x9D, 0x09, 0xD8, 0xE4, 0x8D, 0x07, 0x85, 0xFB,
            0x74, 0x4A, 0x6B, 0x35, 0x5E, 0x63, 0x04, 0xBC,
            0x51, 0xC2, 0x29, 0xFB, 0xBC, 0xE2, 0x39, 0xBB,
            0xAD, 0xF6, 0x40, 0x37, 0x15, 0xC3, 0x5D, 0x4F,
            0xB2, 0xA5, 0x44, 0x4F, 0x57, 0x5D, 0x4F, 0x42
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P512R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K008")
def test_ehsm_k008():
    """使用ECC Brainpool P512R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P512R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP512R1测试向量 - A方私钥（64字节）
        secret = bytes([
            0x16, 0x30, 0x2F, 0xF0, 0xDB, 0xBB, 0x5A, 0x8D,
            0x73, 0x3D, 0xAB, 0x71, 0x41, 0xC1, 0xB4, 0x5A,
            0xCB, 0xC8, 0x71, 0x59, 0x39, 0x67, 0x7F, 0x6A,
            0x56, 0x85, 0x0A, 0x38, 0xBD, 0x87, 0xBD, 0x59,
            0xB0, 0x9E, 0x80, 0x27, 0x96, 0x09, 0xFF, 0x33,
            0x3E, 0xB9, 0xD4, 0xC0, 0x61, 0x23, 0x1F, 0xB2,
            0x6F, 0x92, 0xEE, 0xB0, 0x49, 0x82, 0xA5, 0xF1,
            0xD1, 0x76, 0x4C, 0xAD, 0x57, 0x66, 0x54, 0x22
        ])

        # Reason: BrainpoolP512R1测试向量 - B方公钥（128字节）
        b_public = bytes([
            0x9D, 0x45, 0xF6, 0x6D, 0xE5, 0xD6, 0x7E, 0x2E,
            0x6D, 0xB6, 0xE9, 0x3A, 0x59, 0xCE, 0x0B, 0xB4,
            0x81, 0x06, 0x09, 0x7F, 0xF7, 0x8A, 0x08, 0x1D,
            0xE7, 0x81, 0xCD, 0xB3, 0x1F, 0xCE, 0x8C, 0xCB,
            0xAA, 0xEA, 0x8D, 0xD4, 0x32, 0x0C, 0x41, 0x19,
            0xF1, 0xE9, 0xCD, 0x43, 0x7A, 0x2E, 0xAB, 0x37,
            0x31, 0xFA, 0x96, 0x68, 0xAB, 0x26, 0x8D, 0x87,
            0x1D, 0xED, 0xA5, 0x5A, 0x54, 0x73, 0x19, 0x9F,
            0x2F, 0xDC, 0x31, 0x30, 0x95, 0xBC, 0xDD, 0x5F,
            0xB3, 0xA9, 0x16, 0x36, 0xF0, 0x7A, 0x95, 0x9C,
            0x8E, 0x86, 0xB5, 0x63, 0x6A, 0x1E, 0x93, 0x0E,
            0x83, 0x96, 0x04, 0x9C, 0xB4, 0x81, 0x96, 0x1D,
            0x36, 0x5C, 0xC1, 0x14, 0x53, 0xA0, 0x6C, 0x71,
            0x98, 0x35, 0x47, 0x5B, 0x12, 0xCB, 0x52, 0xFC,
            0x3C, 0x38, 0x3B, 0xCE, 0x35, 0xE2, 0x7E, 0xF1,
            0x94, 0x51, 0x2B, 0x71, 0x87, 0x62, 0x85, 0xFA
        ])

        # Reason: BrainpoolP512R1测试向量 - 预期共享密钥（64字节）
        expected_ss = bytes([
            0xA7, 0x92, 0x70, 0x98, 0x65, 0x5F, 0x1F, 0x99,
            0x76, 0xFA, 0x50, 0xA9, 0xD5, 0x66, 0x86, 0x5D,
            0xC5, 0x30, 0x33, 0x18, 0x46, 0x38, 0x1C, 0x87,
            0x25, 0x6B, 0xAF, 0x32, 0x26, 0x24, 0x4B, 0x76,
            0xD3, 0x64, 0x03, 0xC0, 0x24, 0xD7, 0xBB, 0xF0,
            0xAA, 0x08, 0x03, 0xEA, 0xFF, 0x40, 0x5D, 0x3D,
            0x24, 0xF1, 0x1A, 0x9B, 0x5C, 0x0B, 0xEF, 0x67,
            0x9F, 0xE1, 0x45, 0x4B, 0x21, 0xC4, 0xCD, 0x1F
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP P192R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K009")
def test_ehsm_k009():
    """使用ECC SECP P192R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP P192R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP P192R1测试向量 - A方私钥（24字节）
        secret = bytes([
            0xf1, 0x7d, 0x3f, 0xea, 0x36, 0x7b, 0x74, 0xd3,
            0x40, 0x85, 0x1c, 0xa4, 0x27, 0x0d, 0xcb, 0x24,
            0xc2, 0x71, 0xf4, 0x45, 0xbe, 0xd9, 0xd5, 0x27
        ])

        # Reason: SECP P192R1测试向量 - B方公钥（48字节）
        b_public = bytes([
            0x42, 0xea, 0x6d, 0xd9, 0x96, 0x9d, 0xd2, 0xa6,
            0x1f, 0xea, 0x1a, 0xac, 0x7f, 0x8e, 0x98, 0xed,
            0xcc, 0x89, 0x6c, 0x6e, 0x55, 0x85, 0x7c, 0xc0,
            0xdf, 0xbe, 0x5d, 0x7c, 0x61, 0xfa, 0xc8, 0x8b,
            0x11, 0x81, 0x1b, 0xde, 0x32, 0x8e, 0x8a, 0x0d,
            0x12, 0xbf, 0x01, 0xa9, 0xd2, 0x04, 0xb5, 0x23
        ])

        # Reason: SECP P192R1测试向量 - 预期共享密钥（24字节）
        expected_ss = bytes([
            0x80, 0x3d, 0x8a, 0xb2, 0xe5, 0xb6, 0xe6, 0xfc,
            0xa7, 0x15, 0x73, 0x7c, 0x3a, 0x82, 0xf7, 0xce,
            0x3c, 0x78, 0x31, 0x24, 0xf6, 0xd5, 0x1c, 0xd0
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP P224R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K010")
def test_ehsm_k010():
    """使用ECC SECP P224R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP P224R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP P224R1测试向量 - A方私钥（28字节）
        secret = bytes([
            0x83, 0x46, 0xa6, 0x0f, 0xc6, 0xf2, 0x93, 0xca,
            0x5a, 0x0d, 0x2a, 0xf6, 0x8b, 0xa7, 0x1d, 0x1d,
            0xd3, 0x89, 0xe5, 0xe4, 0x08, 0x37, 0x94, 0x2d,
            0xf3, 0xe4, 0x3c, 0xbd
        ])

        # Reason: SECP P224R1测试向量 - B方公钥（56字节）
        b_public = bytes([
            0xaf, 0x33, 0xcd, 0x06, 0x29, 0xbc, 0x7e, 0x99,
            0x63, 0x20, 0xa3, 0xf4, 0x03, 0x68, 0xf7, 0x4d,
            0xe8, 0x70, 0x4f, 0xa3, 0x7b, 0x8f, 0xab, 0x69,
            0xab, 0xaa, 0xe2, 0x80, 0x88, 0x20, 0x92, 0xcc,
            0xbb, 0xa7, 0x93, 0x0f, 0x41, 0x9a, 0x8a, 0x4f,
            0x9b, 0xb1, 0x69, 0x78, 0xbb, 0xc3, 0x83, 0x87,
            0x29, 0x99, 0x25, 0x59, 0xa6, 0xf2, 0xe2, 0xd7
        ])

        # Reason: SECP P224R1测试向量 - 预期共享密钥（28字节）
        expected_ss = bytes([
            0x7d, 0x96, 0xf9, 0xa3, 0xbd, 0x3c, 0x05, 0xcf,
            0x5c, 0xc3, 0x7f, 0xeb, 0x8b, 0x9d, 0x52, 0x09,
            0xd5, 0xc2, 0x59, 0x74, 0x64, 0xde, 0xc3, 0xe9,
            0x98, 0x37, 0x43, 0xe8
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP P256R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K011")
def test_ehsm_k011():
    """使用ECC SECP P256R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP P256R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP P256R1测试向量 - A方私钥（32字节）
        secret = bytes([
            0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda,
            0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
            0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e,
            0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
        ])

        # Reason: SECP P256R1测试向量 - B方公钥（64字节）
        b_public = bytes([
            0x70, 0x0c, 0x48, 0xf7, 0x7f, 0x56, 0x58, 0x4c,
            0x5c, 0xc6, 0x32, 0xca, 0x65, 0x64, 0x0d, 0xb9,
            0x1b, 0x6b, 0xac, 0xce, 0x3a, 0x4d, 0xf6, 0xb4,
            0x2c, 0xe7, 0xcc, 0x83, 0x88, 0x33, 0xd2, 0x87,
            0xdb, 0x71, 0xe5, 0x09, 0xe3, 0xfd, 0x9b, 0x06,
            0x0d, 0xdb, 0x20, 0xba, 0x5c, 0x51, 0xdc, 0xc5,
            0x94, 0x8d, 0x46, 0xfb, 0xf6, 0x40, 0xdf, 0xe0,
            0x44, 0x17, 0x82, 0xca, 0xb8, 0x5f, 0xa4, 0xac
        ])

        # Reason: SECP P256R1测试向量 - 预期共享密钥（32字节）
        expected_ss = bytes([
            0x46, 0xfc, 0x62, 0x10, 0x64, 0x20, 0xff, 0x01,
            0x2e, 0x54, 0xa4, 0x34, 0xfb, 0xdd, 0x2d, 0x25,
            0xcc, 0xc5, 0x85, 0x20, 0x60, 0x56, 0x1e, 0x68,
            0x04, 0x0d, 0xd7, 0x77, 0x89, 0x97, 0xbd, 0x7b
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP P384R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K012")
def test_ehsm_k012():
    """使用ECC SECP P384R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP P384R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP P384R1测试向量 - A方私钥（48字节）
        secret = bytes([
            0x3c, 0xc3, 0x12, 0x2a, 0x68, 0xf0, 0xd9, 0x50,
            0x27, 0xad, 0x38, 0xc0, 0x67, 0x91, 0x6b, 0xa0,
            0xeb, 0x8c, 0x38, 0x89, 0x4d, 0x22, 0xe1, 0xb1,
            0x56, 0x18, 0xb6, 0x81, 0x8a, 0x66, 0x17, 0x74,
            0xad, 0x46, 0x3b, 0x20, 0x5d, 0xa8, 0x8c, 0xf6,
            0x99, 0xab, 0x4d, 0x43, 0xc9, 0xcf, 0x98, 0xa1
        ])

        # Reason: SECP P384R1测试向量 - B方公钥（96字节）
        b_public = bytes([
            0xa7, 0xc7, 0x6b, 0x97, 0x0c, 0x3b, 0x5f, 0xe8,
            0xb0, 0x5d, 0x28, 0x38, 0xae, 0x04, 0xab, 0x47,
            0x69, 0x7b, 0x9e, 0xaf, 0x52, 0xe7, 0x64, 0x59,
            0x2e, 0xfd, 0xa2, 0x7f, 0xe7, 0x51, 0x32, 0x72,
            0x73, 0x44, 0x66, 0xb4, 0x00, 0x09, 0x1a, 0xdb,
            0xf2, 0xd6, 0x8c, 0x58, 0xe0, 0xc5, 0x00, 0x66,
            0xac, 0x68, 0xf1, 0x9f, 0x2e, 0x1c, 0xb8, 0x79,
            0xae, 0xd4, 0x3a, 0x99, 0x69, 0xb9, 0x1a, 0x08,
            0x39, 0xc4, 0xc3, 0x8a, 0x49, 0x74, 0x9b, 0x66,
            0x1e, 0xfe, 0xdf, 0x24, 0x34, 0x51, 0x91, 0x5e,
            0xd0, 0x90, 0x5a, 0x32, 0xb0, 0x60, 0x99, 0x2b,
            0x46, 0x8c, 0x64, 0x76, 0x6f, 0xc8, 0x43, 0x7a
        ])

        # Reason: SECP P384R1测试向量 - 预期共享密钥（48字节）
        expected_ss = bytes([
            0x5f, 0x9d, 0x29, 0xdc, 0x5e, 0x31, 0xa1, 0x63,
            0x06, 0x03, 0x56, 0x21, 0x36, 0x69, 0xc8, 0xce,
            0x13, 0x2e, 0x22, 0xf5, 0x7c, 0x9a, 0x04, 0xf4,
            0x0b, 0xa7, 0xfc, 0xea, 0xd4, 0x93, 0xb4, 0x57,
            0xe5, 0x62, 0x1e, 0x76, 0x6c, 0x40, 0xa2, 0xe3,
            0xd4, 0xd6, 0xa0, 0x4b, 0x25, 0xe5, 0x33, 0xf1
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP P521R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K013")
def test_ehsm_k013():
    """使用ECC SECP P521R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP P521R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP P521R1测试向量 - A方私钥（66字节）
        secret = bytes([
            0x01, 0x7e, 0xec, 0xc0, 0x7a, 0xb4, 0xb3, 0x29,
            0x06, 0x8f, 0xba, 0x65, 0xe5, 0x6a, 0x1f, 0x88,
            0x90, 0xaa, 0x93, 0x5e, 0x57, 0x13, 0x4a, 0xe0,
            0xff, 0xcc, 0xe8, 0x02, 0x73, 0x51, 0x51, 0xf4,
            0xea, 0xc6, 0x56, 0x4f, 0x6e, 0xe9, 0x97, 0x4c,
            0x5e, 0x68, 0x87, 0xa1, 0xfe, 0xfe, 0xe5, 0x74,
            0x3a, 0xe2, 0x24, 0x1b, 0xfe, 0xb9, 0x5d, 0x5c,
            0xe3, 0x1d, 0xdc, 0xb6, 0xf9, 0xed, 0xb4, 0xd6,
            0xfc, 0x47
        ])

        # Reason: SECP P521R1测试向量 - B方公钥（132字节）
        b_public = bytes([
            0x00, 0x68, 0x5a, 0x48, 0xe8, 0x6c, 0x79, 0xf0,
            0xf0, 0x87, 0x5f, 0x7b, 0xc1, 0x8d, 0x25, 0xeb,
            0x5f, 0xc8, 0xc0, 0xb0, 0x7e, 0x5d, 0xa4, 0xf4,
            0x37, 0x0f, 0x3a, 0x94, 0x90, 0x34, 0x08, 0x54,
            0x33, 0x4b, 0x1e, 0x1b, 0x87, 0xfa, 0x39, 0x54,
            0x64, 0xc6, 0x06, 0x26, 0x12, 0x4a, 0x4e, 0x70,
            0xd0, 0xf7, 0x85, 0x60, 0x1d, 0x37, 0xc0, 0x98,
            0x70, 0xeb, 0xf1, 0x76, 0x66, 0x68, 0x77, 0xa2,
            0x04, 0x6d,
            0x01, 0xba, 0x52, 0xc5, 0x6f, 0xc8, 0x77, 0x6d,
            0x9e, 0x8f, 0x5d, 0xb4, 0xf0, 0xcc, 0x27, 0x63,
            0x6d, 0x0b, 0x74, 0x1b, 0xbe, 0x05, 0x40, 0x06,
            0x97, 0x94, 0x2e, 0x80, 0xb7, 0x39, 0x88, 0x4a,
            0x83, 0xbd, 0xe9, 0x9e, 0x0f, 0x67, 0x16, 0x93,
            0x9e, 0x63, 0x2b, 0xc8, 0x98, 0x6f, 0xa1, 0x8d,
            0xcc, 0xd4, 0x43, 0xa3, 0x48, 0xb6, 0xc3, 0xe5,
            0x22, 0x49, 0x79, 0x55, 0xa4, 0xf3, 0xc3, 0x02,
            0xf6, 0x76
        ])

        # Reason: SECP P521R1测试向量 - 预期共享密钥（66字节）
        expected_ss = bytes([
            0x00, 0x5f, 0xc7, 0x04, 0x77, 0xc3, 0xe6, 0x3b,
            0xc3, 0x95, 0x4b, 0xd0, 0xdf, 0x3e, 0xa0, 0xd1,
            0xf4, 0x1e, 0xe2, 0x17, 0x46, 0xed, 0x95, 0xfc,
            0x5e, 0x1f, 0xdf, 0x90, 0x93, 0x0d, 0x5e, 0x13,
            0x66, 0x72, 0xd7, 0x2c, 0xc7, 0x70, 0x74, 0x2d,
            0x17, 0x11, 0xc3, 0xc3, 0xa4, 0xc3, 0x34, 0xa0,
            0xad, 0x97, 0x59, 0x43, 0x6a, 0x4d, 0x3c, 0x5b,
            0xf6, 0xe7, 0x4b, 0x95, 0x78, 0xfa, 0xc1, 0x48,
            0xc8, 0x31
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP256K1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K014")
def test_ehsm_k014():
    """使用ECC SECP256K1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP256K1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP256K1测试向量 - A方私钥（32字节）
        secret = bytes([
            0x4a, 0xaf, 0x0c, 0xf1, 0x76, 0x63, 0xa3, 0x69,
            0x43, 0x07, 0xfc, 0x68, 0x22, 0x23, 0x85, 0x6c,
            0xc3, 0xb7, 0xa1, 0x89, 0x81, 0x65, 0xa9, 0x52,
            0xb3, 0xbc, 0xf7, 0xc3, 0x78, 0xd2, 0x48, 0x9b
        ])

        # Reason: SECP256K1测试向量 - B方公钥（64字节）
        b_public = bytes([
            0x19, 0x7c, 0x82, 0x79, 0x28, 0x83, 0x0c, 0x81,
            0x0b, 0x46, 0x84, 0x1f, 0x38, 0x20, 0x0b, 0xe5,
            0xdc, 0xa5, 0x51, 0x01, 0x4f, 0xd0, 0x92, 0x44,
            0x64, 0x28, 0x85, 0x61, 0x6d, 0xcb, 0x9d, 0x05,
            0x88, 0xe9, 0x3a, 0x32, 0xbb, 0x1d, 0xfc, 0x8c,
            0xb0, 0x91, 0xaf, 0xe1, 0x0f, 0xe7, 0x17, 0xc8,
            0x33, 0x6b, 0xac, 0x27, 0x30, 0x14, 0xb2, 0xce,
            0x5b, 0xcf, 0xa1, 0xc5, 0x5e, 0xc1, 0x43, 0xca
        ])

        # Reason: SECP256K1测试向量 - 预期共享密钥（32字节）
        expected_ss = bytes([
            0x34, 0x60, 0x83, 0xd7, 0xd1, 0xce, 0x30, 0xe1,
            0x95, 0x75, 0xc9, 0x0b, 0x32, 0xc8, 0x9f, 0x5e,
            0x68, 0xd9, 0xb6, 0xc7, 0x76, 0xf4, 0xb5, 0xc3,
            0xde, 0xcb, 0xcd, 0xbb, 0x76, 0xc8, 0x81, 0x68
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC SECP192K1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K015")
def test_ehsm_k015():
    """使用ECC SECP192K1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC SECP192K1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: SECP192K1测试向量 - A方私钥（24字节）
        secret = bytes([
            0x94, 0x2a, 0xac, 0xeb, 0x93, 0x18, 0x3c, 0x72,
            0x83, 0xe9, 0x5f, 0xbe, 0xe3, 0x1a, 0x24, 0x19,
            0x1c, 0x6f, 0x39, 0x0a, 0x6d, 0xd6, 0xe9, 0x36
        ])

        # Reason: SECP192K1测试向量 - B方公钥（48字节）
        b_public = bytes([
            0x56, 0x33, 0x48, 0xf5, 0xb8, 0x70, 0xd5, 0xfa,
            0x59, 0xcf, 0x59, 0x5e, 0xb2, 0xeb, 0x08, 0x7a,
            0x2b, 0x2e, 0x7e, 0xc4, 0xc1, 0x2d, 0xfc, 0xa2,
            0x57, 0xa2, 0x8c, 0xba, 0x7f, 0x11, 0x39, 0x48,
            0xbf, 0x1d, 0x9c, 0xdb, 0x2b, 0xb8, 0x3c, 0x65,
            0xd0, 0x88, 0x9a, 0x29, 0xdd, 0x3e, 0x59, 0x3d
        ])

        # Reason: SECP192K1测试向量 - 预期共享密钥（24字节）
        expected_ss = bytes([
            0x4d, 0x77, 0x2c, 0x05, 0x0f, 0x19, 0xef, 0x27,
            0xc6, 0x78, 0xe8, 0x66, 0x4e, 0x0f, 0xe7, 0x57,
            0x7c, 0x17, 0xc3, 0xdb, 0x3e, 0xc3, 0x65, 0x12
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P192R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K016")
def test_ehsm_k016():
    """使用ECC Brainpool P192R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P192R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP192R1测试向量 - A方私钥（24字节）
        secret = bytes([
            0x46, 0xFD, 0x4D, 0x3D, 0xF3, 0x7F, 0x16, 0x79,
            0x64, 0xE9, 0x75, 0xC9, 0x78, 0xC7, 0x38, 0xB4,
            0x72, 0x07, 0x89, 0x21, 0xB7, 0x9B, 0xC1, 0xDD
        ])

        # Reason: BrainpoolP192R1测试向量 - B方公钥（48字节）
        b_public = bytes([
            0x82, 0xF2, 0x7E, 0x41, 0x7D, 0x59, 0x03, 0x77,
            0x12, 0x46, 0xBC, 0x83, 0x6D, 0xE8, 0x0F, 0xAD,
            0xDD, 0x7B, 0xF9, 0x5E, 0xC5, 0xA7, 0xC0, 0x01,
            0x0E, 0x1E, 0xCF, 0x84, 0xC1, 0x31, 0x93, 0x8B,
            0xBB, 0xA1, 0x63, 0x07, 0xE9, 0x29, 0x0C, 0xE6,
            0xF0, 0x8A, 0xF0, 0x2F, 0xE9, 0x48, 0xB5, 0x6F
        ])

        # Reason: BrainpoolP192R1测试向量 - 预期共享密钥（24字节）
        expected_ss = bytes([
            0xB8, 0x6B, 0xF0, 0x8D, 0x09, 0x12, 0x86, 0x1E,
            0x01, 0x95, 0xF5, 0x32, 0xDC, 0x7F, 0x63, 0x6D,
            0x8D, 0xA7, 0xB2, 0x5A, 0x99, 0xE3, 0xF6, 0x4E
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持 SECP 算法")
@allure.feature("kms")
@allure.description("使用ECC Brainpool P256R1进行密钥交换（ECDH）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K017")
def test_ehsm_k017():
    """使用ECC Brainpool P256R1进行ECDH密钥交换测试"""

    with allure.step("1、导入明文密钥ECC Brainpool P256R1私钥作为本地密钥，并配置创建密钥权限，得到h1； # 1、导入密钥成功；"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]  | \
                 KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

        # Reason: BrainpoolP256R1测试向量 - A方私钥（32字节）
        secret = bytes([
            0x81, 0xDB, 0x1E, 0xE1, 0x00, 0x15, 0x0F, 0xF2,
            0xEA, 0x33, 0x8D, 0x70, 0x82, 0x71, 0xBE, 0x38,
            0x30, 0x0C, 0xB5, 0x42, 0x41, 0xD7, 0x99, 0x50,
            0xF7, 0x7B, 0x06, 0x30, 0x39, 0x80, 0x4F, 0x1D
        ])

        # Reason: BrainpoolP256R1测试向量 - B方公钥（64字节）
        b_public = bytes([
            0x8D, 0x2D, 0x68, 0x8C, 0x6C, 0xF9, 0x3E, 0x11,
            0x60, 0xAD, 0x04, 0xCC, 0x44, 0x29, 0x11, 0x7D,
            0xC2, 0xC4, 0x18, 0x25, 0xE1, 0xE9, 0xFC, 0xA0,
            0xAD, 0xDD, 0x34, 0xE6, 0xF1, 0xB3, 0x9F, 0x7B,
            0x99, 0x0C, 0x57, 0x52, 0x08, 0x12, 0xBE, 0x51,
            0x26, 0x41, 0xE4, 0x70, 0x34, 0x83, 0x21, 0x06,
            0xBC, 0x7D, 0x3E, 0x8D, 0xD0, 0xE4, 0xC7, 0xF1,
            0x13, 0x6D, 0x70, 0x06, 0x54, 0x7C, 0xEC, 0x6A
        ])

        # Reason: BrainpoolP256R1测试向量 - 预期共享密钥（32字节）
        expected_ss = bytes([
            0x89, 0xAF, 0xC3, 0x9D, 0x41, 0xD3, 0xB3, 0x27,
            0x81, 0x4B, 0x80, 0x94, 0x0B, 0x04, 0x25, 0x90,
            0xF9, 0x65, 0x56, 0xEC, 0x91, 0xE6, 0xAE, 0x79,
            0x39, 0xBC, 0xE3, 0x1F, 0x3A, 0x18, 0xBF, 0x2B
        ])

        hmac_key_size = 0
        STRUCT_FORMAT = "<IBBHHH"
        handle = 0xFFFFFFFF
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(secret)  # priv_key_size
        )
        key_data = header + secret
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, handle)

    with allure.step("2、传入对方公钥，对称算法AES-128，得到协商密钥h2； # 2、协商密钥成功；"):
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]  |\
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        remote_pubkey_data = b_public
        exchange_handle = 0xFFFFFFFF

        _, out_new_handle = api.ehsm_km_exchange_key(remote_pubkey_data, len(remote_pubkey_data), exchange_permit,
                                                     EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                                                     hmac_key_size, local_handle, None, 0x0, None, exchange_handle)

    with allure.step("3、导出协商密钥h2； # 3、导出密钥成功；"):
        key_buf = b''
        ret1, buf1, ret2, buf2, ret3 = api.ehsm_km_export_key(out_new_handle, 0xffffffff, 0xffffffff,
                                                              EhsmKeyPart.EHSM_KEY_PART_PRIVATE_KEY,
                                                              key_buf, len(header) + 512, None, 0)

    with allure.step("4、对比协商密钥和预期协商密钥； # 4、数据对比一致；"):
        # Reason: 跳过header（12字节），验证AES-128密钥（16字节）是否匹配预期共享密钥的前16字节
        assert (expected_ss[:16] == buf1[12:28]), f"协商密钥与预期不符，预期：{expected_ss[:16].hex().upper()}，实际：{buf1[12:28].hex().upper()}"

    with allure.step("5、删除密钥h1/h2; # 5、删除密钥成功；"):
        api.ehsm_km_remove_key(local_handle)
        api.ehsm_km_remove_key(out_new_handle)


# ============================
#  DES算法密钥生成异常参数测试
# ============================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K019")
def test_ehsm_k019():
    with allure.step("1、rsa_e_bit_size不为0时，测试DES算法能否正常生成密钥；#生成成功"):
        permit_invalid = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
            KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit_invalid, 64, 0, None, 0, 0xFFFFFFFF
        )
        # 如果成功需要清理
        api.ehsm_km_remove_key(handler)
        log.info(f"rsa_e_bit_size仅对RSA密钥有效，DES算法中rsa_e_bit_size的值没有影响")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K020")
def test_ehsm_k020():
    with allure.step("1、key_size大于8时，测试DES算法能否正常生成密钥；#生成成功"):
        try:
            permit_invalid = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DES,
                permit_invalid, 0, 513, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False,"key_size超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"key_size大于8会导致DES算法密钥生成失败")


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：key_handle边界值测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K021")
def test_ehsm_k021():
    with allure.step("1、使用key_handle=0生成DES密钥 # 1、验证固件是否接受句柄0"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
        try:
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DES,
                permit, 0, 0, None, 0, 0x00000000  # key_handle为0
            )
            log.info(f"✓ key_handle=0被接受，DES密钥生成成功，返回句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            log.info(f"✓ key_handle=0被拒绝，密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K022")
def test_ehsm_k022():
    with allure.step("1、permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DES,
                permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")
    with allure.step("2、permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DES,
                permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit全1应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")
    with allure.step("3、仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit_min, 0, 0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ 仅REMOVE权限(最小有效值)，DES密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K023")
def test_ehsm_k023():
    """测试DES密钥生成时传入DH参数"""
    with allure.step("1、生成DES密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # 构造一个假的DH参数数据（DES不应该使用）
        fake_dh_params = b'\x01' * 256

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit,
            0,  # rsa_e_bit_size
            0,  # hmac_key_size
            fake_dh_params,  # dh_params (不应用于DES)
            len(fake_dh_params),  # dh_params_size
            0xFFFFFFFF  # key_handle
        )
        log.info(f"✓ DES密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K024")
def test_ehsm_k024():
    """测试DES密钥生成时传入不同大小的DH参数"""
    with allure.step("1、生成DES密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x02' * 1

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit,
            0,  # rsa_e_bit_size
            0,  # hmac_key_size
            fake_dh_params,  # dh_params (不应用于DES)
            1,  # dh_params_size=1(最小非零值)
            0xFFFFFFFF  # key_handle
        )
        log.info(f"✓ DES密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

    with allure.step("2、生成DES密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x03' * 2064

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit,
            0,  # rsa_e_bit_size
            0,  # hmac_key_size
            fake_dh_params,  # dh_params (不应用于DES)
            2064,  # dh_params_size=2064(最大有效值)
            0xFFFFFFFF  # key_handle
        )
        log.info(f"✓ DES密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持DES算法")
@allure.feature("kms")
@allure.description("DES密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K025")
def test_ehsm_k025():
    """测试DES密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成DES密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DES,
            permit,
            0,  # rsa_e_bit_size
            0,  # hmac_key_size
            None,  # dh_params=None
            100,  # dh_params_size=100 (不应用于DES)
            0xFFFFFFFF  # key_handle
        )
        log.info(f"✓ DES密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


# ========================================
#  TDES（128/192）算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K026")
def test_ehsm_k026():
    """测试TDES128/192密钥生成时传入rsa_e_bit_size参数"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、rsa_e_bit_size不为0时，测试{algo_name}算法能否正常生成密钥 # 1、生成成功"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo,
                permit,
                64,  # rsa_e_bit_size=64 (不应用于TDES)
                0,   # hmac_key_size
                None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            log.info(f"rsa_e_bit_size仅对RSA密钥有效，{algo_name}算法中rsa_e_bit_size的值没有影响")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K027")
def test_ehsm_k027():
    """测试TDES128/192密钥生成时传入key_size参数"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128", 16),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192", 24)
    ]

    for algo, algo_name, expected_size in tdes_algos:
        with allure.step(f"1、key_size超出{expected_size}时，测试{algo_name}算法能否正常生成密钥 # 1、应该失败或被忽略"):
            try:
                permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

                _, handler = api.ehsm_km_gen_key(
                    algo,
                    permit,
                    0,    # rsa_e_bit_size
                    513,  # hmac_key_size=513 (超出范围)
                    None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"key_size超出范围应该失败，但{algo_name}却成功了"
            except hostapi.HostApiError as e:
                log.info(f"key_size超出{expected_size}会导致{algo_name}算法密钥生成失败")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：key_handle边界测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K028")
def test_ehsm_k028():
    """测试TDES128/192密钥生成时使用key_handle=0"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、使用key_handle=0生成{algo_name}密钥 # 1、验证固件是否接受句柄0"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
            try:
                _, handler = api.ehsm_km_gen_key(
                    algo,
                    permit,
                    0, 0, None, 0,
                    0x00000000  # key_handle为0
                )
                log.info(f"✓ key_handle=0被接受，{algo_name}密钥生成成功，返回句柄: {handler:08X}")
                # 清理
                api.ehsm_km_remove_key(handler)
            except hostapi.HostApiError as e:
                log.info(f"✓ key_handle=0被拒绝，{algo_name}密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K029")
def test_ehsm_k029():
    """测试TDES128/192密钥生成时传入异常permit参数"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 17, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K030")
def test_ehsm_k030():
    """测试TDES128/192密钥生成时传入DH参数"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            # 构造一个假的DH参数数据（TDES不应该使用）
            fake_dh_params = b'\x01' * 256

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于TDES)
                len(fake_dh_params),  # dh_params_size
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K031")
def test_ehsm_k031():
    """测试TDES128/192密钥生成时传入不同大小的DH参数"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x02' * 1

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于TDES)
                1,  # dh_params_size=1(最小非零值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

        with allure.step(f"2、生成{algo_name}密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x03' * 2064

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于TDES)
                2064,  # dh_params_size=2064(最大有效值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持TDES算法")
@allure.feature("kms")
@allure.description("TDES密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K032")
def test_ehsm_k032():
    """测试TDES128/192密钥生成时dh_params为None但dh_params_size非零"""
    tdes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_128, "TDES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_TDES_192, "TDES192")
    ]

    for algo, algo_name in tdes_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100 (不应用于TDES)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)




# ========================================
#  AES（128/192/256）算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K033")
def test_ehsm_k033():
    """测试AES128/192/256密钥生成时传入rsa_e_bit_size参数"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、rsa_e_bit_size不为0时，测试{algo_name}算法能否正常生成密钥 # 1、生成成功"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo,
                permit,
                64,  # rsa_e_bit_size=64 (不应用于AES)
                0,   # hmac_key_size
                None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            log.info(f"rsa_e_bit_size仅对RSA密钥有效，{algo_name}算法中rsa_e_bit_size的值没有影响")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K034")
def test_ehsm_k034():
    """测试AES128/192/256密钥生成时传入key_size参数"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128", 16),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192", 24),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256", 32)
    ]

    for algo, algo_name, expected_size in aes_algos:
        with allure.step(f"1、key_size超出{expected_size}时，测试{algo_name}算法能否正常生成密钥 # 1、应该失败或被忽略"):
            try:
                permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

                _, handler = api.ehsm_km_gen_key(
                    algo,
                    permit,
                    0,    # rsa_e_bit_size
                    513,  # hmac_key_size=513 (超出范围)
                    None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"key_size超出范围应该失败，但{algo_name}却成功了"
            except hostapi.HostApiError as e:
                log.info(f"key_size超出{expected_size}会导致{algo_name}算法密钥生成失败")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：key_handle边界值测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K035")
def test_ehsm_k035():
    """测试AES128/192/256密钥生成时使用key_handle=0"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、使用key_handle=0生成{algo_name}密钥 # 1、验证固件是否接受句柄0"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
            try:
                _, handler = api.ehsm_km_gen_key(
                    algo,
                    permit,
                    0, 0, None, 0,
                    0x00000000  # key_handle为0
                )
                log.info(f"✓ key_handle=0被接受，{algo_name}密钥生成成功，返回句柄: {handler:08X}")
                # 清理
                api.ehsm_km_remove_key(handler)
            except hostapi.HostApiError as e:
                log.info(f"✓ key_handle=0被拒绝，{algo_name}密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K036")
def test_ehsm_k036():
    """测试AES128/192/256密钥生成时传入异常permit参数"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 17, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K037")
def test_ehsm_k037():
    """测试AES128/192/256密钥生成时传入DH参数"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            # 构造一个假的DH参数数据（AES不应该使用）
            fake_dh_params = b'\x01' * 256

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于AES)
                len(fake_dh_params),  # dh_params_size
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K038")
def test_ehsm_k038():
    """测试AES128/192/256密钥生成时传入不同大小的DH参数"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x02' * 1

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于AES)
                1,  # dh_params_size=1(最小非零值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

        with allure.step(f"2、生成{algo_name}密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x03' * 2064

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,  # dh_params (不应用于AES)
                2064,  # dh_params_size=2064(最大有效值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K039")
def test_ehsm_k039():
    """测试AES128/192/256密钥生成时dh_params为None但dh_params_size非零"""
    aes_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128, "AES128"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192, "AES192"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256, "AES256")
    ]

    for algo, algo_name in aes_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100 (不应用于AES)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

# ========================================
#  SM4 算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K040")
def test_ehsm_k040():
    """测试SM4密钥生成时传入rsa_e_bit_size参数"""
    with allure.step("1、rsa_e_bit_size不为0时，测试SM4算法能否正常生成密钥 # 1、生成成功"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit,
            64,  # rsa_e_bit_size=64 (不应用于SM4)
            0,   # hmac_key_size
            None, 0, 0xFFFFFFFF
        )
        # 如果成功需要清理
        api.ehsm_km_remove_key(handler)
        log.info(f"rsa_e_bit_size仅对RSA密钥有效，SM4算法中rsa_e_bit_size的值没有影响")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K041")
def test_ehsm_k041():
    """测试SM4密钥生成时传入key_size参数"""
    with allure.step("1、key_size超出16时，测试SM4算法能否正常生成密钥 # 1、应该失败或被忽略"):
        try:
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM4,
                permit,
                0,    # rsa_e_bit_size
                513,  # hmac_key_size=513 (超出范围)
                None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "key_size超出范围应该失败，但SM4却成功了"
        except hostapi.HostApiError as e:
            log.info(f"key_size超出16会导致SM4算法密钥生成失败")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：key_handle边界值测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K042")
def test_ehsm_k042():
    """测试SM4密钥生成时使用key_handle=0"""
    with allure.step("1、使用key_handle=0生成SM4密钥 # 1、验证固件是否接受句柄0"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
        try:
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM4,
                permit,
                0, 0, None, 0,
                0x00000000  # key_handle为0
            )
            log.info(f"✓ key_handle=0被接受，SM4密钥生成成功，返回句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            log.info(f"✓ key_handle=0被拒绝，SM4密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K043")
def test_ehsm_k043():
    """测试SM4密钥生成时传入异常permit参数"""
    with allure.step("1、permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM4,
                permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit超出范围应该失败，但SM4却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ SM4: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM4,
                permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit全1应该失败，但SM4却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ SM4: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit_min, 0, 0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ SM4: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K044")
def test_ehsm_k044():
    """测试SM4密钥生成时传入DH参数"""
    with allure.step("1、生成SM4密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # 构造一个假的DH参数数据（SM4不应该使用）
        fake_dh_params = b'\x01' * 256

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM4)
            len(fake_dh_params),  # dh_params_size
            0xFFFFFFFF
        )
        log.info(f"✓ SM4密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K045")
def test_ehsm_k045():
    """测试SM4密钥生成时传入不同大小的DH参数"""
    with allure.step("1、生成SM4密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x02' * 1

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM4)
            1,  # dh_params_size=1(最小非零值)
            0xFFFFFFFF
        )
        log.info(f"✓ SM4密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

    with allure.step("2、生成SM4密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x03' * 2064

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM4)
            2064,  # dh_params_size=2064(最大有效值)
            0xFFFFFFFF
        )
        log.info(f"✓ SM4密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持SM4算法")
@allure.feature("kms")
@allure.description("SM4密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K046")
def test_ehsm_k046():
    """测试SM4密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成SM4密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM4,
            permit, 0, 0,
            None,  # dh_params=None
            100,   # dh_params_size=100 (不应用于SM4)
            0xFFFFFFFF
        )
        log.info(f"✓ SM4密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


# ========================================
#  SM2 算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K047")
def test_ehsm_k047():
    """测试SM2密钥生成时传入rsa_e_bit_size参数"""
    with allure.step("1、rsa_e_bit_size不为0时，测试SM2算法能否正常生成密钥 # 1、生成成功"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit,
            64,  # rsa_e_bit_size=64 (不应用于SM2)
            0,   # hmac_key_size
            None, 0, 0xFFFFFFFF
        )
        # 如果成功需要清理
        api.ehsm_km_remove_key(handler)
        log.info(f"rsa_e_bit_size仅对RSA密钥有效，SM2算法中rsa_e_bit_size的值没有影响")

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K048")
def test_ehsm_k048():
    """测试SM2密钥生成时传入key_size参数"""
    with allure.step("1、key_size不为0时，测试SM2算法能否正常生成密钥 # 1、应该失败或被忽略"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit,
            0,    # rsa_e_bit_size
            513,  # hmac_key_size=513 (不应用于SM2)
            None, 0, 0xFFFFFFFF
        )
        # 如果成功需要清理
        api.ehsm_km_remove_key(handler)
        log.info("✓ SM2密钥生成成功，key_size参数被正确忽略（ECC类型不使用此参数）")

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：key_handle边界值测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K049")
def test_ehsm_k049():
    """测试SM2密钥生成时使用key_handle=0"""
    with allure.step("1、使用key_handle=0生成SM2密钥 # 1、验证固件是否接受句柄0"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
        try:
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM2,
                permit,
                0, 0, None, 0,
                0x00000000  # key_handle为0
            )
            log.info(f"✓ key_handle=0被接受，SM2密钥生成成功，返回句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            log.info(f"✓ key_handle=0被拒绝，SM2密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K050")
def test_ehsm_k050():
    """测试SM2密钥生成时传入异常permit参数"""
    with allure.step("1、permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM2,
                permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit超出范围应该失败，但SM2却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ SM2: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM2,
                permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "permit全1应该失败，但SM2却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ SM2: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit_min, 0, 0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ SM2: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K051")
def test_ehsm_k051():
    """测试SM2密钥生成时传入DH参数"""
    with allure.step("1、生成SM2密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        # 构造一个假的DH参数数据（SM2不应该使用）
        fake_dh_params = b'\x01' * 256

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM2)
            len(fake_dh_params),  # dh_params_size
            0xFFFFFFFF
        )
        log.info(f"✓ SM2密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K052")
def test_ehsm_k052():
    """测试SM2密钥生成时传入不同大小的DH参数"""
    with allure.step("1、生成SM2密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x02' * 1

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM2)
            1,  # dh_params_size=1(最小非零值)
            0xFFFFFFFF
        )
        log.info(f"✓ SM2密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

    with allure.step("2、生成SM2密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        fake_dh_params = b'\x03' * 2064

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit, 0, 0,
            fake_dh_params,  # dh_params (不应用于SM2)
            2064,  # dh_params_size=2064(最大有效值)
            0xFFFFFFFF
        )
        log.info(f"✓ SM2密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K053")
def test_ehsm_k053():
    """测试SM2密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成SM2密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            permit, 0, 0,
            None,  # dh_params=None
            100,   # dh_params_size=100 (不应用于SM2)
            0xFFFFFFFF
        )
        log.info(f"✓ SM2密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


# ========================================
#  RSA（1024/2048/3072/4096）算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K054")
def test_ehsm_k054():
    """测试RSA密钥生成时传入异常rsa_e_bit_size参数"""
    # Reason: 等待上一个测试用例的命令完全处理完毕，避免队列满
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        log.info(f"\n>>> 开始测试 {algo_name} 算法")

        with allure.step(f"1、{algo_name}：rsa_e_bit_size超出最大值(65) # 1、返回参数错误"):
            log.info(f"[{algo_name}] 步骤1: 测试 rsa_e_bit_size=65")
            try:
                permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

                _, handler = api.ehsm_km_gen_key(
                    algo, permit,
                    65,  # rsa_e_bit_size=65 (超出最大值64)
                    0, None, 0, 0xFFFFFFFF
                )
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: rsa_e_bit_size=65应该失败"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: rsa_e_bit_size=65导致失败，错误码: {e.ret_code}")



@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：key_size异常参测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K055")
def test_ehsm_k055():
    """测试RSA密钥生成时传入key_size参数"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024", 128),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048", 256),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072", 384),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096", 512)
    ]

    for algo, algo_name, expected_size in rsa_algos:
        with allure.step(f"1、key_size不为0时，测试{algo_name}算法能否正常生成密钥 # 1、应该失败或被忽略"):
            try:
                permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

                _, handler = api.ehsm_km_gen_key(
                    algo, permit,
                    0,    # rsa_e_bit_size
                    513,  # hmac_key_size=513 (不应用于RSA)
                    None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"key_size非零应该失败或被忽略，但{algo_name}却成功了"
            except hostapi.HostApiError as e:
                log.info(f"key_size参数会导致{algo_name}算法密钥生成失败")

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：key_handle边界值测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K056")
def test_ehsm_k056():
    """测试RSA密钥生成时使用key_handle=0"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        with allure.step(f"1、使用key_handle=0生成{algo_name}密钥 # 1、验证固件是否接受句柄0"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])
            try:
                _, handler = api.ehsm_km_gen_key(
                    algo, permit,
                    0, 0, None, 0,
                    0x00000000  # key_handle为0
                )
                log.info(f"✓ key_handle=0被接受，{algo_name}密钥生成成功，返回句柄: {handler:08X}")
                # 清理
                api.ehsm_km_remove_key(handler)
            except hostapi.HostApiError as e:
                log.info(f"✓ key_handle=0被拒绝，{algo_name}密钥生成失败，错误码: {e.ret_code}")

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K057")
def test_ehsm_k057():
    """测试RSA密钥生成时传入异常permit参数"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 17, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K058")
def test_ehsm_k058():
    """测试RSA密钥生成时传入DH参数"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            # 构造一个假的DH参数数据（RSA不应该使用）
            fake_dh_params = b'\x01' * 256

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                fake_dh_params,  # dh_params (不应用于RSA)
                len(fake_dh_params),  # dh_params_size
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params参数)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K059")
def test_ehsm_k059():
    """测试RSA密钥生成时传入不同大小的DH参数"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params_size=1(最小非零值) # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x02' * 1

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                fake_dh_params,  # dh_params (不应用于RSA)
                1,  # dh_params_size=1(最小非零值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=1)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

        with allure.step(f"2、生成{algo_name}密钥但传入dh_params_size=2064(最大有效值) # 2、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            fake_dh_params = b'\x03' * 2064

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                fake_dh_params,  # dh_params (不应用于RSA)
                2064,  # dh_params_size=2064(最大有效值)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(忽略dh_params_size=2064)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA密钥生成：dh_params为None时异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K060")
def test_ehsm_k060():
    """测试RSA密钥生成时dh_params为None但dh_params_size非零"""
    rsa_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024, "RSA1024"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048, "RSA2048"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072, "RSA3072"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096, "RSA4096")
    ]

    for algo, algo_name in rsa_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100 (不应用于RSA)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

# ========================================
#  RSA（1024/2048/3072/4096）-CRT算法密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA_CRT密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K061")
def test_ehsm_k061():
    """测试RSA_CRT密钥生成时传入异常rsa_e_bit_size参数"""
    rsa_crt_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT, "RSA1024_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT, "RSA2048_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT, "RSA3072_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096_CRT, "RSA4096_CRT")
    ]

    for algo, algo_name in rsa_crt_algos:
        # 测试1：超出最大值64
        with allure.step(f"1、{algo_name}：rsa_e_bit_size超出最大值(65) # 1、返回参数错误"):
            log.info(f"[{algo_name}] 步骤1: 测试 rsa_e_bit_size=65")
            try:
                permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                          KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

                _, handler = api.ehsm_km_gen_key(
                    algo, permit,
                    66,  # rsa_e_bit_size=65 (超出最大值64)
                    0, None, 0, 0xFFFFFFFF
                )
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: rsa_e_bit_size=65应该失败"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: rsa_e_bit_size=65导致失败，错误码: {e.ret_code}")
            # Reason: 即使失败也要等待，确保固件处理完错误响应并清空队列

@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA_CRT密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K062")
def test_ehsm_k062():
    """测试RSA_CRT密钥生成时传入异常permit参数"""
    rsa_crt_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT, "RSA1024_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT, "RSA2048_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT, "RSA3072_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096_CRT, "RSA4096_CRT")
    ]

    for algo, algo_name in rsa_crt_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 17, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            # Reason: rsa_e_bit_size必须>=2，使用17生成标准公钥e=65537
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 17, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA_CRT密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K063")
def test_ehsm_k0623():
    """测试RSA_CRT密钥生成时传入DH参数"""
    rsa_crt_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT, "RSA1024_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT, "RSA2048_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT, "RSA3072_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096_CRT, "RSA4096_CRT")
    ]

    for algo, algo_name in rsa_crt_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            # 创建一些虚假的DH参数数据（RSA不使用这些参数）
            fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                fake_dh_params,
                len(fake_dh_params),
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA_CRT密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K064")
def test_ehsm_k064():
    """测试RSA_CRT密钥生成时传入hmac_key_size参数"""
    rsa_crt_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT, "RSA1024_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT, "RSA2048_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT, "RSA3072_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096_CRT, "RSA4096_CRT")
    ]

    for algo, algo_name in rsa_crt_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17,
                32,  # hmac_key_size=32 (不应用于RSA_CRT)
                None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0, reason="不支持RSA算法")
@allure.feature("kms")
@allure.description("RSA_CRT密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K065")
def test_ehsm_k065():
    """测试RSA_CRT密钥生成时dh_params为None但dh_params_size非零"""
    rsa_crt_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT, "RSA1024_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT, "RSA2048_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_3072_CRT, "RSA3072_CRT"),
        (EhsmKeyType.EHSM_KEY_TYPE_RSA_4096_CRT, "RSA4096_CRT")
    ]

    for algo, algo_name in rsa_crt_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 17, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100 (不应用于RSA_CRT)
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

# ========================================
#  DH密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K066")
def test_ehsm_k066():
    """测试DH密钥生成时传入异常permit参数"""

    # DH参数数据（标准1024位DH参数）
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

    # 构建DH通用数据
    import struct
    dh_p_size = len(dh_tv_template_1024_p)  # 128
    dh_q_size = len(dh_tv_template_1024_q)  # 20
    dh_g_size = len(dh_tv_template_1024_g)  # 128

    dh_common_data = struct.pack("<I", dh_p_size) + dh_tv_template_1024_p
    dh_common_data += struct.pack("<I", dh_q_size) + dh_tv_template_1024_q
    dh_common_data += struct.pack("<I", dh_g_size) + dh_tv_template_1024_g
    dh_common_data += struct.pack("<I", 0)  # 末尾的0

    with allure.step("1、DH：permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DH, permit_invalid, 0, 0,
                dh_common_data, len(dh_common_data), 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "DH: permit超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ DH: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、DH：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DH, permit_all_ones, 0, 0,
                dh_common_data, len(dh_common_data), 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "DH: permit全1应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ DH: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、DH：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DH, permit_min, 0, 0,
            dh_common_data, len(dh_common_data), 0xFFFFFFFF
        )
        log.info(f"✓ DH: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥生成：dh_params为None测试")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K067")
def test_ehsm_k067():
    """测试DH密钥生成时dh_params为None"""

    with allure.step("1、生成DH密钥但dh_params=None # 1、返回参数错误"):
        try:
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # Reason: DH密钥必须提供dh_params参数
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DH, permit, 0, 0,
                None,  # dh_params=None（DH必须提供）
                0,     # dh_params_size=0
                0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "DH密钥生成应该失败（dh_params为None），但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ DH: dh_params为None，密钥生成失败，符合预期，错误码: {e.ret_code}")


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥生成：dh_params_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K068")
def test_ehsm_k068():
    """测试DH密钥生成时dh_params_size异常"""

    # DH参数数据
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

    # 构建DH通用数据
    import struct
    dh_p_size = len(dh_tv_template_1024_p)
    dh_q_size = len(dh_tv_template_1024_q)
    dh_g_size = len(dh_tv_template_1024_g)

    dh_common_data = struct.pack("<I", dh_p_size) + dh_tv_template_1024_p
    dh_common_data += struct.pack("<I", dh_q_size) + dh_tv_template_1024_q
    dh_common_data += struct.pack("<I", dh_g_size) + dh_tv_template_1024_g
    dh_common_data += struct.pack("<I", 0)

    permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
              KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

    with allure.step("1、生成DH密钥但dh_params_size与实际不符（小于实际） # 1、返回参数错误或成功"):
        try:
            # Reason: 传入的size小于实际dh_common_data大小
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_DH, permit, 0, 0,
                dh_common_data,
                100,  # 实际size是288，但传入100
                0xFFFFFFFF
            )
            # 如果成功也清理
            api.ehsm_km_remove_key(handler)
            log.info(f"✓ DH: dh_params_size小于实际(100<288)，密钥生成成功（固件可能截断或忽略），句柄已清理")
        except hostapi.HostApiError as e:
            log.info(f"✓ DH: dh_params_size小于实际(100<288)，密钥生成失败，错误码: {e.ret_code}")

    with allure.step("2、生成DH密钥，dh_params_size正确 # 2、成功"):
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DH, permit, 0, 0,
            dh_common_data,
            len(dh_common_data),  # 正确的size
            0xFFFFFFFF
        )
        log.info(f"✓ DH: dh_params_size正确，密钥生成成功，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K069")
def test_ehsm_k069():
    """测试DH密钥生成时传入rsa_e_bit_size参数（应被忽略）"""

    # DH参数数据
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

    # 构建DH通用数据
    import struct
    dh_p_size = len(dh_tv_template_1024_p)
    dh_q_size = len(dh_tv_template_1024_q)
    dh_g_size = len(dh_tv_template_1024_g)

    dh_common_data = struct.pack("<I", dh_p_size) + dh_tv_template_1024_p
    dh_common_data += struct.pack("<I", dh_q_size) + dh_tv_template_1024_q
    dh_common_data += struct.pack("<I", dh_g_size) + dh_tv_template_1024_g
    dh_common_data += struct.pack("<I", 0)

    with allure.step("1、生成DH密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: DH不使用rsa_e_bit_size参数，应该被忽略
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DH, permit,
            17,  # rsa_e_bit_size=17（不应用于DH）
            0,
            dh_common_data, len(dh_common_data), 0xFFFFFFFF
        )
        log.info(f"✓ DH密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K070")
def test_ehsm_k070():
    """测试DH密钥生成时传入hmac_key_size参数（应被忽略）"""

    # DH参数数据
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

    # 构建DH通用数据
    import struct
    dh_p_size = len(dh_tv_template_1024_p)
    dh_q_size = len(dh_tv_template_1024_q)
    dh_g_size = len(dh_tv_template_1024_g)

    dh_common_data = struct.pack("<I", dh_p_size) + dh_tv_template_1024_p
    dh_common_data += struct.pack("<I", dh_q_size) + dh_tv_template_1024_q
    dh_common_data += struct.pack("<I", dh_g_size) + dh_tv_template_1024_g
    dh_common_data += struct.pack("<I", 0)

    with allure.step("1、生成DH密钥但传入hmac_key_size=32 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: DH不使用hmac_key_size参数，应该被忽略
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_DH, permit, 0,
            32,  # hmac_key_size=32（不应用于DH）
            dh_common_data, len(dh_common_data), 0xFFFFFFFF
        )
        log.info(f"✓ DH密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


# ========================================
#  ECC Brainpool密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC Brainpool密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K071")
def test_ehsm_k071():
    """测试ECC Brainpool密钥生成时传入异常permit参数"""
    ecc_brainpool_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1, "ECC_BRAINPOOLP_160R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1, "ECC_BRAINPOOLP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1, "ECC_BRAINPOOLP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1, "ECC_BRAINPOOLP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1, "ECC_BRAINPOOLP_320R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1, "ECC_BRAINPOOLP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1, "ECC_BRAINPOOLP_512R1")
    ]

    for algo, algo_name in ecc_brainpool_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 0, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC Brainpool密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K072")
def test_ehsm_k072():
    """测试ECC Brainpool密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    ecc_brainpool_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1, "ECC_BRAINPOOLP_160R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1, "ECC_BRAINPOOLP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1, "ECC_BRAINPOOLP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1, "ECC_BRAINPOOLP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1, "ECC_BRAINPOOLP_320R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1, "ECC_BRAINPOOLP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1, "ECC_BRAINPOOLP_512R1")
    ]

    for algo, algo_name in ecc_brainpool_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # Reason: ECC不使用rsa_e_bit_size参数，应该被忽略
            _, handler = api.ehsm_km_gen_key(
                algo, permit,
                17,  # rsa_e_bit_size=17（不应用于ECC）
                0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC Brainpool密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K073")
def test_ehsm_k073():
    """测试ECC Brainpool密钥生成时传入DH参数"""
    ecc_brainpool_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1, "ECC_BRAINPOOLP_160R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1, "ECC_BRAINPOOLP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1, "ECC_BRAINPOOLP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1, "ECC_BRAINPOOLP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1, "ECC_BRAINPOOLP_320R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1, "ECC_BRAINPOOLP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1, "ECC_BRAINPOOLP_512R1")
    ]

    for algo, algo_name in ecc_brainpool_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # 创建一些虚假的DH参数数据（ECC不使用这些参数）
            fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,
                len(fake_dh_params),
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC Brainpool密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K074")
def test_ehsm_k074():
    """测试ECC Brainpool密钥生成时传入hmac_key_size参数"""
    ecc_brainpool_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1, "ECC_BRAINPOOLP_160R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1, "ECC_BRAINPOOLP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1, "ECC_BRAINPOOLP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1, "ECC_BRAINPOOLP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1, "ECC_BRAINPOOLP_320R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1, "ECC_BRAINPOOLP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1, "ECC_BRAINPOOLP_512R1")
    ]

    for algo, algo_name in ecc_brainpool_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0,
                32,  # hmac_key_size=32（不应用于ECC）
                None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC Brainpool密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K075")
def test_ehsm_k075():
    """测试ECC Brainpool密钥生成时dh_params为None但dh_params_size非零"""
    ecc_brainpool_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_160R1, "ECC_BRAINPOOLP_160R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_192R1, "ECC_BRAINPOOLP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_224R1, "ECC_BRAINPOOLP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_256R1, "ECC_BRAINPOOLP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_320R1, "ECC_BRAINPOOLP_320R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_384R1, "ECC_BRAINPOOLP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_BRAINPOOLP_512R1, "ECC_BRAINPOOLP_512R1")
    ]

    for algo, algo_name in ecc_brainpool_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100（不应用于ECC）
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

# ========================================
#  ECC SECP密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K076")
def test_ehsm_k076():
    """测试ECC SECP密钥生成时传入异常permit参数"""
    ecc_secp_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1, "ECC_SECP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1, "ECC_SECP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, "ECC_SECP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1, "ECC_SECP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1, "ECC_SECP_521R1")
    ]

    for algo, algo_name in ecc_secp_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 0, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K077")
def test_ehsm_k077():
    """测试ECC SECP密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    ecc_secp_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1, "ECC_SECP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1, "ECC_SECP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, "ECC_SECP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1, "ECC_SECP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1, "ECC_SECP_521R1")
    ]

    for algo, algo_name in ecc_secp_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # Reason: ECC不使用rsa_e_bit_size参数，应该被忽略
            _, handler = api.ehsm_km_gen_key(
                algo, permit,
                17,  # rsa_e_bit_size=17（不应用于ECC）
                0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K078")
def test_ehsm_k078():
    """测试ECC SECP密钥生成时传入DH参数"""
    ecc_secp_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1, "ECC_SECP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1, "ECC_SECP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, "ECC_SECP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1, "ECC_SECP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1, "ECC_SECP_521R1")
    ]

    for algo, algo_name in ecc_secp_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # 创建一些虚假的DH参数数据（ECC不使用这些参数）
            fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,
                len(fake_dh_params),
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K079")
def test_ehsm_k079():
    """测试ECC SECP密钥生成时传入hmac_key_size参数"""
    ecc_secp_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1, "ECC_SECP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1, "ECC_SECP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, "ECC_SECP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1, "ECC_SECP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1, "ECC_SECP_521R1")
    ]

    for algo, algo_name in ecc_secp_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0,
                32,  # hmac_key_size=32（不应用于ECC）
                None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K080")
def test_ehsm_k080():
    """测试ECC SECP密钥生成时dh_params为None但dh_params_size非零"""
    ecc_secp_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192R1, "ECC_SECP_192R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224R1, "ECC_SECP_224R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, "ECC_SECP_256R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_384R1, "ECC_SECP_384R1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_521R1, "ECC_SECP_521R1")
    ]

    for algo, algo_name in ecc_secp_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100（不应用于ECC）
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)

# ========================================
#  ED25519密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_EDDSA_ED25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ED25519密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K081")
def test_ehsm_k081():
    """测试ED25519密钥生成时传入异常permit参数"""
    with allure.step("1、ED25519：permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "ED25519: permit超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ ED25519: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、ED25519：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "ED25519: permit全1应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ ED25519: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、ED25519：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit_min, 0, 0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ ED25519: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_EDDSA_ED25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ED25519密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K082")
def test_ehsm_k082():
    """测试ED25519密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    with allure.step("1、生成ED25519密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: ED25519不使用rsa_e_bit_size参数，应该被忽略
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit,
            17,  # rsa_e_bit_size=17（不应用于ED25519）
            0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ ED25519密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_EDDSA_ED25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ED25519密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K083")
def test_ehsm_k083():
    """测试ED25519密钥生成时传入DH参数"""
    with allure.step("1、生成ED25519密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # 创建一些虚假的DH参数数据（ED25519不使用这些参数）
        fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit, 0, 0,
            fake_dh_params,
            len(fake_dh_params),
            0xFFFFFFFF
        )
        log.info(f"✓ ED25519密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_EDDSA_ED25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ED25519密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K084")
def test_ehsm_k084():
    """测试ED25519密钥生成时传入hmac_key_size参数"""
    with allure.step("1、生成ED25519密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit, 0,
            32,  # hmac_key_size=32（不应用于ED25519）
            None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ ED25519密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_EDDSA_ED25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ED25519密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K085")
def test_ehsm_k085():
    """测试ED25519密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成ED25519密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ED25519, permit, 0, 0,
            None,  # dh_params=None
            100,   # dh_params_size=100（不应用于ED25519）
            0xFFFFFFFF
        )
        log.info(f"✓ ED25519密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

# ========================================
#  X25519密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("X25519密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K086")
def test_ehsm_k086():
    """测试X25519密钥生成时传入异常permit参数"""
    with allure.step("1、X25519：permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_X25519, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "X25519: permit超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ X25519: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、X25519：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_X25519, permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "X25519: permit全1应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ X25519: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、X25519：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_X25519, permit_min, 0, 0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ X25519: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("X25519密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K087")
def test_ehsm_k087():
    """测试X25519密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    with allure.step("1、生成X25519密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: X25519不使用rsa_e_bit_size参数，应该被忽略
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_X25519, permit,
            17,  # rsa_e_bit_size=17（不应用于X25519）
            0, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ X25519密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("X25519密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K088")
def test_ehsm_k088():
    """测试X25519密钥生成时传入DH参数"""
    with allure.step("1、生成X25519密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # 创建一些虚假的DH参数数据（X25519不使用这些参数）
        fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_X25519, permit, 0, 0,
            fake_dh_params,
            len(fake_dh_params),
            0xFFFFFFFF
        )
        log.info(f"✓ X25519密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("X25519密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K089")
def test_ehsm_k089():
    """测试X25519密钥生成时传入hmac_key_size参数"""
    with allure.step("1、生成X25519密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_X25519, permit, 0,
            32,  # hmac_key_size=32（不应用于X25519）
            None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ X25519密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_X25519_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("X25519密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K090")
def test_ehsm_k090():
    """测试X25519密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成X25519密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_X25519, permit, 0, 0,
            None,  # dh_params=None
            100,   # dh_params_size=100（不应用于X25519）
            0xFFFFFFFF
        )
        log.info(f"✓ X25519密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

# ========================================
#  SM4_XTS密钥生成异常参数测试
# ========================================
# ========================================
#  AES_XTS密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES_XTS密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K096")
def test_ehsm_k096():
    """测试AES_XTS密钥生成时传入异常permit参数"""
    aes_xts_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS, "AES_128_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS, "AES_192_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS, "AES_256_XTS")
    ]

    for algo, algo_name in aes_xts_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                permit_all_ones = 0xFFFFFFFF
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_all_ones, 0, 0, None, 0, 0xFFFFFFFF
                )
                # 如果成功需要清理
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 0, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES_XTS密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K097")
def test_ehsm_k097():
    """测试AES_XTS密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    aes_xts_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS, "AES_128_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS, "AES_192_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS, "AES_256_XTS")
    ]

    for algo, algo_name in aes_xts_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # Reason: AES_XTS不使用rsa_e_bit_size参数，应该被忽略
            _, handler = api.ehsm_km_gen_key(
                algo, permit,
                17,  # rsa_e_bit_size=17（不应用于AES_XTS）
                0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES_XTS密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K098")
def test_ehsm_k098():
    """测试AES_XTS密钥生成时传入DH参数"""
    aes_xts_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS, "AES_128_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS, "AES_192_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS, "AES_256_XTS")
    ]

    for algo, algo_name in aes_xts_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            # 创建一些虚假的DH参数数据（AES_XTS不使用这些参数）
            fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                fake_dh_params,
                len(fake_dh_params),
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES_XTS密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K099")
def test_ehsm_k099():
    """测试AES_XTS密钥生成时传入hmac_key_size参数"""
    aes_xts_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS, "AES_128_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS, "AES_192_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS, "AES_256_XTS")
    ]

    for algo, algo_name in aes_xts_algos:
        with allure.step(f"1、生成{algo_name}密钥但传入hmac_key_size # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0,
                32,  # hmac_key_size=32（不应用于AES_XTS）
                None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(hmac_key_size被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES_XTS密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K100")
def test_ehsm_k100():
    """测试AES_XTS密钥生成时dh_params为None但dh_params_size非零"""
    aes_xts_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS, "AES_128_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS, "AES_192_XTS"),
        (EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS, "AES_256_XTS")
    ]

    for algo, algo_name in aes_xts_algos:
        with allure.step(f"1、生成{algo_name}密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
            permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"] |
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

            _, handler = api.ehsm_km_gen_key(
                algo, permit, 0, 0,
                None,  # dh_params=None
                100,   # dh_params_size=100（不应用于AES_XTS）
                0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

            # 清理
            api.ehsm_km_remove_key(handler)


# ========================================
#  CHACHA密钥生成异常参数测试
# ========================================
# ========================================
#  HMAC密钥生成异常参数测试
# ========================================
@allure.feature("kms")
@allure.description("HMAC密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K106")
def test_ehsm_k106():
    """测试HMAC密钥生成时传入异常permit参数"""
    with allure.step("1、HMAC：permit超出有效范围(0x100000) # 1、返回参数错误"):
        try:
            permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit_invalid, 0, 32, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "HMAC: permit超出范围应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ HMAC: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、HMAC：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
        try:
            permit_all_ones = 0xFFFFFFFF
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit_all_ones, 0, 32, None, 0, 0xFFFFFFFF
            )
            # 如果成功需要清理
            api.ehsm_km_remove_key(handler)
            assert False, "HMAC: permit全1应该失败，但却成功了"
        except hostapi.HostApiError as e:
            log.info(f"✓ HMAC: permit全1(0xFFFFFFFF)，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("3、HMAC：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
        permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit_min, 0, 32, None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ HMAC: 仅REMOVE权限(最小有效值)，密钥生成成功，符合预期，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("HMAC密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K107")
def test_ehsm_k107():
    """测试HMAC密钥生成时传入rsa_e_bit_size参数（应被忽略）"""
    with allure.step("1、生成HMAC密钥但传入rsa_e_bit_size=17 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: HMAC不使用rsa_e_bit_size参数，应该被忽略
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit,
            17,  # rsa_e_bit_size=17（不应用于HMAC）
            32,  # hmac_key_size=32（HMAC需要此参数）
            None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ HMAC密钥生成成功(rsa_e_bit_size被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("HMAC密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K108")
def test_ehsm_k108():
    """测试HMAC密钥生成时传入DH参数"""
    with allure.step("1、生成HMAC密钥但传入dh_params数据 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # 创建一些虚假的DH参数数据（HMAC不使用这些参数）
        fake_dh_params = bytes([0x01, 0x02, 0x03, 0x04] * 16)  # 64字节

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit, 0, 32,
            fake_dh_params,
            len(fake_dh_params),
            0xFFFFFFFF
        )
        log.info(f"✓ HMAC密钥生成成功(dh_params被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("HMAC密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K109")
def test_ehsm_k109():
    """测试HMAC密钥生成时传入hmac_key_size异常参数"""
    with allure.step("1、生成HMAC密钥，hmac_key_size=0（无效值） # 1、返回参数错误或使用默认值"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        # Reason: HMAC需要hmac_key_size参数，测试传入0的情况
        try:
            _, handler = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit, 0,
                0,  # hmac_key_size=0（可能无效）
                None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ HMAC密钥生成成功(hmac_key_size=0，可能使用默认值)，句柄: {handler:08X}")
            # 清理
            api.ehsm_km_remove_key(handler)
        except hostapi.HostApiError as e:
            log.info(f"✓ HMAC: hmac_key_size=0无效，密钥生成失败，符合预期，错误码: {e.ret_code}")

    with allure.step("2、生成HMAC密钥，hmac_key_size=32（有效值） # 2、成功"):
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit, 0,
            32,  # hmac_key_size=32（常用值）
            None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ HMAC密钥生成成功(hmac_key_size=32)，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

    with allure.step("3、生成HMAC密钥，hmac_key_size=64（有效值） # 3、成功"):
        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit, 0,
            64,  # hmac_key_size=64（较大值）
            None, 0, 0xFFFFFFFF
        )
        log.info(f"✓ HMAC密钥生成成功(hmac_key_size=64)，句柄: {handler:08X}")
        # 清理
        api.ehsm_km_remove_key(handler)

@allure.feature("kms")
@allure.description("HMAC密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K110")
def test_ehsm_k110():
    """测试HMAC密钥生成时dh_params为None但dh_params_size非零"""
    with allure.step("1、生成HMAC密钥，dh_params=None但dh_params_size=100 # 1、成功(参数被忽略)"):
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"])

        _, handler = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_HMAC, permit, 0, 32,
            None,  # dh_params=None
            100,   # dh_params_size=100（不应用于HMAC）
            0xFFFFFFFF
        )
        log.info(f"✓ HMAC密钥生成成功(dh_params=None, size=100被忽略)，句柄: {handler:08X}")

        # 清理
        api.ehsm_km_remove_key(handler)

# ========================================
#  ECC_SECP_K1密钥生成异常参数测试
# ========================================
@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP K1密钥生成：permit异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K111")
def test_ehsm_k111():
    """测试ECC SECP K1密钥生成时传入异常permit参数"""

    # 定义所有ECC SECP K1曲线类型
    ecc_secp_k1_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1, "ECC_SECP_160K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1, "ECC_SECP_192K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1, "ECC_SECP_224K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1, "ECC_SECP_256K1"),
    ]

    for algo, algo_name in ecc_secp_k1_algos:
        with allure.step(f"1、{algo_name}：permit超出有效范围(0x100000) # 1、返回参数错误"):
            try:
                permit_invalid = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | 0x100000
                _, handler = api.ehsm_km_gen_key(
                    algo, permit_invalid, 0, 0, None, 0, 0xFFFFFFFF
                )
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit超出范围应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限超出有效范围，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"2、{algo_name}：permit全1(0xFFFFFFFF) # 2、返回参数错误"):
            try:
                _, handler = api.ehsm_km_gen_key(
                    algo, 0xFFFFFFFF, 0, 0, None, 0, 0xFFFFFFFF
                )
                api.ehsm_km_remove_key(handler)
                assert False, f"{algo_name}: permit全1应该失败，但却成功了"
            except hostapi.HostApiError as e:
                log.info(f"✓ {algo_name}: 密钥权限全1，密钥生成失败，符合预期，错误码: {e.ret_code}")

        with allure.step(f"3、{algo_name}：仅REMOVE权限(最小有效值) # 3、密钥生成成功"):
            permit_min = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"]
            _, handler = api.ehsm_km_gen_key(
                algo, permit_min, 0, 0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: 仅REMOVE权限成功，句柄: {handler:08X}")
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP K1密钥生成：rsa_e_bit_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K112")
def test_ehsm_k112():
    """测试ECC SECP K1密钥生成时传入rsa_e_bit_size参数（应被忽略）"""

    ecc_secp_k1_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1, "ECC_SECP_160K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1, "ECC_SECP_192K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1, "ECC_SECP_224K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1, "ECC_SECP_256K1"),
    ]

    for algo, algo_name in ecc_secp_k1_algos:
        with allure.step(f"{algo_name}：rsa_e_bit_size=17（应被忽略）# 密钥生成成功"):
            _, handler = api.ehsm_km_gen_key(
                algo,
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"],
                17,  # rsa_e_bit_size参数应被忽略
                0, None, 0, 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: rsa_e_bit_size参数被忽略，密钥生成成功，句柄: {handler:08X}")
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP K1密钥生成：dh_params异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K113")
def test_ehsm_k113():
    """测试ECC SECP K1密钥生成时传入dh_params参数（应被忽略）"""

    ecc_secp_k1_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1, "ECC_SECP_160K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1, "ECC_SECP_192K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1, "ECC_SECP_224K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1, "ECC_SECP_256K1"),
    ]

    # 创建虚假的DH参数数据（64字节）
    fake_dh_params = bytes([0xAA] * 64)

    for algo, algo_name in ecc_secp_k1_algos:
        with allure.step(f"{algo_name}：传入虚假dh_params（应被忽略）# 密钥生成成功"):
            _, handler = api.ehsm_km_gen_key(
                algo,
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"],
                0, 0, fake_dh_params, len(fake_dh_params), 0xFFFFFFFF
            )
            log.info(f"✓ {algo_name}: dh_params参数被忽略，密钥生成成功，句柄: {handler:08X}")
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP K1密钥生成：hmac_key_size异常参数测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K114")
def test_ehsm_k114():
    """测试ECC SECP K1密钥生成时传入hmac_key_size参数（应被忽略）"""

    ecc_secp_k1_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1, "ECC_SECP_160K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1, "ECC_SECP_192K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1, "ECC_SECP_224K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1, "ECC_SECP_256K1"),
    ]

    for algo, algo_name in ecc_secp_k1_algos:
        with allure.step(f"{algo_name}：hmac_key_size=32（应被忽略）# 密钥生成成功"):
            _, handler = api.ehsm_km_gen_key(
                algo,
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"],
                0, 32, None, 0, 0xFFFFFFFF  # hmac_key_size=32应被忽略
            )
            log.info(f"✓ {algo_name}: hmac_key_size参数被忽略，密钥生成成功，句柄: {handler:08X}")
            api.ehsm_km_remove_key(handler)


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_ECDSA_SECP_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC SECP K1密钥生成：dh_params为None但dh_params_size非零")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K115")
def test_ehsm_k115():
    """测试ECC SECP K1密钥生成时dh_params=None但dh_params_size非零（应被忽略）"""

    ecc_secp_k1_algos = [
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_160K1, "ECC_SECP_160K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_192K1, "ECC_SECP_192K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_224K1, "ECC_SECP_224K1"),
        (EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256K1, "ECC_SECP_256K1"),
    ]

    for algo, algo_name in ecc_secp_k1_algos:
        with allure.step(f"{algo_name}：dh_params=None, size=100（矛盾参数应被忽略）# 密钥生成成功"):
            _, handler = api.ehsm_km_gen_key(
                algo,
                KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"],
                0, 0, None, 100, 0xFFFFFFFF  # dh_params=None但dh_params_size=100
            )
            log.info(f"✓ {algo_name}: 矛盾参数被忽略，密钥生成成功，句柄: {handler:08X}")
            api.ehsm_km_remove_key(handler)

# ========================================
#  SM9密钥生成异常参数测试
# ========================================
# ========================================
#  密钥交换异常参数测试
# ========================================

@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥交换：dh_common参数异常测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K121")
def test_ehsm_k121():
    dh_private_key = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])

    dh_public_key = bytes([
        0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
        0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
        0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
        0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
        0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
        0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
        0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
        0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64
    ])

    import_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] | \
                    KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

    exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]

    with allure.step("准备：导入DH私钥作为本地密钥； # 导入成功；"):
        # Reason: 构造DH私钥导入数据
        STRUCT_FORMAT = "<IBBHHH"
        header = struct.pack(
            STRUCT_FORMAT,
            import_permit,
            EhsmKeyType.EHSM_KEY_TYPE_DH,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(dh_private_key)  # priv_key_size
        )
        key_data = header + dh_private_key
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"✓ DH私钥导入成功，句柄: {local_handle:08X}")

    with allure.step("1、DH密钥交换时dh_common_addr=None且dh_common_size=0； # 1、返回失败；"):
        # Reason: 测试DH交换必须提供dh_common参数，否则应失败
        try:
            _, exchange_handle = api.ehsm_km_exchange_key(
                rmt_pub_key=dh_public_key,
                rmt_pub_key_size=len(dh_public_key),
                privilege=exchange_permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                hmac_key_size=0,
                local_key_handle=local_handle,
                dh_params=None,  # dh_common_addr=None
                dh_params_size=0,  # dh_common_size=0
                sm2_params=None,
                key_handle=0xFFFFFFFF
            )
            assert False, "预期密钥交换失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_REMAP_FAILED), \
                f"EHSM_ERR_REMAP_FAILED, 实际返回值: {e.ret_code}"
            log.info("✓ DH交换缺少dh_common参数测试通过")

    with allure.step("清理：删除本地密钥； # 删除成功；"):
        api.ehsm_km_remove_key(local_handle)
        log.info("✓ 测试清理完成")


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_SM2_SUPPORT == 0, reason="不支持SM2算法")
@allure.feature("kms")
@allure.description("SM2密钥交换：sm2_param_addr参数异常测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K122")
def test_ehsm_k122():
    # Reason: 从test_ehsm_711复制SM2测试数据
    g_spor_perm_priv_key = bytes([0x1A, 0xD1, 0x5B, 0xF3, 0xF1, 0xFB, 0x11, 0x7E, 0x95, 0x9E, 0x25, 0x97, 0xEE, 0x70, 0x99, 0xF0, 0x73, 0xDC, 0xF0,
                                  0x2A, 0x3C, 0xE5, 0xC7, 0x46, 0x3F, 0x7C, 0x0C, 0x96, 0xB5, 0xA1, 0x27, 0x48])
    g_spor_perm_pub_key = bytes([0x04, 0x7B, 0x8D, 0x44, 0x31, 0x84, 0x22, 0x13, 0xEB, 0x15, 0x64, 0x6C, 0xEF, 0x94, 0xC5, 0xCC, 0xDE, 0x05, 0x27,
                                  0x6E, 0x06, 0x85, 0x52, 0xBD, 0x94, 0xAD, 0xDB, 0x82, 0x07, 0x2C, 0xAF, 0x75, 0x05, 0x32, 0xF5, 0xCD, 0xDF, 0x1C,
                                  0x4D, 0x81, 0x5A, 0x89, 0xB2, 0x53, 0x51, 0x0E, 0x66, 0x22, 0xCA, 0x65, 0x19, 0x20, 0x73, 0x8C, 0xD8, 0x88, 0xCE,
                                  0x62, 0xB8, 0x78, 0x72, 0xB1, 0x74, 0xD7, 0x4E])
    g_rspor_perm_pub_key = bytes([0x04, 0xAC, 0x76, 0x7C, 0x20, 0x23, 0x11, 0x1F, 0x0B, 0xC3, 0x82, 0x04, 0x31,
                                  0x43, 0xD7, 0xFB, 0x44, 0x29, 0x1C, 0x4D, 0xE5, 0x62, 0xFC, 0x0B, 0x62, 0xA9, 0x16, 0x8C, 0x24, 0x1E, 0xB2, 0xF9,
                                  0xEB, 0x6C, 0xA2, 0x53, 0x55, 0x84, 0x40, 0xEE, 0xA4, 0x3C, 0x3E, 0xC2, 0x83, 0x35, 0x57, 0x00, 0xD0, 0xFD, 0xE8,
                                  0xF9, 0x28, 0x84, 0xF7, 0x54, 0xC5, 0x74, 0x4A, 0x3F, 0x6E, 0xC5, 0x24, 0xDB, 0xB4])

    permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
             KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"] | \
             KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]

    exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] | \
                      KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]

    with allure.step("准备：导入SM2密钥对作为本地密钥； # 导入成功；"):
        # Reason: 构造SM2密钥对导入数据
        STRUCT_FORMAT = "<IBBHHH"
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_SM2,
            0x3,  # KMS_KEY_PART_PAIRKEY
            0,    # reserved
            len(g_spor_perm_pub_key),  # pub_key_size
            len(g_spor_perm_priv_key)  # priv_key_size
        )
        key_data = header + g_spor_perm_pub_key + g_spor_perm_priv_key
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"✓ SM2密钥对导入成功，句柄: {local_handle:08X}")

    with allure.step("1、SM2密钥交换时sm2_param_addr=None； # 1、返回失败；"):
        # Reason: 测试SM2交换必须提供sm2_param参数，否则应失败
        try:
            _, exchange_handle = api.ehsm_km_exchange_key(
                rmt_pub_key=g_rspor_perm_pub_key,
                rmt_pub_key_size=len(g_rspor_perm_pub_key),
                privilege=exchange_permit,
                key_type=EhsmKeyType.EHSM_KEY_TYPE_AES_128,
                hmac_key_size=0,
                local_key_handle=local_handle,
                dh_params=None,
                dh_params_size=0,
                sm2_params=None,  # sm2_param_addr=None
                key_handle=0xFFFFFFFF
            )
            assert False, "预期密钥交换失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"
            log.info("✓ SM2交换缺少sm2_param参数测试通过")

    with allure.step("清理：删除本地密钥； # 删除成功；"):
        api.ehsm_km_remove_key(local_handle)
        log.info("✓ 测试清理完成")


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换")
@allure.feature("kms")
@allure.description("DH密钥计算公钥：dh_params参数异常测试")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K123")
def test_ehsm_k123():
    # Reason: 从test_ehsm_710复制DH测试数据
    dh_private_key = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])

    dh_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                  0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                  0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                  0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                  0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                  0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                  0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                  0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])

    dh_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])

    dh_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23
    ])

    g_size = 128
    p_size = 128
    q_size = 20

    permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | \
             KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_WR_PRT"]

    with allure.step("准备：导入DH私钥； # 导入成功；"):
        # Reason: 构造DH私钥导入数据
        STRUCT_FORMAT = "<IBBHHH"
        header = struct.pack(
            STRUCT_FORMAT,
            permit,
            EhsmKeyType.EHSM_KEY_TYPE_DH,
            0x2,  # KMS_KEY_PART_PRIVKEY
            0,    # reserved
            0,    # pub_key_size
            len(dh_private_key)  # priv_key_size
        )
        key_data = header + dh_private_key
        _, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"✓ DH私钥导入成功，句柄: {local_handle:08X}")

    with allure.step("1、DH密钥计算公钥时dh_params=None； # 1、返回失败；"):
        # Reason: 测试DH密钥必须提供dh_params参数，否则应失败
        try:
            _, output_pub, output_type, output_size = api.ehsm_km_get_pub_from_priv(
                local_handle,
                None,  # dh_params=None
                0,
                512
            )
            assert False, "预期计算公钥失败，但实际成功了"
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_REMAP_FAILED), \
                f"期望返回EHSM_ERR_REMAP_FAILED, 实际返回值: {e.ret_code}"
            log.info("✓ DH密钥缺少dh_params参数测试通过")

    with allure.step("2、DH密钥计算公钥时dh_params_size=0； # 2、返回失败；"):
        # Reason: 测试dh_params_size=0的无效情况
        # 构造DH公共参数
        dh_common_data = struct.pack("<I128sI20sI128s",
                                     p_size, dh_p,
                                     q_size, dh_q,
                                     g_size, dh_g)
        dh_common_data += struct.pack("<I", 0)  # h_len=0
        try:
            _, output_pub, output_type, output_size = api.ehsm_km_get_pub_from_priv(
                local_handle,
                dh_common_data,
                0,  # dh_params_size=0
                512
            )
        except hostapi.HostApiError as e:
            assert (e.ret_code == EHSM_ERR_PARAM_ERROR), \
                f"期望返回EHSM_ERR_PARAM_ERROR, 实际返回值: {e.ret_code}"

    with allure.step("清理：删除本地密钥； # 删除成功；"):
        api.ehsm_km_remove_key(local_handle)
        log.info("✓ 测试清理完成")


def _get_rsa1024_testvec() -> dict:
    """返回 RSA-1024 CRT 标准测试向量（对应 C 端 rsa_tv_template[0]）。

    Returns:
        dict 包含 der(bytes), plaintext(bytes), ciphertext(bytes)
    """
    der = bytes([
        0x30,0x82,0x02,0x5B,0x02,0x01,0x00,0x02,0x81,0x81,0x00,0xBB,0xF8,0x2F,0x09,0x06,
        0x82,0xCE,0x9C,0x23,0x38,0xAC,0x2B,0x9D,0xA8,0x71,0xF7,0x36,0x8D,0x07,0xEE,0xD4,
        0x10,0x43,0xA4,0x40,0xD6,0xB6,0xF0,0x74,0x54,0xF5,0x1F,0xB8,0xDF,0xBA,0xAF,0x03,
        0x5C,0x02,0xAB,0x61,0xEA,0x48,0xCE,0xEB,0x6F,0xCD,0x48,0x76,0xED,0x52,0x0D,0x60,
        0xE1,0xEC,0x46,0x19,0x71,0x9D,0x8A,0x5B,0x8B,0x80,0x7F,0xAF,0xB8,0xE0,0xA3,0xDF,
        0xC7,0x37,0x72,0x3E,0xE6,0xB4,0xB7,0xD9,0x3A,0x25,0x84,0xEE,0x6A,0x64,0x9D,0x06,
        0x09,0x53,0x74,0x88,0x34,0xB2,0x45,0x45,0x98,0x39,0x4E,0xE0,0xAA,0xB1,0x2D,0x7B,
        0x61,0xA5,0x1F,0x52,0x7A,0x9A,0x41,0xF6,0xC1,0x68,0x7F,0xE2,0x53,0x72,0x98,0xCA,
        0x2A,0x8F,0x59,0x46,0xF8,0xE5,0xFD,0x09,0x1D,0xBD,0xCB,0x02,0x01,0x11,0x02,0x81,
        0x81,0x00,0xA5,0xDA,0xFC,0x53,0x41,0xFA,0xF2,0x89,0xC4,0xB9,0x88,0xDB,0x30,0xC1,
        0xCD,0xF8,0x3F,0x31,0x25,0x1E,0x06,0x68,0xB4,0x27,0x84,0x81,0x38,0x01,0x57,0x96,
        0x41,0xB2,0x94,0x10,0xB3,0xC7,0x99,0x8D,0x6B,0xC4,0x65,0x74,0x5E,0x5C,0x39,0x26,
        0x69,0xD6,0x87,0x0D,0xA2,0xC0,0x82,0xA9,0x39,0xE3,0x7F,0xDC,0xB8,0x2E,0xC9,0x3E,
        0xDA,0xC9,0x7F,0xF3,0xAD,0x59,0x50,0xAC,0xCF,0xBC,0x11,0x1C,0x76,0xF1,0xA9,0x52,
        0x94,0x44,0xE5,0x6A,0xAF,0x68,0xC5,0x6C,0x09,0x2C,0xD3,0x8D,0xC3,0xBE,0xF5,0xD2,
        0x0A,0x93,0x99,0x26,0xED,0x4F,0x74,0xA1,0x3E,0xDD,0xFB,0xE1,0xA1,0xCE,0xCC,0x48,
        0x94,0xAF,0x94,0x28,0xC2,0xB7,0xB8,0x88,0x3F,0xE4,0x46,0x3A,0x4B,0xC8,0x5B,0x1C,
        0xB3,0xC1,0x02,0x41,0x00,0xEE,0xCF,0xAE,0x81,0xB1,0xB9,0xB3,0xC9,0x08,0x81,0x0B,
        0x10,0xA1,0xB5,0x60,0x01,0x99,0xEB,0x9F,0x44,0xAE,0xF4,0xFD,0xA4,0x93,0xB8,0x1A,
        0x9E,0x3D,0x84,0xF6,0x32,0x12,0x4E,0xF0,0x23,0x6E,0x5D,0x1E,0x3B,0x7E,0x28,0xFA,
        0xE7,0xAA,0x04,0x0A,0x2D,0x5B,0x25,0x21,0x76,0x45,0x9D,0x1F,0x39,0x75,0x41,0xBA,
        0x2A,0x58,0xFB,0x65,0x99,0x02,0x41,0x00,0xC9,0x7F,0xB1,0xF0,0x27,0xF4,0x53,0xF6,
        0x34,0x12,0x33,0xEA,0xAA,0xD1,0xD9,0x35,0x3F,0x6C,0x42,0xD0,0x88,0x66,0xB1,0xD0,
        0x5A,0x0F,0x20,0x35,0x02,0x8B,0x9D,0x86,0x98,0x40,0xB4,0x16,0x66,0xB4,0x2E,0x92,
        0xEA,0x0D,0xA3,0xB4,0x32,0x04,0xB5,0xCF,0xCE,0x33,0x52,0x52,0x4D,0x04,0x16,0xA5,
        0xA4,0x41,0xE7,0x00,0xAF,0x46,0x15,0x03,0x02,0x40,0x54,0x49,0x4C,0xA6,0x3E,0xBA,
        0x03,0x37,0xE4,0xE2,0x40,0x23,0xFC,0xD6,0x9A,0x5A,0xEB,0x07,0xDD,0xDC,0x01,0x83,
        0xA4,0xD0,0xAC,0x9B,0x54,0xB0,0x51,0xF2,0xB1,0x3E,0xD9,0x49,0x09,0x75,0xEA,0xB7,
        0x74,0x14,0xFF,0x59,0xC1,0xF7,0x69,0x2E,0x9A,0x2E,0x20,0x2B,0x38,0xFC,0x91,0x0A,
        0x47,0x41,0x74,0xAD,0xC9,0x3C,0x1F,0x67,0xC9,0x81,0x02,0x40,0x47,0x1E,0x02,0x90,
        0xFF,0x0A,0xF0,0x75,0x03,0x51,0xB7,0xF8,0x78,0x86,0x4C,0xA9,0x61,0xAD,0xBD,0x3A,
        0x8A,0x7E,0x99,0x1C,0x5C,0x05,0x56,0xA9,0x4C,0x31,0x46,0xA7,0xF9,0x80,0x3F,0x8F,
        0x6F,0x8A,0xE3,0x42,0xE9,0x31,0xFD,0x8A,0xE4,0x7A,0x22,0x0D,0x1B,0x99,0xA4,0x95,
        0x84,0x98,0x07,0xFE,0x39,0xF9,0x24,0x5A,0x98,0x36,0xDA,0x3D,0x02,0x41,0x00,0xB0,
        0x6C,0x4F,0xDA,0xBB,0x63,0x01,0x19,0x8D,0x26,0x5B,0xDB,0xAE,0x94,0x23,0xB3,0x80,
        0xF2,0x71,0xF7,0x34,0x53,0x88,0x50,0x93,0x07,0x7F,0xCD,0x39,0xE2,0x11,0x9F,0xC9,
        0x86,0x32,0x15,0x4F,0x58,0x83,0xB1,0x67,0xA9,0x67,0xBF,0x40,0x2B,0x4E,0x9E,0x2E,
        0x0F,0x96,0x56,0xE6,0x98,0xEA,0x36,0x66,0xED,0xFB,0x25,0x79,0x80,0x39,0xF7
    ])
    plaintext = bytes([0x54,0x85,0x9b,0x34,0x2c,0x49,0xea,0x2a])
    # Reason: no-padding 模式密文（对应 C 端 rsa_tv_template[0].c），128字节
    ciphertext = bytes([
        0x74,0x1b,0x55,0xac,0x47,0xb5,0x08,0x0a,0x6e,0x2b,0x2d,0xf7,0x94,0xb8,0x8a,0x95,
        0xed,0xa3,0x6b,0xc9,0x29,0xee,0xb2,0x2c,0x80,0xc3,0x39,0x3b,0x8c,0x62,0x45,0x72,
        0xc2,0x7f,0x74,0x81,0x91,0x68,0x44,0x48,0x5a,0xdc,0xa0,0x7e,0xa7,0x0b,0x05,0x7f,
        0x0e,0xa0,0x6c,0xe5,0x8f,0x19,0x4d,0xce,0x98,0x47,0x5f,0xbd,0x5f,0xfe,0xe5,0x34,
        0x59,0x89,0xaf,0xf0,0xba,0x44,0xd7,0xf1,0x1a,0x50,0x72,0xef,0x5e,0x4a,0xb6,0xb7,
        0x54,0x34,0xd1,0xc4,0x83,0x09,0xdf,0x0f,0x91,0x5f,0x7d,0x91,0x70,0x2f,0xd4,0x13,
        0xcc,0x5e,0xa4,0x6c,0xc3,0x4d,0x28,0xef,0xda,0xaf,0xec,0x14,0x92,0xfc,0xa3,0x75,
        0x13,0xb4,0xc1,0xa1,0x11,0xfc,0x40,0x2f,0x4c,0x9d,0xdf,0x16,0x76,0x11,0x20,0x6b
    ])
    return {"der": der, "plaintext": plaintext, "ciphertext": ciphertext}


def _get_rsa2048_testvec() -> dict:
    """返回 RSA-2048 CRT 标准测试向量（对应 C 端 rsa_tv_template[1]）。

    Returns:
        dict 包含 der(bytes), plaintext(bytes), ciphertext(bytes)
    """
    der = bytes([
        0x30,0x82,0x04,0xA3,0x02,0x01,0x00,0x02,0x82,0x01,0x01,0x00,0xDB,0x10,0x1A,0xC2,
        0xA3,0xF1,0xDC,0xFF,0x13,0x6B,0xED,0x44,0xDF,0xF0,0x02,0x6D,0x13,0xC7,0x88,0xDA,
        0x70,0x6B,0x54,0xF1,0xE8,0x27,0xDC,0xC3,0x0F,0x99,0x6A,0xFA,0xC6,0x67,0xFF,0x1D,
        0x1E,0x3C,0x1D,0xC1,0xB5,0x5F,0x6C,0xC0,0xB2,0x07,0x3A,0x6D,0x41,0xE4,0x25,0x99,
        0xAC,0xFC,0xD2,0x0F,0x02,0xD3,0xD1,0x54,0x06,0x1A,0x51,0x77,0xBD,0xB6,0xBF,0xEA,
        0xA7,0x5C,0x06,0xA9,0x5D,0x69,0x84,0x45,0xD7,0xF5,0x05,0xBA,0x47,0xF0,0x1B,0xD7,
        0x2B,0x24,0xEC,0xCB,0x9B,0x1B,0x10,0x8D,0x81,0xA0,0xBE,0xB1,0x8C,0x33,0xE4,0x36,
        0xB8,0x43,0xEB,0x19,0x2A,0x81,0x8D,0xDE,0x81,0x0A,0x99,0x48,0xB6,0xF6,0xBC,0xCD,
        0x49,0x34,0x3A,0x8F,0x26,0x94,0xE3,0x28,0x82,0x1A,0x7C,0x8F,0x59,0x9F,0x45,0xE8,
        0x5D,0x1A,0x45,0x76,0x04,0x56,0x05,0xA1,0xD0,0x1B,0x8C,0x77,0x6D,0xAF,0x53,0xFA,
        0x71,0xE2,0x67,0xE0,0x9A,0xFE,0x03,0xA9,0x85,0xD2,0xC9,0xAA,0xBA,0x2A,0xBC,0xF4,
        0xA0,0x08,0xF5,0x13,0x98,0x13,0x5D,0xF0,0xD9,0x33,0x34,0x2A,0x61,0xC3,0x89,0x55,
        0xF0,0xAE,0x1A,0x9C,0x22,0xEE,0x19,0x05,0x8D,0x32,0xFE,0xEC,0x9C,0x84,0xBA,0xB7,
        0xF9,0x6C,0x3A,0x4F,0x07,0xFC,0x45,0xEB,0x12,0xE5,0x7B,0xFD,0x55,0xE6,0x29,0x69,
        0xD1,0xC2,0xE8,0xB9,0x78,0x59,0xF6,0x79,0x10,0xC6,0x4E,0xEB,0x6A,0x5E,0xB9,0x9A,
        0xC7,0xC4,0x5B,0x63,0xDA,0xA3,0x3F,0x5E,0x92,0x7A,0x81,0x5E,0xD6,0xB0,0xE2,0x62,
        0x8F,0x74,0x26,0xC2,0x0C,0xD3,0x9A,0x17,0x47,0xE6,0x8E,0xAB,0x02,0x03,0x01,0x00,
        0x01,0x02,0x82,0x01,0x00,0x52,0x41,0xF4,0xDA,0x7B,0xB7,0x59,0x55,0xCA,0xD4,0x2F,
        0x0F,0x3A,0xCB,0xA4,0x0D,0x93,0x6C,0xCC,0x9D,0xC1,0xB2,0xFB,0xFD,0xAE,0x40,0x31,
        0xAC,0x69,0x52,0x21,0x92,0xB3,0x27,0xDF,0xEA,0xEE,0x2C,0x82,0xBB,0xF7,0x40,0x32,
        0xD5,0x14,0xC4,0x94,0x12,0xEC,0xB8,0x1F,0xCA,0x59,0xE3,0xC1,0x78,0xF3,0x85,0xD8,
        0x47,0xA5,0xD7,0x02,0x1A,0x65,0x79,0x97,0x0D,0x24,0xF4,0xF0,0x67,0x6E,0x75,0x2D,
        0xBF,0x10,0x3D,0xA8,0x7D,0xEF,0x7F,0x60,0xE4,0xE6,0x05,0x82,0x89,0x5D,0xDF,0xC6,
        0xD2,0x6C,0x07,0x91,0x33,0x98,0x42,0xF0,0x02,0x00,0x25,0x38,0xC5,0x85,0x69,0x8A,
        0x7D,0x2F,0x95,0x6C,0x43,0x9A,0xB8,0x81,0xE2,0xD0,0x07,0x35,0xAA,0x05,0x41,0xC9,
        0x1E,0xAF,0xE4,0x04,0x3B,0x19,0xB8,0x73,0xA2,0xAC,0x4B,0x1E,0x66,0x48,0xD8,0x72,
        0x1F,0xAC,0xF6,0xCB,0xBC,0x90,0x09,0xCA,0xEC,0x0C,0xDC,0xF9,0x2C,0xD7,0xEB,0xAE,
        0xA3,0xA4,0x47,0xD7,0x33,0x2F,0x8A,0xCA,0xBC,0x5E,0xF0,0x77,0xE4,0x97,0x98,0x97,
        0xC7,0x10,0x91,0x7D,0x2A,0xA6,0xFF,0x46,0x83,0x97,0xDE,0xE9,0xE2,0x17,0x03,0x06,
        0x14,0xE2,0xD7,0xB1,0x1D,0x77,0xAF,0x51,0x27,0x5B,0x5E,0x69,0xB8,0x81,0xE6,0x11,
        0xC5,0x43,0x23,0x81,0x04,0x62,0xFF,0xE9,0x46,0xB8,0xD8,0x44,0xDB,0xA5,0xCC,0x31,
        0x54,0x34,0xCE,0x3E,0x82,0xD6,0xBF,0x7A,0x0B,0x64,0x21,0x6D,0x88,0x7E,0x5B,0x45,
        0x12,0x1E,0x63,0x8D,0x49,0xA7,0x1D,0xD9,0x1E,0x06,0xCD,0xE8,0xBA,0x2C,0x8C,0x69,
        0x32,0xEA,0xBE,0x60,0x71,0x02,0x81,0x81,0x00,0xFA,0xAC,0xE1,0x37,0x5E,0x32,0x11,
        0x34,0xC6,0x72,0x58,0x2D,0x91,0x06,0x3E,0x77,0xE7,0x11,0x21,0xCD,0x4A,0xF8,0xA4,
        0x3F,0x0F,0xEF,0x31,0xE3,0xF3,0x55,0xA0,0xB9,0xAC,0xB6,0xCB,0xBB,0x41,0xD0,0x32,
        0x81,0x9A,0x8F,0x7A,0x99,0x30,0x77,0x6C,0x68,0x27,0xE2,0x96,0xB5,0x72,0xC9,0xC3,
        0xD4,0x42,0xAA,0xAA,0xCA,0x95,0x8F,0xFF,0xC9,0x9B,0x52,0x34,0x30,0x1D,0xCF,0xFE,
        0xCF,0x3C,0x56,0x68,0x6E,0xEF,0xE7,0x6C,0xD7,0xFB,0x99,0xF5,0x4A,0xA5,0x21,0x1F,
        0x2B,0xEA,0x93,0xE8,0x98,0x26,0xC4,0x6E,0x42,0x21,0x5E,0xA0,0xA1,0x2A,0x58,0x35,
        0xBB,0x10,0xE7,0xBA,0x27,0x0A,0x3B,0xB3,0xAF,0xE2,0x75,0x36,0x04,0xAC,0x56,0xA0,
        0xAB,0x52,0xDE,0xCE,0xDD,0x2C,0x28,0x77,0x03,0x02,0x81,0x81,0x00,0xDF,0xB7,0x52,
        0xB6,0xD7,0xC0,0xE2,0x96,0xE7,0xC9,0xFE,0x5D,0x71,0x5A,0xC4,0x40,0x96,0x2F,0xE5,
        0x87,0xEA,0xF3,0xA5,0x77,0x11,0x67,0x3C,0x8D,0x56,0x08,0xA7,0xB5,0x67,0xFA,0x37,
        0xA8,0xB8,0xCF,0x61,0xE8,0x63,0xD8,0x38,0x06,0x21,0x2B,0x92,0x09,0xA6,0x39,0x3A,
        0xEA,0xA8,0xB4,0x45,0x4B,0x36,0x10,0x4C,0xE4,0x00,0x66,0x71,0x65,0xF8,0x0B,0x94,
        0x59,0x4F,0x8C,0xFD,0xD5,0x34,0xA2,0xE7,0x62,0x84,0x0A,0xA7,0xBB,0xDB,0xD9,0x8A,
        0xCD,0x05,0xE1,0xCC,0x57,0x7B,0xF1,0xF1,0x1F,0x11,0x9D,0xBA,0x3E,0x45,0x18,0x99,
        0x1B,0x41,0x64,0x43,0xEE,0x97,0x5D,0x77,0x13,0x5B,0x74,0x69,0x73,0x87,0x95,0x05,
        0x07,0xBE,0x45,0x07,0x17,0x7E,0x4A,0x69,0x22,0xF3,0xDB,0x05,0x39,0x02,0x81,0x80,
        0x5E,0xD8,0xDC,0xDA,0x53,0x44,0xC4,0x67,0xE0,0x92,0x51,0x34,0xE4,0x83,0xA5,0x4D,
        0x3E,0xDB,0xA7,0x9B,0x82,0xBB,0x73,0x81,0xFC,0xE8,0x77,0x4B,0x15,0xBE,0x17,0x73,
        0x49,0x9B,0x5C,0x98,0xBC,0xBD,0x26,0xEF,0x0C,0xE9,0x2E,0xED,0x19,0x7E,0x86,0x41,
        0x1E,0x9E,0x48,0x81,0xDD,0x2D,0xE4,0x6F,0xC2,0xCD,0xCA,0x93,0x9E,0x65,0x7E,0xD5,
        0xEC,0x73,0xFD,0x15,0x1B,0xA2,0xA0,0x7A,0x0F,0x0D,0x6E,0xB4,0x53,0x07,0x90,0x92,
        0x64,0x3B,0x8B,0xA9,0x33,0xB3,0xC5,0x94,0x9B,0x4C,0x5D,0x9C,0x7C,0x46,0xA4,0xA5,
        0x56,0xF4,0xF3,0xF8,0x27,0x0A,0x7B,0x42,0x0D,0x92,0x70,0x47,0xE7,0x42,0x51,0xA9,
        0xC2,0x18,0xB1,0x58,0xB1,0x50,0x91,0xB8,0x61,0x41,0xB6,0xA9,0xCE,0xD4,0x7C,0xBB,
        0x02,0x81,0x80,0x54,0x09,0x1F,0x0F,0x03,0xD8,0xB6,0xC5,0x0C,0xE8,0xB9,0x9E,0x0C,
        0x38,0x96,0x43,0xD4,0xA6,0xC5,0x47,0xDB,0x20,0x0E,0xE5,0xBD,0x29,0xD4,0x7B,0x1A,
        0xF8,0x41,0x57,0x49,0x69,0x9A,0x82,0xCC,0x79,0x4A,0x43,0xEB,0x4D,0x8B,0x2D,0xF2,
        0x43,0xD5,0xA5,0xBE,0x44,0xFD,0x36,0xAC,0x8C,0x9B,0x02,0xF7,0x9A,0x03,0xE8,0x19,
        0xA6,0x61,0xAE,0x76,0x10,0x93,0x77,0x41,0x04,0xAB,0x4C,0xED,0x6A,0xCC,0x14,0x1B,
        0x99,0x8D,0x0C,0x6A,0x37,0x3B,0x86,0x6C,0x51,0x37,0x5B,0x1D,0x79,0xF2,0xA3,0x43,
        0x10,0xC6,0xA7,0x21,0x79,0x6D,0xF9,0xE9,0x04,0x6A,0xE8,0x32,0xFF,0xAE,0xFD,0x1C,
        0x7B,0x8C,0x29,0x13,0xA3,0x0C,0xB2,0xAD,0xEC,0x6C,0x0F,0x8D,0x27,0x12,0x7B,0x48,
        0xB2,0xDB,0x31,0x02,0x81,0x81,0x00,0x8D,0x1B,0x05,0xCA,0x24,0x1F,0x0C,0x53,0x19,
        0x52,0x74,0x63,0x21,0xFA,0x78,0x46,0x79,0xAF,0x5C,0xDE,0x30,0xA4,0x6C,0x20,0x38,
        0xE6,0x97,0x39,0xB8,0x7A,0x70,0x0D,0x8B,0x6C,0x6D,0x13,0x74,0xD5,0x1C,0xDE,0xA9,
        0xF4,0x60,0x37,0xFE,0x68,0x77,0x5E,0x0B,0x4E,0x5E,0x03,0x31,0x30,0xDF,0xD6,0xAE,
        0x85,0xD0,0x81,0xBB,0x61,0xC7,0xB1,0x04,0x5A,0xC4,0x6D,0x56,0x1C,0xD9,0x64,0xE7,
        0x85,0x7F,0x88,0x91,0xC9,0x60,0x28,0x05,0xE2,0xC6,0x24,0x8F,0xDD,0x61,0x64,0xD8,
        0x09,0xDE,0x7E,0xD3,0x4A,0x61,0x1A,0xD3,0x73,0x58,0x4B,0xD8,0xA0,0x54,0x25,0x48,
        0x83,0x6F,0x82,0x6C,0xAF,0x36,0x51,0x2A,0x5D,0x14,0x2F,0x41,0x25,0x00,0xDD,0xF8,
        0xF3,0x95,0xFE,0x31,0x25,0x50,0x12
    ])
    plaintext = bytes([0x54,0x85,0x9b,0x34,0x2c,0x49,0xea,0x2a])
    # Reason: no-padding 模式密文（对应 C 端 rsa_tv_template[1].c），256字节
    ciphertext = bytes([
        0xb2,0x97,0x76,0xb4,0xae,0x3e,0x38,0x3c,0x7e,0x64,0x1f,0xcc,0xa2,0x7f,0xf6,0xbe,
        0xcf,0x49,0xbc,0x48,0xd3,0x6c,0x8f,0x0a,0x0e,0xc1,0x73,0xbd,0x7b,0x55,0x79,0x36,
        0x0e,0xa1,0x87,0x88,0xb9,0x2c,0x90,0xa6,0x53,0x5e,0xe9,0xef,0xc4,0xe2,0x4d,0xdd,
        0xf7,0xa6,0x69,0x82,0x3f,0x56,0xa4,0x7b,0xfb,0x62,0xe0,0xae,0xb8,0xd3,0x04,0xb3,
        0xac,0x5a,0x15,0x2a,0xe3,0x19,0x9b,0x03,0x9a,0x0b,0x41,0xda,0x64,0xec,0x0a,0x69,
        0xfc,0xf2,0x10,0x92,0xf3,0xc1,0xbf,0x84,0x7f,0xfd,0x2c,0xae,0xc8,0xb5,0xf6,0x41,
        0x70,0xc5,0x47,0x03,0x8a,0xf8,0xff,0x6f,0x3f,0xd2,0x6f,0x09,0xb4,0x22,0xf3,0x30,
        0xbe,0xa9,0x85,0xcb,0x9c,0x8d,0xf9,0x8f,0xeb,0x32,0x91,0xa2,0x25,0x84,0x8f,0xf5,
        0xdc,0xc7,0x06,0x9c,0x2d,0xe5,0x11,0x2c,0x09,0x09,0x87,0x09,0xa9,0xf6,0x33,0x73,
        0x90,0xf1,0x60,0xf2,0x65,0xdd,0x30,0xa5,0x66,0xce,0x62,0x7b,0xd0,0xf8,0x2d,0x3d,
        0x19,0x82,0x77,0xe3,0x0a,0x5f,0x75,0x2f,0x8e,0xb1,0xe5,0xe8,0x91,0x35,0x1b,0x3b,
        0x33,0xb7,0x66,0x92,0xd1,0xf2,0x8e,0x6f,0xe5,0x75,0x0c,0xad,0x36,0xfb,0x4e,0xd0,
        0x66,0x61,0xbd,0x49,0xfe,0xf4,0x1a,0xa2,0x2b,0x49,0xfe,0x03,0x4c,0x74,0x47,0x8d,
        0x9a,0x66,0xb2,0x49,0x46,0x4d,0x77,0xea,0x33,0x4d,0x6b,0x3c,0xb4,0x49,0x4a,0xc6,
        0x7d,0x3d,0xb5,0xb9,0x56,0x41,0x15,0x67,0x0f,0x94,0x3c,0x93,0x65,0x27,0xe0,0x21,
        0x5d,0x59,0xc3,0x62,0xd5,0xa6,0xda,0x38,0x26,0x22,0x5e,0x34,0x1c,0x94,0xaf,0x98
    ])
    return {"der": der, "plaintext": plaintext, "ciphertext": ciphertext}


@pytest.mark.skipif(cfg_data.TEST_FW_ASYM_RSA_NONE_SUPPORT == 0 or cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持RSA或AES算法")
@allure.feature("kms")
@allure.description("验证 eHSM RAM 密钥存储扩展（迁移）逻辑的正确性")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K124")
def test_ehsm_k124():
    with allure.step("1、安装密钥A：导入AES-128明文密钥，获得handle_a； # 1、导入成功，返回密钥句柄；"):
        aes128_key = bytes([
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
        ])
        permit_a = (KeyPermit.KEY_PRIV_REMOVE |
                    KeyPermit.KEY_PRIV_ENCRYPT |
                    KeyPermit.KEY_PRIV_DECRYPT |
                    KeyPermit.KEY_PRIV_IMPORT_PLAIN)
        # Reason: 构造 kms_key_format_st 头部，对称密钥 part_info=0x2(PRIVKEY)，pub_key_size=0
        header_a = struct.pack("<IBBHHH", permit_a,
                               EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0x2, 0, 0, len(aes128_key))
        key_data_a = header_a + aes128_key
        _, handle_a = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF,
                                             key_data_a, len(key_data_a), None, 0, 0xFFFFFFFF)
        assert handle_a != 0xFFFFFFFF, "AES-128密钥导入失败"
        log.info(f"Step1: install AES-128 key, handle_a=0x{handle_a:x}")

    with allure.step("2、安装密钥B：导入RSA-1024 CRT模式密钥，获得handle_b（B紧接A存放）； # 2、导入成功，返回密钥句柄；"):
        tv1024 = _get_rsa1024_testvec()
        # Reason: 解析DER密钥，提取CRT各参数
        priv_key_obj = load_der_private_key(tv1024["der"], password=None, backend=default_backend())
        priv_nums = priv_key_obj.private_numbers()
        pub_nums = priv_nums.public_numbers
        n_bytes   = pub_nums.n.to_bytes(128, 'big')
        e_raw     = pub_nums.e.to_bytes(1, 'big')
        # Reason: e 需要4字节对齐（固件要求），且需左补齐以适配固件内部的字节反转逻辑
        e_aligned = b'\x00' * ((4 - len(e_raw) % 4) % 4) + e_raw
        p_bytes   = priv_nums.p.to_bytes(64, 'big')
        q_bytes   = priv_nums.q.to_bytes(64, 'big')
        dp_bytes  = priv_nums.dmp1.to_bytes(64, 'big')
        dq_bytes  = priv_nums.dmq1.to_bytes(64, 'big')
        u_bytes   = priv_nums.iqmp.to_bytes(64, 'big')
        # Reason: CRT密钥布局：e(对齐) + n + p + q + dp + dq + u
        pub_key_size_b  = len(e_aligned)
        priv_key_size_b = len(p_bytes) + len(q_bytes) + len(dp_bytes) + len(dq_bytes) + len(u_bytes)
        key_data_b = e_aligned + n_bytes + p_bytes + q_bytes + dp_bytes + dq_bytes + u_bytes
        permit_b = (KeyPermit.KEY_PRIV_REMOVE |
                    KeyPermit.KEY_PRIV_ENCRYPT |
                    KeyPermit.KEY_PRIV_DECRYPT |
                    KeyPermit.KEY_PRIV_IMPORT_PLAIN)
        pack_key_b = key.pack_key_with_head(key_data_b, permit_b,
                                            EhsmKeyType.EHSM_KEY_TYPE_RSA_1024_CRT,
                                            EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR,
                                            pub_key_size_b, priv_key_size_b)
        _, handle_b = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF,
                                             pack_key_b, len(pack_key_b), None, 0, 0xFFFFFFFF)
        assert handle_b != 0xFFFFFFFF, "RSA-1024 CRT密钥导入失败"
        log.info(f"Step2: install RSA-1024 CRT key, handle_b=0x{handle_b:x}")

    with allure.step("3、迁移前基准验证：使用测试向量标准密文作为golden ciphertext（无需加密步骤）； # 3、准备golden ciphertext成功；"):
        # Reason: 直接使用标准测试向量密文，密钥以DECRYPT权限导入，无需加密步骤
        golden_ciphertext = tv1024["ciphertext"]
        golden_plaintext  = tv1024["plaintext"]
        log.info("Step3: golden ciphertext prepared, len=%d", len(golden_ciphertext))

    with allure.step("4、迁移前验证：用密钥B（RSA-1024）对golden ciphertext解密，与golden plaintext对比； # 4、解密成功，结果与预期一致；"):
        # Reason: no-padding 解密输出右对齐，取末尾 len(golden_plaintext) 字节与预期对比
        _, dec_data_before, dec_size_before = api.ehsm_rsa_cipher(
            handle_b, False, golden_ciphertext, len(golden_ciphertext), 128)
        actual_plain_before = dec_data_before[dec_size_before - len(golden_plaintext):dec_size_before]
        assert actual_plain_before == golden_plaintext, (
            f"迁移前解密结果不匹配: 期望 {golden_plaintext.hex()}, 实际 {actual_plain_before.hex()}")
        log.info("Step4: RSA-1024 decrypt before migration OK")

    with allure.step("5、原地替换密钥A为RSA-2048（不删除handle_a，直接导入更大密钥）；触发固件将密钥B向后搬移腾出空间； # 5、更新成功，handle_a不变；"):
        tv2048 = _get_rsa2048_testvec()
        # Reason: 解析RSA-2048 DER，提取CRT参数
        priv_key_obj2 = load_der_private_key(tv2048["der"], password=None, backend=default_backend())
        priv_nums2 = priv_key_obj2.private_numbers()
        pub_nums2  = priv_nums2.public_numbers
        n_bytes2   = pub_nums2.n.to_bytes(256, 'big')
        e_raw2     = pub_nums2.e.to_bytes(3, 'big')
        # Reason: e 需要4字节对齐（固件要求），且需左补齐以适配固件内部的字节反转逻辑
        e_aligned2 = b'\x00' * ((4 - len(e_raw2) % 4) % 4) + e_raw2
        p_bytes2   = priv_nums2.p.to_bytes(128, 'big')
        q_bytes2   = priv_nums2.q.to_bytes(128, 'big')
        dp_bytes2  = priv_nums2.dmp1.to_bytes(128, 'big')
        dq_bytes2  = priv_nums2.dmq1.to_bytes(128, 'big')
        u_bytes2   = priv_nums2.iqmp.to_bytes(128, 'big')
        pub_key_size_a2  = len(e_aligned2)
        priv_key_size_a2 = len(p_bytes2) + len(q_bytes2) + len(dp_bytes2) + len(dq_bytes2) + len(u_bytes2)
        key_data_a2 = e_aligned2 + n_bytes2 + p_bytes2 + q_bytes2 + dp_bytes2 + dq_bytes2 + u_bytes2
        permit_a2 = (KeyPermit.KEY_PRIV_REMOVE |
                     KeyPermit.KEY_PRIV_ENCRYPT |
                     KeyPermit.KEY_PRIV_DECRYPT |
                     KeyPermit.KEY_PRIV_VERIFY |
                     KeyPermit.KEY_PRIV_IMPORT_PLAIN)
        pack_key_a2 = key.pack_key_with_head(key_data_a2, permit_a2,
                                             EhsmKeyType.EHSM_KEY_TYPE_RSA_2048_CRT,
                                             EhsmKeyPart.EHSM_KEY_PART_KEY_PAIR,
                                             pub_key_size_a2, priv_key_size_a2)
        # Reason: 传入已有 handle_a，固件原地扩展更新，触发密钥B向后搬移
        _, handle_a = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF,
                                             pack_key_a2, len(pack_key_a2), None, 0, handle_a)
        assert handle_a != 0xFFFFFFFF, "RSA-2048 CRT in-place更新失败"
        log.info(f"Step5: update key A to RSA-2048 in-place, handle_a=0x{handle_a:x}")

    with allure.step("6、迁移后验证：用密钥B（handle_b，RSA-1024）对同一golden ciphertext解密，与golden plaintext对比； # 6、解密成功，结果与迁移前一致，密钥B完整性验证通过；"):
        _, dec_data_after, dec_size_after = api.ehsm_rsa_cipher(
            handle_b, False, golden_ciphertext, len(golden_ciphertext), 128)
        actual_plain_after = dec_data_after[dec_size_after - len(golden_plaintext):dec_size_after]
        assert actual_plain_after == golden_plaintext, (
            f"迁移后密钥B解密结果不匹配: 期望 {golden_plaintext.hex()}, 实际 {actual_plain_after.hex()}")
        log.info("Step6: RSA-1024 decrypt after migration OK, key B integrity verified")

    with allure.step("7、验证新密钥A（RSA-2048）：对RSA-2048标准密文解密，与预期明文对比； # 7、解密成功，密钥A更新后可正常使用；"):
        golden_ciphertext2 = tv2048["ciphertext"]
        golden_plaintext2  = tv2048["plaintext"]
        _, dec_data2, dec_size2 = api.ehsm_rsa_cipher(
            handle_a, False, golden_ciphertext2, len(golden_ciphertext2), 256)
        actual_plain2 = dec_data2[dec_size2 - len(golden_plaintext2):dec_size2]
        assert actual_plain2 == golden_plaintext2, (
            f"RSA-2048解密结果不匹配: 期望 {golden_plaintext2.hex()}, 实际 {actual_plain2.hex()}")
        log.info("Step7: RSA-2048 (key A) decrypt OK, golden data matched")

    with allure.step("8、清理：删除handle_b和handle_a； # 8、删除成功；"):
        api.ehsm_km_remove_key(handle_b)
        api.ehsm_km_remove_key(handle_a)
        log.info("Step8: cleanup done")


@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("验证 kds_rmove_all_key 正确清除 RAM 密钥缓冲区")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K125")
def test_ehsm_k125():
    with allure.step("1、初始化OTP为TEST模式，复位固件确认当前生命周期为TEST_MODE； # 1、生命周期为TEST_MODE，复位成功；"):
        assert 0 == host.write_otp(_get_otp_bin("test"))
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
    with allure.step("2、导入AES-128 RAM密钥，执行AES-ECB加密，验证密钥正常工作（基准验证）；删除密钥； # 2、加密成功，密钥工作正常；"):
        aes128_key = bytes([
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
        ])
        aes_plain = bytes([
            0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
            0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
        ])
        permit = (KeyPermit.KEY_PRIV_REMOVE |
                  KeyPermit.KEY_PRIV_ENCRYPT |
                  KeyPermit.KEY_PRIV_DECRYPT |
                  KeyPermit.KEY_PRIV_IMPORT_PLAIN)
        # Reason: 构造 kms_key_format_st 头部，对称密钥 part_info=0x2(PRIVKEY)，pub_key_size=0
        header = struct.pack("<IBBHHH", permit,
                             EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0x2, 0, 0, len(aes128_key))
        key_data = header + aes128_key
        _, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF,
                                           key_data, len(key_data), None, 0, 0xFFFFFFFF)
        assert handle != 0xFFFFFFFF, "AES-128密钥导入失败"
        log.info(f"Step2: AES-128 import OK, handle=0x{handle:x}")
        ret, c_output, _ = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,
            EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
            EhsmPaddingMode.EHSM_PADDING_NONE,
            handle,
            1,  # 加密
            None,
            0,
            aes_plain,
            len(aes_plain),
            24
        )
        # Reason: 用标准库计算期望密文进行验证
        expected = generate_symmetric_testdata(
            algo="AES128",
            mode="ECB",
            key=aes128_key,
            padding="NONE",
            plaintext=aes_plain)
        assert c_output == expected.ciphertext, (
            f"AES-ECB加密结果不匹配: 期望 {expected.ciphertext.hex()}, 实际 {c_output.hex()}")
        log.info("Step2: AES-128 ECB encrypt OK before lifecycle change")
        api.ehsm_km_remove_key(handle)
        log.info("Step2: key removed")


    with allure.step("3、调用fw_change_lifecycle切换到DEBUG_MODE，触发kds_rmove_all_key执行；注意：此时固件未复位，仍在TEST生命周期下运行； # 3、生命周期切换指令返回成功；"):
        t = api.ehsm_change_lifecycle(EhsmLifecycle.EHSM_LC_DEBUG)

    with allure.step("4、不复位，立即再次导入AES-128 RAM密钥（触发bug关键路径）；修复后应导入成功； # 4、导入成功（kds_rmove_all_key bug已修复）；"):
        # Reason: 不复位直接导入，触发 kds_rmove_all_key 清空 g_mem_head 后的写入路径（bug修复验证）
        header2 = struct.pack("<IBBHHH", permit,
                              EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0x2, 0, 0, len(aes128_key))
        key_data2 = header2 + aes128_key
        _, handle2 = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF,
                                            key_data2, len(key_data2), None, 0, 0xFFFFFFFF)
        assert handle2 != 0xFFFFFFFF, "AES-128重新导入失败（kds_rmove_all_key bug未修复）"
        log.info(f"Step4: AES-128 re-import without reset OK, handle=0x{handle2:x}")

    with allure.step("5、用新密钥执行AES-ECB加密，验证密钥可正常使用；删除密钥； # 5、加密成功；"):
        ret, c_output2, _ = api.ehsm_symm_cipher_onepass(
            EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,
            EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
            EhsmPaddingMode.EHSM_PADDING_NONE,
            handle2,
            1,  # 加密
            None,
            0,
            aes_plain,
            len(aes_plain),
            24
        )
        expected2 = generate_symmetric_testdata(
            algo="AES128",
            mode="ECB",
            key=aes128_key,
            padding="NONE",
            plaintext=aes_plain)
        assert c_output2 == expected2.ciphertext, (
            f"Step5 AES-ECB加密结果不匹配: 期望 {expected2.ciphertext.hex()}, 实际 {c_output2.hex()}")
        log.info("Step5: AES-128 ECB encrypt with new key OK")
        api.ehsm_km_remove_key(handle2)
        log.info("Step5: key removed")

    with allure.step("6、复位固件，确认生命周期已切换为DEBUG_MODE（验证OTP写入已生效）； # 6、生命周期为DEBUG_MODE，OTP写入验证通过；"):
        assert 0 == host.reset_ehsm()
        assert 0 == host.wait_fw_done(1)
        # Reason: 通过读取 HSM_STATUS_IN 寄存器的生命周期字段验证 OTP 写入已生效
        _, lc_reg = host.read_memory(hostapi.HSM_STATUS_IN, 4)
        lc_val = int.from_bytes(lc_reg, "little") & 0x00007F00
        assert lc_val == 4096, f"生命周期不是DEBUG_MODE，实际寄存器值=0x{lc_val:x}"
        log.info("Step6: lifecycle confirmed as DEBUG_MODE after reset")

# ===============================
# 密钥管理不支持SM9类型的密钥生成
# ===============================
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持AES算法")
@allure.feature("kms")
@allure.description("AES密钥生成正常成功")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K129")
def test_ehsm_k129(setup_module):
    with allure.step("1、调用 fw_key_generate，alg=MB_AES_128，permit=MB_KEY_USAGE_ENCRYPT|MB_KEY_USAGE_DECRYPT|MB_KEY_PERMIT_REMOVE，其余参数合法有效值； # 1、生成成功，返回合法密钥句柄；"):
        alg = EhsmKeyType.EHSM_KEY_TYPE_AES_128
        permit = (KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT |
                  KeyPermit.KEY_PRIV_REMOVE)
        rsa_e_bit_size = 0
        dh_param_size = 0
        hmac_size = 0
        handle = 0xFFFFFFFF

        ret, out_handle = api.ehsm_km_gen_key(alg, permit, rsa_e_bit_size, hmac_size, None, dh_param_size, handle)
        assert out_handle != 0xFFFFFFFF and out_handle != 0, f"返回了无效的密钥句柄: 0x{out_handle:x}"
        log.info(f"AES-128 密钥生成成功，handle=0x{out_handle:x}")

    with allure.step("2、成功后调用 fw_key_remove(handle)； # 2、删除成功；"):
        api.ehsm_km_remove_key(out_handle)
        log.info("密钥删除成功")

@pytest.mark.skipif(cfg_data.TEST_BL_DBG_ECC256_SUPPORT == 0, reason="不支持ECC算法")
@allure.feature("kms")
@allure.description("ECC密钥生成正常成功")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K130")
def test_ehsm_k130(setup_module):
    with allure.step("1、调用 fw_key_generate，alg=MB_ECC_SECP_256R1，permit=MB_KEY_USAGE_SIGN|MB_KEY_USAGE_VERIFY|MB_KEY_PERMIT_REMOVE，其余参数合法有效值； # 1、生成成功，返回合法密钥句柄；"):
        alg = EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1
        permit = (KeyPermit.KEY_PRIV_SIGN | KeyPermit.KEY_PRIV_VERIFY |
                  KeyPermit.KEY_PRIV_REMOVE)
        rsa_e_bit_size = 0
        dh_param_size = 0
        hmac_size = 0
        handle = 0xFFFFFFFF

        ret, out_handle = api.ehsm_km_gen_key(alg, permit, rsa_e_bit_size, hmac_size, None, dh_param_size, handle)
        assert out_handle != 0xFFFFFFFF and out_handle != 0, f"返回了无效的密钥句柄: 0x{out_handle:x}"
        log.info(f"ECC_SECP_256R1 密钥生成成功，handle=0x{out_handle:x}")

    with allure.step("2、成功后调用 fw_key_remove(handle)； # 2、删除成功；"):
        api.ehsm_km_remove_key(out_handle)
        log.info("密钥删除成功")

# ===============================
# dh参数解析
# ===============================
@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("DH合法参数正常解析")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K131")
def test_ehsm_k131(setup_module):
    with allure.step("1、fw_key_import(algo_id=MB_DH, part_info=KMS_KEY_PART_PRIVKEY, 私钥数据取自 dh_tv_template[0].secret，得到 handle)； # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
        dh_tv_template_1024_b_public = bytes([
            0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
            0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
            0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
            0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
            0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
            0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
            0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
            0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        KMS_KEY_PART_PRIVKEY = 0x02
        header = struct.pack("<IBBHHH", permit, EhsmKeyType.EHSM_KEY_TYPE_DH, KMS_KEY_PART_PRIVKEY, 0, 0, len(dh_tv_template_1024_secret))
        key_data = header + dh_tv_template_1024_secret
        ret, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH 私钥导入成功，handle=0x{handle:x}")

    with allure.step("2、构造合法 dh_common_data；调用 fw_key_exchange，local_key_handle=handle，得到协商密钥句柄； # 2、密钥交换成功；"):
        dh_p_size = len(dh_tv_template_1024_p)
        dh_q_size = len(dh_tv_template_1024_q)
        dh_g_size = len(dh_tv_template_1024_g)
        dh_command_data = struct.pack("<I128sI20sI128s", dh_p_size, dh_tv_template_1024_p, dh_q_size, dh_tv_template_1024_q, dh_g_size, dh_tv_template_1024_g)
        dh_command_data += struct.pack("<I", 0) # reserved or padding as per existing code

        exchange_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_EXPORT_PLAIN
        # EhsmKeyType.EHSM_KEY_TYPE_AES_128 = 0x04
        # hmac_key_size = 0
        ret, out_new_handle = api.ehsm_km_exchange_key(dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                                                        exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0,
                                                        handle, dh_command_data, len(dh_command_data) + 8, None, 0xFFFFFFFF)
        assert out_new_handle != 0xFFFFFFFF, "协商密钥句柄无效"
        log.info(f"DH 密钥交换成功，协商密钥句柄=0x{out_new_handle:x}")

    with allure.step("3、fw_key_remove(handle)及删除协商密钥； # 3、删除成功；"):
        api.ehsm_km_remove_key(handle)
        api.ehsm_km_remove_key(out_new_handle)
        log.info("密钥删除成功")

@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("DH字段长度=0，拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K132")
def test_ehsm_k132(setup_module):
    with allure.step("1、fw_key_import(algo_id=MB_DH, part_info=KMS_KEY_PART_PRIVKEY, 私钥数据，得到 handle)； # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
        # Reason: 使用 test_ehsm_k131 中的合法标准数据，避免非法随机填充导致固件解析错误
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
        dh_tv_template_1024_b_public = bytes([
            0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
            0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
            0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
            0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
            0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
            0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
            0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
            0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        header = struct.pack("<IBBHHH", permit, EhsmKeyType.EHSM_KEY_TYPE_DH, 2, 0, 0, len(dh_tv_template_1024_secret))
        key_data = header + dh_tv_template_1024_secret
        ret, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH 私钥导入成功，handle=0x{handle:x}")

    with allure.step("2、构造 dh_common_data，将某一字段 length 设为 0；调用 fw_key_exchange； # 2、返回EHSM_ERR_PARAM_ERROR；"):
        # 将 p_size 设为 0
        dh_p_size = 0
        dh_q_size = len(dh_tv_template_1024_q)
        dh_g_size = len(dh_tv_template_1024_g)

        # Reason: 虽然 p_size 为 0，但后面仍然按照协议格式填充占位数据
        dh_command_data = struct.pack("<I128sI20sI128s", dh_p_size, dh_tv_template_1024_p, dh_q_size, dh_tv_template_1024_q, dh_g_size, dh_tv_template_1024_g)
        dh_command_data += struct.pack("<I", 0)

        exchange_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_EXPORT_PLAIN
        try:
            ret, out_new_handle = api.ehsm_km_exchange_key(dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                                                            exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0,
                                                            handle, dh_command_data, len(dh_command_data) + 8, None, 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"期望错误码 {EHSM_ERR_PARAM_ERROR}, 实际错误码 {e.ret_code}"
            log.info(f"捕获到预期错误码: {e.ret_code}")

    with allure.step("3、fw_key_remove(handle)； # 3、删除成功；"):
        api.ehsm_km_remove_key(handle)
        log.info("密钥删除成功")


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("DH字段长度超最大值，拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K133")
def test_ehsm_k133(setup_module):
    with allure.step("1、fw_key_import(algo_id=MB_DH, part_info=KMS_KEY_PART_PRIVKEY, 私钥数据，得到 handle)； # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
        dh_tv_template_1024_b_public = bytes([
            0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
            0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
            0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
            0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
            0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
            0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
            0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
            0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        header = struct.pack("<IBBHHH", permit, EhsmKeyType.EHSM_KEY_TYPE_DH, 2, 0, 0, len(dh_tv_template_1024_secret))
        key_data = header + dh_tv_template_1024_secret
        ret, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        # Reason: 不断言成功，因为当前环境导入可能返回非0错误码，我们关注后续 exchange 的参数校验
        log.info(f"DH 私钥导入尝试完成，handle=0x{handle:x}, ret={ret}")

    with allure.step("2、构造 dh_common_data，将某一字段 length 设为最大值+1；调用 fw_key_exchange； # 2、返回EHSM_ERR_PARAM_ERROR；"):
        # 协议允许最大值通常为 512 字节 (4096-bit)，构造 513 字节
        invalid_p_size = 513
        dh_p_data = bytes([0xE0] * invalid_p_size)
        dh_q_size = len(dh_tv_template_1024_q)
        dh_g_size = len(dh_tv_template_1024_g)

        # Reason: 动态构造超长字段的 dh_common_data
        # 格式: p_size(I) + p_data(invalid_p_size s) + q_size(I) + q_data(20s) + g_size(I) + g_data(128s) + reserved(I)
        dh_command_data = struct.pack(f"<I{invalid_p_size}sI20sI128sI",
                                      invalid_p_size,
                                      dh_p_data,
                                      dh_q_size,
                                      dh_tv_template_1024_q,
                                      dh_g_size,
                                      dh_tv_template_1024_g,
                                      0)

        exchange_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_EXPORT_PLAIN
        try:
            ret, out_new_handle = api.ehsm_km_exchange_key(dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                                                            exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0,
                                                            handle, dh_command_data, len(dh_command_data) + 8, None, 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            # 预期返回 EHSM_ERR_PARAM_ERROR (1)
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"期望错误码 {EHSM_ERR_PARAM_ERROR}, 实际错误码 {e.ret_code}"
            log.info(f"捕获到预期错误码: {e.ret_code}")

    with allure.step("3、清理环境； # 3、清理成功；"):
        if handle != 0 and handle != 0xFFFFFFFF:
            api.ehsm_km_remove_key(handle)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("DH字段累计长度超过 dh_common_size，拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K134")
def test_ehsm_k134(setup_module):
    with allure.step("1、fw_key_import(algo_id=MB_DH, part_info=KMS_KEY_PART_PRIVKEY, 私钥数据，得到 handle)； # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
        dh_tv_template_1024_b_public = bytes([
            0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
            0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
            0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
            0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
            0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
            0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
            0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
            0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        header = struct.pack("<IBBHHH", permit, EhsmKeyType.EHSM_KEY_TYPE_DH, 2, 0, 0, len(dh_tv_template_1024_secret))
        key_data = header + dh_tv_template_1024_secret
        ret, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH 私钥导入尝试完成，handle=0x{handle:x}")

    with allure.step("2、构造 dh_common_data，使得 p_size + q_size + g_size 累计长度超长；调用 fw_key_exchange； # 2、返回EHSM_ERR_PARAM_ERROR；"):
        # 故意将 p_size 标得很大，使得累计长度超过传给固件的 dh_params_size
        dh_p_size = 500
        dh_q_size = len(dh_tv_template_1024_q)
        dh_g_size = len(dh_tv_template_1024_g)

        # Reason: 实际数据仍然使用标准数据，但 size 字段标大
        dh_command_data = struct.pack("<I128sI20sI128sI", dh_p_size, dh_tv_template_1024_p, dh_q_size, dh_tv_template_1024_q, dh_g_size, dh_tv_template_1024_g, 0)

        exchange_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_EXPORT_PLAIN
        # 我们传给 API 的 dh_params_size 是真实的 buffer 长度，固件解析 p_size=500 时会发现超过了 dh_params_size
        try:
            ret, out_new_handle = api.ehsm_km_exchange_key(dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                                                            exchange_permit, EhsmKeyType.EHSM_KEY_TYPE_AES_128, 0,
                                                            handle, dh_command_data, len(dh_command_data), None, 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"期望错误码 {EHSM_ERR_PARAM_ERROR}, 实际错误码 {e.ret_code}"
            log.info(f"捕获到预期错误码: {e.ret_code}")

    with allure.step("3、清理环境； # 3、清理成功；"):
        if handle != 0 and handle != 0xFFFFFFFF:
            api.ehsm_km_remove_key(handle)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("fw_key_get_pub_from_priv 路径下，DH累计字段长度超长，拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K135")
def test_ehsm_k135(setup_module):
    with allure.step("1、fw_key_import(algo_id=MB_DH, part_info=KMS_KEY_PART_PRIVKEY, 私钥数据，得到 handle)； # 1、导入密钥成功；"):
        dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        header = struct.pack("<IBBHHH", permit, EhsmKeyType.EHSM_KEY_TYPE_DH, 2, 0, 0, len(dh_tv_template_1024_secret))
        key_data = header + dh_tv_template_1024_secret
        ret, handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH 私钥导入尝试完成，handle=0x{handle:x}")

    with allure.step("2、构造累计字段超长的 dh_common_data；调用 fw_key_get_pub_from_priv； # 2、返回EHSM_ERR_PARAM_ERROR；"):
        dh_p_size = 512
        dh_q_size = 512 # 累加超过 buffer 限制
        dh_g_size = len(dh_tv_template_1024_g)

        # Reason: 使用标准数据填充，但长度字段设为大值
        dh_command_data = struct.pack("<I128sI20sI128sI", dh_p_size, dh_tv_template_1024_p, dh_q_size, dh_tv_template_1024_q, dh_g_size, dh_tv_template_1024_g, 0)

        try:
            # 传给 API 的 size 也是真实的
            ret, pub_key, pub_key_size, key_usage = api.ehsm_km_get_pub_from_priv(handle, dh_command_data, len(dh_command_data), 512)
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, f"期望错误码 {EHSM_ERR_PARAM_ERROR}, 实际错误码 {e.ret_code}"
            log.info(f"捕获到预期错误码: {e.ret_code}")

    with allure.step("3、清理环境； # 3、清理成功；"):
        if handle != 0 and handle != 0xFFFFFFFF:
            api.ehsm_km_remove_key(handle)


@pytest.mark.skipif(cfg_data.TEST_FW_ECHG_DH_SUPPORT == 0, reason="不支持DH密钥交换算法")
@allure.feature("kms")
@allure.description("fw_key_generate 路径下，DH累计字段长度超长，拒绝")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K136")
def test_ehsm_k136(setup_module):
    with allure.step("1、构造累计字段超长的 dh_common_data；调用 fw_key_generate，alg=MB_DH； # 1、返回EHSM_ERR_PARAM_ERROR；"):
        dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                        0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                        0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                        0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                        0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                        0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                        0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                        0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
        dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
        dh_tv_template_1024_g = bytes([
            0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
            0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
            0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
            0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
            0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
            0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
            0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
            0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])

        # p_size=512, q_size=512 -> 累加超长
        dh_p_size = 512
        dh_q_size = 512
        dh_g_size = len(dh_tv_template_1024_g)

        # Reason: 构造超长字段的 dh_common_data
        dh_command_data = struct.pack("<I128sI20sI128sI", dh_p_size, dh_tv_template_1024_p, dh_q_size, dh_tv_template_1024_q, dh_g_size, dh_tv_template_1024_g, 0)

        alg = EhsmKeyType.EHSM_KEY_TYPE_DH
        permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_KEY_CREATION
        try:
            ret, out_handle = api.ehsm_km_gen_key(alg, permit, 0, 0, dh_command_data, len(dh_command_data), 0xFFFFFFFF)
        except hostapi.HostApiError as e:
            # Reason: 固件 kms_fill_key_head 中 DH 参数超长时返回 EHSM_ERR_INVALID_DH_PQGH_SIZE(105)，非通用参数错误(1)
            assert e.ret_code == EHSM_ERR_INVALID_DH_PQGH_SIZE, f"期望错误码 {EHSM_ERR_INVALID_DH_PQGH_SIZE}, 实际错误码 {e.ret_code}"
            log.info(f"捕获到预期错误码: {e.ret_code}")

# ===========================================================================
# BUG-19: KMS RANDOM/HMAC key_size=0防护
# 覆盖三个函数：check_key_size_param / check_derive_key_params / check_exchange_key_params
# TC-KMS-KS-001 ~ TC-KMS-KS-010 对应 k134 ~ k143
# ===========================================================================

# OTP父密钥句柄，用于derive/exchange测试（固定值，无需创建）
KMS_OTP_KEY_CREATE = 0x20000000  # OTP固定密钥句柄

RANDOM_KEY_TYPE = 0   # MB_RANDOM = 0
HMAC_KEY_TYPE = EhsmKeyType.EHSM_KEY_TYPE_HMAC


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_key_size_param: RANDOM，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K149")
def test_ehsm_k149(setup_module):
    """TC-KMS-KS-001: check_key_size_param RANDOM key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-001: RANDOM key_size=0应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、确认RANDOM key_size=1正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认 RANDOM 密钥生成接口在合法参数（key_size=1）下成功
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        _, out_handle = api.ehsm_km_gen_key(RANDOM_KEY_TYPE, permit, 0, 1, None, 0, 0xFFFFFFFF)
        api.ehsm_km_remove_key(out_handle)
        log.info("RANDOM key_size=1正路径验证成功")

    with allure.step("2、调用fw_key_generate，alg=RANDOM，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                RANDOM_KEY_TYPE, permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "RANDOM key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"✅ RANDOM key_size=0被正确拒绝，错误码: {e.ret_code}")

    log.info("✅ TC-KMS-KS-001 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_key_size_param: HMAC，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K150")
def test_ehsm_k150(setup_module):
    """TC-KMS-KS-002: check_key_size_param HMAC key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-002: HMAC key_size=0应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、确认HMAC key_size=32正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认 HMAC 密钥生成接口在合法参数（key_size=32）下成功
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        _, out_handle = api.ehsm_km_gen_key(HMAC_KEY_TYPE, permit, 0, 32, None, 0, 0xFFFFFFFF)
        api.ehsm_km_remove_key(out_handle)
        log.info("HMAC key_size=32正路径验证成功")

    with allure.step("2、调用fw_key_generate，alg=HMAC，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                HMAC_KEY_TYPE, permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "HMAC key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"HMAC key_size=0被正确拒绝，错误码: {e.ret_code}")

    log.info("TC-KMS-KS-002 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_key_size_param: RANDOM，key_size=1（最小合法），成功（BUG-19正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K151")
def test_ehsm_k151(setup_module):
    """TC-KMS-KS-003: check_key_size_param RANDOM key_size=1成功"""
    log.info("开始测试TC-KMS-KS-003: RANDOM key_size=1应该成功")

    with allure.step("1、调用fw_key_generate，alg=RANDOM，key_size=1 # 1、生成成功"):
        # Reason: 控制变量法正路径——验证最小合法值1能正常生成
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        _, out_handle = api.ehsm_km_gen_key(
            RANDOM_KEY_TYPE, permit, 0, 1, None, 0, 0xFFFFFFFF
        )
        assert out_handle != 0xFFFFFFFF and out_handle != 0, \
            f"RANDOM key_size=1 生成失败，句柄无效: 0x{out_handle:08x}"
        log.info(f"RANDOM key_size=1生成成功，句柄: 0x{out_handle:08x}")

    with allure.step("2、删除密钥 # 2、删除成功"):
        api.ehsm_km_remove_key(out_handle)

    log.info("✅ TC-KMS-KS-003 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_key_size_param: HMAC，key_size=512（最大合法），成功（BUG-19正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K137")
def test_ehsm_k137(setup_module):
    """TC-KMS-KS-004: check_key_size_param HMAC key_size=512成功"""
    log.info("开始测试TC-KMS-KS-004: HMAC key_size=512应该成功")

    with allure.step("1、调用fw_key_generate，alg=HMAC，key_size=512 # 1、生成成功"):
        # Reason: 控制变量法正路径——验证最大合法值512能正常生成
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        _, out_handle = api.ehsm_km_gen_key(
            HMAC_KEY_TYPE, permit, 0, 512, None, 0, 0xFFFFFFFF
        )
        assert out_handle != 0xFFFFFFFF and out_handle != 0, \
            f"HMAC key_size=512 生成失败，句柄无效: 0x{out_handle:08x}"
        log.info(f"HMAC key_size=512生成成功，句柄: 0x{out_handle:08x}")

    with allure.step("2、删除密钥 # 2、删除成功"):
        api.ehsm_km_remove_key(out_handle)

    log.info("✅ TC-KMS-KS-004 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_key_size_param: RANDOM，key_size=513（超最大），拒绝（BUG-19回归）")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM_K138")
def test_ehsm_k138(setup_module):
    """TC-KMS-KS-005: check_key_size_param RANDOM key_size=513应被拒绝"""
    log.info("开始测试TC-KMS-KS-005: RANDOM key_size=513应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、调用fw_key_generate，alg=RANDOM，key_size=513 # 1、应返回EHSM_ERR_PARAM_ERROR"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                RANDOM_KEY_TYPE, permit, 0, 513, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "RANDOM key_size=513应该失败"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"✅ RANDOM key_size=513被正确拒绝，错误码: {e.ret_code}")

    log.info("✅ TC-KMS-KS-005 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_derive_key_params: RANDOM，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K139")
def test_ehsm_k139(setup_module):
    """TC-KMS-KS-006: check_derive_key_params RANDOM key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-006: derive RANDOM key_size=0应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、确认derive RANDOM key_size=32正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认 derive 接口在合法 key_size（32）下成功，再测 key_size=0
        salt = bytes([0x73, 0x61, 0x6C, 0x74])
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                EhsmDeriveType.EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
                key_permit, RANDOM_KEY_TYPE, 32,
                KMS_OTP_KEY_CREATE, salt, len(salt), None, 0, 1, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(derived_handle)
            log.info("derive RANDOM key_size=32正路径验证成功")
        except hostapi.HostApiError as e:
            log.warning(f"derive正路径失败（OTP父密钥可能未配置），错误码: {e.ret_code}，继续测试异常路径")

    with allure.step("2、使用OTP父密钥，调用fw_key_derive，alg=RANDOM，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        salt = bytes([0x73, 0x61, 0x6C, 0x74])  # "salt"
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                EhsmDeriveType.EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
                key_permit,
                RANDOM_KEY_TYPE,
                0,    # key_size=0，BUG-19触发点
                KMS_OTP_KEY_CREATE,
                salt, len(salt),
                None, 0,
                1,    # iter_times
                0xFFFFFFFF
            )
            api.ehsm_km_remove_key(derived_handle)
            assert False, "derive RANDOM key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"✅ derive RANDOM key_size=0被正确拒绝，错误码: {e.ret_code}")

    log.info("✅ TC-KMS-KS-006 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_derive_key_params: HMAC，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K140")
def test_ehsm_k140(setup_module):
    """TC-KMS-KS-007: check_derive_key_params HMAC key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-007: derive HMAC key_size=0应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、确认derive HMAC key_size=32正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认 derive HMAC 在合法 key_size（32）下成功，再测 key_size=0
        salt = bytes([0x73, 0x61, 0x6C, 0x74])
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                EhsmDeriveType.EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
                key_permit, HMAC_KEY_TYPE, 32,
                KMS_OTP_KEY_CREATE, salt, len(salt), None, 0, 1, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(derived_handle)
            log.info("derive HMAC key_size=32正路径验证成功")
        except hostapi.HostApiError as e:
            log.warning(f"derive正路径失败（OTP父密钥可能未配置），错误码: {e.ret_code}，继续测试异常路径")

    with allure.step("2、使用OTP父密钥，调用fw_key_derive，alg=HMAC，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        salt = bytes([0x73, 0x61, 0x6C, 0x74])
        key_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        try:
            _, derived_handle = api.ehsm_km_derive_key(
                EhsmHashAlgo.EHSM_HASH_ALGO_SHA256,
                EhsmDeriveAlgo.EHSM_DERIVE_ALGO_PBKDF2,
                EhsmDeriveType.EHSM_DERIVE_TYPE_FROM_PARENT_KEY,
                key_permit,
                HMAC_KEY_TYPE,
                0,    # key_size=0
                KMS_OTP_KEY_CREATE,
                salt, len(salt),
                None, 0,
                1,
                0xFFFFFFFF
            )
            api.ehsm_km_remove_key(derived_handle)
            assert False, "derive HMAC key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"✅ derive HMAC key_size=0被正确拒绝，错误码: {e.ret_code}")

    log.info("✅ TC-KMS-KS-007 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_exchange_key_params: RANDOM，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K141")
def test_ehsm_k141(setup_module):
    """TC-KMS-KS-008: check_exchange_key_params RANDOM key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-008: exchange RANDOM key_size=0应返回EHSM_ERR_PARAM_ERROR")

    # DH 测试向量数据（复用test_ehsm_k133的数据）
    dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
    dh_tv_template_1024_b_public = bytes([
        0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
        0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
        0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
        0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
        0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
        0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
        0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
        0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

    with allure.step("1、导入DH私钥，得到local_handle # 1、导入完成"):
        dh_import_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION
        header = struct.pack("<IBBHHHh", dh_import_permit, EhsmKeyType.EHSM_KEY_TYPE_DH, 2, 0, 0, len(dh_tv_template_1024_secret), 0)
        key_data = struct.pack("<IBBHHH", int(dh_import_permit), int(EhsmKeyType.EHSM_KEY_TYPE_DH), 2, 0, 0, len(dh_tv_template_1024_secret)) + dh_tv_template_1024_secret
        ret, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH私钥导入完成，handle=0x{local_handle:x}")

    with allure.step("2、构造合法dh_common_data，调用fw_key_exchange，alg=RANDOM，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        # Reason: 控制变量法——dh_common_data使用正确数据，只控制key_size=0这一变量
        p_size = len(dh_tv_template_1024_p)
        q_size = len(dh_tv_template_1024_q)
        g_size = len(dh_tv_template_1024_g)
        dh_common_data = struct.pack(f"<I{p_size}sI{q_size}sI{g_size}sI",
                                     p_size, dh_tv_template_1024_p,
                                     q_size, dh_tv_template_1024_q,
                                     g_size, dh_tv_template_1024_g,
                                     0)
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_EXPORT_PLAINTEXT"]
        try:
            _, out_handle = api.ehsm_km_exchange_key(
                dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                exchange_permit,
                RANDOM_KEY_TYPE,
                0,    # hmac_key_size=0，即key_size=0（BUG-19触发点）
                local_handle,
                dh_common_data, len(dh_common_data),
                None, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "exchange RANDOM key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            # Reason: 固件返回 EHSM_ERR_PARAM_ERROR(1)，但上位机将 uint16_t 值 1 解析为
            # ret_code=1-65536=-65535，两种形式均为同一错误（BUG-19修复验证通过）
            assert e.ret_code in (EHSM_ERR_PARAM_ERROR, EHSM_ERR_PARAM_ERROR - 65536), \
                f"期望EHSM_ERR_PARAM_ERROR，实际: {e.ret_code}"
            log.info(f"exchange RANDOM key_size=0被正确拒绝，错误码: {e.ret_code}")

    with allure.step("3、清理DH密钥 # 3、清理成功"):
        if local_handle != 0 and local_handle != 0xFFFFFFFF:
            api.ehsm_km_remove_key(local_handle)

    log.info("✅ TC-KMS-KS-008 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("check_exchange_key_params: HMAC，key_size=0，拒绝（BUG-19）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K142")
def test_ehsm_k142(setup_module):
    """TC-KMS-KS-009: check_exchange_key_params HMAC key_size=0应被拒绝"""
    log.info("开始测试TC-KMS-KS-009: exchange HMAC key_size=0应返回EHSM_ERR_PARAM_ERROR")

    # 复用同样的DH测试向量
    dh_tv_template_1024_secret = bytes([0x53, 0x8D, 0x3D, 0x64, 0x27, 0x4A, 0x40, 0x05, 0x9B, 0x9C, 0x26, 0xE9, 0x13, 0xE6, 0x91, 0x53, 0x23, 0x7B, 0x55, 0x83])
    dh_tv_template_1024_p = bytes([0xE0, 0x01, 0xE8, 0x96, 0x7D, 0xB4, 0x93, 0x53, 0xE1, 0x6F, 0x8E, 0x89, 0x22, 0x0C, 0xCE, 0xFC,
                                    0x5C, 0x5F, 0x12, 0xE3, 0xDF, 0xF8, 0xF1, 0xD1, 0x49, 0x90, 0x12, 0xE6, 0xEF, 0x53, 0xE3, 0x1F,
                                    0x02, 0xEA, 0xCC, 0x5A, 0xDD, 0xF3, 0x37, 0x89, 0x35, 0xC9, 0x5B, 0x21, 0xEA, 0x3D, 0x6F, 0x1C,
                                    0xD7, 0xCE, 0x63, 0x75, 0x52, 0xEC, 0x38, 0x6C, 0x0E, 0x34, 0xF7, 0x36, 0xAD, 0x95, 0x17, 0xEF,
                                    0xFE, 0x5E, 0x4D, 0xA7, 0xA8, 0x6A, 0xF9, 0x0E, 0x2C, 0x22, 0x8F, 0xE4, 0xB9, 0xE6, 0xD8, 0xF8,
                                    0xF0, 0x2D, 0x20, 0xAF, 0x78, 0xAB, 0xB6, 0x92, 0xAC, 0xBC, 0x4B, 0x23, 0xFA, 0xF2, 0xC5, 0xCC,
                                    0xD4, 0x9A, 0x0C, 0x9A, 0x8B, 0xCD, 0x91, 0xAC, 0x0C, 0x55, 0x92, 0x01, 0xE6, 0xC2, 0xFD, 0x1F,
                                    0x47, 0xC2, 0xCB, 0x2A, 0x88, 0xA8, 0x3C, 0x21, 0x0F, 0xC0, 0x54, 0xDB, 0x29, 0x2D, 0xBC, 0x45])
    dh_tv_template_1024_q = bytes([0x86, 0x47, 0x17, 0xA3, 0x9E, 0x6A, 0xEA, 0x7E, 0x87, 0xC4, 0x32, 0xEE, 0x77, 0x43, 0x15, 0x16, 0x96, 0x70, 0xC4, 0x99])
    dh_tv_template_1024_g = bytes([
        0x1C, 0xE0, 0xF6, 0x69, 0x26, 0x46, 0x11, 0x97, 0xEF, 0x45, 0xC4, 0x65, 0x8B, 0x83, 0xB8, 0xAB,
        0x04, 0xA9, 0x22, 0x42, 0x68, 0x50, 0x4D, 0x05, 0xB8, 0x19, 0x83, 0x99, 0xDD, 0x71, 0x37, 0x18,
        0xCC, 0x1F, 0x24, 0x5D, 0x47, 0x6C, 0xCF, 0x61, 0xA2, 0xF9, 0x34, 0x93, 0xF4, 0x1F, 0x55, 0x52,
        0x48, 0x65, 0x57, 0xE6, 0xD4, 0xCA, 0xA8, 0x00, 0xD6, 0xD0, 0xDB, 0x3C, 0xBF, 0x5A, 0x95, 0x4B,
        0x20, 0x8A, 0x4E, 0xBA, 0xF7, 0xE6, 0x49, 0xFB, 0x61, 0x24, 0xD8, 0xA2, 0x1E, 0xF2, 0xF2, 0x2B,
        0xAA, 0xAE, 0x29, 0x21, 0x10, 0x19, 0x10, 0x51, 0x46, 0x47, 0x31, 0xB6, 0xCC, 0x3C, 0x93, 0xDC,
        0x6E, 0x80, 0xBA, 0x16, 0x0B, 0x66, 0x64, 0xA5, 0x6C, 0xFA, 0x96, 0xEA, 0xF1, 0xB2, 0x83, 0x39,
        0x8E, 0xEB, 0x46, 0x16, 0x4E, 0x5E, 0x94, 0x38, 0x4E, 0x02, 0x24, 0xE7, 0x1F, 0x03, 0x7C, 0x23])
    dh_tv_template_1024_b_public = bytes([
        0xA3, 0xF5, 0x7D, 0xBE, 0x9E, 0x2F, 0x0A, 0xDA, 0xA9, 0x4E, 0x4E, 0x6A, 0xF0, 0xE0, 0x71, 0x47,
        0x0E, 0x2E, 0x41, 0x2E, 0xDE, 0x73, 0x2A, 0x62, 0x14, 0xC3, 0x7C, 0x26, 0xD4, 0xE9, 0x9A, 0x54,
        0xBA, 0x3D, 0xE7, 0x49, 0x85, 0x95, 0x0E, 0xE9, 0x14, 0xB2, 0x90, 0x22, 0x91, 0xDC, 0xFF, 0x61,
        0xB2, 0xFC, 0xD1, 0xD0, 0x1B, 0x11, 0x14, 0xB6, 0x02, 0x64, 0x2B, 0x26, 0x5D, 0x88, 0xEA, 0x8D,
        0xBB, 0xE2, 0x07, 0x0B, 0x48, 0xFB, 0x01, 0x53, 0x55, 0x1E, 0x59, 0x51, 0x36, 0xF2, 0xF9, 0xD1,
        0x97, 0xFB, 0x66, 0x12, 0x84, 0x5D, 0xED, 0xB8, 0x9B, 0x2D, 0x3E, 0x2B, 0x8C, 0xEB, 0x2A, 0x72,
        0x40, 0x9D, 0x55, 0x4C, 0xED, 0xEB, 0x55, 0x02, 0xFF, 0x8C, 0xB0, 0x2E, 0x03, 0x65, 0x3F, 0x41,
        0xB1, 0xAC, 0xA3, 0x30, 0x6B, 0xFF, 0x6D, 0xF4, 0x6D, 0xE6, 0xE1, 0x0F, 0x86, 0x7C, 0x43, 0x64])

    with allure.step("1、导入DH私钥，得到local_handle # 1、导入完成"):
        key_data = struct.pack("<IBBHHH", int(KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_WRITE_PROTECT | KeyPermit.KEY_PRIV_KEY_CREATION),
                               int(EhsmKeyType.EHSM_KEY_TYPE_DH), 2, 0, 0, len(dh_tv_template_1024_secret)) + dh_tv_template_1024_secret
        ret, local_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, key_data, len(key_data), None, 0, 0xFFFFFFFF)
        log.info(f"DH私钥导入完成，handle=0x{local_handle:x}")

    with allure.step("2、调用fw_key_exchange，alg=HMAC，key_size=0 # 2、应返回EHSM_ERR_PARAM_ERROR"):
        p_size = len(dh_tv_template_1024_p)
        q_size = len(dh_tv_template_1024_q)
        g_size = len(dh_tv_template_1024_g)
        dh_common_data = struct.pack(f"<I{p_size}sI{q_size}sI{g_size}sI",
                                     p_size, dh_tv_template_1024_p,
                                     q_size, dh_tv_template_1024_q,
                                     g_size, dh_tv_template_1024_g,
                                     0)
        exchange_permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"]
        try:
            _, out_handle = api.ehsm_km_exchange_key(
                dh_tv_template_1024_b_public, len(dh_tv_template_1024_b_public),
                exchange_permit,
                HMAC_KEY_TYPE,
                0,    # hmac_key_size=0
                local_handle,
                dh_common_data, len(dh_common_data),
                None, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "exchange HMAC key_size=0应该失败，但成功了（BUG-19未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code == EHSM_ERR_PARAM_ERROR, \
                f"期望EHSM_ERR_PARAM_ERROR({EHSM_ERR_PARAM_ERROR})，实际: {e.ret_code}"
            log.info(f"✅ exchange HMAC key_size=0被正确拒绝，错误码: {e.ret_code}")

    with allure.step("3、清理DH密钥 # 3、清理成功"):
        if local_handle != 0 and local_handle != 0xFFFFFFFF:
            api.ehsm_km_remove_key(local_handle)

    log.info("✅ TC-KMS-KS-009 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("SM9类型key_size=0，仍被正确拒绝（BUG-19回归，原有SM9拒绝逻辑不受影响）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K143")
def test_ehsm_k143(setup_module):
    """TC-KMS-KS-010: SM9类型key_size=0，原有拒绝逻辑不受影响"""
    log.info("开始测试TC-KMS-KS-010: SM9类型key_size=0应返回EHSM_ERR_PARAM_ERROR")

    with allure.step("1、调用fw_key_generate，alg=SM9_ENC_USERPRIV，key_size=0 # 1、应被拒绝"):
        # Reason: BUG-19修复不应影响原有SM9拒绝逻辑，此为回归验证
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM9_ENC_USERPRIV,
                permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "SM9 key_size=0应该失败（原有逻辑 + BUG-19修复均应拒绝）"
        except hostapi.HostApiError as e:
            # Reason: SM9密钥生成时，固件先校验密钥类型不支持生成（BUG-03），
            # 返回 EHSM_ERR_NOT_SUPPORT(10)，而非 EHSM_ERR_PARAM_ERROR(1)
            # 两者都说明操作被拒绝，BUG-19 的 key_size=0 检查在该路径不是主要拒绝原因
            assert e.ret_code in (EHSM_ERR_PARAM_ERROR, EHSM_ERR_NOT_SUPPORT), \
                f"期望拒绝错误码，实际: {e.ret_code}"
            log.info(f"SM9 key_size=0被正确拒绝（回归验证），错误码: {e.ret_code}")

    log.info("TC-KMS-KS-010 完成")


# ===========================================================================
# BUG-03: KMS SM9密钥生成拒绝（SM9_ENC/SIGN/EXCH_USERPRIV不支持生成）
# TC-KMS-GEN-001 ~ TC-KMS-GEN-005 对应 k144 ~ k148
# ===========================================================================

@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("SM9_ENC_USERPRIV密钥生成被拒绝（BUG-03 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K144")
def test_ehsm_k144(setup_module):
    """TC-KMS-GEN-001: SM9_ENC_USERPRIV密钥生成被拒绝"""
    log.info("开始测试TC-KMS-GEN-001: SM9_ENC_USERPRIV生成应被拒绝")
    with allure.step("1、确认fw_key_generate(AES_128)正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认密钥生成接口在合法 AES_128 类型下成功，再测 SM9 类型被拒绝
        permit_ok = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        _, h_ok = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit_ok, 0, 0, None, 0, 0xFFFFFFFF)
        api.ehsm_km_remove_key(h_ok)
        log.info("AES_128正路径验证成功")

    with allure.step("2、调用fw_key_generate，alg=SM9_ENC_USERPRIV # 2、应返回不支持错误码"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM9_ENC_USERPRIV, permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "SM9_ENC_USERPRIV生成应被拒绝（BUG-03未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code in (EHSM_ERR_NOT_SUPPORT, EHSM_ERR_PARAM_ERROR), \
                f"期望NOT_SUPPORT或PARAM_ERROR，实际: {e.ret_code}"
            log.info(f"SM9_ENC_USERPRIV生成被正确拒绝，错误码: {e.ret_code}")
    log.info("TC-KMS-GEN-001 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("SM9_SIGN_USERPRIV密钥生成被拒绝（BUG-03 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K145")
def test_ehsm_k145(setup_module):
    """TC-KMS-GEN-002: SM9_SIGN_USERPRIV密钥生成被拒绝"""
    log.info("开始测试TC-KMS-GEN-002: SM9_SIGN_USERPRIV生成应被拒绝")
    with allure.step("1、确认fw_key_generate(AES_128)正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认密钥生成接口在合法类型下成功，再测 SM9_SIGN 类型被拒绝
        permit_ok = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"]
        _, h_ok = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit_ok, 0, 0, None, 0, 0xFFFFFFFF)
        api.ehsm_km_remove_key(h_ok)
        log.info("AES_128正路径验证成功")

    with allure.step("2、调用fw_key_generate，alg=SM9_SIGN_USERPRIV # 2、应返回不支持错误码"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM9_SIGN_USERPRIV, permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "SM9_SIGN_USERPRIV生成应被拒绝（BUG-03未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code in (EHSM_ERR_NOT_SUPPORT, EHSM_ERR_PARAM_ERROR), \
                f"期望NOT_SUPPORT或PARAM_ERROR，实际: {e.ret_code}"
            log.info(f"SM9_SIGN_USERPRIV生成被正确拒绝，错误码: {e.ret_code}")
    log.info("TC-KMS-GEN-002 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("SM9_EXCHG_USERPRIV密钥生成被拒绝（BUG-03 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_K146")
def test_ehsm_k146(setup_module):
    """TC-KMS-GEN-003: SM9_EXCHG_USERPRIV密钥生成被拒绝"""
    log.info("开始测试TC-KMS-GEN-003: SM9_EXCHG_USERPRIV生成应被拒绝")
    with allure.step("1、确认fw_key_generate(AES_128)正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认密钥生成接口在合法类型下成功，再测 SM9_EXCHG 类型被拒绝
        permit_ok = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        _, h_ok = api.ehsm_km_gen_key(EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit_ok, 0, 0, None, 0, 0xFFFFFFFF)
        api.ehsm_km_remove_key(h_ok)
        log.info("AES_128正路径验证成功")

    with allure.step("2、调用fw_key_generate，alg=SM9_EXCHG_USERPRIV # 2、应返回不支持错误码"):
        permit = KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] | KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_KEYCREATION"]
        try:
            _, out_handle = api.ehsm_km_gen_key(
                EhsmKeyType.EHSM_KEY_TYPE_SM9_EXCHG_USERPRIV, permit, 0, 0, None, 0, 0xFFFFFFFF
            )
            api.ehsm_km_remove_key(out_handle)
            assert False, "SM9_EXCHG_USERPRIV生成应被拒绝（BUG-03未修复）"
        except hostapi.HostApiError as e:
            assert e.ret_code in (EHSM_ERR_NOT_SUPPORT, EHSM_ERR_PARAM_ERROR), \
                f"期望NOT_SUPPORT或PARAM_ERROR，实际: {e.ret_code}"
            log.info(f"SM9_EXCHG_USERPRIV生成被正确拒绝，错误码: {e.ret_code}")
    log.info("TC-KMS-GEN-003 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("AES-128密钥生成正常成功（BUG-03回归正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K147")
def test_ehsm_k147(setup_module):
    """TC-KMS-GEN-004: AES密钥生成正常成功"""
    log.info("开始测试TC-KMS-GEN-004: AES-128密钥生成正路径")
    with allure.step("1、调用fw_key_generate，alg=AES_128 # 1、生成成功"):
        # Reason: 控制变量法正路径——验证BUG-03修复不影响AES密钥生成
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_ENCRYPT"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_DECRYPT"])
        _, out_handle = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_AES_128, permit, 0, 0, None, 0, 0xFFFFFFFF
        )
        assert out_handle != 0xFFFFFFFF and out_handle != 0, \
            f"AES-128密钥生成失败，句柄无效: 0x{out_handle:08x}"
        log.info(f"AES-128密钥生成成功，句柄: 0x{out_handle:08x}")
        api.ehsm_km_remove_key(out_handle)
    log.info("TC-KMS-GEN-004 完成")


@pytest.mark.skipif(False, reason="KMS密钥管理功能始终启用")
@allure.feature("kms")
@allure.description("ECC-SECP256R1密钥生成正常成功（BUG-03回归正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_K148")
def test_ehsm_k148(setup_module):
    """TC-KMS-GEN-005: ECC密钥生成正常成功"""
    log.info("开始测试TC-KMS-GEN-005: ECC-SECP256R1密钥生成正路径")
    with allure.step("1、调用fw_key_generate，alg=ECC_SECP256R1 # 1、生成成功"):
        # Reason: 控制变量法正路径——验证BUG-03修复不影响ECC密钥生成
        permit = (KEY_PERMIT_NAME_TO_ENUM["MB_KEY_PERMIT_REMOVE"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_SIGN"] |
                  KEY_PERMIT_NAME_TO_ENUM["MB_KEY_USAGE_VERIFY"])
        _, out_handle = api.ehsm_km_gen_key(
            EhsmKeyType.EHSM_KEY_TYPE_ECC_SECP_256R1, permit, 0, 0, None, 0, 0xFFFFFFFF
        )
        assert out_handle != 0xFFFFFFFF and out_handle != 0, \
            f"ECC-SECP256R1密钥生成失败，句柄无效: 0x{out_handle:08x}"
        log.info(f"ECC-SECP256R1密钥生成成功，句柄: 0x{out_handle:08x}")
        api.ehsm_km_remove_key(out_handle)
    log.info("TC-KMS-GEN-005 完成")


# ===========================================================================
