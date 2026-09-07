# ADR-0034：非安全制造灌装、eHSM BL密钥安装与USER最终提交

- 状态：accepted
- 日期：2026-09-02
- 决策人：项目负责人
- 关联：ADR-0025、ADR-0026、ADR-0031、`device-personalization-and-provisioning-v1`
- 替代关系：细化ADR-0026的制造启动和掉电恢复合同；不改变ADR-0031中`non_sec_boot=1`只能进入受限非安全路径的权限边界

## Context

设备采用一机一密。制造时需要由C908侧工装固件编排OTP/eFuse和证书灌装，但Key安装、对象状态和Lifecycle最终权限必须由eHSM内部执行。项目同时保留安全/非安全两个BootROM顶层分支，以及`non_sec_boot`永久逃生策略位。

## Decision

1. BootROM仍只有`SECURE_BOOT`和`NON_SECURE_BOOT`两个顶层分支，不增加第三个启动模式。`NON_SECURE_BOOT`内部固定区分两个子Profile：
   - `MANUFACTURING_PROVISIONING`：仅当`non_sec_boot=0`、SoC LCS为DEV或MANU、latched `boot_pin.secure_boot[3]=0`并且制造Profile有效时选择；
   - `RESTRICTED_NONSECURE`：`non_sec_boot=1`、LCS异常、非制造组合或制造Profile缺失/非法时选择。该Profile不获得OTP/eFuse/KMU写能力。
2. 制造C908 Provisioning FW通过非安全分支加载。它是独立、构建隔离的制造镜像，但对eHSM BL仍是不可信caller；其签名或站点部署不能替代BL侧授权。
3. Vendor后续eHSM BL交付必须提供typed密钥安装和个性化API。C908只提交逻辑`object_id`、设备/Recipe绑定和受控材料；BL内部决定物理slot、Level、usage、顺序、`last_key`、写窗口和证明。接口至少覆盖能力/身份查询、对象状态查询、wrapped Key安装、内部Key生成、公钥导出、固定PoP、对象证明、finalize和相邻LCS转换；禁止raw OTP、raw slot、任意Key Attribute、任意内存和任意消息签名。
4. BL必须同时验证当前LCS、硬件锁存的制造启动条件、signed recipe/ticket、设备绑定、固定对象映射、依赖和顺序。C908、Host或制造Controller不能单独授权不可逆写入。
5. 对象状态固定为`BLANK`、`PROGRAMMING_PARTIAL`、`PROGRAMMED_INVALID`、`PROGRAMMED_VALID`、`PROVED`、`LOCKED`、`UNKNOWN`。Vendor交付必须定义每个不可逆步骤的接受点、掉电点、readback、operation proof和部分写恢复语义。
6. OTP对象优先按“Key Data → CRC/ECC辅助字段 → readback → Attribute/valid/lock/`last_key`最后提交”实现。只有同一device、recipe、object和材料完全一致，剩余OTP bit仍满足单向编程且backend明确支持续写时，才允许从`PROGRAMMING_PARTIAL`恢复；否则设备隔离或报废，禁止盲重试、覆盖或换材料续写。
7. 设备保持DEV完成RTL个性化、全部Root/Key/UDS、Device Key、静态证书、对象证明和产品安全启动预演。DEV→MANU前必须完成全部Level1，并原则上完成全部目标Key对象及证明；MANU只执行已批准的收口、锁定、最终检查和相邻Lifecycle转换。
8. `MANU→USER`是量产最终提交。提交USER前必须以最终Key、最终证书和最终策略执行reset/reload及与量产一致的完整安全启动、Measurement、动态证书和SPDM/签名证明；所有证据通过后才调用BL typed LCS转换并readback。`LCS=USER last`是必要条件，但不能替代半写恢复、LCS掉电语义和最终产品证明。
9. 在DEV中的普通中断可以重新由pin进入`MANUFACTURING_PROVISIONING`，先查询对象状态，再按第5～6条继续或隔离。MANU中的重入只允许完成收口白名单，不得补写需要DEV权限的Level1对象。
10. `non_sec_boot=1`始终选择`RESTRICTED_NONSECURE`，即使在USER也不开放制造灌装。若产品要求在安全启动完全失效后仍可烧写该位，必须提供不依赖产品FMC/GSP或C908 Provisioning FW成功启动的强授权维修路径，例如Secure ATE/专用维修端口或不可变eHSM BL窄操作；该硬件路径未绑定前，不得对外声明具备USER态“起不来后再开启”的逃生能力。

## Consequences

- BootROM只增加非安全子Profile选择和独立加载配置；它不链接OTP写驱动，也不直接调用Key安装API。
- eHSM BL typed Provisioning API、部分写状态机、LCS提交语义和硬件制造授权是量产实现门禁；当前Vendor实现未具备的部分不得描述为已完成。
- 受限非安全和制造Provisioning共用顶层`NON_SECURE_BOOT`枚举，但必须使用不同stage profile、权限表、镜像源和测试Expected。
- `non_sec_boot`仍是永久安全降级位，不成为通用OTP入口。

## 设计落点

- [《NGU800P安全软件详细设计》第6章 BootROM安全启动详细设计](../docs/05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)
- [《NGU800P安全软件详细设计》第10章 Lifecycle、Key、OTP/eFuse、证书与轮换](../docs/05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)
- [《BootROM软件设计》专题](../docs/05-software-design/bootrom.md)
- [《安全配置与注入》操作专题](../docs/07-operations/provisioning.md)
