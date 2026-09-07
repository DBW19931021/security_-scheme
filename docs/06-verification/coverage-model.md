---
title: "覆盖模型"
status: review_ready_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0019
  - SRC-0023
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-08-25
supersedes: []
superseded_by: []
---

# Purpose

定义流片前安全覆盖模型；完整门禁见[主详设第16章](../05-software-design/NGU800P安全软件详细设计.md)。

# Coverage dimensions

每个Feature至少按stage、`non_sec_boot`/LCS/Strap、算法Profile、image/Die、正常/负例、reset/掉电、权限/隔离、错误/RAS、cache/并发和环境覆盖。`non_sec_boot`必须覆盖值0兼容、值1覆盖、读取/ECC/镜像异常、0→1烧写和复位重锁存。层级为L0静态工具、L1 host unit、L2 baremetal、L3 EMU、L4 FPGA/样片。

追溯必须闭合`Source -> Requirement/ADR -> Design -> Code -> Case -> Evidence -> Release`。环境缺能力记录`BLOCKED_BY_ENV`，不能计为PASS。

# Open questions

ADR-0020已关闭`OPEN-DESIGN-020`并冻结Feature闭环、高严重度缺陷关闭、BLOCKED、waiver和签署原则；具体覆盖阈值、Owner/签署名单在执行前补齐。`OPEN-EMU-001`继续冻结环境能力/版本。

# Change history

- 2026-07-24：同步主详设第16章。
