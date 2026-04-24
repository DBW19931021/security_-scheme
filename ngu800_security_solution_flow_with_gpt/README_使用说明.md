# NGU800 安全方案固定流程包 V2

这个工程包的目标是：

```text
持续吸收资料变化
→ 收敛约束
→ 形成 baseline
→ 生成章节级详设
→ 生成实现级详设
→ 形成 code rules 和 traceability
→ 约束 Codex 后续代码开发
→ 支持增量更新
```

## 目录说明

- `codex/skills/ngu800-security/`
  - Codex 使用的安全方案 Skill
- `security_workflow/`
  - 工作流中间产物
- `security_inputs/`
  - 输入材料目录
- `tools/`
  - PDF 导出脚本

## 推荐工作流

### 步骤 0：准备输入
把材料放到：

```text
security_inputs/
```

并维护：

```text
security_inputs/inputs_manifest.md
```

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
```

输出：

```text
security_workflow/04_impl_design/*.md
```

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
→ implementation-level design
→ code rules
→ traceability
→ final check / change impact
```

## 为什么要增加 `04_impl_design/`

因为章节级详设适合评审和方案说明，但不能直接指导 RTL / FW / driver 实现。

`04_impl_design/` 用来承接：

- eFuse / OTP 规划
- key hierarchy / key ladder
- FW header / image format
- mailbox 接口
- SPDM report
- lifecycle 控制
- manufacturing / provisioning 细节

这一步是“方案 → 代码”的桥梁，不能跳过。

## 注意事项

- 不要跳过 `01_constraints.md`
- 不要跳过 `02_baseline.md`
- 不要跳过 `04_impl_design/`
- 新资料加入后，先更新 `inputs_manifest.md`
- 有目录或流程改动时，必须同步更新：
  - `SKILL.md`
  - `prompts/`
  - `README_使用说明.md`
