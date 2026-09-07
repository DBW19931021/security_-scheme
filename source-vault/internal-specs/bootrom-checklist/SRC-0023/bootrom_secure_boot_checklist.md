# BootROM Secure Boot重点Checklist与Strap截图转录

## Source metadata

- Source ID：SRC-0023
- 来源：项目负责人于2026-07-24在Codex对话中提供的两张截图
- 原图1：`C:\Users\admin\AppData\Local\Temp\codex-clipboard-0b5893f7-337d-4158-b0b3-6e7a16643f6f.png`
- 原图1大小：278699 bytes
- 原图1 SHA-256：`ac262c0a6644064ef5b17b89f8a2489a4ed2c0ebdc72cc8c9a6d6067fa2077a5`
- 原图2：`C:\Users\admin\AppData\Local\Temp\codex-clipboard-74ada638-9f77-4c7b-bbf8-218be3e7e8b2.png`
- 原图2大小：54440 bytes
- 原图2 SHA-256：`f18ef7c16fe6e0d51fc2eaa1c2e0530e7868a2f00c8608c3cbf797c665901d2a`
- 转录状态：人工逐行转录并与原图复核
- 证据状态：`PROPOSED`；它是负责人列出的重点Checklist，不自动覆盖SRC-0022硬件数值或accepted ADR

## 截图1：安全启动Checklist

| 分类 | ID | 名称 | 原图说明转录 |
|---|---|---|---|
| 安全启动 | `secureboot.001` | 安全启动模式判定 | 读取`secure_boot` Strap和Lifecycle状态。LCS=USER时必须强制安全启动；其他生命周期按批准的Strap策略决定。LCS读取失败、非法或UNDEFINED时按安全失败处理，不得进入非安全启动。 |
| 安全启动 | `secureboot.002` | 等待eHSM Ready | 安全启动模式下，BootROM等待eHSM完成Bootloader启动、自检、eFuse Autoload和Lifecycle加载。等待必须使用Timer实现有界超时；eHSM未Ready、自检失败、状态异常或Mailbox超时均不得继续启动FMC，不允许旁路eHSM。 |
| 安全启动 | `secureboot.003` | FMC固件验签及强制解密 | BootROM调用真实eHSM `VERIFY_IMAGE`接口。由eHSM检查Header、Image Type、Plain Flag、Code Size、Key ID、Signer、Key Revoke、Hash及Signature，并对受保护Code Region执行认证和解密。正式安全启动中的FMC必须签名并加密，禁止打桩。 |
| 安全启动 | `secureboot.004` | FMC版本防回滚检查 | eHSM将FMC Header中的`Version_Counter`与安全eFuse中的`global_counter`比较；镜像版本低于可信计数器、计数器读取失败或检查结果未知时必须拒绝启动。 |
| 安全启动 | `secureboot.005` | Protected Manifest策略检查 | 仅在eHSM返回PASS后解析解密得到的NGU Protected Manifest。检查Manifest各字段合法性；任一检查失败不得跳转。 |
| 安全启动 | `secureboot.006` | FMC启动度量 | 验签、解密和Manifest检查全部通过后，记录FMC真实Measurement，包括镜像类型、版本、明文Digest、验签结果、Load/Entry信息等。 |
| 安全启动 | `secureboot.007` | FMC受控加载、跳转及失败闭锁 | 将FMC Payload加载到批准区域，确认Measurement记录，一次性跳转FMC Entry。任何阶段失败均记录`boot_fail_reason`、原始eHSM状态和Manifest状态，不跳转FMC，进入Halt/Wait Reset，等待掉电复位或OOB重刷。 |

## 截图2：secure_boot Strap字段

| strap_pin | bit | 数据来源 | default | 说明 |
|---|---|---|---:|---|
| `secure_boot` | `[3]` | `boot_pin.secure_boot` | 0 | 值为0：非安全启动；值为1：安全启动 |

## 转录使用规则

1. `secureboot.001～007`作为第6章BootROM重点Checklist和后续测试追溯输入。
2. `secureboot.002～007`必须与既有Package、eHSM Adapter、Measurement、RAM/Firewall和RAS裁决合并解释，不能建立第二套公共ABI。
3. 截图2的`boot_pin.secure_boot[3]`与SRC-0022当前软件可见`STRAP_PIN_SEC_BOOT_POS=0`存在待解释差异；在确认它们是否属于不同接口/映射之前，不把`[3]`写成BootROM寄存器常量。
4. “Halt/Wait Reset/OOB重刷”按既有ADR解释为失败后不可到达FMC、security只上报RAS；reset由RAS执行，OOB能力仍需Recovery方案批准。
5. Measurement按现有批准合同分两步：验证后准备记录，loader完成后使用实际load/entry提交；不能在加载前伪造实际地址。

