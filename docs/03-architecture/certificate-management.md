---
title: "证书管理"
status: approved_design_with_bindings
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

定义静态制造信任前缀、DICE风格一级动态Firmware Alias证书、Cert0/Cert1存储和Host验证。规范性设计见[主详设第10、11章](../05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)及[ADR-0029](../../decisions/ADR-0029-dice-style-single-dynamic-attestation-certificate.md)。

# 产品证书模型

```text
Root CA（静态，离线HSM）
  -> Intermediate/Product CA（静态）
    -> Device Attestation Issuer（静态，slot14，CA/pathLen=0）
      -> Firmware Alias Leaf（GSP按当前TCB动态生成）
```

- Cert0/Cert1各64 KiB，只保存三张静态Issuer前缀和签名保护的紧凑issuer metadata。
- Flash不保存动态Leaf、CDI、Alias private key、PEM、CSR、CRL或OCSP。
- slot13 UDS只用于eHSM内部KDF；slot14 Device Private只签发本设备动态Leaf。
- GSP每次安全冷启动重建一张动态Leaf；相同设备/Profile/TCB允许稳定重建相同Alias Public Key。
- 动态Leaf和工作区使用GSP受保护内存，不使用Measurement固定16 KiB。
- 该能力对外称为“DICE风格一级动态证明证书链”，当前不声明完整TCG DICE Profile符合性。

# Stage职责

| Stage | DICE贡献 | 明确不做 |
|---|---|---|
| BootROM | 提交实际加载FMC最终readback digest | UDS/CDI/KDF/Key/X.509 |
| FMC | 提交实际加载GSP最终readback digest | UDS/CDI/KDF/Key/X.509 |
| GSP | 验FMC/GSP Entry，形成TCB，派生Alias Key，组装动态Leaf和Report | 导出UDS/CDI/private key，通用X.509路径验证 |
| eHSM | Hash、UDS KDF、确定性Alias KeyGen、slot14/alias签名 | ASN.1/X.509 DER组装 |

“BootROM/FMC只保存Hash”是DICE输入边界；既有Measurement Entry中的counter、load/entry和release审计字段继续保留。

# DICE TCB与Key派生

GSP从stable Measurement snapshot取得唯一FMC/GSP Entry，构造ADR-0029固定96字节`dice_tcb_context_v1`，计算：

```text
tcb_digest = ProfileHash("NGU800P-DICE-TCB-v1" || dice_tcb_context_v1)
CDI_handle = eHSM.KDF(slot13 UDS, domain label, tcb_digest, device binding, profile)
Alias_handle, AliasPublicKey = eHSM.KDF-KeyGen(CDI_handle, alias label, profile)
```

UDS、CDI和Alias private key不可导出。Host nonce不进入Key派生，只进入Report/SPDM transcript签名以提供新鲜性。

# 静态Issuer前缀

离线CA生成：

```text
static_chain_prefix = root_cert_der
                   || intermediate_cert_der
                   || device_issuer_cert_der
```

Device Issuer必须满足：

- X.509 v3；
- `BasicConstraints critical, CA=TRUE, pathLen=0`；
- `KeyUsage critical, keyCertSign`；
- Subject Public Key等于slot14公钥；
- Device Binding、Profile和Issuer用途符合受控Certificate Profile；
- 不得作为通用子CA或数据签名Key。

Root的pathLen至少覆盖Intermediate和Device Issuer两级，Intermediate的pathLen至少覆盖Device Issuer一级。离线Host完成RFC 5280验链、时间/吊销和Profile验证。Device不解析静态证书，只验证signed install ticket、本地binding/Profile/slot8 Root proof/slot14公钥、prefix digest、metadata、Flash readback和commit。

# 动态Firmware Alias Leaf

GSP固定Profile writer生成：

- X.509 v3，`BasicConstraints CA=FALSE`；
- `KeyUsage=digitalSignature`和项目Attestation EKU；
- Subject Public Key为Alias Public Key；
- AKI指向Device Issuer，SKI从Alias Public Key导出；
- 扩展绑定Profile、TCB digest、FMC/GSP digest、Lifecycle、Debug和Security State；
- serial由设备绑定、TCB和Alias Public Key确定性导出；
- `TBSCertificate`由slot14经eHSM签名；
- DER最大4096字节，超限fail-close。

GSP不实现通用证书解析器；OID、DN、AlgorithmIdentifier、SM2 ID、有效期边界和扩展模板由受控Profile生成。动态证明新增GSP峰值不得超过12 KiB。

# SPDM与Report

SPDM Slot0运行期虚拟链为：

```text
Length || Reserved || RootHash
|| static_chain_prefix
|| dynamic_firmware_alias_leaf_der
```

Certificate Provider使用Flash prefix与SRAM dynamic Leaf的scatter/gather视图，不复制完整静态链。Measurement SoC State的`cert_chain_digest`覆盖本boot完整虚拟链；Flash Header的`chain_digest`只覆盖静态prefix object。

Report至少绑定完整链标识、动态Leaf、Measurement stable snapshot、Lifecycle/Debug/Security State和Requester nonce/SPDM transcript hash，并由Alias Key签名。

Host验证顺序：静态链→动态Leaf→Leaf TCB扩展与Measurement一致性→Alias Report/transcript签名→FMC/GSP期望Hash和状态策略。

# Cert0/Cert1与安装

- 继续采用各64 KiB、erase-block aligned、Header v2、0x1000 prefix起始、commit-last、无active pointer。
- Header v2记录static prefix长度/摘要、静态证书数3、Profile、device binding、slot14 Issuer公钥和serial摘要。
- signed install ticket绑定静态prefix、issuer metadata、Root proof、设备、Profile、sequence和目标槽。
- A/B启动选择最大有效sequence；两个槽都无效时核心安全启动可继续，但DICE/SPDM证明保持`NOT_READY`。
- 动态Leaf不写Flash；reset或安全状态变化后必须重建。

# Open bindings

- eHSM UDS KDF、确定性Alias KeyGen、opaque handle签名的准确command和Key Attribute；
- P-256/SM2的企业OID、DN、有效期、AlgorithmIdentifier、SM2 ID和TCB扩展编码；
- SPDM chain/分片/transport/message上限；
- Flash准确base/erase粒度和Host CA/schema。

这些输入阻断产品实现/发布，不重开Stage分工、单层动态证书或不导出派生方向。

# Verification impact

覆盖静态CA/pathLen/KeyUsage、动态Leaf TCB扩展、FMC/GSP digest变化、相同TCB稳定派生、跨设备Issuer、CDI导出拒绝、错误slot13/14/Profile、动态DER超限、nonce重放、虚拟链分片、A/B掉电、reset后Leaf/handle失效、eHSM timeout/quarantine和Host完整验链/Report策略。

# Change history

- 2026-08-19：按SRC-0032/ADR-0029从纯静态三证书链改为三张静态Issuer前缀加一张GSP动态Firmware Alias Leaf；增加UDS/CDI不导出派生、Report签名、12 KiB峰值和SPDM虚拟链。
- 2026-08-04：按SRC-0024纠正Device Private=slot14、DICE root CA=slot8。
- 2026-07-29：建立离线CA、固定PoP、signed install ticket和Device轻量安装原则。
