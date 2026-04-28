# CR-0001: SEC1 加密、固件保护链、Master 章节重排与板级安全并入

Status: applied  
Base Commit: `c9e5a6383613c032a891b905c8dc15fdb8c0bf2c`  
Owner: NGU800 Security Design  
Created: 2026-04-27  
Source Context Pack: `.context/design_context_pack.md`  
Target Branch: current working branch  
Change Type: Design convergence + cross-file synchronization  
Risk Level: medium-high  
Applied: 2026-04-27 by Codex after user execution request  

---

## 1. 背景

当前 `security_workflow/03_detailed_design/03_detailed_design_master.md` 已经整合了 baseline、boot、key/cert、attestation、lifecycle/debug、interface、manufacturing/RMA 等章节，但仍存在以下问题：

1. `SEC1` 加密策略尚未收敛。当前多个章节仍使用“可选加密 / 可选解密 / 签名 only 或签名+加密”的旧口径。
2. 用户已明确要求：`SEC1` 也需要加密。因此 `SEC1` 的镜像保护策略必须从“可选解密”升级为确定要求。
3. `SEC1` 来源仍以启动方案为准，即来自 NOR Flash / 本地 Flash，不由 Host 下发。
4. BootROM 仍应保持最小启动编排角色，不应直接实现复杂密码学验签或解密逻辑。
5. `Root / Key / Cert` 当前在 master 中过早出现，读者在未理解 boot、attestation、debug 之前就进入密钥体系，影响理解。用户要求 Key/Cert 放到安全启动、认证、debug 之后统一介绍。
6. master 文档存在外层中文大标题和源章节原始编号重复的问题，例如 `# 二、Root of Trust...` 后紧接 `# 5. Root of Trust...`。
7. master 当前未正式并入已有的 `03_detailed_design/05_board_security.md`，仍把板级安全设计列为待补内容。
8. 本次变更影响 boot、key/cert、interface、impl design、master、traceability 等多个文件，必须按 Change Request 流程执行，不能只手工改导出版。

---

## 2. 设计裁决

### D-0001. SEC1 强制加密

`SEC1` 固件必须采用：

```text
签名 + 加密
```

该要求为 `[CONFIRMED]`。

含义：

1. `SEC1` 存储在 NOR Flash / 本地 Flash 中时，应以受保护镜像形式保存。
2. `SEC1` 镜像执行前必须完成：
   - header 解析；
   - key_id / signer slot 检查；
   - revoke bitmap 检查；
   - version / rollback floor 检查；
   - signer hash / trust anchor 检查；
   - signature 校验；
   - payload hash 校验；
   - 解密 / unwrap；
   - measurement 记录。
3. `SEC1` 解密必须由 eHSM / 安全子系统密码服务完成，不得由 BootROM 软件直接实现复杂解密算法。
4. BootROM 只负责：
   - 最小初始化；
   - 定位 SEC1 镜像；
   - 调用 eHSM `VERIFY_SEC1` / `VERIFY_IMAGE` 等受控接口；
   - 根据 eHSM 返回结果装载或拒绝启动；
   - 记录早期状态和错误码；
   - 跳转执行 SEC1。

### D-0002. SEC2 与后续运行期固件的加密策略

本 CR 不强制把所有运行期固件全部升级为绝对强制加密，但要求文档统一为：

1. 所有固件必须签名。
2. `SEC1` 必须签名 + 加密。
3. `SEC2`、PM、RAS、Codec 等后续关键固件：
   - 在 USER/PROD 产品形态中应默认采用签名 + 加密；
   - 若某产品阶段采用签名 only，必须由产品安全策略显式允许，并在 attestation / debug / lifecycle 状态中可见；
   - 不得把签名 only 作为 USER/PROD 的默认推荐路径。
4. 文档中不得再泛泛写“关键镜像是否加密待定”。可将未冻结项收敛为：
   - 哪些非敏感运行期镜像允许签名 only；
   - board binding 是否参与解密/验证；
   - SEC2/PM/RAS/Codec 是否首版全部强制加密。

### D-0003. Root of Trust 仍前置，Key/Cert 细节后置

为满足读者理解顺序和用户要求，章节结构采用以下原则：

1. `Root of Trust` 的核心结论仍保留在 baseline / 总体架构前部：
   - eHSM 是 Root of Trust；
   - BootROM 不是密码学根；
   - Host / BMC / OOB 不进入信任链。
