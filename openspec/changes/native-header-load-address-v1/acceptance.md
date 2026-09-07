# Acceptance

## 设计验收

- type 1包恰好等于`1024+Code_Size`，Code Region从1024开始。
- Header 1008～1015是LE64 System Address，1016～1019是LE32 CRC-32/ISO-HDLC，1020～1023全0。
- Header CRC固定覆盖offset 256～1015；制包工具在签名前写入，preflight和Vendor PASS后均重新计算。
- `entry_addr=load_addr`，不存在Manifest parser或兼容fallback。
- 原Manifest字段均有唯一stage/Header/OTP/release-matrix来源。
- preflight和PASS后都执行stage target exact-match，PASS前不信任Header。
- Measurement覆盖完整`Code_Size`，包含CBC零对齐。

## 实现验收场景

1. load地址任一bit篡改导致Header CRC预检失败，并在绕过预检的故障注入场景中导致Vendor验签失败。
2. CRC错误、CRC覆盖字段任一bit损坏或reserved任一非0拒绝；CRC通过不得绕过Vendor验证。
3. load地址虽在安全RAM内但不等于当前stage固定目标时，在提交eHSM前拒绝。
4. `Code_Size`与实际包长不等、非16字节对齐、加法溢出或越过目标Region时拒绝。
5. 旧Manifest包、双parser fallback和Host自选image kind/instance/target均拒绝。
6. 源摘要、memmove、目标回读摘要、Measurement和release顺序可被故障注入逐点验证。
7. P-256、SM2、RSA-2048及当前支持的RSA-3072公钥解析路径均不消费1008～1023；未来算法未重新证明时构建阻断。
