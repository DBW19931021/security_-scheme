基于以下输入生成或更新：
- `security_inputs/inputs_manifest.md`
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/`
- `security_workflow/03_detailed_design/10_full_design.md`
- `security_workflow/04_impl_design/`
- `security_workflow/05_code_rules.md`

决策边界：
- Traceability 必须暴露 `[PROPOSED]`、`[ASSUMED]`、`[TBD]` 的链路状态。
- 不得用 traceability 把未批准设计伪装成可实现、可测试、已冻结需求。

输出文件：
- `security_workflow/06_traceability.md`

要求：
生成：
Source → Source Status → Constraint → Baseline → Detailed Design → Impl Design → Code Module → Test
的追踪矩阵。

CR-0005 后，traceability 必须显式表达：
- `10_full_design.md` 是完整详设与代码落地主入口。
- `04_impl_design` 是同步分片，不是独立事实源。
- 任何实现级字段若只存在于 `04_impl_design` 而不在 `10_full_design.md` 中可见，应标为 sync failure。
