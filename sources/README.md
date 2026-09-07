# 资料入库与放置指南

## 三层资料模型

1. `source-vault/`：原始文件或不可变快照，只保存输入，不直接形成项目结论。
2. `sources/`：Source ID、哈希、版本、适用范围、事实等级、抽取、入库和冲突报告。
3. `docs/`、`requirements/`、`decisions/`、`openspec/`：经分析或批准后的工程结论和变更，不复制原始资料充当结论。

## 不同资料放置位置

| 资料类型 | 原始资料 | 登记与分析 | 后续正式结论 | 默认事实状态 |
|---|---|---|---|---|
| Datasheet | `source-vault/datasheets/SRC-xxxx/` | Source Card、extracted、intake report | 相关 `docs/` 和需求 | `DOCUMENTED`，实测后才可能 `CONFIRMED` |
| Vendor TRM/手册 | `source-vault/vendor-docs/SRC-xxxx/` | Source Card、章节抽取、冲突报告 | 接口/架构文档 | `DOCUMENTED` |
| Vendor 代码压缩包/快照 | `source-vault/vendor-code/SRC-xxxx/` | Source Card、版本/哈希、实现观察 | 软件设计或方案比较 | `VENDOR_IMPLEMENTATION` |
| Vendor RTL投递快照 | `source-vault/vendor_rtl`或`source-vault/vendor-rtl/SRC-xxxx/` | Source Card、树哈希、顶层/filelist/define/parameter、RTL证据和冲突报告 | eHSM内部硬件调查、集成约束和验证计划 | `VENDOR_IMPLEMENTATION`；匹配验证后才可能提升 |
| 独立 Vendor Git 仓库 | 保持在外部独立路径，不复制第二份可编辑源码 | 在 `manifests/repositories.yaml` 登记 remote、commit、用途和只读策略，并创建 Source Card | 仅作为实现参考 | `VENDOR_IMPLEMENTATION` |
| 历史 Review/本地分析 | 保持在原快照位置或 `source-vault/internal-specs/` | 单独 Source ID、候选发现索引、重新验证状态 | open questions、issues、ADR 或 OpenSpec | 文档集为 `PROPOSED`；具体发现先为 `ASSUMPTION` |
| 已有方案基线 | `source-vault/internal-specs/baselines/SRC-xxxx/` | Source Card、基线范围、审批状态、冲突和差距分析 | 相应 `docs/`、ADR、OpenSpec | 正式受控基线可为 `DOCUMENTED`；未审批方案为 `ASSUMPTION` 或 `PROPOSED` |
| 内部规格 | `source-vault/internal-specs/SRC-xxxx/` | Source Card、版本和适用范围 | 需求、架构和 ADR | 依审批和验证状态确定 |
| 标准 | `source-vault/standards/SRC-xxxx/` | Source Card、适用章节和合规范围 | compliance/接口/测试文档 | `DOCUMENTED` |
| 会议纪要 | `source-vault/meeting-notes/SRC-xxxx/` | Source Card、参会人和待确认项 | open questions；批准后再进入 ADR/设计 | 通常为 `ASSUMPTION` 或 `PROPOSED` |
| RTL同步生成头/SoC map | 保持在负责同步的外部代码仓，不复制第二份可编辑源码 | 仓库清单、Source Card、生成版本、目录manifest hash和关键宏抽查 | 地址/寄存器详设、冲突报告和实现同步检查 | 数值为`DOCUMENTED`；EMU/硬件行为验证后才为`CONFIRMED` |

## 项目方案权威层级

- SRC-0017“芯片系统安全方案”是系统/架构上位方案，粒度较粗，负责硬件基础、系统架构和安全原则。
- SRC-0016“芯片安全软件方案”是依赖 SRC-0017 的软件工程落地方案；软件侧最终采用内容以其最新有效版本为准。
- Vendor 只表示 eHSM 及 eHSM 内部 Core，不包含 SoC。Vendor 输入被采纳后必须进入 SRC-0016 或其后续有效版本，不能从 Vendor 资料直接提升为 SoC 设计。
- SRC-0035 `source-vault/vendor_rtl`是eHSM内部硬件实现细节、当前RTL实现基线和问题查询第一入口；直接结论为`VENDOR_IMPLEMENTATION`，不得跳过top/filelist/define/parameter/elaboration条件，也不替代Vendor手册的接口说明或SoC/产品方案Source。
- 密钥轮换策略已批准；SRC-0015机制已由ADR-0021和主详设第10章作为受控软件方案增量，ADR-0026关闭OPEN-DESIGN-014的软件设计裁决；Vendor定制ABI/版本、Bitmap/掉电、KMS和可执行recipe作为实施绑定管理。
- 本工程中 Vendor 文档统一标为 `NOT_CONFIDENTIAL_FOR_THIS_PROJECT`；这不免除版权、授权和对外分发规则。

