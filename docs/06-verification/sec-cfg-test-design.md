---
title: "NGU800P SEC_CFG测试设计"
status: proposed_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0018
  - SRC-0022
  - SRC-0025
  - SRC-0026
  - SRC-0027
owners:
  - GSP
  - RTL_DV
last_reviewed: 2026-08-13
supersedes: []
superseded_by: []
---

# Purpose

把SEC_CFG寄存器、启动状态、错误、LCS、UID、Debug及Firewall边界展开为可执行测试主题。所有条目当前为`NOT_EXECUTED`，不能视为已通过。

# Top-level case scope

统一工作簿v0.3.8只保留两条SEC_CFG顶层软件/协同Case：

- `SOC-SECCFG-001`：单次遍历并比较全部17个寄存器，不按寄存器拆Case；
- `SOC-SECCFG-DBG-001`：只验证`dbg_en_cfg`默认关闭、主Die写入及从Die实际Debug通道关闭→联通→关闭。

`soc_dbg_en_out`不作为`SOC-SECCFG-DBG-001`的功能判据，而是加入Mailbox Debug Auth成功/失败和`CLOSE_DEBUG`用例。下表的12个`SECCFG-*`主题保留为底层模型/DV追溯点，不是12个新增顶层Codex Case。

# Oracle priority

1. 与目标D0 build匹配的RTL/CSR生成源和EMU/FPGA/硅readback Evidence。
2. 关闭后的OPEN-CONFLICT-015和OPEN-CONFLICT-014。
3. SRC-0026中不冲突的寄存器offset、bit和逻辑说明。
4. 软件建议只作为PROPOSED检查方法，不替代硬件原子性或清除语义。

# Case design

| Test ID | 场景 | 关键检查 | 当前状态 |
|---|---|---|---|
| SECCFG-REG-001 | 地址与基本读 | 目标base/size、`0x000..0x040`无错位、Firewall授权 | BLOCKED |
| SECCFG-STATUS-001 | 启动阶段 | 每阶段error优先、done和timeout、raw 64-bit采集 | READY_FOR_MODEL |
| SECCFG-HWERR-001 | ECC/TRNG/bus故障注入 | 对应level、raw值、IRQ/源清除/重触发 | READY_FOR_DV |
| SECCFG-FWERR-001 | BL/FW故障注入 | raw 64-bit和匹配build；未知bit保持INCONCLUSIVE | BLOCKED |
| SECCFG-LCS-001 | 生命周期切换/样片 | `lcs`与status lifecycle一致、非法值fail-close | BLOCKED |
| SECCFG-UID-001 | UID读取 | 160-bit稳定、word/byte order、provisioning一致 | BLOCKED |
| SECCFG-DBG-001 | TEST/DEV/MANU/USER Debug | cfg、128-bit输出、实际gate三者一致 | BLOCKED |
| SECCFG-FW-001 | 未授权UserId读写 | Slave无访问；Firewall错误地址/info/status正确 | BLOCKED |
| SECCFG-RO-001 | 对RO offset发起写 | 状态不被软件修改；APB响应符合集成规格 | READY_FOR_DV |
| SECCFG-RESET-001 | reset assert/release | reset值、动态重新采样、可见时序 | READY_FOR_DV |
| SECCFG-ATOMIC-001 | 64-bit跨word同时变化 | high-low-high结果与RTL snapshot合同对照 | BLOCKED |
| SECCFG-ORDER-001 | 连续多阶段变化 | 不以非零/hw_done误判后续阶段ready | READY_FOR_MODEL |

# Detailed checks

## Status

对hardware boot、bootloader、firmware和SoC verify分别构造done-only、error-only、error+done、超时和正常顺序。循环必须先判error再判done；任一错误或timeout保存三组raw值，不能synthetic ready判PASS。

## Hardware and firmware errors

严重错误组合中断和ECC 1-bit独立中断由`EDA-ERR-CRITICAL-IRQ-001`与`EDA-ERR-ECC1B-IRQ-001`覆盖。普通软件只读取、记录和执行批准的清除，不增加eHSM注错命令。FW error bit定义缺失时只验证raw传播和build关联，结果保持`INCONCLUSIVE`而不是猜bit。

## LCS and UID

LCS覆盖TEST/DEV/MANU/USER/DEBUG/DESTROY及非法样式，并同时采集status lifecycle bits。UID覆盖reset前后、有效时刻、连续读取稳定性、五word拼接、CPU byte序和量产数据库对比。

## Debug

`dbg_en_cfg`与`soc_dbg_en_out`必须分开：前者由主Die控制从Die Debug，必须以实际从Die Debug通道为判据；后者由eHSM Mailbox Debug鉴权驱动，必须在鉴权成功、鉴权失败和`CLOSE_DEBUG`时与实际Debug访问同步检查。没有连接矩阵时保持`INCONCLUSIVE`。

## Firewall and RO writes

对14个有效UserId及reserved UserId执行授权/拒绝。hide模式必须同时证明Slave无副作用和错误快照正确。RO写仅在DV受控环境执行，生产软件禁止主动探测。

# Evidence package

每项Evidence至少记录DUT/RTL/eHSM BL-FW/BootROM-GSP build、平台、UserId、LCS、reset类型、原始寄存器dump、Firewall dump、注入方法、期望来源、日志和结论。环境无能力时标记`BLOCKED`，不可标记PASS。
