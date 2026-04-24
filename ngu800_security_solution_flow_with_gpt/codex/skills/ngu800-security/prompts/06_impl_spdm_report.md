基于以下输入生成 SPDM / Attestation report 实现级设计：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/03_attestation.md`
- `security_inputs/inputs_manifest.md`
- `templates/attestation_report_template.md`

输出到：
- `security_workflow/04_impl_design/spdm_report.md`

要求：
1. 给出 report header
2. 给出 measurement block
3. 给出 lifecycle/debug status block
4. 给出 cert chain block
5. 给出 signature block
6. 给出 GM / 国际算法映射
7. 尽量给出 C-like 结构