## NGU800P SoC地址与寄存器权威源

- SRC-0022登记的`../baremetal/components/chip_riscv_c908_common/include/ngu800p`当前有效生成头，是NGU800P SoC地址、寄存器base/offset/bitfield和IRQ常量的第一权威源；依据见ADR-0009。
- `gsp-pmp-rmp-omp`中的同名头是实现镜像，必须与SRC-0022同步，但不能在发生差异时静默覆盖SRC-0022。
- Vendor示例地址只适用于其明确声明的平台，不能覆盖SoC生成头。
- 每次冻结数值都引用Source ID、文件、宏/字段、address domain和生成版本；关键冻结点记录文件hash。
- 地址存在不等于接口语义明确。Owner、wrapper/直连关系、状态/清除时序、cache、RAS/reset和产品策略仍由系统/软件方案、匹配集成说明、Evidence或负责人裁决确定。
- 本规则不把`baremetal` test/stub/demo/synthetic数据和历史Expected提升为目标设计或测试oracle。

## eHSM内部RTL权威源

- [SRC-0035《OSR eHSM 4019 Vendor RTL硬件实现快照》Source Card](source-cards/SRC-0035.md)登记的`source-vault/vendor_rtl`是当前eHSM内部RTL事实的第一查询源，适用于模块层级、端口、内部总线/译码、状态机、reset/default表达式、信号宽度及内部连接。
- 每次引用必须锁定Source ID、树哈希、相对路径、module/instance/symbol、紧凑行范围和配置条件；不得仅写裸路径或“当前RTL”。
- 使用叶模块前必须证明它由实际顶层/filelist实例化，并记录命令行define、parameter/generate、wrapper和库绑定。没有这些输入时，结论只能限定为当前源码候选配置。
- SRC-0035是实现证据，不自动证明量产配置、FPGA/EMU/硅片行为；Vendor手册、FSP、测试或Evidence与RTL冲突时必须升级，不能静默选RTL。
- SRC-0035不定义NGU800P SoC地址/IRQ/PMA/Firewall/clock/reset/power/RAS连接，不定义eFuse产品位、Lifecycle、Key、Boot或软件策略；这些继续使用各自SoC/方案权威源。
- 目录包含Key/KEK字面量，按`RESTRICTED_PENDING_CONFIRMATION`和只读快照处理；不得抄录数值、对外分发、上传第三方服务或作为量产Key/一机一密证据。
- 具体步骤见[《eHSM Vendor RTL调查与证据规则》工程约束](../docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md)，决策依据见[ADR-0032《eHSM Vendor RTL硬件实现基线与查询规则》](../decisions/ADR-0032-ehsm-vendor-rtl-authority-and-query-rules.md)。

## PDF、派生结论与冲突

- SRC-0016/SRC-0017 PDF 是当前受控原始基线，不完整复制为另一套 Markdown。
- `docs/09-plans/BASELINE-CONTROL.md` 记录当前有效 PDF、accepted ADR/amendment 和未决冲突。
- 目标 `docs/` 目录只写经过分析或裁决的工程结论；裁决改变 PDF 但新版尚未发布时，以受控 amendment/ADR 临时生效并等待下一版 PDF 吸收。
- 软件方案表示项目目标，Vendor文档/软件代码/RTL表示eHSM/Core的接口意图或实现证据。二者明显不一致时必须保留实际差距，不能以权威顺序静默覆盖。
- 发现规范冲突、目标—实现差距、文档—代码不一致、版本不匹配或边界不清时，立即建立 conflict report，将受影响结论标为 `CONFLICTING`，在继续相关设计/实施前提交用户裁决。

