---
title: "eHSM Mailbox 接口"
status: review_ready_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0005
  - SRC-0012
  - SRC-0014
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
owners:
  - GSP
last_reviewed: 2026-07-28
supersedes: []
superseded_by: []
---

# Purpose

定义 NGU800P BootROM/FMC/GSP 使用 eHSM 的软件适配边界，包括启动状态、Vendor Mailbox机制、共享缓冲区、同步/异步、timeout/busy、RAS错误上报和原始状态保存。ADR-0006已确认Mailbox协议follow Vendor、security软件不自行reset；ADR-0011已冻结首版单在途、Vendor兼容cache/timeout scope、零自动retry和迟到响应quarantine。eHSM内部硬件实现先按SRC-0035实际elaboration查询，接口意图对照Vendor手册，软件行为对照锁定版本FSP；已明确事实直接作为`VENDOR_IMPLEMENTATION/DOCUMENTED`基线，只有elaboration缺口、NGU800P端口绑定、项目硬件参数、来源冲突或动态验证保持待冻结。

# Scope

覆盖 Host 侧 port/adapter 与 eHSM/Core command 接口；不定义 eHSM 内部实现，也不把 Vendor 的 OSR FPGA 地址、magic 或 timer 行为当成 SoC 事实。

# Confirmed facts

- 公司 `components/security` 当前只暴露 `ehsm_verify_decrypt_stub()`，BootROM 构建没有生产 eHSM port/transport。
- `verify_image()` 直接调用 stub；当前 status enum 没有 timeout/busy/ready/reset/self-test/LCS/transport 错误。
- BootROM 当前没有 Vendor Host driver/context/Mailbox API 调用。
- ADR-0008已确认eHSM作为受信任master可以访问整个2 MiB安全RAM，不设置Region级Firewall权限限制；Host及其他master的隔离不变。
- SRC-0022是RTL同步SoC数值权威源：`SECURITY_SUBSYS_MAILBOX`的local/remap base为`0x1000_0841_0000`、NoC/system base为`0x1010_0841_0000`、大小4 KiB，寄存器是84-message布局。ADR-0010已裁决该块不用于eHSM wrapper；eHSM采用Vendor direct Host的16 channel、每channel stride `0x1000`布局，Vendor公共代码保持不变。direct aperture准确base尚未在当前SRC-0022定位，属于集成同步门禁。
- C908 common当前提供64字节cache line的range维护/barrier、64位微秒timer和IRQ wrapper；Makefile主构建会纳入这些实现，但三个CDK工程仍引用不存在的`chip_riscv_dummy`，编码前必须重新生成或修正。
- CE-SEC-010进一步确认`NGU800P_EHSM_O_MAILBOX_IRQ1～16`编码为APLIC 9、source 78～93（encoded `0x0009_004E～0x0009_005D`）。当前没有权威资料证明它们与Vendor channel 0～15的逐项映射；首版全程poll不受该项阻断。
- CE-SEC-010确认当前SRC-0022仍没有Vendor direct aperture和Host status/error的base/offset/bitfield，不得把4 KiB通用Mailbox或OSR样例地址作为fallback。

详细路径和行号见 [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)、[CE-SEC-007](../../evidence/code-investigations/CE-SEC-007-ngu800p-ehsm-port-binding.md)和[CE-SEC-008](../../evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md)。

# Documented facts

- SRC-0016 §2.1～§2.3 要求软件落实 eHSM 启动、自检和安全启动交互。
- 软件最终采用的命令、参数、时序和异常规则必须写入有效软件方案或受控 amendment。
- Vendor 仅定义 eHSM/Core 能力，SoC port 参数和系统失败策略由 NGU800P 方案决定。
- ADR-0006规定NGU800P Mailbox command/response/packet/note语义follow Vendor；reset动作由RAS策略决定，安全软件只上报错误并阻断release。
- SRC-0012 §9.1和SRC-0005 §12.2～§12.3记录eHSM Mailbox最多支持16个channel；当前4019 Host port配置为16，NGU800P集成头文件也枚举16个eHSM Mailbox IRQ，因此本项目当前基线为16个channel。
- SRC-0005状态表与`SYS_HSM_STA0`寄存器表定义`hw_boot_done/error`、`bootloader_done/error`、`firmware_done/error`和`soc_verify_done/error`；SRC-0012明确`bootloader_done`不能单独证明Mailbox命令可成功，仍必须检查`bootloader_err`。
- SRC-0014规定Host逻辑地址必须通过port转换为eHSM可见物理/remote address；只有统一地址空间的平台才允许identity mapping。SRC-0005定义`SYS_SOC_MEM_BAL/BAH`形成64位SoC总线基址。

