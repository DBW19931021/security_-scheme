---
title: "NGU800P安全工程两项任务总计划"
status: active
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0020
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-08-25
supersedes: []
superseded_by: []
---

# 计划定位

本工程只统计两个顶层任务。其他DD、INV、DEV、TST和EMU记录都是这两个任务的子工作或治理记录，不再作为并列主线。

| 顶层任务 | 唯一目标 | 设计/计划仓库 | 最终代码仓库 | 当前状态 |
|---|---|---|---|---|
| 任务一：baremetal eHSM测试Case开发 | 移植OSR Host Demo并验证eHSM交付的全部功能和接口 | `security_-scheme` | `../baremetal` | 一级case list已建立；待二级展开和实现 |
| 任务二：SoC安全固件详细设计与开发 | 完成Wing-M130/eHSM BL集成和C908 BootROM/FMC/GSP产品安全软件 | `security_-scheme` | `../gsp-pmp-rmp-omp`；Wing-M130 BL按Vendor交付/集成方式管理 | 正式详细设计冻结中；尚未编码 |

# 任务一：baremetal eHSM测试Case开发

## 目标

把SRC-0018 `ehsm_demo_test()`及其BL/FW/可选入口完整移植为baremetal可执行case，用于在EMU上确认eHSM提供的全部功能、command和接口是否正常。

## 执行步骤

1. 从Vendor Host Demo形成最终case list：每个entry内部的command、算法、模式、同步/异步、正负向和边界场景分别成行。
2. 无特殊差异时复用Host command组包、解析和测试向量；只适配baremetal port、case入口、runner、timeout、日志和结果协议。
3. BL态、FW态、OTP/LCS/Debug/Key/升级等case按前置状态和破坏性分组，不把所有Demo入口无条件串行执行。
4. 在`baremetal`完成编译和非硬件dry-run；EMU到位后执行真实eHSM回归并保存Evidence。

## 完成标准

- Vendor交付的每项功能/接口都有Case ID、前置条件、输入、Expected、raw result、可执行入口和Evidence路径。
- 顶层执行follow baremetal规则；单个Demo总体success打印不作为全部case PASS。
- 该任务只验证eHSM能力，不定义BootROM/FMC/GSP产品安全启动流程。

## 跟踪入口

- 顶层任务：[`TASK-SEC-BAREMETAL-EHSM-001`](../../tasks/active/TASK-SEC-BAREMETAL-EHSM-001.md)。
- Case清单：[`EHSM-DEMO-CASE-CATALOG.md`](../../tests/cases/EHSM-DEMO-CASE-CATALOG.md)。
- 当前源码调查：[`INV-SEC-003`](../../tasks/active/INV-SEC-003-ehsm-demo-case-and-reuse-map.md)。

# 任务二：SoC安全固件详细设计与开发

## 目标

以SRC-0017系统原则和SRC-0016软件方案为目标，交付最终运行在NGU800P SoC上的安全固件：

- Wing-M130/eHSM BL：已批准以Vendor业务为默认基线，不改写内部业务逻辑；冻结版本、配置、启动状态、自检位图、eHSM FW校验、Mailbox兼容和集成差异。只有确认的NGU800P集成缺口才进入变更。
- C908 BootROM：完成eHSM ready、自检、FMC包读取、verify/decrypt、Header Overlay/typed-stage policy、Measurement、加载和跳转。
- C908 FMC：建立Host通道，验证/解密/加载GSP，管理rollback/Measurement并release GSP。
- C908 GSP：验证/解密/加载Runtime，维护Measurement，提供SPDM Responder，并落实后续LCS/Debug/Key/Cert/Update/Multi-Die/Firewall控制。

## 阶段顺序

| 阶段 | 内容 | 出口 | 是否允许编码 |
|---|---|---|---|
| B0 详细设计冻结 | 冻结角色、启动链、ABI、结构体、API、状态机、安全RAM/地址、owner、错误/RAS职责、清零、代码/case映射 | accepted详设、已关闭冲突、冻结清单 | 否 |
| B1 OpenSpec和实施任务 | 将已冻结Feature转换为proposal/spec/design/tasks，明确目标文件和验证命令 | 人工批准的实现任务 | 否，批准后进入B2/B3 |
| B2 Wing-M130/eHSM BL集成 | 核对Vendor BL/FW配套版本和NGU800P配置；仅处理批准的集成差异 | 可集成eHSM BL/FW镜像及Evidence | 是，按单独任务 |
| B3 C908安全固件实现 | 在`gsp-pmp-rmp-omp`实现公共安全组件、BootROM、FMC、GSP和工具 | 可构建固件和实现侧测试 | 是，按Feature滚动 |
| B4 联调和EMU验证 | 与eHSM BL/FW、RTL、Flash、Host和Runtime集成 | 可执行启动链和问题Evidence | 是 |
| B5 流片前释放 | 完成P0用例、风险闭环、版本清单和发布Evidence | 安全方案释放结论 | 仅修复批准问题 |

## B0必须完整冻结的设计域

