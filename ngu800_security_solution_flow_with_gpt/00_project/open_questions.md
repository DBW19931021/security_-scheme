# 未关闭问题 Open Questions

## 目的

本文件用于集中记录尚未冻结、仍需项目组或安全负责人裁决的问题。任何阻塞 baseline、主路径设计、实现接口冻结或验证闭环的问题，都应登记在这里。

## 记录字段

| Question ID | Topic | Blocking Area | Owner | Status | Needed Decision |
|---|---|---|---|---|---|
| OQ-0001 | 设计变更管理流程试运行 | CR / GPT / Codex 协作流程 | TBD | `[OPEN]` | 后续第一个真实 CR 完成后，确认是否需要调整模板字段和状态流 |
| OQ-0002 | SEC1 是否强制加密 | Boot / FW Protection | NGU800 Security Design | `[CLOSED]` | 已由 `CR-0001` / `DEC-0001` 裁决：SEC1 正式安全启动路径必须签名 + 加密 |
| OQ-0003 | SEC2/PM/RAS/Codec 是否全部强制加密 | Boot / Key / Product Policy | TBD | `[OPEN]` | 冻结除 SEC1 外，哪些运行期镜像允许在特定产品阶段采用签名 only |
| OQ-0004 | board binding 是否参与 firmware verify | Board / Boot / Manufacturing | TBD | `[OPEN]` | 冻结 board/die binding 是否首版参与 verify / decrypt decision 及字段位置 |
| OQ-0005 | JTAG scope bitmap 与 MUX 控制权 | Debug / Board / RTL | TBD | `[OPEN]` | 冻结 CPU/GPU/DRAM/Flash/安全子系统/板级 MCU scope、CPLD/MUX 控制寄存器和关闭策略 |
| OQ-0006 | 管理子系统 DMA / firewall / UserID | Board / Interface / RTL | TBD | `[OPEN]` | 冻结管理子系统 DMA 可访问 buffer、UserID、firewall region 和审计字段 |
| OQ-0007 | Attestation report 是否包含 image protection policy | Attestation / SPDM | TBD | `[OPEN]` | 冻结 image protection policy 放在 measurement flags、lifecycle block 还是独立 policy block |
| OQ-0008 | OOB/BMC 是否允许作为 provisioning proxy | Manufacturing / Board | TBD | `[OPEN]` | 若允许，需冻结工站鉴权、OOB 桥接白名单和审计要求 |

## 状态定义

| Status | 含义 |
|---|---|
| `[OPEN]` | 尚未关闭 |
| `[IN_REVIEW]` | 正在评审 |
| `[BLOCKED]` | 阻塞设计或实现冻结 |
| `[CLOSED]` | 已关闭 |
| `[SUPERSEDED]` | 已被其他问题或决策替代 |
