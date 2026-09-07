## Requirements

### Requirement: Vendor 交付件 Review 入口文档

系统 SHALL 在 `docs/review/` 下提供中文 vendor 交付件 review 入口文档，用于说明 `osr_ehsm` 目录的交付范围、组件关系、源码/工具/测试资源边界和推荐阅读顺序。

#### Scenario: 项目组成员首次了解 vendor 交付包

- **WHEN** 项目组成员打开 `osr_ehsm` 并准备参加内部介绍
- **THEN** 文档 SHALL 列出 BL、FW、Host API、External Driver API、tools、python tests、PDF docs、OpenSpec 的职责和 review 优先级
- **AND** 文档 SHALL 区分源码、预编译工具、测试镜像、外部接口头文件和构建产物

#### Scenario: Reviewer 准备开始代码审查

- **WHEN** reviewer 准备进入源码 review
- **THEN** 文档 SHALL 提供入场检查清单，包括 hash 校验、构建验证、工具帮助、PDF 对照和 CodeGraph 查询建议

### Requirement: Bootloader 深度 Review 框架

系统 SHALL 提供一份中文 Bootloader 深度 review 主文档，覆盖 `ehsm_bl-2.3.5-4019-72f8fdc` 的入口初始化、调度命令面、secure boot image 验证、secure upgrade image 处理、OTP/KMU/key、debug auth、平台驱动和安全辅助机制。

#### Scenario: Reviewer 追踪 BL 启动链路

- **WHEN** reviewer 阅读 BL review 文档
- **THEN** 文档 SHALL 给出从 `main()` 到 `secboot_entry()`、`mailbox_init()`、`secboot_do_self_test()`、`sch_start()` 的阅读路径和流程图

#### Scenario: Reviewer 追踪 BL 安全启动链路

- **WHEN** reviewer 审查 secure boot image 处理
- **THEN** 文档 SHALL 给出 image header 字段、`naked`/`plain` 策略、version counter、防 rollback、解密、签名/CMAC、public key hash 校验的 review checklist

#### Scenario: Reviewer 追踪 BL 升级链路

- **WHEN** reviewer 审查 secure upgrade image 处理
- **THEN** 文档 SHALL 给出外层 upgrade image 验签/CMAC、外层解密、内层 boot image 解析、重加密、签名重算和分块状态机的 review checklist

### Requirement: 安全不变量和信任边界记录

系统 SHALL 在 BL review 文档中记录信任边界和安全不变量，明确 Host mailbox/shared memory/debug UART 输入默认不可信，OTP/KMU/root key/public key hash/version counter/lifecycle 为信任根数据但仍需检查访问错误和使用权限。

#### Scenario: Reviewer 分析 mailbox command handler

- **WHEN** reviewer 分析 `mbcmd_parser.c` 中任一 command handler
- **THEN** review 记录 SHALL 包含 command ID、输入结构、输出结构、生命周期限制、地址检查、key/OTP 权限、失败行为和 review 结论

#### Scenario: Reviewer 分析 CPU/M130 相关行为

- **WHEN** review 内容涉及 CPU core、CSR、异常/中断、CLIC、启动上下文、特权级、PMP、WMSIS、CPU porting 或核相关寄存器/时序/集成行为
- **THEN** 文档 SHALL 优先引用 `docs/CPU/` 下资料
- **AND** 若源码注释或推断与 CPU 资料冲突，文档 SHALL 标记为待确认

### Requirement: Review 结论必须可追踪

系统 SHALL 要求每一条风险、结论或 vendor 待确认问题关联代码路径、模块、证据来源和验证方式。

#### Scenario: 记录安全风险

- **WHEN** reviewer 在 BL/FW/Host/tools 中发现风险
- **THEN** 风险记录 SHALL 包含 ID、模块、代码位置、风险描述、影响、状态和证据/验证

#### Scenario: 记录 vendor 待确认问题

- **WHEN** reviewer 无法仅凭源码、PDF 或测试确认某个行为
- **THEN** 文档 SHALL 记录问题 ID、关联模块和期望 vendor 提供的材料

### Requirement: 专题文档索引与引用维护

系统 SHALL 使用 `docs/review/03_bootloader_deep_review.md` 维护 Bootloader 专题文档主索引，并在每篇专题文档中提供返回该主索引的链接。

#### Scenario: 新增或重命名 Review 专题文档

- **WHEN** reviewer 在 `docs/review/` 下新增或重命名 Bootloader、Host/BL交互或与Bootloader安全链路直接相关的专题文档
- **THEN** `03_bootloader_deep_review.md` 的“专题文档索引” SHALL 同步登记专题名称、Markdown链接和覆盖内容
- **AND** 专题文档 SHALL 在标题后提供指向 `03_bootloader_deep_review.md` 的“主Review索引”链接
- **AND** 所有相对链接 SHALL 通过文件存在性检查

### Requirement: 文档语言和命名规则

系统 SHALL 使用简体中文撰写 vendor review 文档和 OpenSpec review spec；代码符号、路径、命令、API、寄存器名、宏名、协议字段名和英文缩写 SHALL 保留英文原文。

#### Scenario: 新增 review 文档或更新 review spec

- **WHEN** 后续新增或更新 `docs/review/`、`openspec/specs/vendor-ehsm-review/` 或相关 OpenSpec change
- **THEN** 正文 SHALL 使用简体中文
- **AND** 源码符号和协议字段 SHALL 保持与代码一致的英文原文

### Requirement: Review 迭代任务

系统 SHALL 在 review 文档中维护后续填充计划，至少覆盖 BL 入口初始化、command handler 矩阵、image verification、image upgrade、OTP/KMU/debug auth/mmap/mailbox/sysreg 和测试验证。

#### Scenario: 完成一个 BL 模块 review

- **WHEN** reviewer 完成一个 BL 模块的逐函数审查
- **THEN** 对应任务 SHALL 更新状态
- **AND** 文档 SHALL 补充模块职责、输入来源、关键检查、失败路径、风险结论和验证结果
