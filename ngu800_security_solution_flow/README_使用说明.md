# NGU800 安全方案固定流程包

这个工程包的目标只有一个：

**先抓约束，再定基线，再生成 NGU800 安全详细设计，再导出 PDF。**

它不追求通用到所有领域，优先满足你当前的安全方案需求。

---

## 目录说明

- `.codex/skills/ngu800-security/`
  - Codex 使用的安全方案 skill
- `security_workflow/`
  - 工作流中间产物和 prompts
- `tools/`
  - PDF 导出脚本
- `security_inputs/`
  - 你的输入材料目录；建议把 `inputs_manifest.md` 放这里

---

## 推荐工作流

### 步骤 0：准备输入
把你的材料放到：

```text
security_inputs/
```

并维护：

```text
security_inputs/inputs_manifest.md
```

---

### 步骤 1：生成约束表
在 Codex 中使用：

```text
请执行 `security_workflow/prompts/01_生成约束表.md`
```

目标输出：

```text
security_workflow/01_constraints.md
```

---

### 步骤 2：生成设计基线
在 Codex 中使用：

```text
请执行 `security_workflow/prompts/02_生成设计基线.md`
```

目标输出：

```text
security_workflow/02_baseline.md
```

---

### 步骤 3：生成完整详设
在 Codex 中使用：

```text
请执行 `security_workflow/prompts/03_生成完整详设.md`
```

目标输出：

```text
security_workflow/03_detailed_design.md
```

---

### 步骤 4：最终检查 / 变更更新
在 Codex 中使用：

```text
请执行 `security_workflow/prompts/04_最终检查与增量更新.md`
```

---

## PDF 导出

### 依赖
Linux 环境下建议安装：

```bash
sudo apt-get install pandoc
npm install -g @mermaid-js/mermaid-cli
```

说明：
- 当前导出流程会先把 Markdown 中的 Mermaid 图预渲染成 PNG，再生成 PDF。
- 这样最终 PDF 中显示的是实际图形，不是 Mermaid 源代码。
- 默认通过 `google-chrome` 和 [tools/puppeteer_mermaid_config.json](/home/may-pc/share/code/ngu800/secure/security_-scheme/ngu800_security_solution_flow/tools/puppeteer_mermaid_config.json) 驱动 Mermaid 渲染。

### 导出命令

```bash
bash tools/build_security_pdf.sh
```

默认会把：

```text
security_workflow/03_detailed_design.md
```

导出为：

```text
security_workflow/03_detailed_design.pdf
```

当前实际导出链路：

```text
Markdown -> Mermaid PNG 预渲染 -> pandoc + weasyprint -> PDF
```

如需手动指定输入输出，也可以直接执行：

```bash
python3 tools/export_markdown_pdf.py security_workflow/03_detailed_design.md security_workflow/03_detailed_design.pdf
```

---

## 这套流程解决什么问题

相较于“直接让 Codex 从原始资料写完整方案”，它强制多做了两层：

1. `01_constraints.md`
   - 把硬约束抽出来
2. `02_baseline.md`
   - 把设计口径统一下来

这样可以明显减少以下问题：
- 明明给了资料，却没抓住关键约束
- 文档成型了，但方案不稳
- 章节之间不一致
- 结构体像模板，不像从资料压出来的接口

---

## 增量更新怎么做

新增材料后：

1. 更新 `security_inputs/inputs_manifest.md`
2. 重新执行 `01_生成约束表.md`
3. 再执行 `02_生成设计基线.md`
4. 最后执行 `04_最终检查与增量更新.md`

如果变化较大，再重跑 `03_生成完整详设.md`

---

## 注意事项

- 不要跳过 `01_constraints.md`
- 不要直接从 raw inputs 生成最终详设
- `02_baseline.md` 是整份文档的一致性来源
- 有疑义的地方宁可标 `[TBD]`，不要假装成 `[CONFIRMED]`