1. 系统角色和BootROM→eHSM→FMC→GSP→Runtime完整时序。
2. eHSM Host adapter、follow Vendor的Mailbox、共享buffer、ready/self-test/timeout、错误上报与RAS reset职责边界。
3. eHSM native package使用规则、NGU Native Header offset1008/1016/1020 Overlay与Header CRC、typed-stage registry和固件制作工具合同；旧Manifest和旧8B零reserved wire格式标记为superseded。
4. verify/decrypt输入输出、可信结果、loader、地址域、load/entry和release接口。
5. rollback counter宽度、比较、更新、掉电和耗尽规则。
6. Measurement Table跨stage合同：BootROM `check_version=0`暂存FMC candidate并commit FMC Entry→FMC初始化exact-match commit/readback proof→接收同值GSP的门禁已冻结；继续完成物理容量、PMA和SPDM映射。
7. BootROM、FMC、GSP逐阶段状态机、失败终态、日志和清零。
8. SPDM、Root→Intermediate→Device Issuer静态前缀、GSP一级动态Firmware Alias Leaf和eHSM签名provider；产品目标包含secure session，准确eHSM/OID/SPDM Profile到齐前保持`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
9. LCS、Debug/RMA、Key/OTP/Cert/Rotation、制造灌装。
10. 正常更新、OOB恢复、Multi-Die、UCIe和Firewall。
11. 构建/链接/镜像打包、版本、配置和禁止stub/simulated success门禁。
12. Feature→Requirement→Design→Code/API→Case→Evidence追溯。
13. `0x1000_0500_0000`起始2 MiB安全RAM的执行/Host ingress/plaintext/eHSM packet/Measurement分区、地址域、W^X、Firewall、cache和清零。

## 当前工作

当前只执行B0。单一[`NGU800P安全软件详细设计`](../05-software-design/NGU800P安全软件详细设计.md)第3～16章及附录A～C已全部形成，专题页已同步，不再存在“下一章待写”。BootROM先按SRC-0034读取`non_sec_boot`只读快照：可靠值1强制现有受限非安全，值0才执行SRC-0023 Strap合同，即`boot_pin.secure_boot[3]`、default 0、0非安全/1安全、USER强制安全；SoC LCS不依赖eHSM Autoload。policy-fuse绑定由OPEN-DESIGN-024管理，当前Strap生成头冲突由OPEN-CONFLICT-010管理。平台数值、Runtime、provisioning、SPDM、更新、Multi-Die、RAS/发布/测试签署等未冻结内容统一列于附录B和`open-questions.yaml`。裁决/输入到齐前仍不授权产品编码。

## 跟踪入口

- 顶层任务：[`TASK-SEC-SOC-FW-001`](../../tasks/active/TASK-SEC-SOC-FW-001.md)。
- 正式详设主入口：[`NGU800P安全软件详细设计`](../05-software-design/NGU800P安全软件详细设计.md)。
- 第一轮评审：[`SoC安全固件首轮详设评审包`](SOC安全固件首轮详设评审包-启动链与核心合同.md)。

# 两项任务的关系

- 任务一可以在产品详设未全部完成前独立开始编码，因为它验证的是eHSM Host接口；但其Expected仍需受控，不能用stub或Demo总体打印替代真实结果。
- 任务二的C908产品编码必须等待对应详细设计冻结和人工批准。
- 任务一可以为任务二提供“某条eHSM command在目标环境可用”的Evidence，但不能决定产品何时调用、传什么业务payload或失败后如何处理。

# Change history

- 2026-08-21：任务二Package合同改为无Manifest的Native Header Overlay与typed-stage policy；主详设和实施计划已同步，产品代码未授权。
- 2026-07-24：主详设第5章完成合入，冻结2 MiB双地址、P1生命周期、统一layout源、权限/Owner、Host ingress/plaintext/执行区、Firewall/PMP/PMA分工、cache和清零主体；下一章进入BootROM，平台数值继续开放。
- 2026-07-24：接受ADR-0016并确认最终单一完整详设交付；GSP替代OMP/Q&P，专题文件批准后必须将规范性内容合入主文档。
- 2026-07-24：完成INV/CE-SEC-010；确认2 MiB双地址、16路eHSM IRQ编码、当前linker混合视图和PMA/Firewall缺口，形成真实MMIO、PC视图和平台属性输入门禁。
- 2026-07-23：完成CE-SEC-009并启动B0-R3 package/verify/loader切片；形成两份接口候选详设和OPEN-CONFLICT-008/OPEN-DESIGN-008，未修改两个代码仓。
- 2026-07-23：回填Measurement跨stage合同；FMC自身epoch来源和FMC→GSP counter/commit/release门禁已冻结，最终C ABI转入OPEN-DESIGN-007。
- 2026-07-23：接受ADR-0010/0011；冻结Vendor direct/源码只读、三stage首版poll、单在途、active cache/timeout scope、零自动retry、timeout quarantine和RAS未ready终态；新增eHSM Host Adapter详细合同。
- 2026-07-22：进入B0-R2；新增2 MiB安全RAM完整设计域，冻结Mailbox follow Vendor和security只上报/RAS决定reset边界；登记OPEN-CONFLICT-006。
- 2026-07-22：接受ADR-0007；2 MiB安全RAM改为P1生命周期复用布局，旧080x被替代，PMP/RMP/MMP常驻，BootROM/FMC尾部回收，GSP/Measurement/Mailbox连续且SPDM并入GSP。
- 2026-07-22：按负责人确认，将工程明确收敛为两个顶层任务；任务一为baremetal Host Case移植，任务二为Wing-M130/eHSM BL集成及C908 BootROM/FMC/GSP详细设计与开发。
- 2026-07-22：接受并补充ADR-0005；当时Measurement load/entry采用64位+domain并只作loader结果快照；最终ABI后由ADR-0022更新为64位`SOC_PA`且不带domain。counter按Vendor统一为16字节，Wing-M130/eHSM BL保持Vendor业务基线。
