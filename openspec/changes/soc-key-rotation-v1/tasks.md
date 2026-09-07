# Tasks

| Task ID | 目标仓库 | 状态 | 内容 | 完成标准 |
|---|---|---|---|---|
| OSP-ROT-001 | security_-scheme | completed | 深度抽取SRC-0015并冻结设计层轮换机制 | ADR、详设、规格和Source Card一致 |
| OSP-ROT-002 | security_-scheme | completed | 核对当前SRC-0018交付能力 | CE-SEC-013登记专用命令缺失和通用接口不可替代 |
| OSP-ROT-003 | security_-scheme | blocked_by_vendor_input | 绑定Vendor Bitmap、command ABI、状态查询、掉电和KMS recipe | Vendor定制TRM/Host/BL/FW/release note及KMS输入到齐；OPEN-DESIGN-014的软件设计已关闭 |
| OSP-ROT-004 | gsp-pmp-rmp-omp | not_authorized | 实现GSP内部typed rotation service及reset/RAS编排 | 另行授权，且OSP-ROT-003完成 |
| OSP-ROT-005 | baremetal | not_authorized | 实现轮换正反例、生命周期和逐掉电点EMU case | 另行授权，使用可恢复OTP/EMU策略并产出Evidence |

所有任务禁止把Git提交作为交付步骤。当前change只允许修改`security_-scheme`设计与追溯文件。
