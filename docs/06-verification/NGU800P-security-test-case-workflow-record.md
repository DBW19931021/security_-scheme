---
title: "NGU800P 安全测试用例设计工作流记录"
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
owners:
  - GSP
last_reviewed: 2026-08-13
supersedes: []
superseded_by: []
---

# NGU800P 安全测试用例设计工作流记录

当前正式基线：[飞书安全测试Case表的`Security`工作表](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)。2026-08-20已在登录状态下逐行核对为63条。企业策略禁止下载/导出，因此未生成新的本地XLSX镜像；[`NGU800P_Security_Case_Table_Final_v5.xlsx`](../../tests/cases/outputs/20260813-security-final/NGU800P_Security_Case_Table_Final_v5.xlsx)及更早工作簿保留为历史过程版本，不覆盖、不删除。

## 1. 工作流目标与边界

- 目标一：维护安全方案、详细设计、需求、接口、风险和验证依据的可追溯关系。
- 目标二：整理测试用例，形成后续 Codex 可直接用于 bare-metal 移植规划与实现的指导文档和表格。
- 当前终点：在线用例设计完成并核对为63条，已形成SoC架构/功能/Case说明和`001～061`共61条软件/BSP主导开发指导；`062～063`为EDA/硬件协同。既有`../baremetal`实现只作为历史事实，本轮未修改或继续移植。
- 被测对象：eHSM 固件、eHSM 硬件和 NGU800P SoC 安全集成；Host 是激励与观测端。
- 当前仍不做：不修改 eHSM BL/FW，不新增测试命令；未在 EMU/FPGA
  执行前不宣称硬件 PASS。

## 2. 输入与事实状态

| 项目 | 当前状态 | 说明 |
|---|---|---|
| BL/FW 目标命令 | VERIFIED_IN_HEADERS | BL 15、FW 37，共 52 条 |
| Host-only 候选 | PROFILE_CANDIDATE | 18 条；当前不生成用例 |
| eHSM DUT 边界 | CORRECTED | eHSM 固件和硬件都是 DUT；真实到达 eHSM 的负向场景必须保留 |
| Host 本地负向 | OUT_OF_SCOPE | 只验证 Host 参数、缓冲区、日志、timeout 等本地软件行为 |
| v0.3 固定三类设计 | SUPERSEDED_DESIGN_PRESERVED | 156 条 Mailbox + 67 条 SoC |
| v0.3.1 风险裁剪设计 | SUPERSEDED_DESIGN_PRESERVED | 82 条 Mailbox + 67 条 SoC |
| v0.3.2 BASIC-only 设计 | SUPERSEDED_DESIGN_PRESERVED | 误将 eHSM 负向与 Host 本地负向一并删除：55 条软件 + 8 项硬件 |
| v0.3.3 eHSM DUT 修订 | SUPERSEDED_DESIGN_PRESERVED | 52 BASIC + 14 eHSM 负向 + 3 SoC 软件 = 69；8 项被整体放在硬件表 |
| v0.3.4 软件可构造功能复审版 | SUPERSEDED_DESIGN_PRESERVED | 52 BASIC + 14 eHSM 负向 + 8 SoC 软件 = 74；8 项改为关联软件 case 的硬件补充 |
| v0.3.5 术语基线 | SUPERSEDED_DESIGN_PRESERVED | 用例范围和数量不变；说明性英文术语统一改为中文解释，新增“术语说明”工作表 |
| v0.3.6 当前工作簿 | BASIC_IMPLEMENTATION_RECORDED | 不改变 case ID、命令 ID 和用例范围；回填 52 条 BASIC 的真实覆盖、代码入口、CMD、实现状态和 Evidence |
| v0.3.8 SoC收敛设计 | PROPOSED_DESIGN_COMPLETE | 52 BASIC + 14 eHSM负向 + 7 SoC软件/协同 = 73条可移植；另列3项EDA/硬件 |
| 本地v5 13列表 | SUPERSEDED_HISTORY_PRESERVED | 76条历史快照；保留字段复审和方案演进记录，不再作为当前开发基线 |
| 在线飞书最终表 | PROPOSED_DESIGN_COMPLETE | 63条连续编号；61条软件/BSP主导和2条EDA/硬件协同，全部NOT_EXECUTED |
| baremetal Mailbox BASIC | BUILD_STATIC_COMPLETE | 52 条 BASIC 全部注册；51 条真实 typed API；`MB-BL-006-BASIC` 为唯一 `GATED_INCONCLUSIVE` |
| 安全测试执行 | TARGET_NOT_EXECUTED | 已完成构建/链接，不存在 EMU/FPGA case PASS/FAIL Evidence |

