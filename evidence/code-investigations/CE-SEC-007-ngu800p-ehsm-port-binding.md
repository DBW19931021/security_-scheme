# CE-SEC-007：NGU800P eHSM Port绑定与平台原语

## Evidence metadata

- Evidence ID：CE-SEC-007
- 日期：2026-07-23
- 父任务：INV-SEC-007
- 资料基线：SRC-0016、SRC-0017、SRC-0018、SRC-0022；NGU800P公司代码只读工作区
- 适用范围：NGU800P D0 C908侧eHSM Mailbox/status/cache/timer/IRQ集成
- 操作：只读复核内部方案PDF、Vendor Host源码、NGU800P地址/寄存器/中断头文件、C908 CSI平台实现和构建元数据；未修改代码仓，未构建/测试，未执行Git命令

## FACT-00：SoC地址/寄存器权威源已由项目负责人确认

- `CONFIRMED_AUTHORITY`：项目负责人于2026-07-23确认，`../baremetal`中的SoC map和寄存器地址准确且已同步RTL，后续地址问题以该代码为准；ADR-0009将此范围收敛到SRC-0022。
- `DOCUMENTED`：SRC-0022目录头文件自动生成日期为2026-07-15、寄存器版本`0.85r_0708`；51个文件、2162612 bytes，本次目录manifest SHA-256为`2969fdcf644b381f53c6279f8886f9f541e6c70bafb98997b35e3f582423daa6`。
- `CODE_FACT`：`gsp-pmp-rmp-omp`同名关键头的生成日期为2026-07-10；本次抽查的2 MiB RAM、Security Mailbox和IRQ相关值与SRC-0022一致，但后续数值裁决不再以该镜像为依据。

边界：本事实确认资料权威和数值绑定，不把`baremetal` test/stub/demo提升为规范，也不把尚未说明的接口Owner、wrapper关系和行为标成已确认硬件行为。

## FACT-01：当前`SECURITY_SUBSYS_MAILBOX`是4 KiB通用Mailbox

- `DOCUMENTED`：SRC-0022 `config_bus_address_mapping.h:629-638`定义local/remap视图`0x1000_0841_0000`、大小`0x1000`；`subsys_address_mapping.h:992-1000`定义NoC/system视图`0x1010_0841_0000`、大小`0x1000`。
- `DOCUMENTED`：SRC-0022 `regs/mailbox_security.h:1-15`标记生成日期为2026-07-15、寄存器版本`0.85r_0708`，并把`MAILBOX_SECURITY_BASE`绑定到`0x1010_0841_0000`。
- `CODE_FACT`：同一头文件从`MESSAGE0`到`MESSAGE83`定义84组通用message寄存器；每组按`message/row/mask/interrupt_status`排列、stride为`0x10`，例如`MESSAGE0`位于`0x0～0xC`，`MESSAGE83`位于`0x530～0x53C`。

结论：该窗口在当前代码中是84-message通用Security Subsystem Mailbox，不能仅凭名称把它当成Vendor eHSM Mailbox。

## FACT-02：Vendor eHSM Mailbox需要16 × 4 KiB寄存器孔径

- `VENDOR_IMPLEMENTATION`：SRC-0018 Host `ehsm_host-2.3.1-4019-2ee044d/src/mailbox.c:6-39`定义channel stride `0x1000`，并通过`base + 0x1000 * channel`寻址。
- `VENDOR_IMPLEMENTATION`：同一结构体中，单channel寄存器布局为：
  - `s2h_info[2]`：`0x000/0x004`
  - `h2s_info[2]`：`0x080/0x084`
  - `s2h_note/h2s_note`：`0x100/0x104`
  - `s2h_*_int*`：`0x110～0x11C`
  - `h2s_*_int*`：`0x120～0x12C`
- `CONFIRMED`：CE-SEC-006已确认当前项目使用16个channel。

因此，直接复用且不修改Vendor `mailbox.c`时，Host侧可访问孔径必须至少为`16 * 0x1000 = 0x10000`字节。ADR-0010已批准该Vendor direct合同；SRC-0022当前4 KiB Mailbox是独立通用块，不作为协议转换wrapper。

