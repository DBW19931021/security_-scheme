---
title: "设备证明"
status: review_ready_with_open_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0024
  - SRC-0032
owners:
  - GSP
last_reviewed: 2026-08-19
supersedes: []
superseded_by: []
---

# Purpose

维护Device Attestation专题；规范性设计见[《NGU800P安全软件详细设计》第11章](../05-software-design/NGU800P安全软件详细设计.md#第11章-device-attestationspdm与内部密码服务)。

# Baseline

- 证明服务只在安全启动、eHSM FW、必需Runtime和Measurement稳定后启动。
- 签名经GSP唯一`security_service_task`调用eHSM；slot14持久Device Issuer key只签动态Leaf，当前TCB Alias Key签Report/SPDM transcript。软件必须支持P-256和SM2两条能力路径。
- ADR-0026/SRC-0024冻结每设备Provisioning Profile只选择P-256或SM2之一进入slot14和Cert0/1，禁止同一scalar跨曲线复用；同机不同时使用两套长期身份。
- ADR-0029固定Root→Intermediate→Device Attestation Issuer三张静态前缀，GSP以FMC/GSP Measurement和slot13 UDS生成一级动态Firmware Alias Leaf。BootROM/FMC不生成证书。
- Cert0/Cert1仅作静态前缀A/B；SPDM Slot0运行期拼接动态Leaf。Device不解析静态X.509，GSP只用固定Profile writer组装动态Leaf；完整验链由Host完成。
- SPDM 1.2 secure session固定使用动态Alias Leaf的证书`KEY_EXCHANGE -> FINISH`，拒绝PSK和无会话fallback。Measurement不返回地址，只返回summary block。
- 报告来自当前启动已最终完成且双Header读取一致的稳定Measurement snapshot；Die1保持独立实例。
- 任何证书/key/Measurement前置条件失败都不得返回伪证明。

# Open questions

单设备单Profile、DICE单层Stage分工和动态证书方向已经由ADR-0026/0029关闭。剩余输入是eHSM UDS KDF/KeyGen命令和Key Attribute、企业OID/DN/有效期、SPDM transport、完整链/消息上限、Measurement block和session wire；未冻结前不得进入Responder发布或EMU PASS判定。

# Verification impact

分别覆盖P-256/SM2静态Issuer→动态Leaf→Alias Report签名及`KEY_EXCHANGE -> FINISH`端到端路径，并覆盖FMC/GSP hash变化、相同TCB稳定Alias、错误TCB扩展、跨设备Issuer、CDI导出拒绝、PSK拒绝、nonce/transcript篡改、并发snapshot、eHSM timeout、reset重建和敏感buffer清零。

# Change history

- 2026-08-19：按SRC-0032/ADR-0029从静态Leaf证明改为三张静态Issuer前缀加GSP一级动态Firmware Alias Leaf，Report/SPDM签名使用UDS/TCB派生的Alias Key。
- 2026-07-29：按负责人原则把PKI复杂性移至Host/CA；Device不构造PKCS#10、不解析X.509。
- 2026-08-04：按SRC-0024把Device Private物理槽从早期误记slot7纠正为slot14。
- 2026-07-29：建立ADR-0027候选；补充三证书链、PKCS#10和Cert A/B到SPDM SlotID 0的关系。
- 2026-07-29：接受ADR-0026；软件保留P-256/SM2能力，但每台设备只使用一个长期Attestation Profile。
- 2026-07-29：同步ADR-0025单Device Private槽；软件保留双算法能力，单设备选择或扩槽双路径等待OPEN-CONFLICT-013。
- 2026-07-28：冻结静态X.509证书握手、拒绝PSK、P-256与SM2双Attestation路径、Measurement summary且不返回地址；剩余wire输入保持`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
- 2026-07-24：同步主详设第11章。