## 3. 工作流执行记录

| 日期 | 阶段 | 活动 | 输出/结论 | 状态 |
|---|---|---|---|---|
| 2026-07-29 | W1-01～03 | 盘点工作区、历史表和 Vendor 头文件 | BL 15、FW 37、Host-only 18 | COMPLETED |
| 2026-07-29 | W1-04～06 | 每命令固定派生 BASIC/BOUNDARY/POLICY，并细分 SoC | v0.3 共 223 条 | SUPERSEDED |
| 2026-07-29 | W1-07 | 可执行移植与执行 | 留给后续 Codex | OUT_OF_SCOPE |
| 2026-08-03 | W1-08 | 按风险和共享路径重构 | v0.3.1：82 条 Mailbox + 67 条 SoC | SUPERSEDED |
| 2026-08-03 | W1-09 | 按硬件目标收敛，删除 Host 软件负向和过细 SoC | v0.3.2：52 BASIC + 3 SoC = 55；另列 8 项硬件需求 | SUPERSEDED |
| 2026-08-03 | W1-10 | 纠正 DUT 边界，区分 Host 本地失败与 eHSM 拒绝 | v0.3.3：52 BASIC + 14 eHSM 负向 + 3 SoC = 69；另列 8 项硬件需求 | COMPLETED |
| 2026-08-03 | W1-11 | 复审硬件需求的软件可构造部分 | v0.3.4：新增 5 条 SoC 软件 case，共 74 条可移植；8 项硬件表只保留内部补充 | COMPLETED |
| 2026-08-04 | W1-12 | 中文化测试表术语 | v0.3.5：说明性文字中文化，技术匹配标识保留，新增术语说明工作表 | COMPLETED |
| 2026-08-03 | W2-01 | baremetal 阶段 0～3：eHSM Host 公共源码、GSP port、一次性 runtime 和 52 条 BASIC registry | service channel 1；26 REAL_API + 26 GATED_INCONCLUSIVE；`gsp-bm` 构建通过 | BUILD_STATIC_COMPLETE |
| 2026-08-10 | W2-02～04 | 取消已批准静态门禁并补齐参考 Host 已启用算法、签名和密钥管理矩阵 | 51 REAL_API + 1 GATED_INCONCLUSIVE；双 OTP Profile 构建/静态 Evidence 完成 | BUILD_STATIC_COMPLETE_TARGET_GAP |
| 2026-08-10 | W2-05 | 回填实现状态工作簿 | 形成 v0.3.6；代码接入、构建状态和目标执行状态分列；负向/SoC 用例仍为 PLANNED | COMPLETED |
| 2026-08-13 | W2-06 | 按安全方案收敛SoC范围 | 形成v0.3.8：7条SoC软件/协同和3条EDA/硬件需求 | COMPLETED |
| 2026-08-13 | W2-07 | 参考mailbox/spinlock统一最终表格式和说明粒度 | 形成单页13列最终表、连续编号001～076、架构/功能/Case说明和工程索引 | COMPLETED |
| 2026-08-13 | W2-08 | 复审Mailbox用例“输入”针对性 | 逐条重写52条BASIC和14条eHSM负向的实际API、参数、顺序、矩阵及恢复动作；删除无关通用模板 | COMPLETED |
| 2026-08-13 | W2-09 | 复审全部用例“通过准则”针对性 | 逐条收敛52条BASIC、14条eHSM负向、7条SoC和3条EDA准则；公共判定规则集中写入说明文档 | COMPLETED |
| 2026-08-14 | W2-10 | 复审全部用例“前置条件/输出/测试目的”针对性 | 明确BL/FW阶段和Mailbox通道配置；逐条重写76条输出和目的；删除“合同有效”“允许副作用”和家族输出模板 | COMPLETED |
| 2026-08-20 | W2-11 | 核对飞书在线最终Case并回写开发说明 | 逐行确认001～063；更新为47 BASIC + 7 eHSM负向 + 7 SoC + 2 EDA；识别Debug挑战类型和Firewall配置Owner负向覆盖边界 | COMPLETED_NOT_EXECUTED |

