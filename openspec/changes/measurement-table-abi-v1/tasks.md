# Tasks

| Task ID | 目标仓库 | 状态 | 内容 | 完成标准 |
|---|---|---|---|---|
| OSP-MEAS-001 | security_-scheme | completed | 形成可变长度Header/Firmware/唯一State布局、CRC、commit、append和snapshot设计 | OpenSpec、ADR、接口详设和主详设一致 |
| OSP-MEAS-002 | security_-scheme | completed | 审批ADR-0022字段取舍和逻辑ABI | 负责人2026-07-27逐项批准 |
| OSP-MEAS-003 | security_-scheme | partially_completed | ADR-0028已冻结16 KiB物理Region；继续冻结产品实际`max_fw_entries` | 最大独立微核/固件实例清单到齐，且`max_fw_entries<=126` |
| OSP-MEAS-004 | gsp-pmp-rmp-omp | not_authorized | 实现生成C ABI及BootROM/FMC/GSP builder/consumer/snapshot | 单独实施授权、OPEN-CONFLICT-006 DoR满足 |
| OSP-MEAS-005 | baremetal | not_authorized | 实现ABI/golden/torn-write/reset/cache/可变count测试 | 单独测试实施授权和EMU Profile满足 |
| OSP-MEAS-006 | security_-scheme | pending | ADR批准后更新下一版高亮测试规划 | 不覆盖v0.2工作簿；待测试规划轮次 |

所有任务均禁止Git提交；当前只修改`security_-scheme`设计和追溯文件。
