# INV-SEC-008：Vendor Poll Cache/Timeout/迟到响应只读调查

## TASK BRIEF

- Task ID：INV-SEC-008
- 状态：completed
- Owner：GSP
- 父任务：TASK-SEC-SOC-FW-001 / B0-R2
- 目标：确认在不修改Vendor Host公共代码的前提下，首版单在途、cache维护、command deadline、retry和late-response应如何落地。
- 非目标：不修改Vendor、公司产品或baremetal代码；不猜测cache/PMA、MMIO地址或timeout数值；不执行Git、构建或测试。
- 输入：SRC-0018、SRC-0022、ADR-0010、CE-SEC-005～007、NGU800P公司代码只读工作区。
- 输出：`evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md`、ADR-0011、`docs/04-interfaces/ehsm-host-adapter.md`。

## 调查问题

1. Vendor poll路径在何处执行cache维护，响应前是否存在可移植钩子？
2. Vendor timer port如何支持不同command的deadline？
3. timeout后能否证明command未执行，能否安全重试或复用context？
4. 首版最小并发模型如何避免修改Vendor公共代码？

## 完成记录

- 2026-07-23：确认Vendor公共cache钩子只在发送前调用；poll响应handler会直接读取context/rsp，项目侧不存在响应前插入第二个invalidate的公共钩子。
- 2026-07-23：确认Vendor timeout port没有command/deadline参数；首版必须依赖单在途和adapter安装的active timeout scope。
- 2026-07-23：确认公共`EHSM_ERR_TIMEOUT`无法区分提交前和提交后超时，且无sequence/cancel/abort；因此timeout统一视为acceptance unknown并隔离slot。
- 2026-07-23：负责人批准首版保守基线：BootROM/FMC/GSP分别一个静态context，GSP统一service串行化，首版不自动retry，迟到响应未闭环前不复用。
- 全程未执行Git、构建或测试，未修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor源码。
