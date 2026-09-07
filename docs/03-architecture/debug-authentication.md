---
title: "调试鉴权"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
owners:
  - GSP
last_reviewed: 2026-07-28
supersedes: []
superseded_by: []
---

# Purpose

维护Debug/RMA鉴权专题；规范性状态机见[主详设第10章](../05-software-design/NGU800P安全软件详细设计.md)。

# Baseline

流程固定为GSP本地检查、eHSM `GET_CHALLENGE`、设备绑定签名token、eHSM `DEBUG_AUTH`、成功后打开唯一SoC全局Debug enable，并在本地最大开放时长、reset、LCS变化、安全错误或显式close时关闭。

NGU800P没有Debug scope、Die/core bitmap或分级授权。产品API和token不得携带caller可控scope；token至少绑定device、nonce、期限/计数和policy version。Challenge一次性使用，DEBUG_AUTH授权只消费一次并立即关闭Vendor user-auth状态。Die0/Die1跟随同一全局开关；OTP/key、eHSM秘密、安全RAM明文区和raw Mailbox继续受独立访问控制保护。

# Approved boundary and inputs

`OPEN-DESIGN-013`已关闭：SoC只有一个全局Debug开关，timeout/reset/LCS变化/安全错误/close必须关闭。RMA固定由DEBUG LCS承载，但使用独立RMA授权和只读诊断白名单；普通Debug token不得替代，不新增RMA LCS且不降低安全启动。全局enable寄存器、最大期限、授权主体和关闭失败RAS映射由产品权限矩阵补齐。

# Verification impact

覆盖重放、过期、错设备、token含scope字段拒绝、Die0/Die1同步开关、授权不可复用、读回不一致、reset自动关闭和敏感资产访问。

# Change history

- 2026-07-27：当时冻结的scope上限条款已由2026-07-28最终裁决取代；challenge/token和强制关闭原则保留。
- 2026-07-24：同步主详设第10章和Vendor Host命令Evidence。
- 2026-07-28：按负责人裁决删除Debug scope，冻结唯一SoC全局开关和一次性授权消费。
