# CR-0006: 固件包格式、制作流程与设备侧 verify/decrypt 流程补强

Status: accepted-for-application / applied-by-codex / refined-2026-05-09 / owner-review-pending
Base Commit: `df815e2`
Owner: Security Owner / Boot Owner / Tooling Owner
Codex Role: accepted-apply；落实用户裁决，不新增 eHSM 未支持的 physical ABI
Created: 2026-05-08
Accepted: 2026-05-08
Applied: 2026-05-08
Source / Context: 用户要求安全启动流程应包含固件格式、固件制作过程、启动时解密和验签过程，并参考 `SRC-001 当前安全方案基线` 第 7 章的流程化表达和图形化说明；2026-05-09 用户进一步指出 `10_full_design.md` 未清楚说明最终 SEC1 固件制品组成、签名位置、必须被签名覆盖的区域，以及制作过程和设备侧解密验签过程的工程对应关系
Primary Topic: firmware package format / image build flow / verify-decrypt boot flow
Change Type: detailed-design expansion / implementation-design synchronization
Risk Level: medium

---

## 1. 背景

`SRC-001 当前安全方案基线` 第 6/7 章已经从方案表达角度描述过固件包格式、平台侧制作流程和设备侧验证/解包流程，典型表达为：

- 平台侧生成 payload hash、随机 CEK、IV；
- 对 payload 加密；
- 生成 signed region、signature、wrapped CEK；
- 设备侧解析 header、校验 signer、验签、rollback、unwrap、解密、hash 校验并加载执行。

CR-0004 后，NGU800 安全方案已经裁决：

- SEC1 / SEC2 的 physical verify/decrypt container 必须 follow eHSM native secure boot image header；
- NGU 不再定义与 eHSM 并列的 physical FW header；
- NGU 项目级 metadata 放入 eHSM Code region 的 protected manifest；
- per-image CEK / wrapped CEK 仍为 `eHSM-customization-TBD`，不得写成已冻结 physical ABI。

用户在 2026-05-08 进一步指出：安全启动的详设应明确固件格式、制作过程、启动时解密和验签过程，并用图形和流程描述增强可落地性。

因此本 CR 的目标是：保留 `SRC-001` 第 7 章“流程化讲清楚”的优点，但把物理格式和设备侧职责改写到 CR-0004/CR-0005 后的 eHSM-native 口径。

## 2. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `df815e2` |
| 输入来源 | `SRC-001 当前安全方案基线` 第 6/7 章；`SRC-006 eHSM Firmware TRM`；`SRC-007 eHSM Bootloader TRM`；CR-0001/CR-0003/CR-0004/CR-0005 |
| 当前主详设 | `security_workflow/03_detailed_design/10_full_design.md` 是完整详设与代码落地主入口 |
| 当前实现分片 | `security_workflow/04_impl_design/efuse_key_fw_header_design.md` 包含 eHSM native header / NGU manifest / key / OTP / verify-decrypt path 设计 |
| 已知缺口 | 文档已给出 eHSM header 与 manifest 分层，但平台侧制作流程、设备侧验证/解密流程和图形化说明不够直观 |
| 用户裁决 | 需要补充固件制作和启动解密验签流程，并尽量图形化和流程化 |

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 物理固件容器 | 仍采用 eHSM native secure boot image header，不恢复自定义 NGU physical header | 已由 CR-0004 冻结；避免两套 header 事实源 | `[CONFIRMED]` |
| NGU 项目级格式 | NGU image type、policy、rollback domain、measurement slot、payload digest 等进入 protected manifest / policy table | 保留项目策略表达，同时不覆盖 eHSM native header | `[CONFIRMED]` |
| 平台侧制作流程 | 文档应明确 payload/manifest 生成、eHSM native package 生成、签名/加密/版本计数绑定、发布验收检查 | 让工具链可照文档落地 | `[CONFIRMED]` |
| 设备侧 verify/decrypt 流程 | 文档应明确 BootROM/SEC 提供镜像地址和白名单 output buffer，eHSM 完成 native header 检查、验签、rollback、decrypt output，随后 BootROM/SEC 解析 manifest 并 release | 让启动代码和 eHSM 适配层可照文档落地 | `[CONFIRMED]` |
| current_plan 旧格式 | `header + Signed Region + signature + wrapped_cek + enc_payload` 只能作为设计意图参考，不作为 wire/storage physical format | 旧格式与 CR-0004 冲突 | `[CONFIRMED]` |
| per-image CEK / wrapped CEK | 继续保持 `eHSM-customization-TBD`；文档只说明若 eHSM owner 后续支持才可进入扩展路径 | 避免 Codex 发明 eHSM 未确认 ABI | `[CONFIRMED]` |
| 图形化表达 | boot/full/impl 文档应补充固件包布局图、平台侧制作流程图、设备侧验证流程图 | 满足用户希望“直观”的要求 | `[CONFIRMED]` |

