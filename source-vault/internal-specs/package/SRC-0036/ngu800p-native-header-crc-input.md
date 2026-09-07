# NGU800P Native Header CRC字段输入

- Source ID：SRC-0036
- 日期：2026-09-01
- Owner：项目负责人
- 状态：用户确认的设计输入

## 用户确认内容

1. 在Native Header原`ngu_reserved[8]`空间中增加CRC字段，用于Header格式校验。
2. `load_addr`及Native Header总长度保持不变。

## 工程解释

- offset 1016～1019定义为`ngu_header_crc32`，采用LE32存储。
- offset 1020～1023继续作为`ngu_reserved[4]`，发布时全0，接收时任一非0拒绝。
- CRC算法固定为CRC-32/ISO-HDLC；输入为Native Header offset 256～1015共760字节，不包含offset 0～255的`Signature`，也不包含CRC字段自身。
- 发布工具在Header其他字段全部定稿后计算并写入CRC，再执行Vendor兼容签名；CRC字段和末尾reserved仍位于Vendor签名/CMAC覆盖范围。
- BootROM/FMC/GSP在提交eHSM前执行CRC预检，并在Vendor PASS后的稳定认证Header上重新计算和比较。
- CRC只用于格式损坏和传输误码的早期筛查，不提供密码认证，不替代Vendor签名/CMAC、typed-stage policy或完整Code验证。
- 本输入只适用于NGU800P Vendor type 1 SoC stage包；Vendor type 0 eHSM FW不解释该Overlay。

