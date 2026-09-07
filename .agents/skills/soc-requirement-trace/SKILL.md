---
name: soc-requirement-trace
description: "Maintain end-to-end SoC security traceability from Requirement to architecture, interface, software repository and file, test case, Evidence, and status. Use when adding or changing requirements, designs, code links, tests, evidence, or coverage status."
---

# 需求追踪

1. 验证 Requirement ID 唯一且 statement、rationale、来源、适用范围和事实状态完整。
2. 建立 Requirement → Architecture → Interface → Repository/File/Commit → Test → Evidence → Status 链路。
3. 只关联实际存在的路径、测试和 Evidence；计划项必须显式标为未完成。
4. 发现无设计、无实现、无测试或无 Evidence 的断点时记录 coverage gap。
5. 行为变化时同步 OpenSpec、ADR 和项目状态。
6. 运行 `tools/scripts/project_check.py` 并报告剩余断点。

使用 `templates/trace-entry.yaml`。
