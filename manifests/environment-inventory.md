# 环境盘点

- 盘点日期：2026-07-20（Asia/Shanghai）
- 映射盘：`Z:` → `\\172.16.20.246\share`
- 安全工程仓库：`Z:\code\ngu800\secure\security_-scheme`
- 公司代码仓库：`Z:\code\ngu800\secure\gsp-pmp-rmp-omp`
- 当前访问：两个目录均可读；经用户批准可写安全工程仓库。
- Git 网络共享保护：当前 Windows 用户与共享目录所有者 SID 不同；盘点使用单命令、精确路径的临时 `safe.directory`，未修改全局 Git 配置。

## 初始化前文件系统

`security_-scheme` 仅包含 `.git`，提交树为空。`gsp-pmp-rmp-omp` 包含 `boards`、`CI`、`components`、`docs`、`openspec`、`solutions`、`tools` 等目录，以及根级 `AGENTS.md`。

## Git 仓库

详细信息见 `repositories.yaml`。公司代码仓库在 `master@08b29c7b7a29...`，盘点时存在大量既有未提交修改；后续以当前 HEAD 作为只读基线，允许默认分支工作区修改，但必须保护已有无关改动且不得执行任何 Git 提交。

## 原始资料

初始化前 `security_-scheme` 中没有 PDF、DOCX、XLSX、PPTX、Markdown、压缩包或图片资料。公司代码仓库内文档属于代码仓库内容，本次仅登记仓库，不把其内容复制为权威 Source。

## 工具

- Git：2.53.0.windows.3（Codex bundled runtime）
- Python：3.12.13（Codex bundled runtime）
- OpenSpec CLI：未安装，状态 `pending`
- PyYAML：当前 bundled Python 未安装；项目检查使用无依赖的保守 YAML 子集校验。

## 风险和异常

- 网络共享上的 Git 状态扫描较慢。
- 公司代码仓库既有修改的归属和意图未知。
- OpenSpec CLI 正式初始化需要 Node.js 20.19+ 和经批准的包安装。
- 原始资料进入 Git 前需确认公司保密规则、单文件大小和 Git LFS 策略。
