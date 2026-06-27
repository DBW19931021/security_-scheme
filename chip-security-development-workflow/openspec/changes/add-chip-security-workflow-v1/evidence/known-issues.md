# Known Issues

## Open Issues

- 第二款芯片尚未执行，因此 L4/L5 不可晋升。
- Gate checker 目前是结构检查，不解析 Markdown 表格中的全部 ID 关系。
- 通用项目尚未提供组织级 CI 集成和审批系统连接器。

## Accepted Risks

- V1 使用 Markdown + JSON + Python 3.8 标准库，优先保证可读、可移植和低依赖。
- `REVIEW` 不导致 CLI 非零退出；组织流程必须把人工审批作为独立门禁。

## Closed Issues

- CLI 直接执行时的 Python package import 失败已由入口路径回归测试覆盖。
- NGU800 MCTP 的摘要/完整设计重复已收敛到组件 OpenSpec `design.md`。
