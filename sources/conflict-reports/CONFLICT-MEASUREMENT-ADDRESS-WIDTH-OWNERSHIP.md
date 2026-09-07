# Measurement地址字段宽度与职责冲突

> 2026-08-21替代说明：64位System Address和Measurement只保存loader成功结果快照的结论仍有效。ADR-0030已删除Manifest；当前地址执行Owner是Header offset1008的LE64 `load_addr`与typed-stage loader的精确目标匹配，`entry_addr=load_addr`，不存在包内address domain或Local/System转换。以下Manifest/domain文字只保留裁决历史，不得作为现行实现依据。完整合同见[《NGU800P安全软件详细设计》第3章和第9章](../../docs/05-software-design/NGU800P安全软件详细设计.md#第3章-固件packagengu-manifest制作与发布)。

## Identity

- Conflict ID: CONFLICT-MEASUREMENT-ADDRESS-WIDTH-OWNERSHIP
- Open Question: OPEN-CONFLICT-004
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人；Measurement/Loader实施Owner待指定
- Decision required by: 冻结Measurement Table C ABI和Manifest/loader接口之前

## Conflict classification

- Type: internal_spec_inconsistency / data_width_overflow / ownership_overlap
- Affected scope: Measurement固件条目的`load_addr/entry_addr`字段、GSP 64位canonical地址、Manifest/loader与Measurement职责边界。
- Safe-to-continue scope: Measurement digest、fw_type、die_id、verify/release状态语义；Manifest和loader使用64位canonical地址的设计。
- Must-stop scope: 已解除地址宽度/职责冲突；Measurement其余未决字段仍按各自设计门禁处理。

## Final owner decision（2026-07-22）

- Measurement中的`load_addr`和`entry_addr`均保留并改为`uint64_t`。
- 两个地址都必须携带显式address domain，继续遵守W0-R1-06/ADR-0004；不允许无domain解释、截断或隐式alias。
- Manifest/loader仍是加载和跳转执行地址的唯一Owner；Measurement中的两个地址是loader成功执行后的只读审计快照，不能反向作为load/jump输入。
- OPEN-CONFLICT-004关闭；最终字段offset、对齐和table整体版本仍在FW-C-007正常详设中冻结，不再属于本冲突。

## Source A：SRC-0016 Measurement示例使用32位地址

- Source ID/version: SRC-0016，芯片安全软件方案v1.2。
- Applicable version: NGU800P D0；正式适用修订待确认。
- Evidence: 第15页§4.3 `ngu_meas_fw_entry_t`包含`uint32_t load_addr`和`uint32_t entry_addr`，注释为可选。
- Interpretation: 若按示例直接冻结，Measurement条目只能表达32位地址。

## Source B：批准的GSP地址是64位canonical值

- Decision: ADR-0004 / OPEN-CONFLICT-003 resolution。
- Evidence: GSP Manifest `load_addr/entry_addr`必须使用NoC/system canonical地址`0x1010_0808_0000`；`0x1000_0808_0000`仅为local/remap view。
- Constraint: `0x1010_0808_0000`不能由`uint32_t`表示；截断会产生错误目标地址。

## Source C：地址职责已分配给Manifest/loader

- Decision: ADR-0004。
- Evidence: load/entry/address-domain由Manifest/loader合同负责；Measurement Table承载跨stage度量信息，当前不采用Handoff。
- Interpretation: Measurement继续保存地址会形成重复owner和一致性问题；若保留，必须说明它是审计快照还是执行依据。

## Exact conflict

当前三条要求不能同时原样成立：

1. Measurement地址字段保持32位；
2. Measurement保存GSP实际load/entry；
3. GSP实际地址使用64位canonical值。

同时，Manifest/loader和Measurement哪个是地址唯一事实源尚未在SRC-0016示例结构体中体现。

## Impact

- Security: 地址截断或domain丢失可能导致错误加载、错误release或审计误导。
- Software: Measurement producer、SPDM转换和loader可能各自维护不同地址副本。
- Verification: 无法为GSP Measurement字段定义确定Expected。
- Compatibility: 直接冻结32位layout后再扩展到64位会造成ABI破坏。

## Options

### Option A：Measurement不保存load/entry（推荐）

- Manifest/loader成为load/entry/address-domain的唯一正式owner。
- Measurement记录image identity、digest、version/counter、die_id、verify/release状态，不重复地址。
- 若需要审计地址，从受控Manifest和loader Evidence关联，不进入Measurement wire ABI。

优点：符合ADR-0004职责分工，避免截断和双事实源；缺点：单独读取Measurement时看不到加载地址。

### Option B：Measurement保留64位地址并增加domain（已采用）

- 把地址改为`uint64_t`，同时增加`load_addr_domain/entry_addr_domain`。
- 明确这些字段只是已执行loader结果快照，不是新的执行输入。

优点：审计完整；代价是table增大、producer/consumer和一致性规则更复杂。

### Option C：保持32位可选字段

- 只对低4GB镜像记录地址，GSP等高地址条目写0或不使用。

不推荐：同一ABI对不同镜像表达能力不同，容易被误用，也不能证明domain。

## 决策结果

负责人采用Option B：Measurement保留64位`load_addr`和`entry_addr`并携带显式domain。结合ADR-0004，两个字段定义为loader完成后的受控结果快照，Manifest/loader保持执行地址Owner。

## Resolution

1. `load_addr`和`entry_addr`均为`uint64_t`，并分别关联显式address domain。
2. Measurement地址只作loader执行结果快照，Manifest/loader仍是执行地址唯一Owner。
3. 该裁决由ADR-0005承载，并需进入SRC-0016后续版本或受控amendment，同时同步Measurement/SPDM测试。

## Review history

- 2026-07-22：在任务二首轮详细设计复核SRC-0016第15页和ADR-0004时发现；建立OPEN-CONFLICT-004，阻断Measurement最终C layout。
- 2026-07-22：负责人部分裁决Measurement `load_addr`保留并改为64位；`entry_addr`和字段职责仍待确认，冲突保持开放。
- 2026-07-22：负责人进一步确认`entry_addr`同样保留并改为64位；结合ADR-0004将两个地址定义为loader结果快照，关闭OPEN-CONFLICT-004。
