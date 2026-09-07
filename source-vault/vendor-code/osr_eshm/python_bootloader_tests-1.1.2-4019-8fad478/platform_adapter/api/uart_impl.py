
import logging as log
from sys import api_version
from typing import Optional
import struct
import platform_adapter.uart_lib.ehsm_bl_errno as bl_errno

import pytest
from .constants import (
    EhsmAeadMode,
    EhsmAuthAlgo,
    EhsmBlGenKeyType,
    EhsmChallengeType,
    EhsmCipherMode,
    EhsmCtrlField,
    EhsmDrvMode,
    EhsmInstallKeyType,
    EhsmKeyLevel,
    EhsmKeyType,
    EhsmLifecycle,
    EhsmPaddingMode,
    KeyPermit,
    EhsmSymmAlgo,
    EhsmMacMode,
    EhsmHashAlgo,
    EhsmRsaPaddingMode,
    EhsmSm9EncType,
    EhsmSm9PaddingMode,
    EhsmRngAlgo,
    EhsmDeriveAlgo,
    EhsmDeriveType,
    EhsmPqcSignMode,
    EhsmPqcSignAlgo,
    EhsmPqcHashAlgo,
    EhsmPqcOutType,
)
from utils.util import api
from .base import ApiInterface
from platform_adapter.uart_lib import hostapi
import allure
from platform_adapter.host.loader import get_host_interface

host = get_host_interface()