### 3.1 当前 baremetal 落地状态

- 代码根：`../baremetal/components/security`，通过现有 `security_test`
  CMD 注册，不增加独立 solution、Runner 或脚本框架；
- eHSM Host `2.3.1-4019-2ee044d` 的公共 `api.c`、`mailbox.c`、公开头
  和命令结构已复制进 baremetal 独立编译，原 Vendor/eHSM 仓库不参与构建；
- GSP 使用 Vendor 普通业务 service channel 1、poll mode、100 ms；
- 52 条 BASIC 均已注册，51 条接入真实 typed API 和本地 Expected；当前只有
  `MB-BL-006-BASIC` 因升级镜像、提交与恢复合同未补齐而保持
  `GATED_INCONCLUSIVE`；
- 算法类 BASIC 按批准参考 Host 已启用路径补齐矩阵，保持一条稳定 case ID，
  在 case 内参数化执行全部算法、模式、方向和 typed API 路径；
- build/static Evidence：
  `../baremetal/components/security/docs/evidence/20260803-phase0-3-basic-port.md`。

## 4. W1-10 边界纠正

用户明确指出：“不测试软件 bug”仅指不测试 Host 端本地软件 bug，不代表不测试 eHSM 软件。eHSM 的软件和硬件都是测试对象。因此采用以下裁决：

- 删除 Host 本地 `NULL`、buffer、local enum、timeout、日志等健壮性测试；
- 保留错误签名、错误摘要、错误 MAC/tag、篡改升级包、非法 Key/OTP/LCS/协议等 eHSM 负向测试；
- eHSM 负向必须有 Mailbox 提交证据、command ID 和 raw response；只被 Host 本地拦截不能算 eHSM PASS；
- 不恢复每条命令机械派生的 BOUNDARY/POLICY，而是按安全服务、失败 oracle、副作用、fixture 和恢复方式归并为 14 个稳定 case family；
- 仍禁止修改 eHSM BL/FW、增加测试命令、钩子或特殊返回。

### 4.1 W1-11 软件可构造功能复审

再次逐项检查 8 项硬件需求后，确认“软件不能证明全部内部事实”不等于“不能建立软件功能 case”。采用以下处置：

- Mailbox 16 路实例与 INFO/NOTE 功能握手各新增一条软件 case；
- Firewall 配置权限和 SRAM 隔离继续使用原两条软件 case；
- HSM 正常读取保留，并新增一条使用现有 eHSM 负向命令构造错误映射的软件 case；
- reset 和 IRQ 各新增一条独立测试 Profile case，产品首版仍可保持 poll，产品 security 路径仍不得擅自 reset；
- 8 项硬件表不删除，但改为明确关联软件 case，只补充内部译码、逐周期/CDC、逐 bit、精确复位相位和多源并发；
- 若软件激励和观测路径存在、只是映射尚未冻结，则用例为 `INCONCLUSIVE`，不删除。

### 4.2 W1-12 术语中文化

