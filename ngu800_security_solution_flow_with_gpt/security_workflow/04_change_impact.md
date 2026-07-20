# NGU800 安全方案变更影响分析

## 1. 新增 / 更新输入摘要

| Change ID | New / Updated Source | Summary | Priority |
|---|---|---|---|
| CHG-005 | `SRC-005 管理子系统方案` / `security_inputs/board/管理子系统.pdf` | 新增管理子系统方案；总体架构、模块职责、带外链路、电源/复位流程、单/双 Die 约束作为系统级输入采用；涉及安全且考虑不足、明显存在漏洞或与既定安全基线冲突的内容不直接继承 | medium |
| CHG-010 | `SRC-008 当前收敛安全软件方案 2.0` / `security_inputs/current_plan/芯片安全软件方案_2.0.pdf` | 用户明确 2.0 PDF 为最新收敛方案；无特殊说明时当前方案以该版为准，旧 `SRC-001` 降级为历史流程参考 | high |
| CHG-011 | `SRC-009 OSR eHSM 软件代码包 4019`; `SRC-010 eHSM4.0 ROM Patch 方案` | 用户新增 OSR eHSM BL/FW/Host/API/tool 代码包，并明确后续 HSM 已提供安全服务原则上以 OSR 代码为准；新增 eHSM4.0 ROM Patch 方案作为 OTP + hardware BOOT patch 机制输入 | high |

## 2. 输入要点提取

| Topic | Extracted Input | Security Handling |
|---|---|---|
| 带外管理通道 | BMC、OAM、模组 MCU、GPU、板级 MCU/GPU 之间存在 SMBus/I2C、I3C、PCIe、UART、JTAG 等链路 | 链路和流程可参考；安全服务不得直达，必须经 SEC/eHSM 收敛 |
| SMBus/I2C / I3C | SMBus/I2C 支持低速管理；I3C 支持更高频率和 slave/master 数据流 | 可作为 OOB 管理通道，高权限操作必须鉴权和 lifecycle gating |
| JTAG | 可接入 GPU JTAGBUS、寄存器空间、DRAM、GPU Flash、安全子系统、CPU 调试单元、板级 MCU | 不直接采用默认开放语义；必须引入 debug auth、scope bitmap、MUX gating、审计和 USER 默认关闭 |
| DMA | 管理子系统内部考虑通用 AXI DMA，低速外设绑定物理通道 | 必须限制到 firewall 白名单 buffer，禁止访问安全域 |
| mailbox / 中断 / 互斥 | 管理子系统存在 mailbox、中断和互斥访问机制 | 只能作为协作机制，不得替代权限检查或安全服务入口 |
| 电源 / 复位 | 板级 MCU 管理 GPU 电源、上下电顺序、异常响应和定位 | 影响安全状态时必须进入状态机和审计 |
| 单 Die / 双 Die | OOB 对外只呈现一个管理设备，硬件接口对外只 DIE0 出 OOB 接口 | 需联动 board/die binding、证明报告和跨 Die 访问策略 |
| OSR eHSM 软件代码 | `security_inputs/sw/` 提供 BL/FW/Host API、mailbox req/rsp、secure boot verify/upgrade、OTP/key/counter、debug/lifecycle、外部 OTP/Flash driver API、image/OTP tool 等真实实现输入 | eHSM 已提供服务必须以 OSR 代码为实现事实源；方案和后续代码不得继续发明并列 eHSM ABI |
| eHSM ROM Patch | `eHSM4.0 Patch方案.pdf` 描述 OTP patch 表由硬件 BOOT 加载，CPU 访问 IROM 命中时无感返回替换指令 | 作为 ROM 实际执行指令可变更机制纳入 OTP、制造、锁定、验收、审计和 attestation 影响分析 |

## 3. 受影响约束

