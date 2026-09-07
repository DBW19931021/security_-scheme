# CE-SEC-003：eHSM Demo一级入口调查证据

## 调查元数据

- 对应任务：INV-SEC-003。
- 状态：`evidence_ready_level_1 / level_2_pending`。
- 代码来源：SRC-0018 `ehsm_host-2.3.1-4019-2ee044d`。
- 执行日期：2026-07-22。
- 执行约束：只读；未执行Git、构建或测试；未修改Vendor、`gsp-pmp-rmp-omp`或`baremetal`源码。

## 调查问题和结论

| ID | 状态 | 结论 | 证据/限制 |
|---|---|---|---|
| FACT-301 | `CODE_FACT` | `ehsm_demo_test()`在`CONFIG_BUILD_HOST_DEMO`下初始化Host library，写Demo OTP默认值、reset/等待ready，再按版本进入BL或FW entry。 | `demo/test.c:21-60`；尚未核实所有port hook实现。 |
| FACT-302 | `CODE_FACT` | `CONFIG_HOST_BL_PATCH_TEST_ENABLE`启用时走`demo_bl_patch_test_entry()`并跳过普通BL/FW分支。 | `demo/test.c:34-36,61`；patch子case待Level 2。 |
| FACT-303 | `CODE_FACT` | BL顶层调用OTP key、OTP/REG读写、Debug Auth、image verify/upgrade、UART和self-test；Debug Auth连续调用两次。 | `demo/bl_demo/bl_demo.c:27-58`；两次调用意图未知。 |
| FACT-304 | `CODE_FACT` | FW顶层调用crypto、RNG、OTP/control、Debug、Key Manager、image verify/upgrade等入口；lifecycle入口实现存在但顶层调用被注释。 | `demo/fw_demo/fw_demo.c:74-168`；`otp/ehsm_demo_chg_lifecycle.c:82`。 |
| FACT-305 | `CODE_FACT` | `test_parallel()`只发现一处被注释的调用，未在当前Host快照源码中找到声明或定义。 | `demo/fw_demo/fw_demo.c:87`及对非build源码的符号检索；需确认漏交付/旧引用/不支持。 |
| GAP-301 | `GAP` | 顶层entry不能证明内部case组合完整，也不能提供稳定的逐case结果；Vendor Demo最终只打印总体success。 | 需要Level 2逐函数体展开和baremetal结果协议。 |
| TARGET-301 | `TARGET_DESIGN` | baremetal按BL/FW/fixture拆分case并完整覆盖Vendor能力；command组包无差异时优先复用，顶层follow baremetal。 | 负责人确认；详见DD-02，不是已实现事实。 |
| TARGET-302 | `TARGET_DESIGN` | gsp产品路径第一阶段主要使用`bl_demo`能力，只复用合适底层函数/格式，payload和启动/SPDM编排由软件方案决定。 | 负责人确认；详见DD-02，不是已实现事实。 |

## 当前调用链

```text
ehsm_demo_test
  -> ehsm_driver_init_library(WAIT_AND_POLL)
  -> ehsm_port_write_otp
  -> demo_reset_ehsm_wait_ready
  -> poll REG_HSM_STATUS_0
  -> ehsm_demo_test_get_version
       -> BL: ehsm_bl_demo_entry
       -> FW: ehsm_fw_demo_entry

CONFIG_HOST_BL_PATCH_TEST_ENABLE
  -> demo_bl_patch_test_entry
```

该链仅描述当前Vendor Demo代码，不是NGU800P产品启动链。Demo中的OTP初始化、reset和无界ready轮询不能直接移植为产品行为。

## 已回填

- `docs/05-software-design/ehsm-osr-host-porting.md`：双路径边界、复用分类和后续调查。
- `tests/cases/EHSM-DEMO-CASE-CATALOG.md`：Root、BL和FW一级入口。
- `docs/04-interfaces/ehsm-mailbox.md`：baremetal与产品路径的command/port边界。

## 未完成和下一步

1. 读取每个entry函数体，列出实际Host API/command、参数组合、expected/raw result和错误路径。
2. 核对Host header、BL/FW命令实现和Vendor文档的枚举/结构一致性。
3. 核对所有`CONFIG_*`及生成器标记是否真正影响构建。
4. 形成破坏性fixture和恢复要求，避免OTP/LCS/Key/升级case无条件连跑。
5. 形成`bl_demo`底层函数A/B/C/D复用表和NGU800P产品语义接口。

## 结论边界

本报告只完成一级入口调查。它支持建立完整性清单和后续任务，不支持声称所有eHSM接口已盘点完成、Vendor Demo已通过、NGU800P硬件已确认或产品移植已就绪。
