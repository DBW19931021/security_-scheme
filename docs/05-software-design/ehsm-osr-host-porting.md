---
title: "DD-02 eHSM/OSR Host双路径移植详细设计"
status: draft
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
owners:
  - GSP
last_reviewed: 2026-07-22
supersedes: []
superseded_by: []
---

# Purpose

冻结DD-02的工作边界：同一套OSR eHSM Host交付在NGU800P工程中形成两个不同目标、不同顶层控制权和不同验收口径的移植路径。本文只定义详设和后续实施约束，不授权修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor源码。

# 已确认的负责人原则

1. `baremetal`路径以验证eHSM提供的全部功能和接口是否正常为目标，必须覆盖`ehsm_demo_test()`可达和交付中提供但未默认调用的功能/case；最终以受控case list呈现。
2. `baremetal`路径中的具体eHSM command拼装，在无NGU800P差异、安全缺陷、接口版本差异或测试隔离要求时可以复用Vendor Demo实现；顶层注册、选择、执行、结果输出和runner组织遵守`baremetal`工程规则。
3. `gsp-pmp-rmp-omp`路径以当前有效芯片安全软件方案的产品功能为唯一目标。第一阶段主要以`bl_demo`所展示的eHSM能力作为实现输入，但不移植其Demo执行顺序。
4. 产品路径中的安全启动、镜像验证、升级、Debug、Measurement和SPDM等调用关系与任务串联以芯片安全软件方案及批准详设为准。
5. 产品路径可以移植经过核实的底层通用函数；Mailbox command的协议格式可以复用，但上层payload内容、地址、buffer、密钥/slot、生命周期、调用时机和错误策略必须由NGU800P实际启动链决定，不能照搬Demo常量或流程。
6. `baremetal`能力验证由独立baremetal软件栈承载并运行在安全核上，不是GSP固件中的task、service或测试模式；GSP固件只实现产品方案明确需要的eHSM调用。项目组织Owner不得被写成运行时软件Owner。

# Vendor代码事实

- `SRC-0018`中的`demo/test.c:22-60`实现`ehsm_demo_test()`：初始化Host library，写入Demo OTP默认值，reset并等待状态，再根据eHSM版本进入`ehsm_bl_demo_entry()`或`ehsm_fw_demo_entry()`。
- `demo/bl_demo/bl_demo.c:27-58`包含Bootloader态的OTP key、OTP/寄存器读写、Debug Auth、镜像验证/升级、UART baud divisor和self-test入口；其中Debug Auth在顶层连续调用两次，是否表示两个预期场景或重复调用需要逐函数核实。
- `demo/fw_demo/fw_demo.c:74-168`包含Hash/HMAC/对称/AEAD/MAC、SM2/ECDSA/RSA、RNG、OTP/LCS/control field、Debug、Key Manager、镜像验证/升级等入口；生命周期变更实现存在但在顶层被注释。`test_parallel()`只发现被注释的调用引用，当前快照未找到声明或定义，必须登记为待核实引用，不能先描述为已提供接口。
- 上述内容是`VENDOR_IMPLEMENTATION`，证明Vendor快照中存在相应实现，不证明NGU800P硬件已经支持、测试已经通过，也不自动形成SoC发布Expected。

# 双路径总体设计

