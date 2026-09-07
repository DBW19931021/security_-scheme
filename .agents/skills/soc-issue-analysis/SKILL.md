---
name: soc-issue-analysis
description: "Investigate SoC security defects and anomalies from symptom and reproduction through evidence, hypotheses, elimination, root cause, fix, regression, and design or documentation impact. Use for security test failures, protocol faults, boot failures, inconsistent sources, field issues, or recurring engineering problems."
---

# 问题闭环

1. 固定环境、版本、分支、commit、芯片修订和复现条件。
2. 保存原始 Evidence，区分观察事实与假设。
3. 建立候选假设并记录逐项排除证据，不只写最终修复。
4. 确认根因影响范围、触发条件和安全后果。
5. 记录修复、回退、回归测试和残余风险。
6. 更新架构、需求、OpenSpec、ADR、文档和 Skill（仅在方法稳定时）。
7. 未达到证据门槛时保持 `investigating`，不得宣布根因已确认。

使用 `templates/issue-analysis.md`。
