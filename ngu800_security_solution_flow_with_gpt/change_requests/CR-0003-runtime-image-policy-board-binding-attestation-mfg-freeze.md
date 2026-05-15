# CR-0003: Runtime Image 策略、Board Binding、调试/DMA/OOB 边界与 V2.4 冻结准备

Status: applied  
Base Commit: `df815e2`  
Owner: Security Owner / Architecture Review  
Codex Role: apply only after owner accepted  
Created: 2026-05-07  
Applied: 2026-05-07 by Codex after user execution request  
Source Context Pack: `.context/design_context_pack.md`  
Target Document: `security_workflow/03_detailed_design/10_full_design.md`  
Change Type: architecture review closure + detailed design freeze preparation  
Risk Level: high  

---

## 1. 背景

当前 `security_workflow/03_detailed_design/10_full_design.md` 版本为 V2.4，状态为：

```text
CR-0001 applied，待 GPT / 人工复核
```

CR-0001 已经收敛了以下主干设计：

1. SEC1 必须 sign + encrypt。
2. SEC1 来源仍为 NOR Flash / 本地 Flash。
3. BootROM 只做最小加载与编排，不直接实现复杂密码学验签/解密。
4. eHSM 是 Root of Trust / First Cryptographic Verifier。
5. Host 不可信，不下发 SEC1。
6. Board / OOB / BMC 不进入 Root of Trust，信任级别不高于 Host。
7. Root / Key / Cert 详细章节后置。
8. 板级安全章节已纳入 full design。

但是 V2.4 仍存在影响详细设计冻结的问题：

1. SEC2 / PM / RAS / Codec / runtime image 加密策略尚未完全冻结。
2. signature-only runtime image 的准入条件不明确。
3. recovery image 的 image_type、signer、trust anchor、rollback counter、decrypt policy 未冻结。
4. board binding 是否参与 firmware verify / decrypt / release decision 未冻结。
5. JTAG scope bitmap、eHSM debug bitmap、板级 MUX 控制权和 bit-level mapping 未冻结。
6. DMA / firewall / UserID / 地址白名单未冻结。
7. OOB / BMC 是否允许作为 provisioning proxy 及其认证、审计、失败回滚边界未冻结。
8. Attestation report 中 image protection policy、decrypt_applied、board_bind_result、debug/RMA state、PowerBrake/PG/FAULT/reset event 的编码策略未冻结。
9. Manufacturing / RMA 中 Root injection mode、OTP/eFuse readback 验收、RMA re-acceptance 流程未冻结。

因此，本 CR 用于关闭 V2.4 进入 reviewed baseline 前的关键开放项，并明确哪些问题可以继续保留为 `[ASSUMED]` / `[TBD]` 到实现设计或产品策略阶段。

---

## 2. V2.4 当前状态裁决

### D-0006: V2.4 保持 pending review

```text
[CONFIRMED] V2.4 当前保持 pending review，不升级为完整 reviewed baseline。
```

原因：

1. V2.4 已经完成 CR-0001 后的主干方案整合。
2. SEC1 sign+encrypt、eHSM RoT、BootROM minimal、Host untrusted、Board/OOB not RoT 等主干口径可视为局部 reviewed。
3. runtime image policy、board binding、JTAG scope/MUX、DMA/firewall/UserID、OOB proxy、attestation report、manufacturing/RMA 仍未完全冻结。
4. 若现在直接升级为 reviewed baseline，容易把 `[ASSUMED]` / `[TBD]` 内容误读成 `[CONFIRMED]`。

建议在 `10_full_design.md` 文档头部写成：

```text
Version: V2.4
Status: pending review
CR-0001: applied
Partial reviewed areas:
- SEC1 sign + encrypt
- eHSM as Root of Trust / First Cryptographic Verifier
- BootROM minimal orchestration
- Host untrusted
- Board/OOB not Root of Trust
```

---

## 3. 本 CR 设计裁决

### D-0007: SEC2 强制 sign + encrypt

```text
[CONFIRMED] SEC2 在正式安全启动路径中必须 sign + encrypt。
```

要求：