| Constraint ID | Previous Statement | Updated Statement | Impact Level |
|---|---|---|---|
| `C-BOARD-01` | 不存在 | 管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决 | high |
| `C-BOARD-02` | 不存在 | 带外管理通道不得成为安全策略绕过路径 | high |
| `C-BOARD-03` | 不存在 | JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制 | high |
| `C-BOARD-04` | 不存在 | 管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须被隔离和审计 | high |
| `C-ACCESS-01` / `C-ACCESS-02` | 已要求安全子系统隔离、UserID + Firewall | 适用范围扩展到管理子系统 DMA、OOB 桥接、JTAG MUX、板级复位相关访问路径 | medium |
| `C-DEBUG-01` / `C-DEBUG-02` | 已要求 USER 关闭调试、DEBUG/RMA 必须认证 | 适用范围扩展到板级 JTAG、CPLD/MUX、边界扫描和板级 MCU 调试路径 | medium |
| `C-SRC-02` | 不存在 | eHSM 已提供安全服务必须以 OSR 软件代码包为实现事实源 | high |
| `C-EHSM-02` | `C-EHSM-01` 只覆盖 TRM/OTP/key/counter source-conformance | OSR BL/FW/Host API 代码进入 eHSM source-conformance gate | high |
| `C-EHSM-03` | 不存在 | eHSM4.0 ROM Patch 作为 OTP + 硬件 BOOT 机制纳入安全约束 | high |

## 4. 受影响 Baseline

| Baseline Area | Impact | Updated Decision |
|---|---|---|
| Board / OOB trust model | `SRC-005 管理子系统方案` 增加管理子系统整体架构和带外链路 | BMC/OOB/板级 MCU/管理子系统不进入 Root of Trust，只作为受控链路或代理 |
| JTAG / debug model | `SRC-005 管理子系统方案` 描述 JTAG 具有访问 GPU/CPU/DRAM/Flash/安全子系统能力 | JTAG 不按默认开放能力采用，必须 lifecycle + debug auth + scope + MUX gating |
| DMA / mailbox / interrupt | `SRC-005 管理子系统方案` 描述管理子系统 DMA、mailbox、中断和互斥机制 | 这些机制只可用于受控协作，不得绕过 SEC/eHSM 和 firewall |
| Power / reset control | `SRC-005 管理子系统方案` 描述板级电源、上下电和复位管理 | 影响安全状态的事件必须进入安全状态机或审计 |
| eHSM software source | `SRC-009` 提供 OSR 4019 版本 BL/FW/Host API 和工具输入 | eHSM 已提供服务、mailbox ABI、Host API、tool 行为以 OSR 代码为实现事实源；字段级差异进入后续 source-conformance |
| eHSM ROM patch | `SRC-010` 提供 ROM patch 机制说明 | Patch 是 OTP + hardware BOOT + CPU/IROM 指令替换机制，不是运行期热补丁入口 |

## 5. 受影响章节

| Chapter | Why Impacted | Regenerate Needed |
|---|---|---|
| `01_constraints.md` | 新增管理子系统、OOB、JTAG、DMA、复位相关安全约束 | done |
| `02_baseline.md` | 增加 Board / OOB / 管理子系统边界和 adopted/rejected 裁决 | done |
| `03_detailed_design/05_board_security.md` | 原为占位章节；本次资料直接影响板级安全设计 | done |
| `03_detailed_design/06_interface.md` | 需要同步 BMC/OOB/JTAG/DMA/mailbox/复位接口边界 | done |
| `03_detailed_design/10_full_design.md` | 总详设需要同步板级安全和外部访问控制口径 | done |
| `05_code_rules.md` | 新增约束会影响代码实现规则 | done |
| `06_traceability.md` | 需要补充 `SRC-005 管理子系统方案 -> C-BOARD-* -> 章节 -> 代码/测试` 追踪链路 | done |
| `03_detailed_design/04_lifecycle_debug.md` | JTAG scope 和 debug auth 与生命周期强相关 | done |
| `04_impl_design/mailbox_if.md` | OOB/JTAG/provisioning 代理命令可能需要字段级扩展 | check needed |
| `04_impl_design/spdm_report.md` | board/die binding、电源/复位状态、debug/JTAG 状态是否入报告待裁决 | check needed |
| `04_impl_design/manufacturing_provisioning.md` | MANU/ATE/SLT 阶段 JTAG 测试路径清理和锁定需要检查 | check needed |

## 6. 受影响实现文件

| Impl File | Why Impacted | Regenerate Needed |
|---|---|---|
| `04_impl_design/mailbox_if.md` | 可能需要 OOB proxy、JTAG auth proxy、power/reset status 命令或字段 | partial / pending field freeze |
| `04_impl_design/spdm_report.md` | 可能需要增加 board/die binding、debug/JTAG、电源/复位异常状态 | partial / pending policy |
| `04_impl_design/manufacturing_provisioning.md` | 需要描述 ATE/SLT/EVB JTAG 路径在 USER 前的锁定、清理和审计 | partial / pending process |
| `04_impl_design/efuse_key_fw_header_design.md` | 可能涉及 JTAG_FORCE_DISABLE、debug auth enable、board/die binding 控制位 | partial / pending bit allocation |
| `[TBD] firewall_access_rules` | 管理子系统 DMA、OOB bridge、JTAG MUX 需要 firewall/UserID 策略 | new impl theme needed |

