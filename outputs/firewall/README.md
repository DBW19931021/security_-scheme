# NGU800P Firewall 交付导航

本目录保存便于评审和分发的正式报告。规范性内容仍按工程既有工作流分别维护，不以本目录复制件替代来源、需求、主详设或测试状态。

## 正式报告

- `NGU800P_Firewall_专题报告.docx`：架构、原理、默认值、16个UserId、authority位编码、寄存器候选、软件初始化、错误诊断、测试建议与编码门禁。

## 工作流落点

- 输入：`source-vault/internal-specs/firewall/SRC-0025/`
- 来源治理：`sources/source-cards/SRC-0025.md`、`sources/intake-reports/SRC-0025-firewall-intake.md`
- 冲突：`sources/conflict-reports/CONFLICT-SRC-0025-FIREWALL-DEFAULTS-AND-BINDING.md`、`OPEN-CONFLICT-014`
- 机读需求：`requirements/firewall-userid-authority-map.yaml`、`requirements/firewall-requirements.yaml`
- 架构：`docs/03-architecture/firewall-isolation.md`
- 主详设：`docs/05-software-design/NGU800P安全软件详细设计.md` 第5.9节
- 测试设计：`docs/06-verification/firewall-test-design.md`
- 测试矩阵：`tests/matrices/firewall-traceability.yaml`
- 测试评审工作簿：`tests/cases/NGU800P-security-test-cases-v0.3.6-firewall-review.xlsx`

## 当前边界

- UserId/authority：16个编码已完整登记，可指导软件校验、掩码生成和参数化testcase。
- CSR/默认policy：base、reset、Region默认窗口、R4名称、bit27标签、高16位/安全属性/lock/burst仍受`OPEN-CONFLICT-014`阻断。
- 测试：49条细化项中40条`READY`、9条`BLOCKED`，全部`NOT_EXECUTED`。