### 3.1 2026-05-09 精化裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| SEC1 最终制品组成 | SEC1 最终发布制品必须表达为 eHSM native secure image package；工程视角显式拆为 `Part A: eHSM native header/control metadata`、`Part S: eHSM signature/authentication material`、`Part B: eHSM protected Code region`；Code region 解密验签通过后的明文逻辑布局为 `NGU protected manifest + SEC1 payload + padding/alignment` | 让详设能直接指导 image packager、BootROM/eHSM adapter 和测试向量实现，并避免把签名隐藏在 header/meta 文字中 | `[CONFIRMED]` |
| 签名/认证覆盖范围 | NGU 对 SEC1 的最低认证覆盖要求是完整 Code region，即 `NGU protected manifest + SEC1 payload + padding/alignment counted by Code_Size` | manifest 决定 image type、policy、version、load/entry 和 release 条件，不能放在签名保护范围之外 | `[CONFIRMED]` |
| 加密覆盖范围 | SEC1 正式安全启动路径必须对完整 Code region 执行 eHSM sign+encrypt profile；header 仍为明文 eHSM native header，便于 eHSM 解密前解析 | 对齐 eHSM native header 设计，同时保证 SEC1 payload 和 manifest 机密性 | `[CONFIRMED]` |
| 未认证 header 字段使用限制 | 若 eHSM native header 中某些控制字段没有被 eHSM 认证/AAD 覆盖，BootROM / SEC 不得将这些字段作为 NGU 项目级 release 决策依据；同等安全语义必须在已认证保护的 manifest 中表达 | 防止攻击者修改明文 header 中的项目级策略语义绕过 release policy | `[CONFIRMED]` |
| 工程验收向量 | 工具链和 QEMU/stub 测试必须覆盖篡改 manifest `ngu_image_type`、`entry_addr`、`version_counter`、payload 字节、`Code_Size` 或截断 Code region 的失败路径 | 把文档要求落到可验证的工程测试 | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_inputs/inputs_manifest.md` | Yes | 登记 CR-0006 变更输入 |
| `security_workflow/01_constraints.md` | Yes | 增加固件包制作与 verify/decrypt 一致性约束 |
| `security_workflow/02_baseline.md` | Yes | 增加 firmware package build/verify baseline |
| `security_workflow/03_detailed_design/01_boot.md` | Yes | 在镜像格式章节补平台制作与设备验证流程图 |
| `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | Yes | 补实现级固件包布局、制作流程、验证流程和工具链规则 |
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 同步 boot 章节与第 10 章实现级内容，作为代码落地主入口 |
| `security_workflow/05_code_rules.md` | Yes | 增加 firmware package / image packager / verify-decrypt 规则 |
| `security_workflow/06_traceability.md` | Yes | 增加固件制作与设备验证追踪链路 |
| `security_workflow/04_change_impact.md` | Yes | 记录 CR-0006 影响与一致性检查 |
| `05_traceability/design_impact_matrix.md` | Yes | 记录 CR-0006 影响矩阵 |
| `00_project/decision_log.md` | Yes | 记录固件包制作/验证流程裁决 |
| `00_project/changelog.md` | Yes | 记录仓库变更 |
| `templates/boot_image_format_template.md` | Yes | 更新后续生成模板，避免再次输出旧 physical header |
| `codex/skills/ngu800-security/templates/boot_image_format_template.md` | Yes | 同步 skill 模板 |

## 5. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `01_constraints.md` | 新增或补充 C-BOOT 约束，要求平台制作和设备验证共享 eHSM-native + protected manifest 契约 | 不得把旧 NGU header 升级为 physical ABI |
| `02_baseline.md` | Baseline summary 和 secure boot 关键约束增加 package build/verify 一致性 | 不改变 RoT、First Verifier、Host trust boundary |
| `01_boot.md` | 在 6.10 增加 layout 图、平台制作流程图、设备验证流程图和逐步流程 | 不引入 BootROM 软件实现复杂 crypto 的表述 |
| `efuse_key_fw_header_design.md` | 在 manifest/header 设计处补完整实现级流程；明确旧 current_plan 格式只作参考 | 不冻结 manifest bit-level ABI、exact key ID、wrapped CEK |
| `10_full_design.md` | 同步 boot 章节和第 10 章实现级分片，不做摘要替代 | 不破坏 CR-0005 主事实源规则 |
| `05_code_rules.md` | 增加工具链和 verify/decrypt path MUST / MUST NOT 规则 | 不覆盖既有 R-FW 规则语义 |
| `06_traceability.md` | 增加 T-FW-PKG 或等价 trace | 不删除既有 T-EHSM/T-BOOT trace |
| templates | 增加 package build/verify flow 章节要求 | 不再输出旧 Minimal Header 模板作为默认 |

## 6. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| `header + Signed Region + signature + wrapped_cek + enc_payload` 是最终物理格式 | `SRC-001` 参考文本 / 历史草案 | eHSM native header + Code region protected manifest + payload；wrapped CEK 仅为 customization TBD | CR-0004 已裁决 eHSM-native 优先 |
| BootROM 解析自定义 header 并执行 unwrap/decrypt | 历史草案 | BootROM 只定位镜像、准备受控 output buffer、调用 eHSM，成功后解析 manifest/放行 | BootROM 不实现复杂 crypto |
| 工具链可自行定义 key slot / CEK 包裹格式 | 历史草案 / 模板风险 | 工具链必须跟随 eHSM owner-confirmed key ID、control field、image tool 或 command ABI | 避免工具链发明 eHSM physical ABI |