## 7. 冻结风险

| Item | Risk Level | Needed Action |
|---|---|---|
| JTAG scope bitmap | high | 冻结 CPU/GPU/DRAM/Flash/安全子系统/板级 MCU/边界扫描 scope 映射 |
| JTAG MUX / CPLD 控制权 | high | 冻结由 SEC/eHSM 授权结果驱动的控制方式，确认不存在板级直通 |
| 管理子系统 DMA 白名单 | high | 冻结 UserID、firewall region、可访问 buffer、禁止访问区域 |
| BMC/OOB provisioning proxy | medium | 裁决是否允许 BMC/OOB 在 MANU 阶段承担 provisioning 代理 |
| 电源/复位/PowerBrake 安全状态 | medium | 裁决哪些事件进入 attestation report，哪些进入本地审计 |
| board/die binding | medium | 冻结首版是否参与镜像验证和证明报告 |

## 8. 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| Manifest freshness | pass | `SRC-005 管理子系统方案` 和 `CHG-005` 已登记 |
| Constraint freshness | pass | 新增 `C-BOARD-01` 至 `C-BOARD-04` |
| Baseline freshness | pass | 已新增 Board / OOB / 管理子系统边界 |
| Chapter freshness | pass | `04_lifecycle_debug.md`、`05_board_security.md`、`06_interface.md`、`10_full_design.md` 已更新 |
| Impl freshness | pending | 字段级输入不足，先标记为 check needed |
| Code rules freshness | pass | 已新增 `R-BOARD-*` |
| Traceability freshness | pass | 已新增 `T-BOARD-*` |
| Unsupported confirmed claims | pass | 对未冻结字段使用 `[TBD]` 或 pending 标记 |

---

## 9. CR-0003 增量影响记录

| 主题 | CR-0003 裁决 | 后续影响 |
|---|---|---|
| SEC2 image protection | `[CONFIRMED]` SEC2 正式安全启动路径必须 sign + encrypt | `VERIFY_IMAGE(SEC2)`、FW Header、FW_KEK、USER freeze、测试用例需按 mandatory decrypt 处理 |
| PM/RAS/Codec runtime policy | `[ASSUMED]` USER/PROD 默认 sign + encrypt；signature-only 为 `[TBD]` 白名单例外 | 产品安全策略需冻结 image_type/lifecycle/SKU/debug/release/rollback 条件 |
| Board binding | `[ASSUMED]` 默认进入 attestation，不默认阻断 SEC1 | 是否参与 SEC2/runtime release decision 仍需专项冻结 |
| JTAG scope / MUX | 策略已收敛：USER/PROD 默认关闭，打开需 lifecycle + auth + scope + timeout + audit | bit-level mapping、eHSM debug bitmap、MUX 寄存器归属仍阻塞实现冻结 |
| DMA / firewall / UserID | `[CONFIRMED]` 默认拒绝安全资源，只允许白名单 staging/data buffer | UserID、firewall region、地址范围、错误隐藏和审计字段仍待 RTL/实现冻结 |
| OOB/BMC proxy | `[ASSUMED]` 可作为 provisioning transport proxy；不得成为 trust anchor | 命令格式、认证、审计、失败回滚、rate limit / lockout 仍待冻结 |
| Attestation report | `[CONFIRMED]` measurement/lifecycle/debug/secure_boot/rollback 必须覆盖；image policy/decrypt/board 方向为 `[ASSUMED]` | 字段位置、PowerBrake/PG/FAULT/reset event 是否进入主 report 仍待冻结 |
| Manufacturing/RMA | USER freeze 覆盖 SEC1/SEC2 decrypt key/FW_KEK、debug、anti-rollback、test trust cleanup；RMA 禁止 long-open debug 和 decrypt bypass | Root injection mode、OTP/eFuse 验收、RMA re-acceptance 流程仍待冻结 |

---

## 10. CR-0004 增量影响记录

