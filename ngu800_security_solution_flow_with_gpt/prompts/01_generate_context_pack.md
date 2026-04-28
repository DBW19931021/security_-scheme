# Prompt 01：Codex 生成设计上下文包

## 目标

让 Codex 从当前仓库整理 `.context/design_context_pack.md`，交给 GPT 做设计裁决。

## Prompt

你是 NGU800 安全方案仓库的上下文整理助手。请只整理上下文，不做设计裁决，不修改安全方案正文。

请执行：

1. 读取当前 git commit hash 和 working tree status。
2. 阅读 `security_inputs/inputs_manifest.md`，生成 inputs_manifest 摘要。
3. 阅读 `security_workflow/01_constraints.md`，生成 constraints 摘要。
4. 阅读 `security_workflow/02_baseline.md`，生成 baseline 摘要。
5. 根据本次主题，摘录相关原文片段，并标注文件路径和行号。
6. 整理已知冲突。
7. 整理待关闭 `[TBD]`、`待补`、`OPEN` 关键词。
8. 根据 `05_traceability/file_sync_checklist.md` 推荐可能影响的文件。
9. 输出到 `.context/design_context_pack.md`。

严格限制：

- Codex 只整理上下文，不做架构裁决。
- Codex 不得把 `[TBD]` 改成 `[CONFIRMED]`。
- Codex 不得修改 `01_constraints.md`、`02_baseline.md`、`03_detailed_design/*.md`、`04_impl_design/*.md`。

输出格式必须对齐 `.context/design_context_pack_template.md`。

