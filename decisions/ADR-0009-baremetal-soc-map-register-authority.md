# ADR-0009：NGU800P SoC地址与寄存器以baremetal RTL同步头为权威源

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0022
- 相关证据：CE-SEC-007
- 相关冲突：OPEN-CONFLICT-006、OPEN-CONFLICT-007
- 补充关系：补充ADR-0001/0002的SoC资料权威规则

## 背景

此前B0-R2从`gsp-pmp-rmp-omp`实现工作区核对NGU800P地址/寄存器头，发现4 KiB通用Mailbox与Vendor direct Host的16×4 KiB布局不一致，并因缺少明确SoC地址权威源而提出另行补充eHSM地址表。

项目负责人现确认：SoC map和各寄存器地址在`baremetal`代码中准确维护，且已经同步到RTL；后续地址相关问题以`baremetal`代码为准。该确认需要与“baremetal历史test/stub/demo不定义产品目标”的既有规则同时成立。

## 决策

> 2026-07-29窄范围例外：项目负责人指定SRC-0023截图为`secure_boot`字段与极性的产品权威，即`boot_pin.secure_boot[3]`、default 0、`0=非安全/1=安全`。当前SRC-0022生成头的bit0 `SEC_BOOT`/bit3 `DIE_ID`在本字段上作为冲突实现证据，不得覆盖产品合同；真实BootROM寄存器绑定等待RTL/生成头修正或批准映射。SRC-0022的其他SoC地址、寄存器和IRQ权威性不变。

1. NGU800P SoC地址空间、寄存器base/offset/bitfield、IRQ编号/名称及其他由RTL生成并同步的软件硬件常量，以SRC-0022当前有效版本为第一权威源。
2. `gsp-pmp-rmp-omp`内的同名头是实现侧镜像，可能因同步时间不同而滞后；它们用于代码事实和兼容性检查，不用于覆盖SRC-0022或裁决地址差异。
3. SRC-0017/SRC-0016继续定义系统架构和软件工程目标；SRC-0022负责当前RTL的SoC数值绑定。Vendor只定义eHSM/Core内禀行为，不得以Vendor示例地址覆盖SRC-0022。
4. 地址数值、寄存器字段存在于生成头中时，不再把“请再次提供地址”作为普通open question。若目标设计与SRC-0022不一致，必须登记冲突并由项目负责人决定修改方案、RTL还是软件。
5. 地址值权威不自动决定接口语义。寄存器块的Owner、访问master、wrapper/直连关系、状态机、清除时序、cache属性、RAS动作和产品使用方式仍必须由匹配的集成说明、代码路径、方案或裁决确定。
6. SRC-0022中的数值事实标为`DOCUMENTED`；项目负责人确认的是资料权威与RTL同步关系。具体读写行为只有取得EMU/FPGA/硅上Evidence后才标为`CONFIRMED`。
7. 本裁决不扩大到`baremetal`中的test、stub、demo、synthetic数据和历史Expected；这些仍只作为`CODE_FACT`和差距证据，不能反向定义目标设计、最终oracle或发布条件。
8. 引用SRC-0022时至少记录文件、宏/寄存器名、生成日期/版本；冻结关键ABI或Expected时记录对应文件哈希。新RTL大版本或不兼容生成头使用新Source ID，普通living-reference更新则更新SRC-0022卡片并执行影响分析。

## 对现有冲突的影响

### OPEN-CONFLICT-006

- 产品软件权威数值：`MANAGEMENT_NOC_S9_SRAM_BASE=0x1010_0500_0000`、容量2 MiB；`0x1000_0500_0000`仅保留为RTL历史事实并在产品输入中拒绝。
- 不再寻找另一份map来回答数值。
- 负责人此前已经批准该2 MiB承载全部安全子系统软件；因此硬件宏名`MANAGEMENT_SUBSYS_SRAM`不重开项目用途。ADR-0016已关闭GSP/OMP关系，C908 PC/linker固定使用System Address；仍需冻结PMA、精确Region容量和Firewall参数。

### OPEN-CONFLICT-007

- 已有权威数值：`SECURITY_SUBSYS_MAILBOX` local/remap `0x1000_0841_0000`、NoC/system `0x1010_0841_0000`、大小4 KiB，寄存器为84-message布局。
- ADR-0010后续裁决采用Vendor direct `16 × 0x1000`合同，4 KiB通用Mailbox不作为eHSM wrapper；方向冲突关闭。
- SRC-0022当前检索范围尚未出现direct aperture宏。该准确base作为RTL/地址头集成同步门禁，不允许使用4 KiB通用Mailbox或OSR样例地址代替。

## 影响

- 后续地址调查先检索SRC-0022，不再优先使用GSP镜像或Vendor地址。
- 方案文档引用共享安全RAM地址时必须写baremetal System Address宏名，避免只复制裸数值或引入Local/System domain字段。
- GSP/baremetal实现若与SRC-0022不同，建立同步差距，不静默选择其中一个。
- 不修改`baremetal`或`gsp-pmp-rmp-omp`；本次只更新`security_-scheme`治理、设计和证据。

## Review history

- 2026-07-29：增加`secure_boot`窄范围例外；当前生成头与SRC-0023产品合同的同步缺口由OPEN-CONFLICT-010管理。
- 2026-07-23：项目负责人确认`baremetal` SoC map/寄存器地址准确且已同步RTL；接受本ADR。