1. 所有文档中不得再把 SEC2 加密描述为“可选”默认策略。
2. SEC2 verify path 必须包含 signature verify、rollback check、revoke check、decrypt / unwrap、measurement 和 controlled release。
3. SEC2 decrypt failure 必须阻断安全控制面启动。

### D-0008: PM / RAS / Codec 默认 sign + encrypt，但保留产品策略例外

```text
[ASSUMED] PM / RAS / Codec 等关键 runtime image 在 USER/PROD 产品形态中默认 sign + encrypt。
[TBD] 是否允许某些非敏感 runtime image signature-only，由产品安全策略和 image_type 白名单冻结。
```

要求：

1. 默认推荐策略是 sign + encrypt。
2. signature-only 只能作为显式白名单例外，不能作为默认路径。
3. signature-only 准入条件至少绑定 image_type、lifecycle、product SKU、debug state、release policy、rollback policy、是否包含敏感逻辑/数据。
4. 若进入 signature-only 路径，该状态必须能被 measurement / attestation 或安全状态表体现。

### D-0009: Recovery image 独立策略

```text
[TBD] recovery image 的 image_type、signer、trust anchor、rollback counter、decrypt policy 需要在详细设计冻结前关闭。
```

建议方向：

1. recovery image 使用独立 `image_type`。
2. recovery image 可使用独立 signer / recovery trust anchor，但不得绕过 eHSM 验证。
3. recovery image 必须具备 rollback 保护。
4. recovery image 是否加密由恢复场景决定；若包含安全逻辑或敏感修复能力，应 sign + encrypt。
5. recovery boot 状态必须进入 measurement / attestation / audit。

### D-0010: Board binding 默认进入 attestation，不默认阻断 SEC1

```text
[ASSUMED] V2.4 阶段 board binding 默认进入 attestation，不默认参与 SEC1 verify/decrypt/release 决策。
[TBD] board binding 是否参与 SEC2/runtime image release decision，由后续 board binding 实现级设计冻结。
```

要求：

1. SEC1 启动不应依赖 board binding。
2. board identity / board binding result 应进入 attestation report 或扩展证明数据。
3. 若后续裁决 board binding 参与 firmware release decision，必须定义 binding material、check owner、failure policy、RMA / board replacement、dual-die behavior、manufacturing provisioning flow。

### D-0011: JTAG scope / MUX 安全策略

```text
[CONFIRMED] USER/PROD 下 JTAG 默认关闭。
[CONFIRMED] JTAG 打开必须经过 lifecycle + debug auth + scope bitmap + session timeout + audit。
[ASSUMED] 板级 MUX 控制权应由 SEC/eHSM 授权信号约束，OOB/BMC 不得单独打开。
[TBD] JTAG scope bitmap bit-level mapping、eHSM debug bitmap 与板级 MUX 寄存器归属。
```

要求：

1. JTAG 不得存在 USER/PROD 常开路径。
2. OOB / BMC / 板级 MCU 不得单独绕过 SEC/eHSM 打开 JTAG MUX。
3. Debug auth 结果必须包含 scope、timeout、session id、audit。
4. JTAG open/close 状态应进入 debug state / audit，并建议进入 attestation report 或 event log。
5. `JTAG_FORCE_DISABLE` / equivalent policy 在 USER freeze 中必须生效。

### D-0012: DMA / firewall / UserID 默认拒绝

```text
[CONFIRMED] 管理子系统 DMA / Host DMA / OOB DMA 对安全资源默认拒绝。
[CONFIRMED] DMA 只能访问显式白名单 staging/data buffer。
[TBD] 具体 UserID、firewall region、地址范围、错误隐藏策略、审计字段由 RTL/实现设计冻结。
```

禁止访问范围至少包括：

1. eHSM internal memory；
2. OTP / eFuse；
3. Secure SRAM；
4. SEC1 / SEC2 执行区；
5. recovery image 区；
6. cert / policy / metadata 安全区；
7. measurement_table 安全写区域；
8. debug / lifecycle / rollback 控制寄存器。

要求：

1. firewall 配置权归 SEC/eHSM 或安全控制面。
2. 非安全 master 不得修改安全资源访问策略。
3. UserID 不能被截断、丢弃或由非安全 master 伪造。
4. violation 应产生错误码 / audit / security event。