## FACT-03：中断拓扑也区分eHSM Mailbox与通用Mailbox

- `CODE_FACT`：`components/chip_riscv_c908_common/include/ngu800p/ngu800p_ints.h:1492-1616`连续定义`NGU800P_EHSM_O_MAILBOX_IRQ1`～`IRQ16`。
- `CODE_FACT`：同一文件从`:1624`开始另行定义`NGU800P_MAILBOX_MAILBOX_INTR*`通用Mailbox中断。

`INFERENCE`：独立的中断命名说明eHSM事件与通用Mailbox事件在中断拓扑中分别呈现，并与ADR-0010“direct eHSM Mailbox和4 KiB通用Mailbox分开”的目标一致；准确IRQ/channel绑定仍须平台头和EMU Evidence确认。

## FACT-04：SRC-0022尚未出现Vendor direct aperture/status数值绑定

- 在`components/chip_riscv_c908_common/include/ngu800p`范围检索`EHSM`、`HSM_STATUS`、`HSM_ERR`和`O_HSM`，只找到16路eHSM Mailbox中断、eHSM时钟/复位控制和UART pinmux；没有eHSM Mailbox base、Host可见`REG_HSM_STATUS_0`或eHSM error输出的地址/offset定义。
- SRC-0017相关页只描述C908通过Mailbox command/address和SoC memory与eHSM交互；SRC-0016相关页描述ready/timeout/fail-close和RAS/reset边界，但两份内部方案均未给出eHSM专用MMIO数值绑定。

结论：不能把`0x1000_0841_0000`或`0x1010_0841_0000`作为Vendor direct `EHSM_PORT_MAILBOX_REG_BASE_ADDR`。ADR-0010已经关闭transport方向，但当前SRC-0022尚未给出批准direct aperture和Host status/error的准确宏；这在真实MMIO编码前作为RTL/地址头集成同步门禁。

## FACT-05：C908已有可复用的cache/barrier原语

- `CODE_FACT`：`components/chip_riscv_c908_common/src/sys/weak.c:27-47`提供`csi_dcache_clean_invalid_range()`和`csi_dcache_invalid_range()`的SoC wrapper。
- `CODE_FACT`：`components/csi/csi2/include/core/core_rv64.h:235-236,335-338`把range操作对齐边界和cache line定义为64字节。
- `CODE_FACT`：同一文件`:875-1007`的invalidate/clean/clean+invalidate range实现会按cache line扩展范围，并在操作前后执行`__DSB()`；`csi_rv64_gcc.h:1137-1156`将`__DSB()`实现为`fence iorw, iorw`，将`__DMB()`实现为`fence rw, rw`。

目标约束：

1. 当前C908软件基线采用64字节cache line；eHSM context slot、packet/cmd/rsp及可能被单独invalidate的output必须按64字节边界隔离。
2. 维护函数会扩展到完整cache line；禁止与同时被CPU使用的普通对象共享首尾cache line。
3. send前对packet/cmd/input执行精确范围clean或clean+invalidate并完成barrier；发布note前再次保证MMIO顺序。
4. 后续CE-SEC-008确认Vendor poll在观察完成后直接读取context内部rsp，没有响应前第二个port cache hook；因此内部rsp可见性依赖send前invalidate、在途no-touch和MMIO读取顺序。Vendor成功返回后，项目adapter只对外部output执行invalidate+read fence；未闭环的late response不得复用或清零slot。

`GAP`：2 MiB安全RAM在BootROM/FMC/GSP各阶段最终使用cached还是non-cacheable属性、PMA/PBMT/MMU配置和Firewall属性仍没有硬件/启动配置证据。

## FACT-06：C908已有64位微秒时间源和IRQ原语

- `CODE_FACT`：`components/chip_riscv_c908_common/src/sys/tick.c:230-239`提供64位`csi_tick_get_us()`；`components/csi/csi2/include/drv/tick.h:62-69`公开该接口。
- `CODE_FACT`：`components/chip_riscv_c908_common/src/arch/c908vk-cp-xt_v2/system.c:421-427`在`SystemInit()`中初始化cache、interrupt、时钟和tick。
- `CODE_FACT`：`components/chip_riscv_c908_common/src/sys/irq_port.c:22-45`提供IRQ enable/disable/enabled/priority wrapper。