- 将 oracle、fixture、vector、stage、baseline、raw response/status 等说明性术语统一改为中文解释；
- 新增“术语说明”工作表，集中解释 BASIC、EHSM_NEGATIVE、Host、Mailbox、MMIO、IRQ、CDC、RTL/DV、master_id、Region、reset matrix 等保留标识；
- 命令名、寄存器名、用例 ID、错误码、状态码和准入门禁码不翻译，避免破坏代码、日志和头文件匹配；
- 用例范围、数量、判据和移植边界不变。

### 4.3 W2-06 SoC安全方案用例收敛

2026-08-13依据SRC-0025、SRC-0026、SRC-0027及项目负责人多轮裁决重新收敛SoC范围：

- SEC_CFG只保留“全部17寄存器查询比较”和“`dbg_en_cfg`从Die Debug开关”两条；
- `soc_dbg_en_out`加入Mailbox Debug Auth成功/失败及`CLOSE_DEBUG`判据；
- SEC_CFG/SPIFC Firewall只测默认权限，不修改配置；
- SRAM默认权限和重新配置保留两条，每条内部都遍历Region0～Region4；
- Mailbox只保留一条IRQ软件Case，遍历适用通道；
- 普通软件无法构造的严重错误组合中断和ECC 1-bit独立中断改为EDA必测；
- eHSM Master权限在没有现有公开访问路径时由条件性EDA补齐；
- 原SoC channel、INFO/NOTE、HSM错误软件注入、复位等Case不再进入本轮安全方案顶层清单。

形成v0.3.8：Mailbox软件66 + SoC软件/协同7 = Codex可移植73；EDA/硬件需求3项不移植。所有条目仍为`NOT_EXECUTED`。

### 4.4 W2-07 最终格式与说明粒度统一

- 参考`mailbox/docs/cases`和`spinlock/docs/cases`的README、逻辑/功能说明、CASE_TABLE及最终CSV/XLSX，将安全用例收敛为同一套“先讲架构和功能，再冻结Case，最后输出13列表”的过程；
- 登记SRC-0028，仅把参考工程作为表格格式和说明粒度来源，不把其功能Expected带入安全用例；
- 最终表固定为类别、模块、描述、编号、类型、优先级、前置条件、输入、输出、测试目的、通过准则、归属团队、负责人13列；
- “输入”细化为可执行步骤、遍历范围、测试数据、超时和恢复；“输出”只写实际采集项；“通过准则”明确PASS/FAIL/INCONCLUSIVE；
- 形成连续新编号`NGU800P-D0-SECURITY-001～076`，同时在描述中保留原追踪ID；
- 通过分段渲染检查全部76行，重新导入核对`Security!A1:M77`，公式错误扫描为0；目标环境未执行，因此无硬件PASS声明。

### 4.5 W2-08 Mailbox“输入”逐命令复审

用户复审指出`NGU800P-D0-SECURITY-001`等Mailbox Case的“输入”套用了通用五步模板，导致`get_version`也出现算法矩阵、清理和健康查询等无关动作。本轮按以下规则纠正：

- 52条BASIC逐命令绑定正式API和真实参数；例如`001`只保留清零版本结构、调用`ehsm_get_version`、保存`0xff10`原始响应和检查`version.type=BL`四步；
- 算法Case只列其实际算法/模式/方向/单次或分段/句柄或明文Key矩阵，并列出对应`init/update/finish`或one-pass API；
- OTP、寄存器、Debug、升级、Lifecycle和Key Manager只写本命令真正需要的读回、恢复、删除或关闭动作；无副作用命令不强行增加清理；
- 14条eHSM负向逐条明确合法基线、单点篡改、对应正式API或隔离raw组包路径、副作用检查和合法复测；
- 自动审计确认`001～066`编号连续，66条“输入”均为独立内容，旧通用模板关键句命中0项；重新导入和分段目视检查通过。

### 4.6 W2-09 “通过准则”逐Case复审

用户复审指出最终表的“通过准则”重复包含Evidence、Host本地拒绝、恢复和无法判定等公共话术，单元格过长且不能突出当前Case真正判断什么。本轮按以下规则纠正：