### D-0013: OOB / BMC provisioning proxy

```text
[ASSUMED] 允许 OOB / BMC 作为 provisioning transport proxy。
[CONFIRMED] OOB / BMC 不得成为 trust anchor，不得接触 root secret、device private key、FW_KEK 明文。
[TBD] OOB proxy 命令格式、认证方式、审计记录、失败回滚策略。
```

要求：

1. OOB / BMC 只能作为传输代理或管理入口。
2. 所有安全裁决必须由 SEC/eHSM 完成。
3. 所有敏感材料必须以加密/封装形式传输。
4. OOB proxy 必须具备 request authentication、anti-replay、audit log、failure rollback、rate limit / lockout、lifecycle gating。

### D-0014: Attestation report 字段策略

```text
[CONFIRMED] Attestation report 必须包含 measurement、lifecycle、debug state、secure boot state、rollback state。
[ASSUMED] Attestation report 增加 image protection policy / decrypt_applied / image_type policy 字段。
[ASSUMED] 若 board binding 参与证明或验证，则 board_bind_result 进入 report。
[TBD] PowerBrake / PG / FAULT / reset event 是否进入主 report，还是进入扩展 event log。
```

要求：

1. report 不得只返回签名，必须返回签名覆盖的状态。
2. report signature 覆盖范围必须包含 nonce / challenge、measurement、lifecycle、debug state、secure boot state、rollback state、report version、image protection policy if present。
3. image protection policy 不能替代 measurement，只能作为安全策略状态。
4. RMA/debug state 必须在 report 中可见，防止把 RMA/debug 态伪装成 USER/PROD 态。

### D-0015: Manufacturing / RMA freeze checklist

```text
[CONFIRMED] USER freeze 必须锁定 secure boot、debug、anti-rollback、SEC1/SEC2 decrypt key / FW_KEK、test trust cleanup。
[CONFIRMED] RMA 不允许 long-open debug，不允许绕过 challenge/auth，不允许长期保留 SEC1/SEC2 decrypt bypass。
[TBD] Root injection mode、OTP/eFuse readback 验收方式、RMA re-acceptance 流程。
```

USER freeze 最少包含：

1. `SECURE_BOOT_EN`；
2. `DEBUG_AUTH_EN`；
3. `JTAG_FORCE_DISABLE`；
4. `ANTI_ROLLBACK_EN`；
5. `FW_ENCRYPT_EN` at least SEC1 + SEC2；
6. FW_KEK / image protect key lock；
7. signer anchor lock；
8. rollback counter init / lock policy；
9. test key / test signer / test debug cleanup；
10. lifecycle transition to USER；
11. provisioning audit record。

RMA 最少要求：

1. RMA 必须认证；
2. debug scope 必须受限；
3. session 必须过期；
4. RMA 操作必须审计；
5. RMA 后恢复 USER/PROD 需重新验收；
6. 不允许遗留 debug bypass / test trust / decrypt bypass。

---

## 4. 必须在详细设计冻结前关闭的问题

| ID | 问题 | 当前建议 |
|---|---|---|
| FZ-001 | SEC2 是否强制加密 | 本 CR 建议确认 SEC2 sign+encrypt |
| FZ-002 | PM/RAS/Codec 是否默认加密 | 默认 sign+encrypt，例外走产品白名单 |
| FZ-003 | signature-only runtime image 准入条件 | 需定义 image_type/lifecycle/SKU/debug/release policy |
| FZ-004 | recovery image 策略 | 必须定义 image_type/signer/anchor/counter/decrypt |
| FZ-005 | board binding 是否参与 firmware decision | 本阶段默认只进 attestation，后续冻结 release decision |
| FZ-006 | JTAG scope/MUX 控制策略 | 策略需冻结，bit mapping 可后续 |
| FZ-007 | DMA/firewall/UserID 默认策略 | 默认拒绝 + 白名单 staging buffer |
| FZ-008 | OOB proxy 边界 | 允许 proxy，不允许成为 trust anchor |
| FZ-009 | attestation report policy 字段 | 需定义字段或明确延期 |
| FZ-010 | USER freeze checklist | 需固化 SEC1/SEC2 key lock/debug disable/test cleanup |
| FZ-011 | RMA re-acceptance | 需定义返厂后恢复 USER/PROD 的验收策略 |