# Vendor implementation observations

1. Host `ehsm_driver_init_library()` 调用port和Mailbox init；`ehsm_ctx_init()`绑定channel、command/response buffer地址。
2. Vendor packed command为160字节（`cmd_id/cmd_id_inv/data[39]`），response为20字节（`ret_code/data[4]`），packet由两个64位remote address组成；具体命令payload继续follow对应Vendor API。
3. 每channel寄存器跨度为`0x1000`；send把64位packet地址拆成低/高32位写入info寄存器，note置位前检查busy/timeout并执行cache和critical section。
4. Host支持interrupt、wait-and-poll、send-and-peek；Mailbox send有note-bit busy/timeout路径和critical section。
5. port层需要实现channel/MMIO/shared memory/OTP、System Address范围校验、cache/barrier、interrupt、status/error register、timer和RAS错误上报；Vendor reset函数不进入production普通恢复路径。产品`ehsm_port_addr_to_raddr()`/`ehsm_port_raddr_to_addr()`只校验当前active descriptor并同值返回，不建立Local/System映射；OSR样例的无范围identity cast不得直接复用。
6. OSR m130 port的timer timeout恒为false，reset/status地址是OSR固定值；不能直接用于NGU800P。
7. OSR demo ready循环无timeout，且表达式会在任一`BOOT_DONE`/`HSM_READY` bit置位时退出，弱于Vendor手册描述，不能作为产品目标。产品门禁采用“错误优先”：任一`hw_boot_err`/`bootloader_err`立即fail-close；只有`hw_boot_done=1 && bootloader_done=1`且相关错误位均为0时，BootROM才把eHSM BL视为可接受命令。
8. BL与Host的self-test bit18/bit19差异已裁决：当前交付采用Bootloader定义，Host定义错误；bit18/`0x40000`=`TRNG`，bit19/`0x80000`保持unknown/reserved。无论解释结果如何都必须保留原始bitmap。
9. 公开`ehsm_ctx_st`为224字节，内部内嵌packet、160字节cmd和20字节rsp；流式`ehsm_session_st`为512字节并要求跨init/update/finish保持有效。interrupt callback通过packet地址反推context，因此context生命周期可能跨函数返回。
10. eHSM BL的`mmap_remap_addr_u64()`把64位remote System Address拆为高/低32位，配置`SYS_SOC_MEM_BA`后映射到eHSM内部窗口；remote read/write和Mailbox调度都复用该Vendor内部机制。SoC产品软件不复制、不反向推导该映射，只提交baremetal System Address。
11. Vendor Host Demo和`api.c`提供`EHSM_DRV_MODE_WAIT_AND_POLL`轮询路径；负责人已冻结BootROM使用轮询，不为BootROM设计Vendor未提供的中断路径。
12. Vendor公共poll路径只在send前调用一次无参数`ehsm_port_flush_and_invalidate_cache()`；响应handler随后直接读取context/rsp，没有响应前第二个cache钩子。Vendor timeout钩子也没有command/deadline参数。首版通过单在途、active cache/timeout scope和在途no-touch满足该合同，不修改Vendor公共代码。

## Vendor启动状态位

SRC-0005状态输出表和`SYS_HSM_STA0`寄存器表的对应关系如下。Host侧读取`REG_HSM_STATUS_0`时使用状态输出位；eHSM内部寄存器位仅用于解释Vendor实现，不得让C908绕过port直接依赖eHSM内部地址。

