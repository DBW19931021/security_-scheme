---
title: "OTP/Key Registry单一基线同步计划"
status: completed
evidence_state: DOCUMENTED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0024
owners: []
last_reviewed: 2026-08-04
supersedes: []
superseded_by: []
---

# OTP/Key Registry 单一基线同步计划

> 范围：只修订 `security_-scheme` 的方案记录及 `baremetal/components/security`
> 的引用关系，不修改 eHSM 测试功能代码、Vendor 快照或产品代码。

1. 将项目负责人提供的 OTP Table 34 和 16-slot Key 表转录为受控来源
   `SRC-0024`，记录适用范围和未给出的字段。
2. 修正 ADR-0025/0026、主详设第10章及 Key/OTP/Certificate/Provisioning
   专题，使槽号、等级、类型、权限和 OTP 地址公式与 `SRC-0024` 一致。
3. 同步 OpenSpec、Open Question、Project Status、Baseline Control 和变更记录，
   删除“精确 OTP offset 未提供”和错误的 slot 7/11 等旧结论。
4. 将 baremetal 的 16-slot manifest 定位为方案基线的派生测试清单，关闭
   `EHSM-OTP-004`；保留 eHSM BL/FW 旧 map、属性编码和 fixture 材料等真实缺口。
5. 通过全仓文本扫描、CSV/源码表核对、Markdown/YAML/OpenSpec 检查验证一致性。
