# 工作空间初始化报告

- 日期：2026-07-20（Asia/Shanghai）
- 总体检查：`PASS`
- 安全工程仓库：`Z:\code\ngu800\secure\security_-scheme`
- 公司代码仓库：`Z:\code\ngu800\secure\gsp-pmp-rmp-omp`

## 创建结果

已直接在现有 `security_-scheme` Git 仓库中创建安全工程结构，没有创建嵌套 Git 仓库。当前共识别 187 个非 Git 文件，包括基础文档、41 个安全文档入口、OpenSpec 人工兼容模板、任务/ADR/Source Card/Requirement 模板、六个项目级 Skill 和轻量检查工具。

## 当前架构

- Work 在本仓库读取资料、编写方案、维护 OpenSpec/ADR/任务并评审结果。
- 远端 Codex 仅在批准任务下进入独立的 `gsp-pmp-rmp-omp` 仓库实现。
- OpenSpec 位于 `openspec/`；用户已执行 init，当前 schema 为 `spec-driven`，CLI 版本未知。
- Evidence 位于 `evidence/`，大文件可以保存不可变路径和哈希引用。
- 本仓库是正式事实、问题、决策和追踪关系的唯一来源。

## 识别结果

- `security_-scheme`：初始化前为干净 `master`，远端为 `git@github.com:DBW19931021/security_-scheme.git`，原提交树为空。
- `gsp-pmp-rmp-omp`：`master@08b29c7b7a29...`，远端为公司 GitLab HTTP 地址，盘点时存在大量既有未提交修改。
- 资料入库：已登记 15 份 Vendor PDF、2 份 NGU800P 初步方案基线、1 份 OSR eHSM 代码快照和 1 套历史 Review 文档集；ADR-0001/ADR-0002 已确认方案权威层级、Vendor 边界、PDF 基线控制和冲突升级机制。
- 可用工具：Git 2.53.0、Python 3.12.13；OpenSpec 已由用户初始化，但当前 Codex PATH 不包含 CLI。

## 自动检查

| 检查 | 状态 | 结果 |
|---|---|---|
| frontmatter | PASS | 42 个设计文档具备 frontmatter |
| source-ids | PASS | 文档 Source ID 均可解析；当前登记 20 项 |
| requirement-ids | PASS | Requirement ID 唯一；当前登记 0 项 |
| traceability | PASS | 0 个追踪路径均存在 |
| tasks | PASS | 任务结构完整；active=0, completed=0 |
| obsolete-sources | PASS | 正式文档未引用已识别的 OBSOLETE 来源 |
| yaml | PASS | 18 个 YAML 文件通过保守子集校验 |
| broken-links | PASS | Markdown 本地链接均可解析 |
| assumptions | WARN | 40 个初始设计入口按要求标为 ASSUMPTION，需随资料入库逐项收敛 |
| skills | PASS | 6 个项目级 Skill 结构有效 |

## 未完成项和风险

- OpenSpec 已初始化；当前 Codex PATH 无 CLI，因此版本和原生 `openspec validate` 结果待在实际 OpenSpec 终端补充。
- `skill-creator` 自带 `quick_validate.py` 因 bundled Python 缺少 PyYAML 无法运行；已由仓库内无第三方依赖检查完成等价结构校验。
- 公司代码仓库既有修改的归属和保护范围待确认；允许默认分支工作区修改，但不得覆盖无关改动或执行任何 Git 提交。
- Vendor 资料/代码适用的 eHSM/Core 版本、SRC-0018 正式交付来源，以及两份内部方案的 Owner/批准版本仍待确认；Vendor 文档在本工程中不保密。
- SRC-0018 的 Bootloader 目录后缀与 `SW_changelist.md` 不一致；SRC-0019 中 1 条候选发现已复核并完成裁决，其余 16 条待重新验证。
- eHSM BL/Host自检位图差异已按Vendor回复和ADR-0003关闭：采用Bootloader定义，bit18/`0x40000`=`TRNG`，Host定义错误，bit19/`0x80000`保持unknown/reserved；原始Vendor回复材料待补录。
- 原始大文件进入 Git 前需确认保密和 Git LFS 策略。
- Git 对网络共享报告所有者不一致；本次没有修改全局 `safe.directory`。

## 优先下一步

1. 确认 Vendor 资料/代码适用的 eHSM/Core 版本、SRC-0018 交付来源和两份内部方案的 Owner/批准版本。
2. 核对已批准密钥轮换策略是否完整进入芯片安全软件方案，并重新验证历史候选项；发现明显冲突立即提交负责人裁决。
3. 对决定采纳的 Vendor 建议先更新受控 amendment/芯片安全软件方案，再建立 issue/OpenSpec/Evidence 闭环。
