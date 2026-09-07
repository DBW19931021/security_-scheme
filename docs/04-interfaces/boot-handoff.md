---
title: "BootROM/FMC/GSP 启动 Handoff ABI"
status: deferred
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
owners:
  - GSP
last_reviewed: 2026-07-22
supersedes: []
superseded_by: []
---

# Purpose

本文保留W0-R1-05曾讨论的版本化信任传递提议，供历史追溯。负责人已在ADR-0004裁决当前不采用：本文件不属于当前实现基线，不作为详设、OpenSpec、编码或测试前提，不冻结地址、枚举、结构体或物理内存位置。

当前方案按以下职责分工：跨阶段需要保留的镜像身份、digest、instance/die及方案规定的验证/释放度量信息由Measurement Table记录；Header Overlay、loader和Measurement统一使用baremetal System Address且不携带domain，entry固定等于load；错误/raw status由统一错误、日志和Evidence机制处理。不得把无关控制状态或敏感数据塞入Measurement Table。只有未来出现具体、可复现且上述机制无法满足的需求时，才可重新提案。

# R1-05 的含义与提出原因

Handoff 不是 eHSM command、函数调用或新的业务流程，而是“上一个独立固件把哪些已经验证的启动结果交给下一个固件”的阶段间合同。例如 BootROM 验证并加载 FMC 后，FMC 需要知道本次启动实例、验证结果、镜像范围、counter/measurement 状态和上游错误；这些信息要么由 FMC 全部重新获取/重新验证，要么通过明确的共享结构传递，不能依赖上一级的局部变量、BSS、隐含固定地址或口头约定。

“版本化”只表示该共享结构带 `magic + major/minor + size/flags`，不是给固件包再增加一个产品版本。这样 BootROM 与 FMC 分开编译或字段扩展时，consumer 能识别不兼容的 major、跳过兼容的可选扩展并拒绝越界/旧启动周期数据，避免两边结构布局不一致却继续启动。

本提议与 OSR Host `ehsm_ctx` 不同：`ehsm_ctx` 是单个stage内Host调用eHSM的命令上下文；Handoff是BootROM/FMC/GSP这些独立固件之间的信任结果传递。复用OSR Host业务代码不会自动解决阶段间合同。

R1-05历史评审曾比较以下三个方向；ADR-0004已经终止该选择，当前产品不实现任何独立Handoff ABI：

1. **最小版本化Handoff（历史提议，未采用）**：曾提议只传递本次boot ID、producer/consumer、验证过的镜像描述、counter/measurement引用和raw eHSM/error摘要。
2. **下一级全部重新获取和重新验证（未采用）**：会重复eHSM/介质访问，且仍不能独立解决启动实例、release状态和Measurement来源。
3. **隐含全局变量/固定SRAM/裸指针约定（永久禁止）**：缺少兼容、边界和旧数据防护，不得进入产品实现。

上述内容是R1-05提出时的论证背景，不代表当前要求。ADR-0004已裁决现阶段不引入该合同；仅当Measurement Table、Header Overlay/typed-stage loader及现有错误/Evidence机制出现具体、可复现且无法解决的跨阶段状态缺口时，才重新提案并单独裁决。

# Scope

覆盖 handoff header、image verification result、anti-rollback、measurement 引用、eHSM/error 摘要、地址域、提交顺序、内存保护和接收方验证。eHSM command 见 [eHSM Mailbox](ehsm-mailbox.md)，错误结构见[错误处理](../05-software-design/error-handling.md)。

# Confirmed facts

- 当前 BootROM/FMC/GSP 没有生产 handoff 结构或调用链；BootROM/FMC/GSP 默认入口仍是 hello-world/demo 骨架。
- 当前 `verify_image()` 只返回进程内结构，调用 eHSM stub；FMC/GSP 没有 production caller。
- 当前 measurement 是各镜像自身 BSS 数组，不具备跨 stage 信任传递。
- 早期GSP package/test与linker曾存在两种地址view；该现状已由2026-07-28的System Address统一裁决取代，不再构成当前Handoff需求。

