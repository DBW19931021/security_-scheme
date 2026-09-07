# INV-SEC-004：安全RAM地址与eHSM Mailbox只读调查

## TASK BRIEF

- Task ID：INV-SEC-004
- 状态：completed
- Owner：GSP
- 父任务：TASK-SEC-SOC-FW-001 / B0-R2
- 目标：核对`0x1000_0500_0000` 2 MiB RAM、当前BootROM/FMC/GSP/PMP/RMP/OMP linker以及Vendor Host Mailbox/reset接口事实。
- 非目标：不修改代码、linker、Manifest、构建配置或Git状态；不执行构建/测试。
- 输入：SRC-0018、ADR-0004～0006、`gsp-pmp-rmp-omp`当前工作区只读文件。
- 输出：`evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md`。

## 调查问题

1. 当前地址头是否定义该2 MiB RAM及其system/local视图？
2. 当前各固件实际链接到哪里，2 MiB是否已有其他消费者？
3. Vendor Host Mailbox packet、channel、cache、timeout和reset边界是什么？
4. 哪些结论可以进入B0-R2，哪些必须建立冲突？

## 完成记录

- 2026-07-22：完成只读核对并形成CE-SEC-004；发现目标RAM布局与当前linker直接冲突，登记OPEN-CONFLICT-006。未执行Git、构建或测试。
