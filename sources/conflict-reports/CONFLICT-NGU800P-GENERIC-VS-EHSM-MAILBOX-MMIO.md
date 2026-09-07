# NGU800P通用Mailbox与Vendor eHSM Mailbox绑定冲突

## Identity

- Conflict ID: CONFLICT-NGU800P-GENERIC-VS-EHSM-MAILBOX-MMIO
- Open Question: OPEN-CONFLICT-007
- Status: resolved_direction / integration_binding_pending
- Evidence state: CONFIRMED
- Owner: 项目负责人已裁决方向；SoC RTL/地址头/eHSM集成Owner补齐数值绑定；GSP消费
- Resolution date: 2026-07-23
- Decision required by: 无；真实MMIO实现前仍须补齐direct aperture准确base

## Conflict classification

- Type: hardware_binding_semantics_ambiguity / incompatible_register_layout
- Affected scope: BootROM/FMC/GSP eHSM port、ready/status读取、FMC/GSP中断模式、EMU真实Mailbox测试。
- Safe-to-continue scope: Vendor direct wire/stride/channel合同、adapter/port API、64位地址转换合同、cache/timer wrapper、poll/interrupt状态机和fake-MMIO测试设计。
- Must-stop scope: direct aperture准确base、status/error地址未同步到baremetal权威头之前的真实MMIO编码及基于该地址的EMU Expected。

## Source A：SRC-0022权威NGU800P生成头

- 项目负责人确认`baremetal`中的SoC map/寄存器地址准确且已同步RTL；ADR-0009规定SRC-0022是SoC地址/寄存器/IRQ第一权威源。
- `config_bus_address_mapping.h:629-638`：local/remap `0x1000_0841_0000`，大小4 KiB。
- `subsys_address_mapping.h:992-1000`：NoC/system `0x1010_0841_0000`，大小4 KiB。
- `regs/mailbox_security.h`：生成日期2026-07-15、寄存器版本`0.85r_0708`；84组message寄存器，每组stride `0x10`，布局为message/row/mask/interrupt-status。

## Source B：当前Vendor eHSM Mailbox合同

- SRC-0018 Host `src/mailbox.c:6-39`：每channel stride `0x1000`。
- 当前项目已确认eHSM内部/服务侧使用16个channel；**只有C908直接使用Vendor `mailbox.c`布局时**，Host可访问孔径才至少为64 KiB。
- 单channel使用`info/note/soc_int/hsm_int`布局，不是84-message通用Mailbox布局。

## Source C：NGU800P中断与缺失语义

- `ngu800p_ints.h:1492-1616`独立枚举16路eHSM Mailbox IRQ；随后另行枚举通用Mailbox IRQ。
- SRC-0022没有另一个eHSM专用Mailbox base，也没有Host可见status/error地址；这与“当前地址表不完整”不同，因为项目负责人已经确认SRC-0022是当前准确的RTL同步地址源。
- SRC-0016/SRC-0017描述交互和失败行为，但未提供数值MMIO绑定。

项目负责人已裁决：现有4 KiB块不承担把Vendor direct合同转换成另一套transport的职责；eHSM交互保持Vendor实现方式。当前问题仅为direct aperture数值尚未在已检索的SRC-0022头中定位。

## 风险

若把`SECURITY_SUBSYS_MAILBOX_BASE`直接作为Vendor direct `EHSM_PORT_MAILBOX_REG_BASE_ADDR`：

1. channel 1从`base + 0x1000`开始即越过整个4 KiB通用窗口。
2. channel 0的`h2s_info/note/int`offset会访问通用Mailbox中不同message记录，语义完全错误。
3. ready/status读取仍无地址来源，可能把未相关寄存器值解释为eHSM安全状态。
4. 最坏结果是错误放行安全启动、破坏其他Mailbox用户或产生不可诊断的总线错误。

## 选项与裁决

### Option A：保持Vendor direct Mailbox合同（已批准）

- Vendor `src/mailbox.c`及公共API不修改；继续使用16 channel、每channel `0x1000` stride和`info/note/interrupt`布局。
- NGU800P只通过Vendor支持的custom header、port函数、外部build集成和项目adapter完成移植。
- 4 KiB/84-message `SECURITY_SUBSYS_MAILBOX`与eHSM direct aperture分开，不用于协议转换。
- direct aperture准确base、status/error和IRQ绑定必须进入SRC-0022 baremetal权威生成头或其后续同步版本。

### Option B：4 KiB wrapper transport（已否决）

- 会要求绕过或替换Vendor公共`mailbox.c`的direct MMIO布局，与“Vendor代码原则上不可修改、eHSM交互follow Vendor实现”冲突。

### Option C：修改/fork Vendor公共代码适配其他transport（已否决）

- 降低后续Vendor升级可追溯性并形成项目私有协议分支，不符合当前集成边界。

## 裁决后的实现输入

不再需要项目负责人选择wrapper或direct。SoC RTL/地址头/eHSM集成同步只需提供事实绑定：

1. C908可访问Vendor direct aperture的local/remap与NoC/system base，size至少覆盖`16 * 0x1000`。
2. `EHSM_PORT_MAILBOX_REG_BASE_ADDR`应引用的SRC-0022宏名和MMIO属性。
3. Host可见`REG_HSM_STATUS_0`及其他error/status输出的base、offset、宽度、有效时点和清除语义。
4. 16路eHSM IRQ与channel的逐项映射、触发类型和ack/clear顺序。
5. 若当前RTL/生成头确实没有该direct aperture，则按已批准合同登记RTL集成gap并更新SRC-0022，不得改Vendor公共代码或复用4 KiB通用Mailbox。

## 临时约束

- 不允许把`SECURITY_SUBSYS_MAILBOX_BASE`或`MAILBOX_SECURITY_BASE`交给Vendor direct `mailbox.c`。
- Vendor direct aperture的权威宏缺失时production/EMU构建必须失败，不得回退到OSR `0x80000000`、猜测地址、wrapper、stub或simulated success。
- Vendor交付目录保持只读；若port API不足，先报告不兼容，不在Vendor公共源码中打补丁。
- BootROM继续按poll设计；这不消除status和Mailbox MMIO绑定缺口。

## Review history

- 2026-07-23：B0-R2只读核对Vendor Host、NGU800P地址/寄存器/中断头和内部方案时发现，建立OPEN-CONFLICT-007。
- 2026-07-23：项目负责人确认SoC地址/寄存器以RTL同步baremetal代码为准；接受ADR-0009/SRC-0022。撤回“默认新增64 KiB SoC孔径”建议，冲突一度收敛为4 KiB块的eHSM wrapper语义；该中间结论随后由ADR-0010的Vendor direct裁决取代。
- 2026-07-23：项目负责人批准Option A并进一步明确eHSM交互follow Vendor实现、Vendor代码原则上不可修改。关闭wrapper/direct方向冲突：采用Vendor direct 16×4 KiB合同，4 KiB通用Mailbox不用于eHSM；准确base转为RTL/地址头集成同步门禁。见ADR-0010。
