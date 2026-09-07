---
title: "恢复机制"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
owners:
  - GSP
last_reviewed: 2026-07-27
supersedes: []
superseded_by: []
---

# Purpose

维护Recovery/OOB后的可信恢复边界；规范性设计见[主详设第12章](../05-software-design/NGU800P安全软件详细设计.md)。

# Baseline

Recovery只允许查询非敏感状态、清理未提交候选、接收批准package和请求reset；不提供raw memory/Flash/eHSM或任意镜像选择。OOB只写固定FMC候选分区；下次BootROM/eHSM是真实最终信任判定。Counter已推进时禁止回退到较低`rollback_counter`镜像。

# Approved boundary and inputs

ADR-0020已关闭`OPEN-DESIGN-003/016`并冻结受限恢复、RAS职责和不可逆恢复策略；token、partition和RAS数值作为平台输入。

# Verification impact

覆盖无有效slot、unknown metadata、counter前移、新镜像损坏、OOB写成功但BootROM拒绝和重放/越界。

# Change history

- 2026-07-27：接受ADR-0020；关闭OPEN-DESIGN-003/016，冻结失败终态和受限恢复边界。
- 2026-07-24：同步主详设第12章。
