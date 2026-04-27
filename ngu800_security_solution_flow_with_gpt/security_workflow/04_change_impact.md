# NGU800 安全方案变更影响分析

## 1. 新增 / 更新输入摘要

| Change ID | New / Updated Source | Summary | Priority |
|---|---|---|---|
| CHG-005 | `SRC-005` / `security_inputs/board/管理子系统.pdf` | 新增管理子系统方案；总体架构、模块职责、带外链路、电源/复位流程、单/双 Die 约束作为系统级输入采用；涉及安全且考虑不足、明显存在漏洞或与既定安全基线冲突的内容不直接继承 | medium |

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
| Board / OOB trust model | `SRC-005` 增加管理子系统整体架构和带外链路 | BMC/OOB/板级 MCU/管理子系统不进入 Root of Trust，只作为受控链路或代理 |
| JTAG / debug model | `SRC-005` 描述 JTAG 具有访问 GPU/CPU/DRAM/Flash/安全子系统能力 | JTAG 不按默认开放能力采用，必须 lifecycle + debug auth + scope + MUX gating |
| DMA / mailbox / interrupt | `SRC-005` 描述管理子系统 DMA、mailbox、中断和互斥机制 | 这些机制只可用于受控协作，不得绕过 SEC/eHSM 和 firewall |
| Power / reset control | `SRC-005` 描述板级电源、上下电和复位管理 | 影响安全状态的事件必须进入安全状态机或审计 |

## 5. 受影响章节

| Chapter | Why Impacted | Regenerate Needed |
|---|---|---|
| `01_constraints.md` | 新增管理子系统、OOB、JTAG、DMA、复位相关安全约束 | done |
| `02_baseline.md` | 增加 Board / OOB / 管理子系统边界和 adopted/rejected 裁决 | done |
| `03_detailed_design/05_board_security.md` | 原为占位章节；本次资料直接影响板级安全设计 | done |
| `03_detailed_design/06_interface.md` | 需要同步 BMC/OOB/JTAG/DMA/mailbox/复位接口边界 | done |
| `03_detailed_design/10_full_design.md` | 总详设需要同步板级安全和外部访问控制口径 | done |
| `05_code_rules.md` | 新增约束会影响代码实现规则 | done |
| `06_traceability.md` | 需要补充 `SRC-005 -> C-BOARD-* -> 章节 -> 代码/测试` 追踪链路 | done |
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
| Manifest freshness | pass | `SRC-005` 和 `CHG-005` 已登记 |
| Constraint freshness | pass | 新增 `C-BOARD-01` 至 `C-BOARD-04` |
| Baseline freshness | pass | 已新增 Board / OOB / 管理子系统边界 |
| Chapter freshness | pass | `04_lifecycle_debug.md`、`05_board_security.md`、`06_interface.md`、`10_full_design.md` 已更新 |
| Impl freshness | pending | 字段级输入不足，先标记为 check needed |
| Code rules freshness | pass | 已新增 `R-BOARD-*` |
| Traceability freshness | pass | 已新增 `T-BOARD-*` |
| Unsupported confirmed claims | pass | 对未冻结字段使用 `[TBD]` 或 pending 标记 |
