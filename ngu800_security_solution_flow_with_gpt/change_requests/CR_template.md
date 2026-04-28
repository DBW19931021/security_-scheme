# Change Request Template

> 文件命名建议：`CR-YYYYMMDD-short-topic.md`

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-YYYYMMDD-XXX` |
| Title |  |
| Status | `draft / accepted / applied / reviewed / closed / superseded` |
| Owner |  |
| Reviewer |  |
| Created Date |  |
| Source / Context Pack | `.context/design_context_pack.md` |
| Related Decision ID |  |

## 2. 背景

说明为什么提出本 CR：

- 新增输入资料：
- 项目组新裁决：
- 现有方案冲突：
- 实现/RTL/验证阻塞：
- 安全漏洞或风险：

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash |  |
| inputs_manifest 摘要 |  |
| constraints 摘要 |  |
| baseline 摘要 |  |
| 相关详设章节 |  |
| 相关实现级文档 |  |
| 已知冲突 |  |
| 待关闭 TBD |  |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
|  |  |  | `[PROPOSED] / [CONFIRMED] / [TBD]` |

## 5. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
|  | `Yes / No / TBD` |  |

## 6. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
|  |  |  |

## 7. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
|  |  |  |  |

## 8. 不允许 Codex 自行改变的内容

Codex 执行本 CR 时不得自行改变以下内容：

- 安全架构根口径，例如 RoT / First Verifier / Host trust boundary。
- 未在本 CR 中明确授权修改的设计章节。
- 未经 GPT/项目组裁决的 `[TBD]`。
- 未经来源支撑的 `[CONFIRMED]`。
- 与本 CR 无关的正文、图、表和接口字段。

## 9. 验收标准

- [ ] 所有受影响文件均已按要求修改。
- [ ] 未影响文件没有被无关修改。
- [ ] 旧口径已替换或删除。
- [ ] 新口径在 constraints / baseline / detailed design / impl design / traceability 中一致。
- [ ] 没有新增无依据 `[CONFIRMED]`。
- [ ] 未关闭问题已登记到 `00_project/open_questions.md`。
- [ ] 新设计裁决已登记到 `00_project/decision_log.md`。
- [ ] 仓库变更已登记到 `00_project/changelog.md`。

## 10. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 |  |
| 修改文件 |  |
| 未修改但检查过的文件 |  |
| 未完成项 |  |
| 执行说明 |  |

## 11. GPT 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PASS / PASS_WITH_NITS / NEEDS_FIX / BLOCKED` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | `Yes / No` |

