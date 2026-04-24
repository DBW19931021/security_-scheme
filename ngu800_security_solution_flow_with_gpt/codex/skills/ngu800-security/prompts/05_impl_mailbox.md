基于以下输入生成 mailbox 实现级接口定义：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/06_interface.md`
- `security_inputs/inputs_manifest.md`
- `templates/mailbox_if_template.md`

输出到：
- `security_workflow/04_impl_design/mailbox_if.md`

要求：
1. 给出 command ID 分配建议
2. 给出 request / response header
3. 给出 verify image / lifecycle / debug auth / key service 的 req/resp
4. 给出 error code model
5. 给出 lifecycle restrictions
6. 尽量给出 C-like 结构体