| 主题 | CR-0004 裁决 | 后续影响 |
|---|---|---|
| eHSM native image header | physical secure boot image header follow eHSM TRM；NGU `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t` 不再作为 wire/storage verification header | `01_boot.md`、`06_interface.md`、`10_full_design.md`、`efuse_key_fw_header_design.md`、`mailbox_if.md` 已同步；manifest ABI 仍 `[TBD]` |
| NGU protected manifest | NGU 项目级 `image_type`、policy、measurement、rollback domain、expected algorithm profile 放入受保护 manifest / policy table | 需要 eHSM owner 确认 manifest parser 边界和 ABI；当前不写死实现字段 |
| OTP / control field / Version Counter | physical OTP、algorithm control field、key ID / level / purpose、Version Counter follow eHSM TRM；NGU `OTP-0..OTP-7`、`*_MIN_VER` 仅作 logical alias | 新增 `ehsm_source_conformance_matrix.md`；exact key ID、control bit、counter 映射保持 `[TBD]` |
| SEC1 / SEC2 sign + encrypt | 加密镜像必须走 verify+decrypt output path；NVM only verify 不适用于 SEC1/SEC2 encrypted deploy | `VERIFY_SEC1 / VERIFY_IMAGE` 改为 NGU wrapper/profile，底层映射 eHSM Bootloader/Firmware command |
| Algorithm authority | `SocBootAlg / SocUpgradeAlg` 或等价 eHSM control field 是算法 authority；NGU profile 仅做审计/一致性检查 | `02_key_cert.md`、`10_full_design.md`、mailbox/provisioning 字段已调整 |
| Attestation measurement wording | `eHSM Image_Type` 与 NGU manifest `image_type` 分开进入 measurement 语义 | `spdm_report.md` 已避免把泛化 `image_type` 误写成 eHSM physical header 字段 |

## 10.1 CR-0004 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| Manifest freshness | pass | 已登记 `SRC-006/SRC-007` 与 CR-0004 conflict log |
| Constraint freshness | pass | 已新增 `C-BOOT-06/C-BOOT-07/C-EHSM-01`，并更新 `C-UPDATE-01` |
| Baseline freshness | pass | 已增加 eHSM source-conformance baseline 与 adopted/rejected 决策 |
| Chapter freshness | pass | Boot、Key/Cert、Interface、Manufacturing、Full Design 已同步 CR-0004 主口径 |
| Impl freshness | pass-with-TBD | `efuse_key_fw_header_design.md` 已重构；`mailbox_if.md`、`manufacturing_provisioning.md`、`spdm_report.md` 已同步；未冻结项保留 `[TBD]` |
| Code rules freshness | pass | 已新增/更新 `R-FW-*` eHSM 对齐规则 |
| Traceability freshness | pass | 已新增 `T-EHSM-*` trace rows 与 BLOCKED candidate |
| Unsupported confirmed claims | pass-with-open-items | exact key ID、control bit、manifest ABI、per-image CEK / wrapped CEK 未升级为 `[CONFIRMED]` |

---

## 11. CR-0005 增量影响记录

| 主题 | CR-0005 裁决 | 后续影响 |
|---|---|---|
| Full Design source of truth | `10_full_design.md` 是完整详设与代码落地主入口 | 后续 FW / driver / tool / test / review 先读 `10_full_design.md` |
| `04_impl_design` 角色 | `04_impl_design` 保留为 editing shard / extracted implementation shard，不再作为独立事实源 | 分片仍可局部维护，但必须同步主详设第 10 章 |
| 实现级内容合并 | `04_impl_design` 正文不做省略性删减，全量嵌入 `10_full_design.md` 第 10 章 | `10_full_design.md` 从章节级整合版升级为完整代码落地详设 |
| 流程同步 | README、Skill、prompts、templates、code rules、traceability、change impact 必须体现新的事实源关系 | 防止后续自动化或人工按旧流程生成两套 ABI 权威 |
| 安全裁决保持 | CR-0005 不改变 CR-0001 至 CR-0004 的安全架构结论，不关闭原 `[TBD]` | 本 CR 只改变文档组织和事实源优先级 |

