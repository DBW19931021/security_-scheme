# Source Vault

此目录保存原始文件或不可变快照，不在这里直接改写、摘录或形成项目结论。每份正式入库资料都应分配 Source ID，并在 `../sources/source-index.yaml` 和 `../sources/source-cards/` 登记。

| 目录 | 内容 |
|---|---|
| `incoming/` | 尚未分类的临时收件区；完成登记后移动到正式类别 |
| `datasheets/` | 厂商正式 Datasheet 原文件 |
| `vendor-docs/` | TRM、应用笔记、接口手册及其他 Vendor 文档 |
| `vendor-code/` | 非独立 Git 仓库的 Vendor 代码压缩包或只读快照；若夹带本地 review/构建产物，应在 Source Card 中拆分来源 |
| `standards/` | DMTF、TCG、NIST、ISO/IEC 等标准原文 |
| `internal-specs/` | 公司内部规格、正式方案和评审材料 |
| `internal-specs/baselines/` | 已有方案基线的不可变原始版本 |
| `meeting-notes/` | 会议纪要和讨论记录；不能自动作为已确认硬件事实 |

推荐每个版本使用独立目录，例如 `datasheets/SRC-0001/`。保留原文件名并记录 SHA-256、版本、日期、适用芯片/修订、来源和保密等级。大型或敏感文件是否进入 Git 由公司策略决定；即使原文件存放在外部受控位置，Source Card、哈希和路径索引仍应进入本工程仓库。

对于已包含历史 review 的代码快照，保留其原始目录结构，不在这里更新 review 状态或修改源码。代码快照与历史 review 应分别登记 Source ID；当前项目的候选发现状态统一维护在 `../sources/historical-review-index.yaml`。

Vendor 仅指 eHSM 及其内部 Core，不包含 SoC。本工程中的 Vendor 文档分类为 `NOT_CONFIDENTIAL_FOR_THIS_PROJECT`；源文件原有标记保留不改。Vendor 代码按交付快照管理，不逐文件建卡；收到附带 delivery/release note 的新版本时，建立新的 Source ID 并关联被替代快照。
