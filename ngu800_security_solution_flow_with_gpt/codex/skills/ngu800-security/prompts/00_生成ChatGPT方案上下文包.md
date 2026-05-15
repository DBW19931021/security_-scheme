请不要直接修改安全方案正文。

本步骤用于给 ChatGPT / 项目组 / security owner 准备方案设计上下文包，输出到：

- `.context/design_context_pack.md`

输入：
- `security_inputs/inputs_manifest.md`
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/`
- `security_workflow/04_impl_design/`
- `00_project/decision_log.md`
- `00_project/open_questions.md`
- `change_requests/`

要求：
1. 摘要当前已批准结论、已接受 CR、签核来源和冻结敏感项
2. 列出 `[PROPOSED]`、`[ASSUMED]`、`[TBD]` 的设计问题
3. 列出输入冲突、缺失资料和受影响章节 / 实现文件
4. 给出需要 ChatGPT / owner 裁决的问题清单
5. 不新增 `[CONFIRMED]` 结论
6. 不把 Codex 推理写成 source of truth