| Host可见状态位 | mask | eHSM内部`SYS_HSM_STA0`位 | 含义 | BootROM处理 |
|---|---:|---:|---|---|
| 0 | `0x01` | 16 | `hw_boot_done` | 与`bootloader_done`共同构成放行条件 |
| 1 | `0x02` | 17 | `hw_boot_err` | 立即fail-close |
| 2 | `0x04` | 18 | `bootloader_done` | 与`hw_boot_done`共同构成放行条件 |
| 3 | `0x08` | 19 | `bootloader_err` | 立即fail-close；done不能覆盖error |
| 4 | `0x10` | 20 | `firmware_done` | 后续Firmware服务阶段使用，不替代BootROM的BL门禁 |
| 5 | `0x20` | 21 | `firmware_err` | 后续阶段fail-close |
| 6 | `0x40` | 22 | `soc_verify_done` | SoC verify状态证据，不作为BL ready替代 |
| 7 | `0x80` | 23 | `soc_verify_err` | fail-close并保存raw状态 |

因此BootROM的BL门禁等价于：done mask `0x05`必须全部置位，error mask `0x0A`必须全部为0。读取时先检查error，再判断done；不得把Vendor Demo中的`(BOOT_DONE | HSM_READY) != 0`当成该合同。

## NGU800P端口绑定检查

| 端口项 | 当前合同 | 状态 |
|---|---|---|
| eHSM服务channel数 | 16 | 已冻结 |
| Vendor direct Host channel stride | `0x1000`，公共`mailbox.c`按`base + channel * 0x1000`寻址 | 已冻结，不修改Vendor公共代码 |
| eHSM Host可访问孔径 | 至少`0x10000`，non-cacheable；准确base必须来自SRC-0022权威宏 | 方向已冻结；数值绑定待同步 |
| SRC-0022通用SoC Mailbox | local/remap `0x1000_0841_0000`、NoC/system `0x1010_0841_0000`、size `0x1000`、84-message；不用于eHSM | 数值及用途边界已冻结 |
| eHSM绑定方式 | Vendor direct Mailbox；NGU800P只实现custom header、port函数、外部build和项目adapter | 已冻结，OPEN-CONFLICT-007关闭 |
| Host可见status/error地址 | 保留完整raw值，准确base/offset/清除语义由RTL寄存器规格提供 | 集成绑定待同步，不重开transport方向 |
| IRQ | 公司头已枚举`EHSM_O_MAILBOX_IRQ1～16`；BootROM不使用，FMC/GSP模式冻结后再绑定 | 部分冻结 |
| remote address | 64位baremetal System Address；C908、共享descriptor和Vendor remote字段数值一致，port只做active descriptor范围校验并同值返回；local/remap输入拒绝 | 已冻结，不受OPEN-CONFLICT-006地址视图阻断 |
| cache line | 当前C908软件基线为64字节；slot和可独立invalidate对象按64字节隔离 | 已冻结到当前CPU/SDK基线 |
| cache/barrier | Vendor send前钩子验证active transaction并执行write/release barrier；在途no-touch；Vendor成功返回后adapter执行acquire/read fence。两个批准候选属性均不执行data clean/invalidate | ADR-0011已冻结；最终PMA待SoC稳定后在`NON_CACHEABLE/HARDWARE_COHERENT`中裁决 |
| timer | 64位单调微秒deadline，复用`csi_tick_get_us()`；单在途active scope向Vendor无参数timeout钩子提供当前operation时限 | 机制已冻结，各operation数值待定 |
| concurrency | 每stage一个256字节、64字节对齐context slot；最多一条在途；GSP唯一service串行化 | ADR-0011已冻结 |
| retry/late response | 首版自动retry为0；timeout视为acceptance unknown并quarantine整个service/slot | ADR-0011已冻结 |

`OPEN-CONFLICT-007`的transport方向已经关闭。CE-SEC-010已冻结16路IRQ的SoC encoded值，但未冻结IRQ与Vendor channel逐项映射。当前只由direct aperture/status/error权威宏缺失阻断首版真实MMIO编码；IRQ/channel表仅阻断未来interrupt change，不阻断首版poll。Vendor兼容adapter/port接口、fake-MMIO测试、64位地址检查、cache/timer wrapper和错误模型可继续详设。