## 推荐目录示例

```text
source-vault/datasheets/SRC-0001/ngu800p-datasheet-v1.2.pdf
sources/source-cards/SRC-0001.md
sources/extracted/SRC-0001/registers.md
sources/intake-reports/SRC-0001-intake.md
sources/conflict-reports/SRC-0001-vs-SRC-0003.md
```

同一资料的新版本应分配新 Source ID，并在 Source Card 中填写 `Supersedes` / `Superseded by`；不要直接覆盖旧版本。

## 每份资料的入库步骤

1. 先放入 `source-vault/incoming/`，确认它不是重复版本。
2. 分配下一个 Source ID，例如 `SRC-0001`。
3. 移入对应类别的 `SRC-xxxx/` 目录，保持原文件不变。
4. 计算 SHA-256，并登记名称、类型、版本、日期、所有者、路径、适用芯片/修订、保密等级和事实状态。
5. 复制 `sources/templates/SOURCE-CARD-template.md` 创建 `sources/source-cards/SRC-xxxx.md`。
6. 更新 `sources/source-index.yaml`；索引是查找入口，Source Card 保存详细上下文。
7. 如需文字抽取，写入 `sources/extracted/SRC-xxxx/`，并标明页码/章节；抽取内容不能替代原文件。
8. 在 `sources/intake-reports/` 记录关键安全内容、影响范围和待确认问题；发现冲突时在 `sources/conflict-reports/` 建立报告。
9. 只有完成事实分级和评审后，才把结论更新到 `docs/`、`requirements/`、ADR 或 OpenSpec。

## Vendor 代码中包含历史 Review 时

1. 整个代码交付快照分配一个 Source ID，记录组件版本、文件数、大小、目录树哈希和是否混有构建产物。
2. 不逐个源码文件分配 Source ID；代码位置由快照 Source ID、组件路径、文件和行号共同定位。
3. 若快照中包含项目组 review、自动分析报告或旧 OpenSpec，将它们作为不同来源再分配 Source ID，不能标成 Vendor 正式结论。
4. 把带编号的历史发现登记到 `sources/historical-review-index.yaml`，默认状态为 `ASSUMPTION / pending_revalidation`。
5. 只有重新核对代码版本、Vendor 文档和独立 Evidence 后，才能把候选项提升为 open question、正式 issue、需求、ADR 或 OpenSpec change。
6. 新版本代码必须分配新 Source ID；不得覆盖旧快照或在 `source-vault/` 内修改源码来“修复”历史发现。
7. Vendor 更新应附 delivery/release note；新 Source Card 记录 note、整体组件版本、树哈希和 `Supersedes`，不要求逐文件登记。

Vendor RTL投递同样按整个交付快照管理，不逐RTL文件创建Source Card。除delivery/release note和树哈希外，还必须登记顶层、filelist、外部define/parameter、wrapper/库、约束和匹配验证版本；缺失项必须作为结论限制，不能默认补齐。

## 关键规则

- Datasheet 是正式文档事实，不自动等于当前 RTL 或实测行为。
- Vendor 代码永远先标为 `VENDOR_IMPLEMENTATION`，不能据此定义正式 OTP、生命周期、密钥或寄存器行为。
- Vendor RTL同样先标为`VENDOR_IMPLEMENTATION`；它可以定义当前eHSM内部实现事实，但不能单独定义NGU800P SoC连接、产品策略、量产配置或硅片行为。
- Vendor 的有效范围仅为 eHSM 及其内部 Core；任何 SoC 级外推都需要内部系统方案或其他正式 SoC Source。
- 采纳的 Vendor 建议必须进入芯片安全软件方案，未进入前保持 `PROPOSED`。
- 历史 Review 能证明“曾有此分析”，不能证明分析正确；作者、版本、复现步骤和 Evidence 不完整时，具体发现保持 `ASSUMPTION`。
- “已有方案基线”必须记录版本、审批状态和适用范围；不能因为正在使用就自动标成 `CONFIRMED`。
- OpenSpec change 只引用 Source ID 和相关章节，不把大段原始资料复制进去。
- 原始文件过大或受保密限制时，可以存放在外部只读位置，但必须记录稳定路径、哈希、访问限制和备份责任人。
