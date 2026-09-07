## ADDED Requirements

### Requirement: 中文代码库总览文档
系统 SHALL 提供一份中文 Markdown 总览文档，说明 OSR eHSM 交付包的主要组件、职责、构建入口、关键模块和推荐阅读路径。

#### Scenario: 工程师需要快速了解目录
- **WHEN** 工程师首次打开 `osr_eshm` 交付包
- **THEN** 总览文档列出 BL、FW、HOST API、外部驱动 API、tools、docs、OpenSpec workspace 的职责

#### Scenario: 工程师需要定位构建入口
- **WHEN** 工程师在总览文档中查找构建信息
- **THEN** 文档指出 BL、FW、HOST 的 CMake 入口、主要配置项和主要构建产物

### Requirement: 带图的中文流程文档
系统 SHALL 提供一份中文 Markdown 流程文档，使用 Mermaid 图说明 secure boot、image verification、upgrade processing、firmware command dispatch 和 host mailbox command flow。

#### Scenario: Reviewer 追踪 secure boot
- **WHEN** reviewer 阅读流程文档
- **THEN** 文档展示从 BL `main()` 到 `secboot_entry()`、BL scheduler polling、验证后 FW jump、FW `secboot_entry()` 的路径

#### Scenario: Reviewer 追踪命令分发
- **WHEN** reviewer 阅读流程文档
- **THEN** 文档展示从 HOST API 命令构造到 mailbox、FW communication polling、scheduler dispatch、service handler 执行和 response writeback 的路径

### Requirement: 安全敏感机制说明
系统 SHALL 用中文记录源码中观察到的安全敏感机制，包括 lifecycle gate、version counter check、OTP key usage check、CFI counter、TRNG initial discard、debug/test command 风险。

#### Scenario: 安全 reviewer 扫描关键机制
- **WHEN** 安全 reviewer 查看文档
- **THEN** 文档列出安全机制并指向对应源码区域

### Requirement: 文档默认中文规则
系统 SHALL 在 OpenSpec 配置中声明：除非用户明确要求其他语言，后续项目文档和 OpenSpec 文档默认使用简体中文。

#### Scenario: 后续创建新文档
- **WHEN** 后续通过 OpenSpec 或普通文档流程创建 proposal、design、spec、tasks、分析报告或沉淀材料
- **THEN** 文档正文默认使用简体中文，代码符号、命令、路径、API、宏名和寄存器名保留英文原文

### Requirement: CPU 资料准源规则
系统 SHALL 在 OpenSpec 配置中声明：`osr_eshm` 承载 eHSM 代码的处理器核为 M130，凡涉及 CPU core、CSR、异常/中断、CLIC、启动上下文、特权级、PMP、WMSIS、CPU porting 或核相关寄存器/时序/集成行为的说明，均以 `docs/CPU/` 下资料为准。

#### Scenario: 后续分析 CPU 初始化或中断代码
- **WHEN** 后续分析 `src/driver/cpu/m130/`、`port_m130.c`、`clic.c` 或其他核相关实现
- **THEN** 分析和文档必须优先查阅并引用 `docs/CPU/Wing-M130_Integration_Manual.pdf` 与 `docs/CPU/Wing-M130_Technical_Reference_Manual_v1p0.pdf`

#### Scenario: 源码推断与 CPU 资料冲突
- **WHEN** 源码注释、历史文档或静态分析推断与 `docs/CPU/` 资料存在不一致
- **THEN** 文档必须标记为待确认，并优先采用 `docs/CPU/` 资料作为核相关内容的准源

### Requirement: 分析限制与后续任务
文档 SHALL 说明当前分析限制，并记录后续提升结构准确性的任务。

#### Scenario: CodeGraph 尚未初始化
- **WHEN** 该目录在分析时没有 CodeGraph 索引
- **THEN** 文档记录该限制，并建议后续初始化 CodeGraph 以补充精确调用图
