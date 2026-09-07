# SRC-0025 Firewall 入库报告

## 入库结果

- 已原样保存 7 张 PNG，并登记逐项 SHA-256。
- 已转录 SRAM、sec_cfg、spifc 的逻辑需求、寄存器 offset/字段/reset、16 个 UserId 和 authority 位图。
- 本来源补齐当前 `SRC-0022` 未覆盖的 Firewall 设计输入，但不覆盖 `SRC-0022` 的 SoC 数值权威规则。

## 可直接进入设计的内容

- `DOCUMENTED`：UserId 枚举、authority 压缩算法、地址检查与权限检查合取、唯一 Region 命中、Hide 响应、错误信息和写 1 清除语义。
- `PROPOSED`：软件 policy 校验、原子配置顺序、reserved UserId 拒绝、读回和 fail-close。

## 必须停止外推的内容

- Firewall 实例 base、48-bit 地址高 16 位 CSR、安全属性 CSR、lock/retention、burst 行为。
- 截图 reset 值、重叠 Region 默认地址、R4 Master 名称和 bit27 标签。
- 在上述冲突关闭前，不生成产品 MMIO 头、不冻结最终 RAM Region policy、不把截图默认值作为 EMU PASS oracle。

## 影响范围

- 架构：`docs/03-architecture/firewall-isolation.md`
- 主详设：`docs/05-software-design/NGU800P安全软件详细设计.md` 第 5.9 节
- 需求：`requirements/firewall-requirements.yaml`
- 可机读映射：`requirements/firewall-userid-authority-map.yaml`
- 验证：`docs/06-verification/firewall-test-design.md` 与统一测试工作簿后继版本

