基于以下输入生成或更新 lifecycle / manufacturing 实现级设计：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/04_lifecycle_debug.md`
- `security_workflow/03_detailed_design/07_manufacturing_rma.md`
- `security_inputs/inputs_manifest.md`
- `templates/impl_design_template.md`

决策边界：
- lifecycle 编码、debug scope、RMA 策略、provisioning 命令、OTP lock 顺序和审计字段只有在 approved baseline / accepted CR / decision_log / 官方接口资料支撑时才能标为 `[CONFIRMED]`。
- 没有批准来源时，只能写 `[PROPOSED]`、`[ASSUMED]` 或 `[TBD]`。

输出到：
- `security_workflow/04_impl_design/lifecycle_control.md`（如拆分）
- `security_workflow/04_impl_design/manufacturing_provisioning.md`

CR-0005 同步要求：
- `04_impl_design` 是编辑分片 / extracted implementation shard。
- lifecycle、debug auth、MANU→USER、RMA、audit、error model 等完整实现级正文必须同步到 `security_workflow/03_detailed_design/10_full_design.md` 的“实现级落地详设全集”章节。

要求：
1. 给出 lifecycle 状态、允许命令、禁止命令和转换条件
2. 给出 debug auth、scope、expire、audit 的实现字段
3. 给出 MANU → USER 的 provisioning / lock / verify / audit 顺序
4. 给出 RMA / DEBUG 的进入、退出和重新冻结规则
5. 给出失败处理和不可回滚动作的 error model
6. 给出需要 ChatGPT / owner 冻结的问题清单
