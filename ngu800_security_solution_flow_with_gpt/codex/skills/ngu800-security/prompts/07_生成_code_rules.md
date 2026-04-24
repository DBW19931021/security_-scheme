基于以下输入生成或更新：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/`
- `security_workflow/04_impl_design/`

输出文件：
- `security_workflow/05_code_rules.md`

要求：
1. 将设计结论转成 MUST / MUST NOT / SHOULD 规则
2. 每条规则至少包含：
   - Rule ID
   - Rule Statement
   - Applies To
   - Source Constraint
   - Baseline Decision
   - Impl Document
   - Violation Impact
