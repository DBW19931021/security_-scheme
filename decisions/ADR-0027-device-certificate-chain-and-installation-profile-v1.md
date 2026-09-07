# ADR-0027：证书链格式、离线签发与轻量Device安装Profile v1

> 2026-08-19后续裁决：[ADR-0029《DICE风格一级动态证明证书》](ADR-0029-dice-style-single-dynamic-attestation-certificate.md)保留本ADR的复杂PKI离线化、固定PoP、signed install ticket、Cert0/1 A/B和Device不解析通用X.509原则；本ADR中“三张静态证书且第三张为最终Device Leaf、Host预生成完整SPDM Blob”的拓扑已被替代。现行方案为Root→Intermediate→Device Attestation Issuer三张静态前缀，GSP每次安全启动追加一张动态Firmware Alias Leaf。

- Status: superseded_by_ADR-0029
- Date: 2026-07-29
- Owner: 项目负责人
- Related: SRC-0024、ADR-0020、ADR-0026、OPEN-DESIGN-015、OPEN-DESIGN-022、`device-personalization-and-provisioning-v1`
- Last reviewed: 2026-08-19

## Context

[ADR-0026《量产灌装、设备证明、证书A/B与制造接口合同》](ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)和SRC-0024已经冻结槽14单Device Private Key、每设备单Attestation Profile、Cert0/Cert1各64 KiB、commit-last和制造typed协议，但只描述了Root→Intermediate→Device Leaf的逻辑关系，没有冻结证书格式、设备绑定、离线签发、安装证据、SPDM包装和逐步安装过程，尚不足以直接指导CA工具、Provisioning FW和GSP Certificate Provider实现。

项目负责人进一步明确总体原则：在满足设备身份和证明安全性的前提下，PKI复杂性必须放在Provisioning Host、KMS和离线CA，Device端不构造PKCS#10、不执行完整RFC 5280证书路径验证，也不实现证书时间、CRL或OCSP策略。本ADR据此补齐轻量证书Profile，不改变一机一密、16槽OTP、单设备单长期身份或Cert0/Cert1 A/B原则。

## Superseded proposal（历史记录）

1. 首版每条设备证明证书链固定包含三张X.509 v3证书，按`Root CA DER || Intermediate/Product CA DER || Device Attestation Leaf DER`顺序组成。Provisioning Host/CA完成DER构造和完整RFC 5280验证，再生成可直接保存和返回的SPDM CertificateChain Blob；Device不解析证书DN、OID、Extension或签名路径。
2. Root证书包含在CertificateChain Blob中。OTP槽8按SRC-0024是DICE root CA asymm对象；离线Host和Device必须通过批准的slot 8 operation proof绑定Root `SubjectPublicKeyInfo`摘要，并计算SPDM `RootHash=BaseHash(full Root Certificate DER)`。截图没有冻结slot 8的材料形态或Anchor编码，相关Key Attribute/Profile到齐前本条保持实施阻断，禁止沿用旧slot6公钥Hash假设。
3. Cert0/Cert1是同一设备、同一槽14私钥和同一Profile的内部Flash A/B副本。首版SPDM只暴露一个逻辑证书槽`SlotID=0`，其内容来自启动时选中的有效Cert0或Cert1；不得把Flash槽号暴露成两个SPDM身份槽。
4. 槽14私钥按产品Provisioning策略由eHSM内部生成且不经普通接口导出。Device只导出公钥，并对固定宽度`NGU800P-DEVICE-POP-V1`结构执行一次fresh nonce PoP签名；不构造PKCS#10、不构造X.509、不接受任意摘要或任意消息签名。SRC-0024的“明文导入”是OTP属性能力，不向Controller开放通用导入。CA签发接口直接接收受控Enrollment Record：设备公钥、PoP、nonce、device binding、Profile、recipe和制造Evidence。
5. Provisioning Host/CA必须验证PoP、公钥、device binding、Profile、工单、唯一性和完整证书链。CA只从批准Profile生成字段，不复制制造站任意输入。若未来外部CA强制PKCS#10，作为独立CA适配变更重新评审，不把ASN.1构造或通用签名能力下沉Device。
6. Host完成签发和验证后，使用ADR-0026已经批准的signed recipe/typed制造信任链生成`signed_cert_install_ticket`，至少绑定recipe ID、device binding、Profile、sequence、证书数、SPDM blob长度及摘要、Root SPKI摘要、SPDM RootHash、Leaf公钥摘要和Leaf serial摘要。
7. Device安装时只做固定合同检查：验证install ticket签名；比较本地device binding/Profile/槽14公钥摘要/slot8 Root身份证明；检查长度和摘要；Flash readback；最后写Header和commit。Device不执行ASN.1/X.509字段、扩展、证书签名链、当前时间或吊销验证。
8. Flash只保存可直接返回的SPDM CertificateChain Blob和Cert Slot Header，不保存Enrollment Record、PoP nonce、CA私钥、设备私钥、MES ticket或PEM文本。install ticket和PoP保留在外部审计系统；Device RAM中的临时ticket在事务结束后清零。
9. 当前时间、吊销和PKI策略由CA、制造验收端和SPDM Requester执行。首版Device不实现可信UTC证书判断、CRL或OCSP，也不因此把证书判为有效；真实Attestation验收必须由外部Requester完成完整链和时间策略验证。
10. 证书安装执行`erase -> receive blob/ticket -> verify ticket/fixed bindings/hash -> readback -> header -> commit-last -> cold boot select -> external attestation proof`。`commit_marker`之前掉电的槽一律无效；旧槽至少保留至新槽完成一次冷启动和真实外部设备证明。
11. 具体企业OID、Subject/Issuer模板、证书有效期、CA序列号分配、CA endpoint/schema、SM2证书OID兼容矩阵、Flash base/erase粒度和SPDM transport仍是实施绑定；缺失时禁止生成量产证书或使用临时OID/test CA。

## Consequences

- 主详设、证书专题和OpenSpec可以据此形成完整候选实现合同。
- “复杂PKI离线化、Device轻量化、不在Device构造PKCS#10”继续有效；`OPEN-DESIGN-022`已由ADR-0029关闭，以上纯静态最终Leaf方案仅保留为历史记录。
- `OPEN-DESIGN-015`继续管理SPDM transport、消息/证书上限、算法协商、Measurement block和session wire；本ADR只收敛证书链对象与SPDM证书链包装。
- Vendor现有X.509 parser不再是Device证书安装前置；P-256/SM2完整Profile兼容性由离线CA/Host工具和外部SPDM互操作测试证明。
