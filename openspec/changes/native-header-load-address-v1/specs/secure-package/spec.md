# Secure Package Delta

## Requirement: No NGU Manifest

NGU800P type 1 SoC package必须由1024字节Native Header直接连接`Code_Size`字节firmware Code Region，不得存在NGU Manifest或第二项目Header。

### Scenario: Legacy Manifest package

- Given 包的Code Region以旧Manifest开始
- When 产品loader接收该包
- Then 必须拒绝
- And 不得fallback到旧parser

## Requirement: Authenticated load address and Header CRC overlay

Native Header偏移1008必须为LE64 `load_addr`，偏移1016必须为覆盖Header offset 256～1015的LE32 CRC-32/ISO-HDLC，偏移1020～1023必须全0；三者必须进入Vendor签名/CMAC输入。

### Scenario: Header tail tampered

- Given 已签名包的load地址、Header CRC或reserved被修改
- When eHSM验证该包
- Then 验证必须失败
- And 不得load、Measurement或release

### Scenario: Header CRC mismatch before submission

- Given Header offset 256～1015任一字节损坏或offset 1016的CRC错误
- When 产品preflight执行
- Then 必须以NOT_SUBMITTED拒绝
- And CRC通过不得被解释为密码认证成功

## Requirement: Stage-owned policy

镜像类型、实例、die、Profile、Lifecycle和release policy必须来自typed stage context、OTP和release matrix；包不得自声明这些语义。

### Scenario: Valid address for wrong stage

- Given Header地址位于安全RAM但不等于当前stage固定目标
- When preflight执行
- Then 必须在提交eHSM前拒绝

## Requirement: Code Size is the loaded image size

`Code_Size`必须等于包长减1024，并同时作为认证、解密、copy、range和Measurement长度；CBC零对齐必须视为Code Region一部分。

### Scenario: Hidden payload length assumed

- Given caller试图在设备侧去除未编码的padding或使用另一payload size
- When loader构造copy或Measurement请求
- Then 必须拒绝该请求
