# SRC-0016 Die1 Measurement 策略表述冲突

## Identity

- Conflict ID: CONFLICT-SRC-0016-DIE1-MEASUREMENT
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人；Measurement/SPDM实施Owner待指定
- Decision required by: 固化 Die1 measurement table、SPDM MEASUREMENTS 和流片前验收预期之前

## Conflict classification

- Type: internal_spec_inconsistency / boundary_ambiguity
- Affected scope: Die1 固件是否在 measurement table 中形成独立条目、SPDM 如何呈现 Die1、Verifier 如何判断 Die1 固件状态。
- Safe-to-continue scope: Die1 镜像由 Die0 eHSM 验签解密、UCIe/SRAM Firewall、Die1 Debug gating 等不依赖 measurement 表达的测试设计。
- Must-stop scope: 为 Die1 measurement 的条目数量、`fw_type`、`die_id`、hash/版本字段或 SPDM block 建立确定性验收预期。

## Source A：允许/预留独立 Die1 条目

- Source ID and version: SRC-0016，芯片安全软件方案 v1.2。
- Applicable version: NGU800P D0；正式适用修订仍待补齐。
- Document evidence: 第 14～15 页，§4.3 measurement table 定义包含 `NGU_FW_TYPE_DIE1_FW = 0x0100`，固件条目包含 `die_id`（0: Die0，1: Die1）。
- Related evidence: 第 4 页 §2.1 表述 eHSM 验证 FMC/GSP/Runtime/Die1 镜像时都比较 `image_counter` 与 `stored_global_counter`。
- Expected interpretation: 数据结构和启动流程允许为 Die1 镜像建立可区分的 measurement 条目。

## Source B：不单独记录 Die1 固件状态

- Source ID and version: SRC-0016，芯片安全软件方案 v1.2。
- Document evidence: 第 33 页，§9.4 明确写明“由于 Die1 的所有固件与 Die0 相同，所以不单独记录 Die1 的固件状态”。
- Related evidence: 同页 §9.1 规定 Die1 measurement 和远程证明由 Die0 统一纳入。
- Expected interpretation: measurement table/SPDM 不为 Die1 建立独立固件状态条目，或至少不重复记录与 Die0 相同的固件。

## Exact conflict

SRC-0016 同时提供了可区分 Die1 的数据结构，又规定不单独记录 Die1 固件状态。当前无法确定：

1. `NGU_FW_TYPE_DIE1_FW`/`die_id` 只是未来预留，当前版本不得使用；
2. Die1 应独立记录，但相同 hash 可复用数据；
3. 只有 Die0/Die1 固件不同时才独立记录；
4. SPDM 对内部 table 的呈现与 table 本身采用不同聚合策略。

## Impact

- Security: Verifier 可能无法区分 Die1 是否实际加载了期望镜像，或者实现/测试重复计算同一固件状态。
- Software: measurement table producer、SPDM responder 和 Verifier 的条目编码/解析无法定稿。
- Verification: 测试用例无法确定应期望 0、1 或条件化 Die1 条目。
- Interoperability: 不同实现可能对相同设备产生不同 measurement record，影响远程证明策略。
- Schedule: 不阻断多 Die 启动/Firewall/Debug 测试，只阻断 Die1 measurement 的确定性验收。

## Options

### Option A：始终建立独立 Die1 条目

- 使用 `fw_type=NGU_FW_TYPE_DIE1_FW` 和 `die_id=1`；即使 hash 与 Die0 相同也明确记录实际加载实例。
- 优点：可审计、与现有结构一致；代价是 measurement/SPDM 记录变大并需要定义重复 hash 的语义。

### Option B：当前版本不建立独立 Die1 条目

- 把相关枚举/字段视为预留，SPDM 仅通过 SoC 状态字段说明 Die1 状态。
- 优点：实现简单、符合 §9.4 字面；风险是 Verifier 无法直接确认 Die1 加载实例。

### Option C：按固件/profile 条件化

- Die0/Die1 完全共用同一映像时聚合，版本/hash/用途不同或 profile 要求时建立独立条目。
- 优点：兼顾体积和表达力；代价是规则、兼容性和测试矩阵更复杂。

## Recommendation

推荐 Option A。远程证明的首要目标是可审计地表达“哪个 Die 实际加载了什么”；重复 hash 可以通过相同 hash 值体现，但不应丢失 `die_id=1` 的实例信息。若必须控制 SPDM 记录大小，可在内部 table 保留独立条目，再由明确 profile 定义聚合呈现。

## Required owner decision

- Die1 是否必须形成独立 measurement table 条目？
- `NGU_FW_TYPE_DIE1_FW` 和 `die_id` 在 NGU800P 当前版本是有效字段还是预留字段？
- SPDM MEASUREMENTS 是否逐条呈现 Die1，若聚合，Verifier 如何确认 Die1 实际 release 状态？
- 裁决应写入 SRC-0016 后续版本或受控 amendment，并同步更新测试与 Verifier 规范。

## Resolution and review history

- 2026-07-22：负责人批准推荐Option A。内部Measurement Table始终建立`fw_type=NGU_FW_TYPE_DIE1_FW`、`die_id=1`的独立Die1实例记录；相同映像可以复用相同digest，但不得丢失实例语义。SPDM可由后续profile决定逐条或聚合呈现，聚合时仍须让Verifier确认Die1有效measurement/release状态。见ADR-0004。
- 2026-07-21：在审视 SRC-0020 测试覆盖时发现；建立 OPEN-CONFLICT-002，相关测试用例标为阻塞/待裁决。
