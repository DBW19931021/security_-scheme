# SRC-0016 与 SRC-0017 方案基线关系确认

## Status

- screening_status: relationship_confirmed
- evidence_state: DOCUMENTED
- confirmed_conflict: false
- decision: `decisions/ADR-0001-solution-authority-and-vendor-boundary.md`

## Sources and applicable versions

- `SRC-0017`：NGU800P 芯片系统安全方案，系统/架构上位方案，粒度较粗。
- `SRC-0016`：芯片安全软件方案 v1.2，依赖 SRC-0017 的软件工程落地方案。

## Confirmed relationship

两份文档不是替代关系，也不是平级竞争基线：

1. SRC-0017 定义系统级硬件基础、安全架构、边界和原则。
2. SRC-0016 以工程落地为出发点，负责软件方案、接口、流程和实现约束。
3. SRC-0016 的硬件前提、架构边界和原则必须依赖并符合 SRC-0017。
4. 软件侧采纳的 Vendor eHSM/Core 建议必须进入 SRC-0016 或其后续有效版本，才能成为项目最终软件方案。
5. 密钥轮换策略已经批准，具体工程内容以 SRC-0016 或其后续有效版本为准。

## Conflict handling

- 若 SRC-0016 与 SRC-0017 的硬件、架构或原则不一致，优先保留 SRC-0017 上位约束，并通过方案评审/OpenSpec 修订软件方案。
- 若 SRC-0016 与 Vendor 建议不一致，软件侧以 SRC-0016 最新有效内容为准；需要采纳 Vendor 新建议时，先更新软件方案。
- 两份文档的正式版本、Owner 和完整审批记录仍需补齐，但不影响本次已确认的层级关系。
- 仍需对 Secure Boot、更新/OOB、生命周期/Debug、密钥管理、Attestation/SPDM、多 Die 和制造灌装进行章节级一致性检查。

## Resolution

- 原 `potential_overlap` 已关闭。
- 本报告不再作为未决冲突；后续发现具体条目不一致时，另建条目级 conflict report。
- 新的软件设计应引用 SRC-0017 的上位依据和 SRC-0016 的落地规定。
