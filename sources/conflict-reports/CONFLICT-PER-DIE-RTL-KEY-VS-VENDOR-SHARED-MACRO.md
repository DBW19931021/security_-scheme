# 一机一密RTL Key目标与Vendor共享宏集成方式的实施缺口

## Identity

- Conflict ID: CONFLICT-PER-DIE-RTL-KEY-VS-VENDOR-SHARED-MACRO
- Open Question: OPEN-CONFLICT-012
- Status: TARGET_ACCEPTED_IMPLEMENTATION_BINDING_PENDING
- Evidence state: CONFLICTING
- Owner: RTL/DFT Owner；eHSM集成Owner；KMS/制造Owner
- Related: SRC-0017、SRC-0018、SRC-0035、ADR-0025、ADR-0032、OPEN-DESIGN-014

## Classification

- Type: accepted_product_target_vs_current_integration_mechanism
- Affected scope: 量产RTL Root/Install KEK/Scan Key个性化、netlist复用、ATE灌装、硬件不可读性和KMS绑定
- Safe-to-continue scope: 详细设计、KMS对象模型、制造协议、测试计划和RTL输入清单
- Must-stop scope: 量产netlist定版、真实RTL Key灌装、进入USER和任何以共享宏值宣称完成一机一密的发布

## Accepted target

[ADR-0025《一机一密RTL Key与16槽OTP Key基线》](../../decisions/ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md)确认量产RTL Key按die唯一、只供硬件消费，KMS按设备托管对应句柄。

## Current evidence

当前Vendor HP TRM把`OSR_RTL_KEY`、`OSR_RTL_KEY_INSTALL_KEK_EHSM`、`OSR_RTL_KEY_INSTALL_KEK_SOC`和`OSR_SCAN_RTL_KEY`描述为集成时修改的RTL宏。该形式适合表达集成输入，但若同一量产netlist对全部die使用相同常量，不能实现一机一密。

SRC-0035进一步确认当前RTL快照在`rtl/osr_define_key.v:4-11`中以字面量宏提供上述四类输入，候选顶层实例链`rtl/osr_ehsm_top.v:2433`→`rtl/wrapper/osr_ahb_bus_wrapper.v:1828-2085`会到达Boot/KMU；KMU在`rtl/sys/osr_kmu.v:395-403`选择Scan或Install KEK宏，Boot解密路径在`rtl/sys/otp_1part/osr_boot.v:627`→`rtl/crypto_plain/osr_seip_kdec.v:171`选择Scan或RTL Key宏。报告只引用位置，不记录任何Key字面值。

由于SRC-0035缺正式filelist、外部define、wrapper/库和elaboration Evidence，上述事实证明“当前源码候选路径使用共享字面量宏”，仍不能单独证明量产netlist的最终绑定方式；它强化而不关闭本实施缺口。

现有资料尚未给出NGU800P逐die个性化载体、写入接口、可见性、锁定位、失败处置以及软件如何取得“已正确安装但不可读”的操作证明。不能从宏名推断这些硬件能力已经存在。

## Recommended binding

1. 量产netlist只实现硬件消费路径和个性化接口，不固化产品共享明文。
2. 每die秘密通过隐藏OTP/eFuse、PUF+Helper Data、硬件Key Ladder或等价不可读机制注入/派生。
3. 制造工具只传输由站点HSM保护的设备专属灌装对象；CPU不可读取最终明文。
4. 完成后使用known-answer operation、状态/锁定位和负向读取测试证明，不回读秘密本身。
5. 失败die进入隔离/报废，不重用其他设备Key，不允许软件fallback到共享Key。

## Required evidence

- RTL block和DFT/ATE个性化接口；
- per-die数据来源与KMS句柄绑定；
- reset、zeroize、debug/scan和生命周期下的可见性矩阵；
- 锁定位、重复写、掉电、ECC和报废语义；
- operation proof和EMU/FPGA/样片测试方法；
- 生产netlist/配置扫描，证明不存在共享明文默认值。

## Review history

- 2026-08-26：以SRC-0035重新核对宏定义和候选顶层使用链；确认当前源码仍是共享字面量宏接口，但因elaboration和逐die载体缺失，OPEN-CONFLICT-012继续保持`BLOCKED_BY_RTL_BINDING`。未摘录Key值。
- 2026-07-29：负责人批准一机一密目标；建立实施绑定缺口。该缺口不要求重新裁决产品目标，只要求RTL/DFT/制造方案提供可验证实现。
