## 背景

`osr_eshm` 是 eHSM 软件交付目录，包含 BL、FW、HOST API、外部驱动 API、工具和 PDF TRM。该目录本身不是 git repository，当前也尚未初始化 CodeGraph；但工程仍然需要一份中文、可搜索、可持续更新的代码结构与安全流程说明，方便后续设计、安全 review 和团队交接。

本变更的文档对象分为三类：

- BL：secure boot、镜像校验/解密/升级、OTP/key/self-test/debug 相关逻辑。
- FW：运行时初始化、mailbox/UART 通信、scheduler、service handler 和 crypto/KMS 服务。
- HOST：公开 API wrapper、shared-memory packet、mailbox register access 和 response mode。

## 目标 / 非目标

**目标：**

- 用简体中文沉淀 `osr_eshm` 目录结构、构建入口、关键模块和推荐阅读路径。
- 用 Mermaid 图描述 secure boot、image verification、upgrade、FW service dispatch 和 HOST mailbox command flow。
- 将文档语言规则写入 `openspec/config.yaml`，后续文档默认中文。
- 将 CPU 资料准源规则写入 `openspec/config.yaml`，后续涉及 M130 core、CSR、异常/中断、CLIC、特权级、PMP、WMSIS、CPU porting 等核相关内容时，以 `docs/CPU/` 下资料为准。
- 保留代码符号、命令、路径、API、寄存器名、宏名和协议字段的英文原文，避免和源码脱节。
- 用 OpenSpec 记录本次文档能力、设计决策和后续任务。

**非目标：**

- 不修改 BL/FW/HOST/tool 的运行时行为。
- 不替代厂商 PDF TRM。
- 不在未初始化 CodeGraph 的情况下声称完整调用图精确性。
- 不生成 Markdown 外部的独立图片资产。

## 设计决策

### 决策：文档默认使用简体中文

所有新增或更新的项目文档、OpenSpec proposal/design/spec/tasks、分析报告和沉淀材料默认使用简体中文。

理由：

- 当前团队工作语境是中文。
- 中文文档更适合作为内部设计和安全评审材料。
- 代码符号保留英文原文，仍能和源码、命令、宏名直接对齐。

备选方案：

- 保持英文文档。该方案不符合当前使用者的阅读和沉淀习惯。

### 决策：可读分析放在 `docs/`

主要可读产物放在：

- `docs/osr_ehsm_codebase_overview.md`
- `docs/osr_ehsm_security_and_command_flows.md`

理由：

- `docs/` 已经是该交付包的文档目录。
- Markdown 可搜索、可 review，也方便和 PDF TRM 并列。
- Mermaid 图可以直接随文档维护。

备选方案：

- 全部写在 `openspec/changes/...`。这利于规划归档，但日常阅读入口不够直接。

### 决策：OpenSpec 记录规则与可追踪变更

OpenSpec 负责记录为什么写、如何写、能力要求和后续任务；面向日常阅读的说明仍放在 `docs/`。

理由：

- OpenSpec 适合沉淀变更意图和约束。
- 后续可以 archive 成稳定 spec，也可以继续追加命令矩阵、生命周期策略表等新任务。

备选方案：

- 只写 docs，不写 OpenSpec。这样会丢失后续文档规则和任务追踪。

### 决策：CPU 核相关内容以 `docs/CPU/` 为准

`osr_eshm` 中 BL/FW 代码运行在 M130 处理器核上；凡涉及 CPU core、CSR、异常/中断、CLIC、启动上下文、特权级、PMP、WMSIS、CPU porting 或核相关寄存器/时序/集成行为的说明，均优先以 `docs/CPU/` 下资料为准。

理由：

- CPU 核资料是解释 `src/driver/cpu/m130/`、`port_m130.c`、`clic.c`、CSR/PMP/中断行为的准源。
- 源码注释和历史文档可能只描述局部实现，不能替代 M130 Integration Manual/TRM 对核行为的定义。
- 后续分析 CPU 初始化、异常、中断、特权级和平台 porting 时，需要避免仅凭源码推断硬件语义。

备选方案：

- 仅以源码为准。该方案可解释当前实现，但无法裁决核寄存器、异常/中断语义和集成约束。

### 决策：概览文档和流程文档拆分

`osr_ehsm_codebase_overview.md` 回答“目录里有什么、怎么构建、从哪里读起”。  
`osr_ehsm_security_and_command_flows.md` 回答“启动、命令、安全校验和升级流程怎么走”。

理由：

- 新人可以先读短路径总览。
- 安全评审可以直接进入 flow 和 trust boundary。

备选方案：

- 单一长文档。维护简单，但阅读和定位成本更高。

### 决策：将 CodeGraph 初始化作为后续任务

文档中明确记录本轮未初始化 CodeGraph，并建议后续执行 `codegraph init -i`。

理由：

- 项目指令偏好 CodeGraph 做结构分析。
- 当前文档基于源码扫描和关键模块阅读，已经能提供有效导览，但后续仍应补精确调用图。

备选方案：

- 暂停所有文档，等待 CodeGraph 初始化后再写。这样会阻塞当前代码梳理交付。

## 风险 / 取舍

- [Risk] 未使用 CodeGraph 时，静态扫描可能遗漏间接调用或动态 dispatch。  
  Mitigation：保留 CodeGraph 初始化后续任务，后续用索引调用图修正文档。

- [Risk] Mermaid 图在部分 Markdown 查看器中不渲染。  
  Mitigation：图外提供中文表格和流程说明，确保不渲染也可读。

- [Risk] BL/FW/HOST 的 mailbox header 可能存在漂移。  
  Mitigation：在 tasks 中保留命令矩阵和 header 对比任务。

- [Risk] 文档引用了带版本号的目录名。  
  Mitigation：本轮按交付快照精确引用；后续替换交付包时同步更新。

## 迁移计划

无运行时迁移。该变更只新增/更新 Markdown 和 OpenSpec 文档。

回滚方式是删除或恢复以下文件：

- `docs/osr_ehsm_codebase_overview.md`
- `docs/osr_ehsm_security_and_command_flows.md`
- `openspec/config.yaml`
- `openspec/changes/document-osr-ehsm-codebase/`

## 待确认问题

- 是否现在为 `osr_eshm` 执行 `codegraph init -i`，并用 CodeGraph 结果更新调用图？
- 是否将本次 `codebase-documentation` 归档为稳定 OpenSpec spec？
- HOST API 到 command ID 到 BL/FW handler 的命令矩阵是否单独开一个 OpenSpec change？
