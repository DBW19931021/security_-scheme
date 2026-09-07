---
title: "错误处理"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
owners:
  - GSP
last_reviewed: 2026-07-27
supersedes: []
superseded_by: []
---

# Purpose

定义 BootROM、FMC、GSP、安全公共组件和 eHSM adapter 的统一错误模型、传播、RAS上报、失败终态和敏感信息处理，使同一失败可从原始硬件/传输结果追溯到启动 stage 和测试 Evidence。W0-R1-09已批准；ADR-0006已冻结“security只上报、reset由RAS策略决定”，ADR-0010已冻结RAS未ready时的早期失败顺序；ADR-0014已冻结公共状态码编码和domain值，domain内reason仍按模块逐项分配。

# Scope

覆盖port、Mailbox transport、eHSM protocol/command、package/parser、policy、counter、Measurement Table、loader/release、平台保护和调用方错误。具体错误数值在设计批准和OpenSpec中冻结。当前不定义Handoff错误域。

# Confirmed facts

- 当前 `security_status_t` 主要覆盖 header/length/address/verify/decrypt/rollback/policy/not-supported，不能表达 ready、timeout、busy、reset、自检、LCS、transport、stage 和 raw code。
- 当前 `verify_image()` 失败时提前返回；measurement slot 错误可能造成返回值与 result 内状态不一致。
- BootROM demo 忽略安全 demo 返回值；FMC/GSP 没有 production caller 或失败终态。
- Vendor Host 通用层存在 busy/timeout/poll 返回，但 OSR port timeout 恒为 false，不能证明 NGU800P 实际超时行为。

代码证据见 CE-SEC-001/002。

# Documented facts

- SRC-0016及ADR-0030要求 eHSM 未 ready、Mailbox timeout、verify/decrypt、Header Overlay/typed-stage policy/counter 等失败均受控阻断，不允许绕过到下一级。
- SRC-0017 定义 eHSM 硬件/软件错误、status 和中断输入；具体 SoC 错误路由和终态仍需平台确认。
- Vendor error code 只描述 eHSM/Core/Host 实现，不能单独决定 SoC reset、recovery 或审计策略。
- ADR-0006规定security stage只记录、阻断和上报，不直接执行eHSM/system reset；RAS负责选择和执行reset/watchdog/隔离策略。

# Vendor implementation observations

- Host Mailbox 区分 busy、timeout、need-poll 和 command response，可保留为 raw transport/eHSM code。
- eHSM hardware/firmware status/error register 可作为诊断输入，但位图、clear 语义和严重度必须按 NGU800P 适用版本确认。
- reset 是平台动作，不应隐藏在所有错误的自动 retry 中。

# Assumptions

不假定最终数值编码、status/error register地址、read-clear规则、各operation timeout数值、RAS report通道和RAS动作映射。ADR-0011已冻结首版自动retry次数为0、timeout acceptance unknown和service quarantine；security不直接reset及RAS未ready时的fail-stop顺序也已冻结，不再作为候选行为。

# Proposed design

## 错误结构

候选 `security_error` 至少包含：

| 字段组 | 字段语义 |
|---|---|
| ABI | version、struct size、flags |
| 位置 | producer stage、operation/command、sub-stage |
| 分类 | error domain、mapped code、severity、retryability |
| 原始证据 | raw port/transport/eHSM/SoC status/error code，不因映射丢失 |
| 时序 | boot instance、command sequence、timestamp/deadline、retry/reset count |
| 资源 | image type、rollback domain、Region ID、受控长度；不得记录敏感payload或完整System Address |
| 处置 | downstream blocked、zeroization complete、RAS report status、requested action class、audit ID；不含直接reset执行权 |

跨 stage 结构使用固定宽度字段，不直接暴露编译器 enum/pointer。未知 raw bit 保留，不静默清零或错误解释。

## Error domain

