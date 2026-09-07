# INV-SEC-002：FMC/GSP验签解密加载链

- 调查状态：`evidence_ready / awaiting_design_review`
- 证据报告：[CE-SEC-002](../../evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md)
- 完成日期：2026-07-21

## 调查背景

- 对应Feature：SEC-FEAT-003～006、009、019。
- 对应正式详设：`docs/03-architecture/secure-boot.md`、`docs/03-architecture/anti-rollback.md`、`docs/05-software-design/fmc.md`、`docs/05-software-design/gsp.md`。
- 触发问题：已有盘点观察到image/header/manifest/policy/measurement框架和`verify_image()`，但需要确认实际验签、解密、加载、measurement、release和跳转链以及stub边界。
- 目标仓库：`../gsp-pmp-rmp-omp`。
- 只读基线：引用 `manifests/repositories.yaml`，不为本任务执行Git命令。

## 调查目标

确认FMC/GSP镜像从解析、策略检查、eHSM验签/解密、counter/measurement到加载和跳转的真实代码路径及缺口。

## 需要回答的问题

1. image/header/manifest的解析入口、结构体、字段校验和版本/字节序规则在哪里？
2. `verify_image()`及其下游真实调用链是什么，何处仍使用eHSM/crypto/cert/transport stub？
3. 签名覆盖区、解密输出、load/entry范围和buffer所有权如何处理？
4. anti-rollback counter在哪里比较/更新，measurement何时写入，失败记录如何表示？
5. FMC→GSP→Runtime的release/加载/跳转条件和失败终态是什么？
6. Host/unit/QEMU测试覆盖哪些production逻辑，哪些只覆盖mock/stub？

## 建议检查范围

- `../gsp-pmp-rmp-omp/components/security/`
- `../gsp-pmp-rmp-omp/solutions/fmc/`、`solutions/gsp/`及必要glue。
- image packager、Host/QEMU测试、构建配置和链接脚本。
- 关键字：`verify_image`、`manifest`、`policy`、`measurement`、`rollback`、`counter`、`decrypt`、`load`、`entry`、`jump`。

## 证据要求

- 每个结论提供仓库相对路径、符号和紧凑行范围。
- 提供数据流、调用链、失败路径和构建/测试条件。
- 明确`CODE_FACT`、`INFERENCE`、`GAP`、`UNKNOWN`和`CONFLICTING`。
- 不把Host测试存在等同于production硬件路径已接入。

## 输出

- 报告：`evidence/code-investigations/CE-SEC-002-fmc-gsp-verification-chain.md`
- 回填：DD-01系统启动链和DD-03镜像/Manifest/验证/回滚。
- 追溯：SEC-FEAT-003～006/009/019及相关测试case。

## 限制和停止条件

- 当前任务只读，不修改业务代码、构建文件或正式设计结论。
- 不执行Git命令，不写入两个代码仓。
- 方案、代码和Vendor证据冲突时停止受影响结论并报告。

## 验收

- [x] 解析到跳转的完整调用链有代码证据；当前链在stub/measurement后结束，loader/jump为GAP。
- [x] eHSM/crypto/cert/transport的production与stub边界已明确。
- [x] counter、measurement、失败终态和清零行为有结论或UNKNOWN。
- [x] 测试覆盖、目标设计差距和下一步动作明确。

## 调查结果

- FMC/GSP target 均链接 security，但默认 main 分别为裸机/FreeRTOS hello-world，没有 production verify/release。
- 唯一 verify flow 直接调用 `ehsm_verify_decrypt_stub()`；制包工具不签名、不加密，Host/QEMU tests 使用 stub/synthetic data。
- rollback 只有字段和模拟 checked bit；measurement 为本镜像 BSS store，没有真实 digest、失败记录或跨 stage handoff。
- 调查时发现GSP packager/tests与linker/README地址view不一致并创建OPEN-CONFLICT-003；2026-07-22已按ADR-0004采用NoC/system canonical `0x1010_0808_0000`关闭，local/remap转换仅允许在port/loader。
- 已回填`secure-boot.md`、`anti-rollback.md`、`fmc.md`和`gsp.md`。待负责人完成设计评审后，将本任务移动到`tasks/completed/`。

后续批准记录：当前不采用版本化Handoff；ADR-0030后FMC/GSP详设使用Measurement Table、Header Overlay/typed-stage loader、统一错误和既有release接口。任务中对`manifest`的搜索仍用于识别并删除当前代码遗留，不代表目标设计保留Manifest。
