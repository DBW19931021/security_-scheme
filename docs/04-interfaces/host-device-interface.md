---
title: "Host/Device 接口"
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

定义Host可见控制面与安全边界。

# Baseline

Host可以按批准协议下发固件候选、查询非敏感状态和发起受控Debug/更新请求；不能直接访问明文firmware、执行RAM、Measurement写、OTP/key slot、raw eHSM Mailbox、Firewall或release寄存器。GSP只接受typed request，验证caller、长度、nonce/sequence、LCS和ACL。

ADR-0019已确认不向Host开放GSP通用算法、Key、Certificate、Rotation、raw eHSM或其他安全服务；不得注册相应路由或提供raw Vendor command passthrough。

# Open questions

`OPEN-DESIGN-006`已关闭；`OPEN-DESIGN-015/016`分别管理SPDM和更新等独立产品协议，它们不构成GSP通用安全服务开放。

# Change history

- 2026-07-24：同步主详设第8、10～12章。
