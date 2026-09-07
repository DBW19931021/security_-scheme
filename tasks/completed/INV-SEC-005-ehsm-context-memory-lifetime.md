# INV-SEC-005：eHSM Context内存放置与生命周期只读调查

## TASK BRIEF

- Task ID：INV-SEC-005
- 状态：completed
- Owner：GSP
- 父任务：TASK-SEC-SOC-FW-001 / B0-R2
- 目标：判断Vendor Mailbox是否要求独立物理Region，以及packet/cmd/rsp/context能否放入BootROM/FMC/GSP普通栈。
- 非目标：不修改Vendor或产品代码、linker、构建配置和Git状态；不执行构建/测试。
- 输入：SRC-0018、CE-SEC-004、ADR-0008、Vendor Host当前受控快照。
- 输出：`evidence/code-investigations/CE-SEC-005-ehsm-context-memory-lifetime.md`。

## 调查问题

1. Vendor packet、cmd和rsp如何分配及寻址？
2. context是否可能跨函数返回、interrupt callback、poll或流式调用存活？
3. 独立Mailbox物理Region是否是协议要求？
4. 与stage data/stack合并时需要哪些生命周期和cache约束？

## 完成记录

- 2026-07-23：完成只读核对并形成CE-SEC-005。结论为独立Mailbox物理Region不是Vendor协议要求；负责人随后批准stage-local固定arena，并禁止普通函数栈承载通用异步/流式/timeout未闭环context。未执行Git、构建或测试。
