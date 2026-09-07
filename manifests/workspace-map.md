# 工作空间映射

| ID | 路径 | 职责 | Git 边界 |
|---|---|---|---|
| REPO-SECURITY-SCHEME | `.` | 安全方案、需求、决策、正式详设、全部项目任务、测试规划/工作簿、开发追溯和 Evidence 索引 | 当前仓库 |
| REPO-GSP-FIRMWARE | `../gsp-pmp-rmp-omp` | GSP 负责正式安全软件代码和必要实现侧测试 | 独立公司仓库 |
| REPO-BAREMETAL-SECURITY-TEST | `../baremetal` | GSP 负责可执行测试代码、EMU runner、测试工具和必要测试数据 | 独立公司仓库 |
| SOURCE-VAULT | `./source-vault` | 原始资料 | 是否跟踪大文件按资料策略决定 |
| SRC-0035-EHSM-VENDOR-RTL | `./source-vault/vendor_rtl` | eHSM内部硬件实现细节、RTL实现基线和问题查询第一入口 | 当前仓内只读快照；树哈希锁定，不是独立Git或RTL开发树 |
| OUTPUTS | `./outputs` | 报告和导出物 | 正式报告可跟踪，临时输出忽略 |

不创建嵌套 Git 仓库。正式详设、任务规划和测试规划只在 `security_-scheme` 维护；两个代码仓不新增项目级规划副本，`security_-scheme` 也不复制第二份可编辑公司代码或可执行测试。SRC-0035只用于只读RTL调查，结论为`VENDOR_IMPLEMENTATION`并要求实际elaboration，不覆盖SoC/产品权威；目录内容、Key字面量和派生文件不得外发或写回。用户口头所称 `baremental` 对应磁盘上的 `baremetal` 目录。
