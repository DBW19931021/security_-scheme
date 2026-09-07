# Proposal：BootROM模式回退与eHSM自主自检

## Why

负责人决定降低BootROM策略复杂度：LCS异常统一进入受限非安全启动；非安全启动不写Measurement/启动审计；eHSM依据自身eFuse决定并执行自检，BootROM只消费ready/error和raw结果。

## What Changes

- 更新Lifecycle×Strap模式矩阵。
- 定义受限非安全路径的禁止权限和无审计行为。
- 删除BootROM发起自检的状态/API。
- 保留BootROM对eHSM ready/error和raw自检结果的读取门禁。

## Source/Decision

- 项目负责人2026-07-28裁决。
- [ADR-0024](../../../decisions/ADR-0024-manifest-simplification-boot-mode-and-ehsm-self-test.md)。

## Scope

本change只修改`security_-scheme`规范和详设，不授权修改BootROM、Vendor、baremetal或测试工作簿。
