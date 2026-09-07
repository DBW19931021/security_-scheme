# INV-SEC-006：eHSM Mailbox Vendor基线只读调查

## TASK BRIEF

- Task ID：INV-SEC-006
- 状态：completed
- Owner：GSP
- 父任务：TASK-SEC-SOC-FW-001 / B0-R2
- 目标：从当前Vendor手册、Host/BL代码和NGU800P只读代码中关闭Mailbox channel数量、地址转换、BootROM运行模式和ready/error规则。
- 非目标：不修改Vendor、公司产品或baremetal代码，不冻结尚无项目输入的MMIO绝对地址、deadline、RAS和arena容量，不执行Git、构建或测试。
- 输入：SRC-0005、SRC-0012、SRC-0014、SRC-0018、负责人2026-07-23输入。
- 输出：`evidence/code-investigations/CE-SEC-006-ehsm-mailbox-vendor-baseline.md`。

## 调查问题

1. 当前eHSM/NGU800P Mailbox channel数量是否已有资料依据？
2. Host到eHSM和eHSM内部访问SoC的地址转换如何实现？
3. BootROM应使用poll还是interrupt？
4. ready/error位及放行条件能否从Vendor手册整理？
5. non-cacheable陈述应如何区分MMIO与共享RAM访问？

## 完成记录

- 2026-07-23：完成Vendor PDF页面、Host/BL关键函数及NGU800P 16路IRQ枚举只读核对，形成CE-SEC-006并回填详设/约束。关闭16 channel、地址转换机制、BootROM poll和ready/error位问题；保留SoC绑定、cache维护、deadline、RAS和arena参数。未执行Git、构建或测试，未修改两个代码仓。
