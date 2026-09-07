# 2026-07-21 Work × Codex轻量工作流入库与适配报告

## Identity

- Source ID：SRC-0021。
- 原始文件：`source-vault/internal-specs/workflow-inputs/SRC-0021/work_codex_lightweight_workflow.md`。
- SHA-256：`33202e862ecfec7fa7391eca6c2797ac03c7a79c6b83b8f5fdb5e9f1a3ac9125`。
- 大小：16973 bytes。
- 状态：`DOCUMENTED`流程输入，不定义芯片事实。

## 适配决定

- 不新建`docs/design`：复用`docs/03-architecture`、`docs/04-interfaces`、`docs/05-software-design`。
- 不新建`docs/traceability`：复用requirements、Feature矩阵、测试矩阵、ADR、冲突报告和CHANGELOG。
- 调查任务使用`tasks/active/INV-SEC-xxx`，证据使用`evidence/code-investigations/CE-SEC-xxx`。
- 模板分别放入`tasks/templates`和`evidence/templates`。
- 项目禁止Git操作的约束高于原文“必要时查看Git历史”，调查只使用清单中已有的只读基线记录。

## 首批任务

1. INV-SEC-001：BootROM与eHSM启动交互。
2. INV-SEC-002：FMC/GSP验签解密加载链。

## 影响

- 支撑TASK-SEC-DD-001的DD-01～DD-03。
- 当前只建立调查和证据框架，不修改两个代码仓、不执行测试、不改变安全方案结论。
