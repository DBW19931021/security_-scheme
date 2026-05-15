基于以下输入生成 mailbox 实现级接口定义：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/06_interface.md`
- `security_inputs/inputs_manifest.md`
- `templates/mailbox_if_template.md`

决策边界：
- command ID、payload、错误码、timeout、busy 并发语义只有在官方 eHSM/RTL/Host API、accepted CR 或 decision_log 支撑时才能标为 `[CONFIRMED]`。
- 没有批准来源时，字段和结构体只能作为 `[PROPOSED]` 实现建议，供 ChatGPT / owner / RTL / FW review。

输出到：
- `security_workflow/04_impl_design/mailbox_if.md`

CR-0005 同步要求：
- `mailbox_if.md` 是编辑分片 / extracted implementation shard。
- command ID、request/response、状态机、错误码、lifecycle restrictions 等完整正文必须同步到 `security_workflow/03_detailed_design/10_full_design.md` 的“实现级落地详设全集”章节。
- 若分片与 `10_full_design.md` 冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

要求：
1. 给出 command ID 分配建议
2. 给出 request / response header
3. 给出 verify image / lifecycle / debug auth / key service 的 req/resp
4. 给出 error code model
5. 给出 lifecycle restrictions
6. 尽量给出 C-like 结构体
