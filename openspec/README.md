# OpenSpec 状态

用户已执行 `openspec init`，当前 schema 为 `spec-driven`。`config.yaml` 已加入 SoC 安全事实分级、资料引用、人工审批、任务验收、Evidence 和禁止 Git 写操作等项目规则。

当前 Codex Windows 运行环境的 `PATH` 中未发现 `openspec` 命令，因此本次无法读取 CLI 版本或执行原生校验；这不改变仓库已经初始化的事实。后续可在实际安装 OpenSpec 的终端运行：

```text
openspec schemas --json
openspec validate --all
```

`openspec/specs/` 保存当前行为契约，`openspec/changes/` 保存活动变更，`openspec/changes/archive/` 保存已归档变更。原始 Datasheet、Vendor 代码和方案基线不放入 OpenSpec change，change 通过 Source ID 引用它们。
