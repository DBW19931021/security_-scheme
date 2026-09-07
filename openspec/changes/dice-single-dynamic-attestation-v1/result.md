# Result

- 设计结果：BootROM/FMC只贡献FMC/GSP digest，GSP生成一级动态Firmware Alias证书。
- 静态证书：Root→Intermediate→Device Attestation Issuer，继续离线生成并保存在Cert0/1。
- 动态证书：GSP组装、slot14签发、当前boot SRAM驻留；eHSM派生Alias Key并签Report。
- SRAM：Measurement仍为16 KiB专用区；GSP新增动态证明峰值预算不超过12 KiB。
- 产品代码：未修改、未授权。
- 未完成：eHSM command/Key Attribute、企业OID/DN、SPDM wire和真实链/EMU Evidence。
