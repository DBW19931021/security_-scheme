# INV-SEC-007：NGU800P eHSM Port绑定只读调查

## TASK BRIEF

- Task ID：INV-SEC-007
- 状态：completed
- Owner：GSP
- 父任务：TASK-SEC-SOC-FW-001 / B0-R2
- 目标：核实NGU800P当前地址/寄存器头是否已经给出Vendor eHSM Mailbox与status/error绑定，并确认C908可复用的cache、timer、IRQ和构建基础。
- 非目标：不修改Vendor、公司产品或baremetal代码，不猜测RTL未提供的地址，不执行Git、构建或测试。
- 输入：SRC-0016、SRC-0017、SRC-0018、CE-SEC-006、NGU800P公司代码只读工作区。
- 输出：`evidence/code-investigations/CE-SEC-007-ngu800p-ehsm-port-binding.md`。

## 调查问题

1. 当前`SECURITY_SUBSYS_MAILBOX`是否就是Vendor eHSM Mailbox？
2. eHSM专用Mailbox/status/error准确MMIO是否已在公司头文件或内部方案中给出？
3. C908当前是否已有可用于eHSM port的cache/barrier、64位timer和IRQ原语？
4. BootROM/FMC/GSP主构建是否实际包含这些原语？

## 完成记录

- 2026-07-23：确认当前4 KiB、84-message的`MAILBOX_SECURITY`与Vendor 16 × 4 KiB eHSM Mailbox在容量、stride和寄存器语义上不兼容；16路eHSM IRQ与通用Mailbox IRQ也分别枚举。
- 2026-07-23：确认当前公司头文件和两份内部方案没有eHSM专用Mailbox/status/error数值绑定，建立OPEN-CONFLICT-007并禁止误用`0x1000/0x1010_0841_0000`。
- 2026-07-23：确认C908 common提供64字节cache line/range维护/barrier、64位微秒timer和IRQ wrapper；Makefile主构建纳入common层，但CDK工程仍引用不存在的dummy组件。
- 2026-07-23补充：负责人确认SoC map/寄存器地址以RTL同步`baremetal`生成头为准；登记SRC-0022并接受ADR-0009。确认4 KiB/84-message数值不变，但撤回“默认另找64 KiB SoC孔径”的建议；OPEN-CONFLICT-007一度收敛为该块是否为eHSM wrapper及其映射语义，该中间结论随后由ADR-0010取代。
- 2026-07-23最终裁决：批准Vendor direct Option A并接受ADR-0010；Vendor公共代码不修改，4 KiB通用Mailbox不作为eHSM wrapper。direct aperture/status准确宏转为RTL/地址头集成同步门禁；BootROM/FMC/GSP首版全程poll、Vendor FW ready后interrupt仅为后续可选优化，以及RAS未ready终态同时冻结。
- 全程未执行Git、构建或测试，未修改`gsp-pmp-rmp-omp`或`baremetal`。