目标约束：port timeout使用单调的64位微秒deadline和无符号差值比较，不使用Vendor OSR的恒false timeout，也不把32位毫秒wrap细节扩散到业务层。BootROM仍固定poll；IRQ原语只为后续FMC/GSP模式保留。

## FACT-07：Makefile主构建链包含common平台层，CDK工程元数据已过时

- `CODE_FACT`：BootROM/FMC/GSP Makefile分别选择`chip_riscv_bootrom/fmc/gsp`；三个组件的`sub.mk:46`均包含`components/chip_riscv_c908_common/sub.mk`。
- `CODE_FACT`：`chip_riscv_c908_common/sub.mk:19-29`把`src/sys/*.c`、C908 arch和必要driver加入主构建，因此主Makefile路径可获得上述cache/timer/IRQ实现。
- `CODE_FACT`：三个`solutions/*/cdk/.../project.cdkproj`仍引用不存在的`components/chip_riscv_dummy`路径。

结论：主Makefile平台原语可作为详设输入；CDK工程不能据此宣称可构建，编码阶段必须重新生成或修正CDK工程，并对Makefile/CDK实际source list做一致性检查。

## 端口合同收敛

当前可以冻结：

- `channel_count = 16`
- Vendor direct Host布局的`channel_stride = 0x1000`
- `mailbox_aperture_size >= 0x10000`且MMIO non-cacheable
- Vendor公共源码保持不变；NGU800P只实现custom/port/build/adapter
- SRC-0022 4 KiB通用Mailbox数值为local/remap `0x1000_0841_0000`、NoC/system `0x1010_0841_0000`，不用于eHSM
- `cache_line_size = 64`
- BootROM mode为`WAIT_AND_POLL`
- FMC mode为`WAIT_AND_POLL`
- GSP首版从entry到Vendor FW ready后均使用`WAIT_AND_POLL`；interrupt仅为ready后的后续独立优化
- timeout clock使用64位单调微秒
- 共享地址使用64位remote address，并通过显式address-domain转换
- security只上报故障并阻断release，不直接reset
- RAS未ready时保存静态错误记录、有限重试上报，超时后关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出

本轮调查时不能冻结、后续状态如下：

- Vendor direct aperture的SRC-0022准确base宏
- Host可见status/error寄存器的base/offset
- 2 MiB安全RAM最终cache/PMA/PBMT属性
- command-specific deadline数值仍待输入；ADR-0011已冻结首版自动retry为0、timeout acceptance unknown和late-response quarantine
- 每stage service channel仍待输入；ADR-0011已冻结每stage一个256字节context slot和最多一条在途

## 冲突与停止条件

- `OPEN-CONFLICT-007`已由ADR-0010按Vendor direct Option A关闭；4 KiB通用Mailbox不用于eHSM。
- 在SRC-0022同步direct aperture/status/error准确宏前，停止最终base常量和真实MMIO实现；不重开wrapper方向，不修改Vendor公共代码。
- 不受阻的port API、地址域检查、cache/timer wrapper、fake-MMIO单元测试设计和构建禁入规则可以继续详设。

## 设计回填

- 更新`docs/04-interfaces/ehsm-mailbox.md`。
- 更新`docs/05-software-design/bootrom.md`。
- 更新`requirements/open-questions.yaml`、B0-R2任务状态和项目状态。

## 2026-07-23负责人裁决回填

- 批准Vendor direct Option A；所有eHSM交互首先follow Vendor实现，Vendor公共代码原则上不可修改。
- BootROM/FMC/GSP首版全程固定poll；Vendor FW ready后的interrupt仅为后续独立优化，不能由首版业务层切换。
- RAS尚未ready时采用静态错误记录、有限上报等待、关闭普通中断和无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出。
- 以上形成ADR-0010；CE中的事实观察保持不变，目标设计按ADR-0010及后续ADR-0011收敛。