| Domain | 示例 | 默认处理原则 |
|---|---|---|
| `PORT` | MMIO/timer/cache/barrier/RAS-report配置缺失 | fail-close；不得调用command，也不得自行reset补救 |
| `TRANSPORT` | busy/timeout/channel/sequence/late response | 首版不自动retry；timeout视为acceptance unknown，busy视为串行owner异常，service/slot进入quarantine |
| `PROTOCOL` | response size/version/command mismatch | 拒绝 response，保存 raw packet metadata |
| `EHSM_COMMAND` | verify/decrypt/self-test/LCS/key 返回失败 | 按 command policy 阻断；不改写 raw code |
| `FORMAT` | Header/Overlay/reserved/length/overflow/旧Manifest错误 | 拒绝 package，不调用 loader |
| `POLICY` | image/LCS/algorithm/address/permission 不允许 | fail-close，无 silent fallback |
| `COUNTER` | unreadable/domain mismatch/rollback/exhausted | 不设置 rollback-checked，不 release |
| `MEASUREMENT` | digest/slot/table/sequence/commit失败 | required Measurement未完成则不release下一级或输出成功SPDM结果 |
| `LOADER` | overlap/range/copy/PMA/barrier/指令侧同步错误 | 清理目标/临时区，禁止 entry |
| `STAGE_RELEASE` | stage/owner/Measurement/typed policy/protection/release错误 | consumer不使用未验证字段，失败时下一级不可达 |
| `PLATFORM` | Firewall/RAS report/interrupt/privilege 错误 | fail-close并保留错误记录；不得因上报失败自行reset |

## Severity 与处置

- `INFO/WARN` 不得用于降低 required security gate；只描述不影响 gate 的诊断。
- `ERROR` 阻断当前 image/service，可在已批准的受限控制面内报告。
- `FATAL` 阻断当前启动链并上报RAS；halt/watchdog/reset/recovery wait等实际动作由RAS策略选择，security只保持下一级不可达。
- `retryable` 不是 severity。首版所有eHSM operation均不可自动retry；未来只有在独立设计证明幂等性、提交状态可判定且迟到响应可区分后才允许改变。
- OTP/eFuse/key/counter/LCS/Flash activation 等操作 timeout 后固定为acceptance unknown，禁止在当前service中查询或重复写；由RAS批准的reset/recovery状态机在新boot instance中核对实际状态。

## 传播规则

1. 最靠近失败源的层记录 raw code；上层只追加 stage/policy/terminal action，不覆盖原值。
2. 每层返回值和 result/error 结构必须一致；不能返回失败但留下 success flag，反之亦然。
3. required gate失败后，成功release/jump API不可达。
4. production 不允许把 stub/simulated success 映射为 `NONE`；test provider 状态必须显式标记并被 production build 拒绝。
5. 日志和 Evidence 使用稳定的 domain/mapped code；测试同时断言 downstream blocked 和 raw code 保留。
6. 上报RAS失败不能把安全失败转成成功，也不能触发security私自reset；早期stage保存最小错误记录并进入fail-close等待/halt。

## RAS未ready的早期终态

1. 阻断release/jump，保存最小`EARLY_SECURITY_ERROR_RECORD`；记录stage/operation/domain、mapped/raw code、关键raw status、timestamp、zeroization和requested action，不保存敏感payload。
2. 使用当前stage安全RAM固定静态slot；payload写入并执行write/release fence后最后提交`valid`，两个批准PMA候选均不执行data clean/invalidate。它属于统一错误/日志机制，不创建Handoff ABI或独立永久Region。
3. 撤销临时Host/loader权限并尽力清零敏感buffer；清零失败提升为FATAL。
4. 在批准的有限deadline内只轮询RAS ready并重试上报，不重试原安全操作，不调用Vendor reset。
5. RAS接受后保持fail-close等待其动作；deadline到期则记录`RAS_REPORT_UNAVAILABLE`，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出。
6. 不假定普通安全RAM跨reset retention；后续若需要跨reset审计，必须有独立硬件资料和方案。

## 敏感信息和日志

- 不记录 key、完整 challenge/token、解密明文、私钥相关材料或 Mailbox 敏感 payload。
- 地址仅记录 domain、受控范围/offset 和必要低敏诊断；是否输出完整地址由发布日志策略决定。
- 错误退出必须执行对应buffer owner的zeroization；zeroization失败提升严重度并阻断release。
- 生产日志与测试详细日志分级；测试开关不得进入 USER 生命周期默认发布配置。

# Platform inputs

ADR-0020已关闭OPEN-DESIGN-003/018并冻结失败终态、severity/action分离、security只请求RAS动作、零自动retry和quarantine原则。以下精确数值仍作为平台/EMU输入：

