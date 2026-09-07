# Proposal：BootROM non_sec_boot eFuse强制非安全启动

## Why

产品需要一个不可由普通软件控制的永久逃生通路：eFuse `non_sec_boot`默认0不影响既有策略，烧写为1后由BootROM强制选择现有受限非安全启动。

## What Changes

- 在既有Lifecycle×Strap模式矩阵之前增加`non_sec_boot`最高优先级覆盖。
- 定义BootROM只读、单次、带有效性/ECC状态的启动策略快照。
- 强制值1复用既有受限非安全路径；读取异常进入启动策略输入错误终态。
- 增加RTL/eFuse/制造绑定和验证要求，不猜测物理bit或寄存器。

## Source/Decision

- [SRC-0034](../../../sources/source-cards/SRC-0034.md)。
- [ADR-0031](../../../decisions/ADR-0031-non-sec-boot-efuse-override.md)。

## Scope

本change更新`security_-scheme`规范与详设，不授权修改BootROM产品代码、RTL、eFuse烧写工具或测试工作簿。
