# BootROM Mode Requirements

## Requirement: LCS异常进入受限非安全启动

BootROM在LCS读取失败、值非法、`UNDEFINED`、来源有效性无法证明或策略组合未批准时，必须选择受限非安全启动，不得进入安全FMC链。

### Scenario: LCS source untrusted

- Given BootROM无法证明LCS读取来源有效
- When 选择启动模式
- Then 结果为`RESTRICTED_NONSECURE`
- And 不创建Measurement或启动审计

## Requirement: USER强制安全启动

`non_sec_boot=0`且`LCS=USER`时，BootROM必须忽略`boot_pin.secure_boot[3]`并进入安全启动。`non_sec_boot=1`时按后续`bootrom-non-sec-boot-efuse-override-v1`强制受限非安全启动。

## Requirement: 非USER遵循截图Strap极性并选择固定子Profile

`non_sec_boot=0`时，已识别非USER LCS必须按SRC-0023的`boot_pin.secure_boot[3]`选择顶层模式：值0进入`NON_SECURE_BOOT`，值1进入`SECURE_BOOT`。值0时，DEV/MANU必须选择`MANUFACTURING_PROVISIONING`，其他LCS必须选择`RESTRICTED_NONSECURE`；两个子Profile不得fallback。

### Scenario: DEV/MANU manufacturing profile

- Given `non_sec_boot=0`、LCS为DEV或MANU且`secure_boot=0`
- When BootROM选择非安全子Profile
- Then 结果为`MANUFACTURING_PROVISIONING`
- And BootROM只加载独立Provisioning FW，不直接写OTP

### Scenario: Other non-secure lifecycle

- Given `non_sec_boot=0`、LCS不是DEV/MANU且策略允许`secure_boot=0`
- When BootROM选择非安全子Profile
- Then 结果为`RESTRICTED_NONSECURE`
- And eHSM BL制造接口不可达

### Scenario: RTL binding unavailable

- Given 当前RTL/生成头未提供与SRC-0023一致的命名字段或批准映射
- When 产品BootROM尝试读取Strap
- Then 平台输入无效并阻断真实模式绑定
- And 不得读取当前bit0、把当前`DIE_ID` bit3改释为`secure_boot`或使用私有裸位号

## Requirement: eHSM自主自检

eHSM必须依据自身eFuse策略决定并执行自检；BootROM不得发起自检命令，只能读取ready/error及raw自检状态。

### Scenario: eFuse disables self-test

- Given eHSM eFuse配置本次不执行自检
- When eHSM BL完成并报告ready且无error
- Then BootROM不得因“未执行自检”拒绝安全启动

### Scenario: Required self-test fails

- Given eFuse要求执行自检
- And eHSM报告自检失败或Bootloader error
- When BootROM评估ready
- Then FMC安全启动不得继续
