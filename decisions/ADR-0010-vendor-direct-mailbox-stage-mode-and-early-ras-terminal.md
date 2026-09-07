# ADR-0010：Vendor direct Mailbox、分阶段驱动模式与早期RAS失败终态

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018、SRC-0022
- 相关证据：CE-SEC-006、CE-SEC-007
- 相关问题：OPEN-CONFLICT-007、OPEN-DESIGN-001、OPEN-DESIGN-003
- 补充关系：补充ADR-0006和ADR-0009

## 背景

Vendor Host公共实现`src/mailbox.c`直接按`EHSM_PORT_MAILBOX_REG_BASE_ADDR + channel * 0x1000`访问16个channel的`info/note/interrupt`寄存器，没有可在不改公共代码的情况下替换为84-message协议wrapper的抽象层。Vendor提供的移植边界是`ehsm_host_custom.h`配置和`ehsm_host_port.h`平台函数。

SRC-0022同时记录了NGU800P的4 KiB、84-message `SECURITY_SUBSYS_MAILBOX`。此前OPEN-CONFLICT-007把“使用SoC wrapper”与“保持Vendor direct布局”作为方向选项。项目负责人明确：所有eHSM交互首先follow Vendor实现方式，Vendor代码原则上不可修改；因此该方向不应继续作为普通方案裁决。

项目负责人同时冻结了BootROM/FMC/GSP使用poll或interrupt的阶段边界，并批准RAS尚未ready时采用项目推荐的早期fail-close终态。

## 决策一：Vendor代码与Mailbox合同

1. 接受OPEN-CONFLICT-007 Option A：eHSM Host侧采用Vendor direct Mailbox合同，保留16个channel、每channel `0x1000` stride、`info/note/interrupt`寄存器布局和Vendor公共调用语义。
2. `SECURITY_SUBSYS_MAILBOX`是NGU800P通用4 KiB/84-message Mailbox，不用于把Vendor direct合同转换为另一套transport；OPEN-CONFLICT-007方案方向关闭。
3. SRC-0018 Vendor交付目录按只读第三方基线管理，不修改、打补丁或在目录内维护NGU800P fork。NGU800P适配通过以下受支持边界完成：
   - 项目侧`ehsm_host_custom.h`配置；
   - 项目侧`ehsm_host_port.h`函数实现；
   - 外部build/include/source选择；
   - Vendor API外的项目adapter、错误映射和业务编排。
4. Vendor OSR样例中的绝对地址`0x80000000`、共享内存`0x60000000`、OSR reset magic和恒false timeout仍只是样例平台值，不能因“Vendor代码不改”而直接成为NGU800P SoC常量。
5. NGU800P的`EHSM_PORT_MAILBOX_REG_BASE_ADDR`必须绑定到能完整覆盖Vendor 16×4 KiB布局的C908可访问direct aperture，MMIO属性为non-cacheable。准确base必须来自SRC-0022所代表的RTL同步baremetal权威源或其后续同步版本。
6. 本次只读检索尚未在SRC-0022找到该direct aperture对应宏；这是RTL/地址头/软件集成同步缺口，不是继续选择wrapper或修改Vendor代码的方案问题。绑定补齐前禁止真实MMIO编码、禁止使用4 KiB通用Mailbox代替，也禁止回退到OSR `0x80000000`。
7. 若后续证明当前RTL无法提供Vendor direct aperture，必须作为“RTL与已批准Vendor集成合同不一致”重新升级冲突；不得先改Vendor公共代码绕过。

## 决策二：分阶段驱动模式