# Assumptions

- 不假定Vendor OSR FPGA的绝对寄存器/Mailbox base、shared-memory地址、OTP大小、CPU frequency或identity地址映射适用于NGU800P；但eHSM/Core内禀协议事实和当前交付的16个channel不再作为未知项。
- BootROM driver mode已冻结为poll；不假定其timeout/retry策略可跨FMC/GSP复用，reset明确不由security adapter执行。
- `SECURITY_SUBSYS_MAILBOX`已明确不承担eHSM协议转换。除非未来RTL变化形成新冲突并重新裁决，不设计wrapper transport或Vendor公共代码fork。

# Proposed design

## 分层

```text
Boot flow / Security service
  -> eHSM command adapter（稳定的项目 API、错误映射、buffer ownership）
    -> eHSM transport（Mailbox packet、poll/interrupt、timeout/busy）
      -> NGU800P port（MMIO、timer、cache、barrier、status、RAS report）
        -> eHSM/Core
```

EMU/产品 target 不得调用或包含 `ehsm_*_stub`、simulated success 或测试 provider。现有 stub/test/demo 只作为 `CODE_FACT` 和差距，不作为接口目标或验收预期；未接通能力必须不可达、禁用或 fail-close。

## 候选接口族

接口名尚未冻结，先冻结职责：

| 接口职责 | 必须输入 | 必须输出 | 失败要求 |
|---|---|---|---|
| port init | stage、driver mode、平台配置 | 原始 port 状态 | 参数/地址/中断/timer 不完整即失败 |
| read boot status | 指定 status/error register 集 | 原始值、采样时间 | 不在 adapter 内丢弃未知 bit |
| wait ready | 目标条件、deadline、retry/RAS-report policy | 最后状态、耗时、次数 | deadline必须可终止；禁止无限循环和security直接reset |
| start/poll self-test | command context、deadline | command ret、完成状态、原始 bitmap | 当前按Bootloader映射解释bit18=`TRNG`；bit19保持unknown/reserved；始终保留raw |
| read LCS/eFuse | 资源 ID/offset、授权上下文 | 原始值和可信度 | 越界、权限或读错误 fail-close |
| verify/decrypt image | package、size、output region、profile | raw eHSM ret、plaintext extent、metadata | 明确 cache、地址、长度和清零所有权 |
| report eHSM/security fault | 完整`security_error`、raw status、stage、建议RAS action | RAS受理/排队状态、audit ID | 只上报并阻断release；不得直接调用eHSM/system reset |

## Command context 最低字段

- stage/consumer、Mailbox channel、同步/异步模式、deadline。
- command ID/sequence、request/response长度和64位System Address span。
- raw transport ret、raw eHSM ret、最后 status/error registers。
- buffer owner、cache direction、清零责任和 completion 状态。
- retry count、RAS report/action request、开始/结束时间和审计关联ID。

## 错误和恢复原则

- 区分 `PORT`、`TRANSPORT`、`PROTOCOL`、`EHSM_COMMAND`、`POLICY` 和 `CALLER` 来源；保留 raw code。
- 首版`BUSY`不重试；在单在途模型中它表示Owner/旧事务/平台状态异常，service直接进入`QUARANTINED`。
- timeout 后不得假定 eHSM 未执行 command；需要 command-specific completion/recovery 规则。
- security adapter不执行reset。OTP/key/counter/lifecycle等不可逆command必须单独定义掉电/超时后的查询，并把需要的复位/隔离动作提交RAS策略。
- BootROM 阶段任何未批准降级都不得绕过 eHSM 验证后继续 handoff。

## Vendor Mailbox兼容合同

