---
title: "NGU800P 安全测试用例设计与 Codex 移植指导"
status: active
evidence_state: PROPOSED_DESIGN_COMPLETE_NOT_EXECUTED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0020
  - SRC-0022
  - SRC-0025
  - SRC-0026
  - SRC-0027
  - SRC-0028
  - SRC-0029
  - SRC-0030
owners:
  - GSP
last_reviewed: 2026-08-20
---

# NGU800P 安全测试用例设计与 Codex 移植指导

## 1. 用途和权威基线

本文是后续Codex在`../baremetal`中规划、开发安全测试Case的直接指导。当前最终业务基线是[飞书安全测试Case表的Security工作表](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)，本地[用例目录](../../tests/cases/security/CASE_TABLE.md)提供编号和代码判定索引。

2026-08-20已逐行核对在线表。受企业策略限制，飞书表不能下载或导出，因此工程内`NGU800P_Security_Case_Table_Final_v5.xlsx`只作为历史快照保留，不能再作为当前数量、编号或范围的依据。

本轮只修订用例设计和开发契约，不修改`../baremetal`、eHSM BL/FW或产品代码。全部Case状态仍为`NOT_EXECUTED`。

## 2. 最终数量与包含关系

| 集合 | 编号 | 数量 | 与其他集合的关系 |
|---|---|---:|---|
| BL Mailbox BASIC | `001～015` | 15 | Mailbox BASIC的BL部分 |
| FW Mailbox BASIC | `016～047` | 32 | Mailbox BASIC的FW部分 |
| Mailbox BASIC | `001～047` | 47 | 15 BL + 32 FW |
| eHSM负向 | `048～054` | 7 | 与BASIC并列，验证eHSM安全拒绝路径 |
| Mailbox软件范围 | `001～054` | 54 | 47 BASIC + 7负向 |
| SoC软件/协同 | `055～061` | 7 | 与Mailbox范围并列 |
| 软件/BSP主导范围 | `001～061` | 61 | 54 Mailbox + 7 SoC |
| EDA/硬件协同 | `062～063` | 2 | 不移植为普通软件故障注入Case |
| 最终表 | `001～063` | 63 | 61软件/协同 + 2 EDA |

旧v5中的5条Mailbox BASIC、7条eHSM负向和1条条件性EDA已不在在线最终表中。后续Codex不得按旧`001～076`编号创建用例，也不得继续实现已经移除的`EHSM-NEG-UPGRADE-*`、`EHSM-NEG-CRYPTO-*`、`EHSM-NEG-KEY-002`、`EHSM-NEG-PROTOCOL-001`或`EDA-FW-EHSM-MASTER-001`独立Case。

## 3. DUT和代码边界

被测对象是eHSM固件、eHSM硬件及NGU800P SoC安全集成。Host测试软件只负责构造输入、提交Mailbox、发起总线访问、读取寄存器、处理中断和采集证据。

后续实现必须遵守：

1. 先检查`../baremetal`现状及其约束，再规划目录、Runner和文件名；本文不预设工程结构。
2. 不修改或定制eHSM BL/FW，不增加测试命令、测试钩子、故障开关或特殊返回。
3. Host参数检查只属于测试适配层质量，不能作为eHSM负向Case的PASS。
4. 不伪造eHSM Master ID、真实Debug联通、硬件错误源或IRQ。
5. 缺少确定性输入时返回`INCONCLUSIVE`，并在日志中写清缺少的绑定和解除条件。

## 4. 公共执行与判定规则

每个代码Case必须同时记录最终编号和原追踪ID，并输出唯一终态：`PASS`、`FAIL`或`INCONCLUSIVE`。

- `PASS`：前置条件满足，真实目标执行完成，全部直接判据通过，恢复动作成功，Evidence完整。
- `FAIL`：目标执行完成，但返回、数据、状态、副作用、权限、IRQ或恢复结果不符合在线Case。
- `INCONCLUSIVE`：缺少批准输入/资源，或请求在到达DUT前被Host拦截，因而无法判断eHSM/SoC行为。

