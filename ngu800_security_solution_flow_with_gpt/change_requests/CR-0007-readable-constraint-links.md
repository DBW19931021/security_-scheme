# CR-0007: 生效约束 ID 可读性与跳转链接增强

Status: accepted-for-application / applied-by-codex / owner-review-pending
Base Commit: `df815e2`
Owner: Security Owner / Document Owner
Codex Role: accepted-apply；仅增强文档可读性和追溯链接，不改变安全结论
Created: 2026-05-11
Accepted: 2026-05-11
Applied: 2026-05-11
Source / Context: 用户指出 `10_full_design.md` 各章节“生效约束 ID”只有裸 ID，可读性较差，希望增加链接或其他方式增强阅读体验
Primary Topic: documentation readability / constraint traceability links
Change Type: documentation-only readability enhancement
Risk Level: low

---

## 1. 背景

`10_full_design.md` 每个章节都有“生效约束 ID”小节，用于追溯该章节受哪些 `01_constraints.md` 约束约束。但原格式只有：

```text
- `C-BOOT-04`
```

读者需要手动跳到 `01_constraints.md` 搜索 ID 才能知道含义，评审体验较差。

本 CR 将其改为：

```text
- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
```

## 2. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `df815e2` |
| 主详设 | `security_workflow/03_detailed_design/10_full_design.md` |
| 约束源文件 | `security_workflow/01_constraints.md` |
| 已知问题 | 生效约束 ID 可追溯但不可读，缺少直接跳转 |

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 约束 anchor | 在 `01_constraints.md` 每个 `C-xxx` 约束标题前增加稳定 HTML anchor | 避免依赖 Markdown 对中文标题的自动 anchor 规则 | `[CONFIRMED]` |
| 章节约束显示 | 在 `10_full_design.md` 的“生效约束 ID”中使用“链接 + 一句话摘要” | 提升评审可读性，减少来回搜索 | `[CONFIRMED]` |
| 安全结论 | 本 CR 不改变任何约束正文、baseline、安全策略或实现级 ABI | 仅为文档可读性增强 | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/01_constraints.md` | Yes | 为每个 `C-xxx` 约束标题增加稳定 anchor |
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 将各章“生效约束 ID”改为链接 + 摘要，并增加约束链接说明 |
| `05_traceability/design_impact_matrix.md` | Yes | 登记 CR-0007 文档可读性影响 |
| `00_project/changelog.md` | Yes | 记录仓库级变更 |

## 5. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `01_constraints.md` | 仅增加 `<a id="c-xxx"></a>` anchor | 不改约束正文、状态、来源和 rationale |
| `10_full_design.md` | 生效约束 ID 改为 Markdown 链接并追加一句摘要 | 不改章节设计正文和安全结论 |
| `design_impact_matrix.md` | 增加 CR-0007 影响矩阵 | 不改变既有 CR 影响结论 |
| `changelog.md` | 追加 CHG 记录 | 不改既有变更记录语义 |

## 6. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| 裸 `C-xxx` ID 列表 | `10_full_design.md` | `[C-xxx](../01_constraints.md#c-xxx) - 摘要` | 裸 ID 可读性差，评审需要手动搜索 |

## 7. 不允许 Codex 自行改变的内容

- 不得改变任何安全架构结论。
- 不得修改约束正文、baseline、CR-0001~CR-0006 的设计语义。
- 不得把摘要当成新的事实源；摘要只用于阅读，完整约束仍以 `01_constraints.md` 为准。

## 8. 验收标准

- [x] `01_constraints.md` 中每个约束标题前有稳定 anchor。
- [x] `10_full_design.md` 中“生效约束 ID”不再是裸 ID，而是链接 + 摘要。
- [x] `10_full_design.md` 包含约束链接说明。
- [x] 未改变安全结论。
- [x] 本次修改文件通过 `git diff --check -- ...`。

## 9. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-11 |
| 修改文件 | `security_workflow/01_constraints.md`; `security_workflow/03_detailed_design/10_full_design.md`; `05_traceability/design_impact_matrix.md`; `00_project/changelog.md`; `change_requests/CR-0007-readable-constraint-links.md` |
| 未完成项 | 无 |
| 执行说明 | 本次只增强约束追溯可读性，不改变约束正文和安全方案结论。 |

## 10. GPT / Owner 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 | 待复核 |
| Review 结论 | `owner-review-pending` |
| 阻塞问题 | 无 |
| 非阻塞建议 | 后续可考虑把约束摘要集中生成成 glossary / appendix |
| 是否允许关闭 CR | No，待 owner review |
