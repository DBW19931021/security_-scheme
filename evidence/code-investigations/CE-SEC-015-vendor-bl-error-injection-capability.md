# CE-SEC-015：Vendor BL 软件注错能力调查证据

## 调查元数据

- 对应任务：`INV-SEC-015`。
- 状态：`evidence_ready / awaiting_design_review`。
- 代码来源：SRC-0018。
- 快照：BL `2.3.5-4019-72f8fdc`；Python BL tests/server `1.1.2-4019-8fad478`。
- 执行日期：2026-08-11。
- 执行约束：只读代码、生成头和 ELF；未修改 Vendor 或任何业务代码，未构建、未执行固件测试。

## 结论

| ID | 分类 | 结论 | 证据 |
|---|---|---|---|
| `FACT-1501` | `CODE_FACT` | Host `ehsm_inject_error()` 先检查/重置 context，将 command ID 设为 `MB_CMD_ID_BL_INJECT_ERR=0xFF13`，复制 6 个 32-bit trigger word，然后走普通 `ehsm_send_cmd()`。 | `python_bootloader_tests-1.1.2-4019-8fad478/server/src/api.c:3242-3267`；`server/src/bl_mb.h:391-405` |
| `FACT-1502` | `CODE_FACT` | Payload 由 `fw_trig[2]`、`hw_trig[2]`、`alarm_trig[2]` 组成，目标分别是 EMU `ERR_FW_0/1`、`ERR_HW_TRIG_0/1`、`FUSA_ALARM_TRIG_0/1`。 | `server/include/ehsmdrv/basic/api.h:91-100`；`server/src/mb.h:1378-1389`；`server/src/bl_mb.h:391-405` |
| `FACT-1503` | `CODE_FACT` | Python UART Host 先把 24-byte values 写到 Host server RAM，再传递 `ctx_addr + values_addr`；server command 回调最终调用 C `ehsm_inject_error()`。 | `platform_adapter/api/uart_impl.py:1902-1908`；`platform_adapter/uart_lib/hostapi.py:2152-2165`；`server/hostapi_commands.c:2803-2815,4126-4140` |
| `FACT-1504` | `CODE_FACT` | 当前 BL parser 的 switch 没有 `MB_CMD_ID_BL_INJECT_ERR/0xFF13` case，default 返回 `EHSM_ERR_INVALID_CMD`。 | `ehsm_bl-2.3.5-4019-72f8fdc/src/component/mbcmd_parser.c:558-616` |
| `FACT-1505` | `CODE_FACT` | 当前 BL `inc/config.h` 没有 `CONFIG_BL_INJECT_ERR_ENABLE`；当前 BL 源码也没有 `inject_err/fw_trig/hw_trig/alarm_trig` 处理。 | `ehsm_bl-2.3.5-4019-72f8fdc/inc/config.h:1-91`；对当前 BL 目录的字符串/符号检索 |
| `FACT-1506` | `CODE_FACT` | 当前 BL ELF 符号表包含 `mbcmdpars_parse_cmd`、`emu_trigger_fw_error`，但没有 inject handler 或 hw/alarm trigger handler。 | `python_bootloader_tests-1.1.2-4019-8fad478/resource/elf/ehsm_bl.elf` 的 `nm` 符号核对 |
| `FACT-1507` | `CODE_FACT` | Host `api.h` 中的 `// #% #if CONFIG_BL_INJECT_ERR_ENABLE` 是被 C 注释的生成模板标记，不是当前编译单元的 C 预处理条件。 | `server/include/ehsmdrv/basic/api.h:91,100,3713-3725` |
| `GAP-1501` | `GAP` | Host 请求格式已存在，但当前匹配 BL 固件没有处理路径；Host 单侧“开启宏”不能使能该功能。 | `FACT-1501`–`FACT-1507` |
| `TARGET-1501` | `TARGET_DESIGN` | Vendor 固件默认不修改。`SOC-HSM-ERROR-001` 只在新的匹配 BL source/image/config/build ID 明确含 `0xFF13` handler 后才可作为软件 case；当前返回 `INCONCLUSIVE/BL_ERROR_INJECT_UNAVAILABLE`。 | 项目负责人输入；本报告代码事实 |

## Host 触发链

```text
baremetal/Python case
  -> 准备 ehsm_inject_error_st (6 x uint32_t)
  -> ehsm_inject_error(ctx, values)
  -> command 0xFF13 + fw_trig/hw_trig/alarm_trig
  -> ehsm_send_cmd()
  -> Vendor direct Mailbox
  -> [必须存在] eHSM BL 0xFF13 handler
  -> EMU ERR_FW_0/1, ERR_HW_TRIG_0/1, FUSA_ALARM_TRIG_0/1
  -> o_hsm_err_fw/o_hsm_err_hw/alarm/status/IRQ
```

当前链路在“eHSM BL `0xFF13` handler”处中断。Host 依然可以发出 raw command，但目标 BL parser 会走 `default` 并返回 invalid command；这不能作为软件注错 PASS。

## `CONFIG_BL_INJECT_ERR_ENABLE` 启用后的预期语义

根据生成 Host API 和 command 布局，完整能力需要两侧同时匹配：

1. Host 侧包含 `ehsm_inject_error_st`、`ehsm_inject_error()` 和 `0xFF13` command 结构。
2. BL 侧在相同 command ID 下注册 handler，并只在批准的 test build/LCS 中允许执行。
3. Handler 验证 command 完整性/权限后，将 6 个 word 写入对应 EMU trigger 寄存器。
4. SoC wrapper/RAS/APLIC 将生效的 status/error/alarm 信号发布给 C908，Host 根据权威 bit/IRQ/clear/reset 合同判定。

第 2–4 项当前均不能从 Host 头文件单独得到证明。

## 解除 `SOC-HSM-ERROR-001` 门禁所需材料

1. 包含 `0xFF13` handler 的 BL source/image/config/build ID 和 release note。
2. `CONFIG_BL_INJECT_ERR_ENABLE` 的实际生成/编译方式，以及是否仅 TEST/DEV LCS 可用。
3. `fw_trig/hw_trig/alarm_trig` 每个允许 bit 与顶层 output/status/IRQ 的权威映射。
4. 触发位的 W1T/自清/锁存、寄存器 clear、eHSM reset 和 power reset 语义。
5. 安全的首个测试 bit、是否会实际破坏数据、对应的恢复/处置方法和 EMU/FPGA 执行授权。

## 已回填

- `docs/06-verification/NGU800P软件不便覆盖的硬件安全验证项.md`。
- `docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md`。
- `../baremetal/components/security/openspec/changes/enforce-mailbox-only-otp-access/design.md`。
- `../baremetal/components/security/openspec/LEGACY_ITEMS.md`。

## 结论边界

本报告证明“当前可见 BL 2.3.5 快照不包含 `0xFF13` handler”，不证明 Vendor 其他分支、内网新版 BL 或未交付 test build 也不包含该功能。后续如取得新 baseline，必须重新执行 source/image/config/ELF 四项一致性核对。