2. `Root / Key / Cert 体系详细设计` 后移到 boot、attestation、debug、interface、board 之后统一介绍。
3. 这样既不丢失 RoT 基线，又避免过早展开 key/cert 细节。

### D-0004. Master 文档统一编号

`03_detailed_design_master.md` 必须统一为单一编号体系。

建议采用阿拉伯数字多级标题：

```text
# 1. 设计基线摘要
## 1.1 ...
# 2. 安全总体架构
## 2.1 ...
# 3. 安全启动详细设计
...
```

要求：

1. 删除或降级源章节中重复的一级标题，例如：
   - `# 5. Root of Trust、密钥体系与证书体系`
   - `# 6. 安全启动详细设计`
   - `# 8. 设备身份与远程度量证明设计`
2. 保留源章节内容正文，不得压缩成概要。
3. 可以将源章节的原始 metadata（章节文件、状态、标记口径）保留为普通说明块，但不得破坏全书编号。

### D-0005. 板级安全章节正式并入 Master

`03_detailed_design/05_board_security.md` 已经是正式章节，应并入 `03_detailed_design_master.md`。

要求：

1. 不再把板级安全作为“待补章节”。
2. 并入内容应保留其关键设计：
   - BMC / OOB / Sideband 不高于 Host；
   - 管理子系统总体架构和流程可遵循，但安全细节必须二次裁决；
   - JTAG 必须受 lifecycle、debug auth、scope bitmap、MUX gating 和审计控制；
   - 管理子系统 DMA 不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区等；
   - 电源/复位/PowerBrake/PG/FAULT 等影响安全状态的事件应进入状态机或审计；
   - 单 Die / 双 Die / board binding / die binding 的开放问题必须保留。
3. 对仍未冻结的 board binding、JTAG scope bitmap、DMA/firewall/UserID 等，不得误写成已冻结。

### D-0006. Master 是导出版，源章节仍是事实源

本次不允许只改 `03_detailed_design_master.md`。

凡是 master 中因本 CR 发生口径变化的内容，必须同步到源章节：

1. `03_detailed_design/01_boot.md`
2. `03_detailed_design/02_key_cert.md`
3. `03_detailed_design/06_interface.md`
4. 必要时同步 `04_impl_design/*`

避免后续重新整合 master 时旧口径覆盖新口径。

### D-0007. 实现级文件必须体现 SEC1 加密

`SEC1` 强制加密影响实现级设计，至少需要检查并必要时修改：

1. `04_impl_design/efuse_key_fw_header_design.md`
   - `enc_algo`
   - `enc_flags`
   - `key_slot`
   - `wrapped_cek`
   - `nonce/iv`
   - `aad`
   - `ciphertext_len`
   - `image_type = SEC1` 的强制加密策略
2. `04_impl_design/mailbox_if.md`
   - `VERIFY_SEC1`
   - `VERIFY_IMAGE`
   - policy flags
   - decrypt result
   - error code
   - address whitelist
3. `04_impl_design/manufacturing_provisioning.md`
   - FW_KEK / image protect key provisioning
   - USER 前 key/anchor 锁定
4. `04_impl_design/spdm_report.md`
   - 若加密状态、secure boot policy 或 non-secure bypass 状态进入 report，应检查 report 字段是否需要更新。

---

## 3. 目标章节顺序

`03_detailed_design_master.md` 建议重排为：

```text
# 1. 设计基线摘要
# 2. 安全总体架构
# 3. 安全启动详细设计
# 4. 设备身份与远程度量证明设计
# 5. 安全调试与生命周期控制
# 6. 内外部接口设计
# 7. 板级安全设计
# 8. Root of Trust、密钥体系与证书体系
# 9. 制造、灌装、部署与 RMA
# 10. 风险、依赖、冻结项与开放问题
# 11. 附录
```

说明：

1. `Root of Trust` 的核心基线在第 1/2 章保留。
2. `Root / Key / Cert` 详细章节后移到第 8 章。
3. `板级安全设计` 从待补变为正式章节。
4. 原“待补章节清单”应改为“风险、依赖、冻结项与开放问题”，只保留真正未冻结事项。

---

## 4. 影响文件与修改要求

### 4.0 受影响文件摘要

本 CR 已授权并应用到以下文件范围：

- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/01_boot.md`
- `security_workflow/03_detailed_design/02_key_cert.md`
- `security_workflow/03_detailed_design/03_attestation.md`
- `security_workflow/03_detailed_design/04_lifecycle_debug.md`
- `security_workflow/03_detailed_design/05_board_security.md`
- `security_workflow/03_detailed_design/06_interface.md`
- `security_workflow/03_detailed_design/07_manufacturing_rma.md`
- `security_workflow/03_detailed_design/03_detailed_design_master.md`
- `security_workflow/03_detailed_design/03_detailed_design_master_v2.4.md`
- `security_workflow/03_detailed_design/10_full_design.md`
- `security_workflow/04_impl_design/efuse_key_fw_header_design.md`
- `security_workflow/04_impl_design/mailbox_if.md`
- `security_workflow/04_impl_design/spdm_report.md`
- `security_workflow/04_impl_design/manufacturing_provisioning.md`
- `security_workflow/06_traceability.md`
- `00_project/decision_log.md`
- `00_project/changelog.md`
- `00_project/open_questions.md`
- `05_traceability/design_impact_matrix.md`

### 4.1 `security_workflow/01_constraints.md`

修改类型：`modify`

必须修改：

1. 新增或增强约束：
   - `SEC1` 必须签名 + 加密；
   - `SEC1` 解密必须走 eHSM；
   - BootROM 不得实现复杂密码学解密；
   - Host 不参与 SEC1 投递。
2. 保留：
   - `SEC1` 从 NOR Flash 加载；
   - BootROM 最小加载与 eHSM 调用；
   - Host 不可信；
   - eHSM 为 Root of Trust。
3. 不得把 SEC2/PM/RAS/Codec 全部无条件写成必须加密，除非在产品策略中明确分级。

建议新增约束 ID：

```text
C-BOOT-04: SEC1 image confidentiality
```

建议内容：

```text
[CONFIRMED] SEC1 固件在正式安全启动路径中必须采用签名 + 加密保护。SEC1 解密、key unwrap、hash/signature 校验必须通过 eHSM 或安全子系统受控密码服务完成，BootROM 不得直接实现复杂解密逻辑。
```

---

### 4.2 `security_workflow/02_baseline.md`

修改类型：`modify`

必须修改：

1. 在 Secure Boot baseline 中加入：
   - SEC1 = signed + encrypted；
   - SEC1 来源 = NOR Flash / 本地 Flash；
   - First verifier/decrypt service = eHSM。
2. 在 Host baseline 中强调：
   - Host 不下发 SEC1；
   - Host 只下发 SEC2 及后续镜像密文/受保护包。
3. 在 Key baseline 中加入：
   - FW Encrypt Branch 至少对 SEC1 为强制启用；
   - FW_KEK / CEK / wrapped key 策略进入实现级设计。

---

### 4.3 `security_workflow/03_detailed_design/01_boot.md`

修改类型：`modify`

必须修改：

1. 将 SEC1 校验规则中的“可选解密”改为：
   - `SEC1 必须解密`；
   - `SEC1 解密由 eHSM 完成`；
   - `SEC1 解密失败必须阻止启动`。
2. 将“量产关键镜像建议支持可选加密”改为更精确口径：
   - SEC1 强制加密；
   - SEC2/PM/RAS/Codec 按产品策略，USER/PROD 默认推荐签名 + 加密。
3. 关闭或重写开放问题：
   - 原“首版是否默认启用关键镜像加密”不得继续笼统存在；
   - 可改成“除 SEC1 外，哪些运行期镜像允许签名 only”。
4. 时序图和图下说明中明确：
   - BootROM 发起 VERIFY_SEC1；
   - eHSM 完成 verify + decrypt；
   - BootROM 只装载解密/解包后的受控结果或根据 eHSM 结果继续流程。
5. 不得改变：
   - SEC1 从 NOR Flash 加载；
   - BootROM 不直接实现密码学；
   - Host 只下发 SEC2 及后续固件。

---

### 4.4 `security_workflow/03_detailed_design/02_key_cert.md`

修改类型：`modify`

必须修改：

1. FW Encrypt Branch 从“首版可选占位”改为：
   - 对 SEC1 为强制；
   - 对 SEC2/后续关键固件按产品策略启用。
2. 增加或强化：
   - `FW_KEK`
   - `CEK`
   - `wrapped CEK`
   - `NGU800:FW:ENC`
   - lifecycle gating
   - key slot / key_id 与 image_type 绑定。
3. 保留证书链首版未完全冻结的开放项，不要把 X.509 / full cert chain 误写成已冻结。
4. 注意该章节在 master 中后移，但源文件自身文件名不必改变。

---

### 4.5 `security_workflow/03_detailed_design/03_attestation.md`

修改类型：`inspect / maybe modify`

需要检查：

1. SEC1 measurement 是否反映 verify + decrypt 成功后的受控镜像。
2. Attestation report 是否需要体现：
   - secure_boot_state；
   - image_confidentiality_policy；
   - non-secure boot / bypass 状态。
3. 本 CR 不强制新增复杂字段，除非现有 report 无法表达 SEC1 加密策略。

---

### 4.6 `security_workflow/03_detailed_design/04_lifecycle_debug.md`

修改类型：`inspect / maybe modify`

需要检查：

1. USER/PROD 下 SEC1 解密 key 是否受 lifecycle gating。
2. RMA/DEBUG 下是否允许加载 rescue image，应保持专用 signer / recovery trust，而不是绕过 SEC1 加密策略。
3. 板级 JTAG/OOB debug 与原 lifecycle/debug auth 章节保持一致。

---

### 4.7 `security_workflow/03_detailed_design/05_board_security.md`

修改类型：`inspect / no-change likely`

要求：

1. 作为 master 正式板级章节来源。
2. 如果源章节已经完整，原则上不改源文件。
3. 若发现与 SEC1 加密或 Host/OOB 边界冲突，只记录冲突，不自行设计新口径。

---

### 4.8 `security_workflow/03_detailed_design/06_interface.md`

修改类型：`modify`

必须修改：

1. `VERIFY_IMAGE` / `VERIFY_SEC1` 描述不得再只写“验签 / 可选解密”。
2. 建议写成：
   - `VERIFY_SEC1`: verify + decrypt mandatory；
   - `VERIFY_IMAGE`: verify mandatory, decrypt policy-dependent；
   - `image_type == SEC1` 时 decrypt required。
3. mailbox 请求/响应字段需要表达：
   - `enc_required`
   - `decrypt_applied`
   - `key_slot`
   - `wrapped_cek_present`
   - `decrypt_fail`
   - `policy_mismatch`
4. 地址白名单中明确：
   - Host 不得指定安全区地址；
   - SEC1 解密结果只允许进入 BootROM/SEC 认可的受控执行区或 staging 区。

---

### 4.9 `security_workflow/03_detailed_design/07_manufacturing_rma.md`

修改类型：`inspect / maybe modify`

需要检查：

1. Manufacturing provisioning 是否包括 FW_KEK / image protect key。
2. USER freeze 是否锁定 SEC1 解密相关 key slot / signer anchor / rollback counter。
3. RMA 是否不得绕过 SEC1 加密和 debug auth。
4. JTAG 测试路径进入 USER 前必须锁定或清理。

---

### 4.10 `security_workflow/03_detailed_design/03_detailed_design_master.md`

修改类型：`modify`

必须修改：

1. 按第 3 节目标章节顺序重排。
2. 保留原详细内容，不得压缩成概要。
3. 统一编号体系。
4. 删除源章节原始一级标题造成的重复编号。
5. 并入 `05_board_security.md` 正文。
6. 将 SEC1 加密策略改为 confirmed。
7. 删除或替换旧口径：
   - “SEC1 可选解密”
   - “首版是否默认启用关键镜像加密”
   - “板级安全待补”
8. 增加“风险、依赖、冻结项与开放问题”章节，用来承接仍未冻结事项。
9. 生成或更新导出版文件，建议文件名：
   - `security_workflow/03_detailed_design/03_detailed_design_master_v2.4.md`
   - 或根据仓库既有命名规范生成。

注意：

不得生成只包含概要的新 master，不得删除原章节大量正文。

---

### 4.11 `security_workflow/03_detailed_design/10_full_design.md`

修改类型：`inspect / maybe modify`

如果该文件仍作为完整设计导出版或后续整合来源，需要同步：

1. SEC1 强制加密；
2. Key/Cert 后置；
3. 板级章节状态；
4. 编号一致性。

若该文件已废弃，应在 changelog / decision_log 中说明其状态，避免后续误用。

---

### 4.12 `security_workflow/04_impl_design/efuse_key_fw_header_design.md`

修改类型：`modify`

必须检查并必要时修改：

1. FW header 是否支持 SEC1 加密：
   - `enc_algo`
   - `enc_mode`
   - `enc_flags`
   - `key_slot`
   - `wrapped_cek_offset`
   - `wrapped_cek_len`
   - `iv/nonce`
   - `aad`
   - `ciphertext_offset`
   - `ciphertext_len`
2. 增加 image type policy：
   - `IMAGE_TYPE_SEC1`: `SIGN_REQUIRED | ENCRYPT_REQUIRED | ROLLBACK_REQUIRED`
3. rollback counter 映射是否覆盖 SEC1。
4. signer anchor 与 encryption key slot 是否可区分。
5. 双算法字段是否保留国密 / 国际算法表达能力。

---

### 4.13 `security_workflow/04_impl_design/mailbox_if.md`

修改类型：`modify`

必须检查并必要时修改：

1. 增加或明确：
   - `VERIFY_SEC1`
   - `VERIFY_IMAGE`
   - `VERIFY_AND_MEASURE`
2. `VERIFY_SEC1` 必须包含：
   - image address / length；
   - image type；
   - expected lifecycle mask；
   - decrypt required；
   - rollback policy；
   - output buffer / destination constraints；
   - measurement slot；
   - result code。
3. 错误码至少区分：
   - signature fail；
   - hash fail；
   - decrypt fail；
   - key slot invalid；
   - policy mismatch；
   - rollback fail；
   - address invalid；
   - lifecycle deny。
4. 明确：
   - image_type == SEC1 时，decrypt 不可被 caller 关闭；
   - Host 不得直接调用 eHSM mailbox；
   - SEC/C908 是唯一安全服务 caller。

---

### 4.14 `security_workflow/04_impl_design/spdm_report.md`

修改类型：`inspect / maybe modify`

需要检查：

1. report 是否体现：
   - secure_boot_state；
   - lifecycle_state；
   - debug_state；
   - SEC1 hash/version/rollback；
   - optionally image protection policy。
2. 若已有字段足够表达，则无需强行新增。
3. 不得把加密状态当作“证明安全”的唯一依据，仍必须校验 measurement 和 signer/trust anchor。

---

### 4.15 `security_workflow/04_impl_design/manufacturing_provisioning.md`

修改类型：`inspect / maybe modify`

需要检查：

1. FW_KEK / image protect key provisioning；
2. SEC1 signer anchor；
3. SEC1 rollback counter 初值；
4. USER freeze 是否锁定 SEC1 解密 key slot；
5. 测试 key / 测试 signer / 测试 debug 白名单清理；
6. RMA 是否不得长期开放 SEC1 解密绕过路径。

---

### 4.16 `00_project/decision_log.md`

修改类型：`modify`

必须新增至少以下 decision：

1. `DEC-0001`: SEC1 强制签名 + 加密。
2. `DEC-0002`: Root/Key/Cert 详细章节后置，RoT 基线仍前置。
3. `DEC-0003`: 板级安全章节正式并入 master。
4. `DEC-0004`: Master 统一编号体系。

---

### 4.17 `00_project/changelog.md`

修改类型：`modify`

必须新增 CR 应用记录，包含：

1. CR ID；
2. Summary；
3. Files changed；
4. Applied commit；
5. Status。

---

### 4.18 `00_project/open_questions.md`

修改类型：`modify`

必须更新：

1. 关闭“SEC1 是否强制加密”的开放问题。
2. 保留或新增：
   - SEC2/PM/RAS/Codec 是否全部强制加密；
   - board binding 是否参与 firmware verify；
   - JTAG scope bitmap；
   - DMA/firewall/UserID；
   - report 是否包含 image protection policy。
3. 不得把仍未冻结项误删。

---

### 4.19 `05_traceability/design_impact_matrix.md`

修改类型：`modify`

必须记录 CR-0001 对以下内容的影响：

1. constraints；
2. baseline；
3. boot；
4. key/cert；
5. interface；
6. board；
7. manufacturing；
8. impl design；
9. master 导出版。

---

### 4.20 `security_workflow/06_traceability.md`

修改类型：`modify after CR`

如果仓库中该文件作为正式追踪文件使用，需同步：

1. 新增 `SEC1_ENCRYPT_REQUIRED` 需求追踪；
2. 关联到：
   - constraints；
   - boot；
   - key/cert；
   - mailbox；
   - fw_header；
   - manufacturing；
   - tests / QEMU mock；
3. 若该文件已被 `05_traceability` 替代，应在 changelog 中说明。

---

## 5. 需要替换 / 删除的旧口径

Codex 应搜索并处理以下旧口径：

### 5.1 必须替换

```text
SEC1 可选解密
```

替换为：

```text
SEC1 必须解密，且解密必须由 eHSM / 安全子系统受控密码服务完成。
```

---

```text
量产关键镜像建议支持可选加密，但首版最小必需是完整性和执行放行控制
```

替换为：

```text
SEC1 在正式安全启动路径中必须签名 + 加密；SEC2 及后续关键运行期固件在 USER/PROD 产品形态中默认建议签名 + 加密，若采用签名 only 必须由产品安全策略显式允许。
```

---

```text
VERIFY_IMAGE = 固件验签 / 可选解密
```

替换为：

```text
VERIFY_IMAGE = 固件验签 + 按 image_type/policy 执行解密；其中 SEC1 的解密为强制要求。
```

---

```text
板级安全设计待补
```

替换为：

```text
板级安全设计已作为正式章节并入 master；未冻结项保留在风险、依赖、冻结项与开放问题章节。
```

---

### 5.2 不允许简单删除，必须重写或迁移

以下内容不得直接删除：

1. cert chain 是否 full chain 的 TBD；
2. board binding 是否默认开启的 TBD；
3. JTAG scope bitmap TBD；
4. DMA/firewall/UserID TBD；
5. OOB provisioning proxy TBD。

这些内容应迁移到“风险、依赖、冻结项与开放问题”章节或 `00_project/open_questions.md`。

---

## 6. 不允许 Codex 自行改变的内容

Codex 不得自行改变以下安全设计口径：

1. eHSM 是 Root of Trust。
2. BootROM 不是密码学根。
3. BootROM 不实现复杂密码学验签/解密。
4. SEC1 来自 NOR Flash / 本地 Flash，不由 Host 下发。
5. Host 不可信，只能投递 SEC2 及后续镜像/触发流程/读取状态。
6. SEC/C908 是安全服务调用控制面。
7. eHSM 是安全服务执行面。
8. 私钥不得离开 eHSM。
9. BMC / OOB / Sideband 不进入 Root of Trust，信任级别不高于 Host。
10. USER/PROD 下 JTAG 默认关闭。
11. Debug/RMA 必须认证并有 scope / expire / audit。
12. 未冻结的 board binding、JTAG scope、DMA/firewall/UserID 不得被误标成 confirmed。
13. 不得压缩 master 文档为概要，不得删除原详细章节正文。

如发现冲突，Codex 必须写入：

```text
00_project/open_questions.md
```

并在执行摘要中列出，不得自行拍板。

---

## 7. 验收标准

### 7.1 设计一致性验收

必须满足：

1. 所有相关文件中，`SEC1` 均被描述为签名 + 加密。
2. 所有相关文件中，`SEC1` 解密均由 eHSM / 安全子系统受控密码服务完成。
3. 不存在 BootROM 直接实现复杂解密算法的描述。
4. `SEC1` 来源仍为 NOR Flash / 本地 Flash。
5. Host 不下发 SEC1。
6. Host 只下发 SEC2 及后续镜像/密文包，不参与信任链和 release。
7. Key/Cert 详细章节在 master 中后置。
8. Root of Trust 基线仍在前文可见。
9. 板级安全章节已经正式并入 master。
10. master 中不再将板级安全列为待补。

### 7.2 文档质量验收

必须满足：

1. `03_detailed_design_master.md` 不得被压缩成概要。
2. 原章节详细内容应基本保留。
3. 标题编号统一。
4. 不再出现大标题与源章节编号并列冲突。
5. 待补章节清单改为风险/依赖/开放问题。
6. 所有真正未冻结项均迁移到开放问题，不得丢失。

### 7.3 实现级验收

必须满足：

1. FW header 结构支持 SEC1 加密。
2. mailbox 接口支持 SEC1 verify + decrypt mandatory。
3. 错误码区分 decrypt fail / policy mismatch / key invalid / rollback fail。
4. manufacturing provisioning 至少检查 FW_KEK / image protect key。
5. traceability 记录 SEC1 加密需求到实现文件的映射。

### 7.4 流程验收

必须满足：

1. `00_project/decision_log.md` 已更新。
2. `00_project/changelog.md` 已更新。
3. `00_project/open_questions.md` 已更新。
4. `05_traceability/design_impact_matrix.md` 已更新。
5. 执行 `tools/check_cr_sync.py change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md` 并记录结果。
6. 输出 `git diff --stat` 和 `git diff` 供 GPT 复核。

---

## 8. Codex 执行 Prompt

将以下内容直接给 Codex 执行：

```text
请读取并执行 change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md。