- 52条Mailbox BASIC分别绑定实际命令结果，例如`001`只判断`ehsm_get_version`返回`EHSM_OK`且`version.type=BL`；算法Case只判断本Case矩阵、向量/闭环结果和必要的临时Key删除；
- 14条eHSM负向分别写明应拒绝的具体篡改、禁止副作用和合法基线复测，不再复述整套Evidence要求；
- 7条SoC和3条EDA分别绑定寄存器比较、Debug实通路、Firewall权限、IRQ或错误注入的直接结果，范围和执行层不变；
- “全部针对性条件满足为PASS、任一不满足为FAIL、绑定不足为INCONCLUSIVE、Host本地拒绝不能证明eHSM PASS”等公共规则统一移入功能与Case说明文档；
- 自动审计确认76条准则全部非空，旧通用模板关键句命中0项，最长84个字符；重新导入、公式扫描和五段视觉检查通过。

### 4.7 W2-10 “前置条件/输出/测试目的”逐Case复审

用户复审指出“service channel 1/poll/100 ms合同有效”“允许副作用”和家族级输出模板难以直接指导实现，且OTP读取等Case继承了不适用的写前/写后字段。本轮按以下规则纠正：

- 52条BASIC和14条eHSM负向均在前置条件中明确eHSM正常启动及实际BL/FW阶段；通道配置改写为“Mailbox服务通道1、轮询等待、单次命令超时配置100 ms”；
- SoC/EDA只在功能真实依赖eHSM阶段时增加对应条件；纯`dbg_en_cfg`联调用例不强加无关eHSM阶段；
- 52条BASIC、14条负向、7条SoC和3条EDA均使用独立输出目录，逐条写明API返回、原始响应、关键数据、状态前后值、IRQ/波形或恢复结果；
- `002`只输出两次OTP读取返回、两个20-byte UID、预填值覆盖和两次/批准UID比较，不再出现写前写后字段；
- “测试目的”逐条改成要证明的命令功能或安全属性；旧“允许副作用”被展开为具体允许变化或必须不变的对象；
- 自动审计确认76条三类字段均非空，66条Mailbox阶段全部匹配，旧通用模板及“合同”命中0项；重新导入、公式扫描和五段视觉检查通过。

## 5. v0.3.8 历史用例模型

### 5.1 软件可移植用例

- Mailbox BASIC 52：每个 BL/FW 目标命令保持一条稳定 case ID；算法类 case
  在同一 ID 内覆盖已批准且参考 Host 已启用的完整参数化路径；
- eHSM 负向 14：验签 2、升级 2、Crypto 3、认证 2、Key 2、OTP 1、状态 1、协议 1；
- SoC 软件/协同 7：SEC_CFG全寄存器、`dbg_en_cfg`从Die Debug、SEC_CFG默认权限、SPIFC默认权限、SRAM默认权限、SRAM重新配置、Mailbox IRQ。

合计73条，逐条进入工作簿`Codex移植指导`。SRAM两个Case各自在Case内部遍历5个Region，不按Region拆分。

### 5.2 EDA/硬件验证

保留3项：`EDA-ERR-CRITICAL-IRQ-001`、`EDA-ERR-ECC1B-IRQ-001`和条件性`EDA-FW-EHSM-MASTER-001`。它们不进入baremetal移植表。`SOC-SECCFG-DBG-001`所需的Debug模块/EDA协同仍属于该软件/协同Case，不重复计数。

## 6. eHSM 负向构造和判定规则

1. 以已经通过的合法向量为基线，只修改一个安全属性。
2. 优先使用 Vendor 正式 typed API；只有协议负向需要隔离的 raw request builder。
3. 必须记录 Mailbox 已提交、command ID、raw response、公开 API 结果以及对象/寄存器前后状态。
4. PASS 要同时满足“按合同拒绝”“无禁止副作用”“失败后合法请求仍可工作”。
5. Host 本地拒绝视为 `NOT_APPLICABLE/INCONCLUSIVE`，不是 eHSM PASS。
6. Vendor 失败码、AEAD 失败输出、Key/OTP/LCS 副作用合同未冻结时，不猜测，记录待确认项。