- `ehsm_mb_cmd_st`、`ehsm_mb_rsp_st`、`ehsm_mb_packet_st`的wire语义follow SRC-0018，不在NGU800P公共层重新定义第二套packet。
- 当前eHSM基线开放16个Mailbox channel，Vendor direct Host布局的每channel stride为`0x1000`，因此C908侧direct aperture必须完整覆盖至少`0x10000`。首版并发已冻结为每stage一个context slot、最多一条在途事务；16个硬件channel不等于首版并发目标。
- Vendor交付目录只读。NGU800P不修改`src/mailbox.c`、公共API或其他Vendor源文件；平台差异只通过项目侧custom header、port实现、外部构建选择和adapter处理。若这些正式移植点不足，停止并报告兼容性缺口。
- BootROM/FMC/GSP首版均固定使用`EHSM_DRV_MODE_WAIT_AND_POLL`。通用Host库虽然支持interrupt/send-and-peek，但当前业务层不得启用；GSP interrupt仅作为Vendor FW ready后的后续独立优化。
- Host侧所有packet/cmd/rsp/业务buffer都以baremetal 64位System Address登记到active descriptor；`ehsm_port_addr_to_raddr()`校验完整span后同值返回。反向回调只接受当前active descriptor中的同一System Address。eHSM BL/FW继续使用Vendor `SYS_SOC_MEM_BAL/BAH`和内部remap window；SoC侧不定义第二套映射。
- Ready等待先保存完整raw status；任一`hw_boot_err`或`bootloader_err`立即fail-close并上报。BootROM只有在`hw_boot_done`与`bootloader_done`均为1且相关错误位为0时才进入Mailbox命令阶段。Vendor未规定额外稳定窗口，因此当前不自行发明稳定采样次数；有界deadline属于项目策略，仍待冻结。
- eHSM硬件可以访问整个2 MiB；adapter仍只提交当前command所需的packet/cmd/rsp和业务buffer有效System Address，执行64位range/length/overflow检查。全范围硬件权限不能替代软件参数检查。
- 独立`SEC_RAM_EHSM_MAILBOX`已批准取消。BootROM arena位于GSP低地址启动overlay，FMC arena位于固定128 KiB FMC区，GSP arena位于前880 KiB静态区；不另建固定pool。bulk密文使用Host ingress，明文只写入尚未执行的目标Region。
- 首版context不得作为普通函数局部变量放入栈；每stage使用一个64字节对齐、占256字节的静态slot。首版只使用one-shot typed API，不注册init/update/finish路由，不分配512字节`ehsm_session_st`。
- Mailbox MMIO aperture必须按Vendor port合同映射为non-cacheable/strongly ordered。负责人确认NGU800P的eHSM→SoC RAM访问不携带/不强制可供软件依赖的`non-cacheable`属性；这既不表示eHSM会替C908设置内存属性，也不表示自动cache coherent。
- adapter在Vendor API前登记active transaction的context及全部`EHSM_SHM`对象；Vendor send前钩子在两个批准候选属性下均不执行data clean/invalidate，只执行write/release barrier，在途期间Host禁止访问。Vendor成功返回后adapter对外部output执行acquire/read fence，不执行data invalidate。共享区最终PMA只允许`NON_CACHEABLE`和`HARDWARE_COHERENT`两种候选，待SoC稳定后裁决；裁决和EMU可见性证据完成前，产品端口保持`BLOCKED_BY_PMA_INPUT`，`CACHED_WITH_MAINTENANCE`配置必须拒绝。
- 首版任何Vendor timeout都视为acceptance unknown，自动retry为0；保存packet/channel/raw status并上报RAS，不自动reset或重发。
- timeout、BUSY异常或late response未闭环时，整个service和对应slot进入`QUARANTINED`，不得清零、覆盖、复用或发出下一条命令。

## NGU800P port配置合同

该配置是C908侧编译期/平台注入合同，不是跨阶段Handoff ABI。实现命名可在编码评审时调整，但字段职责不得丢失。

