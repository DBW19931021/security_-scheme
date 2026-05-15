# NGU800 安全方案变更影响分析

## 1. 新增 / 更新输入摘要

| Change ID | New / Updated Source | Summary | Priority |
|---|---|---|---|
| CHG-005 | `SRC-005 管理子系统方案` / `security_inputs/board/管理子系统.pdf` | 新增管理子系统方案；总体架构、模块职责、带外链路、电源/复位流程、单/双 Die 约束作为系统级输入采用；涉及安全且考虑不足、明显存在漏洞或与既定安全基线冲突的内容不直接继承 | medium |

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

## 3. 受影响约束

| Constraint ID | Previous Statement | Updated Statement | Impact Level |
|---|---|---|---|
| `C-BOARD-01` | 不存在 | 管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决 | high |
| `C-BOARD-02` | 不存在 | 带外管理通道不得成为安全策略绕过路径 | high |
| `C-BOARD-03` | 不存在 | JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制 | high |
| `C-BOARD-04` | 不存在 | 管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须被隔离和审计 | high |
| `C-ACCESS-01` / `C-ACCESS-02` | 已要求安全子系统隔离、UserID + Firewall | 适用范围扩展到管理子系统 DMA、OOB 桥接、JTAG MUX、板级复位相关访问路径 | medium |
| `C-DEBUG-01` / `C-DEBUG-02` | 已要求 USER 关闭调试、DEBUG/RMA 必须认证 | 适用范围扩展到板级 JTAG、CPLD/MUX、边界扫描和板级 MCU 调试路径 | medium |

## 4. 受影响 Baseline

| Baseline Area | Impact | Updated Decision |
|---|---|---|
| Board / OOB trust model | `SRC-005 管理子系统方案` 增加管理子系统整体架构和带外链路 | BMC/OOB/板级 MCU/管理子系统不进入 Root of Trust，只作为受控链路或代理 |
| JTAG / debug model | `SRC-005 管理子系统方案` 描述 JTAG 具有访问 GPU/CPU/DRAM/Flash/安全子系统能力 | JTAG 不按默认开放能力采用，必须 lifecycle + debug auth + scope + MUX gating |
| DMA / mailbox / interrupt | `SRC-005 管理子系统方案` 描述管理子系统 DMA、mailbox、中断和互斥机制 | 这些机制只可用于受控协作，不得绕过 SEC/eHSM 和 firewall |
| Power / reset control | `SRC-005 管理子系统方案` 描述板级电源、上下电和复位管理 | 影响安全状态的事件必须进入安全状态机或审计 |

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
| NGU protected manifest | NGU 项目级 `ngu_image_type`、policy、measurement、rollback domain、expected algorithm profile 放入受保护 manifest / policy table | 需要 eHSM owner 确认 manifest parser 边界和 ABI；当前不写死实现字段 |
| OTP / control field / Version Counter | physical OTP、algorithm control field、key ID / level / purpose、Version Counter follow eHSM TRM；NGU `OTP-0..OTP-7`、`*_MIN_VER` 仅作 logical alias | 新增 `ehsm_source_conformance_matrix.md`；exact key ID、control bit、counter 映射保持 `[TBD]` |
| SEC1 / SEC2 sign + encrypt | 加密镜像必须走 verify+decrypt output path；NVM only verify 不适用于 SEC1/SEC2 encrypted deploy | `VERIFY_SEC1 / VERIFY_IMAGE` 改为 NGU wrapper/profile，底层映射 eHSM Bootloader/Firmware command |
| Algorithm authority | `SocBootAlg / SocUpgradeAlg` 或等价 eHSM control field 是算法 authority；NGU profile 仅做审计/一致性检查 | `02_key_cert.md`、`10_full_design.md`、mailbox/provisioning 字段已调整 |
| Attestation measurement wording | `eHSM Image_Type` 与 NGU `ngu_image_type` 分开进入 measurement 语义 | `spdm_report.md` 已避免把泛化 `image_type` 误写成 eHSM physical header 字段 |

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