## 7. 形成和核验过程

1. 重新核对 52 条 BL/FW 目标命令和 18 条 Host-only 候选。
2. 保留 52 条 BASIC，并从安全验证目标提取 14 条最小 eHSM 负向族。
3. v0.3.4～v0.3.7保留历史模型；v0.3.8按安全方案功能重新形成7条SoC软件/协同Case和3项EDA需求。
4. 不覆盖任何历史工作簿；保留Mailbox稳定case ID，SoC使用新的功能级case ID。
5. 重新导入工作簿，检查全部工作表、关键范围和公式错误，并逐表渲染检查。
6. 同步指导文档、工作流状态、测试矩阵、项目状态、来源登记和变更记录。

## 8. 后续 Codex 开发约束

1. 先检查 `../baremetal` 的规则、baseline、已有修改和测试设施，再规划代码落点。
2. 只按在线最终编号实现`001～061`共61条软件/BSP主导Case；`062～063`只建立EDA协同任务和必要的软件观察器，不生成软件模拟故障PASS。
3. 按52 BASIC → 14 eHSM负向 → 7 SoC软件/协同的顺序实施。
4. 不修改 eHSM BL/FW；不增加测试命令、错误注入钩子或特殊返回。
5. 地址、master ID、Region、权限、raw 失败码和副作用未冻结时输出 `INCONCLUSIVE`。
6. Evidence 记录 case/vector ID、branch/commit、RTL/eHSM/软件 baseline、提交证据、raw response、状态快照、日志和哈希。

## 9. 待补输入

| 输入项 | 用途 | 当前处置 |
|---|---|---|
| `../baremetal` baseline 与工作区状态 | 移植基线 | 已登记 master / `149edc4df84b85b33529c7fa96f9675cf5306513`；保留既有未提交修改 |
| eHSM verify/upgrade raw code 和副作用 | 冻结负向 oracle | Vendor/交付代码确认 |
| AEAD 认证失败后的输出缓冲区行为 | 冻结 CRYPTO-003 oracle | Vendor 确认 |
| Key policy 与 OTP/LCS 合同 | 冻结 KEY/OTP/STATE oracle | Vendor/项目方案确认 |
| 隔离的 raw request builder | 协议负向真实送达 | baremetal 规划时确认；无能力则 `INCONCLUSIVE` |
| SEC_CFG/Debug/Firewall/Region/Mailbox IRQ映射 | SoC软件与协同测试 | RTL/SRC-0022和Debug模块冻结 |
| 严重错误与ECC 1-bit IRQ/计数合同 | 两条EDA必测 | RTL/DV/SoC集成冻结并关联需求ID |
| eHSM Master公开访问路径 | 判断软件或EDA覆盖 | 无路径时执行EDA-FW-EHSM-MASTER-001 |

## 10. 完成判定

设计任务完成：在线最终表共63条，61条软件/BSP主导Case已有开发契约，2条EDA/硬件Case明确由RTL-DV/EDA构造真实故障并由软件观察。历史版本保留。当前未执行任何目标Case，不得宣称整体移植完成或硬件PASS。

## 11. 2026-08-14 SEC_CFG/SPIFC默认权限修正

- 登记`SRC-0029`，仅替代`SRC-0027`中SEC_CFG/SPIFC默认允许eHSM访问的旧表述。
- `SOC-FW-SECCFG-001`和`SOC-FW-SPIFC-001`统一改为：启动核访问成功，eHSM及其他核访问被拒绝，未授权写无副作用，Firewall配置不变。
- `EDA-FW-EHSM-MASTER-001`改为目标化判定：真实eHSM Master对SEC_CFG/SPIFC应被拒绝，对SRAM Region0～4应成功。
- `SOC-FW-SRAM-001`和`SOC-FW-SRAM-RECFG-001`不变；SRAM默认仍允许启动核和eHSM访问，且两个Case分别在内部遍历5个Region。
- 形成最终表v4并保留v3历史；未修改baremetal或eHSM固件，未执行目标测试。