1. BootROM固定使用`EHSM_DRV_MODE_WAIT_AND_POLL`。
2. FMC固定使用`EHSM_DRV_MODE_WAIT_AND_POLL`。
3. GSP从entry开始，到Vendor eHSM FW加载、验证、启动并确认ready之前，固定使用`EHSM_DRV_MODE_WAIT_AND_POLL`。
4. 当前首版基线中，Vendor FW ready之后GSP仍继续poll；首版不把interrupt作为实现或发布条件。
5. Vendor FW ready后的interrupt只作为后续可选优化。未来若通过独立设计变更启用，至少满足：
   - Vendor FW的`firmware_done=1 && firmware_err=0`，并保留raw status；
   - GSP中断控制器、16路channel映射、ISR/callback、锁和task唤醒路径已经初始化；
   - 切换前没有未闭环poll transaction、late response或被复用的context；
   - 按Vendor顺序清理pending note/interrupt并使能；
   - 初始化失败时只有在能证明Vendor状态、channel和context仍一致时才允许保持poll，否则fail-close。
6. 不允许在BootROM、FMC或Vendor FW ready前因性能原因提前启用中断，也不允许业务层自行混用poll和interrupt访问同一channel。

## 决策三：RAS尚未ready时的失败终态

BootROM/FMC及GSP早期初始化阶段发生阻断性安全失败，而RAS上报端尚未ready时，执行以下固定顺序：

1. 立即锁死当前release/jump路径，保证下一级不可达。
2. 保存最小`EARLY_SECURITY_ERROR_RECORD`：magic/version、valid、boot instance、stage、operation、domain、mapped/raw code、关键raw status、timestamp、zeroization状态和requested RAS action class；不得保存key、明文或敏感Mailbox payload。
3. 该记录使用当前stage安全RAM中的固定静态slot，不新增Handoff ABI或独立永久Region；先写payload，执行write/release fence后最后提交`valid`。`NON_CACHEABLE`与`HARDWARE_COHERENT`两个批准候选属性下均不执行data clean/invalidate。
4. 撤销临时Host/loader权限，尽力清零当前stage拥有的敏感临时buffer；清零失败写入记录并提升为FATAL。
5. 在批准的有限deadline内轮询RAS ready并重试“上报错误记录”动作；只重试上报，不重试原安全操作，不调用Vendor reset。
6. RAS接受后仍保持fail-close，等待RAS决定reset/watchdog/隔离/不复位。
7. deadline到期仍未被RAS接受时，记录`RAS_REPORT_UNAVAILABLE`，完成fence、关闭普通中断后进入无限`WFI` fail-stop循环；循环只允许平台批准的Reset/NMI退出，不处理普通IRQ，不继续启动、自动reset、喂狗绕过或反复写不可逆资源。
8. 当前记录保证在本次未推进启动链的stage内可消费；跨reset retention只有在后续硬件资料证明存在时才声明，不假设普通SRAM天然跨reset保留。

RAS ready寄存器/通知通道、deadline数值和slot准确offset仍是实现参数；无限WFI终态已经冻结，不再开放halt原语选择，也不再把“等待还是继续启动/自行reset”作为开放策略。

## 影响

- OPEN-CONFLICT-007按目标方向关闭；只保留direct aperture准确base和status/error地址的集成同步门禁。
- FMC不再开放poll/interrupt选择；GSP首版全程poll，Vendor FW ready后的中断只作为后续可选优化，不再阻断当前详设。
- OPEN-DESIGN-003中“RAS未ready时的早期终态”部分关闭；其他stage的最终RAS action mapping、受限服务/OOB恢复等产品策略仍可继续细化。
- Vendor代码更新继续按交付快照和release note管理；NGU800P平台代码不得混入Vendor源目录。
- 本次不授权修改`baremetal`、`gsp-pmp-rmp-omp`或Vendor源码。

## Review history

- 2026-07-23：项目负责人批准OPEN-CONFLICT-007 Option A；确认eHSM交互follow Vendor实现且Vendor代码原则上不可修改。
- 2026-07-23：冻结BootROM/FMC poll；GSP首版全程poll，Vendor FW ready后的interrupt只作为后续可选优化。
- 2026-07-23：批准RAS尚未ready时采用项目推荐的持久错误记录、有限上报等待和fail-stop终态。
