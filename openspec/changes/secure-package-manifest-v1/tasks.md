# Tasks

> **SUPERSEDED（2026-08-21）**：历史任务保留；不得据此新建Manifest实现任务。现行任务以`native-header-load-address-v1/tasks.md`为准。

| Task ID | 目标仓库 | 状态 | 内容 | 完成标准 |
|---|---|---|---|---|
| OSP-PKG-001 | security_-scheme | completed | 接受ADR-0014并关闭OPEN-CONFLICT-008/OPEN-DESIGN-008 | 裁决、规格、追溯一致 |
| OSP-PKG-002 | security_-scheme | completed | 冻结Manifest v1和公共ABI registry | 布局、值、reserved规则明确 |
| OSP-PKG-002A | security_-scheme | completed | 按ADR-0024精简Manifest并删除TLV/expected digest | payload固定offset128、loader源/目标双摘要、无旧布局兼容 |
| OSP-PKG-003 | security_-scheme | review_with_deferred_input | 冻结BootROM/FMC/GSP函数级状态机 | ADR-0015已冻结GSP全生命周期唯一Owner；OPEN-DESIGN-010等待Runtime依赖/release资料 |
| OSP-PKG-004 | gsp-pmp-rmp-omp | not_authorized | 实现registry生成、parser、verify、loader和stage编排 | 实现任务另行批准且通过DoR |
| OSP-PKG-005 | baremetal | not_authorized | 实现ABI/包negative和eHSM可执行case | 测试任务另行批准且通过DoR |

所有任务禁止把Git提交作为交付步骤。当前change只允许修改`security_-scheme`中的设计与追溯文件。
