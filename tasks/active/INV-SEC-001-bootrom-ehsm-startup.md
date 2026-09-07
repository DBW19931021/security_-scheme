# INV-SEC-001：BootROM与eHSM启动交互

- 调查状态：`evidence_ready / awaiting_design_review`
- 证据报告：[CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)
- 完成日期：2026-07-21

## 调查背景

- 对应Feature：SEC-FEAT-001、002、005、019。
- 对应正式详设：`docs/03-architecture/secure-boot.md`、`docs/04-interfaces/ehsm-mailbox.md`、`docs/05-software-design/bootrom.md`。
- 触发问题：正式详设需要确认公司代码当前是否真实等待eHSM ready、如何读取self-test/LCS、如何处理timeout/reset/error以及何时release FMC。
- 目标仓库：`../gsp-pmp-rmp-omp`；Vendor实现参考为SRC-0018，只用于核对eHSM/Core能力。
- 只读基线：引用 `manifests/repositories.yaml`，不为本任务执行Git命令。

## 调查目标

确认BootROM到eHSM再到FMC的当前真实调用链、构建入口和失败行为，识别demo/stub与production路径边界。

## 需要回答的问题

1. BootROM安全启动入口在哪里，由哪个构建目标、宏和链接配置纳入？
2. 是否存在eHSM上电/ready等待、自检状态、LCS/eFuse读取；调用的实际函数和下游实现是什么？
3. timeout、busy、retry、reset、返回值和日志如何处理；失败后是否阻止FMC release/跳转？
4. FMC镜像的地址、长度、entry和handoff数据从哪里取得并如何校验？
5. 当前路径中哪些属于demo、stub、mock或test-only，哪些可进入production构建？
6. OPEN-CONFLICT-001影响哪些符号和位级判断；不受影响的原始状态采集范围是什么？

## 建议检查范围

- `../gsp-pmp-rmp-omp/components/security/`
- `../gsp-pmp-rmp-omp/solutions/bootrom/`
- 必要的FMC/GSP glue、构建文件、链接脚本和配置头。
- SRC-0018中与ready/self-test/Host API/Bootloader交互直接相关的实现。
- 关键字：`bootrom`、`ehsm`、`ready`、`selftest`、`verify_image`、`release`、`jump`、`timeout`、`lifecycle`。

建议范围只用于提高搜索效率，不限制确认调用链所必需的关联调查。

## 证据要求

- 每个结论提供仓库相对路径、符号和紧凑行范围。
- 给出从BootROM入口到eHSM接口和FMC release/跳转的完整调用链。
- 检查宏、Makefile/Kconfig、构建目标、链接脚本和平台差异。
- 区分 `CODE_FACT`、`INFERENCE`、`GAP`、`UNKNOWN` 和 `CONFLICTING`。
- 历史Review仅作线索，必须重新阅读关键函数体。

## 输出

- 报告：`evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md`
- 回填：DD-01系统边界/启动链和DD-02 eHSM adapter。
- 追溯：SEC-FEAT-001/002/005/019、OPEN-CONFLICT-001及相关测试case。

## 限制和停止条件

- 当前任务只读，不修改业务代码、构建文件或正式设计结论。
- 不执行Git命令，不写入两个代码仓。
- 版本或位图证据冲突时保持`CONFLICTING`，不自行选择一方。

## 验收

- [x] 启动入口、构建条件和完整调用链有代码证据。
- [x] ready/self-test/LCS/timeout/reset/error行为分别有结论或UNKNOWN。
- [x] demo/stub/test-only与production路径已区分。
- [x] 目标设计差距和下一步详设动作明确。

## 调查结果

- 默认 BootROM 为 hello-world 入口，`SECURE_DEMO` 默认关闭；返回后进入启动文件 `__exit` 无限循环。
- 可选 demo 使用内存 synthetic package 和 `ehsm_verify_decrypt_stub()`，没有真实 ready/self-test/LCS/Mailbox/timeout/reset/FMC handoff。
- Vendor Host 提供可参考的 adapter/transport 构件，但 OSR port 的 timeout hook 为空且平台地址不可直接复用。
- 调查时再次确认OPEN-CONFLICT-001；后续于2026-07-22按Vendor回复采用Bootloader定义、Host错误并关闭，原始bitmap仍需保留。
- 已回填 `secure-boot.md`、`ehsm-mailbox.md` 和 `bootrom.md`。待负责人完成设计评审后，将本任务移动到 `tasks/completed/`。

后续批准记录：W0-R1-01～04、06～11已批准，R1-05待评审；现有test/stub流程只作为调查事实，不作为目标设计或最终oracle。
