# NGU800P non_sec_boot eFuse强制非安全启动输入

- Source ID：SRC-0034
- 日期：2026-08-25
- Owner：项目负责人
- 状态：用户确认的设计输入

## 用户确认内容

1. NGU800P在eFuse中新增1 bit启动策略字段`non_sec_boot`。
2. `non_sec_boot`默认逻辑值为0；值0不触发强制覆盖，BootROM继续执行既有启动模式判定。
3. `non_sec_boot`烧写为逻辑值1后，BootROM必须强制选择非安全启动。
4. 该判断由BootROM在启动早期执行。

## 设计边界

- “强制非安全启动”复用产品现有的受限非安全启动路径，不新增可访问OTP、生产密钥、raw eHSM、受保护Debug或安全RAM的非受限旁路。
- `non_sec_boot`是OTP/eFuse单向策略位；产品软件不得提供运行期清零、回退为0或Host覆盖接口。
- BootROM只消费复位稳定、只读、带有效性/ECC状态的逻辑快照，不获得任意offset的raw eFuse读写接口。
- 字段的物理word/bit/offset、blank/programmed编码、ECC/valid/mirror、复位锁存时序、只读寄存器/API名称、烧写/锁定Owner和Lifecycle权限尚未提供，必须由目标D0 RTL/eFuse/制造资料绑定，不得从截图推断。
- 读失败、ECC异常、镜像不一致或来源无效不能解释为逻辑1，也不能静默当作逻辑0；产品BootROM必须阻断两条FMC release路径并进入启动策略输入错误终态。

## 截图说明

用户附图只确认字段名称`non_sec_boot`、1 bit和“非安全启动标志位”语义。图中未提供可作为产品实现依据的物理地址、bit编号、编码、锁定或访问路径。
