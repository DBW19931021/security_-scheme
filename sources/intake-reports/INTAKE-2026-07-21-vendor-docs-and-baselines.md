# 2026-07-21 Vendor 文档与方案基线入库报告

## Scope

- Vendor PDF：15 份，共 19506456 bytes。
- NGU800P 初步方案基线：2 份，共 10708121 bytes。
- Vendor code 快照不属于本批；后续已在 SRC-0018/SRC-0019 单独登记。

## Intake result

| Source ID | 名称 | 版本 | 页数 | 初始状态 | 路径 |
|---|---|---:|---:|---|---|
| SRC-0001 | Wing-M130 Integration Manual | r1p0 | 50 | DOCUMENTED | `source-vault/vendor-docs/doc/CPU/Wing-M130_Integration_Manual.pdf` |
| SRC-0002 | Wing-M130 Processor Technical Reference Manual | r1p0 | 392 | DOCUMENTED | `source-vault/vendor-docs/doc/CPU/Wing-M130_Technical_Reference_Manual_v1p0.pdf` |
| SRC-0003 | OSR eHSM 仿真环境说明 | UNKNOWN | 25 | DOCUMENTED | `source-vault/vendor-docs/doc/eHSM仿真环境说明.pdf` |
| SRC-0004 | OSR eHSM HP Custom Design Technical Manual | UNKNOWN | 16 | DOCUMENTED | `source-vault/vendor-docs/doc/TRM/OSR_eHSM_HP_Custom_Design_Technical_Manual.pdf` |
| SRC-0005 | OSR Standard eHSM 技术参考手册 | UNKNOWN | 144 | DOCUMENTED | `source-vault/vendor-docs/doc/TRM/OSR_eHSM_HP_Technical_Reference_Manual_CN.pdf` |
| SRC-0006 | OSR TRNG FPGA Implementation Guide | v1.0 | 17 | DOCUMENTED | `source-vault/vendor-docs/doc/TRM/OSR_TRNG_FPGA_Implementation_Guide.pdf` |
| SRC-0007 | OSR TRNG RO 电路实现指导 | v1.2 | 23 | DOCUMENTED | `source-vault/vendor-docs/doc/TRM/OSR_TRNG_RO_Implementation_Guide.pdf` |
| SRC-0008 | OSR eHSM TC | UNKNOWN | 75 | DOCUMENTED | `source-vault/vendor-docs/doc/VR/OSR_eHSM_TC.pdf` |
| SRC-0009 | OSR eHSM TP | UNKNOWN | 15 | DOCUMENTED | `source-vault/vendor-docs/doc/VR/OSR_eHSM_TP.pdf` |
| SRC-0010 | OSR eHSM VR | UNKNOWN | 12 | DOCUMENTED | `source-vault/vendor-docs/doc/VR/OSR_eHSM_VR.pdf` |
| SRC-0011 | eHSM BL 硬件 OTP ROM Patch 测试说明 | 1.0 / 4019 | 16 | DOCUMENTED | `source-vault/vendor-docs/OSR_eHSM_Bootloader_Patch_Test_Guide_4019_1.0.pdf` |
| SRC-0012 | OSR eHSM Bootloader 技术参考手册 | 1.1 / 4019 | 57 | DOCUMENTED | `source-vault/vendor-docs/OSR_eHSM_Bootloader_TRM_4019_1.1.pdf` |
| SRC-0013 | OSR eHSM Firmware 技术参考手册 | 1.1 / 4019 | 90 | DOCUMENTED | `source-vault/vendor-docs/OSR_eHSM_Firmware_TRM_4019_1.1.pdf` |
| SRC-0014 | OSR eHSM Host API 驱动库参考手册 | 1.0 / 4019 | 257 | DOCUMENTED | `source-vault/vendor-docs/OSR_eHSM_Host_API_TRM_4019_v1.0.pdf` |
| SRC-0015 | 云天励飞 26Q2 定制需求方案 | 26Q2 | 6 | PROPOSED | `source-vault/vendor-docs/osr_定制需求/云天励飞26Q2 定制需求方案.pdf` |
| SRC-0016 | 芯片安全软件方案 v1.2 | v1.2 | 33 | PROPOSED | `docs/09-plans/芯片安全软件方案v1.2 (1).pdf` |
| SRC-0017 | NGU800P 芯片系统安全方案 | UNKNOWN | 56 | PROPOSED | `docs/09-plans/芯片系统安全方案.pdf` |

