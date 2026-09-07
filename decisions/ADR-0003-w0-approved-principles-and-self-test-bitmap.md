# ADR-0003：W0 已批准设计原则与 eHSM 自检位图协议

- 状态：accepted
- 日期：2026-07-22
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0012、SRC-0014、SRC-0016、SRC-0017、SRC-0018、SRC-0020
- 相关 Requirement ID：待已批准原则建立Requirement后补齐；R1-05当前不采用
- 相关冲突：CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP / OPEN-CONFLICT-001
- 替代关系：补充 ADR-0001/0002

## 背景

W0 首轮设计评审提出 W0-R1-01～10。项目负责人逐项评审后批准 R1-01～04、R1-06～10，要求进一步解释 R1-05“版本化 Handoff”后再决定，并新增“现有 test/stub 流程不作为最终方案和测试依据”的全局原则。

同时，SRC-0012/Bootloader 与 SRC-0014/Host 对 self-test bit18/bit19 的定义冲突。项目负责人确认 Vendor 已回复：Bootloader 定义正确，Host 定义错误。

## 决策

### 1. 已批准 W0 原则

- W0-R1-01：production target 不得链接或调用 test/demo stub；原有 stub 仅用于早期软件栈流程测试，EMU 和最终实现不存在 stub/simulated success。
- W0-R1-02：eHSM 软件按 command adapter→transport→NGU800P port 分层。2026-07-22负责人进一步明确为双路径：`baremetal`必须以case list覆盖`ehsm_demo_test()`提供的全部功能/接口，command拼装无特殊差异时可复用，顶层执行follow baremetal规则；`gsp-pmp-rmp-omp`第一阶段主要以`bl_demo`能力为输入，产品任务编排、安全启动和SPDM串联以芯片安全软件方案为准。产品路径可复用底层通用函数和command格式，但payload业务内容、平台地址、timer、reset、buffer、错误及安全门禁必须按NGU800P实际链路落实，不照搬Demo流程。实际改动量以实施调查为准。
- W0-R1-03：所有 wait/command 都必须有 deadline；不可逆命令 timeout 后先查询实际状态，不盲目重试。
- W0-R1-04：保留 raw status/error/self-test bitmap；解释按已裁决的 Bootloader 协议执行。
- W0-R1-06：2026-07-28最终裁决更新为共享安全RAM跨组件地址全部使用baremetal System Address且不携带domain；port/loader只做范围校验，不建立Local/System映射。
- W0-R1-07：rollback success 只能来自真实 counter service，不由 manifest 或 stub 模拟。
- W0-R1-08：required measurement/digest 失败时不得成功 handoff、release 或 SPDM 输出。
- W0-R1-09：错误按 stage/domain/mapped/raw code 传播，返回值、result 和 handoff 状态一致。
- W0-R1-10：成功/失败路径均按唯一 Owner 清零临时敏感数据，必需清零未完成时不得 release。

W0-R1-05在本ADR形成时保持`pending_explanation_and_owner_review`。2026-07-22后续裁决已由ADR-0004更新为`deferred_not_adopted_for_current_baseline`：当前不采用版本化Handoff，不作为设计或编码前提。

### 2. 新增 W0-R1-11：最终方案与测试依据

1. 现有 `gsp-pmp-rmp-omp`/`baremetal` 中的 test 流程、stub 流程、demo、synthetic package/measurement 和历史测试预期只用于说明当前代码事实及差距，不作为目标设计、最终 test oracle、验收或发布依据。
2. 最终依据为：SRC-0017 上位原则、SRC-0016 最新有效软件方案、accepted ADR/amendment、批准 Requirement/OpenSpec、`security_-scheme` 最新版本化测试工作簿及目标环境 Evidence。
3. EMU 和产品实现不得包含 stub、simulated success、test key/cert/provider、未批准 hardcode、silent fallback 或“先打桩后默认成功”等可能遗留到最终版本的编码方式。
4. 尚未实现或参数未明确的能力必须在 production 中不可达、禁用或显式 fail-close，不能伪装为完成。
5. 可复用现有测试基础设施的 runner/build 机制，但不得沿用其 stub 行为或 Expected 作为最终产品预期；新的可执行测试由最终测试用例表和批准方案派生。

### 3. eHSM 自检位图协议裁决

对当前 4019 配套范围采用 Bootloader 定义：

| 位 | 项目解释 | 处理 |
|---|---|---|
| bit16 | SHA2 聚合自检 | 按 Bootloader 定义 |
| bit17 | SHA3 自检 | 按 Bootloader 定义 |
| bit18 / `0x40000` | TRNG 自检 | 正式项目解释 |
| bit19 / `0x80000` | 当前 Bootloader 未定义 | 不解释为 TRNG；作为 unknown/reserved 保留 raw 值 |

SRC-0014/Host 中 `SHA256=0x40000`、`TRNG=0x80000` 对当前配套版本是错误定义。NGU800P Host/adapter 不得复制这两个宏，应修正或在兼容层覆盖，并为 Host 错误定义增加回归测试。

项目负责人转述的 Vendor 回复是本次裁决输入；原始 Vendor 邮件、issue 或变更说明尚未作为独立 Source 入库。后续取得原件时补登记 Evidence，但不影响本 ADR 当前生效。

## 安全影响

- 消除 TRNG/SHA256 位误判导致的错误放行或错误阻断。
- 明确现有 stub/test 流程不能反向定义最终产品行为。
- 保留 Vendor Host 通用实现的复用价值，同时隔离 OSR 平台假设。

## 软件影响

- 后续 Adapter 以 Bootloader 位图为协议基准；Host 错误宏不得进入项目公共 ABI。
- production/test 构建隔离成为硬门禁。
- 现有测试代码只能作为现状盘点，不直接迁移其 stub flow 和 Expected。
- R1-05当前不采用；不创建Handoff专用ABI/SRAM/commit协议。跨阶段度量使用Measurement Table，load/entry使用ADR-0030的Header Overlay/typed-stage loader合同，错误使用统一错误/日志机制；见ADR-0004/0030。

## 测试影响

- NGU800P-D0-SECURITY-093 不再因 OPEN-CONFLICT-001 阻塞；v0.3 中应以 bit18=TRNG、bit19=unknown/reserved 更新 Expected，并高亮变更。
- 应新增 Host 错误宏防回归、raw bitmap 保留和 unknown bit 不误判测试。
- 不在 v0.2 上覆盖修改；新的 Expected 进入后续版本工作簿。

## 待办

- 按ADR-0004执行R1-05当前不采用的裁决；仅在出现具体必要性时重新提案。
- 取得 Vendor 回复原件并登记为 Source/Evidence。
- 在批准的 OpenSpec/实施任务中修正 Host 侧映射；本 ADR 不授权当前修改代码仓。

## 参考资料

- SRC-0012 第50页及 SRC-0018 Bootloader `selftest.h`。
- SRC-0014 第245页及 SRC-0018 Host `bl_api.h`。
- `sources/conflict-reports/CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP.md`。
- `docs/09-plans/W0首轮设计评审包.md`。
