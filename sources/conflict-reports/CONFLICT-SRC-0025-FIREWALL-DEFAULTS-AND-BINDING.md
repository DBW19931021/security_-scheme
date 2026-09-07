# SRC-0025 Firewall 默认值与 RTL 绑定冲突

## Identity

- Conflict ID: CONFLICT-SRC-0025-FIREWALL-DEFAULTS-AND-BINDING
- Open Question: OPEN-CONFLICT-014
- Status: open
- Evidence state: CONFLICTING
- Owner: 项目负责人；RTL/Firewall/IP/寄存器 Owner 待指定
- Affected sources: SRC-0025, SRC-0022, SRC-0017, SRC-0016

## 冲突与缺口

1. 逻辑需求称 Firewall 总开关默认关闭；寄存器表称 SRAM/sec_cfg/spifc 的 `check_en/hide_en` reset 均为 1。
2. R1 `0x05000000..0x057FFFFF` 覆盖 R2-R5；需求又规定多 Region 命中失败，全部 Region reset enable=1 时二者不能同时作为可用默认态。
3. R4 authority `0x00000F00` 按位图是 Codec MCU，逻辑表称 MM Core。
4. bit27 应按压缩公式表示 die1 eHSM read，截图字段说明疑似写为 die0。
5. base 仍为 `0xxx_0000` 占位；当前 `SRC-0022` 生成头中未找到该 Firewall CSR 组。
6. 48-bit 高地址 CSR、安全属性 CSR、lock/reset retention、burst 与错误覆盖策略未给出。

## 允许继续范围

- 完整 UserId 枚举、authority 位计算和 reserved 防护。
- 软件 policy schema、静态校验、最小权限、读回、失败关闭和测试设计。
- 对截图默认值做“观测/对比”测试，不做最终 PASS/FAIL。

## 必须停止范围

- 产品 MMIO base/offset/field 头文件和初始化写序列。
- 将截图 Region 地址/authority/reset 直接编为量产 policy。
- 使用截图默认值形成 EMU/RTL sign-off oracle。

## 关闭所需 Evidence

1. 与目标 D0/tape-out revision 匹配的 RTL instance、CSR 生成源和寄存器版本。
2. 完整 48-bit 地址字段、安全属性、lock/reset/retention 和 burst 规范。
3. reset 仿真或门级/EMU readback，说明开关与 Region 默认态。
4. 唯一命中、多命中、Hide、Slave select 抑制和错误锁存的 DV Evidence。
5. R4 Master 命名与 UserId 网表/NoC 属性映射签核。

## 推荐方案

- 保留截图 UserId/authority 编码为 `DOCUMENTED` 候选，并生成单一机读映射供软件与测试共享。
- 量产初始化采用显式 policy：先隔离请求源，校验并写入互不重叠的地址/最小 authority，读回后最后打开 `check_en`；不能依赖冲突中的硬件默认值。
- RTL/寄存器 Owner 提供完整生成头后，按 `SRC-0022` 更新规则登记新 Source revision，并关闭本冲突或拆分残余问题。