你不是本次安全架构设计者，不得自行改变安全设计口径。你只负责按照 CR 的设计裁决修改仓库。

执行要求：
1. 严格保留原 03_detailed_design_master.md 的详细内容，不得压缩成概要。
2. 对 master 进行章节重排、编号统一、内容并入和旧口径替换。
3. 同步修改源章节，避免 master 与源文件不一致。
4. 同步修改实现级设计文件中与 SEC1 加密、FW header、mailbox、manufacturing 相关的内容。
5. 更新 decision_log、changelog、open_questions、design_impact_matrix。
6. 不得自行改变以下口径：
   - eHSM 是 Root of Trust；
   - BootROM 不直接实现复杂密码学；
   - SEC1 来自 NOR Flash；
   - Host 不下发 SEC1；
   - Host 不进入信任链；
   - BMC/OOB 不高于 Host；
   - USER/PROD JTAG 默认关闭。
7. 如果发现冲突或无法确认的点，写入 open_questions.md，不要自行拍板。
8. 修改完成后运行：
   python tools/check_cr_sync.py change_requests/CR-0001-sec1-encryption-fw-protection-master-sync.md

完成后输出：
1. git diff --stat
2. 修改文件列表
3. 每个文件修改摘要
4. 被替换/删除的旧口径
5. 未完成项
6. 冲突项
7. check_cr_sync.py 输出
8. 是否建议提交 commit
```

---

## 9. GPT 复核 Prompt

Codex 完成后，将 `git diff --stat` 和 `git diff` 发给 GPT，并使用以下 prompt：

```text
这是 Codex 按 CR-0001 修改后的 diff。请作为安全架构评审者检查是否符合 CR。

