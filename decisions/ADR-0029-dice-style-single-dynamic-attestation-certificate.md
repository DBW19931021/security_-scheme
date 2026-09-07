# ADR-0029：DICE风格一级动态证明证书

- 状态：accepted
- 日期：2026-08-19
- 决策人：项目负责人
- 来源：SRC-0032
- 关联：ADR-0022、ADR-0026、ADR-0027、ADR-0028
- 替代关系：替代主详设和证书专题中“本版本只使用静态X.509、DICE仅为未来增强”的结论；保留ADR-0027的复杂PKI离线化、Cert0/1 A/B和Device不解析通用X.509原则

## Context

当前设计已经由BootROM提交FMC Measurement、FMC提交GSP Measurement，并由GSP提供SPDM和证明服务，但证书仍被定义为完全静态。项目负责人要求补入DICE风格能力：BootROM/FMC保持极小实现，只保存下一阶段实际加载Hash；GSP汇总测量并生成一级动态证书，功能关系参考SRC-0032截图。

如果直接让现有`Device Attestation Leaf`签发另一个动态证书，会违反X.509 CA/KeyUsage路径约束。因此静态第三张证书必须定义为受限Device Attestation Issuer，动态Firmware Alias证书才是本次启动的最终Leaf。

## Decision

### 1. 产品声明边界

产品实现“DICE风格一级动态证明证书链”：静态制造信任链后只增加一个由GSP每次安全冷启动重新生成的Firmware Alias Leaf。当前不声明完整符合TCG DICE Hardware Requirements、全部Layering Profile或全部扩展OID。

### 2. Stage职责

1. BootROM在FMC完成Vendor认证、Header Overlay/typed-stage policy、原地加载和最终目标readback后计算FMC digest，提交第一个Measurement Entry并release FMC。BootROM不读取UDS、不生成CDI/Key/CSR/X.509。
2. FMC在GSP完成同等门禁后计算GSP digest，提交第二个Measurement Entry并release GSP。FMC不读取UDS、不生成CDI/Key/CSR/X.509。
3. “只保存Hash”是DICE输入边界；既有Measurement Entry继续保存counter、load/entry和release审计元数据，不能为了DICE删除安全启动门禁事实。
4. GSP必须验证稳定Measurement snapshot中Entry 0/1分别是唯一FMC/GSP、producer和commit有效、digest来自最终目标readback，然后才可进入DICE派生。

### 3. TCB摘要和不导出派生

GSP构造固定96字节`dice_tcb_context_v1`：

| Offset | Size | 字段 |
|---:|---:|---|
| 0 | 8 | `magic="NGDICE1\0"` |
| 8 | 2 | `version=1` |
| 10 | 2 | `attestation_profile` |
| 12 | 2 | `hash_algorithm` |
| 14 | 2 | `digest_len=32` |
| 16 | 4 | `lifecycle` |
| 20 | 4 | `debug_state` |
| 24 | 4 | `security_state`位图 |
| 28 | 4 | `reserved=0` |
| 32 | 32 | `fmc_digest` |
| 64 | 32 | `gsp_digest` |

所有整数little-endian。`security_state`至少由受控Registry编码secure boot路径、eHSM self-test、rollback proof和关键Firewall lock状态；未知位必须为0。`tcb_digest=Hash("NGU800P-DICE-TCB-v1" || dice_tcb_context_v1)`。

eHSM执行不导出派生：

```text
CDI_handle = KDF(UDS slot13,
                 label="NGU800P-DICE-CDI-v1",
                 context=tcb_digest || device_binding_digest || attestation_profile)

AliasKey_handle, AliasPublicKey = KDF-KeyGen(
                 CDI_handle,
                 label="NGU800P-DICE-ALIAS-v1",
                 context=attestation_profile)
```

UDS、CDI和Alias private key始终留在eHSM；GSP只取得opaque handle和Alias Public Key。相同设备、Profile和TCB状态允许稳定派生相同Alias Key；会话新鲜性由Host nonce和Report签名提供，不把随机nonce加入TCB派生。

### 4. 静态前缀和一级动态证书

Cert0/Cert1继续保存Host/CA离线生成并验证的静态前缀：

```text
Root CA -> Intermediate/Product CA -> Device Attestation Issuer
```