Evidence至少包含：Case双ID、硬件/RTL/eHSM/软件版本、执行载体、关键输入摘要、公开API结果、原始eHSM响应或总线结果、前后状态、恢复结果和最终判定。密钥、挑战响应等敏感材料只记录长度、句柄、用途和不可逆摘要，不打印明文。

## 5. Mailbox BASIC开发规则

最终表包含47条BASIC。每条Case只实现在线“输入”列规定的API和参数矩阵：

1. 前置条件必须确认eHSM正常启动并处于Case要求的BL或FW阶段。
2. 使用工程批准的Mailbox服务通道、等待模式和超时；这些是运行配置，不是独立测试目标。
3. 保存公开API结果和原始eHSM响应；有输出数据的命令按本Case语义比较。
4. 写入、安装、创建、切换、打开Debug或进入WFI等有状态命令，要执行本Case指定的读回、关闭、删除或恢复。
5. 密码算法在同一Case内按批准的算法/模式/方向和测试向量循环；只有失败阶段、资源或恢复方式不同才拆分代码子项，不增加最终Case编号。
6. 不给`get_version`、`get_challenge`等无副作用命令附加无关的密钥清理、算法循环或通用健康查询。

47条具体编号与追踪ID见[CASE_TABLE.md](../../tests/cases/security/CASE_TABLE.md)。

## 6. 七条eHSM负向Case

负向Case的共同构造方法是：先证明批准材料可成功，再一次只改变一个安全属性，确保请求真实提交到eHSM，最后检查拒绝、无禁止副作用和合法复测。

| 编号 | 追踪ID | 激励和直接判据 |
|---:|---|---|
| 048 | `EHSM-NEG-VERIFY-001` | 分别篡改签名、载荷和摘要；eHSM拒绝且不复制、不启动、不放行、不更新相关状态；合法材料仍成功 |
| 049 | `EHSM-NEG-VERIFY-002` | 分别使用错误签名、错误载荷和低回滚计数；SoC验签拒绝，Measurement/release/计数器保持基线 |
| 050 | `EHSM-NEG-AUTH-001` | BL阶段提交错误Debug响应；eHSM拒绝，对应Debug状态和实际通路保持关闭 |
| 051 | `EHSM-NEG-AUTH-002` | FW阶段提交错误Debug响应；eHSM拒绝，对应Debug状态和实际通路保持关闭 |
| 052 | `EHSM-NEG-KEY-001` | 对受完整性保护的安装/导入材料做单点篡改；拒绝且Key/OTP对象无部分更新；正确材料复测成功 |
| 053 | `EHSM-NEG-OTP-001` | 越界、跨对象和受保护OTP访问真实到达eHSM后被拒绝；不泄露真值、不产生部分写入 |
| 054 | `EHSM-NEG-STATE-001` | 生命周期/control field回退、越级、重复或保留值转换被拒绝；OTP状态不变且设备可继续查询 |

如果正式Host API会在本地拦截Case 053所需的非法请求，可使用工程内已有且获批的raw请求入口；如果没有该入口，不得新增eHSM测试命令，结果应为`INCONCLUSIVE`。

## 7. Debug两类寄存器和两条鉴权链

### 7.1 `dbg_en_cfg`

`SOC-SECCFG-DBG-001`只验证主Die控制从DieDebug：复位默认0、打开前真实从DieDebug拒绝、主Die写开并回读、真实通路联通、写回0并回读、真实通路恢复拒绝。仅做寄存器读写不能判PASS，必须与Debug模块协同验证实际通路。

### 7.2 `soc_dbg_en_out`与`hsm_dbg_en`

Mailbox Debug鉴权在实现前必须绑定挑战类型，不能只因为在线表写了`soc_dbg_en_out`就默认所有鉴权都属于SoC Debug：