## 11.1 CR-0005 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| CR gate | pass | 已新增 `CR-0005-single-full-design-code-landing-spec.md`，状态 accepted-for-application / applied-by-codex / owner-review-pending |
| Full design freshness | pass | `10_full_design.md` 已新增第 10 章“实现级落地详设全集”，嵌入 `04_impl_design` 分片正文 |
| Impl shard freshness | pass | `04_impl_design/README.md` 与各分片已增加 CR-0005 source-of-truth notice |
| Workflow docs freshness | pass | `README_使用说明.md`、Skill、prompts、templates 已同步主事实源规则 |
| Code rules freshness | pass | 已新增 `R-DOC-*` 文档事实源 / 代码落地规则 |
| Traceability freshness | pass | 已新增 `T-DOC-001` |
| Unsupported confirmed claims | pass | 本 CR 未新增安全字段冻结，未关闭 CR-0004 保留的 `[TBD]` |

---

## 12. CR-0006 增量影响记录

| 主题 | CR-0006 裁决 | 后续影响 |
|---|---|---|
| 固件包物理格式 | eHSM native secure boot image header 仍是唯一 physical verify/decrypt container | 不恢复 `common_firmware_header` / `signed_region_v1` 作为 wire/storage ABI |
| NGU manifest | NGU image type、policy、rollback domain、measurement slot、expected algorithm profile 等进入 protected manifest | manifest ABI 仍需 owner 后续冻结；eHSM PASS 前不得信任 manifest |
| 平台侧制作流程 | image packager 生成 `manifest + payload` 的 Code region，并通过 eHSM native packaging / owner-confirmed flow 输出正式包 | 需要后续工具侧冻结 CLI、source-conformance report、golden vector |
| 设备侧 verify/decrypt | BootROM/SEC 只定位包和准备 output buffer；eHSM 完成 native header check、signature、rollback、decrypt output；BootROM/SEC 再解析 manifest 和 release policy | BootROM/SEC 不得实现复杂 crypto，不得在 eHSM PASS 前使用 manifest |
| 旧 current_plan 第 7 章 | 保留流程表达价值，但旧 `header + Signed Region + signature + wrapped_cek + enc_payload` 只作历史流程意图参考 | 文档中必须避免把 wrapped CEK / custom header 当作已冻结 physical ABI |
| 图形化表达 | boot/full/impl 已补充固件包布局图、平台侧制作流程图、设备侧 verify/decrypt 时序图 | 提升评审和代码落地可读性 |

## 12.1 CR-0006 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| CR gate | pass | 已新增 `CR-0006-firmware-package-build-verify-flow.md`，状态 accepted-for-application / applied-by-codex / owner-review-pending |
| Manifest freshness | pass | 已登记 `CHG-008` |
| Constraint freshness | pass | 已新增 `C-BOOT-08` |
| Baseline freshness | pass | 已新增 Firmware Package Build/Verify Contract baseline |
| Chapter freshness | pass | `01_boot.md` 和 `10_full_design.md` 第 3 章已补 layout/build/verify 图和流程 |
| Impl freshness | pass-with-TBD | `efuse_key_fw_header_design.md` 与 `10_full_design.md` 第 10 章已补实现级流程；manifest ABI / exact key ID / golden vector 保持 `[TBD]` |
| Code rules freshness | pass | 已新增 `R-FW-009` 至 `R-FW-012` |
| Traceability freshness | pass | 已新增 `T-FW-PKG-001` |
| Open questions freshness | pass | 已新增 `OQ-0016` |
| Unsupported confirmed claims | pass-with-open-items | 未把 per-image CEK / wrapped CEK、manifest ABI、exact key ID、exact command ABI 升级为 `[CONFIRMED]` |

---

## 13. CR-0014 增量影响记录

| 主题 | CR-0014 裁决 | 后续影响 |
|---|---|---|
| 当前方案源 | `SRC-008 当前收敛安全软件方案 2.0` 作为当前安全软件方案基线；旧 `SRC-001` 降级为历史流程参考 | 后续 constraints、baseline、full design、code rules、traceability 和导出版方案如无特殊说明均按 `SRC-008` 解释 |
| 固件包流程来源 | eHSM native package、FMC 制作和设备侧验证流程以 `SRC-008` 第 4 章 / 第 4.5 节为当前来源 | `C-BOOT-08`、`01_boot.md`、`10_full_design.md`、`efuse_key_fw_header_design.md`、`T-FW-PKG-001` 已同步 |
| 单 FMC + OOB 恢复 | `SRC-008` 与 CR-0013 均确认首版单 FMC 固定分区，OOB MCU 受控重刷，不再采用 FMC A/B fallback | `05_code_rules.md` 已替换旧 CR-0008 A/B 和 key rotation 绑定规则 |
| 未冻结项 | `SRC-008` 不自动冻结 bit-level ABI、exact key ID、exact OTP/control bit、OOB/QSPI register、工具 CLI/golden vector | open questions 保持不变，字段级实现仍需 owner 后续冻结 |

