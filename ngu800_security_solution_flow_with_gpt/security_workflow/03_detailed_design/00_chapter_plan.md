# NGU800 章节级详设规划

## 章节清单

| File | Goal | Key Constraints | Key Baseline Decisions | Need Impl Sync |
|---|---|---|---|---|
| 00_architecture.md | 定义总体架构与边界 | C-ROOT-01 / C-HOST-01 / C-ACCESS-01 | RoT=eHSM / Host不可信 | mailbox / lifecycle |
| 01_boot.md | 定义 secure boot 链与执行放行 | C-BOOT-* / C-UPDATE-01 | first verifier=eHSM / release owner=SEC | fw_header / mailbox |
| 02_key_cert.md | 定义 key/cert 体系 | C-ROOT-01 / C-KEY-* / C-ATT-01 | key不出eHSM / 双算法 | efuse / key hierarchy / fw header |
| 03_attestation.md | 定义 identity 和 report | C-ATT-01 / C-DEBUG-02 | attestation key在eHSM | spdm_report |
| 04_lifecycle_debug.md | 定义生命周期与调试策略 | C-KEY-02 / C-DEBUG-* | USER禁未授权debug | lifecycle_ctrl |
| 05_board_security.md | 定义 BMC / OOB / SMBus 边界 | host_boundary / board_security | board trust model | board binding |
| 06_interface.md | 定义 mailbox 和外部接口边界 | C-IF-01 / C-ACCESS-* | secure service via mailbox | mailbox_if |
| 07_manufacturing_rma.md | 定义制造 / 灌装 / RMA | C-MFG-01 | MANU→USER冻结 | provisioning |
| 08_failure_recovery.md | 定义失败处理和恢复 | C-UPDATE-* | anti-rollback | recovery path |
| 09_risks_open_issues.md | 风险 / 冻结项 / 开放问题 | 全部 | freeze-sensitive items | all |
