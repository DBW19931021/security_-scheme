# CE-SEC-008：Vendor Poll路径的Cache、Timeout与迟到响应约束

## Evidence metadata

- Evidence ID：CE-SEC-008
- 日期：2026-07-23
- 父任务：INV-SEC-008
- 资料基线：SRC-0018、SRC-0022；NGU800P公司代码只读工作区
- 适用范围：NGU800P D0 BootROM/FMC/GSP首版eHSM Host adapter
- 操作：只读复核Vendor Host 2.3.1公共API、Mailbox和port合同，以及C908 cache/timer原语；未修改代码仓，未构建/测试，未执行Git命令

## FACT-01：Vendor Context和共享对象生命周期

- `VENDOR_IMPLEMENTATION`：`include/ehsmdrv/basic/api.h:11-17`定义公开`ehsm_ctx_st`为`56 * 4 = 224`字节。
- `VENDOR_IMPLEMENTATION`：`include/ehsmdrv/basic/api.h:22-26`定义流式算法使用的`ehsm_session_st`为512字节，并要求init/update/finish期间持续有效。
- `VENDOR_IMPLEMENTATION`：`src/types_internal.h:17-36`显示context内部包含channel、callback、packet、cmd、rsp和`responded`状态；packet/cmd/rsp不是可在调用返回前随意销毁的临时对象。
- `VENDOR_IMPLEMENTATION`：`src/api.c:48-54`在响应时通过packet地址反推context地址；因此timeout或异步未闭环时复用context会把迟到响应指向新事务。

结论：首版必须使用stage-local静态arena；`ehsm_ctx_st`按64字节对齐并独占256字节slot。Vendor存在512字节流式session能力，但项目首版最终裁决为只使用one-shot typed API，不启用流式路由、不分配session。

## FACT-02：Vendor Poll路径的Timeout由无参数port钩子决定

- `VENDOR_IMPLEMENTATION`：`src/api.c:166-180`在`EHSM_DRV_MODE_WAIT_AND_POLL`中创建timer并反复调用`ehsm_port_is_timeout()`和`ehsm_mb_poll()`。
- `VENDOR_IMPLEMENTATION`：`src/mailbox.c:109-136`在发送前等待`s2h_note`空闲时也使用同一组timer钩子。
- `VENDOR_PORT_CONTRACT`：`include/ehsmdrv/basic/port/ehsm_host_port.h:159-183`把timer定义为64位，但`ehsm_port_is_timeout(timer)`没有command或deadline参数，时限由port实现决定。
- `CODE_FACT`：OSR样例`port/osr/m130/port/ehsm_host_port.c:210-214`恒返回false，不可用于NGU800P production。
- `CODE_FACT`：SRC-0022配套C908 common `src/sys/tick.c:162/239`提供64位`csi_tick_get_us()`。

结论：若不修改Vendor公共代码，要实现command-specific deadline，项目adapter必须在进入Vendor API前安装唯一active timeout policy，port通过该scope解释`ehsm_port_is_timeout()`。首版单在途使这一静态scope可证明安全；若允许并发，现有Vendor port签名无法可靠区分不同command的deadline。

## FACT-03：Vendor Cache钩子只在发送前调用

- `VENDOR_PORT_CONTRACT`：`ehsm_host_port.h:72-80`定义`ehsm_port_flush_and_invalidate_cache()`，并明确它在每次发送Mailbox命令前调用。
- `VENDOR_IMPLEMENTATION`：`src/mailbox.c:122`是Vendor公共代码中唯一一次调用该cache钩子；随后写packet地址和note。
- `VENDOR_IMPLEMENTATION`：poll完成时`src/mailbox.c:139-153`直接调用响应handler；`src/api.c:48-143`直接读取context内的cmd/rsp并回写结果，没有poll响应前的第二个cache invalidate钩子。
- `CODE_FACT`：C908 common提供`csi_dcache_clean_range()`、`soc_dcache_clean_invalid_range()`、`soc_dcache_invalid_range()`和barrier，可用于项目port实现。

结论：

1. 项目port不能设计一个Vendor永远不会调用的“poll返回前cache hook”，也不能修改Vendor `mailbox.c/api.c`增加钩子。
2. adapter在调用Vendor API前登记本事务全部`EHSM_SHM`对象；Vendor的无参数cache钩子维护这些64字节独占范围。
3. context/response cache line在send前必须被invalidate，且Host在事务在途期间不得访问，从而避免把旧line重新带入cache；Mailbox MMIO必须按non-cacheable/strongly ordered属性访问。
4. Vendor API成功返回后，项目wrapper在caller读取外部output前执行范围invalidate和read fence；Vendor内部rsp读取仍依赖“send前invalidate + 在途no-touch + MMIO顺序”合同。
5. Context arena最终只允许`NON_CACHEABLE`或`HARDWARE_COHERENT`，待SoC稳定后唯一裁决；若所选属性不能保证第4点中的Vendor内部rsp读取看到eHSM写回，必须升级平台兼容性问题，不得补丁Vendor公共代码。

## FACT-04：Timeout后无法从公共返回码证明命令是否已执行

- `VENDOR_IMPLEMENTATION`：`EHSM_ERR_TIMEOUT`既可由`ehsm_mb_send()`等待旧note清除超时返回，也可由已提交命令后的poll等待超时返回。
- `VENDOR_IMPLEMENTATION`：公共API没有返回“命令是否已经置note/被eHSM接受”的独立字段，也没有transaction sequence、cancel或abort API。
- `VENDOR_IMPLEMENTATION`：context由packet地址关联，迟到响应仍可能写回原context/rsp/output对象。

结论：首版把所有Vendor `EHSM_ERR_TIMEOUT`视为`acceptance_unknown`，禁止自动重试；对应context、session和共享buffer进入`QUARANTINED`，不得清零、覆盖或复用。security只上报并阻断，reset/recovery由RAS决定。

## FACT-05：首版串行化是Vendor兼容的最小实现

- Vendor硬件有16个channel不等于项目必须同时使用16条事务。
- 单在途允许唯一active cache descriptor、唯一active timeout policy和唯一context owner，避开Vendor无参数port钩子的歧义。
- 首版BootROM、FMC和GSP均使用`WAIT_AND_POLL`；三stage按启动顺序运行，可以分别拥有一个静态slot，并通过平台配置选择channel。

结论：首版冻结每stage一个context slot、每stage最多一条在途事务；GSP通过唯一eHSM service owner/锁/队列串行化调用。首版所有command的`auto_retry_max=0`；`BUSY`在已串行化前提下视为异常占用而不是正常流控。

## 设计回填

- 新增ADR-0011。
- 新增`docs/04-interfaces/ehsm-host-adapter.md`。
- 更新`ehsm-mailbox.md`、`error-handling.md`、OpenSpec约束、open questions、任务和项目状态。

## 限制

- 本报告没有证明2 MiB安全RAM在C908侧的最终PMA/PBMT/MMU属性。
- 本报告没有给出Vendor direct Mailbox/status/error准确MMIO地址。
- 本报告没有给出各operation的最终timeout数值；这些值需来自Vendor适用资料、性能预算和EMU测量。
- 未运行EMU/cache一致性、timeout注入或late-response测试。
