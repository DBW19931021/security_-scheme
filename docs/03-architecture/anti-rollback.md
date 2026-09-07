---
title: "防回滚"
status: approved_with_vendor_binding
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0033
owners:
  - GSP
last_reviewed: 2026-08-21
supersedes: []
superseded_by: []
---

# Purpose

定义各固件版本 counter 的可信来源、比较、更新、掉电恢复、耗尽和 measurement/audit 规则，防止已撤销或旧版本被重新启动。

# Scope

覆盖 BootROM/FMC/GSP/Runtime image 的 logical version、物理 monotonic counter、rollback domain、读取/比较/更新授权和启动/更新流程中的提交点。

# 当前代码事实（目标设计已替代）

- 当前公司代码旧manifest有`version_counter`和`rollback_domain`字段；ADR-0030要求production删除该parser。
- 当前`verify_flow()`只把旧字段复制到measurement，属于待替换差距。
- `policy_check()` 不强制 `POLICY_ROLLBACK_REQUIRED`。
- `ehsm_stub` 没有 counter input/compare/update，却无条件标记 `MEASUREMENT_ROLLBACK_CHECKED`。
- production 源码中未定位到物理 counter read/compare/update、掉电恢复或耗尽处理。

完整证据见 [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。

# Documented facts

- SRC-0016 的启动、更新、key rotation 等流程涉及 anti-rollback；具体 domain、counter 和更新策略必须以其有效版本为准。
- SRC-0017 提供系统级 rollback protection 原则和不可逆资源约束。
- Vendor counter 能力只能作为实现输入，不能替代 SoC image-domain 策略。

# Vendor implementation observations

CE-SEC-011已确认Vendor安全启动counter合同：

- Header offset 608和SOC OTP均使用16字节字段；当前测试向量是从首octet开始逐bit推进的单向thermometer编码。
- BL/FW比较允许相同版本；只有逻辑版本严格更高时才需要写OTP。
- BL成功验证非naked SoC镜像后总会把候选值暂存到eHSM DRAM，`check_version=OFF`也不关闭该副作用；BL verify本身不写OTP。
- eHSM FW启动时才消费DRAM候选并写SOC OTP，Vendor写函数内部执行写后回读。
- FW运行期`SOC_VERIFY check_version=ON`则在返回Host前直接更新OTP。
- Host 64位通用counter不是16字节版本counter，且当前FW快照没有实现该通用服务。

# Assumptions

物理SOC version counter已定位到eHSM OTP `base+0x40`，但不假定完整编码级数、允许跳级、寿命、耗尽、reset/掉电保留或产品LCS提交接口。逻辑宽度已由ADR-0005冻结为Vendor 16字节格式，不再把32位示例作为候选最终ABI。

# Proposed design

## 已冻结的Stage职责

- BootROM固定以`check_version=0`验证Vendor type 1 FMC；BL把认证Header的16字节counter暂存为RAM candidate。BootROM不独立读取、比较或写stored counter。
- BootROM把已验证FMC candidate写入唯一、`RELEASE_AUTHORIZED`且已commit的FMC Measurement条目；RAM candidate不等于OTP已提交。
- FMC初始化从FMC Entry复制`expected_candidate[16]`并调用BL staged-candidate commit API。BL必须先与RAM candidate逐字节一致；无candidate、失效或不一致都在OTP改动前拒绝。
- candidate一致后BL读取stored值：candidate低于stored拒绝；相等成功但不写；更高按Vendor单向编码写入；两种成功都必须返回权威readback。
- counter proof成立后FMC才接收GSP。GSP固定`check_version=0`且必须等于已提交SoC global counter；PMP/RMP/MMP/Die1也只匹配该值。
- Vendor type 0 eHSM FW使用独立counter域，遵守Vendor type 0策略，不要求等于SoC global counter。
- `OPEN-CONFLICT-005/009`已关闭：Vendor手册/代码均为16字节；eHSM BL新增typed staged-candidate commit API，不使用通用64位Counter、raw OTP或FW启动副作用。

## 必须冻结的 counter contract

| 字段 | 必须定义 |
|---|---|
| image/domain map | 每个 image type 对应的唯一 rollback domain/counter ID，是否共享 |
| representation | 逻辑宽度固定16字节；继续冻结Vendor字节序、物理编码和最大值 |
| reader/updater | FMC是唯一提交发起者；BL负责candidate匹配、read/compare/update/readback；LCS/role权限 |
| comparison | `<`、`==`、`>` 和无效/未烧写值的确定行为 |
| commit point | verify/decrypt、Flash 写入、activation、measurement 和 reboot 的先后 |
| atomicity | timeout/reset/power-cut 后如何判断 counter 与 image 是否一致 |
| endurance | 写寿命、保留余量、耗尽预警和拒绝策略 |
| recovery | 失败包、回滚包、recovery image 和 RMA 的例外及授权 |

## Candidate提交状态机

| 条件 | 固定行为 |
|---|---|
| FMC `expected_candidate != BL RAM candidate`、candidate缺失/失效 | OTP改动前拒绝，FMC terminal |
| FMC candidate `<` trusted SoC counter | 拒绝，不接收GSP |
| FMC candidate `==` trusted SoC counter | 不写，readback证明后接收GSP |
| FMC candidate `>` trusted SoC counter | BL单向写入并readback证明后接收GSP |
| GSP/PMP/RMP/MMP/Die1 counter `!=` committed SoC counter | 隔离对应镜像，不允许其单独推进global counter |
| eHSM FW counter | 独立Vendor type 0域，不与SoC域比较 |
| counter unreadable/invalid/domain mismatch | fail-close，不把Header candidate或Host值当trusted stored counter |
| counter exhausted | 阻止需要递增的更新并进入批准的运维/恢复流程 |

## 单一Global Counter评估

所有type 1 SoC stage镜像的一个16字节counter统一命名为`rollback_counter`，唯一来源是Native Header `Version_Counter[16]`；删除Manifest后不存在第二份包内counter或供Host读取的包内`version`。FMC、GSP和后续SoC镜像必须携带同一值；type 2/3产品stage包拒绝；eHSM FW是独立域。未改变的SoC固件如需进入新rollback domain，应以新值重新封装和签名。

FMC初始化时推进counter，之后GSP可能仍验证失败。此时只能在本boot接收相同已提交值的新GSP，否则进入受限恢复或整机重启；不得启动较低值旧GSP。

## 安全不变量

1. `MEASUREMENT_ROLLBACK_CHECKED` 只能由真实 counter service 成功结果产生。
2. counter提交只允许基于BL已经完整验证并暂存的FMC candidate，且必须与FMC回传值一致；所有Vendor type 1 SoC stage路径固定`check_version=0`，不得使用FW运行期自动更新。
3. 超时后不得盲目重试不可逆写；必须先查询实际状态。
4. activation 与 counter 更新必须有可证明的掉电一致性，不允许出现“新 counter + 旧 image”导致不可恢复。
5. 测试环境的模拟 counter 和 production counter API 必须构建隔离。

# Open questions

- 每类 image 的 counter ID/domain/初值/最大值；宽度已统一为16字节。
- 完整128级编码、允许跳级/多bit写、最大推进次数、初始未烧写值、寿命和耗尽预警。
- BL DRAM candidate在eHSM/SoC reset和掉电下的保留/清除，以及多次成功verify覆盖规则。
- 产品LCS可用的16字节staged-candidate commit接口准确command/FW绑定，以及提交/readback/status和`FW_ERROR_OTP_WRITE_FAILED`到Host error的映射。
- `OPEN-CONFLICT-009`已关闭；实施前冻结eHSM BL新增API的command/packing/LCS/交付版本。
- FMC rollback counter来源语义已冻结；Measurement逻辑C layout/integrity/commit已由ADR-0022批准，ADR-0028固定物理16 KiB和126项硬上限；产品实际实例数、PMA/Firewall及counter update后掉电重启等待Host状态机仍需实现与验证。
- 多 Die 是否共享 counter；Die1/Recovery/OOB 的例外。
- 掉电原子性、耗尽、RMA 和制造态策略。

# Implementation impact

- 从 `ehsm_result_t` 移除或限制可伪造 rollback-success 位，改由真实 counter service 返回结构化结果。
- `policy_check()`需结合受信typed stage/image/LCS决定rollback required，不能依赖包内自声明。
- Measurement只保存ABI规定的counter快照和counter-domain flag；package version、compare/update result、raw error进入counter result与审计。

# Verification impact

- L1：全部比较分支、domain mismatch、未烧写/最大值、权限和错误映射。
- L3/L4：真实 counter、reset/timeout/power-cut、重复请求、耗尽边界、activation 原子性。
- counter CI/EMU回归固定使用可重置的模拟资源；真实不可逆烧写测试只允许在批准的专用样片上执行，必须单独授权、记录初值/预算并禁止使用共享EMU或共享样片。两级测试都必须覆盖，不能互相替代。

# References

- SRC-0016 anti-rollback/启动/更新相关章节。
- SRC-0017 rollback protection 相关章节。
- [CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)。
- [ADR-0005](../../decisions/ADR-0005-b0-r1-address-counter-and-vendor-bl-boundary.md)。
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)。
- [Measurement Table接口](../04-interfaces/measurement-table.md)。
- [CE-SEC-011](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)。
- [OPEN-CONFLICT-009](../../sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md)。