| 维度 | DD-02A：baremetal eHSM能力验证 | DD-02B：gsp产品安全链 |
|---|---|---|
| 目标 | 验证eHSM交付的全部功能、接口、模式和错误路径 | 实现芯片安全软件方案规定的产品Feature |
| 目标仓库/执行栈 | `../baremetal`独立软件栈，运行在安全核上；后续批准任务内实施 | `../gsp-pmp-rmp-omp`产品固件；后续批准任务内实施 |
| 项目计划/详设/case list | 统一保存在`security_-scheme` | 统一保存在`security_-scheme` |
| 顶层控制 | follow baremetal的case注册、命令入口、runner和结果规则；不经过GSP固件 | follow BootROM/FMC/GSP职责、启动状态机和SPDM/Runtime集成设计 |
| Vendor Demo地位 | 全功能覆盖清单和低层case实现的主要输入 | 能力/API/命令格式和底层通用函数的实现输入 |
| command拼装 | 无特殊差异时优先复用，并逐case保留输入、输出和raw result | 复用协议格式/编码函数；payload业务内容由实际启动链生成 |
| 平台代码 | 替换为baremetal目标平台port | 替换为NGU800P production port |
| PASS依据 | 最新有效case list/工作簿、eHSM接口规范、真实返回和目标环境Evidence | SRC-0017→SRC-0016→批准详设/Requirement/OpenSpec→真实Evidence |
| 禁止事项 | 不以单个Demo最终打印作为所有子case PASS；不可逆case不得默认连跑 | 不移植Demo main/固定OTP数据/测试向量/无界轮询/模拟成功/测试密钥 |

# DD-02A：baremetal完整能力验证移植

## 覆盖规则

1. 以`ehsm_demo_test()`为根建立两级清单：一级记录root、BL、FW和可选编译入口；二级把每个`*_entry()`内部的算法、模式、key type、同步/异步、正向/负向和边界组合展开为独立case。
2. 一级或二级入口不得因默认被注释、编译宏关闭、重复调用或具有破坏性而从清单中消失；必须分别标成`enabled_by_default`、`compile_time_optional`、`source_present_not_called`、`duplicate_pending_review`或`destructive_controlled`。
3. “全部覆盖”是case inventory完整，不等于所有case在同一镜像、同一生命周期、同一OTP实例中无条件顺序执行。BL态和FW态分别执行；OTP写入、LCS切换、control field、key安装/删除、升级和Debug状态变更使用独立fixture及恢复策略。
4. Vendor command组包、响应解析和测试向量可作为直接复用候选；发现地址域、对齐/cache、endianness、API版本、生命周期、密钥策略或Expected与NGU800P环境不一致时，保留协议格式并改写环境/策略相关内容。
5. 每个可执行case必须输出稳定的Case ID、阶段、输入profile、raw eHSM result、mapped result、PASS/FAIL/INCONCLUSIVE及Evidence路径，不能只依赖`demo ends with success`。
6. 顶层不得沿用Vendor Demo的无界ready轮询；timeout、reset、资源清理和失败隔离由baremetal测试框架统一负责。

## Case list出口

一级清单见[`tests/cases/EHSM-DEMO-CASE-CATALOG.md`](../../tests/cases/EHSM-DEMO-CASE-CATALOG.md)。完成二级源码展开后，再映射到不覆盖v0.2的新版本测试工作簿；工作簿case与`baremetal`可执行入口保持一一或一对多可追溯关系。

# DD-02B：gsp产品安全链移植

## 第一阶段输入边界

| `bl_demo`能力 | 产品采用方式 | 当前设计状态 |
|---|---|---|
| Host library/ctx、Mailbox command与response解析 | 提取通用底层能力，接入NGU800P port和统一错误 | DD-02继续冻结API/参数 |
| ready/reset/self-test/raw bitmap | 按Bootloader位图裁决和有界等待实现，不沿用Demo无界循环 | 位图已批准；寄存器/timeout/reset参数待确认 |
| image verify | 复用底层命令/格式；镜像typed-stage身份、Header Overlay地址、算法Profile和调用时机由安全启动详设提供 | 与DD-03联动 |
| image upgrade | 只作为eHSM能力输入；产品更新/OOB状态机由软件方案定义 | 后续DD-06/DEV-SEC-011 |
| OTP key、OTP/REG读写 | 仅在方案规定的Owner、slot/offset、权限和生命周期下封装，不提供任意生产读写入口 | 参数/权限待专题冻结 |
| Debug Auth | 复用协议能力，授权策略、token、LCS门禁和失败终态由软件方案定义 | 后续DD-06 |
| UART baud divisor | 视为Vendor平台/Demo能力，不自动进入产品安全业务层 | NGU800P port核实 |
| Demo entry和打印 | 不移植为产品顶层 | 已排除 |

