# NGU800 追踪矩阵（强化版 V1.0）

状态：当前阶段设计追踪文件（已纳入 `SRC-005 管理子系统方案`、`SRC-008 当前收敛安全软件方案 2.0`、`SRC-009 OSR eHSM 软件代码包 4019` 和 `SRC-010 eHSM4.0 ROM Patch 方案` 增量输入）
适用范围：NGU800 / NGU800P 安全方案、完整详设、实现级编辑分片、代码开发、测试验证
目的：建立从输入资料到代码与测试的可追踪闭环，避免“方案写了但无法落代码”或“代码改了但脱离方案”

---

# 1. 使用说明

本文件不是普通附录，而是**设计与实现的一致性控制文件**。

它用于回答以下问题：

1. 某个设计结论来自哪份资料？
2. 某个约束进入了哪条 baseline 决策？
3. 某个 baseline 结论落到了哪一章详设？
4. 某个详设是否已经收敛到实现级文件？
5. 某个实现级定义是否已经进入 `10_full_design.md` 这个代码落地主入口？
6. 某个实现级定义最终应该落到哪个代码模块？
7. 某个代码模块是否已经有配套测试？

---

# 2. 状态定义

| Status | 含义 |
|---|---|
| INPUT_ONLY | 只有输入资料，尚未进入约束 |
| CONSTRAINED | 已进入约束表 |
| BASELINED | 已进入设计基线 |
| DETAILED | 已进入章节级详设 |
| IMPL_READY | 已进入实现级设计 |
| CODE_PENDING | 可进入代码开发，但尚未编码 |
| CODED | 已有代码实现 |
| TEST_PENDING | 代码已实现，但测试未补齐 |
| TESTED | 已有测试覆盖 |
| BLOCKED | 被缺失输入 / 未决冲突阻塞 |

---

# 3. 追踪矩阵

