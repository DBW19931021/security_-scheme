# Inputs Manifest

Use this file as the active source inventory for NGU800 security design inputs.

## Source Inventory

| Source ID | Path | Title / Topic | Owner | Version / Date | Confidence | Use Policy | Constraint Strength | Decision Scope | Applies To | Must Follow | Optional Reference | Ignore / Out of Scope | Supersedes | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SRC-001 | `security_inputs/current_plan/安全方案.pdf` | 当前安全方案基线 |  |  | `draft` | `partial` | `SOFT` | `overall baseline` | `ch4-ch14` | 总体功能点、职责划分主线需要遵循 | Host部分仅作参考；结构体与格式可在更高优先级资料下调整 |  |  | 当前总体基线 |
| SRC-002 | `security_inputs/ip_manuals/ehsm/` | eHSM 资料目录级策略 |  |  | `confirmed` | `preferred` | `HARD` | `ehsm capability / mailbox / lifecycle / debug` | `ch5/ch6/ch9/ch11/ch12` | eHSM 能力边界、Mailbox 交互模型、生命周期与 debug 能力需优先跟随 | 与系统总体架构冲突处需单列确认 |  |  | 目录级策略，可后续补单文件覆盖 |
| SRC-003 | `security_inputs/soc_arch/启动方案.pdf` | 启动方案 |  |  | `draft` | `preferred` | `HARD` | `boot path / stage handoff / mode matrix` | `ch4/ch6/ch7` | 启动路径、secure/non-secure 口径优先遵循 |  |  |  | 当前 boot 主参考 |
| SRC-004 | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | 安全子系统硬件方案 |  |  | `draft` | `partial` | `SOFT` | `subsystem boundary / integration` | `ch4/ch10/ch11/ch14` | 安全子系统与管理子系统边界需重点参考 | 其他未覆盖处不自动视为强约束 |  |  | 当前硬件集成参考 |

## Conflict Log

| Conflict ID | Topic | Preferred Source ID | Other Source ID | Reason | Impacted Chapters | Status |
|---|---|---|---|---|---|---|

## Change Intake

| Change ID | Date | New / Updated Sources | Summary of Change | Expected Impact | Freeze Risk | Registration Notes |
|---|---|---|---|---|---|---|

## Open Questions

| Q ID | Topic | Blocking Area | Needed From | Status |
|---|---|---|---|---|
