# BootROM Secure Boot Strap位号与Lifecycle策略冲突

> 2026-07-29裁决：项目负责人指定以SRC-0023截图为本项产品权威，固定`boot_pin.secure_boot[3]`、default 0、`0=非安全启动/1=安全启动`。当前SRC-0022生成头仍把bit0命名为`SEC_BOOT`、bit3命名为`DIE_ID`，所以策略已关闭，真实RTL/寄存器绑定仍为`BLOCKED_BY_RTL_SYNC`。当前完整行为见[《NGU800P安全软件详细设计》第6章](../../docs/05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)。

> 2026-08-25：SRC-0034/ADR-0031新增`non_sec_boot`最高优先级覆盖。只有该值为0时消费本报告的Lifecycle×Strap结论；值1时包括USER在内均进入现有受限非安全路径。该新增字段的物理绑定由OPEN-DESIGN-024管理，不属于本Strap冲突。

## Identity

- Conflict ID: CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY
- Open Question: OPEN-CONFLICT-010
- Status: design_resolved / BLOCKED_BY_RTL_SYNC
- Evidence state: CONFLICTING
- Owner: 项目负责人；SoC/RTL/BootROM启动Owner
- Decision required by: 已识别非USER的BootROM模式判定寄存器编码和真实EMU Strap Expected之前

## Conflict classification

- Type: user_checklist_vs_rtl_synchronized_register_binding / interface_mapping
- Affected scope: `secure_boot` Strap读取、BootROM secure/non-secure模式判定、测试Expected和发布配置。
- Safe-to-continue scope: `non_sec_boot=0`时的归一化模式策略、USER强制安全、LCS异常受限非安全，以及完整安全启动链、eHSM/FMC验证、Measurement和失败闭锁。
- Must-stop scope: BootROM真实寄存器绑定、真实EMU Strap Expected、使用当前bit0旧极性、把当前名为`DIE_ID`的APB bit3强行当成`secure_boot`，或用私有裸位号绕过RTL/生成头修正。

## Source A：SRC-0023截图

- 字段：`boot_pin.secure_boot`
- bit：`[3]`
- default：0
- 0=非安全启动，1=安全启动

## Source B：SRC-0022 RTL同步生成头

`mgmt_sys_apb_reg.h`和`sec_sys_apb_reg.h`均定义：

```text
STRAP_PIN_SEC_BOOT_POS  = 0
STRAP_PIN_SEC_BOOT_MASK = 0x1
STRAP_PIN_DIE_ID_POS    = 3
STRAP_PIN_DIE_ID_MASK   = 0x8
```

生成日期为2026-07-15，寄存器版本`0.85r_0708`。项目负责人此前确认SRC-0022是SoC地址/寄存器/IRQ数值的第一权威源。

## 仍未证明的实现映射

1. 截图的`boot_pin[3]`是RTL顶层输入向量，进入MGMT/SEC软件寄存器时重排为bit0。
2. 截图属于另一版本或另一寄存器视图。
3. SRC-0022生成头或截图位号之一有误。

产品定义已经由负责人选择SRC-0023，不再在两份资料之间二选一；但没有匹配的RTL mapping或修正后的生成头时，BootROM仍无法安全绑定到软件可见寄存器。

## 已接受结论

1. 产品字段以SRC-0023为准：`boot_pin.secure_boot[3]`，default 0，`0=非安全启动`、`1=安全启动`。
2. 在`non_sec_boot=0`时，USER忽略Strap并强制安全启动；已识别非USER状态采用`0=受限非安全、1=安全`。
3. LCS读取失败、非法或未识别时按ADR-0024进入受限非安全；SoC LCS读取不依赖eHSM Autoload。
4. 当前SRC-0022的bit0/bit3定义只作为冲突实现证据，不得覆盖产品合同。产品代码和真实case必须等待RTL/生成头修正，或等待权威映射把截图字段暴露为明确命名宏；禁止literal bit。
5. 非安全路径仍执行ADR-0018最小权限边界，不等于开放OTP、Debug、raw eHSM或Host任意执行。

OPEN-DESIGN-012的产品矩阵已关闭；OPEN-CONFLICT-010保持`BLOCKED_BY_RTL_SYNC`，只阻断寄存器绑定和真实EMU Expected，不重开产品字段与极性。

## Review history

- 2026-07-29：负责人明确“按照截图为准”；接受`boot_pin.secure_boot[3]`、default 0、`0=非安全/1=安全`为唯一产品合同。当前生成头与之冲突，OPEN-CONFLICT-010重开为`BLOCKED_BY_RTL_SYNC`，禁止直接使用当前bit0或当前bit3。
- 2026-07-24：当时裁决以SRC-0022为准并关闭OPEN-CONFLICT-010/OPEN-DESIGN-012；该位号/极性和冲突状态已被2026-07-29“以截图为准”的裁决替代，USER强制安全与SoC LCS独立读取继续有效。
- 2026-07-24：负责人提供Checklist/Strap截图；CE-SEC-012与SRC-0022比对发现bit3/bit0差异，建立本冲突。未修改代码仓或执行Git/构建/测试。
