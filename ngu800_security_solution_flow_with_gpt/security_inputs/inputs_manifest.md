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
| SRC-002 | `security_inputs/ip_manuals/ehsm/` | eHSM资料目录级策略 |  |  | `confirmed` | `preferred` | eHSM集成、secure boot支撑能力、密钥体系、接口复用思路、efuse字段、生产阶段操作建议、固件镜像字段 | 采用该硬件子系统这一前提需要遵循；eHSM中已经明确定义的固件字段、OTP/eFuse排布、key slot语义、计数器和生产阶段操作优先按eHSM定义设计；当前按单控制器方案设计 | 最终方案尽量复用其设计思路和能力边界；算法框架需同时兼容国密和国际标准两套；若项目确有新增需求，应在eHSM基线上做兼容性扩展，而不是重定义已有字段 | 若与用户已明确冻结的系统架构口径冲突，不直接跟随，需在输出方案文档中保留差异记录 |  | 该条为目录级策略，适用于 `ip_manuals/ehsm/` 下资料，后续可增加单文件覆盖项；用户于 2026-04-22 明确要求方案同时支持国密和国际标准两套算法栈，并于 2026-04-23 进一步明确 eHSM 已定义技术细节应尽量优先沿用 |
| SRC-003 | `security_inputs/soc_arch/启动方案.pdf` | 启动方案 |  |  | `draft` | `preferred` | boot flow、secure boot流程、阶段职责划分、微核数量与职责 | 安全/非安全启动流程、各系统中的微核数量和职责当前以本文件为准；`sec1` 固件来源以启动方案描述为准，从 NOR Flash 加载 |  | 若后续发现明显错误或与更高优先级材料冲突，可调整，但在新的冻结决定出现前按本文件执行 |  | 作为当前启动流程主参考；用户于 2026-04-22 明确确认流程口径，并于 2026-04-23 进一步确认 `sec1` 从 NOR Flash 加载 |
| SRC-004 | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | 安全子系统硬件方案 |  |  | `draft` | `partial` | 安全子系统架构、管理子系统架构、集成边界 | 重点参考客户需求和安全子系统架构方案、管理子系统架构方案 |  | 未被上述重点覆盖的部分暂不视为强约束 |  | 用于硬件集成和架构边界判断 |
| SRC-005 | `security_inputs/board/管理子系统.pdf` | 管理子系统方案 |  |  | `draft` | `preferred` | 管理子系统总体架构、模块划分、系统流程、管理面交互路径、与安全子系统的集成边界 | 文档中的总体架构和流程口径原则上需要遵循，特别是管理子系统模块职责、数据/控制流向、集成位置和系统级流程编排 | 当文档中的安全设计考虑不足、未覆盖关键攻击面、与既定安全基线冲突或明显存在安全漏洞时，不直接跟随；这类内容仅作为问题输入，必须在安全方案输出中明确指出差异、风险和替代设计 | 不将该文档中未经安全评审确认的弱化安全假设、越权访问路径、绕过鉴权的便捷流程直接继承到正式安全方案 |  | 该文档用于补充管理子系统视角下的总体架构与流程；对其中安全相关内容采用“遵循总体架构与流程、但不盲从安全细节”的策略，若发现考虑不够或明显不安全之处，应在约束、详设或风险章节中显式指出并给出安全侧裁决 |

## Conflict Log

| Conflict ID | Topic | Preferred Source ID | Other Source ID | Reason | Impacted Chapters | Status |
|---|---|---|---|---|---|---|
| CF-001 | 安全核两级固件命名、位置与职责映射 | `SRC-003` | `SRC-001`, `SRC-004` | 用户已明确安全核两级固件统一命名为 `sec1` / `sec2`，并以启动方案作为流程与职责基线 | boot chain、阶段职责、镜像布局 | `resolved` |
| CF-002 | Host下发镜像的存放路径 | `SRC-003`, `SRC-004` | `SRC-001` | 进一步收敛后按启动方案口径：`sec1` 固件来自 NOR Flash，由 BootROM/eHSM 路径完成验证与装载；Host 仅下发 `sec2` 及其后续固件到管理子系统 IRAM | host boundary、镜像加载、内存布局 | `resolved` |
| CF-003 | FSP语义与Root of Trust口径 | `SRC-002`, `SRC-003` | `SRC-001` | 用户已明确 FSP 指 eHSM 内的核；系统最早执行入口与首个密码学验证根仍按分层口径描述 | trust boundary、模块命名、Root of Trust | `resolved` |

## Change Intake

| Change ID | Date | New / Updated Sources | Summary of Change | Expected Impact | Freeze Risk | Registration Notes |
|---|---|---|---|---|---|---|
| CHG-001 | 2026-04-22 | `SRC-001`, `SRC-002`, `SRC-003`, `SRC-004` | 首批方案基线、eHSM目录级策略、启动方案和安全子系统硬件方案已登记 | 将影响完整详设基线、secure boot、eHSM集成和子系统架构定义 | `unknown` | `SRC-002` 为目录级策略，可后续按单文件细化 |
| CHG-002 | 2026-04-22 | `SRC-002`, `SRC-003`, `SRC-004` | 用户补充冻结口径：启动流程和微核职责以 `SRC-003` 为准；方案必须同时支持国密和国际标准两套算法；安全核两级固件命名统一为 `sec1` / `sec2`；Host下发固件时 `sec1` 放在安全子系统 firewall 划分出的非安全区域，其他固件放在管理子系统 IRAM；FSP 指 eHSM 内部核；efuse 字段与生产阶段操作优先对齐 eHSM，按单控制器设计 | 将直接影响约束表、设计基线、镜像布局、OTP/eFuse规划与算法章节 | `medium` | 该变更已用于回写 `01_constraints.md`，后续文档需沿用同一口径 |
| CHG-003 | 2026-04-23 | `SRC-003` | 用户补充修正：`sec1` 固件来源应以启动方案描述为准，从 NOR Flash 加载；Host 不负责下发 `sec1`。同时要求详设增加术语表、图下文字说明以及 OTP 布局图表 | 将影响约束表、设计基线、启动章节、总体架构图、OTP 章节和最终交付风格 | `medium` | 本轮修正以 `SRC-003` 优先，覆盖之前关于 Host 下发 `sec1` 的表述 |
| CHG-004 | 2026-04-23 | `SRC-002` | 用户补充冻结口径：eHSM 中已明确的方案和技术细节应尽量以 eHSM 为准，尤其是固件字段、OTP/eFuse 排布、key slot 语义和生产阶段操作 | 将直接影响约束表、设计基线、镜像头字段、OTP/eFuse 布局、升级封装和量产流程章节 | `medium` | 本轮修正把 eHSM 从“优先参考”进一步上升为“已定义技术细节优先沿用”的集成规则 |
| CHG-005 | 2026-04-27 | `SRC-005` | 新增管理子系统文档，并明确使用口径：总体架构和流程原则上遵循；涉及安全且考虑不足、明显存在漏洞或与既定安全基线冲突的内容不直接继承，需在输出中显式指出问题并给出安全侧替代方案 | 将影响总体架构、板级安全、接口边界、Host/管理子系统交互、启动/装载流程及风险章节 | `medium` | 本轮变更用于把管理子系统资料纳入正式输入，并建立“系统流程可参考、安全细节需二次裁决”的输入策略 |

## Open Questions

| Q ID | Topic | Blocking Area | Needed From | Status |
|---|---|---|---|---|
| Q-001 | eHSM资料中若与既定架构明显冲突时的取舍 | eHSM集成、接口复用、boot链路 | 用户确认 | `closed` |
