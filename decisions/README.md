# Architecture Decision Records

重大安全决策使用 ADR，状态只使用 `proposed`、`accepted`、`rejected`、`deprecated`、`superseded`。文件名建议为 `ADR-xxxx-short-name.md`。

## 当前决策

- [ADR-0001：方案权威层级与 Vendor 边界](ADR-0001-solution-authority-and-vendor-boundary.md) — `accepted`
- [ADR-0002：基线派生与冲突升级机制](ADR-0002-baseline-derivation-and-conflict-escalation.md) — `accepted`
- [ADR-0003：W0批准原则与自检位图](ADR-0003-w0-approved-principles-and-self-test-bitmap.md) — `accepted`
- [ADR-0004：Die1 Measurement、GSP地址域与Handoff范围](ADR-0004-die1-measurement-gsp-address-and-handoff-scope.md) — `accepted`
- [ADR-0005：B0-R1地址、Counter与Vendor BL边界](ADR-0005-b0-r1-address-counter-and-vendor-bl-boundary.md) — `accepted`
- [ADR-0006：B0-R2安全RAM、Mailbox与RAS Reset边界](ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md) — `accepted`
- [ADR-0007：安全RAM生命周期复用与常驻固件范围](ADR-0007-security-ram-lifecycle-reuse-and-resident-scope.md) — `accepted`
- [ADR-0008：eHSM对安全RAM的访问边界](ADR-0008-ehsm-security-ram-access-boundary.md) — `accepted`
- [ADR-0009：NGU800P SoC Map/寄存器权威源](ADR-0009-baremetal-soc-map-register-authority.md) — `accepted`
- [ADR-0010：Vendor Direct Mailbox、阶段模式与早期RAS终态](ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md) — `accepted`
- [ADR-0011：eHSM首版单在途、Cache、Timeout与迟到响应](ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md) — `accepted`
- [ADR-0012：BootROM LCS/Counter、GSP Vendor FW与重新下发边界](ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md) — `accepted`
- [ADR-0013：GSP镜像、eHSM能力开放与Runtime隔离范围](ADR-0013-gsp-operation-image-service-and-isolation-profile.md) — `accepted`
- [ADR-0014：安全固件包、NGU Manifest与长度规范化](ADR-0014-secure-package-manifest-and-length-canonicalization.md) — `accepted`
- [ADR-0015：GSP全生命周期eHSM Service唯一Owner](ADR-0015-gsp-lifetime-ehsm-service-owner.md) — `accepted`
- [ADR-0016：GSP替代OMP/Q&P产品固件](ADR-0016-gsp-replaces-omp-product-image.md) — `accepted`
- [ADR-0017：三套产品安全固件算法Profile](ADR-0017-three-product-secure-package-algorithm-profiles.md) — `accepted`
- [ADR-0018：BootROM Strap、Lifecycle与LCS读取时序](ADR-0018-bootrom-strap-lifecycle-and-lcs-timing.md) — `accepted`
- [ADR-0019：Rollback Counter、Manifest Version与GSP Host边界](ADR-0019-rollback-counter-manifest-version-and-no-host-gsp-service.md) — `accepted`
- [ADR-0020：失败终态、Provisioning、Lifecycle、SPDM、更新、Multi-Die与发布原则](ADR-0020-runtime-provisioning-attestation-update-and-release-principles.md) — `accepted`
- [ADR-0021：SoC Key轮换采用SRC-0015 Vendor机制](ADR-0021-soc-key-rotation-vendor-mechanism.md) — `accepted`
- [ADR-0022：Measurement Table ABI v1、Commit与Snapshot](ADR-0022-measurement-table-abi-v1.md) — `accepted`
- [ADR-0023：工程文档可视化与双向链接规则](ADR-0023-document-visualization-and-bidirectional-links.md) — `accepted`
- [ADR-0024：Manifest精简、LCS异常启动策略与eHSM自检Owner](ADR-0024-manifest-simplification-boot-mode-and-ehsm-self-test.md) — `accepted`
- [ADR-0025：一机一密RTL Key与16槽OTP Key基线](ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md) — `accepted`
- [ADR-0026：量产灌装、设备证明、证书A/B与制造接口合同](ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md) — `accepted`
- [ADR-0027：证书链格式、离线签发与轻量Device安装Profile v1](ADR-0027-device-certificate-chain-and-installation-profile-v1.md) — `proposed`
- [ADR-0028：2 MiB安全SRAM精确划分与受控原地加载](ADR-0028-security-sram-fixed-layout-and-in-place-loader.md) — `accepted`
- [ADR-0029：DICE风格一级动态证明证书](ADR-0029-dice-style-single-dynamic-attestation-certificate.md) — `accepted`
- [ADR-0030：删除NGU Manifest并复用Native Header尾部承载load_addr](ADR-0030-remove-manifest-and-use-native-header-tail.md) — `accepted`
- [ADR-0031：non_sec_boot eFuse强制受限非安全启动](ADR-0031-non-sec-boot-efuse-override.md) — `accepted`
- [ADR-0032：eHSM Vendor RTL硬件实现基线与查询规则](ADR-0032-ehsm-vendor-rtl-authority-and-query-rules.md) — `accepted`
- [ADR-0033：Native Header增加CRC32格式校验](ADR-0033-native-header-crc32-format-check.md) — `accepted`
- [ADR-0034：非安全制造灌装、eHSM BL密钥安装与USER最终提交](ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md) — `accepted`

## 主文档入口

- [《NGU800P安全软件详细设计》— 当前完整软件详设与裁决落实入口](../docs/05-software-design/NGU800P安全软件详细设计.md)
