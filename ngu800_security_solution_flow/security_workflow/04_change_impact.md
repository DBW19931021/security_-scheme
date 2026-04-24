# NGU800 安全方案变更影响分析

## 1. 新增 / 更新输入摘要

| Change ID | New / Updated Source | Summary | Priority |
|---|---|---|---|
| CHK-001 | 无新增输入 | 本轮仅执行 `03_detailed_design.md` 的最终交付检查，未引入新的原始资料、约束或基线输入 | `medium` |
| CHK-002 | `security_workflow/03_detailed_design.md` | 为满足“每个主章节必须包含 Mermaid 架构图和时序图”的交付要求，补充了第 1、2、15 章的 Mermaid 图 | `high` |
| CHG-003 | `SRC-003` / 用户补充修正 | 修正 `sec1` 固件来源：应从 NOR Flash 加载，不由 Host 下发；同时要求详设开头增加术语表、关键图下增加文字说明，并在 OTP/eFuse 章节给出明确布局图表 | `high` |

## 2. 受影响约束

| Constraint ID | Previous Statement | Updated Statement | Impact Level |
|---|---|---|---|
| 无 | 无 | 本轮未修改 `01_constraints.md` 中任何约束内容 | `none` |
| `C-BOOT-004` | `sec1` 作为两级固件之一，负责最小 bring-up | 明确 `sec1` 镜像来自 NOR Flash，由 BootROM/eHSM 路径验证与装载 | `high` |
| `C-HOST-001` | Host 下发 `sec1`、`sec2` 与后续固件 | Host 仅下发 `sec2` 及后续固件，`sec1` 不属于 Host 供应路径 | `high` |

## 3. 受影响章节

| Chapter | Why Impacted | Regenerate Needed |
|---|---|---|
| 第 1 章 文档概述 | 原稿缺少 Mermaid 架构图与时序图，不满足交付模板检查要求 | `yes` |
| 第 2 章 输入材料与设计基线 | 原稿缺少 Mermaid 架构图与时序图，不满足交付模板检查要求 | `yes` |
| 第 1 章 文档概述 | 新增术语描述列表，满足正式设计报告阅读要求 | `yes` |
| 第 3 章 安全目标、保护资产与威胁边界 | 更正 `sec1` 来源并补充图下说明 | `yes` |
| 第 4 章 NGU800 安全总体架构 | 4.2 架构图和 4.3 时序图存在理解歧义，已重画并增加步骤说明 | `yes` |
| 第 5 章 Root of Trust、密钥体系与证书体系 | 新增 OTP/eFuse 布局表并调整小节编号 | `yes` |
| 第 6 章 安全启动详细设计 | 更正 `sec1` 来源和 Host 下发边界，并补充图下步骤说明 | `yes` |
| 第 11 章 内外部接口设计 | 清理 Host 写入 `sec1` 的旧表述 | `yes` |
| 第 15 章 附录 | 原稿缺少 Mermaid 架构图与时序图，不满足交付模板检查要求 | `yes` |
| 第 3-14 章 | 本轮仅复核一致性，未修改正文内容 | `no` |

## 4. 接口 / 结构 / 位图影响

| Item | Previous | New | Hardware Impact | Software Impact |
|---|---|---|---|---|
| 主章节 Mermaid 覆盖率 | 第 1、2、15 章无 Mermaid 图 | 第 1、2、15 章均已补齐 Mermaid 架构图和时序图 | `none` | `none` |
| `sec1` 供应路径 | Host 下发 `sec1` 的旧口径 | `sec1` 固件来源改为 NOR Flash | `medium` | `high` |
| Host 缓冲写入规则 | Host 可写 `sec1` 缓冲的旧表述 | Host 仅写管理子系统 IRAM 中的 `sec2` / 后续固件窗口 | `medium` | `high` |
| OTP/eFuse 布局 | 仅有字段族，无明确布局图表 | 新增区段图与字段布局表 | `high` | `high` |

## 5. 冻结风险

| Item | Risk Level | Needed Action |
|---|---|---|
| `sec1` NOR Flash 装载地址和执行区映射 | `medium` | 在后续实现阶段补齐具体 load address 和 region 定义 |
| Host 外部协议字段 | `high` | 待新增 Host 需求输入后再更新 `01/02/03` 文档 |
| eFuse 细化位分配 | `high` | 需单独产出字段表并补齐 owner |
| 板级真实边界 | `medium` | 待 BMC/OOB/SMBus 专项资料后再冻结 |
| 制造 / RMA owner | `medium` | 待制造与售后流程 owner 明确 |
| Mermaid 覆盖合规性 | `closed` | 本轮已修复 |

## 6. 建议执行动作
- 当前无新增原始输入，不需要重新生成 `01_constraints.md` 或 `02_baseline.md`。
- `03_detailed_design.md` 已完成交付一致性修订，并同步吸收了 `sec1` 来源、术语表、图下说明和 OTP/eFuse 布局修正。
- 下一轮若新增 Host、Board、eFuse 或制造/RMA 资料，应先更新 `inputs_manifest.md`，再按全流程执行增量更新。