| 配置字段/能力 | 类型/单位 | 规则 |
|---|---|---|
| `mailbox_mmio_cpu_base` | `uint64_t` | 绑定SRC-0022的Vendor direct aperture宏；当前缺失，禁止fallback到4 KiB通用Mailbox或OSR地址 |
| `mailbox_mmio_size` | `uint64_t` | 必须`>= 0x10000` |
| `ehsm_channel_count` | `uint32_t` | 固定16 |
| `channel_stride` | `uint32_t` | 固定`0x1000`，与Vendor公共`mailbox.c`一致 |
| `hsm_status0_cpu_addr` | `uint64_t` | 读取Host可见raw status；地址待RTL输入 |
| `hsm_error_registers` | address/width/clear-semantics表 | 不允许只保留解释后的枚举 |
| `irq_by_channel[16]` | SoC encoded IRQ | 仅interrupt模式使用，必须逐channel匹配 |
| `ehsm_port_addr_to_raddr()` | System Address/length → 同值64位remote | 校验active descriptor、range、length和overflow；local/remap输入拒绝 |
| `ehsm_port_raddr_to_addr()` | 64位remote/length → 同值System Address | 只接受当前active descriptor中的地址，禁止任意反向映射 |
| `cache_line_size` | 字节 | 当前C908基线固定64 |
| `active_cache_scope` | owner + I/O descriptor集合 | adapter在Vendor API前安装；无参数Vendor cache钩子只能维护当前唯一事务 |
| `ehsm_port_flush_and_invalidate_cache()` | Vendor固定无参数ABI | 验证active context和全部共享对象；在两个批准候选属性下不执行data clean/invalidate，只执行write/release fence |
| `adapter_complete_external_rx()` | address/length/direction | Vendor成功返回后、caller读取外部output前完成acquire/read fence，不执行data invalidate；不是新增Vendor公共钩子 |
| `active_timeout_scope` | owner + `timeout_us` | adapter在Vendor API前安装，返回后清除；缺失/为0/嵌套即fail-close |
| `time_now_us()` | `uint64_t` | 单调时钟；复用`csi_tick_get_us()` |
| `report_fault()` | error/raw/stage/action-request | 只上报；不得在port内直接reset |

Port初始化必须对direct base、size、stride、channel count、status地址、timer和当前driver mode所需IRQ做完整性检查。任何缺失在EMU/产品构建或初始化阶段fail-close；不得改用OSR样例地址、4 KiB通用Mailbox、猜测地址、stub或simulated success。

# 已关闭的原Open questions

- 硬件channel数量：当前NGU800P/4019基线为16。
- 地址表达：C908、linker、共享descriptor和eHSM remote字段统一使用baremetal System Address；Host port只做active descriptor范围校验并同值返回。eHSM侧Vendor内部remap不构成SoC软件Local/System映射。
- BootROM模式：固定poll，不采用中断。
- FMC模式：固定poll，不采用中断。
- GSP模式：首版从entry开始，并在Vendor FW加载、验证、启动且满足`firmware_done && !firmware_err`后继续poll。interrupt仅为Vendor FW ready后的后续独立优化；未来若启用，必须重新通过设计变更，并初始化中断控制器、16路映射、ISR/callback、锁和task唤醒路径，清理pending note/interrupt，保证没有未闭环poll transaction或late response。
- Ready位：采用Vendor状态位并执行错误优先门禁；不沿用Host Demo的“任一done位即退出”表达式。
- eHSM访问SoC RAM的属性：负责人确认该访问不携带/不强制软件可依赖的`non-cacheable`属性；Mailbox MMIO固定non-cacheable/strongly ordered，共享RAM则在两个批准PMA候选下只使用barrier且data clean/invalidate调用为0，两者不能混用。
- RAS未ready：按ADR-0010保存静态`EARLY_SECURITY_ERROR_RECORD`、阻断release、撤销权限并尽力清零；有限deadline内只重试RAS上报，超时后记录`RAS_REPORT_UNAVAILABLE`，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出，不自行reset或继续启动。
- Context并发：首版每stage一个256字节slot且最多一条在途；GSP通过唯一service owner/锁/队列串行化。
- Retry/late response：首版自动retry为0；timeout统一视为acceptance unknown并quarantine，迟到响应未闭环前不得复用。

# 剩余项目问题

