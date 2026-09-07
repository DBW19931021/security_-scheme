# NGU800P Firewall配置权限修正

## 1. 来源信息

- 日期：2026-08-14
- 提供方：项目负责人
- 适用对象：NGU800P D0 SEC_CFG、SPIFC和SRAM Firewall配置权限测试。
- 证据状态：DOCUMENTED

## 2. 配置权限

1. SEC_CFG、SPIFC和SRAM三个Firewall的配置寄存器均只有启动核有权配置。
2. eHSM和其他所有核均无权修改任何Firewall配置。
3. Firewall配置权限与受保护目标的数据访问权限是两个独立维度：eHSM默认可访问SRAM数据，不代表eHSM可以配置SRAM Firewall。
4. SEC_CFG和SPIFC不执行成功重新配置；相关测试保持默认配置。
5. SRAM由启动核执行批准的地址范围和允许Master ID重新配置，并在同一Case内遍历Region0～Region4、验证后恢复原配置。

## 3. 用例影响

- `SOC-FW-SRAM-RECFG-001`：在既有Case内部增加配置Owner判定。启动核配置SRAM成功；软件可控的非启动核配置尝试被拒绝且配置不变。SEC_CFG/SPIFC不执行成功重新配置。
- `EDA-FW-EHSM-MASTER-001`：没有公开软件路径时，由真实eHSM Master补充验证三个Firewall配置写均被拒绝且无配置副作用。
- 不新增顶层Case，不改变`SOC-FW-SECCFG-001`、`SOC-FW-SPIFC-001`和`SOC-FW-SRAM-001`的数据访问权限判据。

## 4. 与既有来源的关系

本来源补充`SRC-0029`的数据访问权限修正，明确三个Firewall的统一配置Owner。若既有材料使用泛化的“C908可配置”，应解释为“承担启动核角色的C908可配置”，不能扩大到其他C908或eHSM。
