---
name: soc-security-design-review
description: "Review SoC security designs, OpenSpec proposals, and architecture changes across assets, threats, trust boundaries, boot, keys, lifecycle, debug, update, recovery, attestation, isolation, manufacturing, RMA, and fault handling. Use before approving security architecture or implementation-impacting changes."
---

# 安全设计评审

1. 读取相关 Source Card、需求、威胁模型、ADR、OpenSpec 和待确认问题。
2. 分开列出已确认事实、正式文档事实、Vendor 实现、假设和提案。
3. 评审资产、攻击面、信任边界、Secure Boot、Key Management、Lifecycle、OTP/eFuse、Debug/Test、Firmware Update、Recovery、Anti-rollback、Attestation、Host/BMC/OOB、Die 间信任、故障注入、量产、RMA、运维和测试可达性。
4. 检查状态机、异常路径、掉电恢复、回滚、权限边界和密钥流。
5. 对每项发现给出证据、影响、严重度、建议和关闭条件。
6. 遇到资料冲突或未确认硬件行为时停止形成确定性结论并创建 open question。

使用 `templates/design-review.md` 输出可审查报告。
