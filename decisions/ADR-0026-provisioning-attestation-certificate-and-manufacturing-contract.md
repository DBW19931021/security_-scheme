# ADR-0026：量产灌装、设备证明、证书A/B与制造接口合同

> 2026-08-19后续裁决：[ADR-0029](ADR-0029-dice-style-single-dynamic-attestation-certificate.md)保留本ADR的一机一密、slot13/14、Cert0/1 A/B、固定PoP和制造接口原则；Cert0/1内容更新为Root→Intermediate→Device Attestation Issuer静态前缀，最终Firmware Alias Leaf由GSP按当前TCB动态生成。
>
> 2026-09-02后续裁决：[ADR-0034](ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)将C908 Provisioning FW固定到DEV/MANU的非安全制造子Profile，要求Vendor eHSM BL提供typed Key安装/状态/证明接口，并把产品安全启动预演和`MANU→USER`定义为最终提交。

- Status: accepted
- Date: 2026-07-29
- Owner: 项目负责人
- Related: SRC-0024、ADR-0020、ADR-0021、ADR-0025、OPEN-CONFLICT-011～013、OPEN-DESIGN-014、OPEN-DESIGN-015、`device-personalization-and-provisioning-v1`
- Last reviewed: 2026-08-04

## Context

ADR-0025已经冻结一机一密和16个OTP-KMU对象，但最终量产recipe仍需要明确Root/Level2先后顺序、单个Device Private槽的算法使用方式、UDS语义、物理Key ID、Cert0/Cert1布局及制造系统接口。项目负责人已逐项批准主详设第10章的推荐方案，并要求把外部实施输入与已批准的软件设计分开管理。

## Decision

1. 制造流程可称为MANU工序，但设备在Key灌装期间保持DEV LCS。固定顺序为：RTL逐die个性化与证明 → Chip Root(slot 0) → 其余Level1对象(slot 1～5，其中Device Root先于任何Level2) → 全部Level2对象(slot 6～15) → Device Issuer Private/UDS的目标operation proof → PoP/Enrollment/离线CA签发静态Issuer前缀 → Cert0 → 锁定 → 相邻转换至MANU再至USER → 冷启动生成动态Leaf并完成设备证明。Device不构造PKCS#10，具体边界见ADR-0027/0029。
2. 软件和测试保留P-256与SM2两套Device Attestation能力，但一台设备只能由受控Provisioning Profile选择其中一种。物理槽14只持久化所选算法的一把Device Issuer private Key，不允许同机同时使用两套长期设备私钥，也不允许同一scalar跨曲线复用。该对象的OTP属性按SRC-0024包含七项权限；产品Provisioning仍优先使用eHSM内部生成、只导出公钥和PoP的受控流程，不向Controller开放任意Key导入或任意摘要签名接口。Cert0/Cert1是所选单一Profile、单一设备身份和单一Issuer私钥的静态前缀A/B；动态Leaf只驻留本boot GSP SRAM。
3. 物理槽13的UDS按SRC-0024固定为Level 2、`asymm`对象，并具有截图列出的七项OTP/KMU权限。产品软件不得把这些属性能力扩大为Host可调用的通用Key服务；具体算法、材料形态和批准的内部usage由Provisioning Profile与Key Attribute合同冻结，不再把UDS描述为slot 11的opaque KDF-only对象。
4. 物理槽8的DICE root CA key按SRC-0024固定为Level 2、`asymm`对象并具有七项权限；其具体保存私钥、公钥还是复合eHSM对象，以及与证书Root绑定的编码，必须由Key Attribute/证书Profile实施输入明确，不得继续无依据地写成slot 6的公钥Hash。物理Key ID按ADR-0025/SRC-0024表格顺序固定为0～15；eHSM内部OTP基址和Table 34 offset公式已冻结。业务代码仍禁止散布raw OTP地址，必须由方案registry生成具名对象和地址；尚未冻结的是属性位、CRC、ECC、锁位、backend和材料，而不是slot顺序或地址公式。
5. Flash当前只承载FMC固件、Cert0和Cert1。Cert0和Cert1各占64 KiB并按擦除块对齐；若擦除块大于64 KiB，则各使用一个完整擦除块。每个证书槽固定为：
   - `0x0000～0x00FF`：256 B Header；
   - `0x0100～0x03FF`：签名保护的紧凑Issuer metadata；
   - `0x0400～0x0FFF`：保留区，未使用字节为擦除态`0xFF`；
   - `0x1000～0xFFFF`：三张证书静态Issuer前缀，最大60 KiB。
