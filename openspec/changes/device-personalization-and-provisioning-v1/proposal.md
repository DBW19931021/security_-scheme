# Proposal：Device Personalization、16槽OTP、Cert0/1与制造灌装v1

## Why

现有详设第10章最初只描述了原则，没有形成可指导RTL Key个性化、KMS、OTP灌装、设备Issuer私钥/静态证书前缀签发、Flash Cert0/1更新和制造系统对接的完整工程合同。项目负责人已批准一机一密、最终16槽OTP对象表和SRC-0015 KEK流程，需要把批准目标与待裁决物理绑定分开管理。ADR-0027先冻结复杂PKI离线化原则；2026-08-19又由ADR-0029冻结静态Issuer前缀加GSP一级动态Firmware Alias Leaf的运行期拓扑，并关闭OPEN-DESIGN-022。

## Scope

- 在`security_-scheme`补充正式详细设计、ADR、冲突、接口候选、测试和追溯。
- 不修改Vendor、`gsp-pmp-rmp-omp`、`baremetal`和测试工作簿。
- 不执行任何Git操作。

## Accepted inputs

- RTL Key量产一机一密；
- OTP-KMU采用ADR-0025所列16个物理Key对象；
- OTP Table 34和16-slot对象、Level、Key类型及已给权限以SRC-0024为唯一方案基线，baremetal只派生实现和验证记录；
- SoC Key轮换采用SRC-0015双层KEK封装；
- Flash当前只存FMC固件、Cert0和Cert1。

## Approved design outputs

- Cert0/Cert1各64 KiB、擦除块对齐、commit-last、扫描选择最大有效sequence；
- 制造系统采用signed recipe和typed command，不暴露raw OTP/eHSM；
- 单一Device Private slot14按每设备Attestation Profile选择P-256或SM2；
- Chip Root后安装slot1～5 Level1，Device Root在slot6～15 Level2对象之前可用，保持DEV完成Key灌装后再切换MANU/USER；
- UDS按SRC-0024固定为slot13/Level2/asymm/七项权限；具体算法、材料形态和产品内部usage由Key Attribute/Profile实施合同补齐。

以上内容已由项目负责人于2026-07-29批准，并由[ADR-0026](../../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)冻结。精确Vendor/RTL/Flash/KMS/CA/MES参数属于实施绑定，不重新打开本设计。

## Proposed certificate profile extension

[ADR-0029](../../../decisions/ADR-0029-dice-style-single-dynamic-attestation-certificate.md)规定首版固定Root CA、Intermediate/Product Attestation CA和Device Attestation Issuer三张X.509 v3静态前缀；Host/CA负责静态前缀构造与验链，Device制造路径只生成Issuer Key、签固定PoP、验证signed install ticket和本地绑定/hash并保存前缀。安全冷启动时BootROM只记录FMC Hash、FMC只记录GSP Hash，GSP按固定Profile组装一级动态Firmware Alias Leaf，eHSM以slot13 UDS派生不可导出的Alias Key并使用slot14签发Leaf；Cert0/Cert1仅作静态前缀内部A/B，对外仍映射SPDM SlotID 0。复杂PKI离线化和一级动态拓扑均已确认，准确OID/DN/有效期、eHSM派生命令及SPDM wire由OPEN-DESIGN-023/015管理。

## Success criteria

主详设第10章能够独立说明资产、层级、角色、数据流、接口、状态机、掉电、错误、审计、证书链格式、PoP、离线签发、轻量安装、验证和测试；批准设计、待裁决Profile与未到齐的实施绑定明确分离，不能把候选或批准目标误认为已实现。
