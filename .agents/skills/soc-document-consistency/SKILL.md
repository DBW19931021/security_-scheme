---
name: soc-document-consistency
description: "Audit SoC security documentation for stale conclusions, obsolete sources, unresolved assumptions, ADR or OpenSpec drift, broken links, missing requirement mappings, untested design items, missing Evidence, and invalid versions or paths. Use during reviews, releases, source updates, and task closure."
---

# 文档一致性检查

1. 运行 `python tools/scripts/project_check.py`。
2. 检查 frontmatter、Source ID、Requirement ID、路径和本地链接。
3. 查找 `OBSOLETE` 来源引用、未关闭 Assumption、过期 ADR、OpenSpec/正式文档/代码行为不一致。
4. 查找无需求映射的设计、无测试的需求、无 Evidence 的完成任务和失效版本。
5. 对每项问题给出文件、证据、影响、建议修复和责任状态。
6. 不自动改变事实等级或删除历史；需要语义判断的变更交由人工评审。

使用 `templates/consistency-report.md`。
