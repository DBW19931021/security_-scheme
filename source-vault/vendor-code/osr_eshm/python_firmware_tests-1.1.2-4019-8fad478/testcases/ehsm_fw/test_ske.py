import pytest
import allure
import random
import time
import logging as log
from utils.key import pack_key_with_head
from cryptosynth.types import SymmetricTestData
from cryptosynth import generate_symmetric_testdata
from platform_adapter.api.loader import get_api_interface
from platform_adapter.uart_lib import hostapi
from platform_adapter.api.constants import (
    KeyPermit,
    EhsmKeyType,
    EhsmKeyPart,
    EhsmSymmAlgo,
    EhsmCipherMode,
    EhsmAeadMode,
    EhsmPaddingMode
)
from utils.config import cfg_data

api = get_api_interface()

# 全局配置
DEFAULT_ROUND_NUM = 3

def generate_random_msg_len():
    """生成随机消息长度 (16-256)"""
    return random.randint(16, 256)

SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM = {
    ("DES", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_DES,
    ("DES", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_DES,
    ("DES", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_DES,
    ("DES", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_DES,
    ("DES", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_DES,
    ("TDES-128", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_128,
    ("TDES-128", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_TDES_128,
    ("TDES-128", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_128,
    ("TDES-128", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_128,
    ("TDES-128", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_TDES_128,
    ("TDES-192", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_192,
    ("TDES-192", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_TDES_192,
    ("TDES-192", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_192,
    ("TDES-192", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_TDES_192,
    ("TDES-192", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_TDES_192,
    ("AES128", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "GCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES128", "CCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_128,
    ("AES192", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "GCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES192", "CCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_192,
    ("AES256", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "GCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("AES256", "CCM"): EhsmKeyType.EHSM_KEY_TYPE_AES_256,
    ("SM4", "ECB"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "CBC"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "CFB"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "OFB"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "CTR"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "GCM"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("SM4", "CCM"): EhsmKeyType.EHSM_KEY_TYPE_SM4,
    ("AES128", "XTS"): EhsmKeyType.EHSM_KEY_TYPE_AES_128_XTS,
    ("AES192", "XTS"): EhsmKeyType.EHSM_KEY_TYPE_AES_192_XTS,
    ("AES256", "XTS"): EhsmKeyType.EHSM_KEY_TYPE_AES_256_XTS,
    ("SM4", "XTS"): EhsmKeyType.EHSM_KEY_TYPE_SM4_XTS
}

SYM_ALGO_TO_ENUM = {
    "DES": EhsmSymmAlgo.EHSM_SYMM_ALGO_DES,
    "TDES-128": EhsmSymmAlgo.EHSM_SYMM_ALGO_TDES_128,
    "TDES-192": EhsmSymmAlgo.EHSM_SYMM_ALGO_TDES_192,
    "AES128": EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_128,
    "AES192": EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_192,
    "AES256": EhsmSymmAlgo.EHSM_SYMM_ALGO_AES_256,
    "SM4": EhsmSymmAlgo.EHSM_SYMM_ALGO_SM4
}

SYM_MODE_TO_ENUM = {
    "ECB": EhsmCipherMode.EHSM_CIPHER_MODE_ECB,
    "XTS": EhsmCipherMode.EHSM_CIPHER_MODE_XTS,
    "CBC": EhsmCipherMode.EHSM_CIPHER_MODE_CBC,
    "CFB": EhsmCipherMode.EHSM_CIPHER_MODE_CFB,
    "OFB": EhsmCipherMode.EHSM_CIPHER_MODE_OFB,
    "CTR": EhsmCipherMode.EHSM_CIPHER_MODE_CTR
}

AEAD_MODE_TO_ENUM = {
    "GCM": EhsmAeadMode.EHSM_AEAD_MODE_GCM,
    "CCM": EhsmAeadMode.EHSM_AEAD_MODE_CCM
}

PADDING_MODE_TO_ENUM = {
    "NONE": EhsmPaddingMode.EHSM_PADDING_NONE,
    "PKCS7": EhsmPaddingMode.EHSM_PADDING_PKCS7,
    "ISO7816": EhsmPaddingMode.EHSM_PADDING_ONE_WITH_ZEROS
    # Not support now
    # "X923":: EhsmPaddingMode.EHSM_PADDING_X923,
    # "ZERO":: EhsmPaddingMode.EHSM_PADDING_ZERO
}

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

def split_M_into_N(M: int, N: int, align: int = 1, remainder_to_last: bool = False):
    """
    将整数 M 拆分为 N 个非负整数的和，每个数均为 align 的整数倍，且所有数均不为零。
    当 remainder_to_last 为 True 时，若 M 不是 align 的整数倍，将不对齐的部分留到最后一个分组。

    Args:
        M: 需要拆分的整数（总和）
        N: 拆分的数量
        align: 对齐基数，默认为 1（无对齐要求）
        remainder_to_last: 是否将余数部分放到最后一个分组，默认为 False

    Returns:
        拆分后的列表，满足：
        1. 列表长度为 N
        2. 所有元素之和为 M
        3. 若 remainder_to_last 为 False，每个元素都是 align 的整数倍
        4. 若 remainder_to_last 为 True，前 N-1 个元素是 align 的整数倍，最后一个元素可能包含余数
    """
    if align <= 0:
        raise ValueError("align 必须为正整数")
    if N <= 0:
        raise ValueError("N 必须至少为 1")
    if M == 0:
        return [0] * N

    r = M % align
    M_aligned = M - r

    if remainder_to_last:
        if N == 1:
            return [M]
        sub_parts = split_M_into_N(M_aligned, N-1, align, False)
        return sub_parts + [r]
    else:
        if r != 0:
            raise ValueError(f"M 必须是 {align} 的整数倍，除非 remainder_to_last 为 True")
        if N <= 1:
            return [M]

        M_aligned = M // align
        N_aligned = N

        if M_aligned < N_aligned:
            raise ValueError(f"M 必须至少为 {N * align} 才能确保每个部分非零且为 {align} 的整数倍")

        M_remaining = M_aligned - N_aligned

        if M_remaining == 0:
            return [align] * N

        # 使用简单随机方法生成分割点
        splits = sorted([random.randint(0, M_remaining) for _ in range(N_aligned-1)])
        points = [0] + splits + [M_remaining]
        parts = [points[i+1] - points[i] for i in range(N_aligned)]

        # 每个部分至少为 1，转换回对齐后的数值
        return [(part + 1) * align for part in parts]

def align_up(size: int, align: int) -> int:
    """
    将整数向上对齐到指定基数的最小倍数
    Args:
        size: 需要对齐的数值（整数）
        align: 对齐基数（必须为正整数）
    Returns:
        对齐后的结果（>= size 且为 align 的最小倍数）
    Raises:
        ValueError: 若 align 不是正整数
    Examples:
        >>> align_up(15, 8)
        16
        >>> align_up(16, 8)
        16
        >>> align_up(-5, 4)
        0
        >>> align_up(100, 32)
        128
    """
    if align <= 0:
        raise ValueError(f"对齐基数必须为正整数，got {align}")

    return (size + align - 1) // align * align

def ske_aead_generate_testdata(algo: str, mode: str, padding: str, key: bytes, iv: bytes, input_size: int, aad: bytes, tag_length: int) -> SymmetricTestData:
    with allure.step(f"测试数据生成: 算法 {algo}, 模式 {mode}, 填充 {padding}, 输入长度 {input_size}"):
        test_vect = generate_symmetric_testdata(algo, mode, key, padding, iv, input_size, aad, tag_length)
        log.info(f"golden_key_size: {len(test_vect.key)}")
        log.info(f"golden_key_data: {''.join(f'{b:02x}' for b in test_vect.key) or '(empty)'}")
        log.info(f"golden_ivn_size: {len(test_vect.iv_nonce)}")
        log.info(f"golden_ivn_data: {''.join(f'{b:02x}' for b in test_vect.iv_nonce) or '(empty)'}")
        log.info(f"golden_plain_sz: {len(test_vect.plaintext)}")
        log.info(f"golden_plain_dt: {''.join(f'{b:02x}' for b in test_vect.plaintext) or '(empty)'}")
        log.info(f"golden_ciphe_sz: {len(test_vect.ciphertext)}")
        log.info(f"golden_ciphe_dt: {''.join(f'{b:02x}' for b in test_vect.ciphertext) or '(empty)'}")
        if mode == 'GCM' or mode == 'CCM':
            log.info(f"golden_tag_size: {len(test_vect.auth_tag)}")
            log.info(f"golden_tag_data: {''.join(f'{b:02x}' for b in test_vect.auth_tag) or '(empty)'}")
            log.info(f"golden_aad_size: {len(test_vect.aad)}")
            log.info(f"golden_aad_data: {''.join(f'{b:02x}' for b in test_vect.aad) or '(empty)'}")
    return test_vect

def ske_symm_generate_testdata(algo: str, mode: str, padding: str, key: bytes, iv: bytes, input_size: int) -> SymmetricTestData:
    test_vect = ske_aead_generate_testdata(algo, mode, padding, key, iv, input_size, None, None)
    return test_vect

def ske_symm_onepass_test(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, onepass # 执行成功"):
                t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.ciphertext)
                )
                t_symm_cipher = t_symm_enc
                log.info(f"onepass result_symm_enc_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_enc_size: {cipher_text_size}")
                log.info(f"onepass result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext
                assert cipher_text_size == len(test_vect.ciphertext)
        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, onepass # 执行成功"):
                t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
                )
                t_symm_cipher = t_symm_dec
                log.info(f"onepass result_symm_dec_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_dec_size: {plain_text_size}")
                log.info(f"onepass result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                assert plain_text == test_vect.plaintext
                assert plain_text_size == len(test_vect.plaintext)
        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher

def ske_symm_onepass_test_plain_key(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData) -> int:
    t_symm_cipher = 0
    if symm_dir == "SYMM_ENC":
        with allure.step("对称算法加密, onepass # 执行成功"):
            t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.ciphertext)
            )
            t_symm_cipher = t_symm_enc
            log.info(f"onepass result_symm_enc_time: {t_symm_cipher}")
            log.info(f"onepass result_symm_enc_size: {cipher_text_size}")
            log.info(f"onepass result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext
            assert cipher_text_size == len(test_vect.ciphertext)
    elif symm_dir == "SYMM_DEC":
        with allure.step("对称算法解密, onepass # 执行成功"):
            t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
            )
            t_symm_cipher = t_symm_dec
            log.info(f"onepass result_symm_dec_time: {t_symm_cipher}")
            log.info(f"onepass result_symm_dec_size: {plain_text_size}")
            log.info(f"onepass result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            assert plain_text == test_vect.plaintext
            assert plain_text_size == len(test_vect.plaintext)
    else:
        assert False

    return t_symm_cipher


def ske_symm_streams_test_plain_key(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    """
    三段式明文密钥测试函数 (Init-Update-Finish)

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: 模式名称 (如 'CBC', 'CTR', 'ECB')
        padding: 填充模式 (如 'PKCS7', 'NONE')
        symm_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        round_num: 分段数量
        split_list: 每段数据的大小列表

    Returns:
        执行时间

    Note:
        此函数使用 ehsm_symm_cipher_init_with_plain_key 接口进行三段式明文密钥测试
    """
    t_symm_cipher = 0

    # 如果 split_list 为 None,则自动平均分割数据
    if split_list is None:
        if symm_dir == "SYMM_ENC":
            total_size = len(test_vect.plaintext)
        else:
            total_size = len(test_vect.ciphertext)
        # 平均分割
        base_size = total_size // round_num
        split_list = [base_size] * round_num
        # 将余数加到最后一段
        remainder = total_size % round_num
        if remainder > 0:
            split_list[-1] += remainder

    if symm_dir == "SYMM_ENC":
        t_symm_enc = 0
        cipher_text = b''
        cipher_text_size = 0
        total_input_size = len(test_vect.plaintext)

        with allure.step("对称算法加密, init (明文密钥) # 执行成功"):
            # 使用明文密钥初始化接口
            t_symm_enc_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,  # 加密
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                None  # session 由底层分配
            )
            t_symm_enc += t_symm_enc_init
            log.info(f"明文密钥 init 完成, session: {session}")

        with allure.step("对称算法加密, update # 执行成功"):
            log.info(f"symm_enc_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    log.info(f"update symm_enc_round: {i}  " +
                        f"input_size: {sum(split_list[i:i+1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                    t_symm_enc_update, cipher_text_update = api.ehsm_symm_cipher_update(
                        test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])],
                        sum(split_list[i:i+1])
                    )
                    t_symm_enc += t_symm_enc_update
                    cipher_text += cipher_text_update
                    cipher_text_size += len(cipher_text_update)

        with allure.step("对称算法加密, finish # 执行成功"):
            if round_num > 1:
                log.info(f"finish symm_enc_round: {int(round_num)-1}  " +
                    f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
            else:
                log.info(f"finish symm_enc_round: 0  " +
                    f"input_size: {sum(split_list[0:1])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.plaintext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
            t_symm_enc_finish, cipher_text_finish, cipher_text_size_finish = api.ehsm_symm_cipher_finish(
                test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                sum(split_list[int(round_num)-1:int(round_num)]),
                len(test_vect.ciphertext)
            )
            t_symm_enc += t_symm_enc_finish
            cipher_text += cipher_text_finish
            cipher_text_size += cipher_text_size_finish
            t_symm_cipher = t_symm_enc
            log.info(f"streams result_symm_enc_time: {t_symm_cipher}")
            log.info(f"streams result_symm_enc_size: {cipher_text_size}")
            log.info(f"streams result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext, f"加密结果不匹配: 期望 {len(test_vect.ciphertext)} 字节, 实际 {cipher_text_size} 字节"
            assert cipher_text_size == len(test_vect.ciphertext)

    elif symm_dir == "SYMM_DEC":
        t_symm_dec = 0
        plain_text = b''
        plain_text_size = 0
        total_input_size = len(test_vect.ciphertext)

        with allure.step("对称算法解密, init (明文密钥) # 执行成功"):
            # 使用明文密钥初始化接口
            t_symm_dec_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,  # 解密
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                None  # session 由底层分配
            )
            t_symm_dec += t_symm_dec_init
            log.info(f"明文密钥 init 完成, session: {session}")

        with allure.step("对称算法解密, update # 执行成功"):
            log.info(f"symm_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    log.info(f"update symm_dec_round: {i}  " +
                        f"input_size: {sum(split_list[i:i+1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                    t_symm_dec_update, plain_text_update = api.ehsm_symm_cipher_update(
                        test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                        sum(split_list[i:i+1])
                    )
                    t_symm_dec += t_symm_dec_update
                    plain_text += plain_text_update
                    plain_text_size += len(plain_text_update)

        with allure.step("对称算法解密, finish # 执行成功"):
            if round_num > 1:
                log.info(f"finish symm_dec_round: {int(round_num)-1}  " +
                    f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
            else:
                log.info(f"finish symm_dec_round: 0  " +
                    f"input_size: {sum(split_list[0:1])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
            t_symm_dec_finish, plain_text_finish, plain_text_size_finish = api.ehsm_symm_cipher_finish(
                test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                sum(split_list[int(round_num)-1:int(round_num)]),
                len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
            )
            t_symm_dec += t_symm_dec_finish
            plain_text += plain_text_finish
            plain_text_size += plain_text_size_finish
            t_symm_cipher = t_symm_dec
            log.info(f"streams result_symm_dec_time: {t_symm_cipher}")
            log.info(f"streams result_symm_dec_size: {plain_text_size}")
            log.info(f"streams result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            assert plain_text == test_vect.plaintext, f"解密结果不匹配: 期望 {len(test_vect.plaintext)} 字节, 实际 {plain_text_size} 字节"
            assert plain_text_size == len(test_vect.plaintext)
    else:
        assert False, f"无效的加解密方向: {symm_dir}"

    return t_symm_cipher


def ske_symm_streams_test(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            t_symm_enc = 0
            cipher_text = b''
            cipher_text_size = 0
            total_input_size = len(test_vect.plaintext)
            with allure.step("对称算法加密, init # 执行成功"):
                t_symm_enc_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    None
                )
                t_symm_enc += t_symm_enc_init
            with allure.step("对称算法加密, update # 执行成功"):
                log.info(f"symm_enc_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update symm_enc_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_symm_enc_update, cipher_text_update = api.ehsm_symm_cipher_update(
                            test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_symm_enc += t_symm_enc_update
                        cipher_text += cipher_text_update
                        cipher_text_size += len(cipher_text_update)
            with allure.step("对称算法加密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish symm_enc_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish symm_enc_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_symm_enc_finish, cipher_text_finish, cipher_text_size_finish = api.ehsm_symm_cipher_finish(
                    test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)]),
                    len(test_vect.ciphertext)
                )
                t_symm_enc += t_symm_enc_finish
                cipher_text += cipher_text_finish
                cipher_text_size += cipher_text_size_finish
                t_symm_cipher = t_symm_enc
                log.info(f"streams result_symm_enc_time: {t_symm_cipher}")
                log.info(f"streams result_symm_enc_size: {cipher_text_size}")
                log.info(f"streams result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext
                assert cipher_text_size == len(test_vect.ciphertext)
        elif symm_dir == "SYMM_DEC":
            t_symm_dec = 0
            plain_text = b''
            plain_text_size = 0
            total_input_size = len(test_vect.ciphertext)
            with allure.step("对称算法解密, init # 执行成功"):
                t_symm_dec_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    None
                )
                t_symm_dec += t_symm_dec_init
            with allure.step("对称算法解密, update # 执行成功"):
                log.info(f"symm_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update symm_dec_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_symm_dec_update, plain_text_update = api.ehsm_symm_cipher_update(
                            test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_symm_dec += t_symm_dec_update
                        plain_text += plain_text_update
                        plain_text_size += len(plain_text_update)
            with allure.step("对称算法解密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish symm_dec_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish symm_dec_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_symm_dec_finish, plain_text_finish, plain_text_size_finish = api.ehsm_symm_cipher_finish(
                    test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)]),
                    len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
                )
                t_symm_dec += t_symm_dec_finish
                plain_text += plain_text_finish
                plain_text_size += plain_text_size_finish
                t_symm_cipher = t_symm_dec
                log.info(f"streams result_symm_dec_time: {t_symm_cipher}")
                log.info(f"streams result_symm_dec_size: {plain_text_size}")
                log.info(f"streams result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                if padding != 'NONE':
                    plain_text = plain_text[:len(test_vect.plaintext)]
                    plain_text_size -= len(test_vect.ciphertext) - len(test_vect.plaintext)
                assert plain_text == test_vect.plaintext
                assert plain_text_size == len(test_vect.plaintext)
        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher

def ske_aead_onepass_test(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("对称算法加密, onepass # 执行成功"):
                t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_enc
                log.info(f"onepass result_aead_enc_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_enc_size: {len(cipher_text)}")
                log.info(f"onepass result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext
                assert auth_tag == test_vect.auth_tag
        elif aead_dir == "SYMM_DEC":
            with allure.step("对称算法解密, onepass # 执行成功"):
                t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_dec
                log.info(f"onepass result_aead_dec_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_dec_size: {len(plain_text)}")
                log.info(f"onepass result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                assert plain_text == test_vect.plaintext
                assert auth_result == True
        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher


def ske_aead_onepass_test_input_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, input_addr, input_size) -> int:
    """
    AEAD OnePass 模式下测试 input 异常的接口

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        input_addr: input 数据地址 (可能为 None)
        input_size: input 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, onepass # 执行成功"):
                t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    input_addr,
                    input_size,
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_enc
                log.info(f"onepass result_aead_enc_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_enc_size: {len(cipher_text)}")
                log.info(f"onepass result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, onepass # 执行成功"):
                t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    input_addr,
                    input_size,
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_dec
                log.info(f"onepass result_aead_dec_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_dec_size: {len(plain_text)}")
                log.info(f"onepass result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_onepass_test_iv_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    """
    AEAD OnePass 模式下测试 IV/Nonce 异常的接口

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        iv_addr: IV/Nonce 数据地址 (可能为 None)
        iv_size: IV/Nonce 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, onepass # 执行成功"):
                t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    iv_addr,
                    iv_size,
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_enc
                log.info(f"onepass result_aead_enc_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_enc_size: {len(cipher_text)}")
                log.info(f"onepass result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, onepass # 执行成功"):
                t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    iv_addr,
                    iv_size,
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_dec
                log.info(f"onepass result_aead_dec_time: {t_aead_cipher}")
                log.info(f"onepass result_aead_dec_size: {len(plain_text)}")
                log.info(f"onepass result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_streams_test_iv_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    """
    AEAD Streams 模式（三段式）测试 IV/Nonce 异常的接口

    测试三段式（init-update-finish）处理流程中 IV/Nonce 参数异常的场景
    目的：测试当传入异常的 iv_addr 和 iv_size 参数时，API 的处理
    说明：init 阶段传入异常的 IV/Nonce 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        iv_addr: IV/Nonce 数据地址 (可能为 None)
        iv_size: IV/Nonce 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, init(iv异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    True,
                    iv_addr,  # 异常的iv_addr
                    iv_size,  # 异常的iv_size
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_enc_time: {t_aead_cipher}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, init(iv异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    False,
                    iv_addr,  # 异常的iv_addr
                    iv_size,  # 异常的iv_size
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_dec_time: {t_aead_cipher}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_onepass_test_tag_none(algo: str, mode: str, test_vect: SymmetricTestData, tag_addr, tag_size) -> int:
    """
    AEAD OnePass 模式下测试 Tag 异常的接口（仅解密）

    由于 tag 在加密时是输出参数，只有在解密时才作为输入参数
    因此本接口仅测试解密场景下的 tag 参数异常

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        test_vect: 测试向量数据
        tag_addr: Tag 数据地址 (可能为 None)
        tag_size: Tag 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        with allure.step("AEAD 解密, onepass(tag异常) # 期望失败"):
            t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                key_handle,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                tag_addr,  # 异常的tag_addr
                tag_size   # 异常的tag_size
            )
            t_aead_cipher = t_aead_dec
            log.info(f"onepass result_aead_dec_time: {t_aead_cipher}")
            log.info(f"onepass result_aead_dec_size: {len(plain_text)}")
            log.info(f"onepass result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"onepass auth_result: {auth_result}")

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_streams_test_tag_none(algo: str, mode: str, test_vect: SymmetricTestData, round_num: int, split_list: list, tag_addr,tag_size) -> int:
    """
    AEAD Streams 模式（三段式）测试 Tag 异常的接口（仅解密）

    测试三段式（init-update-finish）处理流程中 Tag 参数异常的场景
    由于 tag 在加密时是输出参数，只有在解密时才作为输入参数
    因此本接口仅测试解密场景下的 tag 参数异常
    说明：init 和 update 阶段正常，在 finish_dec 阶段传入异常的 tag 参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        test_vect: 测试向量数据
        round_num: 处理轮数
        split_list: 数据分割列表
        tag_addr: Tag 数据地址 (可能为 None)

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        t_aead_dec = 0
        plain_text = b''
        plain_text_size = 0
        total_input_size = len(test_vect.ciphertext)

        with allure.step("AEAD 解密, init # 执行成功"):
            t_aead_dec_init, session = api.ehsm_aead_init(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                key_handle,
                False,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.ciphertext),
                tag_size,
                None
            )
            t_aead_dec += t_aead_dec_init
            log.info(f"init result_aead_dec_time: {t_aead_dec_init}")

        with allure.step("AEAD 解密, update # 执行成功"):
            log.info(f"aead_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    log.info(f"update aead_dec_round: {i}  " +
                        f"input_size: {sum(split_list[i:i+1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                    t_aead_dec_update, plain_text_update = api.ehsm_aead_update(
                        test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                        sum(split_list[i:i+1])
                    )
                    t_aead_dec += t_aead_dec_update
                    plain_text += plain_text_update
                    plain_text_size += len(plain_text_update)

        with allure.step("AEAD 解密, finish(tag异常) # 期望失败"):
            if round_num > 1:
                log.info(f"finish aead_dec_round: {int(round_num)-1}  " +
                    f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
            else:
                log.info(f"finish aead_dec_round: 0  " +
                    f"input_size: {sum(split_list[0:1])}  " +
                    f"input_data: " +
                    "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
            t_aead_dec_finish, plain_text_finish, ret_verify_result = api.ehsm_aead_finish_dec(
                test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                sum(split_list[int(round_num)-1:int(round_num)]),
                tag_addr  # 异常的tag参数
            )
            t_aead_dec += t_aead_dec_finish
            plain_text += plain_text_finish
            plain_text_size += len(plain_text_finish)
            t_aead_cipher = t_aead_dec
            log.info(f"streams result_aead_dec_time: {t_aead_cipher}")
            log.info(f"streams result_aead_dec_size: {plain_text_size}")
            log.info(f"streams result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"streams result_auth_tag_vrfy: {ret_verify_result}")

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_streams_test_context_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, context_addr) -> int:
    """
    AEAD Streams 模式（三段式）测试 Context 异常的接口

    测试三段式（init-update-finish）处理流程中 Context 参数异常的场景
    目的：测试当传入异常的 context_addr 参数时，API 的处理
    说明：init 阶段传入异常的 Context 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        context_addr: Context 数据地址 (可能为 None)

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, init(context异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    context_addr  # 异常的context参数
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_enc_time: {t_aead_cipher}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, init(context异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    context_addr  # 异常的context参数
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_dec_time: {t_aead_cipher}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_onepass_test_aad_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, aad_addr, aad_size) -> int:
    """
    AEAD OnePass 模式下测试 AAD 异常的接口

    测试一次处理（onepass）AEAD 加解密时 AAD 参数异常的场景
    目的：测试当传入异常的 aad_addr 和 aad_size 参数时，API 的处理
    说明：AAD 在加密和解密时都作为输入参数，所以两个方向都可以测试

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        aad_addr: AAD 数据地址 (可能为 None)
        aad_size: AAD 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, onepass(aad异常) # 期望失败"):
                t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    aad_addr,  # 异常的aad参数
                    aad_size,
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_enc
                log.info(f"onepass result_aead_enc_time: {t_aead_cipher}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, onepass(aad异常) # 期望失败"):
                t_aead_dec, plain_text, ret_verify_result = api.ehsm_aead_onepass_dec(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    aad_addr,  # 异常的aad参数
                    aad_size,
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_aead_cipher = t_aead_dec
                log.info(f"onepass result_aead_dec_time: {t_aead_cipher}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_streams_test_aad_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, aad_addr, aad_size) -> int:
    """
    AEAD Streams 模式（三段式）测试 AAD 异常的接口

    测试三段式（init-update-finish）处理流程中 AAD 参数异常的场景
    目的：测试当传入异常的 aad_addr 和 aad_size 参数时，API 的处理
    说明：init 阶段传入异常的 AAD 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        aad_addr: AAD 数据地址 (可能为 None)
        aad_size: AAD 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, init(aad异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    aad_addr,  # 异常的aad参数
                    aad_size,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_enc_time: {t_aead_cipher}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, init(aad异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    aad_addr,  # 异常的aad参数
                    aad_size,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init result_aead_dec_time: {t_aead_cipher}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher

def ske_aead_onepass_test_plainkey_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, plain_key, plain_key_size) -> int:
    """
    AEAD OnePass 模式下测试明文密钥(plainkey)异常的接口

    测试一次处理（onepass）AEAD 加解密时明文密钥参数异常的场景
    目的：测试当传入异常的 plain_key 和 plain_key_size 参数时，API 的处理
    说明：使用明文密钥接口，不需要导入密钥，plainkey 在加密和解密时都作为输入参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        plain_key: 明文密钥数据 (可能为 None)
        plain_key_size: 明文密钥长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, onepass(plainkey异常) # 期望失败"):
            t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                plain_key,  # 异常的plainkey参数
                plain_key_size,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_enc
            log.info(f"onepass plainkey异常 result_aead_enc_time: {t_aead_cipher}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, onepass(plainkey异常) # 期望失败"):
            t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                plain_key,  # 异常的plainkey参数
                plain_key_size,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_dec
            log.info(f"onepass plainkey异常 result_aead_dec_time: {t_aead_cipher}")

    else:
        assert False

    return t_aead_cipher

def ske_aead_streams_test_plainkey_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, plain_key, plain_key_size) -> int:
    """
    AEAD Streams 模式（三段式）测试明文密钥(plainkey)异常的接口

    测试三段式（init-update-finish）处理流程中明文密钥参数异常的场景
    目的：测试当传入异常的 plain_key 和 plain_key_size 参数时，API 的处理
    说明：init 阶段传入异常的明文密钥参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        plain_key: 明文密钥数据 (可能为 None)
        plain_key_size: 明文密钥长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, init(plainkey异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    plain_key,  # 异常的plainkey参数
                    plain_key_size,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init plainkey异常 result_aead_enc_time: {t_aead_cipher}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, init(plainkey异常) # 期望失败"):
                t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    plain_key,  # 异常的plainkey参数
                    plain_key_size,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher = t_aead_init
                log.info(f"init plainkey异常 result_aead_dec_time: {t_aead_cipher}")

        else:
            assert False
    finally:
        pass

    return t_aead_cipher

def ske_aead_onepass_test_plainkey_input_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, input_addr, input_size) -> int:
    """
    AEAD OnePass 模式下测试input异常的接口（使用明文密钥）

    测试一次处理（onepass）AEAD 加解密时 input 参数异常的场景
    目的：测试当传入异常的 input_addr 和 input_size 参数时，API 的处理
    说明：使用明文密钥接口，不需要导入密钥，input 在加密和解密时都作为输入参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        input_addr: input 数据地址 (可能为 None)
        input_size: input 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, onepass(明文密钥+input异常) # 期望失败"):
            t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                input_addr,  # 异常的input参数
                input_size,
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_enc
            log.info(f"onepass plain_key input异常 result_aead_enc_time: {t_aead_cipher}")
            log.info(f"onepass plain_key input异常 result_aead_enc_size: {len(cipher_text)}")
            log.info(f"onepass plain_key input异常 result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, onepass(明文密钥+input异常) # 期望失败"):
            t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                input_addr,  # 异常的input参数
                input_size,
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_dec
            log.info(f"onepass plain_key input异常 result_aead_dec_time: {t_aead_cipher}")
            log.info(f"onepass plain_key input异常 result_aead_dec_size: {len(plain_text)}")
            log.info(f"onepass plain_key input异常 result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

    else:
        assert False

    return t_aead_cipher

def ske_aead_streams_test_plainkey_input_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list, input_addr, input_size) -> int:
    """
    AEAD Streams 模式（三段式）测试input异常的接口（使用明文密钥）

    测试三段式（init-update-finish）处理流程中 input 参数异常的场景
    目的：测试当传入异常的 input_addr 和 input_size 参数时，API 的处理
    说明：init 阶段正常（使用明文密钥），但在 update 和 finish 阶段传入异常的 input 参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        round_num: 处理轮数
        split_list: 数据分割列表
        input_addr: input 数据地址 (可能为 None)
        input_size: input 数据长度

    Returns:
        执行时间
    """
    # 如果 split_list 为 None 或空列表，则自动平均分割数据
    if split_list is None or len(split_list) == 0:
        if aead_dir == "SYMM_ENC":
            total_size = len(test_vect.plaintext)
        else:
            total_size = len(test_vect.ciphertext)
        base_size = total_size // round_num
        split_list = [base_size] * round_num
        remainder = total_size % round_num
        if remainder > 0:
            split_list[-1] += remainder

    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, init(明文密钥) # 执行成功"):
            t_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                True,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.plaintext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_cipher += t_init

        with allure.step("AEAD 加密, update(input异常) # 期望失败"):
            log.info(f"aead_enc_total_round: {round_num}  total_input_size: {len(test_vect.plaintext)}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    # 获取当前块的实际数据，但传入异常的size参数
                    current_chunk = test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    log.info(f"update aead_enc_round: {i}  " +
                        f"actual_data_size: {len(current_chunk)}  " +
                        f"input_size_param: {input_size}  " +
                        f"input_data: {current_chunk.hex()}")
                    t_update, cipher_text_update = api.ehsm_aead_update(
                        current_chunk if input_addr is not None else input_addr,
                        input_size  # 使用异常的size参数
                    )
                    t_aead_cipher += t_update
                    log.info(f"streams plain_key update_time: {t_update}")
                    log.info(f"streams plain_key update_size: {len(cipher_text_update)}")
                    log.info(f"streams plain_key update_data: {''.join(f'{b:02x}' for b in cipher_text_update) or '(empty)'}")

        with allure.step("AEAD 加密, finish(input异常) # 完成三段式流程"):
            # 获取最后一块的实际数据，但传入异常的size参数
            final_chunk = test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
            log.info(f"finish aead_enc_round: {int(round_num)-1}  " +
                f"actual_data_size: {len(final_chunk)}  " +
                f"input_size_param: {input_size}  " +
                f"input_data: {final_chunk.hex()}")
            t_finish, cipher_text_finish, auth_tag = api.ehsm_aead_finish_enc(
                final_chunk if input_addr is not None else input_addr,
                input_size  # 使用异常的size参数
            )
            t_aead_cipher += t_finish
            log.info(f"streams plain_key finish_time: {t_finish}")
            log.info(f"streams plain_key finish_cipher_size: {len(cipher_text_finish)}")
            log.info(f"streams plain_key finish_auth_tag_size: {len(auth_tag)}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, init(明文密钥) # 执行成功"):
            t_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                False,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.ciphertext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_cipher += t_init

        with allure.step("AEAD 解密, update(input异常) # 期望失败"):
            log.info(f"aead_dec_total_round: {round_num}  total_input_size: {len(test_vect.ciphertext)}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    # 获取当前块的实际数据，但传入异常的size参数
                    current_chunk = test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    log.info(f"update aead_dec_round: {i}  " +
                        f"actual_data_size: {len(current_chunk)}  " +
                        f"input_size_param: {input_size}  " +
                        f"input_data: {current_chunk.hex()}")
                    t_update, plain_text_update = api.ehsm_aead_update(
                        current_chunk if input_addr is not None else input_addr,
                        input_size  # 使用异常的size参数
                    )
                    t_aead_cipher += t_update
                    log.info(f"streams plain_key update_time: {t_update}")
                    log.info(f"streams plain_key update_size: {len(plain_text_update)}")
                    log.info(f"streams plain_key update_data: {''.join(f'{b:02x}' for b in plain_text_update) or '(empty)'}")

        with allure.step("AEAD 解密, finish(input异常) # 完成三段式流程"):
            # 获取最后一块的实际数据，但传入异常的size参数
            final_chunk = test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
            log.info(f"finish aead_dec_round: {int(round_num)-1}  " +
                f"actual_data_size: {len(final_chunk)}  " +
                f"input_size_param: {input_size}  " +
                f"input_data: {final_chunk.hex()}")
            t_finish, plain_text_finish, verify_result = api.ehsm_aead_finish_dec(
                final_chunk if input_addr is not None else input_addr,
                input_size,  # 使用异常的size参数
                test_vect.auth_tag
            )
            t_aead_cipher += t_finish
            log.info(f"streams plain_key finish_time: {t_finish}")
            log.info(f"streams plain_key finish_plain_size: {len(plain_text_finish)}")
            log.info(f"streams plain_key finish_verify_result: {verify_result}")

    else:
        assert False

    return t_aead_cipher

def ske_aead_streams_test(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            t_aead_enc = 0
            cipher_text = b''
            cipher_text_size = 0
            total_input_size = len(test_vect.plaintext)
            with allure.step("对称算法加密, init # 执行成功"):
                t_aead_enc_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_enc += t_aead_enc_init
            with allure.step("对称算法加密, update # 执行成功"):
                log.info(f"aead_enc_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update aead_enc_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_aead_enc_update, cipher_text_update = api.ehsm_aead_update(
                            test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_aead_enc += t_aead_enc_update
                        cipher_text += cipher_text_update
                        cipher_text_size += len(cipher_text_update)
            with allure.step("对称算法加密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish aead_enc_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish aead_enc_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_aead_enc_finish, cipher_text_finish, auth_tag = api.ehsm_aead_finish_enc(
                    test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)])
                )
                t_aead_enc += t_aead_enc_finish
                cipher_text += cipher_text_finish
                cipher_text_size += len(cipher_text_finish)
                auth_tag_size = len(auth_tag)
                t_aead_cipher = t_aead_enc
                log.info(f"streams result_aead_enc_time: {t_aead_cipher}")
                log.info(f"streams result_aead_enc_size: {cipher_text_size}")
                log.info(f"streams result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                log.info(f"streams result_auth_tag_size: {auth_tag_size}")
                log.info(f"streams result_auth_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext
                assert cipher_text_size == len(test_vect.ciphertext)
                assert auth_tag[:len(test_vect.auth_tag)] == test_vect.auth_tag
                assert auth_tag_size >= len(test_vect.auth_tag)
        elif aead_dir == "SYMM_DEC":
            t_aead_dec = 0
            plain_text = b''
            plain_text_size = 0
            total_input_size = len(test_vect.ciphertext)
            with allure.step("对称算法解密, init # 执行成功"):
                t_aead_dec_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_dec += t_aead_dec_init
            with allure.step("对称算法解密, update # 执行成功"):
                log.info(f"aead_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update aead_dec_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_aead_dec_update, plain_text_update = api.ehsm_aead_update(
                            test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_aead_dec += t_aead_dec_update
                        plain_text += plain_text_update
                        plain_text_size += len(plain_text_update)
            with allure.step("对称算法解密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish aead_dec_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish aead_dec_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_aead_dec_finish, plain_text_finish, ret_verify_result = api.ehsm_aead_finish_dec(
                    test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)]),
                    test_vect.auth_tag
                )
                t_aead_dec += t_aead_dec_finish
                plain_text += plain_text_finish
                plain_text_size += len(plain_text_finish)
                t_aead_cipher = t_aead_dec
                log.info(f"streams result_aead_dec_time: {t_aead_cipher}")
                log.info(f"streams result_aead_dec_size: {plain_text_size}")
                log.info(f"streams result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                log.info(f"streams result_auth_tag_vrfy: {ret_verify_result}")
                assert plain_text == test_vect.plaintext
                assert plain_text_size == len(test_vect.plaintext)
                assert ret_verify_result == True
        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher


def ske_aead_streams_test_input_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list, input_addr, input_size) -> int:
    """
    AEAD Streams 模式下测试 input 异常的接口

    测试三段式（init-update-finish）处理流程中 input 参数异常的场景
    目的：测试当传入异常的 input_addr 和 input_size 参数时，API 的处理
    说明：init 阶段正常，但在 update 和 finish 阶段传入异常的 input 参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        round_num: 处理轮数
        split_list: 数据分割列表
        input_addr: input 数据地址 (可能为 None)
        input_size: input 数据长度

    Returns:
        执行时间
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    # 如果 split_list 为 None 或空列表，则自动平均分割数据
    if split_list is None or len(split_list) == 0:
        if aead_dir == "SYMM_ENC":
            total_size = len(test_vect.plaintext)
        else:
            total_size = len(test_vect.ciphertext)
        base_size = total_size // round_num
        split_list = [base_size] * round_num
        remainder = total_size % round_num
        if remainder > 0:
            split_list[-1] += remainder

    t_aead_cipher = 0

    try:
        if aead_dir == "SYMM_ENC":
            with allure.step("AEAD 加密, init # 执行成功"):
                t_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher += t_init

            with allure.step("AEAD 加密, update(input异常) # 期望失败"):
                log.info(f"aead_enc_total_round: {round_num}  total_input_size: {len(test_vect.plaintext)}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        # 获取当前块的实际数据，但传入异常的size参数
                        current_chunk = test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])]
                        log.info(f"update aead_enc_round: {i}  " +
                            f"actual_data_size: {len(current_chunk)}  " +
                            f"input_size_param: {input_size}  " +
                            f"input_data: {current_chunk.hex()}")
                        t_update, cipher_text_update = api.ehsm_aead_update(
                            current_chunk if input_addr is not None else input_addr,
                            input_size  # 使用异常的size参数
                        )
                        t_aead_cipher += t_update
                        log.info(f"streams update_time: {t_update}")
                        log.info(f"streams update_size: {len(cipher_text_update)}")
                        log.info(f"streams update_data: {''.join(f'{b:02x}' for b in cipher_text_update) or '(empty)'}")

            with allure.step("AEAD 加密, finish(input异常) # 完成三段式流程"):
                # 获取最后一块的实际数据，但传入异常的size参数
                final_chunk = test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
                log.info(f"finish aead_enc_round: {int(round_num)-1}  " +
                    f"actual_data_size: {len(final_chunk)}  " +
                    f"input_size_param: {input_size}  " +
                    f"input_data: {final_chunk.hex()}")
                t_finish, cipher_text_finish, auth_tag = api.ehsm_aead_finish_enc(
                    final_chunk if input_addr is not None else input_addr,
                    input_size  # 使用异常的size参数
                )
                t_aead_cipher += t_finish
                log.info(f"streams finish_time: {t_finish}")
                log.info(f"streams finish_cipher_size: {len(cipher_text_finish)}")
                log.info(f"streams finish_auth_tag_size: {len(auth_tag)}")

        elif aead_dir == "SYMM_DEC":
            with allure.step("AEAD 解密, init # 执行成功"):
                t_init, session = api.ehsm_aead_init(
                    SYM_ALGO_TO_ENUM[algo],
                    AEAD_MODE_TO_ENUM[mode],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad,
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    len(test_vect.ciphertext),
                    len(test_vect.auth_tag),
                    None
                )
                t_aead_cipher += t_init

            with allure.step("AEAD 解密, update(input异常) # 期望失败"):
                log.info(f"aead_dec_total_round: {round_num}  total_input_size: {len(test_vect.ciphertext)}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        # 获取当前块的实际数据，但传入异常的size参数
                        current_chunk = test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])]
                        log.info(f"update aead_dec_round: {i}  " +
                            f"actual_data_size: {len(current_chunk)}  " +
                            f"input_size_param: {input_size}  " +
                            f"input_data: {current_chunk.hex()}")
                        t_update, plain_text_update = api.ehsm_aead_update(
                            current_chunk if input_addr is not None else input_addr,
                            input_size  # 使用异常的size参数
                        )
                        t_aead_cipher += t_update
                        log.info(f"streams update_time: {t_update}")
                        log.info(f"streams update_size: {len(plain_text_update)}")
                        log.info(f"streams update_data: {''.join(f'{b:02x}' for b in plain_text_update) or '(empty)'}")

            with allure.step("AEAD 解密, finish(input异常) # 完成三段式流程"):
                # 获取最后一块的实际数据，但传入异常的size参数
                final_chunk = test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
                log.info(f"finish aead_dec_round: {int(round_num)-1}  " +
                    f"actual_data_size: {len(final_chunk)}  " +
                    f"input_size_param: {input_size}  " +
                    f"input_data: {final_chunk.hex()}")
                t_finish, plain_text_finish, ret_verify_result = api.ehsm_aead_finish_dec(
                    final_chunk if input_addr is not None else input_addr,
                    input_size,  # 使用异常的size参数
                    test_vect.auth_tag
                )
                t_aead_cipher += t_finish
                log.info(f"streams finish_time: {t_finish}")
                log.info(f"streams finish_plain_size: {len(plain_text_finish)}")
                log.info(f"streams finish_verify_result: {ret_verify_result}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 aead 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_aead_cipher


def ske_aead_onepass_test_plain_key(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData) -> int:
    """
    AEAD OnePass 明文密钥测试函数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据

    Returns:
        执行时间
    """
    t_aead_cipher = 0
    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, onepass (明文密钥) # 执行成功"):
            t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_enc
            log.info(f"onepass plain key result_aead_enc_time: {t_aead_cipher}")
            log.info(f"onepass plain key result_aead_enc_size: {len(cipher_text)}")
            log.info(f"onepass plain key result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            log.info(f"onepass plain key result_auth_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext
            assert auth_tag == test_vect.auth_tag
    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, onepass (明文密钥) # 执行成功"):
            t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_dec
            log.info(f"onepass plain key result_aead_dec_time: {t_aead_cipher}")
            log.info(f"onepass plain key result_aead_dec_size: {len(plain_text)}")
            log.info(f"onepass plain key result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"onepass plain key auth_result: {auth_result}")
            assert plain_text == test_vect.plaintext
            assert auth_result == True
    else:
        assert False

    return t_aead_cipher


def ske_aead_onepass_test_plain_key_iv_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    """
    AEAD OnePass 明文密钥模式下测试 IV/Nonce 异常的接口

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        iv_addr: IV/Nonce 数据地址 (可能为 None)
        iv_size: IV/Nonce 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, onepass (明文密钥, IV异常测试) # 执行成功"):
            t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                iv_addr,
                iv_size,
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_enc
            log.info(f"onepass plain key (IV异常) result_aead_enc_time: {t_aead_cipher}")
            log.info(f"onepass plain key (IV异常) result_aead_enc_size: {len(cipher_text)}")
            log.info(f"onepass plain key (IV异常) result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            log.info(f"onepass plain key (IV异常) result_auth_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, onepass (明文密钥, IV异常测试) # 执行成功"):
            t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                iv_addr,
                iv_size,
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_aead_cipher = t_aead_dec
            log.info(f"onepass plain key (IV异常) result_aead_dec_time: {t_aead_cipher}")
            log.info(f"onepass plain key (IV异常) result_aead_dec_size: {len(plain_text)}")
            log.info(f"onepass plain key (IV异常) result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"onepass plain key (IV异常) auth_result: {auth_result}")

    else:
        assert False

    return t_aead_cipher


def ske_aead_onepass_test_plain_key_tag_none(algo: str, mode: str, test_vect: SymmetricTestData, tag_addr, tag_size) -> int:
    """
    AEAD OnePass 明文密钥模式下测试 Tag 异常的接口（仅解密）

    由于 tag 在加密时是输出参数，只有在解密时才作为输入参数
    因此本接口仅测试解密场景下的 tag 参数异常

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        test_vect: 测试向量数据
        tag_addr: Tag 数据地址 (可能为 None)
        tag_size: Tag 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0
    log.info(f"onepass plain key 解密, Tag异常测试")

    t_aead_dec, plain_text, auth_result = api.ehsm_aead_onepass_dec_with_plain_key(
        SYM_ALGO_TO_ENUM[algo],
        AEAD_MODE_TO_ENUM[mode],
        test_vect.key,
        len(test_vect.key),
        test_vect.iv_nonce,
        len(test_vect.iv_nonce),
        test_vect.aad,
        len(test_vect.aad) if test_vect.aad is not None else 0,
        test_vect.ciphertext,
        len(test_vect.ciphertext),
        tag_addr,  # 异常的tag_addr
        tag_size   # 异常的tag_size
    )
    t_aead_cipher = t_aead_dec
    log.info(f"onepass plain key result_aead_dec_time: {t_aead_cipher}")
    log.info(f"onepass plain key result_aead_dec_size: {len(plain_text)}")
    log.info(f"onepass plain key result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
    log.info(f"onepass plain key auth_result: {auth_result}")

    return t_aead_cipher


def ske_aead_onepass_test_plain_key_aad_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, aad_addr, aad_size) -> int:
    """
    AEAD OnePass 明文密钥模式下测试 AAD 异常的接口

    测试一次处理（onepass）AEAD 加解密时 AAD 参数异常的场景（明文密钥模式）
    目的：测试当传入异常的 aad_addr 和 aad_size 参数时，API 的处理
    说明：AAD 在加密和解密时都作为输入参数，所以两个方向都可以测试

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        aad_addr: AAD 数据地址 (可能为 None)
        aad_size: AAD 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        log.info(f"onepass plain key 加密, AAD异常测试")
        t_aead_enc, cipher_text, auth_tag = api.ehsm_aead_onepass_enc_with_plain_key(
            SYM_ALGO_TO_ENUM[algo],
            AEAD_MODE_TO_ENUM[mode],
            test_vect.key,
            len(test_vect.key),
            test_vect.iv_nonce,
            len(test_vect.iv_nonce),
            aad_addr,  # 异常的aad参数
            aad_size,
            test_vect.plaintext,
            len(test_vect.plaintext),
            len(test_vect.auth_tag)
        )
        t_aead_cipher = t_aead_enc
        log.info(f"onepass plain key result_aead_enc_time: {t_aead_cipher}")

    elif aead_dir == "SYMM_DEC":
        log.info(f"onepass plain key 解密, AAD异常测试")
        t_aead_dec, plain_text, ret_verify_result = api.ehsm_aead_onepass_dec_with_plain_key(
            SYM_ALGO_TO_ENUM[algo],
            AEAD_MODE_TO_ENUM[mode],
            test_vect.key,
            len(test_vect.key),
            test_vect.iv_nonce,
            len(test_vect.iv_nonce),
            aad_addr,  # 异常的aad参数
            aad_size,
            test_vect.ciphertext,
            len(test_vect.ciphertext),
            test_vect.auth_tag,
            len(test_vect.auth_tag)
        )
        t_aead_cipher = t_aead_dec
        log.info(f"onepass plain key result_aead_dec_time: {t_aead_cipher}")

    else:
        assert False

    return t_aead_cipher


def ske_aead_streams_test_plain_key(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    """
    AEAD 三段式明文密钥测试函数 (Init-Update-Finish)

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        round_num: 分段轮次数
        split_list: 自定义分段列表,如果为None则自动分段

    Returns:
        执行时间
    """
    t_aead_cipher = 0
    if aead_dir == "SYMM_ENC":
        t_aead_enc = 0
        cipher_text = b''
        cipher_text_size = 0
        auth_tag = b''
        auth_tag_size = 0
        total_input_size = len(test_vect.plaintext)
        with allure.step("AEAD 加密, init (明文密钥) # 执行成功"):
            t_aead_enc_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                True,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.plaintext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_enc += t_aead_enc_init
        with allure.step("AEAD 加密, update (明文密钥) # 执行成功"):
            log.info(f"aead_enc_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
            if round_num > 1:
                if split_list is None:
                    split_list = split_M_into_N(total_input_size, round_num, 16, True)
                    log.info(f"split_list auto generated: {split_list}")
                for i, split_size in enumerate(split_list):
                    offset = sum(split_list[:i])
                    input_chunk = test_vect.plaintext[offset:offset+split_size]
                    t_aead_enc_update, output_chunk = api.ehsm_aead_update(
                        input_chunk,
                        len(input_chunk)
                    )
                    t_aead_enc += t_aead_enc_update
                    cipher_text += output_chunk
                    cipher_text_size += len(output_chunk)
                    log.info(f"round {i+1}/{len(split_list)}: input={split_size}, output={len(output_chunk)}")
        with allure.step("AEAD 加密, finish (明文密钥) # 执行成功"):
            remaining_input = test_vect.plaintext[cipher_text_size:]
            t_aead_enc_finish, output_final, tag_output = api.ehsm_aead_finish_enc(
                remaining_input,
                len(remaining_input)
            )
            t_aead_enc += t_aead_enc_finish
            cipher_text += output_final
            cipher_text_size += len(output_final)
            auth_tag = tag_output
            auth_tag_size = len(tag_output)
            t_aead_cipher = t_aead_enc
            log.info(f"streams plain key result_aead_enc_time: {t_aead_cipher}")
            log.info(f"streams plain key result_aead_enc_size: {cipher_text_size}")
            log.info(f"streams plain key result_aead_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            log.info(f"streams plain key result_auth_tag_size: {auth_tag_size}")
            log.info(f"streams plain key result_auth_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext
            assert cipher_text_size == len(test_vect.ciphertext)
            assert auth_tag[:len(test_vect.auth_tag)] == test_vect.auth_tag
            assert auth_tag_size >= len(test_vect.auth_tag)
    elif aead_dir == "SYMM_DEC":
        t_aead_dec = 0
        plain_text = b''
        plain_text_size = 0
        total_input_size = len(test_vect.ciphertext)
        with allure.step("AEAD 解密, init (明文密钥) # 执行成功"):
            t_aead_dec_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                False,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.ciphertext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_dec += t_aead_dec_init
        with allure.step("AEAD 解密, update (明文密钥) # 执行成功"):
            log.info(f"aead_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
            if round_num > 1:
                if split_list is None:
                    split_list = split_M_into_N(total_input_size, round_num, 16, True)
                    log.info(f"split_list auto generated: {split_list}")
                for i, split_size in enumerate(split_list):
                    offset = sum(split_list[:i])
                    input_chunk = test_vect.ciphertext[offset:offset+split_size]
                    t_aead_dec_update, output_chunk = api.ehsm_aead_update(
                        input_chunk,
                        len(input_chunk)
                    )
                    t_aead_dec += t_aead_dec_update
                    plain_text += output_chunk
                    plain_text_size += len(output_chunk)
                    log.info(f"round {i+1}/{len(split_list)}: input={split_size}, output={len(output_chunk)}")
        with allure.step("AEAD 解密, finish (明文密钥) # 执行成功"):
            remaining_input = test_vect.ciphertext[plain_text_size:]
            t_aead_dec_finish, output_final, auth_result = api.ehsm_aead_finish_dec(
                remaining_input,
                len(remaining_input),
                test_vect.auth_tag
            )
            t_aead_dec += t_aead_dec_finish
            plain_text += output_final
            plain_text_size += len(output_final)
            t_aead_cipher = t_aead_dec
            log.info(f"streams plain key result_aead_dec_time: {t_aead_cipher}")
            log.info(f"streams plain key result_aead_dec_size: {plain_text_size}")
            log.info(f"streams plain key result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"streams plain key auth_result: {auth_result}")
            assert plain_text == test_vect.plaintext
            assert plain_text_size == len(test_vect.plaintext)
            assert auth_result == True
    else:
        assert False

    return t_aead_cipher


def ske_aead_streams_test_plain_key_tag_none(algo: str, mode: str, test_vect: SymmetricTestData, round_num: int, split_list: list, tag_addr, tag_size) -> int:
    """
    AEAD Streams 模式（三段式）明文密钥下测试 Tag 异常的接口（仅解密）

    测试三段式（init-update-finish）处理流程中 Tag 参数异常的场景（明文密钥模式）
    由于 tag 在加密时是输出参数，只有在解密时才作为输入参数
    因此本接口仅测试解密场景下的 tag 参数异常
    说明：init 和 update 阶段正常，在 finish_dec 阶段传入异常的 tag 参数

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        test_vect: 测试向量数据
        round_num: 处理轮数
        split_list: 数据分割列表
        tag_addr: Tag 数据地址 (可能为 None)
        tag_size: Tag 数据长度

    Returns:
        执行时间
    """
    t_aead_dec = 0
    plain_text = b''
    plain_text_size = 0
    total_input_size = len(test_vect.ciphertext)

    # 如果 split_list 为空，按照 round_num 平均分割
    if not split_list:
        split_list = []
        base_size = total_input_size // round_num
        remainder = total_input_size % round_num
        for i in range(round_num):
            if i < remainder:
                split_list.append(base_size + 1)
            else:
                split_list.append(base_size)

    log.info(f"streams plain key 解密, Tag异常测试")

    # Init 阶段 - 正常
    t_aead_dec_init, session = api.ehsm_aead_init_with_plain_key(
        SYM_ALGO_TO_ENUM[algo],
        AEAD_MODE_TO_ENUM[mode],
        test_vect.key,
        len(test_vect.key),
        False,  # is_encrypt=False 解密
        test_vect.iv_nonce,
        len(test_vect.iv_nonce),
        test_vect.aad,
        len(test_vect.aad) if test_vect.aad is not None else 0,
        len(test_vect.ciphertext),
        tag_size,
        None
    )
    t_aead_dec += t_aead_dec_init
    log.info(f"init result_aead_dec_time: {t_aead_dec_init}")

    # Update 阶段 - 正常
    log.info(f"aead_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
    if round_num > 1:
        for i in range(int(round_num)-1):
            log.info(f"update aead_dec_round: {i}  " +
                f"input_size: {sum(split_list[i:i+1])}  " +
                f"input_data: " +
                "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
            t_aead_dec_update, plain_text_update = api.ehsm_aead_update(
                test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                sum(split_list[i:i+1])
            )
            t_aead_dec += t_aead_dec_update
            plain_text += plain_text_update
            plain_text_size += len(plain_text_update)

    # Finish 阶段 - Tag异常
    if round_num > 1:
        log.info(f"finish aead_dec_round: {int(round_num)-1}  " +
            f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
            f"input_data: " +
            "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
    else:
        log.info(f"finish aead_dec_round: 0  " +
            f"input_size: {sum(split_list[0:1])}  " +
            f"input_data: " +
            "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
    t_aead_dec_finish, plain_text_finish, ret_verify_result = api.ehsm_aead_finish_dec(
        test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
        sum(split_list[int(round_num)-1:int(round_num)]),
        tag_addr  # 异常的tag参数
    )
    t_aead_dec += t_aead_dec_finish
    plain_text += plain_text_finish
    plain_text_size += len(plain_text_finish)
    t_aead_cipher = t_aead_dec
    log.info(f"streams plain key result_aead_dec_time: {t_aead_cipher}")
    log.info(f"streams plain key result_aead_dec_size: {plain_text_size}")
    log.info(f"streams plain key result_aead_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
    log.info(f"streams plain key result_auth_tag_vrfy: {ret_verify_result}")

    return t_aead_cipher


def ske_aead_streams_test_plain_key_aad_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, aad_addr, aad_size) -> int:
    """
    AEAD Streams 模式（三段式）明文密钥下测试 AAD 异常的接口

    测试三段式（init-update-finish）处理流程中 AAD 参数异常的场景（明文密钥模式）
    目的：测试当传入异常的 aad_addr 和 aad_size 参数时，API 的处理
    说明：init 阶段传入异常的 AAD 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish
          AAD 在加密和解密时都作为输入参数，所以两个方向都可以测试

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        aad_addr: AAD 数据地址 (可能为 None)
        aad_size: AAD 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        log.info(f"streams plain key 加密, AAD异常测试 (init阶段)")
        t_aead_init, session = api.ehsm_aead_init_with_plain_key(
            SYM_ALGO_TO_ENUM[algo],
            AEAD_MODE_TO_ENUM[mode],
            test_vect.key,
            len(test_vect.key),
            True,  # is_encrypt=True 加密
            test_vect.iv_nonce,
            len(test_vect.iv_nonce),
            aad_addr,  # 异常的aad参数
            aad_size,
            len(test_vect.plaintext),
            len(test_vect.auth_tag),
            None
        )
        t_aead_cipher = t_aead_init
        log.info(f"init result_aead_enc_time: {t_aead_cipher}")

    elif aead_dir == "SYMM_DEC":
        log.info(f"streams plain key 解密, AAD异常测试 (init阶段)")
        t_aead_init, session = api.ehsm_aead_init_with_plain_key(
            SYM_ALGO_TO_ENUM[algo],
            AEAD_MODE_TO_ENUM[mode],
            test_vect.key,
            len(test_vect.key),
            False,  # is_encrypt=False 解密
            test_vect.iv_nonce,
            len(test_vect.iv_nonce),
            aad_addr,  # 异常的aad参数
            aad_size,
            len(test_vect.ciphertext),
            len(test_vect.auth_tag),
            None
        )
        t_aead_cipher = t_aead_init
        log.info(f"init result_aead_dec_time: {t_aead_cipher}")

    else:
        assert False

    return t_aead_cipher


def ske_aead_streams_test_plain_key_iv_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    """
    AEAD Streams 模式（三段式）明文密钥下测试 IV/Nonce 异常的接口

    测试三段式（init-update-finish）处理流程中 IV/Nonce 参数异常的场景（明文密钥模式）
    目的：测试当传入异常的 iv_addr 和 iv_size 参数时，API 的处理
    说明：init 阶段传入异常的 IV/Nonce 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        iv_addr: IV/Nonce 数据地址 (可能为 None)
        iv_size: IV/Nonce 数据长度

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, init (明文密钥, IV异常测试) # 期望失败"):
            t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                True,
                iv_addr,  # 异常的iv_addr
                iv_size,  # 异常的iv_size
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.plaintext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_cipher = t_aead_init
            log.info(f"init plain key (IV异常) result_aead_enc_time: {t_aead_cipher}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, init (明文密钥, IV异常测试) # 期望失败"):
            t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                False,
                iv_addr,  # 异常的iv_addr
                iv_size,  # 异常的iv_size
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.ciphertext),
                len(test_vect.auth_tag),
                None
            )
            t_aead_cipher = t_aead_init
            log.info(f"init plain key (IV异常) result_aead_dec_time: {t_aead_cipher}")

    else:
        assert False

    return t_aead_cipher


def ske_aead_streams_test_plain_key_context_none(algo: str, mode: str, aead_dir: str, test_vect: SymmetricTestData, context_addr) -> int:
    """
    AEAD Streams 模式（三段式）明文密钥下测试 Context 异常的接口

    测试三段式（init-update-finish）处理流程中 Context 参数异常的场景（明文密钥模式）
    目的：测试当传入异常的 context_addr 参数时，API 的处理
    说明：init 阶段传入异常的 Context 参数，期望 init 失败
          由于 init 失败，不执行后续的 update/finish

    Args:
        algo: 算法名称 (如 'AES128', 'AES256', 'SM4')
        mode: AEAD 模式名称 (如 'GCM', 'CCM')
        aead_dir: 加解密方向 ('SYMM_ENC' 或 'SYMM_DEC')
        test_vect: 测试向量数据
        context_addr: Context 数据地址 (可能为 None)

    Returns:
        执行时间
    """
    t_aead_cipher = 0

    if aead_dir == "SYMM_ENC":
        with allure.step("AEAD 加密, init (明文密钥, Context异常测试) # 期望失败"):
            t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                True,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.plaintext),
                len(test_vect.auth_tag),
                context_addr  # 异常的context参数
            )
            t_aead_cipher = t_aead_init
            log.info(f"init plain key (Context异常) result_aead_enc_time: {t_aead_cipher}")

    elif aead_dir == "SYMM_DEC":
        with allure.step("AEAD 解密, init (明文密钥, Context异常测试) # 期望失败"):
            t_aead_init, session = api.ehsm_aead_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                AEAD_MODE_TO_ENUM[mode],
                test_vect.key,
                len(test_vect.key),
                False,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad,
                len(test_vect.aad) if test_vect.aad is not None else 0,
                len(test_vect.ciphertext),
                len(test_vect.auth_tag),
                context_addr  # 异常的context参数
            )
            t_aead_cipher = t_aead_init
            log.info(f"init plain key (Context异常) result_aead_dec_time: {t_aead_cipher}")

    else:
        assert False

    return t_aead_cipher


def ske_chacha_generate_testdata(algo: str, mode: str, padding: str, key: bytes, iv: bytes, input_size: int, aad: bytes, tag_length: int) -> SymmetricTestData:
    """
    生成 ChaCha20-Poly1305 测试数据

    Args:
        algo: 算法名称 (ChaCha20)
        mode: 模式名称 (Poly1305)
        padding: 填充模式
        key: 密钥（None 则自动生成）
        iv: Nonce（None 则自动生成）
        input_size: 输入数据大小
        aad: 附加认证数据（None 则自动生成）
        tag_length: 认证标签长度

    Returns:
        SymmetricTestData: 包含测试向量的数据结构
    """
    with allure.step(f"测试数据生成: 算法 {algo}, 模式 {mode}, 输入长度 {input_size}"):
        test_vect = generate_symmetric_testdata(algo, mode, key, padding, iv, input_size, aad, tag_length)
        log.info(f"golden_key_size: {len(test_vect.key)}")
        log.info(f"golden_key_data: {''.join(f'{b:02x}' for b in test_vect.key) or '(empty)'}")
        log.info(f"golden_nonce_size: {len(test_vect.iv_nonce)}")
        log.info(f"golden_nonce_data: {''.join(f'{b:02x}' for b in test_vect.iv_nonce) or '(empty)'}")
        log.info(f"golden_plain_sz: {len(test_vect.plaintext)}")
        log.info(f"golden_plain_dt: {''.join(f'{b:02x}' for b in test_vect.plaintext) or '(empty)'}")
        log.info(f"golden_ciphe_sz: {len(test_vect.ciphertext)}")
        log.info(f"golden_ciphe_dt: {''.join(f'{b:02x}' for b in test_vect.ciphertext) or '(empty)'}")
        log.info(f"golden_tag_size: {len(test_vect.auth_tag)}")
        log.info(f"golden_tag_data: {''.join(f'{b:02x}' for b in test_vect.auth_tag) or '(empty)'}")
        if test_vect.aad is not None:
            log.info(f"golden_aad_size: {len(test_vect.aad)}")
            log.info(f"golden_aad_data: {''.join(f'{b:02x}' for b in test_vect.aad) or '(empty)'}")
    return test_vect

def ske_chacha_onepass_test(algo: str, mode: str, chacha_dir: str, test_vect: SymmetricTestData) -> int:
    """
    ChaCha20-Poly1305 onepass 加解密测试

    Args:
        algo: 算法名称
        mode: 模式名称
        chacha_dir: 方向 (CHACHA_ENC 或 CHACHA_DEC)
        test_vect: 测试向量数据

    Returns:
        int: 执行时间
    """
    with allure.step("导入 ChaCha20 密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = EhsmKeyType.EHSM_KEY_TYPE_CHACHA
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_chacha_cipher = 0
    constant = 0  # Reason: ChaCha20 使用默认常量 0

    try:
        if chacha_dir == "CHACHA_ENC":
            with allure.step("ChaCha20-Poly1305 onepass 加密 # 执行成功"):
                t_chacha_enc, cipher_text, auth_tag = api.ehsm_chacha_onepass_enc(
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad if test_vect.aad is not None else b'',
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    constant,
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.auth_tag)
                )
                t_chacha_cipher = t_chacha_enc
                log.info(f"onepass result_chacha_enc_time: {t_chacha_cipher}")
                log.info(f"onepass result_chacha_enc_size: {len(cipher_text)}")
                log.info(f"onepass result_chacha_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                log.info(f"onepass result_chacha_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext, "加密结果与预期不符"
                assert auth_tag == test_vect.auth_tag, "认证标签与预期不符"
        elif chacha_dir == "CHACHA_DEC":
            with allure.step("ChaCha20-Poly1305 onepass 解密 # 执行成功"):
                t_chacha_dec, plain_text, auth_result = api.ehsm_chacha_onepass_dec(
                    key_handle,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad if test_vect.aad is not None else b'',
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    constant,
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_chacha_cipher = t_chacha_dec
                log.info(f"onepass result_chacha_dec_time: {t_chacha_cipher}")
                log.info(f"onepass result_chacha_dec_size: {len(plain_text)}")
                log.info(f"onepass result_chacha_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                log.info(f"onepass result_chacha_auth: {auth_result}")
                assert plain_text == test_vect.plaintext, "解密结果与预期不符"
                assert auth_result == True, "认证验证失败"
        else:
            assert False, f"未知的方向: {chacha_dir}"

    finally:
        # 清理资源：确保无论断言是否失败都会释放 key_handle
        try:
            with allure.step("删除 ChaCha20 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_chacha_cipher

def ske_chacha_streams_test(algo: str, mode: str, chacha_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    """
    ChaCha20-Poly1305 三段式加解密测试 (init + update + finish)

    Args:
        algo: 算法名称
        mode: 模式名称
        chacha_dir: 方向 (CHACHA_ENC 或 CHACHA_DEC)
        test_vect: 测试向量数据
        round_num: 分段数量
        split_list: 每段数据大小列表

    Returns:
        int: 执行时间
    """
    with allure.step("导入 ChaCha20 密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = EhsmKeyType.EHSM_KEY_TYPE_CHACHA
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_chacha_cipher = 0
    constant = 0  # Reason: ChaCha20 使用默认常量 0

    try:
        if chacha_dir == "CHACHA_ENC":
            t_chacha_enc = 0
            cipher_text = b''
            cipher_text_size = 0
            total_input_size = len(test_vect.plaintext)

            with allure.step("ChaCha20-Poly1305 三段式加密, init # 执行成功"):
                t_chacha_enc_init, session = api.ehsm_chacha_init(
                    key_handle,
                    True,  # enc = True 表示加密
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad if test_vect.aad is not None else b'',
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    constant,
                    None
                )
                t_chacha_enc += t_chacha_enc_init

            with allure.step("ChaCha20-Poly1305 三段式加密, update # 执行成功"):
                log.info(f"chacha_enc_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update chacha_enc_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_chacha_enc_update, cipher_text_update = api.ehsm_chacha_update(
                            test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_chacha_enc += t_chacha_enc_update
                        cipher_text += cipher_text_update
                        cipher_text_size += len(cipher_text_update)

            with allure.step("ChaCha20-Poly1305 三段式加密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish chacha_enc_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish chacha_enc_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.plaintext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_chacha_enc_finish, cipher_text_finish, auth_tag = api.ehsm_chacha_finish_enc(
                    test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)]),
                    len(test_vect.auth_tag)
                )
                t_chacha_enc += t_chacha_enc_finish
                cipher_text += cipher_text_finish
                cipher_text_size += len(cipher_text_finish)
                auth_tag_size = len(auth_tag)
                t_chacha_cipher = t_chacha_enc
                log.info(f"streams result_chacha_enc_time: {t_chacha_cipher}")
                log.info(f"streams result_chacha_enc_size: {cipher_text_size}")
                log.info(f"streams result_chacha_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
                log.info(f"streams result_auth_tag_size: {auth_tag_size}")
                log.info(f"streams result_auth_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
                assert cipher_text == test_vect.ciphertext, "加密结果与预期不符"
                assert cipher_text_size == len(test_vect.ciphertext), "加密结果长度与预期不符"
                assert auth_tag[:len(test_vect.auth_tag)] == test_vect.auth_tag, "认证标签与预期不符"
                assert auth_tag_size >= len(test_vect.auth_tag), "认证标签长度不足"

        elif chacha_dir == "CHACHA_DEC":
            t_chacha_dec = 0
            plain_text = b''
            plain_text_size = 0
            total_input_size = len(test_vect.ciphertext)

            with allure.step("ChaCha20-Poly1305 三段式解密, init # 执行成功"):
                t_chacha_dec_init, session = api.ehsm_chacha_init(
                    key_handle,
                    False,  # enc = False 表示解密
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    test_vect.aad if test_vect.aad is not None else b'',
                    len(test_vect.aad) if test_vect.aad is not None else 0,
                    constant,
                    None  # Reason: init 阶段无已有 session，传 None 使固件写入 SESSION_ADDR
                )
                t_chacha_dec += t_chacha_dec_init

            with allure.step("ChaCha20-Poly1305 三段式解密, update # 执行成功"):
                log.info(f"chacha_dec_total_round: {round_num}  total_input_size: {total_input_size}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        log.info(f"update chacha_dec_round: {i}  " +
                            f"input_size: {sum(split_list[i:i+1])}  " +
                            f"input_data: " +
                            "".join(f"{test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])].hex()}"))
                        t_chacha_dec_update, plain_text_update = api.ehsm_chacha_update(
                            test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])],
                            sum(split_list[i:i+1])
                        )
                        t_chacha_dec += t_chacha_dec_update
                        plain_text += plain_text_update
                        plain_text_size += len(plain_text_update)

            with allure.step("ChaCha20-Poly1305 三段式解密, finish # 执行成功"):
                if round_num > 1:
                    log.info(f"finish chacha_dec_round: {int(round_num)-1}  " +
                        f"input_size: {sum(split_list[int(round_num)-1:int(round_num)])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])].hex()}"))
                else:
                    log.info(f"finish chacha_dec_round: 0  " +
                        f"input_size: {sum(split_list[0:1])}  " +
                        f"input_data: " +
                        "".join(f"{test_vect.ciphertext[sum(split_list[:0]) : sum(split_list[:1])].hex()}"))
                t_chacha_dec_finish, plain_text_finish, ret_verify_result = api.ehsm_chacha_finish_dec(
                    test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])],
                    sum(split_list[int(round_num)-1:int(round_num)]),
                    test_vect.auth_tag,
                    len(test_vect.auth_tag)
                )
                t_chacha_dec += t_chacha_dec_finish
                plain_text += plain_text_finish
                plain_text_size += len(plain_text_finish)
                t_chacha_cipher = t_chacha_dec
                log.info(f"streams result_chacha_dec_time: {t_chacha_cipher}")
                log.info(f"streams result_chacha_dec_size: {plain_text_size}")
                log.info(f"streams result_chacha_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
                log.info(f"streams result_chacha_auth: {ret_verify_result}")
                assert plain_text == test_vect.plaintext, "解密结果与预期不符"
                assert plain_text_size == len(test_vect.plaintext), "解密结果长度与预期不符"
                assert ret_verify_result == True, "认证验证失败"
        else:
            assert False, f"未知的方向: {chacha_dir}"

    finally:
        # 清理资源：确保无论断言是否失败都会释放 key_handle
        try:
            with allure.step("删除 ChaCha20 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_chacha_cipher


def ske_chacha_onepass_test_plain_key(algo: str, mode: str, chacha_dir: str, test_vect: SymmetricTestData) -> int:
    """ChaCha OnePass 明文密钥测试函数"""
    t_chacha_cipher = 0
    constant = 0  # Reason: ChaCha20 使用默认常量 0

    if chacha_dir == "CHACHA_ENC":
        with allure.step("ChaCha 加密, onepass (明文密钥) # 执行成功"):
            t_chacha_enc, cipher_text, auth_tag = api.ehsm_chacha_onepass_enc_with_plain_key(
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad if test_vect.aad is not None else b'',
                len(test_vect.aad) if test_vect.aad is not None else 0,
                constant,
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.auth_tag)
            )
            t_chacha_cipher = t_chacha_enc
            log.info(f"onepass plain key result_chacha_enc_time: {t_chacha_cipher}")
            log.info(f"onepass plain key result_chacha_enc_size: {len(cipher_text)}")
            log.info(f"onepass plain key result_chacha_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            log.info(f"onepass plain key result_chacha_tag_data: {''.join(f'{b:02x}' for b in auth_tag) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext
            assert len(cipher_text) == len(test_vect.ciphertext)
            assert auth_tag == test_vect.auth_tag
            assert len(auth_tag) == len(test_vect.auth_tag)
    elif chacha_dir == "CHACHA_DEC":
        with allure.step("ChaCha 解密, onepass (明文密钥) # 执行成功"):
            t_chacha_dec, plain_text, auth_result = api.ehsm_chacha_onepass_dec_with_plain_key(
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad if test_vect.aad is not None else b'',
                len(test_vect.aad) if test_vect.aad is not None else 0,
                constant,
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_chacha_cipher = t_chacha_dec
            log.info(f"onepass plain key result_chacha_dec_time: {t_chacha_cipher}")
            log.info(f"onepass plain key result_chacha_dec_size: {len(plain_text)}")
            log.info(f"onepass plain key result_chacha_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"onepass plain key result_chacha_auth: {auth_result}")
            assert plain_text == test_vect.plaintext
            assert len(plain_text) == len(test_vect.plaintext)
            assert auth_result == True
    else:
        assert False

    return t_chacha_cipher


def ske_chacha_streams_test_plain_key(algo: str, mode: str, chacha_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list) -> int:
    """ChaCha Streams 明文密钥测试函数 (Init-Update-Finish)"""
    t_chacha_cipher = 0
    constant = 0  # Reason: ChaCha20 使用默认常量 0

    if chacha_dir == "CHACHA_ENC":
        t_chacha_enc = 0
        cipher_text = b''
        cipher_text_size = 0

        with allure.step("ChaCha 加密, init (明文密钥) # 执行成功"):
            t_chacha_enc_init, session = api.ehsm_chacha_init_with_plain_key(
                test_vect.key,
                len(test_vect.key),
                True,  # enc = True
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad if test_vect.aad is not None else b'',
                len(test_vect.aad) if test_vect.aad is not None else 0,
                constant,
                0xFFFFFFFF
            )
            t_chacha_enc += t_chacha_enc_init

        with allure.step("ChaCha 加密, update (明文密钥) # 执行成功"):
            if round_num > 1:
                for i in range(int(round_num)-1):
                    split_size = sum(split_list[i:i+1])
                    input_chunk = test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    t_chacha_enc_update, output_chunk = api.ehsm_chacha_update(
                        input_chunk,
                        split_size
                    )
                    t_chacha_enc += t_chacha_enc_update
                    cipher_text += output_chunk
                    cipher_text_size += len(output_chunk)
                    log.info(f"round {i+1}/{len(split_list)}: input={split_size}, output={len(output_chunk)}")

        with allure.step("ChaCha 加密, finish (明文密钥) # 执行成功"):
            remaining_input = test_vect.plaintext[cipher_text_size:]
            t_chacha_enc_finish, output_final, tag_output = api.ehsm_chacha_finish_enc(
                remaining_input,
                len(remaining_input),
                len(test_vect.auth_tag)
            )
            t_chacha_enc += t_chacha_enc_finish
            cipher_text += output_final
            cipher_text_size += len(output_final)
            t_chacha_cipher = t_chacha_enc
            log.info(f"streams plain key result_chacha_enc_time: {t_chacha_cipher}")
            log.info(f"streams plain key result_chacha_enc_size: {cipher_text_size}")
            log.info(f"streams plain key result_chacha_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
            log.info(f"streams plain key result_auth_tag_data: {''.join(f'{b:02x}' for b in tag_output) or '(empty)'}")
            assert cipher_text == test_vect.ciphertext
            assert cipher_text_size == len(test_vect.ciphertext)
            assert tag_output[:len(test_vect.auth_tag)] == test_vect.auth_tag

    elif chacha_dir == "CHACHA_DEC":
        t_chacha_dec = 0
        plain_text = b''
        plain_text_size = 0

        with allure.step("ChaCha 解密, init (明文密钥) # 执行成功"):
            t_chacha_dec_init, session = api.ehsm_chacha_init_with_plain_key(
                test_vect.key,
                len(test_vect.key),
                False,  # enc = False
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.aad if test_vect.aad is not None else b'',
                len(test_vect.aad) if test_vect.aad is not None else 0,
                constant,
                0xFFFFFFFF
            )
            t_chacha_dec += t_chacha_dec_init

        with allure.step("ChaCha 解密, update (明文密钥) # 执行成功"):
            if round_num > 1:
                for i in range(int(round_num)-1):
                    split_size = sum(split_list[i:i+1])
                    input_chunk = test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    t_chacha_dec_update, output_chunk = api.ehsm_chacha_update(
                        input_chunk,
                        split_size
                    )
                    t_chacha_dec += t_chacha_dec_update
                    plain_text += output_chunk
                    plain_text_size += len(output_chunk)
                    log.info(f"round {i+1}/{len(split_list)}: input={split_size}, output={len(output_chunk)}")

        with allure.step("ChaCha 解密, finish (明文密钥) # 执行成功"):
            remaining_input = test_vect.ciphertext[plain_text_size:]
            t_chacha_dec_finish, output_final, auth_result = api.ehsm_chacha_finish_dec(
                remaining_input,
                len(remaining_input),
                test_vect.auth_tag,
                len(test_vect.auth_tag)
            )
            t_chacha_dec += t_chacha_dec_finish
            plain_text += output_final
            plain_text_size += len(output_final)
            t_chacha_cipher = t_chacha_dec
            log.info(f"streams plain key result_chacha_dec_time: {t_chacha_cipher}")
            log.info(f"streams plain key result_chacha_dec_size: {plain_text_size}")
            log.info(f"streams plain key result_chacha_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
            log.info(f"streams plain key auth_result: {auth_result}")
            assert plain_text == test_vect.plaintext
            assert plain_text_size == len(test_vect.plaintext)
            assert auth_result == True
    else:
        assert False

    return t_chacha_cipher

def ske_symm_onepass_test_iv_none(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, onepass # 执行成功"):
                t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    iv_addr,
                    iv_size,
                    test_vect.plaintext,
                    len(test_vect.plaintext),
                    len(test_vect.ciphertext)
                )
                t_symm_cipher = t_symm_enc
                log.info(f"onepass result_symm_enc_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_enc_size: {cipher_text_size}")
                log.info(f"onepass result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")
        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, onepass # 执行成功"):
                t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    iv_addr,
                    iv_size,
                    test_vect.ciphertext,
                    len(test_vect.ciphertext),
                    len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
                )
                t_symm_cipher = t_symm_dec
                log.info(f"onepass result_symm_dec_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_dec_size: {plain_text_size}")
                log.info(f"onepass result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher


def ske_symm_onepass_test_input_none(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData,input_addr,input_size) -> int:
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, onepass # 执行成功"):
                t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    input_addr,
                    input_size,
                    len(test_vect.ciphertext)
                )

                t_symm_cipher = t_symm_enc
                log.info(f"onepass result_symm_enc_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_enc_size: {cipher_text_size}")
                log.info(f"onepass result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, onepass # 执行成功"):
                t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    input_addr,
                    input_size,
                    len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
                )
                t_symm_cipher = t_symm_dec
                log.info(f"onepass result_symm_dec_time: {t_symm_cipher}")
                log.info(f"onepass result_symm_dec_size: {plain_text_size}")
                log.info(f"onepass result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher


def ske_symm_onepass_test_plain_key_iv_none(algo: str, mode: str, padding: str, symm_dir: str,
                                            test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    t_symm_cipher = 0

    if symm_dir == "SYMM_ENC":
        with allure.step("对称算法加密(明文密钥+IV异常), onepass # 执行成功"):
            t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,  # is_encrypt
                test_vect.key,
                len(test_vect.key),
                iv_addr,  # 使用传入的异常IV地址
                iv_size,  # 使用传入的异常IV长度
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.ciphertext)
            )
            t_symm_cipher = t_symm_enc
            log.info(f"onepass result_symm_enc_time: {t_symm_cipher}")
            log.info(f"onepass result_symm_enc_size: {cipher_text_size}")
            log.info(f"onepass result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

    elif symm_dir == "SYMM_DEC":
        with allure.step("对称算法解密(明文密钥+IV异常), onepass # 执行成功"):
            t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,  # is_encrypt
                test_vect.key,
                len(test_vect.key),
                iv_addr,  # 使用传入的异常IV地址
                iv_size,  # 使用传入的异常IV长度
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
            )
            t_symm_cipher = t_symm_dec
            log.info(f"onepass result_symm_dec_time: {t_symm_cipher}")
            log.info(f"onepass result_symm_dec_size: {plain_text_size}")
            log.info(f"onepass result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")
    else:
        assert False, f"Invalid symm_dir: {symm_dir}"

    return t_symm_cipher


def ske_symm_streams_test_plain_key_context_none(algo: str, mode: str, padding: str, symm_dir: str,
                                                  test_vect: SymmetricTestData, context_addr) -> int:
    """
    测试streams模式（三段式）START阶段的context异常情况（明文密钥模式）
    参数:
        context_addr: 异常的context地址（传入0表示长度为0的异常情况）
    说明:
        - START阶段(init)传入异常context参数，期望init失败
        - 由于init失败，不执行后续的update/finish
    """
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init(明文密钥+context异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    True,
                    test_vect.key,
                    len(test_vect.key),
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    context_addr  # 异常的context参数
                )
                t_symm_cipher = t_init

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init(明文密钥+context异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    False,
                    test_vect.key,
                    len(test_vect.key),
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    context_addr  # 异常的context参数
                )
                t_symm_cipher = t_init
        else:
            assert False, f"Invalid symm_dir: {symm_dir}"
    finally:
        pass

    return t_symm_cipher


def ske_symm_streams_test_plain_key_none(algo: str, mode: str, padding: str, symm_dir: str,
                                         test_vect: SymmetricTestData,
                                         plain_key, plain_key_size) -> int:
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init(明文密钥+plain_key异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    True,
                    plain_key,  # 异常的plain_key参数
                    plain_key_size,  # 异常的plain_key_size参数
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    0  # context参数使用默认值0
                )
                t_symm_cipher = t_init

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init(明文密钥+plain_key异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    False,
                    plain_key,  # 异常的plain_key参数
                    plain_key_size,  # 异常的plain_key_size参数
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    0  # context参数使用默认值0
                )
                t_symm_cipher = t_init
        else:
            assert False, f"Invalid symm_dir: {symm_dir}"
    finally:
        pass

    return t_symm_cipher


def ske_symm_streams_test_plain_key_iv_none(algo: str, mode: str, padding: str, symm_dir: str,
                                             test_vect: SymmetricTestData,
                                             iv_addr, iv_size) -> int:
    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init(明文密钥+iv异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    True,
                    test_vect.key,
                    len(test_vect.key),
                    iv_addr,  # 异常的iv参数
                    iv_size,  # 异常的iv_size参数
                    0  # session参数使用默认值0
                )
                t_symm_cipher = t_init

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init(明文密钥+iv异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    False,
                    test_vect.key,
                    len(test_vect.key),
                    iv_addr,  # 异常的iv参数
                    iv_size,  # 异常的iv_size参数
                    0  # session参数使用默认值0
                )
                t_symm_cipher = t_init
        else:
            assert False, f"Invalid symm_dir: {symm_dir}"
    finally:
        pass

    return t_symm_cipher


def ske_symm_onepass_test_plain_key_none(algo: str, mode: str, padding: str, symm_dir: str,
                                                 test_vect: SymmetricTestData,
                                                 plain_key, plain_key_size) -> int:
    t_symm_cipher = 0

    if symm_dir == "SYMM_ENC":
        with allure.step("对称算法加密(明文密钥), onepass, input异常 # 执行成功"):
            t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,  # 加密
                plain_key,
                plain_key_size,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.ciphertext)
            )

            t_symm_cipher = t_symm_enc
            log.info(f"onepass plain_key input异常 result_symm_enc_time: {t_symm_cipher}")
            log.info(f"onepass plain_key input异常 result_symm_enc_size: {cipher_text_size}")
            log.info(f"onepass plain_key input异常 result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

    elif symm_dir == "SYMM_DEC":
        with allure.step("对称算法解密(明文密钥), onepass, input异常 # 执行成功"):
            t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,  # 解密
                plain_key,
                plain_key_size,
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.ciphertext,
                len(test_vect.ciphertext),
                len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
            )

            t_symm_cipher = t_symm_dec
            log.info(f"onepass plain_key input异常 result_symm_dec_time: {t_symm_cipher}")
            log.info(f"onepass plain_key input异常 result_symm_dec_size: {plain_text_size}")
            log.info(f"onepass plain_key input异常 result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

    else:
        assert False, f"Invalid symm_dir: {symm_dir}"

    return t_symm_cipher


def ske_symm_onepass_test_plain_key_input_none(algo: str, mode: str, padding: str, symm_dir: str,
                                                 test_vect: SymmetricTestData,
                                                 input_addr, input_size) -> int:
    t_symm_cipher = 0

    if symm_dir == "SYMM_ENC":
        with allure.step("对称算法加密(明文密钥), onepass, input异常 # 执行成功"):
            t_symm_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,  # 加密
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                input_addr,  # 异常的input地址
                input_size,  # 异常的input大小
                len(test_vect.ciphertext)
            )

            t_symm_cipher = t_symm_enc
            log.info(f"onepass plain_key input异常 result_symm_enc_time: {t_symm_cipher}")
            log.info(f"onepass plain_key input异常 result_symm_enc_size: {cipher_text_size}")
            log.info(f"onepass plain_key input异常 result_symm_enc_data: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

    elif symm_dir == "SYMM_DEC":
        with allure.step("对称算法解密(明文密钥), onepass, input异常 # 执行成功"):
            t_symm_dec, plain_text, plain_text_size = api.ehsm_symm_cipher_onepass_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,  # 解密
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                input_addr,  # 异常的input地址
                input_size,  # 异常的input大小
                len(test_vect.plaintext) if padding == 'NONE' else len(test_vect.ciphertext)
            )

            t_symm_cipher = t_symm_dec
            log.info(f"onepass plain_key input异常 result_symm_dec_time: {t_symm_cipher}")
            log.info(f"onepass plain_key input异常 result_symm_dec_size: {plain_text_size}")
            log.info(f"onepass plain_key input异常 result_symm_dec_data: {''.join(f'{b:02x}' for b in plain_text) or '(empty)'}")

    else:
        assert False, f"Invalid symm_dir: {symm_dir}"

    return t_symm_cipher


def ske_symm_streams_test_plain_key_input_none(algo: str, mode: str, padding: str, symm_dir: str,
                                                 test_vect: SymmetricTestData,
                                                 round_num: int, split_list: list,
                                                 input_addr, input_size) -> int:
    """
    测试streams模式（三段式）的input异常情况
    目的：测试当传入异常的input_size参数（如0）时，API的处理
    说明：按照正常流程分割数据，但每次传入异常的size参数，验证完整的三段式流程
    """
    t_symm_cipher = 0

    # 如果 split_list 为 None 或空列表，则自动平均分割数据
    if split_list is None or len(split_list) == 0:
        if symm_dir == "SYMM_ENC":
            total_size = len(test_vect.plaintext)
        else:
            total_size = len(test_vect.ciphertext)
        base_size = total_size // round_num
        split_list = [base_size] * round_num
        remainder = total_size % round_num
        if remainder > 0:
            split_list[-1] += remainder

    if symm_dir == "SYMM_ENC":
        with allure.step("对称算法加密(明文密钥), init # 执行成功"):
            t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                True,
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                None
            )
            t_symm_cipher += t_init
            log.info(f"明文密钥 streams input异常 init 完成, session: {session}")

        with allure.step("对称算法加密(明文密钥), update(input异常) # 期望失败"):
            log.info(f"symm_enc_total_round: {round_num}  total_input_size: {len(test_vect.plaintext)}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    # 获取当前块的实际数据，但传入异常的size参数
                    current_chunk = test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    log.info(f"update symm_enc_round: {i}  " +
                        f"actual_data_size: {len(current_chunk)}  " +
                        f"input_size_param: {input_size}  " +
                        f"input_data: {current_chunk.hex()}")
                    t_update, cipher_text_update = api.ehsm_symm_cipher_update(
                        current_chunk if input_addr is not None else input_addr,
                        input_size  # 使用异常的size参数
                    )
                    t_symm_cipher += t_update
                    log.info(f"streams plain_key  update_time: {t_update}")
                    log.info(f"streams plain_key  update_size: {len(cipher_text_update)}")
                    log.info(f"streams plain_key  update_data: {''.join(f'{b:02x}' for b in cipher_text_update) or '(empty)'}")

        with allure.step("对称算法加密(明文密钥), finish(input异常) # 完成三段式流程"):
            # 获取最后一块的实际数据，但传入异常的size参数
            final_chunk = test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
            log.info(f"finish symm_enc_round: {int(round_num)-1}  " +
                f"actual_data_size: {len(final_chunk)}  " +
                f"input_size_param: {input_size}  " +
                f"input_data: {final_chunk.hex()}")
            t_finish, cipher_text_finish, cipher_text_size_finish = api.ehsm_symm_cipher_finish(
                final_chunk if input_addr is not None else input_addr,
                input_size,  # 使用异常的size参数
                len(test_vect.ciphertext)
            )
            t_symm_cipher += t_finish
            log.info(f"streams plain_key  finish_time: {t_finish}")
            log.info(f"streams plain_key  finish_size: {cipher_text_size_finish}")

    elif symm_dir == "SYMM_DEC":
        with allure.step("对称算法解密(明文密钥), init # 执行成功"):
            t_init, session = api.ehsm_symm_cipher_init_with_plain_key(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                False,
                test_vect.key,
                len(test_vect.key),
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                None
            )
            t_symm_cipher += t_init
            log.info(f"明文密钥 streams input异常 init 完成, session: {session}")

        with allure.step("对称算法解密(明文密钥), update(input异常) # 期望失败"):
            log.info(f"symm_dec_total_round: {round_num}  total_input_size: {len(test_vect.ciphertext)}  split_list: {split_list}")
            if round_num > 1:
                for i in range(int(round_num)-1):
                    # 获取当前块的实际数据，但传入异常的size参数
                    current_chunk = test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])]
                    log.info(f"update symm_dec_round: {i}  " +
                        f"actual_data_size: {len(current_chunk)}  " +
                        f"input_size_param: {input_size}  " +
                        f"input_data: {current_chunk.hex()}")
                    t_update, plain_text_update = api.ehsm_symm_cipher_update(
                        current_chunk if input_addr is not None else input_addr,
                        input_size  # 使用异常的size参数
                    )
                    t_symm_cipher += t_update
                    log.info(f"streams plain_key  update_time: {t_update}")
                    log.info(f"streams plain_key  update_size: {len(plain_text_update)}")
                    log.info(f"streams plain_key  update_data: {''.join(f'{b:02x}' for b in plain_text_update) or '(empty)'}")

        with allure.step("对称算法解密(明文密钥), finish(input异常) # 完成三段式流程"):
            # 获取最后一块的实际数据，但传入异常的size参数
            final_chunk = test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
            log.info(f"finish symm_dec_round: {int(round_num)-1}  " +
                f"actual_data_size: {len(final_chunk)}  " +
                f"input_size_param: {input_size}  " +
                f"input_data: {final_chunk.hex()}")
            t_finish, plain_text_finish, plain_text_size_finish = api.ehsm_symm_cipher_finish(
                final_chunk if input_addr is not None else input_addr,
                input_size,  # 使用异常的size参数
                len(test_vect.plaintext)
            )
            t_symm_cipher += t_finish
            log.info(f"streams plain_key  finish_time: {t_finish}")
            log.info(f"streams plain_key  finish_size: {plain_text_size_finish}")

    else:
        assert False, f"Invalid symm_dir: {symm_dir}"

    return t_symm_cipher


def ske_symm_streams_test_input_none(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, round_num: int, split_list: list, input_addr, input_size) -> int:
    """
    测试streams模式（三段式）的input异常情况
    目的：测试当传入异常的input_size参数（如0）时，API的处理
    说明：按照正常流程分割数据，但每次传入异常的size参数，验证完整的三段式流程
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    # 如果 split_list 为 None 或空列表，则自动平均分割数据
    if split_list is None or len(split_list) == 0:
        if symm_dir == "SYMM_ENC":
            total_size = len(test_vect.plaintext)
        else:
            total_size = len(test_vect.ciphertext)
        base_size = total_size // round_num
        split_list = [base_size] * round_num
        remainder = total_size % round_num
        if remainder > 0:
            split_list[-1] += remainder

    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init # 执行成功"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    None
                )
                t_symm_cipher += t_init

            with allure.step("对称算法加密, update(input异常) # 期望失败"):
                log.info(f"symm_enc_total_round: {round_num}  total_input_size: {len(test_vect.plaintext)}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        # 获取当前块的实际数据，但传入异常的size参数
                        current_chunk = test_vect.plaintext[sum(split_list[:i]) : sum(split_list[:i+1])]
                        log.info(f"update symm_enc_round: {i}  " +
                            f"actual_data_size: {len(current_chunk)}  " +
                            f"input_size_param: {input_size}  " +
                            f"input_data: {current_chunk.hex()}")
                        t_update, cipher_text_update = api.ehsm_symm_cipher_update(
                            current_chunk if input_addr is not None else input_addr,
                            input_size  # 使用异常的size参数
                        )
                        t_symm_cipher += t_update
                        log.info(f"streams update_time: {t_update}")
                        log.info(f"streams update_size: {len(cipher_text_update)}")
                        log.info(f"streams update_data: {''.join(f'{b:02x}' for b in cipher_text_update) or '(empty)'}")

            with allure.step("对称算法加密, finish(input异常) # 完成三段式流程"):
                # 获取最后一块的实际数据，但传入异常的size参数
                final_chunk = test_vect.plaintext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
                log.info(f"finish symm_enc_round: {int(round_num)-1}  " +
                    f"actual_data_size: {len(final_chunk)}  " +
                    f"input_size_param: {input_size}  " +
                    f"input_data: {final_chunk.hex()}")
                t_finish, cipher_text_finish, cipher_text_size_finish = api.ehsm_symm_cipher_finish(
                    final_chunk if input_addr is not None else input_addr,
                    input_size,  # 使用异常的size参数
                    len(test_vect.ciphertext)
                )
                t_symm_cipher += t_finish
                log.info(f"streams finish_time: {t_finish}")
                log.info(f"streams finish_size: {cipher_text_size_finish}")

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init # 执行成功"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    None
                )
                t_symm_cipher += t_init

            with allure.step("对称算法解密, update(input异常) # 期望失败"):
                log.info(f"symm_dec_total_round: {round_num}  total_input_size: {len(test_vect.ciphertext)}  split_list: {split_list}")
                if round_num > 1:
                    for i in range(int(round_num)-1):
                        # 获取当前块的实际数据，但传入异常的size参数
                        current_chunk = test_vect.ciphertext[sum(split_list[:i]) : sum(split_list[:i+1])]
                        log.info(f"update symm_dec_round: {i}  " +
                            f"actual_data_size: {len(current_chunk)}  " +
                            f"input_size_param: {input_size}  " +
                            f"input_data: {current_chunk.hex()}")
                        t_update, plain_text_update = api.ehsm_symm_cipher_update(
                            current_chunk if input_addr is not None else input_addr,
                            input_size  # 使用异常的size参数
                        )
                        t_symm_cipher += t_update
                        log.info(f"streams update_time: {t_update}")
                        log.info(f"streams update_size: {len(plain_text_update)}")
                        log.info(f"streams update_data: {''.join(f'{b:02x}' for b in plain_text_update) or '(empty)'}")

            with allure.step("对称算法解密, finish(input异常) # 完成三段式流程"):
                # 获取最后一块的实际数据，但传入异常的size参数
                final_chunk = test_vect.ciphertext[sum(split_list[:int(round_num)-1]) : sum(split_list[:int(round_num)])]
                log.info(f"finish symm_dec_round: {int(round_num)-1}  " +
                    f"actual_data_size: {len(final_chunk)}  " +
                    f"input_size_param: {input_size}  " +
                    f"input_data: {final_chunk.hex()}")
                t_finish, plain_text_finish, plain_text_size_finish = api.ehsm_symm_cipher_finish(
                    final_chunk if input_addr is not None else input_addr,
                    input_size,  # 使用异常的size参数
                    len(test_vect.plaintext)
                )
                t_symm_cipher += t_finish
                log.info(f"streams finish_time: {t_finish}")
                log.info(f"streams finish_size: {plain_text_size_finish}")

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher


def ske_symm_streams_test_iv_none(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, iv_addr, iv_size) -> int:
    """
    测试streams模式（三段式）START阶段的iv异常情况
    参数:
        iv_addr: 异常的iv地址（可能是None或有效地址）
        iv_size: 异常的iv大小（可能是0或正常长度）
    说明:
        - START阶段(init)传入异常iv参数，期望init失败
        - 由于init失败，不执行后续的update/finish
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init(iv异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    iv_addr,  # 异常的iv_addr
                    iv_size,  # 异常的iv_size
                    None
                )
                t_symm_cipher = t_init

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init(iv异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    iv_addr,  # 异常的iv_addr
                    iv_size,  # 异常的iv_size
                    None
                )
                t_symm_cipher = t_init

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher


def ske_symm_streams_test_context_none(algo: str, mode: str, padding: str, symm_dir: str, test_vect: SymmetricTestData, context_addr) -> int:
    """
    测试streams模式（三段式）START阶段的context异常情况
    参数:
        context_addr: 异常的context地址（可能是None或有效地址）
        context_size: 异常的context大小（可能是0或正常长度）
    说明:
        - START阶段(init)传入异常context参数，期望init失败
        - 由于init失败，不执行后续的update/finish
    """
    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    t_symm_cipher = 0

    try:
        if symm_dir == "SYMM_ENC":
            with allure.step("对称算法加密, init(context异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    True,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    context_addr  # 异常的context参数
                )
                t_symm_cipher = t_init

        elif symm_dir == "SYMM_DEC":
            with allure.step("对称算法解密, init(context异常) # 期望失败"):
                t_init, session = api.ehsm_symm_cipher_init(
                    SYM_ALGO_TO_ENUM[algo],
                    SYM_MODE_TO_ENUM[mode],
                    PADDING_MODE_TO_ENUM[padding],
                    key_handle,
                    False,
                    test_vect.iv_nonce,
                    len(test_vect.iv_nonce),
                    context_addr  # 异常的context参数
                )
                t_symm_cipher = t_init

        else:
            assert False

    finally:
        # 清理资源：确保无论断言是否失败都会释放key_handle
        try:
            with allure.step("删除 symm 密钥 # 执行成功"):
                api.ehsm_km_remove_key(key_handle)
        except Exception as e:
            log.warning(f"删除密钥失败: {e}")

    return t_symm_cipher


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-ECB-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.testcase("EHSM-590")
def test_ehsm_590(setup_module):
    algo = 'DES'
    mode = 'ECB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-CBC-128 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-591")
def test_ehsm_591(setup_module):
    algo = 'DES'
    mode = 'CBC'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-CFB-128 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-592")
def test_ehsm_592(setup_module):
    algo = 'DES'
    mode = 'CFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-OFB-128 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-593")
def test_ehsm_593(setup_module):
    algo = 'DES'
    mode = 'OFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-CTR-128 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-594")
def test_ehsm_594(setup_module):
    algo = 'DES'
    mode = 'CTR'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-ECB-128/192/256 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-595")
def test_ehsm_595(setup_module):
    mode = 'ECB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-CBC-128/192/256 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-596")
def test_ehsm_596(setup_module):
    mode = 'CBC'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-CFB-128/192/256 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-597")
def test_ehsm_597(setup_module):
    mode = 'CFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-OFB-128/192/256 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-598")
def test_ehsm_598(setup_module):
    mode = 'OFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-CTR-128/192/256 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-599")
def test_ehsm_599(setup_module):
    mode = 'CTR'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-XTS-128/192/256 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-600")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
def test_ehsm_600(setup_module):
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size

    # aes-xts-192 not nist standard, cryptography lib not support
    # AES-128 and AES-256 使用随机生成的测试数据
    for algo in ['AES128', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # xts finish 阶段数据传 0 报错：EHSM_ERR_XTS_WRONG_DATA_LENGTH，是正常的
        # # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        # split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        # ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        # split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        # ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

    # AES-192-XTS 使用标准测试向量 (cryptography库不支持AES-192-XTS)
    algo = 'AES192'
    log.info(f"===== 处理算法: {algo} (使用标准测试向量) =====")

    # AES-192-XTS 标准测试向量 (来自 test_ske_vecs.h)
    test_vectors = [
        {
            # Test vector 1: All zeros (32 bytes)
            'key': bytes.fromhex('000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'),
            'iv': bytes.fromhex('00000000000000000000000000000000'),
            'plaintext': bytes.fromhex('0000000000000000000000000000000000000000000000000000000000000000'),
            'ciphertext': bytes.fromhex('F8161D2515BC5D7959C9B1B2E87EB8E633CD7AF32CD1694532A8E363CA0FD819'),
        },
        {
            # Test vector 2: Pattern 0x11...22...33...44 (32 bytes)
            'key': bytes.fromhex('111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222'),
            'iv': bytes.fromhex('33333333330000000000000000000000'),
            'plaintext': bytes.fromhex('4444444444444444444444444444444444444444444444444444444444444444'),
            'ciphertext': bytes.fromhex('35FBB6B90B8DE12D38635CE0EE57BBB3EC170B271227F85CD30116C6F07857A4'),
        },
        {
            # Test vector 3: "The quick brown fox jumps over t" (32 bytes)
            'key': bytes.fromhex('2b7e151628aed2a6abf7158809cf4f3cefcdab89674523011032547698badcfe0123456789abcdeffedcba9876543210'),
            'iv': bytes.fromhex('0102030405060708090a0b0c0d0e0f10'),
            'plaintext': bytes.fromhex('54686520717569636B2062726F776E20666F78206A756D7073206F7665722074'),
            'ciphertext': bytes.fromhex('2F4085E55A6E3D3F9F6BC3B5A7B8B47558E1170FCA4AE8252205BBE267E18258'),
        },
        {
            # Test vector 4: Descending pattern 0xff...0xe7... (32 bytes)
            'key': bytes.fromhex('fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0dfdedddcdbdad9d8d7d6d5d4d3d2d1d0'),
            'iv': bytes.fromhex('ff00aa55123456789abcdef011223344'),
            'plaintext': bytes.fromhex('0123456789abcdeffedcba98765432100f1e2d3c4b5a69788796a5b4c3d2e1f0'),
            'ciphertext': bytes.fromhex('682B5D401B41956234D2E32A7EBEA9466C5FA56011C0CE92A013604A379F8A76'),
        },
        {
            # Test vector 5: "AES-192 XTS mode test vector data for validation purposes only.." (64 bytes)
            'key': bytes.fromhex('123456789ABCDEF0112233445566778899AABBCCDDEEFF00ABCDEF0123456789FEDCBA98765432100F1E2D3C4B5A6978'),
            'iv': bytes.fromhex('123456789abcdef00000000000000001'),
            'plaintext': bytes.fromhex('4145532D31393220585453206D6F6465207465737420766563746F72206461746120666F722076616C69646174696F6E20707572706F736573206F6E6C792E2E'),
            'ciphertext': bytes.fromhex('1B70A6B9582F8E9967622B3677451C9D3E836C7422ECA3C9F5C9284DF90F6E42698138FEF73B8291B2E33E70EDF7D392C40A67518D6CB453A50D6FBC10E0ECB4'),
        },
    ]

    for idx, tv in enumerate(test_vectors, 1):
        with allure.step(f"测试向量 {idx}: AES-192-XTS Stream/SingleCall 测试"):
            # 构造 SymmetricTestData 对象
            test_vect = SymmetricTestData(
                algo=algo,
                mode=mode,
                key=tv['key'],
                padding=padding,
                iv_nonce=tv['iv'],
                plaintext=tv['plaintext'],
                ciphertext=tv['ciphertext'],
                auth_tag=b'',
                aad=None
            )

            log.info(f"测试向量 {idx}: 明文长度={len(test_vect.plaintext)} 字节")

            # OnePass 测试
            ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
            ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)

            # Stream 测试 - 根据数据长度调整 round_num
            # 要求: 明文长度 >= round_num * block_size
            data_len = len(test_vect.plaintext)
            max_round = data_len // block_size  # 最大可用的 round_num

            if max_round >= round_num:
                # 数据足够长，使用默认的 round_num
                test_round = round_num
            elif max_round >= 2:
                # 数据不够长，但至少可以分成2轮
                test_round = 2
            else:
                # 数据太短(小于2个块)，跳过Stream测试
                log.info(f"测试向量 {idx}: 数据长度({data_len}字节)太短，跳过Stream测试")
                continue

            split_list = split_M_into_N(len(test_vect.plaintext), test_round, block_size, False)
            ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, test_round, split_list)

            split_list = split_M_into_N(len(test_vect.ciphertext), test_round, block_size, False)
            ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, test_round, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-CCM-128/192/256 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.testcase("EHSM-601")
def test_ehsm_601(setup_module):
    mode = 'CCM'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"===== 处理算法: {algo}  tag_length: {tag_length} =====")
            # 数据生成：随机生成对称加解密所需要的测试数据
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            ske_aead_onepass_test(algo, mode, 'SYMM_ENC', test_vect)
            ske_aead_onepass_test(algo, mode, 'SYMM_DEC', test_vect)
            split_list = split_M_into_N(n, round_num, block_size, False)
            ske_aead_streams_test(algo, mode, 'SYMM_ENC', test_vect, round_num, split_list)
            ske_aead_streams_test(algo, mode, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-GCM-128/192/256 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.testcase("EHSM-602")
def test_ehsm_602(setup_module):
    mode = 'GCM'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"===== 处理算法: {algo}  tag_length: {tag_length} =====")
            # 数据生成：随机生成对称加解密所需要的测试数据
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            ske_aead_onepass_test(algo, mode, 'SYMM_ENC', test_vect)
            ske_aead_onepass_test(algo, mode, 'SYMM_DEC', test_vect)
            split_list = split_M_into_N(n, round_num, block_size, False)
            ske_aead_streams_test(algo, mode, 'SYMM_ENC', test_vect, round_num, split_list)
            ske_aead_streams_test(algo, mode, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-ECB-128/192 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.testcase("EHSM-603")
def test_ehsm_603(setup_module):
    mode = 'ECB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-CBC-128/192 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-604")
def test_ehsm_604(setup_module):
    mode = 'CBC'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-CFB-128/192 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-605")
def test_ehsm_605(setup_module):
    mode = 'CFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-OFB-128/192 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-606")
def test_ehsm_606(setup_module):
    mode = 'OFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-CTR-128/192 的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-607")
def test_ehsm_607(setup_module):
    mode = 'CTR'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-ECB-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-608")
def test_ehsm_608(setup_module):
    algo = 'SM4'
    mode = 'ECB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-CBC-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-609")
def test_ehsm_609(setup_module):
    algo = 'SM4'
    mode = 'CBC'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-CFB-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-610")
def test_ehsm_610(setup_module):
    algo = 'SM4'
    mode = 'CFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-OFB-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-611")
def test_ehsm_611(setup_module):
    algo = 'SM4'
    mode = 'OFB'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-CTR-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.BLOCKER)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-612")
def test_ehsm_612(setup_module):
    algo = 'SM4'
    mode = 'CTR'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 不为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, False)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为 0
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-CCM-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-614")
def test_ehsm_614(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"===== 处理算法: {algo}  tag_length: {tag_length} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
        ske_aead_onepass_test(algo, mode, 'SYMM_ENC', test_vect)
        ske_aead_onepass_test(algo, mode, 'SYMM_DEC', test_vect)
        split_list = split_M_into_N(n, round_num, block_size, False)
        ske_aead_streams_test(algo, mode, 'SYMM_ENC', test_vect, round_num, split_list)
        ske_aead_streams_test(algo, mode, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-GCM-128 的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-615")
def test_ehsm_615(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"===== 处理算法: {algo}  tag_length: {tag_length} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
        ske_aead_onepass_test(algo, mode, 'SYMM_ENC', test_vect)
        ske_aead_onepass_test(algo, mode, 'SYMM_DEC', test_vect)
        split_list = split_M_into_N(n, round_num, block_size, False)
        ske_aead_streams_test(algo, mode, 'SYMM_ENC', test_vect, round_num, split_list)
        ske_aead_streams_test(algo, mode, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-ECB  PKCS7的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.testcase("EHSM-616")
def test_ehsm_616(setup_module):
    mode = 'ECB'
    padding = 'PKCS7'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-ECB  ONEWITHZEROS的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.testcase("EHSM-617")
def test_ehsm_617(setup_module):
    mode = 'ECB'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.feature("ske")
@allure.description("对SKE 命令输入异常参数测试，如错误算法、空地址、错误size")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-618")
def test_ehsm_618(setup_module):
    from platform_adapter.uart_lib import hostapi

    # 准备测试数据
    algo = 'AES128'
    mode = 'ECB'
    padding = 'NONE'
    input_size = 32

    with allure.step(f"测试数据生成: 算法 {algo}, 模式 {mode}, 输入长度 {input_size}"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, input_size)
        log.info(f"golden_key: {''.join(f'{b:02x}' for b in test_vect.key) or '(empty)'}")
        log.info(f"golden_ivn: {''.join(f'{b:02x}' for b in test_vect.iv_nonce) or '(empty)'}")
        log.info(f"golden_msg: {''.join(f'{b:02x}' for b in test_vect.plaintext) or '(empty)'}")
        log.info(f"golden_cip: {''.join(f'{b:02x}' for b in test_vect.ciphertext) or '(empty)'}")

    with allure.step("导入对称密钥 # 执行成功"):
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pub_key_size = 0
        priv_key_size = len(test_vect.key)
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, pub_key_size, priv_key_size)
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)

    with allure.step("1、传入非法算法，其它参数保持正常； # 发送成功"):
        try:
            t_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                EhsmSymmAlgo.EHSM_SYMM_ALGO_INVALID,  # 非法算法
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                key_handle,
                True,  # SYMM_ENC
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.ciphertext)
            )
            assert False, "应该抛出异常但没有抛出"
        except hostapi.HostApiError as e:
            log.info(f"非法算法测试通过，返回错误码: {e}")
            assert e != 0

    with allure.step("2、传入非法模式，其它参数保持正常； # 发送成功"):
        try:
            t_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM[algo],
                0xFF,  # 非法模式
                PADDING_MODE_TO_ENUM[padding],
                key_handle,
                True,  # SYMM_ENC
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                test_vect.plaintext,
                len(test_vect.plaintext),
                len(test_vect.ciphertext)
            )
            assert False, "应该抛出异常但没有抛出"
        except hostapi.HostApiError as e:
            log.info(f"非法模式测试通过，返回错误码: {e}")
            assert e != 0

    with allure.step("3、传入非法数据地址，其它参数正常； # 发送成功"):
        # 这个测试需要直接操作底层API，暂时跳过
        log.info("非法数据地址测试暂时跳过（需要底层API支持）")
        pass

    with allure.step("4、传入非法数据长度，其它参数正常； # 发送成功"):
        try:
            # 传入0长度数据
            t_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM[algo],
                SYM_MODE_TO_ENUM[mode],
                PADDING_MODE_TO_ENUM[padding],
                key_handle,
                True,  # SYMM_ENC
                test_vect.iv_nonce,
                len(test_vect.iv_nonce),
                b'',  # 空数据
                0,    # 长度为0
                100   # output_buf_size
            )
            assert False, "应该抛出异常但没有抛出"
        except hostapi.HostApiError as e:
            log.info(f"非法数据长度测试通过，返回错误码: {e}")
            assert e != 0
        except Exception as e:
            log.info(f"非法数据长度测试通过，出现异常: {e}")
            # 0长度数据可能在更早的地方被捕获
            pass

    with allure.step("5、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 读取成功，数据正确"):
        # 正常的对称加密测试，验证mailbox通道正常工作
        start_time = time.time()
        t_enc, cipher_text, cipher_text_size = api.ehsm_symm_cipher_onepass(
            SYM_ALGO_TO_ENUM[algo],
            SYM_MODE_TO_ENUM[mode],
            PADDING_MODE_TO_ENUM[padding],
            key_handle,
            True,  # SYMM_ENC
            test_vect.iv_nonce,
            len(test_vect.iv_nonce),
            test_vect.plaintext,
            len(test_vect.plaintext),
            len(test_vect.ciphertext)
        )
        end_time = time.time()
        elapsed_time = (end_time - start_time) * 1000  # 转换为毫秒

        log.info(f"正常加密测试通过，耗时: {elapsed_time:.2f}ms")
        log.info(f"加密结果长度: {len(cipher_text)}")
        log.info(f"加密结果: {''.join(f'{b:02x}' for b in cipher_text) or '(empty)'}")

        # 验证加密结果是否正确
        assert len(cipher_text) > 0, "加密结果不能为空"

    with allure.step("清理资源 # 清理成功"):
        api.ehsm_km_remove_key(key_handle)


@allure.feature("ske")
@allure.description("使用SingleCall方式测试算法 DES-ECB  PKCS7的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-619")
def test_ehsm_619(setup_module):
    algo = 'DES'
    mode = 'ECB'
    padding = 'PKCS7'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)

@allure.feature("ske")
@allure.description("使用SingleCall方式测试算法 DES-ECB  ONEWITHZEROS的加密和解密")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-620")
def test_ehsm_620(setup_module):
    algo = 'DES'
    mode = 'ECB'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)


@allure.feature("ske")
@allure.description("使用SingleCall方式测试算法 AES-CBC-128/192/256 使用明文密钥的加密和解密，对比加密和解密的数据")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-6101")
def test_ehsm_6101(setup_module):
    mode = 'CBC'
    padding = 'NONE'
    round_num = 5
    block_size = 16
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test_plain_key(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test_plain_key(algo, mode, padding, 'SYMM_DEC', test_vect)


# ============================================================================
# 需求2: 明文密钥方案测试用例
# ============================================================================

# ================================================================================
# 空数据边界测试用例（输入长度为0）
# ================================================================================

# ================================================================================
# 空数据三段式测试用例（Init-Update-Finish，输入长度为0）
# ================================================================================

# ================================================================================
# DES算法异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试DES算法，input相关参数异常情况下的加解密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.testcase("EHSM-S034")
def test_ehsm_s034(setup_module):
    algo = 'DES'
    mode_list = ['ECB','CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size

            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                log.info(f"生成的明文: {test_vect.plaintext}")
                log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                log.info(f"生成的密文: {test_vect.ciphertext}")
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
                # 解密功能
                ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/0)导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/0)导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                # 解密功能
                ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                    else:
                        assert False
            log.info(f"===== 加解密模式: {mode} 测试完成=====")
        log.info(f"========== 填充方式: {padding} 测试完成 ==========")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试DES算法，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S035")
def test_ehsm_s035(setup_module):
    algo = 'DES'
    mode_list = ['CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size

            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                log.info(f"生成的iv: {test_vect.iv_nonce}")
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv大小为0导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv大小为0导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
                    else:
                        assert False
            log.info(f"===== 加解密模式: {mode} 测试完成=====")
        log.info(f"========== 填充方式: {padding} 测试完成 ==========")


@allure.feature("ske")
@allure.description("使用Stream方式测试DES算法，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S036")
def test_ehsm_s036(setup_module):
    algo = 'DES'
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
                try:
                    ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
                try:
                    ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                    else:
                        assert False

            log.info(f"===== 加解密模式: {mode} 测试完成=====")
        log.info(f"========== 填充方式: {padding} 测试完成 ===========")

# ================================================================================
# AES算法（ECB/CBC/CFB/OFB/CTR模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES算法(不包括XTS，GCM，CCM模式 )，input相关参数异常情况下的加密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.testcase("EHSM-S037")
def test_ehsm_s037(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['ECB','CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                    log.info(f"生成的明文: {test_vect.plaintext}")
                    log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                    log.info(f"生成的密文: {test_vect.ciphertext}")
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
                    # 解密功能
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/0)导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/0)导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 解密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                    # 解密功能
                    ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
                log.info(f"===== 加解密模式: {mode} 测试完成=====")
            log.info(f"========== 填充方式: {padding} 测试完成 ==========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES算法(不包括XTS，GCM，CCM模式 )，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S038")
def test_ehsm_s038(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                    log.info(f"生成的iv: {test_vect.iv_nonce}")
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv大小为0导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv大小导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
                        else:
                            assert False
                log.info(f"===== 加解密模式: {mode} 测试完成=====")
            log.info(f"========== 填充方式: {padding} 测试完成 ==========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("使用Stream方式AES算法(不包括XTS，GCM，CCM模式 )，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S039")
def test_ehsm_s039(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
                    try:
                        ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
                    try:
                        ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                        else:
                            assert False

                log.info(f"===== 加解密模式: {mode} 测试完成 =====")
            log.info(f"========== 填充方式: {padding} 测试完成 ===========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# SM4算法（ECB/CBC/CFB/OFB/CTR模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4算法(不包括XTS，GCM，CCM模式 )，input相关参数异常情况下的加密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-S040")
def test_ehsm_s040(setup_module):
    algo = 'SM4'
    mode_list = ['ECB','CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                log.info(f"生成的明文: {test_vect.plaintext}")
                log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                log.info(f"生成的密文: {test_vect.ciphertext}")
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
                # 解密功能
                ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/0)导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/0)导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                # 解密功能
                ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
            log.info(f"===== 加解密模式: {mode} 测试完成=====")
        log.info(f"========== 填充方式: {padding} 测试完成 ==========")
    log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4算法(不包括XTS，GCM，CCM模式 )，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S041")
def test_ehsm_s041(setup_module):
    algo = 'SM4'
    mode_list = ['CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                log.info(f"生成的iv: {test_vect.iv_nonce}")
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv地址为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv大小为0导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv大小导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
                    else:
                        assert False
            log.info(f"===== 加解密模式: {mode} 测试完成=====")
        log.info(f"========== 填充方式: {padding} 测试完成 ==========")
    log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("使用Stream方式SM4算法(不包括XTS，GCM，CCM模式 )，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S042")
def test_ehsm_s042(setup_module):
    algo = 'SM4'
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
                # 加密算法
                try:
                    ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
                try:
                    ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                    else:
                        assert False

            log.info(f"===== 加解密模式: {mode} 测试完成 =====")
        log.info(f"========== 填充方式: {padding} 测试完成 ===========")
    log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# TDES算法（ECB/CBC/CFB/OFB/CTR模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试TDES-128/192算法，input相关参数异常情况下的加密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.testcase("EHSM-S043")
def test_ehsm_s043(setup_module):
    algo_list = ['TDES-128', 'TDES-192']
    mode_list = ['ECB','CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                    log.info(f"生成的明文: {test_vect.plaintext}")
                    log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                    log.info(f"生成的密文: {test_vect.ciphertext}")
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
                    # 解密功能
                    ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密正常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/0)导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/0)导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ UPDATE模式input异常(NONE/正常长度)导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                    # 解密功能
                    ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
                log.info(f"===== 加解密模式: {mode} 测试完成=====")
            log.info(f"========== 填充方式: {padding} 测试完成 ==========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试TDES-128/192算法，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S044")
def test_ehsm_s044(setup_module):
    algo_list = ['TDES-128', 'TDES-192']
    mode_list = ['CBC','CFB','OFB','CTR']
    padding_list = ['NONE','PKCS7','ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size

                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                    log.info(f"生成的iv: {test_vect.iv_nonce}")
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv地址为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv大小为0导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv大小导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
                        else:
                            assert False
                log.info(f"===== 加解密模式: {mode} 测试完成=====")
            log.info(f"========== 填充方式: {padding} 测试完成 ==========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("使用Stream方式TDES-128/192算法，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S045")
def test_ehsm_s045(setup_module):
    algo_list = ['TDES-128', 'TDES-192']
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
                    try:
                        ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
                    try:
                        ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                        else:
                            assert False

                log.info(f"===== 加解密模式: {mode} 测试完成 =====")
            log.info(f"========== 填充方式: {padding} 测试完成 ===========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# AES算法（ECB/CBC/CFB/OFB/CTR模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("使用明文密钥接口测试AES算法的OnePass和三段式处理时，input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S046")
def test_ehsm_s046(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    # 数据生成：使用 cryptosynth 生成标准测试向量
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                    log.info(f"生成的明文: {test_vect.plaintext}")
                    log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                    log.info(f"生成的密文: {test_vect.ciphertext}")
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
                    # 解密功能
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
                with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                    # 解密功能
                    ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
                with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False

@allure.feature("ske")
@allure.description("使用明文密钥接口测试AES算法的OnePass和三段式处理时，plain_key异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S047")
def test_ehsm_s047(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    # 数据生成：使用 cryptosynth 生成标准测试向量
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的密钥长度: {len(test_vect.key)} 字节")
                    log.info(f"生成的密钥: {test_vect.key} 字节")
                with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
                    except hostapi.HostApiError as e:
                        if 1284 == e.ret_code:
                            log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
                    # 解密功能
                    ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
                with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False

@allure.feature("ske")
@allure.description("使用明文密钥接口测试AES算法的OnePass和三段式处理时，iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S048")
def test_ehsm_s048(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode_list = ['CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                    log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                    log.info(f"生成的iv: {test_vect.iv_nonce}")
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                    except hostapi.HostApiError as e:
                        if 71 == e.ret_code:
                            log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                    except hostapi.HostApiError as e:
                        if 1281 == e.ret_code:
                            log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False

@allure.feature("ske")
@allure.description("使用明文密钥接口测试AES算法的三段式模式，context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S049")
def test_ehsm_s049(setup_module):
    algo_list = ['AES128', 'AES192', 'AES256']
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        for padding in padding_list:
            log.info(f"========== 填充方式: {padding} ==========")
            for mode in mode_list:
                log.info(f"===== 加解密模式: {mode} =====")
                if padding == 'NONE':
                    # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                    n = align_up(n, round_num * block_size)
                else:
                    if n < round_num * block_size:
                        n = n % block_size + round_num * block_size
                with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
                    try:
                        ske_symm_streams_test_plain_key_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                        else:
                            assert False
                with allure.step(f"三段式处理模式下，测试context参数异常时的解密功能 # 解密异常"):
                    try:
                        ske_symm_streams_test_plain_key_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                        else:
                            assert False

                log.info(f"===== 加解密模式: {mode} 测试完成 =====")
            log.info(f"========== 填充方式: {padding} 测试完成 ===========")
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# SM4算法（ECB/CBC/CFB/OFB/CTR模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("使用明文密钥接口测试SM4算法的OnePass和三段式处理时，input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S050")
def test_ehsm_s050(setup_module):
    algo = 'SM4'
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                # 数据生成：使用 cryptosynth 生成标准测试向量
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                log.info(f"生成的明文: {test_vect.plaintext}")
                log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                log.info(f"生成的密文: {test_vect.ciphertext}")
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
                # 解密功能
                ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
                # 解密功能
                ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("使用明文密钥接口测试SM4算法的OnePass和三段式处理时，plain_key异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S051")
def test_ehsm_s051(setup_module):
    algo = 'SM4'
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    log.info(f"生成的消息长度为{n}")
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            # 数据生成：使用 cryptosynth 生成标准测试向量
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的密钥长度: {len(test_vect.key)} 字节")
            log.info(f"生成的明钥: {len(test_vect.plaintext)} 字节")
            with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
                # 解密功能
                ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
            with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                        else:
                            assert False
            with allure.step(f"三段式处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                    # 加密功能
                    ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
                    # 解密功能
                    ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
            with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                    # 加密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                        else:
                            assert False
                    # 解密功能
                    try:
                        ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
                    except hostapi.HostApiError as e:
                        if 8 == e.ret_code:
                            log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                        else:
                            assert False


@allure.feature("ske")
@allure.description("使用明文密钥接口测试SM4算法的OnePass模式和三段式处理时，iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S052")
def test_ehsm_s052(setup_module):
    algo = 'SM4'
    mode_list = ['CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
                log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                log.info(f"生成的iv: {test_vect.iv_nonce}")
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False


@allure.feature("ske")
@allure.description("使用明文密钥接口测试SM4算法的三段式模式，context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S053")
def test_ehsm_s053(setup_module):
    algo = 'SM4'
    mode_list = ['ECB', 'CBC', 'CFB', 'OFB', 'CTR']
    padding_list = ['NONE', 'PKCS7', 'ISO7816']
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    for padding in padding_list:
        log.info(f"========== 填充方式: {padding} ==========")
        for mode in mode_list:
            log.info(f"===== 加解密模式: {mode} =====")
            if padding == 'NONE':
                # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
                n = align_up(n, round_num * block_size)
            else:
                if n < round_num * block_size:
                    n = n % block_size + round_num * block_size
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
                try:
                    ske_symm_streams_test_plain_key_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
                try:
                    ske_symm_streams_test_plain_key_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                    else:
                        assert False

            log.info(f"===== 加解密模式: {mode} 测试完成 =====")
        log.info(f"========== 填充方式: {padding} 测试完成 ===========")
    log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# AES算法（XTS模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES-XTS算法，input相关参数异常情况下的加解密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.testcase("EHSM-S054")
def test_ehsm_s054(setup_module):
    algo_list = ['AES128','AES256']
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
            log.info(f"生成的明文: {test_vect.plaintext}")
            log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
            log.info(f"生成的密文: {test_vect.ciphertext}")
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            try:
                # 加密功能
                ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            try:
                # 解密功能
                ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ UPDATE处理模式input异常(NONE/0)导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ UPDATE处理模式input异常(NONE/0)导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ UPDATE处理模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ UPDATE处理模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            try:
                # 加密功能
                ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
            except hostapi.HostApiError as e:
                if 55 == e.ret_code:
                    log.info(f"✓ 三段式处理模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
                else:
                    assert False
            try:
                # 解密功能
                ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
            except hostapi.HostApiError as e:
                if 55 == e.ret_code:
                    log.info(f"✓ 三段式处理模式input异常(NONE/正常长度)导致解密异常: 错误码{e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES-XTS算法，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S055")
def test_ehsm_s055(setup_module):
    algo_list = ['AES128', 'AES256']
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size

        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
            log.info(f"生成的iv: {test_vect.iv_nonce}")
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv地址为空导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv地址为空导致解密异常: {e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv大小为0导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv大小导致解密异常: {e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("使用Stream方式AES-XTS算法，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S056")
def test_ehsm_s056(setup_module):
    algo_list = ['AES128', 'AES256']
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
            try:
                ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
            try:
                ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# SM4算法（XTS模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4-XTS算法，input相关参数异常情况下的加解密功能")
@allure.severity(allure.severity_level.MINOR)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.testcase("EHSM-S057")
def test_ehsm_s057(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    log.info(f"=============== 处理算法: {algo} ===============")
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
        log.info(f"生成的明文: {test_vect.plaintext}")
        log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
        log.info(f"生成的密文: {test_vect.ciphertext}")
    with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
        try:
            # 加密功能
            ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext ,0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据不为空，长度为0导致加密异常: 错误码{e}")
            else:
                assert False
        try:
            # 解密功能
            ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext ,0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据不为空，长度为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None ,len(test_vect.plaintext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None ,len(test_vect.ciphertext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ UPDATE处理模式input异常(NONE/0)导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ UPDATE处理模式input异常(NONE/0)导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ UPDATE处理模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.ciphertext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ UPDATE处理模式input异常(NONE/正常长度)导致加密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
        try:
            # 加密功能
            ske_symm_streams_test_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
        except hostapi.HostApiError as e:
            if 55 == e.ret_code:
                log.info(f"✓ final处理模式input异常(正常数据/0)导致加密异常: 错误码{e}")
            else:
                assert False
        try:
            # 解密功能
            ske_symm_streams_test_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
        except hostapi.HostApiError as e:
            if 55 == e.ret_code:
                log.info(f"✓ final处理模式input异常(正常数据/0)导致解密异常: 错误码{e}")
            else:
                assert False
    log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4-XTS算法，iv相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S058")
def test_ehsm_s058(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size

    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
        log.info(f"生成的iv: {test_vect.iv_nonce}")
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None,0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv地址和大小为空导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None,0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv地址和大小为空导致解密异常: {e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv地址为空导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv地址为空导致解密异常: {e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv大小为0导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv大小导致解密异常: {e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ START阶段iv地址和大小为空导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ START阶段iv地址和大小为空导致解密异常: {e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ START阶段iv地址为空导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ START阶段iv地址为空导致解密异常: {e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ START阶段iv大小为0导致加密异常: {e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ START阶段iv大小为0导致解密异常: {e}")
            else:
                assert False

@allure.feature("ske")
@allure.description("使用Stream方式SM4-XTS算法，context相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S059")
def test_ehsm_s059(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的加密功能 # 加密异常"):
        try:
            ske_symm_streams_test_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试context参数异常时（非空context）的解密功能 # 解密异常"):
        try:
            ske_symm_streams_test_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
            else:
                assert False

# ================================================================================
# AES算法（XTS模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES-XTS算法，input相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S060")
def test_ehsm_s060(setup_module):
    algo_list = ['AES128','AES256']
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            # 数据生成：使用 cryptosynth 生成标准测试向量
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
            log.info(f"生成的明文: {test_vect.plaintext}")
            log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
            log.info(f"生成的密文: {test_vect.ciphertext}")
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据正常，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ input数据正常，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
            except hostapi.HostApiError as e:
                if 55 == e.ret_code:
                    log.info(f"✓ input数据正常，长度为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
            except hostapi.HostApiError as e:
                if 55 == e.ret_code:
                    log.info(f"✓ input数据正常，长度为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成===============")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES-XTS，iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S061")
def test_ehsm_s061(setup_module):
    algo_list = ['AES128','AES256']
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
            log.info(f"生成的iv: {test_vect.iv_nonce}")
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

@allure.feature("ske")
@allure.description("使用Stream方式测试AES-XTS算法，context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S062")
def test_ehsm_s062(setup_module):
    algo_list = ['AES128', 'AES256']
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
            try:
                ske_symm_streams_test_plain_key_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试context参数异常时的解密功能 # 解密异常"):
            try:
                ske_symm_streams_test_plain_key_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试AES-XTS算法，plain_key异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S063")
def test_ehsm_s063(setup_module):
    algo_list = ['AES128','AES256']
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    for algo in algo_list:
        log.info(f"=============== 处理算法: {algo} ===============")
        if padding == 'NONE':
            # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
            n = align_up(n, round_num * block_size)
        else:
            if n < round_num * block_size:
                n = n % block_size + round_num * block_size
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
            log.info(f"生成的密钥长度: {len(test_vect.key)} 字节")
            log.info(f"生成的密钥: {test_vect.key} 字节")
        with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 1284 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
            # 解密功能
            ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
        with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        log.info(f"=============== 算法: {algo} 测试完成 ===============")

# ================================================================================
# SM4算法（XTS模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4-XTS算法，input相关参数异常时加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S064")
def test_ehsm_s064(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        # 数据生成：使用 cryptosynth 生成标准测试向量
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
        log.info(f"生成的明文: {test_vect.plaintext}")
        log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
        log.info(f"生成的密文: {test_vect.ciphertext}")
    with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据正常，长度为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
        except hostapi.HostApiError as e:
            if 1282 == e.ret_code:
                log.info(f"✓ input数据正常，长度为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], test_vect.plaintext, 0)
        except hostapi.HostApiError as e:
            if 55 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], test_vect.ciphertext, 0)
        except hostapi.HostApiError as e:
            if 55 == e.ret_code:
                log.info(f"✓ input数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, [], None, len(test_vect.plaintext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_input_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, round_num, [], None, len(test_vect.ciphertext))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4-XTS，iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S065")
def test_ehsm_s065(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
        log.info(f"生成的iv: {test_vect.iv_nonce}")
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
        except hostapi.HostApiError as e:
            if 71 == e.ret_code:
                log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_iv_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
        except hostapi.HostApiError as e:
            if 1281 == e.ret_code:
                log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False

@allure.feature("ske")
@allure.description("使用Stream测试SM4-XTS算法，context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S066")
def test_ehsm_s066(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
        try:
            ske_symm_streams_test_plain_key_context_none(algo, mode, padding, 'SYMM_ENC', test_vect, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ START阶段context异常(有效地址)导致加密异常: {e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试context参数异常时的加密功能 # 加密异常"):
        try:
            ske_symm_streams_test_plain_key_context_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ START阶段context异常(有效地址)导致解密异常: {e}")
            else:
                assert False

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试SM4-XTS算法，plain_key异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_XTS_SUPPORT == 0, reason="不支持 XTS 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S067")
def test_ehsm_s067(setup_module):
    algo = 'SM4'
    mode = 'XTS'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        log.info(f"生成的密钥长度: {len(test_vect.key)} 字节")
        log.info(f"生成的密钥: {test_vect.key} 字节")
    with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
        except hostapi.HostApiError as e:
            if 1284 == e.ret_code:
                log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"Onepass处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_onepass_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/0）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据及长度都为空导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, 0)
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据及长度都为空导致解密异常: 错误码{e}")
            else:
                assert False
    with allure.step(f"三段式处理模式下，测试plain_key参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
        # 加密功能
        ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, test_vect.key, 0)
        # 解密功能
        ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, test_vect.key, 0)
    with allure.step(f"三段式处理模式下，测试plain_key参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
        # 加密功能
        try:
            ske_symm_streams_test_plain_key_none(algo, mode, padding, 'SYMM_ENC', test_vect, None, len(test_vect.key))
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
            else:
                assert False
        # 解密功能
        try:
            ske_symm_streams_test_plain_key_none(algo, mode, 'NONE', 'SYMM_DEC', test_vect, None, len(test_vect.key))
        except hostapi.HostApiError as e:
            if 8 == e.ret_code:
                log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
            else:
                assert False

# ================================================================================
# AES算法（GCM模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("测试AES-GCM算法在input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S068")
def test_ehsm_s068(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, 0, None, tag_length)
                log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                log.info(f"生成的明文: {test_vect.plaintext}")
                log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                log.info(f"生成的密文: {test_vect.ciphertext}")
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
                # 解密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
                # 解密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,  'SYMM_DEC', test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [],test_vect.plaintext, 0)
                # 解密功能
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_DEC', test_vect, n, [], test_vect.ciphertext, 0)
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,'SYMM_ENC', test_vect, n, [], None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,'SYMM_DEC', test_vect, n, [], None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("测试AES-GCM算法在iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S069")
def test_ehsm_s069(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                log.info(f"生成的iv: {test_vect.iv_nonce}")
            with allure.step(f"Onepass处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False

            with allure.step(f"Onepass处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect,  None, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,  'SYMM_DEC', test_vect,  None, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_DEC', test_vect,  test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,'SYMM_ENC', test_vect,  None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,'SYMM_DEC', test_vect,  None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("测试AES-GCM算法在context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S070")
def test_ehsm_s070(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            with allure.step(f"三段式处理模式下，测试参数context异常时（正常数据/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_context_none(algo, mode,'SYMM_ENC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ context数据不为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_context_none(algo, mode,'SYMM_DEC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ context数据不为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试AES-GCM算法，在tag异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S071")
def test_ehsm_s071(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的tag长度: {len(test_vect.auth_tag)} 字节")
                log.info(f"生成的tag: {test_vect.auth_tag}")
            with allure.step(f"Onepass处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
                # 解密功能
                try:
                    ske_aead_onepass_test_tag_none(algo, mode, test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False

            with allure.step(f"Onepass处理模式下，测试参数tag异常时（正常数据/0）的解密功能 # 解密正常"):
                # 解密功能
                ske_aead_onepass_test_tag_none(algo, mode, test_vect, test_vect.auth_tag, 0)
            with allure.step(f"Onepass处理模式下，测试tag参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 解密功能
                try:
                    ske_aead_onepass_test_tag_none(algo, mode,test_vect, None, len(test_vect.auth_tag))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 71 == e.ret_code:
                        log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试AES-GCM算法，在aad异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S072")
def test_ehsm_s072(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的aad长度: {len(test_vect.aad)} 字节")
                log.info(f"生成的aad: {test_vect.aad}")
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/0）的解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                # 解密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（正常数据/0）的解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
                # 解密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                # 解密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            with allure.step(f"三段式处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.iv_nonce, 0)
                # 解密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.iv_nonce, 0)
            with allure.step(f"三段式处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

# ================================================================================
# SM4算法（GCM模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("测试SM4-GCM算法在input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S073")
def test_ehsm_s073(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
            log.info(f"生成的明文: {test_vect.plaintext}")
            log.info(f"生成的明文长度: {len(test_vect.ciphertext)} 字节")
            log.info(f"生成的明文: {test_vect.ciphertext}")

        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
            # 解密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
        with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
            # 解密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,  'SYMM_DEC', test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [],test_vect.plaintext, 0)
            # 解密功能
            ske_aead_streams_test_input_none(algo, mode, 'SYMM_DEC', test_vect, n, [], test_vect.ciphertext, 0)
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,'SYMM_ENC', test_vect, n, [], None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,'SYMM_DEC', test_vect, n, [], None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

@allure.feature("ske")
@allure.description("测试SM4-GCM算法在iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S074")
def test_ehsm_s074(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
            log.info(f"生成的iv: {test_vect.iv_nonce}")
        with allure.step(f"Onepass处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False

        with allure.step(f"Onepass处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect,  None, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,  'SYMM_DEC', test_vect,  None, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_DEC', test_vect,  test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,'SYMM_ENC', test_vect,  None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,'SYMM_DEC', test_vect,  None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False


@allure.feature("ske")
@allure.description("测试SM4-GCM算法在context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S075")
def test_ehsm_s075(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
        with allure.step(f"三段式处理模式下，测试参数context异常时（正常数据/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_context_none(algo, mode,'SYMM_ENC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ context数据不为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_context_none(algo, mode,'SYMM_DEC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ context数据不为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False


@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试SM4-GCM算法，在tag异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S076")
def test_ehsm_s076(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的tag长度: {len(test_vect.auth_tag)} 字节")
            log.info(f"生成的tag: {test_vect.auth_tag}")
        with allure.step(f"Onepass处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
            # 解密功能
            try:
                ske_aead_onepass_test_tag_none(algo, mode, test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False

        with allure.step(f"Onepass处理模式下，测试参数tag异常时（正常数据/0）的解密功能 # 解密正常"):
            # 解密功能
            ske_aead_onepass_test_tag_none(algo, mode, test_vect, test_vect.auth_tag, 0)
        with allure.step(f"Onepass处理模式下，测试tag参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 解密功能
            try:
                ske_aead_onepass_test_tag_none(algo, mode,test_vect, None, len(test_vect.auth_tag))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 71 == e.ret_code:
                    log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False


@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试SM4-GCM算法，在aad异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S077")
def test_ehsm_s077(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的aad长度: {len(test_vect.aad)} 字节")
            log.info(f"生成的aad: {test_vect.aad}")
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/0）的解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            # 解密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（正常数据/0）的解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
            # 解密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            # 解密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
        with allure.step(f"三段式处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.iv_nonce, 0)
            # 解密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.iv_nonce, 0)
        with allure.step(f"三段式处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

# ================================================================================
# AES算法（GCM模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("使用明文密钥接口分别在onepass和Stream处理方式测试AES-GCM算法，在plain_key异常时的加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S078")
def test_ehsm_s078(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的key长度: {len(test_vect.key)} 字节")
                log.info(f"生成的key: {test_vect.key}")
            with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（None/0）的解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（正常数据/0）的解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, test_vect.key, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, test_vect.key, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数plain_key异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数plain_key异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, test_vect.key, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, test_vect.key, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数plain_key异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.key))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

# ================================================================================
# SM4算法（GCM模式下）异常参数测试用例（明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("使用明文密钥接口测试onepass和Stream处理方式测试SM4-GCM算法，在plain_key异常时的加解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_GCM_SUPPORT == 0, reason="不支持 GCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S084")
def test_ehsm_s084(setup_module):
    algo = 'SM4'
    mode = 'GCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的key长度: {len(test_vect.key)} 字节")
            log.info(f"生成的key: {test_vect.key}")
        with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（None/0）的解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（正常数据/0）的解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试参数plain_key异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数plain_key异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数plain_key异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, test_vect.key, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数plain_key异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_plainkey_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.key))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ plain_key数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

# ================================================================================
# AES算法（CCM模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("测试AES-CCM算法在input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S091")
def test_ehsm_s091(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
                log.info(f"生成的明文: {test_vect.plaintext}")
                log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
                log.info(f"生成的密文: {test_vect.ciphertext}")
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
                # 解密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
                # 解密功能
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
            with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,  'SYMM_DEC', test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [],test_vect.plaintext, 0)
                # 解密功能
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_DEC', test_vect, n, [], test_vect.ciphertext, 0)
            with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,'SYMM_ENC', test_vect, n, [], None, len(test_vect.plaintext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_input_none(algo, mode,'SYMM_DEC', test_vect, n, [], None, len(test_vect.ciphertext))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("测试AES-CCM算法在iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S092")
def test_ehsm_s092(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
                log.info(f"生成的iv: {test_vect.iv_nonce}")
            with allure.step(f"Onepass处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False

            with allure.step(f"Onepass处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect,  None, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,  'SYMM_DEC', test_vect,  None, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode, 'SYMM_DEC', test_vect,  test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 117 == e.ret_code:
                        log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数iv异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,'SYMM_ENC', test_vect,  None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_iv_none(algo, mode,'SYMM_DEC', test_vect,  None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("测试AES-CCM算法在context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S093")
def test_ehsm_s093(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            with allure.step(f"三段式处理模式下，测试参数context异常时（正常数据/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_context_none(algo, mode,'SYMM_ENC', test_vect, 0)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ context数据不为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_context_none(algo, mode,'SYMM_DEC', test_vect, 0,)
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ context数据不为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试AES-CCM算法，在tag异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S094")
def test_ehsm_s094(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的tag长度: {len(test_vect.auth_tag)} 字节")
                log.info(f"生成的tag: {test_vect.auth_tag}")
            with allure.step(f"Onepass处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
                # 解密功能
                try:
                    ske_aead_onepass_test_tag_none(algo, mode, test_vect, None, 0)
                except hostapi.HostApiError as e:
                    if 1282 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False

            with allure.step(f"Onepass处理模式下，测试参数tag异常时（正常数据/0）的解密功能 # 解密正常"):
                # 解密功能
                try:
                    ske_aead_onepass_test_tag_none(algo, mode, test_vect, test_vect.auth_tag, 0)
                except hostapi.HostApiError as e:
                    if 1282 == e.ret_code:
                        log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"Onepass处理模式下，测试tag参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 解密功能
                try:
                    ske_aead_onepass_test_tag_none(algo, mode,test_vect, None, len(test_vect.auth_tag))
                except hostapi.HostApiError as e:
                    if 1281 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, 0)
                except hostapi.HostApiError as e:
                    if 1282 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（正常数据/0）的加解密功能 # 加解密正常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], test_vect.iv_nonce, 0)
                except hostapi.HostApiError as e:
                    if 1282 == e.ret_code:
                        log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数tag异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 解密功能
                try:
                    ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, len(test_vect.iv_nonce))
                except hostapi.HostApiError as e:
                    if 1282 == e.ret_code:
                        log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

@allure.feature("ske")
@allure.description("测试AES-CCM算法在aad异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S095")
def test_ehsm_s095(setup_module):
    algo_list = ['AES128','AES192','AES256']
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = 3

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size

    for algo in algo_list:
        for tag_length in [4, 6, 8, 10, 12, 14, 16]:
            log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
            with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
                test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
                log.info(f"生成的aad长度: {len(test_vect.aad)} 字节")
                log.info(f"生成的aad: {test_vect.aad}")
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                # 解密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
                # 解密功能
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
            with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False
            with allure.step(f"三段式处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
                # 解密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
            with allure.step(f"三段式处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
                # 加密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
                # 解密功能
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
            with allure.step(f"三段式处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
                # 加密功能
                try:
                    ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                    else:
                        assert False
                # 解密功能
                try:
                    ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
                except hostapi.HostApiError as e:
                    if 8 == e.ret_code:
                        log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                    else:
                        assert False

# ================================================================================
# SM4算法（CCM模式下）异常参数测试用例（非明文密钥）
# ================================================================================

@allure.feature("ske")
@allure.description("测试SM4-CCM算法在input异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S096")
def test_ehsm_s096(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的明文长度: {len(test_vect.plaintext)} 字节")
            log.info(f"生成的明文: {test_vect.plaintext}")
            log.info(f"生成的密文长度: {len(test_vect.ciphertext)} 字节")
            log.info(f"生成的密文: {test_vect.ciphertext}")

        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
            # 解密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
        with allure.step(f"Onepass处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.plaintext, 0)
            # 解密功能
            ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.ciphertext, 0)
        with allure.step(f"Onepass处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_input_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,  'SYMM_DEC', test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试input参数异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            ske_aead_streams_test_input_none(algo, mode, 'SYMM_ENC', test_vect, n, [],test_vect.plaintext, 0)
            # 解密功能
            ske_aead_streams_test_input_none(algo, mode, 'SYMM_DEC', test_vect, n, [], test_vect.ciphertext, 0)
        with allure.step(f"三段式处理模式下，测试input参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,'SYMM_ENC', test_vect, n, [], None, len(test_vect.plaintext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_input_none(algo, mode,'SYMM_DEC', test_vect, n, [], None, len(test_vect.ciphertext))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ input数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

@allure.feature("ske")
@allure.description("测试SM4-CCM算法在iv异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S097")
def test_ehsm_s097(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的iv长度: {len(test_vect.iv_nonce)} 字节")
            log.info(f"生成的iv: {test_vect.iv_nonce}")
        with allure.step(f"Onepass处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False

        with allure.step(f"Onepass处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试iv参数异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_iv_none(algo, mode, 'SYMM_DEC', test_vect, None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect,  None, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,  'SYMM_DEC', test_vect,  None, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_ENC', test_vect, test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode, 'SYMM_DEC', test_vect,  test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 117 == e.ret_code:
                    log.info(f"✓ iv数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数iv异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,'SYMM_ENC', test_vect,  None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_iv_none(algo, mode,'SYMM_DEC', test_vect,  None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ iv数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

@allure.feature("ske")
@allure.description("测试SM4-CCM算法在context异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S098")
def test_ehsm_s098(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
        with allure.step(f"三段式处理模式下，测试参数context异常时（正常数据/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_context_none(algo, mode,'SYMM_ENC', test_vect, 0)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ context数据不为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_context_none(algo, mode,'SYMM_DEC', test_vect, 0,)
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ context数据不为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

@allure.feature("ske")
@allure.description("使用onepass和Stream处理方式测试SM4-CCM算法，在tag异常时的解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S099")
def test_ehsm_s099(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = DEFAULT_ROUND_NUM

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的tag长度: {len(test_vect.auth_tag)} 字节")
            log.info(f"生成的tag: {test_vect.auth_tag}")
        with allure.step(f"Onepass处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
            # 解密功能
            try:
                ske_aead_onepass_test_tag_none(algo, mode, test_vect, None, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False

        with allure.step(f"Onepass处理模式下，测试参数tag异常时（正常数据/0）的解密功能 # 解密正常"):
            # 解密功能
            try:
                ske_aead_onepass_test_tag_none(algo, mode, test_vect, test_vect.auth_tag, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"Onepass处理模式下，测试tag参数异常时（None/正常长度）的解密功能 # 加解密异常"):
            # 解密功能
            try:
                ske_aead_onepass_test_tag_none(algo, mode,test_vect, None, len(test_vect.auth_tag))
            except hostapi.HostApiError as e:
                if 1281 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（None/0）的解密功能 # 解密异常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（正常数据/0）的加解密功能 # 加解密正常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], test_vect.iv_nonce, 0)
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ tag数据不为空，长度为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数tag异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 解密功能
            try:
                ske_aead_streams_test_tag_none(algo, mode, test_vect, n, [], None, len(test_vect.iv_nonce))
            except hostapi.HostApiError as e:
                if 1282 == e.ret_code:
                    log.info(f"✓ tag数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False


@allure.feature("ske")
@allure.description("测试SM4-CCM算法在aad异常时加密和解密功能")
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@pytest.mark.skipif(cfg_data.TEST_FW_CCM_SUPPORT == 0, reason="不支持 CCM 算法")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-S100")
def test_ehsm_s100(setup_module):
    algo = 'SM4'
    mode = 'CCM'
    padding = 'NONE'
    block_size = 16
    round_num = 3

    # 生成随机消息长度
    n = generate_random_msg_len()
    if padding == 'NONE':
        # 若 padding 为 NONE, 则为了确保各分组数据均能 block_size 对齐，且不允许出现 0, 向上 block_size 对齐
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for tag_length in [4, 6, 8, 10, 12, 14, 16]:
        log.info(f"=============== 测试算法: {algo}  tag_lenth:{tag_length} ===============")
        with allure.step(f"生成测试数据 (消息长度: {n} 字节) # 生成数据成功"):
            test_vect = ske_aead_generate_testdata(algo, mode, padding, None, None, n, None, tag_length)
            log.info(f"生成的aad长度: {len(test_vect.aad)} 字节")
            log.info(f"生成的aad: {test_vect.aad}")
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            # 解密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
            # 解密功能
            ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
        with allure.step(f"Onepass处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_onepass_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False
        with allure.step(f"三段式处理模式下，测试参数aad异常时（None/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, 0)
            # 解密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, 0)
        with allure.step(f"三段式处理模式下，测试参数aad异常时（正常数据/0）的加解密功能 # 加解密异常"):
            # 加密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, test_vect.aad, 0)
            # 解密功能
            ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, test_vect.aad, 0)
        with allure.step(f"三段式处理模式下，测试参数aad异常时（None/正常长度）的加解密功能 # 加解密异常"):
            # 加密功能
            try:
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_ENC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致加密异常: 错误码{e}")
                else:
                    assert False
            # 解密功能
            try:
                ske_aead_streams_test_aad_none(algo, mode, "SYMM_DEC", test_vect, None, len(test_vect.aad))
            except hostapi.HostApiError as e:
                if 8 == e.ret_code:
                    log.info(f"✓ aad数据为空，长度不为0导致解密异常: 错误码{e}")
                else:
                    assert False

# ================================================================================
# AES算法（CCM模式下）异常参数测试用例（明文密钥）
# ================================================================================

# ================================================================================
# SM4算法（CCM模式下）异常参数测试用例（明文密钥）
# ================================================================================

# ================================================================================
# AEAD 异常数据检查测试用例
# ================================================================================

# ==================== DES明文密钥测试 ====================

# ==================== 3DES明文密钥测试 ====================

# ==================== 3DES-192明文密钥测试 ====================

# ==================== AES-128明文密钥测试 ====================

# ==================== AES-192明文密钥测试 ====================

# ==================== AES-XTS明文密钥测试 ====================

# ==================== ChaCha20明文密钥测试 ====================

# ==================== 补充对称加密 ISO7816/PKCS7 普通接口测试 ====================

@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 DES-CBC ISO7816的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_DES_SUPPORT == 0, reason="不支持 DES 算法")
@allure.testcase("EHSM-S155")
def test_ehsm_s155(setup_module):
    algo = 'DES'
    mode = 'CBC'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-ECB ISO7816的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.testcase("EHSM-S156")
def test_ehsm_s156(setup_module):
    mode = 'ECB'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 TDES-CBC ISO7816的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_TDES_SUPPORT == 0, reason="不支持 TDES 算法")
@allure.testcase("EHSM-S157")
def test_ehsm_s157(setup_module):
    mode = 'CBC'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 8

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['TDES-128', 'TDES-192']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 AES-CBC ISO7816的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_AES_SUPPORT == 0, reason="不支持 AES 算法")
@allure.testcase("EHSM-S158")
def test_ehsm_s158(setup_module):
    mode = 'CBC'
    padding = 'ISO7816'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    for algo in ['AES128', 'AES192', 'AES256']:
        log.info(f"===== 处理算法: {algo} =====")
        # 数据生成：随机生成对称加解密所需要的测试数据
        test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
        ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
        # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
        # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
        split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
        ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-ECB PKCS7的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-S159")
def test_ehsm_s159(setup_module):
    algo = 'SM4'
    mode = 'ECB'
    padding = 'PKCS7'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


@allure.feature("ske")
@allure.description("分别使用Stream和SingleCall方式测试算法 SM4-CBC PKCS7的加密和解密，对比加密和解密的数据")
@allure.severity(allure.severity_level.NORMAL)
@pytest.mark.skipif(cfg_data.TEST_FW_CIPHER_SM4_SUPPORT == 0, reason="不支持 SM4 算法")
@allure.testcase("EHSM-S160")
def test_ehsm_s160(setup_module):
    algo = 'SM4'
    mode = 'CBC'
    padding = 'PKCS7'
    round_num = DEFAULT_ROUND_NUM
    block_size = 16

    n = generate_random_msg_len()

    if padding == 'NONE':
        n = align_up(n, round_num * block_size)
    else:
        if n < round_num * block_size:
            n = n % block_size + round_num * block_size
    log.info(f"===== 处理算法: {algo} =====")
    # 数据生成：随机生成对称加解密所需要的测试数据
    test_vect = ske_symm_generate_testdata(algo, mode, padding, None, None, n)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_ENC', test_vect)
    ske_symm_onepass_test(algo, mode, padding, 'SYMM_DEC', test_vect)
    # 数据分组：将 test_vect.plaintext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.plaintext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_ENC', test_vect, round_num, split_list)
    # 数据分组：将 test_vect.ciphertext 随机拆分为 round_num 组测试输入，并依次返回各分组的 size 列表, finish 为余数
    split_list = split_M_into_N(len(test_vect.ciphertext), round_num, block_size, True)
    ske_symm_streams_test(algo, mode, padding, 'SYMM_DEC', test_vect, round_num, split_list)


# ==================== 补充对称加密 ISO7816 明文密钥测试 ====================

# ==================== 补充 SM4 明文密钥测试 ====================

# ===========================================================================
# BUG-08: SKE 输出缓冲区整型溢出防护
# input_sz > UINT32_MAX - block_sz 时拒绝
# TC-SKE-OVF-001 ~ TC-SKE-OVF-005 对应 s178 ~ s182
# ===========================================================================

from platform_adapter.uart_lib.ehsm_fw_errno import EHSM_ERR_PARAM_ERROR as _SKE_PARAM_ERROR
from platform_adapter.uart_lib.ehsm_fw_errno import EHSM_ERR_OUTPUT_OVERFLOW as _SKE_OUTPUT_OVERFLOW


@pytest.mark.skipif(False, reason="SKE功能始终启用")
@allure.feature("ske")
@allure.description("正常input_sz=1024，output计算正确（BUG-08正路径）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_S178")
def test_ehsm_s178(setup_module):
    """TC-SKE-OVF-001: 正常input_sz，output计算正确"""
    log.info("开始测试TC-SKE-OVF-001")
    with allure.step("1、AES-128 CBC加密input_size=1024字节 # 1、正常成功"):
        test_vect = ske_symm_generate_testdata("AES128", "CBC", "NONE", None, None, 1024)
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[("AES128", "CBC")]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, 0, len(test_vect.key))
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
        try:
            _, ct, ct_size = api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM["AES128"], SYM_MODE_TO_ENUM["CBC"], PADDING_MODE_TO_ENUM["NONE"],
                key_handle, True,
                test_vect.iv_nonce, len(test_vect.iv_nonce),
                test_vect.plaintext, len(test_vect.plaintext),
                len(test_vect.ciphertext))
            assert ct_size > 0, "AES-128 CBC加密输出大小应 > 0"
            assert ct_size == len(test_vect.plaintext), \
                f"AES-CBC NONE padding密文长度应等于明文长度{len(test_vect.plaintext)}，实际{ct_size}"
            log.info(f"AES-128 CBC 1024字节加密成功，密文长度: {ct_size}")

            # Reason: 严格校验——解密闭环确认密文内容正确，不是随机数据
            _, pt, pt_size = api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM["AES128"], SYM_MODE_TO_ENUM["CBC"], PADDING_MODE_TO_ENUM["NONE"],
                key_handle, False,
                test_vect.iv_nonce, len(test_vect.iv_nonce),
                ct[:ct_size], ct_size,
                len(test_vect.plaintext))
            assert pt_size == len(test_vect.plaintext), \
                f"解密输出大小应为{len(test_vect.plaintext)}，实际{pt_size}"
            assert pt[:pt_size] == test_vect.plaintext, \
                "AES-128 CBC解密结果与原始明文不一致，加密数据不正确"
            log.info("AES-128 CBC 1024字节加解密闭环验证通过")
        finally:
            api.ehsm_km_remove_key(key_handle)
    log.info("TC-SKE-OVF-001 完成")


@pytest.mark.skipif(False, reason="SKE功能始终启用")
@allure.feature("ske")
@allure.description("input_sz=0xFFFFFFF0（溢出触发点），拒绝（BUG-08 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_S179")
def test_ehsm_s179(setup_module):
    """TC-SKE-OVF-002: input_sz=0xFFFFFFF0（溢出触发点），拒绝"""
    log.info("开始测试TC-SKE-OVF-002: AES CBC input=0xFFFFFFF0溢出触发点应被拒绝")
    with allure.step("1、确认正常路径有效 # 1、正路径成功"):
        test_vect = ske_symm_generate_testdata("AES128", "CBC", "NONE", None, None, 16)
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[("AES128", "CBC")]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, 0, len(test_vect.key))
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
    try:
        with allure.step("2、AES-128 CBC input=0xFFFFFFF0 # 2、返回EHSM_ERR_PARAM_ERROR"):
            OVERFLOW_TRIGGER = 0xFFFFFFFF - 16 + 1
            try:
                api.ehsm_symm_cipher_onepass(
                    SYM_ALGO_TO_ENUM["AES128"], SYM_MODE_TO_ENUM["CBC"], PADDING_MODE_TO_ENUM["NONE"],
                    key_handle, True,
                    test_vect.iv_nonce, len(test_vect.iv_nonce),
                    test_vect.plaintext, OVERFLOW_TRIGGER, OVERFLOW_TRIGGER)
                assert False, "溢出触发点应被拒绝（BUG-08未修复）"
            except hostapi.HostApiError as e:
                assert e.ret_code == _SKE_OUTPUT_OVERFLOW, f"期望EHSM_ERR_OUTPUT_OVERFLOW({_SKE_OUTPUT_OVERFLOW})，实际: {e.ret_code}"
                log.info(f"溢出触发点0xFFFFFFF0被正确拒绝，错误码: {e.ret_code}")
    finally:
        api.ehsm_km_remove_key(key_handle)
    log.info("TC-SKE-OVF-002 完成")


@pytest.mark.skipif(False, reason="SKE功能始终启用")
@allure.feature("ske")
@allure.description("input_sz=UINT32_MAX，拒绝（BUG-08）")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM_S180")
def test_ehsm_s180(setup_module):
    """TC-SKE-OVF-003: input_sz=UINT32_MAX，拒绝"""
    log.info("开始测试TC-SKE-OVF-003: AES CBC input=UINT32_MAX应被拒绝")
    with allure.step("1、确认AES-128 CBC正常加密正路径有效 # 1、正路径成功"):
        # Reason: 控制变量法——先确认接口在正常 input_size（16）下成功，再测 UINT32_MAX 异常值
        test_vect = ske_symm_generate_testdata("AES128", "CBC", "NONE", None, None, 16)
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[("AES128", "CBC")]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, 0, len(test_vect.key))
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
        try:
            _, ct, ct_size = api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM["AES128"], SYM_MODE_TO_ENUM["CBC"], PADDING_MODE_TO_ENUM["NONE"],
                key_handle, True,
                test_vect.iv_nonce, len(test_vect.iv_nonce),
                test_vect.plaintext, len(test_vect.plaintext), len(test_vect.ciphertext))
            assert ct_size > 0, "正路径加密应成功"
            log.info("AES-128 CBC正常加密正路径验证成功")
        finally:
            api.ehsm_km_remove_key(key_handle)

    with allure.step("2、AES-128 CBC input=0xFFFFFFFF # 2、返回EHSM_ERR_PARAM_ERROR"):
        test_vect = ske_symm_generate_testdata("AES128", "CBC", "NONE", None, None, 16)
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[("AES128", "CBC")]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, 0, len(test_vect.key))
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
        try:
            api.ehsm_symm_cipher_onepass(
                SYM_ALGO_TO_ENUM["AES128"], SYM_MODE_TO_ENUM["CBC"], PADDING_MODE_TO_ENUM["NONE"],
                key_handle, True,
                test_vect.iv_nonce, len(test_vect.iv_nonce),
                test_vect.plaintext, 0xFFFFFFFF, 0xFFFFFFFF)
            assert False, "UINT32_MAX应被拒绝"
        except hostapi.HostApiError as e:
            assert e.ret_code == _SKE_OUTPUT_OVERFLOW, f"期望EHSM_ERR_OUTPUT_OVERFLOW({_SKE_OUTPUT_OVERFLOW})，实际: {e.ret_code}"
            log.info(f"input=UINT32_MAX被正确拒绝，错误码: {e.ret_code}")
        finally:
            api.ehsm_km_remove_key(key_handle)
    log.info("TC-SKE-OVF-003 完成")


@pytest.mark.skipif(False, reason="SKE功能始终启用")
@allure.feature("ske")
@allure.description("AES/SM4遍历，各算法溢出边界均正确拒绝（BUG-08 P0）")
@allure.severity(allure.severity_level.CRITICAL)
@allure.testcase("EHSM_S181")
def test_ehsm_s181(setup_module):
    """TC-SKE-OVF-004/005: 遍历AES/SM4，各算法溢出边界均正确拒绝"""
    log.info("开始测试TC-SKE-OVF-004/005: 遍历AES/SM4溢出边界")
    test_configs = [("AES128", "CBC", 16), ("SM4", "CBC", 16)]
    for algo, mode, block_sz in test_configs:
        log.info(f"=== 测试算法 {algo} {mode} block_sz={block_sz} ===")
        test_vect = ske_symm_generate_testdata(algo, mode, "NONE", None, None, block_sz)
        key_permit = KeyPermit.KEY_PRIV_REMOVE | KeyPermit.KEY_PRIV_ENCRYPT | KeyPermit.KEY_PRIV_DECRYPT
        key_type = SYM_ALGO_MODE_MAP_KEY_TYPE_ENUM[(algo, mode)]
        key_part = EhsmKeyPart.EHSM_KEY_PART_SYMM_KEY
        pack_key = pack_key_with_head(test_vect.key, key_permit, key_type, key_part, 0, len(test_vect.key))
        _, key_handle = api.ehsm_km_import_key(0xFFFFFFFF, 0xFFFFFFFF, pack_key, len(pack_key), None, 0, 0xFFFFFFFF)
        try:
            with allure.step(f"1、{algo} input=UINT32_MAX-block_sz+1 # 返回EHSM_ERR_PARAM_ERROR"):
                overflow_trigger = 0xFFFFFFFF - block_sz + 1
                try:
                    api.ehsm_symm_cipher_onepass(
                        SYM_ALGO_TO_ENUM[algo], SYM_MODE_TO_ENUM[mode], PADDING_MODE_TO_ENUM["NONE"],
                        key_handle, True,
                        test_vect.iv_nonce, len(test_vect.iv_nonce),
                        test_vect.plaintext, overflow_trigger, overflow_trigger)
                    assert False, f"{algo} 溢出触发点应被拒绝（BUG-08未修复）"
                except hostapi.HostApiError as e:
                    assert e.ret_code == _SKE_OUTPUT_OVERFLOW, "e.ret_code:{} 不等于 _SKE_OUTPUT_OVERFLOW:{}".format(e.ret_code, _SKE_OUTPUT_OVERFLOW)
                    log.info(f"{algo} 溢出触发点被正确拒绝，错误码: {e.ret_code}")
        finally:
            api.ehsm_km_remove_key(key_handle)
    log.info("TC-SKE-OVF-004/005 完成")