1. 集成绑定：Vendor direct aperture及Host status/error的SRC-0022准确宏、地址和MMIO属性是什么？这是实现输入，不再开放wrapper/direct方向；4 KiB通用Mailbox不得使用。
2. 共享RAM在C908侧最终采用何种PMA/PBMT/MMU cache属性？context arena的唯一候选固定为`NON_CACHEABLE`和`HARDWARE_COHERENT`，待SoC稳定后裁决并以EMU证明Vendor内部rsp可见性；`CACHED_WITH_MAINTENANCE`不是发布候选。
3. 每类operation的最终`timeout_us`和RAS action request是什么？首版retry和timeout后回收策略已冻结，不再开放。
4. RAS实际ready/notify/report通道和early report deadline数值是什么？无限WFI终态已冻结，不再选择halt原语。
5. 每stage首版service channel ID、one-shot静态I/O descriptor最大数以及由此得到的arena总大小是什么？首版stream session容量固定为0。

# Implementation impact

- DD-02采用双路径，完整边界见[DD-02 eHSM/OSR Host双路径移植](../05-software-design/ehsm-osr-host-porting.md)。
- `baremetal`能力验证路径必须覆盖`ehsm_demo_test()`全部功能/case；无特殊差异时可复用Vendor command拼装，顶层case注册、runner、timeout和结果格式遵守baremetal规则。
- BootROM/FMC/GSP产品路径第一阶段主要使用`bl_demo`所展示的能力，移植/复用OSR Host底层通用函数和command格式；上层payload、调用顺序、安全启动/升级/Debug/SPDM串联由软件方案及专题详设决定。
- 两条路径分别实现目标port，避免OSR地址、timer、直接reset、默认OTP、测试向量和平台策略渗入NGU800P产品公共层；实际改动量由实施调查确认。
- production port不实现供业务层直接调用的reset恢复语义；若为兼容Vendor函数签名保留符号，也必须返回/上报“不由security执行”，实际动作走RAS接口。
- BootROM/FMC/GSP分别提供专用`.ehsm_context_arena` linker section：BootROM位于低地址启动overlay，FMC位于`SEC_RAM_FMC_REUSE`，GSP位于`SEC_RAM_GSP_STATIC`；不另建context pool，不得用普通栈帧承载timeout未决对象。首版不启用异步或流式API。
- 需给 EMU/产品 build 增加 stub/simulated success/test provider 禁入检查和平台配置完整性检查。
- 需扩充统一错误结构，不能只返回当前 `security_status_t` 的单一枚举。
- Vendor Host 通用业务代码已确定为优先移植/复用输入；具体引入/裁剪方式仍需满足 license、ROM 体积、API 稳定性、版本兼容、NGU800P port 和安全门禁评审。

# Verification impact

- Vendor Demo一级覆盖清单保存在[`tests/cases/EHSM-DEMO-CASE-CATALOG.md`](../../tests/cases/EHSM-DEMO-CASE-CATALOG.md)；每个entry仍需展开到独立command/case，不能只运行顶层Demo并以最终打印判定成功。
- Port contract tests：MMIO fake、timer progression、deadline、barrier/cache 调用顺序、地址转换。
- Transport tests：busy、late response、invalid response、wrong channel/sequence、timeout、interrupt/poll parity。
- Context lifetime tests：异步入口不可达、流式init/update/finish路由不可达、timeout后slot不提前复用、并发channel不互相覆盖、cache line不产生false sharing。
- Command tests：自检raw bitmap、LCS error、verify/decrypt failure、RAS上报和禁止security直接reset。
- EMU：真实ready、Mailbox、timeout、status/error快照、RAS动作和错误时禁止下游启动。

# References

