# Proposal

## 背景

SRC-0033要求删除NGU Manifest并复用Vendor Native Header `Public_Key_Ext`末尾16字节；SRC-0036进一步把后8字节定义为4字节Header CRC和4字节reserved。CE-SEC-016确认当前Vendor实现只消费该字段前384字节，OTP公钥Key ID只哈希按算法重组后的公钥而不哈希原始400字节扩展区，且尾部16字节处于签名/CMAC覆盖范围。

## 目标

- 固定无Manifest的唯一SoC stage包格式。
- 固定偏移1008的LE64 `load_addr`、偏移1016的LE32 Header CRC32和偏移1020的4字节零reserved。
- 固定CRC-32/ISO-HDLC参数及offset 256～1015覆盖范围，并在preflight和Vendor PASS后双阶段校验。
- 把原Manifest语义迁移到Native Header、typed stage context、OTP和release matrix。
- 保留精确长度、真实验签/解密、源/目标摘要、Measurement和release门禁。
- 明确旧Manifest包不兼容且不提供fallback。

## 非目标

- 不修改Vendor Native Header总长度、原生Image_Type、签名/加密算法或Version_Counter。
- 不修改type 0 eHSM FW包。
- 不在reserved中增加version、entry、flags或payload size；Header CRC不承载业务语义。
- 本change不授权产品代码修改。

## 审批状态

ADR-0030/0033已接受设计。实现需单独授权并取得最终release tool、link map、PMA/Firewall和真实eHSM E2E Evidence。
