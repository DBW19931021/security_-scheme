---
title: "生命周期"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0022
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-08-25
supersedes: []
superseded_by: []
---

# Purpose

维护Lifecycle专题Evidence和开放绑定；规范性设计以[主详设第10章](../05-software-design/NGU800P安全软件详细设计.md)为准。

# Baseline

- 识别TEST、DEV、MANU、USER、DEBUG、DESTROY和INVALID；raw常量从RTL/baremetal权威定义生成。
- BootROM先读取`non_sec_boot`只读快照：可靠值1强制现有受限非安全，值0才直接读取SoC LCS且不依赖eHSM Autoload；模式矩阵按ADR-0031/0018/0024。
- Lifecycle转换为typed、单向、可审计的不可逆事务；BootROM不写LCS。
- 产品普通转换只允许`TEST -> DEV -> MANU -> USER -> DEBUG -> DESTROY`相邻边；GSP在Vendor调用前精确校验，不能以“任意更大值”替代。直接DESTROY仅走独立二次授权入口。
- USER/其他状态进入DEBUG前先删除全部FW RAM/NVM用户Key并取得证明；随后LCS写失败时进入`KEYS_DELETED_LCS_UNCHANGED`，不得恢复旧Key或报告转换成功。
- GSP发起受控转换，eHSM执行最终权限判断；timeout/acceptance unknown不自动重试。

# Approved boundary and inputs

`OPEN-DESIGN-013`已关闭：USER不直接开放Debug；RMA固定由DEBUG LCS承载，使用独立RMA授权和只读诊断白名单，不新增RMA LCS、不接受普通Debug token替代且不关闭安全启动；DESTROY不可恢复。Debug只有一个SoC全局开关，无scope。raw绑定、最大开放时长和RAS数值从RTL/产品权限矩阵生成，不由业务代码猜测。

# Verification impact

覆盖`non_sec_boot`值0/1/输入异常及0→1不可逆烧写、非法LCS raw、逆向/跳级、非相邻跳转、删Key后LCS失败、掉电/timeout、一次性授权复用、读回不一致、审计失败和不可逆资源保护。

# References

- [ADR-0018](../../decisions/ADR-0018-bootrom-strap-lifecycle-and-lcs-timing.md)
- [ADR-0031](../../decisions/ADR-0031-non-sec-boot-efuse-override.md)
- [统一开放项](../../requirements/open-questions.yaml)

# Change history

- 2026-08-25：增加`non_sec_boot`最高优先级启动覆盖及不可逆烧写边界。
- 2026-07-27：接受ADR-0020并关闭OPEN-DESIGN-013；冻结Lifecycle/Debug/RMA/DESTROY原则，精确平台值转为生成输入。
- 2026-07-24：同步主详设第10章。
- 2026-07-28：冻结精确相邻转换图、DEBUG前删除FW用户Key、`KEYS_DELETED_LCS_UNCHANGED`和全局Debug开关。
