# CR-0015 Security Module Atomic Rename

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0015` |
| Title | 安全组件及项目自研接口移除 `ngu_` / `NGU_` 前缀 |
| Status | `applied` |
| Mode | `implementation-only` |
| Owner | security owner / project owner |
| Reviewer | Codex follow-up review |
| Created Date | 2026-06-10 |
| Source / Context Pack | 用户在 Codex 协作会话中确认方案 A：原子 rename，不保留兼容别名 |
| Related Decision ID | `DEC-0020` |

## 2. 背景

当前安全组件目录、include namespace、文件、API、宏、测试和工具广泛使用
`ngu_` / `NGU_`。用户已冻结新规则：现有组件整体重命名为 `security`，
项目自研符号按功能模块命名，并且不保留旧接口兼容层。

本 CR 只处理工程命名和仓库引用，不改变安全架构、协议行为、二进制布局
或数值语义。

## 3. 当前仓库上下文

| 上下文项 | 摘要 |
|---|---|
| 当前 git commit hash | `6ff33404c115ba1a38c37bb03193e50c1d50f7ee` |
| inputs_manifest 摘要 | `SRC-008` 是当前安全软件方案基线；本 CR 不新增输入源 |
| constraints 摘要 | 本 CR 不改变安全约束 |
| baseline 摘要 | 本 CR 不改变 Root of Trust、boot、attestation 或接口 baseline |
| 相关详设章节 | 仅同步其中出现的旧代码路径和符号，不改变设计结论 |
| 相关实现级文档 | 组件代码、构建、测试、工具、OpenSpec 和代码说明文档 |
| 已知冲突 | 现有 development principle 要求旧 `ngu_*` ABI 保持原名，已被本次用户裁决 supersede |
| 待关闭问题 | 无新增安全设计开放问题 |

## 4. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 组件目录 | `components/ngu_security` 原子 rename 为 `components/security` | 用户明确不保留旧目录名 | `[CONFIRMED]` |
| Include namespace | `include/ngu_security` rename 为 `include/security` | 保持组件名和公共 include 一致 | `[CONFIRMED]` |
| API 兼容 | 不保留 forwarding header、typedef、宏或 wrapper | 用户选择方案 A，要求彻底删除旧前缀 | `[CONFIRMED]` |
| 自研命名 | 通用对象使用 `security_*`；子模块使用功能前缀 | 降低芯片代号耦合并保留所有权 | `[CONFIRMED]` |
| 标准命名空间 | 不把项目常量机械改成可能冲突的标准 `SPDM_*` / `MCTP_*` | 第三方标准定义必须保持权威 | `[CONFIRMED]` |
| 行为边界 | 数值、布局、wire format、协议和安全策略不变 | 本 CR 是命名迁移，不是功能变更 | `[CONFIRMED]` |
| 第三方边界 | 上游 libspdm/libmctp 源码和公开命名不改 | 保持 snapshot 可升级和许可证边界 | `[CONFIRMED]` |

## 5. 影响矩阵

| 层级 | 对象 | 是否影响 | 影响说明 | 状态 |
|---|---|---|---|---|
| 输入 | inputs manifest | No | 不新增输入资料 | `no-change` |
| 约束 | `01_constraints.md` | No | 不改变安全约束 | `no-change` |
| Baseline | `02_baseline.md` | No | 不改变安全 baseline | `no-change` |
| 详细设计 | boot/key/attestation/interface/full design | Yes | 只更新代码路径和符号引用 | `applied` |
| 实现设计 | component/API/code landing references | Yes | 更新路径、文件和 API 名称 | `applied` |
| Code rules | 命名与组件边界 | Yes | supersede 旧 ABI 保留规则 | `applied` |
| Traceability | code/test mapping | Yes | 更新代码和测试路径 | `applied` |
| Test | host/packager/target build | Yes | 增加残留扫描和回归验证 | `applied` |
| Project records | CR/decision/changelog/impact | Yes | 记录本次破坏性源码 rename | `applied` |

## 6. 受影响文件

| 文件范围 | 是否必须修改 | 影响说明 |
|---|---|---|
| `components/ngu_security/**` | Yes | 整体 rename，更新全部项目自研命名和文档引用 |
| `solutions/*/Makefile` direct consumers | Yes | 组件名改为 `security` |
| `solutions/bootrom/app/src/bootrom_secure_demo.c` | Yes | 更新 include、类型、宏和函数 |
| root `AGENTS.md` / `README.md` | Yes | 更新组件路径说明和命令 |
| `security_workflow` code/path references | Conditional | 只改实现引用，不改安全设计语义 |
| upstream libspdm/libmctp files | No | 保持上游源码不变 |

## 7. 不允许改变的内容

- Root of Trust、BootROM/eHSM/FMC/GSP 职责和可信链。
- manifest/eHSM/SPDM/MCTP 的数值、字段布局和 wire encoding。
- structure field order、width、packing 和 serialized layout。
- secure boot、measurement、attestation 和 mailbox 行为。
- 上游第三方公开 API 和源码命名。

## 8. 验收标准

- [x] `components/security` 是唯一安全组件目录。
- [x] `include/security` 是唯一公共 include namespace。
- [x] production、build、test、tool 和 active usage docs 中不存在
      `ngu_security`、`ngu_` 或 `NGU_`。
- [x] 旧名称只允许出现在 immutable upstream 文件和本 CR/迁移设计的
      old-to-new 记录中。
- [x] 不存在旧 API compatibility alias 或 forwarding header。
- [x] 上游第三方源码未发生命名改写。
- [x] structure size、numeric constants 和 test vectors 保持一致。
- [x] security host tests、image packager tests 全部通过。
- [x] BootROM、FMC、GSP 构建通过。
- [ ] OpenSpec validation 和 `git diff --check` 通过。
- [ ] generated knowledge indexes 已按新路径重建。

## 9. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-06-10 |
| 修改文件 | `components/security/**`; BootROM/FMC/GSP direct consumers; root path docs; workflow project map and CR records |
| 未修改但检查过的文件 | inputs manifest、security baseline、upstream libspdm/libmctp source namespace |
| 未完成项 | 最终 OpenSpec/diff 检查和 generated knowledge index rebuild |
| 执行说明 | 已完成组件目录、include namespace、项目自研文件/API/宏、构建产物、测试和工具的原子 rename；未增加旧接口兼容层 |

## 10. GPT / Owner 复核记录

| 项目 | 内容 |
|---|---|
| Review 时间 |  |
| Review 结论 | `PENDING_WRITTEN_SPEC_REVIEW` |
| 阻塞问题 |  |
| 非阻塞建议 |  |
| 是否允许关闭 CR | No |