| Trace ID | Source | Constraint ID | Baseline Decision | Detailed Design | Impl Design | Code Module | Test Case | Status | Notes |
|---|---|---|---|---|---|---|---|---|---|
| T-SRC-001 | SRC-008 / CR-0014 | C-SRC-01 | `芯片安全软件方案_2.0.pdf` 是当前收敛方案源；旧 `SRC-001` 降级为历史流程参考 | 10_full_design.md / 01_boot.md | efuse_key_fw_header_design.md / 05_code_rules.md | all FW / driver / tool / test modules | review checklist / `test_doc_source_sync.py` | IMPL_READY | 后续方案、代码规则和导出版如无特殊说明均按 SRC-008 解释 |
| T-SRC-002 | SRC-009 / CR-0018 | C-SRC-02 | OSR eHSM 软件代码包 4019 是 eHSM 已提供安全服务、mailbox ABI、Host API 和工具行为的实现事实源 | 10_full_design.md / 01_boot.md / 06_interface.md / 07_manufacturing_rma.md | ehsm_source_conformance_matrix.md / mailbox_if.md / efuse_key_fw_header_design.md / manufacturing_provisioning.md | `sec/ehsm_adapter.*` / `drivers/mailbox/ehsm_mailbox.*` / `tools/image_packager/*` / `tools/provisioning/*` | OSR source-conformance checklist / generated mailbox ABI tests / golden vectors | BASELINED | `10_full_design.md` 和 `04_impl_design` 字段级适配尚未完成，需后续专项 CR |
| T-SRC-003 | SRC-010 / CR-0018 | C-EHSM-03 | eHSM4.0 ROM Patch 是 OTP + hardware BOOT + CPU/IROM 指令替换机制，不是运行期热补丁入口 | 10_full_design.md / 01_boot.md / 07_manufacturing_rma.md / 09_risks_open_issues.md | efuse_key_fw_header_design.md / manufacturing_provisioning.md / spdm_report.md / ehsm_source_conformance_matrix.md | `tools/provisioning/*` / OTP tool integration / RTL patch config review | patch OTP programming review / patch hit golden test / audit checklist | BASELINED | Patch_en/Patch_addr/Patch_data offset、锁定策略、验收脚本和证明/审计表达仍为 TBD |
| T-SRC-004 | User decision / CR-0019 | C-SRC-03 | Vendor eHSM BootROM/Firmware 和 Wing 工具链变更必须先提醒、说明、记录和验证；默认不静默修改 vendor 交付物 | FSP OpenSpec / code rules / build documentation | vendor change-control principles / 05_code_rules.md | `ehsm_bootrom/**` / `ehsm_firmware/**` / `/opt/wing_tool/**` / build scripts / shell environment | vendor-change review checklist / build verification / `git status` impact check | IMPL_READY | 本规则是开发强约束，不改变 eHSM ABI、Root of Trust 或 BootROM/Firmware 安全设计 |
| T-DOC-001 | CR-0005 / user decision | N/A | `10_full_design.md` 是完整详设与代码落地主入口；`04_impl_design` 是编辑分片，不是独立事实源 | 10_full_design.md | 04_impl_design/*.md synchronized into 10_full_design 第 10 章 | all FW / driver / tool / test modules | `test_doc_source_sync.py` / review checklist | IMPL_READY | 若实现级字段只存在于分片而不在主详设中可见，应视为 sync failure |
| T-ROOT-001 | eHSM docs / project baseline | C-ROOT-01 | Root of Trust = eHSM | 00_architecture.md / 02_key_cert.md | efuse_key_fw_header_design.md | `sec/ehsm_adapter.*` / `sec/key_service.*` | `test_root_trust.c` | IMPL_READY | Root Secret 仅在 eHSM 使用 |
| T-BOOT-001 | boot docs / subsystem docs | C-BOOT-01 | 所有 FW 执行前必须验签 | 01_boot.md | efuse_key_fw_header_design.md / mailbox_if.md | `sec/verify_flow.*` | `test_verify_before_release.c` | IMPL_READY | 覆盖 SEC1 / SEC2 / 后续微核 |
| T-BOOT-002 | boot docs | C-BOOT-02 | SEC/C908 = 唯一 boot controller | 01_boot.md | mailbox_if.md | `sec/boot_ctrl.*` | `test_release_owner.c` | IMPL_READY | Host / 管理核不得直接 release |
| T-BOOT-003 | baseline / eHSM integration | C-BOOT-03 | BootROM 不做复杂 crypto | 01_boot.md | fw_header / mailbox_if | `bootrom/bootrom_main.*` | `test_bootrom_boundary.c` | CODE_PENDING | 需在代码审查中重点检查 |
| T-BOOT-004 | CR-0001 / boot docs / eHSM integration | C-BOOT-04 | SEC1_ENCRYPT_REQUIRED：SEC1 必须签名 + 加密，解密由 eHSM / 安全子系统受控服务完成 | 01_boot.md / 02_key_cert.md | efuse_key_fw_header_design.md / mailbox_if.md / manufacturing_provisioning.md / spdm_report.md | `sec/verify_flow.*` / `sec/key_service.*` / `bootrom/bootrom_main.*` | `test_sec1_encrypt_required.c` / `test_sec1_decrypt_fail_blocks_boot.c` | IMPL_READY | Host 不下发 SEC1；BootROM 不直接实现复杂解密 |
| T-BOOT-005 | CR-0003 / runtime image policy | C-BOOT-05 | SEC2_ENCRYPT_REQUIRED：SEC2 必须 sign + encrypt；PM/RAS/Codec USER/PROD 默认 sign + encrypt，signature-only 仅白名单 | 01_boot.md / 02_key_cert.md | efuse_key_fw_header_design.md / mailbox_if.md / spdm_report.md / manufacturing_provisioning.md | `sec/verify_flow.*` / `sec/key_service.*` / `sec/attest_service.*` | `test_sec2_encrypt_required.c` / `test_runtime_signature_only_whitelist.c` | IMPL_READY | signature-only 白名单和 recovery policy 仍需产品/owner 冻结 |
| T-EHSM-001 | CR-0004 / SRC-006 / SRC-007 | C-BOOT-06 | eHSM native header 是 SEC1/SEC2 密码学 verify/decrypt container；NGU metadata 进入 Code region protected manifest | 01_boot.md / 06_interface.md / 10_full_design.md | efuse_key_fw_header_design.md / ehsm_source_conformance_matrix.md / mailbox_if.md / spdm_report.md | `components/security/src/verify_flow.c` / `components/security/src/attest/*` / `components/security/tools/image_packager/*` | `test_ehsm_header.c` / `test_manifest_gate.c` | TESTED | manifest ABI、eHSM 是否解析 manifest 仍 TBD |
| T-EHSM-004 | SRC-009 / CR-0018 | C-EHSM-02 | OSR 代码必须进入 eHSM source-conformance gate；adapter、verify flow、mailbox driver、工具和测试不得只按 NGU 抽象开发 | 10_full_design.md / 06_interface.md / 07_manufacturing_rma.md | ehsm_source_conformance_matrix.md / mailbox_if.md / manufacturing_provisioning.md | `sec/ehsm_adapter.*` / `sec/verify_flow.*` / `drivers/mailbox/ehsm_mailbox.*` / `tools/provisioning/*` | OSR command/field/error diff tests / adapter integration tests | BASELINED | `bl_verify_image`、`soc_verify`、`fw_upgrade`、OTP/debug/counter/key 服务需逐项对照 |
| T-EHSM-PATCH-001 | SRC-010 / CR-0018 | C-EHSM-03 | ROM patch 配置从 OTP 加载到 patch 模块，CPU 访问 IROM 命中时返回替换指令并屏蔽 IROM 访问 | 10_full_design.md / 07_manufacturing_rma.md / 09_risks_open_issues.md | efuse_key_fw_header_design.md / manufacturing_provisioning.md / spdm_report.md | `tools/provisioning/*` / OTP tool integration / RTL patch config | `test_ehsm_rom_patch_otp_lock.c` / patch audit checklist | BASELINED | 字段级布局、MANU/USER 权限、attestation/audit 是否报告 patch 状态仍需冻结 |
| T-CODE-NAME-001 | CR-0015 / DEC-0020 | N/A | 安全组件采用功能命名空间，不保留芯片前缀兼容层 | component OpenSpec / development principles | rename-security-module design and evidence | `components/security/**` / direct BootROM-FMC-GSP consumers | `components/security/tests/host/check_security_naming.sh` / all host runners / target clean builds | TESTED | third-party upstream namespace和迁移记录中的 old-to-new 名称不属于违规残留 |
| T-EHSM-002 | CR-0004 / SRC-006 / SRC-007 | C-EHSM-01 | physical OTP/control/key/counter 以 eHSM TRM 为准；NGU OTP/key/counter 名称仅为 logical alias | 02_key_cert.md / 07_manufacturing_rma.md / 10_full_design.md | efuse_key_fw_header_design.md / ehsm_source_conformance_matrix.md / manufacturing_provisioning.md | `sec/key_service.*` / `tools/provisioning/*` | `test_no_custom_physical_otp_layout.c` / `test_key_alias_requires_ehsm_mapping.c` | IMPL_READY | exact key ID、OTP/control bit、per-image counter TBD |
| T-EHSM-003 | CR-0004 / SRC-006 / SRC-007 | C-BOOT-07 | SEC1/SEC2 sign+encrypt 必须走 eHSM verify+decrypt output path；NVM only verify 不适用 | 01_boot.md / 06_interface.md | mailbox_if.md / efuse_key_fw_header_design.md / ehsm_source_conformance_matrix.md | `bootrom/bootrom_main.*` / `sec/verify_flow.*` | `test_encrypted_image_rejects_nvm_only_verify.c` / `test_output_buffer_whitelist.c` | IMPL_READY | SEC1 exact Bootloader command path、output buffer 地址仍需冻结 |
| T-FW-PKG-001 | CR-0006 / SRC-008 ch4-4.5 / CR-0004 / CR-0014 / DEC-0017 | C-BOOT-08 | 平台侧固件制作工具与设备侧 verify/decrypt 路径共享 eHSM native header + NGU protected manifest 契约；SEC1 最低认证覆盖范围为完整 Code region，即 manifest + payload + padding/alignment | 01_boot.md / 10_full_design.md | efuse_key_fw_header_design.md / ehsm_source_conformance_matrix.md / mailbox_if.md | `tools/image_packager/*` / `bootrom/bootrom_main.*` / `sec/verify_flow.*` / `sec/ehsm_adapter.*` | `test_image_packager_ehsm_native_layout.c` / `test_manifest_untrusted_before_ehsm_pass.c` / `test_package_golden_vector.c` / `test_sec1_code_region_auth_coverage.c` / `test_sec1_package_tamper_vectors.c` | IMPL_READY | `SRC-001` 仅保留历史流程参考；manifest ABI、tool CLI、exact eHSM key ID/command mapping 和 golden/tamper vector 仍需 owner 冻结 |
| T-CRYPTO-001 | eHSM docs | C-IF-01 | 所有正式安全路径 crypto via eHSM | 06_interface.md / 02_key_cert.md | mailbox_if.md / efuse_key_fw_header_design.md | `drivers/mailbox/*` / `sec/ehsm_adapter.*` | `test_crypto_path_only_ehsm.c` | IMPL_READY | 禁止软件绕过 |
| T-KEY-001 | eHSM docs / key baseline | C-KEY-01 | 私钥不出 eHSM | 02_key_cert.md | efuse_key_fw_header_design.md | `sec/key_service.*` | `test_private_key_non_export.c` | IMPL_READY | Host / 普通核不可见 |
| T-KEY-002 | lifecycle baseline | C-KEY-02 | key usage = lifecycle gated | 04_lifecycle_debug.md / 02_key_cert.md | efuse_key_fw_header_design.md / spdm_report.md | `sec/lifecycle_ctrl.*` / `sec/key_service.*` | `test_key_lifecycle_gate.c` | IMPL_READY | USER / DEBUG 权限不同 |
| T-DEBUG-001 | lifecycle docs | C-DEBUG-01 | USER 禁止未授权 debug | 04_lifecycle_debug.md | mailbox_if.md / spdm_report.md / manufacturing_provisioning.md | `sec/debug_ctrl.*` | `test_user_debug_denied.c` | IMPL_READY | 量产关键约束 |
| T-DEBUG-002 | debug auth docs | C-DEBUG-02 | DEBUG/RMA 必须鉴权 | 04_lifecycle_debug.md | mailbox_if.md | `sec/debug_auth.*` | `test_debug_auth_challenge.c` | IMPL_READY | challenge-response 路径 |
| T-HOST-001 | boot docs / subsystem docs | C-HOST-01 | Host 不可信，只具投递能力 | 00_architecture.md / 06_interface.md / 05_board_security.md | mailbox_if.md | `host_proxy/*` / `sec/host_req_mgr.*` | `test_host_cannot_release.c` | IMPL_READY | Host 不得直接调用 eHSM |
| T-ACCESS-001 | subsystem / firewall docs | C-ACCESS-01 | 安全子系统必须隔离 | 00_architecture.md / 06_interface.md | mailbox_if.md / efuse_key_fw_header_design.md | `sec/access_ctrl.*` | `test_secure_region_denied.c` | IMPL_READY | OTP / Secure SRAM / eHSM 私域不可直访 |
| T-ACCESS-002 | subsystem / firewall docs | C-ACCESS-02 | UserID + Firewall 必须启用 | 00_architecture.md / 06_interface.md | mailbox_if.md | `rtl/firewall_cfg` / `sec/firewall_cfg.*` | `test_userid_firewall_rules.c` | CODE_PENDING | 需与 RTL 配合冻结 |
| T-BOARD-001 | `SRC-005 管理子系统方案` | C-BOARD-01 | 管理子系统总体架构和流程可遵循，安全边界由安全方案裁决 | 05_board_security.md / 10_full_design.md | mailbox_if.md / spdm_report.md / manufacturing_provisioning.md | `sec/board_sec_policy.*` / `sec/oob_req_mgr.*` | `test_oob_cannot_bypass_sec.c` | CODE_PENDING | 系统流程采用，安全细节二次裁决 |
| T-BOARD-002 | `SRC-005 管理子系统方案` | C-BOARD-02 | 带外管理通道不得成为安全策略绕过路径 | 05_board_security.md / 06_interface.md | mailbox_if.md | `sec/oob_req_mgr.*` / `host_proxy/oob_proxy.*` | `test_oob_lifecycle_gate.c` | CODE_PENDING | SMBus/I2C/I3C/Sideband 只能受控转发 |
| T-BOARD-003 | `SRC-005 管理子系统方案` | C-BOARD-03 | JTAG 必须受 lifecycle、debug auth、scope bitmap、timeout、audit 和板级 MUX 联合控制 | 05_board_security.md / 04_lifecycle_debug.md / 06_interface.md | mailbox_if.md / manufacturing_provisioning.md | `sec/debug_auth.*` / `sec/jtag_scope_ctrl.*` / `rtl/jtag_mux_ctrl` | `test_user_jtag_denied.c` / `test_jtag_scope_auth.c` | CODE_PENDING | 策略已按 CR-0003 收敛；bit-level mapping 与 CPLD/MUX 寄存器归属仍 BLOCKED |
| T-BOARD-004 | `SRC-005 管理子系统方案` | C-BOARD-04 | 管理子系统 DMA、Host DMA、OOB DMA 对安全资源默认拒绝，只能访问白名单 staging/data buffer | 05_board_security.md / 06_interface.md | mailbox_if.md / spdm_report.md | `sec/firewall_cfg.*` / `sec/power_reset_sec_state.*` | `test_mgmt_dma_firewall.c` / `test_power_reset_audit.c` | CODE_PENDING | 默认拒绝已收敛；DMA region、UserID、PG/FAULT/PowerBrake 状态策略仍需冻结 |
| T-BOARD-005 | CR-0003 / board binding policy | C-ATT-02 | board binding 默认进入 attestation，不默认阻断 SEC1；是否参与 SEC2/runtime release decision 待冻结 | 03_attestation.md / 05_board_security.md | spdm_report.md / manufacturing_provisioning.md | `sec/attest_service.*` / `sec/board_sec_policy.*` | `test_board_bind_reported.c` | CODE_PENDING | release decision 仍为 TBD |
| T-BOARD-006 | CR-0003 / OOB provisioning proxy | C-BOARD-02 / C-MFG-01 | OOB/BMC 可作为 provisioning transport proxy，但不得成为 trust anchor 或接触明文根材料 | 05_board_security.md / 06_interface.md / 07_manufacturing_rma.md | mailbox_if.md / manufacturing_provisioning.md | `sec/oob_req_mgr.*` / `host_proxy/oob_proxy.*` | `test_oob_proxy_not_trust_anchor.c` | CODE_PENDING | 命令格式、认证、审计、失败回滚仍需冻结 |
| T-UPD-001 | update baseline | C-UPDATE-01 | anti-rollback mandatory | 08_failure_recovery.md / 01_boot.md | efuse_key_fw_header_design.md / mailbox_if.md | `sec/update_mgr.*` | `test_rollback_floor.c` | IMPL_READY | counter 先验签后提升 |
| T-UPD-002 | SRC-008 / CR-0013 / OOB MCU FMC reflash recovery baseline | C-UPDATE-02 | FMC 防变砖采用单 FMC 主区域 + OOB MCU 受控重刷；BootROM/eHSM 下次启动必须重新 verify/decrypt/rollback/revoke；key slot rotation 不再绑定 FMC 本地备份分区 | 01_boot.md / 08_failure_recovery.md / 10_full_design.md | efuse_key_fw_header_design.md / mailbox_if.md / manufacturing_provisioning.md | `bootrom/fmc_recovery_status.*` / `sec/key_rotation_mgr.*` / `board/oob_fmc_reflash.*` | `test_oob_fmc_reflash_recovery.c` / `test_key_rotation_oob_recoverability.c` | CODE_PENDING | OOB secure boot、QSPI ownership、NOR 写保护、恢复授权 capsule、状态/审计记录、exact eHSM key ID 和 counter/revoke 表达仍需冻结 |
| T-ATT-001 | attestation baseline | C-ATT-01 / C-ATT-02 | 支持 device identity + SPDM report；report 必须覆盖 measurement、lifecycle、debug、secure_boot、rollback | 03_attestation.md | spdm_report.md | `sec/attest_service.*` | `test_att_report_sign.c` / `test_att_report_state_fields.c` | IMPL_READY | 私钥不离开 eHSM；image policy/board/event 字段仍需编码冻结 |
| T-MFG-001 | manufacturing baseline | C-MFG-01 | 必须定义 key 注入 / 锁定 / 审计 | 07_manufacturing_rma.md | manufacturing_provisioning.md / efuse_key_fw_header_design.md | `tools/provisioning/*` | `test_provision_lock.c` | IMPL_READY | MANU → USER 动作需冻结 |

---

# 4. 代码模块建议映射

## 4.1 SEC / 安全控制面

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `sec/boot_ctrl.*` | 启动编排 / release 状态机 | C-BOOT-02 |
| `sec/verify_flow.*` | 镜像验证调用流程 | C-BOOT-01 / C-UPDATE-01 |
| `sec/host_req_mgr.*` | Host 请求收敛与白名单检查 | C-HOST-01 / C-ACCESS-01 |
| `sec/lifecycle_ctrl.*` | 生命周期状态与 gating | C-KEY-02 / C-DEBUG-* |
| `sec/debug_auth.*` | challenge / auth / scope 控制 | C-DEBUG-02 |
| `sec/attest_service.*` | 证明请求封装与结果转交 | C-ATT-01 |
| `sec/update_mgr.*` | 升级 / counter / rollback 路径 | C-UPDATE-* |
| `sec/board_sec_policy.*` | 板级安全策略收敛，管理子系统安全裁决 | C-BOARD-* |
| `sec/oob_req_mgr.*` | BMC/OOB/Sideband 请求白名单、生命周期检查和转发 | C-BOARD-01 / C-BOARD-02 |
| `sec/jtag_scope_ctrl.*` | JTAG scope bitmap、授权结果和关闭策略 | C-BOARD-03 / C-DEBUG-02 |
| `sec/power_reset_sec_state.*` | 电源/复位/PowerBrake 安全状态和审计 | C-BOARD-04 |

## 4.2 eHSM 适配层 / 驱动层

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `drivers/mailbox/ehsm_mailbox.*` | 寄存器访问 / doorbell / irq | mailbox_if |
| `sec/ehsm_adapter.*` | 通用 req/resp 包封装 | C-IF-01 |
| `sec/key_service.*` | key derive / eHSM key reference / key policy | C-KEY-* |
| `sec/counter_service.*` | rollback counter / version floor | C-UPDATE-01 |

## 4.3 制造 / 工具侧

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `tools/provisioning/otp_writer.*` | OTP/eFuse 写入 | C-MFG-01 |
| `tools/provisioning/lifecycle_mgr.*` | MANU→USER 推进 | C-MFG-01 / C-KEY-02 |
| `tools/provisioning/audit_logger.*` | 审计记录 | C-MFG-01 |
| `tools/image_packager/*` | eHSM native package 生成、NGU manifest dump、source-conformance report、golden vector 输出 | C-BOOT-08 / C-BOOT-06 |

---

## 4.4 RTL / Firewall / Board Control

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `rtl/firewall_cfg` | UserID / firewall / DMA region 隔离 | C-ACCESS-02 / C-BOARD-04 |
| `rtl/jtag_mux_ctrl` | JTAG MUX / CPLD 受控打开和关闭 | C-BOARD-03 |
| `rtl/oob_bridge_ctrl` | OOB / Sideband 桥接权限控制 | C-BOARD-02 |

---

# 5. 测试用例建议映射

## 5.1 启动 / 验签类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_verify_before_release.c` | 未验签固件不得执行 | T-BOOT-001 |
| `test_release_owner.c` | 只有 SEC 能 release | T-BOOT-002 |
| `test_bootrom_boundary.c` | BootROM 不得越界承担 crypto 职责 | T-BOOT-003 |
| `test_rollback_floor.c` | 低版本镜像必须被拒绝 | T-UPD-001 |
| `test_sec1_encrypt_required.c` | SEC1 镜像缺少加密或 wrapped CEK 时必须被拒绝 | T-BOOT-004 |
| `test_sec1_decrypt_fail_blocks_boot.c` | SEC1 解密失败必须阻止启动且不能降级执行 | T-BOOT-004 |
| `test_sec2_encrypt_required.c` | SEC2 镜像缺少加密或 wrapped CEK 时必须被拒绝 | T-BOOT-005 |
| `test_runtime_signature_only_whitelist.c` | signature-only runtime image 必须命中产品白名单且可被证明路径观测 | T-BOOT-005 |
| `test_image_packager_ehsm_native_layout.c` | 工具产物必须采用 eHSM native header，NGU manifest 位于受保护 Code region | T-FW-PKG-001 |
| `test_manifest_untrusted_before_ehsm_pass.c` | eHSM PASS 前 BootROM/SEC 不得信任 manifest 中的 load/entry/policy | T-FW-PKG-001 |
| `test_package_golden_vector.c` | image packager 输出与 BootROM/SEC/eHSM adapter golden vector 一致 | T-FW-PKG-001 |
| `test_sec1_code_region_auth_coverage.c` | SEC1 package 的认证覆盖范围必须至少包含完整 Code region：manifest、payload 和计入 `Code_Size` 的 padding/alignment | T-FW-PKG-001 |
| `test_sec1_package_tamper_vectors.c` | 篡改 manifest `image_type`、`entry_addr`、`version_counter`、payload 字节、`Code_Size` 或截断 Code region 后不得 release | T-FW-PKG-001 |

## 5.2 Host / Interface 类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_host_cannot_release.c` | Host 不得直接放行执行 | T-HOST-001 |
| `test_secure_region_denied.c` | Host / 非安全域不能访问安全域 | T-ACCESS-001 |
| `test_userid_firewall_rules.c` | UserID / Firewall 策略生效 | T-ACCESS-002 |
| `test_crypto_path_only_ehsm.c` | 正式安全路径不允许软件绕过 eHSM | T-CRYPTO-001 |
| `test_oob_cannot_bypass_sec.c` | BMC/OOB 不能绕过 SEC 直接进入安全服务 | T-BOARD-001 |
| `test_oob_lifecycle_gate.c` | OOB 高权限请求受 lifecycle gating | T-BOARD-002 |
| `test_mgmt_dma_firewall.c` | 管理子系统 DMA 不能访问安全区 | T-BOARD-004 |
| `test_oob_proxy_not_trust_anchor.c` | OOB/BMC provisioning proxy 不能成为信任根或接触明文根材料 | T-BOARD-006 |

## 5.3 Lifecycle / Debug / Attestation 类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_user_debug_denied.c` | USER 禁未授权 debug | T-DEBUG-001 |
| `test_debug_auth_challenge.c` | challenge-response 调试鉴权 | T-DEBUG-002 |
| `test_user_jtag_denied.c` | USER 态 JTAG 默认关闭 | T-BOARD-003 |
| `test_jtag_scope_auth.c` | JTAG scope 必须来自授权结果 | T-BOARD-003 |
| `test_key_lifecycle_gate.c` | key 权限受生命周期控制 | T-KEY-002 |
| `test_att_report_sign.c` | report 关键字段被签名覆盖 | T-ATT-001 |
| `test_att_report_state_fields.c` | lifecycle/debug/secure_boot/rollback/image policy 状态进入 report | T-ATT-001 |
| `test_board_bind_reported.c` | board binding 默认进入证明数据，不阻断 SEC1 | T-BOARD-005 |

## 5.4 Manufacturing 类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_provision_lock.c` | key 注入后锁定不可回写 | T-MFG-001 |
| `test_manu_to_user_freeze.c` | MANU→USER 冻结动作完整 | T-MFG-001 |

---

# 6. 当前阻塞项（BLOCKED 候选）

以下内容后续可能进入 `BLOCKED` 状态，需在实现前进一步冻结：

| Item | Why Blocked | Affected Trace |
|---|---|---|
| Debug port 129bit 最终位图 | 尚未冻结端口位图映射 | T-DEBUG-002 |
| JTAG scope bitmap 与 MUX/CPLD 控制权 | 策略已冻结，bit-level mapping 与寄存器归属未冻结 | T-BOARD-003 |
| 管理子系统 DMA region / UserID | 默认拒绝已冻结，具体可访问 buffer、firewall region 和 master 标识未冻结 | T-BOARD-004 |
| PowerBrake / PG / FAULT / reset 安全状态 | 尚未冻结哪些事件进入证明或审计 | T-BOARD-004 |
| Counter ID 到 image_type 的最终映射 | CR-0004 后旧 `*_MIN_VER` 仅为 logical rollback domain；per-image physical counter 仍需 eHSM customization 或 owner-confirmed counter service | T-UPD-001 / T-EHSM-002 |
| manifest ABI 与 eHSM manifest parser | NGU metadata 已进入 protected manifest，但 bit-level ABI 和解析主体未冻结 | T-EHSM-001 |
| exact eHSM key ID / OTP control bit mapping | CR-0004 已要求 source-conformance，但 exact mapping 需 eHSM/RTL owner 冻结 | T-EHSM-002 |
| SEC1 early boot exact eHSM command path | `VERIFY_SEC1` 方向上应映射 Bootloader `bl_verify_image` 或等价 ROM path，需 BootROM/eHSM 集成确认 | T-EHSM-003 |
| Image packager CLI / golden vector | CR-0006 已冻结制作/验证流程方向，但工具参数、manifest ABI 和 golden vector 格式仍需 tooling / SEC FW owner 冻结 | T-FW-PKG-001 |
| OSR eHSM 代码与 full/impl design 差异清单 | `SRC-009` 已成为实现事实源，但 `10_full_design.md` / `04_impl_design` 尚未逐项同步 command、req/rsp、错误码、key ID、OTP/control bit、tool CLI 和 golden vector | T-SRC-002 / T-EHSM-004 / T-FW-PKG-001 |
| eHSM4.0 ROM Patch 字段级集成 | `SRC-010` 已冻结机制方向，但 Patch_en/Patch_addr/Patch_data offset、enable 编码、烧录权限、USER 锁定、验收脚本和证明/审计表达未冻结 | T-SRC-003 / T-EHSM-PATCH-001 / T-MFG-001 |
| Runtime signature-only 白名单 | SEC2 已强制加密，PM/RAS/Codec 默认加密；非敏感 runtime signature-only 白名单仍需产品策略 | T-BOOT-005 / T-KEY-001 |
| Board binding release decision | 默认进入 attestation、不阻断 SEC1；是否参与 SEC2/runtime release decision 未定 | T-BOARD-005 / T-ATT-001 / T-MFG-001 |
| OOB MCU FMC 重刷与 key slot rotation | OOB secure boot、QSPI ownership、NOR FMC 主区域写保护、恢复授权 capsule、状态/审计记录、掉电保护、normal/update key alias 到 eHSM exact key ID / purpose / counter / revoke 映射未冻结 | T-UPD-002 / T-EHSM-002 |
| 共享内存最终落点 | IRAM / DDR / firewall share memory 未最终裁决 | T-HOST-001 / T-IF-001 |

---

# 7. 增量更新规则

当新增资料或方案口径变更时，必须按以下顺序更新：

```text
inputs_manifest
→ 01_constraints.md
→ 02_baseline.md
→ 03_detailed_design/
→ 04_impl_design/
→ 05_code_rules.md
→ 06_traceability.md
```

若某项变更影响实现级接口或结构体，则不得只改章节文档而不改本追踪矩阵。

---

# 8. 当前阶段结论

当前追踪矩阵已经把以下链条建立起来：

```text
Source
→ Constraint
→ Baseline Decision
→ Detailed Design Chapter
→ Impl Design File
→ Recommended Code Module
→ Suggested Test Case
```

这意味着你现在已经不是单纯“写方案”，而是已经进入：

> **方案 → 实现 → 代码 → 测试 的工程闭环阶段**

后续每次新增：
- `mailbox_if.md`
- `spdm_report.md`
- `fw_header.md`
- `manufacturing_provisioning.md`

都应同步刷新本文件。
