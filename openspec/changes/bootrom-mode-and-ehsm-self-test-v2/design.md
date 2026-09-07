# Design

## 模式选择

`USER -> SECURE`；已识别非USER按SRC-0023的`boot_pin.secure_boot[3]`选择：0为`NON_SECURE_BOOT`，1为`SECURE_BOOT`。值0时DEV/MANU绑定`MANUFACTURING_PROVISIONING`，其他LCS绑定`RESTRICTED_NONSECURE`；LCS失败、非法、UNDEFINED、来源不可信或策略组合未批准统一选择`RESTRICTED_NONSECURE`。两个非安全子Profile使用独立镜像/权限/release配置且禁止fallback。

当前SRC-0022生成头仍把bit0命名为`SEC_BOOT`、bit3命名为`DIE_ID`。产品策略已定，但真实寄存器绑定和EMU Expected保持`BLOCKED_BY_RTL_SYNC`；不得直接使用当前bit0、当前bit3或私有裸位号。

## 非安全路径

非安全路径独立于安全路径，不调用安全FMC验签/Measurement/release链，不写启动审计。它不得开放OTP、生产Key、counter update、raw eHSM、受保护Debug或安全RAM明文区。

## 自检

eHSM BL读取自身eFuse配置并自主执行自检。BootROM不发送start-self-test，只等待Bootloader ready/error并读取raw status/bitmap。

## 失败

eHSM要求自检且结果失败时安全路径不得release FMC。eFuse指示不执行自检时，不以“未执行”判失败。

## 相关设计

- [主详设第6章](../../../docs/05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)
- [BootROM专题](../../../docs/05-software-design/bootrom.md)
