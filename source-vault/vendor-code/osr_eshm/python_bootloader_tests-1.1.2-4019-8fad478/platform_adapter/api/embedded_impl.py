from utils.util import api
from .base import ApiInterface
import allure
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
)


class EmbeddedApi(ApiInterface):

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

    def ehsm_aead_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_aead_finish_enc(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes, bytes]:
        pass

    def ehsm_aead_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes
    ) -> tuple[int, bytes, bool]:
        pass

    def ehsm_bl_close_debug(self, type: int, soc_dbg_bitmap: Optional[bytes]) -> int:
        pass

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

    def ehsm_bl_encrypt_key(
        self, key_level: EhsmKeyLevel, input_data: bytes, size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_fw_auth(
        self, type: int, arg: bytes, auth_data: bytes
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_get_challenge(
        self, challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_get_random_key(
        self, key_level: EhsmKeyLevel, key_type: EhsmBlGenKeyType
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_get_self_test_result(self) -> tuple[int, bytes]:
        pass

    def ehsm_bl_get_socid(self) -> int:
        pass

    def ehsm_bl_get_version(self) -> tuple[int, bytes]:
        pass

    def ehsm_bl_inject_error(self, values: bytes) -> int:
        pass

    def ehsm_bl_read_otp(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    def ehsm_bl_read_reg(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    def ehsm_bl_self_test(self) -> int:
        pass

    def ehsm_bl_set_uart_baudrate(self, baud_div: int) -> int:
        pass

    def ehsm_bl_set_hsm_freq(self, hsm_freq: int) -> int:
        pass

    def ehsm_bl_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool
    ) -> tuple[int, bytes]:
        pass

    def ehsm_bl_verify_image_discrete(
        self,
        image_header: bytes,
        image_size: int,
        image_code: Optional[bytes],
        only_copy_code: bool,
        check_version: bool,
        boot: bool
    ) -> tuple[int, bytes]:
        """镜像头和代码分离校验（未实现）"""
        raise NotImplementedError("此平台暂不支持镜像头和代码分离校验")

    def ehsm_bl_write_otp(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    def ehsm_bl_write_reg(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

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

    def ehsm_chacha_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_chacha_finish_enc(
        self,
        input: bytes,
        input_size: int,
        tag_size: int
    ) -> tuple[int, bytes, bytes]:
        pass

    def ehsm_chacha_finish_dec(
        self,
        input: bytes,
        input_size: int,
        tag: bytes,
        tag_size: int
    ) -> tuple[int, bytes, bool]:
        pass

    def ehsm_change_control_field(self, field: EhsmCtrlField, value: int) -> int:
        pass

    def ehsm_change_lifecycle(self, lifecycle: EhsmLifecycle) -> int:
        pass

    def ehsm_close_debug(
        self, type: EhsmChallengeType, soc_dbg_bitmap: Optional[bytes]
    ) -> int:
        pass

    def ehsm_create_counter(self) -> tuple[int, bytes, bytes]:
        pass

    def ehsm_ctx_init(self, mb_ch: int, is_async: bool) -> None:
        pass

    def ehsm_ctx_deinit(self) -> None:
        pass

    def ehsm_ctx_poll(self) -> int:
        pass

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

    def ehsm_delete_counter(self, counter_id: int) -> int:
        pass

    def ehsm_driver_get_version(self) -> int:
        pass

    def ehsm_driver_init_library(self, drv_mode: EhsmDrvMode) -> int:
        pass

    def ehsm_ecdsa_finish_gen(self, sig: bytes, sig_size: int) -> tuple[int, bytes]:
        pass

    def ehsm_ecdsa_finish_verify(
        self, sig: bytes, sig_size: int
    ) -> tuple[int, bool]:
        pass

    def ehsm_ecdsa_init(
        self, algo: EhsmHashAlgo, key_handle: int, gen_sig: int, session: bytes
    ) -> tuple[int, bytes]:
        pass

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

    def ehsm_ecdsa_update(self, msg: bytes, msg_size: int) -> int:
        pass

    def ehsm_enter_wfi(self) -> int:
        pass

    def ehsm_gen_random(
        self, algo: EhsmRngAlgo, rand_buf: bytes, rand_size: int, skip_read_output: bool = False
    ) -> tuple[int, bytes]:
        pass

    def ehsm_get_challenge(
        self, challenge_type: EhsmChallengeType
    ) -> tuple[int, bytes]:
        pass

    def ehsm_get_emu_status(self) -> tuple[int, bytes]:
        pass

    def ehsm_get_utc_time(self, utc_time: bytes) -> tuple[int, bytes]:
        pass

    def ehsm_get_version(self, version: bytes) -> tuple[int, bytes]:
        pass

    def ehsm_hash_finish(
        self, digest: bytes, digest_size: int
    ) -> tuple[int, bytes, int]:
        pass

    def ehsm_hash_init(self, algo: EhsmHashAlgo, session: int) -> tuple[int, bytes]:
        pass

    def ehsm_hash_onepass(
        self,
        algo: EhsmHashAlgo,
        msg: bytes,
        msg_size: int,
        digest: bytes,
        digest_size: int
    ) -> tuple[int, bytes, int]:
        pass

    def ehsm_hash_update(self, msg: bytes, msg_size: int) -> int:
        pass

    def ehsm_hmac_finish_gen(self, hmac: bytes, hmac_size: int) -> tuple[int, bytes]:
        pass

    def ehsm_hmac_finish_verify(
        self, hmac: bytes, hmac_size: int
    ) -> tuple[int, bool]:
        pass

    def ehsm_hmac_init(
        self, algo: EhsmHashAlgo, key_handle: int, gen_hmac: bool, session: int
    ) -> tuple[int, bytes]:
        pass

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

    def ehsm_hmac_update(self, msg: bytes, msg_size: int) -> int:
        pass

    def ehsm_increase_counter(
        self, counter_id: int, increase_value: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_inject_error(self, values: bytes) -> int:
        pass

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

    def ehsm_install_random_key(
        self,
        key_level: EhsmKeyLevel,
        key_type: EhsmInstallKeyType,
        key_slot_id: int,
        last_key: int,
    ) -> int:
        pass

    def ehsm_km_derive_key(
        self,
        hash_algo: EhsmHashAlgo,
        derive_algo: EhsmDeriveAlgo,
        derive_type: EhsmDeriveType,
        privilege: int,
        key_type: EhsmKeyType,
        key_size: int,
        parent_key_handle: int,
        salt: bytes,
        salt_size: int,
        password: bytes,
        password_size: int,
        iter_times: int,
        key_handle: bytes,
    ) -> tuple[int, bytes]:
        pass

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
    ) -> tuple[int, bytes]:
        pass

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

    def ehsm_km_get_pub_from_priv(
        self,
        key_handle: int,
        dh_params: bytes,
        dh_params_size: int,
        pub_key_size: int,
    ) -> tuple[int, bytes, int, int]:
        pass

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

    def ehsm_km_remove_key(self, key_handle: int) -> int:
        pass

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

    def ehsm_mac_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    def ehsm_mac_finish_gen(
        self,
        msg: bytes,
        msg_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_mac_finish_verify(
        self,
        msg: bytes,
        msg_size: int,
        mac: bytes
    ) -> tuple[int, bool]:
        pass

    def ehsm_read_counter(self, counter_id: int) -> tuple[int, bytes]:
        pass

    def ehsm_read_otp(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    def ehsm_read_reg(self, ehsm_src_addr: int, size: int) -> tuple[int, bytes]:
        pass

    def ehsm_rsa_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int,
    ) -> tuple[int, bytes, int]:
        pass

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

    def ehsm_rsa_sign_init(
        self,
        algo: EhsmHashAlgo,
        key_handle: int,
        gen_sig: bool,
        padding: EhsmRsaPaddingMode,
        session: int
    ) -> tuple[int, int]:
        pass

    def ehsm_rsa_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    def ehsm_rsa_sign_finish_gen(
        self,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_rsa_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int,
        salt_size: int
    ) -> tuple[int, bool]:
        pass

    def ehsm_set_uart_baudrate(self, baud_div: int) -> int:
        pass

    def ehsm_set_utc_time(self, utc_time: int) -> int:
        pass

    def ehsm_sm2_cipher(
        self,
        key_handle: int,
        enc: bool,
        input: bytes,
        input_size: int,
        output_buff_size: int
    ) -> tuple[int, bytes, int]:
        pass

    def ehsm_sm2_sign_onepass_gen(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_sm2_sign_onepass_gen_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_sm2_sign_onepass_verify(
        self,
        key_handle: int,
        msg: bytes,
        msg_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    def ehsm_sm2_sign_onepass_verify_with_digest(
        self,
        key_handle: int,
        digest: bytes,
        digest_size: int,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

    def ehsm_sm2_sign_init(
        self,
        key_handle: int,
        gen_sig: bool,
        session: bytes
    ) -> tuple[int, bytes]:
        pass

    def ehsm_sm2_sign_update(
        self,
        msg: bytes,
        msg_size: int
    ) -> int:
        pass

    def ehsm_sm2_sign_finish_gen(
        self,
        sig_buff_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_sm2_sign_finish_verify(
        self,
        sig: bytes,
        sig_size: int
    ) -> tuple[int, bool]:
        pass

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
        pass

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

    def ehsm_symm_cipher_update(
        self,
        input: bytes,
        input_size: int
    ) -> tuple[int, bytes]:
        pass

    def ehsm_symm_cipher_finish(
        self,
        input: bytes,
        input_size: int,
        output_size: int
    ) -> tuple[int, bytes, int]:
        pass

    def ehsm_upgrade_fw_image(
        self,
        image: bytes,
        image_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        pass

    def ehsm_upgrade_fw_image_init(
        self,
        image_header: bytes,
        header_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        pass

    def ehsm_upgrade_fw_image_update(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        pass

    def ehsm_upgrade_fw_image_finish(
        self,
        body_block: bytes,
        block_size: int,
        storage_addr: int = None  # 可选参数：用于异常参数测试
    ) -> tuple[int, bytes]:
        pass

    def ehsm_verify_image(
        self,
        image: bytes,
        image_size: int,
        check_version: bool,
        boot: bool
    ) -> tuple[int, bytes]:
        pass

    def ehsm_verify_tbbr_img(
        self,
        img_size: int,
        img_data: bytes,
        trusted_fw_nv_ctr_in_otp: int,
        non_trusted_fw_nv_ctr_in_otp: int,
    ) -> tuple[int, bytes, bytes]:
        pass

    def ehsm_write_otp(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass

    def ehsm_write_reg(self, src_data: bytes, ehsm_dest_addr: int, size: int) -> int:
        pass
