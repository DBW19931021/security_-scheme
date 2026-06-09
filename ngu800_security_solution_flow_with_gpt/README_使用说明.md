# NGU800 安全方案固定流程包 V2

这个工程包的目标是：

```text
持续吸收资料变化
→ 收敛约束
→ 形成 baseline
→ 生成章节级详设
→ 生成实现级详设
→ 将实现级详设全量合入 10_full_design
→ 形成 code rules 和 traceability
→ 约束 Codex 后续代码开发
→ 支持增量更新
```

## 职责分工

推荐分工如下：

- ChatGPT / 项目组 / security owner：负责方案设计、架构裁决、冲突取舍和冻结结论
- Codex：负责把已批准结论落实到仓库，包括 CR、constraints、baseline、章节详设、实现级设计、code rules、traceability、代码和测试

Codex 不应自行冻结新的安全架构结论。没有用户冻结结论、accepted CR、signed-off baseline、decision_log 或官方资料支撑时，只能输出 `[PROPOSED]`、`[ASSUMED]` 或 `[TBD]`。

## 目录说明

- `codex/skills/ngu800-security/`
  - Codex 使用的安全方案 Skill
- `security_workflow/`
  - 工作流中间产物
  - `security_workflow/03_detailed_design/10_full_design.md` 是 CR-0005 后的完整详设与代码落地主入口
  - `security_workflow/04_impl_design/` 是实现级编辑分片 / extracted implementation shards，不再作为独立事实源
- `security_inputs/`
  - 输入材料目录
- `tools/`
  - PDF 导出脚本

## 推荐工作流

### 步骤 -1：给 ChatGPT / 项目组准备上下文包

当需要先做方案设计或重新裁决时，执行：

```text
codex/skills/ngu800-security/prompts/00_生成ChatGPT方案上下文包.md
```

输出：

```text
.context/design_context_pack.md
```

这个步骤只整理输入、冲突、开放问题和待裁决清单，不冻结新结论。

### 步骤 0：准备输入
把材料放到：

```text
security_inputs/
```

并维护：

```text
security_inputs/inputs_manifest.md
```

当前 source precedence：

- `security_inputs/current_plan/芯片安全软件方案_2.0.pdf` 已登记为 `SRC-008 当前收敛安全软件方案 2.0`。
- 2026-06-03 后，如无 accepted CR、`00_project/decision_log.md`、官方 eHSM/TRM、后续用户特殊说明或源内明确例外，当前安全软件方案以 `SRC-008` 为准。
- 旧 `security_inputs/current_plan/安全方案.pdf` / `SRC-001` 仅保留历史流程参考，不再作为当前方案基线。

### 步骤 1：生成约束表
执行：

```text
codex/skills/ngu800-security/prompts/01_生成约束表.md
```

输出：

```text
security_workflow/01_constraints.md
```

### 步骤 2：生成设计基线
执行：

```text
codex/skills/ngu800-security/prompts/02_生成设计基线.md
```

输出：

```text
security_workflow/02_baseline.md
```

### 步骤 3：生成章节规划
执行：

```text
codex/skills/ngu800-security/prompts/03A_生成详设章节规划.md
```

输出：

```text
security_workflow/03_detailed_design/00_chapter_plan.md
```

### 步骤 4：逐章生成详设
执行：

```text
codex/skills/ngu800-security/prompts/03B_逐章生成详设.md
```

输出：

```text
security_workflow/03_detailed_design/*.md
```

### 步骤 5：生成实现级详设
依次执行：

```text
codex/skills/ngu800-security/prompts/04_impl_efuse_key_fw_header.md
codex/skills/ngu800-security/prompts/05_impl_mailbox.md
codex/skills/ngu800-security/prompts/06_impl_spdm_report.md
codex/skills/ngu800-security/prompts/06B_impl_lifecycle_manufacturing.md
```

输出：

```text
security_workflow/04_impl_design/*.md
```

CR-0005 后，`04_impl_design/*.md` 只是编辑分片。实现级正文不能只停留在分片中，必须全量同步进：

```text
security_workflow/03_detailed_design/10_full_design.md
```

### 步骤 5B：同步完整详设主入口

将章节级详设和实现级分片合并到完整详设主入口：

```text
security_workflow/03_detailed_design/10_full_design.md
```

要求：

- `10_full_design.md` 必须能直接指导代码落地、评审和测试设计。
- `04_impl_design` 中的字段表、C-like 结构体、状态机、命令、错误码、OTP/key/counter mapping、manufacturing/SPDM 细节必须在 `10_full_design.md` 可见。
- 不得用摘要替代实现级正文。
- 若 `10_full_design.md` 与 `04_impl_design` 冲突，以 accepted CR、`00_project/decision_log.md`、official TRM 和 `10_full_design.md` 为准。

### 步骤 6：生成代码规则
执行：

```text
codex/skills/ngu800-security/prompts/07_生成_code_rules.md
```

输出：

```text
security_workflow/05_code_rules.md
```

### 步骤 7：生成追踪矩阵
执行：

```text
codex/skills/ngu800-security/prompts/08_生成_traceability.md
```

输出：

```text
security_workflow/06_traceability.md
```

### 步骤 8：最终检查 / 增量更新
执行：

```text
codex/skills/ngu800-security/prompts/09_最终检查与增量更新.md
```

必要时输出：

```text
security_workflow/04_change_impact.md
```

## 这版 V2 相比旧流程的变化

旧流程：

```text
constraints → baseline → full design → final check
```

V2 流程：

```text
constraints
→ baseline
→ chapter-level detailed design
→ implementation-level design shards
→ full design code landing spec
→ code rules
→ traceability
→ final check / change impact
```

## 为什么保留 `04_impl_design/`

因为分片文件更适合局部编辑、分模块 review 和增量更新。

`04_impl_design/` 用来承接：

- eFuse / OTP 规划
- key hierarchy / key ladder
- FW header / image format
- mailbox 接口
- SPDM report
- lifecycle 控制
- manufacturing / provisioning 细节

CR-0005 后，这一步不再是独立事实源。真正的“方案 → 代码”桥梁是：

```text
security_workflow/03_detailed_design/10_full_design.md
```

`04_impl_design/` 里的内容必须全量合入 `10_full_design.md` 第 10 章，作为完整详设的一部分指导代码。

## 注意事项

- 不要跳过 `01_constraints.md`
- 不要跳过 `02_baseline.md`
- 不要跳过实现级详设内容；如果使用 `04_impl_design/` 分片，必须同步到 `10_full_design.md`
- 不要让 `04_impl_design/` 与 `10_full_design.md` 形成并列事实源
- 不要让 Codex 自行把推理结论标为 `[CONFIRMED]`
- 新方案结论应先由 ChatGPT / 项目组 / security owner 评审，再由 Codex 落实
- 新资料加入后，先更新 `inputs_manifest.md`
- 有目录或流程改动时，必须同步更新：
  - `SKILL.md`
  - `prompts/`
  - `templates/`
  - `README_使用说明.md`
  - `README_使用说明.md`
