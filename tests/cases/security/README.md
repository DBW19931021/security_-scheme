# NGU800P 安全测试最终交付入口

## 当前权威基线

- 在线最终表：[飞书安全测试 Case 表（Security 工作表）](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)
- 本地用例目录：[CASE_TABLE.md](CASE_TABLE.md)
- 架构、功能与Case说明：[NGU800P安全架构功能与测试用例说明.md](../../../docs/06-verification/NGU800P安全架构功能与测试用例说明.md)
- Codex开发指导：[NGU800P安全测试用例设计与Codex移植指导.md](../../../docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md)
- 全过程记录：[NGU800P-security-test-case-workflow-record.md](../../../docs/06-verification/NGU800P-security-test-case-workflow-record.md)

2026-08-20已在登录状态下逐行核对飞书最终表。企业策略禁止下载/导出，因此没有生成新的本地XLSX镜像；在线表是当前业务字段和范围的唯一最终基线。工程内[本地最终表v5](../outputs/20260813-security-final/NGU800P_Security_Case_Table_Final_v5.xlsx)保留为历史快照，不能再作为后续代码用例数量和编号依据。

## 当前范围

| 范围 | 编号 | 数量 | 后续执行 |
|---|---|---:|---|
| Mailbox BASIC | `001～047` | 47 | 软件/BSP移植，连接真实eHSM |
| eHSM负向 | `048～054` | 7 | 软件/BSP移植，必须证明请求真实到达eHSM |
| SoC软件/协同 | `055～061` | 7 | 软件/BSP实现；Debug、Master或IRQ场景按需协同 |
| EDA/硬件协同 | `062～063` | 2 | RTL-DV/EDA注入；软件侧只观察、清除和记录 |
| **合计** | `001～063` | **63** | `001～061`为软件主导范围 |

全部Case仍为`PROPOSED_DESIGN_COMPLETE / NOT_EXECUTED`，文档核对不代表EMU、FPGA、EDA或硅上已经PASS。

## 表格和代码写法

飞书表固定13列：类别名称、模块名称、测试用例描述、用例编号、用例类型、优先级、前置条件、输入、输出、测试目的、通过准则、归属团队、负责人。后续代码应遵守：

1. “输入”只实现当前Case所需API、参数、调用顺序、数据和恢复，不追加无关公共步骤。
2. “输出”和“通过准则”绑定当前命令或功能的直接结果；公共日志字段不能代替具体判定。
3. 原追踪ID与最终编号同时进入注册名、日志和Evidence。
4. 同质实例在一条Case内部遍历，例如两条SRAM Case都各自覆盖Region0～4。
5. Host本地拒绝不算eHSM拒绝；真实硬件故障不由软件模拟冒充。
6. 不修改eHSM BL/FW，不增加测试命令、测试钩子或特殊返回。

## 已确认的实现边界

- SEC_CFG/SPIFC默认仅启动核可访问，eHSM和其他核均拒绝；两条Case不修改Firewall配置。
- SRAM默认允许启动核和eHSM访问，其他核拒绝；默认权限和重新配置保持两个Case，每条都遍历5个Region。
- 三个Firewall的产品配置Owner均为启动核；但飞书Case 060只直接覆盖启动核成功重配SRAM五Region，非启动核/eHSM配置写拒绝未作为独立在线Case冻结，开发时不得擅自宣称已覆盖。
- `dbg_en_cfg`验证主Die控制从DieDebug；Mailbox Debug鉴权必须先冻结挑战类型，再在`soc_dbg_en_out`或`hsm_dbg_en/o_hsm_status[16]`中选择正确判据并验证对应真实Debug通路。
- 严重错误与ECC 1-bit的真实激励由RTL-DV/EDA负责；软件可提供寄存器、IRQ、清除和计数观察器。
