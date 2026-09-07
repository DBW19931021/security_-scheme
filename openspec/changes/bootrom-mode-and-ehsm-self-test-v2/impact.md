# Impact

## 软件

- BootROM模式判定和状态机需按新策略实现。
- 移除BootROM self-test start API，保留status读取。

## 安全

- LCS异常不再fail-close到安全停机，而是进入权限严格受限的非安全启动；该行为必须由独立profile隔离。
- 非安全路径不生成可能被误解为可信启动证据的Measurement或审计。

## 测试

- 增加LCS失败/非法/UNDEFINED/来源不可信/未批准组合的非安全模式Expected。
- 增加“BootROM未发送自检命令”的观测。
- 归一化策略测试固定`boot_pin.secure_boot[3]`为0非安全/1安全；当前RTL/生成头同步前，真实寄存器与EMU Expected标记`BLOCKED_BY_RTL_SYNC`。

## 继续有效

- `non_sec_boot=0`时的USER强制安全、Vendor Header安全链、Counter和安全路径release门禁。
- 2026-07-29负责人指定Strap以SRC-0023截图为准：`boot_pin.secure_boot[3]`、default 0、0非安全/1安全；当前SRC-0022 bit0/bit3定义不得用于产品绑定。
