from abc import ABC, abstractmethod
from typing import Tuple, Optional, Union
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


class ApiInterface(ABC):
    # 数据地址数量上限配置
    # old otp_base 0x60040000， max = 0x40000 / 2048 - 2 = 126  (可用空间: 0x40000 = 262144 bytes)
    # new otp_base 0x60070000， max = 0x70000 / 2048 - 2 = 222  (可用空间: 0x70000 = 458752 bytes)
    # normal stack size = MAX_DATA_ADDR_COUNT * 2048  (除CTX和SESSION外的通用可用RAM大小)
    MAX_DATA_ADDR_COUNT = 64

    def __init__(self, default_ctx=0x6000_0000):
        super().__init__()
        self.CTX_ADDR = default_ctx
        self.SESSION_ADDR = self.CTX_ADDR + 2048 * 1
        # 动态生成 DATA1_ADDR 到 DATA{MAX_DATA_ADDR_COUNT}_ADDR
        for i in range(1, self.MAX_DATA_ADDR_COUNT + 1):
            setattr(self, f'DATA{i}_ADDR', self.CTX_ADDR + 2048 * (i + 1))
        self.DATA_SIZE = 2048

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_aead_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_aead_finish_enc(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes, bytes]:
        pass

    @abstractmethod
    def ehsm_aead_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes
    ) -> tuple[int, bytes, bool]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_bl_close_debug(self, challenge_type: EhsmChallengeType, soc_dbg_bitmap: Optional[bytes]) -> int:
        pass

    @abstractmethod
    def ehsm_bl_debug_auth(
        self,
        challenge_type: EhsmChallengeType,
        algo: EhsmAuthAlgo,
        sig: bytes,
        sig_size: int,
        pub_key: Optional[bytes],
        pub_key_size: int,
        soc_dbg_bitmap: Optional[bytes],
    ) -> int:
        pass

    @abstractmethod
    def ehsm_bl_encrypt_key(
        self, key_level: EhsmKeyLevel, input_data: bytes, size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_fw_auth(
        self, type: int, arg: bytes, auth_data: bytes
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_get_challenge(
        self, challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_get_random_key(
        self, key_level: EhsmKeyLevel, key_type: EhsmBlGenKeyType
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_get_self_test_result(self) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_get_socid(self) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_get_version(self) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_inject_error(self, values: bytes) -> int:
        pass

    @abstractmethod
    def ehsm_bl_read_otp(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_read_reg(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_self_test(self) -> int:
        pass

    @abstractmethod
    def ehsm_bl_set_uart_baudrate(self, baud_div: int) -> int:
        pass

    @abstractmethod
    def ehsm_bl_set_hsm_freq(self, hsm_freq: int) -> int:
        pass

    @abstractmethod
    def ehsm_bl_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_bl_verify_image_discrete(
        self,
        image_header: bytes,
        image_size: int,
        image_code: Optional[bytes],
        only_copy_code: bool,
        check_version: bool,
        boot: bool
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
        pass

    @abstractmethod
    def ehsm_bl_write_otp(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    @abstractmethod
    def ehsm_bl_write_reg(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_chacha_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_chacha_finish_enc(
        self,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        pass

    @abstractmethod
    def ehsm_chacha_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_change_control_field(self, field: EhsmCtrlField, value: bytes) -> int:
        pass

    @abstractmethod
    def ehsm_change_lifecycle(self, lifecycle: EhsmLifecycle) -> int:
        pass

    @abstractmethod
    def ehsm_close_debug(
        self, type: EhsmChallengeType, soc_dbg_bitmap: Optional[bytes]
    ) -> int:
        pass

    @abstractmethod
    def ehsm_create_counter(self) -> tuple[int, bytes, bytes]:
        pass

    @abstractmethod
    def ehsm_ctx_init(self, mb_ch: int, is_async: bool) -> None:
        pass

    @abstractmethod
    def ehsm_ctx_deinit(self) -> None:
        pass

    @abstractmethod
    def ehsm_ctx_poll(self) -> int:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_delete_counter(self, counter_id: int) -> int:
        pass

    @abstractmethod
    def ehsm_driver_get_version(self) -> int:
        pass

    @abstractmethod
    def ehsm_driver_init_library(self, drv_mode: EhsmDrvMode) -> int:
        pass

    @abstractmethod
    def ehsm_ecdsa_finish_gen(self, sig: bytes, sig_size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_ecdsa_finish_verify(
        self, sig: bytes, sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_ecdsa_init(
        self, algo: EhsmHashAlgo, key_handle: int, gen_sig: int, session: bytes
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_ecdsa_onepass_gen(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int,
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_ecdsa_onepass_verify(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_ecdsa_onepass_gen_with_digest(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int,
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_ecdsa_onepass_verify_with_digest(
        self,
        hash_algo: EhsmHashAlgo,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_ecdsa_update(self, msg: bytes, msg_size: int) -> int:
        pass

    @abstractmethod
    def ehsm_enter_wfi(self) -> int:
        pass

    @abstractmethod
    def ehsm_gen_random(
        self, algo: EhsmRngAlgo, rand_buf: bytes, rand_size: int, skip_read_output: bool = False
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_get_challenge(
        self, challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_get_emu_status(
        self,
        emu_addr: int = None
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_get_utc_time(self, utc_time: bytes) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_get_version(self, version: bytes) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_hash_finish(
        self, digest: bytes, digest_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_hash_init(self, algo: EhsmHashAlgo, session: bytes) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_hash_onepass(
        self,
        algo: EhsmHashAlgo,
        msg: bytes,
        msg_size: int,
        digest: bytes,
        digest_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_hash_update(self, msg: bytes, msg_size: int) -> int:
        pass

    @abstractmethod
    def ehsm_hmac_finish_gen(self, hmac: bytes, hmac_size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_hmac_finish_verify(
        self, hmac: bytes, hmac_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_hmac_init(
        self, algo: EhsmHashAlgo, key_handle: int, gen_hmac: bool, session: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_hmac_onepass_gen(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_hmac_onepass_verify(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        hmac: bytes,
        hmac_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_hmac_update(self, msg: bytes, msg_size: int) -> int:
        pass

    @abstractmethod
    def ehsm_hmac_onepass_gen_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        key_size: int,
        msg: bytes,
        msg_size: int,
        hmac_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_hmac_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        key_size: int,
        gen_hmac: bool,
        session: int
    ) -> tuple[int, int]:
        pass

    @abstractmethod
    def ehsm_increase_counter(
        self, counter_id: int, increase_value: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_inject_error(self, values: bytes) -> int:
        pass

    @abstractmethod
    def ehsm_install_encrypted_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmInstallKeyType,
        key_slot_id: int,
        last_key: int,
        input_data: bytes,
        size: int,
    ) -> int:
        pass

    @abstractmethod
    def ehsm_install_random_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmInstallKeyType,
        key_slot_id: int,
        last_key: int,
    ) -> int:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_km_get_pub_from_priv(
        self,
        key_handle: int,
        dh_params: bytes,
        dh_params_size: int,
        pub_key_size: int,
    ) -> tuple[int, bytes, int, int]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_km_remove_key(self, key_handle: int) -> int:
        pass

    @abstractmethod
    def ehsm_km_sm9_exchange_key(
        self,
        privilege: int,
        key_type: EhsmKeyType,
        role: int,
        user_priv_key_handle: int,
        user_tmp_key_handle: int,
        hmac_key_size: int,
        extra_params: int,
    ) -> tuple[int, int]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_mac_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    @abstractmethod
    def ehsm_mac_finish_gen(
        self,
        msg: bytes,
        msg_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_mac_finish_verify(
        self,
        msg: bytes,
        msg_size: int,
        mac: bytes
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_read_counter(self, counter_id: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_read_otp(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_read_reg(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_rsa_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_rsa_sign_init(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        gen_sig: bool,
        padding: EhsmRsaPaddingMode,
        session: int
    ) -> tuple[int, int]:
        pass

    @abstractmethod
    def ehsm_rsa_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    @abstractmethod
    def ehsm_rsa_sign_finish_gen(
        self,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_rsa_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
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

        参数:
            algo: HASH算法
            use_plain_key: 是否使用明文密钥
            key_handle: 密钥句柄(use_plain_key=False时使用)
            key_data: 明文密钥数据(use_plain_key=True时使用)
            padding: RSA填充模式
            gen_sig: True=签名, False=验签
            is_digest: True=输入是摘要, False=输入是原始消息
            input: 输入数据(消息或摘要)
            input_size: 输入数据长度
            output_buff_size: 输出缓冲区大小
            salt_size: PSS模式的salt大小

        返回:
            (exec_time, signature, sig_size, verify_result)
            - 签名时: signature有效, verify_result无效
            - 验签时: signature无效, verify_result有效
        """
        pass

    @abstractmethod
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

        参数:
            use_plain_key: 是否使用明文密钥
            key_handle: 密钥句柄(use_plain_key=False时使用)
            key_data: 明文密钥数据(use_plain_key=True时使用)
            gen_sig: True=签名, False=验签
            is_digest: True=输入是摘要(E值), False=输入是原始消息
            input: 输入数据(消息或摘要)
            input_size: 输入数据长度
            output_buff_size: 输出缓冲区大小
            signature: 验签模式下需要传入的签名数据

        返回:
            (exec_time, signature, sig_size, verify_result)
            - 签名时: signature有效, verify_result无效
            - 验签时: signature无效, verify_result有效
        """
        pass

    @abstractmethod
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

        参数:
            algo: HASH算法
            use_plain_key: 是否使用明文密钥
            key_handle: 密钥句柄(use_plain_key=False时使用)
            key_data: 明文密钥数据(use_plain_key=True时使用, mb_ecc_key_st结构)
            gen_sig: True=签名, False=验签
            is_digest: True=输入是摘要, False=输入是原始消息
            input: 输入数据(消息或摘要)
            input_size: 输入数据长度
            output_buff_size: 输出缓冲区大小
            signature: 验签模式下需要传入的签名数据

        返回:
            (exec_time, signature, sig_size, verify_result)
            - 签名时: signature有效, verify_result无效
            - 验签时: signature无效, verify_result有效
        """
        pass

    @abstractmethod
    def ehsm_set_uart_baudrate(self, baud_div: int) -> int:
        pass

    @abstractmethod
    def ehsm_set_utc_time(self, utc_time: int) -> int:
        pass

    @abstractmethod
    def ehsm_sm2_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_rsa_cipher_with_plain_key(
        self,
        key_data: bytes,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_sm2_cipher_with_plain_key(
        self,
        key_data: bytes,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_rsa_sign_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        gen_sig: bool,
        padding: EhsmRsaPaddingMode,
        session: int
    ) -> tuple[int, int]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_init_with_plain_key(
        self,
        key_data: bytes,
        gen_sig: bool,
        session: int
    ) -> tuple[int, int]:
        pass

    @abstractmethod
    def ehsm_ecdsa_init_with_plain_key(
        self,
        algo: EhsmHashAlgo,
        key_data: bytes,
        gen_sig: bool,
        session: int
    ) -> tuple[int, int]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_sm9_sign_onepass_gen_with_plain_key(
        self,
        key_data: bytes,
        msg: bytes,
        msg_size: int,
        sig_size: int,
        kgc_pub_key: bytes,
        fp12g: bytes
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_onepass_gen(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_onepass_gen_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_onepass_verify(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_onepass_verify_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_init(
        self,
        key_handle: int,
        gen_sig: bool,
        session: bytes
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    @abstractmethod
    def ehsm_sm2_sign_finish_gen(
        self,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm2_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    @abstractmethod
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
        fp12g: bytes
    ) -> tuple[int, int, bytes]:
        pass

    @abstractmethod
    def ehsm_sm9_sign_onepass_gen(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig_size: int,
        kgc_pub_key: bytes,
        fp12g: Optional[bytes]
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        skip_read_output: bool = False,
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
    def ehsm_symm_cipher_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_symm_cipher_finish(
        self,
        input: bytes,
        input_size: int,
        output_size: int
    ) -> tuple[int, bytes, int]:
        pass

    @abstractmethod
    def ehsm_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_upgrade_fw_image_init(
        self,
        image_header: bytes,
        header_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        """Three-stage upgrade firmware image - init stage"""
        pass

    @abstractmethod
    def ehsm_upgrade_fw_image_update(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        """Three-stage upgrade firmware image - update stage"""
        pass

    @abstractmethod
    def ehsm_upgrade_fw_image_finish(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        """Three-stage upgrade firmware image - finish stage"""
        pass

    @abstractmethod
    def ehsm_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool,
        image_out_byte: int= None
    ) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_verify_tbbr_img(
        self,
        img_size: int,
        img_data: bytes,
        trusted_fw_nv_ctr_in_otp: int,
        non_trusted_fw_nv_ctr_in_otp: int,
    ) -> tuple[int, bytes, bytes]:
        pass

    @abstractmethod
    def ehsm_write_otp(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    @abstractmethod
    def ehsm_write_reg(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    @abstractmethod
    def ehsm_test_read_memory(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    @abstractmethod
    def ehsm_test_write_memory(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    @abstractmethod
    def ehsm_test_jump_to_addr(self, addr: int) -> int:
        pass

    @abstractmethod
    def ehsm_test_jump_to_loop(self) -> int:
        pass

    @abstractmethod
    def ehsm_test_inject_otp_write_error(self) -> int:
        """
        注入 OTP 写失败占位接口。
        TODO(开发): 下位机实现 CMD_HOSTAPI_TEST_INJECT_OTP_WRITE_ERROR(163) 后本函数生效。
        调用后固件 otp_write 返回 0x123，触发 secboot_update_ver_cnt 的失败分支。
        """
        pass

    @abstractmethod
    def ehsm_pqc_dsa_onepass_ex(
        self,
        sign_algo: EhsmPqcSignAlgo,
        sign_mode: EhsmPqcSignMode,
        hash_algo: EhsmPqcHashAlgo,
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
        PQC DSA 签名/验签统一接口

        参数:
            sign_algo: PQC 签名算法
            sign_mode: PQC 签名模式（Pure DSA with message 或 Pre-Hash DSA with digest）
            hash_algo: Pre-Hash DSA 使用的 Hash 算法（仅当 sign_mode 为 PRE_HASH_DSA_WITH_DIGEST 时有效）
            use_plain_key: 是否使用明文密钥
            key_handle: 密钥句柄（use_plain_key=False 时使用）
            key: 明文密钥数据（use_plain_key=True 时使用）
            key_type: 密钥类型（EhsmKeyType.EHSM_KEY_TYPE_ML_DSA_44/65/87）
            gen_sig: True=签名, False=验签
            input: 输入数据（消息或摘要）
            input_size: 输入数据长度
            output_buff_size: 输出缓冲区大小
            signature: 验签模式下需要传入的签名数据
            context: ML-DSA context string（可选，0-255字节）

        返回:
            (exec_time, signature, sig_size, verify_result)
            - 签名时: signature 有效, verify_result 无效
            - 验签时: signature 无效, verify_result 有效
        """
        pass

    @abstractmethod
    def ehsm_pqc_ml_kem_ex(
        self,
        is_encaps: bool,
        use_plain_parent_key: bool,
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
        ML-KEM 密钥封装/解封统一接口

        参数:
            is_encaps: True=封装, False=解封
            use_plain_parent_key: 是否使用明文父密钥
            parent_key_handle: 父密钥句柄（use_plain_parent_key=False 时使用）
            parent_key: 明文父密钥数据（use_plain_parent_key=True 时使用）
            cipher_key_data: 密文数据缓冲区
            cipher_key_data_size: 封装时为缓冲区大小，解封时为密文数据实际长度
            ss_out_type: 共享密钥输出类型（密钥句柄或明文密钥）
            ss_key_type: 共享密钥类型
            ss_privilege: 共享密钥权限
            ss_key_handle: 共享密钥句柄
            ss_key_size: 共享密钥大小

        返回:
            (exec_time, cipher_key_data, cipher_key_data_size, ss_key_handle, ss_key, ss_key_size)
        """
        pass