## 13.1 CR-0014 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| CR gate | pass | 已新增 `CR-0014-current-plan-v2-source-of-truth-sync.md`，状态 applied / review pending |
| Manifest freshness | pass | 已新增 `SRC-008`、`CF-008`、`CHG-010`，并将 `SRC-001` 降级为 historical-reference |
| Constraint freshness | pass | 已新增 `C-SRC-01`，更新 `C-BOOT-08` 和 `C-UPDATE-02` 来源 |
| Baseline freshness | pass | 已增加 current plan source baseline |
| Chapter freshness | pass | `01_boot.md` 已移除 SEC2/runtime A/B 建议残留；`10_full_design.md` 已登记 `SRC-008` |
| Impl freshness | pass | `efuse_key_fw_header_design.md` 来源已改为 `SRC-008`；未改动 eHSM physical ABI |
| Code rules freshness | pass | `R-FW-017` 至 `R-FW-020` 已同步单 FMC + OOB 恢复和 OOB 不放行规则 |
| Traceability freshness | pass | 已新增 `T-SRC-001`，更新 FW package / OOB recovery trace source |
| Unsupported confirmed claims | pass-with-open-items | 未关闭 manifest ABI、exact eHSM key ID、OOB/QSPI ABI、tool CLI/golden vector 等开放项 |

---

## 14. CR-0018 增量影响记录

| 主题 | CR-0018 裁决 | 后续影响 |
|---|---|---|
| OSR eHSM 软件事实源 | `SRC-009 OSR eHSM 软件代码包 4019` 是 eHSM 已提供安全服务、mailbox command/req/rsp、Host API、secure boot verify/upgrade、OTP/key/counter、debug/lifecycle、image/OTP tool 行为的实现事实源 | 后续 eHSM adapter、BootROM/SEC verify flow、mailbox driver、image packager、provisioning/debug/lifecycle tool 和测试向量必须从 OSR 代码出发做 source-conformance |
| 适配原则 | 安全方案需要适配 OSR 代码；若方案需要 OSR 代码未提供或语义不同的服务，先登记差异，再通过 wrapper、policy 限制、eHSM customization 或方案调整 CR 处理 | 不允许在 NGU 文档中直接发明并列 eHSM ABI；但不改变 Root of Trust、Host 不可信、BootROM 最小化、SEC1/SEC2 sign+encrypt 等已冻结原则 |
| ROM Patch 机制 | `SRC-010 eHSM4.0 ROM Patch 方案` 按 OTP patch 表 + 硬件 BOOT 加载 + CPU/IROM 无感指令替换建模 | 需要进入 OTP 空间、制造烧录、USER 锁定、patch 验收、审计和 attestation/report 影响分析 |
| 字段级冻结 | OSR command field、error code、OTP offset、key ID、control bit、patch layout、tool CLI、golden vector 仍保持 `[TBD]` | `10_full_design.md` 与 `04_impl_design/*.md` 需要后续专项 CR 做逐项适配 |

## 14.1 CR-0018 方案更新需求判断

| Area | Need Update | Reason | Priority |
|---|---|---|---|
| `security_inputs/inputs_manifest.md` | done | 已登记 `SRC-009/SRC-010`、冲突和变更入口 | high |
| `security_workflow/01_constraints.md` | done | 已新增 `C-SRC-02`、`C-EHSM-02`、`C-EHSM-03` | high |
| `security_workflow/02_baseline.md` | done | 已增加 OSR 软件事实源、ROM patch baseline、freeze-sensitive items | high |
| `security_workflow/05_code_rules.md` | done | 已新增 OSR source-conformance、ROM patch 禁止运行期热补丁、patch 制造规则 | high |
| `security_workflow/06_traceability.md` | done | 已新增 `T-SRC-002`、`T-SRC-003`、`T-EHSM-004`、`T-EHSM-PATCH-001` | high |
| `security_workflow/03_detailed_design/10_full_design.md` | needed | 需要把 OSR 代码事实源同步到 boot、interface、key/cert、manufacturing、attestation/report 章节和第 10 章实现级落地主规格 | high |
| `security_workflow/04_impl_design/*.md` | needed | `ehsm_source_conformance_matrix.md`、`mailbox_if.md`、`efuse_key_fw_header_design.md`、`manufacturing_provisioning.md`、`spdm_report.md` 需要逐项对齐 OSR command/field/error/tool 行为和 ROM patch 字段 | high |
| Code / tests | not-yet | 当前只完成方案输入和约束同步；代码实现前应先完成 OSR source-conformance matrix 与 golden vector | high |

