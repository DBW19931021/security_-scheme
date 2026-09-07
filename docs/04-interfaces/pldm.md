---
title: "PLDM 接口"
status: deferred_profile
evidence_state: UNKNOWN
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
owners:
  - GSP
last_reviewed: 2026-07-24
supersedes: []
superseded_by: []
---

# Purpose

记录PLDM是否作为正常更新transport；它不是已批准的安全语义Owner。

# Boundary

无论最终采用PLDM还是其他Host协议，安全边界均由主详设第12章的typed update transaction、真实package验证、inactive slot、原子metadata和下一次安全启动决定。Transport不得传递raw eHSM/OTP命令或绕过GSP。

# Open questions

ADR-0020已关闭`OPEN-DESIGN-016`的产品原则，但未指定正常更新transport。是否采用PLDM、命令子集、消息大小、认证/anti-replay和错误映射作为平台Profile输入；输入到齐前不生成产品PLDM ABI或Expected。

# Change history

- 2026-07-24：明确PLDM仅为待定transport。
