# Impact

- 替代ADR-0007旧P1容量拓扑，保留其隔离、W^X和生命周期原则。
- 删除独立plaintext物理区；Host仍只能写Ingress。
- MMP从2 MiB SRAM驻留集合移至DDR边界。
- linker/layout生成器需要新增固定offset、FMC/GSP overlay和原地加载断言。
- Measurement OpenSpec的物理Region由开放变为16 KiB；实际`max_fw_entries`仍由启动拓扑生成。
- PMA/Firewall和MMP DDR Profile未关闭前，不授权产品编码。

