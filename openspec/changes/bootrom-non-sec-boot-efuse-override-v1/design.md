# Design

## 选择顺序

BootROM在最小平台初始化后读取一次只读Boot Policy Fuse快照：

```text
VALID/ECC_OK && non_sec_boot=1 -> RESTRICTED_NONSECURE
invalid/read failure/ECC error/mirror mismatch -> BOOT_POLICY_INPUT_ERROR
VALID/ECC_OK && non_sec_boot=0 -> existing LCS × secure_boot policy
```

值1覆盖USER强制安全和所有Strap组合；值0不改变既有模式矩阵。

## 硬件/软件边界

BootROM只消费复位稳定的只读逻辑快照，不取得任意offset eFuse接口。准确word/bit、编码、ECC、valid、复位锁存、寄存器/API和烧写Owner由OPEN-DESIGN-024绑定；缺失时产品集成为`BLOCKED_BY_NON_SEC_BOOT_BINDING`。

## 非安全路径

强制分支复用既有`NONSECURE_FMC` Profile，不等待eHSM，不执行安全FMC验签/解密、counter、Measurement、SoC State或安全启动审计。非安全Profile缺失时终止，不回退安全启动。

## 相关设计

- [主详设第6章](../../../docs/05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)
- [OTP/eFuse接口](../../../docs/04-interfaces/otp-efuse.md)
- [BootROM专题](../../../docs/05-software-design/bootrom.md)
