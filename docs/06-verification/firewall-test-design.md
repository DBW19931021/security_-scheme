---
title: "NGU800P Firewall 测试设计"
status: proposed_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0022
  - SRC-0025
  - SRC-0027
owners:
  - GSP
  - RTL_DV
last_reviewed: 2026-08-13
supersedes: []
superseded_by: []
---

# Purpose

49项细化条目继续作为Firewall模型和RTL/DV参考，但不进入本轮SoC顶层Case或Codex移植数量。本轮只验证安全方案直接使用的4项功能，不以测试Firewall IP为目标：

- `SOC-FW-SECCFG-001`：不改配置，默认仅启动核可读写，eHSM和其他核均无权限；
- `SOC-FW-SPIFC-001`：不改配置，默认仅启动核可读写，eHSM和其他核均无权限；
- `SOC-FW-SRAM-001`：默认权限，单Case内部遍历Region0～Region4；
- `SOC-FW-SRAM-RECFG-001`：验证eHSM/其他核对三个Firewall的配置写被拒绝，并由启动核修改SRAM地址范围和允许Master ID，单Case内部遍历Region0～Region4。

三个Firewall均只有启动核具备配置权限，eHSM和其他核的配置写必须被阻断且配置不变。SEC_CFG/SPIFC Firewall不执行成功重新配置；两个SRAM Case都必须覆盖全部5个Region，但不得按Region拆成独立Case。

由于SEC_CFG/SPIFC不允许在本轮Case中执行成功重新配置，软件只对其验证非启动发起方配置写被阻断和配置读回不变；启动核正向重新配置闭环只在SRAM五Region完成，SEC_CFG/SPIFC的启动核Owner绑定由RTL/CSR集成证据确认。

# Inputs

- [Firewall架构与UserId](../03-architecture/firewall-isolation.md)
- [Firewall需求](../../requirements/firewall-requirements.yaml)
- [UserId机读映射](../../requirements/firewall-userid-authority-map.yaml)
- [测试条目CSV](../../tests/cases/firewall/NGU800P-firewall-detailed-cases-v0.1.csv)
- OPEN-CONFLICT-014：base/reset/Region/R4/bit27/高地址/安全属性/lock/burst

# Oracle priority

1. 与目标D0/tape-out revision匹配的RTL/CSR生成源和DV Evidence。
2. 已批准的需求、ADR、主详设与关闭后的OPEN-CONFLICT-014。
3. SRC-0025中不冲突的`DOCUMENTED`逻辑和UserId/authority位图。
4. 冲突中的截图默认值只做observed-value记录，不做最终PASS/FAIL。

# Coverage model

下述交叉覆盖只保留为49项专项细化参考，不自动派生顶层软件Case：instance（SRAM/sec_cfg/spifc）、`check_en`、`hide_en`、read/write、secure/non-secure、UserId、authority、Region边界、Slave reached/blocked和错误诊断。顶层执行以四条方案级Case为准。

## UserId数据驱动

- 对14个有效UserId分别执行只写、只读、读写和无权限组合；确认读写位不串扰。
- 对`0x0/0x8`验证软件拒绝授权且不发生无符号减一。
- authority bits[31:28]为1时policy验证必须拒绝；硬件如何响应须等待RTL绑定。
- R4 `0x00000F00`在名称冲突关闭前只按原始位图记录，不把“MM/Codec”名称作为oracle。

# Required observations

每个负向访问至少保存：发起Master/UserId、地址、方向、secure标记、TNUI response、Slave入口valid/accept或副作用、Firewall配置readback、错误low32/high16、R/W、UserId、Region状态以及clear前后状态。Hide=1时必须证明“TNUI OK且Slave未收到请求”。

# Groups and status

| Group | Case IDs | Count | Current status |
|---|---|---:|---|
| 控制与响应 | `FW-TC-001..005` | 5 | READY |
| Region地址 | `FW-TC-010..018` | 9 | 8 READY / 1 BLOCKED |
| 权限和UserId | `FW-TC-020..032` | 13 | 12 READY / 1 BLOCKED |
| 错误诊断 | `FW-TC-040..044` | 5 | READY |
| sec_cfg/spifc | `FW-TC-050..053` | 4 | READY |
| 安全属性 | `FW-TC-060..063` | 4 | BLOCKED |
| 软件policy/fail-close | `FW-TC-070..075` | 6 | READY |
| Burst | `FW-TC-080..082` | 3 | BLOCKED |
| **合计** |  | **49** | **40 READY / 9 BLOCKED / NOT_EXECUTED** |

# Software and RTL/DV split

2026-08-20在线最终表中，软件/EMU的顶层Case只覆盖SEC_CFG/SPIFC默认数据权限、SRAM五Region默认数据权限，以及启动核对SRAM五Region的成功配置/readback和授权/未授权数据闭环。历史`EDA-FW-EHSM-MASTER-001`已从最终表删除。产品仍规定三个Firewall仅启动核可配置，但eHSM和其他核配置写拒绝尚未在在线Case中闭环，必须由Case Owner补入批准Case或建立RTL-DV/集成补充Evidence；不得从Case 060的PASS自动推导。除非安全方案新增需求，本轮不把burst、Hide、内部唯一命中等Firewall IP细节扩展为顶层Case。

# Execution gate

- 当前无产品代码修改授权；本文和工作簿只形成测试设计，不代表已执行。
- base/CSR字段未进入SRC-0022前，涉及真实MMIO的用例返回`INCONCLUSIVE/BLOCKED_BY_RTL_BINDING`，不能使用猜测地址。
- 所有执行必须记录RTL、软件、workbook、policy和平台版本；未观测到错误、构建成功或TNUI返回OK均不能单独形成PASS。

# References

- [统一安全测试用例设计与Codex移植指导](NGU800P安全测试用例设计与Codex移植指导.md)
- [软件功能case与硬件补充验证项](NGU800P软件不便覆盖的硬件安全验证项.md)
- [OPEN-CONFLICT-014](../../sources/conflict-reports/CONFLICT-SRC-0025-FIREWALL-DEFAULTS-AND-BINDING.md)
