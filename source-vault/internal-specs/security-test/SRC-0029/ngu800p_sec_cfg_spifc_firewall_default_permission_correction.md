# NGU800P SEC_CFG/SPIFC Firewall 默认权限修正

## 1. 来源信息

- 日期：2026-08-14
- 提供方：项目负责人
- 适用对象：NGU800P D0 SEC_CFG、SPIFC和SRAM Firewall安全方案测试用例。
- 证据状态：DOCUMENTED

## 2. 修正后的默认权限

1. SEC_CFG Firewall默认仅允许启动核读写；eHSM和其他所有核均无权访问。
2. SPIFC Firewall默认仅允许启动核读写；eHSM和其他所有核均无权访问。
3. SEC_CFG和SPIFC Firewall用例只验证默认权限，不修改Firewall配置。
4. SRAM Firewall规则不变：Region0～Region4默认允许启动核和eHSM读写，其他核无权限；默认权限和重新配置仍为两个顶层Case，每个Case内部遍历5个Region。

## 3. 用例影响

- `SOC-FW-SECCFG-001`：启动核访问成功；eHSM及其他核访问被拒绝，写操作无副作用。
- `SOC-FW-SPIFC-001`：启动核访问成功；eHSM及其他核访问被拒绝，写操作无副作用。
- `EDA-FW-EHSM-MASTER-001`：按目标区分预期，eHSM访问SEC_CFG/SPIFC应被拒绝，访问SRAM Region0～4应成功。

## 4. 与既有来源的关系

本来源只替代`SRC-0027`第3节第1项中“SEC_CFG和SPIFC默认允许eHSM访问”的表述，不替代`SRC-0027`的其他测试范围、合并原则或软件/EDA分工。