第三张证书绑定slot14 Device Private Key，必须是受限CA：`BasicConstraints CA=TRUE,pathLenConstraint=0`，`KeyUsage`包含`keyCertSign`，不得签发除本设备Firmware Alias之外的证书。GSP不解析这三张证书；Provisioning同时安装签名保护的紧凑issuer metadata，提供Issuer DN、AKI、Profile、有效期边界和静态前缀摘要。

GSP使用固定Profile X.509 writer组装唯一动态`Firmware Alias Certificate`：

- X.509 v3、`BasicConstraints CA=FALSE`；
- Subject Public Key为Alias Public Key；
- `KeyUsage=digitalSignature`，EKU为项目Attestation OID；
- AKI指向Device Attestation Issuer，SKI由Alias Public Key导出；
- 受认证项目扩展绑定Profile、`tcb_digest`、FMC digest、GSP digest、Lifecycle、Debug和`security_state`；
- serial由`Hash(device_binding_digest || tcb_digest || AliasPublicKey)`确定性截断并保证为正非零；
- `TBSCertificate`由slot14 Device Private Key经eHSM签名；
- 动态证书最大DER长度首版固定4096字节，超限fail-close。

eHSM只提供Hash/KDF/KeyGen/Sign；ASN.1、X.509 v3、DER长度和扩展组装全部在GSP完成。GSP不引入通用证书路径验证器。

### 5. Report和Host验证

GSP形成规范化Report：静态前缀标识/摘要、动态证书、稳定Measurement snapshot、Lifecycle/Debug/Security State、Requester nonce或SPDM transcript hash。Report正文由Alias private key签名。

Host验证顺序固定为：

1. 验证Root→Intermediate→Device Attestation Issuer静态链和时间/吊销策略；
2. 用Device Attestation Issuer验证动态Firmware Alias证书；
3. 比较动态证书扩展中的FMC/GSP/TCB/状态与Report及Measurement；
4. 用动态证书Alias Public Key验证Report或SPDM transcript签名和Requester nonce；
5. 根据期望FMC/GSP Hash、Lifecycle、Debug和安全状态判定认证通过。

SPDM Slot0返回`RootHash || static prefix || dynamic Firmware Alias Leaf`；Challenge、Measurement或session transcript签名使用Alias private key。准确transport、分片和消息上限仍由OPEN-DESIGN-015冻结。

### 6. SRAM和生命周期

- 固定16 KiB Measurement Region仍只保存Measurement，不存动态证书、CDI、Key或ASN.1 scratch。
- 动态证书只存在于当前boot的GSP受保护内存；reset后失效并重新派生，不写Cert0/Cert1。
- 首版为GSP新增不超过12 KiB的动态证明工作峰值：4 KiB最终证书、最多3 KiB TBS/DER工作区、最多1 KiB KDF/Sign I/O和最多4 KiB Report/链组装元数据；Measurement stable snapshot和SPDM transport缓冲按既有预算管理，不重复计入。
- 工作区从GSP静态880 KiB或FMC回收后的128 KiB动态池分配，必须进入link map和高水位门禁；错误、timeout或quarantine时按eHSM生命周期合同清零/隔离。

## Consequences

- BootROM和FMC ROM/固件不增加X.509、KDF或证书链处理栈，只复用现有最终readback digest和Measurement writer。
- GSP增加固定Profile X.509 writer、DICE context、typed eHSM派生、动态证书Provider和Host Report组装。
- ADR-0027中第三张静态`Device Attestation Leaf`改为`Device Attestation Issuer`；其离线PKI、ticket、A/B安装和Device不解析通用X.509原则继续有效。
- Vendor必须提供或适配“UDS不导出KDF→确定性Alias KeyGen→opaque handle签名”typed能力；准确command/Key Attribute未绑定前，代码实现保持阻断。

## Rejected alternatives

- BootROM或FMC生成X.509：增加ROM/FMC栈和攻击面，不符合用户要求。
- 用非CA Device Leaf签动态证书：不满足标准X.509路径约束。
- 把CDI或Alias private key输出到C908 SRAM：破坏DICE密钥隔离目标。
- 把动态证书写入Measurement 16 KiB：破坏固定Region职责和容量合同。
- 每个stage都生成一张动态证书：超出本次“一级动态证书”范围。