# Change history

- 2026-07-24：接入CE-SEC-011；冻结Vendor 16字节单向编码、BL DRAM暂存、FW启动提交和内部写后回读事实，排除Host 64位counter/raw OTP替代，并建立OPEN-CONFLICT-009管理提交stage冲突。
- 2026-07-24：负责人决定Counter接口细节延期；按16字节抽象接口继续逻辑详设，Vendor绑定移至实现/EMU前门禁。
- 2026-07-23：批准BootROM committed FMC Measurement条目作为FMC自身epoch唯一跨stage来源；冻结FMC/GSP epoch一致性和counter update/Measurement/release顺序。
- 2026-07-23：负责人批准GSP package作为全局security epoch锚点、FMC唯一更新counter；其他固件必须匹配已提交epoch，失败镜像隔离并等待Host重发。首版不实现跨镜像整包Manifest或原子激活；未来若需要，必须通过独立ADR/OpenSpec重新设计。
- 2026-07-22：依据ADR-0005将Version/rollback Counter逻辑宽度冻结为Vendor 16字节格式；比较、更新、掉电和耗尽语义继续开放。
- 2026-07-21：依据 CE-SEC-002 确认当前 counter 仅为字段和模拟状态，建立首轮 counter contract、比较状态机和原子性门禁。
- 2026-07-28：冻结BootROM `check_version=0`暂存FMC candidate、FMC初始化exact-match后提交、GSP/Runtime等于已提交值及eHSM FW独立counter域。
