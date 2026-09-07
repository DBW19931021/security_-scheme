# INV-SEC-003：eHSM Demo Case与产品复用映射

## 调查背景

- 对应Feature：SEC-FEAT-002、004、016、019。
- 对应正式详设：`docs/05-software-design/ehsm-osr-host-porting.md`、`docs/04-interfaces/ehsm-mailbox.md`。
- 触发问题：负责人要求baremetal完整覆盖`ehsm_demo_test()`全部功能/case，并要求gsp产品路径第一阶段主要基于`bl_demo`能力、但按软件方案重建payload和任务编排。
- 目标仓库/快照：`security_-scheme/source-vault/vendor-code/osr_eshm/ehsm_host-2.3.1-4019-2ee044d`，SRC-0018。
- 只读基线：引用`manifests/repositories.yaml`和SRC-0018登记；本任务不执行Git命令。

## 调查目标

完整展开Vendor Demo的case/command组合，并形成`bl_demo`底层Host实现到NGU800P产品路径的A/B/C/D复用映射。

## 需要回答的问题

1. `ehsm_demo_test()`、BL/FW entry及可选/注释入口分别包含哪些实际command、算法、模式、输入和正负向case？
2. 每个case受哪些宏、eHSM版本、BL/FW状态、LCS、OTP/Key fixture和破坏性条件控制？
3. Vendor command组包/解析中哪些可直接复用，哪些只能复用格式，哪些平台代码必须在baremetal或NGU800P production port重做？
4. `bl_demo`能力如何映射到软件方案规定的产品语义接口；哪些Demo常量、entry顺序或辅助函数不得进入产品路径？
5. BL顶层两次Debug Auth调用及FW `test_parallel()`未解析引用分别是什么性质？

## 建议检查范围

- `demo/test.c`、`demo/bl_demo/**`、`demo/fw_demo/**`、`demo/common/**`。
- Host public/private API、command struct/enum、Mailbox transport、port hooks和构建宏。
- 关键字：`entry`、`CONFIG_`、`EHSM_CMD`、`async`、`callback`、`timeout`、`otp`、`lifecycle`、`debug`、`image`、`key`。
- 需要沿调用链读到实际command构造、响应校验和错误返回，不能只按入口文件名生成case。

## 当前进度

- [x] Level 1：`ehsm_demo_test()`、BL/FW顶层入口盘点。
- [x] 建立`tests/cases/EHSM-DEMO-CASE-CATALOG.md`一级清单。
- [ ] Level 2：逐entry展开command/算法/模式/正负向/边界case。
- [ ] 建立编译宏、版本、fixture、破坏性和恢复映射。
- [ ] 建立`bl_demo` Host API的A/B/C/D复用表和产品语义接口表。
- [ ] 抽查关键command函数体及错误路径。

## 证据要求

- 路径、符号和紧凑行范围；入口到command/transport调用链。
- 输入/output/raw result、错误返回、buffer owner和敏感材料清理。
- 宏、版本、driver mode、同步/异步、callback和目标平台差异。
- 区分`CODE_FACT`、`INFERENCE`、`TARGET_DESIGN`、`GAP`、`UNKNOWN`和`CONFLICTING`。

## 输出

- 阶段证据：`evidence/code-investigations/CE-SEC-003-ehsm-demo-entry-inventory.md`。
- 最终回填：`tests/cases/EHSM-DEMO-CASE-CATALOG.md`二级清单、DD-02复用表、eHSM Mailbox接口和开发/测试任务。

## 限制和停止条件

- 当前任务只读Vendor、gsp和baremetal代码，只写`security_-scheme`调查/设计资料。
- 不修改Vendor源码、`gsp-pmp-rmp-omp`或`baremetal`，不执行Git、构建或测试。
- 遇到文档/代码版本、Host/BL/FW格式、方案/接口语义冲突时停止受影响映射并提交冲突报告。
- 不把Vendor Demo结果描述为NGU800P硬件已通过或产品安全链已实现。

## 验收

- [ ] 全部一级entry已展开到可执行case粒度或有明确UNKNOWN原因。
- [ ] 关键结论具有函数体、调用链、宏和错误路径证据。
- [ ] A/B/C/D复用映射和产品语义接口可直接指导后续OpenSpec/实施任务。
- [ ] baremetal与gsp两个目标的case/代码边界没有混写。