代码位置和限制见 CE-SEC-001/002。

# Documented facts

- SRC-0016 pp.1–12 要求 BootROM/eHSM/FMC/GSP/Runtime 逐级验证、measurement、load 和 release，任一级失败不得继续下游。
- SRC-0017 的系统隔离原则要求受信任启动核控制加载、release 和 Firewall；Host 不拥有直接 release 权限。
- Vendor 仅覆盖 eHSM/Core，不定义 SoC handoff、地址域和下一级执行状态。

# Deferred proposed ABI model（当前不实施）

## 结构分层

```text
security_handoff_header
  + producer / consumer / boot instance
  + verified_image_descriptor
  + rollback_result
  + measurement_reference
  + eHSM/result snapshot
  + terminal security_error (success时必须为NONE)
  + extension area
```

以下内容仅保留为历史候选模型，不冻结C类型名、数值编码或实现语义，也不进入当前OpenSpec/实施任务：

| 组 | 必需字段 | 约束 |
|---|---|---|
| 版本 | magic、ABI major/minor、header size、total size、feature flags | 接收方拒绝未知 major、越界 size 和不支持的必需 flag |
| 路径 | producer stage、expected consumer、boot/reset instance、sequence | 不能把旧启动周期的 handoff 重放到当前 boot |
| 镜像 | image type、logical version、package/plaintext extent、load/entry System Address | 地址保持64位且必须属于consumer allowlist；禁止alias或地址换算 |
| 验证 | profile、signature/decrypt/policy result、raw eHSM result reference | 只有真实 production provider 能设置 PASS；test result 不得进入 production handoff |
| 回滚 | rollback domain、package version、trusted counter、compare/update result | `checked` 只能来自真实 counter service；失败或 UNKNOWN 阻断 handoff |
| 度量 | table ABI version、location/domain/extent、sequence、digest metadata | 接收方校验 table 边界/版本/owner；空 digest 不得作为已验证 measurement |
| 错误 | stage、domain、mapped code、raw transport/eHSM/status、flags | 成功 handoff 的 terminal error 必须为 NONE；raw code 不得丢失 |
| 保护 | producer commit state、protection/lock state、clear-complete state | 未完成清零、write/release barrier或保护切换时不能标 committed；两个批准PMA候选均不执行data clean/invalidate |

## 编码规则候选

- 所有跨 stage 固定宽度整数；不把 C enum 大小、编译器 padding 或本地指针写入 ABI。
- 候选使用little-endian，与当前C908/Header Overlay解析保持一致；需要在批准设计中明确，不从Vendor示例自动推断。
- 地址只使用baremetal权威头定义的64位System Address；不分配address-domain ID，不支持Local/System alias或换算。
- 可变区只使用相对 handoff 起始位置的 offset/length；接收方先检查整数溢出和总范围，再解析。
- unknown optional extension 可以跳过；unknown required flag 必须 fail-close。
- CRC 只能用于检测意外损坏，不能作为安全认证。可信性依赖启动链、写权限、Firewall/内存保护和接收方重校验。

# Producer commit protocol

1. 在受控内存中将整个 handoff 清零，`commit_state=INVALID`。
2. 写入 header、image/counter/measurement/error 和 extension。
3. 完成临时敏感缓冲区清零、write/release barrier和目标区保护配置；两个批准PMA候选均不得执行data clean/invalidate。
4. 对所有 size/offset/System Address span/required-result 做 producer 自检。
5. 最后写入 `commit_state=COMMITTED` 和 sequence；随后不再修改已提交字段。
6. release/jump 下一级；若返回，记录为新的 stage error，不复用旧成功 handoff。

本历史提议若未来重新启用，也必须使用已批准的对齐32位commit、write/release与acquire/read barrier合同；`NON_CACHEABLE`与`HARDWARE_COHERENT`两个候选均不执行data clean/invalidate。当前提议未采用，不能生成产品ABI。