| 挑战/目标 | 状态观察 | 实际通路判定 |
|---|---|---|
| `SOC_DEBUG` | `soc_dbg_en_out[0]`及SoC集成后的最终Debug使能 | SoC JTAG/Debug在鉴权前拒绝、成功后允许、`CLOSE_DEBUG`后再次拒绝 |
| `EHSM_DEBUG` | `hsm_dbg_en`，其SoC可见映射为`o_hsm_status[16]` | eHSM子系统内部JTAG/DAP分支在鉴权前拒绝、成功后允许、关闭后再次拒绝 |

`hsm_dbg_en`由eHSM产生，并在eHSM wrapper/subsystem内参与JTAG路径选择；SoC全局Debug链使用SoC Debug输出和SoC侧门控。二者不能互相替代。

在线Case 008、031、050、051及关闭Debug的009、032当前没有在Case文本中明确记录挑战类型。开发前必须由接口Owner冻结：若为`SOC_DEBUG`，使用`soc_dbg_en_out`判据；若为`EHSM_DEBUG`，使用`o_hsm_status[16]`和真实eHSM DAP判据；未冻结时相应通路判定为`INCONCLUSIVE`，不得测试错链后报PASS。

## 8. 七条SoC软件/协同Case

| 编号 | 追踪ID | 实现要求 |
|---:|---|---|
| 055 | `SOC-SECCFG-001` | 无寄存器选择参数；单Case遍历offset `0x000～0x040`的17个寄存器，保存raw/拼接值并与当前场景已冻结期望比较 |
| 056 | `SOC-SECCFG-DBG-001` | 按第7.1节验证`dbg_en_cfg`和真实从DieDebug关闭→打开→关闭 |
| 057 | `SOC-FW-SECCFG-001` | 不改Firewall配置；启动核访问成功，eHSM和其他核访问拒绝，未授权写不改变目标或哨兵 |
| 058 | `SOC-FW-SPIFC-001` | 不改Firewall配置；启动核访问成功，eHSM和其他核访问拒绝，未授权写不改变目标或哨兵 |
| 059 | `SOC-FW-SRAM-001` | 单Case遍历Region0～4；启动核/eHSM读写成功，其他核读写拒绝；目标和相邻哨兵符合预期 |
| 060 | `SOC-FW-SRAM-RECFG-001` | 单Case由启动核依次修改Region0～4的地址范围和允许Master ID；验证readback、新Master只访问新范围、未授权/越界拒绝、其他Region不变并逐项恢复 |
| 061 | `SOC-MAILBOX-IRQ-001` | 对适用Mailbox通道用真实无副作用命令触发响应；验证IRQ来源/计数、清除无风暴、再次触发，以及屏蔽时无ISR但轮询仍完成 |

### 8.1 Firewall权限矩阵

| 目标 | 默认数据访问：启动核 | 默认数据访问：eHSM | 默认数据访问：其他核 | 配置Owner |
|---|---|---|---|---|
| SEC_CFG | 允许 | 拒绝 | 拒绝 | 仅启动核 |
| SPIFC | 允许 | 拒绝 | 拒绝 | 仅启动核 |
| SRAM Region0～4 | 允许 | 允许 | 拒绝 | 仅启动核 |

数据访问权和配置权必须分别判断。eHSM默认能访问SRAM数据，不代表能修改SRAM Firewall配置。

### 8.2 在线Case 060的覆盖边界

产品方案已冻结“三个Firewall只有启动核可配置”，但在线Case 060的具体步骤只直接证明启动核可成功重配SRAM五Region，并没有让eHSM/其他核尝试写三个Firewall配置窗口。后续Codex应严格实现在线Case，不把该产品约束偷偷扩写到Case 060，也不能从Case 060 PASS推导负向配置Owner矩阵已验证。

在宣称Firewall配置Owner完整覆盖前，应由Case Owner选择以下方式之一并回填最终基线：把非启动核/eHSM配置写拒绝加入批准Case，或明确由RTL-DV/EDA/集成Evidence补充。当前状态记为“产品约束已知、在线Case覆盖未闭环”。

