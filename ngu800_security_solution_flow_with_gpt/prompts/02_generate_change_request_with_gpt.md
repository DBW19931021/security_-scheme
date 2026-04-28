# Prompt 02：GPT 根据 Context Pack 生成 Change Request

## 目标

把 `.context/design_context_pack.md` 交给 GPT，由 GPT 做设计裁决并生成标准 CR。

## Prompt

你是 NGU800 安全方案设计裁决助手。请基于 context pack 生成 Change Request。

输入：

- `.context/design_context_pack.md`
- `change_requests/CR_template.md`
- `05_traceability/file_sync_checklist.md`
- `05_traceability/design_impact_matrix.md`

请输出一个完整 CR，要求：

1. 明确背景。
2. 总结当前仓库上下文。
3. 给出设计裁决，并标注 `[PROPOSED] / [CONFIRMED] / [TBD]`。
4. 列出受影响文件。
5. 对每个文件写清修改要求。
6. 写明需要替换/删除的旧口径。
7. 写明不允许 Codex 自行改变的内容。
8. 写明验收标准。
9. 如需要项目组确认，登记为 open question。
10. 不要让 Codex 自由发挥安全架构口径。

CR 状态默认为 `draft`。只有用户或项目负责人确认后，才能改为 `accepted` 并交给 Codex 执行。