- SRC-0016 §2.1～§2.3。
- SRC-0005 §12.2～§12.3、§9.3.41～§9.3.42及状态寄存器表。
- SRC-0012 §2.1、§9.1。
- SRC-0014 Host port地址转换与cache维护接口章节。
- SRC-0018 Host/BL 快照。
- [BootROM 软件设计](../05-software-design/bootrom.md)。
- [CE-SEC-001](../../evidence/code-investigations/CE-SEC-001-bootrom-ehsm-startup.md)。
- [CE-SEC-004](../../evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md)。
- [CE-SEC-005](../../evidence/code-investigations/CE-SEC-005-ehsm-context-memory-lifetime.md)。
- [CE-SEC-006](../../evidence/code-investigations/CE-SEC-006-ehsm-mailbox-vendor-baseline.md)。
- [CE-SEC-007](../../evidence/code-investigations/CE-SEC-007-ngu800p-ehsm-port-binding.md)。
- [安全RAM布局](../03-architecture/security-ram-layout.md)。
- [ADR-0006](../../decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md)。
- [ADR-0008](../../decisions/ADR-0008-ehsm-security-ram-access-boundary.md)。
- [ADR-0009](../../decisions/ADR-0009-baremetal-soc-map-register-authority.md)。
- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)。
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)。
- [eHSM Host Adapter详细合同](ehsm-host-adapter.md)。
- [CE-SEC-008](../../evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md)。
- [CE-SEC-010](../../evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md)。
- [通用Mailbox与eHSM绑定冲突](../../sources/conflict-reports/CONFLICT-NGU800P-GENERIC-VS-EHSM-MAILBOX-MMIO.md)。
- `sources/conflict-reports/CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP.md`。

# Change history

- 2026-07-24：完成CE-SEC-010；冻结16路eHSM IRQ的APLIC 9/source 78～93编码，确认逐channel映射只阻断未来中断模式，并再次确认SRC-0022缺少direct aperture/status/error权威绑定。
- 2026-07-23：接受ADR-0011；冻结每stage单context/单在途、GSP统一service、256字节slot、Vendor兼容active cache/timeout scope、首版零自动retry及timeout quarantine。依据CE-SEC-008校正poll响应前不存在第二个Vendor cache钩子。
- 2026-07-23：接受ADR-0010；OPEN-CONFLICT-007采用Vendor direct Option A，Vendor公共代码保持不变，4 KiB通用Mailbox不用于eHSM；冻结BootROM/FMC/GSP首版全程poll、Vendor FW ready后interrupt仅为后续可选优化，以及RAS未ready早期fail-stop。
- 2026-07-23：接受ADR-0009/SRC-0022作为SoC地址/寄存器权威源；冻结4 KiB/84-message数值，撤回默认64 KiB SoC孔径假设，把OPEN-CONFLICT-007收敛为eHSM wrapper语义和映射问题；该中间结论随后由ADR-0010的Vendor direct裁决取代。
- 2026-07-23：完成NGU800P port只读绑定核对；确认当前4 KiB/84-message通用Mailbox不能替代Vendor 16×4 KiB eHSM Mailbox，建立OPEN-CONFLICT-007；收敛64字节cache line、64位微秒timer、Makefile平台原语和port配置合同，并记录CDK dummy路径缺口。
- 2026-07-23：复核Vendor手册、Host/BL代码和NGU集成头文件，关闭16 channel、地址转换机制、BootROM poll和ready/error位问题；区分Mailbox MMIO non-cacheable与eHSM访问SoC RAM属性，仅保留SoC绑定及项目策略问题。
- 2026-07-23：负责人批准取消独立Mailbox物理Region并采用stage-local固定arena；禁止把通用异步、流式及timeout未闭环context放入普通函数栈。
- 2026-07-28：首版固定one-shot typed API，流式init/update/finish路由必须不可达且不分配`ehsm_session_st`；PMA发布候选仅保留`NON_CACHEABLE`和`HARDWARE_COHERENT`。
- 2026-07-22：依据ADR-0006冻结Mailbox follow Vendor和security只上报/RAS决定reset边界；补充Vendor packet结构、2 MiB共享区关系和production禁止直接reset规则。
- 2026-07-22：按负责人确认拆分baremetal全功能验证与gsp产品移植；前者完整覆盖Vendor Demo case并优先复用组包，后者只复用底层/格式且由实际安全启动链提供payload和编排。
- 2026-07-22：按Vendor回复采用Bootloader自检位图并标记Host定义错误；登记OSR Host通用业务代码复用边界及EMU/产品无stub/simulated success约束。
- 2026-07-21：依据 CE-SEC-001 建立 eHSM adapter/transport/port 首轮分层、接口职责、错误原则和待确认硬件参数；未修改公司代码。
