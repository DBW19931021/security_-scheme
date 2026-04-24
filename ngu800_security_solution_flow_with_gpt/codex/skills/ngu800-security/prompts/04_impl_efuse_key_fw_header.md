基于以下输入生成实现级设计：
- `security_workflow/01_constraints.md`
- `security_workflow/02_baseline.md`
- `security_workflow/03_detailed_design/02_key_cert.md`
- `security_inputs/inputs_manifest.md`
- `templates/efuse_plan_template.md`
- `templates/key_hierarchy_template.md`
- `templates/boot_image_format_template.md`

输出到：
- `security_workflow/04_impl_design/efuse_design.md`
- `security_workflow/04_impl_design/key_hierarchy.md`
- `security_workflow/04_impl_design/fw_header.md`

如果当前阶段暂不拆分，也可以先输出：
- `security_workflow/04_impl_design/efuse_key_fw_header_design.md`

要求：
1. eFuse/OTP：给出分区 / 字段 / 位宽 / 访问主体 / 生命周期限制
2. Key：给出 key hierarchy / key ladder / 双算法映射
3. FW Header：给出 minimal header / signed header / signature block / enc meta
4. 尽量给出 C-like 结构
5. 明确哪些字段必须进入 signed region
