---
title: "NGU800P SoC安全方案EDA必测与硬件协同验证项"
status: active
evidence_state: PROPOSED_NOT_EXECUTED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0022
  - SRC-0025
  - SRC-0026
  - SRC-0027
owners:
  - GSP
  - RTL/DV
last_reviewed: 2026-08-20
---

# NGU800P SoC安全方案EDA必测与硬件协同验证项

## 1. 当前结论

2026-08-20核对的[飞书最终安全测试表](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)只保留2条EDA/硬件Case：`062 / EDA-ERR-CRITICAL-IRQ-001`和`063 / EDA-ERR-ECC1B-IRQ-001`。两条都需要RTL-DV/EDA构造真实硬件错误；软件负责寄存器、中断、清除和计数观察，不能用软件模拟错误冒充覆盖。

历史设计中的`EDA-FW-EHSM-MASTER-001`已从在线最终表删除，不再计入63条Case。eHSM真实Master的数据权限和三个Firewall仅启动核可配置仍是产品约束，但相关负向覆盖需要Case Owner另行在最终Case或RTL-DV/集成Evidence中闭环。

`SOC-SECCFG-DBG-001`需要Debug模块协同验证真实从DieDebug通路，但仍属于`056`软件/协同Case，不重复计为EDA Case。

## 2. 软件与EDA分工

- 软件适合完成：正式接口配置、SEC_CFG读取、ISR、批准的mask/clear、计数读取和Evidence整理。
- EDA必须完成：普通软件无法合法构造的watchdog/OTP CRC/TRNG/ECC错误、原始脉冲和组合源。
- 软件观察器存在不等于软件可以制造错误；EDA波形存在也不能替代SoC软件可见寄存器/IRQ闭环。
- 禁止修改eHSM固件、增加注错命令、使用软件伪中断或由启动核伪造eHSM Master ID。

## 3. 最终EDA Case表

| 最终编号 | 追踪ID | RTL-DV/EDA激励 | 软件承担 | 通过判据 |
|---:|---|---|---|---|
| 062 | `EDA-ERR-CRITICAL-IRQ-001` | 逐源及组合注入watchdog、OTP Key CRC、TRNG异常和不可纠正ECC | 读取`o_hsm_err_hw`映射、记录IRQ、执行批准清除 | 目标bit、组合中断、清除、重触发和无无关位符合合同 |
| 063 | `EDA-ERR-ECC1B-IRQ-001` | 对PKE0～3、KMU、DRAM、IRAM、IROM逐源注入ECC 1-bit，并覆盖连续/密集注入 | 记录独立IRQ，读取锁存/计数并清除 | 脉冲转可查询状态、计数、清除、重触发和计数边界符合合同 |

飞书两条Case的“输入”目前只写“EDA覆盖？”。执行计划必须在开始回归前补齐注入点、预期位、IRQ、清除寄存器、计数器、波形和Evidence路径，但不能改变Case编号或把缺失输入猜成默认值。

## 4. 严重错误组合中断

覆盖源包括：

- `wdt_timeout`；
- `otp_key_crc_err`；
- `hw_trng_retry_warning`、`hw_trng_retry_fail`、`hw_trng_ht_fail`；
- `mem_ecc_mb_pke3/2/1/0`、`mem_ecc_mb_kmu`、`mem_ecc_mb_dram`、`mem_ecc_mb_iram`、`mem_ecc_mb_irom`。

EDA先建立无错误基线，再逐源检查唯一目标bit和组合IRQ，随后做代表性多源同时注入；按冻结合同清除或撤销错误源，并再次注入验证可重触发。正常Mailbox业务失败不能替代这些硬件故障。

## 5. ECC 1-bit独立中断和计数

覆盖PKE0～3、KMU、DRAM、IRAM和IROM。每个源至少验证一次可纠正错误的原始脉冲、SoC外部锁存/查询、独立IRQ、软件清除和再次触发；连续/密集注入还要验证计数器宽度、饱和或回绕以及复位语义。

如果RTL只提供IRQ而没有冻结计数器，ISR次数可能因合并或屏蔽而少计，不能宣称满足“知道发生几次”的要求。

## 6. Firewall覆盖缺口的处理

当前在线Case 057/058/059验证默认数据访问，Case 060验证启动核成功重配SRAM五Region。它们没有完整执行“eHSM和其他核写三个Firewall配置窗口均被拒绝”的负向矩阵。

该产品约束不能恢复为已删除的`EDA-FW-EHSM-MASTER-001`并假装属于最终表。后续应由Case Owner明确选择并留痕：

1. 把负向配置Owner步骤加入批准的最终Case；或
2. 建立不计入当前63条Case的RTL-DV/集成补充Evidence，并在发布覆盖矩阵中映射到产品约束。

未完成前，Firewall配置Owner负向覆盖状态为`OPEN_COVERAGE_GAP`。

## 7. 当前阻塞输入

- 严重错误组合IRQ号、极性、锁存/清除和复位语义；
- ECC 1-bit IRQ号、脉冲转电平/锁存电路和清除CSR；
- ECC计数器CSR、宽度、饱和/回绕和复位语义；
- RTL-DV注入点、DUT baseline、Owner、回归入口和Evidence位置；
- Firewall配置Owner负向覆盖的最终责任和载体。

上述输入缺失时相关条目保持`NOT_EXECUTED/INCONCLUSIVE`，不能登记PASS。
