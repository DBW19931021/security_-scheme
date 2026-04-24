# NGU800 追踪矩阵（强化版 V1.0）

状态：当前阶段设计追踪文件  
适用范围：NGU800 / NGU800P 安全方案、实现级设计、代码开发、测试验证  
目的：建立从输入资料到代码与测试的可追踪闭环，避免“方案写了但无法落代码”或“代码改了但脱离方案”

---

# 1. 使用说明

本文件不是普通附录，而是**设计与实现的一致性控制文件**。

它用于回答以下问题：

1. 某个设计结论来自哪份资料？
2. 某个约束进入了哪条 baseline 决策？
3. 某个 baseline 结论落到了哪一章详设？
4. 某个详设是否已经收敛到实现级文件？
5. 某个实现级定义最终应该落到哪个代码模块？
6. 某个代码模块是否已经有配套测试？

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
| T-ROOT-001 | eHSM docs / project baseline | C-ROOT-01 | Root of Trust = eHSM | 00_architecture.md / 02_key_cert.md | efuse_key_fw_header_design.md | `sec/ehsm_adapter.*` / `sec/key_service.*` | `test_root_trust.c` | IMPL_READY | Root Secret 仅在 eHSM 使用 |
| T-BOOT-001 | boot docs / subsystem docs | C-BOOT-01 | 所有 FW 执行前必须验签 | 01_boot.md | efuse_key_fw_header_design.md / mailbox_if.md | `sec/verify_flow.*` | `test_verify_before_release.c` | IMPL_READY | 覆盖 SEC1 / SEC2 / 后续微核 |
| T-BOOT-002 | boot docs | C-BOOT-02 | SEC/C908 = 唯一 boot controller | 01_boot.md | mailbox_if.md | `sec/boot_ctrl.*` | `test_release_owner.c` | IMPL_READY | Host / 管理核不得直接 release |
| T-BOOT-003 | baseline / eHSM integration | C-BOOT-03 | BootROM 不做复杂 crypto | 01_boot.md | fw_header / mailbox_if | `bootrom/bootrom_main.*` | `test_bootrom_boundary.c` | CODE_PENDING | 需在代码审查中重点检查 |
| T-CRYPTO-001 | eHSM docs | C-IF-01 | 所有正式安全路径 crypto via eHSM | 06_interface.md / 02_key_cert.md | mailbox_if.md / efuse_key_fw_header_design.md | `drivers/mailbox/*` / `sec/ehsm_adapter.*` | `test_crypto_path_only_ehsm.c` | IMPL_READY | 禁止软件绕过 |
| T-KEY-001 | eHSM docs / key baseline | C-KEY-01 | 私钥不出 eHSM | 02_key_cert.md | efuse_key_fw_header_design.md | `sec/key_service.*` | `test_private_key_non_export.c` | IMPL_READY | Host / 普通核不可见 |
| T-KEY-002 | lifecycle baseline | C-KEY-02 | key usage = lifecycle gated | 04_lifecycle_debug.md / 02_key_cert.md | efuse_key_fw_header_design.md / spdm_report.md | `sec/lifecycle_ctrl.*` / `sec/key_service.*` | `test_key_lifecycle_gate.c` | IMPL_READY | USER / DEBUG 权限不同 |
| T-DEBUG-001 | lifecycle docs | C-DEBUG-01 | USER 禁止未授权 debug | 04_lifecycle_debug.md | mailbox_if.md / spdm_report.md / manufacturing_provisioning.md | `sec/debug_ctrl.*` | `test_user_debug_denied.c` | IMPL_READY | 量产关键约束 |
| T-DEBUG-002 | debug auth docs | C-DEBUG-02 | DEBUG/RMA 必须鉴权 | 04_lifecycle_debug.md | mailbox_if.md | `sec/debug_auth.*` | `test_debug_auth_challenge.c` | IMPL_READY | challenge-response 路径 |
| T-HOST-001 | boot docs / subsystem docs | C-HOST-01 | Host 不可信，只具投递能力 | 00_architecture.md / 06_interface.md / 05_board_security.md | mailbox_if.md | `host_proxy/*` / `sec/host_req_mgr.*` | `test_host_cannot_release.c` | IMPL_READY | Host 不得直接调用 eHSM |
| T-ACCESS-001 | subsystem / firewall docs | C-ACCESS-01 | 安全子系统必须隔离 | 00_architecture.md / 06_interface.md | mailbox_if.md / efuse_key_fw_header_design.md | `sec/access_ctrl.*` | `test_secure_region_denied.c` | IMPL_READY | OTP / Secure SRAM / eHSM 私域不可直访 |
| T-ACCESS-002 | subsystem / firewall docs | C-ACCESS-02 | UserID + Firewall 必须启用 | 00_architecture.md / 06_interface.md | mailbox_if.md | `rtl/firewall_cfg` / `sec/firewall_cfg.*` | `test_userid_firewall_rules.c` | CODE_PENDING | 需与 RTL 配合冻结 |
| T-UPD-001 | update baseline | C-UPDATE-01 | anti-rollback mandatory | 08_failure_recovery.md / 01_boot.md | efuse_key_fw_header_design.md / mailbox_if.md | `sec/update_mgr.*` | `test_rollback_floor.c` | IMPL_READY | counter 先验签后提升 |
| T-UPD-002 | recovery baseline | C-UPDATE-02 | 必须定义恢复机制 | 08_failure_recovery.md | mailbox_if.md / manufacturing_provisioning.md | `sec/recovery_mgr.*` | `test_recovery_path.c` | CODE_PENDING | A/B 是否启用仍可裁决 |
| T-ATT-001 | attestation baseline | C-ATT-01 | 支持 device identity + SPDM report | 03_attestation.md | spdm_report.md | `sec/attest_service.*` | `test_att_report_sign.c` | IMPL_READY | 私钥不离开 eHSM |
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

