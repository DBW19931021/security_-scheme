## Tasks

- [x] 创建 OpenSpec change `document-osr-ehsm-codebase`。
- [x] 检查 package manifest、README、CMake、顶层目录和已有 docs。
- [x] 分析 BL startup、secure boot、mailbox scheduler、command parser、image verification 和 upgrade flow。
- [x] 分析 FW startup、communication ingress、command pool、scheduler、service dispatch 和 crypto service initialization。
- [x] 分析 HOST API 的 mailbox command construction 和 response modes。
- [x] 创建中文文档 `docs/osr_ehsm_codebase_overview.md`。
- [x] 创建中文文档 `docs/osr_ehsm_security_and_command_flows.md`。
- [x] 添加 package layout、build model、BL boot、FW startup、FW dispatch、HOST mailbox flow、image verification、upgrade state flow 等 Mermaid 图。
- [x] 添加 OpenSpec proposal、design、capability spec 和 tasks。
- [x] 在 `openspec/config.yaml` 中写入“后续项目文档和 OpenSpec 文档默认使用简体中文”的规则。
- [x] 在 OpenSpec 中写入 CPU 资料准源约束：M130 核相关内容以 `docs/CPU/` 下资料为准。
- [ ] 可选后续：执行 `codegraph init -i`，用索引调用路径修正文档和图。
- [ ] 可选后续：生成 HOST API 到 command ID 到 FW/BL handler 的命令矩阵。
- [ ] 可选后续：对比 BL/FW/HOST mailbox header 定义是否存在漂移。