## 9. 两条EDA/硬件协同Case

### 9.1 `062 / EDA-ERR-CRITICAL-IRQ-001`

RTL-DV/EDA逐源及组合注入watchdog超时、OTP Key CRC、TRNG warning/fail/health-test fail和各RAM不可纠正ECC，验证`o_hsm_err_hw`位映射、严重错误组合中断、清除、重触发和无无关位。软件只能读取状态、处理中断并执行批准的清除操作，不能伪造错误源替代EDA。

### 9.2 `063 / EDA-ERR-ECC1B-IRQ-001`

RTL-DV/EDA对PKE0～3、KMU、DRAM、IRAM和IROM逐源注入可纠正ECC 1-bit错误，验证eHSM脉冲到SoC锁存/计数、独立中断、清除、重触发及计数边界。软件可提供观察器，但不能仅靠写状态寄存器宣称覆盖真实ECC通路。

飞书两条Case的“输入”目前只写“EDA覆盖？”，后续执行计划必须把注入点、预期位、IRQ、清除寄存器、计数器和Evidence路径补成RTL-DV/EDA任务；此处的职责拆分不改变在线Case数量。

## 10. 建议的代码组织方式

具体目录由后续Codex检查`../baremetal`后决定，但逻辑上应分成：

- Case registry：最终编号、原追踪ID、阶段、执行函数和资源需求；
- Mailbox adapter：调用既有typed API并保存raw response；必要时使用既有批准raw入口；
- SoC observer：SEC_CFG快照、Firewall访问结果、IRQ计数/清除和Debug协同回调；
- 数据清单：批准测试向量、预置Key/OTP状态、期望寄存器值、Master ID和恢复数据；
- Evidence writer：统一输出版本、输入摘要、前后状态和PASS/FAIL/INCONCLUSIVE。

一个最终Case可以包含多个参数化子项，但每个子项必须独立记录输入、预期、实际和恢复；任一必测子项FAIL则整个Case FAIL，任一必测子项因绑定缺失无法执行且无FAIL时整个Case INCONCLUSIVE。

## 11. 编码前必须冻结的输入

| 输入 | 影响Case | 缺失时处理 |
|---|---|---|
| 47条Mailbox命令的正式API、命令ID、BL/FW可用阶段和错误码 | 001～054 | 不猜命令或错误码；相关子项`INCONCLUSIVE` |
| Debug挑战类型、`soc_dbg_en_out`/`hsm_dbg_en`绑定及实际JTAG/DAP操作方法 | 008、009、031、032、050、051、056 | 不以错误状态位替代真实通路 |
| SEC_CFG base、17个offset、场景期望和跨32-bit读取规则 | 055～056 | 只可建骨架，不得写硬编码量产地址 |
| 三个Firewall CSR、启动核/eHSM/其他核Master ID、非法访问响应和恢复值 | 057～060 | 未冻结子项`INCONCLUSIVE` |
| Mailbox通道到IRQ映射、mask/clear方法和ISR约束 | 061 | 不把poll成功当IRQ成功 |
| 两类错误源的注入点、位映射、IRQ、清除和计数合同 | 062～063 | 交RTL-DV/EDA，不生成模拟PASS |

## 12. 交付完成条件

后续代码任务只有在以下条件同时满足时才能声明完成：

1. 注册编号严格为`001～061`，并与[用例目录](../../tests/cases/security/CASE_TABLE.md)逐条对应；
2. `062～063`形成明确的EDA任务和软件观察接口，而非普通软件模拟Case；
3. 每个Case有可审计的输入、实际输出、直接判据、恢复和Evidence路径；
4. Debug挑战类型和Firewall配置Owner覆盖缺口已显式处理；
5. 未修改eHSM固件，未用stub、test hook或Host本地拒绝形成虚假PASS；
6. 目标EMU/FPGA/EDA或硅上执行前，状态保持`NOT_EXECUTED`。
