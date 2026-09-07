---
title: "日志与审计"
status: review_ready_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
owners:
  - GSP
last_reviewed: 2026-07-24
supersedes: []
superseded_by: []
---

# Purpose

维护日志、审计和脱敏专题；规范性设计见[主详设第14章](NGU800P安全软件详细设计.md)。

# Baseline

运行日志和安全审计分离。审计覆盖启动、LCS、Debug、key/cert、counter、update/OOB、Multi-Die和RAS；记录事件/主体/对象/结果/generation/sequence，不记录key、plaintext、完整token、session secret或raw Mailbox packet。USER日志最小化。高权限事务在要求审计的场景中，审计不可用应在不可逆点前拒绝。

# Open questions

ADR-0020已关闭`OPEN-DESIGN-018`并冻结审计与敏感日志原则；审计存储、容量、integrity、sequence/time、访问和满载数值由平台/EMU Profile补齐。

# Change history

- 2026-07-24：同步主详设第14章。
