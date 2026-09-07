# INV-SEC-015：Vendor BL 软件注错能力

## 调查背景

- 对应顶层任务：`TASK-SEC-BAREMETAL-EHSM-001`。
- 对应 case：`SOC-HSM-ERROR-001`。
- 触发问题：确认 `CONFIG_BL_INJECT_ERR_ENABLE` 启用后 eHSM BL 与 Host 各做什么，以及当前 Vendor 交付是否真实包含 `0xFF13` handler。
- 目标代码快照：SRC-0018 中的 BL `2.3.5-4019-72f8fdc`、Host/Python tests `1.1.2-4019-8fad478`。

## 调查目标

1. 确认 Host `ehsm_inject_error()` 的 command ID、payload 和发送链。
2. 确认 `fw_trig/hw_trig/alarm_trig` 的目标寄存器。
3. 确认当前 BL 源码、配置和 ELF 是否包含该命令 handler。
4. 给出 `SOC-HSM-ERROR-001` 当前可用的 verdict 和解除门禁材料。

## 范围和限制

- 只读 `../osr_eshm` 对应 Vendor 快照、生成 Host API 和 ELF 符号。
- 不修改 Vendor BL/FW/Host、`../baremetal` 或 GSP 代码。
- Host 头文件中出现 API 不等于目标固件存在 handler，必须完成两侧核对。

## 输出和状态

- 证据：`evidence/code-investigations/CE-SEC-015-vendor-bl-error-injection-capability.md`。
- 状态：`evidence_ready / awaiting_design_review`。

## 验收

- [x] Host command 构造与发送链已读到函数体。
- [x] 目标 BL parser/config/ELF 已核对。
- [x] 代码事实、目标设计和缺口已分类。
- [x] 已回填测试设计和 OpenSpec 遗留项。
