---
name: soc-source-intake
description: "Register SoC security source material, extract identity and applicability, classify evidence authority, create Source Cards, and report conflicts or open questions. Use for new Datasheets, TRMs, standards, internal specifications, vendor code snapshots, meeting records, or revised source versions."
---

# 资料入库流程

1. 读取 `sources/source-index.yaml` 和相关 Source Card。
2. 计算或记录文件哈希，提取名称、版本、日期、所有者、适用芯片/修订和有效范围。
3. 将来源区分为正式规范、实测/RTL 证据、Vendor 实现、会议结论或未知材料。
4. 只使用仓库允许的事实状态；不得把 Vendor 代码提升为芯片规范。
5. 创建或更新 Source Card、source index 和 intake report。
6. 与现有结论比较，生成 conflict report 和 open questions；不要直接覆盖正式设计。
7. 列出受影响的需求、文档、ADR、OpenSpec 和测试。

使用 `templates/` 中的模板。无法确认的字段写 `UNKNOWN`，不要推断。
