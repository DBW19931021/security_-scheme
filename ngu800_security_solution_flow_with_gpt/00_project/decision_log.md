# 设计决策记录 Decision Log

## 目的

本文件用于记录 NGU800 安全方案中的架构裁决。凡是影响 Root of Trust、boot、key/cert、attestation、debug/lifecycle、interface、manufacturing/RMA 等安全主路径的设计判断，都应在这里留下可追溯记录。

## 记录字段

| Decision ID | Date | Topic | Decision | Reason | Impacted Files | Status |
|---|---|---|---|---|---|---|
| DEC-0000 | 2026-04-27 | 初始化设计变更管理流程 | 建立 decision log、changelog、open questions、CR 模板、context pack、同步检查表、影响矩阵、prompts 和 CR 同步检查脚本 | 安全方案后续会持续接收新资料和接口变化，需要先由 GPT 做设计裁决，再由 Codex 严格按 CR 落地 | `00_project/*`; `change_requests/*`; `.context/*`; `05_traceability/*`; `prompts/*`; `tools/check_cr_sync.py` | `[CONFIRMED]` |
| DEC-0001 | 2026-04-27 | `CR-0001-sec1-encryption-fw-protection-master-sync` / SEC1 强制签名 + 加密 | SEC1 在正式安全启动路径中必须签名 + 加密；SEC1 解密 / unwrap 必须由 eHSM / 安全子系统受控密码服务完成，BootROM 不直接实现复杂解密 | SEC1 是 First Mutable Stage 且来自 NOR / 本地 Flash，需要同时保护完整性和机密性，并保持 BootROM 最小化边界 | `01_constraints.md`; `02_baseline.md`; `01_boot.md`; `02_key_cert.md`; `06_interface.md`; `efuse_key_fw_header_design.md`; `mailbox_if.md`; `manufacturing_provisioning.md`; `spdm_report.md`; `06_traceability.md` | `[CONFIRMED]` |
| DEC-0002 | 2026-04-27 | `CR-0001-sec1-encryption-fw-protection-master-sync` / Root-Key-Cert 章节后置 | Root of Trust 核心基线仍在前文保留，Root / Key / Cert 详细设计后移到 boot、attestation、debug、interface、board 之后统一介绍 | 先让读者理解启动链、证明、调试和接口，再展开密钥/证书细节，避免理解顺序跳跃 | `03_detailed_design_master.md`; `03_detailed_design_master_v2.4.md`; `02_key_cert.md` | `[CONFIRMED]` |
| DEC-0003 | 2026-04-27 | `CR-0001-sec1-encryption-fw-protection-master-sync` / 板级安全并入 master | `05_board_security.md` 作为正式板级安全章节并入 master，不再作为待补章节 | `SRC-005` 已纳入输入，板级/OOB/JTAG/DMA/Power/Reset 安全边界需要进入总详设 | `05_board_security.md`; `03_detailed_design_master.md`; `03_detailed_design_master_v2.4.md`; `06_interface.md`; `04_lifecycle_debug.md` | `[CONFIRMED]` |
| DEC-0004 | 2026-04-27 | `CR-0001-sec1-encryption-fw-protection-master-sync` / Master 统一编号体系 | master 采用单一阿拉伯数字多级标题体系，删除源章节原始一级标题造成的重复编号 | 避免 `# 二、...` 与 `# 5. ...` 并列，提升总详设可读性和导出版质量 | `03_detailed_design_master.md`; `03_detailed_design_master_v2.4.md` | `[CONFIRMED]` |

## 状态定义

| Status | 含义 |
|---|---|
| `[PROPOSED]` | 已提出但尚未接受 |
| `[CONFIRMED]` | 已确认，可作为后续设计依据 |
| `[SUPERSEDED]` | 已被后续决策替代 |
| `[REJECTED]` | 已拒绝，不再采用 |
