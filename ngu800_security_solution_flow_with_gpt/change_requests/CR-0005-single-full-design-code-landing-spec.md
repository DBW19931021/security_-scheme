# CR-0005: 10_full_design 作为唯一代码落地详设入口

Status: accepted-for-application / applied-by-codex / owner-review-pending
Base Commit: `df815e2`
Owner: Security Owner / Architecture Review
Codex Role: accepted-apply；落实用户裁决，不新增安全架构结论
Created: 2026-05-08
Accepted: 2026-05-08
Applied: 2026-05-08
Source / Context: 用户明确要求 `10_full_design.md` 可作为详设并直接指导代码落地，且 `04_impl_design` 内容不做省略性或总结性删减，应尽量全量合并
Primary Topic: documentation source-of-truth / code landing specification
Change Type: workflow source-of-truth refactor
Risk Level: medium

---

## 1. 背景

CR-0004 暴露出一个结构性问题：当章节级详设、实现级详设和整合版详设都包含字段、结构体、OTP/key/counter 或 mailbox ABI 时，后续代码实现容易误读某个分片文档为独立事实源，进而产生与 eHSM TRM、accepted CR 或 `10_full_design.md` 不一致的实现口径。

用户在 2026-05-08 明确裁决：

```text
10_full_design 这一份文档就可以当作是详设，完全可以指导代码的落地。
应该把 04_impl_design 里的内容也直接合并入 10_full_design。
不要做省略性或者总结性的删减，内容要尽量全和细。
```

本 CR 因此只改变文档事实源关系和同步规则，不改变 CR-0001 至 CR-0004 已裁决的安全架构结论。

## 2. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `df815e2` |
| 当前主详设 | `security_workflow/03_detailed_design/10_full_design.md`，目前为章节级整合版，已包含 CR-0001/CR-0003/CR-0004 主口径 |
| 当前实现级分片 | `security_workflow/04_impl_design/*.md`，包含 eFuse/key/FW header、mailbox、manufacturing、SPDM report、eHSM source-conformance matrix |
| 已知问题 | `10_full_design.md` 与 `04_impl_design/*.md` 同时可被实现人员读取，存在事实源分叉风险 |
| 用户裁决 | `10_full_design.md` 应成为可直接指导代码落地的完整详设，不应只保留摘要 |

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| `10_full_design.md` 角色 | `10_full_design.md` 是代码实现、评审、ChatGPT 方案审查和 Codex 落地的完整详设主入口 | 降低多文档事实源冲突，符合用户希望单文档指导代码落地的协作方式 | `[CONFIRMED]` |
| `04_impl_design/*.md` 角色 | `04_impl_design` 保留为编辑分片 / extracted implementation shard / source appendix，不再作为独立事实源 | 便于分模块维护，但不得与 `10_full_design.md` 形成并列 ABI 权威 | `[CONFIRMED]` |
| 合并方式 | 将 `04_impl_design` 中有效正文尽量全量嵌入 `10_full_design.md`，不做省略性或总结性删减 | 满足“全和细”的代码落地要求 | `[CONFIRMED]` |
| 冲突处理 | 若 `10_full_design.md`、`04_impl_design`、CR、decision_log 冲突，安全结论优先级为 accepted CR / decision_log / official TRM / `10_full_design.md`，分片文档不得覆盖主详设 | 避免再次出现 eHSM header / OTP layout 类冲突 | `[CONFIRMED]` |
| 未冻结项 | 原有 `[TBD]` 不因合并而升级为 `[CONFIRMED]` | 本 CR 是文档组织裁决，不是安全字段冻结 CR | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 增加完整实现级落地详设章节，嵌入 `04_impl_design` 全量内容 |
| `security_workflow/04_impl_design/README.md` | Yes | 标明本目录不再是独立事实源 |
| `security_workflow/04_impl_design/*.md` | Yes | 增加非独立事实源 banner，指向 `10_full_design.md` |
| `README_使用说明.md` | Yes | 更新推荐工作流和事实源关系 |
| `codex/skills/ngu800-security/SKILL.md` | Yes | 更新 mandatory pipeline / final source-of-truth rule |
| `codex/skills/ngu800-security/prompts/*` | Yes | 更新实现级生成与最终检查提示，要求同步主详设 |
| `codex/skills/ngu800-security/templates/full_design_template.md` | Yes | 增加实现级落地详设章节 |
| `security_workflow/05_code_rules.md` | Yes | 增加文档事实源规则 |
| `security_workflow/06_traceability.md` | Yes | 增加 source-of-truth trace |
| `security_workflow/04_change_impact.md` | Yes | 记录 CR-0005 影响与检查结果 |
| `00_project/decision_log.md` | Yes | 记录文档事实源裁决 |
| `00_project/changelog.md` | Yes | 记录本次变更 |

