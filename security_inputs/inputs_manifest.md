# Inputs Manifest

Use this file as the active source inventory for NGU800 security design inputs.

Current status:
- Initial project references have been registered.
- When new files are added under `security_inputs/`, register them here first.
- Directory-level policy may be used when the same guidance applies to a whole folder. Add per-file overrides later if needed.

## Source Inventory

| Source ID | Path | Title / Topic | Owner | Version / Date | Confidence | Use Policy | Applies To | Must Follow | Optional Reference | Ignore / Out of Scope | Supersedes | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SRC-001 | `security_inputs/current_plan/安全方案.pdf` | 当前安全方案基线 |  |  | `draft` | `partial` | 完整详设基线、总体安全功能、方案范围 | 总体方案和功能点需要遵循 | host部分仅作参考；固件头文件格式和加密格式可参考，尽量对齐eHSM需求 | 细节设计和实现内容在有更合适的补充材料时可调整 |  | 作为当前基线方案使用 |
| SRC-002 | `security_inputs/ip_manuals/ehsm/` | eHSM资料目录级策略 |  |  | `confirmed` | `preferred` | eHSM集成、secure boot支撑能力、密钥体系、接口复用思路 | 采用该硬件子系统这一前提需要遵循 | 最终方案尽量复用其设计思路和能力边界 | 若与既定架构方案明显冲突，不直接跟随，需在输出方案文档时列出并与用户确认 |  | 该条为目录级策略，适用于 `ip_manuals/ehsm/` 下资料，后续可增加单文件覆盖项 |
| SRC-003 | `security_inputs/soc_arch/启动方案.pdf` | 启动方案 |  |  | `draft` | `preferred` | boot flow、secure boot流程、阶段职责划分 | 在没有明显错误的前提下，当前启动流程基本按此方案执行 |  | 若后续发现明显错误或与更高优先级材料冲突，可调整 |  | 作为当前启动流程主参考 |
| SRC-004 | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | 安全子系统硬件方案 |  |  | `draft` | `partial` | 安全子系统架构、管理子系统架构、集成边界 | 重点参考客户需求和安全子系统架构方案、管理子系统架构方案 |  | 未被上述重点覆盖的部分暂不视为强约束 |  | 用于硬件集成和架构边界判断 |

## Conflict Log

| Conflict ID | Topic | Preferred Source ID | Other Source ID | Reason | Impacted Chapters | Status |
|---|---|---|---|---|---|---|

## Change Intake

| Change ID | Date | New / Updated Sources | Summary of Change | Expected Impact | Freeze Risk | Registration Notes |
|---|---|---|---|---|---|---|
| CHG-001 | 2026-04-22 | `SRC-001`, `SRC-002`, `SRC-003`, `SRC-004` | 首批方案基线、eHSM目录级策略、启动方案和安全子系统硬件方案已登记 | 将影响完整详设基线、secure boot、eHSM集成和子系统架构定义 | `unknown` | `SRC-002` 为目录级策略，可后续按单文件细化 |

## Open Questions

| Q ID | Topic | Blocking Area | Needed From | Status |
|---|---|---|---|---|
| Q-001 | eHSM资料中若与既定架构明显冲突时的取舍 | eHSM集成、接口复用、boot链路 | 用户确认 | `open` |
