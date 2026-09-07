# ADR-0002：基线派生与冲突升级机制

- 状态：accepted
- 日期：2026-07-21
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0001～SRC-0019
- 相关 Requirement ID：无
- 替代关系：补充 ADR-0001

## 背景

SRC-0016/SRC-0017 是 PDF 形式的当前方案基线，其他 Vendor 文档、软件代码、RTL、测试和 review 又会持续提供新的事实或实现观察。若完整重写 PDF，容易产生重复维护；若只保留 PDF，则难以对增量裁决、冲突、适用范围和工程追踪进行版本化管理。

“软件方案为最终依据”描述的是规范目标和项目采纳状态，不表示 Vendor 文档、代码或测试中发现的明显矛盾可以被忽略。目标与当前实现可能存在差距，也可能暴露方案假设错误、版本不匹配或边界未定义，需要项目负责人裁决。

## 决策

### 1. PDF 与工程结论文档的分工

- SRC-0017/SRC-0016 PDF 继续作为当前受控原始基线，不完整复制成另一套 Markdown 基线。
- `docs/09-plans/BASELINE-CONTROL.md` 维护当前有效 PDF、上下位关系、已接受 ADR、临时修订和未决冲突。
- `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 等目标目录只沉淀经过分析或裁决的可执行结论，不复述 PDF 全文。
- 若裁决改变当前 PDF 内容但新版 PDF 尚未发布，先创建受控 amendment/ADR，并登记到 BASELINE-CONTROL；正式合入下一版 PDF 后再标记 amendment 已被吸收。

### 2. 规范目标与实现证据分开管理

- SRC-0017：系统/架构上位规范目标。
- SRC-0016：软件工程落地和项目采纳的软件规范目标。
- Vendor 文档：eHSM/Core 的文档证据。
- Vendor软件代码和tests：对应交付快照的软件实现证据，不自动成为规范。
- Vendor RTL：对应交付快照的eHSM内部硬件实现证据；SRC-0035是当前第一查询源，但仍为`VENDOR_IMPLEMENTATION`，不定义SoC集成或产品方案。
- 实测、构建和测试：当前环境下的验证 Evidence。

权威层级用于决定项目最终采纳什么；实现证据用于判断当前版本实际上做了什么。二者冲突时必须记录差距，不能用“软件方案为准”掩盖实现事实。

### 3. 必须升级的冲突

发现以下任一情况时，立即把相关结论标为 `CONFLICTING`，停止在受影响范围继续外推、设计或实施，并在当前任务内报告项目负责人：

1. SRC-0016 与 SRC-0017 的硬件前提、架构边界或安全原则不一致。
2. SRC-0016 的流程、接口、参数或实现要求与适用 Vendor 文档/软件代码/RTL明显不一致。
3. Vendor文档、同版本Vendor软件代码和RTL的接口或行为不一致。
4. Vendor软件代码、RTL、测试或实测显示软件方案依赖的能力不存在、返回值/状态机不同或安全边界无法满足。
5. 无法判断某行为属于 SoC、eHSM、eHSM Core、Host 软件或外部制造/运维系统。
6. Source 版本、配置、生命周期、OTP/Key Slot、算法或测试环境不一致，导致结论可能不可比。

### 4. 冲突报告内容

每次升级至少提供：

- 冲突 ID 和主题。
- Source A 的 Source ID、版本、章节/页码和原结论。
- Source B 的 Source ID、版本、代码路径/符号/行号或 Evidence。
- 冲突类型：规范冲突、目标—实现差距、文档—代码不一致、版本不匹配或边界不清。
- 安全、软件、测试、量产和计划影响。
- 当前可安全继续的范围以及必须停止的范围。
- 2～3 个处置选项、各自代价和推荐项。
- 需要项目负责人明确裁决的问题。

报告写入 `sources/conflict-reports/`，同时在 `requirements/open-questions.yaml` 登记未决问题。需要改变项目方案时，通过 ADR/OpenSpec 记录裁决。

### 5. 裁决后的更新

- 若认定软件方案正确、Vendor 实现不满足：建立实现 gap/issue 和验证 Evidence，不修改上位原则。
- 若认定软件方案需调整：先记录 ADR/amendment，再更新软件方案及相关需求、设计和测试。
- 若认定 Vendor 文档错误或代码版本不匹配：保留冲突报告，等待 Vendor 回复或新 delivery note。
- 若只是边界说明不足：补充系统/软件方案的职责边界，未明确前不得标为 `CONFIRMED`。
- 所有裁决都更新 BASELINE-CONTROL、Source Card、相关 open question 和项目状态。

## 选择理由

- 避免重复维护整套 PDF 内容。
- 让项目方案权威与实际代码事实同时可见。
- 确保明显冲突在进入实现前由项目负责人裁决。
- 支持裁决先以增量记录生效，后续再合并进正式 PDF 新版本。

## 风险

- 如果长期不把 accepted amendment 合入新版 PDF，会形成“PDF + 多个增量文件”的复杂有效基线，因此 BASELINE-CONTROL 必须持续维护。
- Vendor 代码缺少准确 delivery note 时，文档—代码冲突可能无法立即判断是版本问题还是实现问题。
- 冲突升级会暂停部分任务，但比在不清晰边界上继续实现更安全。

## 参考资料

- ADR-0001：方案权威层级与 Vendor 边界。
- SRC-0016：芯片安全软件方案 v1.2。
- SRC-0017：NGU800P 芯片系统安全方案。
- SRC-0018：OSR eHSM 4019 软件交付快照。
- SRC-0035：OSR eHSM 4019 Vendor RTL硬件实现快照。
- ADR-0032：eHSM Vendor RTL硬件实现基线与查询规则。
- `sources/templates/CONFLICT-REPORT-template.md`。
