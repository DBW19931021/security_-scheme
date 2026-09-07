# Design

## 系统上下文

BootROM初始化Header并追加FMC；FMC消费FMC Entry、验证GSP并追加GSP；GSP追加eHSM FW和实际Runtime实例，最后提交唯一SoC State。State提交后Table不可再扩展。

## 信任边界

- Measurement Region位于Host不可访问的安全RAM。
- BootROM、FMC、GSP按stage顺序拥有Header append权和各自Entry写权限。
- eHSM不写Measurement；eHSM状态进入Adapter/RAS。
- CRC检测撕裂/随机损坏，不替代Firewall、可信Writer或SPDM签名。

## 布局

Header/Firmware/State均为128字节；对象尾部offset 120为CRC、124为commit。Header只保留结构、实际count/length、设备标识和一个reserved。Firmware只保留实例、Hash、16字节counter、验证/release、64位`SOC_PA`和一个reserved。State只保留SoC状态、证书摘要和一个reserved。

## 状态机

`EMPTY -> WRITING -> COMMITTED`

追加Entry：

`Header WRITING -> Entry COMMITTED -> update count/length/CRC -> Header COMMITTED`

State COMMITTED后Table finalized。

## reset

ABI不保存generation。BootROM必须先把旧Header置EMPTY并清零整个固定Region，再建立新Table；清零或可见性失败时fail-close。

## snapshot

Reader复制Header A、紧凑Firmware列表和唯一State，再复制Header B；只有A/B一致且所有对象CRC/commit有效时接受。变化时重试一次，仍变化返回`SNAPSHOT_BUSY`。

## 地址

Manifest和loader验证baremetal System Address范围及Region。Measurement只保存loader最终使用的同一64位System Address，不保存domain、remote或alias地址。

## 测试影响

需要覆盖可变count/length、容量上限、乘加溢出、紧凑前缀、重复实例、Header append、State finalization、reset全清零、CRC/commit、snapshot并发和eHSM FW package Hash。

## 未解决输入

- `max_fw_entries`及微核共享/独立release清单；
- OPEN-CONFLICT-006物理地址、PMA/cache和Firewall；
- OPEN-DESIGN-015 SPDM wire block/profile。
