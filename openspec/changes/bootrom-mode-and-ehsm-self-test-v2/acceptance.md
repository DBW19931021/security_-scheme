# Acceptance

- LCS异常的所有分支均进入受限非安全模式，不回落安全模式。
- 在`non_sec_boot=0`时，USER仍忽略Strap并强制安全启动；值1覆盖规则见后续`bootrom-non-sec-boot-efuse-override-v1`。
- 已识别非USER的归一化策略固定为`boot_pin.secure_boot[3]`值0进入非安全顶层、值1安全；值0时DEV/MANU选择`MANUFACTURING_PROVISIONING`，其他LCS选择`RESTRICTED_NONSECURE`，禁止fallback。
- 当前RTL/生成头同步前，真实寄存器绑定和EMU Expected明确为`BLOCKED_BY_RTL_SYNC`，且不能直接使用当前bit0/bit3。
- 非安全路径没有Measurement/SoC State/启动审计写入。
- BootROM状态机不存在发起eHSM自检的命令。
- eHSM eFuse要求/不要求自检两种路径均有明确Expected。
- 主详设、ADR、OpenSpec、专题和项目状态一致。
