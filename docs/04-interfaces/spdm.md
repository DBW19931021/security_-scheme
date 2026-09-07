---
title: "SPDM 接口"
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

定义SPDM Responder与DICE风格一级动态证书的接口边界；主合同见[详设第11章](../05-software-design/NGU800P安全软件详细设计.md#第11章-device-attestationspdm与内部密码服务)和[ADR-0029](../../decisions/ADR-0029-dice-style-single-dynamic-attestation-certificate.md)。

# First-release profile

产品支持SPDM 1.2的Version、Capabilities、Algorithms、Digests、Certificate、Challenge、Measurements和secure session。只允许证书`KEY_EXCHANGE -> FINISH`，拒绝PSK和无会话fallback。软件支持P-256与SM2两套能力，每台设备只启用Provisioning Profile选择的一套Issuer/Alias路径。

Measurement来自stable snapshot，不返回内部地址，保留Die1独立实例。BootROM不作为普通Entry；FMC和GSP固定为前两个Entry，并作为DICE TCB输入。

# Runtime certificate-chain mapping

Flash Cert0/Cert1保存：

```text
static_chain_prefix = Root CA DER
                   || Intermediate/Product CA DER
                   || Device Attestation Issuer DER
```

GSP验证FMC/GSP Measurement后，通过eHSM以slot13 UDS派生当前Alias Key，组装并由slot14签发动态Firmware Alias Leaf。SlotID 0的运行期虚拟链为：

```text
SPDM CertificateChain:
  Length_LE16 || Reserved_0
  || RootHash = BaseHash(full Root DER)
  || static_chain_prefix
  || dynamic_firmware_alias_leaf_der
```

- Device Issuer必须是`CA=TRUE,pathLen=0,keyCertSign`；动态Leaf必须是`CA=FALSE,digitalSignature`。
- 动态Leaf公钥等于当前Alias Public Key，扩展绑定TCB/FMC/GSP/Lifecycle/Debug/Security State。
- `Length`由GSP运行期计算；Certificate Provider对Flash prefix和SRAM Leaf提供锁定的scatter/gather分片视图。
- Flash `chain_digest`覆盖static prefix object；Measurement SoC State的`cert_chain_digest`覆盖本boot完整虚拟链。
- GSP不解析静态X.509；只校验Header v2、issuer metadata、slot14公钥和摘要，并用固定Profile writer生成Leaf。
- 动态Leaf最大4096字节，reset后重新生成。

# Signing

- slot14 Device Issuer private key只签动态Leaf的TBSCertificate。
- 当前Alias private key只以opaque handle存在于eHSM，签Challenge、Measurement、Report或session transcript。
- Host nonce不参与Alias Key派生，但必须进入签名transcript，以保证新鲜性。
- eHSM timeout或completion unknown时Alias handle和证书状态quarantine，Responder不得返回成功。

# Report verification

Host/Requester必须按顺序：

1. 验证Root→Intermediate→Device Issuer静态链及时间/吊销；
2. 验证Device Issuer对动态Leaf的签名和CA/pathLen/KeyUsage；
3. 比较动态Leaf的TCB扩展与Measurement stable snapshot和状态；
4. 用Alias Public Key验证nonce/transcript/Measurement签名；
5. 比较FMC/GSP期望Hash、Lifecycle、Debug和安全启动策略。

# Open bindings

`OPEN-DESIGN-015`继续冻结transport、完整链/分片/消息上限、Measurement block index和session wire；`OPEN-DESIGN-023`冻结企业OID/DN/有效期/SM2编码以及eHSM UDS KDF/确定性Alias KeyGen/opaque sign的准确command和Key Attribute。上述输入未到齐前，Responder产品发布保持阻断。

# Verification impact

覆盖P-256/SM2静态Issuer与动态Leaf、Slot0、RootHash、虚拟链分片、TCB扩展、FMC/GSP hash变化、相同TCB稳定Alias、跨设备Issuer、非CA Issuer、DER超限、CDI导出拒绝、nonce/transcript重放、Measurement并发、eHSM timeout、reset后重建、PSK/降级拒绝和Host完整Report策略。

# Change history

- 2026-08-19：按SRC-0032/ADR-0029将Slot0从Host预生成静态Leaf Blob改为Flash静态Issuer前缀加GSP动态Firmware Alias Leaf；Challenge/Measurement签名改用当前Alias Key。
- 2026-07-29：确立复杂PKI离线化、Device轻量安装和Cert0/1映射Slot0原则。