## 14.2 CR-0018 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| CR gate | pass | 已新增 `CR-0018-osr-ehsm-software-and-rom-patch-source-sync.md`，状态 applied / owner-review-pending |
| Manifest freshness | pass | 已新增 `SRC-009/SRC-010`、`CF-009/CF-010`、`CHG-011` |
| Constraint freshness | pass | 已新增 OSR 软件事实源、source-conformance gate 和 ROM patch 机制约束 |
| Baseline freshness | pass | 已新增 eHSM software source、ROM patch source、source precedence 和冻结敏感项 |
| Chapter freshness | pending | `10_full_design.md` 与章节级详设尚未执行字段级适配 |
| Impl freshness | pending | `04_impl_design` 仍需后续专项对照 OSR 代码和 patch PDF |
| Code rules freshness | pass | 已新增 `R-FW-021` 至 `R-FW-024`、`R-MFG-008`、`R-DOC-006`、`R-DOC-007` |
| Traceability freshness | pass | 已新增 OSR source-conformance 和 ROM patch trace rows |
| Open questions freshness | pass | 已新增 OSR 代码差异清单与 ROM patch 字段级集成开放问题 |
| Unsupported confirmed claims | pass-with-open-items | 未把 OSR 代码中观察到的字段、offset、key ID、工具 CLI 或 patch OTP layout 自动升级为字段级 `[CONFIRMED]` |

---

## 15. CR-0019 增量影响记录

| 主题 | CR-0019 裁决 | 后续影响 |
|---|---|---|
| Vendor 代码变更控制 | 修改 `ehsm_bootrom/**`、`ehsm_firmware/**` 前必须先提醒用户，并说明原因、改动点、影响、回退和验证 | 后续开发不得静默修改 OSR/vendor 代码；需要把变更原因和验证结果写入相关文档 |
| 工具链变更控制 | 修改 Wing 工具链安装、权限、软链接、动态库、PATH/LD_LIBRARY_PATH 或工具链相关脚本前必须执行同等提醒和记录 | `/opt/wing_tool`、`~/.bashrc`、`WING_TOOL_BIN` 等不再作为可静默调整项；任何 workaround 必须标明本地性质 |
| OpenSpec 约束 | FSP OpenSpec 增加 vendor 变更控制上下文和 artifact rules | 后续 proposal/design/spec/tasks 生成时必须默认携带该强约束 |
| 安全方案同步 | constraints/code rules/traceability 新增 vendor 变更控制项 | 代码评审和后续 Codex 任务必须按 `C-SRC-03` / `R-DOC-008` 执行 |

## 15.1 CR-0019 本轮一致性检查

| Check Item | Result | Notes |
|---|---|---|
| CR gate | pass | 已新增 `CR-0019-vendor-code-toolchain-change-control.md`，状态 accepted-for-application / applied-by-codex / owner-review-pending |
| OpenSpec freshness | pass | `fsp/development_constraints/openspec/config.yaml` 与 `fsp/development_constraints/openspec/development_principles.md` 已写入强约束 |
| Development constraints layout | pass | FSP 根目录旧约束入口已归口到 `fsp/development_constraints/`，并新增目录 README |
| Manifest freshness | pass | 已新增 `CHG-012` |
| Constraint freshness | pass | 已新增 `C-SRC-03` |
| Code rules freshness | pass | 已新增 `R-DOC-008`，并将优先规则范围更新为 `R-DOC-001 ~ R-DOC-008` |
| Traceability freshness | pass | 已新增 `T-SRC-004` |
| Vendor source impact | pass | 本 CR 未修改 `ehsm_bootrom/**`、`ehsm_firmware/**` 源码或 Wing 工具链状态 |
| Unsupported confirmed claims | pass | 本 CR 只冻结变更控制流程，不新增 eHSM ABI、OTP/key/counter 字段或工具 CLI 事实 |
