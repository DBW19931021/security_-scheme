# BootROM non_sec_boot Override Requirements

## Requirement: 默认值不改变既有启动策略

当BootROM可靠读取`non_sec_boot=0`时，必须继续执行既有SoC LCS与`boot_pin.secure_boot[3]`模式矩阵。

### Scenario: USER and deasserted override

- Given Boot Policy Fuse快照有效、ECC正常且`non_sec_boot=0`
- And SoC LCS为USER
- When BootROM选择启动模式
- Then 结果仍为`SECURE_BOOT`

## Requirement: 值1强制受限非安全启动

当BootROM可靠读取`non_sec_boot=1`时，必须忽略SoC LCS和`secure_boot`的原模式结果，选择现有`RESTRICTED_NONSECURE`路径。

### Scenario: USER and asserted override

- Given Boot Policy Fuse快照有效、ECC正常且`non_sec_boot=1`
- And SoC LCS为USER且`secure_boot=1`
- When BootROM选择启动模式
- Then 结果为`RESTRICTED_NONSECURE`
- And eHSM等待、安全FMC验签/解密、counter与Measurement状态均不可达

## Requirement: 启动策略位读取异常不得触发降级

BootROM无法证明Boot Policy Fuse快照有效时，必须进入`BOOT_POLICY_INPUT_ERROR`终态，不得release安全或非安全FMC。

### Scenario: ECC failure

- Given `non_sec_boot`只读快照报告ECC异常
- When BootROM评估启动策略
- Then 安全和非安全FMC release均不可达
- And 不得把异常值解释为0或1

## Requirement: BootROM只消费只读快照

BootROM必须只使用复位稳定、不可变、带有效性/ECC状态的命名平台接口，不得读取或写入任意eFuse offset。

### Scenario: RTL binding unavailable

- Given 目标D0资料未提供`non_sec_boot`准确word/bit/编码和只读视图
- When 构建产品BootROM平台port
- Then 对应绑定为`BLOCKED_BY_NON_SEC_BOOT_BINDING`
- And 不得使用截图推断、裸bit或临时常量

## Requirement: 强制分支保持最小权限

`non_sec_boot=1`只能选择现有受限非安全Profile，不得扩大资源权限。

### Scenario: Non-secure profile unavailable

- Given `non_sec_boot=1`
- And 产品未生成完整`NONSECURE_FMC` source/Region/privilege/release Profile
- When BootROM进入强制非安全分支
- Then 进入受限终态
- And 不得回退到安全启动或开放安全资源