## 7. 不允许 Codex 自行改变的内容

- 不得改变 Root of Trust、First Cryptographic Verifier、BootROM minimal role、SEC boot control plane、Host untrusted boundary。
- 不得改变 SEC1/SEC2 sign+encrypt mandatory 和 eHSM verify+decrypt output path。
- 不得将 manifest bit-level ABI、exact key ID、exact OTP/control bit、per-image rollback counter、per-image CEK / wrapped CEK 从 `[TBD]` 升级为 `[CONFIRMED]`。
- 不得新增 eHSM 未确认命令、字段 offset、key slot 或 physical OTP layout。
- 不得删除 CR-0004/CR-0005 的 source-conformance 和主事实源规则。

## 8. 验收标准

- [x] 已建立 CR-0006 并登记 source/impact。
- [x] `01_boot.md` 和 `10_full_design.md` 安全启动章节包含固件包布局图、平台侧制作流程、设备侧 verify/decrypt 流程。
- [x] `efuse_key_fw_header_design.md` 和 `10_full_design.md` 第 10 章包含实现级固件包格式、制作流程、验证流程和工具链规则。
- [x] `10_full_design.md` 已补充 SEC1 最终制品组成、签名/认证覆盖范围、加密覆盖范围、未认证 header 字段使用限制、制作端与设备端对应关系。
- [x] `05_code_rules.md` 和 `06_traceability.md` 已补充 SEC1 Code region 认证覆盖、未认证 header 字段限制和 tamper vector 测试追踪。
- [x] 文档明确 current_plan 第 7 章旧自定义 header 只作为流程意图参考，不作为 physical ABI。
- [x] 未把 per-image CEK / wrapped CEK、manifest ABI、exact key ID、exact OTP bit 升级为 `[CONFIRMED]`。
- [x] code rules、traceability、change impact、decision_log、changelog 已同步。
- [x] `git diff --check` 通过。
- [x] 搜索确认没有新增“NGU 自定义 physical header 为最终格式”的旧口径。

## 9. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-08 |
| 修改文件 | `change_requests/CR-0006-firmware-package-build-verify-flow.md`; `security_inputs/inputs_manifest.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/04_impl_design/ehsm_source_conformance_matrix.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `security_workflow/04_change_impact.md`; `05_traceability/design_impact_matrix.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `00_project/open_questions.md`; `templates/boot_image_format_template.md`; `codex/skills/ngu800-security/templates/boot_image_format_template.md` |
| 未修改但检查过的文件 | `security_inputs/current_plan/安全方案.pdf`; `security_inputs/inputs_manifest.md`; `05_traceability/file_sync_checklist.md`; `05_traceability/design_impact_matrix.md`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md`; `security_workflow/03_detailed_design/01_boot.md`; `security_workflow/04_impl_design/efuse_key_fw_header_design.md`; `security_workflow/03_detailed_design/10_full_design.md` |
| 未完成项 | 无；待 owner/GPT review 后关闭 CR |
| 执行说明 | 本 CR 只补强固件包制作/验证流程表达，不发明 eHSM 未确认 physical ABI。已验证 `04_impl_design` 实现分片正文同步到 `10_full_design.md` 第 10 章，CR-0006 关键 trace/rule/open question 均可检索，旧 current_plan 包格式未被误写成最终 physical ABI，`git diff --check` 通过，无行尾空白。 |

### 9.1 2026-05-09 精化执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-09 |
| 修改文件 | `security_inputs/inputs_manifest.md`; `05_traceability/design_impact_matrix.md`; `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md`; `change_requests/CR-0006-firmware-package-build-verify-flow.md`; `00_project/decision_log.md`; `00_project/changelog.md` |
| 未修改但检查过的文件 | `security_inputs/inputs_manifest.md`; `05_traceability/file_sync_checklist.md`; `05_traceability/design_impact_matrix.md`; `security_workflow/03_detailed_design/10_full_design.md` |
| 未完成项 | eHSM TRM 中签名字段 exact offset、key ID exact mapping、manifest bit-level ABI 仍按既有 `[TBD]` 跟踪 |
| 执行说明 | 本次只精化 CR-0006 已覆盖的固件包格式/制作/verify-decrypt 主题，不改变 RoT、First Verifier、SEC1/SEC2 强制 sign+encrypt、eHSM native header 优先级，也不新增 eHSM 未确认 physical ABI。 |

## 10. GPT / Owner 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 | 待复核 |
| Review 结论 | `owner-review-pending` |
| 阻塞问题 | 无已知阻塞；manifest ABI / exact key ID / wrapped CEK 等仍跟随既有 OQ |
| 非阻塞建议 | 建议后续由工具链 owner 基于本章输出 `tools/image_packager` 的参数规范和 golden vector |
| 是否允许关闭 CR | No，待 owner review |
