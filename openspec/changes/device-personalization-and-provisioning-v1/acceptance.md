# Acceptance

## Design acceptance

- 项目负责人已于2026-07-29批准ADR-0026范围内设计项；2026-08-19又以ADR-0029批准静态Issuer前缀加GSP一级动态Firmware Alias Leaf并关闭OPEN-DESIGN-022。
- 已批准内容与候选内容逐项分级。
- SRC-0024 Table 34、16槽表、密钥层级、KMS/KEK数据流、制造状态机、证书A/B布局、X.509链格式、PoP/Enrollment、离线签发、轻量安装时序及验证边界均可从主详设直接阅读；baremetal派生表逐行一致。
- 真实秘密不出现在文档、日志示例或配置。
- 所有不可逆点包含掉电、unknown、readback/proof和停止条件。
- 与Vendor 4019及RTL当前资料的差距不被描述为已实现。
- DEV/MANU非安全制造子Profile与`RESTRICTED_NONSECURE`的矩阵、权限隔离和无fallback已明确。
- eHSM BL不信任C908、typed制造能力、对象partial状态、恢复条件和USER最终提交门禁已明确。

## Certificate profile acceptance

- [x] 项目负责人以ADR-0029批准并关闭OPEN-DESIGN-022；ADR-0027中复杂PKI离线化原则继续有效，纯静态最终Leaf拓扑被替代。
- [x] 固定三张静态Issuer证书DER顺序、一级动态Leaf、Root包含规则和Root Anchor/RootHash差异已明确。
- [x] Device不构造PKCS#10，只签固定PoP；Host/CA接收Enrollment Record完成离线签发。
- [x] Cert0/Cert1静态前缀A/B、GSP动态Leaf与SPDM外部SlotID 0运行期虚拟链的映射已明确。
- [x] Host/CA全量链验证与Device ticket/绑定/hash轻量检查边界已明确。
- [x] Device不解析证书时间、不实现CRL/OCSP的边界已明确。
- [ ] OPEN-DESIGN-023所列企业OID、DN模板、有效期、serial/SM2策略、eHSM派生命令及CA endpoint/schema形成受控实施Profile。

## Implementation acceptance

实施验收等待RTL个性化合同、Vendor BL typed制造API及状态/partial-write/接受点/LCS语义、OPEN-DESIGN-023的eHSM/证书Profile、Key Attribute/CRC/ECC/backend、Flash map/erase粒度、KMS/CA/MES接口、CA Profile、Provisioning wire数值和USER独立维修入口绑定；Table 34 offset、16-slot与证书拓扑不再列为缺失。本OpenSpec当前不授权代码实施。