## 12. 2026-08-14 Firewall配置Owner修正

- 登记`SRC-0030`：SEC_CFG、SPIFC和SRAM三个Firewall均只有启动核有配置权限，eHSM和其他核均无配置权限。
- 数据访问与配置权限分开判定：eHSM默认可以访问SRAM数据，但不能配置SRAM Firewall。
- 不新增顶层Case；将配置Owner判定并入`SOC-FW-SRAM-RECFG-001`。非启动核配置写应被拒绝且配置不变；随后仅由启动核遍历SRAM Region0～4完成地址范围和允许Master ID重新配置及恢复。
- `EDA-FW-EHSM-MASTER-001`补充真实eHSM Master对三个Firewall配置写均被拒绝的判据。
- SEC_CFG/SPIFC不执行成功重新配置。形成最终表v5并保留v4历史；未修改baremetal或eHSM固件，未执行目标测试。

## 13. 2026-08-14 项目组总体介绍文档（历史记录）

- 以本地受控最终表v5核对`Security!A1:M77`，确认76条Case、连续编号`001～076`，分类为Mailbox BASIC 52、eHSM负向14、SoC软件/协同7和EDA/硬件3；全部为功能正确性、P0。
- 新增《NGU800P eHSM、Firewall与SEC_CFG总体介绍（项目组简版）》，面向项目组说明三个模块在SoC中的位置、默认安全状态、寄存器分组、集成关系和Case总体构成。
- 文档明确区分Firewall数据访问权限与配置权限：SEC_CFG/SPIFC默认仅启动核访问，SRAM默认允许启动核和eHSM；三个Firewall均仅启动核可配置。
- SEC_CFG列出17个寄存器的功能分组及复位值边界；Firewall只列方案使用的寄存器组，不把`OPEN-CONFLICT-014/015`中的候选base/reset写成量产事实。
- 当时在线飞书连接不可用，因此该轮统计基线为本地v5；该限制已由W2-11的在线复核解除。本轮历史记录仍保留，未回写为当时已读取。

## 14. 2026-08-20 飞书最终表在线复核与开发基线收敛

本轮在内置浏览器已登录状态下读取飞书`Security`工作表，并只修改工程说明文档，没有修改在线表、`../baremetal`、eHSM固件或产品代码。

复核结论：

- 最终编号连续为`NGU800P-D0-SECURITY-001～063`，不存在064；
- `001～047`为47条Mailbox BASIC，其中BL 15条、FW 32条；
- `048～054`为7条eHSM负向；
- `055～061`为7条SoC软件/协同；
- `062～063`为2条EDA/硬件协同；
- 软件/BSP主导范围为`001～061`共61条，EDA故障注入范围为2条；
- 原本地v5的`001～076`和73条移植口径正式降为历史快照；原`EDA-FW-EHSM-MASTER-001`不再计入最终表。

开发指导同时记录两个不能静默推断的问题：

1. 在线Debug鉴权Case未写明挑战类型。实现必须先区分`SOC_DEBUG`与`EHSM_DEBUG`，分别使用`soc_dbg_en_out + SoC JTAG`或`hsm_dbg_en/o_hsm_status[16] + eHSM DAP`判定。
2. 产品方案规定三个Firewall仅启动核可配置，但在线Case 060只直接覆盖启动核成功重配SRAM五Region，未覆盖非启动核/eHSM配置写拒绝。该负向矩阵需由最终Case或RTL-DV/集成Evidence另行闭环。

飞书因企业策略禁止下载/导出，故未创建声称与在线表完全同步的新XLSX。当前采用“在线表为业务权威、本地Markdown为开发索引、v5为历史快照”的管理方式。全部Case继续保持`NOT_EXECUTED`。
