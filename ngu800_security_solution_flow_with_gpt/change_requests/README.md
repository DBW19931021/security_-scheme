# Change Requests 使用说明

## 目的

`change_requests/` 用于承载 NGU800 安全方案的设计变更请求。后续流程约定为：

1. Codex 只负责从仓库整理上下文，不做设计裁决。
2. GPT 根据 context pack 生成设计裁决和 CR。
3. Codex 严格按 CR 修改仓库，不自行改变安全架构口径。
4. GPT 或人工 reviewer 对 Codex diff 做安全一致性复核。

## 状态流转

```text
draft -> accepted -> applied -> reviewed -> closed
                         \-> superseded
```

| 状态 | 含义 | 允许动作 |
|---|---|---|
| `draft` | CR 草案，尚未接受 | 可讨论、补充上下文 |
| `accepted` | CR 已接受，允许 Codex 执行 | Codex 可按 CR 修改仓库 |
| `applied` | Codex 已完成修改 | 等待 GPT/人工 review |
| `reviewed` | diff 已复核 | 可修复问题或准备关闭 |
| `closed` | 变更闭环完成 | 不再修改本 CR |
| `superseded` | 被其他 CR 替代 | 不再按本 CR 执行 |

## 推荐流程

1. 使用 `prompts/01_generate_context_pack.md` 生成 `.context/design_context_pack.md`。
2. 使用 `prompts/02_generate_change_request_with_gpt.md` 让 GPT 生成 CR。
3. 将 CR 保存为 `change_requests/CR-YYYYMMDD-short-topic.md`。
4. CR 状态从 `draft` 更新为 `accepted` 后，使用 `prompts/03_apply_change_request_with_codex.md` 执行。
5. 执行后运行 `tools/check_cr_sync.py <CR 文件路径>` 做轻量检查。
6. 使用 `prompts/04_review_codex_diff_with_gpt.md` 做 diff 复核。
7. 复核通过后更新 `decision_log.md`、`changelog.md`，并关闭 CR。

## 强制 CR 条件

凡是影响两个以上文件、影响 baseline、或影响 boot/key/cert/attestation/debug/interface/manufacturing 任一安全主路径的变更，必须先生成 Change Request，不得直接修改正文。

