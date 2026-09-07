# Impact

## Design

- BootROM启动策略输入增加1个只读快照字段和最高优先级分支。
- 既有受限非安全加载、权限和release Profile保持不变。
- USER强制安全规则增加唯一例外：`non_sec_boot`可靠断言为1。

## Implementation

- BootROM纯策略函数、平台Boot Policy Fuse port和错误枚举需要后续实现。
- RTL/eFuse控制器需提供复位稳定、只读、带valid/ECC状态的字段视图。
- Provisioning/维修流程需提供受控0→1烧写、readback、锁定和审计。

## Verification

- 增加值0兼容、值1覆盖、读取异常、权限隔离、单向烧写和复位重新锁存测试。
- 准确产品寄存器测试等待OPEN-DESIGN-024关闭。