## 产品调用和串联规则

1. BootROM/FMC/GSP上层只能调用语义化产品接口，例如“验证指定阶段镜像”“读取并判定self-test”“执行批准的counter操作”，不能直接复制Demo中的固定payload顺序。
2. 语义化接口在进入Host command层前，必须完成内部typed-stage、Header offset1008/1016/1020、Header CRC、固定baremetal System Address、buffer extent、受信算法Profile、LCS/权限和deadline检查；不得建立canonical/local地址转换，CRC通过也不得绕过Vendor认证。
3. command层负责按Vendor格式编码请求、发送、解析响应并保留raw status；不得替上层选择镜像、密钥、地址、release条件或失败后的下一状态。
4. `bl_demo`未出现但软件方案需要的功能，继续从Vendor正式API/文档和`fw_demo`中寻找底层能力；没有对应实现时形成GAP，不得用stub补成成功。
5. SPDM协议栈不从`bl_demo`移植。SPDM调用哪些Measurement、证书、签名或随机数能力，由DD-05/SPDM详设决定；eHSM Host层只提供经批准的底层服务。

# 复用分类

| 分类 | 内容 | 规则 |
|---|---|---|
| A：优先复用 | command ID/结构编码、通用序列化/反序列化、与平台无关的response校验、明确无策略的Host辅助函数 | 核对版本、边界检查、license和错误传播后复用 |
| B：格式复用、内容重建 | Mailbox header和payload字段格式、image verify/upgrade、OTP/Debug/Key命令请求结构 | 字段值由case或实际启动链提供，不复制Demo常量 |
| C：按目标重做 | MMIO/base/channel、shared memory、64位System Address同值校验、PMA/barrier、timer、reset、interrupt、critical section、日志 | baremetal与production分别实现各自port |
| D：禁止进入产品路径 | Demo main/entry顺序、固定OTP初始化、测试key/vector/provider、无界轮询、`printf`成功判定、simulated success/silent fallback | 仅作为Vendor代码事实或测试输入 |

# 共同停止条件

- Vendor Host/BL/FW之间的command格式、枚举或语义存在冲突。
- SRC-0016要求的业务行为在Vendor API中不存在，或只能通过测试/调试接口完成。
- Demo中的OTP/LCS/Debug/Key行为与批准方案、不可逆资源策略或目标eHSM版本不一致。
- NGU800P平台参数、System Address范围、PMA/barrier、Mailbox、reset或timeout来源不明确，且继续设计会产生确定性错误；两个批准PMA候选均不执行data clean/invalidate。

遇到上述情况时，未受影响的清单盘点可继续；受影响接口标成`CONFLICTING`并按项目流程提交负责人裁决。

# 当前下一步

1. 完成`ehsm_demo_test()`二级case展开，记录每个entry内部的真实command、输入组合、编译宏、前置状态、Expected和破坏性等级。
2. 只读盘点`bl_demo`调用的Host API及其底层实现，形成A/B/C/D复用映射和目标文件/符号表。
3. 读取baremetal现有case注册/runner规范，冻结DD-02A顶层入口，不继承其既有stub Expected。
4. 对照SRC-0016安全启动和SPDM章节，冻结DD-02B第一阶段实际需要的语义化接口清单。
5. 参数和冲突关闭后，分别创建`baremetal`可执行测试实施任务与`gsp-pmp-rmp-omp`产品实现OpenSpec；二者不得合并为一个跨仓任务。

# Change history

- 2026-07-22：根据负责人确认，将DD-02拆为baremetal完整eHSM能力验证移植和gsp产品安全链移植；明确Demo case覆盖、command复用、产品编排和SPDM边界。未修改两个代码仓，未执行Git、构建或测试。
