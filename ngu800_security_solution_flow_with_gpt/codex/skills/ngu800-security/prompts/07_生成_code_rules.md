基于以下输入生成或更新：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/`
- `security_workflow/03_detailed_design/10_full_design.md`
- `security_workflow/04_impl_design/`

决策边界：
- Code rules 只能由已批准设计结论转换而来。
- 如果某条规则来源是 `[PROPOSED]` / `[ASSUMED]` / `[TBD]`，规则必须标注为 review-blocked 或 implementation-blocked，不得作为强制开发规则下发。

输出文件：
- `security_workflow/05_code_rules.md`

要求：
1. 将设计结论转成 MUST / MUST NOT / SHOULD 规则
2. 每条规则至少包含：
   - Rule ID
   - Rule Statement
   - Applies To
   - Source Status
   - Rule Status
   - Source Constraint
   - Baseline Decision
   - Impl Document
   - Violation Impact
3. CR-0005 后，规则必须把 `10_full_design.md` 作为代码落地主事实源；`04_impl_design` 只能作为同步分片引用