## 4.2 eHSM 适配层 / 驱动层

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `drivers/mailbox/ehsm_mailbox.*` | 寄存器访问 / doorbell / irq | mailbox_if |
| `sec/ehsm_adapter.*` | 通用 req/resp 包封装 | C-IF-01 |
| `sec/key_service.*` | key derive / key slot / key policy | C-KEY-* |
| `sec/counter_service.*` | rollback counter / version floor | C-UPDATE-01 |

## 4.3 制造 / 工具侧

| 模块名建议 | 职责 | 主要来源 |
|---|---|---|
| `tools/provisioning/otp_writer.*` | OTP/eFuse 写入 | C-MFG-01 |
| `tools/provisioning/lifecycle_mgr.*` | MANU→USER 推进 | C-MFG-01 / C-KEY-02 |
| `tools/provisioning/audit_logger.*` | 审计记录 | C-MFG-01 |

---

# 5. 测试用例建议映射

## 5.1 启动 / 验签类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_verify_before_release.c` | 未验签固件不得执行 | T-BOOT-001 |
| `test_release_owner.c` | 只有 SEC 能 release | T-BOOT-002 |
| `test_bootrom_boundary.c` | BootROM 不得越界承担 crypto 职责 | T-BOOT-003 |
| `test_rollback_floor.c` | 低版本镜像必须被拒绝 | T-UPD-001 |

## 5.2 Host / Interface 类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_host_cannot_release.c` | Host 不得直接放行执行 | T-HOST-001 |
| `test_secure_region_denied.c` | Host / 非安全域不能访问安全域 | T-ACCESS-001 |
| `test_userid_firewall_rules.c` | UserID / Firewall 策略生效 | T-ACCESS-002 |
| `test_crypto_path_only_ehsm.c` | 正式安全路径不允许软件绕过 eHSM | T-CRYPTO-001 |

## 5.3 Lifecycle / Debug / Attestation 类

| Test Case | 验证目标 | 关联 Trace |
|---|---|---|
| `test_user_debug_denied.c` | USER 禁未授权 debug | T-DEBUG-001 |
| `test_debug_auth_challenge.c` | challenge-response 调试鉴权 | T-DEBUG-002 |
| `test_key_lifecycle_gate.c` | key 权限受生命周期控制 | T-KEY-002 |
| `test_att_report_sign.c` | report 关键字段被签名覆盖 | T-ATT-001 |

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
| Counter ID 到 image_type 的最终映射 | 需要与 eFuse / header 一起冻结 | T-UPD-001 |
| Board binding 默认策略 | 量产默认开关未定 | T-ATT-001 / T-MFG-001 |
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
