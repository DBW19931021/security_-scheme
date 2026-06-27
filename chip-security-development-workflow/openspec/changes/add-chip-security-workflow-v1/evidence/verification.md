# Verification Evidence

## Environment

- 日期：2026-06-09
- Python：3.8，标准库 `unittest`
- OpenSpec：spec-driven schema，strict validation
- 工作区：`security_-scheme/chip-security-development-workflow`

## Commands and Results

| Command | Result |
|---|---|
| `python3 -m unittest discover -s tests -v` | PASS，21 tests |
| `openspec validate add-chip-security-workflow-v1 --strict --no-interactive` | PASS |
| NGU800 MCTP `openspec validate ... --strict` | PASS |
| `check_openspec_evidence.py ... --gate G2` | PASS |
| `check_openspec_evidence.py ... --gate G5` | Mechanical checks PASS，approval REVIEW |
| direct CLI `--help` regression test | PASS |
| `git diff --check` | PASS |

## Requirement Coverage

- WF-REQ-001/002：生命周期、OpenSpec 功能循环和规范文件合同测试。
- WF-REQ-003/004/005：事实源、G0-G6 evaluator 和人工审批边界。
- WF-REQ-006/007：evidence、problem record 和 retrospective 模板与 Gate。
- WF-REQ-008：候选采集和 L0-L3 mechanical evaluation。
- WF-REQ-009：三级 onboarding pack 和角色 checklist。
- WF-REQ-010：NGU800 MCTP 试点仅修改组件文档层的迁移步骤；已有实现代码
  不因通用工作流收口而再次修改。

## Unverified Items

- 尚未在第二款芯片上运行完整 P0-P6 生命周期。
- L4 跨芯片验证和 L5 Skill 尚未实施。
- G3-G6 的正式人类审批尚未记录。

## Residual Risks

- Markdown heading 合同只能验证结构，不能替代安全内容评审。
- 路径映射和角色名称仍需各芯片项目初始化时确认。
- 第二款芯片可能暴露需要扩展的 Gate 条件和模板字段。
