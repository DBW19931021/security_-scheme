# Design：Device Personalization与制造灌装

规范性批准设计位于[《NGU800P安全软件详细设计》第10章](../../../docs/05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)，制造合同见[ADR-0026](../../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)，运行期一级动态证明见[ADR-0029](../../../decisions/ADR-0029-dice-style-single-dynamic-attestation-certificate.md)。

## Design split

1. 已批准目标：一机一密、SRC-0024 Table 34与16槽、KMS/KEK、Vendor轮换机制、Chip Root→Level1→Level2顺序、每设备单Attestation Profile、Cert0/1和typed制造接口。
2. RTL绑定：逐die隐藏个性化、硬件不可读和operation proof。
3. eHSM OTP：Device Root先于Level2、typed object、write-once和unknown隔离。
4. 证书：槽14生成Device Issuer私钥，slot8提供DICE Root公共身份证明，CA签发静态Issuer前缀，Cert0/1使用Flash A/B；GSP在每次安全冷启动生成一级动态Firmware Alias Leaf。
5. 制造启动：BootROM仍只有安全/非安全两个顶层分支；值0、DEV/MANU、Strap=0选择`MANUFACTURING_PROVISIONING`，值1/LCS异常/其他非制造组合选择`RESTRICTED_NONSECURE`。
6. 制造接口：C908 Provisioning FW是不可信caller，eHSM BL提供typed query/install/generate/proof/finalize/LCS并验证硬件制造条件、LCS、signed recipe/ticket、设备绑定和固定映射；不可逆命令无自动retry。
7. 量产提交：DEV中完成对象和最终配置产品安全启动预演，MANU只收口，`MANU→USER`最后提交；部分写只按BL/backend明确合同恢复。

## Certificate chain profile v1

本节对应[ADR-0027](../../../decisions/ADR-0027-device-certificate-chain-and-installation-profile-v1.md)与已批准的[ADR-0029](../../../decisions/ADR-0029-dice-style-single-dynamic-attestation-certificate.md)：

```text
Flash chain payload =
    Root CA Certificate DER
 || Intermediate/Product Attestation CA Certificate DER
 || Device Attestation Issuer Certificate DER

SPDM SlotID 0 Runtime CertificateChain =
    Length_LE16 || Reserved_0 || RootHash
 || Flash chain payload
 || Dynamic Firmware Alias Leaf DER
```

- 三张静态证书均采用X.509 v3；Root/Intermediate必须是CA，Device Attestation Issuer必须是受限CA（`pathLen=0`、只允许`keyCertSign`）；GSP动态Firmware Alias证书必须是非CA Leaf并只能用于Device Attestation签名。
- OTP槽8是DICE root CA asymm对象；Root SPKI绑定必须通过批准operation proof完成，具体材料/Anchor编码待Key Attribute/Profile；SPDM `RootHash`是完整Root Certificate DER摘要，两者不能混用。
- 制造Device只按产品策略生成槽14 Issuer私钥、导出公钥并签固定宽度fresh nonce PoP；不构造PKCS#10、不构造或解析通用X.509，也不接受任意digest。
- Host/CA校验PoP、signed recipe、设备/Profile绑定，构造并验证三张静态Issuer前缀，再生成signed install ticket。
- Device安装前只校验ticket签名、本地设备/Profile/槽14 Issuer公钥/slot8 Root绑定、静态前缀长度/摘要和Flash readback。
- BootROM只提交FMC最终目标readback Hash，FMC只提交GSP最终目标readback Hash；二者不实现DICE、KDF或X.509。
- GSP验证两个Measurement Entry，构造固定TCB Context；eHSM以slot13 UDS执行域分离KDF和确定性Alias KeyGen，UDS/CDI/Alias private均不导出；GSP按固定Profile组装Leaf并请求slot14签名。
- SPDM provider运行期以scatter/gather拼接Flash静态前缀与SRAM动态Leaf；动态Leaf不回写Cert0/1，reset后重建。
- Cert0/Cert1是同一身份的内部Flash A/B，不映射为两个SPDM身份；首版外部只暴露SlotID 0。
- 设备无批准可信UTC时不执行当前时间和吊销策略；固定Leaf有效期字段来自批准Profile，Verifier/CA负责完整时间/吊销检查，首版设备不下载CRL/OCSP。

规范性字段表、安装时序和错误规则见[主详设第10.10章](../../../docs/05-software-design/NGU800P安全软件详细设计.md#1010-device-private-keycsr证书签发与安装)。

## Explicit non-goals

- 不用当前Vendor通用安装接口模拟USER Key轮换。
- 不把KMS/ATE/CA合并为一个不受控脚本。
- 不在仓库保存任何量产Key明文或可复用密文。
- 不把Cert0/Cert1解释为P-256/SM2两套同时生效的设备身份。
- 不让制造Controller提供任意摘要给槽14签名，也不把PoP、Enrollment Record、install ticket或私钥写入Flash。
- 不在Device端实现通用ASN.1/X.509解析或路径验证、当前时间、CRL或OCSP策略；只允许GSP使用批准模板的固定Profile动态Leaf writer。
- 不在实施绑定到齐前猜测真实Flash base/擦除粒度、RTL personalization寄存器、Vendor新command ID或外部系统endpoint。
- 不把`non_sec_boot=1`解释为制造入口，也不允许USER+pin进入Provisioning；USER事后断言逃生位必须依赖独立强授权维修硬件路径。
