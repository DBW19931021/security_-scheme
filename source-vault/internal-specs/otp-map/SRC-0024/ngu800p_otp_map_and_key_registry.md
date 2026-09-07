# NGU800P OTP内存映射与16-slot Key表转录

## Source metadata

- Source ID：SRC-0024
- 来源：项目负责人在2026-08-04 Codex对话中提供的OTP内存映射截图、16-slot Key表截图及后续明确裁决
- 适用目标：`security_-scheme`，作为NGU800P SoC安全方案的OTP布局和Key对象表输入
- 同步规则：`baremetal/components/security`只能派生、实现和验证本表，不建立独立方案基线
- 转录状态：根据截图逐项转录；截图未给出的内容保持“未给出”，不得猜测
- 原图状态：本工作空间未保存可复核的原始图片文件，无法登记原图hash；本转录由项目负责人后续文字裁决确认其目标和权威关系

## OTP内存映射

eHSM内部OTP基地址为`0x33000000`。下表的offset和地址用于eHSM内部OTP访问；GSP没有安全域OTP直访通道，只能通过eHSM Mailbox请求访问。

| Field | Offset | Size/Word | Multi-update | Decoded by |
|---|---:|---:|---|---|
| Life Cycle | `0x000` | 1 | yes | HW |
| UID | `0x004` | 5 | no | FW |
| HW Control | `0x018` | 2 | yes | HW |
| FW Control-eHSM | `0x020` | 2 | yes | HW |
| FW Control-SoC | `0x028` | 2 | yes | HW |
| Error Response Control | `0x030` | 8 | no | FW |
| eHSM Version Counter | `0x050` | 4 | yes | FW |
| SoC Version Counter | `0x060` | 4 | yes | FW |
| OTP Key N Attribute | `0x070 + 0x4 * 0xA * N` | 1 | yes | HW |
| OTP Key N | `0x070 + 0x4 * (0xA * N + 0x1)` | 8 | no | HW |
| OTP Key N CRC | `0x070 + 0x4 * (0xA * N + 0x9)` | 1 | no | HW |

解释：`Word=4 bytes`，`N=0..15`。每个Key record为10 words/40 bytes，stride为`0x28`；Key区覆盖offset `0x070..0x2EF`。

## 16-slot Key表

| ID | Name | 等级 | Key类型 | 位置 | 备注 | 已提供密钥权限 |
|---:|---|---:|---|---|---|---|
| 0 | Chip root key | 0 | symm | OTP-KMU | 芯片根密钥 | 未给出 |
| 1 | Device root key | 1 | symm | OTP-KMU | 设备根密钥 | 未给出 |
| 2 | USER root key | 1 | 未给出 | OTP-KMU | 未给出 | 未给出 |
| 3 | eHSM debug/verify key | 1 | asymm | OTP-KMU | eHSM鉴权验签密钥 | 未给出 |
| 4 | eHSM FW/update verify key | 1 | asymm | OTP-KMU | eHSM镜像验签密钥 | 未给出 |
| 5 | eHSM FW/update encrypt key | 1 | symm | OTP-KMU | eHSM镜像解密密钥 | 未给出 |
| 6 | SoC FW/update verify Rotation key | 2 | asymm | OTP-KMU | SoC验签轮换密钥 | 未给出 |
| 7 | SoC FW/update encrypt Rotation key | 2 | symm | OTP-KMU | SoC解密轮换密钥 | 未给出 |
| 8 | DICE root CA key | 2 | asymm | OTP-KMU | DICE root CA key，用于追溯 | 七项权限 |
| 9 | SoC debug verify key | 2 | asymm | OTP-KMU | SoC鉴权密钥 | 未给出 |
| 10 | SoC FW/update verify key | 2 | asymm | OTP-KMU | SoC镜像验签密钥 | 未给出 |
| 11 | SoC FW/update encrypt key | 2 | symm | OTP-KMU | SoC镜像解密密钥 | 未给出 |
| 12 | SoC debug verify Rotation key | 2 | asymm | OTP-KMU | SoC鉴权轮换密钥 | 未给出 |
| 13 | UDS | 2 | asymm | OTP-KMU | UDS | 七项权限 |
| 14 | Device private Key | 2 | asymm | OTP-KMU | DICE设备私钥，attestation key | 七项权限 |
| 15 | User auth key | 2 | asymm | OTP-KMU | 未给出 | 未给出 |

截图对slot 8、13、14给出的七项权限为：

1. 密钥可以用于签名，或生成MAC；
2. 密钥可以用于验签或验证MAC；
3. 密钥可以用于加密；
4. 密钥可以用于解密；
5. 密钥可以用于派生或者协商出新密钥；
6. 密钥允许删除；
7. 密钥允许明文导入。

## 使用约束

1. 本表是`security_-scheme`中的SoC方案输入；baremetal中的CSV、C表、case绑定和fixture说明必须与本表一致。
2. “已提供密钥权限”记录OTP/KMU对象属性能力；产品服务是否对Host/GSP暴露仍受主详设的调用者、Lifecycle和typed service权限控制，但不得把未提供的权限反向写成Key表事实。
3. slot 2的Key类型、各slot具体算法、Key Attribute位编码、CRC参数、bit/byte endian、ECC、锁位、blank polarity和真实材料引用均未由截图给出，必须作为实施输入补齐。
4. 本表冻结物理对象顺序和等级，不代表当前eHSM BL/FW旧17-slot实现已经匹配；实现版本差异必须单独管理。
