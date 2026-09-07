# SRC-0027 SoC安全测试范围入库报告

## 入库结果

- 原样保存2张截图，并登记SHA-256。
- 记录SEC_CFG/Debug、三类Firewall、Mailbox IRQ和错误中断的最终测试范围裁决。
- 用于收敛统一测试工作簿、Codex移植指导和EDA验证需求；不修改baremetal或eHSM固件。

## 可进入测试设计的内容

- `dbg_en_cfg`与`soc_dbg_en_out`的职责和判据分离。
- SEC_CFG全寄存器单用例遍历、SEC_CFG/SPIFC默认权限、SRAM默认权限与重新配置两个用例各自覆盖5个Region。
- 严重错误组合中断、ECC 1-bit独立中断和外部计数由EDA构造，软件承担观察/清除/证据采集。

## 仍需绑定

- SEC_CFG/Firewall地址、Master ID、IRQ和清除寄存器。
- ECC 1-bit计数器CSR及宽度、溢出、清除和复位语义。
- eHSM Master实际访问路径；没有现有公开路径时由EDA直接补充。