---

## 5. 可以保留到实现设计或产品策略阶段的问题

| 问题 | 建议状态 | 阶段 |
|---|---|---|
| 非敏感 runtime image signature-only 白名单 | `[TBD]` | 产品安全策略 |
| X.509 full chain 是否默认内嵌 report | `[TBD]` | SPDM / 证书基础设施 |
| 国密 / 国际算法默认 SKU | `[TBD]` | 产品策略 |
| JTAG scope bit-level mapping | `[TBD]` | 实现设计 / eHSM/板级接口 |
| firewall region 具体地址 | `[TBD]` | RTL/实现设计 |
| UserID 最终分配表 | `[TBD]` | RTL/SoC 集成 |
| PowerBrake/PG/FAULT/reset 是否进入主 report | `[ASSUMED]` | Attestation / event log 设计 |
| dual-die report 汇总方式 | `[TBD]` | Board/die binding 专项设计 |
| OTP/eFuse readback 验收方式 | `[TBD]` | Manufacturing |
| RMA re-acceptance 细节 | `[TBD]` | RMA 流程设计 |

---

## 6. 影响文件与 Codex 修改清单

### 6.1 项目记录文件

```text
00_project/decision_log.md
00_project/open_questions.md
00_project/changelog.md
```

### 6.2 约束与基线

```text
security_workflow/01_constraints.md
security_workflow/02_baseline.md
```

### 6.3 章节级详设

```text
security_workflow/03_detailed_design/01_boot.md
security_workflow/03_detailed_design/03_attestation.md
security_workflow/03_detailed_design/04_lifecycle_debug.md
security_workflow/03_detailed_design/05_board_security.md
security_workflow/03_detailed_design/06_interface.md
security_workflow/03_detailed_design/07_manufacturing_rma.md
security_workflow/03_detailed_design/10_full_design.md
```

### 6.4 实现级设计

```text
security_workflow/04_impl_design/efuse_key_fw_header_design.md
security_workflow/04_impl_design/mailbox_if.md
security_workflow/04_impl_design/spdm_report.md
security_workflow/04_impl_design/manufacturing_provisioning.md
```

### 6.5 规则与追踪

```text
security_workflow/05_code_rules.md
security_workflow/06_traceability.md
05_traceability/design_impact_matrix.md
```

---

## 7. Codex 执行要求

Codex 必须：

1. 保持 `10_full_design.md` 状态为 `pending review`，不得升级为 reviewed baseline。
2. 将 SEC2 更新为 sign+encrypt mandatory。
3. 将 PM/RAS/Codec/runtime image 更新为 USER/PROD 默认 sign+encrypt，但 signature-only 白名单仍为 TBD/ASSUMED。
4. 将 board binding 默认写为进入 attestation，不默认阻断 SEC1；是否参与 SEC2/runtime release decision 保持 TBD。
5. 将 USER/PROD JTAG 默认关闭、JTAG open 必须 lifecycle + auth + scope + timeout + audit 的策略同步到 lifecycle_debug、board_security、interface、code rules。
6. 将 DMA/firewall/UserID 默认拒绝和白名单 staging buffer 原则同步到 board_security、interface、impl design、code rules。
7. 将 OOB/BMC provisioning proxy 写成 ASSUMED transport proxy，不得写成 trust anchor。
8. 将 attestation report 中 lifecycle/debug/secure_boot/rollback 写成 CONFIRMED，将 image protection policy/decrypt_applied/board_bind_result 写成 ASSUMED/TBD。
9. 将 manufacturing USER freeze checklist 更新为包含 SEC1/SEC2 decrypt key/FW_KEK lock、debug disable、anti-rollback、test trust cleanup。
10. 更新 decision_log.md、open_questions.md、changelog.md、design_impact_matrix.md、05_code_rules.md、06_traceability.md。
11. 不要把未冻结的 bit-level mapping、DMA address table、report extension、RMA re-acceptance 写成 CONFIRMED。
12. 修改完成后输出 git diff --stat、git diff、修改摘要、未完成项和冲突项。

---

## 8. 直接给 Codex 的 Prompt

