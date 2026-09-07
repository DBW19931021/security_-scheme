---
title: "OOB 接口"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
owners:
  - GSP
last_reviewed: 2026-07-27
supersedes: []
superseded_by: []
---

# Purpose

定义OOB MCU更新FMC候选分区的外层接口；主合同见[详设第12章](../05-software-design/NGU800P安全软件详细设计.md)。

# Baseline

OOB请求绑定固定target partition、length、外部release version、`rollback_counter`、hash、nonce/sequence和授权token。OOB MCU验证外层请求、写固定FMC候选分区并读回；不写LCS/counter/key/Measurement，也不能宣布固件可信。下一次SoC reset后BootROM/eHSM完成最终验签、解密、Header Overlay/typed-stage policy、rollback-counter和Measurement。

# Approved boundary and inputs

ADR-0020已关闭`OPEN-DESIGN-016`并冻结OOB权限边界；token/anti-replay、partition、写Owner、reset请求和审计数值作为平台Profile输入。

# Change history

- 2026-07-27：接受ADR-0020并关闭OPEN-DESIGN-016；同步`version/rollback_counter`命名和OOB固定候选分区原则。
- 2026-07-24：同步主详设第12章。