## Classification

- Vendor TRM、集成手册、实现指南和验证材料当前为 `DOCUMENTED`，表示文件中确有记录，不表示 NGU800P RTL 或芯片行为已确认。
- Vendor 仅指 eHSM 及其内部 Core，不包含 NGU800P SoC；Vendor 资料不得直接定义 SoC 行为。
- Vendor 定制需求方案 `SRC-0015` 整体仍为 `PROPOSED`；其中密钥轮换策略已经批准，具体落地以芯片安全软件方案为准。
- 两份 NGU800P 方案基线 `SRC-0016`、`SRC-0017` 为 `PROPOSED`，可以用于方案收敛，但不能直接升级为 `CONFIRMED`。
- 本工程中 Vendor 文档统一分类为 `NOT_CONFIDENTIAL_FOR_THIS_PROJECT`；源 PDF 上原有的 `OSR CONFIDENTIAL` 标记保留为文件客观信息，但不作为本工程保密分类。

## Confirmed baseline hierarchy

- SRC-0017 是系统/架构上位方案，负责硬件基础、系统架构和安全原则。
- SRC-0016 是依赖 SRC-0017 的软件工程落地方案，负责采纳后的最终软件方案。
- Vendor 建议被采纳后必须进入 SRC-0016 或其后续有效版本；不能从 Vendor PDF 直接提升为项目软件方案。

## Initial topics

- Wing-M130 集成和处理器行为；
- OSR eHSM 标准/定制设计；
- eHSM bootloader、firmware、host API；
- TRNG FPGA/RO 实现；
- Vendor 验证计划、用例和报告；
- NGU800P Secure Boot、生命周期、Debug、远程认证、制造灌装、OOB、多 Die、Attestation/SPDM；
- eHSM 边界内的 key 轮换与 OTP bitmap 定制输入；SoC 方案由内部基线控制。

## Known duplicates and boundaries

- Vendor code 交付树 `source-vault/vendor-code/osr_eshm/docs` 中存在与 `source-vault/vendor-docs/` 同名的若干 PDF。本批只登记 `vendor-docs` 下的资料，避免重复 Source ID；代码树后续作为 SRC-0018 登记。
- 未移动或重命名 Vendor 原始交付文件，保留其交付目录结构和溯源关系。
- 未从 Vendor 文档自动生成正式硬件结论、需求或确定性测试预期。

## Open questions

1. OSR eHSM HP 标准 TRM、Custom Design Manual 与 4019 系列 TRM 的适用关系是什么？
2. Wing-M130 文档适用于 NGU800P 的具体芯片/Die/CPU 修订是什么？
3. TC、TP、VR 各自对应的 eHSM RTL、固件和验证环境版本是什么？
4. `SRC-0016` 和 `SRC-0017` 的正式 Owner、批准版本、批准日期和适用芯片修订是什么？
5. 已批准的密钥轮换策略是否已经完整体现在 SRC-0016 v1.2？
6. Vendor code 快照的交付批次、整体版本和与各 TRM 的映射是什么？

## Next processing

1. 由负责人确认适用 eHSM/Core 版本、资料所有者和两份内部方案的控制信息；Vendor 文档保密分类已关闭为不保密。
2. 对 `SRC-0012`～`SRC-0015` 与 `SRC-0016`/`SRC-0017` 进行第一轮需求和架构差距分析。
3. Vendor code 快照已登记为 SRC-0018，并已把 bootloader/firmware/host/test 组件版本初步映射到对应 TRM；版本差异仍待确认。
4. Vendor 建议只有被采纳并写入芯片安全软件方案后，才更新 requirements、docs/03-architecture、docs/04-interfaces 或 OpenSpec。