6. Cert Header v2固定为：

   | Offset | 字段 | 类型/长度 |
   |---:|---|---|
   | 0 | `magic` | `u32` |
   | 4 | `format_version` | `u16` |
   | 6 | `header_len` | `u16` |
   | 8 | `slot_id` | `u32` |
   | 12 | `sequence` | `u32` |
   | 16 | `static_prefix_len` | `u32` |
   | 20 | `cert_count` | `u32`，静态值为3；运行期完整链为4 |
   | 24 | `attestation_profile` | `u32` |
   | 28 | `logical_key_id` | `u32` |
   | 32 | `device_binding_hash` | 32 B |
   | 64 | `chain_digest` | 32 B，覆盖metadata与静态前缀对象 |
   | 96 | `issuer_public_key_hash` | 32 B |
   | 128 | `issuer_serial_hash` | 32 B |
   | 160 | `reserved` | 88 B，写零 |
   | 248 | `header_crc32c` | `u32` |
   | 252 | `commit_marker` | `u32`，最后写入 |

7. Cert0/Cert1不使用可撕裂active pointer。启动时扫描两个槽，仅接受Header v2、边界、静态前缀长度/Reserved、Profile、设备绑定、Issuer公钥/序列号摘要、静态前缀digest、CRC和commit均有效的槽，再选择最大有效`sequence`；相同`sequence`但内容不同进入`CERT_RECOVERY_REQUIRED`。更新写inactive/较旧槽，验证signed install ticket、完成读回和固定字段/hash校验后最后写`commit_marker`；旧槽至少保留到一次真实冷启动和外部Attestation证明通过。运行期SPDM链由静态前缀和GSP当前启动动态Leaf拼接，动态Leaf不回写Cert0/1；证书槽绝不保存设备私钥。完整X.509验证由Host/CA/Requester完成，不下沉Device。
8. 制造侧采用独立、签名且与产品构建隔离的C908 Provisioning FW。它不是产品GSP，进入USER后必须同时由构建和LCS保证不可达。Host传输采用canonical CBOR/COSE签名recipe，设备侧解析为固定二进制typed action；禁止raw OTP/eHSM、调用方自选slot/usage/`last_key`和任意内存访问。
9. Provisioning v1命令集合固定为：
   - `PROV_GET_CAPABILITIES`
   - `PROV_GET_IDENTITY`
   - `PROV_BEGIN`
   - `PROV_QUERY_OBJECT`
   - `PROV_INSTALL_WRAPPED_KEY`
   - `PROV_GENERATE_DEVICE_KEY`
   - `PROV_GET_DEVICE_PUBLIC_KEY`
   - `PROV_SIGN_PROOF`
   - `PROV_WRITE_CERT_SLOT`
   - `PROV_VERIFY_CERT_BINDING`
   - `PROV_TRANSITION_LCS`
   - `PROV_FINALIZE`
   - `PROV_ABORT`

   明确禁止`OTP_READ/WRITE`、`EHSM_RAW_COMMAND`、`INSTALL_KEY(slot...)`和`MEM_READ/WRITE`。
10. 制造参与方固定包含KMS/HSM、CA、MES、Manufacturing Controller、Secure ATE、Provisioning FW、eHSM和Flash。精确命令数值、transport framing、Flash base/真实擦除粒度以及KMS/CA/MES endpoint和schema属于实施绑定，不重新打开本ADR的设计裁决。

## Consequences

- `OPEN-CONFLICT-013`关闭；最终recipe可以基于本ADR继续细化，但只有取得RTL/Vendor/Flash/KMS/CA/MES实施绑定后才允许生成可执行量产工艺。
- `OPEN-DESIGN-014`在软件设计层关闭；Vendor轮换交付仍由`OPEN-CONFLICT-011`管理，一机一密RTL实现仍由`OPEN-CONFLICT-012`管理。
- `OPEN-DESIGN-015`不再包含“单设备单/双Attestation Key”的裁决，只继续管理SPDM transport、消息/证书上限、Measurement block和session wire合同。
- OpenSpec `device-personalization-and-provisioning-v1`状态改为“设计已批准、实施未授权”。
- 本ADR只修改`security_-scheme`中的设计、约束与追溯，不修改Vendor、`gsp-pmp-rmp-omp`、`baremetal`或测试工作簿，不执行Git操作。

## Review history

- 2026-08-04：按SRC-0024和负责人澄清修正物理槽、Level和权限：Device Private=slot 14、UDS=slot 13、DICE root CA=slot 8，slot 3～5为Level 1；baremetal只派生同一方案基线。