class UartApi(ApiInterface):

    @api
    @allure.step("AEAD加密计算，一次计算完成")
    def ehsm_aead_onepass_enc(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_handle: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        input: bytes,
        input_size: int,
        tag_size: int,
        skip_read_output: bool = False
    ) -> tuple[int, bytes, bytes]:
        aad_addr = 0
        input_addr = 0
        iv_addr = 0
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
            iv_addr = self.DATA1_ADDR
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        if input is not None:
            if input != b'':
                host.write_memory(self.DATA4_ADDR, input)
            input_addr = self.DATA4_ADDR
        t = hostapi.ehsm_aead_onepass_enc(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            iv_addr,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            self.DATA4_ADDR,
            self.DATA3_ADDR,
            tag_size
        )
        # Reason: 性能测试时跳过大数据读取，避免通信开销
        if input_size != 0 and not skip_read_output:
            _, output1 = host.read_memory(self.DATA4_ADDR, input_size)
        else:
            output1 = b''
        _, output2 = host.read_memory(self.DATA3_ADDR, tag_size)
        return t, output1, output2

    @api
    @allure.step("AEAD解密计算，一次计算完成")
    def ehsm_aead_onepass_dec(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_handle: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int,
        skip_read_output: bool = False
    ) -> tuple[int, bytes, bool]:
        aad_addr = 0
        input_addr = 0
        iv_addr = 0
        tag_addr = 0
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
            iv_addr = self.DATA1_ADDR
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        if input is not None:
            if input != b'':
                host.write_memory(self.DATA4_ADDR, input)
            input_addr = self.DATA4_ADDR
        if tag:
            host.write_memory(self.DATA3_ADDR, tag)
            tag_addr = self.DATA3_ADDR
        t, ret_verify_result = hostapi.ehsm_aead_onepass_dec(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            iv_addr,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            self.DATA4_ADDR,
            tag_addr,
            tag_size
        )
        # Reason: 性能测试时跳过大数据读取，避免通信开销
        if input_size != 0 and not skip_read_output:
            _, output = host.read_memory(self.DATA4_ADDR, input_size)
        else:
            output = b''
        return t, output, ret_verify_result

    @api
    @allure.step("AEAD加密/解密计算，初始化")
    def ehsm_aead_init(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_handle: int,
        enc: bool,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        data_size: int,
        tag_size: int,
        session: int
    ) -> tuple[int, int]:
        iv_addr = 0
        session_addr = 0
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
            iv_addr = self.DATA1_ADDR
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        if session is None:
            session_addr = self.SESSION_ADDR
        t = hostapi.ehsm_aead_init(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            enc,
            iv_addr,
            nonce_size,
            aad_addr,
            aad_size,
            data_size,
            tag_size,
            session_addr
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("AEAD加密/解密计算，更新数据")
    def ehsm_aead_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        input_addr = 0
        if input is not None:
            if input != b'' :
                host.write_memory(self.DATA1_ADDR, input)
            input_addr = self.DATA1_ADDR
        t = hostapi.ehsm_aead_update(
            self.CTX_ADDR,
            input_addr,
            input_size,
            self.DATA2_ADDR
        )
        if input_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, input_size)
        else:
            output = b''
        return t, output

    @api
    @allure.step("AEAD加密计算，验证结果")
    def ehsm_aead_finish_enc(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes, bytes]:
        input_addr = 0
        if input is not None:
            if input != b'':
                host.write_memory(self.DATA1_ADDR, input)
            input_addr = self.DATA1_ADDR
        t = hostapi.ehsm_aead_finish_enc(
            self.CTX_ADDR,
            input_addr,
            input_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR
        )
        if input_size != 0:
            _, output1 = host.read_memory(self.DATA2_ADDR, input_size)
        else:
            output1 = b''
        _, output2 = host.read_memory(self.DATA3_ADDR, 16)
        return t, output1, output2

    @api
    @allure.step("AEAD解密计算，验证结果")
    def ehsm_aead_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes
    ) -> tuple[int, bytes, bool]:
        if input and input_size != 0:
            host.write_memory(self.DATA1_ADDR, input)
        if tag:
            host.write_memory(self.DATA3_ADDR, tag)
        t, ret_verify_result = hostapi.ehsm_aead_finish_dec(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR
        )
        if input_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, input_size)
        else:
            output = b''
        return t, output, ret_verify_result

    @api
    @allure.step("AEAD加密计算，OnePass (使用明文密钥)")
    def ehsm_aead_onepass_enc_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_data: bytes,
        key_size: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        input_addr = 0
        key_addr = 0
        aad_addr = 0
        # 写入明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # 写入输入数据到 DATA3
        if input is not None:
            if input != b'':
                host.write_memory(self.DATA3_ADDR, input)
            input_addr = self.DATA3_ADDR
        # 调用底层接口
        t = hostapi.ehsm_aead_onepass_enc_with_plain_key(
            self.CTX_ADDR,
            algo,
            mode,
            key_addr,
            key_size,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            self.DATA6_ADDR,
            self.DATA7_ADDR,
            tag_size
        )
        # 读取输出数据
        cipher_text = b''
        if input and input_size != 0:
            _, cipher_text = host.read_memory(self.DATA6_ADDR, input_size)
        _, auth_tag = host.read_memory(self.DATA7_ADDR, tag_size)
        return t, cipher_text, auth_tag

    @api
    @allure.step("AEAD解密计算，OnePass (使用明文密钥)")
    def ehsm_aead_onepass_dec_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_data: bytes,
        key_size: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        key_addr = 0
        # 写入明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # 写入输入数据到 DATA3
        input_addr = 0
        if input is not None:
            if input != b'':
                host.write_memory(self.DATA3_ADDR, input)
            input_addr = self.DATA3_ADDR
        # 写入认证标签到 DATA7
        tag_addr = 0
        if tag is not None:
            if tag != b'':
                host.write_memory(self.DATA7_ADDR, tag)
            tag_addr = self.DATA7_ADDR
        # 调用底层接口
        t, verify_result = hostapi.ehsm_aead_onepass_dec_with_plain_key(
            self.CTX_ADDR,
            algo,
            mode,
            key_addr,
            key_size,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            input_addr,
            input_size,
            self.DATA6_ADDR,
            tag_addr,
            tag_size
        )
        # 读取输出数据
        plain_text = b''
        if input and input_size != 0:
            _, plain_text = host.read_memory(self.DATA6_ADDR, input_size)
        return t, plain_text, verify_result

    @api
    @allure.step("AEAD加密/解密计算，初始化 (使用明文密钥)")
    def ehsm_aead_init_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmAeadMode,
        key_data: bytes,
        key_size: int,
        enc: bool,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        data_size: int,
        tag_size: int,
        session: int
    ) -> tuple[int, int]:
        # 写入明文密钥到 DATA5
        session_addr = 0
        key_addr = 0
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        if session is None:
            session_addr = self.SESSION_ADDR
        # 调用底层接口
        t = hostapi.ehsm_aead_init_with_plain_key(
            self.CTX_ADDR,
            algo,
            mode,
            key_addr,
            key_size,
            enc,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            data_size,
            tag_size,
            session_addr
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("ChaCha加密计算，OnePass (使用明文密钥)")
    def ehsm_chacha_onepass_enc_with_plain_key(
        self,
        key_data: bytes,
        key_size: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        # 写入明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # 写入输入数据到 DATA3
        if input and input_size != 0:
            host.write_memory(self.DATA3_ADDR, input)
        # 调用底层接口
        t = hostapi.ehsm_chacha_onepass_enc_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            key_size,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            self.DATA3_ADDR,
            input_size,
            self.DATA6_ADDR,
            self.DATA7_ADDR,
            tag_size
        )
        # 读取输出数据
        _, cipher_text = host.read_memory(self.DATA6_ADDR, input_size)
        _, auth_tag = host.read_memory(self.DATA7_ADDR, tag_size)
        return t, cipher_text, auth_tag

    @api
    @allure.step("ChaCha解密计算，OnePass (使用明文密钥)")
    def ehsm_chacha_onepass_dec_with_plain_key(
        self,
        key_data: bytes,
        key_size: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        # 写入明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # 写入输入数据到 DATA3
        if input and input_size != 0:
            host.write_memory(self.DATA3_ADDR, input)
        # 写入认证标签到 DATA7
        if tag:
            host.write_memory(self.DATA7_ADDR, tag)
        # 调用底层接口
        t = hostapi.ehsm_chacha_onepass_dec_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            key_size,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            self.DATA3_ADDR,
            input_size,
            self.DATA6_ADDR,
            self.DATA7_ADDR,
            tag_size
        )
        # 读取输出数据和认证结果
        _, plain_text = host.read_memory(self.DATA6_ADDR, input_size)
        _, auth_result_bytes = host.read_memory(self.DATA8_ADDR, 4)
        auth_result = struct.unpack("<I", auth_result_bytes)[0] == 1
        return t, plain_text, auth_result

    @api
    @allure.step("ChaCha加密/解密计算，初始化 (使用明文密钥)")
    def ehsm_chacha_init_with_plain_key(
        self,
        key_data: bytes,
        key_size: int,
        enc: bool,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        session: int
    ) -> tuple[int, int]:
        # 写入明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
        # 写入 nonce 到 DATA1
        if nonce:
            host.write_memory(self.DATA1_ADDR, nonce)
        # 写入 AAD 到 DATA2
        aad_addr = 0
        if aad:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # 调用底层接口
        t = hostapi.ehsm_chacha_init_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            key_size,
            enc,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("BL下关闭调试功能")
    def ehsm_bl_close_debug(
        self,
        challenge_type: EhsmChallengeType,
        soc_dbg_bitmap: Optional[bytes]
        ) -> int:
        bitmap_addr = 0
        if soc_dbg_bitmap is not None:
            host.write_memory(self.DATA1_ADDR, soc_dbg_bitmap)
            bitmap_addr = self.DATA1_ADDR

        t = hostapi.ehsm_bl_close_debug(self.CTX_ADDR, int(challenge_type), bitmap_addr)
        return t


    @api
    @allure.step(
        "调试鉴权：类型 {challenge_type}，算法 {algo}，签名值 {sig}， 公钥 {pub_key}， soc maps {soc_dbg_bitmap}"
    )
    def ehsm_bl_debug_auth(
        self,
        challenge_type: EhsmChallengeType,
        algo: EhsmAuthAlgo,
        sig: Optional[bytes],
        sig_size: int,
        pub_key: Optional[bytes],
        pub_key_size: int,
        soc_dbg_bitmap: Optional[bytes],
    ) -> int:
        # Reason: 支持sig=None用于异常参数测试
        sig_addr = 0
        if sig is not None:
            host.write_memory(self.DATA1_ADDR, sig)
            sig_addr = self.DATA1_ADDR

        pub_key_addr = 0
        if pub_key is not None:
            host.write_memory(self.DATA2_ADDR, pub_key)
            pub_key_addr = self.DATA2_ADDR

        bitmap_addr = 0
        if soc_dbg_bitmap is not None:
            host.write_memory(self.DATA3_ADDR, soc_dbg_bitmap)
            bitmap_addr = self.DATA3_ADDR

        t = hostapi.ehsm_bl_debug_auth(
            self.CTX_ADDR,
            challenge_type,
            algo,
            sig_addr,
            sig_size,
            pub_key_addr,
            pub_key_size,
            bitmap_addr
        )
        return t

    @api
    @allure.step("加密外部提供的密钥值，输出被对应ROOT KEY加密的密钥值及相应的CRC")
    def ehsm_bl_encrypt_key(
        self,
        key_level: EhsmKeyLevel,
        input_data: bytes,
        size: int
    ) -> tuple[int, bytes]:
        if input_data == None:
            input_data_addr = 0
        else:
            host.write_memory(self.DATA1_ADDR, input_data)
            input_data_addr = self.DATA1_ADDR

        t, ret = hostapi.ehsm_bl_encrypt_key(
            self.CTX_ADDR,
            key_level,
            input_data_addr,
            size,
            self.DATA2_ADDR
        )

        _, output = host.read_memory(self.DATA2_ADDR, ret)
        return t,output


    @api
    @allure.step("国密固件认证")
    def ehsm_bl_fw_auth(
        self,
        type: int,
        arg: bytes,
        auth_data: bytes
    ) -> tuple[int, bytes]:
        auth_addr = 0
        if auth_data:
            host.write_memory(self.DATA1_ADDR, auth_data)
            auth_addr = self.DATA1_ADDR
        # 如果arg为空，则不写入空值
        if arg != b'':
            host.write_memory(self.DATA2_ADDR, arg)
        t, ret = hostapi.ehsm_bl_fw_auth(
            self.CTX_ADDR,
            type,
            self.DATA2_ADDR,
            auth_addr,
            self.DATA3_ADDR
            )

        _, output = host.read_memory(self.DATA3_ADDR, ret)
        return t, output


    @api
    @allure.step("获取挑战字类型 {challenge_type}")
    def ehsm_bl_get_challenge(
        self,
        challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_bl_get_challenge(
            self.CTX_ADDR,
            int(challenge_type),
            self.DATA1_ADDR
        )

        _, output = host.read_memory(self.DATA1_ADDR, ret)
        return t, output


    @api
    @allure.step("生成可写入OTP的被ROOT KEY加密的密钥值及相应的CRC")
    def ehsm_bl_get_random_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmBlGenKeyType
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_bl_get_random_key(
            self.CTX_ADDR,
            key_level,
            key_type,
            self.DATA1_ADDR
        )
        _, output = host.read_memory(self.DATA1_ADDR, ret)
        return t, output


    @api
    @allure.step("查询算法自检结果")
    def ehsm_bl_get_self_test_result(self) -> tuple[int, bytes]:
        t,ret = hostapi.ehsm_bl_get_self_test_result(self.CTX_ADDR, self.DATA1_ADDR)
        _, output = host.read_memory(self.DATA1_ADDR, ret)
        return t, output


    @api
    @allure.step("bl下获取SOCID")
    def ehsm_bl_get_socid(self) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_bl_get_socid(self.CTX_ADDR, self.DATA1_ADDR)
        _, output = host.read_memory(self.DATA1_ADDR, ret)
        return t, output


    @api
    @allure.step("获取BootLoader的版本号")
    def ehsm_bl_get_version(self) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_bl_get_version(self.CTX_ADDR, self.DATA1_ADDR)
        _, output = host.read_memory(self.DATA1_ADDR, ret)
        return t, output

    @api
    @allure.step("bl下向 eHSM 注入错误并可在 SOC 端观察到对应的错误信号")
    def ehsm_bl_inject_error(self, values: bytes) -> int:
        host.write_memory(self.DATA1_ADDR, values)
        t = hostapi.ehsm_bl_inject_error(self.CTX_ADDR, self.DATA1_ADDR)
        return t


    @api
    @allure.step("bl下读取OTP数据")
    def ehsm_bl_read_otp(
        self,
        ehsm_src_addr: int,
        size: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_bl_read_otp(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_src_addr,
            size
        )

        _, output = host.read_memory(self.DATA1_ADDR, size)
        return t, output


    @api
    @allure.step("bl下读取CFG地址范围的寄存器值")
    def ehsm_bl_read_reg(
        self,
        ehsm_src_addr: int,
        size: int
        ) -> tuple[int, bytes]:

        t = hostapi.ehsm_bl_read_reg(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_src_addr,
            size
        )
        _, output = host.read_memory(self.DATA1_ADDR, size)

        return t, output


    @api
    @allure.step("bl下启动算法自检")
    def ehsm_bl_self_test(self) -> int:
        t = hostapi.ehsm_bl_self_test(self.CTX_ADDR)
        return t


    @api
    @allure.step("bl下设置串口波特率")
    def ehsm_bl_set_uart_baudrate(self, baud_div: int) -> int:

        t = hostapi.ehsm_bl_set_uart_baudrate(self.CTX_ADDR, baud_div)
        return t

    @api
    @allure.step("bl下设置HSM频率")
    def ehsm_bl_set_hsm_freq(self, hsm_freq: int) -> int:
        t = hostapi.ehsm_bl_set_hsm_freq(self.CTX_ADDR, hsm_freq)
        return t

    @api
    @allure.step("bl下升级固件镜像")
    def ehsm_bl_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, image)
        t = hostapi.ehsm_bl_upgrade_fw_image(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            image_size,
            self.DATA3_ADDR
        )
        _, output = host.read_memory(self.DATA3_ADDR, image_size - 1024)
        return t, output

    @api
    @allure.step("bl下校验安全启动镜像")
    def ehsm_bl_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, image)
        t = hostapi.ehsm_bl_verify_image(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            image_size,
            check_version,
            boot,
            self.DATA3_ADDR
        )
        _, output = host.read_memory(self.DATA3_ADDR, image_size)
        return t, output

    @api
    @allure.step("bl下校验安全启动镜像(镜像头和代码分离)")
    def ehsm_bl_verify_image_discrete(
        self,
        image_header: bytes,
        image_size: int,
        image_code: Optional[bytes] = None,
        only_copy_code: bool = False,
        check_version: bool = True,
        boot: bool = False
    ) -> tuple[int, bytes]:
        """
        BL下校验安全启动镜像（镜像头和代码分离）

        Args:
            image_header: 镜像头数据（1024字节）
            image_size: 镜像总大小（头部+代码）
            image_code: 代码区数据（None表示紧跟镜像头后面）
            only_copy_code: 是否只复制代码区
            check_version: 是否检查版本
            boot: 校验成功后是否启动（仅对eHSM镜像有效）

        Returns:
            (校验耗时(us), 输出镜像数据)
        """
        # Reason: 写入镜像头到 DATA1_ADDR
        host.write_memory(self.DATA1_ADDR, image_header)

        # Reason: 写入代码区（如果单独提供）
        code_addr = 0
        if image_code is not None:
            host.write_memory(self.DATA2_ADDR, image_code)
            code_addr = self.DATA2_ADDR

        # 调用底层API
        vrf_time = hostapi.ehsm_bl_verify_image_discrete(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            image_size,
            code_addr,
            only_copy_code,
            check_version,
            boot,
            self.DATA3_ADDR
        )

        # Reason: 读取输出镜像（根据only_copy_code决定读取大小）
        if only_copy_code:
            output_size = image_size - 1024  # 只输出代码区
        else:
            output_size = image_size  # 输出完整镜像

        _, output = host.read_memory(self.DATA3_ADDR, output_size)

        return vrf_time, output

    @api
    @allure.step("bl下写入OTP数据")
    def ehsm_bl_write_otp(
        self,
        src_data: bytes,
        ehsm_dest_addr: int,
        size: int
        ) -> int:
        host.write_memory(self.DATA1_ADDR, src_data)

        t = hostapi.ehsm_bl_write_otp(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_dest_addr,
            size
        )
        return t


    @api
    @allure.step("bl下写入CFG地址范围的寄存器值")
    def ehsm_bl_write_reg(
        self,
        src_data: bytes,
        ehsm_dest_addr: int,
        size: int
        ) -> int:
        host.write_memory(self.DATA1_ADDR, src_data)

        t = hostapi.ehsm_bl_write_reg(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_dest_addr,
            size
        )
        return t

    @api
    @allure.step("CHACHA加密计算，一次计算完成")
    def ehsm_chacha_onepass_enc(
        self,
        key_handle: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        aad_addr = 0
        host.write_memory(self.DATA1_ADDR, nonce)
        if aad is not None:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        host.write_memory(self.DATA3_ADDR, input)
        t = hostapi.ehsm_chacha_onepass_enc(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            self.DATA3_ADDR,
            input_size,
            self.DATA4_ADDR,
            self.DATA5_ADDR,
            tag_size,
        )
        _, output1 = host.read_memory(self.DATA4_ADDR, input_size)
        _, output2 = host.read_memory(self.DATA5_ADDR, tag_size)
        return t, output1, output2

    @api
    @allure.step("CHACHA解密计算，一次计算完成")
    def ehsm_chacha_onepass_dec(
        self,
        key_handle: int,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        aad_addr = 0
        host.write_memory(self.DATA1_ADDR, nonce)
        if aad is not None:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        host.write_memory(self.DATA3_ADDR, input)
        host.write_memory(self.DATA4_ADDR, tag)
        t, verify_result = hostapi.ehsm_chacha_onepass_dec(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            self.DATA3_ADDR,
            input_size,
            self.DATA4_ADDR,
            tag_size,
            self.DATA5_ADDR
        )
        _, output1 = host.read_memory(self.DATA5_ADDR, input_size)
        return t, output1, bool(verify_result)

    @api
    @allure.step("CHACHA加密/解密计算，初始化")
    def ehsm_chacha_init(
        self,
        key_handle: int,
        enc: bool,
        nonce: bytes,
        nonce_size: int,
        aad: bytes,
        aad_size: int,
        constant: int,
        session: bytes
    ) -> tuple[int, bytes]:
        aad_addr = 0
        session_addr = 0
        host.write_memory(self.DATA1_ADDR, nonce)
        if aad is not None:
            host.write_memory(self.DATA2_ADDR, aad)
            aad_addr = self.DATA2_ADDR
        # Reason: session is an output buffer; only write if caller provides existing session data
        if session is None:
            session_addr = self.SESSION_ADDR
        t, ret = hostapi.ehsm_chacha_init(
            self.CTX_ADDR,
            key_handle,
            enc,
            self.DATA1_ADDR,
            nonce_size,
            aad_addr,
            aad_size,
            constant,
            session_addr
        )
        _, output = host.read_memory(self.SESSION_ADDR, 4)
        return t, output

    @api
    @allure.step("CHACHA加密/解密计算，更新数据")
    def ehsm_chacha_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, input)

        t = hostapi.ehsm_chacha_update(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR
        )
        _, output = host.read_memory(self.DATA2_ADDR, input_size)
        return t, output

    @api
    @allure.step("bl下CHACHA加密计算，验证结果")
    def ehsm_chacha_finish_enc(
        self,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        host.write_memory(self.DATA1_ADDR, input)
        t = hostapi.ehsm_chacha_finish_enc(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR,
            tag_size
        )
        _, output1 = host.read_memory(self.DATA2_ADDR, input_size)
        _, output2 = host.read_memory(self.DATA3_ADDR, tag_size)
        return t, output1, output2

    @api
    @allure.step("bl下CHACHA解密计算，验证结果")
    def ehsm_chacha_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        host.write_memory(self.DATA1_ADDR, input)
        host.write_memory(self.DATA2_ADDR, tag)
        t, verify_result = hostapi.ehsm_chacha_finish_dec(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR,
            tag_size,
            self.DATA3_ADDR
        )
        _, output1 = host.read_memory(self.DATA3_ADDR, input_size)
        return t, output1, bool(verify_result)

    @api
    @allure.step("修改OTP控制字段的值")
    def ehsm_change_control_field(
        self,
        field: EhsmCtrlField,
        value: bytes
    ) -> int:
        value_addr = 0
        if value is not None:
            host.write_memory(self.DATA1_ADDR,value)
            value_addr = self.DATA1_ADDR
        t = hostapi.ehsm_change_control_field(self.CTX_ADDR, field, value_addr)
        return t


    @api
    @allure.step("更改 eHSM 生命周期")
    def ehsm_change_lifecycle(
        self,
        lifecycle: EhsmLifecycle
    ) -> int:
        t = hostapi.ehsm_change_lifecycle(self.CTX_ADDR, lifecycle)
        return t


    @api
    @allure.step("关闭调试")
    def ehsm_close_debug(
        self,
        type: EhsmChallengeType,
        soc_dbg_bitmap: Optional[bytes]
    ) -> int:
        bitmap = 0
        if soc_dbg_bitmap is not None:
            host.write_memory(self.DATA1_ADDR, soc_dbg_bitmap)
            bitmap = self.DATA1_ADDR

        t = hostapi.ehsm_close_debug(
            self.CTX_ADDR,
            type,
            bitmap
        )
        return t

    @api
    @allure.step("创建（使能）一个计数器")
    def ehsm_create_counter(self) -> tuple[int, bytes, bytes]:
        t, ret1, ret2 = hostapi.ehsm_create_counter(self.CTX_ADDR, self.DATA1_ADDR, self.DATA2_ADDR)

        return t, ret1, ret2


    @api
    @allure.step("初始化 API 调用的context")
    def ehsm_ctx_init(
        self,
        mb_ch: int,
        is_async: bool
    ) -> None:
        hostapi.init_ctx(self.CTX_ADDR, mb_ch, is_async)

    @api
    @allure.step("去初始化 UART")
    def ehsm_ctx_deinit(self) -> None:
        hostapi.deinit_ctx()

    @api
    @allure.step("异步模式下，查询之前调用的任务是否已完成")
    def ehsm_ctx_poll(self) -> int:
        t = hostapi.ehsm_ctx_poll(self.CTX_ADDR)
        return t

    @api
    @allure.step("完成鉴权并进行相应的调试配置操作")
    def ehsm_debug_auth(
        self,
        challenge_type: EhsmChallengeType,
        algo: EhsmAuthAlgo,
        sig: bytes,
        sig_size: int,
        pub_key: bytes,
        pub_key_size: int,
        soc_dbg_bitmap: Optional[bytes],
    ) -> int:
        pubkey_addr = 0
        bitmap = 0
        if pub_key is not None:
            host.write_memory(self.DATA1_ADDR, pub_key)
            pubkey_addr = self.DATA1_ADDR
        if soc_dbg_bitmap is not None:
            host.write_memory(self.DATA2_ADDR, soc_dbg_bitmap)
            bitmap = self.DATA2_ADDR
        host.write_memory(self.DATA3_ADDR, sig)

        t = hostapi.ehsm_debug_auth(
            self.CTX_ADDR,
            challenge_type,
            algo,
            self.DATA3_ADDR,
            sig_size,
            pubkey_addr,
            pub_key_size,
            bitmap
        )
        return t

    def ehsm_delete_counter(
        self,
        counter_id: int
        ) -> int:
        t,ret = hostapi.ehsm_delete_counter(
            self.CTX_ADDR,
            counter_id
        )
        return t,ret


    def ehsm_driver_get_version(self) -> int:
        pass

    @api
    @allure.step("驱动初始化")
    def ehsm_driver_init_library(self, drv_mode: EhsmDrvMode) -> int:
        t = hostapi.ehsm_driver_init_library(drv_mode)

        return t

    def ehsm_ecdsa_finish_gen(self,
        sig: bytes,
        sig_size: int
        ) -> tuple[int, bytes]:
        host.write_memory(self.DATA2_ADDR, sig_size.to_bytes(4, 'little'))
        t, ret_sig_size = hostapi.ehsm_ecdsa_finish_gen(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            self.DATA2_ADDR
        )
        _, output = host.read_memory(self.DATA1_ADDR, ret_sig_size)
        return t, output

    def ehsm_ecdsa_finish_verify(
        self,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, sig)
        t, verify_result = hostapi.ehsm_ecdsa_finish_verify(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            sig_size
        )
        return t, verify_result

    def ehsm_ecdsa_init(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        gen_sig: int,
        session: bytes
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_ecdsa_init(
            self.CTX_ADDR,
            algo,
            key_handle,
            gen_sig,
            self.DATA1_ADDR
        )
        _,output = host.read_memory(self.DATA1_ADDR, ret)
        return t ,ret,output

    @api
    @allure.step("ECDSA签名值生成，使用消息一次计算完成")
    def ehsm_ecdsa_onepass_gen(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int,
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, msg)
        host.write_memory(self.DATA3_ADDR, sig_size.to_bytes(4, 'little'))
        t, ret = hostapi.ehsm_ecdsa_onepass_gen(
            self.CTX_ADDR,
            algo,
            key_handle,
            self.DATA1_ADDR,
            msg_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR
        )
        _, output = host.read_memory(self.DATA2_ADDR, ret)
        return t, output

    @api
    @allure.step("ECDSA验证签名值，使用消息一次计算完成")
    def ehsm_ecdsa_onepass_verify(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, msg)
        host.write_memory(self.DATA2_ADDR, sig)
        t, verify_result = hostapi.ehsm_ecdsa_onepass_verify(
            self.CTX_ADDR,
            algo,
            key_handle,
            self.DATA1_ADDR,
            msg_size,
            self.DATA2_ADDR,
            sig_size,
        )
        return t, bool(verify_result)

    @api
    @allure.step("ECDSA签名值生成，使用 digest 一次计算完成")
    def ehsm_ecdsa_onepass_gen_with_digest(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int,
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, digest)
        host.write_memory(self.DATA3_ADDR, sig_size.to_bytes(4, 'little'))
        t, ret = hostapi.ehsm_ecdsa_onepass_gen_with_digest(
            self.CTX_ADDR,
            algo,
            key_handle,
            self.DATA1_ADDR,
            digest_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR
        )
        _, output = host.read_memory(self.DATA2_ADDR, ret)
        return t, output

    @api
    @allure.step("ECDSA验证签名值，使用 digest 一次计算完成")
    def ehsm_ecdsa_onepass_verify_with_digest(
        self,
        hash_algo: EhsmHashAlgo,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, digest)
        host.write_memory(self.DATA2_ADDR, sig)
        t, verify_result = hostapi.ehsm_ecdsa_onepass_verify_with_digest(
            self.CTX_ADDR,
            hash_algo,
            key_handle,
            self.DATA1_ADDR,
            digest_size,
            self.DATA2_ADDR,
            sig_size
        )
        return t, bool(verify_result)

    @api
    @allure.step("ECDSA签名/验签统一接口 (ex版本)")
    def ehsm_ecdsa_onepass_ex(
        self,
        algo: EhsmHashAlgo,
        use_plain_key: bool,
        key_handle: int,
        key_data: Optional[bytes],
        gen_sig: bool,
        is_digest: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        signature: Optional[bytes] = None
    ) -> tuple[int, bytes, int, bool]:
        """
        ECDSA签名/验签统一接口 (ex版本)

        Reason: 统一接口支持明文密钥/密钥句柄、签名/验签、原始消息/摘要的所有组合

        Args:
            signature: 验签模式下需要传入的签名数据
        """
        # 写入输入数据(消息或摘要)
        host.write_memory(self.DATA1_ADDR, input)

        # 验签模式: 写入签名数据
        sig_size = output_buff_size  # 默认使用输出缓冲区大小
        if not gen_sig and signature:
            host.write_memory(self.DATA3_ADDR, signature)
            sig_size = len(signature)  # Reason: 验签时sig_size应为实际签名长度

        # 写入明文密钥(如果使用明文密钥)
        key_addr = 0
        if use_plain_key and key_data:
            # Reason: 如果是 mb_ecc_key_st 结构(长度>20字节),需要重新打包地址
            if len(key_data) > 20:
                import struct
                # 解析结构头: curve_id(4) + privkey_addr(8) + pubkey_addr(8) = 20字节
                curve_id = struct.unpack("<L", key_data[0:4])[0]

                # ECC 曲线密钥大小映射表 (curve_id -> (privkey_size, pubkey_size))
                ecc_sizes = {
                    0x12: (20, 40), 0x13: (24, 48), 0x14: (28, 56), 0x15: (32, 64),
                    0x16: (40, 80), 0x17: (48, 96), 0x18: (64, 128), 0x19: (24, 48),
                    0x1a: (28, 56), 0x1b: (32, 64), 0x1c: (48, 96), 0x1d: (66, 132),
                    0x26: (20, 40), 0x27: (24, 48), 0x28: (28, 56), 0x29: (32, 64),
                }

                privkey_size, pubkey_size = ecc_sizes.get(curve_id, (32, 64))  # 默认P-256

                # Reason: 重新计算地址,基地址是DATA2_ADDR,数据从第20字节开始
                struct_size = 20
                data_base = self.DATA2_ADDR + struct_size

                privkey_addr = data_base
                pubkey_addr = privkey_addr + privkey_size

                # Reason: 重新打包结构体,使用新的地址
                new_struct_header = struct.pack(
                    "<LQQ",  # curve_id(4字节) + privkey_addr(8字节) + pubkey_addr(8字节)
                    curve_id,
                    privkey_addr,
                    pubkey_addr
                )

                # 重新组合: 结构头(20字节) + 实际密钥数据
                key_data = new_struct_header + key_data[20:]

            host.write_memory(self.DATA2_ADDR, key_data)
            key_addr = self.DATA2_ADDR

        # 调用 hostapi
        t, ret_size = hostapi.ehsm_ecdsa_onepass_ex(
            self.CTX_ADDR,
            algo,
            use_plain_key,
            key_handle,
            key_addr,
            gen_sig,
            is_digest,
            self.DATA1_ADDR,
            input_size,
            self.DATA3_ADDR,  # 签名输出地址/签名输入地址
            sig_size,         # 签名模式:输出缓冲区大小; 验签模式:输入签名实际长度
            self.DATA4_ADDR   # ���签结果地址
        )

        # 读取输出
        if gen_sig:
            # 签名操作: 读取签名
            _, sig = host.read_memory(self.DATA3_ADDR, ret_size)
            return t, sig, ret_size, False
        else:
            # 验签操作: ret_size是验签结果(0=失败,1=成功)
            verify_result = bool(ret_size)
            return t, b'', 0, verify_result

    def ehsm_ecdsa_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        host.write_memory(self.DATA1_ADDR, msg)
        t,ret = hostapi.ehsm_ecdsa_update(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            msg_size
        )
        return t,ret

    def ehsm_enter_wfi(self) -> int:
        t,ret = hostapi.ehsm_enter_wfi(self.CTX_ADDR)
        return t,ret

    def ehsm_gen_random(
        self,
        algo: EhsmRngAlgo,
        rand_buf: bytes,
        rand_size: int,
        skip_read_output: bool = False
    ) -> tuple[int, bytes]:
        rand_addr = 0
        if rand_buf:
            host.write_memory(self.DATA1_ADDR, rand_buf)
            rand_addr = self.DATA1_ADDR
        t, ret = hostapi.ehsm_gen_random(
            self.CTX_ADDR,
            algo,
            rand_addr,
            rand_size
        )
        # Reason: 性能测试时跳过大数据读取，避免通信开销
        if not skip_read_output:
            _, output = host.read_memory(self.DATA1_ADDR, rand_size)
        else:
            output = b''
        return t, output

    def ehsm_get_challenge(
        self,
        challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_get_challenge(
            self.CTX_ADDR,
            challenge_type,
            self.DATA1_ADDR
        )
        _, output = host.read_memory(self.DATA1_ADDR, 48)
        return t, output

    def ehsm_get_challenge_error(
        self,
        ctx_addr: int,
        challenge_type: EhsmChallengeType,
        output_addr: int
    ) -> tuple[int, bytes]:
        """
        用于测试异常参数的get_challenge接口，允许传入自定义ctx_addr和output_addr
        """
        t, ret = hostapi.ehsm_get_challenge(
            ctx_addr,
            challenge_type,
            output_addr
        )
        _, output = host.read_memory(output_addr, 48)
        return t, output

    def ehsm_get_emu_status(
        self,
        emu_addr:int = None
    ) -> tuple[int, bytes]:
        emu_address = 0
        if emu_addr == None:
            emu_address = self.DATA1_ADDR
        t, ret = hostapi.ehsm_get_emu_status(
            self.CTX_ADDR,
            emu_address
        )
        _, output = host.read_memory(emu_address, 28)
        return t, output

    def ehsm_get_utc_time(
        self,
        utc_time: bytes
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_get_utc_time(
            self.CTX_ADDR,
            self.DATA1_ADDR
        )
        _, output = host.read_memory(self.DATA1_ADDR, 4)
        return t, output

    def ehsm_get_version(self, version: bytes) -> tuple[int, bytes]:
        ver = 0
        if version:
            host.write_memory(self.DATA1_ADDR,version)
            ver = self.DATA1_ADDR
        t, ret = hostapi.ehsm_get_version(
            self.CTX_ADDR,
            ver
        )
        _, output = host.read_memory(ver, 128)
        return t, output

    def ehsm_hash_finish(
        self, digest: bytes, digest_size: int
    ) -> tuple[int, bytes, int]:
        t, ret_digest_size = hostapi.ehsm_hash_finish(
            self.CTX_ADDR,
            self.DATA2_ADDR,
            digest_size
        )
        _, digest = host.read_memory(self.DATA2_ADDR, ret_digest_size)
        return t, digest, ret_digest_size

    def ehsm_hash_init(
        self,
        algo: EhsmHashAlgo,
        session: bytes
    ) -> tuple[int, bytes]:
        session_addr = 0
        if session is None:
            session_addr = self.SESSION_ADDR
        t = hostapi.ehsm_hash_init(
            self.CTX_ADDR,
            int(algo),
            session_addr
        )
        _, output = host.read_memory(self.DATA1_ADDR, 12)
        return t, output

    def ehsm_hash_onepass(
        self,
        algo: EhsmHashAlgo,
        msg: bytes,
        msg_size: int,
        digest: bytes,
        digest_size: int
    ) -> tuple[int, bytes, int]:
        msg_addr = 0
        if msg is not None:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t, ret_digest_size = hostapi.ehsm_hash_onepass(
            self.CTX_ADDR,
            algo,
            msg_addr,
            msg_size,
            self.DATA2_ADDR,
            digest_size
        )
        _, output = host.read_memory(self.DATA2_ADDR, ret_digest_size)
        return t, output, ret_digest_size

    @api
    @allure.step("哈希更新")
    def ehsm_hash_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        msg_addr = 0
        if msg is not None:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t = hostapi.ehsm_hash_update(
            self.CTX_ADDR,
            msg_addr,
            msg_size
        )
        return t

    def ehsm_hmac_finish_gen(
        self,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_hmac_finish_gen(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            hmac_size
        )
        _, output = host.read_memory(self.DATA1_ADDR, hmac_size)
        return t, output

    def ehsm_hmac_finish_verify(
        self,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, hmac)
        t, ret_verify_result = hostapi.ehsm_hmac_finish_verify(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            hmac_size
        )
        return t, ret_verify_result

    def ehsm_hmac_init(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        gen_hmac: bool,
        session: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_hmac_init(
            # host.write_memory(self.DATA1_ADDR, session)
            self.CTX_ADDR,
            algo,
            key_handle,
            gen_hmac,
            self.SESSION_ADDR
        )
        _, output = host.read_memory(self.DATA1_ADDR, 12)
        return t, output

    def ehsm_hmac_onepass_gen(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bytes]:
        msg_addr = 0
        if msg is not None:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t = hostapi.ehsm_hmac_onepass_gen(
            self.CTX_ADDR,
            algo,
            key_handle,
            msg_addr,
            msg_size,
            self.DATA2_ADDR,
            hmac_size
        )
        _, output = host.read_memory(self.DATA2_ADDR, hmac_size)
        return t, output

    def ehsm_hmac_onepass_verify(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bool]:
        msg_addr = 0
        hmac_addr = 0
        if msg is not None:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        if hmac is not None:
            host.write_memory(self.DATA2_ADDR, hmac)
            hmac_addr = self.DATA2_ADDR
        t, ret_verify_result = hostapi.ehsm_hmac_onepass_verify(
            self.CTX_ADDR,
            algo,
            key_handle,
            msg_addr,
            msg_size,
            hmac_addr,
            hmac_size
        )
        assert 8 <= hmac_size
        return t, ret_verify_result

    def ehsm_hmac_update(self, msg: bytes, msg_size: int) -> int:
        msg_addr = 0
        if msg is not None:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t = hostapi.ehsm_hmac_update(
            self.CTX_ADDR,
            msg_addr,
            msg_size
        )
        return t

    @api
    @allure.step("HMAC计算生成，OnePass (使用明文密钥)")
    def ehsm_hmac_onepass_gen_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        key_size: int,
        msg: bytes,
        msg_size: int,
        hmac_size: int
    ) -> tuple[int, bytes]:
        # 写入明文密钥到 DATA5
        key_addr = 0
        msg_addr = 0
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入消息到 DATA1
        if msg:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        # 调用底层接口
        t = hostapi.ehsm_hmac_onepass_gen_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            key_addr,
            key_size,
            msg_addr,
            msg_size,
            self.DATA6_ADDR,
            hmac_size
        )
        # 读取 HMAC 结果
        _, hmac = host.read_memory(self.DATA6_ADDR, hmac_size)
        return t, hmac

    @api
    @allure.step("HMAC计算验证，OnePass (使用明文密钥)")
    def ehsm_hmac_onepass_verify_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        key_size: int,
        msg: bytes,
        msg_size: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bool]:
        # 写入明文密钥到 DATA5
        key_addr = 0
        msg_addr = 0
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入消息到 DATA1
        if msg:
            host.write_memory(self.DATA1_ADDR, msg)
        # 写入待验证的 HMAC 到 DATA2
        if hmac:
            host.write_memory(self.DATA2_ADDR, hmac)
            msg_addr = self.DATA1_ADDR
        # 调用底层接口
        t = hostapi.ehsm_hmac_onepass_verify_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            key_addr,
            key_size,
            msg_addr,
            msg_size,
            self.DATA2_ADDR,
            hmac_size,
            self.DATA8_ADDR
        )
        # 读取验证结果
        _, verify_result_bytes = host.read_memory(self.DATA8_ADDR, 4)
        verify_result = struct.unpack("<I", verify_result_bytes)[0] == 1
        return t, verify_result

    @api
    @allure.step("HMAC计算初始化 (使用明文密钥)")
    def ehsm_hmac_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        key_size: int,
        gen_hmac: bool,
        session: int
    ) -> tuple[int, int]:
        # 写入明文密钥到 DATA5
        key_addr = 0
        session_addr = 0
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        if session != 0:
            session_addr = self.SESSION_ADDR
        # 调用底层接口
        t = hostapi.ehsm_hmac_init_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            key_addr,
            key_size,
            gen_hmac,
            session_addr
        )
        return t, int(self.SESSION_ADDR)

    def ehsm_increase_counter(
        self,
        counter_id: int,
        increase_value: int
    ) -> tuple[int, bytes]:
        t, ret = hostapi.ehsm_increase_counter(
            self.CTX_ADDR,
            counter_id,
            increase_value,
            self.DATA1_ADDR
        )
        return t, ret

    def ehsm_inject_error(self, values: bytes) -> int:
        host.write_memory(self.DATA1_ADDR, values)
        t , ret = hostapi.ehsm_inject_error(
            self.CTX_ADDR,
            self.DATA1_ADDR
        )
        return t,ret

    def ehsm_install_encrypted_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmInstallKeyType,
        key_slot_id: int,
        last_key: int,
        input_data: bytes,
        size:int
    ) -> int:
        host.write_memory(self.DATA1_ADDR, input_data)
        t ,ret= hostapi.ehsm_install_encrypted_key(
            self.CTX_ADDR,
            key_level,
            key_type,
            key_slot_id,
            last_key,
            self.DATA1_ADDR,
            size
        )
        return t,ret

    def ehsm_install_random_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmInstallKeyType,
        key_slot_id: int,
        last_key: int,
    ) -> int:
        t = hostapi.ehsm_install_random_key(
            self.CTX_ADDR,
            key_level,
            key_type,
            key_slot_id,
            last_key
        )
        return t

    def ehsm_km_derive_key(
        self,
        hash_algo: EhsmHashAlgo,
        derive_algo: EhsmDeriveAlgo,
        derive_type: EhsmDeriveType,
        privilege: int,
        key_type: int,
        key_size: int,
        parent_key_handle: int,
        salt: bytes,
        salt_size: int,
        password: bytes,
        password_size: int,
        iter_times: int,
        key_handle: int,
    ) -> tuple[int, int]:
        if (salt!=None):
            host.write_memory(self.DATA1_ADDR, salt)
        password_addr = 0
        if (password!=None):
            host.write_memory(self.DATA2_ADDR, password)
            password_addr = self.DATA2_ADDR
        t, ret_key_handle = hostapi.ehsm_km_derive_key(
            self.CTX_ADDR,
            hash_algo,
            derive_algo,
            derive_type,
            privilege,
            key_type,
            key_size,
            parent_key_handle,
            self.DATA1_ADDR,
            salt_size,
            password_addr,
            password_size,
            iter_times,
            key_handle
        )
        return t, ret_key_handle

    def ehsm_km_derive_key_to_soc(
        self,
        hash_algo: EhsmHashAlgo,
        derive_algo: EhsmDeriveAlgo,
        derive_type: EhsmDeriveType,
        parent_key_handle: int,
        salt: bytes,
        salt_size: int,
        iter_times: int,
        soc_channel_id: int,
    ) -> int:
        host.write_memory(self.DATA1_ADDR, salt)
        t = hostapi.ehsm_km_derive_key_to_soc(
            self.CTX_ADDR,
            hash_algo,
            derive_algo,
            derive_type,
            parent_key_handle,
            self.DATA1_ADDR,
            salt_size,
            iter_times,
            soc_channel_id
        )
        return t

    def ehsm_km_exchange_key(
        self,
        rmt_pub_key: bytes,
        rmt_pub_key_size: int,
        privilege: int,
        key_type: int,
        hmac_key_size: int,
        local_key_handle: int,
        dh_params: bytes,
        dh_params_size: int,
        sm2_params: bytes,
        key_handle: int,
    ) -> tuple[int, int]:
        dh_params_addr = 0
        if (rmt_pub_key!=None):
            host.write_memory(self.DATA1_ADDR,rmt_pub_key)
        if (dh_params!=None):
            host.write_memory(self.DATA2_ADDR,dh_params)
            dh_params_addr = self.DATA2_ADDR
        if sm2_params!=None:
            host.write_memory(self.DATA3_ADDR,sm2_params)
        t, ret_handle = hostapi.ehsm_km_exchange_key(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            rmt_pub_key_size,
            privilege,
            key_type,
            hmac_key_size,
            local_key_handle,
            dh_params_addr,
            dh_params_size,
            self.DATA3_ADDR,
            key_handle
        )
        return t,ret_handle

    def ehsm_km_export_key(
        self,
        target_key_handle: int,
        transport_key_handle: int,
        auth_key_handle: int,
        key_part: int,
        key_data: bytes,
        key_data_size: int,
        mac: bytes,
        mac_size: int
    ) -> tuple[int, bytes, int, bytes, int]:
        t, r_key_size, r_mac_size = hostapi.ehsm_km_export_key(
            self.CTX_ADDR,
            target_key_handle,
            transport_key_handle,
            auth_key_handle,
            key_part,
            self.DATA1_ADDR,
            key_data_size,
            self.DATA2_ADDR,
            mac_size
        )
        if r_key_size > 0:
            _, key_data = host.read_memory(self.DATA1_ADDR, r_key_size)
        if r_mac_size > 0:
            _, mac = host.read_memory(self.DATA2_ADDR, r_mac_size)
        else:
            mac = None
        return t, key_data, r_key_size, mac, r_mac_size

    def ehsm_km_gen_key(
        self,
        key_type: EhsmKeyType,
        privilege: int,
        rsa_e_bit_size: int,
        hmac_key_size: int,
        dh_params: bytes,
        dh_params_size: int,
        key_handle: int
    ) -> tuple[int, int]:
        dh_params_addr = 0
        if dh_params is not None:
            host.write_memory(self.DATA1_ADDR,dh_params)
            dh_params_addr = self.DATA1_ADDR
        t, ret_key_handle = hostapi.ehsm_km_gen_key(
            self.CTX_ADDR,
            int(key_type),
            privilege,
            rsa_e_bit_size,
            hmac_key_size,
            dh_params_addr,
            dh_params_size,
            key_handle
        )
        return t, ret_key_handle

    def ehsm_km_get_pub_from_priv(
        self,
        key_handle: int,
        dh_params: bytes,
        dh_params_size: int,
        pub_key_size: int,
    ) -> tuple[int, bytes, int, int]:
        dh_params_addr = 0
        if dh_params is not None:
            host.write_memory(self.DATA1_ADDR, dh_params)
            dh_params_addr = self.DATA1_ADDR
        pub_key_size_byte = pub_key_size.to_bytes(4, 'little')
        host.write_memory(self.DATA3_ADDR, pub_key_size_byte)
        t, ret_alg_id,ret_pub_k_size = hostapi.ehsm_km_get_pub_from_priv(
            self.CTX_ADDR,
            key_handle,
            dh_params_addr,
            dh_params_size,
            self.DATA2_ADDR,
            self.DATA3_ADDR,
            self.DATA4_ADDR
        )
        _, output_pub = host.read_memory(self.DATA2_ADDR, ret_pub_k_size)
        return t, output_pub, ret_alg_id, ret_pub_k_size

    @api
    @allure.step("导入密钥")
    def ehsm_km_import_key(
        self,
        transport_key_handle: int,
        auth_key_handle: int,
        key_data: bytes,
        key_data_size: int,
        mac: bytes,
        mac_size: int,
        key_handle: int
    ) -> tuple[int, int]:
        key_addr = 0
        if key_data is not None:
            host.write_memory(self.DATA1_ADDR, key_data)
            key_addr = self.DATA1_ADDR
        mac_addr = 0
        if mac is not None:
            host.write_memory(self.DATA2_ADDR, mac)
            mac_addr = self.DATA2_ADDR
        t, ret_key_handle = hostapi.ehsm_km_import_key(
            self.CTX_ADDR,
            transport_key_handle,
            auth_key_handle,
            key_addr,
            key_data_size,
            mac_addr,
            mac_size,
            key_handle
        )
        return t, ret_key_handle

    def ehsm_km_remove_key(self, key_handle: int) -> int:
        t = hostapi.ehsm_km_remove_key(
            self.CTX_ADDR,
            key_handle
        )
        return t

    def ehsm_km_sm9_exchange_key(
        self,
        privilege: int,
        key_type: EhsmKeyType,
        role: int,
        user_priv_key_handle: int,
        user_tmp_key_handle: int,
        hmac_key_size: int,
        extra_params: Optional[bytes],
    ) -> tuple[int, int]:
        t, key_handle = hostapi.ehsm_km_sm9_exchange_key(
            self.CTX_ADDR,
            privilege,
            key_type,
            role,
            user_priv_key_handle,
            user_tmp_key_handle,
            hmac_key_size,
            extra_params
        )
        return t, key_handle

    @api
    @allure.step("一次完成计算生成MAC")
    def ehsm_mac_onepass_gen(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_handle: int,
        iv: bytes,
        iv_size: int,
        msg: bytes,
        msg_size: int,
        mac: bytes,
        mac_size: int
    ) -> tuple[int, bytes]:
        iv_addr = 0
        msg_addr = 0
        if iv is not None:
            if iv:  # 只有当 iv 有内容时才写入内存
                host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        # Reason: 只有为None时msg_addr给0，其他情况都给地址（包括空bytes）
        if msg is not None:
            if msg:  # 只有当msg有内容时才写入内存
                host.write_memory(self.DATA2_ADDR, msg)
            msg_addr = self.DATA2_ADDR
        t = hostapi.ehsm_mac_onepass_gen(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            iv_addr,
            iv_size,
            msg_addr,
            msg_size,
            self.DATA3_ADDR,
            mac_size
        )
        _, mac = host.read_memory(self.DATA3_ADDR, mac_size)
        return t, mac

    @api
    @allure.step("一次完成计算并验证MAC值")
    def ehsm_mac_onepass_verify(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_handle: int,
        iv: bytes,
        iv_size: int,
        msg: bytes,
        msg_size: int,
        mac: bytes,
        mac_size: int
    ) -> tuple[int, bool]:
        iv_addr = 0
        msg_addr = 0
        mac_addr = 0
        # Reason: 只有为None时iv_addr给0，其他情况都给地址
        if iv is not None:
            if iv:  # Only write if not empty
                host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        # Reason: 只有为None时msg_addr给0，其他情况都给地址（包括空bytes）
        if msg is not None:
            if msg:  # 只有当msg有内容时才写入内存
                host.write_memory(self.DATA2_ADDR, msg)
            msg_addr = self.DATA2_ADDR
        if mac:
            host.write_memory(self.DATA3_ADDR, mac)
            mac_addr = self.DATA3_ADDR
        t, ret_verify_result = hostapi.ehsm_mac_onepass_verify(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            iv_addr,
            iv_size,
            msg_addr,
            msg_size,
            mac_addr,
            mac_size
        )
        return t, ret_verify_result

    @api
    @allure.step("MAC初始化")
    def ehsm_mac_init(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_handle: int,
        gen_mac: bool,
        iv: bytes,
        iv_size: int,
        mac_size: int,
        session: int
    ) -> tuple[int, int]:
        iv_addr = 0
        # Reason: 只有为None时iv_addr给0，其他情况都给地址
        if iv is not None:
            if iv:  # Only write if not empty
                host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        t = hostapi.ehsm_mac_init(
            self.CTX_ADDR,
            algo,
            mode,
            key_handle,
            gen_mac,
            iv_addr,
            iv_size,
            mac_size,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("MAC计算，更新消息数据")
    def ehsm_mac_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        msg_addr = 0
        # Reason: 只有为None时msg_addr给0，其他情况都给地址（包括空bytes）
        if msg is not None:
            if msg:  # 只有当msg有内容时才写入内存
                host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t = hostapi.ehsm_mac_update(
            self.CTX_ADDR,
            msg_addr,
            msg_size
        )
        return t

    @api
    @allure.step("生成MAC结果")
    def ehsm_mac_finish_gen(
        self,
        msg: bytes,
        msg_size: int,
        mac_size: int = 16
    ) -> tuple[int, bytes]:
        msg_addr = 0
        # Reason: 只有为None时msg_addr给0，其他情况都给地址（包括空bytes）
        if msg is not None:
            if msg:  # 只有当msg有内容时才写入内存
                host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        t = hostapi.ehsm_mac_finish_gen(
            self.CTX_ADDR,
            msg_addr,
            msg_size,
            self.DATA2_ADDR
        )
        _, mac = host.read_memory(self.DATA2_ADDR, mac_size)
        return t, mac

    @api
    @allure.step("验证MAC值")
    def ehsm_mac_finish_verify(
        self,
        msg: bytes,
        msg_size: int,
        mac: bytes,
        mac_size: int = 16
    ) -> tuple[int, bool]:
        msg_addr = 0
        mac_addr = 0
        # Reason: 只有为None时msg_addr给0，其他情况都给地址（包括空bytes）
        if msg is not None:
            if msg:  # 只有当msg有内容时才写入内存
                host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        if mac:
            host.write_memory(self.DATA2_ADDR, mac)
            mac_addr = self.DATA2_ADDR
        t, verify_result = hostapi.ehsm_mac_finish_verify(
            self.CTX_ADDR,
            msg_addr,
            msg_size,
            mac_addr
        )
        return t, verify_result

    @api
    @allure.step("MAC计算生成，OnePass (使用明文密钥)")
    def ehsm_mac_onepass_gen_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_data: bytes,
        key_size: int,
        iv: bytes,
        iv_size: int,
        msg: bytes,
        msg_size: int,
        mac_size: int
    ) -> tuple[int, bytes]:
        key_addr = 0
        iv_addr = 0
        msg_addr = 0
        # Reason: 写明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # Reason: 写 IV 到 DATA3 (GMAC 需要)
        if iv is not None:
            if iv:  # 只有当 iv 有内容时才写入内存
                host.write_memory(self.DATA3_ADDR, iv)
            iv_addr = self.DATA3_ADDR
        # Reason: 写消息到 DATA1
        if msg:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        # Reason: 调用底层接口
        t = hostapi.ehsm_mac_onepass_gen_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            mode.value,
            key_addr,
            key_size,
            iv_addr,
            iv_size,
            msg_addr,
            msg_size,
            self.DATA6_ADDR,
            mac_size
        )
        # Reason: 读取 MAC 结果从 DATA6
        _, mac = host.read_memory(self.DATA6_ADDR, mac_size)
        return t, mac

    @api
    @allure.step("MAC计算验证，OnePass (使用明文密钥)")
    def ehsm_mac_onepass_verify_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_data: bytes,
        key_size: int,
        iv: bytes,
        iv_size: int,
        msg: bytes,
        msg_size: int,
        mac: bytes,
        mac_size: int
    ) -> tuple[int, bool]:
        key_addr = 0
        iv_addr = 0
        msg_addr = 0
        mac_addr = 0
        # Reason: 写明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # Reason: 写 IV 到 DATA3 (GMAC 需要)
        # Reason: 只有为None时iv_addr给0，其他情况都给地址
        if iv is not None:
            if iv:  # Only write if not empty
                host.write_memory(self.DATA3_ADDR, iv)
            iv_addr = self.DATA3_ADDR
        # Reason: 写消息到 DATA1
        if msg:
            host.write_memory(self.DATA1_ADDR, msg)
            msg_addr = self.DATA1_ADDR
        # Reason: 写待验证的 MAC 到 DATA2
        if mac:
            host.write_memory(self.DATA2_ADDR, mac)
            mac_addr = self.DATA2_ADDR
        # Reason: 调用底层接口
        t = hostapi.ehsm_mac_onepass_verify_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            mode.value,
            key_addr,
            key_size,
            iv_addr,
            iv_size,
            msg_addr,
            msg_size,
            mac_addr,
            mac_size,
            self.DATA8_ADDR
        )
        # Reason: 读取验证结果从 DATA8
        _, verify_result_bytes = host.read_memory(self.DATA8_ADDR, 4)
        verify_result = struct.unpack("<I", verify_result_bytes)[0] == 1
        return t, verify_result

    @api
    @allure.step("MAC计算初始化 (使用明文密钥)")
    def ehsm_mac_init_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmMacMode,
        key_data: bytes,
        key_size: int,
        gen_mac: bool,
        iv: bytes,
        iv_size: int,
        mac_size: int,
        session: int
    ) -> tuple[int, int]:
        key_addr = 0
        iv_addr = 0
        # Reason: 写明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # Reason: 写 IV 到 DATA3 (GMAC 需要)
        # Reason: 只有 iv 为 None 时 iv_addr 才为 0，其他情况（包括空字节串）都传地址
        if iv is not None:
            if iv:  # 只有当 iv 有内容时才写入内存，避免 RSP_ERR_DATA_LENGTH 错误
                host.write_memory(self.DATA3_ADDR, iv)
            iv_addr = self.DATA3_ADDR  # 无论是否有内容，只要不是 None 就传地址
        # Reason: 调用底层接口
        t = hostapi.ehsm_mac_init_with_plain_key(
            self.CTX_ADDR,
            algo.value,
            mode.value,
            key_addr,
            key_size,
            gen_mac,
            iv_addr,
            iv_size,
            mac_size,
            self.SESSION_ADDR
        )
        # Reason: 读取 session 从 SESSION_ADDR
        _, session_bytes = host.read_memory(self.SESSION_ADDR, 4)
        session = struct.unpack("<I", session_bytes)[0]
        return t, session

    @api
    @allure.step("读取某个计数器的值")
    def ehsm_read_counter(
        self,
        counter_id: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_read_counter(
            self.CTX_ADDR,
            counter_id,
            self.DATA1_ADDR
        )
        _, counter_value = host.read_memory(self.DATA1_ADDR, 64)
        return t, counter_value

    @api
    @allure.step("读取OTP数据")
    def ehsm_read_otp(
        self,
        ehsm_src_addr: int,
        size: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_read_otp(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_src_addr,
            size
        )
        _, buf = host.read_memory(self.DATA1_ADDR, size)
        return t, buf

    @api
    @allure.step("读取CFG地址范围的寄存器值")
    def ehsm_read_reg(
        self,
        ehsm_src_addr: int,
        size: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_read_reg(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            ehsm_src_addr,
            size
        )
        _, buf = host.read_memory(self.DATA1_ADDR, size)
        return t, buf

    @api
    @allure.step("一次完成RSA加密/解密计算")
    def ehsm_rsa_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
    ) -> tuple[int, bytes, int]:
        host.write_memory(self.DATA1_ADDR, input)
        t, output_size = hostapi.ehsm_rsa_cipher(
            self.CTX_ADDR,
            key_handle,
            enc,
            self.DATA1_ADDR,
            input_size,
            self.DATA3_ADDR,
            output_buff_size
        )
        _, output = host.read_memory(self.DATA3_ADDR, output_size)
        return t, output, output_size

    @api
    @allure.step("RSA签名值生成，一次计算完成")
    def ehsm_rsa_sign_onepass_gen(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        padding: EhsmRsaPaddingMode,
        msg: bytes,
        msg_size: int,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bytes, int]:
        host.write_memory(self.DATA1_ADDR, msg)
        t, ret_sig_size = hostapi.ehsm_rsa_sign_onepass_gen(
            self.CTX_ADDR,
            algo,
            key_handle,
            padding,
            self.DATA1_ADDR,
            msg_size,
            self.DATA3_ADDR,
            sig_size,
            salt_size
        )
        _, sig = host.read_memory(self.DATA3_ADDR, ret_sig_size)
        return t, sig, ret_sig_size

    @api
    @allure.step("RSA验证签名值，一次计算完成")
    def ehsm_rsa_sign_onepass_verify(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        padding: EhsmRsaPaddingMode,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, msg)
        host.write_memory(self.DATA3_ADDR, sig)
        t, verify_result = hostapi.ehsm_rsa_sign_onepass_verify(
            self.CTX_ADDR,
            algo,
            key_handle,
            padding,
            self.DATA1_ADDR,
            msg_size,
            self.DATA3_ADDR,
            sig_size,
            salt_size
        )
        return t, verify_result

    @api
    @allure.step("RSA签名值生成，使用 digest 一次计算完成")
    def ehsm_rsa_sign_onepass_gen_with_digest(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        padding: EhsmRsaPaddingMode,
        digest: bytes,
        digest_size: int,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bytes, int]:
        host.write_memory(self.DATA1_ADDR, digest)
        t, ret_sig_size = hostapi.ehsm_rsa_sign_onepass_gen_with_digest(
            self.CTX_ADDR,
            algo,
            key_handle,
            padding,
            self.DATA1_ADDR,
            digest_size,
            self.DATA3_ADDR,
            sig_size,
            salt_size
        )
        _, sig = host.read_memory(self.DATA3_ADDR, ret_sig_size)
        return t, sig, ret_sig_size

    @api
    @allure.step("RSA验证签名值，使用 digest 一次计算完成")
    def ehsm_rsa_sign_onepass_verify_with_digest(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        padding: EhsmRsaPaddingMode,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, digest)
        host.write_memory(self.DATA3_ADDR, sig)
        t, verify_result = hostapi.ehsm_rsa_sign_onepass_verify_with_digest(
            self.CTX_ADDR,
            algo,
            key_handle,
            padding,
            self.DATA1_ADDR,
            digest_size,
            self.DATA3_ADDR,
            sig_size,
            salt_size
        )
        return t, verify_result

    @api
    @allure.step("RSA签名/验签统一接口 (ex版本)")
    def ehsm_rsa_sign_onepass_ex(
        self,
        algo: EhsmHashAlgo,
        use_plain_key: bool,
        key_handle: int,
        key_data: Optional[bytes],
        padding: EhsmRsaPaddingMode,
        gen_sig: bool,
        is_digest: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        salt_size: int,
        signature: Optional[bytes] = None
    ) -> tuple[int, bytes, int, bool]:
        """
        RSA签名/验签统一接口 (ex版本)

        Reason: 统一接口支持明文密钥/密钥句柄、签名/验签、原始消息/摘要的所有组合

        注意: key_data应该是已经打包好的 ehsm_rsa_key_st 结构,但地址字段可能需要重新计算。
        如果key_data是预打包的结构(前76字节是结构体),这里会重新计算地址字段以适配实际写入地址。

        Args:
            signature: 验签模式下需要传入的签名数据
        """
        # 写入输入数据(消息或摘要)
        host.write_memory(self.DATA1_ADDR, input)

        # 验签模式:写入签名数据
        if not gen_sig and signature:
            host.write_memory(self.DATA3_ADDR, signature)

        # 写入明文密钥(如果使用明文密钥)
        key_addr = 0
        if use_plain_key and key_data:
            # Reason: key_data应该已经是ehsm_rsa_key_st格式,但需要用实际的DATA2_ADDR重新打包地址
            # 如果key_data长度>76字节,说明是完整的打包结构(76字节结构+实际数据)
            # 我们需要用DATA2_ADDR作为基地址重新计算结构中的地址字段
            if len(key_data) > 76:
                import struct
                # 解析前76字节的结构
                # 格式: B(crt_mode) + 3B(reserved) + L(n_byte_sz) + L(e_byte_sz) + 8Q(8个地址)
                struct_header = key_data[:76]
                key_components = key_data[76:]  # 实际的密钥数据

                # 解析结构头
                unpacked = struct.unpack("<BBBBLLQQQQQQQQ", struct_header)
                crt_mode = unpacked[0]
                n_byte_sz = unpacked[4]
                e_byte_sz = unpacked[5]

                # Reason: 重新计算地址,基地址是DATA2_ADDR,数据从第76字节开始
                struct_size = 76
                data_base = self.DATA2_ADDR + struct_size
                current_offset = data_base

                # 计算各组件地址
                n_addr = current_offset
                current_offset += n_byte_sz

                e_addr = current_offset
                current_offset += e_byte_sz

                # 从原始数据中提取各组件来计算大小
                pos = 0
                n_data = key_components[pos:pos+n_byte_sz]
                pos += n_byte_sz
                e_data = key_components[pos:pos+e_byte_sz]
                pos += e_byte_sz

                # d的地址和大小
                # Reason: CRT模式下不使用d,只在NO_CRT模式下需要d
                if crt_mode == 0:
                    # NO_CRT模式: d的大小等于n的大小
                    if pos < len(key_components):
                        d_size = n_byte_sz
                        d_addr = current_offset
                        current_offset += d_size
                        pos += d_size
                    else:
                        d_addr = 0
                else:
                    # CRT模式: 不使用d,直接设为0
                    d_addr = 0

                # CRT参数地址
                if crt_mode == 1 and pos < len(key_components):
                    # p, q, dp, dq, u 的大小都是 n_byte_sz / 2
                    crt_component_size = n_byte_sz // 2

                    p_addr = current_offset
                    current_offset += crt_component_size
                    pos += crt_component_size

                    q_addr = current_offset
                    current_offset += crt_component_size
                    pos += crt_component_size

                    dp_addr = current_offset
                    current_offset += crt_component_size
                    pos += crt_component_size

                    dq_addr = current_offset
                    current_offset += crt_component_size
                    pos += crt_component_size

                    u_addr = current_offset
                    current_offset += crt_component_size
                else:
                    p_addr = q_addr = dp_addr = dq_addr = u_addr = 0

                # Reason: 重新打包结构体,使用新的地址
                new_struct_header = struct.pack(
                    "<BBBBLLQQQQQQQQ",
                    crt_mode,
                    0, 0, 0,  # reserved
                    n_byte_sz,
                    e_byte_sz,
                    n_addr,
                    e_addr,
                    d_addr,
                    p_addr,
                    q_addr,
                    dp_addr,
                    dq_addr,
                    u_addr
                )

                # 重新组合完整的密钥数据
                key_data = new_struct_header + key_components

            # 写入重新打包后的密钥数据
            host.write_memory(self.DATA2_ADDR, key_data)
            key_addr = self.DATA2_ADDR

        # 调用 hostapi
        t, ret_size, verify_result = hostapi.ehsm_rsa_sign_onepass_ex(
            self.CTX_ADDR,
            algo,
            use_plain_key,
            key_handle,
            key_addr,
            padding,
            gen_sig,
            is_digest,
            self.DATA1_ADDR,
            input_size,
            self.DATA3_ADDR,  # 签名输出地址
            output_buff_size,
            salt_size,
            self.DATA4_ADDR   # 验签结果地址
        )

        # 读取输出
        if gen_sig:
            # 签名操作: 读取签名
            _, sig = host.read_memory(self.DATA3_ADDR, ret_size)
            return t, sig, ret_size, False
        else:
            # 验签操作: 读取验签结果
            _, verify_result_bytes = host.read_memory(self.DATA4_ADDR, 4)
            verify_result = bool.from_bytes(verify_result_bytes, "little")
            return t, b'', 0, verify_result

    @api
    @allure.step("RSA签名/验签计算，初始化")
    def ehsm_rsa_sign_init(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        gen_sig: bool,
        padding: EhsmRsaPaddingMode,
        session: int
    ) -> tuple[int, int]:
        t = hostapi.ehsm_rsa_sign_init(
            self.CTX_ADDR,
            algo,
            key_handle,
            int(gen_sig),
            padding,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("RSA签名/验签计算，更新数据")
    def ehsm_rsa_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        host.write_memory(self.DATA1_ADDR, msg)
        t = hostapi.ehsm_rsa_sign_update(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            msg_size
        )
        return t

    @api
    @allure.step("RSA签名/验签计算，生成签名值")
    def ehsm_rsa_sign_finish_gen(
        self,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_rsa_sign_finish_gen(
            self.CTX_ADDR,
            self.DATA2_ADDR,
            sig_size,
            salt_size
        )
        _, sig = host.read_memory(self.DATA2_ADDR, sig_size)
        return t, sig

    @api
    @allure.step("RSA签名/验签计算，验证签名值")
    def ehsm_rsa_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA3_ADDR, sig)
        t, verify_result = hostapi.ehsm_rsa_sign_finish_verify(
            self.CTX_ADDR,
            self.DATA3_ADDR,
            sig_size,
            salt_size
        )
        return t, verify_result

    @api
    @allure.step("设置串口波特率")
    def ehsm_set_uart_baudrate(
        self,
        baud_div: int
    ) -> int:
        t = hostapi.ehsm_set_uart_baudrate(
            self.CTX_ADDR,
            baud_div
        )
        return t

    @api
    @allure.step("设置EHSM UTC-Timer时间")
    def ehsm_set_utc_time(
        self,
        utc_time: int
    ) -> int:
        t = hostapi.ehsm_set_utc_time(
            self.CTX_ADDR,
            utc_time
        )
        return t

    @api
    @allure.step("SM2加密/解密计算，一次完成计算")
    def ehsm_sm2_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        if input and input_size != 0:
            host.write_memory(self.DATA1_ADDR, input)
        t, ret_output_size = hostapi.ehsm_sm2_cipher(
            self.CTX_ADDR,
            key_handle,
            enc,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR,
            output_buff_size
        )
        if ret_output_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, ret_output_size)
        else:
            output = b''
        return t, output, ret_output_size

    @api
    @allure.step("SM2签名值生成，一次计算完成")
    def ehsm_sm2_sign_onepass_gen(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, msg)
        t, ret_sig_size = hostapi.ehsm_sm2_sign_onepass_gen(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            msg_size,
            self.DATA2_ADDR,
            sig_buff_size
        )
        _, sig = host.read_memory(self.DATA2_ADDR, ret_sig_size)
        return t, sig

    @api
    @allure.step("SM2签名值生成，使用 digest 一次计算完成")
    def ehsm_sm2_sign_onepass_gen_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, digest)
        t, ret_sig_size = hostapi.ehsm_sm2_sign_onepass_gen_with_digest(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            digest_size,
            self.DATA2_ADDR,
            sig_buff_size
        )
        _, sig = host.read_memory(self.DATA2_ADDR, ret_sig_size)
        return t, sig

    @api
    @allure.step("SM2签名值生成，一次计算完成")
    def ehsm_sm2_sign_onepass_verify(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, msg)
        host.write_memory(self.DATA2_ADDR, sig)
        t, verify_result = hostapi.ehsm_sm2_sign_onepass_verify(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            msg_size,
            self.DATA2_ADDR,
            sig_size
        )
        return t, verify_result

    @api
    @allure.step("SM2签名值生成，使用 digest 一次计算完成")
    def ehsm_sm2_sign_onepass_verify_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA1_ADDR, digest)
        host.write_memory(self.DATA2_ADDR, sig)
        t, verify_result = hostapi.ehsm_sm2_sign_onepass_verify_with_digest(
            self.CTX_ADDR,
            key_handle,
            self.DATA1_ADDR,
            digest_size,
            self.DATA2_ADDR,
            sig_size
        )
        return t, verify_result

    @api
    @allure.step("SM2签名/验签统一接口 (ex版本)")
    def ehsm_sm2_sign_onepass_ex(
        self,
        use_plain_key: bool,
        key_handle: int,
        key_data: Optional[bytes],
        gen_sig: bool,
        is_digest: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        signature: Optional[bytes] = None
    ) -> tuple[int, bytes, int, bool]:
        """
        SM2签名/验签统一接口 (ex版本)

        Reason: 统一接口支持明文密钥/密钥句柄、签名/验签、原始消息/摘要(E值)的所有组合

        Args:
            signature: 验签模式下需要传入的签名数据
        """
        # 写入输入数据(消息或摘要)
        host.write_memory(self.DATA1_ADDR, input)

        # 验签模式: 写入签名数据
        sig_size = output_buff_size  # 默认使用输出缓冲区大小
        if not gen_sig and signature:
            host.write_memory(self.DATA3_ADDR, signature)
            sig_size = len(signature)  # Reason: 验签时sig_size应为实际签名长度,不是缓冲区大小

        # 写入明文密钥(如果使用明文密钥)
        key_addr = 0
        if use_plain_key and key_data:
            host.write_memory(self.DATA2_ADDR, key_data)
            key_addr = self.DATA2_ADDR

        # 调用 hostapi
        t, ret_size, _ = hostapi.ehsm_sm2_sign_onepass_ex(
            self.CTX_ADDR,
            use_plain_key,
            key_handle,
            key_addr,
            gen_sig,
            is_digest,
            self.DATA1_ADDR,
            input_size,
            self.DATA3_ADDR,  # 签名输出地址/签名输入地址
            sig_size,         # 签名模式:输出缓冲区大小; 验签模式:输入签名实际长度
            self.DATA4_ADDR   # 验签结果地址
        )

        # 读取输出
        if gen_sig:
            # 签名操作: 读取签名
            _, sig = host.read_memory(self.DATA3_ADDR, ret_size)
            return t, sig, ret_size, False
        else:
            # 验签操作: 读取验签结果
            _, verify_result_bytes = host.read_memory(self.DATA4_ADDR, 4)
            verify_result = bool.from_bytes(verify_result_bytes, "little")
            return t, b'', 0, verify_result

    @api
    @allure.step("SM2签名/验签计算，初始化")
    def ehsm_sm2_sign_init(
        self,
        key_handle: int,
        gen_sig: bool,
        session: int
    ) -> tuple[int, bytes]:
        t = hostapi.ehsm_sm2_sign_init(
            self.CTX_ADDR,
            key_handle,
            gen_sig,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("SM2签名/验签计算，更新数据")
    def ehsm_sm2_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        host.write_memory(self.DATA1_ADDR, msg)
        t = hostapi.ehsm_sm2_sign_update(
            self.CTX_ADDR,
            self.DATA1_ADDR,
            msg_size
        )
        return t

    @api
    @allure.step("SM2加密/解密计算，一次完成计算")
    def ehsm_sm2_sign_finish_gen(
        self,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        t, ret_sig_size = hostapi.ehsm_sm2_sign_finish_gen(
            self.CTX_ADDR,
            self.DATA2_ADDR,
            sig_buff_size
        )
        _, sig = host.read_memory(self.DATA2_ADDR, ret_sig_size)
        return t, sig

    @api
    @allure.step("SM2签名/验签计算，验证签名值")
    def ehsm_sm2_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        host.write_memory(self.DATA2_ADDR, sig)
        t, verify_result = hostapi.ehsm_sm2_sign_finish_verify(
            self.CTX_ADDR,
            self.DATA2_ADDR,
            sig_size
        )
        return t, verify_result

    @api
    @allure.step("SM9加密/解密计算，一次完成计算")
    def ehsm_sm9_cipher(
        self,
        key_handle: int,
        enc: bool,
        enc_type: EhsmSm9EncType,
        padding: EhsmSm9PaddingMode,
        key2_size: int,
        hid: int,
        kgc_pub_key: bytes,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        id: bytes,
        id_size: int,
        fp12g: Optional[bytes]
    ) -> tuple[int, int, bytes]:
        fp12g_addr = 0
        host.write_memory(self.DATA1_ADDR, kgc_pub_key)
        host.write_memory(self.DATA2_ADDR, id)
        if enc is False and fp12g is not None:
            host.write_memory(self.DATA3_ADDR, fp12g)
            fp12g_addr = self.DATA3_ADDR
        host.write_memory(self.DATA4_ADDR, input)
        t, output_size = hostapi.ehsm_sm9_cipher(
            self.CTX_ADDR,
            key_handle,
            enc,
            enc_type,
            padding,
            key2_size,
            hid,
            self.DATA1_ADDR,
            self.DATA4_ADDR,
            input_size,
            self.DATA6_ADDR,
            output_buff_size,
            self.DATA2_ADDR,
            id_size,
            fp12g_addr
        )
        if output_size > 0:
            _, actual_output = host.read_memory(self.DATA6_ADDR, output_size)
        else:
            actual_output = bytes()
        return t, output_size, actual_output

    @api
    @allure.step("SM9签名值生成，一次计算完成")
    def ehsm_sm9_sign_onepass_gen(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig_size: int,
        kgc_pub_key: bytes,
        fp12g: Optional[bytes]
    ) -> tuple[int, bytes]:
        host.write_memory(self.DATA1_ADDR, kgc_pub_key)
        fp12g_addr = 0
        if fp12g is not None:
            host.write_memory(self.DATA2_ADDR, fp12g)
            fp12g_addr = self.DATA2_ADDR
        host.write_memory(self.DATA3_ADDR, msg)
        t = hostapi.ehsm_sm9_sign_onepass_gen(
            self.CTX_ADDR,
            key_handle,
            self.DATA3_ADDR,
            msg_size,
            self.DATA5_ADDR,
            sig_size,
            self.DATA1_ADDR,
            fp12g_addr
        )
        _, sig = host.read_memory(self.DATA5_ADDR, sig_size)
        return t, sig

    @api
    @allure.step("SM9验证签名值，一次计算完成")
    def ehsm_sm9_sign_onepass_verify(
        self,
        msg: bytes,
        msg_size: int,
        id: bytes,
        id_size: int,
        hid: int,
        kgc_pub_key: bytes,
        fp12g: Optional[bytes],
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        # Reason: 如果id_size为0,id_addr应该传0(NULL),不写入内存
        if id_size > 0 and id:
            host.write_memory(self.DATA1_ADDR, id)
            id_addr = self.DATA1_ADDR
        else:
            id_addr = 0  # NULL地址

        host.write_memory(self.DATA2_ADDR, kgc_pub_key)
        fp12g_addr = 0
        if fp12g is not None:
            host.write_memory(self.DATA3_ADDR, fp12g)
            fp12g_addr = self.DATA3_ADDR
        host.write_memory(self.DATA4_ADDR, msg)
        host.write_memory(self.DATA6_ADDR, sig)
        t, verify_result = hostapi.ehsm_sm9_sign_onepass_verify(
            self.CTX_ADDR,
            self.DATA4_ADDR,
            msg_size,
            id_addr,
            id_size,
            hid,
            self.DATA2_ADDR,
            fp12g_addr,
            self.DATA6_ADDR,
            sig_size
        )
        return t, verify_result

    @api
    @allure.step("SKE 加解密")
    def ehsm_symm_cipher_onepass(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmCipherMode,
        padding: EhsmPaddingMode,
        key_handle: int,
        enc: bool,
        iv: bytes,
        iv_size: int,
        input_data: bytes,
        input_size: int,
        output_buf_size: int,
        skip_read_output: bool = False
    ) -> tuple[int, bytes, int]:
        iv_addr = 0
        input_addr = 0
        if iv:
            host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        if input_data:
            host.write_memory(self.DATA2_ADDR, input_data)
            input_addr = self.DATA2_ADDR
        if output_buf_size != 0:
            host.write_memory(self.DATA4_ADDR, output_buf_size.to_bytes(4, 'little'))
        t, ret_output_size = hostapi.ehsm_symm_cipher_onepass(
            self.CTX_ADDR,
            algo,
            mode,
            padding,
            key_handle,
            enc,
            iv_addr,
            iv_size,
            input_addr,
            input_size,
            self.DATA3_ADDR,
            self.DATA4_ADDR
        )
        # Reason: 性能测试时跳过大数据读取，避免通信开销
        if ret_output_size != 0 and not skip_read_output:
            _, output = host.read_memory(self.DATA3_ADDR, ret_output_size)
        else:
            output = b''
        return t, output, ret_output_size

    @api
    @allure.step("SKE 加解密，使用明文密钥")
    def ehsm_symm_cipher_onepass_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmCipherMode,
        padding: EhsmPaddingMode,
        enc: bool,
        key_data: bytes,
        key_size: int,
        iv: bytes,
        iv_size: int,
        input_data: bytes,
        input_size: int,
        output_buf_size: int,
    ) -> tuple[int, bytes, int]:
        iv_addr = 0
        input_addr = 0
        key_addr = 0
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        if iv:
            host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        if input_data is not None:
            if input_data != b'' :
                host.write_memory(self.DATA2_ADDR, input_data)
            input_addr = self.DATA2_ADDR
        if output_buf_size != 0:
            host.write_memory(self.DATA4_ADDR, output_buf_size.to_bytes(4, 'little'))
        t, ret_output_size = hostapi.ehsm_symm_cipher_onepass_plain_key(
            self.CTX_ADDR,
            algo,
            mode,
            padding,
            enc,
            key_addr,
            key_size,
            iv_addr,
            iv_size,
            input_addr,
            input_size,
            self.DATA3_ADDR,
            self.DATA4_ADDR
        )
        if ret_output_size != 0:
            _, output = host.read_memory(self.DATA3_ADDR, ret_output_size)
        else:
            output = b''
        return t, output, ret_output_size

    @api
    @allure.step("对称加密/解密计算，初始化")
    def ehsm_symm_cipher_init(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmCipherMode,
        padding: EhsmPaddingMode,
        key_handle: int,
        enc: bool,
        iv: bytes,
        iv_size: int,
        session: int
    ) -> tuple[int, int]:
        iv_addr = 0
        session_addr = 0
        if iv:
            host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        if session is None:
            session_addr = self.SESSION_ADDR
        t = hostapi.ehsm_symm_cipher_init(
            self.CTX_ADDR,
            algo,
            mode,
            padding,
            key_handle,
            enc,
            iv_addr,
            iv_size,
            session_addr
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("对称加密/解密计算，初始化 (使用明文密钥)")
    def ehsm_symm_cipher_init_with_plain_key(
        self,
        algo: EhsmSymmAlgo,
        mode: EhsmCipherMode,
        padding: EhsmPaddingMode,
        enc: bool,
        key_data: bytes,
        key_size: int,
        iv: bytes,
        iv_size: int,
        session: int
    ) -> tuple[int, int]:
        iv_addr = 0
        key_addr = 0
        # 写入明文密钥到 DATA5
        if key_data is not None:
            host.write_memory(self.DATA5_ADDR, key_data)
            key_addr = self.DATA5_ADDR
        # 写入 IV 到 DATA1
        if iv:
            host.write_memory(self.DATA1_ADDR, iv)
            iv_addr = self.DATA1_ADDR
        # 调用底层 C 接口
        t = hostapi.ehsm_symm_cipher_init_with_plain_key(
            self.CTX_ADDR,
            algo,
            mode,
            padding,
            key_addr,
            key_size,
            enc,
            iv_addr,
            iv_size,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("对称加密/解密计算，更新数据")
    def ehsm_symm_cipher_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        input_addr = 0  # 初始化为0，当input为None时传空指针给底层
        if input is not None:  # 只要不是None，就是正常数据（即使是空字节串b''）
            if input_size != 0:
                host.write_memory(self.DATA1_ADDR, input)
            input_addr = self.DATA1_ADDR  # 传有效地址表示正常调用
        # else: input为None时，input_addr保持为0，用于异常测试
        t = hostapi.ehsm_symm_cipher_update(
            self.CTX_ADDR,
            input_addr,  # None→0（异常），b''→有效地址（正常）
            input_size,
            self.DATA2_ADDR
        )
        if input_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, input_size)
        else:
            output = b''
        return t, output

    @api
    @allure.step("对称加密/解密计算，结束计算")
    def ehsm_symm_cipher_finish(
        self,
        input: bytes,
        input_size: int,
        output_size: int
    ) -> tuple[int, bytes, int]:
        input_addr = 0  # 初始化为0，当input为None时传空指针给底层
        if input is not None:  # 只要不是None，就是正常数据（即使是空字节串b''）
            if input_size != 0:
                host.write_memory(self.DATA1_ADDR, input)
            input_addr = self.DATA1_ADDR  # 传有效地址表示正常调用
        # else: input为None时，input_addr保持为0，用于异常测试
        t, ret_output_size = hostapi.ehsm_symm_cipher_finish(
            self.CTX_ADDR,
            input_addr,  # None→0（异常），b''→有效地址（正常）
            input_size,
            self.DATA2_ADDR,
            output_size
        )
        if ret_output_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, ret_output_size)
        else:
            output = b''
        return t, output, ret_output_size

    @api
    @allure.step("对称加密/解密计算，更新数据")
    def ehsm_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int,
        skip_read_output: bool = False,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        # Reason: 参数校验，镜像为空或size为0时传空地址给下位机检测
        image_addr = 0
        if image is not None and len(image) != 0 and image_size != 0:
            # Reason: 分块写入镜像数据，每次最多 16KB，避免超过 64KB 限制
            chunk_size = 16 * 1024
            for offset in range(0, len(image), chunk_size):
                chunk = image[offset : offset + chunk_size]
                host.write_memory(self.DATA1_ADDR + offset, chunk)
            image_addr = self.DATA1_ADDR
        if storage_addr is None:
            storage_addr = self.DATA1_ADDR
        t = hostapi.ehsm_upgrade_fw_image(
            self.CTX_ADDR,
            image_addr,
            image_size,
            storage_addr
        )

        # Reason: 性能测试时跳过内存读取操作以准确测量升级时间
        if skip_read_output:
            return t, b''
        else:
            _, image_out = host.read_memory(storage_addr, image_size - 1024)
            return t, image_out

    @api
    @allure.step("三段式升级固件镜像-初始化阶段")
    def ehsm_upgrade_fw_image_init(
        self,
        image_header: bytes,
        header_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        image_addr = 0  # 初始化为0
        if image_header is not None and len(image_header) != 0 and header_size != 0:
            host.write_memory(self.DATA1_ADDR, image_header)
            image_addr = self.DATA1_ADDR
        if storage_addr is None:
            storage_addr = self.DATA3_ADDR
        t = hostapi.ehsm_upgrade_fw_image_init(
            self.CTX_ADDR,
            image_addr,
            header_size,
            storage_addr
        )
        return t, b''

    @api
    @allure.step("三段式升级固件镜像-更新阶段")
    def ehsm_upgrade_fw_image_update(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        image_addr = 0
        if body_block is not None and len(body_block) != 0 and block_size != 0:
            host.write_memory(self.DATA1_ADDR, body_block)
            image_addr = self.DATA1_ADDR
        if storage_addr is None:
            storage_addr = self.DATA3_ADDR
        t = hostapi.ehsm_upgrade_fw_image_update(
            self.CTX_ADDR,
            image_addr,
            block_size,
            storage_addr
        )
        _, image_out = host.read_memory(self.DATA3_ADDR, block_size)
        return t, image_out

    @api
    @allure.step("三段式升级固件镜像-完成阶段")
    def ehsm_upgrade_fw_image_finish(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        image_addr = 0  # 初始化为0
        if body_block is not None and len(body_block) != 0 and block_size != 0:
            host.write_memory(self.DATA1_ADDR, body_block)
            image_addr = self.DATA1_ADDR
        if storage_addr is None:
            storage_addr = self.DATA3_ADDR
        t = hostapi.ehsm_upgrade_fw_image_finish(
            self.CTX_ADDR,
            image_addr,
            block_size,
            storage_addr
        )
        _, image_out = host.read_memory(self.DATA3_ADDR, block_size)
        return t, image_out

    @api
    @allure.step("校验安全启动镜像")
    def ehsm_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool,
        image_out_byte:int = None,
        skip_read_output: bool = False
    ) -> tuple[int, bytes]:
        image_addr = 0  # 初始化为0
        image_out_addr = 0
        if image is not None and len(image) != 0 and image_size != 0:
            chunk_size = 16 * 1024
            for offset in range(0, len(image), chunk_size):
                chunk = image[offset : offset + chunk_size]
                host.write_memory(self.DATA1_ADDR + offset, chunk)
            image_addr = self.DATA1_ADDR
        if image_out_byte == None:
            image_out_addr = self.DATA1_ADDR
        t = hostapi.ehsm_verify_image(
            self.CTX_ADDR,
            image_addr,
            image_size,
            check_version,
            boot,
            image_out_addr
        )
        if skip_read_output:
            return t, b''
        else:
            _, image_out = host.read_memory(image_out_addr, image_size)
            return t, image_out

    @api
    @allure.step("校验TBBR镜像")
    def ehsm_verify_tbbr_img(
        self,
        img_size: int,
        img_data: bytes,
        trusted_fw_nv_ctr_in_otp: int,
        non_trusted_fw_nv_ctr_in_otp: int,
    ) -> tuple[int, bytes, bytes]:
        host.write_memory(self.DATA1_ADDR, img_data)
        t = hostapi.ehsm_verify_tbbr_img(
            self.CTX_ADDR,
            img_size,
            self.DATA1_ADDR,
            trusted_fw_nv_ctr_in_otp,
            non_trusted_fw_nv_ctr_in_otp,
            self.DATA2_ADDR,
            self.DATA3_ADDR
        )
        _, trusted_fw_nv_ctr_in_cert = host.read_memory(self.DATA2_ADDR, 16)
        _, non_trusted_fw_nv_ctr_in_otp_cert = host.read_memory(self.DATA3_ADDR, 16)
        return t, trusted_fw_nv_ctr_in_cert, non_trusted_fw_nv_ctr_in_otp_cert

    @api
    @allure.step("写入OTP数据")
    def ehsm_write_otp(
        self,
        src_data: bytes,
        ehsm_dest_addr: int,
        size: int
    ) -> int:
        src_addr = 0
        if src_data != None:
            host.write_memory(self.DATA1_ADDR, src_data)
            src_addr = self.DATA1_ADDR
        t = hostapi.ehsm_write_otp(
            self.CTX_ADDR,
            src_addr,
            ehsm_dest_addr,
            size,
        )

    @api
    @allure.step("写入CFG地址范围的寄存器值")
    def ehsm_write_reg(
        self,
        src_data: bytes,
        ehsm_dest_addr: int,
        size: int
    ) -> int:
        src_addr = 0
        if src_data != None:
            host.write_memory(self.DATA1_ADDR, src_data)
            src_addr = self.DATA1_ADDR
        t = hostapi.ehsm_write_reg(
            self.CTX_ADDR,
            src_addr,
            ehsm_dest_addr,
            size,
        )
        return t

    @api
    @allure.step("读取 {ehsm_src_addr} 地址范围的寄存器值")
    def ehsm_test_read_memory(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        try:
            t = hostapi.ehsm_test_read_memory(
                self.CTX_ADDR,
                self.DATA1_ADDR,
                ehsm_src_addr,
                size
            )
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_INVALID_CMD == e.ret_code:# error: 34, EHSM_ERR_INVALID_CMD
                log.debug("error: 该命令不支持，请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
                pytest.xfail("预期失败：命令不支持,（34），请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
            else:
                raise

        _, output = host.read_memory(self.DATA1_ADDR, size)
        return t, output

    @api
    @allure.step("写入 {ehsm_dest_addr} 地址范围的寄存器值")
    def ehsm_test_write_memory(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        host.write_memory(self.DATA1_ADDR, src_data)
        try:
            t = hostapi.ehsm_test_write_memory(
                self.CTX_ADDR,
                self.DATA1_ADDR,
                ehsm_dest_addr,
                size,
            )
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_INVALID_CMD == e.ret_code:# error: 34, EHSM_ERR_INVALID_CMD
                log.debug("error: 该命令不支持，请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
                pytest.xfail("预期失败：命令不支持,（34），请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
            else:
                raise
        return t

    @api
    @allure.step("跳转到指定地址 {addr}")
    def ehsm_test_jump_to_addr(self, addr: int) -> int:
        try:
            t = hostapi.ehsm_test_jump_to_addr(
                self.CTX_ADDR,
                addr
            )
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_INVALID_CMD == e.ret_code:# error: 34, EHSM_ERR_INVALID_CMD
                log.debug("error: 该命令不支持，请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
                pytest.xfail("预期失败：命令不支持,（34），请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
            else:
                raise
        return t

    @api
    @allure.step("跳转到循环")
    def ehsm_test_jump_to_loop(self) -> int:
        t = 0
        try:
            t = hostapi.ehsm_test_jump_to_loop(
                self.CTX_ADDR,
            )
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_INVALID_CMD == e.ret_code:# error: 34, EHSM_ERR_INVALID_CMD
                log.debug("error: 该命令不支持，请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
                pytest.xfail("预期失败：命令不支持,（34），请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译")
            if "1437252556" in str(e):
                log.debug("info: 异步模式返回值：0x55aabbcc")
            else:
                raise
        return t

    @api
    @allure.step("注入 OTP 写失败（secboot_update_ver_cnt 中 otp_write 返回 0x123）")
    def ehsm_test_inject_otp_write_error(self) -> int:
        """
        注入 OTP 写失败占位接口。

        调用后，固件执行 secboot_update_ver_cnt 中的 otp_write 时返回错误码 0x123，
        触发 expt_det_add_error(FW_ERROR_OTP_WRITE_FAILED) 并阻塞。

        TODO(开发): 需在下位机 server/hostapi_commands.c 中实现
        CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR (CMD ID = 163) 的 handler，
        并在 src/test_api.c 中新增：
            uint32_t ehsm_test_inject_otp_write_error(ehsm_ctx_st *ctx);
        该函数设置注入标志，使下一次 otp_write 调用返回 0x123。
        同步更新 server/cmd_table.c 中的命令表和 CMD_MAX_COUNT。

        Reason: 覆盖 BUG-01 的 otp_write 失败分支（secboot.c:234/249 行），
        Python 侧无法直接控制 OTP 硬件故障，需要下位机 test CMD 注入。
        """
        t = 0
        try:
            t = hostapi.ehsm_test_inject_otp_write_error(
                self.CTX_ADDR,
            )
        except hostapi.HostApiError as e:
            if bl_errno.EHSM_ERR_INVALID_CMD == e.ret_code:
                # Reason: CMD 163 需要开发实现后才能生效，未实现时标记为 xfail，与 ehsm_test_read_memory 保持一致
                log.debug("error: 该命令不支持，请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译，"
                          "并在 hostapi_commands.c 中实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163)")
                pytest.xfail("预期失败：命令不支持,（34），请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译，"
                             "并在 hostapi_commands.c 中实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163)")
            else:
                raise
        except Exception as e:
            # Reason: 下位机未注册 CMD 163 时，soccmd 层在协议层面返回 RSP_ERR_UNKNOWN_CMD(1)，
            # 抛出 CmdRspError("1") 而非 HostApiError，同样标记为 xfail
            if "1" == str(e) or "RSP_ERR_UNKNOWN_CMD" in str(type(e).__name__):
                log.debug("error: 该命令不支持（下位机 RSP_ERR_UNKNOWN_CMD=1），请打开下位机 "
                          "CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译，"
                          "并在 hostapi_commands.c 中实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163)")
                pytest.xfail("预期失败：命令不支持（下位机 RSP_ERR_UNKNOWN_CMD=1），"
                             "请打开下位机 CONFIG_EHSM_TEST_CMDS_ENABLE 宏进行编译，"
                             "并在 hostapi_commands.c 中实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163)")
            else:
                raise
        return t

    @api
    @allure.step("RSA加密/解密计算 (使用明文密钥)")
    def ehsm_rsa_cipher_with_plain_key(
        self,
        key_data: bytes,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        # Reason: key_data应该已经是ehsm_rsa_key_st格式,但需要用实际的DATA5_ADDR重新打包地址
        # 如果key_data长度>76字节,说明是完整的打包结构(76字节结构+实际数据)
        # 我们需要用DATA5_ADDR作为基地址重新计算结构中的地址字段
        if len(key_data) > 76:
            import struct
            # 解析前76字节的结构
            # 格式: B(crt_mode) + 3B(reserved) + L(n_byte_sz) + L(e_byte_sz) + 8Q(8个地址)
            struct_header = key_data[:76]
            key_components = key_data[76:]  # 实际的密钥数据

            # 解析结构头
            unpacked = struct.unpack("<BBBBLLQQQQQQQQ", struct_header)
            crt_mode = unpacked[0]
            n_byte_sz = unpacked[4]
            e_byte_sz = unpacked[5]

            # Reason: 重新计算地址,基地址是DATA5_ADDR,数据从第76字节开始
            struct_size = 76
            data_base = self.DATA5_ADDR + struct_size
            current_offset = data_base

            # 计算各组件地址
            n_addr = current_offset
            current_offset += n_byte_sz

            e_addr = current_offset
            current_offset += e_byte_sz

            # 从原始数据中提取各组件来计算大小
            pos = 0
            n_data = key_components[pos:pos+n_byte_sz]
            pos += n_byte_sz
            e_data = key_components[pos:pos+e_byte_sz]
            pos += e_byte_sz

            # d的地址和大小
            # Reason: CRT模式下不使用d,只在NO_CRT模式下需要d
            if crt_mode == 0:
                # NO_CRT模式: d的大小等于n的大小
                if pos < len(key_components):
                    d_size = n_byte_sz
                    d_addr = current_offset
                    current_offset += d_size
                    pos += d_size
                else:
                    d_addr = 0
            else:
                # CRT模式: 不使用d,直接设为0
                d_addr = 0

            # CRT参数地址
            if crt_mode == 1 and pos < len(key_components):
                # p, q, dp, dq, u 的大小都是 n_byte_sz / 2
                crt_component_size = n_byte_sz // 2

                p_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                q_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                dp_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                dq_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                u_addr = current_offset
                current_offset += crt_component_size
            else:
                p_addr = q_addr = dp_addr = dq_addr = u_addr = 0

            # Reason: 重新打包结构体,使用新的地址
            new_struct_header = struct.pack(
                "<BBBBLLQQQQQQQQ",
                crt_mode,
                0, 0, 0,  # reserved
                n_byte_sz,
                e_byte_sz,
                n_addr,
                e_addr,
                d_addr,
                p_addr,
                q_addr,
                dp_addr,
                dq_addr,
                u_addr
            )

            # 重新组合完整的密钥数据
            key_data = new_struct_header + key_components

        # Reason: 写明文密钥到 DATA5
        host.write_memory(self.DATA5_ADDR, key_data)
        # Reason: 写输入数据到 DATA1
        host.write_memory(self.DATA1_ADDR, input)
        # Reason: 调用底层 UART 协议函数
        t, output_size = hostapi.ehsm_rsa_cipher_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            len(key_data),
            enc,
            self.DATA1_ADDR,
            input_size,
            self.DATA3_ADDR,
            output_buff_size
        )
        # Reason: 从 DATA3 读取输出数据
        _, output = host.read_memory(self.DATA3_ADDR, output_size)
        return t, output, output_size

    @api
    @allure.step("SM2加密/解密计算 (使用明文密钥)")
    def ehsm_sm2_cipher_with_plain_key(
        self,
        key_data: bytes,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        # Reason: 写明文密钥到 DATA5
        host.write_memory(self.DATA5_ADDR, key_data)
        # Reason: 写输入数据到 DATA1
        if input and input_size != 0:
            host.write_memory(self.DATA1_ADDR, input)
        # Reason: 调用底层 UART 协议函数
        t, ret_output_size = hostapi.ehsm_sm2_cipher_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            len(key_data),
            enc,
            self.DATA1_ADDR,
            input_size,
            self.DATA2_ADDR,
            output_buff_size
        )
        # Reason: 从 DATA2 读取输出数据
        if ret_output_size != 0:
            _, output = host.read_memory(self.DATA2_ADDR, ret_output_size)
        else:
            output = b''
        return t, output, ret_output_size

    @api
    @allure.step("RSA签名/验签计算初始化 (使用明文密钥)")
    def ehsm_rsa_sign_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        gen_sig: bool,
        padding: EhsmRsaPaddingMode,
        session: int
    ) -> tuple[int, int]:
        # Reason: key_data应该已经是ehsm_rsa_key_st格式,但需要用实际的DATA5_ADDR重新打包地址
        # 如果key_data长度>76字节,说明是完整的打包结构(76字节结构+实际数据)
        # 我们需要用DATA5_ADDR作为基地址重新计算结构中的地址字段
        if len(key_data) > 76:
            import struct
            # 解析前76字节的结构
            # 格式: B(crt_mode) + 3B(reserved) + L(n_byte_sz) + L(e_byte_sz) + 8Q(8个地址)
            struct_header = key_data[:76]
            key_components = key_data[76:]  # 实际的密钥数据

            # 解析结构头
            unpacked = struct.unpack("<BBBBLLQQQQQQQQ", struct_header)
            crt_mode = unpacked[0]
            n_byte_sz = unpacked[4]
            e_byte_sz = unpacked[5]

            # Reason: 重新计算地址,基地址是DATA5_ADDR,数据从第76字节开始
            struct_size = 76
            data_base = self.DATA5_ADDR + struct_size
            current_offset = data_base

            # 计算各组件地址
            n_addr = current_offset
            current_offset += n_byte_sz

            e_addr = current_offset
            current_offset += e_byte_sz

            # 从原始数据中提取各组件来计算大小
            pos = 0
            n_data = key_components[pos:pos+n_byte_sz]
            pos += n_byte_sz
            e_data = key_components[pos:pos+e_byte_sz]
            pos += e_byte_sz

            # d的地址和大小
            # Reason: CRT模式下不使用d,只在NO_CRT模式下需要d
            if crt_mode == 0:
                # NO_CRT模式: d的大小等于n的大小
                if pos < len(key_components):
                    d_size = n_byte_sz
                    d_addr = current_offset
                    current_offset += d_size
                    pos += d_size
                else:
                    d_addr = 0
            else:
                # CRT模式: 不使用d,直接设为0
                d_addr = 0

            # CRT参数地址
            if crt_mode == 1 and pos < len(key_components):
                # p, q, dp, dq, u 的大小都是 n_byte_sz / 2
                crt_component_size = n_byte_sz // 2

                p_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                q_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                dp_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                dq_addr = current_offset
                current_offset += crt_component_size
                pos += crt_component_size

                u_addr = current_offset
                current_offset += crt_component_size
            else:
                p_addr = q_addr = dp_addr = dq_addr = u_addr = 0

            # Reason: 重新打包结构体,使用新的地址
            new_struct_header = struct.pack(
                "<BBBBLLQQQQQQQQ",
                crt_mode,
                0, 0, 0,  # reserved
                n_byte_sz,
                e_byte_sz,
                n_addr,
                e_addr,
                d_addr,
                p_addr,
                q_addr,
                dp_addr,
                dq_addr,
                u_addr
            )

            # 重新组合完整的密钥数据
            key_data = new_struct_header + key_components

        # 写入重新打包后的密钥数据
        host.write_memory(self.DATA5_ADDR, key_data)

        # Reason: 调用底层 UART 协议函数
        t = hostapi.ehsm_rsa_sign_init_with_plain_key(
            self.CTX_ADDR,
            algo,
            self.DATA5_ADDR,
            len(key_data),  # 添加key_size参数
            int(gen_sig),
            padding,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("SM2签名/验签计算初始化 (使用明文密钥)")
    def ehsm_sm2_sign_init_with_plain_key(
        self,
        key_data: bytes,
        gen_sig: bool,
        session: int
    ) -> tuple[int, int]:
        # Reason: 写明文密钥到 DATA5
        host.write_memory(self.DATA5_ADDR, key_data)
        # Reason: 调用底层 UART 协议函数,传入密钥长度
        t = hostapi.ehsm_sm2_sign_init_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            len(key_data),  # 添加 key_size 参数
            gen_sig,
            self.SESSION_ADDR
        )
        return t, int(self.SESSION_ADDR)

    @api
    @allure.step("ECDSA签名/验签计算初始化 (使用明文密钥)")
    def ehsm_ecdsa_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        gen_sig: bool,
        session: int
    ) -> tuple[int, int]:
        # Reason: key_data 是 mb_ecc_key_st 结构,需要用正确的 base_addr 重新打包
        # 解包结构头: curve_id, privkey_addr, pubkey_addr
        curve_id, old_privkey_addr, old_pubkey_addr = struct.unpack("<LQQ", key_data[:20])

        # 提取私钥和公钥数据(跳过20字节结构头)
        key_components = key_data[20:]

        # 根据 curve_id 确定私钥大小
        ecc_privkey_sizes = {
            0x12: 20, 0x13: 24, 0x14: 28, 0x15: 32,
            0x16: 40, 0x17: 48, 0x18: 64,
            0x19: 24, 0x1a: 28, 0x1b: 32, 0x1c: 48, 0x1d: 66,
            0x26: 20, 0x27: 24, 0x28: 28, 0x29: 32,
        }
        privkey_size = ecc_privkey_sizes.get(curve_id, 32)

        # 提取私钥和公钥
        private_key = key_components[:privkey_size]
        public_key = key_components[privkey_size:]

        # 用 DATA5_ADDR 作为 base_addr 重新打包密钥结构
        struct_size = 20
        data_offset = struct_size
        base_addr = self.DATA5_ADDR
        current_offset = base_addr + data_offset

        # 重新打包密钥数据
        key_data_repacked = b''
        if private_key and any(b != 0 for b in private_key):  # 非零私钥
            privkey_addr = current_offset
            key_data_repacked += private_key
            current_offset += len(private_key)
        else:  # 全零私钥(验签模式)
            privkey_addr = current_offset
            key_data_repacked += b'\x00' * privkey_size
            current_offset += privkey_size

        if public_key:
            pubkey_addr = current_offset
            key_data_repacked += public_key
        else:
            pubkey_addr = 0

        # 打包 mb_ecc_key_st 结构头
        ecc_key_struct = struct.pack("<LQQ", curve_id, privkey_addr, pubkey_addr)
        repacked_key = ecc_key_struct + key_data_repacked

        # Reason: 写重新打包的密钥到 DATA5
        host.write_memory(self.DATA5_ADDR, repacked_key)
        # Reason: 调用底层 UART 协议函数
        t, session_handle = hostapi.ehsm_ecdsa_init_with_plain_key(
            self.CTX_ADDR,
            algo,
            self.DATA5_ADDR,
            gen_sig,
            self.SESSION_ADDR
        )
        return t, session_handle

    @api
    @allure.step("SM9加密/解密计算 (使用明文密钥)")
    def ehsm_sm9_cipher_with_plain_key(
        self,
        key_data: bytes,
        enc: bool,
        enc_type: EhsmSm9EncType,
        padding: EhsmSm9PaddingMode,
        key2_size: int,
        hid: int,
        kgc_pub_key: bytes,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        id: bytes,
        id_size: int,
        fp12g: bytes
    ) -> tuple[int, int, bytes]:
        # Reason: 写明文密钥到 DATA5
        if key_data:
            host.write_memory(self.DATA5_ADDR, key_data)
        # Reason: 写 KGC 公钥到 DATA1
        host.write_memory(self.DATA1_ADDR, kgc_pub_key)
        # Reason: 写 ID 到 DATA2
        host.write_memory(self.DATA2_ADDR, id)
        # Reason: 写 fp12g 到 DATA3 (解密时需要)
        fp12g_addr = 0
        if enc is False and fp12g is not None:
            host.write_memory(self.DATA3_ADDR, fp12g)
            fp12g_addr = self.DATA3_ADDR
        # Reason: 写输入数据到 DATA4
        host.write_memory(self.DATA4_ADDR, input)
        # Reason: 调用底层 UART 协议函数
        t, output_size = hostapi.ehsm_sm9_cipher_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            len(key_data) if key_data else 0,
            enc,
            enc_type,
            padding,
            key2_size,
            hid,
            self.DATA1_ADDR,
            self.DATA4_ADDR,
            input_size,
            self.DATA6_ADDR,
            output_buff_size,
            self.DATA2_ADDR,
            id_size,
            fp12g_addr
        )
        # Reason: 从 DATA6 读取输出数据
        if output_size > 0:
            _, actual_output = host.read_memory(self.DATA6_ADDR, output_size)
        else:
            actual_output = bytes()
        return t, output_size, actual_output

    @api
    @allure.step("SM9签名值生成 (使用明文密钥)")
    def ehsm_sm9_sign_onepass_gen_with_plain_key(
        self,
        key_data: bytes,
        msg: bytes,
        msg_size: int,
        sig_size: int,
        kgc_pub_key: bytes,
        fp12g: bytes
    ) -> tuple[int, bytes]:
        # Reason: 写明文密钥到 DATA5
        host.write_memory(self.DATA5_ADDR, key_data)
        # Reason: 写 KGC 公钥到 DATA1
        host.write_memory(self.DATA1_ADDR, kgc_pub_key)
        # Reason: 写 fp12g 到 DATA2
        host.write_memory(self.DATA2_ADDR, fp12g)
        # Reason: 写消息到 DATA3
        host.write_memory(self.DATA3_ADDR, msg)
        # Reason: 调用底层 UART 协议函数
        t = hostapi.ehsm_sm9_sign_onepass_gen_with_plain_key(
            self.CTX_ADDR,
            self.DATA5_ADDR,
            self.DATA3_ADDR,
            msg_size,
            self.DATA4_ADDR,
            sig_size,
            self.DATA1_ADDR,
            self.DATA2_ADDR
        )
        # Reason: 从 DATA4 读取签名数据
        _, sig = host.read_memory(self.DATA4_ADDR, sig_size)
        return t, sig

    def _get_data_addr_by_slot(self, slot_num: int) -> int:
        """
        根据槽位号（1-MAX_DATA_ADDR_COUNT）获取对应的DATA地址

        Reason: 辅助函数，用于根据槽位号动态获取DATA地址
        """
        if 1 <= slot_num <= self.MAX_DATA_ADDR_COUNT:
            return getattr(self, f'DATA{slot_num}_ADDR')

        # 槽位号越界，打印告警并返回默认值
        log.warning(
            f"槽位号越界: slot_num={slot_num}, 有效范围=[1, {self.MAX_DATA_ADDR_COUNT}], "
            f"已自动使用 DATA1_ADDR 作为默认值"
        )
        return self.DATA1_ADDR

    @api
    @allure.step("PQC DSA签名/验签统一接口")
    def ehsm_pqc_dsa_onepass_ex(
        self,
        sign_algo: EhsmPqcSignAlgo,
        sign_mode: EhsmPqcSignMode,
        hash_algo: EhsmPqcHashAlgo,
        is_det: bool,
        use_plain_key: bool,
        key_handle: int,
        key: Optional[bytes],
        key_type: EhsmKeyType,
        gen_sig: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
        signature: Optional[bytes] = None,
        context: Optional[bytes] = None
    ) -> tuple[int, bytes, int, bool]:
        """
        PQC DSA签名/验签统一接口

        Reason: 统一接口支持密钥句柄/明文密钥、签名/验签、原始消息/摘要的所有组合
        Reason: 增加 context 参数支持 ML-DSA context string（NIST FIPS 204 标准）
        Reason: 使用DATA1-16地址，根据数据长度自动计算占用的槽位数
        Reason: 添加 key_type 参数，根据 gen_sig 判断密钥类型，不再依赖密钥长度映射
        Reason: 添加 is_det 参数支持确定性/非确定性签名选择（默认值为 True 确定性，便于测试）

        内存槽位分配策略：
        - 每个DATA地址间隔2048字节
        - 根据数据长度计算占用槽位：slots = (size + 2047) // 2048
        - 例如：3072字节占用2个槽位（DATA1+DATA2），下个数据从DATA3开始
        """
        import struct

        # Reason: 每个DATA地址槽位大小为2048字节
        DATA_SLOT_SIZE = 2048
        current_slot = 1  # 从DATA1开始（槽位1）

        # 1. Input数据（消息或摘要）
        # Reason: 当消息长度为0时，地址必须传NULL（0），不分配槽位
        if input_size > 0:
            input_addr = self._get_data_addr_by_slot(current_slot)
            host.write_memory(input_addr, input)
            log.debug(f"[PQC DSA] Input: slot{current_slot} addr=0x{input_addr:x}, size={input_size}")
            # Reason: 计算input占用的槽位数
            slots_needed = (input_size + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE
            current_slot += slots_needed
            log.debug(f"[PQC DSA] Input uses {slots_needed} slot(s), next slot={current_slot}")
        else:
            input_addr = 0  # 长度为0时必须传NULL
            log.debug(f"[PQC DSA] Zero-length input, addr=NULL(0)")

        # 2. Signature数据（验签时输入，签名时输出）
        sig_size = len(signature) if signature else output_buff_size
        signature_addr = self._get_data_addr_by_slot(current_slot)

        if not gen_sig and signature and len(signature) > 0:
            host.write_memory(signature_addr, signature)
            log.debug(f"[PQC DSA] Signature (verify): slot{current_slot} addr=0x{signature_addr:x}, size={len(signature)}")
        else:
            log.debug(f"[PQC DSA] Signature (sign): reserve slot{current_slot} addr=0x{signature_addr:x}, max_size={output_buff_size}")

        # Reason: 签名槽位按最大值分配（验签用实际长度，签名用缓冲区大小）
        slots_needed = max(1, (max(sig_size, output_buff_size) + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE)
        current_slot += slots_needed
        log.debug(f"[PQC DSA] Signature uses {slots_needed} slot(s), next slot={current_slot}")

        # 3. 明文密钥处理（如果使用）
        key_addr = 0
        if use_plain_key and key:
            # Reason: 直接使用传入的 key_type 作为 algo_id
            algo_id = key_type
            key_len = len(key)

            # Reason: 检测是否为 SLH-DSA 算法（key_type 范围：0x36-0x41）
            is_slh_dsa = (0x36 <= algo_id <= 0x41)

            # Reason: SLH-DSA 密钥大小映射表（来自 slh_dsa_basic.h）
            slh_dsa_key_sizes = {
                0x36: (32, 32),  # SLH_DSA_SHA2_128S
                0x37: (32, 32),  # SLH_DSA_SHAKE_128S
                0x38: (32, 32),  # SLH_DSA_SHA2_128F
                0x39: (32, 32),  # SLH_DSA_SHAKE_128F
                0x3A: (48, 48),  # SLH_DSA_SHA2_192S
                0x3B: (48, 48),  # SLH_DSA_SHAKE_192S
                0x3C: (48, 48),  # SLH_DSA_SHA2_192F
                0x3D: (48, 48),  # SLH_DSA_SHAKE_192F
                0x3E: (64, 64),  # SLH_DSA_SHA2_256S
                0x3F: (64, 64),  # SLH_DSA_SHAKE_256S
                0x40: (64, 64),  # SLH_DSA_SHA2_256F
                0x41: (64, 64),  # SLH_DSA_SHAKE_256F
            }

            # Reason: 根据 gen_sig 和算法类型判断密钥处理方式
            if gen_sig:
                key_role = 'privkey'
            else:
                key_role = 'pubkey'

            # Reason: 算法参数集名称映射（用于日志）
            param_set_names = {
                0x33: 'ML-DSA-44',
                0x34: 'ML-DSA-65',
                0x35: 'ML-DSA-87',
                0x36: 'SLH-DSA-SHA2-128S',
                0x37: 'SLH-DSA-SHAKE-128S',
                0x38: 'SLH-DSA-SHA2-128F',
                0x39: 'SLH-DSA-SHAKE-128F',
                0x3A: 'SLH-DSA-SHA2-192S',
                0x3B: 'SLH-DSA-SHAKE-192S',
                0x3C: 'SLH-DSA-SHA2-192F',
                0x3D: 'SLH-DSA-SHAKE-192F',
                0x3E: 'SLH-DSA-SHA2-256S',
                0x3F: 'SLH-DSA-SHAKE-256S',
                0x40: 'SLH-DSA-SHA2-256F',
                0x41: 'SLH-DSA-SHAKE-256F',
            }
            param_set = param_set_names.get(algo_id, f'Unknown(0x{algo_id:x})')
            log.debug(f"[PQC DSA] Using {param_set} {key_role}: {key_len} bytes")

            # 3.1 密钥结构体（28字节，占用1个槽位）
            key_struct_addr = self._get_data_addr_by_slot(current_slot)
            current_slot += 1
            log.debug(f"[PQC DSA] Key struct: slot{current_slot-1} addr=0x{key_struct_addr:x}, size=28")

            # 3.2 实际密钥数据处理
            # Reason: SLH-DSA 签名需要同时提供公钥和私钥，验签只需公钥
            if is_slh_dsa and gen_sig:
                # Reason: SLH-DSA 签名模式：key 格式为 pk + sk
                expected_pk_size, expected_sk_size = slh_dsa_key_sizes[algo_id]
                expected_total_size = expected_pk_size + expected_sk_size

                if key_len != expected_total_size:
                    raise ValueError(f"SLH-DSA 签名密钥长度错误：期望 {expected_total_size} 字节（pk={expected_pk_size} + sk={expected_sk_size}），实际 {key_len} 字节")

                # Reason: 拆分 pk 和 sk
                pk = key[:expected_pk_size]
                sk = key[expected_pk_size:]

                # Reason: 分配两个不同的内存地址，先写公钥
                pubkey_addr = self._get_data_addr_by_slot(current_slot)
                host.write_memory(pubkey_addr, pk)
                slots_needed_pk = max(1, (expected_pk_size + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE)
                log.debug(f"[PQC DSA] Public key: slot{current_slot} addr=0x{pubkey_addr:x}, size={expected_pk_size}, uses {slots_needed_pk} slot(s)")
                current_slot += slots_needed_pk

                # Reason: 再写私钥
                privkey_addr = self._get_data_addr_by_slot(current_slot)
                host.write_memory(privkey_addr, sk)
                slots_needed_sk = max(1, (expected_sk_size + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE)
                log.debug(f"[PQC DSA] Private key: slot{current_slot} addr=0x{privkey_addr:x}, size={expected_sk_size}, uses {slots_needed_sk} slot(s)")
                current_slot += slots_needed_sk

                pubkey_size = expected_pk_size
                privkey_size = expected_sk_size
            else:
                # Reason: ML-DSA 或 SLH-DSA 验签模式：单一密钥处理
                key_data_addr = self._get_data_addr_by_slot(current_slot)
                host.write_memory(key_data_addr, key)

                if gen_sig:
                    # ML-DSA 签名模式: 只有私钥
                    privkey_addr = key_data_addr
                    privkey_size = key_len
                    pubkey_addr = 0
                    pubkey_size = 0
                else:
                    # 验签模式: 只有公钥
                    privkey_addr = 0
                    privkey_size = 0
                    pubkey_addr = key_data_addr
                    pubkey_size = key_len

                slots_needed = max(1, (key_len + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE)
                log.debug(f"[PQC DSA] Key data: slot{current_slot} addr=0x{key_data_addr:x}, size={key_len}, uses {slots_needed} slot(s)")
                current_slot += slots_needed

            # Reason: 构造mb_pqc_key_st结构体（28字节）
            # struct格式: <IQIQI (小端)
            # - algo_id (uint32): ML-DSA-44=0x33, ML-DSA-65=0x34, ML-DSA-87=0x35, SLH-DSA=0x36-0x41
            # - privkey (uint64): 私钥地址
            # - privkey_size (uint32): 私钥大小
            # - pubkey (uint64): 公钥地址
            # - pubkey_size (uint32): 公钥大小
            pqc_key_struct = struct.pack(
                "<IQIQI",
                algo_id,
                privkey_addr,
                privkey_size,
                pubkey_addr,
                pubkey_size
            )

            # 写入结构体
            host.write_memory(key_struct_addr, pqc_key_struct)
            key_addr = key_struct_addr
            log.debug(f"[PQC DSA] Key struct written: algo_id=0x{algo_id:x}, "
                     f"privkey@0x{privkey_addr:x}({privkey_size}B), "
                     f"pubkey@0x{pubkey_addr:x}({pubkey_size}B)")

        # 4. Context string（如果提供）
        pqc_ctx_str_addr = 0
        pqc_ctx_str_size = 0
        if context and len(context) > 0:
            ctx_addr = self._get_data_addr_by_slot(current_slot)
            host.write_memory(ctx_addr, context)
            pqc_ctx_str_addr = ctx_addr
            pqc_ctx_str_size = len(context)

            slots_needed = max(1, (len(context) + DATA_SLOT_SIZE - 1) // DATA_SLOT_SIZE)
            log.debug(f"[PQC DSA] Context: slot{current_slot} addr=0x{ctx_addr:x}, size={len(context)}, uses {slots_needed} slot(s)")
            current_slot += slots_needed
        else:
            log.debug(f"[PQC DSA] No context")

        # 5. Verify result地址（4字节，占用1个槽位）
        verify_result_addr = self._get_data_addr_by_slot(current_slot)
        log.debug(f"[PQC DSA] Verify result: slot{current_slot} addr=0x{verify_result_addr:x}")

        # 6. 调用底层 UART 协议函数
        log.debug(f"[PQC DSA] Call hostapi: use_plain_key={use_plain_key}, key_handle={key_handle:#x}, "
                  f"key_addr=0x{key_addr:x}, gen_sig={gen_sig}, input_size={input_size}, "
                  f"output_buff_size={output_buff_size}, is_det={is_det}")

        t, actual_sig_size, verify_result = hostapi.ehsm_pqc_dsa_onepass_ex(
            self.CTX_ADDR,
            sign_algo,
            sign_mode,
            hash_algo,
            is_det,
            use_plain_key,
            key_handle,
            key_addr,
            gen_sig,
            input_addr,
            input_size,
            pqc_ctx_str_addr,
            pqc_ctx_str_size,
            signature_addr,
            output_buff_size,
            verify_result_addr
        )

        # 7. 读取输出
        # 签名模式：从 signature_addr 读取签名数据
        if gen_sig and actual_sig_size > 0:
            _, sig = host.read_memory(signature_addr, actual_sig_size)
            log.debug(f"[PQC DSA] Signature (gen): read {actual_sig_size} bytes from 0x{signature_addr:x}")
        else:
            sig = b''

        log.debug(f"[PQC DSA] Result: t={t}, sig_size={actual_sig_size}, verify_result={verify_result}")
        return t, sig, actual_sig_size, bool(verify_result)

    @api
    @allure.step("ML-KEM密钥封装/解封统一接口")
    def ehsm_pqc_ml_kem_ex(
        self,
        is_encaps: bool,
        use_plain_parent_key: bool,
        parent_key_type: EhsmKeyType,
        parent_key_handle: int,
        parent_key: Optional[bytes],
        cipher_key_data: bytes,
        cipher_key_data_size: int,
        ss_out_type: EhsmPqcOutType,
        ss_key_type: EhsmKeyType,
        ss_privilege: int,
        ss_key_handle: int,
        ss_key_size: int
    ) -> tuple[int, bytes, int, int, bytes, int]:
        """
        ML-KEM密钥封装/解封统一接口

        Reason: 支持封装/解封、密钥句柄/明文密钥、输出密钥句柄/明文密钥的所有组合
        """
        # Reason: 写入明文父密钥(如果使用明文父密钥)
        parent_key_addr = 0
        if use_plain_parent_key and parent_key:
            import struct

            # Reason: 直接使用传入的 parent_key_type 作为 algo_id
            algo_id = parent_key_type
            key_len = len(parent_key)

            # Reason: 根据 algo_id 确定 ML-KEM 参数集的公钥偏移位置
            # ML-KEM-512(0x30): dk中pk偏移=768, ek=800, dk=1632
            # ML-KEM-768(0x31): dk中pk偏移=1152, ek=1184, dk=2400
            # ML-KEM-1024(0x32): dk中pk偏移=1536, ek=1568, dk=3168
            ml_kem_params = {
                0x30: {'dk_pk_offset': 768, 'ek_size': 800, 'dk_size': 1632},
                0x31: {'dk_pk_offset': 1152, 'ek_size': 1184, 'dk_size': 2400},
                0x32: {'dk_pk_offset': 1536, 'ek_size': 1568, 'dk_size': 3168},
            }

            if algo_id not in ml_kem_params:
                raise ValueError(f"Invalid parent_key_type: {hex(algo_id)}. "
                               f"Expected 0x30 (ML-KEM-512), 0x31 (ML-KEM-768), or 0x32 (ML-KEM-1024)")

            params = ml_kem_params[algo_id]
            dk_pk_offset = params['dk_pk_offset']

            # Reason: 根据 is_encaps 判断密钥类型（类似 DSA 中的 gen_sig）
            # is_encaps=True: 封装操作，使用公钥(ek)
            # is_encaps=False: 解封装操作，使用私钥(dk)
            if is_encaps:
                # Reason: 封装操作，使用公钥
                key_role = 'pubkey'
                privkey_addr = 0
                privkey_size = 0

                # Reason: 写入公钥数据到 DATA5
                host.write_memory(self.DATA5_ADDR, parent_key)
                pubkey_addr = self.DATA5_ADDR
                pubkey_size = key_len

            else:
                # Reason: 解封装操作，使用私钥
                key_role = 'privkey'
                pubkey_addr = 0
                pubkey_size = 0

                # Reason: 写入私钥数据到 DATA6
                host.write_memory(self.DATA6_ADDR, parent_key)
                privkey_addr = self.DATA6_ADDR
                privkey_size = key_len

                # Reason: dk格式：s || pk || H(pk)(32) || z(32)
                # 固件会从 dk[dk_pk_offset] 自动提取公钥
                # 设置 pubkey_addr 指向 dk 中的公钥位置
                pubkey_addr = privkey_addr + dk_pk_offset
                pubkey_size = params['ek_size']

            # Reason: 构造 mb_pqc_key_st 结构体（28字节）
            # struct格式: <IQIQI (小端)
            # - algo_id (uint32): ML-KEM-512 = 0x30, ML-KEM-768 = 0x31, ML-KEM-1024 = 0x32
            # - privkey (uint64): 私钥地址
            # - privkey_size (uint32): 私钥大小
            # - pubkey (uint64): 公钥地址
            # - pubkey_size (uint32): 公钥大小
            pqc_key_struct = struct.pack(
                "<IQIQI",
                algo_id,
                privkey_addr,
                privkey_size,
                pubkey_addr,
                pubkey_size
            )

            # Reason: 写入结构体到 DATA1
            host.write_memory(self.DATA1_ADDR, pqc_key_struct)
            parent_key_addr = self.DATA1_ADDR

        # Reason: DATA2 用于密文密钥数据
        if is_encaps:
            # Reason: 封装模式：DATA2 是输出缓冲区
            cipher_key_data_addr = self.DATA2_ADDR
        else:
            # Reason: 解封模式：写入密文数据到 DATA2
            host.write_memory(self.DATA2_ADDR, cipher_key_data)
            cipher_key_data_addr = self.DATA2_ADDR

        # Reason: DATA3 用于共享密钥输出(明文密钥模式)
        ss_key_addr = self.DATA3_ADDR

        # Reason: DATA4 用于共享密钥句柄输出
        ss_key_handle_addr = self.DATA4_ADDR

        # Reason: 写入输入的密钥句柄值到内存
        # 解封装时：如果 ss_key_handle != 0，表示要使用指定的句柄（0xFFFFFFFF表示自动分配）
        # 封装时：固件会将生成的句柄写入此地址
        if ss_key_handle != 0:
            host.write_memory(ss_key_handle_addr, ss_key_handle.to_bytes(4, 'little'))
            log.debug(f"[ML-KEM] Write ss_key_handle=0x{ss_key_handle:08x} to addr=0x{ss_key_handle_addr:x}")

        # Reason: 初始化执行时间变量，以防异常时未赋值
        t = 0

        # Reason: 调用底层 UART 协议函数
        # 对于解封装操作，即使返回错误码也需要读取 fake_K 用于测试验证
        try:
            t, actual_cipher_size, actual_ss_key_handle = hostapi.ehsm_pqc_ml_kem_ex(
                self.CTX_ADDR,
                is_encaps,
                use_plain_parent_key,
                parent_key_handle,
                parent_key_addr,
                cipher_key_data_addr,
                cipher_key_data_size,
                int(ss_out_type),  # Reason: 显式转换枚举为整数值
                int(ss_key_type),  # Reason: 显式转换枚举为整数值
                ss_privilege,
                ss_key_handle_addr,
                ss_key_addr,
                ss_key_size
            )
        except hostapi.HostApiError as e:
            # Reason: 对于解封装操作，即使发生错误（如 C 值错误返回514），也需要读取返回的 fake_K
            if not is_encaps:
                log.info(f"解封装操作返回错误码 {e.ret_code}，但仍然读取 ss_key (fake_K) 用于验证")
                # Reason: 发生错误时，cipher_size 保持原值，ss_key_handle 为 0
                actual_cipher_size = cipher_key_data_size
                actual_ss_key_handle = 0
                # Reason: 即使错误，也读取 ss_key 用于验证
                if ss_out_type == EhsmPqcOutType.EHSM_PQC_OUT_PLAIN_KEY and ss_key_size > 0:
                    _, ss_key = host.read_memory(self.DATA3_ADDR, ss_key_size)
                    _, cipher_data = host.read_memory(self.DATA2_ADDR, cipher_key_data_size)
                    # Reason: 返回 fake_K，允许测试验证其与期望 K 值是否匹配
                    return t, cipher_data, actual_cipher_size, actual_ss_key_handle, ss_key, ss_key_size
            # Reason: 封装操作或其他错误情况，继续抛出异常
            raise

        # Reason: 读取密文密钥数据（仅封装模式需要读取生成的密文）
        if is_encaps:
            # Reason: 封装模式：从 DATA2 读取生成的密文
            _, cipher_data = host.read_memory(self.DATA2_ADDR, actual_cipher_size)
        else:
            # Reason: 解封装模式：密文是输入参数，直接使用
            cipher_data = cipher_key_data

        # Reason: 读取共享密钥(如果输出类型是明文密钥)
        if ss_out_type == EhsmPqcOutType.EHSM_PQC_OUT_PLAIN_KEY and ss_key_size > 0:
            _, ss_key = host.read_memory(self.DATA3_ADDR, ss_key_size)
        else:
            ss_key = b''

        return t, cipher_data, actual_cipher_size, actual_ss_key_handle, ss_key, ss_key_size