# Consumer validation

接收方必须在消费任何 offset/address 之前依次检查：

1. handoff 物理位置属于本 stage 的只读/受控 allowlist。
2. magic、ABI、header/total size、alignment、commit state 和 boot instance 有效。
3. producer/consumer stage 与实际启动路径一致，terminal error 为 NONE。
4. 每个 offset/length 不溢出且位于handoff内；每个System Address/extent位于本stage allowlist。
5. signature/decrypt/policy/counter/measurement 的 required result 全部成功，且不是 test/simulated provider。
6. measurement table版本、实际`total_len/fw_entry_count`、紧凑实例列表、唯一SoC State、owner、commit和Hash长度有效。
7. 必要 Firewall/protection/clear 状态可由本 stage 独立观测或重校验。

任何失败都不得继续下游 release。接收方可以重新建立本 stage 的 eHSM context，但不能静默把上游失败状态改写为成功。

# Address rule

- R1-05提出时曾建议跨组件地址携带domain并在port/loader转换；该建议已被2026-07-28最终裁决废止，不得进入当前实现。
- 当前GSP Header Overlay、loader、linker、C908和eHSM共享descriptor统一使用baremetal System Address，不建立Local/System映射；旧`0x1010_0808_0000/0x1000_0808_0000`均不得继续使用。本文件仍只是未采用Handoff的历史提议，不定义当前产品地址合同。
- 当前日志、Measurement和Evidence记录实际System Address；不得生成source/target domain或地址转换结果字段。

# Failure and zeroization

- 生产 handoff 不允许表达“部分成功后继续”。required result 中任一失败，producer 不提交成功 handoff。
- package plaintext、临时 key/challenge、Mailbox request/response 和 scratch buffer 的 owner/clear 时点由对应 stage 唯一负责。
- 错误日志不保存密钥、明文 payload 或完整敏感 token；保留 stage/domain/raw code、受控地址范围和长度。
- reset/timeout 后不得复用未提交或 boot instance 不匹配的 handoff。

# Open questions

1. handoff 物理 SRAM、大小、对齐、cache 属性和每个 stage 的读写权限。
2. BootROM/FMC/GSP 的 stage ID、image type、address-domain ID 和 ABI 数值分配。
3. producer commit 的对齐32位原子store、write/release与acquire/read barrier、Firewall lock和下一级reset/release顺序。
4. measurement table 是原地共享、复制还是由安全服务持有；其完整性如何保护。
5. 各master的System Address aperture、PMA/barrier与Firewall配置仍需RTL/EMU Evidence；C908不建立Local/System remap，两个PMA候选均不执行data clean/invalidate。这些属于Header Overlay/loader与平台集成验证问题，不依赖本Handoff提议。

# Implementation impact

- 当前无实现影响：不得创建Handoff结构、producer/consumer adapter、专用SRAM、commit协议或相关编码任务。
- 统一错误结构和Measurement Table继续按各自已批准专题设计，不依赖本提议。

# Verification impact

- 当前不新增Handoff测试。启动链测试从Measurement Table、Header Overlay/typed-stage loader、错误状态和下游不可达观察点派生。

# References

- SRC-0016 pp.1–12。
- SRC-0017 安全启动、隔离和 Firewall 章节。
- CE-SEC-001/002。
- [安全启动](../03-architecture/secure-boot.md)。
- [GSP 地址域冲突](../../sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md)。

# Change history

- 2026-07-22：ADR-0004裁决当前不采用版本化Handoff；文档状态改为deferred，仅保留历史提议，不进入实现/测试范围。
- 2026-07-22：明确R1-05仍待负责人评审，补充Handoff与普通函数/eHSM context的区别、版本化含义、三种选择和最小化边界；未授权编码。
- 2026-07-22：建立跨 BootROM/FMC/GSP 的首轮版本化 handoff ABI、提交/校验、地址域和清零规则；具体地址与数值保持待裁决。