## 5. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| `10_full_design.md` | 新增“实现级落地详设全集”章节；完整嵌入 eFuse/key/FW header、eHSM source-conformance matrix、mailbox、manufacturing、SPDM report 内容 | 不删除 CR-0004 的 `[TBD]`，不把未冻结字段升级为 `[CONFIRMED]` |
| `04_impl_design/*.md` | 添加 banner，说明主事实源为 `10_full_design.md`，本文件为编辑分片 | 不删除正文细节 |
| workflow docs / skill / prompts | 更新“04_impl_design 不能跳过”的旧表述为“可作为分片存在，但必须同步进 10_full_design” | 不改变安全设计裁决流程、CR gate、decision authority gate |
| code rules / traceability / impact | 增加文档事实源与同步检查规则 | 不改变既有安全规则语义 |

## 6. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| 主详设仅为章节汇编、分章仍保留更高事实源地位 | `10_full_design.md` | `10_full_design.md` 是完整详设与代码落地主入口，分片文档为编辑来源 | 用户裁决，避免多文档事实源冲突 |
| `04_impl_design/` 是“方案 → 代码”的强制分片入口 | `README_使用说明.md` / Skill / prompts | `04_impl_design/` 可保留为编辑分片，但实现级内容必须进入 `10_full_design.md` | 使单文档可直接指导代码 |
| 实现级字段只存在于 `04_impl_design/*.md` | impl docs / traceability | 字段必须在 `10_full_design.md` 中可见 | 方便代码审查和 GPT 方案审查 |

## 7. 不允许 Codex 自行改变的内容

- 不得改变 Root of Trust、First Verifier、Host trust boundary、SEC1/SEC2 sign+encrypt、eHSM native header、OTP/key/counter source-conformance 等已裁决安全结论。
- 不得把 manifest ABI、exact key ID、exact OTP/control bit、per-image CEK / wrapped CEK、per-image rollback 等 `[TBD]` 升级为 `[CONFIRMED]`。
- 不得用合并动作删除实现级细节。
- 不得将 `04_impl_design` 的分片正文作为更高优先级来源覆盖 `10_full_design.md`。

## 8. 验收标准

- [x] `10_full_design.md` 包含 `04_impl_design` 中所有实现级正文主题：eFuse/key/FW header、eHSM source-conformance matrix、mailbox、manufacturing/provisioning、SPDM report。
- [x] `10_full_design.md` 中的实现级章节不以摘要替代分片正文。
- [x] `04_impl_design` 已明确标记为非独立事实源 / 编辑分片。
- [x] README、Skill、prompts、template 已同步新的事实源关系。
- [x] code rules / traceability / change impact / decision_log / changelog 已记录本裁决。
- [x] 未新增无依据 `[CONFIRMED]`。
- [x] `git diff --check` 通过。
- [x] 搜索确认旧事实源口径已从活跃工作流文档中替换。

## 9. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-08 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`; `security_workflow/04_impl_design/README.md`; `security_workflow/04_impl_design/*.md`; `README_使用说明.md`; `codex/skills/ngu800-security/SKILL.md`; `codex/skills/ngu800-security/prompts/*`; `codex/skills/ngu800-security/templates/*`; `prompts/*`; `templates/*`; `security_inputs/inputs_manifest.md`; `00_project/decision_log.md`; `00_project/changelog.md`; `05_traceability/design_impact_matrix.md`; `security_workflow/04_change_impact.md`; `security_workflow/05_code_rules.md`; `security_workflow/06_traceability.md` |
| 未修改但检查过的文件 | `tree -a -L 4`; `README_使用说明.md`; `codex/skills/ngu800-security/SKILL.md`; `codex/skills/ngu800-security/prompts/`; `codex/skills/ngu800-security/templates/`; `security_workflow/01_constraints.md`; `security_workflow/02_baseline.md` |
| 未完成项 | 无；待 owner/GPT review 后关闭 CR |
| 执行说明 | 本 CR 为文档事实源和代码落地入口重构，不引入新的安全架构裁决。已验证 `04_impl_design` 全部正文被嵌入 `10_full_design.md`，旧事实源口径已替换，`git diff --check` 通过，无行尾空白。 |

## 10. GPT / Owner 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 | 待复核 |
| Review 结论 | `owner-review-pending` |
| 阻塞问题 | 无已知阻塞 |
| 非阻塞建议 | 合并后建议由 ChatGPT 按 `10_full_design.md` 单文档进行一次完整详设审查 |
| 是否允许关闭 CR | No，待 owner review |
