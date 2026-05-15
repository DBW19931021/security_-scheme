基于以下输入生成 SPDM / Attestation report 实现级设计：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/03_attestation.md`
- `security_inputs/inputs_manifest.md`
- `templates/attestation_report_template.md`

决策边界：
- report header、measurement block、cert chain、签名覆盖范围和算法映射只能在 approved baseline / accepted CR / decision_log / 标准或接口资料支撑时标为 `[CONFIRMED]`。
- 首版字段建议必须标为 `[PROPOSED]`，开放依赖必须标为 `[TBD]`。

输出到：
- `security_workflow/04_impl_design/spdm_report.md`

CR-0005 同步要求：
- `spdm_report.md` 是编辑分片 / extracted implementation shard。
- report header、measurement block、lifecycle/debug block、cert chain、signature block、GM/国际算法映射等完整正文必须同步到 `security_workflow/03_detailed_design/10_full_design.md` 的“实现级落地详设全集”章节。

要求：
1. 给出 report header
2. 给出 measurement block
3. 给出 lifecycle/debug status block
4. 给出 cert chain block
5. 给出 signature block
6. 给出 GM / 国际算法映射
7. 尽量给出 C-like 结构