1. 各stage向RAS上报的event ID/action request映射、RAS ready/report通道和early report deadline数值；无限WFI终态不再开放原语选择。
2. SoC/eHSM status/error register、严重度、clear/latched/pulse 语义和中断路由。
3. 各operation timeout数值、reset/recovery后的不可逆command状态核对接口，以及RAS对watchdog/reset的策略映射；首版自动retry已冻结为0。
4. USER 生命周期允许暴露的日志字段、持久审计位置和保留周期。
5. zeroization 的硬件辅助、编译器保证、cache/writeback 和 reset 覆盖范围。

# Implementation impact

- 扩展/替换单一 `security_status_t`，建立公共 `security_error` 和 stage/domain namespace。
- BootROM/FMC/GSP、eHSM adapter、verify/counter/measurement/loader 使用同一错误对象并保留 raw code。
- 建立唯一单向上报接口`security_error_report_to_ras()`；security调用方只接收“已受理/未受理”，不能调用具体reset primitive。
- BootROM/FMC/GSP early stage各提供一个固定对齐的`EARLY_SECURITY_ERROR_RECORD` slot和原子提交辅助函数；不得动态分配或放入可提前回收的普通栈。
- Vendor `ehsm_port_reset_ehsm()`不得进入production timeout/retry路径；测试需要reset时通过批准的RAS/test fixture。
- eHSM service发生timeout或BUSY异常后必须进入`QUARANTINED`；同一boot instance中后续调用在访问Mailbox前被拒绝，context/buffer不得按普通成功路径回收。首版不分配eHSM流式session。
- EMU/产品 source/build 必须阻止stub、simulated success、test provider、未批准hardcode和silent fallback进入可达路径；现有test/stub Expected不定义错误合同。

# Verification impact

- 每个 domain 至少有一个注入测试，断言 mapped/raw code、stage、terminal action、zeroization 和下游不可达。
- 覆盖busy/timeout/late response、acceptance unknown、零自动retry、service quarantine、错误result一致性、重复错误、RAS上报失败、禁止security直接reset和日志脱敏。
- 覆盖RAS未ready、上报在deadline内恢复、deadline耗尽、记录提交次序、zeroization失败、普通IRQ无法退出WFI循环以及进入循环前下游不可达。
- EMU/RTL验证真实中断、latched/pulse、clear、RAS接收及由RAS执行的reset/watchdog行为。

# References

- SRC-0016 安全启动、更新、LCS/Debug/Key 和失败处理章节。
- SRC-0017 eHSM 错误、状态与中断章节。
- CE-SEC-001/002。
- [eHSM Mailbox](../04-interfaces/ehsm-mailbox.md)。
- [ADR-0006](../../decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md)。
- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)。
- [安全RAM布局](../03-architecture/security-ram-layout.md)。
- [Boot Handoff历史提议（当前不采用）](../04-interfaces/boot-handoff.md)。
- [eHSM Host Adapter详细合同](../04-interfaces/ehsm-host-adapter.md)。
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)。
- [CE-SEC-008](../../evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md)。
- [安全公共ABI与Registry](../04-interfaces/security-common-abi.md)。
- [ADR-0014](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)。

# Change history

- 2026-07-27：接受ADR-0020并关闭OPEN-DESIGN-003/018；冻结各stage失败终态、RAS职责、日志/并发/清零原则，精确数值转平台/EMU输入。
- 2026-07-23：接受ADR-0014；公共返回值冻结为`(domain << 24) | reason`，domain数值由中心registry统一，Vendor raw status继续独立保存；各domain内部reason待模块详设分配。
- 2026-07-23：接受ADR-0011；首版所有eHSM operation自动retry为0，timeout标记acceptance unknown并quarantine整个service/slot；BUSY在单在途模型下作为异常占用处理。
- 2026-07-23：接受ADR-0010；冻结RAS未ready时的静态错误记录、有限上报等待和关闭普通中断后的无限WFI fail-stop循环，不创建Handoff ABI。
- 2026-07-22：依据ADR-0006冻结security只上报错误、RAS决定并执行reset的职责边界；更新错误处置字段、early-boot fail-close和验证要求。
- 2026-07-22：接受ADR-0004；当前不定义Handoff错误域；ADR-0030后阶段错误按Measurement Table、Header Overlay/typed-stage loader、保护和release合同传播。
- 2026-07-22：登记W0-R1-09已批准，并把EMU/产品无stub/simulated success和现有测试非规范依据写入错误合同边界。
- 2026-07-22：依据 CE-SEC-001/002 建立统一 stage/domain/raw-code、严重度、传播、终态和清零首轮设计；数值和平台终态待评审。