重点检查：
1. SEC1 是否明确为签名 + 加密。
2. SEC1 来源是否仍为 NOR Flash / 本地 Flash。
3. BootROM 是否仍不直接实现复杂密码学验签/解密。
4. eHSM 是否仍是验签、解密、密钥、counter、lifecycle 的安全执行面。
5. Host 是否仍只是 SEC2/后续镜像的传输方，不进入信任链，不下发 SEC1。
6. Root / Key / Cert 是否后移，同时 RoT 基线仍在前文可见。
7. 原 03 文档详细内容是否被保留，没有被压缩成概要。
8. 板级安全章节是否正式并入 master，且不再列为待补。
9. JTAG、OOB、DMA、board binding 等未冻结项是否被正确保留为开放问题。
10. FW header、mailbox、manufacturing 是否同步体现 SEC1 加密。
11. 是否仍有“可选解密”“首版是否默认启用关键镜像加密”“板级安全待补”等旧口径残留。

请输出：
- 通过项
- 问题项
- 必须修改项
- 可选优化项
- 是否允许进入 reviewed 状态
```

---

## 10. 本 CR 关闭条件

本 CR 只有在以下条件全部满足后才能关闭：

1. Codex 已完成仓库修改。
2. `check_cr_sync.py` 无阻断性失败。
3. GPT 已复核 diff。
4. GPT 复核中的“必须修改项”已清零。
5. `decision_log.md` / `changelog.md` / `open_questions.md` / `design_impact_matrix.md` 已同步。
6. master 文档和源章节不再存在 SEC1 加密相关旧口径冲突。
7. 导出版已生成或在 changelog 中明确说明生成方式。

关闭后状态改为：

```text
Status: closed
```

如果后续 CR 修改本 CR 的裁决，应将本 CR 状态改为：

```text
Status: superseded
```

并注明 superseded by 的 CR ID。
