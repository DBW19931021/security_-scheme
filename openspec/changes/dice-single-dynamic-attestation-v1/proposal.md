# Proposal

## 背景

当前BootROM/FMC已经逐级提交FMC/GSP Measurement，但证书完全静态。SRC-0032要求BootROM只贡献FMC Hash、FMC只贡献GSP Hash，最终由GSP根据UDS和测量生成一级动态X.509证书。

## 目标

- 不增加BootROM/FMC的X.509、KDF和Key处理。
- 固定DICE TCB上下文、UDS不导出派生和单层Alias Key。
- 把静态链第三张证书改为受限Device Attestation Issuer。
- 由GSP组装动态Firmware Alias Leaf和签名Report。
- 保持Measurement 16 KiB专用区不被证书工作区复用。

## 非目标

- 不实现每stage多层DICE证书。
- 不声明完整TCG DICE Profile符合性。
- 不在Device实现通用X.509路径验证、CRL或OCSP。
- 不冻结Vendor eHSM command ID、企业OID、SPDM分片和transport数值。

## 审批状态

ADR-0029已接受设计方向。产品代码仍需单独授权，并受eHSM KDF-KeyGen能力、Key Attribute、证书OID/Profile和SPDM wire输入约束。