```text
请读取并执行 change_requests/CR-0003-runtime-image-policy-board-binding-attestation-mfg-freeze.md。

你不是安全架构 owner，不得自行做架构冻结裁决。你只负责按 CR 中已经明确的 [CONFIRMED] / [ASSUMED] / [TBD] 状态修改仓库。

执行要求：
1. 保持 10_full_design.md 状态为 pending review，不得升级为 reviewed baseline。
2. 将 SEC2 更新为 sign+encrypt mandatory。
3. 将 PM/RAS/Codec/runtime image 更新为 USER/PROD 默认 sign+encrypt，但 signature-only 白名单仍为 TBD/ASSUMED。
4. 将 board binding 默认写为进入 attestation，不默认阻断 SEC1；是否参与 SEC2/runtime release decision 保持 TBD。
5. 将 USER/PROD JTAG 默认关闭、JTAG open 必须 lifecycle + auth + scope + timeout + audit 的策略同步到 lifecycle_debug、board_security、interface、code rules。
6. 将 DMA/firewall/UserID 默认拒绝和白名单 staging buffer 原则同步到 board_security、interface、impl design、code rules。
7. 将 OOB/BMC provisioning proxy 写成 ASSUMED transport proxy，不得写成 trust anchor。
8. 将 attestation report 中 lifecycle/debug/secure_boot/rollback 写成 CONFIRMED，将 image protection policy/decrypt_applied/board_bind_result 写成 ASSUMED/TBD。
9. 将 manufacturing USER freeze checklist 更新为包含 SEC1/SEC2 decrypt key/FW_KEK lock、debug disable、anti-rollback、test trust cleanup。
10. 更新 decision_log.md、open_questions.md、changelog.md、design_impact_matrix.md、05_code_rules.md、06_traceability.md。
11. 不要把未冻结的 bit-level mapping、DMA address table、report extension、RMA re-acceptance 写成 CONFIRMED。
12. 修改完成后输出 git diff --stat、git diff、修改摘要、未完成项和冲突项。

完成后等待 GPT/owner 复核。
```

---

## 9. GPT / Owner 复核 Prompt

```text
这是 Codex 按 CR-0003 修改后的 diff。请作为 NGU800 安全方案 owner 复核是否符合 CR。

重点检查：
1. 10_full_design.md 是否仍保持 pending review。
2. SEC2 是否已明确为 sign+encrypt mandatory。
3. PM/RAS/Codec 是否是默认 sign+encrypt，但 signature-only 仍为白名单例外。
4. board binding 是否默认进入 attestation，不默认阻断 SEC1。
5. JTAG scope/MUX 是否保持策略 confirmed、bit-level mapping TBD。
6. DMA/firewall/UserID 是否默认拒绝，具体地址表仍 TBD。
7. OOB/BMC proxy 是否没有被写成 RoT。
8. attestation report 字段是否正确区分 CONFIRMED / ASSUMED / TBD。
9. manufacturing/RMA 是否更新 USER freeze 和 RMA 安全边界。
10. 是否有未确认资料被误写成 CONFIRMED。
11. 是否有旧口径残留或跨文件不一致。

请输出：
- 通过项
- 问题项
- 必须修改项
- 可选优化项
- 是否允许 CR 进入 reviewed / closed 状态
```

---

## 10. 关闭条件

本 CR 只有在以下条件全部满足后才能关闭：

1. Codex 已完成所有指定文件修改。
2. GPT / owner 已复核 diff。
3. 必须修改项已清零。
4. `10_full_design.md` 状态仍为 pending review，或 owner 另行明确升级状态。
5. 所有新增结论均正确标记 `[CONFIRMED]`、`[ASSUMED]`、`[TBD]`。
6. `decision_log.md`、`open_questions.md`、`changelog.md`、`design_impact_matrix.md` 已同步。
7. `05_code_rules.md`、`06_traceability.md` 已同步。
8. 后续冻结项已明确记录到 open questions 或下一条 CR。

关闭时状态可改为：

```text
Status: reviewed
```

若后续 owner 确认全部必须项关闭，并允许 V2.4 进入 baseline，可通过下一条 CR 将状态升级为：

```text
V2.5 reviewed baseline
```
