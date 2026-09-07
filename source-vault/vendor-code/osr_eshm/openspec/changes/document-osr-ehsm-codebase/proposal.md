## 背景与动机

`osr_eshm` 是一个由 Bootloader、Firmware、Host API、外部驱动接口、工具和 PDF TRM 组成的 eHSM 软件交付包。当前目录已有厂商 PDF，但缺少可搜索、可 review、可持续更新的文字化代码结构说明。

本变更用于沉淀源码结构、启动链路、mailbox 命令链路、镜像校验/升级链路、安全敏感机制，以及后续补全任务。

## 变更内容

- 在 `docs/` 下新增中文 Markdown 代码库总览文档。
- 在 `docs/` 下新增中文安全与命令流程文档。
- 使用 Mermaid 图表达 package layout、BL boot flow、firmware verification、FW service dispatch、HOST mailbox flow 和 upgrade state flow。
- 将文档能力写入 OpenSpec，便于后续归档和持续更新。
- 明确项目文档默认使用简体中文，代码符号、路径、命令、API、宏名等保留英文原文。
- 明确 CPU 资料准源约束：该 M130 处理器核承载 eHSM 代码，凡涉及核相关内容，均以 `docs/CPU/` 下资料为准。
- 不修改 bootloader、firmware、host API、工具或二进制行为。

## 能力范围

### 新增能力
- `codebase-documentation`：为 OSR eHSM 交付包提供中文、可搜索、带图的代码结构与关键流程文档。

### 修改能力
- 无。

## 影响范围

- 影响文档：`docs/osr_ehsm_codebase_overview.md`、`docs/osr_ehsm_security_and_command_flows.md`。
- 影响 OpenSpec 规则：`openspec/config.yaml`。
- 影响 OpenSpec 变更材料：`openspec/changes/document-osr-ehsm-codebase/`。
- 参考 CPU 资料范围：`docs/CPU/Wing-M130_Integration_Manual.pdf`、`docs/CPU/Wing-M130_Technical_Reference_Manual_v1p0.pdf`。
- 参考代码范围：`ehsm_bl-2.3.5-4019-72f8fdc/`、`ehsm_fw-2.3.2-4019-5a4a0a9/`、`ehsm_host-2.3.1-4019-2ee044d/`、`osr_ehsm_external_driver_api_v1.1/`、`tools/`。
- 不修改构建产物或生成二进制。
