# Firewall测试条目

- `NGU800P-firewall-detailed-cases-v0.1.csv`：49项参数化验证条目，40 READY、9 BLOCKED，全部`NOT_EXECUTED`。
- 49条条目是Firewall模型/RTL-DV专项参考，不是独立测试基线，也不直接派生顶层Case。v0.3.8顶层只保留`SOC-FW-SECCFG-001`、`SOC-FW-SPIFC-001`、`SOC-FW-SRAM-001`和`SOC-FW-SRAM-RECFG-001`。
- 三个Firewall都只有启动核可配置，eHSM和其他核均无配置权限；SEC_CFG/SPIFC不执行成功重新配置；两个SRAM Case各自在Case内部遍历Region0～Region4。
- 当前评审工作簿：`../NGU800P-security-test-cases-v0.3.6-firewall-review.xlsx`；它保留检测到的v0.3.6实现回填内容，并增加`Firewall用例`和`Firewall UserId`工作表。
- 最终可执行case进入`../baremetal`前，必须先关闭或显式处理OPEN-CONFLICT-014，并由任务负责人授权实现。
