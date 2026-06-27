# Retrospective

## Delivered Scope

建立双层芯片安全开发流程、P0-P6 生命周期、G0-G6 Gate、项目初始化、
OpenSpec evidence 检查、onboarding 生成和 L0-L3 知识候选评估。

## Design Deviations

- 原计划假设 NGU800 MCTP 尚未实现；实际收口时它已完成，因此试点改为迁移
  完整设计和现有验证证据，并保持 G5 人工审批 pending。
- V1 未生成最终 Skill，保持设计中“第二款芯片验证后再晋升 L5”的边界。

## Problems and Root Causes

- 单元测试 package import 掩盖了 CLI 直接执行的模块搜索路径问题。
- 早期计划复选框未随实现同步，导致实际完成度与文档状态不一致。

## Rejected Approaches

- 自动批准 Gate 或知识晋升。
- 把芯片事实复制进通用模板。
- 在未经过第二款芯片验证前生成并宣称 L5 通用 Skill。

## Effective Practices

- Markdown 保存人类可评审事实，JSON 保存机械状态。
- 机械检查区分 `FAIL`、`WARN`、`REVIEW` 和 `PASS`。
- 使用真实 NGU800 MCTP change 验证 evidence、retrospective 和候选采集。

## Ineffective Practices

- 只测试模块导入，不测试用户实际执行的 CLI 入口。
- 在实现期间不及时更新任务复选框。

## Known Issues and Debt

需要第二款芯片验证、组织级 CI 接入、更多 ID 交叉引用检查，并为 L4/L5
建立单独计划。

## Reusable Knowledge Candidates

- 人工审批与机械 readiness 分离的 Gate 模型。
- 芯片级 Security Workflow + 功能级 OpenSpec 的双层事实源。

## Baseline and Documentation Follow-up

第二款芯片启动时应直接使用 initializer 创建项目，并记录模板缺口、Gate
误报/漏报和不适用字段，作为 V2 与 L4/L5 评估输入。
